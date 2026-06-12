/*
 * Copyright (c) Lotus. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __PLATFORM_DEF_H__
#define __PLATFORM_DEF_H__

#include <common_def.h>
#include <arch.h>
#include <xmfalcon_def.h>

#define DEBUG_XLAT_TABLE 0

/*******************************************************************************
 * Platform binary types for linking
 ******************************************************************************/
#define PLATFORM_LINKER_FORMAT          "elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH            aarch64

/* Size of cacheable stacks */
#if DEBUG_XLAT_TABLE
#define PLATFORM_STACK_SIZE 0x800
#elif IMAGE_BL1
#define PLATFORM_STACK_SIZE 0x440
#elif IMAGE_BL2
#define PLATFORM_STACK_SIZE 0x400
#elif IMAGE_BL31
#define PLATFORM_STACK_SIZE 0x800
#elif IMAGE_BL32
#define PLATFORM_STACK_SIZE 0x440
#endif


/* Cache line size */
#define CACHE_WRITEBACK_SHIFT   6
#define CACHE_WRITEBACK_GRANULE (1 << CACHE_WRITEBACK_SHIFT)

/* Power domain */
#define PLATFORM_CLUSTER_COUNT		    1
#define PLATFORM_MAX_CPUS_PER_CLUSTER	2
#define PLATFORM_CORE_COUNT             (PLATFORM_CLUSTER_COUNT * \
                                         PLATFORM_MAX_CPUS_PER_CLUSTER)
#define PLAT_NUM_PWR_DOMAINS            (PLATFORM_CORE_COUNT + \
                                         PLATFORM_CLUSTER_COUNT + 1)
/* Power states */
#define PLAT_MAX_PWR_LVL                MPIDR_AFFLVL1
#define PLAT_MAX_OFF_STATE	U(2)
#define PLAT_MAX_RET_STATE	U(1)

/* BL31 specific defines. */
#define BL31_SIZE                       0x32000
#define BL31_BASE                       0x40008000
#define BL31_LIMIT                      (BL31_BASE + BL31_SIZE - 1)

/* Platform specific page table and MMU setup */
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)
#define MAX_XLAT_TABLES             6
#define MAX_MMAP_REGIONS            16


/* GICD and GICC and GICR base */
#define PLAT_LOTUS_GICD_BASE		XMFALCON_GICD_BASE
#define PLAT_LOTUS_GICC_BASE		XMFALCON_GICC_BASE


#define PLAT_LOTUS_UART_BASE		XMFALCON_UART0_BASE
#define PLAT_LOTUS_UART_CLOCK		XMFALCON_UART_CLK
#define PLAT_LOTUS_UART_CLOCK_IN_HZ	XMFALCON_UART_CLK_IN_HZ
#define PLAT_LOTUS_UART_BAUDRATE	XMFALCON_BAUDRATE

#define PLAT_LOTUS_PRIMARY_CPU	0x0

#endif /* __PLATFORM_DEF_H__ */
