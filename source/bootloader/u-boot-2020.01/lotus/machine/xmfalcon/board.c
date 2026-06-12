/*
 * Copyright (c) LOTUS. All rights reserved.
 */
#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/arch/platform.h>
#include <spi_flash.h>
#include <linux/mtd/mtd.h>
#include <nand.h>
#include <netdev.h>
#include <mmc.h>
#include <asm/sections.h>
#include <sdhci.h>
#include <linux/lotus/common.h>
#include <asm/mach-types.h>
#include <env_internal.h>
#include <linux/lotus/chip.h>

#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif
static int boot_media __section(.data) = BOOT_MEDIA_UNKNOWN;
int get_boot_media(void)
{
	unsigned int reg_val, boot_mode, spi_device_mode;

	if (boot_media != BOOT_MEDIA_UNKNOWN)
		return boot_media;

	reg_val = readl(SYS_CTRL_REG_BASE + REG_SYSSTAT);
	boot_mode = get_sys_boot_mode(reg_val);

	switch (boot_mode) {
	case BOOT_FROM_SPI:
		spi_device_mode = get_spi_device_type(reg_val);
		if (spi_device_mode)
			boot_media = BOOT_MEDIA_NAND;
		else
			boot_media = BOOT_MEDIA_SPIFLASH;
		break;
	case BOOT_FROM_EMMC:
		boot_media = BOOT_MEDIA_EMMC;
		break;
	default:
		boot_media = BOOT_MEDIA_UNKNOWN;
		break;
	}

	return boot_media;
}

void show_boot_media(void)
{
	puts("Boot Media: ");
	switch (boot_media) {
	case BOOT_MEDIA_SPIFLASH:
		puts("SPI Nor");
		break;
	case BOOT_MEDIA_NAND:
		puts("SPI Nand");
		break;
	case BOOT_MEDIA_EMMC:
		puts("eMMC");
		break;
	default:
		puts("Unknown");
		break;
	}

	puts("\r\n");
}

#if defined(CONFIG_SHOW_BOOT_PROGRESS)
void show_boot_progress(int progress)
{
	printf("Boot reached stage %d\n", progress);
}
#endif

int board_early_init_f(void)
{
	return 0;
}

int misc_init_r(void)
{
#ifdef CONFIG_RANDOM_ETHADDR
	random_init_r();
#endif
	env_set("verify", "n");

	return 0;
}

void show_chip_info(void)
{
	u32 chip_id = get_chipid();

	printf("Chip ID: 0x%X\n", chip_id);
}

int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_arch_number = MACH_TYPE_LOTUS;
	gd->bd->bi_boot_params = CFG_BOOT_PARAMS;

	get_boot_media();
	show_boot_media();
	show_chip_info();

	return 0;
}

u64 get_ddr_size(void)
{
	return readl(REG_BASE_SCTL + REG_SC_DDRT12);
}

int dram_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->ram_size = get_ddr_size();

	return 0;
}

void board_add_ram_info(int use_default)
{
#ifdef CONFIG_SYS_MEM_TOP_HIDE
	putc('(');
	print_size(CONFIG_SYS_MEM_TOP_HIDE, "");
	puts(" Reserved)");
#endif
}

void reset_cpu(ulong addr)
{
	/* 0x12345678:writing any value will cause a reset. */
	writel(0x12345678, SYS_CTRL_REG_BASE + REG_SC_SYSRES);
	while (1);
}

int timer_init(void)
{
	/*
	 * Under uboot, 0xffffffff is set to load register,
	 * timer_clk equals BUSCLK/2/256.
	 * e.g. BUSCLK equals 50M, it will roll back after 0xffffffff/timer_clk
	 * 43980s equals 12hours
	 */
	__raw_writel(0, TIMER0_REG_BASE + REG_TIMER_CONTROL);
	__raw_writel(~0, TIMER0_REG_BASE + REG_TIMER_RELOAD);

	/* 32 bit, periodic */
	__raw_writel(CFG_TIMER_CTRL, TIMER0_REG_BASE + REG_TIMER_CONTROL);

	return 0;
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;

#ifdef CONFIG_NET_FEMAC
	rc = bspeth_initialize(bis);
#endif

#ifdef	CONFIG_FTGMAC030
	rc = ftgmac030_initialize(bis);
#endif

	return rc;
}

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	int ret = 0;
#ifdef CONFIG_LOTUS_SDHCI
	int dev_num = 0;

	if (get_boot_media() == BOOT_MEDIA_EMMC) {
		ret = sdhci_add_port(dev_num, EMMC_BASE_REG, MMC_TYPE_MMC);
		if (!ret) {
			ret = lotus_mmc_init(dev_num);
			if (ret)
				printf("No EMMC device found !\n");
		}

		dev_num++;
	}

#ifdef CONFIG_AUTO_SD_UPDATE
	ret = sdhci_add_port(dev_num, SDIO0_BASE_REG, MMC_TYPE_SD);
	if (ret)
		return ret;

	ret = lotus_mmc_init(dev_num);
	if (ret)
		printf("No SD device found !\n");
#endif

#endif

	return ret;
}
#endif

enum env_location env_get_location(enum env_operation op, int prio)
{
	int boot_media = get_boot_media();
	enum env_location loc;

	if (prio)
		return ENVL_UNKNOWN;

	switch (boot_media) {
	case BOOT_MEDIA_SPIFLASH:
		loc = ENVL_SPI_FLASH;
		break;
	case BOOT_MEDIA_NAND:
		loc = ENVL_NAND;
		break;
	case BOOT_MEDIA_EMMC:
		loc = ENVL_MMC;
		break;
	default:
		loc = ENVL_UNKNOWN;
		break;
	}

	return loc;
}
