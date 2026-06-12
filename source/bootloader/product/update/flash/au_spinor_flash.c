// SPDX-License-Identifier: GPL-2.0
/* Copyright (c) LOTUS. All rights reserved. */

#include <common.h>

#if (CONFIG_AUTO_UPDATE == 1)
#include <spi_flash.h>
#include "update_comm.h"

#define PERCENT_VALUE		100
#define SPINOR_SUCCESS  UP_SUCCESS
#define SPINOR_FAILURE  UP_FAILURE

static struct spi_flash *g_spinor_flash;

static void schedule_notify(unsigned long offset, unsigned long len, unsigned long off_start)
{
	int percent_complete = -1;
	unsigned long long n;

	do {
		n = (unsigned long long)(offset - off_start) * PERCENT_VALUE;
		int percent;

		do_div(n, len);
		percent = (int)n;

		/* output progress message only at whole percent
		 * steps to reduce the number of messages
		 * printed on (slow) serial consoles
		 */
		if (percent != percent_complete) {
			printf("\rOperation at 0x%lx -- %3d%% complete.", offset, percent);
		}
	} while (0);
}

int au_spinor_flash_init(void)
{
	g_spinor_flash = spi_flash_probe(0, 0, 0, 0);
	if (g_spinor_flash == NULL) {
		printf("spi_flash_probe fail.\n");
		return SPINOR_FAILURE;
	};

	return SPINOR_SUCCESS;
}

int au_spinor_flash_erase(unsigned long offset, unsigned long len)
{
	int ret = SPINOR_SUCCESS;
	struct spi_flash *flash = g_spinor_flash;
	struct mtd_info_ex *spiflash_info = get_spiflash_info();
	unsigned long erase_start, erase_len, erase_step;

	if (flash == NULL) {
		printf("%s: Invalid param.\n", __func__);
		return SPINOR_FAILURE;
	}

	erase_start = offset;
	erase_len   = len;
	erase_step  = spiflash_info->erasesize;

	while (len > 0) {
		if (len < erase_step)
			erase_step = len;
#if (UBOOTVERSION == 202001)
		ret = spi_flash_erase(flash, (u32)offset, erase_step);
#elif (UBOOTVERSION == 201611)
		ret = flash->erase(flash, (u32)offset, erase_step);
#else
		printf("%s:invalid uboot version!\n", __func__);
		return SPINOR_FAILURE;
#endif
		if (ret) {
			printf("%s: erase fail.\n", __func__);
			return SPINOR_FAILURE;
		}

		len -= erase_step;
		offset += erase_step;
		/* notify real time schedule */
		schedule_notify(offset, erase_len, erase_start);
	}
	printf("\n");

	return SPINOR_SUCCESS;
}

int au_spinor_flash_write(unsigned long offset, unsigned len, unsigned lim, unsigned char* buf)
{
	int ret;
	struct spi_flash *flash = g_spinor_flash;
	unsigned long write_start, write_len, write_step;
	unsigned char *pbuf = buf;
	struct mtd_info_ex *spiflash_info = get_spiflash_info();

	if (flash == NULL) {
		printf("%s: Invalid param.\n", __func__);
		return SPINOR_FAILURE;
	}

	write_start = offset;
	write_len   = len;
	write_step  = spiflash_info->erasesize;

	while (len > 0) {
		if (len < write_step)
			write_step = len;

		ret = flash->write(flash, offset, write_step, pbuf);
		if (ret) {
			printf("SPINOR write to offset %lx failed %d\n", offset, ret);
			return SPINOR_FAILURE;
		}

		offset += write_step;
		pbuf   += write_step;
		len    -= write_step;
		/* notify real time schedule */
		schedule_notify(offset, write_len, write_start);
	}
	printf("\n");

	return SPINOR_SUCCESS;
}

int au_spinor_flash_jffs_write(unsigned long offset, unsigned len, unsigned lim, unsigned char* buf)
{
	int ret;
	struct spi_flash *flash = g_spinor_flash;
	unsigned long write_start, write_len, write_step;
	unsigned char *pbuf = buf;
	struct mtd_info_ex *spiflash_info = get_spiflash_info();

	if (flash == NULL) {
		printf("%s: Invalid param.\n", __func__);
		return SPINOR_FAILURE;
	}

	write_start = offset;
	write_len   = len;
	write_step  = spiflash_info->erasesize;

	while (len > 0) {
		if (len < write_step)
			write_step = len;

		ret = flash->write(flash, offset, write_step, pbuf);
		if (ret) {
			printf("SPINOR write to offset %lx failed %d\n", offset, ret);
			return SPINOR_FAILURE;
		}

		offset += write_step;
		pbuf   += write_step;
		len    -= write_step;
		/* notify real time schedule */
		schedule_notify(offset, write_len, write_start);
	}
	printf("\n");

	return SPINOR_SUCCESS;
}

int au_spinor_flash_ubifs_write(unsigned long offset, unsigned len, unsigned lim, unsigned char* buf)
{
	int ret;
	struct spi_flash *flash = g_spinor_flash;
	unsigned long write_start, write_len, write_step;
	unsigned char *pbuf = buf;
	struct mtd_info_ex *spiflash_info = get_spiflash_info();

	if (flash == NULL) {
		printf("%s: Invalid param.\n", __func__);
		return SPINOR_FAILURE;
	}

	write_start = offset;
	write_len   = len;
	write_step  = spiflash_info->erasesize;

	while (len > 0) {
		if (len < write_step)
			write_step = len;

		ret = flash->write(flash, offset, write_step, pbuf);
		if (ret) {
			printf("SPINOR write to offset %lx failed %d\n", offset, ret);
			return SPINOR_FAILURE;
		}

		offset += write_step;
		pbuf   += write_step;
		len    -= write_step;
		/* notify real time schedule */
		schedule_notify(offset, write_len, write_start);
	}
	printf("\n");

	return SPINOR_SUCCESS;
}

#endif
