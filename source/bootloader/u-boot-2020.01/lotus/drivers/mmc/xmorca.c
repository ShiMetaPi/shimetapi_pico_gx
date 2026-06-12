/*
 * Copyright (c) LOTUS. All rights reserved.
 */

#include <asm/arch/platform.h>
#include <linux/lotus/chip.h>
#include <common.h>
#include <malloc.h>
#include <sdhci.h>
#include "mmc_private.h"

#define MIN_FREQ	400000
#define NOT_FOUND	(-1)

#ifdef CONFIG_LOTUS_FPGA
#define PHASE_SCALE	8
#define EDGE_TUNING_PHASE_STEP	1
#else
#define PHASE_SCALE	32
#define EDGE_TUNING_PHASE_STEP	4
#endif

#define SDIO_DRV_DLL_SRST_REQ		(0x1 << 29)
#define SDIO_CLK_EN			(0x1 << 28)
#define SDIO_SRST_REQ			(0x1 << 27)
#define SDIO_CLK_SEL			(0x7 << 24)
#define SDIO_CLK_SEL_400K		(0x1 << 24)

#define REG_EMMC_DRV_DLL_STATUS		(CRG_REG_BASE + 0x210)
#define REG_SDIO0_DRV_DLL_STATUS	(CRG_REG_BASE + 0x228)
#define REG_SDIO1_DRV_DLL_STATUS	(CRG_REG_BASE + 0x23c)
#define SDIO_DRV_DLL_LOCK		BIT(15)
#define SDIO_DRV_DLL_READY		BIT(14)

#define REG_EMMC_SAMPL_DLL_STATUS	(CRG_REG_BASE + 0x208)
#define REG_SDIO0_SAMPL_DLL_STATUS	(CRG_REG_BASE + 0x224)
#define REG_SDIO1_SAMPL_DLL_STATUS	(CRG_REG_BASE + 0x238)
#define SDIO_SAMPL_DLL_SLAVE_READY	BIT(0)

#define REG_EMMC_SAMPL_DLL_CTRL		(CRG_REG_BASE + 0x1f4)
#define REG_SDIO0_SAMPL_DLL_CTRL	(CRG_REG_BASE + 0x22c)
#define REG_SDIO1_SAMPL_DLL_CTRL	(CRG_REG_BASE + 0x240)
#define SDIO_SAMPL_DLL_SLAVE_EN		BIT(16)

#define REG_EMMC_SAMPLB_DLL_CTRL	(CRG_REG_BASE + 0x1f8)
#define EMMC_SAMPLB_DLL_CLK_MASK	(0x1f << 0)
#define emmc_samplb_sel(phase)		((phase) << 0)
#define REG_SDIO0_SAMPLB_DLL_CTRL	(CRG_REG_BASE + 0x21c)
#define REG_SDIO1_SAMPLB_DLL_CTRL	(CRG_REG_BASE + 0x230)
#define SDIO_SAMPLB_DLL_CLK_MASK	(0x1f << 24)
#define sdio_samplb_sel(phase)		((phase) << 24)

#define REG_EMMC_DRV_DLL_CTRL		(CRG_REG_BASE + 0x1fc)
#define REG_SDIO0_DRV_DLL_CTRL		(CRG_REG_BASE + 0x220)
#define REG_SDIO1_DRV_DLL_CTRL		(CRG_REG_BASE + 0x234)
#define SDIO_DRV_PHASE_SEL_MASK		(0x1f << 24)
#define sdio_drv_sel(phase)		((phase) << 24)

#define REG_EMMC_DS_DLL_CTRL		(CRG_REG_BASE + 0x200)
#define EMMC_DS_DLL_MODE_SSEL		BIT(13)
#define EMMC_DS_DLL_SSEL_MASK		0x7f

#define REG_EMMC_DS180_DLL_CTRL		(CRG_REG_BASE + 0x204)
#define EMMC_DS180_DLL_BYPASS		BIT(15)
#define REG_EMMC_DS180_DLL_STATUS	(CRG_REG_BASE + 0x218)
#define EMMC_DS180_DLL_READY		BIT(0)

#define CLK_100K	100000
#define CLK_400K	400000
#define CLK_25M		25000000
#define CLK_50M		50000000
#define CLK_90M		90000000

static void enable_sample(struct sdhci_host *host);
static void set_drv_phase(struct sdhci_host *host,	unsigned int phase);
static void set_sampl_phase(struct sdhci_host *host, unsigned int phase);
static void wait_sampl_dll_slave_ready(struct sdhci_host *host);
static void enable_card_clk(struct sdhci_host *host);
static void enable_internal_clk(struct sdhci_host *host);
static void disable_internal_clk(struct sdhci_host *host);

static void dll_reset_assert(struct sdhci_host *host)
{
	uintptr_t crg_addr;
	unsigned int reg;

	crg_addr = (host->type == MMC_TYPE_MMC) ?
		REG_EMMC_CRG : REG_SDIO0_CRG;
	reg = readl(crg_addr);
	reg |= SDIO_DRV_DLL_SRST_REQ;
	writel(reg, crg_addr);
}

static void dll_reset_deassert(struct sdhci_host *host)
{
	uintptr_t crg_addr;
	unsigned int reg;

	crg_addr = (host->type == MMC_TYPE_MMC) ?
		REG_EMMC_CRG : REG_SDIO0_CRG;
	reg = readl(crg_addr);
	reg &= ~SDIO_DRV_DLL_SRST_REQ;
	writel(reg, crg_addr);
}

static void set_crg(struct sdhci_host *host, unsigned int clk)
{
	uintptr_t crg_addr;
	unsigned int sel, reg;
	unsigned int clk_mux[] = {
		CLK_100K, CLK_400K, CLK_25M, CLK_50M, CLK_90M,
	};

	crg_addr = (host->type == MMC_TYPE_MMC) ?
		REG_EMMC_CRG : REG_SDIO0_CRG;
	reg = readl(crg_addr);
	reg &= ~MMC_CLK_SEL_MASK;

	if (clk <= MIN_FREQ) {
		sel = 1;
	} else {
		for (sel = 0x4; sel > 0; sel--) {
			if (clk >= clk_mux[sel])
				break;
		}
	}

	reg |= mmc_clk_sel(sel);
	writel(reg, crg_addr);
}

static void wait_ds180_dll_ready(void)
{
	unsigned int reg;
	unsigned int timeout = 20;

	do {
		reg = readl(REG_EMMC_DS180_DLL_STATUS);
		if (reg & EMMC_DS180_DLL_READY)
			return;

		udelay(1000); /* delay 1000us */
		timeout--;
	} while (timeout > 0);

	printf("DS180 DLL master not ready.\n");
}

static void lotus_mmc_priv_init(struct sdhci_host *host)
{
	unsigned short ctrl;
	unsigned int reg;

	ctrl = sdhci_readw(host, SDHCI_MSHC_CTRL);
	ctrl &= ~SDHCI_CMD_CONFLIT_CHECK;
	sdhci_writew(host, ctrl, SDHCI_MSHC_CTRL);

	reg = sdhci_readl(host, SDHCI_AXI_MBIIU_CTRL);
	reg |= SDHCI_GM_WR_OSRC_LMT | SDHCI_GM_RD_OSRC_LMT;
	reg &= ~SDHCI_UNDEFL_INCR_EN;
	sdhci_writel(host, reg, SDHCI_AXI_MBIIU_CTRL);

	reg  = sdhci_readl(host, SDHCI_MULTI_CYCLE);
	reg &= ~SDHCI_CMD_DLY_EN;
	reg |= SDHCI_EDGE_DETECT_EN | SDHCI_DATA_DLY_EN;
	sdhci_writel(host, reg, SDHCI_MULTI_CYCLE);

	if (host->type == MMC_TYPE_MMC) {
		/*add opt emmc reset gpio*/
		reg  = sdhci_readl(host, SDHCI_GP_OUT_R);
		reg &= ~SDHCI_MMC_RESET_N;
		sdhci_writel(host, reg, SDHCI_GP_OUT_R);
		mdelay(1);

		reg |= SDHCI_MMC_RESET_N;
		sdhci_writel(host, reg, SDHCI_GP_OUT_R);
		mdelay(1);
	}
}

static int sd_hardware_init(void)
{
	unsigned int reg;

	/* clk enable */
	reg = readl(REG_SDIO0_CRG);
	reg |= SDIO_CLK_EN;
	writel(reg, REG_SDIO0_CRG);

	/* reset assert */
	reg = readl(REG_SDIO0_CRG);
	reg |= SDIO_SRST_REQ | SDIO_DRV_DLL_SRST_REQ;
	writel(reg, REG_SDIO0_CRG);
	udelay(25); /* delay 25us */

	/* reset deassert */
	reg &= ~SDIO_SRST_REQ;
	writel(reg, REG_SDIO0_CRG);
	udelay(1); /* delay 1us */

	udelay(5000); /* delay 5000us */

	return 0;
}

static int emmc_hardware_init(void)
{
	unsigned int reg;

	/* eMMC clk enable */
	reg = readl(REG_EMMC_CRG);
	reg |= SDIO_CLK_EN;
	writel(reg, REG_EMMC_CRG);

	/* eMMC reset assert */
	reg = readl(REG_EMMC_CRG);
	reg |= SDIO_SRST_REQ | SDIO_DRV_DLL_SRST_REQ;
	writel(reg, REG_EMMC_CRG);
	udelay(25); /* delay 25us */

	/* select 400K clk */
	reg = readl(REG_EMMC_CRG);
	reg &= ~SDIO_CLK_SEL;
	reg |= SDIO_CLK_SEL_400K;
	writel(reg, REG_EMMC_CRG);
	udelay(25); /* delay 25us */

	/* eMMC reset deassert */
	reg = readl(REG_EMMC_CRG);
	reg &= ~SDIO_SRST_REQ;
	writel(reg, REG_EMMC_CRG);
	udelay(1); /* delay 1us */

	return 0;
}

static void lotus_mmc_set_ioconfig(struct sdhci_host *host)
{
	unsigned int reg;

	if (host->type == MMC_TYPE_MMC) {
		reg = sdhci_readw(host, SDHCI_EMMC_CTRL);
		reg |= SDHCI_CARD_IS_EMMC;
		sdhci_writew(host, reg, SDHCI_EMMC_CTRL);

		if (host->mmc->selected_mode == MMC_DDR_52) {
			reg = sdhci_readw(host, SDHCI_MULTI_CYCLE);
			reg &= ~SDHCI_DATA_DLY_EN;
			sdhci_writew(host, reg, SDHCI_MULTI_CYCLE);
		}
	}

	sdhci_set_uhs_timing(host);
}

static void set_phase(struct sdhci_host *host)
{
	unsigned int drv_phase, sample_phase;
#ifdef CONFIG_LOTUS_FPGA
	unsigned int fix_num = 4;
#else
	unsigned int fix_num = 1;
#endif

	if (host->mmc->selected_mode == MMC_HS_400_ES ||
	    host->mmc->selected_mode == MMC_HS_400) {
		drv_phase = 8/fix_num;   /* 8 for 90 degree */
		sample_phase = host->tuning_phase/fix_num;
	} else if (host->mmc->selected_mode == MMC_HS_200 ||
	    host->mmc->selected_mode == UHS_SDR104) {
		drv_phase = 20/fix_num;   /* 20 for 225 degree */
		sample_phase = host->tuning_phase/fix_num;
	} else if (host->mmc->selected_mode == UHS_SDR50) {
		drv_phase = 16/fix_num;   /* 16 for 180 degree */
		sample_phase = host->tuning_phase/fix_num;
	} else if (host->mmc->selected_mode == MMC_DDR_52 ||
	    host->mmc->selected_mode == UHS_DDR50) {
		drv_phase = 8/fix_num;    /* 8 for 90 degree */
		sample_phase = 4/fix_num; /* 4 for 45 degree */
	} else if (host->mmc->selected_mode == MMC_HS ||
		   host->mmc->selected_mode == MMC_HS_52) {
		drv_phase = 16/fix_num;   /* 16 for 180 degree */
		sample_phase = 4/fix_num; /* 4 for 45 degree */
	} else if (host->mmc->selected_mode == UHS_SDR25 ||
	    host->mmc->selected_mode == SD_HS) {
		drv_phase = 16/fix_num;   /* 16 for 180 degree */
		sample_phase = 4/fix_num; /* 4 for 45 degree */
	} else {
		drv_phase = 16/fix_num;   /* 16 for 180 degree */
		sample_phase = 0; /* 0 for 0 degree */
	}

	set_drv_phase(host, drv_phase);
	enable_sample(host);
	set_sampl_phase(host, sample_phase);

	udelay(25); /* delay 25us */
}

static void wait_drv_dll_lock(const struct sdhci_host *host)
{
	unsigned int timeout = 20;
	unsigned int reg;
	uintptr_t reg_addr;

	reg_addr = (host->type == MMC_TYPE_MMC) ?
		REG_EMMC_DRV_DLL_STATUS : REG_SDIO0_DRV_DLL_STATUS;
	do {
		reg = readl(reg_addr);
		if (reg & SDIO_DRV_DLL_LOCK)
			return;

		udelay(1000); /* delay 1000us */
		timeout--;
	} while (timeout > 0);

	printf("sdhci: DRV DLL master not locked.\n");
}

static void enable_sampl_dll_slave(struct sdhci_host *host)
{
	unsigned int reg;
	uintptr_t reg_addr;

	reg_addr = (host->type == MMC_TYPE_MMC) ?
		REG_EMMC_SAMPL_DLL_CTRL : REG_SDIO0_SAMPL_DLL_CTRL;
	reg = readl(reg_addr);
	reg |= SDIO_SAMPL_DLL_SLAVE_EN;
	writel(reg, reg_addr);
}

static int lotus_mmc_set_clk(struct sdhci_host *host, unsigned int clk)
{
	dll_reset_assert(host);
	udelay(25); /* delay 25us */
	set_crg(host, clk);
	set_phase(host);
	udelay(25); /* delay 25us */

	if (clk > MMC_HIGH_52_MAX_DTR) {
		enable_sampl_dll_slave(host);
		dll_reset_deassert(host);
	}

	enable_internal_clk(host);

	if (clk > MMC_HIGH_52_MAX_DTR) {
		wait_drv_dll_lock(host);
		wait_sampl_dll_slave_ready(host);
		wait_ds180_dll_ready();
	}

	enable_card_clk(host);
	udelay(100); /* delay 100us */

	return 0;
}

static int lotus_set_io_cfg(struct sdhci_host *host, unsigned int cfg)
{
	unsigned int reg;

	if (host->type != MMC_TYPE_MMC) {//only sd card
		if (cfg) {
			reg = readl(0x11980040);
			reg &= ~(0xf);
			reg |= (0x01);
			writel(reg, 0x11980040);
		} else {
			reg = readl(0x11980040);
			reg &= ~(0xf);
			reg |= (0x00);
			writel(reg, 0x11980040);
			//gpio3_0
			writel(0x01, 0x120b3400);
			writel(0x00, 0x120b3004);
		}
	}

	return 0;
}


#include "lotus_sdhci.c"
