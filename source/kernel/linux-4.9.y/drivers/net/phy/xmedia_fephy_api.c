/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#include <linux/module.h>
#include <linux/xmedia/fephy.h>
#include <linux/xmedia/soc_xm720xxx_chip.h>
#include <mach/platform.h>
#include "xmedia_fephy_api.h"

struct xmedia_fephy_private *xm_fephy_data = NULL;
void __fephy_set_private_date(struct xmedia_fephy_private *xm_data)
{
	if (xm_fephy_data) {
		pr_err("FE PHY private data has been inited.");
		return;
	}
	xm_fephy_data = xm_data;
}
EXPORT_SYMBOL(__fephy_set_private_date);

static bool is_net_support(void)
{
	unsigned int chipid = 0;

	chipid = get_chipid();
	if ((chipid == CHIPID_XM72020330) || (chipid == CHIPID_XM72020300))
		return false;
	else
		return true;
}

unsigned int fephy_get_fw_state(void)
{
	if (!is_net_support())
		return 0;

	if (!xm_fephy_data) {
		pr_err("FE PHY private data should be inited first!\n");
		return 0;
	}
	return xm_fephy_data->get_fw_state();
}
EXPORT_SYMBOL(fephy_get_fw_state);

void fephy_mask_fw_state(void)
{
	if (!is_net_support())
		return;

	if (!xm_fephy_data) {
		pr_err("FE PHY private data should be inited first!\n");
		return;
	}
	xm_fephy_data->mask_fw_state();	
}
EXPORT_SYMBOL(fephy_mask_fw_state);

void fephy_update(void)
{
	if (!is_net_support())
		return;

	if (!xm_fephy_data) {
		pr_err("FE PHY private data should be inited first!\n");
		return;
	}
	xm_fephy_data->fephy_update();	
}
EXPORT_SYMBOL(fephy_update);

unsigned int fephy_get_compat_state(void)
{
	if (!is_net_support())
		return 0;

	if (!xm_fephy_data) {
		pr_err("FE PHY private data should be inited first!\n");
		return 0;
	}

	return xm_fephy_data->compat_state;
}
EXPORT_SYMBOL(fephy_get_compat_state);

unsigned int fephy_init_state(void)
{
	if (!is_net_support())
		return 0;

	if (!xm_fephy_data) {
		pr_err("FE PHY private data should be inited first!\n");
		return 0;
	}
	return xm_fephy_data->fephy_init_state;
}
EXPORT_SYMBOL(fephy_init_state);
