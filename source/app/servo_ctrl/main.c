/*
 * servo_ctrl - PCA9685 16 路舵机控制（板端运行）。
 *
 * 硬件：GK7602V11A
 *   PCA9685 接 I2C3（GPIO4_1=SCL, GPIO4_2=SDA -> /dev/i2c-3），地址 0x40，
 *   与 OLED/MPU6050/SHT20 共享总线。
 *   OE 接 GPIO4_5（默认 JTAG_TDO，要写 func5(0x1005) 切回 GPIO；
 *   OE active LOW：拉低=输出使能，拉高=全通道关闭）。
 *   舵机信号线接到 PCA9685 通道 0..15（GND 共用）。
 *
 * 注意：GPIO4_5 与 SPI 屏的 DC 脚复用同一个 pad（0x100C0020），
 *       本程序不复用 SPI 屏。如果想同时显示，OE 用别的脚或保持 OE 接地。
 *
 * 命令格式（stdin，每行一条）：
 *   s <ch> <us>     设置通道 ch 的 PWM 脉宽为 us μ秒（ch 0..15）
 *   a <us>          所有通道设同一脉宽
 *   c               所有通道居中（1500μs）
 *   o               所有通道关闭
 *   q               退出（先关闭全部输出，再退出）
 */
#include "pca9685.h"
#include "gpio_hal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <sys/mman.h>

/*
 * pad 复用（查 7206V11A PIN_OUT 表3）：
 *   I2C3_SCL   = GPIO4_1 = iocfg_reg48 @ 0x100C0010  func2 -> 0x1002
 *   I2C3_SDA   = GPIO4_2 = iocfg_reg49 @ 0x100C0014  func2 -> 0x1002
 *   OE         = GPIO4_5 = iocfg_reg52 @ 0x100C0020  func5 -> 0x1005（默认 JTAG_TDO）
 */
#define PAGE_SIZE     0x1000u
#define I2C3_SCL_PAD  0x100C0010u
#define I2C3_SDA_PAD  0x100C0014u
#define I2C3_FUNC_VAL 0x1002u
#define OE_PAD        0x100C0020u
#define OE_FUNC_VAL   0x1005u

static void set_pad_func(uint32_t phys, uint32_t func_val)
{
    int fd = open("/dev/mem", O_RDWR);
    if (fd < 0) {
        perror("[pca] open /dev/mem");
        return;
    }
    volatile uint8_t *m = (volatile uint8_t *)mmap(NULL, PAGE_SIZE,
            PROT_READ | PROT_WRITE, MAP_SHARED, fd, phys & ~(PAGE_SIZE - 1u));
    if (m == MAP_FAILED) {
        perror("[pca] mmap pad");
        close(fd);
        return;
    }
    volatile uint32_t *pad = (volatile uint32_t *)(m + (phys & (PAGE_SIZE - 1u)));
    *pad = func_val;
    printf("[pca] pad 0x%08X -> 0x%08X\n", phys, *pad);
    munmap((void *)m, PAGE_SIZE);
    close(fd);
}

static volatile sig_atomic_t g_exit = 0;
static void on_signal(int sig) { (void)sig; g_exit = 1; }

/* OE 控制：active LOW，0=输出使能，1=全通道关闭 */
static gpio_handle_t g_oe = {
    .chip_path      = "/dev/gpiochip4",
    .line_offset    = 5,
    .gpio_mode      = GPIOHANDLE_REQUEST_OUTPUT,
    .default_value  = 1,            /* 默认全通道关闭（安全） */
    .consumer_label = "pca9685-oe",
    .chip_fd        = -1,
    .line_fd        = -1,
};

static void oe_enable(int en)
{
    gpio_set_value(&g_oe, en ? 0 : 1);   /* 1=enable(drive LOW), 0=disable(drive HIGH) */
}

/* 解析 stdin 一行命令 */
static void handle_line(char *line)
{
    int ch, us;
    char cmd;

    /* 跳过前导空白 */
    while (*line && isspace((unsigned char)*line)) line++;
    if (*line == '\0' || *line == '#') {
        return;
    }
    cmd = line[0];

    switch (cmd) {
        case 's':
            if (sscanf(line + 1, "%d %d", &ch, &us) != 2 ||
                ch < 0 || ch >= PCA9685_CHANNELS || us > 50000) {
                printf("ERR: 用法 s <ch 0..15> <us 500..2500> (0 = off)\n");
                break;
            }
            if (pca9685_set_pulse((uint8_t)ch, (uint16_t)us) == 0) {
                printf("[pca] ch%d = %d us\n", ch, us);
            } else {
                printf("[pca] ch%d 失败\n", ch);
            }
            break;

        case 'a':
            if (sscanf(line + 1, "%d", &us) != 1 || us > 50000) {
                printf("ERR: 用法 a <us 500..2500> (0 = off)\n");
                break;
            }
            if (pca9685_set_all_pulse((uint16_t)us) == 0) {
                printf("[pca] 全部 = %d us\n", us);
            } else {
                printf("[pca] 全部失败\n");
            }
            break;

        case 'c':
            if (pca9685_set_all_pulse(SERVO_PULSE_MID_US) == 0) {
                printf("[pca] 全部居中 (%d us)\n", SERVO_PULSE_MID_US);
            }
            break;

        case 'o':
            if (pca9685_all_off() == 0) {
                printf("[pca] 全部关闭\n");
            }
            break;

        case 'q':
        case 'Q':
            g_exit = 1;
            break;

        case 'h':
        case 'H':
        case '?':
            printf("命令:\n"
                   "  s <ch 0..15> <us>  设单通道 PWM 脉宽\n"
                   "  a <us>             全部通道设同一脉宽\n"
                   "  c                  全部居中 (1500us)\n"
                   "  o                  全部关闭\n"
                   "  q                  退出\n"
                   "  h / ?              帮助\n"
                   "典型伺服脉宽: 500us(0°) / 1500us(90°) / 2500us(180°)\n");
            break;

        default:
            printf("未知命令 '%c'（输 h 看帮助）\n", cmd);
            break;
    }
}

static void usage(const char *prog)
{
    printf("用法: %s [-h]\n"
           "  stdin 接受命令控制 16 路 PWM 通道（每行一条）：\n"
           "    s <ch> <us>  a <us>  c  o  q  h\n"
           "  Ctrl+C / Ctrl+D 也退出。\n"
           "  地址在 pca9685.h、pin 在 main.c 顶部修改后重新编译。\n",
           prog);
}

int main(int argc, char **argv)
{
    struct sigaction sa;
    char line[128];

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

    /* pad mux */
    printf("[pca] pad 复用：I2C3(4_1/4_2)->func2，GPIO4_5(OE)->func5\n");
    set_pad_func(I2C3_SCL_PAD, I2C3_FUNC_VAL);
    set_pad_func(I2C3_SDA_PAD, I2C3_FUNC_VAL);
    set_pad_func(OE_PAD,      OE_FUNC_VAL);

    /* OE 先申请为输出，初始 HIGH（全通道关闭）—— 安全 */
    if (gpio_handle_init(&g_oe) < 0) {
        fprintf(stderr, "[pca] GPIO4_5 (OE) 申请失败：%s\n", strerror(errno));
        return 1;
    }
    oe_enable(0);   /* HIGH = 关闭 */

    /* PCA9685 init：50Hz PWM（舵机标准） */
    printf("[pca] init PCA9685 (PWM 50Hz)...\n");
    if (pca9685_init(50) < 0) {
        fprintf(stderr, "[pca] PCA9685 init 失败\n");
        gpio_handle_close(&g_oe);
        return 1;
    }

    /* 全通道输出安全状态（off），然后拉低 OE 使能 */
    pca9685_all_off();
    oe_enable(1);   /* LOW = 输出使能 */
    printf("[pca] OE 已使能，开始接受命令\n");

    printf(
        "\n命令:\n"
        "  s <ch 0..15> <us>  a <us>  c  o  q  h\n"
        "脉宽: 500us(0°) / 1500us(90°) / 2500us(180°)\n"
        "> ");

    while (!g_exit) {
        if (!fgets(line, sizeof(line), stdin)) {
            /* EOF (Ctrl+D) */
            break;
        }
        handle_line(line);
        if (!g_exit) {
            printf("> ");
        }
    }

    /* 退出：关闭所有输出 -> 拉高 OE -> 释放资源 */
    printf("\n[pca] 关闭所有输出，禁用 OE...\n");
    pca9685_all_off();
    oe_enable(0);
    pca9685_deinit();
    gpio_handle_close(&g_oe);
    printf("[pca] 退出\n");
    return 0;
}