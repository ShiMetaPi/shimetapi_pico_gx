/*
 * Copyright (c) Lotus. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __PLAT_PRIVATE_H__
#define __PLAT_PRIVATE_H__

/*******************************************************************************
 * For socs pm ops
 ******************************************************************************/
struct lotus_pm_ops_cb {
	int (*cores_pwr_dm_on)(unsigned long mpidr, uint64_t entrypoint);
	int (*cores_pwr_dm_off)(void);
	int (*cores_pwr_dm_on_finish)(void);
	int (*cores_pwr_dm_suspend)(void);
	int (*cores_pwr_dm_resume)(void);
	int (*sys_pwr_dm_suspend)(void);
	int (*sys_pwr_dm_resume)(void);
	int (*sys_gbl_soft_reset)(void);
	void (*sys_gbl_standby)(void);
	void (*sys_gbl_poweroff)(void);
};

/*******************************************************************************
 * Function and variable prototypes
 ******************************************************************************/
void configure_mmu_el3(unsigned long total_base,
			    unsigned long total_size,
			    unsigned long,
			    unsigned long);


void plat_delay_timer_init(void);

void plat_lotus_gic_driver_init(void);
void plat_lotus_gic_init(void);
void plat_lotus_gic_cpuif_enable(void);
void plat_lotus_gic_cpuif_disable(void);
void plat_lotus_gic_pcpu_init(void);

void plat_lotus_pmc_init(void);
void plat_setup_lotus_pm_ops(struct lotus_pm_ops_cb *ops);
void lotus_suspend_save_param(void);
void lotus_cpu_suspend(void);
void lotus_sys_suspend(void);
void lotus_invalidate_icache(void);

extern unsigned int lotus_sys_suspend_size;
#endif /* __PLAT_PRIVATE_H__ */
