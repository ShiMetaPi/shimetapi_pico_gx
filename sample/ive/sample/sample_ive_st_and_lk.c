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

#define MAX_POINT_NUM 500
#define MIN_DIST 5

typedef struct sample_ive_st_lk_s
{
    xmedia_ive_src_image_s                      astPrevPyr[4];
    xmedia_ive_src_image_s                      astNextPyr[4];
    xmedia_ive_src_mem_info_s                     stPrevPts;
    xmedia_ive_mem_info_s                         stNextPts;
    xmedia_ive_dst_mem_info_s                     stStatus;
    xmedia_ive_dst_mem_info_s                     stErr;
    xmedia_ive_lk_optical_flow_pyr_ctrl_s        stLkPyrCtrl;

    xmedia_ive_src_image_s                      stStSrc;
    xmedia_ive_image_s                         stStDst;
    xmedia_ive_dst_mem_info_s                     stCorner;
    xmedia_ive_st_candi_corner_ctrl_s          stStCandiCornerCtrl;
    xmedia_ive_st_corner_ctrl_s                  stStCornerCtrl;

    xmedia_ive_image_s                         stPyrTmp;
    xmedia_ive_image_s                          stSrcYuv;
    FILE*                                 pFpSrc;

    //xmedia_ive_dst_image_s  astDst;
    //xmedia_ive_src_image_s  stSrcYUV;
    //xmedia_ive_src_image_s     astPrePyr[3];
    //xmedia_ive_src_image_s     astCurPyr[3];
    //xmedia_ive_image_s stPyrTmp;
    //xmedia_ive_dst_mem_info_s stDstCorner;
    //IVE_MEM_INFO_S   stMv;
    //IVE_SRC_MEM_INFO_S astPoint[3];

} sample_ive_st_lk_s;

static sample_ive_st_lk_s s_stStLk;

xmedia_u16 sample_ive_calc_stride(xmedia_u32 u32Width, xmedia_u8 u8Align)
{
    return (u32Width + (u8Align - u32Width % u8Align) % u8Align);
}


static xmedia_s32 sample_ive_st_lk_dma(xmedia_ive_handle *pIveHandle, xmedia_ive_src_image_s *pstSrc,
                                            xmedia_ive_dst_image_s *pstDst,xmedia_ive_dma_ctrl_s *pstDmaCtrl, xmedia_bool bInstant)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_ive_src_data_s stDataSrc;
    xmedia_ive_dst_data_s stDataDst;

    stDataSrc.u64viraddr     = pstSrc->au64viraddr[0];
    stDataSrc.u64phyaddr     = pstSrc->au64phyaddr[0];
    stDataSrc.u32width         = pstSrc->u32width;
    stDataSrc.u32height     = pstSrc->u32height;
    stDataSrc.u32stride     = pstSrc->au32stride[0];

    stDataDst.u64viraddr     = pstDst->au64viraddr[0];
    stDataDst.u64phyaddr     = pstDst->au64phyaddr[0];
    stDataDst.u32width         = pstDst->u32width;
    stDataDst.u32height     = pstDst->u32height;
    stDataDst.u32stride     = pstDst->au32stride[0];
    s32Ret = xmedia_ive_dma(pIveHandle, &stDataSrc, &stDataDst,pstDmaCtrl,bInstant);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32Ret,s32Ret,"Error(%#x),xmedia_ive_dma failed!\n",s32Ret);

    return s32Ret;
}
/******************************************************************************
* function : Copy pyr
******************************************************************************/
static xmedia_void sample_ive_st_lk_copyphy(xmedia_ive_src_image_s astPyrSrc[], xmedia_ive_dst_image_s astPyrDst[], xmedia_u8 u8MaxLevel)
{
    xmedia_u8 i;
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_ive_handle hIveHandle;

    xmedia_ive_dma_ctrl_s stCtrl;
    memset(&stCtrl,0,sizeof(stCtrl));
    stCtrl.en_mode = XMEDIA_IVE_DMA_MODE_DIRECT_COPY;

    for (i = 0; i <= u8MaxLevel; i++)
    {
        s32Ret = sample_ive_st_lk_dma(&hIveHandle,&astPyrSrc[i],&astPyrDst[i],&stCtrl,XMEDIA_FALSE);
        if (XMEDIA_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("sample_ive_st_lk_dma fail,Error(%d)\n",s32Ret);
            break;
        }
    }
}

/******************************************************************************
* function : St lk uninit
******************************************************************************/
static xmedia_void sample_ive_st_lk_uninit(sample_ive_st_lk_s* pstStLk)
{
    xmedia_u16 i;
    for (i = 0; i <= pstStLk->stLkPyrCtrl.u8_max_level; i++)
    {
        IVE_MMZ_FREE(pstStLk->astPrevPyr[i].au64phyaddr[0], pstStLk->astPrevPyr[i].au64viraddr[0]);
        IVE_MMZ_FREE(pstStLk->astNextPyr[i].au64phyaddr[0], pstStLk->astNextPyr[i].au64viraddr[0]);
    }

    IVE_MMZ_FREE(pstStLk->stPrevPts.u64phyaddr, pstStLk->stPrevPts.u64viraddr);
    IVE_MMZ_FREE(pstStLk->stNextPts.u64phyaddr, pstStLk->stNextPts.u64viraddr);
    IVE_MMZ_FREE(pstStLk->stStatus.u64phyaddr, pstStLk->stStatus.u64viraddr);
    IVE_MMZ_FREE(pstStLk->stErr.u64phyaddr, pstStLk->stErr.u64viraddr);

    IVE_MMZ_FREE(pstStLk->stStSrc.au64phyaddr[0], pstStLk->stStSrc.au64viraddr[0]);
    IVE_MMZ_FREE(pstStLk->stStDst.au64phyaddr[0], pstStLk->stStDst.au64viraddr[0]);
    IVE_MMZ_FREE(pstStLk->stCorner.u64phyaddr, pstStLk->stCorner.u64viraddr);

    IVE_MMZ_FREE(pstStLk->stStCandiCornerCtrl.stmem.u64phyaddr, pstStLk->stStCandiCornerCtrl.stmem.u64viraddr);

    IVE_MMZ_FREE(pstStLk->stPyrTmp.au64phyaddr[0], pstStLk->stPyrTmp.au64viraddr[0]);
    IVE_MMZ_FREE(pstStLk->stSrcYuv.au64phyaddr[0], pstStLk->stSrcYuv.au64viraddr[0]);

    IVE_CLOSE_FILE(pstStLk->pFpSrc);
}

/******************************************************************************
* function : St lk init
******************************************************************************/
static xmedia_s32 sample_ive_st_lk_init(sample_ive_st_lk_s* pstStLk,
                                    xmedia_char* pchSrcFileName,
                                    xmedia_u32 u32Width,
                                    xmedia_u32 u32Height,
                                    xmedia_u32 u32PyrWidth,
                                    xmedia_u32 u32PyrHeight,
                                    xmedia_u8 u8MaxLevel)
{
    xmedia_s32 s32Ret = XMEDIA_SUCCESS;
    xmedia_u32 u32Size = 0;
    xmedia_u16 i;

    memset(pstStLk, 0, sizeof(sample_ive_st_lk_s));

    pstStLk->stLkPyrCtrl.en_out_mode = XMEDIA_IVE_LK_OPTICAL_FLOW_PYR_OUT_MODE_BOTH;
    pstStLk->stLkPyrCtrl.b_use_init_flow = XMEDIA_TRUE;
    pstStLk->stLkPyrCtrl.u16_pts_num= MAX_POINT_NUM;
    pstStLk->stLkPyrCtrl.u8_max_level = u8MaxLevel;
    pstStLk->stLkPyrCtrl.u0q8_min_eigthr = 100;
    pstStLk->stLkPyrCtrl.u8_iter_cnt = 10;
    pstStLk->stLkPyrCtrl.u0q8_eps = 2;
    //Init Pyr
    for(i = 0; i <= u8MaxLevel; i++)
    {
        s32Ret = sample_comm_ive_create_image(&pstStLk->astPrevPyr[i], XMEDIA_IVE_IMAGE_TYPE_U8C1,
            u32Width >> i, u32Height >> i);
        SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, ST_LK_INIT_FAIL,
            "Error(%#x),Create prevPyr[%d] image failed!\n", s32Ret,i);

        s32Ret = sample_comm_ive_create_image(&pstStLk->astNextPyr[i], XMEDIA_IVE_IMAGE_TYPE_U8C1,
            pstStLk->astPrevPyr[i].u32width, pstStLk->astPrevPyr[i].u32height);
        SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, ST_LK_INIT_FAIL,
            "Error(%#x),Create nextPyr[%d] image failed!\n", s32Ret,i);
    }
    //Init prev pts
    u32Size = sizeof(xmedia_ive_point_s25q7_s) * MAX_POINT_NUM;
    u32Size = sample_ive_calc_stride(u32Size, IVE_ALIGN);
    s32Ret = sample_comm_ive_create_mem_info(&(pstStLk->stPrevPts), u32Size);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, ST_LK_INIT_FAIL,
        "Error(%#x),Create prevPts mem info failed!\n", s32Ret);
    //Init next pts
    s32Ret = sample_comm_ive_create_mem_info(&(pstStLk->stNextPts), u32Size);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, ST_LK_INIT_FAIL,
        "Error(%#x),Create nextPts mem info failed!\n", s32Ret);

    //Init status
    u32Size = sizeof(xmedia_u8) * MAX_POINT_NUM;
    u32Size = sample_ive_calc_stride(u32Size, IVE_ALIGN);
    s32Ret = sample_comm_ive_create_mem_info(&(pstStLk->stStatus), u32Size);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, ST_LK_INIT_FAIL,
        "Error(%#x),Create status mem info failed!\n", s32Ret);

    //Init errXMEDIA_U9Q7
    u32Size = sizeof(xmedia_u9q7) * MAX_POINT_NUM;
    u32Size = sample_ive_calc_stride(u32Size, IVE_ALIGN);
    s32Ret = sample_comm_ive_create_mem_info(&(pstStLk->stErr), u32Size);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, ST_LK_INIT_FAIL,
        "Error(%#x),Create err mem info failed!\n", s32Ret);

    //Init St
    s32Ret = sample_comm_ive_create_image(&pstStLk->stStSrc, XMEDIA_IVE_IMAGE_TYPE_U8C1, u32Width, u32Height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, ST_LK_INIT_FAIL,
        "Error(%#x),Create src image failed!\n", s32Ret);

    s32Ret = sample_comm_ive_create_image(&pstStLk->stStDst, XMEDIA_IVE_IMAGE_TYPE_U8C1, u32Width, u32Height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, ST_LK_INIT_FAIL,
        "Error(%#x),Create dst image failed!\n", s32Ret);

    pstStLk->stStCandiCornerCtrl.u0q8_quality_level = 25;
    u32Size = 4 * sample_ive_calc_stride(u32Width, IVE_ALIGN) * u32Height + sizeof(xmedia_ive_st_max_eig_s);
    s32Ret = sample_comm_ive_create_mem_info(&(pstStLk->stStCandiCornerCtrl.stmem), u32Size);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, ST_LK_INIT_FAIL,
        "Error(%#x),Create CandiCornerCtrl.stMem mem info failed!\n", s32Ret);

    u32Size = sizeof(xmedia_ive_st_corner_info_s);
    s32Ret = sample_comm_ive_create_mem_info(&(pstStLk->stCorner), u32Size);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, ST_LK_INIT_FAIL,
        "Error(%#x),Create corner mem info failed!\n", s32Ret);

    pstStLk->stStCornerCtrl.u16_max_corner_num = MAX_POINT_NUM;
    pstStLk->stStCornerCtrl.u16_min_dist = MIN_DIST;

    s32Ret = sample_comm_ive_create_image(&pstStLk->stPyrTmp, XMEDIA_IVE_IMAGE_TYPE_U8C1, u32PyrWidth, u32PyrHeight);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, ST_LK_INIT_FAIL,
        "Error(%#x),Create pyrTmp image failed!\n", s32Ret);

    s32Ret = sample_comm_ive_create_image(&pstStLk->stSrcYuv, XMEDIA_IVE_IMAGE_TYPE_YUV420SP, u32Width, u32Height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, ST_LK_INIT_FAIL,
        "Error(%#x),Create srcYuv image failed!\n", s32Ret);

    s32Ret = XMEDIA_FAILURE;
    pstStLk->pFpSrc = fopen(pchSrcFileName, "rb");
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_NULL == pstStLk->pFpSrc, ST_LK_INIT_FAIL,
        "Error,Open file %s failed!\n", pchSrcFileName);

    s32Ret = XMEDIA_SUCCESS;

ST_LK_INIT_FAIL:

    if (XMEDIA_SUCCESS != s32Ret)
    {
        sample_ive_st_lk_uninit(pstStLk);
    }
    return s32Ret;
}
/******************************************************************************
* function : Pyr down
******************************************************************************/
static xmedia_s32 sample_ive_st_lk_pyrdown(sample_ive_st_lk_s* pstStLk, xmedia_ive_src_image_s* pstSrc, xmedia_ive_dst_image_s* pstDst)
{
    xmedia_s32 s32Ret = XMEDIA_SUCCESS;
    xmedia_ive_handle hIveHandle;
    xmedia_ive_dma_ctrl_s  stDmaCtrl = {XMEDIA_IVE_DMA_MODE_INTERVAL_COPY,
                                 0, 2, 1, 2
                                };
    xmedia_ive_filter_ctrl_s stFltCtrl = {{
            1, 2, 3, 2, 1,
            2, 5, 6, 5, 2,
            3, 6, 8, 6, 3,
            2, 5, 6, 5, 2,
            1, 2, 3, 2, 1
        }, 7
    };

    pstStLk->stPyrTmp.u32width = pstSrc->u32width;
    pstStLk->stPyrTmp.u32height = pstSrc->u32height;

    s32Ret = xmedia_ive_filter(&hIveHandle, pstSrc, &pstStLk->stPyrTmp, &stFltCtrl,  XMEDIA_FALSE);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32Ret,s32Ret,"Error(%#x),xmedia_ive_filter failed!\n",s32Ret);

    s32Ret = sample_ive_st_lk_dma(&hIveHandle,&pstStLk->stPyrTmp,pstDst,&stDmaCtrl,XMEDIA_FALSE);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32Ret,s32Ret,"Error(%#x),sample_ive_st_lk_dma failed!\n",s32Ret);

    return s32Ret;
}


/******************************************************************************
* function : St lk proc
******************************************************************************/
static xmedia_s32 sample_ive_st_lk_proc(sample_ive_st_lk_s* pstStLk)
{
    xmedia_u32 u32FrameNum = 10;
    xmedia_u32 i, k;
    xmedia_u16 u16RectNum;
    xmedia_s32 s32Ret = XMEDIA_SUCCESS;
    xmedia_bool bBlock = XMEDIA_TRUE;
    xmedia_bool bFinish = XMEDIA_FALSE;
    xmedia_ive_handle hIveHandle;
    xmedia_ive_dma_ctrl_s stDmaCtrl;
    xmedia_ive_st_corner_info_s* pstCornerInfo = SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(xmedia_ive_st_corner_info_s,pstStLk->stCorner.u64viraddr);
    xmedia_ive_point_s25q7_s *psts25q7NextPts = SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(xmedia_ive_point_s25q7_s, pstStLk->stNextPts.u64viraddr);

    memset(&stDmaCtrl,0,sizeof(stDmaCtrl));
    stDmaCtrl.en_mode = XMEDIA_IVE_DMA_MODE_DIRECT_COPY;

    for (i = 0; i < u32FrameNum; i++)
    {
        SAMPLE_PRT("Proc frame %d\n", i);
        s32Ret = sample_comm_ive_read_file(&pstStLk->stSrcYuv, pstStLk->pFpSrc);
        SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32Ret,s32Ret,"Error(%#x),Read src file failed!\n",s32Ret);

        s32Ret = sample_ive_st_lk_dma(&hIveHandle,&pstStLk->stSrcYuv,&pstStLk->astNextPyr[0],&stDmaCtrl,XMEDIA_FALSE);
        SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32Ret,s32Ret,"Error(%#x),sample_ive_st_lk_dma failed!\n",s32Ret);

        //buid pyr
        for (k = 1; k <= pstStLk->stLkPyrCtrl.u8_max_level; k++)
        {
            s32Ret = sample_ive_st_lk_pyrdown(pstStLk, &pstStLk->astNextPyr[k - 1], &pstStLk->astNextPyr[k]);
            SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32Ret,s32Ret,"Error(%#x),sample_ive_st_lk_pyrdown level %d failed!\n",s32Ret,k);
        }

        if (0 == i)
        {
            s32Ret = xmedia_ive_st_candi_corner(&hIveHandle, &pstStLk->astNextPyr[0], &pstStLk->stStDst,
                                              &pstStLk->stStCandiCornerCtrl, XMEDIA_TRUE);
            SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32Ret,s32Ret,"Error(%#x),xmedia_ive_st_candi_corner failed!\n",s32Ret);

            s32Ret = xmedia_ive_query(hIveHandle, &bFinish, bBlock);
            while (ERR_CODE_IVE_QUERY_TIMEOUT == s32Ret)
            {
                usleep(100);
                s32Ret = xmedia_ive_query(hIveHandle, &bFinish, bBlock);
            }
            SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32Ret,s32Ret,"Error(%#x),xmedia_ive_query failed!\n",s32Ret);

            s32Ret = xmedia_ive_st_corner(&pstStLk->stStDst, &pstStLk->stCorner, &pstStLk->stStCornerCtrl);
            SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32Ret,s32Ret,"Error(%#x),xmedia_ive_st_corner failed!\n",s32Ret);

            pstStLk->stLkPyrCtrl.u16_pts_num = pstCornerInfo->u16_corner_num;
            for (k = 0; k < pstStLk->stLkPyrCtrl.u16_pts_num; k++)
            {
                psts25q7NextPts[k].s25q7x = (xmedia_s32)(pstCornerInfo->ast_corner[k].u16x << 7);
                psts25q7NextPts[k].s25q7y = (xmedia_s32)(pstCornerInfo->ast_corner[k].u16y << 7);
            }

        }
        else
        {
            s32Ret = xmedia_ive_lk_optical_flow_pyr(&hIveHandle,
                    pstStLk->astPrevPyr, pstStLk->astNextPyr,
                    &pstStLk->stPrevPts, &pstStLk->stNextPts,
                    &pstStLk->stStatus, &pstStLk->stErr,
                    &pstStLk->stLkPyrCtrl,XMEDIA_TRUE);
            SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32Ret,s32Ret,"Error(%#x),xmedia_ive_lk_optical_flow_pyr failed!\n",s32Ret);

            s32Ret = xmedia_ive_query(hIveHandle, &bFinish, bBlock);
            while (ERR_CODE_IVE_QUERY_TIMEOUT == s32Ret)
            {
                usleep(100);
                s32Ret = xmedia_ive_query(hIveHandle, &bFinish, bBlock);
            }
            SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32Ret,s32Ret,"Error(%#x),xmedia_ive_query failed!\n",s32Ret);

            u16RectNum = 0;
            for (k = 0; k < pstStLk->stLkPyrCtrl.u16_pts_num; k++)
            {
                if(! (SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(xmedia_u8,pstStLk->stStatus.u64viraddr))[k])
                {
                    continue;
                }
                psts25q7NextPts[u16RectNum].s25q7x = psts25q7NextPts[k].s25q7x;
                psts25q7NextPts[u16RectNum].s25q7y = psts25q7NextPts[k].s25q7y;
                u16RectNum++;
            }

            pstStLk->stLkPyrCtrl.u16_pts_num = u16RectNum;

        }

        memcpy(SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(xmedia_void,pstStLk->stPrevPts.u64viraddr),
            SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(xmedia_void,pstStLk->stNextPts.u64viraddr),
            sizeof(xmedia_ive_point_s25q7_s) * pstStLk->stLkPyrCtrl.u16_pts_num);

        sample_ive_st_lk_copyphy(pstStLk->astNextPyr, pstStLk->astPrevPyr, pstStLk->stLkPyrCtrl.u8_max_level);
    }

    return s32Ret;
}
/******************************************************************************
* function : show St Lk sample
******************************************************************************/
xmedia_void sample_ive_st_lk(xmedia_void)
{
    xmedia_char* pchSrcFileName = "./data/input/stlk/st_lk_720x576_420sp.yuv";
    xmedia_u16 u16Width = 720;
    xmedia_u16 u16Height = 576;
    xmedia_u16 u16PyrWidth = 720;
    xmedia_u16 u16PyrHeight = 576;

    xmedia_s32 s32Ret;
    xmedia_u8 u8MaxLevel = 3;

    memset(&s_stStLk,0,sizeof(s_stStLk));


    s32Ret =  sample_ive_st_lk_init(&s_stStLk,pchSrcFileName,u16Width, u16Height,
        u16PyrWidth,u16PyrHeight, u8MaxLevel);
    SAMPLE_CHECK_EXPR(XMEDIA_SUCCESS != s32Ret, "Error(%#x),sample_ive_st_lk_init failed!\n", s32Ret);

    s32Ret = sample_ive_st_lk_proc(&s_stStLk);
    if (XMEDIA_SUCCESS == s32Ret)
    {
       SAMPLE_PRT("Process success!\n");
    }

    sample_ive_st_lk_uninit(&s_stStLk);
    memset(&s_stStLk,0,sizeof(s_stStLk));

    return ;
}

/******************************************************************************
* function : St_Lk sample signal handle
******************************************************************************/
xmedia_void sample_ive_st_lk_handle_sig(xmedia_void)
{
    sample_ive_st_lk_uninit(&s_stStLk);
    memset(&s_stStLk,0,sizeof(s_stStLk));
}



