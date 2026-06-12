#ifndef __DRV_I2C_H__
#define __DRV_I2C_H__

#include <stdio.h>
#include <unistd.h>

int i2c_write(int fd, unsigned int dev_addr, unsigned int reg_addr, unsigned char *data, int len);
int i2c_read(int fd, unsigned int dev_addr, unsigned int reg_addr, unsigned char *val, int len);

#endif