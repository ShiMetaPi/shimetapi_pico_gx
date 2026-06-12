/*
 * Copyright (c) XMEDIA 2021. All rights reserved.
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_
/******************************************************************************/
#include <config.h>
#include <stdio.h>
#include <lib.h>

/* #define ASSERT(_p) if (!(_p)) { \
 *         printf("%s(%s,%d):assert:(%s)\n", __FILE__, __FUNCTION__, __LINE__, #_p);
 *
 * #define ASSERT1(_p, _fmt, args...) if (!(_p)) { \
 *         printf("%s(%s,%d):assert:(%s)\n" _fmt, __FILE__, __FUNCTION__, __LINE__, #_p, ##args); */

#define ASSERT(_condition) do { \
	if (!(_condition)) { \
		puts("ASSERT "); \
		putchar('['); \
		puts(__FILE__); \
		putchar(','); \
		putdec(__LINE__); \
		puts("]: "); \
		puts(#_condition); \
		puts("\n"); \
	} \
} while(0)

#define __PRINT_MACRO(x) #x
#define PRINT_MARCO(x) #x"="__PRINT_MACRO(x)
/* #pragma message(PRINT_MARCO(MACRO_NAME_XXX)) */

int dump_hex(u32 addr, char *buf, u32 sz_buf, u32 width);

/******************************************************************************/
#endif /* _DEBUG_H_ */
