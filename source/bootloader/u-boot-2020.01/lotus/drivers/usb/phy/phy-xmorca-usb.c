
// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) LOTUS. All rights reserved.
 */
#include <asm/arch/platform.h>
#include <dm.h>
#include <linux/lotus/usb.h>
#include <usb.h>
#include <usb/xhci.h>
#include "phy-usb.h"

#ifdef __USB_DEBUG
#define USB_DEBUG(...) printf(__VA_ARGS__)
#else
#define USB_DEBUG(...)
#endif

#define GTXTHRCFG			0xc108
#define GRXTHRCFG			0xc10c
#define REG_GCTL			0xc110
#define GUSB2PHYCFG			0xc200
#define REG_GUSB3PIPECTL0		0xc2c0
#define PCS_SSP_SOFT_RESET		(0x1 << 31)
#define USBTRDTIM_MASK			0x00003c00
#define USBTRDTIM_16BIT			(0x5 << 10)
#define USBTRDTIM_8BIT			(0x9 << 10)
#define PHYIF				(0x1 << 3)
#define SUSPENDUSB20			(0x1 << 6)
#define ENBLSLPM			(0x1 << 8)
#define LSIPD_MASK				(0x7 << 19)
#define LSIPD_3_BIT				(0x2 << 19)
#define U2_FREECLK_EXISTS		(0x1 << 30)

#define PORT_CAP_DIR			(0x3 << 12)
#define PORT_SET_HOST			(0x1 << 12)
#define PORT_DISABLE_SUSPEND		(0x1 << 17)

#define USB2_G_TXTHRCFG			0x23100000
#define USB2_G_RXTHRCFG			0x23100000

/* CRG PERI_CRG80: usb clk & reset, offset 0x140 */
#define USB2_CTRL			0x140
#define USB2_CRG_DEF_VAL		0x1307
#define USB2_PHY_CFG_REQ			(0x1 << 0)
#define USB2_PHY_PORT_SRST			(0x1 << 1)
#define USB2_PHY_XTAL_CKEN			(0x1 << 2)
#define USB2_VCC_SRST_REQ			(0x1 << 3)
#define USB2_BUS_CKEN			(0x1 << 8)
#define USB2_REF_CKEN			(0x1 << 9)
#define USB2_UTMI_CKEN			(0x1 << 12)
#define USB2_FREECLK_CKSEL		(0x1 << 13)

/* u2 ctrl use reg, reg's misc_base=0x12028000 */
#define MISC_USB2_CTRL_REG		0x24
#define U2_BUS_FILTER_BYPASS	(0xf << 0)
#define U2_PWREN_MODE_MASK	(0x1 << 4)
#define U2_PWREN_MODE_MISC	(0x0 << 4)
#define U2_PWREN_MODE_CTRL	(0x1 << 4)
#define U2_OVERCURR_MODE_MASK	(0x1 << 5)
#define U2_OVERCURR_MODE_DISABLE	(0x0 << 5)
#define U2_OVERCURR_MODE_ENABLE	(0x1 << 5)
#define U2_PWREN_MASK	(0x1 << 6)
#define U2_PWREN_ON	(0x0 << 6)
#define U2_PWREN_OFF	(0x1 << 6)
#define U2_OVERCURR_MASK	(0x1 << 7)
#define U2_OVERCURR_ON	(0x0 << 7)
#define U2_OVERCURR_OFF	(0x1 << 7)
#define U2_FLADJ_30MHZ	(0x20 << 13)

/* Outer phy use reg, reg's misc_base=0x12028000 */
#define MISC_OUT_PHY2_CTRL_REG		0x2C
#define USB2_PLL_EN					(0x1 << 0)
#define USB2_UTMI_DATABUS16_8		(0x1 << 1)

#define MISC_USB2_VBUS_REG			0x980
#define U2_VBUS_EN					(0x1 << 0)

static uintptr_t xhci_base;

int xhci_hcd_init(int index, struct xhci_hccr **hccr, struct xhci_hcor **hcor)
{
	if ((hccr == NULL) || (hcor == NULL))
		return -EINVAL;

	xhci_base = USB2_CTRL_REG_BASE;

	*hccr = (struct xhci_hccr *)(xhci_base);
	*hcor = (struct xhci_hcor *)((uintptr_t) *hccr +
				HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase)));

	return 0;
}

void xhci_hcd_stop(int index)
{
	unsigned int reg;

	/* por reset */
	reg = readl(CRG_REG_BASE + USB2_CTRL);
	reg |= USB2_PHY_CFG_REQ;
	reg |= USB2_PHY_PORT_SRST;
	writel(reg, CRG_REG_BASE + USB2_CTRL);
	udelay(U_LEVEL10);

	/* vcc reset */
	reg = readl(CRG_REG_BASE + USB2_CTRL);
	reg |= USB2_VCC_SRST_REQ;
	writel(reg, CRG_REG_BASE + USB2_CTRL);
}
EXPORT_SYMBOL(xhci_hcd_stop);

static void usb2_eye_config(void)
{
}

static void usb2_trim_config(void)
{
}

static void lotus_u2_phy_clk_init(void)
{
	unsigned int reg;

	/* enable u2 phy clk and do reset */
	reg = (USB2_PHY_CFG_REQ | USB2_PHY_PORT_SRST | \
			USB2_PHY_XTAL_CKEN);
	writel(reg, CRG_REG_BASE + USB2_CTRL);
	USB_DEBUG("USB2_CRG(0x12010140), expext:0x7,"
		"acutal:0x%X\n", readl(CRG_REG_BASE + USB2_CTRL));

	udelay(1);

	/* undo phy reset */
	reg = readl(CRG_REG_BASE + USB2_CTRL);
	reg &= ~USB2_PHY_CFG_REQ;
	reg &= ~USB2_PHY_PORT_SRST;
	writel(reg, CRG_REG_BASE + USB2_CTRL);
	USB_DEBUG("USB2_CRG(0x12010140), expext:0x4,"
		"acutal:0x%X\n", readl(CRG_REG_BASE + USB2_CTRL));
}

static void lotus_u2_ctrl_clk_init(void)
{
	unsigned int reg;

	/* enable u2 ctrl clk and do reset */
	reg = readl(CRG_REG_BASE + USB2_CTRL);
	reg |= (USB2_VCC_SRST_REQ | USB2_BUS_CKEN | USB2_REF_CKEN |
		USB2_UTMI_CKEN);
	writel(reg, CRG_REG_BASE + USB2_CTRL);
	USB_DEBUG("USB2_CRG(0x12010140), expext:0x130c,"
		"acutal:0x%X\n", readl(CRG_REG_BASE + USB2_CTRL));

	udelay(1);

	/* undo ctrl vcc reset */
	reg = readl(CRG_REG_BASE + USB2_CTRL);
	reg &= ~USB2_VCC_SRST_REQ;
	writel(reg, CRG_REG_BASE + USB2_CTRL);
	USB_DEBUG("USB2_CRG(0x12010140), expext:0x1304,"
		"acutal:0x%X\n", readl(CRG_REG_BASE + USB2_CTRL));
}

static void lotus_usb2_clk_init(void)
{
	lotus_u2_phy_clk_init();
	lotus_u2_ctrl_clk_init();
}

static void lotus_u2_phy_misc_config(void)
{
	unsigned int reg;

	/* MISC_CTRL11: config 0x3 according to IC. */
	reg = 0;
	reg |= (USB2_PLL_EN | USB2_UTMI_DATABUS16_8);
	writel(reg, MISC_REG_BASE + MISC_OUT_PHY2_CTRL_REG);
	USB_DEBUG("MISC_OUT_PHY2_CTRL_REG(0x1202802c),"
		"expext:0x3, acutal:0x%X\n",
		readl(MISC_REG_BASE + MISC_OUT_PHY2_CTRL_REG));
}

static inline void lotus_usb2_vbus_valid_en(void)
{
	unsigned int reg;

	/* enable U2 vbus valid */
	reg = readl(MISC_REG_BASE + MISC_USB2_VBUS_REG);
	reg |= U2_VBUS_EN;
	writel(reg, MISC_REG_BASE + MISC_USB2_VBUS_REG);
}

static void lotus_dwc_data_width_config(enum usb_phy_width_mode mode)
{
	unsigned int reg, base = USB2_CTRL_REG_BASE;

	/* GUSB2PHYCFG register: config val 0x40001548(HS/FS) or
	 * 0x00381548(LS) according to IC.
	 *
	 * USBTRDTIM: USB 2.0 Turnaround Time in PHY clocks.
	 * PHYIF: 1 UTMI+ PHY is 16-bit, 0 UTMI+ PHY is 8-bit.
	 */
	reg = 0;
	switch (mode) {
	case PHY_WIDTH_8BIT:
		/* 8-bits usb phy configure */
		reg |= USBTRDTIM_8BIT;
		reg &= ~PHYIF;
		break;
	case PHY_WIDTH_16BIT:
		/* 16-bits usb phy configure */
		reg |= (USBTRDTIM_16BIT | PHYIF);
		break;
	default:
		printf("usb phy width config val error.\n");
		return;
	}

	writel(reg, base + GUSB2PHYCFG);
}

static void lotus_dwc_usb_config(void)
{
	unsigned int reg, base = USB2_CTRL_REG_BASE;

	reg = readl(base + REG_GUSB3PIPECTL0);
	reg |= PCS_SSP_SOFT_RESET;
	writel(reg, base + REG_GUSB3PIPECTL0);
	udelay(U_LEVEL2);

	reg = readl(base + REG_GCTL);
	reg &= ~PORT_CAP_DIR;
	reg |= PORT_SET_HOST; /* host mode is default. */
	writel(reg, base + REG_GCTL);
	udelay(U_LEVEL2);

	reg = readl(base + REG_GUSB3PIPECTL0);
	reg &= ~PCS_SSP_SOFT_RESET;
	reg &= ~PORT_DISABLE_SUSPEND; /* disable suspend */
	writel(reg, base + REG_GUSB3PIPECTL0);
	udelay(U_LEVEL2);

	writel(USB2_G_TXTHRCFG, base + GTXTHRCFG);
	writel(USB2_G_RXTHRCFG, base + GRXTHRCFG);
	udelay(U_LEVEL2);

	lotus_dwc_data_width_config(PHY_WIDTH_16BIT);

	reg = readl(base + GUSB2PHYCFG);
	reg &= ~SUSPENDUSB20;	/* disable suspend */
	reg &= ~ENBLSLPM;	/* suspend not transfer to external PHY */
	reg &= ~U2_FREECLK_EXISTS;
	reg &= ~LSIPD_MASK;
	reg |= LSIPD_3_BIT;
	writel(reg, base + GUSB2PHYCFG);

	USB_DEBUG("GUSB2PHYCFG(0xc200):0x%X\n",
		readl(base + GUSB2PHYCFG));
	USB_DEBUG("GUSB3PIPECTL0(0xc2c0):0x%X\n",
		readl(base + REG_GUSB3PIPECTL0));
	USB_DEBUG("GCTL(0xc110):0x%X\n",
		readl(base + REG_GCTL));
}

void phy_usb_init(int index)
{
	/* u2 utmi data bus config, u2 phy pll en */
	lotus_u2_phy_misc_config();

	/* init crg and clk */
	lotus_usb2_clk_init();

	/* enable u2 vbusvalid */
	lotus_usb2_vbus_valid_en();

	/* USB2 Controller configs */
	lotus_dwc_usb_config();

	/* lotus phy eye diagram config */
	usb2_eye_config();

	/* lotus phy trim config */
	usb2_trim_config();
}
EXPORT_SYMBOL(phy_usb_init);

/* Some usb device can't be recognize in U-boot because
 * of error state, so we power off them and wait a delay
 * to restart them, similar to unplug and then insert.
 */
void lotus_usb_pwren_control(void)
{
	u32 reg, default_val, delay;

	delay = CONFIG_USB_PWREN_OFF_DELAY ?
		CONFIG_USB_PWREN_OFF_DELAY : 100;

	reg = readl(MISC_REG_BASE + MISC_USB2_CTRL_REG);
	default_val = reg;
	reg &= ~U2_PWREN_MODE_MASK; /* set PWREN_MISC mode */
	reg |= U2_PWREN_OFF; /* turn off PWREN */
	writel(reg, MISC_REG_BASE + MISC_USB2_CTRL_REG);

	mdelay(delay);

	reg = readl(MISC_REG_BASE + MISC_USB2_CTRL_REG);
	reg &= ~U2_PWREN_MASK; /* turn on PWREN */
	writel(reg, MISC_REG_BASE + MISC_USB2_CTRL_REG);

	reg = default_val; /* set PWREN_CTRL mode(default val) */
	writel(reg, MISC_REG_BASE + MISC_USB2_CTRL_REG);
}
