/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include "bsp.h"
#include "argparser.h"
#include "cmdshell.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cpluscplus */

CMD_SHELL_T *CMD_SHELL_Search(IN char* cmdstr,
                              IN CMD_SHELL_T* pCmds
                              )
{
    if (NULL == pCmds || NULL == cmdstr)
    {
        return NULL;
    }

    LENGTH_T lenCmdStr = strlen(cmdstr);
    if ( 0 == lenCmdStr )
    {
        WRITE_LOG_DEBUG1("cmdstr is zero length string \n");
        return NULL;
    }

    /**/
    int i = 0;
    while(pCmds[i].cmdstr)
    {
        if (strcmp(pCmds[i].cmdstr, cmdstr) == 0)
        {
            return (&pCmds[i]);
        }
        i++;
    }
    WRITE_LOG_DEBUG1("cann't find cmd: %s \n", cmdstr);
    return NULL;
}

int CMD_SHELL_Run(CMD_SHELL_T* pCmd, int argc, char** argv)
{
    if (NULL == pCmd)
    {
        return ERR_CMD_SHELL_NULL_CMD;
    }
    if (NULL == pCmd->pFunc)
    {
        return ERR_CMD_SHELL_NULL_FP;
    }
    else
    {
        //CMD_SHELL_PRINT_CMDSTRS(argc, argv);
        return ((int)pCmd->pFunc(argc, argv));
    }

}
#if 0
impl in file argparser.c
static void StrToArg(CHAR* pSrc, int* argc, CHAR** argv)
{
    U32 iNum = 0;
    BOOL bNewWord = FALSE;
    while(*pSrc)
    {
        if (*pSrc == ' ')
        {
            if (bNewWord == TRUE)
            {

                *pSrc = 0;
                bNewWord = FALSE;
            }
        }
        else
        {
			if (bNewWord == FALSE)
			{
                bNewWord = TRUE;
                argv[iNum] = pSrc;
			    iNum++;
			}
        }
		pSrc++;

    }
	*argc = iNum;

}
#endif

#define MAX_CMD_LEN    128
int CMD_SHELL_RUN(char* cmdstrs, const  CMD_SHELL_T *pCmds)
{
    char xcmdstr[MAX_CMD_LEN];
    int xargc;
    char* xargv[16] = {0};

    if (NULL == cmdstrs)
    {
        printf("null\n");
        return XMEDIA_FAILURE;
    }

    if (cmdstrs[0] == 0)
    {
        return XMEDIA_FAILURE;
    }
    if (strlen(cmdstrs) >= MAX_CMD_LEN)
    {
        printf("cmdstrs's length is %ld, wbsp.h should less than %d\n",
                strlen(cmdstrs), MAX_CMD_LEN);
        return XMEDIA_FAILURE;
    }
    memset(xcmdstr, 0, MAX_CMD_LEN);

    strncpy(xcmdstr, cmdstrs, MAX_CMD_LEN - 1);

    StrToArg(xcmdstr, &xargc, xargv);

    //CMD_SHELL_PRINT_CMDSTRS(xargc, xargv);

    CMD_SHELL_T* pCmd = CMD_SHELL_Search(xargv[0], (CMD_SHELL_T*)pCmds);
    if (pCmd == NULL)
    {
        WRITE_LOG_DEBUG("cann't find cmdstr:%s\n", cmdstrs);
        return XMEDIA_FAILURE;
    }
    else
    {
        return CMD_SHELL_Run(pCmd, xargc, xargv);
    }
}

int CMD_SHELL_RUN_2(int argc, char* argv[], const CMD_SHELL_T *pCmds)
{
    CMD_SHELL_T* pCmd = NULL;

    char* pCmdStr = argv[0];
    char* pTmp = NULL;

	for (pTmp = pCmdStr; *pTmp != '\0';)
    {
		if (*pTmp++ == '/')
			pCmdStr = pTmp;
	}
    WRITE_LOG_DEBUG1("cmdstr:%s\n", pCmdStr);

    pCmd = CMD_SHELL_Search(pCmdStr, (CMD_SHELL_T*)pCmds);
    if (pCmd == NULL)
    {
        WRITE_LOG_DEBUG("cann't find cmdstr:%s\n", argv[0]);
        return XMEDIA_FAILURE;
    }
    else
    {
        return CMD_SHELL_Run(pCmd, argc, argv);
    }

}
#ifdef __cplusplus
}
#endif /* __cplusplus */
