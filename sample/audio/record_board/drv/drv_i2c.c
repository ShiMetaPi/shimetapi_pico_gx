#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "drv_i2c.h"

int i2c_write(int fd, unsigned int dev_addr, unsigned int reg_addr, unsigned char *data, int len)
{
    int ret;
    unsigned char *wr_data = NULL;

    ret = ioctl(fd, I2C_SLAVE, dev_addr);
    if (ret < 0) {
        printf("set i2c slave addr failed\n");
        return ret;
    }

    wr_data = malloc(len + 1);
    if (!wr_data) {
        printf("malloc failed\n");
        return -1;
    }

    wr_data[0] = reg_addr;
    memcpy(&wr_data[1], data, len);

    ret = write(fd, wr_data, len+1);
    if (ret < 0) {
        printf("i2c write addr 0x%x failed\n", reg_addr);
    }

    free(wr_data);
    return ret;
}

int i2c_read(int fd, unsigned int dev_addr, unsigned int reg_addr, unsigned char *val, int len)
{
    int ret;
    unsigned char buf[1];

    ret = ioctl(fd, I2C_SLAVE, dev_addr);
    if (ret < 0) {
        printf("set i2c slave addr failed\n");
        return -1;
    }

    buf[0] = reg_addr;

    ret = write(fd, buf, 1);
    if (ret < 0) {
        printf("write addr failed\n");
        return ret;
    }

    ret = read(fd, val, len);
    if (ret < 0) {
        printf("read value failed\n");
    }

    return ret;
}