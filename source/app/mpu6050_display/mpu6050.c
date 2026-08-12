/*
 * mpu6050.c - MPU6050 六轴传感器驱动实现。
 *
 * 写时序：MPU6050 寄存器写是「从地址 W | 寄存器号 | 数据...」，第一字节即寄存器号
 *   （不像 SSD1306 那样用控制字节区分命令/数据），所以写寄存器构造 {reg, val} 两字节。
 * 读时序：I2C_RDWR 复合事务，先写寄存器号（repeated-start）再读，见 i2c_hal_read。
 *
 * 换算系数（datasheet §4.18 / §6）：
 *   ±2g    -> a(g)   = raw / 16384
 *   ±250°/s-> ω(°/s) = raw / 131
 *   温度   -> T(℃)  = raw / 340 + 36.53
 */
#include "mpu6050.h"
#include "i2c_hal.h"

#include <stdio.h>
#include <unistd.h>

static int g_fd = -1;

/* 大端拼有符号 16 位：p[0] 高字节，p[1] 低字节。 */
static int16_t be_s16(const uint8_t *p)
{
    return (int16_t)(((uint16_t)p[0] << 8) | (uint16_t)p[1]);
}

/* 写单字节寄存器：buf = {寄存器号, 值}。 */
static int write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    return i2c_hal_write(g_fd, MPU6050_ADDR, buf, 2);
}

int mpu6050_init(void)
{
    uint8_t who = 0;

    g_fd = i2c_hal_open(MPU6050_BUS, MPU6050_ADDR);
    if (g_fd < 0) {
        return -1;
    }

    /* 1) 读 WHO_AM_I 校验。读不到（-1）= I2C 没通；值≠0x68 = 不是 MPU6050。 */
    if (i2c_hal_read(g_fd, MPU6050_ADDR, MPU_REG_WHO_AM_I, &who, 1) < 0) {
        fprintf(stderr, "[mpu] 读 WHO_AM_I 失败：I2C 没通（查 SCL/SDA pad 是否 func2、"
                "SCL/SDA 是否接反、AD0 是否接地、MPU6050 电源）。\n");
        goto fail;
    }
    printf("[mpu] WHO_AM_I = 0x%02X\n", who);
    /*
     * 接受 MPU6050(0x68) / MPU6500(0x70) / MPU9250·9255(0x71)：三者寄存器布局兼容，
     * 数据读取（0x3B 起 14 字节）、量程、唤醒序列完全一样。市售"MPU6050"模块很多
     * 实为 MPU6500，WHO_AM_I 返回 0x70，属正常，照常采集即可。
     */
    if (who != 0x68 && who != 0x70 && who != 0x71) {
        fprintf(stderr, "[mpu] WHO_AM_I=0x%02X：不是 MPU6050/6500/9250 兼容芯片"
                "（期望 0x68/0x70/0x71）。检查接线 / AD0 / 芯片型号。\n", who);
        goto fail;
    }
    printf("[mpu] 识别为 %s\n",
           who == 0x68 ? "MPU6050" : (who == 0x70 ? "MPU6500" : "MPU9250/9255"));

    /* 2) 设备复位（PWR_MGMT_1 bit7=1），等内部稳压器/时钟复位完成。 */
    if (write_reg(MPU_REG_PWR_MGMT_1, 0x80) < 0) {
        fprintf(stderr, "[mpu] 触发复位失败。\n");
        goto fail;
    }
    usleep(100000);   /* 复位典型 <100ms */

    /* 3) 唤醒 + 选 X 轴陀螺仪 PLL 做时钟源（比内部 RC 稳）：PWR_MGMT_1 = 0x01。 */
    if (write_reg(MPU_REG_PWR_MGMT_1, 0x01) < 0) {
        fprintf(stderr, "[mpu] 唤醒失败。\n");
        goto fail;
    }
    usleep(50000);

    /*
     * 4) 采样率 / 低通 / 量程：
     *    CONFIG=1 -> DLPF_CFG=1（陀螺 188Hz/1.9ms，加速度 184Hz/2.0ms），此时内部采样率=1kHz；
     *    SMPLRT_DIV=9 -> 输出 1kHz/(1+9)=100Hz（显示轮询足够；嫌噪声大可把 CONFIG 改 3 降带宽）。
     *    量程 ±2g / ±250°/s（上电默认档，显式写一遍防残留，与换算系数一致）。
     *    INT_ENABLE=0：轮询采集，不开 data-ready 中断。
     */
    write_reg(MPU_REG_SMPLRT_DIV,   9);     /* 100Hz 输出 */
    write_reg(MPU_REG_CONFIG,       1);     /* DLPF on，内部 1kHz */
    write_reg(MPU_REG_ACCEL_CONFIG, 0x00);  /* ±2g */
    write_reg(MPU_REG_GYRO_CONFIG,  0x00);  /* ±250°/s */
    write_reg(MPU_REG_INT_ENABLE,   0x00);  /* 轮询，关中断 */

    printf("[mpu] 初始化成功 @ /dev/i2c-%d addr 0x%02x (±2g / ±250dps, 100Hz)\n",
           MPU6050_BUS, MPU6050_ADDR);
    return 0;

fail:
    i2c_hal_close(g_fd);
    g_fd = -1;
    return -1;
}

int mpu6050_read(mpu6050_data_t *out)
{
    uint8_t buf[14];
    int16_t raw[7];
    int i;

    if (g_fd < 0 || out == NULL) {
        return -1;
    }

    /* 从 0x3B 连续读 14 字节：ax,ay,az,temp,gx,gy,gz（各 2 字节，高字节在前）。 */
    if (i2c_hal_read(g_fd, MPU6050_ADDR, MPU_REG_ACCEL_XOUT_H, buf, 14) < 0) {
        return -1;
    }

    for (i = 0; i < 7; i++) {
        raw[i] = be_s16(&buf[i * 2]);
    }
    out->ax   = (float)raw[0] / 16384.0f;
    out->ay   = (float)raw[1] / 16384.0f;
    out->az   = (float)raw[2] / 16384.0f;
    out->temp = (float)raw[3] / 340.0f + 36.53f;
    out->gx   = (float)raw[4] / 131.0f;
    out->gy   = (float)raw[5] / 131.0f;
    out->gz   = (float)raw[6] / 131.0f;
    return 0;
}

void mpu6050_deinit(void)
{
    if (g_fd >= 0) {
        write_reg(MPU_REG_PWR_MGMT_1, 0x40);   /* SLEEP=1，省电 */
        i2c_hal_close(g_fd);
        g_fd = -1;
    }
}
