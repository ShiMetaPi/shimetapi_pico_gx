/*
 * Copyright (c) Lotus. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __XMFALCON_H__
#define __XMFALCON_H__


/* SRAM mem */
#define SRAM_BASE             0x04010000
#define SRAM_SIZE             0x10000

/* IO mem */
#define DEVICE_BASE             0x04020000
#define DEVICE_SIZE             0x10FE0000

/* DDR mem */
#define DRAM_NS_BASE            0x40000000
#define DRAM_NS_SIZE            0x80000000

/* UART */
#define XMFALCON_UART0_BASE		(0x12040000)
#define XMFALCON_BAUDRATE		(115200)
#define XMFALCON_UART_CLK		(24000000)
#define XMFALCON_UART_CLK_IN_HZ		(XMFALCON_UART_CLK / 1000000)

/* System counter */
#ifdef CONFIG_FPGA
#define SYS_COUNTER_FREQ_IN_TICKS       30000000
#else
#define SYS_COUNTER_FREQ_IN_TICKS       50000000
#endif
#define SYS_COUNTER_FREQ_IN_MHZ         (SYS_COUNTER_FREQ_IN_TICKS / 1000000)

/* Timer */
#define TIMER0_REG_BASE         0x12000000
#define REG_TIMER_RELOAD        0x0
#define REG_TIMER_VALUE         0x4
#define REG_TIMER_CONTROL       0x8

#define LOTUS_TIMER_BASE		    TIMER0_REG_BASE
#define LOTUS_TIMER_VALUE           (LOTUS_TIMER_BASE + REG_TIMER_VALUE)
#define LOTUS_TIMER_FREQ_IN_TICKS	24000000
#define LOTUS_TIMER_FREQ_IN_MHZ		(LOTUS_TIMER_FREQ_IN_TICKS / 1000000)

/* GIC-400 */
#define XMFALCON_GICD_BASE		(0x10401000)
#define XMFALCON_GICC_BASE		(0x10402000)

/* Acpu suspend firmware load address */
#define LOTUS_PM_CODE_START SRAM_BASE
#define LOTUS_PM_CODE_SIZE SRAM_SIZE

#define STR_FLAG_ADDR    0x120f0048
#define STR_FLAG_VALUE   0xC3321142
#define DRAM_SIZE_ADDR   0x120200C0

#define PWR_CTRL0_ADDRESS    0x120f0010
#define PWM_CTRL0_STANDBY    BIT(0)
#define PWM_CTRL0_POWERDOWN  BIT(1)

#define REG_MISC_CTRL129 0x12029108

#endif /* __XMFALCON_H__ */
