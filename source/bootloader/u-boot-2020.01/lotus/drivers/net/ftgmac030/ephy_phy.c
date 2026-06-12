// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <linux/io.h>
#include <linux/bitops.h>
#include <phy.h>

#define MIIM_EPHY_SUB_CFG        0x12
#define MIIM_EPHY_SUB_WDATA      0x13
#define MIIM_EPHY_SUB_RDATA      0x14

#define EPHY_TRIM_REG_BASE      (0x12020000 + 0x8a2c)

typedef union { /* FEPHY TRIM */
	uint32_t all;
	struct {
		uint32_t efuse_tx_100_fs       :4;
		uint32_t efuse_tx_10_outr      :4;
		uint32_t efuse_tx_10_fs        :4;
		uint32_t efuse_rcal_code_rt    :4;
		uint32_t reserved             :16;
	} bitc;
} EPHY_TRIM_REG;


static int ephy_phy_extread(struct phy_device *phydev, int addr,
				int devaddr, int regnum)
{
	int val;
	int sub_cfg;

	sub_cfg = (0<<15) | (devaddr << 8) | (regnum << 0);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_EPHY_SUB_CFG, sub_cfg);

	val = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_EPHY_SUB_RDATA);

	return val;
}

static int ephy_phy_extwrite(struct phy_device *phydev, int addr,
				 int devaddr, int regnum, u16 val)
{
	int sub_cfg;

	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_EPHY_SUB_WDATA, val);

	sub_cfg = (1<<15) | (devaddr << 8) | (regnum << 0);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_EPHY_SUB_CFG, sub_cfg);

	return 0;
}
static int ephy_config(struct phy_device *phydev)
{
	unsigned int reg;
	EPHY_TRIM_REG ephy_trim_reg;

	/* Set green LED for Link, yellow LED for Active */
	//set LED to 0 (page 25 reg 0 3:2:0)
	reg = phydev->drv->readext(phydev, 0, 0x19, 0x00);
	reg &= ~(0x07);
	reg |= 0x03;
	phydev->drv->writeext(phydev, 0, 0x19, 0x00, reg);

	/* Set RG_EPHY_TX_100_LPF fix 4 to Improve the waveform */
	//set RG_EPHY_TX_100_LPF(page 0 reg 33 7:4)
	reg = phydev->drv->readext(phydev, 0, 0x0, 0x21);
	reg &= ~(0xf0);
	reg |= 0x40;
	phydev->drv->writeext(phydev, 0, 0x0, 0x21, reg);

	ephy_trim_reg.all = readl(EPHY_TRIM_REG_BASE);

	pr_info("trim_reg:0x%x\n", ephy_trim_reg.all);
	pr_info("tx_100_fs:0x%x tx_10_outr:0x%x\n", ephy_trim_reg.bitc.efuse_tx_100_fs, ephy_trim_reg.bitc.efuse_tx_10_outr);
	pr_info("tx_10_fs:0x%x rcal_code:0x%x\n", ephy_trim_reg.bitc.efuse_tx_10_fs, ephy_trim_reg.bitc.efuse_rcal_code_rt);


	if (ephy_trim_reg.all) {
		//set EPHY_TX_10_OUTR to 0 (page 0 reg 9 11:8)
		reg = phydev->drv->readext(phydev, 0, 0, 0x09);
		reg &= ~(0xf << 8);
		reg |= (ephy_trim_reg.bitc.efuse_tx_10_outr << 8);
		phydev->drv->writeext(phydev, 0, 0, 0x09, reg);

		//set EPHY_TX_10_FS to 0 (page 0 reg 9 7:4)
		reg = phydev->drv->readext(phydev, 0, 0, 0x09);
		reg &= ~(0xf << 4);
		reg |= (ephy_trim_reg.bitc.efuse_tx_10_fs << 4);
		phydev->drv->writeext(phydev, 0, 0, 0x09, reg);

		//set EPHY_TX_100_FS to 4 (page 0 reg 33 3:0)
		reg = phydev->drv->readext(phydev, 0, 0, 0x21);
		reg &= ~(0xf << 0);
		reg |= (ephy_trim_reg.bitc.efuse_tx_100_fs << 0);
		phydev->drv->writeext(phydev, 0, 0, 0x21, reg);

		// set EPHY_RCAL_CODE_FT to 8 (page 0 reg 34 7:4)
		reg = phydev->drv->readext(phydev, 0, 0, 0x22);
		reg &= ~(0xf << 4);
		reg |= (ephy_trim_reg.bitc.efuse_rcal_code_rt << 4);
		phydev->drv->writeext(phydev, 0, 0, 0x22, reg);
	}

	genphy_config_aneg(phydev);

	return 0;
}

/* Support for ePHY PHY */
static struct phy_driver ephy_driver = {
	.name = "ePHY",
	.uid = 0x0243991f,
	.mask = 0xffffffff,
	.features	= PHY_BASIC_FEATURES,
	.config		= &ephy_config,
	.startup	= &genphy_startup,
	.shutdown	= &genphy_shutdown,
	.readext	= &ephy_phy_extread,
	.writeext	= &ephy_phy_extwrite,
};

int phy_ephy_init(void)
{
	phy_register(&ephy_driver);
	return 0;
}

