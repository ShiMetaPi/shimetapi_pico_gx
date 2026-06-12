// SPDX-License-Identifier: GPL-2.0
/* Copyright (c) LOTUS. All rights reserved. */

#include <common.h>

#if (CONFIG_AUTO_SD_UPDATE == 1)
#include <mmc.h>
#include "update_comm.h"

int store_mmc_init(void)
{
	struct mmc *mmc;
	unsigned int dev_num = 0;

agin:
	mmc = find_mmc_device(dev_num);
	if (mmc == NULL) {
		printf("No mmc driver found!\n");
		return UP_FAILURE;
	}

	if (!IS_SD(mmc)) {
		dev_num++;
		goto agin;
	}

	if (((unsigned long)mmc->block_dev.vendor[0] == 0) ||
			((unsigned long)mmc->block_dev.product[0] == 0)) {
		printf("*No SD card found!\n");
		return UP_FAILURE;
	}
	return UP_SUCCESS;
}

void store_mmc_deinit(void)
{
}

int mmc_get_target_dev(void)
{
	struct mmc *mmc = NULL;
	int dev_num = 0;

agin:
	mmc = find_mmc_device(dev_num);
	if (mmc == NULL) {
		printf("Error: No mmc driver found!\n");
		return -1;
	}

	if (!IS_SD(mmc)) {
		dev_num++;
		goto agin;
	}

	return dev_num;
}

#endif // CONFIG_AUTO_SD_UPDATE
