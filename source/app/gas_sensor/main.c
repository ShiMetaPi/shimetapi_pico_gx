/*
 * gas_sensor - MQ2 气体传感器 + ST7789 SPI 屏显示（板端运行）。
 *
 * 硬件：GK7606V11A
 *   MQ2 DO 接 GPIO0_1（chardev 输入读电平）
 *   MQ2 AO 接 GPIO1_0 / LSADC_CH0（/dev/mem mmap LSADC 寄存器，绕过驱动限制读 10-bit 值）
 *   数据显示在 SPI 屏（ST7789 240x240，/dev/spidev2.0，复用 spi_display 传输层）
 *
 * 屏上：大字 DO 状态 + AO 原始值/百分比 + GAS 报警提示。Ctrl+C 退出。
 *
 * 注：MQ2 模块上的 DO 电平极性（高=有气/低=有气）取决于模块电路设计，
 *     这里以 DO=1 当作"超阈值"画红色 ALERT，DO=0 当作 NORMAL 画绿色。
 */
#include "st7789.h"
#include "spi_hal.h"
#include "mq2.h"
#include "font8x16.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>

/*
 * pad 复用（查 7206V11A PIN_OUT 表3）：
 *   GPIO0_1        = iocfg_reg1  @ 0x112C0004  func0=GPIO（默认即 GPIO，不用切）
 *   LSADC_CH0 pad  = iocfg_reg20 @ 0x12090000  func4=LSADC_CH0 -> 0x1004
 *   （用户要求 AO 接 GPIO1_0，但 xm_lsadc.ko 没编 USE_LSADC_CHANNEL_0，ioctl 走 chn=0
 *    会被驱动的 default: 分支拒收。所以这里开 /dev/lsadc 让驱动帮忙使能时钟 + 退出
 *    reset，再用 /dev/mem mmap LSADC 寄存器自己 enable CH0 + start + read + stop。）
 *
 * SPI 屏的 pad（4_4/4_5/4_7/5_0/5_1）由 spi_hal_init() 内部切，这里不管。
 */
#define PAGE_SIZE     0x1000u
#define LSADC_CH0_PAD 0x12090000u
#define LSADC_CH0_FUN 0x1004u

static void set_pad_func(uint32_t phys, uint32_t func_val)
{
    int fd = open("/dev/mem", O_RDWR);
    if (fd < 0) {
        perror("[mq2] open /dev/mem");
        return;
    }
    volatile uint8_t *m = (volatile uint8_t *)mmap(NULL, PAGE_SIZE,
            PROT_READ | PROT_WRITE, MAP_SHARED, fd, phys & ~(PAGE_SIZE - 1u));
    if (m == MAP_FAILED) {
        perror("[mq2] mmap pad");
        close(fd);
        return;
    }
    volatile uint32_t *pad = (volatile uint32_t *)(m + (phys & (PAGE_SIZE - 1u)));
    *pad = func_val;
    printf("[mq2] pad 0x%08X -> 0x%08X\n", phys, *pad);
    munmap((void *)m, PAGE_SIZE);
    close(fd);
}

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

/* 屏显：标题 + 大字 DO/AO + GAS 报警色块。 */
static void draw_gas(const mq2_data_t *d)
{
    char buf[24];
    const int scale = 3;
    int w;

    st7789_clear();
    st7789_draw_string(0, 0,  "MQ2 GAS", COLOR_CYAN, COLOR_BLACK);
    st7789_draw_string(0, 20, "ADC0/GPIO1_0", COLOR_WHITE, COLOR_BLACK);
    st7789_draw_hline(0, 40, TFT_WIDTH, COLOR_WHITE);

    /* DO 行 */
    st7789_draw_string(0, 50, "DO", COLOR_YELLOW, COLOR_BLACK);
    snprintf(buf, sizeof(buf), "GPIO0_1=%d", d->do_level);
    st7789_draw_string(48, 50, buf, COLOR_WHITE, COLOR_BLACK);

    /* AO 行 */
    st7789_draw_string(0, 70, "AO", COLOR_YELLOW, COLOR_BLACK);
    if (d->ao_err == 0) {
        snprintf(buf, sizeof(buf), "CH0=%4d", d->ao_raw);
    } else {
        snprintf(buf, sizeof(buf), "CH0=ERR");
    }
    st7789_draw_string(48, 70, buf, COLOR_WHITE, COLOR_BLACK);

    /* 大字百分比 */
    if (d->ao_err == 0) {
        snprintf(buf, sizeof(buf), "%d%%", d->ao_pct);
    } else {
        snprintf(buf, sizeof(buf), "--");
    }
    w = (int)strlen(buf) * FONT8X16_W * scale;
    draw_big_string((TFT_WIDTH - w) / 2, 100, buf, COLOR_WHITE, COLOR_BLACK, scale);

    /* GAS 状态：阈值判定，>70% 红警，<=70% 绿正常（阈值可改） */
    {
        const int TH = 70;
        int alert = (d->ao_err == 0 && d->ao_pct >= TH);
        uint16_t c = alert ? COLOR_RED : COLOR_GREEN;
        const char *msg = alert ? "ALERT" : "NORMAL";
        st7789_draw_string(0, 168, "GAS", COLOR_YELLOW, COLOR_BLACK);
        w = (int)strlen(msg) * FONT8X16_W * 3;
        draw_big_string((TFT_WIDTH - w) / 2, 190, msg, c, COLOR_BLACK, 3);
    }

    st7789_draw_string(0, 224, "Ctrl+C quit", COLOR_BLUE, COLOR_BLACK);
    st7789_flush();
}

static void draw_error(const char *msg)
{
    st7789_clear();
    st7789_draw_string(0, 0, "MQ2 FAIL", COLOR_RED, COLOR_BLACK);
    st7789_draw_string(0, 24, msg, COLOR_WHITE, COLOR_BLACK);
    st7789_draw_string(0, 60, "check:", COLOR_YELLOW, COLOR_BLACK);
    st7789_draw_string(0, 80, "DO=GPIO0_1", COLOR_WHITE, COLOR_BLACK);
    st7789_draw_string(0, 98, "AO=GPIO1_0/LSADC_CH0", COLOR_WHITE, COLOR_BLACK);
    st7789_draw_string(0, 116, "VCC 5V (MQ2 heater)", COLOR_WHITE, COLOR_BLACK);
    st7789_flush();
}

static void usage(const char *prog)
{
    printf("用法: %s [-h]\n"
           "  每 200ms 读 MQ2 DO/AO 并刷新 SPI 屏，Ctrl+C 退出。\n"
           "  地址/阈值在 mq2.h、main.c 顶部修改后重新编译。\n",
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

    /* 切 LSADC_CH0 pad 到 func4(0x1004)。GPIO0_1 默认就是 GPIO，不用切。 */
    printf("[mq2] pad 复用：LSADC_CH0 pad (GPIO1_0) -> func4\n");
    set_pad_func(LSADC_CH0_PAD, LSADC_CH0_FUN);

    printf("[mq2] 初始化 SPI 屏（/dev/spidev2.0）...\n");
    if (spi_hal_init() < 0) {
        fprintf(stderr, "[mq2] SPI 传输层初始化失败。\n");
        return 1;
    }
    st7789_init();

    printf("[mq2] 初始化 MQ2（DO=GPIO0_1, AO=GPIO1_0/LSADC_CH0）...\n");
    if (mq2_init() < 0) {
        fprintf(stderr, "[mq2] MQ2 初始化失败，屏上显示排查提示。\n");
        draw_error("open /dev/gpiochip0 or /dev/lsadc failed");
        sleep(3);
        st7789_clear(); st7789_flush();
        st7789_display_on(0); st7789_deinit();
        return 1;
    }

    printf("[mq2] 采集中，每 200ms 刷新，Ctrl+C 退出。\n");
    while (!g_exit) {
        mq2_data_t d;
        if (mq2_read(&d) == 0) {
            printf("[mq2] DO=%d AO=%d (%d%%)\n", d.do_level, d.ao_raw, d.ao_pct);
        }
        draw_gas(&d);
        usleep(200000);
    }

    mq2_deinit();
    st7789_clear(); st7789_flush();
    st7789_display_on(0); st7789_deinit();
    printf("[mq2] 已清屏关显示，退出。\n");
    return 0;
}