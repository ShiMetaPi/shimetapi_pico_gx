/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#include <string.h>
#include "bsp.h"

#include "cmdshell.h"



#include "argparser.h"
#include "btools.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define BTOOLS_NAME "btools"
#define BTOOLS_VERSION "ver0.0.1"

XMEDIA_RET print_help_message(int argc, char* argv[]);

LOCALFUNC XMEDIA_RET do_cmd_install(int argc, char* argv[]);

XMEDIA_RET do_cmd_uninstall(int argc, char* argv[]);

static ARGOpt_t opts[] =
{
    {"h", ARG_TYPE_SINGLE, FALSE, 0,
            "\tprint help msg\n", NULL},

    {"i", ARG_TYPE_SINGLE, FALSE, 0,
            "\tinstall board tools\n" ,NULL},

    {"u", ARG_TYPE_SINGLE, FALSE, 0,
            "\tuninstall board tools\n",NULL},

    {"V", ARG_TYPE_SINGLE, FALSE, 0,
            "\tprint version\n", NULL},
       // end add

    {"END", ARG_TYPE_END, FALSE, 0,
            "\tEND\n", NULL}
};


static CMD_SHELL_T gBToolsCmds[] =
{
    CMD_SHELL_DEF("xmmc", CMD_ENABLE, xmmc,"memory clear")
    CMD_SHELL_DEF("xmmd", CMD_ENABLE, xmmd,"memory display (8bit)")
    CMD_SHELL_DEF("xmmd.l", CMD_ENABLE, xmmd_l,"memory display (32bit)")
    CMD_SHELL_DEF("xmmm", CMD_ENABLE, xmmm,"memory modify")
    CMD_SHELL_DEF("i2c_read", CMD_ENABLE, i2c_read,"i2c device read")
    CMD_SHELL_DEF("i2c_write", CMD_ENABLE, i2c_write,"i2c device read")
   CMD_SHELL_DEF("ssp_read", CMD_ENABLE, ssp_read,"ssp device read")
    CMD_SHELL_DEF("ssp_write", CMD_ENABLE, ssp_write,"ssp device read")
    {NULL, CMD_DISABLE,NULL,0}
};
XMEDIA_RET print_help_message(int argc, char* argv[])
{
    int i = 0;
    (void) argv;
    (void) argc;

    ARGPrintHelp(opts);

    printf("------------------------------------------------------\n");

    while(gBToolsCmds[i].cmdstr != NULL)
    {
        if (gBToolsCmds[i].isEnable == CMD_ENABLE)
        {
            printf(" %-10s : %s\n", gBToolsCmds[i].cmdstr, gBToolsCmds[i].helpstr);
        }
        i++;
    }

    return XMEDIA_SUCCESS;
}


XMEDIA_RET print_version_message(int argc, char* argv[] )
{
    (void) argc;
    (void) argv;
    printf("*** Board tools : %s *** \n",BTOOLS_VERSION);
    return XMEDIA_SUCCESS;
}

/*btools install*/
LOCALFUNC XMEDIA_RET do_cmd_install(int argc, char* argv[])
{
    (void) argv;
    (void) argc;
    int i = 0;
    int ret = 0;
    while(gBToolsCmds[i].cmdstr)
    {
        WRITE_LOG_INFO("%d:installing <%s>.\n", i, gBToolsCmds[i].cmdstr);
        if ( (ret = symlink(BTOOLS_NAME, gBToolsCmds[i].cmdstr)) == 0)
        {
            //WRITE_LOG_INFO(" install <%s> ok.\n", gBToolsCmds[i].cmdstr);
        }
        else if (ret == EEXIST)
        {
            WRITE_LOG_INFO(" <%s> exist.\n" , gBToolsCmds[i].cmdstr);
        }
        else
        {
            perror(" install error!");
        }
        i++;

    }
    return XMEDIA_SUCCESS;
}

/*btools uninstall*/
XMEDIA_RET do_cmd_uninstall(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    char sRealFile[MAXPATHLEN];
    int lenFn = 0;
    int i = 0;
    while(gBToolsCmds[i].cmdstr)
    {

        WRITE_LOG_INFO("%d:uninstalling <%s>.\n",i, gBToolsCmds[i].cmdstr);
        memset(sRealFile, 0, MAXPATHLEN);

        if ((lenFn = readlink(gBToolsCmds[i].cmdstr, sRealFile, MAXPATHLEN)) > 0)
        {
            //printf(" get link info ,%s len: %d\n",sRealFile, lenFn);
            if (lenFn < MAXPATHLEN)
            {
                sRealFile[lenFn] = 0;
                //WRITE_LOG_INFO(" uninstall <%s> ok.\n",  gBToolsCmds[i].cmdstr);
                if (strcmp(sRealFile, BTOOLS_NAME) == 0)
                {
                    if (unlink(gBToolsCmds[i].cmdstr))
                    {
                        perror(" uninstall error");
                    }
                }
                else
                {
                    WRITE_LOG_INFO(" %s not link to me(%s)\n", sRealFile, BTOOLS_NAME);
                }
            }
            else
            {
                WRITE_LOG_ERROR(" buffer is too small, %d\n", lenFn);
            }
        }
        else
        {
            perror(" read link error");
        }
        i++;

    }
    return XMEDIA_SUCCESS;

}

/*link */

int main(int argc , char* argv[])
{
    if (XMEDIA_SUCCESS != LOG_CREATE(LOG_LEVEL_DEBUG, 2048))
    {
        printf("create logqueue error.\n");
        return -1;
    }

    XMEDIA_RET bspDo = XMEDIA_FAILURE;

    char* pCmdStr = argv[0];
    char* pTmp = NULL;

	for (pTmp = pCmdStr; *pTmp != '\0';)
    {
		if (*pTmp++ == '/')
			pCmdStr = pTmp;
	}


    if (strcmp(pCmdStr, BTOOLS_NAME) == 0)
    {

        if ( ARGParser(argc, argv, opts) != XMEDIA_SUCCESS)
        {
            (void)print_help_message(argc, argv );
            return XMEDIA_SUCCESS;
        }
        if (opts[0].isSet)
        {
            (void)print_help_message(argc, argv );
        }
        else if (opts[1].isSet)
        {
            (void)do_cmd_install(0, 0);
        }
        else if (opts[2].isSet)
        {
            (void)do_cmd_uninstall(0,0);
        }
        else if (opts[3].isSet)
        {
            (void)print_version_message(argc, argv);
        }
        else
        {
            (void)print_help_message(argc, argv );
            return XMEDIA_SUCCESS;
        }
        return XMEDIA_SUCCESS;
    }

    bspDo = (XMEDIA_RET)CMD_SHELL_RUN_2(argc, argv, (CMD_SHELL_T*)gBToolsCmds);

    if (bspDo != XMEDIA_SUCCESS)
    {
        printf("\ndo errro\n");
    }
    printf("[END]\n");
    fflush(stdout);
    return 0;
}


/*
shell

*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

