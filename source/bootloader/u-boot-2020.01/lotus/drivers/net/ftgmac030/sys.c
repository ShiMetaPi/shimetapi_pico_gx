// SPDX-License-Identifier: GPL-2.0+

#include <config.h>
#include "bspeth.h"
#include "sys.h"
#include "ftgmac030.h"

static void bspeth_reset(int rst)
{
	u32 val;

	val = _readl(BSPETH_MAC_CRG_REG);
	if (rst)
		val |= ETH_SOFT_RESET;
	else
		val &= ~ETH_SOFT_RESET;

	_writel(val, BSPETH_MAC_CRG_REG);

	udelay(100); /* delay 100us */

}

static inline void bspeth_clk_ena(void)
{

}

static inline void bspeth_clk_dis(void)
{

}

static void bspeth_reset_internal_phy(void)
{

}

static void bspeth_reset_external_phy_by_crg(void)
{
	u32 v;
	u32 phy_reset_n;

	/* bit[1]*/
	phy_reset_n = readl(BSPETH_CFG_CRG_REG);
	phy_reset_n &= ETH_PHY_RST_N;

	/************************************************/
	/* reset external phy with default reset pin */
	v = readl(BSPETH_PHY_CRG_REG);

	if (phy_reset_n)
		v |= ETH_EXTERNAL_PHY_RESET;
	else
		v &= ~ETH_EXTERNAL_PHY_RESET;

	writel(v, BSPETH_PHY_CRG_REG);
	mdelay(10); /* delay 10ms */

	/* then, cancel reset, and should delay some time */
	v = readl(BSPETH_PHY_CRG_REG);

	if (phy_reset_n)
		v &= ~ETH_EXTERNAL_PHY_RESET;
	else
		v |= ETH_EXTERNAL_PHY_RESET;

	writel(v, BSPETH_PHY_CRG_REG);

	mdelay(20); /* delay 20ms */

	/* then, cancel reset, and should delay some time */
	v = readl(BSPETH_PHY_CRG_REG);

	if (phy_reset_n)
		v |= ETH_EXTERNAL_PHY_RESET;
	else
		v &= ~ETH_EXTERNAL_PHY_RESET;

	writel(v, BSPETH_PHY_CRG_REG);

	mdelay(150); /* delay 150ms */

}

static void bspeth_reset_external_phy_by_gpio(void)
{

}

static void bspeth_phy_reset(void)
{
	bspeth_reset_internal_phy();
	bspeth_reset_external_phy_by_crg();
	bspeth_reset_external_phy_by_gpio();
}

static void bspeth_funsel_config(void)
{

}

static void bspeth_funsel_restore(void)
{

}


///**************************************************/
//void bspeth_sys_startup(void)
//{
//	bspeth_clk_ena();
//	/* undo reset */
//	bspeth_reset(0);
//}

void bspeth_sys_allstop(void)
{

}

u32 bspeth_get_phy_intf(void)
{
	u32 phy_intf;

	/* bit[0]: 0 -- rmii，1 -- rgmii bit[3]:1 --mii or gmii*/
	phy_intf = readl(BSPETH_CFG_CRG_REG);
	phy_intf &= ETH_PHY_INTF;

	if (phy_intf & bit(3))
		return FTGMAC030_MODE_MII_GMII;
	else if (phy_intf & bit(0))
		return FTGMAC030_MODE_RGMII;
	else
		return FTGMAC030_MODE_RMII;
}

/* Set rgmii tx delay for 2ns */
void bspeth_set_tx_delay(void)
{
	u32 regval;
	u32 timeout = 100;//1ms

	regval = readl(BSPETH_DLY_CRG_REG);

	if(!(regval & ETH_DLY_SEL)) {
		writel(regval | ETH_DLY_SEL, BSPETH_DLY_CRG_REG);

		do {
			udelay(10);
			regval = readl(BSPETH_DLY_STAT_REG);
			--timeout;
		} while ((!(regval & ETH_DLY_STAT) && timeout));
	}

	if(!timeout)
		printf("Error: bspeth_set_tx_delay timeout!\n");
}

void bspeth_sys_init(void)
{
	bspeth_funsel_config();
	bspeth_sys_allstop();
	bspeth_clk_ena();
	bspeth_reset(1);
	bspeth_reset(0);
	bspeth_phy_reset();
}

void bspeth_sys_exit(void)
{
	bspeth_funsel_restore();
	bspeth_sys_allstop();
}
