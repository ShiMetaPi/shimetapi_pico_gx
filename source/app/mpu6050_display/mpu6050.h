/*
 * mpu6050.h - MPU6050 六轴（三轴加速度 + 三轴陀螺仪 + 温度）驱动（I2C）。
 *
 * 挂在 I2C3（/dev/i2c-3），与 OLED(0x3C) 共享总线；AD0 接地 -> 7 位从地址 0x68。
 * 数据读取用 i2c_hal_read 从 0x3B 起一次读 14 字节连续块
 * （加速度 6 + 温度 2 + 陀螺仪 6），驱动内完成物理量换算。
 *
 * 量程：加速度 ±2g（灵敏度 16384 LSB/g）、陀螺仪 ±250°/s（131 LSB/°/s）——
 * MPU6050 上电默认档，init 里再显式写一遍保证与换算系数一致。
 */
#ifndef MPU6050_H
#define MPU6050_H

#include <stdint.h>

/* ---- 硬件配置（按板子接线修改）---- */
#ifndef MPU6050_BUS
#define MPU6050_BUS   3       /* GPIO4_1(SCL)/GPIO4_2(SDA) = i2c_bus3 -> /dev/i2c-3 */
#endif
#ifndef MPU6050_ADDR
#define MPU6050_ADDR  0x68    /* AD0 接地；AD0 接高则 0x69 */
#endif

/* ---- 寄存器（datasheet §7）---- */
#define MPU_REG_SMPLRT_DIV     0x19
#define MPU_REG_CONFIG         0x1A   /* DLPF_CFG 等 */
#define MPU_REG_GYRO_CONFIG    0x1B   /* bit[4:3] FS_SEL：0=±250°/s */
#define MPU_REG_ACCEL_CONFIG   0x1C   /* bit[4:3] AFS_SEL：0=±2g */
#define MPU_REG_INT_ENABLE     0x38
#define MPU_REG_ACCEL_XOUT_H   0x3B   /* 起 14 字节：ax,ay,az,temp,gx,gy,gz */
#define MPU_REG_PWR_MGMT_1     0x6B   /* bit7 DEVICE_RESET, bit6 SLEEP, bit[2:0] CLKSEL */
#define MPU_REG_WHO_AM_I       0x75
#define MPU_WHO_AM_I_VAL       0x68   /* MPU6050；MPU6500=0x70、MPU9250/9255=0x71，驱动均兼容 */

/* 采样后物理量（驱动内换算好，main 直接拿来显示）。 */
typedef struct {
    float ax, ay, az;   /* 加速度，单位 g */
    float gx, gy, gz;   /* 角速度，单位 °/s */
    float temp;         /* 温度，单位 ℃ */
} mpu6050_data_t;

/* 打开 I2C 并初始化 MPU6050（复位、唤醒、配量程、关中断）。成功 0，失败 -1。 */
int  mpu6050_init(void);

/* 读一次六轴 + 温度，换算成物理量写入 out。成功 0，失败 -1。 */
int  mpu6050_read(mpu6050_data_t *out);

/* 关 I2C 句柄。 */
void mpu6050_deinit(void);

#endif /* MPU6050_H */
