/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
/* linux /dev/mem mmap support head file					*/
/* 															*/
/* 															*/
/* Copyright 2021 CJiang com.                             */
/* Create date: 2021-01-07									*/
/* Modify bspstory											*/
/*                                                          */

#ifndef __MEM_MAP_H__
#define __MEM_MAP_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern void * memmap(unsigned long phy_addr, unsigned long size);
extern int memunmap(void * addr_mapped);
extern void * mmapfile(char* fn, unsigned int size);
extern void * mmapfile_r(char* fn, unsigned int size);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif

