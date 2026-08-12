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

/*
 * 寄存器读：先写 reg 号（repeated-start），再读 len 字节到 buf。
 * 用于连续寄存器块读（如 MPU6050 从 0x3B 一次读 14 字节）。成功 0，失败 -1。
 */
int i2c_hal_read(int fd, uint8_t addr, uint8_t reg, uint8_t *buf, int len);

/* 纯读（无寄存器号，单 msg read）：用于 SHT20 等"先发命令、再直接读"的传感器。成功 0，失败 -1。 */
int i2c_hal_read_noreg(int fd, uint8_t addr, uint8_t *buf, int len);

/* 探测从地址是否应答（零长度写）。1=存在，0=不存在/不支持探测。 */
int i2c_hal_probe(int fd, uint8_t addr);

/* 关闭设备。 */
void i2c_hal_close(int fd);

#endif /* I2C_HAL_H */
