// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <linux/bitops.h>
#include <phy.h>

#define MIIM_JL11X1_PAGE_SELECT      0x1f

static int jl11x1_phy_extread(struct phy_device *phydev, int addr,
				int devaddr, int regnum)
{
	int oldpage = phy_read(phydev, MDIO_DEVAD_NONE,
			       MIIM_JL11X1_PAGE_SELECT);
	int val;

	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_JL11X1_PAGE_SELECT, devaddr);
	val = phy_read(phydev, MDIO_DEVAD_NONE, regnum);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_JL11X1_PAGE_SELECT, oldpage);

	return val;
}

static int jl11x1_phy_extwrite(struct phy_device *phydev, int addr,
				 int devaddr, int regnum, u16 val)
{
	int oldpage = phy_read(phydev, MDIO_DEVAD_NONE,
			       MIIM_JL11X1_PAGE_SELECT);

	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_JL11X1_PAGE_SELECT, devaddr);
	phy_write(phydev, MDIO_DEVAD_NONE, regnum, val);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_JL11X1_PAGE_SELECT, oldpage);

	return 0;
}

static int jl11x1_config(struct phy_device *phydev)
{
	/* Set green LED for Link, yellow LED for Active */
	phydev->drv->writeext(phydev, 0, 7, 19, 0x00);

	genphy_config_aneg(phydev);

	return 0;
}

int jl11x1_startup(struct phy_device *phydev)
{
	int ret;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	ret = genphy_parse_link(phydev);

	if (phydev->speed == SPEED_100) {
		unsigned int reg;

		reg = phydev->drv->readext(phydev, 0, 24, 24);
		reg &= ~(0x7 << 10);
		reg |= (0x5 << 10);
		phydev->drv->writeext(phydev, 0, 24, 24, reg);
		// set page 7 reg 16 tx 3
		reg = phydev->drv->readext(phydev, 0, 7, 16);
		reg &= ~(0xf << 8);
		reg |= (0x3 << 8);
		phydev->drv->writeext(phydev, 0, 7, 16, reg);
	}

	return ret;
}

/* Support for JL11x1 PHY */
static struct phy_driver JL11x1_driver = {
	.name = "JLSemi JL11x1",
	.uid = 0x937c4024,
	.mask = 0xffffffff,
	.features	= PHY_BASIC_FEATURES,
	.config		= &jl11x1_config,
	.startup	= &jl11x1_startup,
	.shutdown	= &genphy_shutdown,
	.readext	= &jl11x1_phy_extread,
	.writeext	= &jl11x1_phy_extwrite,
};

int phy_jlsemi_init(void)
{
	phy_register(&JL11x1_driver);
	return 0;
}

