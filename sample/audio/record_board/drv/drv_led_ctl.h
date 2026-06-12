#ifndef __DRV_LED_CTL_H__
#define __DRV_LED_CTL_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

typedef enum LP5864_POWER {
    POWER_UP    = 0x01,
    POWER_DOWN  = 0x00
} LP5864_POWER_E;


typedef enum LP5864_MODE {
    LP5864_MODE_1 = 0,
    LP5864_MODE_2 = 1,
    LP5864_MODE_3 = 2,
} LP5864_MODE_E;

#define LP5864_MAX_LEDS         72
/** lp5864 I2C addr */
#define LP5864_SLAVE_ADDR       0x48
#define LP5864_SLAVE_ADDR_PWM   0x4a
/** lp5864 reg */
#define LP5864_REG_CHIP_EN      0x00
#define LP5864_REG_DEV_INIT     0x01
#define LP5864_REG_DEV_CONFIG1  0x02
#define LP5864_REG_DEV_CONFIG2  0x03
#define LP5864_REG_DEV_CONFIG3  0x04
#define LP5864_REG_GLOBAL_BRI   0x05
#define LP5864_REG_GROUP0_BRI   0x06
#define LP5864_REG_GPOUP1_BRI   0x07
#define LP5864_REG_GROUP2_BRI   0x08
#define LP5864_REG_RESET        0x9A

int lp5864_power(LP5864_POWER_E power);
int lp5864_set_mode(LP5864_MODE_E mode);
int lp5864_mode1_update_led(unsigned int led_number);
void lp5864_model1_led_test(void);

#endif