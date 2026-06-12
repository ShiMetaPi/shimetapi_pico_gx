/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "memmap.h"
#include "bsp.h"
#include "strfunc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define DEFAULT_MD_LEN 256

XMEDIA_RET xmmd_l(int argc , char* argv[])
{
	unsigned long ulAddr = 0;
	VOID* pMem  = NULL;
	LENGTH_T len;

	if (argc < 2)
	{
		printf("usage: %s <address> [length]. sample: %s 0x80040000 0x100\n", argv[0], argv[0]);
		EXIT("", -1);
	}


	if (argc == 3)
	{
		if ( StrToNumber(argv[2], &len) != XMEDIA_SUCCESS)
		{
			len = DEFAULT_MD_LEN;
		}
	}
	else
	{
		len = DEFAULT_MD_LEN;
	}


	if( StrToNumber(argv[1], &ulAddr) == XMEDIA_SUCCESS)
	{
		printf("====dump memory 0x%08lX====\n", ulAddr);
#ifdef PC_EMULATOR
#define SHAREFILE "../shm"
		printf("**** is Emulator, use share file : %s ****\n", SHAREFILE);
		pMem = mmapfile(SHAREFILE , len);
		if (NULL == pMem)
		{
			EXIT("Memory Map error.", -1);
		}
		pMem += ulAddr;
#else
		pMem = memmap(ulAddr, len);
		if (NULL == pMem)
		{
			EXIT("Memory Map error.", -1);
		}
#endif

		bsp_hexdump2(STDOUT, pMem, len, 16);
	}
	else
	{
		printf("Please input address like 0x12345\n");
	}

	return 0;
}

XMEDIA_RET xmmd(int argc , char* argv[])
{
	unsigned long ulAddr = 0;
	VOID* pMem  = NULL;
	LENGTH_T len;

	if (argc < 2)
	{
		printf("usage: %s <address>. sample: %s 0x80040000\n", argv[0], argv[0]);
		EXIT("", -1);
	}


	if (argc == 3)
	{
		if ( StrToNumber(argv[2], &len) != XMEDIA_SUCCESS)
		{
			len = DEFAULT_MD_LEN;
		}
	}
	else
	{
		len = DEFAULT_MD_LEN;
	}


	if( StrToNumber(argv[1], &ulAddr) == XMEDIA_SUCCESS)
	{
		printf("====dump memory %#010lX====\n", ulAddr);
#ifdef PC_EMULATOR
#define SHAREFILE "../shm"
		printf("**** is Emulator, use share file : %s ****\n", SHAREFILE);
		pMem = mmapfile(SHAREFILE , len);
		if (NULL == pMem)
		{
			EXIT("Memory Map error.", -1);
		}
		pMem += ulAddr;
#else
		pMem = memmap(ulAddr, len);
		if (NULL == pMem)
		{
			EXIT("Memory Map error.", -1);
		}
#endif

		bsp_hexdump(STDOUT, pMem, len, 16);
	}
	else
	{
		printf("Please input address like 0x12345\n");
	}

	return 0;
}

/*memory dump bin*/
XMEDIA_RET xmmdb(int argc , char* argv[])
{
	int retval = 0;
	unsigned long ulAddr = 0;
	VOID* pMem  = NULL;
	VOID* pBase = NULL;
	LENGTH_T len = DEFAULT_MD_LEN;

	FILENAME_T fn;

	memset(fn, 0, MAX_FILENAME_LENGTH);

	if (argc >= 3)
	{
		if ( StrToNumber(argv[2], &len) != XMEDIA_SUCCESS)
		{
			len = DEFAULT_MD_LEN;
		}

		if (argc == 4)
		{
            if (strlen(argv[3]) >= MAX_FILENAME_LENGTH) {
                printf("filename's length is %ld, wbsp.h should less than %d\n",
                        strlen(argv[3]), MAX_FILENAME_LENGTH);
                EXIT("", -1);
            }
            strncpy(fn, argv[3], MAX_FILENAME_LENGTH - 1);
		}
		else
		{
            if (strlen(argv[1]) + strlen(argv[2]) >= MAX_FILENAME_LENGTH) {
                printf("filename's length: %ld, wbsp.h should less than %d\n",
                        strlen(argv[1]) + strlen(argv[2]), MAX_FILENAME_LENGTH);
                EXIT("", -1);
            }

			sprintf(fn, "md_%s-%s", argv[1], argv[2]);
		}
	}
	else
	{
		printf("usage: %s <address> <len> [filename]. sample: %s 0x80040000 \n", argv[0], argv[0]);
		EXIT("", -1);
	}


	if( StrToNumber(argv[1], &ulAddr) == XMEDIA_SUCCESS)
	{
		printf("====dump memory:<%#010lX><%lu> to file:<%s>====\n",
				ulAddr,
				len,
				fn);

#ifdef PC_EMULATOR
#define SHAREFILE "../shm"
		printf("**** is Emulator, use share file : %s ****\n", SHAREFILE);
		pBase  = mmapfile(SHAREFILE , len);
		pMem = pBase ;
		if (NULL == pMem)
		{
			EXIT("Memory Map error.", -1);
		}
		pMem = pBase + ulAddr;
#else
		pBase = memmap(ulAddr, len);
		pMem = pBase ;
		if (NULL == pMem)
		{
			EXIT("Memory Map error.", -1);
		}

#endif

		(void)bsp_md2file(pMem, len, NULL, fn);
	}
	else
	{
		printf("Please input address like 0x12345\n");
	}

	if(pBase != NULL)
	    (void)munmap(pBase, len);

	return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
