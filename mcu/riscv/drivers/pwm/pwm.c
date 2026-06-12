#include <io.h>
#include "pwm.h"

static void pwm_set_bits(void* base, u32 offset, unsigned int mask, u32 data)
{
	void *address = base + offset;
	unsigned int value;

	value = readl(address);
	value &= ~mask;
	value |= (data & mask);
	writel(value, address);
}

static void get_pwm_clk(unsigned int *clk)
{
	void *pwm_clock_addr = (void *)PWM_CLOCK_ADDR_BASE;
	unsigned int val = readl(pwm_clock_addr);

	val = (val >> 8) & 0x3;
	switch (val) {
	case 0:
		*clk = PWM_CLOCK_3M;
		break;
	case 1:
		*clk = PWM_CLOCK_50M;
		break;
	default:
		*clk = PWM_CLOCK_24M;
		break;
	}
}

int pwm_enable(int pwm_id)
{
	if (pwm_id > PWM_MAX_NUM)
		return -1;

	pwm_set_bits((void *)PWM_ADDR_BASE, PWM_CTRL_ADDR(pwm_id), PWM_ENABLE_MASK, (0x1 << PWM_ENABLE_SHIFT));
	return 0;
}

int pwm_disable(int pwm_id)
{
	if (pwm_id > PWM_MAX_NUM)
		return -1;

	pwm_set_bits((void *)PWM_ADDR_BASE, PWM_CTRL_ADDR(pwm_id), PWM_ENABLE_MASK, (0x0 << PWM_ENABLE_SHIFT));
	return 0;
}

int pwm_config(int pwm_id, int duty_cycle_us, int period_us)
{
	unsigned int duty;
	unsigned int period, freq;
	unsigned int clk;

	if ((pwm_id > PWM_MAX_NUM) || (duty_cycle_us > period_us))
		return -1;

	get_pwm_clk(&clk);

	freq = clk/1000000;

	period = freq * period_us;
	duty = period * duty_cycle_us/period_us;

	if ((period < 2) || (duty < 1))
		return -1;

	pwm_set_bits((void *)PWM_ADDR_BASE, PWM_CFG_CYCLE_ADDR(pwm_id), PWM_PERIOD_MASK, period);

	pwm_set_bits((void *)PWM_ADDR_BASE, PWM_CFG_HLINT_ADDR(pwm_id), PWM_DUTY_MASK, duty);

	pwm_set_bits((void *)PWM_ADDR_BASE, PWM_CTRL_ADDR(pwm_id),
						   PWM_KEEP_MASK, (0x1 << PWM_KEEP_SHIFT));
	return 0;
}


