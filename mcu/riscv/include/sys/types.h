/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#ifndef __TYPES_H__
#define __TYPES_H__


typedef signed char 		s8;
typedef unsigned char 		u8;
typedef signed short 		s16;
typedef unsigned short 		u16;
typedef signed int 		s32;
typedef unsigned int 		u32;
typedef signed long long 	s64;
typedef unsigned long long 	u64;

typedef signed char 	int8_t;
typedef unsigned char 	uint8_t;
typedef signed short 	int16_t;
typedef unsigned short 	uint16_t;
typedef signed int 	int32_t;
typedef unsigned int 	uint32_t;
typedef long long     	int64_t;
typedef unsigned long long   uint64_t;
typedef long long	intmax_t;
typedef unsigned long long uintmax_t;

typedef unsigned char 	uchar;
typedef unsigned long 	ulong;
typedef unsigned int 	uint;
typedef unsigned char 	uint8;
typedef char	 	int8;
typedef unsigned short 	uint16;
typedef short	 	int16;
typedef unsigned int 	uint32;
typedef int	 	int32;

typedef unsigned long 	uintptr_t;
typedef unsigned int 	size_t;

#define BITS_PER_LONG 32

#undef NULL
#define NULL ((void *)0)

typedef int bool;
#define true    1
#define false   0

#define ERROR   -1
#define OK      0

#define SZ_4M                          0x400000
#define SZ_8M                          0x800000
#define SZ_16M                         0x1000000
#define SZ_128M                        0x8000000
#define SZ_256M                        0x10000000
#define SZ_512M                        0x20000000
#define SZ_750M                        0x30000000
#define SZ_1G                          0x40000000
#define SZ_2G                          0x80000000
#define SZ_3G                          0xc0000000
#define SZ_4G                          0x100000000
#endif

