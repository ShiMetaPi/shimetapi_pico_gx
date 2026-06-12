// SPDX-License-Identifier: GPL-2.0

//#define DEBUG
#include <config.h>
#include <common.h>
#include <malloc.h>
#include <net.h>
#include <wait_bit.h>
#include <linux/io.h>
#include <linux/mii.h>
#include <linux/iopoll.h>
#include <cpu_func.h>
#include <miiphy.h>
#include <hexdump.h>

#include "ftgmac030.h"
#include "realtek_phy.h"

static int rtl821x_write_page(struct eth_device *dev, int phy_addr, int page)
{
	return ftgmac030_mdio_write(dev, phy_addr, RTL821x_PAGE_SELECT, page);
}

static int rtl8211f_config_init(struct eth_device *dev, int phy_addr)
{
	int temp;
	int err;

	/* change page to 0xa43 */
	err = rtl821x_write_page(dev, phy_addr, 0xa43);
	if (err < 0)
		return err;

	/* config  page 0xa43 offset  0x18 */
	temp = ftgmac030_mdio_read(dev, phy_addr, RTL8211F_PHYCR1);
	temp |= RTL8211F_ALDPS_ENABLE | RTL8211F_ALDPS_PLL_OFF | RTL8211F_ALDPS_XTAL_OFF;

	debug("(%s) REG_PHY(%d) = 0x%x\n", __func__, RTL8211F_PHYCR1, temp);

	err = ftgmac030_mdio_write(dev, phy_addr, RTL8211F_PHYCR1, temp);
	if (err < 0)
		return err;

	/* change page to 0xd08 */
	err = rtl821x_write_page(dev, phy_addr, 0xd08);
		if (err < 0)
			return err;

	/* config  page 0xd08 offset  0x11 */
	temp = ftgmac030_mdio_read(dev, phy_addr, 0x11);

	temp |= RTL8211F_TX_DELAY;
	debug("(%s) REG_PHY(%d) = 0x%x\n", __func__, 0x11, temp);

	err = ftgmac030_mdio_write(dev, phy_addr, 0x11, temp);
	if (err < 0)
		return err;

	/* config  page 0xd08 offset  0x15 */
	temp = ftgmac030_mdio_read(dev, phy_addr, 0x15);

	temp |= RTL8211F_RX_DELAY;
	debug("(%s) REG_PHY(%d) = 0x%x\n", __func__, 0x15, temp);

	err = ftgmac030_mdio_write(dev, phy_addr, 0x15, temp);
		if (err < 0)
			return err;

	/* change page to 0x0 */
	err = rtl821x_write_page(dev, phy_addr, 0);
	if (err < 0)
		return err;

	return 0;
}

int  realtek_phy_init(struct eth_device *dev, int phy_addr)
{
	int err = 0;
	u16 phy_id;

	phy_id = ftgmac030_mdio_read(dev, phy_addr, MII_PHYSID2);

	debug("(%s) REG_PHY(%d) = 0x%x\n", __func__, MII_PHYSID2, phy_id);

	switch (phy_id) {
	case PHY_RTL8211F:
		err = rtl8211f_config_init(dev, phy_addr);
		break;
	default:
		break;
	}

	return err;
}

