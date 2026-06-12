/*
 * Copyright (c) Lotus. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <context.h>
#include <context_mgmt.h>
#include <debug.h>
#include <mmio.h>
#include <platform.h>
#include <plat_lotus.h>
#include <platform_def.h>
#include <psci.h>
#include <plat_private.h>
#include <xmfalcon_def.h>
#include <delay_timer.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

extern uintptr_t lotus_bl31_phys_base;
extern uintptr_t lotus_sec_entry_point;

static struct lotus_pm_ops_cb *lotus_ops;

/*******************************************************************************
 * This handler is called by the PSCI implementation during the `SYSTEM_SUSPEND`
 * call to get the `power_state` parameter. This allows the platform to encode
 * the appropriate State-ID field within the `power_state` parameter which can
 * be utilized in `pwr_domain_suspend()` to suspend to system affinity level.
 ******************************************************************************/
void lotus_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	/* lower affinities use PLAT_MAX_OFF_STATE */
	for (int i = MPIDR_AFFLVL0; i <= PLAT_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;

}

/*******************************************************************************
 * Handler called when an affinity instance is about to enter standby.
 ******************************************************************************/
void lotus_cpu_standby(plat_local_state_t cpu_state)
{
	/*
	 * Enter standby state
	 * dsb is good practice before using wfi to enter low power states
	 */
	dsb();
	wfi();
}

/*******************************************************************************
 * Handler called when an affinity instance is about to be turned on. The
 * level and mpidr determine the affinity instance.
 ******************************************************************************/
int lotus_pwr_domain_on(u_register_t mpidr)
{
	if ((lotus_ops != NULL) && (lotus_ops->cores_pwr_dm_on != NULL))
		lotus_ops->cores_pwr_dm_on(mpidr, lotus_sec_entry_point);

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Handler called when a power domain is about to be turned off. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
void lotus_pwr_domain_off(const psci_power_state_t *target_state)
{
}

/*******************************************************************************
 * Handler called when called when a power domain is about to be suspended. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
void lotus_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	/* gic must save data */
	plat_lotus_gic_save();
	//lotus_console_runtime_end();
}

/*******************************************************************************
 * Handler called when a power domain has just been powered on after
 * being turned off earlier. The target_state encodes the low power state that
 * each level has woken up from.
 ******************************************************************************/
void lotus_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	if ((lotus_ops != NULL) && (lotus_ops->cores_pwr_dm_on_finish != NULL))
		lotus_ops->cores_pwr_dm_on_finish();

	/* Enable the gic cpu interface */
	plat_lotus_gic_pcpu_init();

	/* Program the gic per-cpu distributor or re-distributor interface */
	plat_lotus_gic_cpuif_enable();
}

/*******************************************************************************
 * Handler called when a power domain has just been powered on after
 * having been suspended earlier. The target_state encodes the low power state
 * that each level has woken up from.
 ******************************************************************************/
void lotus_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	const plat_local_state_t *pwr_domain_state = target_state->pwr_domain_state;
	unsigned int stateid_afflvl1 = pwr_domain_state[MPIDR_AFFLVL1];

	/* system is going to powerdown */
	if (stateid_afflvl1 == PLAT_MAX_OFF_STATE) {
		//lotus_console_runtime_init();

		/* gic must restore data */
		plat_lotus_gic_restore();
	}
}

/*******************************************************************************
 * Handler called when the system wants to be powered off
 ******************************************************************************/
__dead2 void lotus_system_off(void)
{
	if ((lotus_ops != NULL) && (lotus_ops->sys_gbl_poweroff != NULL))
		lotus_ops->sys_gbl_poweroff();

	ERROR("System Off: operation not handled.\n");
	panic();
}

/*******************************************************************************
 * Handler called when the system wants to be restarted.
 ******************************************************************************/
__dead2 void lotus_system_reset(void)
{
	/* per-SoC system reset handler */
	if ((lotus_ops != NULL) && (lotus_ops->sys_gbl_soft_reset != NULL))
		lotus_ops->sys_gbl_soft_reset();
	ERROR("System Off: operation not handled.\n");
	panic();

}

/*******************************************************************************
 * Handler called to check the validity of the power state parameter.
 ******************************************************************************/
int32_t lotus_validate_power_state(unsigned int power_state,
		psci_power_state_t *req_state)
{
	int pwr_lvl = psci_get_pstate_pwrlvl(power_state);
	int i;

	if (req_state == NULL) {
		panic();
	}

	if (pwr_lvl > PLAT_MAX_PWR_LVL)
		return PSCI_E_INVALID_PARAMS;

	for (i = 0; i <= pwr_lvl; i++)
		req_state->pwr_domain_state[i] =
			PLAT_MAX_OFF_STATE;

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Platform handler called to check the validity of the non secure entrypoint.
 ******************************************************************************/
int lotus_validate_ns_entrypoint(uintptr_t entrypoint)
{
	/*
	 * Check if the non secure entrypoint lies within the non
	 * secure DRAM.
	 */
	return PSCI_E_SUCCESS;

}

/*******************************************************************************
 * Platform handler called to enter wfi when pwr domain power down.
 ******************************************************************************/
__dead2 void lotus_pwr_domain_pwr_down_wfi(const psci_power_state_t *target_state)
{
	unsigned int target_idx = plat_my_core_pos();

	if (target_idx == 0) {

		//set str flag
		*(unsigned int *)STR_FLAG_ADDR = STR_FLAG_VALUE;

		memset((void *)LOTUS_PM_CODE_START, 0, LOTUS_PM_CODE_SIZE);
		memcpy((void *)LOTUS_PM_CODE_START, &lotus_sys_suspend, lotus_sys_suspend_size);
		lotus_suspend_save_param();

		lotus_invalidate_icache();
		INFO("Power Down ");

		//flush dcache all ddr range
		flush_dcache_range((unsigned long)DRAM_NS_BASE, mmio_read_32((uintptr_t)DRAM_SIZE_ADDR));
		lotus_cpu_suspend();
	} else {
		isb();
		dsb();
		wfi();
	}
	ERROR("Core %d Power-down not supported", target_idx);
	panic();
}

/*******************************************************************************
 * Export the platform handlers to enable psci to invoke them
 ******************************************************************************/
static const plat_psci_ops_t lotus_plat_psci_ops = {
	.cpu_standby			= lotus_cpu_standby,
	.pwr_domain_on			= lotus_pwr_domain_on,
	.pwr_domain_off			= lotus_pwr_domain_off,
	.pwr_domain_suspend		= lotus_pwr_domain_suspend,
	.pwr_domain_on_finish		= lotus_pwr_domain_on_finish,
	.pwr_domain_suspend_finish	= lotus_pwr_domain_suspend_finish,
	.pwr_domain_pwr_down_wfi	= lotus_pwr_domain_pwr_down_wfi,
	.system_off			= lotus_system_off,
	.system_reset			= lotus_system_reset,
	.validate_power_state		= lotus_validate_power_state,
	.validate_ns_entrypoint		= lotus_validate_ns_entrypoint,
	.get_sys_suspend_power_state	= lotus_get_sys_suspend_power_state,
};

/*******************************************************************************
 * Export the platform specific power ops and initialize Power Controller
 ******************************************************************************/
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
		const plat_psci_ops_t **psci_ops)
{

	/*
	 * Flush entrypoint variable to PoC since it will be
	 * accessed after a reset with the caches turned off.
	 */
	lotus_sec_entry_point = sec_entrypoint;
	flush_dcache_range((uintptr_t)&lotus_sec_entry_point, sizeof(uintptr_t));

	/*
	 * Initialize PSCI ops struct
	 */
	*psci_ops = &lotus_plat_psci_ops;

	return 0;
}

void plat_setup_lotus_pm_ops(struct lotus_pm_ops_cb *ops)
{
	lotus_ops = ops;
}
