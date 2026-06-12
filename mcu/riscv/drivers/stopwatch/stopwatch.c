
#include <sys/types.h>
#include <platform.h>
#include <io.h>

typedef union {
	u32 value;
	struct {
		/* [0] timer trigger */
		u32 timer_trigger : 1;
		/* [2] timer clear */
		u32 timer_clear : 1;
		/* [3] running timer trigger */
		u32 running_timer_trigger : 1;
		/* [3:31] reserved */
		u32 reserved : 29;
	};
} stopwatch_cfg;

void stopwatch_trigger(void)
{
	stopwatch_cfg cfg = {0};

	cfg.timer_clear = 0; /* false */
	cfg.timer_trigger = 1; /* true */
	writel(cfg.value, (STOPWATCH_CTRL_REG2));
}
