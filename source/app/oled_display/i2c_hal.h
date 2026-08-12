/*
 * i2c_hal.h - 用户态 I2C 读写封装（基于 /dev/i2c-X + I2C_RDWR）
 *
 * 平台：GK7602V11A (xmorca)，I2C 控制器驱动 i2c-lotus 内核 built-in，
 *       /dev/i2c-0..3 由内核自动创建，默认 100kHz。
 */
#ifndef I2C_HAL_H
#define I2C_HAL_H

#include <stdint.h>

/*
 * 打开 /dev/i2c-<bus> 并绑定 7 位从地址 addr。
 * 成功返回 fd(>=0)，失败返回 -1。
 * addr 传 0 表示暂不绑定（随后用 i2c_hal_write 里每条消息自带的 addr）。
 */
int i2c_hal_open(int bus, uint8_t addr);

/* 向从地址 addr 写入 len 字节。成功 0，失败 -1。 */
int i2c_hal_write(int fd, uint8_t addr, const uint8_t *data, int len);

/* 探测从地址是否应答（零长度写）。1=存在，0=不存在/不支持探测。 */
int i2c_hal_probe(int fd, uint8_t addr);

/* 关闭设备。 */
void i2c_hal_close(int fd);

#endif /* I2C_HAL_H */
