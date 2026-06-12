// SPDX-License-Identifier: GPL-2.0
/* Copyright (c) LOTUS. All rights reserved. */

#ifndef __AU_EMMC_FLASH_H__
#define __AU_EMMC_FLASH_H__

#ifdef CONFIG_SUPPORT_EMMC_BOOT
int au_emmc_flash_init(void);
int au_emmc_flash_erase(unsigned long offset, unsigned long len);
int au_emmc_flash_write(unsigned long offset, unsigned long len, unsigned long lim, unsigned char *buf);
int au_emmc_flash_ext4_write(unsigned long offset, unsigned long len, unsigned long lim, unsigned  char *buf);
int au_emmc_flash_get_block_size(void);
#else
int au_emmc_flash_init(void)
{
	return -1;
}

int au_emmc_flash_erase(unsigned long offset, unsigned long len)
{
	return -1;
}
int au_emmc_flash_write(unsigned long offset, unsigned long len, unsigned long lim, unsigned char *buf)
{
	return -1;
}

int au_emmc_flash_ext4_write(unsigned long offset, unsigned long len, unsigned long lim, unsigned  char *buf)
{
	return -1;
}

int au_emmc_flash_get_block_size(void)
{
	return 1;
}
#endif

#endif
