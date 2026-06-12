/*
 * Copyright (c) LOTUS. All rights reserved.
 */

#include <asm/arch/platform.h>
#include <linux/lotus/chip.h>
#include <common.h>
#include <malloc.h>
#include <sdhci.h>
#include "mmc_private.h"
#include "xmfalcon_drive_cap.c"

#define SDHCI_LOTUS_IO_PD_EN	0 /* enable IO up and down cfg */

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

#define IO_CFG_SR			BIT(10)
#define IO_CFG_PULL_DOWN		BIT(9)
#define IO_CFG_PULL_UP			BIT(8)
#define IO_CFG_DRV_STR_MASK		(0xf << 4)
#define io_cfg_drv_str_sel(str)		((str) << 4)

#define REG_IO_CFG_BASE			0x100C0000
/* EMMC_IOCFG */
#define IO_CFG_EMMC_DATA_LINE_COUNT	8
#define REG_CTRL_EMMC_CLK		0x0000
#define REG_CTRL_EMMC_CMD		0x0004
#define REG_CTRL_EMMC_DATA0		0x0008
#define REG_CTRL_EMMC_DATA1		0x000c
#define REG_CTRL_EMMC_DATA2		0x0010
#define REG_CTRL_EMMC_DATA3		0x0014
#define REG_CTRL_EMMC_DATA4		0x002c
#define REG_CTRL_EMMC_DATA5		0x0030
#define REG_CTRL_EMMC_DATA6		0x0024
#define REG_CTRL_EMMC_DATA7		0x0028
#define REG_CTRL_EMMC_DS		0x001c
#define REG_CTRL_EMMC_RST		0x0018
static unsigned int io_emmc_data_reg[IO_CFG_EMMC_DATA_LINE_COUNT] = {
	REG_CTRL_EMMC_DATA0, REG_CTRL_EMMC_DATA1,
	REG_CTRL_EMMC_DATA2, REG_CTRL_EMMC_DATA3,
	REG_CTRL_EMMC_DATA4, REG_CTRL_EMMC_DATA5,
	REG_CTRL_EMMC_DATA6, REG_CTRL_EMMC_DATA7
};

#define IO_CFG_SDIO0_DATA_LINE_COUNT	4
#define REG_CTRL_SDIO0_CLK		0x001c
#define REG_CTRL_SDIO0_CMD		0x0020
#define REG_CTRL_SDIO0_DATA0		0x0024
#define REG_CTRL_SDIO0_DATA1		0x0028
#define REG_CTRL_SDIO0_DATA2		0x002c
#define REG_CTRL_SDIO0_DATA3		0x0030
static unsigned int io_sdio0_data_reg[IO_CFG_SDIO0_DATA_LINE_COUNT] = {
	REG_CTRL_SDIO0_DATA0, REG_CTRL_SDIO0_DATA1,
	REG_CTRL_SDIO0_DATA2, REG_CTRL_SDIO0_DATA3
};

#define CLK_100K	100000
#define CLK_400K	400000
#define CLK_25M		25000000
#define CLK_50M		50000000
#define CLK_90M		90000000
#define CLK_112M	112000000
#define CLK_150M	150000000
#define CLK_198M	198000000

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
		CLK_100K, CLK_400K, CLK_25M, CLK_50M,
		CLK_90M, CLK_112M, CLK_150M, CLK_198M
	};

	crg_addr = (host->type == MMC_TYPE_MMC) ?
		REG_EMMC_CRG : REG_SDIO0_CRG;
	reg = readl(crg_addr);
	reg &= ~MMC_CLK_SEL_MASK;

	if (clk <= MIN_FREQ) {
		sel = 1;
	} else {
		for (sel = 0x7; sel > 0; sel--) {
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

static void set_drv_str(unsigned int offset, unsigned int pull_up,
				unsigned int pull_down, unsigned int sr,
				unsigned int drv_str)
{
	unsigned int reg;
	const uintptr_t crg_addr = REG_IO_CFG_BASE + offset;

	reg = readl(crg_addr);

	#if SDHCI_LOTUS_IO_PD_EN
	reg &= ~(IO_CFG_PULL_UP | IO_CFG_PULL_DOWN);
	reg |= (pull_up ? IO_CFG_PULL_UP : 0);
	reg |= (pull_down ? IO_CFG_PULL_DOWN : 0);
	#endif

	reg &= ~(IO_CFG_DRV_STR_MASK | IO_CFG_SR);
	reg |= (sr ? IO_CFG_SR : 0);
	reg |= io_cfg_drv_str_sel(drv_str);
	writel(reg, crg_addr);
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

static void lotus_emmc_set_ioconfig(struct sdhci_host *host)
{
	int i;
	int bus_width = host->mmc->bus_width;
	unsigned int reg, _reg;
	unsigned int emmc_intf, emmc_drive_offset;
	unsigned int inch_index, layer_index, speed_index;
	struct sdhci_pad_cell *emmc_drive = emmc_drive_cap;

	const int emmc_speed[] = {
		MMC_HS_400_ES,  4,
		MMC_HS_400,     4,
		MMC_HS_200,     3,
		MMC_DDR_52,     2,
		MMC_HS_52,      1,
		MMC_HS,         1,
		MMC_LEGACY,     0,
	};

	speed_index = 0;

	for (i = 0; i < ARRAY_SIZE(emmc_speed); i += 2) {
		if (host->mmc->selected_mode == emmc_speed[i]) {
			speed_index = emmc_speed[i + 1];
			break;
		}
	}

	/* bit[5-6]: layer  bit[7-8]: inch*/
	emmc_intf = readl(BSP_CFG_CRG_REG);
	layer_index = (emmc_intf >> 5) & 0x03;
	inch_index = (emmc_intf >> 7) & 0x03;

	emmc_drive_offset = inch_index*(EMMC_LAYER_MAX * EMMC_SPEED_MAX) +
						layer_index*(EMMC_SPEED_MAX) + speed_index;

	emmc_drive += emmc_drive_offset;

	if (bus_width == 8) {
		reg = readl(SYS_CTRL_REG_BASE + REG_MISC_CTRL80);
		_reg = readl(SYS_CTRL_REG_BASE + REG_MISC_CTRL78);
		if ((_reg & IO_CTRL_MS) == LSADC_DETECT) {
			if (reg & IO_FLASH_MS) {
				/* flash电源域供电1.8V, 需切换SDIO0电源域供电电压为1.8V */
				/* step1: 配置POWER SWITCH输出电压为1.8V */
				reg = readl(SYS_CTRL_REG_BASE + REG_MISC_CTRL71);
				reg |= PS_POWER_SEL;
				writel(reg, SYS_CTRL_REG_BASE + REG_MISC_CTRL71);

				/* step2: 延迟2ms，要求大于1.2ms */
				mdelay(2);
			}
		} else {
			if (reg & POC_FLASH_MS) {
				/* flash电源域供电1.8V, 需切换SDIO0电源域供电电压为1.8V */
				/* step1: 配置POWER SWITCH输出电压为1.8V */
				reg = readl(SYS_CTRL_REG_BASE + REG_MISC_CTRL71);
				reg |= PS_POWER_SEL;
				writel(reg, SYS_CTRL_REG_BASE + REG_MISC_CTRL71);

				/* step2: 延迟2ms，要求大于1.2ms */
				mdelay(2);
			}
		}
	}

	set_drv_str(REG_CTRL_EMMC_CLK, 0, 1,
			emmc_drive->clk_pad & 0x01,
			emmc_drive->clk_pad >> 1);  /* set drv level */
	set_drv_str(REG_CTRL_EMMC_CMD, 1, 0,
			emmc_drive->dq_cmd_pad & 0x01,
			emmc_drive->dq_cmd_pad >> 1);	/* set drv level */
	for (i = 0; i < bus_width; i++)
		set_drv_str(io_emmc_data_reg[i], 1, 0,
				emmc_drive->dq_cmd_pad & 0x01,
				emmc_drive->dq_cmd_pad >> 1); /* set drv level */

	set_drv_str(REG_CTRL_EMMC_RST, 1, 0, 1, 0x0);  /* set drv level */

	if (host->mmc->selected_mode == MMC_HS_400 ||
		host->mmc->selected_mode == MMC_HS_400_ES) {
		set_drv_str(REG_CTRL_EMMC_DS, 1, 0,
				emmc_drive->dq_cmd_pad & 0x01,
				emmc_drive->dq_cmd_pad >> 1);	/* set drv level */
	}

	if (host->mmc->selected_mode == MMC_DDR_52) {
		reg = sdhci_readw(host, SDHCI_MULTI_CYCLE);
		reg &= ~SDHCI_DATA_DLY_EN;
		sdhci_writew(host, reg, SDHCI_MULTI_CYCLE);
	}
}

static void sd_set_ioconfig(struct sdhci_host *host)
{
	int i;
	unsigned int reg;
	unsigned int sdio_intf, sdio_drive_offset;
	unsigned int inch_index, layer_index, speed_index;
	struct sdhci_pad_cell *sdio_drive = sdio0_drive_cap;

	const int sdio_speed[] = {
		UHS_SDR104, 4,
		UHS_DDR50,  3,
		UHS_SDR50,  2,
		UHS_SDR25,  1,
		UHS_SDR12,  1,
		SD_HS,      0,
		SD_LEGACY,  0,
	};

	speed_index = 0;

	for (i = 0; i < ARRAY_SIZE(sdio_speed); i += 2) {
		if (host->mmc->selected_mode == sdio_speed[i]) {
			speed_index = sdio_speed[i + 1];
			break;
		}
	}

	/* bit[5-6]: layer  bit[9-10]: inch*/
	sdio_intf = readl(BSP_CFG_CRG_REG);
	layer_index = (sdio_intf >> 5) & 0x03;
	inch_index = (sdio_intf >> 9) & 0x03;

	sdio_drive_offset = inch_index*(SDIO_LAYER_MAX * SDIO_SPEED_MAX) +
						layer_index*(SDIO_SPEED_MAX) + speed_index;

	sdio_drive += sdio_drive_offset;

	if (host->mmc->signal_voltage == MMC_SIGNAL_VOLTAGE_180) {
		/* flash电源域供电1.8V, 需切换SDIO0电源域供电电压为1.8V */
		/* step1: 配置POWER SWITCH输出电压为1.8V */
		reg = readl(SYS_CTRL_REG_BASE + REG_MISC_CTRL71);
		reg |= PS_POWER_SEL;
		writel(reg, SYS_CTRL_REG_BASE + REG_MISC_CTRL71);

		/* step2: 延迟2ms，要求大于1ms */
		mdelay(2);

		/* step3: 配置SDIO0_VOUT电压为1.8V */
		reg = readl(SYS_CTRL_REG_BASE + REG_MISC_CTRL76);
		reg |= IO_SDIO0_MS;
		writel(reg, SYS_CTRL_REG_BASE + REG_MISC_CTRL76);

		reg = sdhci_readw(host, SDHCI_HOST_CONTROL2);
		reg |= SDHCI_CTRL_VDD_180;
		sdhci_writew(host, reg, SDHCI_HOST_CONTROL2);
	} else {
		/* flash电源域供电3.3V, 需切换SDIO0电源域供电电压为3.3V */
		/* step1: 配置POWER SWITCH输出电压为3.3V */
		reg = readl(SYS_CTRL_REG_BASE + REG_MISC_CTRL71);
		reg &= ~PS_POWER_SEL;
		writel(reg, SYS_CTRL_REG_BASE + REG_MISC_CTRL71);

		/* step2: 延迟2ms，要求大于1ms */
		mdelay(2);

		/* step3: 配置SDIO0_VOUT电压为3.3V */
		reg = readl(SYS_CTRL_REG_BASE + REG_MISC_CTRL76);
		reg &= ~IO_SDIO0_MS;
		writel(reg, SYS_CTRL_REG_BASE + REG_MISC_CTRL76);

		reg = sdhci_readw(host, SDHCI_HOST_CONTROL2);
		reg &= (~SDHCI_CTRL_VDD_180);
		sdhci_writew(host, reg, SDHCI_HOST_CONTROL2);
	}

	set_drv_str(REG_CTRL_SDIO0_CLK, 0, 1,
			sdio_drive->clk_pad & 0x01,
			sdio_drive->clk_pad >> 1);  /* set drv level */
	set_drv_str(REG_CTRL_SDIO0_CMD, 1, 0,
			sdio_drive->dq_cmd_pad & 0x01,
			sdio_drive->dq_cmd_pad >> 1);  /* set drv level */
	for (i = 0; i < IO_CFG_SDIO0_DATA_LINE_COUNT; i++)
		set_drv_str(io_sdio0_data_reg[i], 1, 0,
				sdio_drive->dq_cmd_pad & 0x01,
				sdio_drive->dq_cmd_pad >> 1); /* set drv level */

	if (host->mmc->selected_mode == UHS_DDR50) {
		reg = sdhci_readw(host, SDHCI_MULTI_CYCLE);
		reg &= ~SDHCI_DATA_DLY_EN;
		sdhci_writew(host, reg, SDHCI_MULTI_CYCLE);
	}
}

static void lotus_mmc_set_ioconfig(struct sdhci_host *host)
{
	unsigned int reg;

	if (host->type == MMC_TYPE_MMC) {
		reg = sdhci_readw(host, SDHCI_EMMC_CTRL);
		reg |= SDHCI_CARD_IS_EMMC;
		sdhci_writew(host, reg, SDHCI_EMMC_CTRL);

		lotus_emmc_set_ioconfig(host);
	} else {
		sd_set_ioconfig(host);
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
	//FIX HS400 and HS400ES to 150M clk
	if ((host->mmc->selected_mode == MMC_HS_400 ||
		host->mmc->selected_mode == MMC_HS_400_ES) && clk > 150000000)
		clk = 150000000;

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

#include "lotus_sdhci.c"
