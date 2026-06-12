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

typedef struct sample_ive_canny_info_s
{
    xmedia_ive_src_image_s stSrc;
    xmedia_ive_dst_image_s stEdge;
    xmedia_ive_dst_image_s stMag;
    xmedia_ive_mem_info_s  stStack;
    xmedia_ive_canny_hys_edge_ctrl_s  stCannyHysEdgeCtrl;
    xmedia_ive_mag_and_ang_ctrl_s stMagAndAngCtrl;
    xmedia_ive_thresh_u16_ctrl_s stThrU16Ctrl;
    FILE* pFpSrc;
    FILE* pFpDst;
} sample_ive_canny_info_s;

static sample_ive_canny_info_s s_stCannyInfo;

/******************************************************************************
* function : Canny uninit
******************************************************************************/
static xmedia_void sample_ive_canny_uninit(sample_ive_canny_info_s* pstCannyInfo)
{
    IVE_MMZ_FREE(pstCannyInfo->stSrc.au64phyaddr[0], pstCannyInfo->stSrc.au64viraddr[0]);
    IVE_MMZ_FREE(pstCannyInfo->stEdge.au64phyaddr[0], pstCannyInfo->stEdge.au64viraddr[0]);
    IVE_MMZ_FREE(pstCannyInfo->stMag.au64phyaddr[0], pstCannyInfo->stMag.au64viraddr[0]);
    IVE_MMZ_FREE(pstCannyInfo->stStack.u64phyaddr, pstCannyInfo->stStack.u64viraddr);
    IVE_MMZ_FREE(pstCannyInfo->stCannyHysEdgeCtrl.stmem.u64phyaddr, \
                 pstCannyInfo->stCannyHysEdgeCtrl.stmem.u64viraddr);

    IVE_CLOSE_FILE(pstCannyInfo->pFpSrc);
    IVE_CLOSE_FILE(pstCannyInfo->pFpDst);
}
/******************************************************************************
* function : Canny init
******************************************************************************/
static xmedia_s32 sample_ive_canny_init(sample_ive_canny_info_s* pstCannyInfo,
                                    xmedia_char* pchSrcFileName, xmedia_char* pchDstFileName, xmedia_u32 u32Width, xmedia_u32 u32Height)
{
    xmedia_s32 s32Ret = XMEDIA_SUCCESS;
    xmedia_u32 u32Size = 0;
    xmedia_s8 as8Mask[25] = {0, 0, 0, 0, 0,
                         0, -1, 0, 1, 0,
                         0, -2, 0, 2, 0,
                         0, -1, 0, 1, 0,
                         0, 0, 0, 0, 0
                        };

    memset(pstCannyInfo, 0, sizeof(sample_ive_canny_info_s));
    memcpy(pstCannyInfo->stCannyHysEdgeCtrl.as8_mask, as8Mask, 25);
    memcpy(pstCannyInfo->stMagAndAngCtrl.as8_mask, as8Mask, 25);
    pstCannyInfo->stCannyHysEdgeCtrl.u16_high_thr = 150;
    pstCannyInfo->stCannyHysEdgeCtrl.u16_low_thr = 50;
    pstCannyInfo->stMagAndAngCtrl.en_out_ctrl = XMEDIA_IVE_MAG_AND_ANG_OUT_CTRL_MAG;
    pstCannyInfo->stMagAndAngCtrl.u16_thr = 0;
    pstCannyInfo->stThrU16Ctrl.en_mode = XMEDIA_IVE_THRESH_U16_MODE_U16_TO_U8_MIN_MID_MAX;
    pstCannyInfo->stThrU16Ctrl.u16_high_thr = 100;
    pstCannyInfo->stThrU16Ctrl.u16_low_thr = 100;
    pstCannyInfo->stThrU16Ctrl.u8_max_val = 255;
    pstCannyInfo->stThrU16Ctrl.u8_mid_val = 0;
    pstCannyInfo->stThrU16Ctrl.u8_min_val = 0;

    s32Ret = sample_comm_ive_create_image(&pstCannyInfo->stSrc, XMEDIA_IVE_IMAGE_TYPE_U8C1, u32Width, u32Height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, CANNY_INIT_FAIL,
        "Error(%#x),Create Src Image failed!\n", s32Ret);

    s32Ret = sample_comm_ive_create_image(&pstCannyInfo->stEdge, XMEDIA_IVE_IMAGE_TYPE_U8C1, u32Width, u32Height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, CANNY_INIT_FAIL,
        "Error(%#x),Create edge Image failed!\n", s32Ret);

    s32Ret = sample_comm_ive_create_image(&pstCannyInfo->stMag, XMEDIA_IVE_IMAGE_TYPE_U16C1, u32Width, u32Height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, CANNY_INIT_FAIL,
        "Error(%#x),Create Mag Image failed!\n", s32Ret);

    u32Size = pstCannyInfo->stSrc.au32stride [0] * pstCannyInfo->stSrc.u32height * 4 + sizeof(xmedia_ive_canny_stack_size_s);
    s32Ret = sample_comm_ive_create_mem_info(&pstCannyInfo->stStack, u32Size);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, CANNY_INIT_FAIL,
        "Error(%#x),Create Stack Mem info failed!\n", s32Ret);

    u32Size = pstCannyInfo->stSrc.au32stride [0] * pstCannyInfo->stSrc.u32height * 3;
    s32Ret = sample_comm_ive_create_mem_info(&pstCannyInfo->stCannyHysEdgeCtrl.stmem, u32Size);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, CANNY_INIT_FAIL,
        "Error(%#x),Create CannyHysEdgeCtrl.stMem Mem info failed!\n", s32Ret);

    s32Ret = XMEDIA_FAILURE;
    pstCannyInfo->pFpSrc = fopen(pchSrcFileName, "rb");
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_NULL == pstCannyInfo->pFpSrc, CANNY_INIT_FAIL,
        "Error,Open file %s failed!\n", pchSrcFileName);

    pstCannyInfo->pFpDst = fopen(pchDstFileName, "wb");
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_NULL == pstCannyInfo->pFpDst, CANNY_INIT_FAIL,
        "Error,Open file %s failed!\n", pchDstFileName);

    s32Ret = XMEDIA_SUCCESS;

CANNY_INIT_FAIL:

    if (XMEDIA_SUCCESS != s32Ret)
    {
        sample_ive_canny_uninit(pstCannyInfo);
    }
    return s32Ret;
}
/******************************************************************************
* function : show complate canny sample
******************************************************************************/
// static xmedia_s32 SAMPLE_IVE_Complate_Canny(sample_ive_canny_info_s* pstCannyInfo)
// {
//     xmedia_s32 s32Ret = XMEDIA_SUCCESS;
//     xmedia_bool bInstant = XMEDIA_TRUE;
//     xmedia_bool bBlock = XMEDIA_TRUE;
//     xmedia_bool bFinish = XMEDIA_FALSE;
//     xmedia_ive_handle IveHandle;

//     s32Ret = sample_comm_ive_read_file(&(pstCannyInfo->stSrc), pstCannyInfo->pFpSrc);
//     SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32Ret,s32Ret,"Error(%#x),Read src file failed!\n",s32Ret);

//     s32Ret = XMEDIA_API_IVE_CannyHysEdge(&IveHandle, &pstCannyInfo->stSrc, 
//                                      &pstCannyInfo->stEdge, &pstCannyInfo->stStack, 
//                                      &pstCannyInfo->stCannyHysEdgeCtrl, bInstant);
//     SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32Ret,s32Ret,"Error(%#x),XMEDIA_API_IVE_CannyHysEdge failed!\n",s32Ret);


//     s32Ret = xmedia_ive_query(IveHandle, &bFinish, bBlock);
//     while (ERR_CODE_IVE_QUERY_TIMEOUT == s32Ret)
//     {
//         usleep(100);
//         s32Ret = xmedia_ive_query(IveHandle, &bFinish, bBlock);
//     }
//     SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32Ret,s32Ret,"Error(%#x),xmedia_ive_query failed!\n",s32Ret);

//     s32Ret = XMEDIA_API_IVE_CannyEdge(&pstCannyInfo->stEdge, &pstCannyInfo->stStack);
//     SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32Ret,s32Ret,"Error(%#x),XMEDIA_API_IVE_CannyEdge failed!\n",s32Ret);

//     s32Ret = sample_comm_ive_write_file(&pstCannyInfo->stEdge, pstCannyInfo->pFpDst);
//     SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32Ret,s32Ret,"Error(%#x),Write edge file failed!\n",s32Ret);

//     return s32Ret;
// }
/******************************************************************************
* function : show part canny sample
******************************************************************************/
static xmedia_s32 SAMPLE_IVE_Part_Canny(sample_ive_canny_info_s* pstCannyInfo)
{
    xmedia_s32 s32Ret = XMEDIA_SUCCESS;
    xmedia_bool bInstant = XMEDIA_TRUE;
    xmedia_bool bBlock = XMEDIA_TRUE;
    xmedia_bool bFinish = XMEDIA_FALSE;
    xmedia_ive_handle IveHandle;

    s32Ret = sample_comm_ive_read_file(&pstCannyInfo->stSrc, pstCannyInfo->pFpSrc);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32Ret,s32Ret,"Error(%#x),Read src file failed!\n",s32Ret);

    bInstant = XMEDIA_FALSE;
    s32Ret = xmedia_ive_mag_and_ang(&IveHandle, &pstCannyInfo->stSrc, &pstCannyInfo->stMag, \
                                  XMEDIA_NULL, &pstCannyInfo->stMagAndAngCtrl, bInstant);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32Ret,s32Ret,"Error(%#x),xmedia_ive_mag_and_ang failed!\n",s32Ret);

    bInstant = XMEDIA_TRUE;
    s32Ret = xmedia_ive_thresh_u16(&IveHandle, &pstCannyInfo->stMag, &pstCannyInfo->stEdge, \
                                   &pstCannyInfo->stThrU16Ctrl, bInstant);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32Ret,s32Ret,"Error(%#x),xmedia_ive_thresh_u16 failed!\n",s32Ret);

    s32Ret = xmedia_ive_query(IveHandle, &bFinish, bBlock);
    while (ERR_CODE_IVE_QUERY_TIMEOUT == s32Ret)
    {
        usleep(100);
        s32Ret = xmedia_ive_query(IveHandle, &bFinish, bBlock);
    }
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32Ret,s32Ret,"Error(%#x),xmedia_ive_query failed!\n",s32Ret);
    printf("123333\n");
    s32Ret = sample_comm_ive_write_file(&pstCannyInfo->stEdge, pstCannyInfo->pFpDst);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32Ret,s32Ret,"Error(%#x),Write edge file failed!\n",s32Ret);
     printf("4444444433\n");
    return s32Ret;
}
/******************************************************************************
* function : show canny sample
******************************************************************************/
xmedia_void sample_ive_canny(xmedia_void)
{
    xmedia_u16 u32Width = 720;
    xmedia_u16 u32Height = 576;
    xmedia_char* pchSrcFileName = "./data/input/canny/canny.yuv";
    xmedia_char achDstFileName[IVE_FILE_NAME_LEN];
    xmedia_s32 s32Ret;
    //xmedia_char chComplete = 0;
    memset(&s_stCannyInfo,0,sizeof(s_stCannyInfo));

    sample_comm_ive_check_ive_mpi_init();
    snprintf(achDstFileName, sizeof(achDstFileName), "./data/output/canny/cannyout_complete.yuv");
    printf("achDstFileName:%s\n",achDstFileName);
    s32Ret = sample_ive_canny_init(&s_stCannyInfo, pchSrcFileName, achDstFileName, u32Width, u32Height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, CANNY_FAIL,
        "Error(%#x),sample_ive_canny_init failed!\n", s32Ret);

    //if ('0' == chComplete)
    //{
       s32Ret =  SAMPLE_IVE_Part_Canny(&s_stCannyInfo);
    //}
    //else
    //{
       //s32Ret =  SAMPLE_IVE_Complate_Canny(&s_stCannyInfo);
    //}

    if (XMEDIA_SUCCESS == s32Ret)
    {
        SAMPLE_PRT("Process success!\n");
    }

    sample_ive_canny_uninit(&s_stCannyInfo);
    memset(&s_stCannyInfo,0,sizeof(s_stCannyInfo));

CANNY_FAIL:
    sample_comm_ive_ive_mpi_exit();
}
/******************************************************************************
* function :Canny sample signal handle
******************************************************************************/
xmedia_void sample_ive_canny_handle_sig(xmedia_void)
{
    sample_ive_canny_uninit(&s_stCannyInfo);
    memset(&s_stCannyInfo,0,sizeof(s_stCannyInfo));
    sample_comm_ive_ive_mpi_exit();
}

