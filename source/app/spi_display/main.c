/*
 * spi_display - ST7789 TFT 显示示例（240x240 RGB565, SPI2）。
 *
 * 硬件：GK7602V11A，TFT 接 SPI2（SCK=GPIO4_7, MOSI=GPIO5_0, CS=GPIO5_1，
 *       DC=GPIO4_5, RES=GPIO4_4），ST7789 240x240 RGB。
 *
 * 行为：
 *   默认  先显示欢迎画面，再每秒循环刷新系统信息（uptime/loadavg/mem），Ctrl+C 退出。
 *   --once 仅显示欢迎画面后退出。
 *   -h/--help 打印用法。
 */
#include "st7789.h"
#include "spi_hal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

static volatile sig_atomic_t g_exit = 0;

static void on_signal(int sig)
{
    (void)sig;
    g_exit = 1;
}

/* 读 /proc/uptime 第一字段（秒）。 */
static double read_uptime(void)
{
    FILE *f = fopen("/proc/uptime", "r");
    double up = 0.0;

    if (f != NULL) {
        if (fscanf(f, "%lf", &up) != 1) {
            up = 0.0;
        }
        fclose(f);
    }
    return up;
}

/* 读 /proc/loadavg 前三个值。 */
static void read_loadavg(double *a, double *b, double *c)
{
    FILE *f = fopen("/proc/loadavg", "r");

    *a = *b = *c = 0.0;
    if (f != NULL) {
        if (fscanf(f, "%lf %lf %lf", a, b, c) != 3) {
            *a = *b = *c = 0.0;
        }
        fclose(f);
    }
}

/* 从 /proc/meminfo 取某项的 kB 值，找不到返回 -1。 */
static long read_meminfo_kb(const char *key)
{
    FILE *f = fopen("/proc/meminfo", "r");
    char line[160];
    long val = -1;

    if (f == NULL) {
        return -1;
    }
    while (fgets(line, sizeof(line), f) != NULL) {
        if (strncmp(line, key, strlen(key)) == 0) {
            char *colon = strchr(line, ':');
            if (colon != NULL && sscanf(colon + 1, "%ld", &val) != 1) {
                val = -1;
            }
            break;
        }
    }
    fclose(f);
    return val;
}

static void draw_welcome(void)
{
    st7789_clear();
    st7789_draw_rect(0, 0, TFT_WIDTH, TFT_HEIGHT, COLOR_WHITE);
    /* 三色方块：亮起即可肉眼判断颜色/方向是否正常。 */
    st7789_fill_rect(16, 16, 48, 48, COLOR_RED);
    st7789_fill_rect(TFT_WIDTH - 64, 16, 48, 48, COLOR_GREEN);
    st7789_fill_rect(TFT_WIDTH / 2 - 24, TFT_HEIGHT / 2 - 24, 48, 48, COLOR_BLUE);
    st7789_draw_string(8, TFT_HEIGHT - 40, "ST7789 240x240", COLOR_WHITE, COLOR_BLACK);
    st7789_draw_string(8, TFT_HEIGHT - 22, "spidev2.0 OK", COLOR_YELLOW, COLOR_BLACK);
    st7789_flush();
}

static void draw_sysinfo(void)
{
    char buf[32];
    double up = read_uptime();
    double la, lb, lc;
    long mem_total;
    long mem_avail;

    read_loadavg(&la, &lb, &lc);
    mem_total = read_meminfo_kb("MemTotal");
    mem_avail = read_meminfo_kb("MemAvailable");

    st7789_clear();
    st7789_draw_string(0, 0, "SYS INFO", COLOR_CYAN, COLOR_BLACK);

    snprintf(buf, sizeof(buf), "UP %.0fs", up);
    st7789_draw_string(0, 24, buf, COLOR_WHITE, COLOR_BLACK);

    snprintf(buf, sizeof(buf), "LOAD %.1f %.1f", la, lb);
    st7789_draw_string(0, 48, buf, COLOR_WHITE, COLOR_BLACK);

    if (mem_total > 0 && mem_avail > 0) {
        snprintf(buf, sizeof(buf), "MEM %ld/%ldMB", mem_avail / 1024, mem_total / 1024);
    } else {
        snprintf(buf, sizeof(buf), "MEM --");
    }
    st7789_draw_string(0, 72, buf, COLOR_WHITE, COLOR_BLACK);

    st7789_draw_string(0, 120, "GK7602V11A", COLOR_GREEN, COLOR_BLACK);

    st7789_flush();
}

static void usage(const char *prog)
{
    printf("用法: %s [--once] [-h]\n"
           "  默认      先显示欢迎画面，再每秒刷新系统信息，Ctrl+C 退出\n"
           "  --once    仅显示欢迎画面，Ctrl+C 退出\n"
           "  -h/--help 显示本帮助\n",
           prog);
}

int main(int argc, char **argv)
{
    int run_sysinfo = 1;
    struct sigaction sa;

    setvbuf(stdout, NULL, _IONBF, 0);   /* 串口控制台立即输出，便于定位卡死点 */

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage(argv[0]);
            return 0;
        }
        if (strcmp(argv[i], "--once") == 0) {
            run_sysinfo = 0;
        }
    }

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    printf("[spi] init ST7789 @ %s ...\n", "/dev/spidev2.0");
    if (spi_hal_init() < 0) {   /* pad 复用 + spidev + DC/RES GPIO + 复位 */
        fprintf(stderr, "[spi] 传输层初始化失败。\n");
        return 1;
    }
    printf("[tft] >>> st7789_init()\n");
    st7789_init();
    printf("[tft] <<< st7789_init() returned\n");
    printf("[spi] 初始化成功，开始显示。\n");

    /* 先显示欢迎画面，停留约 2s；默认模式再每秒循环刷新系统信息。 */
    draw_welcome();
    if (run_sysinfo) {
        sleep(2);
        while (!g_exit) {
            draw_sysinfo();
            sleep(1);
        }
    } else {
        while (!g_exit) {
            sleep(1);
        }
    }

    st7789_clear();
    st7789_flush();
    st7789_display_on(0);
    st7789_deinit();
    printf("[spi] 已清屏关显示，退出。\n");
    return 0;
}
