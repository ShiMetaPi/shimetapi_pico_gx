/*
 * sht20_display - SHT20 温湿度 + ST7789 SPI 屏显示（板端运行）。
 *
 * 硬件：GK7602V11A
 *   SHT20/SI7021/HTU21 接 I2C3（GPIO4_1=SCL, GPIO4_2=SDA -> /dev/i2c-3），地址 0x40，
 *   与 OLED(0x3C)/MPU6050(0x68) 共享总线。
 *   温湿度显示在 SPI 屏（ST7789 240x240，/dev/spidev2.0，复用 spi_display 传输层）。
 *
 * 行为：每 2s 读一次温湿度，大字显示，Ctrl+C 清屏退出。 -h 打印用法。
 * I2C 访问完全效仿 source/app/oled_display：开 fd + I2C_SLAVE 绑地址 + 直接写命令。
 */
#include "st7789.h"
#include "spi_hal.h"
#include "sht20.h"
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
 * I2C3 pad 复用（同 oled_display/mpu6050_display）：
 *   I2C3_SCL = GPIO4_1 @ 0x100C0010  func2 -> 0x1002
 *   I2C3_SDA = GPIO4_2 @ 0x100C0014  func2 -> 0x1002
 */
#define PAGE_SIZE     0x1000u
#define I2C3_SCL_PAD  0x100C0010u
#define I2C3_SDA_PAD  0x100C0014u
#define I2C3_FUNC_VAL 0x1002u

static void set_pad_func(uint32_t phys, uint32_t func_val)
{
    int fd = open("/dev/mem", O_RDWR);
    if (fd < 0) {
        perror("[sht20] open /dev/mem");
        return;
    }
    volatile uint8_t *m = (volatile uint8_t *)mmap(NULL, PAGE_SIZE,
            PROT_READ | PROT_WRITE, MAP_SHARED, fd, phys & ~(PAGE_SIZE - 1u));
    if (m == MAP_FAILED) {
        perror("[sht20] mmap pad");
        close(fd);
        return;
    }
    volatile uint32_t *pad = (volatile uint32_t *)(m + (phys & (PAGE_SIZE - 1u)));
    *pad = func_val;
    printf("[sht20] pad 0x%08X -> 0x%08X\n", phys, *pad);
    munmap((void *)m, PAGE_SIZE);
    close(fd);
}

static void padmux_init(void)
{
    set_pad_func(I2C3_SCL_PAD, I2C3_FUNC_VAL);
    set_pad_func(I2C3_SDA_PAD, I2C3_FUNC_VAL);
}

static volatile sig_atomic_t g_exit = 0;

static void on_signal(int sig)
{
    (void)sig;
    g_exit = 1;
}

/* 基于 font8x16 字模放大 scale 倍画字符（前景 fg / 背景 bg）。 */
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

/* 温湿度页：温度、湿度各一块大字。err!=0 表示读取失败，显示 --.- 红。 */
static void draw_th(const sht20_data_t *d, int err)
{
    char buf[24];
    const int scale = 3;
    int w;

    st7789_clear();
    snprintf(buf, sizeof(buf), "SHT20 @0x%02X", SHT20_ADDR);
    st7789_draw_string(0, 0, buf, COLOR_CYAN, COLOR_BLACK);
    st7789_draw_string(0, 20, "I2C3", COLOR_WHITE, COLOR_BLACK);
    st7789_draw_hline(0, 40, TFT_WIDTH, COLOR_WHITE);

    st7789_draw_string(0, 56, "TEMP  C", COLOR_YELLOW, COLOR_BLACK);
    if (err) {
        snprintf(buf, sizeof(buf), "--.-");
    } else {
        snprintf(buf, sizeof(buf), "%.1f", d->temp);
    }
    w = (int)strlen(buf) * FONT8X16_W * scale;
    draw_big_string((TFT_WIDTH - w) / 2, 82, buf,
                    err ? COLOR_RED : COLOR_WHITE, COLOR_BLACK, scale);

    st7789_draw_string(0, 138, "HUMI %RH", COLOR_YELLOW, COLOR_BLACK);
    if (err) {
        snprintf(buf, sizeof(buf), "--.-");
    } else {
        snprintf(buf, sizeof(buf), "%.1f", d->rh);
    }
    w = (int)strlen(buf) * FONT8X16_W * scale;
    draw_big_string((TFT_WIDTH - w) / 2, 164, buf,
                    err ? COLOR_RED : COLOR_WHITE, COLOR_BLACK, scale);

    st7789_draw_string(0, 224, "Ctrl+C quit", COLOR_BLUE, COLOR_BLACK);
    st7789_flush();
}

static void draw_error(void)
{
    st7789_clear();
    st7789_draw_string(0, 0, "SHT20 FAIL", COLOR_RED, COLOR_BLACK);
    st7789_draw_string(0, 20, "no ACK @0x40", COLOR_WHITE, COLOR_BLACK);
    st7789_draw_string(0, 48, "check wire:", COLOR_YELLOW, COLOR_BLACK);
    st7789_draw_string(0, 68, "SCL=GPIO4_1", COLOR_WHITE, COLOR_BLACK);
    st7789_draw_string(0, 86, "SDA=GPIO4_2", COLOR_WHITE, COLOR_BLACK);
    st7789_draw_string(0, 104, "ADDR 0x40  VCC3.3V", COLOR_WHITE, COLOR_BLACK);
    st7789_flush();
}

static void usage(const char *prog)
{
    printf("用法: %s [-h]\n"
           "  每 2s 读一次温湿度并刷新 SPI 屏，Ctrl+C 退出。\n"
           "  总线/地址在 sht20.h、引脚在 main.c 顶部修改后重新编译。\n",
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

    printf("[sht20] pad 复用：I2C3(4_1/4_2) -> func2\n");
    padmux_init();

    printf("[sht20] 初始化 SPI 屏（/dev/spidev2.0）...\n");
    if (spi_hal_init() < 0) {
        fprintf(stderr, "[sht20] SPI 传输层初始化失败。\n");
        return 1;
    }
    st7789_init();

    printf("[sht20] 初始化 SHT20（/dev/i2c-%d addr 0x%02x）...\n", SHT20_BUS, SHT20_ADDR);
    if (sht20_init() < 0) {
        fprintf(stderr, "[sht20] SHT20 初始化失败，屏上显示排查提示。\n");
        draw_error();
        sleep(3);
        st7789_clear();
        st7789_flush();
        st7789_display_on(0);
        st7789_deinit();
        return 1;
    }

    printf("[sht20] 采集中，每 2s 刷新，Ctrl+C 退出。\n");
    while (!g_exit) {
        sht20_data_t d;
        int ok = (sht20_read(&d) == 0);
        if (ok) {
            printf("[sht20] T=%.1f C  RH=%.1f %%\n", d.temp, d.rh);
        }
        draw_th(&d, !ok);
        sleep(2);
    }

    sht20_deinit();
    st7789_clear();
    st7789_flush();
    st7789_display_on(0);
    st7789_deinit();
    printf("[sht20] 已清屏关显示，退出。\n");
    return 0;
}