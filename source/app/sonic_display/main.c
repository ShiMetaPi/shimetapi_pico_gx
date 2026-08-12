/*
 * sonic_display - HC-SR04 超声波测距 + ST7789 SPI 屏显示（板端运行）。
 *
 * 硬件：GK7602V11A
 *   HC-SR04：TRIG=GPIO6_7（/dev/gpiochip6 line7，输出脉冲），
 *            ECHO=GPIO7_0（/dev/gpiochip7 line0，输入测脉宽）。
 *   距离显示在 SPI 屏（ST7789 240x240，/dev/spidev2.0，复用 spi_display 传输层）。
 *
 * 行为：每 300ms 测一次距并刷新（距离用 3 倍放大大字显示），Ctrl+C 清屏退出。
 *   -h/--help 打印用法。
 *
 * 注意：HC-SR04 的 ECHO 输出 5V，需分压到 3.3V 再接 GPIO7_0（串 10k+20k 或类似）。
 */
#include "st7789.h"
#include "spi_hal.h"
#include "sonic.h"
#include "rgb.h"
#include "gpio_hal.h"
#include "font8x16.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>

/*
 * pad 复用（无 pinctrl，手写 iocfg bits[3:0] + bit12 输入使能）。查 PIN_OUT 表3：
 *   TRIG = GPIO6_7 = iocfg_reg70 @ 0x100C0068  func0(GPIO6_7) -> 0x1000（默认值即此）
 *   ECHO = GPIO7_0 = iocfg_reg71 @ 0x100C006C  func0(GPIO7_0) -> 0x1000（默认值即此）
 * 两个脚默认就是 GPIO，开箱可用；显式写一遍保险。
 */
#define PAGE_SIZE  0x1000u
#define TRIG_PAD   0x100C0068u  /* GPIO6_7 */
#define ECHO_PAD   0x100C006Cu  /* GPIO7_0 */
#define RGB_R_PAD  0x100C0064u  /* GPIO6_6 = iocfg_reg69，func0(GPIO) */
#define RGB_G_PAD  0x100C0060u  /* GPIO6_5 = iocfg_reg68，func0(GPIO) */
#define RGB_B_PAD  0x100C0038u  /* GPIO5_3 = iocfg_reg58，func0(GPIO) */
#define GPIO_FUNC  0x1000u      /* func0(GPIO) + bit12 输入使能 */

static void set_pad_func(uint32_t phys, uint32_t func_val)
{
    int fd = open("/dev/mem", O_RDWR);
    if (fd < 0) {
        perror("[sonic] open /dev/mem");
        return;
    }
    volatile uint8_t *m = (volatile uint8_t *)mmap(NULL, PAGE_SIZE,
            PROT_READ | PROT_WRITE, MAP_SHARED, fd, phys & ~(PAGE_SIZE - 1u));
    if (m == MAP_FAILED) {
        perror("[sonic] mmap pad");
        close(fd);
        return;
    }
    volatile uint32_t *pad = (volatile uint32_t *)(m + (phys & (PAGE_SIZE - 1u)));
    *pad = func_val;
    printf("[sonic] pad 0x%08X -> 0x%08X\n", phys, *pad);
    munmap((void *)m, PAGE_SIZE);
    close(fd);
}

static void padmux_init(void)
{
    set_pad_func(TRIG_PAD, GPIO_FUNC);
    set_pad_func(ECHO_PAD, GPIO_FUNC);
    set_pad_func(RGB_R_PAD, GPIO_FUNC);
    set_pad_func(RGB_G_PAD, GPIO_FUNC);
    set_pad_func(RGB_B_PAD, GPIO_FUNC);
}

static volatile sig_atomic_t g_exit = 0;

static void on_signal(int sig)
{
    (void)sig;
    g_exit = 1;
}

/* 基于 font8x16 字模放大 scale 倍画一个字符（前景 fg / 背景 bg）。 */
static void draw_big_char(int x, int y, char ch, uint16_t fg, uint16_t bg, int scale)
{
    unsigned int idx = (unsigned char)ch;
    const uint8_t *glyph;

    if (idx < FONT8X16_FIRST || idx >= FONT8X16_FIRST + FONT8X16_COUNT) {
        idx = FONT8X16_FIRST;   /* 非可打印 -> 空格 */
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

/* 放大字符串。 */
static void draw_big_string(int x, int y, const char *s, uint16_t fg, uint16_t bg, int scale)
{
    int cx = x;
    for (; s != NULL && *s != '\0'; s++) {
        draw_big_char(cx, y, *s, fg, bg, scale);
        cx += FONT8X16_W * scale;
    }
}

/* 测距结果页：标题 + 大字距离 + 状态 + LED 颜色 + 公式。color 为 rgb_by_distance 返回的颜色名。 */
static void draw_measure(float d, const char *color)
{
    char dbuf[16];
    const int scale = 3;
    const char *status;
    uint16_t stcolor, dcolor;

    st7789_clear();
    st7789_draw_string(0, 0, "SONIC HC-SR04", COLOR_CYAN, COLOR_BLACK);
    st7789_draw_string(0, 20, "TRIG=6_7 ECHO=7_0", COLOR_WHITE, COLOR_BLACK);
    st7789_draw_hline(0, 40, TFT_WIDTH, COLOR_WHITE);
    st7789_draw_string(0, 66, "DISTANCE", COLOR_YELLOW, COLOR_BLACK);

    if (d < 0) {
        snprintf(dbuf, sizeof(dbuf), "--.-");
        dcolor = COLOR_RED;
        status = (d == SONIC_ERR_NO_ECHO) ? "no echo" : "out of range";
        stcolor = COLOR_RED;
    } else {
        snprintf(dbuf, sizeof(dbuf), "%.1f", d);
        dcolor = (d < 5.0f) ? COLOR_RED : ((d > 100.0f) ? COLOR_YELLOW : COLOR_GREEN);
        status = "in range";
        stcolor = COLOR_GREEN;
    }

    int w = (int)strlen(dbuf) * FONT8X16_W * scale;
    int x = (TFT_WIDTH - w) / 2;
    if (x < 0) {
        x = 0;
    }
    draw_big_string(x, 92, dbuf, dcolor, COLOR_BLACK, scale);

    st7789_draw_string(0, 150, "cm", COLOR_YELLOW, COLOR_BLACK);
    st7789_draw_string(0, 178, status, stcolor, COLOR_BLACK);

    char led[24];
    snprintf(led, sizeof(led), "LED %s", color);
    st7789_draw_string(0, 198, led, COLOR_MAGENTA, COLOR_BLACK);

    st7789_draw_string(0, 220, "v=340m/s  cm=us/58", COLOR_BLUE, COLOR_BLACK);

    st7789_flush();
}

static void draw_error(const char *msg)
{
    st7789_clear();
    st7789_draw_string(0, 0, "SONIC FAIL", COLOR_RED, COLOR_BLACK);
    st7789_draw_string(0, 24, msg, COLOR_WHITE, COLOR_BLACK);
    st7789_draw_string(0, 60, "check:", COLOR_YELLOW, COLOR_BLACK);
    st7789_draw_string(0, 80, "TRIG=GPIO6_7", COLOR_WHITE, COLOR_BLACK);
    st7789_draw_string(0, 98, "ECHO=GPIO7_0", COLOR_WHITE, COLOR_BLACK);
    st7789_draw_string(0, 116, "ECHO 5V->3.3V", COLOR_WHITE, COLOR_BLACK);
    st7789_draw_string(0, 134, "VCC 5V GND", COLOR_WHITE, COLOR_BLACK);
    st7789_flush();
}

static void usage(const char *prog)
{
    printf("用法: %s [-h]\n"
           "  每 300ms 测一次距并刷新 SPI 屏，Ctrl+C 退出。\n"
           "  引脚在 sonic.h、main.c 顶部修改后重新编译。\n",
           prog);
}

int main(int argc, char **argv)
{
    struct sigaction sa;

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

    printf("[sonic] pad 复用：GPIO6_7/GPIO7_0 -> func0(GPIO)\n");
    padmux_init();

    printf("[sonic] 初始化 SPI 屏（/dev/spidev2.0）...\n");
    if (spi_hal_init() < 0) {
        fprintf(stderr, "[sonic] SPI 传输层初始化失败。\n");
        return 1;
    }
    st7789_init();

    printf("[sonic] 初始化 HC-SR04（TRIG/ECHO）...\n");
    if (sonic_init() < 0) {
        fprintf(stderr, "[sonic] TRIG/ECHO chardev 申请失败，屏上显示排查提示。\n");
        draw_error("TRIG/ECHO init fail");
        sleep(3);
        st7789_clear();
        st7789_flush();
        st7789_display_on(0);
        st7789_deinit();
        return 1;
    }

    printf("[sonic] 初始化 RGB（R=6_6 G=6_5 B=5_3）...\n");
    if (rgb_init() < 0) {
        fprintf(stderr, "[sonic] RGB 初始化失败（继续，RGB 灯不工作）。\n");
    }

    printf("[sonic] 测距中，每 80ms 刷新，Ctrl+C 退出。\n");
    while (!g_exit) {
        float d = sonic_measure_cm();
        if (d >= 0) {
            printf("[sonic] %6.1f cm\n", d);
        } else if (d == SONIC_ERR_NO_ECHO) {
            printf("[sonic] no echo\n");
        } else {
            printf("[sonic] out of range\n");
        }
        const char *color = rgb_by_distance(d);   /* 按距离设 RGB 并返回颜色名 */
        draw_measure(d, color);
        usleep(80000);   /* ~10fps；≥60ms 满足 HC-SR04 防回波串扰 */
    }

    rgb_deinit();
    sonic_deinit();
    st7789_clear();
    st7789_flush();
    st7789_display_on(0);
    st7789_deinit();
    printf("[sonic] 已清屏关显示，退出。\n");
    return 0;
}
