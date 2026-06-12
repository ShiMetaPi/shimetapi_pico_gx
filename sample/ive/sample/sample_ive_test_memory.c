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

typedef struct test_memory_s
{
    xmedia_ive_src_image_s st_src1;
    xmedia_ive_src_image_s st_src2;
    xmedia_ive_dst_image_s st_dst;
    xmedia_ive_dst_mem_info_s  st_hist;
    xmedia_ive_sub_ctrl_s st_sub_ctrl;
    FILE* pfp_src;
    FILE* pfp_dst;

} test_memory_s;

static test_memory_s g_s_st_test_mem;
/******************************************************************************
* function : test memory uninit
******************************************************************************/
static xmedia_void sample_ive_test_memory_uninit(test_memory_s* pst_test_mem)
{
    IVE_MMZ_FREE(pst_test_mem->st_src1.au64phyaddr[0], pst_test_mem->st_src1.au64viraddr[0]);
    IVE_MMZ_FREE(pst_test_mem->st_src2.au64phyaddr[0], pst_test_mem->st_src2.au64viraddr[0]);
    IVE_MMZ_FREE(pst_test_mem->st_dst.au64phyaddr[0], pst_test_mem->st_dst.au64viraddr[0]);
    IVE_MMZ_FREE(pst_test_mem->st_hist.u64phyaddr, pst_test_mem->st_hist.u64viraddr);

    IVE_CLOSE_FILE(pst_test_mem->pfp_src);
    IVE_CLOSE_FILE(pst_test_mem->pfp_dst);
}
/******************************************************************************
* function : test memory init
******************************************************************************/
static xmedia_s32 sample_ive_test_memory_init(test_memory_s* pst_test_mem, xmedia_char* pch_src_file_name,
        xmedia_char* pch_dst_file_name, xmedia_u32 u32width, xmedia_u32 u32height)
{
    xmedia_s32 s32_ret = XMEDIA_FAILURE;
    xmedia_u32 u32size;

    memset(pst_test_mem, 0, sizeof(test_memory_s));

    s32_ret = sample_comm_ive_create_image(&(pst_test_mem->st_src1), XMEDIA_IVE_IMAGE_TYPE_U8C1, u32width, u32height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, TEST_MEMORY_INIT_FAIL,
        "Error(%#x),Create src1 image failed!\n", s32_ret);

    s32_ret = sample_comm_ive_create_image(&(pst_test_mem->st_src2), XMEDIA_IVE_IMAGE_TYPE_U8C1, u32width, u32height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, TEST_MEMORY_INIT_FAIL,
        "Error(%#x),Create src2 image failed!\n", s32_ret);

    s32_ret = sample_comm_ive_create_image(&(pst_test_mem->st_dst), XMEDIA_IVE_IMAGE_TYPE_U8C1, u32width, u32height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, TEST_MEMORY_INIT_FAIL,
        "Error(%#x),Create dst image failed!\n", s32_ret);

    u32size = XMEDIA_IVE_HIST_NUM * sizeof(xmedia_u32);
    s32_ret = sample_comm_ive_create_mem_info(&(pst_test_mem->st_hist), u32size);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, TEST_MEMORY_INIT_FAIL,
        "Error(%#x),Create hist mem info failed!\n", s32_ret);

    pst_test_mem->st_sub_ctrl.en_mode = XMEDIA_IVE_SUB_MODE_ABS;

    s32_ret = XMEDIA_FAILURE;
    pst_test_mem->pfp_src = fopen(pch_src_file_name, "rb");
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_NULL == pst_test_mem->pfp_src, TEST_MEMORY_INIT_FAIL,
        "Error,Open file %s failed!\n", pch_src_file_name);

    pst_test_mem->pfp_dst = fopen(pch_dst_file_name, "wb");
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_NULL == pst_test_mem->pfp_dst, TEST_MEMORY_INIT_FAIL,
        "Error,Open file %s failed!\n", pch_dst_file_name);

    s32_ret = XMEDIA_SUCCESS;

TEST_MEMORY_INIT_FAIL:

    if (XMEDIA_SUCCESS != s32_ret)
    {
        sample_ive_test_memory_uninit(pst_test_mem);
    }
    return s32_ret;
}
/******************************************************************************
* function : test memory
******************************************************************************/
static xmedia_s32 sample_ive_test_memory_proc(test_memory_s* pst_test_mem)
{
    xmedia_s32 s32_ret;
    xmedia_u32* pu32_hist;
    xmedia_u32 i;
    xmedia_ive_handle ive_handle;
    xmedia_bool b_instant = XMEDIA_FALSE;
    xmedia_bool b_block = XMEDIA_TRUE;
    xmedia_bool b_finish = XMEDIA_FALSE;
    xmedia_ive_src_data_s st_src_data;
    xmedia_ive_dst_data_s st_dst_data;
    xmedia_ive_dma_ctrl_s st_dma_ctrl;

    s32_ret = sample_comm_ive_read_file(&(pst_test_mem->st_src1), pst_test_mem->pfp_src);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),Read src file failed!\n",s32_ret);

    st_dma_ctrl.en_mode = XMEDIA_IVE_DMA_MODE_DIRECT_COPY;

    st_src_data.u64viraddr = pst_test_mem->st_src1.au64viraddr[0];
    st_src_data.u64phyaddr = pst_test_mem->st_src1.au64phyaddr[0];
    st_src_data.u32width = pst_test_mem->st_src1.u32width;
    st_src_data.u32height = pst_test_mem->st_src1.u32height;
    st_src_data.u32stride = pst_test_mem->st_src1.au32stride[0];

    st_dst_data.u64viraddr = pst_test_mem->st_src2.au64viraddr[0];
    st_dst_data.u64phyaddr = pst_test_mem->st_src2.au64phyaddr[0];
    st_dst_data.u32width = pst_test_mem->st_src2.u32width;
    st_dst_data.u32height = pst_test_mem->st_src2.u32height;
    st_dst_data.u32stride = pst_test_mem->st_src2.au32stride[0];
    s32_ret = xmedia_ive_dma(&ive_handle, &st_src_data, &st_dst_data,
                            &st_dma_ctrl, b_instant);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_dma failed!\n",s32_ret);

    s32_ret = xmedia_ive_sub(&ive_handle, &pst_test_mem->st_src1, &pst_test_mem->st_src2, &pst_test_mem->st_dst, &pst_test_mem->st_sub_ctrl, b_instant);
      SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_sub failed!\n",s32_ret);

    b_instant = XMEDIA_TRUE;
    s32_ret = xmedia_ive_hist(&ive_handle, &pst_test_mem->st_dst, &pst_test_mem->st_hist, b_instant);
      SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_hist failed!\n",s32_ret);

    s32_ret = xmedia_ive_query(ive_handle, &b_finish, b_block);
    while (ERR_CODE_IVE_QUERY_TIMEOUT == s32_ret)
    {
        usleep(100);
        s32_ret = xmedia_ive_query(ive_handle, &b_finish, b_block);
    }
      SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_query failed!\n",s32_ret);

    s32_ret = sample_comm_ive_write_file(&pst_test_mem->st_dst, pst_test_mem->pfp_dst);
      SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error,Write dst file failed!\n");

    pu32_hist = SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(xmedia_u32,pst_test_mem->st_hist.u64viraddr);

    if (pu32_hist[0] != pst_test_mem->st_src1.u32width * pst_test_mem->st_src1.u32height)
    {
        s32_ret = XMEDIA_FAILURE;
        SAMPLE_PRT("Test mem error,pu32_hist[0] = %d\n", pu32_hist[0]);
        for (i = 1; i < XMEDIA_IVE_HIST_NUM; i++)
        {
            if (pu32_hist[i] != 0)
            {
                SAMPLE_PRT("Test mem error, pu32_hist[%d] = %d\n", i, pu32_hist[i]);
            }
        }
    }

    return s32_ret;
}
/******************************************************************************
* function : Show test memory sample
******************************************************************************/
xmedia_void sample_ive_test_memory(xmedia_void)
{
    xmedia_s32 s32_ret;
    xmedia_u32 u32width = 720;
    xmedia_u32 u32height = 576;
    xmedia_char* pch_src_file = "./data/input/testmem/test_mem_in.yuv";
    xmedia_char* pch_dst_file = "./data/output/testmem/test_mem_out.yuv";

    memset(&g_s_st_test_mem,0,sizeof(g_s_st_test_mem));
    s32_ret = sample_ive_test_memory_init(&g_s_st_test_mem, pch_src_file, pch_dst_file, u32width, u32height);
    SAMPLE_CHECK_EXPR(XMEDIA_SUCCESS != s32_ret,"Error(%#x),sample_ive_test_memory_init failed!\n",s32_ret);

    s32_ret =  sample_ive_test_memory_proc(&g_s_st_test_mem);
    if (XMEDIA_SUCCESS == s32_ret)
    {
       SAMPLE_PRT("Process success!\n");
    }

    sample_ive_test_memory_uninit(&g_s_st_test_mem);
    memset(&g_s_st_test_mem,0,sizeof(g_s_st_test_mem));
}

/******************************************************************************
* function : TestMemory sample signal handle
******************************************************************************/
xmedia_void sample_ive_test_memory_handle_sig(xmedia_void)
{
    sample_ive_test_memory_uninit(&g_s_st_test_mem);
    memset(&g_s_st_test_mem,0,sizeof(g_s_st_test_mem));
    sample_comm_ive_ive_mpi_exit();
}

