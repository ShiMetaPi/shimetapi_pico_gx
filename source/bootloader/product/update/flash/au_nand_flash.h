// SPDX-License-Identifier: GPL-2.0
/* Copyright (c) LOTUS. All rights reserved. */

#ifndef __AU_NAND_FLASH_H__
#define __AU_NAND_FLASH_H__

#ifdef CONFIG_CMD_NAND
int au_nand_flash_init(void);
int au_nand_flash_erase(unsigned long offset, unsigned long len);
int au_nand_flash_write(unsigned long offset, unsigned long len, unsigned long lim, unsigned char *buf);
int au_nand_flash_yaffs_write(unsigned long offset, unsigned long len, unsigned long lim, unsigned char *buf);
int au_nand_flash_ubifs_write(unsigned long offset, unsigned long len, unsigned long lim, unsigned char *buf);
int au_nand_flash_get_bad_blkcnt(unsigned long offset, unsigned long len);
int au_nand_flash_get_page_size(int with_oob);
int au_nand_flash_get_erase_size(void);
#else
int au_nand_flash_init(void)
{
	return -1;
}

int au_nand_flash_erase(unsigned long offset, unsigned long len)
{
	return -1;
}

int au_nand_flash_write(unsigned long offset, unsigned long len, unsigned long lim, unsigned char *buf)
{
	return -1;
}

int au_nand_flash_yaffs_write(unsigned long offset, unsigned long len, unsigned long lim, unsigned char *buf)
{
	return -1;
}

int au_nand_flash_ubifs_write(unsigned long offset, unsigned long len, unsigned long lim, unsigned char *buf)
{
	return -1;
}

int au_nand_flash_get_bad_blkcnt(unsigned long offset, unsigned long len)
{
	return 0;
}

int au_nand_flash_get_pages_size(int with_oob)
{
	return 1;
}

int au_nand_flash_get_erase_size(void)
{
	return 1;
}
#endif

#endif
