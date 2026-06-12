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
#include <math.h>
#include "sample_comm_ive.h"

typedef struct sample_ive_edge_info_s
{
    xmedia_ive_src_image_s st_src;
    xmedia_ive_dst_image_s st_edge;
    xmedia_ive_dst_image_s st_mag;
    xmedia_ive_mag_and_ang_ctrl_s st_mag_and_ang_ctrl;
    xmedia_ive_thresh_u16_ctrl_s st_thr_u16_ctrl;
    FILE* pfp_src;
    FILE* pfp_dst;
} sample_ive_edge_info_s;

static sample_ive_edge_info_s g_s_st_edge_info;

/******************************************************************************
* function : Edge uninit
******************************************************************************/
static xmedia_void sample_ive_edge_uninit(sample_ive_edge_info_s* pst_edge_info)
{
    IVE_MMZ_FREE(pst_edge_info->st_src.au64phyaddr[0], pst_edge_info->st_src.au64viraddr[0]);
    IVE_MMZ_FREE(pst_edge_info->st_edge.au64phyaddr[0], pst_edge_info->st_edge.au64viraddr[0]);
    IVE_MMZ_FREE(pst_edge_info->st_mag.au64phyaddr[0], pst_edge_info->st_mag.au64viraddr[0]);

    IVE_CLOSE_FILE(pst_edge_info->pfp_src);
    IVE_CLOSE_FILE(pst_edge_info->pfp_dst);
}
/******************************************************************************
* function : Edge init
******************************************************************************/
static xmedia_s32 sample_ive_edge_init(sample_ive_edge_info_s* pst_edge_info,
                                    xmedia_char* pch_src_file_name, xmedia_char* pch_dst_file_name, xmedia_u32 u32width, xmedia_u32 u32height)
{
    xmedia_s32 s32_ret = XMEDIA_SUCCESS;
    xmedia_s8 as8_mask[25] = {0, 0, 0, 0, 0,
                         0, -1, 0, 1, 0,
                         0, -2, 0, 2, 0,
                         0, -1, 0, 1, 0,
                         0, 0, 0, 0, 0
                        };

    memset(pst_edge_info, 0, sizeof(sample_ive_edge_info_s));
    memcpy(pst_edge_info->st_mag_and_ang_ctrl.as8_mask, as8_mask, 25);
    pst_edge_info->st_mag_and_ang_ctrl.en_out_ctrl = XMEDIA_IVE_MAG_AND_ANG_OUT_CTRL_MAG;
    pst_edge_info->st_mag_and_ang_ctrl.u16_thr = 0;
    pst_edge_info->st_thr_u16_ctrl.en_mode = XMEDIA_IVE_THRESH_U16_MODE_U16_TO_U8_MIN_MID_MAX;
    pst_edge_info->st_thr_u16_ctrl.u16_high_thr = 100;
    pst_edge_info->st_thr_u16_ctrl.u16_low_thr = 100;
    pst_edge_info->st_thr_u16_ctrl.u8_max_val = 255;
    pst_edge_info->st_thr_u16_ctrl.u8_mid_val = 0;
    pst_edge_info->st_thr_u16_ctrl.u8_min_val = 0;

    //创建内存空间
    s32_ret = sample_comm_ive_create_image(&pst_edge_info->st_src, XMEDIA_IVE_IMAGE_TYPE_U8C1, u32width, u32height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, CANNY_INIT_FAIL,
        "Error(%#x),Create Src Image failed!\n", s32_ret);

    s32_ret = sample_comm_ive_create_image(&pst_edge_info->st_edge, XMEDIA_IVE_IMAGE_TYPE_U8C1, u32width, u32height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, CANNY_INIT_FAIL,
        "Error(%#x),Create edge Image failed!\n", s32_ret);

    s32_ret = sample_comm_ive_create_image(&pst_edge_info->st_mag, XMEDIA_IVE_IMAGE_TYPE_U16C1, u32width, u32height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, CANNY_INIT_FAIL,
        "Error(%#x),Create Mag Image failed!\n", s32_ret);

    s32_ret = XMEDIA_FAILURE;
    pst_edge_info->pfp_src = fopen(pch_src_file_name, "rb");
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_NULL == pst_edge_info->pfp_src, CANNY_INIT_FAIL,
        "Error,Open file %s failed!\n", pch_src_file_name);

    pst_edge_info->pfp_dst = fopen(pch_dst_file_name, "wb");
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_NULL == pst_edge_info->pfp_dst, CANNY_INIT_FAIL,
        "Error,Open file %s failed!\n", pch_dst_file_name);

    s32_ret = XMEDIA_SUCCESS;

CANNY_INIT_FAIL:

    if (XMEDIA_SUCCESS != s32_ret)
    {
        sample_ive_edge_uninit(pst_edge_info);
    }
    return s32_ret;
}

/******************************************************************************
* function : show part canny sample
******************************************************************************/
static xmedia_s32 sample_ive_part_edge(sample_ive_edge_info_s* pst_edge_info)
{
    xmedia_s32 s32_ret = XMEDIA_SUCCESS;
    xmedia_bool b_instant = XMEDIA_TRUE;
    xmedia_bool b_block = XMEDIA_TRUE;
    xmedia_bool b_finish = XMEDIA_FALSE;
    xmedia_ive_handle ive_handle;

    s32_ret = sample_comm_ive_read_file(&pst_edge_info->st_src, pst_edge_info->pfp_src);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),Read src file failed!\n",s32_ret);

    b_instant = XMEDIA_FALSE;
    s32_ret = xmedia_ive_mag_and_ang(&ive_handle, &pst_edge_info->st_src, &pst_edge_info->st_mag, \
                                  XMEDIA_NULL, &pst_edge_info->st_mag_and_ang_ctrl, b_instant);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_mag_and_ang failed!\n",s32_ret);

    b_instant = XMEDIA_TRUE;
    s32_ret = xmedia_ive_thresh_u16(&ive_handle, &pst_edge_info->st_mag, &pst_edge_info->st_edge, \
                                   &pst_edge_info->st_thr_u16_ctrl, b_instant);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_thresh_u16 failed!\n",s32_ret);

    s32_ret = xmedia_ive_query(ive_handle, &b_finish, b_block);
    while (ERR_CODE_IVE_QUERY_TIMEOUT == s32_ret)
    {
        usleep(100);
        s32_ret = xmedia_ive_query(ive_handle, &b_finish, b_block);
    }
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_query failed!\n",s32_ret);

    s32_ret = sample_comm_ive_write_file(&pst_edge_info->st_edge, pst_edge_info->pfp_dst);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),Write edge file failed!\n",s32_ret);

    return s32_ret;
}
/******************************************************************************
* function : show canny sample
******************************************************************************/
xmedia_void sample_ive_edge(xmedia_void)
{
    xmedia_u16 u32width = 720;
    xmedia_u16 u32height = 576;
    xmedia_char* pch_src_file_name = "./data/input/edge/edge.yuv";
    xmedia_char ach_dst_file_name[IVE_FILE_NAME_LEN];
    xmedia_s32 s32_ret;

    memset(&g_s_st_edge_info,0,sizeof(g_s_st_edge_info));


    snprintf(ach_dst_file_name, sizeof(ach_dst_file_name), "./data/output/edge/edge_out.yuv");

    s32_ret = sample_ive_edge_init(&g_s_st_edge_info, pch_src_file_name, ach_dst_file_name, u32width, u32height);
    SAMPLE_CHECK_EXPR(XMEDIA_SUCCESS != s32_ret,"Error(%#x),sample_ive_edge_init failed!\n",s32_ret);

    s32_ret =  sample_ive_part_edge(&g_s_st_edge_info);

    if (XMEDIA_SUCCESS == s32_ret)
    {
        SAMPLE_PRT("Process success!\n");
    }

    sample_ive_edge_uninit(&g_s_st_edge_info);
    memset(&g_s_st_edge_info,0,sizeof(g_s_st_edge_info));
}
/******************************************************************************
* function :Edge sample signal handle
******************************************************************************/
xmedia_void sample_ive_edge_handle_sig(xmedia_void)
{
    sample_ive_edge_uninit(&g_s_st_edge_info);
    memset(&g_s_st_edge_info,0,sizeof(g_s_st_edge_info));
    sample_comm_ive_ive_mpi_exit();
}

