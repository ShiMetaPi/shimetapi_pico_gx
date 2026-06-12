// SPDX-License-Identifier: GPL-2.0
/* Copyright (c) LOTUS. All rights reserved. */

#include <common.h>

#if (CONFIG_AUTO_UPDATE == 1)
#include <nand.h>
#include "update_comm.h"

#define PERCENT_VALUE		100
#define NAND_SUCCESS UP_SUCCESS
#define NAND_FAILURE UP_FAILURE

static struct mtd_info *g_nand_flash = NULL;

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
		if (percent != percent_complete)
			printf("\rOperation at 0x%lx -- %3d%% complete.", offset, percent);
	} while (0);
}

int au_nand_flash_init(void)
{
	g_nand_flash = nand_info[0];
	if (g_nand_flash == NULL) {
		printf("nand_info() fail.\n");
		return NAND_FAILURE;
	}

	return NAND_SUCCESS;
}

int au_nand_flash_erase(unsigned long offset, unsigned long len)
{
	int ret;
	unsigned long erase_len;
	unsigned long erase_step;
	unsigned long length;
	nand_erase_options_t opts;
	struct mtd_info *nand_flash = g_nand_flash;

	if (nand_flash == NULL) {
		printf("%s: Invalid param.\n", __func__);
		return NAND_FAILURE;
	}

	memset(&opts, 0, sizeof(opts));

	length = len;
	erase_step = nand_flash->erasesize;
	erase_len = length;
	opts.length  = erase_step;
	opts.offset = offset;
	opts.quiet = 1;

	while (length > 0) {
		if (length < erase_step)
			erase_step = length;

		ret = nand_erase_opts(nand_flash, &opts);
		if (ret) {
			printf("nand_erase_opts() fail.\n");
			return NAND_FAILURE;
		}

		length -= erase_step;
		opts.offset += erase_step;
		/* notify real time schedule */
		schedule_notify(opts.offset, erase_len, offset);
	}
	printf("\n");

	return NAND_SUCCESS;
}

int au_nand_flash_write(unsigned long offset, unsigned long len, unsigned long lim, unsigned char *buf)
{
	int ret;
	unsigned long offset_notify;
	unsigned long write_start;
	unsigned long write_len;
	unsigned long write_step;
	size_t length;
	unsigned char *pbuf = buf;
	struct mtd_info *nand_flash = g_nand_flash;

	if (nand_flash == NULL) {
		printf("%s: Invalid param.\n", __func__);
		return NAND_FAILURE;
	}

	offset = offset & (nand_flash->writesize - 1) ? (size_t)(offset +
			(nand_flash->writesize - offset % nand_flash->writesize)) : offset;

	length = len & (nand_flash->erasesize - 1) ? (size_t)(len +
			(nand_flash->erasesize - len % nand_flash->erasesize)) : len;

	write_step  = length;
	write_start = offset;
	offset_notify = offset;
	write_len   = length;

	while (length > 0) {
		size_t rw_size = write_step;

		ret = nand_write_skip_bad(nand_flash, (size_t)offset, &rw_size, NULL, lim, (u_char *)pbuf, 0);
		if (ret) {
			printf("NAND write to offset %lx failed %d\n", offset, ret);
			return NAND_FAILURE;
		}

		offset += write_step;
		pbuf   += write_step;
		length -= write_step;
		offset_notify += write_step;
		/* notify real time schedule */
		schedule_notify(offset_notify, write_len, write_start);
	}
	printf("\n");

	return NAND_SUCCESS;
}

int au_nand_flash_yaffs_write(unsigned long offset, unsigned long len, unsigned long lim, unsigned char *buf)
{
	int ret;
	size_t rw_size = len;
	struct mtd_info *nand_flash = g_nand_flash;

	if (nand_flash == NULL) {
		printf("%s: Invalid param.\n", __func__);
		return NAND_FAILURE;
	}

	ret = nand_write_yaffs_skip_bad(nand_flash, offset, &rw_size, (u_char *)buf);
	if (ret) {
		printf("Write yaffs fail !!\n");
		return NAND_FAILURE;
	}

	printf("Write yaffs done\n");

	return NAND_SUCCESS;
}

int au_nand_flash_ubifs_write(unsigned long offset, unsigned long len, unsigned long lim, unsigned char *buf)
{
	int ret;

	ret = au_nand_flash_write(offset, len, lim, buf);

	printf("Write ubifs done\n");

	return ret;
}

/* get count of area's bad block for nand flash */
int au_nand_flash_get_bad_blkcnt(unsigned long offset, unsigned long len)
{
	int count = 0;
	unsigned long block_offset = 0;
	struct mtd_info *nand_flash = g_nand_flash;

	if (nand_flash == NULL) {
		printf("%s: Invalid param.\n", __func__);
		return NAND_FAILURE;
	}

	if (offset & (nand_flash->erasesize - 1))
		block_offset = offset & (nand_flash->erasesize - 1);

	if (len & (nand_flash->erasesize - 1))
		len = ALIGN(len, nand_flash->erasesize);

	for (int i = 0; i < len / nand_flash->erasesize; i++) {
		if (nand_block_isbad(nand_flash, offset))
			count++;
		offset += nand_flash->erasesize - block_offset;
	}

	return count;
}

int au_nand_flash_get_page_size(int with_oob)
{
	struct mtd_info *nand_flash = g_nand_flash;

	if (with_oob) {
		return (nand_flash->writesize + nand_flash->oobsize);
	} else {
		return (nand_flash->writesize);
	}
}

int au_nand_flash_get_erase_size(void)
{
	struct mtd_info *nand_flash = g_nand_flash;

	return nand_flash->erasesize;
}

#endif // CONFIG_AUTO_UPDATE
