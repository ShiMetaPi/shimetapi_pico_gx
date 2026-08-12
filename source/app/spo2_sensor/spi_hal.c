/*
 * spi_hal.c - SPI 屏传输层实现（硬件 SPI2 + chardev GPIO 控制 DC/RES/CS）。
 *
 * 传输分层：
 *   SCK/MOSI 走 SPI2(func4) 的 /dev/spidev2.0（控制器直驱，内核 spidev）；
 *   DC/RES/CS 走 GPIO(func5/func0)，用 /dev/gpiochipN 的 chardev
 *   （GPIO_GET_LINEHANDLE_IOCTL + GPIOHANDLE_SET_LINE_VALUES_IOCTL）驱动。
 *
 * 历史背景：本平台曾实测 chardev 版"申请 ok 但引脚不动"，被迫改 PL061 直驱
 *   （见 git 历史）。本次按"再试 chardev"的要求改回标准 chardev 写法；为判定
 *   chardev 是否真的驱动引脚，在 init 末尾用只读方式读回 PL061 GPIODATA 寄存器
 *   核对电平（pl061_peek_bit）。若打印的 verify 值与 set 的期望值不符，即证实
 *   chardev 仍不驱动引脚，根因在内核 GPIO 驱动，需另行排查。
 *
 * 仍保留 pad 复用的 mmap：chardev 只管"输出值"，不管"把脚切到 GPIO 功能"——
 *   必须先把 DC/RES 切到 func5、CS 切到 func0，chardev 的输出才到得了引脚。
 *   这一步和 OLED(oled_display) 里切 I2C3 pad 是同一类操作（写 iocfg 寄存器）。
 *
 * PL061 bank 基址(DT)：gpio_chip4@0x120b4000, gpio_chip5@0x120b5000。
 *   DC=GPIO4_5(chip4,line5)、RES=GPIO4_4(chip4,line4)、CS=GPIO5_1(chip5,line1)。
 */
#include "spi_hal.h"
#include "gpio_hal.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/spi/spidev.h>

#define SPI_DEV      "/dev/spidev2.0"
#define SPI_SPEED_HZ 24000000u  /* 24 MHz：本 PL022 实测最高可用档（40/30MHz 被 spi_validate 拒）。整屏 flush ~48ms */
#define SPI_BITS     8
#define SPI_MODE     SPI_MODE_3 /* CPOL=1 CPHA=1：对照 TFT 例程(ST7789 常用 MODE3) */

/* pad 地址 + 功能值（查 7206V11A PIN_OUT 表3，每脚 func 不同）。 */
#define PAD_SCK   0x100C0028u   /* GPIO4_7：func4=SPI2_SCLK */
#define PAD_MOSI  0x100C002Cu   /* GPIO5_0：func4=SPI2_SDO */
#define PAD_CS    0x100C0030u   /* GPIO5_1：原生 func4=SPI2_CSN，本驱动改 func0=GPIO */
#define PAD_DC    0x100C0020u   /* GPIO4_5：默认 JTAG_TDO，要 func5=GPIO */
#define PAD_RES   0x100C001Cu   /* GPIO4_4：默认 JTAG_TDI，要 func5=GPIO */
#define FUNC_SPI2  0x1004u      /* func4 + bit12 */
#define FUNC_GPIO  0x1005u      /* func5 + bit12（JTAG 脚 DC/RES 的 GPIO）*/
#define FUNC_GPIO0 0x1000u      /* func0 + bit12（非 JTAG 脚 CS/GPIO5_1 的 GPIO）*/
#define PAGE_SIZE  0x1000u

/* PL061 bank 基址(DT) + 位号 —— 仅用于只读诊断 pl061_peek_bit。 */
#define GPIO4_BASE 0x120b4000u  /* bank4：DC=bit5, RES=bit4 */
#define GPIO5_BASE 0x120b5000u  /* bank5：CS=bit1 */

/* DC/RES/CS 的 chardev 句柄。default_value = 申请瞬间立即输出的电平。 */
static gpio_handle_t g_dc = {
    .chip_path      = "/dev/gpiochip4",
    .line_offset    = 5,
    .gpio_mode      = GPIOHANDLE_REQUEST_OUTPUT,
    .default_value  = 0,          /* DC 默认低=命令模式 */
    .consumer_label = "tft-dc",
    .chip_fd        = -1,
    .line_fd        = -1,
};
static gpio_handle_t g_res = {
    .chip_path      = "/dev/gpiochip4",
    .line_offset    = 4,
    .gpio_mode      = GPIOHANDLE_REQUEST_OUTPUT,
    .default_value  = 1,          /* RES 默认高=不复位 */
    .consumer_label = "tft-res",
    .chip_fd        = -1,
    .line_fd        = -1,
};
static gpio_handle_t g_cs = {
    .chip_path      = "/dev/gpiochip5",
    .line_offset    = 1,
    .gpio_mode      = GPIOHANDLE_REQUEST_OUTPUT,
    .default_value  = 1,          /* CS 默认高=不选中 */
    .consumer_label = "tft-cs",
    .chip_fd        = -1,
    .line_fd        = -1,
};

static int g_spidev = -1;

/* 全双工 dummy RX：TX-only 在本平台 overrun，给 rx_buf 强制排空 RX FIFO。 */
static uint8_t g_rxdummy[4096];

/*
 * 只读诊断：mmap PL061 bank，读 GPIODATA@0x3FC（读全部 8 位），
 * 返回指定 pin 的当前电平(0/1)，失败返回 -1。
 * 用于在 chardev set 之后核对引脚电平是否真的变化。仅读不写，不影响输出。
 */
static int pl061_peek_bit(uint32_t base, uint8_t pin)
{
    int fd = open("/dev/mem", O_RDONLY);
    volatile uint32_t *m;
    uint32_t data;
    int bit;

    if (fd < 0) {
        return -1;
    }
    m = (volatile uint32_t *)mmap(NULL, 0x1000, PROT_READ, MAP_SHARED, fd, base);
    close(fd);
    if (m == MAP_FAILED) {
        return -1;
    }
    data = m[0x3FC / 4];
    bit = (int)((data >> pin) & 1u);
    munmap((void *)m, 0x1000);
    return bit;
}

/* 把单个 pad 写成 func（经 /dev/mem mmap iocfg）。 */
static void set_pad_func(uint32_t phys, uint32_t func)
{
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("[spi] open /dev/mem");
        return;
    }
    volatile uint8_t *m = (volatile uint8_t *)mmap(NULL, PAGE_SIZE,
            PROT_READ | PROT_WRITE, MAP_SHARED, fd, phys & ~(PAGE_SIZE - 1u));
    if (m == MAP_FAILED) {
        perror("[spi] mmap pad");
        close(fd);
        return;
    }
    volatile uint32_t *pad = (volatile uint32_t *)(m + (phys & (PAGE_SIZE - 1u)));
    *pad = func;
    printf("[spi] pad 0x%08X -> 0x%08X\n", phys, *pad);
    munmap((void *)m, PAGE_SIZE);
    close(fd);
}

/* 单笔 SPI 写事务。CS 手动 framing：前拉低(选中)、后拉高(释放)，对照 TFT 例程。 */
static int spidev_write(const uint8_t *tx, int len)
{
    struct spi_ioc_transfer tr;
    int ret;

    memset(&tr, 0, sizeof(tr));
    tr.tx_buf = (unsigned long)tx;
    if (len <= (int)sizeof(g_rxdummy)) {  /* 全双工排空 RX，防 overrun */
        tr.rx_buf = (unsigned long)g_rxdummy;
    }
    tr.len = (uint32_t)len;
    tr.speed_hz = SPI_SPEED_HZ;
    tr.bits_per_word = SPI_BITS;
    tr.cs_change = 0;
    gpio_set_value(&g_cs, 0);   /* CS 低(选中) */
    ret = ioctl(g_spidev, SPI_IOC_MESSAGE(1), &tr);
    gpio_set_value(&g_cs, 1);   /* CS 高(释放) */
    if (ret < 0) {
        fprintf(stderr, "[spi] spidev_write ioctl(len=%d) 失败: %s\n",
                len, strerror(errno));
    }
    return ret;
}

void spi_hal_reset(void)
{
    gpio_set_value(&g_res, 1); usleep(10000);
    gpio_set_value(&g_res, 0); usleep(10000);  /* RES 低 >=10ms 复位 */
    gpio_set_value(&g_res, 1); usleep(10000);
}

int spi_hal_init(void)
{
    uint8_t mode = SPI_MODE;
    uint8_t bits = SPI_BITS;
    uint32_t speed = SPI_SPEED_HZ;

    /* 1) pad 复用：SCK/MOSI->func4(SPI2)，CS->func0(GPIO)，DC/RES->func5(GPIO)。
     *    chardev 不管 pad 复用，必须先切到 GPIO 功能，输出才到得了引脚。 */
    set_pad_func(PAD_SCK, FUNC_SPI2);
    set_pad_func(PAD_MOSI, FUNC_SPI2);
    set_pad_func(PAD_CS, FUNC_GPIO0);
    set_pad_func(PAD_DC, FUNC_GPIO);
    set_pad_func(PAD_RES, FUNC_GPIO);

    /* 2) 打开并配置 spidev2.0。 */
    printf("[spi] opening %s ...\n", SPI_DEV); fflush(stdout);
    g_spidev = open(SPI_DEV, O_RDWR);
    if (g_spidev < 0) {
        fprintf(stderr, "[spi] open %s 失败: %s\n", SPI_DEV, strerror(errno));
        return -1;
    }
    printf("[spi] spidev opened, fd=%d\n", g_spidev); fflush(stdout);
    ioctl(g_spidev, SPI_IOC_WR_MODE, &mode);
    ioctl(g_spidev, SPI_IOC_WR_BITS_PER_WORD, &bits);
    ioctl(g_spidev, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    printf("[spi] spidev mode/bits/speed set (MODE3/8b/%luMHz)\n", (unsigned long)(speed / 1000000u)); fflush(stdout);

    /* 3) DC/RES/CS 用 chardev 申请为输出（申请瞬间输出 default_value）。 */
    printf("[spi] chardev request DC  @ %s line %u ...\n", g_dc.chip_path, g_dc.line_offset);
    if (gpio_handle_init(&g_dc) < 0) {
        fprintf(stderr, "[spi] DC chardev 申请失败。\n");
        return -1;
    }
    printf("[spi] chardev request RES @ %s line %u ...\n", g_res.chip_path, g_res.line_offset);
    if (gpio_handle_init(&g_res) < 0) {
        fprintf(stderr, "[spi] RES chardev 申请失败。\n");
        return -1;
    }
    printf("[spi] chardev request CS  @ %s line %u ...\n", g_cs.chip_path, g_cs.line_offset);
    if (gpio_handle_init(&g_cs) < 0) {
        fprintf(stderr, "[spi] CS chardev 申请失败。\n");
        return -1;
    }

    /* 4) 诊断：chardev set 后读回 PL061 GPIODATA 核对引脚电平。
      *    期望 DC=0 RES=1 CS=1。若不符 -> chardev 不驱动引脚（内核 GPIO 驱动问题）。 */
    {
        int dc_v = pl061_peek_bit(GPIO4_BASE, 5);
        int res_v = pl061_peek_bit(GPIO4_BASE, 4);
        int cs_v  = pl061_peek_bit(GPIO5_BASE, 1);
        printf("[spi] chardev-verify: DC=%d(expect0) RES=%d(expect1) CS=%d(expect1)"
               "  ==> %s\n", dc_v, res_v, cs_v,
               (dc_v == 0 && res_v == 1 && cs_v == 1)
                   ? "OK(chardev 真驱动了引脚)" : "MISMATCH(chardev 没驱动引脚!)");
        fflush(stdout);
    }

    /* 5) 复位屏。 */
    spi_hal_reset();
    printf("[spi] spi_hal_init done\n"); fflush(stdout);
    return 0;
}

void spi_hal_deinit(void)
{
    if (g_spidev >= 0) {
        close(g_spidev);
        g_spidev = -1;
    }
    gpio_handle_close(&g_dc);
    gpio_handle_close(&g_res);
    gpio_handle_close(&g_cs);
}

void spi_hal_write_cmd(uint8_t cmd)
{
    gpio_set_value(&g_dc, 0);   /* DC=0 命令 */
    spidev_write(&cmd, 1);
}

void spi_hal_write_cmds(const uint8_t *cmds, int n)
{
    gpio_set_value(&g_dc, 0);   /* DC=0 命令 */
    spidev_write(cmds, n);
}

void spi_hal_write_data(const uint8_t *data, int len)
{
    gpio_set_value(&g_dc, 1);   /* DC=1 数据 */
    spidev_write(data, len);
}
