// SPDX-License-Identifier: GPL-2.0
/* Copyright (c) LOTUS. All rights reserved. */

#ifndef __AU_SPINORE_FLASH_H__
#define __AU_SPINORE_FLASH_H__

#ifdef CONFIG_CMD_SF
int au_spinor_flash_init(void);
int au_spinor_flash_erase(unsigned long offset, unsigned long len);
int au_spinor_flash_write(unsigned long offset, unsigned len, unsigned lim, unsigned char* buf);
int au_spinor_flash_jffs_write(unsigned long offset, unsigned len, unsigned lim, unsigned char* buf);
int au_spinor_flash_ubifs_write(unsigned long offset, unsigned len, unsigned lim, unsigned char* buf);
#else
int au_spinor_flash_init(void)
{
	return -1;
}

int au_spinor_flash_erase(unsigned long offset, unsigned long len)
{
	return -1;
}

int au_spinor_flash_write(unsigned long offset, unsigned len, unsigned lim, unsigned char* buf)
{
	return -1;
}

int au_spinor_flash_jffs_write(unsigned long offset, unsigned len, unsigned lim, unsigned char* buf)
{
	return -1;
}

int au_spinor_flash_ubifs_write(unsigned long offset, unsigned len, unsigned lim, unsigned char* buf)
{
	return -1;
}
#endif

#endif
