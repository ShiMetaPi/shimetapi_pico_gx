// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) LOTUS. All rights reserved.
 */
#include <asm/arch/platform.h>
#include <dm.h>
#include <linux/lotus/usb.h>
#include <usb.h>
#include <usb/xhci.h>
#include <linux/lotus/chip.h>

#include "phy-usb.h"

#ifdef __USB_DEBUG
#define USB_DEBUG(...) printf(__VA_ARGS__)
#else
#define USB_DEBUG(...)
#endif

/* CRG PERI_CRG80: usb clk & reset, offset 0x140 */
#define USB2_CTRL			0x140
#define OUTER_PHY_CRG_DEF_VAL		0x130d
#define LOTUS_PHY_CRG_DEF_VAL		0x3130d
#define LOTUS_PHY_REF_CKEN		(0x1 << 17)
#define LOTUS_PHY_APB_CKEN		(0x1 << 16)
#define USB2_UTMI_CKEN			(0x1 << 12)
#define USB2_PHY_APB_CKEN		(0x1 << 11)
#define USB2_REF_CKEN			(0x1 << 9)
#define USB2_BUS_CKEN			(0x1 << 8)
#define USB2_PHY_PLL_CKEN		(0x1 << 4)
#define USB2_PHY_XTAL_CKEN		(0x1 << 2)
#define USB2_FREECLK_CKSEL		(0x1 << 13)
#define USB2_PHY_APB_RST		(0x1 << 10)
#define USB2_VCC_SRST_REQ		(0x1 << 3)
#define USB2_PHY_REQ			(0x1 << 0)
#define USB2_PHY_PORT_TREQ		(0x1 << 1)

#define USB3_CTRL			0x13c
#define USB3_PHY_SRST		(0x1 << 8)
#define USB3_PHY_XTAL_CKEN	(0x1 << 7)
#define USB3_VCC_SRST		(0x1 << 6)
#define USB3_PIPE3_RX_CKEN	(0x1 << 5)
#define USB3_PIPE3_TX_CKEN	(0x1 << 4)
#define USB3_UTMI_CKEN		(0x1 << 3)
#define USB3_SUSPEND_CKEN	(0x1 << 2)
#define USB3_REF_CKEN		(0x1 << 1)
#define USB3_BUS_CKEN		(0x1 << 0)

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

/* Lotus phy use reg, reg's misc_base=0x12028000 */
#define MISC_LOTUS_PHY2_CTRL_REG	0x28
#define MISC_USB_SEL_SCENE1		(0x0 << 0)
#define MISC_USB_SEL_SCENE2		(0x1 << 0)
#define MISC_USB_SCENE_MASK		(0x1 << 0)
#define USB2_UTMI_REG_PROTECT1		(0x1 << 8)
#define USB2_UTMI_REG_PROTECT2		(0x1 << 9)
#define USB2_UTMI_REG_PROTECT3		(0x1 << 10)
#define USB2_HW_CHIRP			(0x1 << 14)
#define USB2_UTMI_IDPULLUP		(0x1 << 15)

/* Outer phy use reg, reg's misc_base=0x12028000 */
#define MISC_OUT_PHY2_CTRL_REG		0x2C
#define USB2_OSCOUTEN			(0x1 << 8)
#define USB2_PLL_EN			(0x1 << 12)
#define USB2_UTMI_DATABUS16_8		(0x1 << 16)
#define MISC_OUT_PHY2_XCFGI_63TO32	0x988
#define CFG_OUT_PHY2_EYE_VAL		(0x1 << 5)

/* reg base(misc_base): 0x12028000 */
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

#define MISC_USB3_CTRL_REG		0x1110
#define U3_BUS_FILTER_BYPASS	(0xf << 0)
#define U3_FLADJ_30MHZ	(0x20 << 4)
#define U3_PWREN_MODE_MASK	(0x1 << 11)
#define U3_PWREN_MODE_MISC	(0x0 << 11)
#define U3_PWREN_MODE_CTRL	(0x1 << 11)
#define U3_OVERCURR_MODE_MASK	(0x1 << 12)
#define U3_OVERCURR_MODE_DISABLE	(0x0 << 12)
#define U3_OVERCURR_MODE_ENABLE	(0x1 << 12)
#define U3_PWREN_MASK	(0x1 << 13)
#define U3_PWREN_ON	(0x0 << 13)
#define U3_PWREN_OFF	(0x1 << 13)
#define U3_OVERCURR_MASK	(0x1 << 14)
#define U3_OVERCURR_ON	(0x0 << 14)
#define U3_OVERCURR_OFF	(0x1 << 14)
#define HOST_U2_PORT_MASK	(0xf << 20)
#define HOST_U2_PORT(n)	(n << 20)
#define HOST_U3_PORT_MASK	(0xf << 24)
#define HOST_U3_PORT(n)	(n << 24)
#define USB2_HOST_VBUS_EN	(0x1 << 30)
#define USB3_HOST_VBUS_EN	(0x1 << 31)

#define MISC_USB3_PHY_REG		0x112c

/* SYSBOOT1[2:2]: usb scene select */
#define SYSBOOT1		0x12020134
#define SYSBOOT1_SEL_USB_SCENE_MASK		(1 << 2)

static uintptr_t xhci_base;

/**
 * lotus_chip_u3_ability - whether current chip support u3.
 *
 * @return:
 * 1: support u3.
 * 0: only support u2.
 */
static int lotus_chip_u3_ability(void)
{
	u32 id = get_chipid();

	USB_DEBUG("%s: chid id 0x%x\n", __func__, id);

	if (id == 0x76050110 || id == 0x06050110)
		return 0;

	/* if id == 0, consider chip support U3. */
	return 1;
}

int xhci_hcd_init(int index, struct xhci_hccr **hccr, struct xhci_hcor **hcor)
{
#ifdef CONFIG_USB_DUAL_PORT_SUPPORT
	unsigned int reg;

	reg = readl(MISC_REG_BASE + MISC_LOTUS_PHY2_CTRL_REG);
	reg &= MISC_USB_SCENE_MASK;

	if (index >= LOTUS_USB_CTRL_UNKNOWN)
		return -ENODEV;
	/* 7605v11 only support u2 host */
	if (!lotus_chip_u3_ability() && index == LOTUS_DWC_U3)
		return -ENODEV;

	/* Scene2 don't init U2 host. */
	if (index == LOTUS_DWC_U2 && reg)
		return -ENODEV;
#endif

	if ((hccr == NULL) || (hcor == NULL))
		return -EINVAL;

#ifdef CONFIG_USB_DUAL_PORT_SUPPORT
	if (index == LOTUS_DWC_U3)
		xhci_base = USB3_CTRL_REG_BASE;
	else if (index == LOTUS_DWC_U2)
		xhci_base = USB2_CTRL_REG_BASE;
#else
	/* single usb port, U3 is default */
	if (index == LOTUS_DWC_U3) {
		/* 7605v11 only support U2 host */
		if (!lotus_chip_u3_ability())
			xhci_base = USB2_CTRL_REG_BASE;
		else
			xhci_base = USB3_CTRL_REG_BASE;
	} else
		return -ENODEV;
#endif
	*hccr = (struct xhci_hccr *)(xhci_base);
	*hcor = (struct xhci_hcor *)((uintptr_t) *hccr +
				HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase)));

	USB_DEBUG("hccr:0x%x, hcor:0x%x.\n", (unsigned int)*hccr, (unsigned int)*hcor);
	return 0;
}

/* Get usb scene from SYSBOOT1 and config to MISC
 * usb_scene: 0-scene1, 1-scene2.
 * scene1:U3 only + U2.
 * scene2:U3(compatible to U2).
 */
#ifdef CONFIG_USB_DUAL_PORT_SUPPORT
static void usb_scene_select(int index, int type)
#else
static void usb_scene_select(int index)
#endif
{
	unsigned int reg, usb_scene = LOTUS_USB_SCENE2;

#ifdef CONFIG_USB_DUAL_PORT_SUPPORT
	reg = readl(SYSBOOT1);
	if (reg & SYSBOOT1_SEL_USB_SCENE_MASK)
		usb_scene = LOTUS_USB_SCENE1;

	/* used by usb download */
	if (index == LOTUS_DWC_U2 && type == USB_INIT_DEVICE)
		usb_scene = LOTUS_USB_SCENE1;
#else
	/* used by usb download */
	if ((index == LOTUS_DWC_U2) ||
		(index == LOTUS_DWC_U3 && !lotus_chip_u3_ability()))
		usb_scene = LOTUS_USB_SCENE1;
#endif

	reg = 0;
	if (usb_scene == LOTUS_USB_SCENE1) {
		/* u3 ctrl connect u3 phy, u2 ctrl connect u2 phy */
		reg |= MISC_USB_SEL_SCENE1;
		printf("%s:select U3 only + U2.\n", __func__);
	} else if (usb_scene == LOTUS_USB_SCENE2) {
		/* u3 ctrl connect u3 phy and u2 phy */
		reg |= MISC_USB_SEL_SCENE2;
		printf("%s:select U3(compatible to U2).\n", __func__);
	}
	writel(reg, MISC_REG_BASE + MISC_LOTUS_PHY2_CTRL_REG);

	USB_DEBUG("MISC_LOTUS_PHY2_CTRL_REG(0x12028028):0x%X\n",
		readl(MISC_REG_BASE + MISC_LOTUS_PHY2_CTRL_REG));
}

static void lotus_u2_phy_eye_config(void)
{
	unsigned int reg;

	/* outer phy eye diagram config */
	reg = readl(MISC_REG_BASE + MISC_OUT_PHY2_XCFGI_63TO32);
	reg |= CFG_OUT_PHY2_EYE_VAL;
	writel(reg, MISC_REG_BASE + MISC_OUT_PHY2_XCFGI_63TO32);
}

static void lotus_dwc_data_width_config(int index,
	enum usb_phy_width_mode mode)
{
	unsigned int reg, base = 0;

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

#ifdef CONFIG_USB_DUAL_PORT_SUPPORT
	if (index == LOTUS_DWC_U3)
		base = USB3_CTRL_REG_BASE;
	else
		base = USB2_CTRL_REG_BASE;
#else
	if (index == LOTUS_DWC_U3 && lotus_chip_u3_ability())
		base = USB3_CTRL_REG_BASE;
	else
		base = USB2_CTRL_REG_BASE;
#endif
	writel(reg, base + GUSB2PHYCFG);
}

static inline void lotus_usb2_vbus_valid_en(void)
{
	unsigned int reg;

	/* enable U2 vbus valid */
	reg = readl(MISC_REG_BASE + MISC_USB3_CTRL_REG);
	reg |= USB2_HOST_VBUS_EN;
	writel(reg, MISC_REG_BASE + MISC_USB3_CTRL_REG);
}

static inline void lotus_usb3_vbus_valid_en(void)
{
	unsigned int reg;

	/* enable U3 vbus valid */
	reg = readl(MISC_REG_BASE + MISC_USB3_CTRL_REG);
	reg |= USB3_HOST_VBUS_EN;
	writel(reg, MISC_REG_BASE + MISC_USB3_CTRL_REG);
}

static void lotus_u2_phy_clk_init(void)
{
	unsigned int reg;

	/* enable u2 phy clk and do reset */
	reg = (USB2_PHY_REQ | USB2_PHY_XTAL_CKEN);
	writel(reg, CRG_REG_BASE + USB2_CTRL);

	udelay(U_LEVEL6);

	/* undo phy reset */
	reg = readl(CRG_REG_BASE + USB2_CTRL);
	reg &= ~USB2_PHY_REQ;
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

	udelay(U_LEVEL6);

	/* undo ctrl vcc reset */
	reg = readl(CRG_REG_BASE + USB2_CTRL);
	reg &= ~USB2_VCC_SRST_REQ;
	writel(reg, CRG_REG_BASE + USB2_CTRL);
}

static void inline lotus_usb2_clk_init(void)
{
	/* write CRG default value 0x130d, including:
	 *
	 * USB2_PHY_REQ: reset PHY UTMI
	 * USB2_PHY_XTAL_CKEN: open phy xtal clk
	 * USB2_VCC_SRST_REQ: reset ctrl VCC
	 * USB2_BUS_CKEN: open bus clk
	 * USB2_REF_CKEN: open ctrl ref clk
	 * USB2_UTMI_CKEN: open UTMI clk
	 * LOTUS_PHY_APB_CKEN: open LOTUS phy apb clk
	 * USB2_UTMI_CKEN: open LOTUS phy ref clk
	 */
	lotus_u2_phy_clk_init();
	lotus_u2_ctrl_clk_init();
	USB_DEBUG("USB2_CRG(0x12010140), expext:0x1304,"
		"acutal:0x%X\n", readl(CRG_REG_BASE + USB2_CTRL));
}

static void inline lotus_usb2_clk_exit(void)
{
	unsigned int reg;

	/* disable usb2 clk and do reset */
	reg = 0;
	reg |= (USB2_VCC_SRST_REQ | USB2_PHY_REQ);
	writel(reg, CRG_REG_BASE + USB2_CTRL);
	USB_DEBUG("USB2_CRG(0x12010140), expext:0x9,"
		"acutal:0x%X\n", readl(CRG_REG_BASE + USB2_CTRL));
}

static void lotus_usb3_clk_init(void)
{
	unsigned int reg;

	/* write CRG default value 0x1ff, including:
	 *
	 * USB3_PHY_SRST: reset U3 PHY UTMI
	 * USB3_PHY_XTAL_CKEN: open U3 phy xtal clk
	 * USB3_VCC_SRST: reset U3 ctrl VCC
	 * USB3_PIPE3_RX_CKEN: open PIPE3 RX clk
	 * USB3_PIPE3_TX_CKEN: open PIPE3 TX clk
	 * USB3_UTMI_CKEN: open UTMI clk
	 * USB3_SUSPEND_CKEN: open U3 ctrl suspend clk
	 * USB3_REF_CKEN: open U3 ctrl ref clk
	 * USB3_BUS_CKEN: open U3 ctrl bus clk
	 */

	reg = 0;
	reg |= (USB3_PHY_SRST | USB3_PHY_XTAL_CKEN | USB3_VCC_SRST |
		USB3_PIPE3_RX_CKEN | USB3_PIPE3_TX_CKEN | USB3_UTMI_CKEN |
		USB3_SUSPEND_CKEN | USB3_REF_CKEN | USB3_BUS_CKEN);
	writel(reg, CRG_REG_BASE + USB3_CTRL);
	USB_DEBUG("USB3_CRG(0x1201013c), expext:0x1ff,"
		"acutal:0x%X\n", readl(CRG_REG_BASE + USB3_CTRL));

	udelay(U_LEVEL6);

	/* undo usb phy and ctrl reset */
	reg = readl(CRG_REG_BASE + USB3_CTRL);
	reg &= ~USB3_PHY_SRST;
	reg &= ~USB3_VCC_SRST;
	writel(reg, CRG_REG_BASE + USB3_CTRL);
	USB_DEBUG("USB3_CRG(0x1201013c), expext:0xbf,"
		"acutal:0x%X\n", readl(CRG_REG_BASE + USB3_CTRL));
}

static void lotus_usb3_clk_exit(void)
{
	unsigned int reg;

	/* disable usb3 clk and do reset */
	reg = 0;
	reg |= (USB3_PHY_SRST | USB3_VCC_SRST);
	writel(reg, CRG_REG_BASE + USB3_CTRL);
	USB_DEBUG("USB3_CRG(0x1201013c), expext:0x140,"
		"acutal:0x%X\n", readl(CRG_REG_BASE + USB3_CTRL));
}

static void lotus_u2_phy_misc_config(void)
{
	unsigned int reg;

	/* MISC_OUT_PHY2_CTRL: config 0x11100 according to IC. */
	reg = 0;
	reg |= (USB2_OSCOUTEN | USB2_PLL_EN | USB2_UTMI_DATABUS16_8);
	writel(reg, MISC_REG_BASE + MISC_OUT_PHY2_CTRL_REG);
	USB_DEBUG("MISC_OUT_PHY2_CTRL_REG(0x1202802c),"
		"expext:0x11100, acutal:0x%X\n",
		readl(MISC_REG_BASE + MISC_OUT_PHY2_CTRL_REG));
}

static void lotus_u2_ctrl_misc_config(void)
{
	unsigned int reg;

	/* MISC_USB2_CTRL_REG: config 0x400ff according to IC. */
	reg = 0;
	reg |= (U2_BUS_FILTER_BYPASS | U2_PWREN_MODE_CTRL | U2_OVERCURR_MODE_ENABLE |\
		U2_PWREN_OFF | U2_OVERCURR_OFF | U2_FLADJ_30MHZ);
	writel(reg, MISC_REG_BASE + MISC_USB2_CTRL_REG);
	USB_DEBUG("MISC_USB2_CTRL_REG(0x12028024),"
		"expext:0x400ff, acutal:0x%X\n",
		readl(MISC_REG_BASE + MISC_USB2_CTRL_REG));
}

static void lotus_u3_ctrl_misc_config(void)
{
	unsigned int reg;

	/* MISC_USB3_CTRL_REG: config 0xc1107a0f according to IC. */
	reg = 0;
	reg |= HOST_U3_PORT(1);
	reg |= HOST_U2_PORT(1);
	reg |= (U3_OVERCURR_OFF | U3_PWREN_OFF | U3_OVERCURR_MODE_ENABLE |\
		U3_PWREN_MODE_CTRL | U3_FLADJ_30MHZ | U3_BUS_FILTER_BYPASS);
	writel(reg, MISC_REG_BASE + MISC_USB3_CTRL_REG);
	USB_DEBUG("MISC_USB3_CTRL_REG(0x12029110), acutal:0x%X\n",
		readl(MISC_REG_BASE + MISC_USB3_CTRL_REG));
}

/* config U3 phy inner reg, insure U3 phy CRG already open. */
static void lotus_u3_phy_misc_config(void)
{
	/* adjust pi_current_trim */
	writel(0x11, MISC_REG_BASE + MISC_USB3_PHY_REG);
	udelay(1);	/* asic say delay 10ns, we delay 1us */
	writel(0x4211, MISC_REG_BASE + MISC_USB3_PHY_REG);
	udelay(1);
	writel(0x11, MISC_REG_BASE + MISC_USB3_PHY_REG);
	udelay(1);

	/* close slew_assist_dis */
	writel(0x2, MISC_REG_BASE + MISC_USB3_PHY_REG);
	udelay(1);
	writel(0x100202, MISC_REG_BASE + MISC_USB3_PHY_REG);
	udelay(1);
	writel(0x2, MISC_REG_BASE + MISC_USB3_PHY_REG);
	udelay(1);
	USB_DEBUG("MISC_USB3_PHY_REG(0x1202912c):0x%X\n",
		readl(MISC_REG_BASE + MISC_USB3_PHY_REG));
}

static void lotus_dwc_usb_config(int index)
{
	unsigned int reg, base = 0;

#ifdef CONFIG_USB_DUAL_PORT_SUPPORT
	if (index == LOTUS_DWC_U2)
		base = USB2_CTRL_REG_BASE;
	else
		base = USB3_CTRL_REG_BASE;
#else
	if (index == LOTUS_DWC_U2 ||
		(index == LOTUS_DWC_U3 && !lotus_chip_u3_ability())) {
		/* USB2 Controller configs */
		base = USB2_CTRL_REG_BASE;
	}

	if ((index == LOTUS_DWC_U3 && lotus_chip_u3_ability())) {
		/* USB3 Controller configs */
		base = USB3_CTRL_REG_BASE;
	}
#endif

	reg = readl(base + REG_GUSB3PIPECTL0);
	reg |= PCS_SSP_SOFT_RESET;
	writel(reg, base + REG_GUSB3PIPECTL0);
	udelay(U_LEVEL2);

	reg = readl(base + REG_GCTL);
	reg &= ~PORT_CAP_DIR;
	reg |= PORT_SET_HOST; /* [13:12] 01: Host; 10: Device; 11: OTG */
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

	lotus_dwc_data_width_config(index, PHY_WIDTH_16BIT);

	reg = readl(base + GUSB2PHYCFG);
	reg &= ~SUSPENDUSB20;	/* disable suspend */
	reg &= ~ENBLSLPM;	/* suspend not transfer to external PHY */
	reg &= ~U2_FREECLK_EXISTS;
	reg &= ~LSIPD_MASK;
	reg |= LSIPD_3_BIT;
	writel(reg, base + GUSB2PHYCFG);

#ifdef CONFIG_USB_DUAL_PORT_SUPPORT
	if (index == LOTUS_DWC_U2) {
#else
	if (index == LOTUS_DWC_U2 ||
		(index == LOTUS_DWC_U3 && !lotus_chip_u3_ability())) {
#endif
		USB_DEBUG("U2 GUSB2PHYCFG(0xc200):0x%X\n",
			readl(base + GUSB2PHYCFG));
		USB_DEBUG("U2 GUSB3PIPECTL0(0xc2c0):0x%X\n",
			readl(base + REG_GUSB3PIPECTL0));
		USB_DEBUG("U2 GCTL(0xc110):0x%X\n",
			readl(base + REG_GCTL));
#ifdef CONFIG_USB_DUAL_PORT_SUPPORT
	} else if (index == LOTUS_DWC_U3) {
#else
	} else if ((index == LOTUS_DWC_U3 && lotus_chip_u3_ability())) {
#endif
		USB_DEBUG("U3 GUSB2PHYCFG(0xc200):0x%X\n",
			readl(base + GUSB2PHYCFG));
		USB_DEBUG("U3 GUSB3PIPECTL0(0xc2c0):0x%X\n",
			readl(base + REG_GUSB3PIPECTL0));
		USB_DEBUG("U3 GCTL(0xc110):0x%X\n",
			readl(base + REG_GCTL));
	}
}

#ifdef CONFIG_USB_DUAL_PORT_SUPPORT
void phy_usb_init(int index, int type)
#else
void phy_usb_init(int index)
#endif
{
#ifdef CONFIG_USB_DUAL_PORT_SUPPORT
	unsigned int reg;

	/* 7605v11 only support u2 host */
	if (!lotus_chip_u3_ability() && index == LOTUS_DWC_U3)
		return;
	usb_scene_select(index, type);
#else
	usb_scene_select(index);
#endif

#ifdef CONFIG_USB_DUAL_PORT_SUPPORT
	reg = readl(MISC_REG_BASE + MISC_LOTUS_PHY2_CTRL_REG);
	reg &= MISC_USB_SCENE_MASK;

	/* Scene2 don't init U2 host. */
	if (index == LOTUS_DWC_U2 && reg)
		return;

	if (index == LOTUS_DWC_U3) {
#else
	if (index == LOTUS_DWC_U3 && lotus_chip_u3_ability()) {
#endif
		printf("USB3 init...\n");
		/* config u3 ctrl/u3 phy/u2 phy */
		lotus_u3_ctrl_misc_config();
		lotus_usb3_vbus_valid_en();
		lotus_u2_phy_misc_config();
		lotus_usb3_clk_init();
		lotus_u3_phy_misc_config();
		lotus_u2_phy_clk_init();
		lotus_dwc_usb_config(index);
#ifdef CONFIG_USB_DUAL_PORT_SUPPORT
	} else if (index == LOTUS_DWC_U2) {
#else
	} else if ((index == LOTUS_DWC_U2) ||
		(index == LOTUS_DWC_U3 && !lotus_chip_u3_ability())) {
#endif
		printf("USB2 init...\n");
		/* config u2 ctrl and u2 phy */
		lotus_u2_ctrl_misc_config();
		lotus_usb2_vbus_valid_en();
		lotus_u2_phy_misc_config();
		lotus_usb2_clk_init();
		lotus_dwc_usb_config(index);
		/* u2 phy eye diagram config */
		lotus_u2_phy_eye_config();
	}
	/* According to IC, usb phy clk init need 1ms delay */
	mdelay(1);
}
EXPORT_SYMBOL(phy_usb_init);

void xhci_hcd_stop(int index)
{
	if (index == LOTUS_DWC_U3 && lotus_chip_u3_ability())
		lotus_usb3_clk_exit();
	else if ((index == LOTUS_DWC_U2) ||
		(index == LOTUS_DWC_U3 && !lotus_chip_u3_ability()))
		lotus_usb2_clk_exit();
}
EXPORT_SYMBOL(xhci_hcd_stop);

/* Some usb device can't be recognize in U-boot because
 * of error state, so we power off them and wait a delay
 * to restart them, similar to unplug and then insert.
 */
void lotus_usb_pwren_control(void)
{
	u32 reg, default_val;
	int delay __attribute__((unused));

#ifndef CONFIG_USB_DUAL_PORT_SUPPORT
	delay = CONFIG_USB_PWREN_OFF_DELAY ?
		CONFIG_USB_PWREN_OFF_DELAY : 100;
#endif
	reg = readl(MISC_REG_BASE + MISC_USB2_CTRL_REG);
	default_val = reg;
	reg &= ~U2_PWREN_MODE_MASK; /* set PWREN_MISC mode */
	reg |= U2_PWREN_OFF; /* turn off PWREN */
	writel(reg, MISC_REG_BASE + MISC_USB2_CTRL_REG);

#ifndef CONFIG_USB_DUAL_PORT_SUPPORT
	mdelay(delay);
#endif

	reg = readl(MISC_REG_BASE + MISC_USB2_CTRL_REG);
	reg &= ~U2_PWREN_MASK; /* turn on PWREN */
	writel(reg, MISC_REG_BASE + MISC_USB2_CTRL_REG);

	reg = default_val; /* set PWREN_CTRL mode(default val) */
	writel(reg, MISC_REG_BASE + MISC_USB2_CTRL_REG);

	/* U3 CTRL PWREN control */
	reg = readl(MISC_REG_BASE + MISC_USB3_CTRL_REG);
	default_val = reg;
	reg &= ~U3_PWREN_MODE_MASK;
	reg |= U3_PWREN_OFF; /* turn off PWREN */
	writel(reg, MISC_REG_BASE + MISC_USB3_CTRL_REG);
	USB_DEBUG("U3 MISC GCTL(0x1110):0x%X\n",
		readl(MISC_REG_BASE + MISC_USB3_CTRL_REG));

#ifndef CONFIG_USB_DUAL_PORT_SUPPORT
	mdelay(delay);
#endif

	reg = readl(MISC_REG_BASE + MISC_USB3_CTRL_REG);
	reg &= ~U3_PWREN_MASK; /* turn on PWREN */
	writel(reg, MISC_REG_BASE + MISC_USB3_CTRL_REG);
	USB_DEBUG("U3 MISC GCTL(0x1110):0x%X\n",
		readl(MISC_REG_BASE + MISC_USB3_CTRL_REG));

	reg = default_val; /* set PWREN_CTRL mode(default val) */
	writel(reg, MISC_REG_BASE + MISC_USB3_CTRL_REG);
	USB_DEBUG("U3 MISC GCTL(0x1110):0x%X\n",
		readl(MISC_REG_BASE + MISC_USB3_CTRL_REG));
}
