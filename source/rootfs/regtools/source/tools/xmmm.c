/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memmap.h"
#include "bsp.h"
#include "strfunc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define DEFAULT_MD_LEN 128

XMEDIA_RET xmmm(int argc , char* argv[])
{
	unsigned long ulAddr = 0;
	unsigned long ulOld, ulNew;
	char strNew[16];
	VOID* pMem  = NULL;

	if (argc <= 1)
	{
		printf("usage: %s <address> <Value>. sample: %s 0x80040000 0x123\n", argv[0], argv[0]);
		EXIT("", -1);
	}

	if (argc == 2)
	{
		if( StrToNumber(argv[1], &ulAddr) == XMEDIA_SUCCESS)
		{
			printf("====dump memory %#lX====\n", ulAddr);
#ifdef PC_EMULATOR
#define SHAREFILE "../shm"
			printf("**** is Emulator, use share file : %s ****\n", SHAREFILE);
			pMem = mmapfile(SHAREFILE , DEFAULT_MD_LEN);
			if (NULL == pMem)
			{
				EXIT("Memory Map error.", -1);
			}

			pMem += ulAddr;
#else
			pMem = memmap(ulAddr, DEFAULT_MD_LEN);
			if (pMem == NULL) {
				printf("memmap failed!\n");
				return -1;
			}
#endif

			ulOld = *(unsigned int*)pMem;
            /*bsp_hexdump(STDOUT, pMem, DEFAULT_MD_LEN, 16);*/
			printf("%s: 0x%08lX\n", argv[1], ulOld);
			printf("NewValue:");
			scanf("%s", strNew);

			if (StrToNumber(strNew, &ulNew) == XMEDIA_SUCCESS)
			{
				*(unsigned int*)pMem = ulNew;
			}
			else
			{
				printf("Input Error\n");
			}

		}
		else
		{
			printf("Please input address like 0x12345\n");
		}
	}
	else if (argc == 3)
	{
		if( StrToNumber(argv[1], &ulAddr) == XMEDIA_SUCCESS &&
				StrToNumber(argv[2], &ulNew) == XMEDIA_SUCCESS)
		{
			pMem = memmap(ulAddr, DEFAULT_MD_LEN);
			if (pMem == NULL) {
				printf("memmap failed!\n");
				return -1;
			}
			ulOld = *(unsigned int*)pMem;
			/*bsp_hexdump(STDOUT, pMem, DEFAULT_MD_LEN, 16);*/
			printf("%s: 0x%08lX --> 0x%08lX \n", argv[1], ulOld, ulNew);
			*(unsigned int*)pMem = ulNew;
		}
	}
	else
	{
		printf("xxx\n");
	}
	return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

