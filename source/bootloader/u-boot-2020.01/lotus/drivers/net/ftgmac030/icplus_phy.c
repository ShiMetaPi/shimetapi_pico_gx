// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <linux/bitops.h>
#include <phy.h>

#define MIIM_ICPLUS_PAGE_SELECT      0x14

static int icplus_phy_extread(struct phy_device *phydev, int addr,
				int devaddr, int regnum)
{
	int oldpage = phy_read(phydev, MDIO_DEVAD_NONE,
			       MIIM_ICPLUS_PAGE_SELECT);
	int val;

	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_ICPLUS_PAGE_SELECT, devaddr);
	val = phy_read(phydev, MDIO_DEVAD_NONE, regnum);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_ICPLUS_PAGE_SELECT, oldpage);

	return val;
}

static int icplus_phy_extwrite(struct phy_device *phydev, int addr,
				 int devaddr, int regnum, u16 val)
{
	int oldpage = phy_read(phydev, MDIO_DEVAD_NONE,
			       MIIM_ICPLUS_PAGE_SELECT);

	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_ICPLUS_PAGE_SELECT, devaddr);
	phy_write(phydev, MDIO_DEVAD_NONE, regnum, val);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_ICPLUS_PAGE_SELECT, oldpage);

	return 0;
}

static int icplus_config(struct phy_device *phydev)
{
	unsigned int reg;

	/* Digital IO Pin Driving Control Register */

	/*RXC DRIVE*/
	reg = phydev->drv->readext(phydev, 0, 4, 22);
	reg &= ~(0x7 << 13);
#ifdef CONFIG_LOTUS_FPGA
	reg |= (0x6 << 13);
#else
	reg |= (0x4 << 13);
#endif
	phydev->drv->writeext(phydev, 0, 4, 22, reg);

	/*RXDx DRIVE*/
	reg = phydev->drv->readext(phydev, 0, 16, 26);
	reg &= ~(0x7FFF);
#ifdef CONFIG_LOTUS_FPGA
	reg |= (0x6DB6);
#else
	reg |= (0x36DB);
#endif
	phydev->drv->writeext(phydev, 0, 16, 26, reg);

	/*RXDx DRIVE*/
	reg = phydev->drv->readext(phydev, 0, 16, 27);
	reg &= ~(0x7FC7);
#ifdef CONFIG_LOTUS_FPGA
	reg |= (0x6D86);
#else
	reg |= (0x2484);
#endif
	phydev->drv->writeext(phydev, 0, 16, 27, reg);

	genphy_config_aneg(phydev);

	return 0;
}

int icplus_startup(struct phy_device *phydev)
{
	int ret;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	return genphy_parse_link(phydev);
}

/* Support for JL11x1 PHY */
static struct phy_driver icplus_driver = {
	.name = "ICPlus IP101A/G",
	.uid = 0x02430c54,
	.mask = 0x0ffffff0,
	.features	= PHY_BASIC_FEATURES,
	.config		= &icplus_config,
	.startup	= &icplus_startup,
	.shutdown	= &genphy_shutdown,
	.readext	= &icplus_phy_extread,
	.writeext	= &icplus_phy_extwrite,
};

int phy_icplus_init(void)
{
	phy_register(&icplus_driver);
	return 0;
}

