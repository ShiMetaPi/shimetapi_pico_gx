// SPDX-License-Identifier: GPL-2.0

#include <linux/lotus/timestamp.h>
#include <common.h>
#include <asm/io.h>

static timestamp_type stopwatch_get_running_timer(void);
#define timestamp_get_time()          stopwatch_get_running_timer()
#define convert_to_final_stamp(value) (value)
#define UNIT_STR "us"
#define TIMESTAMP_FMT "%-16llu"UNIT_STR


#define REG_OPERATION_DELAY_US (10 * 1000)
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

typedef union {
	u32 value;
	struct {
		/* [0:3] timer trigger */
		u32 timer_sel : 4;
		/* [4:31] reserved */
		u32 reserved : 28;
	};
} stopwatch_sel;

/*---------------------------------
 * timestamp
 *--------------------------------*/
static u32 get_timestamp_count(void)
{
	return readl(TIMESTAMP_CNT_ADDR);
}

static void update_timestamp_count(u32 cnt)
{
	writel(cnt, TIMESTAMP_CNT_ADDR);
}

void timestamp_clear(void)
{
	writel(0, TIMESTAMP_CNT_ADDR);
}

void timestamp_mark(const char *func, u32 line, u32 type)
{
	timestamp_type stamp = timestamp_get_time();
	u32 cnt = get_timestamp_count();

	if (cnt >= TIMESTAMP_MAX_ITEM_CNT) {
		printf("No timestamp item!\n");
		return;
	}

	timestamp_item *item =
		(timestamp_item *)(uintptr_t)TIMESTAMP_ITEM_START_ADDR;
	item[cnt].stamp = stamp;
	item[cnt].func = (char *)func;
	item[cnt].line = line;
	item[cnt].type = type;
	cnt++;
	update_timestamp_count(cnt);
}

void timestamp_print(u32 type)
{
	const u32 cnt = get_timestamp_count();
	timestamp_type time0 = 0;
	timestamp_item *items =
		(timestamp_item *)(uintptr_t)TIMESTAMP_ITEM_START_ADDR;
	timestamp_type stamp = 0;

	for (u32 i = 0; i < cnt; i++) {
		timestamp_type time = convert_to_final_stamp(items[i].stamp);
		const char *func = items[i].func;
		u32 line = items[i].line;
		if (type == items[i].type) {
			if (func != NULL) {
				printf("time stamp[%-3u] = "TIMESTAMP_FMT" gap = "TIMESTAMP_FMT""
					" type: %-5u  line: %-5u func: %s\n",
					i + 1, time, time - stamp,
					items[i].type, line, func);
			} else {
				printf("time stamp[%-3u] = "TIMESTAMP_FMT" gap = "TIMESTAMP_FMT""
					" type: %-5u  line: %-5u\n",
					i + 1, time, time - stamp,
					items[i].type, line);
			}
		}
		stamp = time;
		if (i == 0) {
			time0 = time;
		}
		if (i == (cnt - 1)) {
			printf("total time from stamp[%-3d] to [%-3d]  gap = "
				TIMESTAMP_FMT"\n", 1, i + 1, time - time0);
		}
	}
}

static int do_timestamp(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	TIME_STAMP(0);
	return 0;
}
U_BOOT_CMD(timestamp, 1, 1, do_timestamp,
		"Mark a time stamp",
		"");

/*---------------------------------
 * stopwatch
 *--------------------------------*/
static timestamp_type stopwatch_get(u32 point)
{
	stopwatch_sel sel = {0};

	sel.timer_sel = point;
	writel(sel.value, (STOPWATCH_CTRL_REG1));
	udelay(REG_OPERATION_DELAY_US);
	timestamp_type value = readl(STOPWATCH_VALUE_REG);
	value += (timestamp_type)readl(STOPWATCH_VALUE_REG_H) << 32;
	return value;
}

static timestamp_type stopwatch_get_running_timer(void)
{
	stopwatch_cfg cfg = {0};

	cfg.running_timer_trigger = 1; /* true */
	writel(cfg.value, (STOPWATCH_CTRL_REG2));

	/* 4us at least to wait */
	udelay(4);
	timestamp_type value = readl(STOPWATCH_RUNNING_VALUE_REG);
	value += (timestamp_type)readl(STOPWATCH_RUNNING_VALUE_REG_H) << 32;
	return value;
}

void stopwatch_trigger(void)
{
	stopwatch_cfg cfg = {0};

	cfg.timer_clear = 0; /* false */
	cfg.timer_trigger = 1; /* true */
	writel(cfg.value, (STOPWATCH_CTRL_REG2));
}

void stopwatch_clear(void)
{
	stopwatch_cfg cfg = {0};

	cfg.timer_clear = 1; /* true */
	cfg.timer_trigger = 0; /* false */
	writel(cfg.value, (STOPWATCH_CTRL_REG2));
}

void stopwatch_print(void)
{
	timestamp_type time;
	u32 point;
	printf("-----------------------------\n");
	printf("STOPWATCH(boot)[0-%d]: \n", TIMESTAMP_STOPWATCH_POINT_MAX - 1);
	for (point = 0;	point < TIMESTAMP_STOPWATCH_POINT_MAX; point++) {
		time = stopwatch_get(point);
		printf("%-3d  ", point);
		printf(TIMESTAMP_FMT"\n", time);
	}
	printf("-----------------------------\n");
}

/* #define STOPWATCH_TEST */
#ifdef STOPWATCH_TEST
static int do_stopwatch_test(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
#define __NUM 60
	u32 value[__NUM];
	int i;

	for (i = 0; i < __NUM; i += 2) {
		writel(1 << 16, STOPWATCH_CTRL_REG);
		udelay(4);
		value[i] = readl(STOPWATCH_RUNNING_VALUE_REG);
		value[i + 1] = readl(STOPWATCH_RUNNING_VALUE_REG_H);
	}

	printf("------------------------------------------------\n");
	printf("STOPWATCH RUNNING TIMER(boot)[0-%d]: \n", __NUM / 2);
	for (i = 0; i < __NUM; i += 2) {
		printf("%8u_%-8u us", value[i + 1], value[i]);
		if (i == 0)
			printf(", gap: 0        us\n");
		else {
			printf(", gap: %-8u us\n", value[i] - value[i - 2]);
			if (value[i] - value[i - 2] > 10) {
				printf("Gap Exception!!");
				hang();
			}
		}
	}

	printf("------------------------------------------------\n");

	return 0;
}

U_BOOT_CMD(stopwatch_test, 1, 1, do_stopwatch_test,
		"stopwatch_test",
		"");
#endif

