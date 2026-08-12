/*
 * adc_display - LSADC GPIO1_1 / LSADC_CH1 实时电压采样 + ST7789 SPI 屏显示。
 *
 * 硬件：GK7602V11A
 *   AO  接 GPIO1_1 / LSADC_CH1（pad iocfg_reg21 @ 0x12090004）
 *   VCC 接 1.8V（板载 LSADC 参考电压，不可超 3.3V）
 *   GND 接 GND
 *   数据：10-bit ADC，CONTINUOUS 模式，每 100ms 读一次，屏画大字电压。
 *
 * 行为：每 100ms 读一次，屏画大字 "X.XXX V"。Ctrl+C 退出。 -h 打印用法。
 */
#include "st7789.h"
#include "spi_hal.h"
#include "adc.h"
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
 *   LSADC_CH1 = GPIO1_1 = iocfg_reg21 @ 0x12090004  func4 -> 0x1004
 * SPI 屏的 pad 由 spi_hal_init() 内部切。
 */
#define PAGE_SIZE      0x1000u
#define LSADC_CH1_PAD  0x12090004u
#define LSADC_CH1_FUN  0x1004u

static void set_pad_func(uint32_t phys, uint32_t func_val)
{
    int fd = open("/dev/mem", O_RDWR);
    if (fd < 0) {
        perror("[adc] open /dev/mem");
        return;
    }
    volatile uint8_t *m = (volatile uint8_t *)mmap(NULL, PAGE_SIZE,
            PROT_READ | PROT_WRITE, MAP_SHARED, fd, phys & ~(PAGE_SIZE - 1u));
    if (m == MAP_FAILED) {
        perror("[adc] mmap pad");
        close(fd);
        return;
    }
    volatile uint32_t *pad = (volatile uint32_t *)(m + (phys & (PAGE_SIZE - 1u)));
    *pad = func_val;
    printf("[adc] pad 0x%08X -> 0x%08X\n", phys, *pad);
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

/* 屏显：大字电压 + 副标题（通道、原始值）。 */
static void draw_voltage(const adc_data_t *d)
{
    char buf[24];
    const int scale = 4;
    int w;

    st7789_clear();
    st7789_draw_string(0, 0, "LSADC", COLOR_CYAN, COLOR_BLACK);
    snprintf(buf, sizeof(buf), "GPIO1_1  CH%d", ADC_CHN);
    st7789_draw_string(0, 18, buf, COLOR_WHITE, COLOR_BLACK);
    st7789_draw_hline(0, 38, TFT_WIDTH, COLOR_WHITE);

    if (d->mv < 0) {
        draw_big_string(60, 80, "ERR", COLOR_RED, COLOR_BLACK, scale);
    } else {
        /* "1.234 V" 这种格式（用 5 锚点标定值） */
        snprintf(buf, sizeof(buf), "%d.%03d V",
                 d->mv / 1000, d->mv % 1000);
        w = (int)strlen(buf) * FONT8X16_W * scale;
        draw_big_string((TFT_WIDTH - w) / 2, 90, buf, COLOR_WHITE, COLOR_BLACK, scale);
    }

    /* 底部：原始值 */
    snprintf(buf, sizeof(buf), "raw=%4d/4095", d->raw >= 0 ? d->raw : 0);
    st7789_draw_string(0, 218, buf, COLOR_YELLOW, COLOR_BLACK);
    st7789_flush();
}

static void draw_error(const char *msg)
{
    st7789_clear();
    st7789_draw_string(0, 0, "ADC FAIL", COLOR_RED, COLOR_BLACK);
    st7789_draw_string(0, 24, msg, COLOR_WHITE, COLOR_BLACK);
    st7789_draw_string(0, 60, "check:", COLOR_YELLOW, COLOR_BLACK);
    st7789_draw_string(0, 80, "AO=GPIO1_1/LSADC_CH1", COLOR_WHITE, COLOR_BLACK);
    st7789_draw_string(0, 98, "VREF 1.8V", COLOR_WHITE, COLOR_BLACK);
    st7789_draw_string(0, 116, "/dev/lsadc present", COLOR_WHITE, COLOR_BLACK);
    st7789_flush();
}

static void usage(const char *prog)
{
    printf("用法: %s [-h]\n"
           "  每 100ms 读 LSADC GPIO1_1 (CH1) 一次，屏画电压值，Ctrl+C 退出。\n"
           "  VREF/通道在 adc.h 改后重新编译。\n",
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

    /* 切 LSADC_CH1 pad 到 func4(0x1004)。GPIO1_1 默认是 GPIO1_1/func0。 */
    set_pad_func(LSADC_CH1_PAD, LSADC_CH1_FUN);
    /* readback 确认 pad 真切过去了 */
    {
        int fd = open("/dev/mem", O_RDWR);
        if (fd >= 0) {
            volatile uint8_t *m = (volatile uint8_t *)mmap(NULL, PAGE_SIZE,
                    PROT_READ, MAP_SHARED, fd,
                    LSADC_CH1_PAD & ~(PAGE_SIZE - 1u));
            if (m != MAP_FAILED) {
                volatile uint32_t *p = (volatile uint32_t *)(m + (LSADC_CH1_PAD & (PAGE_SIZE - 1u)));
                if (*p != LSADC_CH1_FUN) {
                    fprintf(stderr, "pad 0x%08X readback=0x%08X (期望 0x%08X)\n",
                            LSADC_CH1_PAD, *p, LSADC_CH1_FUN);
                }
                munmap((void *)m, PAGE_SIZE);
            }
            close(fd);
        }
    }

    if (spi_hal_init() < 0) {
        fprintf(stderr, "[adc] SPI 传输层初始化失败。\n");
        return 1;
    }
    st7789_init();

    if (adc_init() < 0) {
        fprintf(stderr, "[adc] ADC 初始化失败，屏上显示排查提示。\n");
        draw_error("open /dev/lsadc failed");
        sleep(3);
        st7789_clear(); st7789_flush();
        st7789_display_on(0); st7789_deinit();
        return 1;
    }

    while (!g_exit) {
        adc_data_t d;
        if (adc_read(&d) == 0) {
            printf("raw=%4d  V=%d.%03d\n",
                   d.raw, d.mv / 1000, d.mv % 1000);
        }
        draw_voltage(&d);
        usleep(100000);
    }

    adc_deinit();
    st7789_clear(); st7789_flush();
    st7789_display_on(0); st7789_deinit();
    printf("[adc] 已清屏关显示，退出。\n");
    return 0;
}