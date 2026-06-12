// SPDX-License-Identifier: GPL-2.0

#include <config.h>
#include <div64.h>
#include <linux/io.h>
#include <asm/arch/platform.h>

static struct {
	unsigned int timebase_l;
	unsigned int timebase_h;
} timebase;

/* Returns tick rate in ticks per second */
unsigned long get_tbclk(void)
{
	return CONFIG_SYS_TIMER_RATE;
}

unsigned long timer_read_counter(void)
{
	return ~readl(CONFIG_SYS_TIMER_COUNTER);
}

unsigned long long get_ticks(void)
{
	unsigned long now = timer_read_counter();

	/* increment tbu if tbl has rolled over */
	if (now < timebase.timebase_l)
		timebase.timebase_h++;
	timebase.timebase_l = now;
	return ((unsigned long long)timebase.timebase_h << 32) | timebase.timebase_l;
}

unsigned long long usec_to_tick(unsigned long usec)
{
	uint64_t tick = usec;

	tick *= get_tbclk();
	do_div(tick, 1000000);
	return tick;
}

void udelay(unsigned long usec)
{
	uint64_t tmp;

	tmp = get_ticks() + usec_to_tick(usec);

	while (get_ticks() < tmp+1)
		;
}

int timer_init(void)
{
	writel(0, TIMER0_REG_BASE + REG_TIMER_CONTROL);
	writel(~0, TIMER0_REG_BASE + REG_TIMER_RELOAD);

	writel(CFG_TIMER_CTRL, TIMER0_REG_BASE + REG_TIMER_CONTROL);

	return 0;
}
