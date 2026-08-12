/*
 * pca9685.c - PCA9685 16 路 PWM/舵机驱动（I2C，板端运行）。
 *
 * 协议（参考 PCA9685 datasheet rev 4 — 2015）：
 *   写寄存器：buf = {reg, val}，单字节寄存器直接 i2c_hal_write
 *   多字节写：buf = {start_reg, val0, val1, ...}，i2c_hal_write 一次发完
 *   设通道 PWM：写 4 字节到 LEDn_ON_L（基址 0x06 + n*4）
 *
 * 时序：
 *   1) 软件复位：写 0x06 到 MODE1
 *   2) 等 > 500us 让内部时钟稳定
 *   3) 设 PRE_SCALE（要 SLEEP=1 才能写）
 *   4) 清 SLEEP 唤醒，AI 自动递增开启
 *   5) 设通道 PWM（ON_L=0, ON_H=0, OFF_L=lo, OFF_H=hi）
 */
#include "pca9685.h"
#include "i2c_hal.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

/* PCA9685 寄存器地址 */
#define REG_MODE1       0x00u
#define REG_MODE2       0x01u
#define REG_LED0_ON_L   0x06u   /* 通道 0 ON  寄存器基址，每通道 +4 */
#define REG_ALL_LED_ON_L  0xFAu
#define REG_ALL_LED_OFF_L 0xFCu
#define REG_PRE_SCALE   0xFEu

/* MODE1 位 */
#define MODE1_RESTART   (1u << 7)
#define MODE1_EXTCLK    (1u << 6)
#define MODE1_AI        (1u << 5)   /* auto-increment */
#define MODE1_SLEEP     (1u << 4)
#define MODE1_SUB1      (1u << 3)
#define MODE1_SUB2      (1u << 2)
#define MODE1_SUB3      (1u << 1)
#define MODE1_ALLCALL   (1u << 0)

/* MODE2 位 */
#define MODE2_INVRT     (1u << 5)
#define MODE2_OCH       (1u << 4)   /* outputs change on STOP (0) vs ACK (1) */
#define MODE2_OUTDRV    (1u << 2)   /* 1=totem-pole（推挽）0=open-drain */
#define MODE2_OUTNE1    (1u << 1)
#define MODE2_OUTNE0    (1u << 0)

/* PCA9685 内部振荡器 25 MHz（typical），PRE_SCALE = round(25e6 / (4096 * freq)) - 1 */
#define PCA9685_OSC_HZ   25000000u

static int g_fd = -1;
static int g_freq_hz = 50;   /* 当前 PWM 频率，用于 set_pulse 计算 count */

static int write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    return i2c_hal_write(g_fd, PCA9685_ADDR, buf, 2);
}

static int read_reg(uint8_t reg, uint8_t *val)
{
    return i2c_hal_read(g_fd, PCA9685_ADDR, reg, val, 1);
}

int pca9685_set_pwm_freq(int freq_hz)
{
    uint8_t prescale;
    uint8_t old_mode;

    if (freq_hz < 24 || freq_hz > 1526) {
        fprintf(stderr, "[pca] freq %d Hz 超出范围 24..1526\n", freq_hz);
        return -1;
    }

    g_freq_hz = freq_hz;   /* 记录下来给 set_pulse 用 */

    prescale = (uint8_t)((PCA9685_OSC_HZ / (4096u * (uint32_t)freq_hz)) - 1u);
    /* 读 MODE1 */
    if (read_reg(REG_MODE1, &old_mode) < 0) {
        return -1;
    }
    uint8_t new_mode = (old_mode & ~MODE1_RESTART) | MODE1_SLEEP;   /* 进 sleep 才能改 prescale */
    if (write_reg(REG_MODE1, new_mode) < 0) {
        return -1;
    }
    if (write_reg(REG_PRE_SCALE, prescale) < 0) {
        return -1;
    }
    /* 退出 sleep（保留 AI 自动递增 + ALLCALL） */
    new_mode = (old_mode & ~MODE1_SLEEP) | MODE1_AI | MODE1_ALLCALL;
    if (write_reg(REG_MODE1, new_mode) < 0) {
        return -1;
    }
    /* 等内部振荡器稳定（datasheet 至少 500us） */
    usleep(700);

    /* 设 MODE2：推挽输出（直接驱动 servo 信号线）+ 不反转 */
    if (write_reg(REG_MODE2, MODE2_OUTDRV) < 0) {
        return -1;
    }
    return 0;
}

int pca9685_init(int pwm_freq_hz)
{
    uint8_t mode1 = 0;

    g_fd = i2c_hal_open(PCA9685_BUS, PCA9685_ADDR);
    if (g_fd < 0) {
        return -1;
    }
    /* 软件复位：写 0x06 到 MODE1（特殊值） */
    if (write_reg(REG_MODE1, MODE1_RESTART) < 0) {
        fprintf(stderr, "[pca] 软件复位失败\n");
        goto fail;
    }
    usleep(10000);   /* 复位后等 ~10ms */

    /* 读 MODE1 确认设备活着 */
    if (read_reg(REG_MODE1, &mode1) < 0) {
        fprintf(stderr, "[pca] 读 MODE1 失败\n");
        goto fail;
    }
    printf("[pca] MODE1=0x%02X (reset OK)\n", mode1);

    /* 设 PWM 频率 */
    if (pca9685_set_pwm_freq(pwm_freq_hz) < 0) {
        goto fail;
    }

    /* 全通道关闭 */
    pca9685_all_off();

    printf("[pca] @ /dev/i2c-%d addr 0x%02x init OK, PWM %d Hz\n",
           PCA9685_BUS, PCA9685_ADDR, pwm_freq_hz);
    return 0;

fail:
    i2c_hal_close(g_fd);
    g_fd = -1;
    return -1;
}

/* 设某通道 OFF 12-bit count（高 4 bit 在 OFF_H bit[3:0]，低 8 bit 在 OFF_L）。 */
int pca9685_set_off_count(uint8_t ch, uint16_t off_count)
{
    if (ch >= PCA9685_CHANNELS) {
        return -1;
    }
    off_count &= 0x0FFFu;   /* 12-bit mask */

    /* 寄存器布局：ON_L, ON_H, OFF_L, OFF_H 从基址 0x06+n*4 开始 */
    uint8_t reg = REG_LED0_ON_L + (ch * 4);
    uint8_t buf[5];
    buf[0] = reg;             /* 起始寄存器 */
    buf[1] = 0;               /* LEDn_ON_L  = 0 */
    buf[2] = 0;               /* LEDn_ON_H  = 0 */
    buf[3] = (uint8_t)(off_count & 0xFFu);              /* LEDn_OFF_L = 低 8 位 */
    buf[4] = (uint8_t)((off_count >> 8) & 0x0Fu);       /* LEDn_OFF_H = 高 4 位 */
    return i2c_hal_write(g_fd, PCA9685_ADDR, buf, 5);
}

/* 设某通道 PWM 脉宽（μ秒）。输入超出 [0, period_us] 截断到 OFF 全 1 (bit12=0 全周期高)。 */
int pca9685_set_pulse(uint8_t ch, uint16_t us)
{
    if (g_fd < 0) {
        return -1;
    }
    if (ch >= PCA9685_CHANNELS) {
        return -1;
    }

    /* period_us = 1e6 / freq_hz
     * count = us * 4096 / period_us
     * 用 32-bit 防溢出 */
    uint32_t period_us = 1000000u / (uint32_t)g_freq_hz;
    uint16_t off_count;
    if (us == 0 || us >= period_us) {
        off_count = 0x1000u;   /* bit 12=1 = full off */
    } else {
        off_count = (uint16_t)(((uint32_t)us * 4096u) / period_us);
        if (off_count >= 0x1000u) {
            off_count = 0x0FFFu;
        }
    }
    return pca9685_set_off_count(ch, off_count);
}

/* 设所有通道同一脉宽。 */
int pca9685_set_all_pulse(uint16_t us)
{
    if (g_fd < 0) {
        return -1;
    }
    uint32_t period_us = 1000000u / (uint32_t)g_freq_hz;
    uint16_t off_count;
    if (us == 0 || us >= period_us) {
        off_count = 0x1000u;
    } else {
        off_count = (uint16_t)(((uint32_t)us * 4096u) / period_us);
        if (off_count >= 0x1000u) {
            off_count = 0x0FFFu;
        }
    }

    uint8_t buf[5];
    buf[0] = REG_ALL_LED_ON_L;
    buf[1] = 0;                  /* ALL_LED_ON_L */
    buf[2] = 0;                  /* ALL_LED_ON_H */
    buf[3] = (uint8_t)(off_count & 0xFFu);
    buf[4] = (uint8_t)((off_count >> 8) & 0x0Fu);
    return i2c_hal_write(g_fd, PCA9685_ADDR, buf, 5);
}

/* 全通道关闭（OFF count = 0x1000）。 */
int pca9685_all_off(void)
{
    if (g_fd < 0) {
        return -1;
    }
    uint8_t buf[5] = {
        REG_ALL_LED_ON_L,
        0x00, 0x00,           /* ALL_LED_ON = 0 */
        0x00, 0x10,           /* ALL_LED_OFF = bit12=1 (全 off) */
    };
    return i2c_hal_write(g_fd, PCA9685_ADDR, buf, 5);
}

void pca9685_deinit(void)
{
    if (g_fd >= 0) {
        /* 软件复位（写 0x06 到 MODE1），让所有通道回到默认状态 */
        write_reg(REG_MODE1, MODE1_RESTART);
        usleep(5000);
        i2c_hal_close(g_fd);
        g_fd = -1;
    }
}