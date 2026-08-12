/*
 * gps_display - GPS（UART1 NMEA）+ ST7789 SPI 屏显示（板端运行）。
 *
 * 硬件：GK7602V11A
 *   GPS 模块接 UART1：GPS-TX -> GPIO3_6(UART1_RXD)，GPS-RX -> GPIO3_5(UART1_TXD)，
 *   即 /dev/ttyAMA1，9600 8N1。PPS -> GPIO4_6（/dev/gpiochip4 line6，输入）。
 *   定位参数显示在 SPI 屏（ST7789 240x240，/dev/spidev2.0，复用 spi_display 传输层）。
 *
 * 行为：持续读 NMEA 行解析，每 500ms 用最新数据刷新屏幕，Ctrl+C 清屏退出。
 *   -h/--help 打印用法。
 *
 * 注意：GPS 冷启动需到窗户/室外才能定位，室内通常 NO FIX（正常）。
 */
#include "st7789.h"
#include "spi_hal.h"
#include "uart_hal.h"
#include "nmea.h"
#include "gpio_hal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>

/*
 * pad 复用（无 pinctrl，手写 iocfg bits[3:0] + bit12 输入使能）。查 PIN_OUT 表3：
 *   UART1_TXD = GPIO3_5 = iocfg_reg44 @ 0x100C0000  func3 -> 0x1003
 *   UART1_RXD = GPIO3_6 = iocfg_reg45 @ 0x100C0004  func3 -> 0x1003
 *   PPS       = GPIO4_6 = iocfg_reg46 @ 0x100C0024  func5(GPIO) -> 0x1005（默认 JTAG_TRSTN，必写 func5！）
 */
#define PAGE_SIZE    0x1000u
#define UART_TX_PAD  0x100C0000u
#define UART_RX_PAD  0x100C0004u
#define PPS_PAD      0x100C0024u
#define UART_FUNC    0x1003u      /* func3(UART1) + bit12 */
#define PPS_FUNC     0x1005u      /* func5(GPIO) + bit12（JTAG 脚的 GPIO）*/

static void set_pad_func(uint32_t phys, uint32_t func_val)
{
    int fd = open("/dev/mem", O_RDWR);
    if (fd < 0) {
        perror("[gps] open /dev/mem");
        return;
    }
    volatile uint8_t *m = (volatile uint8_t *)mmap(NULL, PAGE_SIZE,
            PROT_READ | PROT_WRITE, MAP_SHARED, fd, phys & ~(PAGE_SIZE - 1u));
    if (m == MAP_FAILED) {
        perror("[gps] mmap pad");
        close(fd);
        return;
    }
    volatile uint32_t *pad = (volatile uint32_t *)(m + (phys & (PAGE_SIZE - 1u)));
    *pad = func_val;
    printf("[gps] pad 0x%08X -> 0x%08X\n", phys, *pad);
    munmap((void *)m, PAGE_SIZE);
    close(fd);
}

static void padmux_init(void)
{
    set_pad_func(UART_TX_PAD, UART_FUNC);
    set_pad_func(UART_RX_PAD, UART_FUNC);
    set_pad_func(PPS_PAD, PPS_FUNC);
}

static volatile sig_atomic_t g_exit = 0;

static void on_signal(int sig)
{
    (void)sig;
    g_exit = 1;
}

/* PPS 脚 GPIO4_6：chardev 申请为输入。 */
static gpio_handle_t g_pps = {
    .chip_path      = "/dev/gpiochip4",
    .line_offset    = 6,
    .gpio_mode      = GPIOHANDLE_REQUEST_INPUT,
    .default_value  = 0,
    .consumer_label = "gps-pps",
    .chip_fd        = -1,
    .line_fd        = -1,
};

/* CLOCK_MONOTONIC_RAW 毫秒，用于刷屏节流。 */
static int64_t now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (int64_t)ts.tv_sec * 1000 + (int64_t)ts.tv_nsec / 1000000;
}

/* 定位参数页。pps 为 PPS 电平(0/1/-1)。 */
static void draw_gps(const nmea_data_t *d, int pps)
{
    char line[40];

    st7789_clear();
    st7789_draw_string(0, 0, "GPS NMEA", COLOR_CYAN, COLOR_BLACK);

    snprintf(line, sizeof(line), "UART1  SAT:%02d", d->sat);
    st7789_draw_string(0, 20, line, COLOR_WHITE, COLOR_BLACK);

    st7789_draw_hline(0, 40, TFT_WIDTH, COLOR_WHITE);

    snprintf(line, sizeof(line), "TIME %s", d->time[0] ? d->time : "--");
    st7789_draw_string(0, 50, line, COLOR_WHITE, COLOR_BLACK);
    snprintf(line, sizeof(line), "DATE %s", d->date[0] ? d->date : "--");
    st7789_draw_string(0, 70, line, COLOR_WHITE, COLOR_BLACK);
    snprintf(line, sizeof(line), "LAT  %s", d->lat[0] ? d->lat : "--");
    st7789_draw_string(0, 90, line, COLOR_WHITE, COLOR_BLACK);
    snprintf(line, sizeof(line), "LON  %s", d->lon[0] ? d->lon : "--");
    st7789_draw_string(0, 110, line, COLOR_WHITE, COLOR_BLACK);

    if (d->fix) {
        snprintf(line, sizeof(line), "ALT  %.1f m", d->alt_m);
    } else {
        snprintf(line, sizeof(line), "ALT  --");
    }
    st7789_draw_string(0, 130, line, COLOR_WHITE, COLOR_BLACK);
    if (d->fix) {
        snprintf(line, sizeof(line), "SPD  %.1f kn", d->speed_kn);
    } else {
        snprintf(line, sizeof(line), "SPD  --");
    }
    st7789_draw_string(0, 150, line, COLOR_WHITE, COLOR_BLACK);

    st7789_draw_string(0, 176, d->fix ? "FIX  VALID" : "FIX  NO FIX",
                       d->fix ? COLOR_GREEN : COLOR_RED, COLOR_BLACK);

    snprintf(line, sizeof(line), "PPS  %d", pps);
    st7789_draw_string(0, 200, line, COLOR_YELLOW, COLOR_BLACK);

    st7789_flush();
}

static void draw_error(const char *msg)
{
    st7789_clear();
    st7789_draw_string(0, 0, "GPS UART FAIL", COLOR_RED, COLOR_BLACK);
    st7789_draw_string(0, 24, msg, COLOR_WHITE, COLOR_BLACK);
    st7789_draw_string(0, 60, "check:", COLOR_YELLOW, COLOR_BLACK);
    st7789_draw_string(0, 80, "GPS-TX->3_6", COLOR_WHITE, COLOR_BLACK);
    st7789_draw_string(0, 98, "GPS-RX->3_5", COLOR_WHITE, COLOR_BLACK);
    st7789_draw_string(0, 116, "VCC/GND", COLOR_WHITE, COLOR_BLACK);
    st7789_draw_string(0, 134, "9600 8N1", COLOR_WHITE, COLOR_BLACK);
    st7789_flush();
}

static void usage(const char *prog)
{
    printf("用法: %s [-h]\n"
           "  持续读 UART1 NMEA，每 500ms 刷新 SPI 屏，Ctrl+C 退出。\n"
           "  UART/引脚在 uart_hal.h、main.c 顶部修改后重新编译。\n",
           prog);
}

int main(int argc, char **argv)
{
    struct sigaction sa;
    char line[96];
    nmea_data_t data;
    int ufd;
    int64_t last_flush;

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

    printf("[gps] pad 复用：UART1(3_5/3_6)->func3，GPIO4_6(PPS)->func5\n");
    padmux_init();

    printf("[gps] 初始化 SPI 屏（/dev/spidev2.0）...\n");
    if (spi_hal_init() < 0) {
        fprintf(stderr, "[gps] SPI 传输层初始化失败。\n");
        return 1;
    }
    st7789_init();

    printf("[gps] 打开 UART（%s）...\n", GPS_UART_DEV);
    ufd = uart_open(GPS_UART_DEV);
    if (ufd < 0) {
        fprintf(stderr, "[gps] UART 打开失败，屏上显示排查提示。\n");
        draw_error("open ttyAMA1 fail");
        sleep(3);
        st7789_clear();
        st7789_flush();
        st7789_display_on(0);
        st7789_deinit();
        return 1;
    }

    printf("[gps] 申请 PPS 输入（GPIO4_6）...\n");
    if (gpio_handle_init(&g_pps) < 0) {
        fprintf(stderr, "[gps] PPS 申请输入失败（继续，PPS 显示 -1）。\n");
    }

    nmea_init(&data);
    last_flush = now_ms();
    printf("[gps] 接收 NMEA 中，每 500ms 刷新，Ctrl+C 退出。"
           "（室内通常 NO FIX，到窗户/室外定位）\n");

    while (!g_exit) {
        int n = uart_read_line(ufd, line, sizeof(line));
        if (n > 0 && line[0] == '$') {
            nmea_parse(line, &data);   /* 累积更新 GGA/RMC 字段 */
        }
        /* 节流刷屏：500ms 一次，用最新 data。 */
        int64_t now = now_ms();
        if (now - last_flush >= 500) {
            int pps = (g_pps.line_fd >= 0) ? gpio_get_value(&g_pps) : -1;
            draw_gps(&data, pps);
            last_flush = now;
        }
    }

    uart_close(ufd);
    gpio_handle_close(&g_pps);
    st7789_clear();
    st7789_flush();
    st7789_display_on(0);
    st7789_deinit();
    printf("[gps] 已清屏关显示，退出。\n");
    return 0;
}
