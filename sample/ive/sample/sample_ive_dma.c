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
#include "xmedia_video_common.h"

#include "sample_comm_ive.h"

typedef struct ive_dma_crop_s
{
    xmedia_ive_src_image_s st_src;
    xmedia_ive_dst_image_s st_dst;
    xmedia_video_frame_info video_frame_src;
    xmedia_video_frame_info video_frame_dst;

    FILE* pfp_src;
    FILE* pfp_dst;
}ive_dma_crop_s;

typedef struct ive_dma_padding_s
{
    xmedia_ive_src_image_s st_src;
    xmedia_ive_dst_image_s st_dst;
    xmedia_video_frame_info video_frame_src;
    xmedia_video_frame_info video_frame_dst;

    FILE* pfp_src;
    FILE* pfp_dst;
}ive_dma_padding_s;

static ive_dma_crop_s g_s_st_crop;
static ive_dma_padding_s g_s_st_padding;

static xmedia_void sample_ive_crop_uninit(ive_dma_crop_s* pst_crop)
{
    IVE_MMZ_FREE(pst_crop->st_src.au64phyaddr[0],pst_crop->st_src.au64viraddr[0]);
    IVE_MMZ_FREE(pst_crop->st_dst.au64phyaddr[0],pst_crop->st_dst.au64viraddr[0]);
    IVE_CLOSE_FILE(pst_crop->pfp_src);
    IVE_CLOSE_FILE(pst_crop->pfp_dst);
}
static xmedia_void sample_ive_padding_uninit(ive_dma_padding_s* pst_padding)
{
    IVE_MMZ_FREE(pst_padding->st_src.au64phyaddr[0],pst_padding->st_src.au64viraddr[0]);
    IVE_MMZ_FREE(pst_padding->st_dst.au64phyaddr[0],pst_padding->st_dst.au64viraddr[0]);
    IVE_CLOSE_FILE(pst_padding->pfp_src);
    IVE_CLOSE_FILE(pst_padding->pfp_dst);
}

static xmedia_s32 sample_ive_padding_proc(xmedia_video_frame_info* src,xmedia_video_frame_info* dst,
                                          xmedia_u32 dst_start_x,xmedia_u32 dst_start_y )
{
    xmedia_s32 s32_ret;
    xmedia_ive_handle ive_handle;
    xmedia_bool b_instant = XMEDIA_FALSE;
    xmedia_bool b_block = XMEDIA_TRUE;
    xmedia_bool b_finish = XMEDIA_FALSE;
    xmedia_ive_dma_ctrl_s st_dma_ctrl;

    st_dma_ctrl.en_mode = XMEDIA_IVE_DMA_MODE_DIRECT_COPY;

    if(((src->frame.height + dst_start_y) > dst->frame.height)
        || ((src->frame.width + dst_start_x) > dst->frame.width))
    {
        s32_ret = XMEDIA_FAILURE;
        printf("Error(%#x),padding parameters error!\n",s32_ret);
        return s32_ret;
    }

    /* Y分量 */
    xmedia_ive_src_data_s st_src_data;
    xmedia_ive_dst_data_s st_dst_data;
    st_src_data.u64viraddr = 0;
    st_src_data.u32stride  = src->frame.stride.y_stride;
    st_src_data.u64phyaddr = src->frame.addr.y_phy_addr;
    st_src_data.u32width   = src->frame.width;
    st_src_data.u32height  = src->frame.height;

    st_dst_data.u64viraddr = 0;
    st_dst_data.u32stride  = dst->frame.stride.y_stride;
    st_dst_data.u64phyaddr = dst->frame.addr.y_phy_addr + dst_start_x + dst_start_y * st_dst_data.u32stride;
    st_dst_data.u32width   = src->frame.width;
    st_dst_data.u32height  = src->frame.height;

    s32_ret = xmedia_ive_dma(&ive_handle, &st_src_data, &st_dst_data,&st_dma_ctrl, b_instant);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_dma failed!\n",s32_ret);

    /* uv分量 */
    xmedia_ive_src_data_s st_src_data2;
    xmedia_ive_dst_data_s st_dst_data2;
    st_src_data2.u64viraddr = 0 ;
    st_src_data2.u32stride  = src->frame.stride.c_stride;
    st_src_data2.u64phyaddr = src->frame.addr.c_phy_addr;
    st_src_data2.u32width   = src->frame.width;
    st_src_data2.u32height  = src->frame.height / 2;
    printf("st_src_data2.u32stride: %d\n",st_src_data2.u32stride);

    st_dst_data2.u64viraddr = 0 ;
    st_dst_data2.u32stride = dst->frame.stride.c_stride;
    st_dst_data2.u64phyaddr = dst->frame.addr.c_phy_addr +
                                dst_start_x + (dst_start_y / 2 ) * st_dst_data2.u32stride;

    st_dst_data2.u32width = src->frame.width;
    st_dst_data2.u32height =src->frame.height / 2;
    printf("st_dst_data2.u32stride: %d\n",st_dst_data2.u32stride);

    b_instant = XMEDIA_TRUE;
    s32_ret = xmedia_ive_dma(&ive_handle, &st_src_data2, &st_dst_data2,&st_dma_ctrl, b_instant);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_dma failed!\n",s32_ret);

    s32_ret = xmedia_ive_query(ive_handle, &b_finish, b_block);
    while (ERR_CODE_IVE_QUERY_TIMEOUT == s32_ret)
    {
        usleep(100);
        s32_ret = xmedia_ive_query(ive_handle, &b_finish, b_block);
    }
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_query failed!\n",s32_ret);

    return s32_ret;
}

static xmedia_s32 sample_ive_crop_proc(xmedia_video_frame_info* src, xmedia_video_frame_info* dst,
                                        xmedia_u32 start_x, xmedia_u32 start_y)
{
    xmedia_s32 s32_ret;
    xmedia_ive_handle ive_handle;
    xmedia_bool b_instant = XMEDIA_FALSE;
    xmedia_bool b_block = XMEDIA_TRUE;
    xmedia_bool b_finish = XMEDIA_FALSE;
    xmedia_ive_dma_ctrl_s st_dma_ctrl;

    st_dma_ctrl.en_mode = XMEDIA_IVE_DMA_MODE_DIRECT_COPY;

    /* Y分量 */
    xmedia_ive_src_data_s st_src_data;
    xmedia_ive_dst_data_s st_dst_data;
    st_src_data.u64viraddr = 0;
    st_src_data.u64phyaddr = src->frame.addr.y_phy_addr + start_x + start_y * (src->frame.stride.y_stride);
    st_src_data.u32stride  = src->frame.stride.y_stride;
    st_src_data.u32width   = dst->frame.width;
    st_src_data.u32height  = dst->frame.height;

    st_dst_data.u64viraddr = 0;
    st_dst_data.u64phyaddr = dst->frame.addr.y_phy_addr;
    st_dst_data.u32stride  = dst->frame.stride.y_stride;
    st_dst_data.u32width   = dst->frame.width;
    st_dst_data.u32height  =  dst->frame.height;

    s32_ret = xmedia_ive_dma(&ive_handle, &st_src_data, &st_dst_data,&st_dma_ctrl, b_instant);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_dma failed!\n",s32_ret);

    /* uv分量 */
    xmedia_ive_src_data_s st_src_data2;
    xmedia_ive_dst_data_s st_dst_data2;
    st_src_data2.u64viraddr = 0;
    st_src_data2.u64phyaddr = src->frame.addr.c_phy_addr + start_x + (start_y / 2 ) * src->frame.stride.c_stride;
    st_src_data2.u32stride = src->frame.stride.c_stride;
    st_src_data2.u32width  = dst->frame.width;
    st_src_data2.u32height = dst->frame.height / 2;
    printf("st_src_data2.u32stride: %d\n",st_src_data2.u32stride);

    st_dst_data2.u64viraddr = 0;
    st_dst_data2.u64phyaddr = dst->frame.addr.c_phy_addr;
    st_dst_data2.u32stride =  dst->frame.stride.c_stride;
    printf("st_dst_data2.u32stride: %d\n",st_dst_data2.u32stride);
    st_dst_data2.u32width = dst->frame.width;
    st_dst_data2.u32height = dst->frame.height / 2;

    b_instant = XMEDIA_TRUE;
    s32_ret = xmedia_ive_dma(&ive_handle, &st_src_data2, &st_dst_data2,&st_dma_ctrl, b_instant);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_dma failed!\n",s32_ret);

    s32_ret = xmedia_ive_query(ive_handle, &b_finish, b_block);
    while (ERR_CODE_IVE_QUERY_TIMEOUT == s32_ret)
    {
        usleep(100);
        s32_ret = xmedia_ive_query(ive_handle, &b_finish, b_block);
    }
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_query failed!\n",s32_ret);

    return s32_ret;
}

static xmedia_s32 sample_ive_padding_init(ive_dma_padding_s* pst_dma_padding, xmedia_char* pch_src_file_name,
                                    xmedia_char* pch_dst_file_name, xmedia_u32 u32width, xmedia_u32 u32height,
                                                                    xmedia_u32 dst_width,xmedia_u32 dst_height)
{
    xmedia_s32 s32_ret = XMEDIA_FAILURE;
    xmedia_u8 *pu8_tmp;
    xmedia_u16 i,j;

    s32_ret = sample_comm_ive_create_image(&(pst_dma_padding->st_src), XMEDIA_IVE_IMAGE_TYPE_YUV420SP,
                                            u32width, u32height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, DMA_PADDING_INIT_FAIL,
        "Error(%#x),Create src image failed!\n", s32_ret);

    s32_ret = sample_comm_ive_create_image(&(pst_dma_padding->st_dst),XMEDIA_IVE_IMAGE_TYPE_YUV420SP,
                                                dst_width, dst_height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, DMA_PADDING_INIT_FAIL,
        "Error(%#x),Create dst image failed!\n", s32_ret);

    /* 初始化目标图片为全黑 */
    /* 初始化y平面为0 */
    pu8_tmp = SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(xmedia_u8,pst_dma_padding->st_dst.au64viraddr[0]);
    for (i = 0; i < pst_dma_padding->st_dst.u32height; i++)
    {
        for (j = 0; j < pst_dma_padding->st_dst.u32width; j++)
        {
            pu8_tmp[j]        = 0;
        }
        pu8_tmp += pst_dma_padding->st_dst.au32stride[0];
    }
    /* 初始化u,v平面为128 */
    pu8_tmp = SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(xmedia_u8,pst_dma_padding->st_dst.au64viraddr[1]);
    for (i = 0; i < pst_dma_padding->st_dst.u32height/2; i++)
    {
        for (j = 0; j < pst_dma_padding->st_dst.u32width; j++)
        {
            pu8_tmp[2 * j ]        = 128;
            pu8_tmp[ 2 * j + 1]    = 128;
        }
        pu8_tmp += pst_dma_padding->st_dst.au32stride[1];
    }

    s32_ret = XMEDIA_FAILURE;
    pst_dma_padding->pfp_src = fopen(pch_src_file_name,"rb");
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_NULL == pst_dma_padding->pfp_src, DMA_PADDING_INIT_FAIL,
        "Error,Open file %s failed!\n", pch_src_file_name);

    pst_dma_padding->pfp_dst = fopen(pch_dst_file_name, "wb");
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_NULL == pst_dma_padding->pfp_dst, DMA_PADDING_INIT_FAIL,
        "Error,Open file %s failed!\n", pch_dst_file_name);

    s32_ret = sample_comm_ive_read_file(&(pst_dma_padding->st_src), pst_dma_padding->pfp_src);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, DMA_PADDING_INIT_FAIL,
        "Error(%#x),Read src file failed!\n", s32_ret);
    s32_ret = XMEDIA_SUCCESS;

    pst_dma_padding->video_frame_src.frame.addr.y_phy_addr = pst_dma_padding->st_src.au64phyaddr[0];
    pst_dma_padding->video_frame_src.frame.stride.y_stride = pst_dma_padding->st_src.au32stride[0];
    pst_dma_padding->video_frame_src.frame.addr.c_phy_addr = pst_dma_padding->st_src.au64phyaddr[1];
    pst_dma_padding->video_frame_src.frame.stride.c_stride = pst_dma_padding->st_src.au32stride[1];
    pst_dma_padding->video_frame_src.frame.width           = pst_dma_padding->st_src.u32width;
    pst_dma_padding->video_frame_src.frame.height          = pst_dma_padding->st_src.u32height;
    pst_dma_padding->video_frame_src.frame.pixel_fmt       = XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_420;

    pst_dma_padding->video_frame_dst.frame.addr.y_phy_addr = pst_dma_padding->st_dst.au64phyaddr[0];
    pst_dma_padding->video_frame_dst.frame.stride.y_stride = pst_dma_padding->st_dst.au32stride[0];
    pst_dma_padding->video_frame_dst.frame.addr.c_phy_addr = pst_dma_padding->st_dst.au64phyaddr[1];
    pst_dma_padding->video_frame_dst.frame.stride.c_stride = pst_dma_padding->st_dst.au32stride[1];
    pst_dma_padding->video_frame_dst.frame.width           = pst_dma_padding->st_dst.u32width;
    pst_dma_padding->video_frame_dst.frame.height          = pst_dma_padding->st_dst.u32height;
    pst_dma_padding->video_frame_dst.frame.pixel_fmt       = XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_420;
    return s32_ret;

DMA_PADDING_INIT_FAIL:

    if (XMEDIA_SUCCESS != s32_ret)
    {
        sample_ive_padding_uninit(pst_dma_padding);
    }
    return s32_ret;
}

static xmedia_s32 sample_ive_crop_init(ive_dma_crop_s* pst_dma_crop, xmedia_char* pch_src_file_name,
                         xmedia_char* pch_dst_file_name, xmedia_u32 u32width, xmedia_u32 u32height,
                                                        xmedia_u32 dst_width, xmedia_u32 dst_height)
{
    xmedia_s32 s32_ret = XMEDIA_FAILURE;

    s32_ret = sample_comm_ive_create_image(&(pst_dma_crop->st_src), XMEDIA_IVE_IMAGE_TYPE_YUV420SP,
                                             u32width, u32height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, DMA_CROP_INIT_FAIL,
        "Error(%#x),Create src image failed!\n", s32_ret);

    s32_ret = sample_comm_ive_create_image(&(pst_dma_crop->st_dst),XMEDIA_IVE_IMAGE_TYPE_YUV420SP,
                                             dst_width, dst_height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, DMA_CROP_INIT_FAIL,
        "Error(%#x),Create dst image failed!\n", s32_ret);

    s32_ret = XMEDIA_FAILURE;
    pst_dma_crop->pfp_src = fopen(pch_src_file_name,"rb");
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_NULL == pst_dma_crop->pfp_src, DMA_CROP_INIT_FAIL,
        "Error,Open file %s failed!\n", pch_src_file_name);

    pst_dma_crop->pfp_dst = fopen(pch_dst_file_name, "wb");
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_NULL == pst_dma_crop->pfp_dst, DMA_CROP_INIT_FAIL,
        "Error,Open file %s failed!\n", pch_dst_file_name);

    s32_ret = sample_comm_ive_read_file(&(pst_dma_crop->st_src), pst_dma_crop->pfp_src);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, DMA_CROP_INIT_FAIL,
        "Error(%#x),Read src file failed!\n", s32_ret);
    s32_ret = XMEDIA_SUCCESS;

    pst_dma_crop->video_frame_src.frame.addr.y_phy_addr = pst_dma_crop->st_src.au64phyaddr[0];
    pst_dma_crop->video_frame_src.frame.stride.y_stride = pst_dma_crop->st_src.au32stride[0];
    pst_dma_crop->video_frame_src.frame.addr.c_phy_addr = pst_dma_crop->st_src.au64phyaddr[1];
    pst_dma_crop->video_frame_src.frame.stride.c_stride = pst_dma_crop->st_src.au32stride[1];
    pst_dma_crop->video_frame_src.frame.width           = pst_dma_crop->st_src.u32width;
    pst_dma_crop->video_frame_src.frame.height          = pst_dma_crop->st_src.u32height;
    pst_dma_crop->video_frame_src.frame.pixel_fmt       = XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_420;

    pst_dma_crop->video_frame_dst.frame.addr.y_phy_addr = pst_dma_crop->st_dst.au64phyaddr[0];
    pst_dma_crop->video_frame_dst.frame.stride.y_stride = pst_dma_crop->st_dst.au32stride[0];
    pst_dma_crop->video_frame_dst.frame.addr.c_phy_addr = pst_dma_crop->st_dst.au64phyaddr[1];
    pst_dma_crop->video_frame_dst.frame.stride.c_stride = pst_dma_crop->st_dst.au32stride[1];
    pst_dma_crop->video_frame_dst.frame.width           = pst_dma_crop->st_dst.u32width;
    pst_dma_crop->video_frame_dst.frame.height          = pst_dma_crop->st_dst.u32height;
    pst_dma_crop->video_frame_dst.frame.pixel_fmt       = XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_420;
    return s32_ret;

DMA_CROP_INIT_FAIL:

    if (XMEDIA_SUCCESS != s32_ret)
    {
        sample_ive_crop_uninit(pst_dma_crop);
    }
    return s32_ret;
}

xmedia_void sample_ive_dma(xmedia_void)
{
    xmedia_s32 s32_ret;
    xmedia_u32 u32width  = 572;
    xmedia_u32 u32height = 428;
    xmedia_char* pch_src_file  = "./data/input/dma/dma_572_428_sp420.yuv";
    xmedia_char* pch_dst_file  = "./data/output/dma/crop_output.yuv";
    xmedia_char* pch_dst_file2 = "./data/output/dma/padding_output.yuv";

    /* dma 实现crop */
    // 目标区域的宽、高
    xmedia_u32 crop_dst_w = 200;
    xmedia_u32 crop_dst_h = 200;
    // 指定crop的初始位置
    xmedia_u32 start_x    = 101;
    xmedia_u32 start_y    = 101;
    //start_x、start_y需为偶数
    start_x = start_x + (2 - start_x % 2) % 2;
    start_y = start_y + (2 - start_y % 2) % 2;

    memset(&g_s_st_crop,0,sizeof(g_s_st_crop));
    s32_ret=sample_ive_crop_init(&g_s_st_crop, pch_src_file, pch_dst_file, u32width, u32height,crop_dst_w,crop_dst_h);
    SAMPLE_CHECK_EXPR(XMEDIA_SUCCESS != s32_ret,"Error(%#x),sample_ive_crop_init failed!\n",s32_ret);

    s32_ret = sample_ive_crop_proc(&g_s_st_crop.video_frame_src,&g_s_st_crop.video_frame_dst,start_x,start_y);
    SAMPLE_CHECK_EXPR(XMEDIA_SUCCESS != s32_ret,"Error(%#x),crop process failed!\n",s32_ret);

    s32_ret = sample_comm_ive_write_file(&g_s_st_crop.st_dst,g_s_st_crop.pfp_dst);
    SAMPLE_CHECK_EXPR(XMEDIA_SUCCESS != s32_ret,"Error(%#x),write failed!\n",s32_ret);
    if (XMEDIA_SUCCESS == s32_ret)
    {
       SAMPLE_PRT("crop Process success!\n");
    }
    sample_ive_crop_uninit(&g_s_st_crop);
    memset(&g_s_st_crop,0,sizeof(g_s_st_crop));


    /* dma 实现padding */
    // padding的目标图片的宽、高
    xmedia_u32 padding_dst_w = 800;
    xmedia_u32 padding_dst_h = 800;

    //指定padding到目标图片的初始位置
    xmedia_u32 dst_start_x = 101;
    xmedia_u32 dst_start_y = 101;

    dst_start_x = dst_start_x + (2 - dst_start_x % 2) % 2;
    dst_start_y = dst_start_y + (2 - dst_start_y % 2) % 2;

    memset(&g_s_st_padding,0,sizeof(g_s_st_padding));

    s32_ret=sample_ive_padding_init(&g_s_st_padding, pch_src_file, pch_dst_file2,
                                    u32width, u32height,padding_dst_w,padding_dst_h);
    SAMPLE_CHECK_EXPR(XMEDIA_SUCCESS != s32_ret,"Error(%#x),sample_ive_padding_init failed!\n",s32_ret);

    s32_ret = sample_ive_padding_proc(&g_s_st_padding.video_frame_src,&g_s_st_padding.video_frame_dst,
                                        dst_start_x,dst_start_y);
    SAMPLE_CHECK_EXPR(XMEDIA_SUCCESS != s32_ret,"Error(%#x),padding failed!\n",s32_ret);

    s32_ret = sample_comm_ive_write_file(&g_s_st_padding.st_dst,g_s_st_padding.pfp_dst);
    SAMPLE_CHECK_EXPR(XMEDIA_SUCCESS != s32_ret,"Error(%#x),write failed!\n",s32_ret);

    if (XMEDIA_SUCCESS == s32_ret)
    {
       SAMPLE_PRT("padding Process success!\n");
    }

    sample_ive_padding_uninit(&g_s_st_padding);
    memset(&g_s_st_padding,0,sizeof(g_s_st_padding));
}

/******************************************************************************
* function : dma sample signal handle
******************************************************************************/
xmedia_void sample_ive_dma_handle_sig(xmedia_void)
{
    sample_ive_crop_uninit(&g_s_st_crop);
    memset(&g_s_st_crop,0,sizeof(g_s_st_crop));

    sample_ive_padding_uninit(&g_s_st_padding);
    memset(&g_s_st_padding,0,sizeof(g_s_st_padding));

    sample_comm_ive_ive_mpi_exit();
}