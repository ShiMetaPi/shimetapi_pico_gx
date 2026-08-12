/*
 * motor - TB6612 双路直流电机控制 + ST7789 SPI 屏显（板端运行）。
 *
 * 硬件：GK7602V11A / xm7206v11a / xmorca
 *   TB6612 控制（GPIO chardev + 软件位带 PWM via pthread）：
 *     PWMA = GPIO5_4   AIN1 = GPIO7_2  AIN2 = GPIO7_4  STBY = GPIO5_2
 *     PWMB = GPIO5_5   BIN1 = GPIO6_5  BIN2 = GPIO6_6
 *   屏显：复用 spi_display 的 SPI 屏传输层（/dev/spidev2.0）画大字状态。
 *
 * 控制方式（stdin 单字符命令，敲完回车）：
 *   w  两路前进       s  两路后退       a  左转（后退/前进）
 *   d  右转（前进/后退） 空格 全部停止
 *   + / =  加 5% 速度（上限 100）   -  减 5% 速度（下限 0）
 *   0..9 直接设速度百分比（0=停，9=90%）
 *   b  BRAKE（短路刹车）  c  COAST（高阻滑行）
 *   q  退出
 *
 * 屏显：大字写"前进 F 50%" / "后退 R 75%" / "停止 STOP 0%" 等。
 * Ctrl+C 也退出，退出前自动 standby。
 */
#include "st7789.h"
#include "spi_hal.h"
#include "tb6612.h"
#include "font8x16.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>

static volatile sig_atomic_t g_exit = 0;
static void on_signal(int sig) { (void)sig; g_exit = 1; }

/* 大字辅助（复用 sht20_display 的画法）。 */
static void draw_big_char(int x, int y, char ch, uint16_t fg, uint16_t bg, int scale)
{
    unsigned int idx = (unsigned char)ch;
    const uint8_t *glyph;
    if (idx < FONT8X16_FIRST || idx >= FONT8X16_FIRST + FONT8X16_COUNT) {
        idx = FONT8X16_FIRST;
    }
    glyph = font8x16[idx - FONT8X16_FIRST];
    for (int row = 0; row < FONT8X16_H_; row++) {
        uint8_t line = glyph[row];
        for (int col = 0; col < FONT8X16_W; col++) {
            uint16_t c = (line & (uint8_t)(0x80u >> col)) ? fg : bg;
            st7789_fill_rect(x + col * scale, y + row * scale, scale, scale, c);
        }
    }
}
static void draw_big_string(int x, int y, const char *s, uint16_t fg, uint16_t bg, int scale)
{
    int cx = x;
    for (; s != NULL && *s != '\0'; s++) {
        draw_big_char(cx, y, *s, fg, bg, scale);
        cx += FONT8X16_W * scale;
    }
}

/* 状态：当前方向 + 速度（两路共用）。 */
typedef struct {
    tb6612_dir_t dir;
    int          speed;        /* 0..100 */
} motor_state_t;

static const char *dir_name(tb6612_dir_t d)
{
    switch (d) {
        case TB6612_FWD:   return "FWD";
        case TB6612_REV:   return "REV";
        case TB6612_BRAKE: return "BRK";
        case TB6612_COAST: return "COAST";
    }
    return "?";
}

/* 把两路电机设成指定 (方向, 速度)。dir=COAST+BRAKE 时把速度锁 0（CHIP 安全）。 */
static void apply_motor(const motor_state_t *st)
{
    int sp = st->dir == TB6612_COAST || st->dir == TB6612_BRAKE ? 0 : st->speed;
    tb6612_set_both(st->dir, sp, st->dir, sp);
}

/* 大字屏显："FWD 50%" / "REV 75%" / "BRK 100%" / "COAST  0%" */
static void draw_state(const motor_state_t *st)
{
    char buf[32];
    const int scale = 4;
    int w;
    int v = (st->dir == TB6612_COAST || st->dir == TB6612_BRAKE) ? 0 : st->speed;

    st7789_clear();
    st7789_draw_string(0, 0, "TB6612 MOTOR", COLOR_CYAN, COLOR_BLACK);

    /* 副标题：TB6612 → PWMA/PWMB → GPIO5_4/GPIO5_5 */
    st7789_draw_string(0, 18, "PWMA:GPIO5_4  PWMB:GPIO5_5", COLOR_WHITE, COLOR_BLACK);
    st7789_draw_hline(0, 38, TFT_WIDTH, COLOR_WHITE);

    /* 大字：A 路方向 */
    snprintf(buf, sizeof(buf), "%s", dir_name(st->dir));
    w = (int)strlen(buf) * FONT8X16_W * scale;
    draw_big_string((TFT_WIDTH - w) / 2, 70, buf,
                    st->dir == TB6612_FWD   ? COLOR_GREEN  :
                    st->dir == TB6612_REV   ? COLOR_RED    :
                    st->dir == TB6612_BRAKE ? COLOR_YELLOW :
                                              COLOR_WHITE,
                    COLOR_BLACK, scale);

    /* 大字：速度百分比 */
    snprintf(buf, sizeof(buf), "%d%%", v);
    w = (int)strlen(buf) * FONT8X16_W * scale;
    draw_big_string((TFT_WIDTH - w) / 2, 150, buf, COLOR_WHITE, COLOR_BLACK, scale);

    /* 底部 IN1/IN2 状态 */
    snprintf(buf, sizeof(buf), "AIN1=%d AIN2=%d  BIN1=%d BIN2=%d",
             st->dir == TB6612_FWD || st->dir == TB6612_BRAKE,
             st->dir == TB6612_REV || st->dir == TB6612_BRAKE,
             st->dir == TB6612_FWD || st->dir == TB6612_BRAKE,
             st->dir == TB6612_REV || st->dir == TB6612_BRAKE);
    st7789_draw_string(0, 220, buf, COLOR_YELLOW, COLOR_BLACK);

    st7789_flush();
}

static void usage(const char *prog)
{
    printf("用法: %s [-h]\n"
           "TB6612 双路直流电机控制 + ST7789 SPI 屏显示状态。\n"
           "stdin 命令（每行一字符 + 回车）：\n"
           "  w 前进      s 后退      a 左转      d 右转\n"
           "  空格 停止   b BRAKE   c COAST\n"
           "  + / = 加 5%% (上限 100)   - 减 5%% (下限 0)\n"
           "  0..9 直接设速度\n"
           "  q 退出\n",
           prog);
}

int main(int argc, char **argv)
{
    struct sigaction sa;
    motor_state_t st = { TB6612_COAST, 0 };
    char line[16];

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage(argv[0]);
            return 0;
        }
    }
    setvbuf(stdout, NULL, _IONBF, 0);
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    /* 屏蔽管道/终端断开的 SIGPIPE，防止 stdin 关闭时崩。 */
    signal(SIGPIPE, SIG_IGN);

    printf("[motor] 初始化 SPI 屏...\n");
    if (spi_hal_init() < 0) {
        fprintf(stderr, "[motor] SPI 屏初始化失败\n");
        return 1;
    }
    st7789_init();

    printf("[motor] 初始化 TB6612（7 路 GPIO + 软件 PWM 线程）...\n");
    if (tb6612_init() < 0) {
        fprintf(stderr, "[motor] TB6612 初始化失败\n");
        st7789_clear(); st7789_flush();
        st7789_display_on(0); st7789_deinit();
        return 1;
    }

    /* 显示初始状态 */
    draw_state(&st);
    printf("[motor] 就绪：方向=COAST 速度=0%%。敲命令（回车确认）：\n");

    /* 主循环：读 stdin 一行一字符。 */
    while (!g_exit) {
        printf("> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) {
            /* stdin 关闭（Ctrl+D 或 ssh 断开） */
            break;
        }
        /* 跳过空白，取第一个非空白字符 */
        char cmd = '\0';
        for (int i = 0; line[i]; i++) {
            if (!isspace((unsigned char)line[i])) {
                cmd = line[i];
                break;
            }
        }
        if (cmd == '\0') continue;

        switch (cmd) {
            case 'w': st.dir = TB6612_FWD;   break;
            case 's': st.dir = TB6612_REV;   break;
            case 'a': /* 左转：A 反，B 正 */
                tb6612_set_motor_a(TB6612_REV, st.speed > 0 ? st.speed : 50);
                tb6612_set_motor_b(TB6612_FWD, st.speed > 0 ? st.speed : 50);
                st.dir = TB6612_FWD;  /* 屏上只显示一路 */
                draw_state(&st);
                continue;
            case 'd': /* 右转：A 正，B 反 */
                tb6612_set_motor_a(TB6612_FWD, st.speed > 0 ? st.speed : 50);
                tb6612_set_motor_b(TB6612_REV, st.speed > 0 ? st.speed : 50);
                st.dir = TB6612_FWD;
                draw_state(&st);
                continue;
            case ' ': st.dir = TB6612_COAST; st.speed = 0; break;
            case 'b': st.dir = TB6612_BRAKE; st.speed = 0; break;
            case 'c': st.dir = TB6612_COAST; st.speed = 0; break;
            case '+':
            case '=': st.speed += 5; if (st.speed > 100) st.speed = 100; break;
            case '-': st.speed -= 5; if (st.speed < 0)   st.speed = 0;   break;
            case 'q': g_exit = 1; continue;
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                st.speed = (cmd - '0') * 10;
                if (st.speed < 0) st.speed = 0;
                if (st.speed > 100) st.speed = 100;
                break;
            default:
                printf("[motor] 未知命令 '%c'\n", cmd);
                continue;
        }
        apply_motor(&st);
        printf("[motor] 方向=%s 速度=%d%%\n", dir_name(st.dir), st.speed);
        draw_state(&st);
    }

    /* 退出清理 */
    printf("[motor] 退出：拉低 STBY、停 PWM、关 GPIO...\n");
    tb6612_standby(0);
    tb6612_deinit();
    st7789_clear(); st7789_flush();
    st7789_display_on(0); st7789_deinit();
    printf("[motor] 完成\n");
    return 0;
}
