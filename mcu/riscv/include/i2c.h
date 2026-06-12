#ifndef _I2C_H_
#define _I2C_H_

#include <platform.h>
#include <sys/types.h>
#include <io.h>
#include <lib.h>

struct i2c_client {
	unsigned char i2c_num;
	unsigned short dev_addr;
	unsigned long int reg_addr;
	unsigned int reg_width;
};

int i2c_init(unsigned char i2c_num);
int i2c_recv(const struct i2c_client *client, unsigned char *buf, unsigned int count);
int i2c_send(unsigned char i2c_num, unsigned short dev_addr, const char *buf,unsigned int count);


#endif
