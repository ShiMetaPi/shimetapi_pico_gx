/*
 * Copyright (c) Lotus. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <arch.h>
#include <platform_def.h>
#include <psci.h>
#include <stdio.h>
#include <debug.h>

const unsigned char lotus_power_domain_tree_desc[] = {
        /* No of root nodes */
        PLATFORM_CLUSTER_COUNT,
        /* No of clusters */
   /*     PLATFORM_CLUSTER_COUNT, */
        /* No of CPU cores */
        PLATFORM_CORE_COUNT,
};

/*******************************************************************************
 * This function returns the Tegra default topology tree information.
 ******************************************************************************/
const unsigned char *plat_get_power_domain_tree_desc(void)
{
	return lotus_power_domain_tree_desc;
}

/*******************************************************************************
 * This function implements a part of the critical interface between the psci
 * generic layer and the platform that allows the former to query the platform
 * to convert an MPIDR to a unique linear index. An error code (-1) is returned
 * in case the MPIDR is invalid.
 ******************************************************************************/
int plat_core_pos_by_mpidr(u_register_t mpidr)
{
	unsigned int cluster_id, cpu_id;

	INFO("mpidr=0x%lx\n", mpidr);

	mpidr &= MPIDR_AFFINITY_MASK;

	if (mpidr & ~(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK))
		return -1;

	return mpidr & MPIDR_CPU_MASK;

	cluster_id = (mpidr >> MPIDR_AFF2_SHIFT) & MPIDR_AFFLVL_MASK;
	cpu_id = (mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK;

	INFO("cluster_id=%d, cpu_id=%d\n", cluster_id, cpu_id);

	if (cluster_id >= PLATFORM_CLUSTER_COUNT)
		return -1;

	/*
	 * Validate cpu_id by checking whether it represents a CPU in
	 * one of the two clusters present on the platform.
	 */
	if (cpu_id >= PLATFORM_MAX_CPUS_PER_CLUSTER)
		return -1;

	return (cpu_id + (cluster_id * PLATFORM_MAX_CPUS_PER_CLUSTER));
}
