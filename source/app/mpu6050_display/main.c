/*
 * mpu6050_display - MPU6050 六轴采集 + ST7789 SPI 屏显示（板端运行）。
 *
 * 硬件：GK7602V11A
 *   MPU6050 接 I2C3（GPIO4_1=SCL, GPIO4_2=SDA -> /dev/i2c-3），AD0 接地 -> 0x68，
 *   INT 接 GPIO0_0（/dev/gpiochip0 line0，配输入读取电平），XCL/XDA 悬空。
 *   数据显示在 SPI 屏（ST7789 240x240，/dev/spidev2.0，复用 spi_display 传输层）。
 *
 * 行为：每 150ms 读一次六轴 + 温度并刷新数值表，Ctrl+C 清屏退出。
 *   -h/--help 打印用法。
 */
#include "st7789.h"
#include "spi_hal.h"
#include "mpu6050.h"
#include "gpio_hal.h"

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
 * pad 复用（无 pinctrl，手写 iocfg 寄存器 bits[3:0] + bit12 输入使能）。查 PIN_OUT 表3：
 *   I2C3_SCL = GPIO4_1 = iocfg_reg48 @ 0x100C0010  func2 -> 0x1002
 *   I2C3_SDA = GPIO4_2 = iocfg_reg49 @ 0x100C0014  func2 -> 0x1002
 *   INT      = GPIO0_0 = iocfg_reg0  @ 0x112C0000  func0(GPIO)+输入使能 -> 0x1000（即默认值）
 * 注意 GPIO0_0 在另一块 iocfg（0x112C0000，sensor/I2C pad），与 LCD pad 的 0x100C00xx 不同。
 * SPI 屏的 pad（4_4/4_5/4_7/5_0/5_1）由 spi_hal_init() 内部切，这里不管。
 */
#define PAGE_SIZE     0x1000u
#define I2C3_SCL_PAD  0x100C0010u
#define I2C3_SDA_PAD  0x100C0014u
#define I2C3_FUNC_VAL 0x1002u
#define INT_PAD       0x112C0000u
#define INT_FUNC_VAL  0x1000u

/* 把单个 pad 写成 func_val（经 /dev/mem mmap iocfg）。 */
static void set_pad_func(uint32_t phys, uint32_t func_val)
{
    int fd = open("/dev/mem", O_RDWR);
    if (fd < 0) {
        perror("[mpu] open /dev/mem");
        return;
    }
    volatile uint8_t *m = (volatile uint8_t *)mmap(NULL, PAGE_SIZE,
            PROT_READ | PROT_WRITE, MAP_SHARED, fd, phys & ~(PAGE_SIZE - 1u));
    if (m == MAP_FAILED) {
        perror("[mpu] mmap pad");
        close(fd);
        return;
    }
    volatile uint32_t *pad = (volatile uint32_t *)(m + (phys & (PAGE_SIZE - 1u)));
    *pad = func_val;
    printf("[mpu] pad 0x%08X -> 0x%08X\n", phys, *pad);
    munmap((void *)m, PAGE_SIZE);
    close(fd);
}

static void padmux_init(void)
{
    set_pad_func(I2C3_SCL_PAD, I2C3_FUNC_VAL);
    set_pad_func(I2C3_SDA_PAD, I2C3_FUNC_VAL);
    set_pad_func(INT_PAD, INT_FUNC_VAL);
}

static volatile sig_atomic_t g_exit = 0;

static void on_signal(int sig)
{
    (void)sig;
    g_exit = 1;
}

/* INT 脚 GPIO0_0：chardev 申请为输入。 */
static gpio_handle_t g_int = {
    .chip_path      = "/dev/gpiochip0",
    .line_offset    = 0,
    .gpio_mode      = GPIOHANDLE_REQUEST_INPUT,
    .default_value  = 0,
    .consumer_label = "mpu6050-int",
    .chip_fd        = -1,
    .line_fd        = -1,
};

/* 数值表：标题 + INT 状态 + ACCEL + GYRO + 温度。 */
static void draw_data(const mpu6050_data_t *d, int int_val)
{
    char line[40];

    st7789_clear();
    st7789_draw_string(0, 0, "MPU6050 0x68", COLOR_CYAN, COLOR_BLACK);

    snprintf(line, sizeof(line), "I2C-3  INT:%d", int_val);
    st7789_draw_string(0, 20, line, (int_val > 0) ? COLOR_GREEN : COLOR_WHITE,
                       COLOR_BLACK);

    st7789_draw_hline(0, 40, TFT_WIDTH, COLOR_WHITE);

    st7789_draw_string(0, 48, "ACCEL  g", COLOR_YELLOW, COLOR_BLACK);
    snprintf(line, sizeof(line), "X %+6.2f  Y %+6.2f", d->ax, d->ay);
    st7789_draw_string(8, 68, line, COLOR_WHITE, COLOR_BLACK);
    snprintf(line, sizeof(line), "Z %+6.2f", d->az);
    st7789_draw_string(8, 88, line, COLOR_WHITE, COLOR_BLACK);

    st7789_draw_string(0, 112, "GYRO  dps", COLOR_YELLOW, COLOR_BLACK);
    snprintf(line, sizeof(line), "X %+6.1f  Y %+6.1f", d->gx, d->gy);
    st7789_draw_string(8, 132, line, COLOR_WHITE, COLOR_BLACK);
    snprintf(line, sizeof(line), "Z %+6.1f", d->gz);
    st7789_draw_string(8, 152, line, COLOR_WHITE, COLOR_BLACK);

    snprintf(line, sizeof(line), "T %5.1f C", d->temp);
    st7789_draw_string(0, 178, line, COLOR_YELLOW, COLOR_BLACK);

    st7789_draw_string(0, 220, "Ctrl+C quit", COLOR_BLUE, COLOR_BLACK);

    st7789_flush();
}

/* MPU6050 初始化失败时的屏上提示。 */
static void draw_error(const char *msg)
{
    st7789_clear();
    st7789_draw_string(0, 0, "MPU6050 FAIL", COLOR_RED, COLOR_BLACK);
    st7789_draw_string(0, 24, msg, COLOR_WHITE, COLOR_BLACK);
    st7789_draw_string(0, 60, "check:", COLOR_YELLOW, COLOR_BLACK);
    st7789_draw_string(0, 80, "SCL=GPIO4_1", COLOR_WHITE, COLOR_BLACK);
    st7789_draw_string(0, 98, "SDA=GPIO4_2", COLOR_WHITE, COLOR_BLACK);
    st7789_draw_string(0, 116, "AD0->GND", COLOR_WHITE, COLOR_BLACK);
    st7789_draw_string(0, 134, "VCC 3.3V", COLOR_WHITE, COLOR_BLACK);
    st7789_flush();
}

static void usage(const char *prog)
{
    printf("用法: %s [-h]\n"
           "  每 150ms 采集一次 MPU6050 并刷新 SPI 屏数值表，Ctrl+C 退出。\n"
           "  总线/地址/引脚在 mpu6050.h、main.c 顶部修改后重新编译。\n",
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

    setvbuf(stdout, NULL, _IONBF, 0);   /* 串口控制台立即输出 */

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    printf("[mpu] pad 复用：I2C3(4_1/4_2)->func2，GPIO0_0->func0\n");
    padmux_init();

    printf("[mpu] 初始化 SPI 屏（/dev/spidev2.0）...\n");
    if (spi_hal_init() < 0) {
        fprintf(stderr, "[mpu] SPI 传输层初始化失败。\n");
        return 1;
    }
    st7789_init();
    printf("[mpu] SPI 屏就绪。\n");

    printf("[mpu] 初始化 MPU6050（/dev/i2c-%d addr 0x%02x）...\n", MPU6050_BUS, MPU6050_ADDR);
    if (mpu6050_init() < 0) {
        fprintf(stderr, "[mpu] MPU6050 初始化失败，屏上显示排查提示。\n");
        draw_error("no WHO_AM_I=0x68");
        sleep(5);
        st7789_clear();
        st7789_flush();
        st7789_display_on(0);
        st7789_deinit();
        return 1;
    }

    /* INT 脚申请为输入；失败不致命（INT 显示 --）。 */
    if (gpio_handle_init(&g_int) < 0) {
        fprintf(stderr, "[mpu] GPIO0_0 申请输入失败：%s（INT 将显示 --）\n", strerror(errno));
    }

    printf("[mpu] 采集显示中，每 150ms 刷新，Ctrl+C 退出。\n");
    while (!g_exit) {
        mpu6050_data_t d;
        int iv = (g_int.line_fd >= 0) ? gpio_get_value(&g_int) : -1;

        if (mpu6050_read(&d) == 0) {
            draw_data(&d, iv);
        } else {
            fprintf(stderr, "[mpu] 读取失败（I2C 掉线？）\n");
        }
        usleep(150000);
    }

    mpu6050_deinit();
    gpio_handle_close(&g_int);

    st7789_clear();
    st7789_flush();
    st7789_display_on(0);
    st7789_deinit();
    printf("[mpu] 已清屏关显示，退出。\n");
    return 0;
}
