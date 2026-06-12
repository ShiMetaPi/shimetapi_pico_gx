/*
 * Copyright (c) LOTUS. All rights reserved.
 */

#include <config.h>
#include <linux/kconfig.h>
#include <asm/io.h>
#include <compiler.h>
#include <linux/lotus/common.h>
#include <linux/lotus/timestamp.h>
#include <linux/lotus/step.h>

#include "time.h"

const uintptr_t image_entry = (CONFIG_SYS_TEXT_BASE);

#ifndef CONFIG_LOTUS_DISABLE_CONSOLE
#define info(_s)                uart_early_puts(_s)
#else
#define info(_s)
#endif
#define error(_s)               uart_early_puts(_s)
#define putstr(_s)              uart_early_puts(_s)

#include "hw_decompress.c"
#define GZIP_SIZE_OFFSET 0x4

#ifndef CONFIG_SYS_ICACHE_OFF
/* Invalidate entire I-cache and branch predictor array */
static void invalidate_icache_all(void)
{
	/*
	 * Invalidate all instruction caches to PoU.
	 * Also flushes branch target cache.
	 */
	asm volatile("mcr p15, 0, %0, c7, c5, 0" : : "r"(0));

	/* Invalidate entire branch predictor array */
	asm volatile("mcr p15, 0, %0, c7, c5, 6" : : "r"(0));

	/* Full system DSB - make sure that the invalidation is complete */
	dsb();

	/* ISB - make sure the instruction stream sees it */
	isb();
}
#else
static void invalidate_icache_all(void)
{
}
#endif

u32 get_timestamp_count(void)
{
	return __raw_readl(TIMESTAMP_CNT_ADDR);
}

void update_timestamp_count(u32 cnt)
{
	__raw_writel(cnt, TIMESTAMP_CNT_ADDR);
}

static u64 stopwatch_get_running_timer(void)
{
	writel(1 << 2, STOPWATCH_CTRL_REG2);
	udelay(4);
	u64 value = readl(STOPWATCH_RUNNING_VALUE_REG);
	value += (u64)readl(STOPWATCH_RUNNING_VALUE_REG_H) << 32;
	return value;
}

void timestamp_init(void)
{
	update_timestamp_count(0);
}

void timestamp_mark(const char *func, u32 line, u32 type)
{
#define timestamp_get_time() stopwatch_get_running_timer()
	u64 stamp = timestamp_get_time();
	u32 cnt = get_timestamp_count();

	if (cnt >= TIMESTAMP_MAX_ITEM_CNT) {
		uart_early_puts("No timestamp item!\n");
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

/******************************************************************************/
void start_armboot(void)
{
#ifdef CONFIG_LOTUS_GZIP_BOOT
	unsigned char *dst = NULL;
	unsigned int image_data_len;
	int dst_len;
	int ret;
	int i;
	char *p = NULL;
	char *q = NULL;

	/* timestamp_init(); */
	info("\r\nUncompress ");
	TIME_STAMP(0);

	/* use direct address mode */
	hw_dec_type = 0;
	/* init hw decompress IP */
	hw_dec_init();

	step(35);

	/* start decompress */
	dst = (unsigned char *)image_entry;
	image_data_len = input_data_end - input_data;

	/* get dets length from compress image */
	p = (char *)&dst_len;
	q = (char *)(input_data_end - GZIP_SIZE_OFFSET);
	for (i = 0; i < sizeof(int); i++)
		p[i] = q[i];

	ret = hw_dec_decompress(dst, &dst_len, input_data,
						image_data_len, NULL);
	if (!ret) {
		info("Ok!");
	} else {
		error("Fail!");
		while (1);
	}

	info("\r\n");

	step(36);

	/* uinit hw decompress IP */
	hw_dec_uinit();
#else
	uart_early_puts(" OK, non-compressed boot!\n");
#endif
	TIME_STAMP(0);

	step(36);

	void (*uboot)(void);
	uboot = (void (*))CONFIG_SYS_TEXT_BASE;
	invalidate_icache_all();

	step(37);

	uboot();
}


