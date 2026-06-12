// SPDX-License-Identifier: GPL-2.0
/* Copyright (c) LOTUS. All rights reserved. */

#ifndef __CHIP_ADAPTER_H__
#define __CHIP_ADAPTER_H__

#ifndef LOAD_ADDR

#if defined(CONFIG_TARGET_XM720XXX)
#define LOAD_ADDR ((unsigned char *)0x42000000)
#endif

#if defined(CONFIG_TARGET_XM760XXX)
#define LOAD_ADDR ((unsigned char *)0x41000000)
#endif

#if defined(CONFIG_TARGET_RATELVXX)
#define LOAD_ADDR ((unsigned char *)0x3a000000)
#endif

#endif /* LOAD_ADDR */

typedef enum {
	AD_BOOTFROM_EMMC = 0xF1,
	AD_BOOTFROM_NAND,
	AD_BOOTFROM_SPINAND,
	AD_BOOTFROM_SPINOR,
	AD_BOOTFROM_BUTT
} AD_BOOT_TYPE;

#if defined(CONFIG_TARGET_RATELVXX)
static int get_boot_info(void)
{
    unsigned int reg_val, boot_mode;
    int boot_media = 0xFF;

    reg_val = readl(REG_START_FLAG);
    boot_mode = BOOT_SOURCE(reg_val);

    switch (boot_mode) {
        case BOOT_FROM_NAND:
            boot_media = AD_BOOTFROM_NAND;
            break;
        case BOOT_FROM_SPI:
            boot_media = AD_BOOTFROM_SPINOR;
            break;
        case BOOT_FROM_EMMC:
            boot_media = AD_BOOTFROM_EMMC;
            break;
        default:
#ifdef CONFIG_LOTUS_FPGA
            boot_media = AD_BOOTFROM_SPINOR;
#else
            boot_media = 0xFF;
#endif
            break;
    }

    return boot_media;
}

#elif defined(CONFIG_TARGET_XMORCA)
static int get_boot_info(void)
{
	unsigned int reg;

	reg = readl((void *)(SYS_CTRL_REG_BASE + REG_SYSSTAT));
	if (get_sys_boot_mode(reg) == BOOT_FROM_SPI_NOR) {
		/* SFC_DEVICE_MODE:0:SPINOR 1:SPINAND */
		if (get_spi_device_type(reg)) {
            /* SPINAND */
			return AD_BOOTFROM_NAND;
		} else {
            /* SPINOR */
			return AD_BOOTFROM_SPINOR;
		}
	} else if (get_sys_boot_mode(reg) == BOOT_FROM_SPI_NAND) {
		return AD_BOOTFROM_NAND;
	} else if (get_sys_boot_mode(reg) == BOOT_FROM_EMMC_4BITS ||
               get_sys_boot_mode(reg) == BOOT_FROM_EMMC_8BITS) {
		return AD_BOOTFROM_EMMC;
	}

	return 0xFF;
}
#else
/* CONFIG_TARGET_XM720XXX | CONFIG_TARGET_XM760XXX */
static int get_boot_info(void)
{
	unsigned int reg;

	reg = readl((void *)(SYS_CTRL_REG_BASE + REG_SYSSTAT));
	if (get_sys_boot_mode(reg) == BOOT_FROM_SPI) {
		/* SFC_DEVICE_MODE:0:SPINOR 1:SPINAND */
		if (get_spi_device_type(reg)) {
            /* SPINAND */
			return AD_BOOTFROM_NAND;
		} else {
            /* SPINOR */
			return AD_BOOTFROM_SPINOR;
		}
	} else if (get_sys_boot_mode(reg) == BOOT_FROM_NAND) {
		return AD_BOOTFROM_NAND;
	} else if (get_sys_boot_mode(reg) == BOOT_FROM_EMMC) {
		return AD_BOOTFROM_EMMC;
	}

	return 0xFF;
}
#endif /* CONFIG_TARGET_RATELVXX */

#endif /* __CHIP_ADAPTER_H__ */
