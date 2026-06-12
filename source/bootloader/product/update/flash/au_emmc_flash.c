// SPDX-License-Identifier: GPL-2.0
/* Copyright (c) LOTUS. All rights reserved. */

#include <common.h>

#if (CONFIG_AUTO_UPDATE == 1)
#include <mmc.h>
#include <sparse_format.h>
#include <unsparse.h>

#include "update_comm.h"

#define EMMC_SUCCESS UP_SUCCESS
#define EMMC_FAILURE UP_FAILURE

#define EMMC_BLOCK_SHIFT	9
#define EMMC_BLOCK_SIZE     (1 << EMMC_BLOCK_SHIFT)

int au_emmc_flash_init(void)
{
	struct mmc *mmc = find_mmc_device(0);

	if (!mmc) {
		printf("%s:find mmc device failed\n", __func__);
		return EMMC_FAILURE;
	}

	(void)mmc_init(mmc);

	return EMMC_SUCCESS;
}

int au_emmc_flash_erase(unsigned long offset, unsigned long len)
{
#if 0
	struct mmc *mmc = find_mmc_device(0);

	if (!mmc) {
		printf("%s:find mmc device failed\n", __func__);
		return EMMC_FAILURE;
	}

	if (len % MMC_MAX_BLOCK_LEN) {
		blk_derase(mmc_get_blk_desc(mmc), (offset >> EMMC_BLOCK_SHIFT),
				(len >> EMMC_BLOCK_SHIFT) + 1);
	} else {
		blk_derase(mmc_get_blk_desc(mmc), (offset >> EMMC_BLOCK_SHIFT),
				(len >> EMMC_BLOCK_SHIFT));
	}
#endif
	return EMMC_SUCCESS;
}

int au_emmc_flash_write(unsigned long offset, unsigned long len,
		unsigned long lim, unsigned char *buf)
{
	struct mmc *mmc = find_mmc_device(0);

	if (!mmc) {
		printf("%s:find mmc device failed\n", __func__);
		return EMMC_FAILURE;
	}

	if (len % MMC_MAX_BLOCK_LEN) {
		blk_dwrite(mmc_get_blk_desc(mmc), (offset >> EMMC_BLOCK_SHIFT),
				(len >> EMMC_BLOCK_SHIFT) + 1, buf);
	} else {
		blk_dwrite(mmc_get_blk_desc(mmc), (offset >> EMMC_BLOCK_SHIFT),
				(len >> EMMC_BLOCK_SHIFT), buf);
	}

	return EMMC_SUCCESS;
}

int au_emmc_flash_ext4_write(unsigned long offset, unsigned long len,
		unsigned long lim, unsigned  char *buf)
{
	struct mmc *mmc = find_mmc_device(0);
	int retlen;

	if (!mmc) {
		printf("%s:find mmc device failed\n", __func__);
		return EMMC_FAILURE;
	}

	if (len % MMC_MAX_BLOCK_LEN)
		retlen = ext4_unsparse(mmc, 0, buf,
				(offset >> EMMC_BLOCK_SHIFT),
				(len >> EMMC_BLOCK_SHIFT) + 1);
	else
		retlen = ext4_unsparse(mmc, 0, buf,
				(offset >> EMMC_BLOCK_SHIFT),
				(len >> EMMC_BLOCK_SHIFT));
	return retlen;
}

int au_emmc_flash_get_block_size(void)
{
	return (int)EMMC_BLOCK_SIZE;
}
#endif // CONFIG_AUTO_UPDATE
