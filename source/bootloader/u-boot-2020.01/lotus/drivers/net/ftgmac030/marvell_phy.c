/*
 * Copyright (c) LOTUS. All rights reserved.
 */
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
#include "marvell_phy.h"

static int m88e1116r_config_init(struct eth_device *dev, int phy_addr)
{
	int temp;
	int err;

	temp = ftgmac030_mdio_read(dev, phy_addr, MII_BMCR);
	temp |= BMCR_RESET;
	err = ftgmac030_mdio_write(dev, phy_addr, MII_BMCR, temp);
	if (err < 0)
		return err;

	mdelay(500);

	err = ftgmac030_mdio_write(dev, phy_addr, MII_MARVELL_PHY_PAGE, 0);
	if (err < 0)
		return err;

	temp = ftgmac030_mdio_read(dev, phy_addr, MII_M1011_PHY_SCR);
	temp |= (7 << 12);	/* max number of gigabit attempts */
	temp |= (1 << 11);	/* enable downshift */
	temp |= MII_M1011_PHY_SCR_AUTO_CROSS;

	debug("(%s) REG_PHY(%d) = 0x%x\n", __func__, MII_M1011_PHY_SCR, temp);

	err = ftgmac030_mdio_write(dev, phy_addr, MII_M1011_PHY_SCR, temp);
	if (err < 0)
		return err;

	err = ftgmac030_mdio_write(dev, phy_addr, MII_MARVELL_PHY_PAGE, 2);
	if (err < 0)
		return err;

	temp = ftgmac030_mdio_read(dev, phy_addr, MII_M1116R_CONTROL_REG_MAC);
	temp &= ~(1 << 5);/*care is bit*/
	temp |= (1 << 4);

	debug("(%s) REG_PHY(%d) = 0x%x\n", __func__, MII_M1116R_CONTROL_REG_MAC, temp);

	err = ftgmac030_mdio_write(dev, phy_addr, MII_M1116R_CONTROL_REG_MAC, temp);
	if (err < 0)
		return err;

	err = ftgmac030_mdio_write(dev, phy_addr, MII_MARVELL_PHY_PAGE, 0);
	if (err < 0)
		return err;

	temp = ftgmac030_mdio_read(dev, phy_addr, MII_BMCR);
	temp |= BMCR_RESET;
	err = ftgmac030_mdio_write(dev, phy_addr, MII_BMCR, temp);
	if (err < 0)
		return err;

	mdelay(500);

	return 0;
}

int marvell_phy_init(struct eth_device *dev, int phy_addr)
{
	return m88e1116r_config_init(dev, phy_addr);
}

