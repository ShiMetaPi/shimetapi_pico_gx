#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "drv_led_ctl.h"
#include "drv_i2c.h"

#define LP5864_DEV_I2C_NODE     "/dev/i2c-0"

static int lp5864_i2c_write(unsigned int dev_addr, unsigned int reg_addr, unsigned int data)
{
    int fd;
    int ret;
    unsigned char buf[1] = { 0 };

    fd = open(LP5864_DEV_I2C_NODE, O_RDWR);
    if (fd < 0) {
        printf("open %s failed\n", LP5864_DEV_I2C_NODE);
        return -1;
    }

    buf[0] = data;

    ret = i2c_write(fd, dev_addr, reg_addr, buf, 1);

    close(fd);

    return ret;
}

static int lp5864_i2c_read(unsigned int dev_addr, unsigned int reg_addr, unsigned char *value)
{
    int fd;
    int ret;

    fd = open(LP5864_DEV_I2C_NODE, O_RDWR);
    if (fd < 0) {
        printf("open %s failed\n", LP5864_DEV_I2C_NODE);
        return -1;
    }

    ret = i2c_read(fd, dev_addr, reg_addr, value, 1);

    close(fd);

    return ret;
}

int lp5864_power(LP5864_POWER_E power)
{
    return lp5864_i2c_write(LP5864_SLAVE_ADDR, LP5864_REG_CHIP_EN, power);
}

int lp5864_set_mode(LP5864_MODE_E mode)
{
    unsigned char value;
    lp5864_i2c_read(LP5864_SLAVE_ADDR, LP5864_REG_DEV_INIT, &value);

    value = value & (mode << 1);

    return lp5864_i2c_write(LP5864_SLAVE_ADDR, LP5864_REG_DEV_INIT, value);
}

int lp5864_mode1_update_led(unsigned int led_number)
{
    int i;
    unsigned int onoff_reg;

    for (i = 0; i < LP5864_MAX_LEDS; i++) {
        lp5864_i2c_write(LP5864_SLAVE_ADDR_PWM, i, 0x00);
    }

    onoff_reg = 71 - (0x12 * (led_number % 4) + (led_number / 4));

    printf("lp5864 open led number: %d\n", onoff_reg);

    return lp5864_i2c_write(LP5864_SLAVE_ADDR_PWM, onoff_reg, 0xff);
}

void lp5864_model1_led_test(void)
{
    int i;
    // one by one open led.
    for (i = 0; i < LP5864_MAX_LEDS; i++) {
        lp5864_i2c_write(LP5864_SLAVE_ADDR_PWM, i, 0xff);
        usleep(10000);
    }
    // close all leds.
    for (i = 0; i < LP5864_MAX_LEDS; i++) {
        lp5864_i2c_write(LP5864_SLAVE_ADDR_PWM, i, 0x00);
    }
}