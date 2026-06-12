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
#include <sys/prctl.h>
#include <math.h>
#include "sample_comm_venc.h"
#include "sample_comm_livestream.h"
#include "sample_comm_ive.h"

typedef struct sample_ive_od_s
{
    xmedia_ive_src_image_s stSrc;
    xmedia_ive_dst_image_s stInteg;
    xmedia_ive_integ_ctrl_s stIntegCtrl;
    xmedia_u32 u32W;
    xmedia_u32 u32H;
} sample_ive_od_s;

static xmedia_bool s_bStopSignal = XMEDIA_FALSE;
static pthread_t s_hIveThread = 0;
static sample_ive_od_s s_stOd;
// static sample_ive_switch_s s_stOdSwitch = {XMEDIA_FALSE,XMEDIA_TRUE};
// static sample_vi_config s_stViConfig = {0};



static sample_comm_sensor_type sample_ive_sensor_type[MAX_SENSOR_NUM] = {
    SENSOR0_TYPE,SENSOR1_TYPE,SENSOR2_TYPE,SENSOR3_TYPE,SENSOR4_TYPE
};
static  xmedia_s32 vpss_pipe = 0;
static  xmedia_s32 vpss_ochn[2] = { 0, 1 };
static  xmedia_s32 venc_chn[2] = { 0, 1 };
static  sample_venc_config venc_config = { 0 };
static  xmedia_s32 vi_pipe = 0;
static  xmedia_s32 vi_chn = 0;
static  sample_vpss_config vpss_config = { 0 };
static  sample_isp_param isp_param = { 0 };
static  xmedia_isp_config isp_config = { 0 };
static  sample_vi_config vi_config = { 0 };
xmedia_s32 sample_od_sys_init(sample_sys_config *sys_config)
{
    xmedia_s32 ret = 0;

    ret = sample_comm_sys_init(sys_config);
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_sys_init failed!\n");
        return ret;
    }

    //in online-online mode,vi and vpss must be reset at the same time
    ret = sample_comm_vi_init();
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vi_init failed!\n");
        return ret;
    }

    ret = sample_comm_vpss_init();
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vpss_init failed!\n");
        return ret;
    }

    ret = sample_comm_venc_init();
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_venc_init failed!\n");
        return ret;
    }

    return XMEDIA_SUCCESS;
}

xmedia_void sample_od_sys_exit(void)
{
    sample_comm_venc_stop_get_stream();
    sample_comm_venc_exit();
    sample_comm_vpss_exit();
    sample_comm_vi_exit();
    sample_comm_sys_exit();
}

static xmedia_s32 sample_vi_vpss_venc_init()
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 blk_size = 0;
    xmedia_video_size pic_size = { 0 };
    xmedia_video_size sub_pic_size = { 0 };
    sample_comm_video_param video_param = { 0 };
    sample_sys_config sys_config = { 0 };

    xmedia_s32 vi_dev = 0;

    xmedia_s32 sensor_type = sample_ive_sensor_type[0];
    xmedia_u32 framerate = 0;
    vi_sensor_info sensor_info = { 0 };
    xmedia_bool mirror = XMEDIA_FALSE;
    xmedia_bool flip = XMEDIA_FALSE;

    xmedia_s32 vpss_chnl = 0;
    xmedia_video_size vpss_ochn_size[VPSS_MAX_OCHN_NUM] = { 0 };
    xmedia_u32 wrap_buffer_line = 0;

    xmedia_s32 venc_chnl = 0;
    xmedia_s32 venc_chn_cnt = 2;

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);

    // sys init
    sys_config.sys_conf.pipe_mode[0].vicap_viproc_mode = XMEDIA_WORK_MODE_ONLINE;
    sys_config.sys_conf.pipe_mode[0].viproc_vpss_mode = XMEDIA_WORK_MODE_ONLINE;
    sys_config.sys_conf.pipe_mode[0].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;

    sys_config.vb_conf.max_pool_cnt = 20;

    pic_size.width = sensor_info.width;
    pic_size.height = sensor_info.height;

    /* wrap VB */
    blk_size = sample_comm_sys_get_wrap_buffer_size(pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                                    video_param.data_width, video_param.compress_mode, &wrap_buffer_line);
    sys_config.vb_conf.common_pool[0].block_size = blk_size;
    sys_config.vb_conf.common_pool[0].block_cnt = 1;
    SAMPLE_PRT("wrap vb size : %d \n", blk_size);

    sub_pic_size.width  = 640;
    sub_pic_size.height = 360;
    blk_size = sample_comm_sys_get_buffer_size(sub_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[1].block_size = blk_size;
    sys_config.vb_conf.common_pool[1].block_cnt = 3;

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_od_sys_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sys init failed !\n");
        return ret;
    }

    vi_config.dev_info[vi_dev].dev_en = XMEDIA_TRUE;
    vi_config.dev_info[vi_dev].dev_no = vi_dev;
    vi_config.dev_info[vi_dev].sensor_type = sensor_type;
    vi_config.pipe_info[vi_pipe].pipe_en = XMEDIA_TRUE;
    vi_config.pipe_info[vi_pipe].pipe_no = vi_pipe;
    vi_config.pipe_info[vi_pipe].chn_info[vi_chn].chn_en = XMEDIA_TRUE;
    vi_config.pipe_info[vi_pipe].chn_info[vi_chn].chn_no = vi_chn;
    vi_config.dev_bind_pipe[vi_dev].pipe[0] = vi_pipe;
    vi_config.dev_bind_pipe[vi_dev].pipe[1] = -1;

    isp_config.fps = framerate;
    isp_config.mode_config.work_mode = XMEDIA_ISP_WORK_MODE_MASTER;
    isp_config.mode_config.master_mode.blend_stat_enable = XMEDIA_FALSE;
    isp_config.mode_config.master_mode.slave_num = 0;
    isp_config.pixel_fmt = sensor_info.pixel_format;
    isp_config.size.height = sensor_info.height;
    isp_config.size.width = sensor_info.width;
    isp_config.wdr_mode = sensor_info.wdr_mode;

    // isp pipe init
    isp_param.pipe[vi_pipe] = vi_pipe;
    isp_param.isp_info[vi_pipe].isp_pipe_en = XMEDIA_TRUE;
    isp_param.isp_info[vi_pipe].isp_sensor_en = XMEDIA_TRUE;
    isp_param.isp_info[vi_pipe].sensor_type = sensor_type;
    isp_param.isp_info[vi_pipe].mirror = mirror;
    isp_param.isp_info[vi_pipe].flip = flip;
    memcpy(&(isp_param.isp_info[vi_pipe].isp_config), &isp_config, sizeof(xmedia_isp_config));

    // isp init
    ret = sample_comm_isp_init(&isp_param, &vi_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("isp init failed!\n");
        goto exit0;
    }

    // vi start
    ret = sample_comm_vi_start(&vi_config, &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("start vi failed!\n");
        goto exit1;
    }

    // isp start
    ret = sample_comm_isp_start(&isp_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("start isp failed!\n");
        goto exit2;
    }

    // vpss config
    ret = sample_comm_vpss_get_default_pipe_cfg(&vpss_config.pipe_info[vpss_pipe].pipe_config, pic_size, &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get default pipe cfg failed !\n");
        goto exit2;
    }

    vpss_ochn_size[0].width = sensor_info.width;
    vpss_ochn_size[0].height = sensor_info.height;
    vpss_ochn_size[1].width = sub_pic_size.width;
    vpss_ochn_size[1].height = sub_pic_size.height;

    vpss_config.pipe_info[vpss_pipe].pipe_en = XMEDIA_TRUE;
    vpss_config.pipe_info[vpss_pipe].pipe_no = vpss_pipe;

    vpss_chnl = vpss_ochn[0];
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_no = vpss_ochn[0];
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].wrap_attr.buf_line = wrap_buffer_line;
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].wrap_attr.enable = XMEDIA_TRUE;
    ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
                                                vpss_ochn_size[vpss_chnl], &video_param);
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.cmp_mode = XMEDIA_VIDEO_COMPRESS_MODE_TILE_COMPACT;
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.video_fmt = XMEDIA_VIDEO_FMT_TILE_32x4;

    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get default pipe cfg failed !\n");
        goto exit2;
    }

    vpss_chnl = vpss_ochn[1];
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_no = vpss_ochn[1];
    ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
                                                vpss_ochn_size[vpss_chnl], &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get default pipe cfg failed !\n");
        goto exit2;
    }

    // vpss start
    ret = sample_comm_vpss_start(&vpss_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vpss start failed !\n");
        goto exit2;
    }

    //venc init
    app_liveserver_init();
    app_liveserver_start(vpss_ochn_size,venc_chn_cnt,framerate);

    venc_chnl = venc_chn[0];
    venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
    venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
    venc_config.chn_info[venc_chnl].payload_type = PT_H265;
    venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
    sample_comm_venc_get_default_chn_info(vpss_ochn_size[0], framerate, &venc_config.chn_info[venc_chnl]);

    venc_chnl = venc_chn[1];
    venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
    venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
    venc_config.chn_info[venc_chnl].payload_type = PT_H265;
    venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
    sample_comm_venc_get_default_chn_info(vpss_ochn_size[1], framerate, &venc_config.chn_info[venc_chnl]);

    ret = sample_comm_venc_start(&venc_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start failed !\n");
        goto exit3;
    }

    ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vpss bind venc failed !\n");
        goto exit4;
    }

    ret = sample_comm_venc_start_get_stream(venc_chn, venc_chn_cnt, XMEDIA_FALSE, XMEDIA_FALSE, ".");
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start get stream failed !\n");
        goto exit5;
    }

    return ret;
exit5:
    sample_comm_venc_stop_get_stream();
    app_liveserver_stop();
    app_liveserver_deinit();
    sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
exit4:
    sample_comm_venc_stop(&venc_config);
exit3:
    sample_comm_vpss_stop(&vpss_config);
exit2:
    sample_comm_isp_stop(&isp_param);
exit1:
    sample_comm_vi_stop(&vi_config);
exit0:
    sample_comm_isp_exit(&isp_param);
    sample_od_sys_exit();
    return ret;
}

static xmedia_s32 sample_vi_vpss_venc_uninit()
{

    sample_comm_venc_stop_get_stream();
    app_liveserver_stop();
    app_liveserver_deinit();

    sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);

    sample_comm_venc_stop(&venc_config);

    sample_comm_vpss_stop(&vpss_config);

    sample_comm_isp_stop(&isp_param);

    sample_comm_vi_stop(&vi_config);

    sample_comm_isp_exit(&isp_param);
    sample_od_sys_exit();

    return 0;
}

static xmedia_void sample_ive_od_uninit(sample_ive_od_s* pstOd)
{
    IVE_MMZ_FREE(pstOd->stSrc.au64phyaddr[0], pstOd->stSrc.au64viraddr[0]);
    IVE_MMZ_FREE(pstOd->stInteg.au64phyaddr[0], pstOd->stInteg.au64viraddr[0]);

    SAMPLE_PRT("ive_od_uninit successfully!!! \n");
}

static xmedia_s32 sample_ive_od_init(sample_ive_od_s* pstOd, xmedia_u32 u32Width, xmedia_u32 u32Height)
{
    xmedia_s32 s32Ret = XMEDIA_SUCCESS;

    memset(pstOd, 0, sizeof(sample_ive_od_s));

    s32Ret = sample_comm_ive_create_image(&pstOd->stSrc, XMEDIA_IVE_IMAGE_TYPE_U8C1, u32Width, u32Height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, OD_INIT_FAIL,
        "Error(%#x),Create src image failed!\n", s32Ret);

    s32Ret = sample_comm_ive_create_image(&pstOd->stInteg, XMEDIA_IVE_IMAGE_TYPE_U64C1, u32Width, u32Height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, OD_INIT_FAIL,
        "Error(%#x),Create integ image failed!\n", s32Ret);

    pstOd->stIntegCtrl.en_out_ctrl = XMEDIA_IVE_INTEG_OUT_CTRL_COMBINE;

    pstOd->u32W = u32Width / IVE_CHAR_CALW;
    pstOd->u32H = u32Height / IVE_CHAR_CALH;


OD_INIT_FAIL:

    if (XMEDIA_SUCCESS != s32Ret)
    {
        sample_ive_od_uninit(pstOd);
    }
    return s32Ret;

}

static xmedia_s32 sample_ive_linear_2d_classifer(xmedia_video_point* pstChar, xmedia_s32 s32CharNum,
        xmedia_video_point* pst_linear_point, xmedia_s32 s32Linearnum )
{
    xmedia_s32 s32ResultNum;
    xmedia_s32 i, j;
    xmedia_bool bTestFlag;
    xmedia_video_point* pstNextLinearPoint;

    s32ResultNum = 0;
    pstNextLinearPoint = &pst_linear_point[1];
    for (i = 0; i < s32CharNum; i++)
    {
        bTestFlag = XMEDIA_TRUE;
        for (j = 0; j < (s32Linearnum - 1); j++)
        {
            if ( ( (pstChar[i].y - pst_linear_point[j].y) * (pstNextLinearPoint[j].x - pst_linear_point[j].x) >
                   (pstChar[i].x - pst_linear_point[j].x) * (pstNextLinearPoint[j].y - pst_linear_point[j].y)
                   && (pstNextLinearPoint[j].x != pst_linear_point[j].x))
                 || ( (pstChar[i].x > pst_linear_point[j].x) && (pstNextLinearPoint[j].x == pst_linear_point[j].x) ))
            {
                bTestFlag = XMEDIA_FALSE;
                break;
            }

        }
        if (XMEDIA_TRUE == bTestFlag)
        {
            s32ResultNum++;
        }
    }
    return s32ResultNum;
}


static xmedia_void* sample_ive_od_proc(xmedia_void* pArgs)
{
    xmedia_s32 s32Ret;
    xmedia_u32 i, j;
    sample_ive_od_s* pstOd;
    xmedia_video_frame_info stExtFrmInfo;
    xmedia_s32 s32VpssGrp = 0;
    xmedia_s32 as32VpssChn[] = {XMEDIA_VPSS_ICHN0, XMEDIA_VPSS_ICHN1};
    xmedia_s32 s32MilliSec = 20000;
    xmedia_ive_src_data_s stSrc;
    xmedia_ive_dst_data_s stDst;
    xmedia_ive_handle IveHandle;
    xmedia_bool bFinish = XMEDIA_FALSE;
    xmedia_bool bBlock = XMEDIA_TRUE;
    xmedia_bool bInstant = XMEDIA_TRUE;

    xmedia_video_point stChar[IVE_CHAR_NUM];
    xmedia_video_point astPoints[10] = {{0, 0}};
    ive_linear_data_s stIveLinerData;
    xmedia_u64* pu64VirData = XMEDIA_NULL;
    xmedia_ive_dma_ctrl_s stDmaCtrl = {XMEDIA_IVE_DMA_MODE_DIRECT_COPY, 0, 0, 0, 0};

    xmedia_u64 u64TopLeft, u64TopRight, u64BtmLeft, u64BtmRight;
    xmedia_u64* pu64TopRow, *pu64BtmRow;
    xmedia_u64 u64BlockSum, u64BlockSq;
    xmedia_float fSqVar;

    pstOd = (sample_ive_od_s*)(pArgs);
    pu64VirData = SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(xmedia_u64,pstOd->stInteg.au64viraddr[0]);

    stIveLinerData.pst_linear_point = &astPoints[0];
    stIveLinerData.s32_linear_num = 2;
    stIveLinerData.s32_thresh_num = IVE_CHAR_NUM / 2;
    stIveLinerData.pst_linear_point[0].x = 80;
    stIveLinerData.pst_linear_point[0].y = 0;
    stIveLinerData.pst_linear_point[1].x = 80;
    stIveLinerData.pst_linear_point[1].y = 20;

    while (XMEDIA_FALSE == s_bStopSignal)
    {
        s32Ret = xmedia_vpss_acquire_ochn_frame(s32VpssGrp, as32VpssChn[1], &stExtFrmInfo, s32MilliSec);
        if(XMEDIA_SUCCESS != s32Ret)
        {
          SAMPLE_PRT("Error(%#x),xmedia_vpss_acquire_ochn_frame failed, VPSS_GRP(%d), VPSS_CHN(%d)!\n",
              s32Ret,s32VpssGrp, as32VpssChn[1]);
          continue;
        }

        stSrc.u64viraddr = (xmedia_ulong)xmedia_mmz_map(stExtFrmInfo.frame.addr.y_phy_addr, (stExtFrmInfo.frame.stride.y_stride * stExtFrmInfo.frame.height), 0);
        stSrc.u64phyaddr = stExtFrmInfo.frame.addr.y_phy_addr;
        stSrc.u32stride = stExtFrmInfo.frame.stride.y_stride;
        stSrc.u32width = stExtFrmInfo.frame.width;
        stSrc.u32height = stExtFrmInfo.frame.height;

        stDst.u64viraddr = pstOd->stSrc.au64viraddr[0];
        stDst.u64phyaddr = pstOd->stSrc.au64phyaddr[0];
        stDst.u32stride = stExtFrmInfo.frame.stride.y_stride;
        stDst.u32width =  stExtFrmInfo.frame.width;
        stDst.u32height = stExtFrmInfo.frame.height;

        bInstant = XMEDIA_FALSE;
        s32Ret = xmedia_ive_dma(&IveHandle, &stSrc, &stDst, &stDmaCtrl, bInstant);
        SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS!=s32Ret, EXT_RELEASE,
            "Error(%#x),XMEDIA_API_IVE_DMA failed!\n", s32Ret);

        bInstant = XMEDIA_TRUE;
        s32Ret = xmedia_ive_integ(&IveHandle, &pstOd->stSrc, &pstOd->stInteg, &pstOd->stIntegCtrl, bInstant);
        SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS!=s32Ret, EXT_RELEASE,
            "Error(%#x),XMEDIA_API_IVE_Integ failed!\n", s32Ret);

        s32Ret = xmedia_ive_query(IveHandle, &bFinish, bBlock);
        while (ERR_CODE_IVE_QUERY_TIMEOUT == s32Ret)
        {
            usleep(100);
            s32Ret = xmedia_ive_query(IveHandle, &bFinish, bBlock);
        }

        SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS!=s32Ret, EXT_RELEASE,
            "Error(%#x),xmedia_ive_query failed!\n", s32Ret);

        for (j = 0; j < IVE_CHAR_CALH; j++)
        {
            pu64TopRow = (0 == j) ? (pu64VirData) : ( pu64VirData + (j * pstOd->u32H - 1) * pstOd->stInteg.au32stride[0]);
            pu64BtmRow = pu64VirData + ((j + 1) * pstOd->u32H - 1) * pstOd->stInteg.au32stride[0];

            for (i = 0; i < IVE_CHAR_CALW; i++)
            {
                u64TopLeft  = (0 == j) ? (0) : ((0 == i) ? (0) : (pu64TopRow[i * pstOd->u32W - 1]));
                u64TopRight = (0 == j) ? (0) : (pu64TopRow[(i + 1) * pstOd->u32W - 1]);
                u64BtmLeft  = (0 == i) ? (0) : (pu64BtmRow[i * pstOd->u32W - 1]);
                u64BtmRight = pu64BtmRow[(i + 1) * pstOd->u32W - 1];

                u64BlockSum = (u64TopLeft & 0xfffffffLL) + (u64BtmRight & 0xfffffffLL)
                              - (u64BtmLeft & 0xfffffffLL) - (u64TopRight & 0xfffffffLL);

                u64BlockSq  = (u64TopLeft >> 28) + (u64BtmRight >> 28)
                              - (u64BtmLeft >> 28) - (u64TopRight >> 28);

                // mean
                stChar[j * IVE_CHAR_CALW + i].x = u64BlockSum / (pstOd->u32W * pstOd->u32H);
                // sigma=sqrt(1/(w*h)*sum((x(i,j)-mean)^2)= sqrt(sum(x(i,j)^2)/(w*h)-mean^2)
                fSqVar = u64BlockSq / (pstOd->u32W * pstOd->u32H) - stChar[j * IVE_CHAR_CALW + i].x * stChar[j * IVE_CHAR_CALW + i].x;
                stChar[j * IVE_CHAR_CALW + i].y = (xmedia_u32)sqrt(fSqVar);
            }
        }

        void *virt_addr = (void*)(xmedia_ulong)stSrc.u64viraddr;
        xmedia_mmz_unmap(virt_addr);

        s32Ret = sample_ive_linear_2d_classifer(&stChar[0], IVE_CHAR_NUM,
                                              stIveLinerData.pst_linear_point, stIveLinerData.s32_linear_num);
        if (s32Ret > stIveLinerData.s32_thresh_num)
        {
            SAMPLE_PRT("\033[0;31m Occlusion detected!\033[0;39m\n");
        }
        else
        {

        }

        xmedia_s32 milli_sec = 20000;
        s32Ret = xmedia_venc_send_frame(venc_chn[1], &stExtFrmInfo, milli_sec);
        if (s32Ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("xmedia_venc_send_frame failed with %#x!\n", s32Ret);
        }

        EXT_RELEASE:
            s32Ret = xmedia_vpss_release_ochn_frame(s32VpssGrp,as32VpssChn[1], &stExtFrmInfo);
            if (XMEDIA_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("Error(%#x),xmedia_vpss_release_ochn_frame failed,Grp(%d) chn(%d)!\n",
                    s32Ret,s32VpssGrp,as32VpssChn[1]);
            }

    }

    SAMPLE_PRT("exit od thread...\n");

    return XMEDIA_NULL;
}

xmedia_void sample_ive_od(xmedia_void)
{
    xmedia_s32 s32Ret = XMEDIA_SUCCESS;
    xmedia_char acThreadName[16] = {0};

    memset(&s_stOd,0,sizeof(s_stOd));

    xmedia_u32 width = 640;
    xmedia_u32 height = 360;

    /******************************************
     step 1: start vi vpss venc vo
     ******************************************/
    s32Ret = sample_vi_vpss_venc_init();
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, END_OD_0,
        "Error(%#x),sample_vi_vpss_venc_init failed!\n", s32Ret);

    /******************************************
     step 2: Init OD
     ******************************************/

    s32Ret = sample_ive_od_init(&s_stOd, width, height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, END_OD_0,
        "sample_ive_od_init failed, Error(%#x)!\n", s32Ret);
    s_bStopSignal = XMEDIA_FALSE;

    /******************************************
     step 3: Create work thread
     ******************************************/
    snprintf(acThreadName, 16, "ive_od_proc");
    prctl(PR_SET_NAME, (unsigned long)acThreadName, 0,0,0);
    pthread_create(&s_hIveThread, 0, sample_ive_od_proc, (xmedia_void*)&s_stOd);

    SAMPLE_PAUSE();

    s_bStopSignal = XMEDIA_TRUE;
    pthread_join(s_hIveThread, XMEDIA_NULL);
    s_hIveThread = 0;
    sample_ive_od_uninit(&(s_stOd));
    memset(&s_stOd,0,sizeof(s_stOd));

END_OD_0:
    sample_vi_vpss_venc_uninit();

    return ;
}

/******************************************************************************
* function : Od sample signal handle
******************************************************************************/
xmedia_void sample_ive_od_handle_sig(xmedia_void)
{
    s_bStopSignal = XMEDIA_TRUE;
    if (0 != s_hIveThread)
    {
        pthread_join(s_hIveThread, XMEDIA_NULL);
        s_hIveThread = 0;
    }
    sample_ive_od_uninit(&(s_stOd));
    memset(&s_stOd,0,sizeof(s_stOd));
    sample_vi_vpss_venc_uninit();
}


