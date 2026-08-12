/*
 * oled_display - SSD1306 OLED 显示示例（板端运行）。
 *
 * 硬件：GK7602V11A，OLED 接在 I2C3（GPIO4_1=SCL, GPIO4_2=SDA -> /dev/i2c-3），
 *       SSD1306 128x64，从地址 0x3C（可在 ssd1306.h 改）。
 *
 * 行为：
 *   默认  先显示欢迎画面，再每秒循环刷新系统信息（uptime/loadavg/mem），Ctrl+C 退出。
 *   --once 仅显示欢迎画面后退出。
 *   -h/--help 打印用法。
 */
#include "ssd1306.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>

/*
 * I2C3 pad 复用（无 pinctrl，必须手写 iocfg 寄存器 bits[3:0]）。
 * 查 7206V11A PIN_OUT 表3：I2C3 可复用到 GPIO4_1/GPIO4_2，功能值=func2。
 *   0x100C0010 (LCD_VSYNC/GPIO4_1) func2 = I2C3_SCL
 *   0x100C0014 (LCD_HSYNC/GPIO4_2) func2 = I2C3_SDA
 * 写 0x1002 = func2 + bit12(输入使能)。默认 func0(GPIO/JTAG) 下 I2C3 控制器
 * 没接到引脚，发事务无时钟 -> wait idle timeout；切到 func2 才通。
 * （若 OLED 接的是专用脚 GPIO7_5/GPIO7_6，改下面两组为 0x100C0080/0x100C0084 func4=0x1004）
 */
#define I2C3_SCL_PAD 0x100C0010u
#define I2C3_SDA_PAD 0x100C0014u
#define I2C3_FUNC_VAL 0x1002u
#define PAGE_SIZE 0x1000u

/* 把单个 pad 写成 func_val（经 /dev/mem mmap）。 */
static void set_pad_func(uint32_t phys, uint32_t func_val)
{
    int fd = open("/dev/mem", O_RDWR);
    if (fd < 0) {
        perror("[oled] open /dev/mem");
        return;
    }
    volatile uint8_t *m = (volatile uint8_t *)mmap(NULL, PAGE_SIZE,
            PROT_READ | PROT_WRITE, MAP_SHARED, fd, phys & ~(PAGE_SIZE - 1u));
    if (m == MAP_FAILED) {
        perror("[oled] mmap pad");
        close(fd);
        return;
    }
    volatile uint32_t *pad = (volatile uint32_t *)(m + (phys & (PAGE_SIZE - 1u)));
    *pad = func_val;
    printf("[oled] pad 0x%08X -> 0x%08X\n", phys, *pad);
    munmap((void *)m, PAGE_SIZE);
    close(fd);
}

/* 把 I2C3 的 SCL/SDA pad 切到 func2。 */
static void i2c3_padmux_init(void)
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
    ssd1306_clear();
    ssd1306_draw_rect(0, 0, OLED_WIDTH, OLED_HEIGHT, OLED_COLOR_WHITE);
    ssd1306_draw_string(8, 3, "SSD1306 OLED", OLED_COLOR_WHITE);
    ssd1306_draw_hline(2, 21, OLED_WIDTH - 4, OLED_COLOR_WHITE);
    ssd1306_draw_string(4, 26, "GK7602V11A", OLED_COLOR_WHITE);
    ssd1306_draw_string(4, 44, "/dev/i2c-3 OK", OLED_COLOR_WHITE);
    ssd1306_fill_rect(OLED_WIDTH - 16, OLED_HEIGHT - 16, 12, 12, OLED_COLOR_WHITE);
    ssd1306_flush();
}

static void draw_sysinfo(void)
{
    char buf[24];
    double up = read_uptime();
    double la, lb, lc;
    long mem_total;
    long mem_avail;

    read_loadavg(&la, &lb, &lc);
    mem_total = read_meminfo_kb("MemTotal");
    mem_avail = read_meminfo_kb("MemAvailable");

    ssd1306_clear();
    ssd1306_draw_string(0, 0,  "SYS INFO", OLED_COLOR_WHITE);

    snprintf(buf, sizeof(buf), "UP %.0fs", up);
    ssd1306_draw_string(0, 16, buf, OLED_COLOR_WHITE);

    snprintf(buf, sizeof(buf), "LOAD %.1f %.1f", la, lb);
    ssd1306_draw_string(0, 32, buf, OLED_COLOR_WHITE);

    if (mem_total > 0 && mem_avail > 0) {
        snprintf(buf, sizeof(buf), "MEM %ld/%ldMB", mem_avail / 1024, mem_total / 1024);
    } else {
        snprintf(buf, sizeof(buf), "MEM --");
    }
    ssd1306_draw_string(0, 48, buf, OLED_COLOR_WHITE);

    ssd1306_flush();
}

static void usage(const char *prog)
{
    printf("用法: %s [--once] [-h]\n"
           "  默认       显示欢迎画面后每秒刷新系统信息，Ctrl+C 退出\n"
           "  --once     仅显示欢迎画面后退出\n"
           "  -h/--help  显示本帮助\n"
           "  总线/地址/分辨率在 ssd1306.h 修改后重新编译。\n",
           prog);
}

int main(int argc, char **argv)
{
    int run_sysinfo = 1;
    struct sigaction sa;

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

    printf("[oled] init SSD1306 @ /dev/i2c-%d addr 0x%02x ...\n",
           OLED_I2C_BUS, OLED_I2C_ADDR);
    i2c3_padmux_init();   /* GPIO4_1/GPIO4_2 -> func2(I2C3) */
    if (ssd1306_init() < 0) {
        fprintf(stderr, "[oled] 初始化失败（OLED 无应答？请检查接线/地址/pad 复用）。\n");
        return 1;
    }
    printf("[oled] 初始化成功，开始显示。\n");

    draw_welcome();

    if (run_sysinfo) {
        sleep(2);
        while (!g_exit) {
            draw_sysinfo();
            sleep(1);
        }
    } else {
        sleep(3);
    }

    /* 退出时清屏并关显示。 */
    ssd1306_clear();
    ssd1306_flush();
    ssd1306_display_on(0);
    ssd1306_deinit();
    printf("[oled] 已清屏关显示，退出。\n");
    return 0;
}
