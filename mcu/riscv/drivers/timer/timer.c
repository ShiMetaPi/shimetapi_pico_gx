
#include <common.h>
#include <timer.h>

#define TIMER_LOAD      0
#define TIMER_VALUE     0x4
#define TIMER_CONTROL   0x8

#define CFG_TIMER_CTRL  0xc0

#define TIMER_FEQ       24000000

#define TIMER_DIV       1

#define TIMER_NOR_DIVIDER_US   (TIMER_FEQ / TIMER_DIV / 1000000)

#define UDELAY_US_MAX   (61 * 1000 * 1000UL)

#define REG_TIMER_BASE    TIMER3_REG_BASE

#define timer_reg_get(offset)      readl(REG_TIMER_BASE + (offset))
#define timer_reg_set(offset, val) writel(val, REG_TIMER_BASE + (offset))

static u8 g_timer_init = 0;

ulong timer_get_val(void)
{
	return ~1UL - timer_reg_get(TIMER_VALUE);
}

void udelay(ulong us)
{
	ulong cur, start, end;
	ulong count;

	if (us == 0)
		return;

	assert(g_timer_init == 1);

	start = timer_get_val();

	if (us >= UDELAY_US_MAX) {
		puts("WARNING: long udelay, use ");
		putdec(UDELAY_US_MAX);
		puts("us instead!\n");
		count = UDELAY_US_MAX * TIMER_NOR_DIVIDER_US;
	} else
		count = us * TIMER_NOR_DIVIDER_US;

	end = start + count;

	while (1) {
		cur = timer_get_val();
		if ((end >= start && cur >= end ) ||
			(end < start && cur >= end && cur < start))
			break;
	}
}

void mdelay(u32 msec)
{
	assert(g_timer_init == 1);

	udelay(msec * 1000);
}

void timer_init(void)
{
        timer_reg_set(TIMER_CONTROL, 0);
	timer_reg_set(TIMER_LOAD, 0xffffffff);
        timer_reg_set(TIMER_CONTROL, CFG_TIMER_CTRL);

	g_timer_init = 1;
}



