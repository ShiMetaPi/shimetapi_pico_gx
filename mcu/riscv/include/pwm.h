#ifndef _PWM_H_
#define _PWM_H_

#include <platform.h>
#include <sys/types.h>
#include <io.h>
#include <lib.h>

int pwm_enable(int pwm_id);
int pwm_disable(int pwm_id);
int pwm_config(int pwm_id, int duty_cycle_us, int period_us);


#endif
