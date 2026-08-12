/*
 * sht20.h - SHT20 / SI7021 / HTU21 温湿度传感器驱动（I2C，no-hold-master）。
 *
 * 挂在 I2C3（/dev/i2c-3），与 OLED(0x3C)/MPU6050(0x68) 共享总线；地址 0x40。
 * 测量：发 0xF3(温度)/0xF5(湿度) 触发 no-hold-master，等转换完成(~85ms)后直接读 3 字节
 *       (MSB, LSB, CRC)，不依赖 I2C clock-stretching。
 * 换算：T = -46.85 + 175.72 * raw/2^16；RH = -6 + 125 * raw/2^16（SHT20 datasheet）。
 */
#ifndef SHT20_H
#define SHT20_H

#include <stdint.h>

#ifndef SHT20_BUS
#define SHT20_BUS  3       /* GPIO4_1(SCL)/GPIO4_2(SDA) = i2c_bus3 -> /dev/i2c-3 */
#endif
#ifndef SHT20_ADDR
#define SHT20_ADDR 0x40    /* SHT20/SI7021/HTU21 固定地址 */
#endif

typedef struct {
    float temp;   /* 温度 ℃ */
    float rh;     /* 相对湿度 %RH（已裁剪到 0~100）*/
} sht20_data_t;

/* 打开 I2C + 触发一次温度测量验证应答。成功 0，失败 -1。
 * （效仿 oled_display 的 ssd1306_init：开 fd -> 直接写命令 -> 失败即报无应答） */
int  sht20_init(void);

/* 读一次温湿度（约 170ms，含两次转换）。成功 0，失败 -1。 */
int  sht20_read(sht20_data_t *out);

void sht20_deinit(void);

#endif /* SHT20_H */