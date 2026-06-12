/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>

#include "sample_comm_ive.h"

typedef struct sample_ive_sobel_s
{
    xmedia_ive_src_image_s st_src1;
    xmedia_ive_src_image_s st_src2;
    xmedia_ive_dst_image_s st_dst_h1;
    xmedia_ive_dst_image_s st_dst_h2;
    xmedia_ive_dst_image_s st_dst_v1;
    xmedia_ive_dst_image_s st_dst_v2;
    xmedia_ive_sobel_ctrl_s st_sobel_ctrl;
    FILE* pfp_src;
    FILE* pfp_dst_h1;
    FILE* pfp_dst_h2;
    FILE* pfp_dst_v1;
    FILE* pfp_dst_v2;
} sample_ive_sobel_s;

static xmedia_bool g_s_b_flush_cached = XMEDIA_TRUE;
static sample_ive_sobel_s g_s_st_sobel;
/******************************************************************************
* function : show Sobel uninit
******************************************************************************/
static xmedia_void sample_ive_sobel_uninit(sample_ive_sobel_s* pst_sobel)
{
    IVE_MMZ_FREE(pst_sobel->st_src1.au64phyaddr[0], pst_sobel->st_src1.au64viraddr[0]);
    IVE_MMZ_FREE(pst_sobel->st_src2.au64phyaddr[0], pst_sobel->st_src2.au64viraddr[0]);
    IVE_MMZ_FREE(pst_sobel->st_dst_h1.au64phyaddr[0], pst_sobel->st_dst_h1.au64viraddr[0]);
    IVE_MMZ_FREE(pst_sobel->st_dst_h2.au64phyaddr[0], pst_sobel->st_dst_h2.au64viraddr[0]);
    IVE_MMZ_FREE(pst_sobel->st_dst_v1.au64phyaddr[0], pst_sobel->st_dst_v1.au64viraddr[0]);
    IVE_MMZ_FREE(pst_sobel->st_dst_v2.au64phyaddr[0], pst_sobel->st_dst_v2.au64viraddr[0]);

    IVE_CLOSE_FILE(pst_sobel->pfp_src);
    IVE_CLOSE_FILE(pst_sobel->pfp_dst_h1);
    IVE_CLOSE_FILE(pst_sobel->pfp_dst_h2);
    IVE_CLOSE_FILE(pst_sobel->pfp_dst_v1);
    IVE_CLOSE_FILE(pst_sobel->pfp_dst_v2);
}
/******************************************************************************
* function : show Sobel init
******************************************************************************/
static xmedia_s32 sample_ive_sobel_init(sample_ive_sobel_s* pst_sobel,
                                    xmedia_char* pch_src_file_name, xmedia_u32 u32width, xmedia_u32 u32height)
{
    xmedia_s32 s32_ret = XMEDIA_SUCCESS;
    xmedia_char ach_file_name[IVE_FILE_NAME_LEN];
    xmedia_s8 as8_mask[25] = {0, 0, 0, 0, 0,
                         0, -1, 0, 1, 0,
                         0, -2, 0, 2, 0,
                         0, -1, 0, 1, 0,
                         0, 0, 0, 0, 0
                        };

    memset(pst_sobel, 0, sizeof(sample_ive_sobel_s));

    s32_ret = sample_comm_ive_create_image_by_cached(&(pst_sobel->st_src1), XMEDIA_IVE_IMAGE_TYPE_U8C1, u32width, u32height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, SOBEL_INIT_FAIL,
        "Error(%#x),Create src1 image failed!\n", s32_ret);

    s32_ret = sample_comm_ive_create_image_by_cached(&(pst_sobel->st_src2), XMEDIA_IVE_IMAGE_TYPE_U8C1, u32width, u32height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, SOBEL_INIT_FAIL,
        "Error(%#x),Create src2 image failed!\n", s32_ret);

    s32_ret = sample_comm_ive_create_image(&(pst_sobel->st_dst_h1), XMEDIA_IVE_IMAGE_TYPE_S16C1, u32width, u32height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, SOBEL_INIT_FAIL,
        "Error(%#x),Create dstH1 image failed!\n", s32_ret);

    s32_ret = sample_comm_ive_create_image(&(pst_sobel->st_dst_h2), XMEDIA_IVE_IMAGE_TYPE_S16C1, u32width, u32height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, SOBEL_INIT_FAIL,
        "Error(%#x),Create dstH2 image failed!\n", s32_ret);

    s32_ret = sample_comm_ive_create_image(&(pst_sobel->st_dst_v1), XMEDIA_IVE_IMAGE_TYPE_S16C1, u32width, u32height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, SOBEL_INIT_FAIL,
        "Error(%#x),Create dstV1 image failed!\n", s32_ret);

    s32_ret = sample_comm_ive_create_image(&(pst_sobel->st_dst_v2), XMEDIA_IVE_IMAGE_TYPE_S16C1, u32width, u32height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, SOBEL_INIT_FAIL,
        "Error(%#x),Create dstV2 image failed!\n", s32_ret);

    pst_sobel->st_sobel_ctrl.en_out_ctrl = XMEDIA_IVE_SOBEL_OUT_CTRL_BOTH;
    memcpy(pst_sobel->st_sobel_ctrl.as8_mask, as8_mask, 25);
    s32_ret = XMEDIA_FAILURE;
    pst_sobel->pfp_src = fopen(pch_src_file_name, "rb");
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_NULL == pst_sobel->pfp_src, SOBEL_INIT_FAIL,
        "Error,Open file %s failed!\n", pch_src_file_name);

    snprintf(ach_file_name, sizeof(ach_file_name), "./data/output/sobel/sobelh1.dat");
    pst_sobel->pfp_dst_h1 = fopen(ach_file_name, "wb");
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_NULL == pst_sobel->pfp_dst_h1, SOBEL_INIT_FAIL,
        "Error,Open file %s failed!\n", ach_file_name);

    snprintf(ach_file_name, sizeof(ach_file_name), "./data/output/sobel/sobelh2.dat");
    pst_sobel->pfp_dst_h2 = fopen(ach_file_name, "wb");
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_NULL == pst_sobel->pfp_dst_h2, SOBEL_INIT_FAIL,
        "Error,Open file %s failed!\n", ach_file_name);

    snprintf(ach_file_name, sizeof(ach_file_name), "./data/output/sobel/sobelv1.dat");
    pst_sobel->pfp_dst_v1 = fopen(ach_file_name, "wb");
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_NULL == pst_sobel->pfp_dst_v1, SOBEL_INIT_FAIL,
        "Error,Open file %s failed!\n", ach_file_name);

    snprintf(ach_file_name, sizeof(ach_file_name), "./data/output/sobel/sobelv2.dat");
    pst_sobel->pfp_dst_v2 = fopen(ach_file_name, "wb");
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_NULL == pst_sobel->pfp_dst_v2, SOBEL_INIT_FAIL,
        "Error,Open file %s failed!\n", ach_file_name);

    s32_ret = XMEDIA_SUCCESS;

SOBEL_INIT_FAIL:

    if (XMEDIA_SUCCESS != s32_ret)
    {
        sample_ive_sobel_uninit(pst_sobel);
    }
    return s32_ret;
}
/******************************************************************************
* function : show Sobel proc
******************************************************************************/
static xmedia_s32 sample_ive_sobel_proc(sample_ive_sobel_s* pst_sobel)
{
    xmedia_s32 s32_ret = XMEDIA_SUCCESS;
    xmedia_bool b_instant = XMEDIA_TRUE;
    xmedia_ive_handle ive_handle;
    xmedia_bool b_block = XMEDIA_TRUE;
    xmedia_bool b_finish = XMEDIA_FALSE;

    s32_ret = sample_comm_ive_read_file(&(pst_sobel->st_src1), pst_sobel->pfp_src);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),Read src file failed!\n",s32_ret);

    memcpy(SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(xmedia_void,pst_sobel->st_src2.au64viraddr[0]),
        SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(xmedia_void,pst_sobel->st_src1.au64viraddr[0]), pst_sobel->st_src2.au32stride[0] * pst_sobel->st_src2.u32height);
    s32_ret = sample_ive_sys_mmz_flush_cache(pst_sobel->st_src1.au64phyaddr[0], SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(xmedia_void,pst_sobel->st_src1.au64viraddr[0]),
                                      pst_sobel->st_src1.au32stride[0] * pst_sobel->st_src1.u32height);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),sample_ive_sys_mmz_flush_cache failed!\n",s32_ret);

    s32_ret = xmedia_ive_sobel(&ive_handle, &pst_sobel->st_src1, &pst_sobel->st_dst_h1, &pst_sobel->st_dst_v1, &pst_sobel->st_sobel_ctrl, b_instant);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_sobel failed!\n",s32_ret);

    s32_ret = xmedia_ive_query(ive_handle, &b_finish, b_block);
    while (ERR_CODE_IVE_QUERY_TIMEOUT == s32_ret)
    {
        usleep(100);
        s32_ret = xmedia_ive_query(ive_handle, &b_finish, b_block);
    }

    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_query failed!\n",s32_ret);

    s32_ret = sample_comm_ive_write_file(&pst_sobel->st_dst_h1, pst_sobel->pfp_dst_h1);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),Write dstH1 failed!\n",s32_ret);

    s32_ret = sample_comm_ive_write_file(&pst_sobel->st_dst_v1, pst_sobel->pfp_dst_v1);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),Write dstV1 failed!\n",s32_ret);

    //Second sobel
    // The result of sobel my be error,if you do not call sample_ive_sys_mmz_flush_cache to flush cache
    if (g_s_b_flush_cached == XMEDIA_TRUE)
    {
        s32_ret = sample_ive_sys_mmz_flush_cache(pst_sobel->st_src2.au64phyaddr[0], SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(xmedia_void,pst_sobel->st_src2.au64viraddr[0]),
                                          pst_sobel->st_src2.au32stride[0] * pst_sobel->st_src2.u32height);
        SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),sample_ive_sys_mmz_flush_cache failed!\n",s32_ret);
    }

    s32_ret = xmedia_ive_sobel(&ive_handle, &pst_sobel->st_src2, &pst_sobel->st_dst_h2, &pst_sobel->st_dst_v2, &pst_sobel->st_sobel_ctrl, b_instant);
      SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_sobel failed!\n",s32_ret);

    s32_ret = xmedia_ive_query(ive_handle, &b_finish, b_block);
    while (ERR_CODE_IVE_QUERY_TIMEOUT == s32_ret)
    {
        usleep(100);
        s32_ret = xmedia_ive_query(ive_handle, &b_finish, b_block);
    }
      SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_query failed!\n",s32_ret);

    s32_ret = sample_comm_ive_write_file(&pst_sobel->st_dst_h2, pst_sobel->pfp_dst_h2);
      SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),Write dstH2 failed!\n",s32_ret);

    s32_ret = sample_comm_ive_write_file(&pst_sobel->st_dst_v2, pst_sobel->pfp_dst_v2);
      SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),Write dstV2 failed!\n",s32_ret);

    return s32_ret;
}
/******************************************************************************
* function : show Sobel sample
******************************************************************************/
xmedia_void sample_ive_sobel(xmedia_void)
{
    xmedia_s32 s32_ret;
    xmedia_u32 u32width = 720;
    xmedia_u32 u32height = 576;
    xmedia_char* pch_src_file_name = "./data/input/sobel/sobel.yuv";

    memset(&g_s_st_sobel,0,sizeof(g_s_st_sobel));

    s32_ret = sample_ive_sobel_init(&g_s_st_sobel, pch_src_file_name, u32width, u32height);
    SAMPLE_CHECK_EXPR(XMEDIA_SUCCESS != s32_ret,"Error(%#x),sample_ive_sobel_init failed!\n",s32_ret);

    s32_ret =  sample_ive_sobel_proc(&g_s_st_sobel);
    if (XMEDIA_SUCCESS == s32_ret)
    {
      SAMPLE_PRT("Process success!\n");
    }

    sample_ive_sobel_uninit(&g_s_st_sobel);
    memset(&g_s_st_sobel,0,sizeof(g_s_st_sobel));
}
/******************************************************************************
* function : Sobel sample signal handle
******************************************************************************/
xmedia_void sample_ive_sobel_handle_sig(xmedia_void)
{
    sample_ive_sobel_uninit(&g_s_st_sobel);
    memset(&g_s_st_sobel,0,sizeof(g_s_st_sobel));
    sample_comm_ive_ive_mpi_exit();
}



