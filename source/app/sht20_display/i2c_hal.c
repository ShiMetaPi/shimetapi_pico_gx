/*
 * i2c_hal.c - 用户态 I2C 读写封装实现。
 *
 * 用 I2C_RDWR 复合事务（每条 i2c_msg 自带从地址），不依赖 I2C_SLAVE 预绑定，
 * 是 Linux 用户态访问 I2C 外设的标准、可移植写法。
 */
#include "i2c_hal.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

int i2c_hal_open(int bus, uint8_t addr)
{
    char path[32];

    snprintf(path, sizeof(path), "/dev/i2c-%d", bus);
    int fd = open(path, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "[i2c] open %s failed: %s\n", path, strerror(errno));
        return -1;
    }
    if (addr != 0) {
        /* I2C_SLAVE 预绑定；即使失败也无妨，I2C_RDWR 自带 addr 仍可用。 */
        if (ioctl(fd, I2C_SLAVE, addr) < 0) {
            fprintf(stderr, "[i2c] I2C_SLAVE(0x%02x) warn: %s\n", addr, strerror(errno));
        }
    }
    return fd;
}

int i2c_hal_write(int fd, uint8_t addr, const uint8_t *data, int len)
{
    struct i2c_msg msg;
    struct i2c_rdwr_ioctl_data ioctl_data;

    if (fd < 0 || len <= 0) {
        return -1;
    }

    msg.addr = addr;
    msg.flags = 0;
    msg.len = len;
    msg.buf = (uint8_t *)data; /* i2c_msg.buf 非常量，这里去掉 const（不改写） */

    ioctl_data.msgs = &msg;
    ioctl_data.nmsgs = 1;

    if (ioctl(fd, I2C_RDWR, &ioctl_data) < 0) {
        fprintf(stderr, "[i2c] write(addr=0x%02x, len=%d) failed: %s\n",
                addr, len, strerror(errno));
        return -1;
    }
    return 0;
}

/*
 * 寄存器读：I2C_RDWR 复合事务——msg1 写入 1 字节寄存器号（控制器在两条消息
 * 之间发 repeated-start，不释放总线），msg2 读回 len 字节。等价于
 * i2c_smbus_read_i2c_block_data，但用 I2C_RDWR 更通用，不要求控制器支持
 * SMBus block read。MPU6050 等传感器连续寄存器块读就用它。
 */
int i2c_hal_read(int fd, uint8_t addr, uint8_t reg, uint8_t *buf, int len)
{
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data ioctl_data;

    if (fd < 0 || buf == NULL || len <= 0) {
        return -1;
    }

    msgs[0].addr  = addr;
    msgs[0].flags = 0;
    msgs[0].len   = 1;
    msgs[0].buf   = &reg;

    msgs[1].addr  = addr;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len   = (uint16_t)len;
    msgs[1].buf   = buf;

    ioctl_data.msgs  = msgs;
    ioctl_data.nmsgs = 2;

    if (ioctl(fd, I2C_RDWR, &ioctl_data) < 0) {
        fprintf(stderr, "[i2c] read(addr=0x%02x, reg=0x%02x, len=%d) failed: %s\n",
                addr, reg, len, strerror(errno));
        return -1;
    }
    return 0;
}

/*
 * 纯读（无寄存器号）：单条 I2C_M_RD msg，不先写寄存器号。
 * 用于 SHT20/SI7021 等"先发测量命令、等转换完成、再直接读结果"的传感器
 * （它们的"读"阶段不带寄存器号，与 MPU6050 的"写 reg + restart read"不同）。
 */
int i2c_hal_read_noreg(int fd, uint8_t addr, uint8_t *buf, int len)
{
    struct i2c_msg msg;
    struct i2c_rdwr_ioctl_data ioctl_data;

    if (fd < 0 || buf == NULL || len <= 0) {
        return -1;
    }

    msg.addr  = addr;
    msg.flags = I2C_M_RD;
    msg.len   = (uint16_t)len;
    msg.buf   = buf;

    ioctl_data.msgs  = &msg;
    ioctl_data.nmsgs = 1;

    if (ioctl(fd, I2C_RDWR, &ioctl_data) < 0) {
        fprintf(stderr, "[i2c] read_noreg(addr=0x%02x, len=%d) failed: %s\n",
                addr, len, strerror(errno));
        return -1;
    }
    return 0;
}

int i2c_hal_probe(int fd, uint8_t addr)
{
    struct i2c_msg msg;
    struct i2c_rdwr_ioctl_data ioctl_data;

    if (fd < 0) {
        return 0;
    }
    /* 零长度写：完成即有 ACK。部分控制器不支持零长度，则视为探测失败。 */
    msg.addr = addr;
    msg.flags = 0;
    msg.len = 0;
    msg.buf = NULL;

    ioctl_data.msgs = &msg;
    ioctl_data.nmsgs = 1;

    return ioctl(fd, I2C_RDWR, &ioctl_data) < 0 ? 0 : 1;
}

void i2c_hal_close(int fd)
{
    if (fd >= 0) {
        close(fd);
    }
}
