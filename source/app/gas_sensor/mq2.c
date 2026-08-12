/*
 * mq2.c - MQ2 气体传感器读 DO/AO 实现。
 *
 * DO: chardev /dev/gpiochip0 line 1 读电平（pad 默认 func0=GPIO，不用切）。
 * AO: 直接操作 LSADC 寄存器读 GPIO1_0 / LSADC_CH0。
 *     xm_lsadc.ko 没编 USE_LSADC_CHANNEL_0，ioctl 走 chn=0 直接报错；硬件本身支持
 *     CH0（CONFIG bit 8 = 使能、CHNDATA + 0 << 2 = 偏移 0x2C），所以开 /dev/lsadc
 *     让驱动帮忙 enable clock + exit reset + 使能控制器，然后我们用 /dev/mem mmap
 *     寄存器块自己 enable CH0 + start + read + stop。
 */
#include "mq2.h"
#include "gpio_hal.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/mman.h>

/* MQ2 AO 接 LSADC_CH0（详见 mq2.h 注释）。 */
#define MQ2_AO_CHN       0
#define MQ2_LSADC_DEV    "/dev/lsadc"

/* MQ2 DO 接 GPIO0_1（chardev）。 */
#define MQ2_DO_CHIP      "/dev/gpiochip0"
#define MQ2_DO_LINE      1

/* LSADC 寄存器（基址 0x120a0000）。参考 source/gmp/drv/lsadc/adc.c 头定义。 */
#define LSADC_REG_BASE    0x120a0000u
#define LSADC_REG_SIZE    0x100u      /* 单页就够，覆盖 CONFIG/START/STOP/CHNDATA */
#define PAGE_SIZE         0x1000u
#define LSADC_OFS_CONFIG  0x00u       /* bit8=CH0 en, bit9=CH1 en, bit10=CH2 en,
                                       * bit13=mode(0=single,1=cont), bit15=reset */
#define LSADC_OFS_START   0x1Cu
#define LSADC_OFS_STOP    0x20u
#define LSADC_OFS_CHNDATA 0x2Cu       /* CH0 = +0 << 2, CH1 = +4, CH2 = +8 */

static int          g_lsadc_fd  = -1;   /* /dev/lsadc：让驱动帮忙 enable clock + exit reset */
static int          g_mem_fd    = -1;   /* /dev/mem */
static volatile uint32_t *g_reg = NULL; /* mmap 到 LSADC 基址 */
static gpio_handle_t g_do;

/* 读 / 写 32-bit LSADC 寄存器（带 volatile）。 */
static inline uint32_t lsadc_r(uint32_t ofs)
{
    return g_reg[ofs >> 2];
}
static inline void lsadc_w(uint32_t ofs, uint32_t val)
{
    g_reg[ofs >> 2] = val;
}
static inline void lsadc_rmw(uint32_t ofs, uint32_t val, uint32_t mask)
{
    uint32_t t = g_reg[ofs >> 2];
    t = (t & ~mask) | (val & mask);
    g_reg[ofs >> 2] = t;
}

/* 开 /dev/lsadc（驱动使能时钟 + 退出 reset）+ /dev/mem mmap 寄存器。
 * 如果 /dev/lsadc 不存在（xm_lsadc.ko 没 insmod），自动尝试 insmod。 */
static int lsadc_open(void)
{
    g_lsadc_fd = open(MQ2_LSADC_DEV, O_RDWR);
    if (g_lsadc_fd < 0 && errno == ENOENT) {
        /* 节点不存在，尝试 insmod xm_lsadc.ko */
        if (access("/opt/ko/xm_lsadc.ko", R_OK) == 0) {
            printf("[mq2] /dev/lsadc 不存在，自动 insmod xm_lsadc.ko ...\n");
            if (system("insmod /opt/ko/xm_lsadc.ko") != 0) {
                fprintf(stderr, "[mq2] insmod xm_lsadc.ko 失败\n");
                return -1;
            }
            /* udev/mdev 创建设备节点可能要几百 ms，等一下 */
            usleep(300000);
            g_lsadc_fd = open(MQ2_LSADC_DEV, O_RDWR);
        }
    }
    if (g_lsadc_fd < 0) {
        fprintf(stderr, "[mq2] open %s: %s（%s）\n",
                MQ2_LSADC_DEV, strerror(errno),
                errno == ENOENT ? "尝试 insmod /opt/ko/xm_lsadc.ko 也失败" : "");
        return -1;
    }
    /* 不做任何 ioctl——避免驱动改 CONFIG。CH0 使能由我们自己 mmap 后写。 */

    g_mem_fd = open("/dev/mem", O_RDWR);
    if (g_mem_fd < 0) {
        fprintf(stderr, "[mq2] open /dev/mem: %s\n", strerror(errno));
        return -1;
    }
    volatile uint8_t *m = (volatile uint8_t *)mmap(NULL, PAGE_SIZE,
            PROT_READ | PROT_WRITE, MAP_SHARED, g_mem_fd,
            LSADC_REG_BASE & ~(PAGE_SIZE - 1u));
    if (m == MAP_FAILED) {
        fprintf(stderr, "[mq2] mmap LSADC: %s\n", strerror(errno));
        return -1;
    }
    g_reg = (volatile uint32_t *)(m + (LSADC_REG_BASE & (PAGE_SIZE - 1u)));

    /* 强制切到 single-step 模式（避免驱动残留的 continuous 状态）。 */
    lsadc_rmw(LSADC_OFS_CONFIG, 0u, 1u << 13);   /* bit13 = 0: single */

    printf("[mq2] LSADC 寄存器已 mmap @ 0x%08X (CH0 走 /dev/mem 绕过驱动)\n",
           LSADC_REG_BASE);
    return 0;
}

static void lsadc_close(void)
{
    if (g_reg != NULL) {
        /* 关闭前确保 CH0 已禁能 */
        lsadc_rmw(LSADC_OFS_CONFIG, 0u, 1u << 8);
        munmap((void *)((uintptr_t)g_reg - (LSADC_REG_BASE & (PAGE_SIZE - 1u))),
               PAGE_SIZE);
        g_reg = NULL;
    }
    if (g_mem_fd >= 0) {
        close(g_mem_fd);
        g_mem_fd = -1;
    }
    if (g_lsadc_fd >= 0) {
        close(g_lsadc_fd);    /* 触发 lsadc_release：进 reset、关 clock */
        g_lsadc_fd = -1;
    }
}

/* 读一次 AO 原始值（10-bit）。失败返回 -1。 */
static int lsadc_read_ao(void)
{
    if (g_reg == NULL) {
        return -1;
    }
    /* 1. 使能 CH0 */
    lsadc_rmw(LSADC_OFS_CONFIG, 1u << 8, 1u << 8);
    /* 2. 触发 start（single-step：写 1） */
    lsadc_w(LSADC_OFS_START, 1u);
    /* 3. 等转换完成（参考 sample 的 usleep(1000*1000)，其实 1ms 就够） */
    usleep(2000);
    /* 4. 读 CH0 数据（CHNDATA 偏移 +0 << 2 = 0x2C），12-bit mask 0xFFF */
    int v = (int)(lsadc_r(LSADC_OFS_CHNDATA + (MQ2_AO_CHN << 2)) & 0xFFFu);
    /* 5. 触发 stop（写 1） */
    lsadc_w(LSADC_OFS_STOP, 1u);
    /* 6. 禁能 CH0（不强制，保留也行；保留更省一次写） */
    return v;
}

int mq2_init(void)
{
    /* 配 DO 输入。GPIO0_1 默认 func0=GPIO，pad 不需要切。 */
    memset(&g_do, 0, sizeof(g_do));
    g_do.chip_path      = MQ2_DO_CHIP;
    g_do.line_offset    = MQ2_DO_LINE;
    g_do.gpio_mode      = GPIOHANDLE_REQUEST_INPUT;
    g_do.default_value  = 0;
    snprintf(g_do.consumer_label, sizeof(g_do.consumer_label), "mq2-do");
    if (gpio_handle_init(&g_do) < 0) {
        fprintf(stderr, "[mq2] GPIO0_1 申请输入失败：%s\n", strerror(errno));
        return -1;
    }
    /* 配 AO 模拟输入（绕过驱动的 /dev/mem 直读）。由 main.c 在调本函数之前
     * 用 set_pad_func 写 iocfg_reg20 = 0x1004 让 pad 切到 LSADC_CH0 功能。 */
    if (lsadc_open() < 0) {
        gpio_handle_close(&g_do);
        return -1;
    }
    printf("[mq2] @ DO=GPIO0_1(/dev/gpiochip0 line1) + AO=GPIO1_0/LSADC_CH0(/dev/mem 直读)\n");
    return 0;
}

int mq2_read(mq2_data_t *out)
{
    int v;

    if (out == NULL) {
        return -1;
    }
    out->do_level = gpio_get_value(&g_do);
    v = lsadc_read_ao();
    if (v < 0) {
        out->ao_raw = -1;
        out->ao_pct = -1;
        out->ao_err = -1;
        return -1;
    }
    out->ao_raw = v;
    out->ao_pct = (v * 100) / 4095;   /* 12-bit 满量程 4095 */
    out->ao_err = 0;
    return 0;
}

void mq2_deinit(void)
{
    lsadc_close();
    gpio_handle_close(&g_do);
}