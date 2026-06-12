/*
 * Copyright (c) Lotus. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include <arch_helpers.h>
#include <delay_timer.h>
#include <platform_def.h>
#include <lib/mmio.h>

#define USE_SYS_COUNTER 1

static uint32_t plat_get_timer_value(void)
{
#ifdef USE_SYS_COUNTER
	/* Generic delay timer implementation expects the timer to be a down
	 * counter. We apply bitwise NOT operator to the tick values returned
	 * by read_cntpct_el0() to simulate the down counter. */
	return (uint32_t)(~read_cntpct_el0());
#else
	return (uint32_t)(~mmio_read_32(LOTUS_TIMER_VALUE));
#endif
}

static const timer_ops_t plat_timer_ops = {
	.get_timer_value	= plat_get_timer_value,
	.clk_mult		= 1,
#ifdef USE_SYS_COUNTER
	.clk_div		= SYS_COUNTER_FREQ_IN_MHZ,
#else
	.clk_div		= LOTUS_TIMER_FREQ_IN_MHZ,
#endif
};

void plat_delay_timer_init(void)
{
	timer_init(&plat_timer_ops);
}
