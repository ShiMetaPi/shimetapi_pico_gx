/*
 * Copyright (c) Lotus. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include <arch_helpers.h>
#include <bl_common.h>
#include <debug.h>
#include <platform_def.h>
#include <xlat_tables_v2.h>
#include <mmio.h>


/* Table of regions to map using the MMU.  */
const mmap_region_t plat_mmap[] = {
	MAP_REGION_FLAT(DRAM_NS_BASE, DRAM_NS_SIZE,
			MT_MEMORY | MT_RW | MT_NS),
	MAP_REGION_FLAT(SRAM_BASE, SRAM_SIZE,
			MT_NON_CACHEABLE | MT_RW | MT_EXECUTE | MT_SECURE),
	MAP_REGION_FLAT(DEVICE_BASE, DEVICE_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE),
	{ 0 }

};

/*******************************************************************************
 * Macro generating the code for the function setting up the pagetables as per
 * the platform memory map & initialize the mmu, for the given exception level
 ******************************************************************************/
#define DEFINE_CONFIGURE_MMU_EL(_el)					\
	void configure_mmu_el ## _el(unsigned long total_base,	\
					  unsigned long total_size,	\
					  unsigned long ro_start,	\
					  unsigned long ro_limit)	\
	{								\
		mmap_add_region(total_base, total_base,			\
				total_size,				\
				MT_MEMORY | MT_RW | MT_SECURE);		\
		mmap_add_region(ro_start, ro_start,			\
				ro_limit - ro_start,			\
				MT_MEMORY | MT_RO | MT_SECURE);		\
		mmap_add(plat_mmap);					\
		init_xlat_tables();					\
									\
		enable_mmu_el ## _el(0);				\
	}

/* Define EL3 variants of the function initialising the MMU */
DEFINE_CONFIGURE_MMU_EL(3)

unsigned int plat_get_syscnt_freq2(void)
{
	INFO("Sys counter freq: %d\n", SYS_COUNTER_FREQ_IN_TICKS);
	return SYS_COUNTER_FREQ_IN_TICKS;
}

