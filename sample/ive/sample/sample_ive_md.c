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
#include "xmedia_vgs.h"
#include "sample_comm_ive.h"
#include "xmedia_md.h"
#include "sample_comm_venc.h"
#include "sample_comm_livestream.h"
#define SAMPLE_IVE_MD_IMAGE_NUM 2

typedef struct sample_ive_md_s
{
    xmedia_ive_src_image_s astImg[SAMPLE_IVE_MD_IMAGE_NUM];
    xmedia_ive_dst_mem_info_s stBlob;
    xmedia_md_attr_s stMdAttr;
    sample_rect_array_s stRegion;

} sample_ive_md_s;

static xmedia_bool s_bStopSignal = XMEDIA_FALSE;
static pthread_t s_hMdThread = 0;
static sample_ive_md_s s_stMd;
// static sample_ive_switch_s s_stMdSwitch = {XMEDIA_FALSE,XMEDIA_TRUE};
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
xmedia_s32 sample_md_sys_init(sample_sys_config *sys_config)
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

xmedia_void sample_md_sys_exit(void)
{
    sample_comm_venc_exit();
    sample_comm_vpss_exit();
    sample_comm_vi_exit();
    sample_comm_sys_exit();
}

xmedia_s32 sample_vi_vpss_venc_init()
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

    ret = sample_md_sys_init(&sys_config);
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
    sample_md_sys_exit();
    return ret;
}

xmedia_s32 sample_vi_vpss_venc_uninit()
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
    sample_md_sys_exit();

    return 0;
}

static xmedia_void sample_ive_md_uninit(sample_ive_md_s *pstMd)
{
    xmedia_s32 i;
    xmedia_s32 s32Ret = XMEDIA_SUCCESS;

    for (i = 0; i < SAMPLE_IVE_MD_IMAGE_NUM; i++)
    {
        IVE_MMZ_FREE(pstMd->astImg[i].au64phyaddr[0],pstMd->astImg[i].au64viraddr[0]);
    }

    IVE_MMZ_FREE(pstMd->stBlob.u64phyaddr,pstMd->stBlob.u64viraddr);

    s32Ret = xmedia_md_exit();
    if(s32Ret != XMEDIA_SUCCESS)
    {
       SAMPLE_PRT("xmedia_md_exit fail,Error(%#x)\n",s32Ret);
       return ;
    }

}

static xmedia_s32 sample_ive_md_init(sample_ive_md_s *pstMd,xmedia_u32 u32Width,xmedia_u32 u32Height)
{
    xmedia_s32 s32Ret = XMEDIA_SUCCESS;
    xmedia_s32 i ;
    xmedia_u32 u32Size;
    xmedia_u8 u8WndSz;
    xmedia_vgs_init();
    for (i = 0;i < SAMPLE_IVE_MD_IMAGE_NUM;i++)
    {
        s32Ret = sample_comm_ive_create_image(&pstMd->astImg[i],XMEDIA_IVE_IMAGE_TYPE_U8C1,u32Width,u32Height);
        SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, MD_INIT_FAIL,
            "Error(%#x),Create img[%d] image failed!\n", s32Ret,i);
    }
    u32Size = sizeof(xmedia_ive_ccblob_s);
    s32Ret = sample_comm_ive_create_mem_info(&pstMd->stBlob,u32Size);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, MD_INIT_FAIL,
        "Error(%#x),Create blob mem info failed!\n", s32Ret);

    //Set attr info
    pstMd->stMdAttr.en_alg_mode = XMEDIA_MD_ALG_MODE_BG;
    pstMd->stMdAttr.en_sad_mode = XMEDIA_IVE_SAD_MODE_MB_4X4;
    pstMd->stMdAttr.en_sad_out_ctrl = XMEDIA_IVE_SAD_OUT_CTRL_THRESH;
    pstMd->stMdAttr.u16_sad_thr = 100 * (1 << 1);//100 * (1 << 2);
    pstMd->stMdAttr.u32_width = u32Width;
    pstMd->stMdAttr.u32_height = u32Height;
    pstMd->stMdAttr.st_add_ctrl.u0q16x = 32768;
    pstMd->stMdAttr.st_add_ctrl.u0q16y = 32768;
    pstMd->stMdAttr.st_ccl_ctrl.en_mode = XMEDIA_IVE_CCL_MODE_4C;
    u8WndSz = ( 1 << (2 + pstMd->stMdAttr.en_sad_mode));
    pstMd->stMdAttr.st_ccl_ctrl.u16_init_area_thr = u8WndSz * u8WndSz;
    pstMd->stMdAttr.st_ccl_ctrl.u16_step = u8WndSz;

    s32Ret = xmedia_md_init();
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, MD_INIT_FAIL,
        "Error(%#x),xmedia_md_init failed!\n", s32Ret);

MD_INIT_FAIL:

    if(XMEDIA_SUCCESS != s32Ret)
    {
        sample_ive_md_uninit(pstMd);
    }
    return s32Ret;

}

static xmedia_void * sample_ive_md_proc(xmedia_void * pArgs)
{
    xmedia_s32 s32Ret;
    sample_ive_md_s *pstMd;
    xmedia_md_attr_s *pstMdAttr;
    xmedia_video_frame_info stExtFrmInfo;
    xmedia_s32 s32VpssGrp = 0;
    xmedia_s32 as32VpssChn[] = {XMEDIA_VPSS_ICHN0, XMEDIA_VPSS_ICHN1};
    xmedia_s32 s32MilliSec = 20000;
    xmedia_s32 MdChn = 0;
    xmedia_bool bInstant = XMEDIA_TRUE;
    xmedia_s32 s32CurIdx = 0;
    xmedia_bool bFirstFrm = XMEDIA_TRUE;
    pstMd = (sample_ive_md_s *)(pArgs);
    pstMdAttr = &(pstMd->stMdAttr);
    //Create chn
    s32Ret = xmedia_md_create_chn(MdChn,&(pstMd->stMdAttr));
    if (XMEDIA_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("xmedia_md_create_chn fail,Error(%#x)\n",s32Ret);
        return NULL;
    }

    while (XMEDIA_FALSE == s_bStopSignal)
    {
        s32Ret = xmedia_vpss_acquire_ochn_frame(s32VpssGrp, as32VpssChn[1], &stExtFrmInfo, s32MilliSec);
        if(XMEDIA_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Error(%#x),xmedia_vpss_acquire_ochn_frame failed, VPSS_GRP(%d), VPSS_CHN(%d)!\n",
                s32Ret,s32VpssGrp, as32VpssChn[1]);
            continue;
        }

        if (XMEDIA_TRUE != bFirstFrm)
        {
            s32Ret = sample_comm_ive_dma_image(&stExtFrmInfo,&pstMd->astImg[s32CurIdx],bInstant);
            SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, EXT_RELEASE,
                            "sample_comm_ive_dma_image fail,Error(%#x)\n",s32Ret);
        }
        else
        {
            s32Ret = sample_comm_ive_dma_image(&stExtFrmInfo,&pstMd->astImg[s32CurIdx],bInstant);
            SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, EXT_RELEASE,
                            "sample_comm_ive_dma_image fail,Error(%#x)\n",s32Ret);

            bFirstFrm = XMEDIA_FALSE;
            goto CHANGE_IDX;//first frame just init reference frame
        }

        s32Ret = xmedia_md_process(MdChn,&pstMd->astImg[s32CurIdx],&pstMd->astImg[1 - s32CurIdx],NULL,&pstMd->stBlob);
        SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, EXT_RELEASE,
                        "xmedia_md_process fail,Error(%#x)\n",s32Ret);
        sample_comm_ive_blob_to_rect(SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(xmedia_ive_ccblob_s,pstMd->stBlob.u64viraddr),&(pstMd->stRegion),IVE_RECT_NUM,8,
            pstMdAttr->u32_width,pstMdAttr->u32_height,stExtFrmInfo.frame.width,stExtFrmInfo.frame.height);

         //Draw rect
        s32Ret = sample_comm_vgs_fill_rect(&stExtFrmInfo, &pstMd->stRegion, 0x0000FF00);
        SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, EXT_RELEASE,
                        "sample_comm_vgs_fill_rect fail,Error(%#x)\n",s32Ret);

        xmedia_s32 milli_sec = 20000;
        s32Ret = xmedia_venc_send_frame(venc_chn[1], &stExtFrmInfo, milli_sec);
        if (s32Ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("xmedia_venc_send_frame failed with %#x!\n", s32Ret);
        }

        CHANGE_IDX:
            //Change reference and current frame index
            s32CurIdx =    1 - s32CurIdx;

        EXT_RELEASE:
            s32Ret = xmedia_vpss_release_ochn_frame(s32VpssGrp,as32VpssChn[1], &stExtFrmInfo);
            if (XMEDIA_SUCCESS != s32Ret)
            {
                SAMPLE_PRT("Error(%#x),xmedia_vpss_release_ochn_frame failed,Grp(%d) chn(%d)!\n",
                    s32Ret,s32VpssGrp,as32VpssChn[1]);
            }
     }

     //destroy
     s32Ret = xmedia_md_destroy_chn(MdChn);
     if (XMEDIA_SUCCESS != s32Ret)
     {
         SAMPLE_PRT("xmedia_md_destroy_chn fail,Error(%#x)\n",s32Ret);
     }

     return XMEDIA_NULL;
}

xmedia_void sample_ive_md(xmedia_void)
{
    //xmedia_video_size stSize;
    //pic_size_e enSize = PIC_640x360;
    xmedia_s32 s32Ret = XMEDIA_SUCCESS;
    xmedia_char acThreadName[16] = {0};

    memset(&s_stMd,0,sizeof(s_stMd));
    /******************************************
     step 1: start vi vpss venc vo
     ******************************************/
    s32Ret = sample_vi_vpss_venc_init();
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, END_MD_0,
       "Error(%#x),sample_vi_vpss_venc_init failed!\n", s32Ret);

    //enSize = PIC_640x360;
    xmedia_u32 width  = 640;
    xmedia_u32 height = 360;
    /******************************************
     step 2: Init Md
     ******************************************/
    s32Ret = sample_ive_md_init(&s_stMd,width,height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32Ret, END_MD_0,
        " Error(%#x),sample_ive_md_init failed!\n", s32Ret);
    s_bStopSignal = XMEDIA_FALSE;
    /******************************************
      step 3: Create work thread
     ******************************************/
    snprintf(acThreadName, 16, "ive_md_proc");
    prctl(PR_SET_NAME, (unsigned long)acThreadName, 0,0,0);
    pthread_create(&s_hMdThread, 0, sample_ive_md_proc, (xmedia_void *)&s_stMd);

    SAMPLE_PAUSE();
    s_bStopSignal = XMEDIA_TRUE;
    pthread_join(s_hMdThread, XMEDIA_NULL);
    s_hMdThread = 0;

    sample_ive_md_uninit(&(s_stMd));
    memset(&s_stMd,0,sizeof(s_stMd));

END_MD_0:
    sample_vi_vpss_venc_uninit();
    return ;
}

/******************************************************************************
* function : Md sample signal handle
******************************************************************************/
xmedia_void sample_ive_md_handle_sig(xmedia_void)
{
    s_bStopSignal = XMEDIA_TRUE;
    if (0 != s_hMdThread)
    {
        pthread_join(s_hMdThread, XMEDIA_NULL);
        s_hMdThread = 0;
    }
    sample_ive_md_uninit(&(s_stMd));
    memset(&s_stMd,0,sizeof(s_stMd));

    sample_vi_vpss_venc_uninit();
}



