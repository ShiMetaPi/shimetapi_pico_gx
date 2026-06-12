/*
 * Copyright (c) LOTUS. All rights reserved.
 */

#ifndef __CHIP_REGS_H__
#define __CHIP_REGS_H__


/* -------------------------------------------------------------------- */
#define FMC_REG_BASE		0x10000000

/* -------------------------------------------------------------------- */
#define REG_BASE_SF		0x100d0000
#define GMAC_REG_BASE		0x100d0000

/* -------------------------------------------------------------------- */
#define EMMC_REG_BASE		0x10010000
#define SDIO1_REG_BASE		0x10020000

/* -------------------------------------------------------------------- */
#define USB3_CTRL_REG_BASE	0x10030000
#define USB2_CTRL_REG_BASE      0x10030000

/* -------------------------------------------------------------------- */
#define USB_OHCI_BASE		0x10030000

/* -------------------------------------------------------------------- */
#define DDRC0_REG_BASE		0x11330000

/* -------------------------------------------------------------------- */
#define TIMER0_REG_BASE		0x12000000
#define TIMER1_REG_BASE		0x12000020
#define TIMER2_REG_BASE		0x12001000
#define TIMER3_REG_BASE		0x12001020

#define REG_TIMER_RELOAD		0x0
#define REG_TIMER_VALUE		0x4
#define REG_TIMER_CONTROL	0x8

#define CFG_TIMER_CLK		24000000
#define CFG_TIMERBASE		TIMER0_REG_BASE

/* enable timer.32bit, periodic,mask irq, 1 divider. */
#define CFG_TIMER_CTRL		0xC2

/* -------------------------------------------------------------------- */
/* Clock and Reset Generator REG */
/* -------------------------------------------------------------------- */
#define CRG_REG_BASE		0x12010000

#define REG_CRG80		0x0140
#define REG_CRG81		0x0144
#define REG_CRG91		0x016c
#define REG_CRG110		0x01b8

/* USB 2.0 CRG Control register offset */
#define REG_USB2_CTRL		REG_CRG80

/* FMC CRG register offset */
#define REG_FMC_CRG		REG_CRG81
#define FMC_SRST		(0x1 << 0)
#define FMC_CLK_ENABLE		(0x1 << 1)
#define FMC_CLK_SEL_MASK	(0x7 << 2)
#define FMC_CLK_SEL_SHIFT	0x2
/* SDR/DDR clock */
#define FMC_CLK_24M		0x0
#define FMC_CLK_100M		0x5
#define FMC_CLK_150M		0x4

#define FMC_CLK_200M		0x3

/* Only DDR clock */
#define FMC_CLK_300M		0x2
#define FMC_CLK_360M		0x1

#define fmc_clk_sel(_clk) \
	(((_clk) << FMC_CLK_SEL_SHIFT) & FMC_CLK_SEL_MASK)
#define get_fmc_clk_type(_reg) \
	(((_reg) & FMC_CLK_SEL_MASK) >> FMC_CLK_SEL_SHIFT)

/* Ethernet CRG register offset */
#define REG_ETH_CRG		REG_CRG91
#define REG_ETH_MAC_IF	0x8c

/* Uart CRG register offset */
#define REG_UART_CRG		REG_CRG110
#define uart_clk_sel(_clk)	(((_clk) & 0x3) << 18)
#define UART_CLK_SEL_MASK	(0x3 << 18)
#define UART_CLK_APB		0
#define UART_CLK_24M		1
#define UART_CLK_2M		2

/* SDIO0 CRG register offset */
#define REG_SDIO0_CRG		(CRG_REG_BASE + 0x22c)

/* eMMC CRG register offset */
#define REG_EMMC_CRG		(CRG_REG_BASE + 0x1f4)
#define mmc_clk_sel(_clk)	(((_clk) & 0x7) << 24)
#define MMC_CLK_SEL_MASK	(0x7 << 24)
#define get_mmc_clk_type(_reg)	(((_reg) >> 24) & 0x7)

/* -------------------------------------------------------------------- */
/* System Control REG */
/* -------------------------------------------------------------------- */
#define SYS_CTRL_REG_BASE	0x12020000
#define REG_BASE_SCTL		SYS_CTRL_REG_BASE
#define REG_SC_SYSSTAT		0x8c
#define spi_input_sle(x)	(((x) >> 16) & 0x1)

#define REG_MISC_CTRL71		0x8380
#define PS_POWER_SEL		1

#define REG_MISC_CTRL76		0x8800
#define IO_SDIO0_MS		1

#define REG_MISC_CTRL80		0x8810
#define IO_FLASH_MS		0x10

/* System Control register offset */
#define REG_SC_CTRL		0x0000
#define sc_ctrl_timer0_clk_sel(_clk)	(((_clk) & 0x1) << 16)
#define TIMER0_CLK_SEL_MASK	(0x1 << 16)
#define TIMER_CLK_3M		0
#define TIMER_CLK_BUS		1
#define SC_CTRL_REMAP_CLEAR	(0x1 << 8)

/* System soft reset register offset */
#define REG_SC_SYSRES		0x0008

/* System Status register offset */
#define REG_SYSSTAT		0x008c
/* bit[11]=0: 3-Byte address mode; bit[11]=1: 4-Byte address mode */
#define get_spi_nor_addr_mode(_reg) (((_reg) >> 11) & 0x1)
/* bit[6]=0; SPI nor flash; bit[6]=1: SPI nand flash */
#define get_spi_device_type(_reg) (((_reg) >> 10) & 0x1)
/* bit[1-0]: 00 - eMMC 4bits, 01 - SPI Nor, 10 - eMMC 8bits; 11 - SPI Nand */
#define get_sys_boot_mode(_reg)	 ((_reg) & 0x3)
#define BOOT_FROM_EMMC_4BITS	0x0
#define BOOT_FROM_SPI_NOR	0x1
#define BOOT_FROM_EMMC_8BITS	0x2
#define BOOT_FROM_SPI_NAND	0x3

#define NF_BOOTBW_MASK		(1 << 1)

/* Generic register */
#define REG_SC_DDRT0			0x90
#define REG_SC_DDRT1			0x94
#define REG_SC_DDRT2			0x98
#define REG_SC_DDRT3			0x9c
#define REG_SC_DDRT4			0xa0
#define REG_SC_DDRT5			0xa4
#define REG_SC_DDRT6			0xa8
#define REG_SC_DDRT7			0xac
#define REG_SC_DDRT8			0xb0
#define REG_SC_DDRT9			0xb4
#define REG_SC_DDRT10			0xb8
#define REG_SC_DDRT11			0xbc
#define REG_SC_DDRT12			0xc0
#define REG_SC_DDRT13			0xc4
#define REG_SC_DDRT14			0xc8
#define REG_SC_DDRT15			0xcc
#define REG_SC_SYSBOOT0			0x130
#define REG_SC_SYSBOOT1			0x134
#define REG_SC_SYSBOOT2			0x138
#define REG_SC_SYSBOOT3			0x13c
#define REG_SC_SYSBOOT4			0x140
#define REG_SC_SYSBOOT5			0x144
#define REG_SC_SYSBOOT6			0x148
#define REG_SC_SYSBOOT7			0x14c
#define REG_SC_SYSBOOT8			0x150
#define REG_SC_SYSBOOT9			0x154
#define REG_SC_SYSBOOT10		0x158
#define REG_SC_SYSBOOT11		0x15c
#define REG_SC_SYSBOOT12		0x160
#define REG_SC_SYSBOOT13		0x164
#define REG_SC_SYSBOOT14		0x168
#define REG_SC_SYSBOOT15		0x16c

/********** Communication Register and flag used by bootrom *************/
#define REG_START_FLAG	(SYS_CTRL_REG_BASE + REG_SC_SYSBOOT3)
#define START_MAGIC		0x444f574e
#define SELF_BOOT_TYPE_USBDEV	0x2/* debug */
#define REG_UPDATE_MODE_FLAG	0x12028200

/* -------------------------------------------------------------------- */
/* Peripheral Control REG */
/* -------------------------------------------------------------------- */
#define MISC_REG_BASE		0x12028000

#define MISC_CTRL17		0x0044
#define MISC_CTRL18		0x48
#define MISC_CTRL7		0x001C
#define MISC_CTRL8		0x0020
#define MISC_CTRL9		0x0024
#define MISC_CTRL62		0x80FC
#define MISC_CTRL63		0x8100
#define MISC_CTRL64		0x8104
#define MISC_CTRL65		0x8108
#define MISC_CTRL66		0x810c
#define MISC_CTRL67		0x8110

#define EMMC_ISO_EN		(0x1 << 16)
#define RG_EMMC_LHEN_IN		(0x3f << 17)

/* USB 2.0 MISC Control register offset */
#define REG_USB2_CTRL0		MISC_CTRL7
/* base on needs #define REG_USB2_CTRL1MISC_CTRL9 */

/* FEPHY Control register offset */
#define REG_FEPHY_CTRL0		MISC_CTRL8
#define REG_FEPHY_CTRL1		MISC_CTRL9

/* -------------------------------------------------------------------- */
#define IO_CONFIG_REG_BASE	0x12050000

/* -------------------------------------------------------------------- */
#define UART0_REG_BASE		0x12040000
#define UART1_REG_BASE		0x12041000
#define UART2_REG_BASE		0x12042000

/* -------------------------------------------------------------------- */
#define GPIO0_REG_BASE		0x120B0000
#define GPIO1_REG_BASE		0x120B1000
#define GPIO2_REG_BASE		0x120B2000
#define GPIO3_REG_BASE		0x120B3000
#define GPIO4_REG_BASE		0x120B4000
#define GPIO5_REG_BASE		0x120B5000
#define GPIO6_REG_BASE		0x120B6000
#define GPIO7_REG_BASE		0x120B7000
#define GPIO8_REG_BASE		0x120B8000
#define GPIO9_REG_BASE		0x120B9000

#define FMC_MEM_BASE		0x14000000
#define FMC_TEXT_ADRS		FMC_MEM_BASE
#define DDR_MEM_BASE		0x40000000
#define HW_DEC_INTR		86
/*-----------------------------------------------------------------------
 * EMMC / SD
 * ----------------------------------------------------------------------*/
/* SDIO0 REG */
#define SDIO0_BASE_REG		0x10040000

/* EMMC REG */
#define EMMC_BASE_REG		0x10010000

#define REG_BASE_PERI_CTRL	REG_BASE_SCTL
#define REG_BASE_IO_CONFIG	IO_CONFIG_REG_BASE

#define MMC_IOMUX_START_ADDR	0xF8
#define MMC_IOMUX_END_ADDR	0x13C
#define MMC_IOMUX_CTRL_MASK	(1<<0 | 1<<1)
#define MMC_IOMUX_CTRL		(1<<1)

/*---------------------------------------------------------------------
 *timestamp & stopwatch
 * ---------------------------------------------------------------------*/
#define STOPWATCH_CTRL_REG1		(SYS_CTRL_REG_BASE + MISC_CTRL62)
#define STOPWATCH_VALUE_REG		(SYS_CTRL_REG_BASE + MISC_CTRL63)
#define STOPWATCH_VALUE_REG_H		(SYS_CTRL_REG_BASE + MISC_CTRL64)
#define STOPWATCH_RUNNING_VALUE_REG	(SYS_CTRL_REG_BASE + MISC_CTRL65)
#define STOPWATCH_RUNNING_VALUE_REG_H	(SYS_CTRL_REG_BASE + MISC_CTRL66)
#define STOPWATCH_CTRL_REG2		(SYS_CTRL_REG_BASE + MISC_CTRL67)
#define TIMESTAMP_CNT_ADDR		0x1202016C
#define TIMESTAMP_ITEM_START_ADDR	0x04013000
#define TIMESTAMP_STOPWATCH_POINT_MAX	10
#define TIMESTAMP_ITEM_SIZE		16
#define TIMESTAMP_MAX_ITEM_CNT		200

/* --------------------------------------------------------- */
/* Chip ID */
#define REG_SC_CHIPID		0x12020EE0

#define MCU_SRAM_BASE	0x401C000

#define REG_MCU_FW_DDR_ADDR	0x120F0050
#define REG_MCU_FW_DDR_SIZE	0x120F0054

#endif /* End of __CHIP_REGS_H__ */

