/*
 * pca9685.h - PCA9685 16 路 PWM/舵机驱动（I2C）。
 *
 * PCA9685 是 NXP 出的 16 路 12-bit PWM 控制器，I2C 接口：
 *   地址：0x40~0x7F（默认 0x40，由 A0~A5 跳线决定）
 *   PWM 频率：24Hz ~ 1526Hz（典型舵机 50Hz）
 *   分辨率：12 bit (0..4095) / 周期
 *
 * 板端资源（与 OLED/MPU6050/SHT20 共享 I2C3）：
 *   SCL = GPIO4_1 @ 0x100C0010 func2
 *   SDA = GPIO4_2 @ 0x100C0014 func2
 *   OE  = GPIO4_5 @ 0x100C0020 func5（active LOW：LOW=输出使能，HIGH=全通道关闭）
 *
 * 寄存器布局（参考 PCA9685 datasheet）：
 *   0x00 MODE1     [7]=RESTART [5]=AI [4]=SLEEP [3]=SUB1..[0]=SUB3/ALLCALL
 *   0x01 MODE2     [5]=INVRT [4]=OCH [3..2]=OUTDRV [1]=OUTNE1 [0]=OUTNE0
 *   0x06 LED0_ON_L  ...0x45 LED15_OFF_H  (每个通道 4 字节：ON_L/ON_H/OFF_L/OFF_H)
 *   0xFA ALL_LED_ON_L ... 0xFD ALL_LED_OFF_H  (4 字节)
 *   0xFE PRE_SCALE  PWM 频率分频器 = round(25e6 / (4096 * freq)) - 1
 *   0xFB..0xFD 分别是 ALL_LED 的 L/H/L/H，跟 0x06..0x45 同样布局
 *
 * 舵机典型用法：
 *   50Hz PWM（周期 20ms）
 *   1ms 脉宽 = 0°
 *   1.5ms      = 90°
 *   2ms        = 180°
 *   12-bit 计数：1ms = 205 counts, 1.5ms = 307, 2ms = 410
 *   实际常用：500..2500μs（180° 舵机），1000..2000μs（90° 舵机）
 */
#ifndef PCA9685_H
#define PCA9685_H

#include <stdint.h>

#ifndef PCA9685_BUS
#define PCA9685_BUS     3           /* /dev/i2c-3 */
#endif
#ifndef PCA9685_ADDR
#define PCA9685_ADDR    0x41        /* 板端 A0 跳线已桥接：默认 0x40 → 0x41 */
#endif

#define PCA9685_CHANNELS   16

/* 舵机常用脉宽范围（微秒），与具体舵机型号相关 */
#define SERVO_PULSE_MIN_US  500
#define SERVO_PULSE_MAX_US  2500
#define SERVO_PULSE_MID_US  1500

/* 打开 I2C + 软件复位 + 设 PWM 频率 + 唤醒。频率 50Hz 适合舵机。
 * 返回 0 成功，-1 失败。 */
int pca9685_init(int pwm_freq_hz);

/* 设置某通道 PWM 脉宽（μ秒）。0 = 关闭输出。返回 0 成功，-1 失败。 */
int pca9685_set_pulse(uint8_t ch, uint16_t us);

/* 设置某通道原始 12-bit OFF count（0..4095）。高级用法。 */
int pca9685_set_off_count(uint8_t ch, uint16_t off_count);

/* 全部通道设为同一脉宽。 */
int pca9685_set_all_pulse(uint16_t us);

/* 全通道关闭（OFF count = 0）。 */
int pca9685_all_off(void);

/* 设 PWM 频率（Hz），24..1526。返回 0 成功，-1 失败。 */
int pca9685_set_pwm_freq(int freq_hz);

void pca9685_deinit(void);

#endif /* PCA9685_H */