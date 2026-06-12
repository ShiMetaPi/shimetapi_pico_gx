/*
 * Copyright (c) Lotus. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <delay_timer.h>
#include <errno.h>
#include <mmio.h>
#include <platform_def.h>
#include <plat_private.h>
#include <platform.h>
#include <xmfalcon_def.h>

/* ARM64 REG */
#define REG_SYS_CTRL_BASE       0x12020000
#define REG_SYS_CPU_AARCH_MODE  (REG_SYS_CTRL_BASE + 0x800C)
#define REG_SYS_RVBAR_ADDR0     (REG_SYS_CTRL_BASE + 0x8010)
#define REG_SYS_RVBAR_ADDR1     (REG_SYS_CTRL_BASE + 0x8014)

#define REG_PERI_CRG_BASE     0x12010000
#define REG_PERI_CRG30        (REG_PERI_CRG_BASE + 0x78)
#define REG_CPU_CORE_SRST     REG_PERI_CRG30

#define REG_SYS_SOFT_RESET    0x12020008

#define REG_PWR_CTRL0         0x120f0010
#define START_STANDBY_MASK    BIT(0)
#define START_POWERDOWN_MASK    BIT(1)

#define writel(val, addr)        mmio_write_32((uintptr_t)addr, (uint32_t)val)
#define readl(addr)              mmio_read_32((uintptr_t)addr)

static int cores_pwr_domain_on(unsigned long mpidr, uint64_t entrypoint)
{
	uint32_t cpu, val, cpu_dbg_disable;
	cpu = plat_core_pos_by_mpidr(mpidr);

	if (cpu != 1) {
		ERROR("Can not power on cpu other than cpu1\n");
		return -1;
	}

	writel((uint32_t)((entrypoint >> 2) & 0xFFFFFFFF), REG_SYS_RVBAR_ADDR1);
	writel(0, REG_SYS_RVBAR_ADDR0);
	writel(readl(REG_SYS_CPU_AARCH_MODE) | (0x3 << 14), REG_SYS_CPU_AARCH_MODE);

	val = readl(REG_MISC_CTRL129);
	cpu_dbg_disable = val >> 1;
	if (cpu_dbg_disable) {
		val &= ~0x2;
		writel(val, REG_MISC_CTRL129);
		udelay(10);
	}

	val = readl(REG_CPU_CORE_SRST);
	val &= ~((uint32_t)(0x1 << 1));
	writel(val, REG_CPU_CORE_SRST);

	if (cpu_dbg_disable) {
		udelay(10);
		writel(readl(REG_MISC_CTRL129) | 0x2, REG_MISC_CTRL129);
	}

	return 0;
}

static int cores_pwr_domain_on_finish(void)
{
	return 0;
}

static int sys_pwr_domain_resume(void)
{
	return 0;
}

static int sys_pwr_domain_suspend(void)
{
	return 0;
}

static int sys_pwr_reset(void)
{
	INFO("System: will restart.");

	dsb();
	isb();

	writel(0xdeadbeef, REG_SYS_SOFT_RESET);

	while(1) {
		wfi();
		ERROR("System wfi failed.\n");
	}

	return 0;
}

static void sys_pwr_standby(void)
{
	writel(readl(REG_PWR_CTRL0) | START_STANDBY_MASK, REG_PWR_CTRL0);
	mdelay(50);
	ERROR("sys_pwr_standby failed\n");
	panic();
}

static void sys_pwr_poweroff(void)
{
	writel(readl(REG_PWR_CTRL0) | START_POWERDOWN_MASK, REG_PWR_CTRL0);
	mdelay(50);
	ERROR("sys_pwr_poweroff failed\n");
	panic();
}



static struct lotus_pm_ops_cb pm_ops = {
	.cores_pwr_dm_on = cores_pwr_domain_on,
	.cores_pwr_dm_on_finish = cores_pwr_domain_on_finish,
	.sys_pwr_dm_suspend = sys_pwr_domain_suspend,
	.sys_pwr_dm_resume = sys_pwr_domain_resume,
	.sys_gbl_soft_reset = sys_pwr_reset,
	.sys_gbl_standby = sys_pwr_standby,
	.sys_gbl_poweroff = sys_pwr_poweroff,
};

void plat_lotus_pmc_init(void)
{
	plat_setup_lotus_pm_ops(&pm_ops);
}

