/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "sample_comm_ive.h"
#include "sample_ive_main.h"

static char **s_pp_ch_cmd_argv = NULL;

/******************************************************************************
* function : to process abnormal case
******************************************************************************/
#ifndef __LITEOS__
xmedia_void sample_ive_handle_sig(xmedia_s32 s32Signo)
{
    signal(SIGINT,SIG_IGN);
    signal(SIGTERM,SIG_IGN);

    if (SIGINT == s32Signo || SIGTERM == s32Signo)
    {
        switch (*s_pp_ch_cmd_argv[1])
        {
            case '0':
                {
                    sample_ive_sobel_handle_sig();
                }
                break;
            case '1':
                {
                    sample_ive_edge_handle_sig();
                }
                break;
            case '2':
                {
                    sample_ive_gmm2_handle_sig();
                }
                break;
            case '3':
                {
                    sample_ive_test_memory_handle_sig();
                }
                break;
            case '4':
                {
                    sample_ive_md_handle_sig();
                }
                break;
            case '5':
                {
                    sample_ive_canny_handle_sig();
                }
                break;
            case '6':
                {
                    sample_ive_od_handle_sig();
                }
                break;
            case '7':
                {
                    sample_ive_st_lk_handle_sig();
                }
                break;
            case '8':
                {
                    sample_ive_dma_handle_sig();
                }
                break;
            default :
                {
                }
                break;
        }

        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}
#endif
/******************************************************************************
* function : show usage
******************************************************************************/
xmedia_void sample_ive_usage(xmedia_char* pchPrgName)
{
    printf("Usage : %s <index> [complete] [encode] [vo]\n", pchPrgName);
    printf("index:\n");
    printf("\t 0)Sobel.(FILE->IVE->FILE).\n");
    printf("\t 1)Edge,complete edge by mag_and_ang.(FILE->IVE->FILE).\n");
    printf("\t 2)Gmm2.(FILE->IVE->FILE).\n");
    printf("\t 3)MemoryTest.(FILE->IVE->FILE).\n");
    printf("\t 4)Motion detected.(VI->VPSS->IVE->VGS->VENC).\n");
    printf("\t 5)Canny.(FILE->IVE->FILE).\n");
    printf("\t 6)Od.(VI->VPSS->IVE->VGS->VENC).\n");
    printf("\t 7)St Lk.(FILE->IVE->FILE).\n");
    printf("\t 8)dma.(FILE->IVE->FILE).\n");
}

/******************************************************************************
* function : ive sample
******************************************************************************/
#ifdef __LITEOS__
int app_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    if (argc < 2)
    {
        sample_ive_usage(argv[0]);
        return XMEDIA_FAILURE;
    }
    s_pp_ch_cmd_argv = argv;
#ifndef __LITEOS__
    signal(SIGINT, sample_ive_handle_sig);
    signal(SIGTERM, sample_ive_handle_sig);
#endif

    switch (*argv[1])
    {
        case '0':
            {
                sample_ive_sobel();
            }
            break;
        case '1':
            {
                sample_ive_edge();
            }
            break;
        case '2':
            {
                sample_ive_gmm2();
            }
            break;
        case '3':
            {
                sample_ive_test_memory();
            }
            break;
        case '4':
            {
                sample_ive_md();
            }
            break;
        case '5':
            {
                sample_ive_canny();
            }
            break;
        case '6':
            {
                sample_ive_od();
            }
            break;
        case '7':
            {
                sample_ive_st_lk();
            }
            break;
        case '8':
            {
                sample_ive_dma();
            }
            break;
        default :
            {
                sample_ive_usage(argv[0]);
            }
            break;
    }

    return 0;

}