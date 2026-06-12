/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/times.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include "sample_comm.h"
#include "sample_comm_sys.h"
#include "sample_comm_vi.h"
#include "sample_comm_vpss.h"
#include "sample_comm_isp.h"
#include "sample_comm_vgs.h"
#include "sample_comm_venc.h"
#include "sample_comm_aov.h"
#include "sample_comm_pq_bin.h"
#include "sample_osd_time.h"
#include "sample_aov_audio.h"

static sample_comm_sensor_type sample_aov_sensor_type[MAX_SENSOR_NUM] = { SENSOR0_TYPE, SENSOR1_TYPE, SENSOR2_TYPE,
                                                                          SENSOR3_TYPE, SENSOR4_TYPE };

static sample_aov_work_mode g_aov_work_mode = MEDIA_WORK_NORMAL;
static xmedia_bool g_force_exit = XMEDIA_FALSE;
static xmedia_s32 g_misc_fd = -1;

#define IS_SAME_CAMERA_SENSOR       1

#define SENSOR0_PQ_BIN_PATH             "/usr/param/sensor0/pq.bin"
#ifdef IS_SAME_CAMERA_SENSOR
#define SENSOR1_PQ_BIN_PATH             SENSOR0_PQ_BIN_PATH
#else
#define SENSOR1_PQ_BIN_PATH             "/usr/param/sensor1/pq.bin"
#endif

#define SENSOR0_FPN_BIN_PATH            "/usr/param/sensor0/fpn.bin"
#define SENSOR1_FPN_BIN_PATH            "/usr/param/sensor1/fpn.bin"

#define ISP_AINR_MODEL_PATH             "/usr/param/neuron_network.xmm"
#define AOV_NPU_MODEL_PATH              "/usr/param/gnn_aov_person_detect_640x360_rgb888hwc_v0101_20240909.bin"


xmedia_void sample_aov_set_work_mode_state(sample_aov_work_mode work_mode)
{
    g_aov_work_mode = work_mode;
}

xmedia_void sample_aov_get_work_mode_state(sample_aov_work_mode *work_mode)
{
    *work_mode = g_aov_work_mode;
}

xmedia_s32 sample_aov_create_npu_person_detect(xmedia_s32 *handle)
{
    xmedia_s32 s32_ret = XMEDIA_FAILURE;
    xmedia_svp_modules model[1] = {0};
    xmedia_svp_task_cfg task_cfg = {0};

    model[0].pathname = AOV_NPU_MODEL_PATH;
    model[0].alg_type = XMEDIA_SVP_ALG_TYPE_PERSON;
    model[0].load_mode = XMEDIA_SVP_MODEL_FILE;
    model[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;

    task_cfg.module_num = 1;
    task_cfg.modules = model;
    task_cfg.task_type = XMEDIA_SVP_TASK_AOV;

    s32_ret = sample_comm_aov_npu_create(handle, &task_cfg);
    if (XMEDIA_SUCCESS != s32_ret) {
        SAMPLE_ERR("------------- svp create fail--please check your npu model name & path.\n");
        return s32_ret;
    }

    return XMEDIA_SUCCESS;
}

#ifdef NORMAL_SUPPORT_AUDIO
static xmedia_void sample_aov_audio_start(xmedia_void)
{
    sample_aov_audio_init();
    sample_aov_audio_start_play();
    sample_aov_audio_start_record();
}

static xmedia_void sample_aov_audio_stop(xmedia_void)
{
    sample_aov_audio_stop_play();
    sample_aov_audio_stop_record();
    sample_aov_audio_exit();
}
#endif

static xmedia_void sample_aov_normal_suspend_func()
{
#ifdef NORMAL_SUPPORT_AUDIO
    sample_aov_audio_stop();
#endif
    return;
}

static xmedia_void sample_aov_normal_resume_func()
{
#ifdef NORMAL_SUPPORT_AUDIO
    sample_aov_audio_start();
#endif
    return;
}

xmedia_s32 sample_aov_media_start(sample_aov_init_param *aov_init_param)
{
    sample_comm_aov_init(aov_init_param);
    sample_comm_aov_venc_thread_create();
    sample_comm_aov_work_thread_create();
#ifdef NORMAL_SUPPORT_AUDIO
    sample_aov_audio_start();
#endif
   return XMEDIA_SUCCESS;
}

xmedia_s32 sample_aov_media_stop()
{
    sample_comm_aov_venc_thread_destroy();
    sample_comm_aov_work_thread_destroy();
    sample_comm_aov_exit();
#ifdef NORMAL_SUPPORT_AUDIO
        sample_aov_audio_stop();
#endif

   return XMEDIA_SUCCESS;
}

xmedia_s32 sample_aov_sys_init(sample_sys_config *sys_config)
{
    xmedia_s32 ret = 0;

    ret = sample_comm_sys_init(sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_sys_init failed!\n");
        return ret;
    }

    ret = sample_comm_vgs_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vgs_init failed!\n");
        return ret;
    }

    // in online-online mode,vi and vpss must be reset at the same time
    ret = sample_comm_vi_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vi_init failed!\n");
        return ret;
    }

    ret = sample_comm_vpss_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vpss_init failed!\n");
        return ret;
    }

    ret = sample_comm_venc_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_venc_init failed!\n");
        return ret;
    }

    ret = sample_comm_aov_npu_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_aov_npu_init failed!\n");
        return ret;
    }

    return XMEDIA_SUCCESS;
}

xmedia_void sample_aov_sys_exit(void)
{
    sample_comm_aov_npu_exit();
    sample_comm_venc_exit();
    sample_comm_vpss_exit();
    sample_comm_vi_exit();
    sample_comm_vgs_exit();
    sample_comm_sys_exit();
}

xmedia_s32 sample_aov_single_sensor_stnr_only(xmedia_s32 frame_rate, xmedia_s32 suspend_time_ms)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 i = 0;
    xmedia_s32 pool_num = 0;
    xmedia_s32 blk_size = 0;
    xmedia_video_size pic_size = { 0 };
    xmedia_video_size sub_pic_size = { 0 };
    xmedia_video_size npu_pic_size = { 0 };
    sample_comm_video_param video_param = { 0 };
    sample_sys_config sys_config = { 0 };

    xmedia_s32 vi_dev = 0;
    xmedia_s32 vi_pipe = 0;
    xmedia_s32 vi_chn = 0;
    sample_vi_config vi_config = { 0 };

    xmedia_s32 sensor_type = sample_aov_sensor_type[0];
    xmedia_u32 framerate = 0;
    vi_sensor_info sensor_info = { 0 };
    sample_isp_param isp_param = { 0 };
    xmedia_isp_config isp_config = { 0 };
    xmedia_bool mirror = XMEDIA_FALSE;
    xmedia_bool flip = XMEDIA_FALSE;

    xmedia_s32 vpss_chnl = 0;
    xmedia_s32 vpss_pipe = 0;
    xmedia_s32 vpss_ochn[3] = { 0, 1 ,2};
    sample_vpss_config vpss_config = { 0 };
    xmedia_video_size vpss_ochn_size[VPSS_MAX_OCHN_NUM] = { 0 };
    xmedia_u32 wrap_buffer_line = 0;

    xmedia_s32 venc_chnl = 0;
    xmedia_s32 venc_chn[2] = { 0, 1 };
    xmedia_s32 venc_chn_cnt = 2;
    xmedia_venc_chn_param venc_chn_param;
    sample_venc_config venc_config = { 0 };

    misc_aov_isp_info isp_info = {0};
    xmedia_u32 sleep_time = 500; //us
#ifdef SUPPORT_OSD_TIME
    xmedia_chn_info mpp_chn;
    xmedia_rgn_chn_attr rgn_chn_attr = {0};
    xmedia_rgn_overlay_chn_attr *overlay_chn;
#endif

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    framerate = frame_rate;

    // sys init
    for(i = 0; i < VI_MAX_PIPE_NUM; i++) {
        sys_config.sys_conf.pipe_mode[i].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
        sys_config.sys_conf.pipe_mode[i].viproc_vpss_mode = XMEDIA_WORK_MODE_ONLINE;
        sys_config.sys_conf.pipe_mode[i].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    }
    sys_config.sys_conf.pipe_mode[0].vicap_viproc_mode = XMEDIA_WORK_MODE_ONLINE;

    pic_size.width = sensor_info.width;
    pic_size.height = sensor_info.height;

    /* wrap VB */
    blk_size = sample_comm_sys_get_wrap_buffer_size(pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                                    video_param.data_width, video_param.compress_mode, &wrap_buffer_line);
    sys_config.vb_conf.common_pool[pool_num].block_size = blk_size;
    sys_config.vb_conf.common_pool[pool_num].block_cnt = 1;
    pool_num ++;
    SAMPLE_PRT("wrap vb size : %d \n", blk_size);

    sub_pic_size.width = 640;
    sub_pic_size.height = 480;
    blk_size = sample_comm_sys_get_buffer_size(sub_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[pool_num].block_size = blk_size;
    sys_config.vb_conf.common_pool[pool_num].block_cnt = 3;
    pool_num ++;

    npu_pic_size.width = 640;
    npu_pic_size.height = 360;
    blk_size = sample_comm_sys_get_buffer_size(npu_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[pool_num].block_size = blk_size;
    sys_config.vb_conf.common_pool[pool_num].block_cnt = 3;
    pool_num ++;

    sys_config.vb_conf.max_pool_cnt = pool_num;

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_aov_sys_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("sys init failed !\n");
        return ret;
    }

    isp_info.isp_pipe_num = 1;
    isp_info.isp_pipe[0] = vi_pipe;
#ifdef SUPPORT_SENSOR_PWDN_MODE
    isp_info.support_sensor_pwdn_standby[0] = XMEDIA_TRUE;
#endif
    sample_comm_aov_misc_init(&isp_info);
    g_misc_fd = sample_comm_aov_get_misc_fd();

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
    isp_param.isp_info[vi_pipe].disable_dynamic_fps = XMEDIA_TRUE;
    isp_param.isp_info[vi_pipe].mirror = mirror;
    isp_param.isp_info[vi_pipe].flip = flip;
    memcpy(&(isp_param.isp_info[vi_pipe].isp_config), &isp_config, sizeof(xmedia_isp_config));

    // isp init
    ret = sample_comm_isp_init(&isp_param, &vi_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("isp init failed!\n");
        goto exit0;
    }

    // vi start
    ret = sample_comm_vi_start(&vi_config, &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("start vi failed!\n");
        goto exit1;
    }

    // isp start
    ret = sample_comm_isp_start(&isp_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("start isp failed!\n");
        goto exit2;
    }

    // vpss config
    ret = sample_comm_vpss_get_default_pipe_cfg(&vpss_config.pipe_info[vpss_pipe].pipe_config, pic_size, &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("get default pipe cfg failed !\n");
        goto exit2;
    }

    vpss_ochn_size[0].width = sensor_info.width;
    vpss_ochn_size[0].height = sensor_info.height;
    vpss_ochn_size[1].width = sub_pic_size.width;
    vpss_ochn_size[1].height = sub_pic_size.height;
    vpss_ochn_size[2].width = npu_pic_size.width;
    vpss_ochn_size[2].height = npu_pic_size.height;

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
        SAMPLE_ERR("get default pipe cfg failed !\n");
        goto exit2;
    }
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.depth = 0;

    vpss_chnl = vpss_ochn[1];
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_no = vpss_ochn[1];
    ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
                                                vpss_ochn_size[vpss_chnl], &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("get default pipe cfg failed !\n");
        goto exit2;
    }
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.depth = 0;

    vpss_chnl = vpss_ochn[2];
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_no = vpss_ochn[2];
    ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
                                                vpss_ochn_size[vpss_chnl], &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("get default pipe cfg failed !\n");
        goto exit2;
    }

    // vpss start
    ret = sample_comm_vpss_start(&vpss_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("vpss start failed !\n");
        goto exit2;
    }

    // venc init
    venc_chnl = venc_chn[0];
    venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
    venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
    venc_config.chn_info[venc_chnl].payload_type = PT_H265;
    venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265AVBR;
    venc_config.chn_info[venc_chnl].str_buffer_size = ALIGN_UP(pic_size.width * pic_size.height * 2, 64);
    venc_config.chn_info[venc_chnl].vui_info.vui_en = XMEDIA_TRUE;
    venc_config.chn_info[venc_chnl].vui_info.stream_display_fps = frame_rate;
    sample_comm_venc_get_default_chn_info(vpss_ochn_size[0], framerate, &venc_config.chn_info[venc_chnl]);

    venc_chnl = venc_chn[1];
    venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
    venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
    venc_config.chn_info[venc_chnl].payload_type = PT_H265;
    venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265AVBR;
    venc_config.chn_info[venc_chnl].str_buffer_size = ALIGN_UP(sub_pic_size.width * sub_pic_size.height * 2, 64);
    venc_config.chn_info[venc_chnl].vui_info.vui_en = XMEDIA_TRUE;
    venc_config.chn_info[venc_chnl].vui_info.stream_display_fps = frame_rate;
    sample_comm_venc_get_default_chn_info(vpss_ochn_size[1], framerate, &venc_config.chn_info[venc_chnl]);

    ret = sample_comm_venc_start(&venc_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("venc start failed !\n");
        goto exit3;
    }

    ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("vpss bind venc failed !\n");
        goto exit4;
    }

    ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("vpss bind venc failed !\n");
        goto exit5;
    }

    for(i = 0; i < venc_chn_cnt; i++) {
        ret = xmedia_venc_get_chn_param(venc_chn[i], &venc_chn_param);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("xmedia_venc_get_chn_param failed(0x%x), venc_chn[%d]!\n", ret, venc_chn[i]);
            goto exit6;
        }
        
        venc_chn_param.max_strm_cnt = VENC_STR_BUF_DEFAULT_MAX_FRAME_COUNT;
        
        ret = xmedia_venc_set_chn_param(venc_chn[i], &venc_chn_param);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("xmedia_venc_set_chn_param failed(0x%x), venc_chn[%d]!\n", ret, venc_chn[i]);
            goto exit6;
        }
    }

    xmedia_s32 svp_handle = 0;
    ret = sample_aov_create_npu_person_detect(&svp_handle);
    if (XMEDIA_SUCCESS != ret) {
        SAMPLE_ERR("npu init fail \n");
        goto exit6;
    }

    sample_aov_init_param aov_init_param = {0};

    aov_init_param.isp_param.isp_pipe_num = 1;
    aov_init_param.isp_param.isp_pipe[0] = isp_param.pipe[vi_pipe];

    //vpss normal, aov should disable ochn
    aov_init_param.vpss_param.vpss_pipe_num = 1;
    aov_init_param.vpss_param.vpss_pipe[0] = vpss_pipe;
    aov_init_param.vpss_param.vpss_ochn_num[0] = 1;
    aov_init_param.vpss_param.vpss_ochn[0][0] = vpss_ochn[1];

    aov_init_param.npu_param.npu_detect_num = 1;
    aov_init_param.npu_param.vpss_pipe[0] = vpss_pipe;
    aov_init_param.npu_param.vpss_ochn[0] = vpss_ochn[2];
    aov_init_param.npu_param.npu_svp_handle[0] = svp_handle;
    aov_init_param.npu_param.en_switch_mode[0] = XMEDIA_TRUE;

    aov_init_param.venc_param.venc_chn_num = venc_chn_cnt;
    aov_init_param.venc_param.venc_chn[0] = venc_chn[0];
    aov_init_param.venc_param.en_aov[0] = XMEDIA_TRUE;
    aov_init_param.venc_param.chn_max_size[0] = venc_config.chn_info[0].str_buffer_size;
    aov_init_param.venc_param.chn_threshold_size[0] = aov_init_param.venc_param.chn_max_size[0] / 5;
    aov_init_param.venc_param.venc_chn[1] = venc_chn[1];
    aov_init_param.venc_param.en_aov[1] = XMEDIA_FALSE;
    aov_init_param.venc_param.chn_max_size[1] = venc_config.chn_info[1].str_buffer_size;
    aov_init_param.venc_param.chn_threshold_size[1] = aov_init_param.venc_param.chn_max_size[1] / 5;

    aov_init_param.suspend_time = suspend_time_ms;
    aov_init_param.npu_notice_callback = sample_aov_set_work_mode_state;
    aov_init_param.normal_resume_callback = sample_aov_normal_resume_func;
    aov_init_param.normal_suspend_callback = sample_aov_normal_suspend_func;

    ret = sample_aov_media_start(&aov_init_param);
    if (XMEDIA_SUCCESS != ret) {
        SAMPLE_ERR("media start fail \n");
        goto exit6;
    }

#ifdef SUPPORT_OSD_TIME
    sample_osd_time_init();

    mpp_chn.mod_id = MOD_ID_VENC;
    mpp_chn.dev_id = 0;
    mpp_chn.chn_id = venc_chn[0];

    rgn_chn_attr.is_show = XMEDIA_TRUE;
    rgn_chn_attr.type = XMEDIA_RGN_TYPE_OVERLAY;

    overlay_chn = &rgn_chn_attr.attr.overlay_chn;
    overlay_chn->alpha0 = 128;
    overlay_chn->alpha1 = 128;
    overlay_chn->qp_info.disable = XMEDIA_TRUE;
    overlay_chn->qp_info.is_abs_qp = XMEDIA_FALSE;
    overlay_chn->qp_info.qp_val = -2;
    overlay_chn->dst = XMEDIA_RGN_ATTACH_JPEG_MAIN;
    overlay_chn->point.x = 0;
    overlay_chn->point.y = 0;
    overlay_chn->layer = 0;

    sample_osd_time_attach_to_chn(&mpp_chn, &rgn_chn_attr);
    
    mpp_chn.mod_id = MOD_ID_VENC;
    mpp_chn.dev_id = 0;
    mpp_chn.chn_id = venc_chn[1];
    sample_osd_time_attach_to_chn(&mpp_chn, &rgn_chn_attr);

    sample_osd_time_start();
#endif

    sample_aov_work_mode work_mode = MEDIA_WORK_NORMAL;
    sample_aov_work_mode last_work_mode = MEDIA_WORK_NORMAL;

    while(g_force_exit != XMEDIA_TRUE) {
        sample_aov_get_work_mode_state(&work_mode);
        if (work_mode != last_work_mode) {
            if (work_mode == MEDIA_WORK_AOV) {
                sample_comm_aov_set_work_mode(MEDIA_WORK_AOV);
                last_work_mode = MEDIA_WORK_AOV;
            }

            if (work_mode == MEDIA_WORK_NORMAL) {
                last_work_mode = MEDIA_WORK_NORMAL;
            }
        }

#ifdef SUPPORT_OSD_TIME
        sample_osd_time_update(sleep_time, work_mode);
#endif
        usleep(sleep_time);
    }

    sample_aov_media_stop();
    sample_comm_aov_npu_destroy(svp_handle);

#ifdef SUPPORT_OSD_TIME
    sample_osd_time_destroy();
#endif

exit6:
    sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);

exit5:
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
    sample_aov_sys_exit();

    return ret;
}

xmedia_s32 sample_aov_dual_sensor_stnr_only(xmedia_s32 frame_rate, xmedia_s32 suspend_time_ms)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 i = 0;
    xmedia_s32 pool_num = 0;
    xmedia_s32 blk_size = 0;
    xmedia_video_size pic_size[2] = { 0 };
    xmedia_video_size sub_pic_size = { 0 };
    xmedia_video_size npu_pic_size = { 0 };
    sample_comm_video_param video_param = { 0 };
    sample_sys_config sys_config = { 0 };
    xmedia_s32 media_pipe_num = 2;

    xmedia_s32 vi_dev[2] = {0,1};
    xmedia_s32 vi_pipe[2] = {0,1};
    xmedia_s32 vi_chn = 0;
    sample_vi_config vi_config = { 0 };

    xmedia_s32 sensor_type[2] = {sample_aov_sensor_type[0], sample_aov_sensor_type[1]};
    xmedia_u32 framerate = 0;
    vi_sensor_info sensor_info[2] = { 0 };
    sample_isp_param isp_param = { 0 };
    xmedia_isp_config isp_config = { 0 };
    xmedia_bool mirror = XMEDIA_FALSE;
    xmedia_bool flip = XMEDIA_FALSE;

    xmedia_s32 vpss_chnl = 0;
    xmedia_s32 vpss_pipe[2] = {0,1};
    xmedia_s32 vpss_ochn[3] = { 0, 1 ,2};
    sample_vpss_config vpss_config = { 0 };
    xmedia_video_size vpss_ochn_size[2][VPSS_MAX_OCHN_NUM] = { 0 };
    xmedia_u32 wrap_buffer_line[2] = {0};
    xmedia_vpss_wrap_buf_share_config wrap_share_cfg = { 0 };

    xmedia_s32 venc_chnl = 0;
    xmedia_s32 venc_chn[4] = { 0, 1, 2, 3 };
    xmedia_s32 venc_chn_cnt = 4;
    sample_venc_config venc_config = { 0 };
    xmedia_venc_chn_param venc_chn_param;

    misc_aov_isp_info isp_info = {0};
    xmedia_u32 sleep_time = 500; //us
#ifdef SUPPORT_OSD_TIME
    xmedia_chn_info mpp_chn;
    xmedia_rgn_chn_attr rgn_chn_attr = {0};
    xmedia_rgn_overlay_chn_attr *overlay_chn;
#endif

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    framerate = frame_rate;

    // sys init
    for(i = 0; i < VI_MAX_PIPE_NUM; i++) {
        sys_config.sys_conf.pipe_mode[i].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
        sys_config.sys_conf.pipe_mode[i].viproc_vpss_mode = XMEDIA_WORK_MODE_ONLINE;
        sys_config.sys_conf.pipe_mode[i].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    }

    for(i = 0; i < media_pipe_num; i ++)
    {
        sample_comm_vi_get_sensor_info(sensor_type[i], &sensor_info[i]);

        pic_size[i].width = sensor_info[i].width;
        pic_size[i].height = sensor_info[i].height;

        /*vi-viproc offline raw VB*/
        blk_size = sample_comm_sys_get_buffer_size(pic_size[i], video_param.video_fmt, sensor_info[i].pixel_format,
                                                sensor_info[i].bit_width, video_param.compress_mode);
        sys_config.vb_conf.common_pool[pool_num].block_size = blk_size;
        sys_config.vb_conf.common_pool[pool_num].block_cnt = 1;

        pool_num ++;
    }

    /* wrap VB */
    wrap_share_cfg.enable = XMEDIA_TRUE;
    wrap_share_cfg.wrap_pos = XMEDIA_VPSS_WRAP_POS_PHY_OCHN;

    for(i = 0; i < media_pipe_num; i ++)
    {
        blk_size = sample_comm_sys_get_wrap_buffer_size(pic_size[i], video_param.video_fmt, video_param.pixel_fmt,
                                            video_param.data_width, video_param.compress_mode, &wrap_buffer_line[i]);


        if(wrap_share_cfg.enable == XMEDIA_TRUE)
        {
            wrap_share_cfg.buf_size = (wrap_share_cfg.buf_size > blk_size) ? wrap_share_cfg.buf_size : blk_size;
            if(i == 1)
            {
                sys_config.vb_conf.common_pool[pool_num].block_size = wrap_share_cfg.buf_size;
                sys_config.vb_conf.common_pool[pool_num].block_cnt = 1;
                pool_num ++;
                SAMPLE_PRT("wrap share vb size : %d \n", wrap_share_cfg.buf_size);
            }
        }
        else
        {
            sys_config.vb_conf.common_pool[pool_num].block_size = blk_size;
            sys_config.vb_conf.common_pool[pool_num].block_cnt = 1;
            pool_num ++;
            SAMPLE_PRT("sensor%d wrap vb size : %d \n", i, blk_size);
        }
    }

    sub_pic_size.width = 640;
    sub_pic_size.height = 480;
    blk_size = sample_comm_sys_get_buffer_size(sub_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[pool_num].block_size = blk_size;
    sys_config.vb_conf.common_pool[pool_num].block_cnt = 6;
    pool_num ++;

    npu_pic_size.width = 640;
    npu_pic_size.height = 360;
    blk_size = sample_comm_sys_get_buffer_size(npu_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[pool_num].block_size = blk_size;
    sys_config.vb_conf.common_pool[pool_num].block_cnt = 6;
    pool_num ++;

    sys_config.vb_conf.max_pool_cnt = pool_num;

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_aov_sys_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("sys init failed !\n");
        return ret;
    }

    isp_info.isp_pipe_num = 2;
    isp_info.isp_pipe[0] = vi_pipe[0];
    isp_info.isp_pipe[1] = vi_pipe[1];
#ifdef SUPPORT_SENSOR_PWDN_MODE
    isp_info.support_sensor_pwdn_standby[0] = XMEDIA_TRUE;
    isp_info.support_sensor_pwdn_standby[1] = XMEDIA_TRUE;
#endif
    sample_comm_aov_misc_init(&isp_info);
    g_misc_fd = sample_comm_aov_get_misc_fd();

    for(i = 0; i < media_pipe_num; i ++)
    {
        vi_config.dev_info[i].dev_en = XMEDIA_TRUE;
        vi_config.dev_info[i].dev_no = vi_dev[i];
        vi_config.dev_info[i].sensor_type = sensor_type[i];
        vi_config.pipe_info[i].pipe_en = XMEDIA_TRUE;
        vi_config.pipe_info[i].pipe_no = vi_pipe[i];
        vi_config.pipe_info[i].chn_info[vi_chn].chn_en = XMEDIA_TRUE;
        vi_config.pipe_info[i].chn_info[vi_chn].chn_no = vi_chn;
        vi_config.pipe_info[vi_pipe[i]].lowdelay_attr.enable = XMEDIA_TRUE;
        vi_config.pipe_info[vi_pipe[i]].lowdelay_attr.line_cnt = sensor_info[i].height / 2;
        vi_config.pipe_info[vi_pipe[i]].lowdelay_attr.one_buf_en = XMEDIA_TRUE;
        vi_config.dev_bind_pipe[i].pipe[0] = vi_pipe[i];
        vi_config.dev_bind_pipe[i].pipe[1] = -1;

        if (sensor_type[i] == SMART_SC235HAI_MIPI_2M_2LANE_30FPS_10BIT ||
                sensor_type[i] == SMART_SC485SL_MIPI_4M_30FPS_2LANE_12BIT) {
            mirror = XMEDIA_TRUE;
            flip = XMEDIA_TRUE;
        }

        isp_config.fps = framerate;
        isp_config.mode_config.work_mode = XMEDIA_ISP_WORK_MODE_MASTER;
        isp_config.mode_config.master_mode.blend_stat_enable = XMEDIA_FALSE;
        isp_config.mode_config.master_mode.slave_num = 0;
        isp_config.pixel_fmt = sensor_info[i].pixel_format;
        isp_config.size.height = sensor_info[i].height;
        isp_config.size.width = sensor_info[i].width;
        isp_config.wdr_mode = sensor_info[i].wdr_mode;

        // isp pipe init
        isp_param.pipe[vi_pipe[i]] = vi_pipe[i];
        isp_param.isp_info[vi_pipe[i]].isp_pipe_en = XMEDIA_TRUE;
        isp_param.isp_info[vi_pipe[i]].isp_sensor_en = XMEDIA_TRUE;
        isp_param.isp_info[vi_pipe[i]].sensor_type = sensor_type[i];
        isp_param.isp_info[vi_pipe[i]].disable_dynamic_fps = XMEDIA_TRUE;
        isp_param.isp_info[vi_pipe[i]].mirror = mirror;
        isp_param.isp_info[vi_pipe[i]].flip = flip;
        memcpy(&(isp_param.isp_info[vi_pipe[i]].isp_config), &isp_config, sizeof(xmedia_isp_config));
    }

    // isp init
    ret = sample_comm_isp_init(&isp_param, &vi_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("isp init failed!\n");
        goto exit0;
    }

    // vi start
    ret = sample_comm_vi_start(&vi_config, &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("start vi failed!\n");
        goto exit1;
    }

    // isp start
    ret = sample_comm_isp_start(&isp_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("start isp failed!\n");
        goto exit2;
    }

    // vpss config
    for(i = 0; i < media_pipe_num; i ++) {
        ret = sample_comm_vpss_get_default_pipe_cfg(&vpss_config.pipe_info[vpss_pipe[i]].pipe_config, pic_size[i], &video_param);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("get default pipe cfg failed !\n");
            goto exit2;
        }

        vpss_ochn_size[vpss_pipe[i]][0].width = sensor_info[i].width;
        vpss_ochn_size[vpss_pipe[i]][0].height = sensor_info[i].height;
        vpss_ochn_size[vpss_pipe[i]][1].width = sub_pic_size.width;
        vpss_ochn_size[vpss_pipe[i]][1].height = sub_pic_size.height;
        vpss_ochn_size[vpss_pipe[i]][2].width = npu_pic_size.width;
        vpss_ochn_size[vpss_pipe[i]][2].height = npu_pic_size.height;

        vpss_config.pipe_info[i].pipe_en = XMEDIA_TRUE;
        vpss_config.pipe_info[i].pipe_no = i;

        vpss_chnl = vpss_ochn[0];
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_no = vpss_ochn[0];
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].wrap_attr.buf_line = wrap_buffer_line[i];
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].wrap_attr.enable = XMEDIA_TRUE;
        ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config,
                vpss_ochn_size[vpss_pipe[i]][vpss_chnl], &video_param);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("get default pipe cfg failed !\n");
            goto exit2;
        }
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config.cmp_mode = XMEDIA_VIDEO_COMPRESS_MODE_TILE_COMPACT;
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config.video_fmt = XMEDIA_VIDEO_FMT_TILE_32x4;
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config.depth = 0;

        vpss_chnl = vpss_ochn[1];
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_no = vpss_ochn[1];
        ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config,
                vpss_ochn_size[vpss_pipe[i]][vpss_chnl], &video_param);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("get default pipe cfg failed !\n");
            goto exit2;
        }
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config.depth = 0;

        vpss_chnl = vpss_ochn[2];
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_no = vpss_ochn[2];
        ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config,
                vpss_ochn_size[vpss_pipe[i]][vpss_chnl], &video_param);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("get default pipe cfg failed !\n");
            goto exit2;
        }
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config.depth = 4;
    }

    // set wrap buf share
    if(wrap_share_cfg.enable == XMEDIA_TRUE) {
        ret = xmedia_vpss_set_wrap_buf_share_config(&wrap_share_cfg);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("set wrap buf share failed !\n");
            goto exit2;
        }
    }

    // vpss start
    ret = sample_comm_vpss_start(&vpss_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("vpss start failed !\n");
        goto exit2;
    }

    // venc init
    for(i = 0; i < media_pipe_num; i ++) {
        venc_chnl = venc_chn[2 * i];
        venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
        venc_config.chn_info[venc_chnl].payload_type = PT_H265;
        venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265AVBR;
        venc_config.chn_info[venc_chnl].str_buffer_size = ALIGN_UP(pic_size[i].width * pic_size[i].height * 2, 64);
        venc_config.chn_info[venc_chnl].vui_info.vui_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].vui_info.stream_display_fps = frame_rate;
        sample_comm_venc_get_default_chn_info(vpss_ochn_size[vpss_pipe[i]][0], framerate, &venc_config.chn_info[venc_chnl]);

        venc_chnl = venc_chn[2 * i + 1];
        venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
        venc_config.chn_info[venc_chnl].payload_type = PT_H265;
        venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265AVBR;
        venc_config.chn_info[venc_chnl].str_buffer_size = ALIGN_UP(sub_pic_size.width * sub_pic_size.height * 2, 64);
        venc_config.chn_info[venc_chnl].vui_info.vui_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].vui_info.stream_display_fps = frame_rate;
        sample_comm_venc_get_default_chn_info(vpss_ochn_size[vpss_pipe[i]][1], framerate, &venc_config.chn_info[venc_chnl]);
    }

    ret = sample_comm_venc_start(&venc_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("venc start failed !\n");
        goto exit3;
    }

    for(i = 0; i < media_pipe_num; i ++) {
        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe[i], vpss_ochn[0], venc_chn[2*i+0]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("vpss bind venc failed !\n");
            goto exit4;
        }

        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe[i], vpss_ochn[1], venc_chn[2*i+1]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("vpss bind venc failed !\n");
            goto exit5;
        }
    }

    for(i = 0; i < venc_chn_cnt; i++) {
        ret = xmedia_venc_get_chn_param(venc_chn[i], &venc_chn_param);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("xmedia_venc_get_chn_param failed(0x%x), venc_chn[%d]!\n", ret, venc_chn[i]);
            goto exit5;
        }
        
        venc_chn_param.max_strm_cnt = VENC_STR_BUF_DEFAULT_MAX_FRAME_COUNT;
        
        ret = xmedia_venc_set_chn_param(venc_chn[i], &venc_chn_param);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("xmedia_venc_set_chn_param failed(0x%x), venc_chn[%d]!\n", ret, venc_chn[i]);
            goto exit5;
        }
    }

    xmedia_s32 svp_handle = 0;
    ret = sample_aov_create_npu_person_detect(&svp_handle);
    if (XMEDIA_SUCCESS != ret) {
        SAMPLE_ERR("npu init fail \n");
        goto exit6;
    }

    sample_aov_init_param aov_init_param = {0};

    aov_init_param.isp_param.isp_pipe_num = 2;
    aov_init_param.isp_param.isp_pipe[0] = isp_param.pipe[vpss_pipe[0]];
    aov_init_param.isp_param.isp_pipe[1] = isp_param.pipe[vpss_pipe[1]];

    //vpss normal, aov should disable ochn
    aov_init_param.vpss_param.vpss_pipe_num = 2;
    aov_init_param.vpss_param.vpss_pipe[0] = vpss_pipe[0];
    aov_init_param.vpss_param.vpss_pipe[1] = vpss_pipe[1];
    aov_init_param.vpss_param.vpss_ochn_num[0] = 1;
    aov_init_param.vpss_param.vpss_ochn_num[1] = 1;
    aov_init_param.vpss_param.vpss_ochn[0][0] = vpss_ochn[1];
    aov_init_param.vpss_param.vpss_ochn[1][0] = vpss_ochn[1];

    aov_init_param.npu_param.npu_detect_num = 2;
    aov_init_param.npu_param.vpss_pipe[0] = vpss_pipe[0];
    aov_init_param.npu_param.vpss_ochn[0] = vpss_ochn[2];
    aov_init_param.npu_param.npu_svp_handle[0] = svp_handle;
    aov_init_param.npu_param.en_switch_mode[0] = XMEDIA_TRUE;
    aov_init_param.npu_param.vpss_pipe[1] = vpss_pipe[1];
    aov_init_param.npu_param.vpss_ochn[1] = vpss_ochn[2];
    aov_init_param.npu_param.npu_svp_handle[1] = svp_handle;
    aov_init_param.npu_param.en_switch_mode[1] = XMEDIA_TRUE;

    aov_init_param.venc_param.venc_chn_num = venc_chn_cnt;
    for(i = 0; i < media_pipe_num; i ++)
    {
        aov_init_param.venc_param.venc_chn[2*i] = venc_chn[2*i];
        aov_init_param.venc_param.en_aov[2*i] = XMEDIA_TRUE;
        aov_init_param.venc_param.chn_max_size[2*i] = venc_config.chn_info[venc_chn[2*i]].str_buffer_size;
        aov_init_param.venc_param.chn_threshold_size[2*i] = aov_init_param.venc_param.chn_max_size[2*i] / 5;
        
        aov_init_param.venc_param.venc_chn[2*i+1] = venc_chn[2*i+1];
        aov_init_param.venc_param.en_aov[2*i+1] = XMEDIA_FALSE;
        aov_init_param.venc_param.chn_max_size[2*i+1] = venc_config.chn_info[venc_chn[2*i+1]].str_buffer_size;
        aov_init_param.venc_param.chn_threshold_size[2*i+1] = aov_init_param.venc_param.chn_max_size[2*i+1] / 5;
    }

    aov_init_param.suspend_time = suspend_time_ms;
    aov_init_param.npu_notice_callback = sample_aov_set_work_mode_state;
    aov_init_param.normal_resume_callback = sample_aov_normal_resume_func;
    aov_init_param.normal_suspend_callback = sample_aov_normal_suspend_func;

    ret = sample_aov_media_start(&aov_init_param);
    if (XMEDIA_SUCCESS != ret) {
        SAMPLE_ERR("media start fail \n");
        goto exit6;
    }

#ifdef SUPPORT_OSD_TIME
    sample_osd_time_init();

    mpp_chn.mod_id = MOD_ID_VENC;
    mpp_chn.dev_id = 0;
    mpp_chn.chn_id = venc_chn[0];

    rgn_chn_attr.is_show = XMEDIA_TRUE;
    rgn_chn_attr.type = XMEDIA_RGN_TYPE_OVERLAY;

    overlay_chn = &rgn_chn_attr.attr.overlay_chn;
    overlay_chn->alpha0 = 128;
    overlay_chn->alpha1 = 128;
    overlay_chn->qp_info.disable = XMEDIA_TRUE;
    overlay_chn->qp_info.is_abs_qp = XMEDIA_FALSE;
    overlay_chn->qp_info.qp_val = -2;
    overlay_chn->dst = XMEDIA_RGN_ATTACH_JPEG_MAIN;
    overlay_chn->point.x = 0;
    overlay_chn->point.y = 0;
    overlay_chn->layer = 0;

    sample_osd_time_attach_to_chn(&mpp_chn, &rgn_chn_attr);

    mpp_chn.mod_id = MOD_ID_VENC;
    mpp_chn.dev_id = 0;
    mpp_chn.chn_id = venc_chn[1];
    sample_osd_time_attach_to_chn(&mpp_chn, &rgn_chn_attr);

    mpp_chn.mod_id = MOD_ID_VENC;
    mpp_chn.dev_id = 0;
    mpp_chn.chn_id = venc_chn[2];
    sample_osd_time_attach_to_chn(&mpp_chn, &rgn_chn_attr);

    mpp_chn.mod_id = MOD_ID_VENC;
    mpp_chn.dev_id = 0;
    mpp_chn.chn_id = venc_chn[3];
    sample_osd_time_attach_to_chn(&mpp_chn, &rgn_chn_attr);

    sample_osd_time_start();
#endif

    sample_aov_work_mode work_mode = MEDIA_WORK_NORMAL;
    sample_aov_work_mode last_work_mode = MEDIA_WORK_NORMAL;

    while(g_force_exit != XMEDIA_TRUE) {
        sample_aov_get_work_mode_state(&work_mode);
        if (work_mode != last_work_mode) {
            if (work_mode == MEDIA_WORK_AOV) {
                sample_comm_aov_set_work_mode(MEDIA_WORK_AOV);
                last_work_mode = MEDIA_WORK_AOV;
            }

            if (work_mode == MEDIA_WORK_NORMAL) {
                last_work_mode = MEDIA_WORK_NORMAL;
            }
        }

#ifdef SUPPORT_OSD_TIME
        sample_osd_time_update(sleep_time, work_mode);
#endif
        usleep(sleep_time);
    }

    sample_aov_media_stop();
    sample_comm_aov_npu_destroy(svp_handle);

#ifdef SUPPORT_OSD_TIME
    sample_osd_time_destroy();
#endif

exit6:
    for(i = 0; i < media_pipe_num; i ++)
        sample_comm_sys_vpss_unbind_venc(vpss_pipe[i], vpss_ochn[1], venc_chn[2*i+1]);

exit5:
    for(i = 0; i < media_pipe_num; i ++)
        sample_comm_sys_vpss_unbind_venc(vpss_pipe[i], vpss_ochn[0], venc_chn[2*i+0]);

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
    sample_aov_sys_exit();

    return ret;
}

xmedia_s32 sample_aov_single_sensor_ainr(xmedia_s32 frame_rate, xmedia_s32 suspend_time_ms)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 i = 0;
    xmedia_s32 blk_size = 0;
    xmedia_s32 pool_num = 0;
    xmedia_video_size pic_size = { 0 };
    xmedia_video_size sub_pic_size = { 0 };
    xmedia_video_size npu_pic_size = { 0 };
    xmedia_vb_cal_cfg vb_cfg = {0};
    xmedia_vb_base_info vb_base_info = {0};
    sample_comm_video_param video_param = { 0 };
    sample_sys_config sys_config = { 0 };

    xmedia_s32 vi_dev = 0;
    xmedia_s32 vi_pipe = 0;
    xmedia_s32 vi_chn = 0;
    sample_vi_config vi_config = { 0 };

    xmedia_s32 sensor_type = sample_aov_sensor_type[0];
    xmedia_u32 framerate = 0;
    vi_sensor_info sensor_info = { 0 };
    sample_isp_param isp_param = { 0 };
    xmedia_isp_config isp_config = { 0 };
    xmedia_bool mirror = XMEDIA_FALSE;
    xmedia_bool flip = XMEDIA_FALSE;
    bin_module_info pq_info = {0};

    xmedia_s32 vpss_chnl = 0;
    xmedia_s32 vpss_pipe = 0;
    xmedia_s32 vpss_ochn[3] = { 0, 1 ,2};
    sample_vpss_config vpss_config = { 0 };
    xmedia_video_size vpss_ochn_size[VPSS_MAX_OCHN_NUM] = { 0 };
    xmedia_u32 wrap_buffer_line = 0;

    xmedia_s32 svp_handle = 0;
    sample_comm_npu_workspace_info npu_work_sapce_info = {0};

    xmedia_s32 venc_chnl = 0;
    xmedia_s32 venc_chn[2] = { 0, 1 };
    xmedia_s32 venc_chn_cnt = 2;
    sample_venc_config venc_config = { 0 };
    xmedia_venc_chn_param venc_chn_param;
    sample_aov_init_param aov_init_param = {0};

    misc_aov_isp_info isp_info = {0};
    xmedia_u32 sleep_time = 500; //us
#ifdef SUPPORT_OSD_TIME
    xmedia_chn_info mpp_chn;
    xmedia_rgn_chn_attr rgn_chn_attr = {0};
    xmedia_rgn_overlay_chn_attr *overlay_chn;
#endif

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    framerate = frame_rate;

    if (!(sensor_info.bit_width == XMEDIA_VIDEO_DATA_WIDTH_10 || sensor_info.bit_width == XMEDIA_VIDEO_DATA_WIDTH_12)) {
        SAMPLE_ERR("ainr bit width should be 10bit or 12bit\n");
        return XMEDIA_FAILURE;
    }

    // sys init
    for(i = 0; i < VI_MAX_PIPE_NUM; i++) {
        sys_config.sys_conf.pipe_mode[i].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
        sys_config.sys_conf.pipe_mode[i].viproc_vpss_mode = XMEDIA_WORK_MODE_ONLINE;
        sys_config.sys_conf.pipe_mode[i].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    }

    pic_size.width = sensor_info.width;
    pic_size.height = sensor_info.height;

    /* wrap VB */
    blk_size = sample_comm_sys_get_wrap_buffer_size(pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                                    video_param.data_width, video_param.compress_mode, &wrap_buffer_line);
    sys_config.vb_conf.common_pool[pool_num].block_size = blk_size;
    sys_config.vb_conf.common_pool[pool_num].block_cnt = 1;
    pool_num ++;
    SAMPLE_PRT("wrap vb size : %d \n", blk_size);

    vb_base_info.width = pic_size.width;
    vb_base_info.height = pic_size.height;
    vb_base_info.cmp_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;
    vb_base_info.bit_width = sensor_info.bit_width;
    vb_base_info.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_RAW;
    vb_base_info.video_fmt = video_param.video_fmt;
    vb_base_info.align = DEFAULT_ALIGN;
    vb_base_info.ainr_attr.ainr_en = XMEDIA_TRUE;
    vb_base_info.ainr_attr.ainr_direction = XMEDIA_VB_AINR_DIRECTION_INPUT;

    xmedia_vb_get_buffer_config(&vb_base_info, &vb_cfg);
    sys_config.vb_conf.common_pool[pool_num].block_size = vb_cfg.vb_size;
    sys_config.vb_conf.common_pool[pool_num].block_cnt = 2;
    pool_num ++;

    // vi proc
    vb_base_info.ainr_attr.ainr_direction = XMEDIA_VB_AINR_DIRECTION_OUTPUT;
    xmedia_vb_get_buffer_config(&vb_base_info, &vb_cfg);
    sys_config.vb_conf.common_pool[pool_num].block_size = vb_cfg.vb_size;
    sys_config.vb_conf.common_pool[pool_num].block_cnt = 2;
    pool_num ++;

    sub_pic_size.width = 640;
    sub_pic_size.height = 480;
    blk_size = sample_comm_sys_get_buffer_size(sub_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[pool_num].block_size = blk_size;
    sys_config.vb_conf.common_pool[pool_num].block_cnt = 3;
    pool_num ++;

    npu_pic_size.width = 640;
    npu_pic_size.height = 360;
    blk_size = sample_comm_sys_get_buffer_size(npu_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[pool_num].block_size = blk_size;
    sys_config.vb_conf.common_pool[pool_num].block_cnt = 3;
    pool_num ++;

    sys_config.vb_conf.max_pool_cnt = pool_num;
    sys_config.vb_conf.supplement_config = 1;

    ret = sample_aov_sys_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("sys init failed !\n");
        return ret;
    }

    isp_info.isp_pipe_num = 1;
    isp_info.isp_pipe[0] = vi_pipe;
#ifdef SUPPORT_SENSOR_PWDN_MODE
    isp_info.support_sensor_pwdn_standby[0] = XMEDIA_TRUE;
#endif
    sample_comm_aov_misc_init(&isp_info);
    g_misc_fd = sample_comm_aov_get_misc_fd();

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
    isp_param.isp_info[vi_pipe].disable_dynamic_fps = XMEDIA_TRUE;
    isp_param.isp_info[vi_pipe].mirror = mirror;
    isp_param.isp_info[vi_pipe].flip = flip;
    memcpy(&(isp_param.isp_info[vi_pipe].isp_config), &isp_config, sizeof(xmedia_isp_config));

    // isp init
    ret = sample_comm_isp_init(&isp_param, &vi_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("isp init failed!\n");
        goto exit0;
    }

    // vi start
    ret = sample_comm_vi_start(&vi_config, &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("start vi failed!\n");
        goto exit1;
    }

    // isp start
    ret = sample_comm_isp_start(&isp_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("start isp failed!\n");
        goto exit2;
    }

    // vpss config
    ret = sample_comm_vpss_get_default_pipe_cfg(&vpss_config.pipe_info[vpss_pipe].pipe_config, pic_size, &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("get default pipe cfg failed !\n");
        goto exit2;
    }

    vpss_ochn_size[0].width = sensor_info.width;
    vpss_ochn_size[0].height = sensor_info.height;
    vpss_ochn_size[1].width = sub_pic_size.width;
    vpss_ochn_size[1].height = sub_pic_size.height;
    vpss_ochn_size[2].width = npu_pic_size.width;
    vpss_ochn_size[2].height = npu_pic_size.height;

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
        SAMPLE_ERR("get default pipe cfg failed !\n");
        goto exit2;
    }
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.depth = 0;

    vpss_chnl = vpss_ochn[1];
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_no = vpss_ochn[1];
    ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
                                                vpss_ochn_size[vpss_chnl], &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("get default pipe cfg failed !\n");
        goto exit2;
    }
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.depth = 0;

    vpss_chnl = vpss_ochn[2];
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_no = vpss_ochn[2];
    ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
                                                vpss_ochn_size[vpss_chnl], &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("get default pipe cfg failed !\n");
        goto exit2;
    }

    // vpss start
    ret = sample_comm_vpss_start(&vpss_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("vpss start failed !\n");
        goto exit2;
    }

    ret = sample_aov_create_npu_person_detect(&svp_handle);
    if (XMEDIA_SUCCESS != ret) {
        SAMPLE_ERR("npu init fail \n");
        goto exit3;
    }

    npu_work_sapce_info.npu_modle_num = 2;
    npu_work_sapce_info.npu_model_path[0] = ISP_AINR_MODEL_PATH;
    npu_work_sapce_info.npu_model_path[1] = AOV_NPU_MODEL_PATH;

    ret = sample_comm_sys_npu_acquire_workspace(&npu_work_sapce_info);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("acquire workspace fail\n");
        goto exit4;
    }

    isp_param.ainr_model_path[vi_pipe] = npu_work_sapce_info.npu_model_path[0];

    ret = sample_comm_isp_ainr_init(&isp_param, &vi_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("isp init failed!\n");
        goto exit5;
    } else {
        SAMPLE_PRT("isp load ainr success!\n");
    }

    pq_info.vi.enable = XMEDIA_TRUE;
    pq_info.vi.pipe = vi_pipe;
    pq_info.mcf.enable = XMEDIA_FALSE;
    pq_info.vpss.enable = XMEDIA_FALSE;

    ret = sample_comm_pq_bin_import(&pq_info, SENSOR0_PQ_BIN_PATH);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("isp load pq bin failed!\n");
        goto exit6;
    } else {
        SAMPLE_PRT("isp load pq success!\n");
    }

    ret = sample_comm_isp_enable_fpn(isp_param.pipe[vi_pipe], SENSOR0_FPN_BIN_PATH);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("isp load fpn bin failed!\n");
        goto exit6;
    } else {
        SAMPLE_PRT("isp load fpn success!\n");
    }

// venc init
    venc_chnl = venc_chn[0];
    venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
    venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
    venc_config.chn_info[venc_chnl].payload_type = PT_H265;
    venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265AVBR;
    venc_config.chn_info[venc_chnl].str_buffer_size = ALIGN_UP(pic_size.width * pic_size.height * 2, 64);
    venc_config.chn_info[venc_chnl].vui_info.vui_en = XMEDIA_TRUE;
    venc_config.chn_info[venc_chnl].vui_info.stream_display_fps = frame_rate;
    sample_comm_venc_get_default_chn_info(vpss_ochn_size[0], framerate, &venc_config.chn_info[venc_chnl]);

    venc_chnl = venc_chn[1];
    venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
    venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
    venc_config.chn_info[venc_chnl].payload_type = PT_H265;
    venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265AVBR;
    venc_config.chn_info[venc_chnl].str_buffer_size = ALIGN_UP(sub_pic_size.width * sub_pic_size.height * 2, 64);
    venc_config.chn_info[venc_chnl].vui_info.vui_en = XMEDIA_TRUE;
    venc_config.chn_info[venc_chnl].vui_info.stream_display_fps = frame_rate;
    sample_comm_venc_get_default_chn_info(vpss_ochn_size[1], framerate, &venc_config.chn_info[venc_chnl]);

    ret = sample_comm_venc_start(&venc_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("venc start failed !\n");
        goto exit7;
    }

    ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("vpss bind venc failed !\n");
        goto exit8;
    }

    ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("vpss bind venc failed !\n");
        goto exit9;
    }

    for(i = 0; i < venc_chn_cnt; i++) {
        ret = xmedia_venc_get_chn_param(venc_chn[i], &venc_chn_param);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("xmedia_venc_get_chn_param failed(0x%x), venc_chn[%d]!\n", ret, venc_chn[i]);
            goto exit9;
        }
        
        venc_chn_param.max_strm_cnt = VENC_STR_BUF_DEFAULT_MAX_FRAME_COUNT;
        
        ret = xmedia_venc_set_chn_param(venc_chn[i], &venc_chn_param);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("xmedia_venc_set_chn_param failed(0x%x), venc_chn[%d]!\n", ret, venc_chn[i]);
            goto exit9;
        }
    }

    aov_init_param.isp_param.isp_pipe_num = 1;
    aov_init_param.isp_param.isp_pipe[0] = isp_param.pipe[vi_pipe];

    //vpss normal, aov should disable ochn
    aov_init_param.vpss_param.vpss_pipe_num = 1;
    aov_init_param.vpss_param.vpss_pipe[0] = vpss_pipe;
    aov_init_param.vpss_param.vpss_ochn_num[0] = 1;
    aov_init_param.vpss_param.vpss_ochn[0][0] = vpss_ochn[1];

    aov_init_param.npu_param.npu_detect_num = 1;
    aov_init_param.npu_param.vpss_pipe[0] = vpss_pipe;
    aov_init_param.npu_param.vpss_ochn[0] = vpss_ochn[2];
    aov_init_param.npu_param.npu_svp_handle[0] = svp_handle;
    aov_init_param.npu_param.en_switch_mode[0] = XMEDIA_TRUE;

    aov_init_param.venc_param.venc_chn_num = venc_chn_cnt;
    aov_init_param.venc_param.venc_chn[0] = venc_chn[0];
    aov_init_param.venc_param.en_aov[0] = XMEDIA_TRUE;
    aov_init_param.venc_param.chn_max_size[0] = venc_config.chn_info[0].str_buffer_size;
    aov_init_param.venc_param.chn_threshold_size[0] = aov_init_param.venc_param.chn_max_size[0] / 5;
    aov_init_param.venc_param.venc_chn[1] = venc_chn[1];
    aov_init_param.venc_param.en_aov[1] = XMEDIA_FALSE;
    aov_init_param.venc_param.chn_max_size[1] = venc_config.chn_info[1].str_buffer_size;
    aov_init_param.venc_param.chn_threshold_size[1] = aov_init_param.venc_param.chn_max_size[1] / 5;

    aov_init_param.suspend_time = suspend_time_ms;
    aov_init_param.npu_notice_callback = sample_aov_set_work_mode_state;
    aov_init_param.normal_resume_callback = sample_aov_normal_resume_func;
    aov_init_param.normal_suspend_callback = sample_aov_normal_suspend_func;

    ret = sample_aov_media_start(&aov_init_param);
    if (XMEDIA_SUCCESS != ret) {
        SAMPLE_ERR("media start fail \n");
        goto exit9;
    }

#ifdef SUPPORT_OSD_TIME
    sample_osd_time_init();

    mpp_chn.mod_id = MOD_ID_VENC;
    mpp_chn.dev_id = 0;
    mpp_chn.chn_id = venc_chn[0];

    rgn_chn_attr.is_show = XMEDIA_TRUE;
    rgn_chn_attr.type = XMEDIA_RGN_TYPE_OVERLAY;

    overlay_chn = &rgn_chn_attr.attr.overlay_chn;
    overlay_chn->alpha0 = 128;
    overlay_chn->alpha1 = 128;
    overlay_chn->qp_info.disable = XMEDIA_TRUE;
    overlay_chn->qp_info.is_abs_qp = XMEDIA_FALSE;
    overlay_chn->qp_info.qp_val = -2;
    overlay_chn->dst = XMEDIA_RGN_ATTACH_JPEG_MAIN;
    overlay_chn->point.x = 0;
    overlay_chn->point.y = 0;
    overlay_chn->layer = 0;

    sample_osd_time_attach_to_chn(&mpp_chn, &rgn_chn_attr);
    
    mpp_chn.mod_id = MOD_ID_VENC;
    mpp_chn.dev_id = 0;
    mpp_chn.chn_id = venc_chn[1];
    sample_osd_time_attach_to_chn(&mpp_chn, &rgn_chn_attr);

    sample_osd_time_start();
#endif

    sample_aov_work_mode work_mode = MEDIA_WORK_NORMAL;
    sample_aov_work_mode last_work_mode = MEDIA_WORK_NORMAL;

    while(g_force_exit != XMEDIA_TRUE) {
        sample_aov_get_work_mode_state(&work_mode);
        if (work_mode != last_work_mode) {
            if (work_mode == MEDIA_WORK_AOV) {
                sample_comm_aov_set_work_mode(MEDIA_WORK_AOV);
                last_work_mode = MEDIA_WORK_AOV;
            }

            if (work_mode == MEDIA_WORK_NORMAL) {
                last_work_mode = MEDIA_WORK_NORMAL;
            }
        }

#ifdef SUPPORT_OSD_TIME
        sample_osd_time_update(sleep_time, work_mode);
#endif
        usleep(sleep_time);
    }

    sample_aov_media_stop();
    sample_comm_aov_npu_destroy(svp_handle);

#ifdef SUPPORT_OSD_TIME
    sample_osd_time_destroy();
#endif

    sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);

exit9:

    sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
exit8:
    sample_comm_venc_stop(&venc_config);

exit7:
    sample_comm_isp_disable_fpn(isp_param.pipe[vi_pipe]);

exit6:
    sample_comm_isp_ainr_exit(&isp_param);

exit5:
    sample_comm_sys_npu_release_workspace();

exit4:
    sample_comm_aov_npu_destroy(svp_handle);

exit3:
    sample_comm_vpss_stop(&vpss_config);

exit2:
    sample_comm_isp_stop(&isp_param);

exit1:
    sample_comm_vi_stop(&vi_config);

exit0:
    sample_comm_isp_exit(&isp_param);
    sample_aov_sys_exit();

    return ret;
}

xmedia_s32 sample_aov_dual_sensor_ainr(xmedia_s32 frame_rate, xmedia_s32 suspend_time_ms)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 i = 0;
    xmedia_s32 blk_size = 0;
    xmedia_s32 pool_num = 0;
    xmedia_video_size pic_size[2] = { 0 };
    xmedia_video_size sub_pic_size = { 0 };
    xmedia_video_size npu_pic_size = { 0 };
    xmedia_vb_cal_cfg vb_cfg = {0};
    xmedia_vb_base_info vb_base_info = {0};
    sample_comm_video_param video_param = { 0 };
    sample_sys_config sys_config = { 0 };
    xmedia_s32 media_pipe_num = 2;

    xmedia_s32 vi_dev[2] = {0,1};
    xmedia_s32 vi_pipe[2] = {0,1};
    xmedia_s32 vi_chn = 0;
    sample_vi_config vi_config = { 0 };

    xmedia_s32 sensor_type[2] = {sample_aov_sensor_type[0], sample_aov_sensor_type[1]};
    xmedia_u32 framerate = 0;
    vi_sensor_info sensor_info[2] = { 0 };
    sample_isp_param isp_param = { 0 };
    xmedia_isp_config isp_config = { 0 };
    xmedia_bool mirror = XMEDIA_FALSE;
    xmedia_bool flip = XMEDIA_FALSE;
    bin_module_info pq_info = {0};

    xmedia_s32 vpss_chnl = 0;
    xmedia_s32 vpss_pipe[2] = {0,1};
    xmedia_s32 vpss_ochn[3] = { 0, 1 ,2};
    sample_vpss_config vpss_config = { 0 };
    xmedia_video_size vpss_ochn_size[2][VPSS_MAX_OCHN_NUM] = { 0 };
    xmedia_u32 wrap_buffer_line[2] = {0};
    xmedia_vpss_wrap_buf_share_config wrap_share_cfg = { 0 };

    xmedia_s32 svp_handle = 0;
    sample_comm_npu_workspace_info npu_work_sapce_info = {0};

    xmedia_s32 venc_chnl = 0;
    xmedia_s32 venc_chn[4] = { 0, 1, 2, 3 };
    xmedia_s32 venc_chn_cnt = 4;
    sample_venc_config venc_config = { 0 };
    xmedia_venc_chn_param venc_chn_param;
    sample_aov_init_param aov_init_param = {0};

    misc_aov_isp_info isp_info = {0};
    xmedia_u32 sleep_time = 500; //us
#ifdef SUPPORT_OSD_TIME
    xmedia_chn_info mpp_chn;
    xmedia_rgn_chn_attr rgn_chn_attr = {0};
    xmedia_rgn_overlay_chn_attr *overlay_chn;
#endif

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    framerate = frame_rate;


    // sys init
    for(i = 0; i < VI_MAX_PIPE_NUM; i++) {
        sys_config.sys_conf.pipe_mode[i].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
        sys_config.sys_conf.pipe_mode[i].viproc_vpss_mode = XMEDIA_WORK_MODE_ONLINE;
        sys_config.sys_conf.pipe_mode[i].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    }

    /* wrap VB */
    wrap_share_cfg.enable = XMEDIA_TRUE;
    wrap_share_cfg.wrap_pos = XMEDIA_VPSS_WRAP_POS_PHY_OCHN;

    for(i = 0; i < media_pipe_num; i ++) {
        sample_comm_vi_get_sensor_info(sensor_type[i], &sensor_info[i]);
        
        if (!(sensor_info[i].bit_width == XMEDIA_VIDEO_DATA_WIDTH_10 || sensor_info[i].bit_width == XMEDIA_VIDEO_DATA_WIDTH_12)) {
            SAMPLE_ERR("ainr bit width should be 10bit or 12bit\n");
            return XMEDIA_FAILURE;
        }

        pic_size[i].width = sensor_info[i].width;
        pic_size[i].height = sensor_info[i].height;

        /* wrap VB */
        blk_size = sample_comm_sys_get_wrap_buffer_size(pic_size[i], video_param.video_fmt, video_param.pixel_fmt,
                video_param.data_width, video_param.compress_mode, &wrap_buffer_line[i]);
        if(wrap_share_cfg.enable == XMEDIA_TRUE) {
            wrap_share_cfg.buf_size = (wrap_share_cfg.buf_size > blk_size) ? wrap_share_cfg.buf_size : blk_size;
            if(i == 1) {
                sys_config.vb_conf.common_pool[pool_num].block_size = wrap_share_cfg.buf_size;
                sys_config.vb_conf.common_pool[pool_num].block_cnt = 1;
                pool_num ++;
                SAMPLE_PRT("wrap share vb size : %d \n", wrap_share_cfg.buf_size);
            }
        }
        else {
            sys_config.vb_conf.common_pool[pool_num].block_size = blk_size;
            sys_config.vb_conf.common_pool[pool_num].block_cnt = 1;
            pool_num ++;
            SAMPLE_PRT("sensor%d wrap vb size : %d \n", i, blk_size);
        }
    }

    if(pic_size[0].width * pic_size[0].height >= pic_size[1].width * pic_size[1].height) {
        i = 0;
    }
    else {
        i = 1;
    }
    vb_base_info.width = pic_size[i].width;
    vb_base_info.height = pic_size[i].height;
    vb_base_info.cmp_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;
    vb_base_info.bit_width = sensor_info[i].bit_width;
    vb_base_info.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_RAW;
    vb_base_info.video_fmt = video_param.video_fmt;
    vb_base_info.align = DEFAULT_ALIGN;
    vb_base_info.ainr_attr.ainr_en = XMEDIA_TRUE;
    vb_base_info.ainr_attr.ainr_direction = XMEDIA_VB_AINR_DIRECTION_INPUT;

    xmedia_vb_get_buffer_config(&vb_base_info, &vb_cfg);
    sys_config.vb_conf.common_pool[pool_num].block_size = vb_cfg.vb_size;
    sys_config.vb_conf.common_pool[pool_num].block_cnt = 4;
    pool_num ++;

    // vi proc
    vb_base_info.ainr_attr.ainr_direction = XMEDIA_VB_AINR_DIRECTION_OUTPUT;
    xmedia_vb_get_buffer_config(&vb_base_info, &vb_cfg);
    sys_config.vb_conf.common_pool[pool_num].block_size = vb_cfg.vb_size;
    sys_config.vb_conf.common_pool[pool_num].block_cnt = 2;
    pool_num ++;

    sub_pic_size.width = 640;
    sub_pic_size.height = 480;
    blk_size = sample_comm_sys_get_buffer_size(sub_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[pool_num].block_size = blk_size;
    sys_config.vb_conf.common_pool[pool_num].block_cnt = 6;
    pool_num ++;

    npu_pic_size.width = 640;
    npu_pic_size.height = 360;
    blk_size = sample_comm_sys_get_buffer_size(npu_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[pool_num].block_size = blk_size;
    sys_config.vb_conf.common_pool[pool_num].block_cnt = 6;
    pool_num ++;

    sys_config.vb_conf.max_pool_cnt = pool_num;
    sys_config.vb_conf.supplement_config = 1;

    ret = sample_aov_sys_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("sys init failed !\n");
        return ret;
    }

    isp_info.isp_pipe_num = 2;
    isp_info.isp_pipe[0] = vi_pipe[0];
    isp_info.isp_pipe[1] = vi_pipe[1];
#ifdef SUPPORT_SENSOR_PWDN_MODE
    isp_info.support_sensor_pwdn_standby[0] = XMEDIA_TRUE;
    isp_info.support_sensor_pwdn_standby[1] = XMEDIA_TRUE;
#endif
    sample_comm_aov_misc_init(&isp_info);
    g_misc_fd = sample_comm_aov_get_misc_fd();

    for(i = 0; i < media_pipe_num; i ++) {
        vi_config.dev_info[vi_dev[i]].dev_en = XMEDIA_TRUE;
        vi_config.dev_info[vi_dev[i]].dev_no = vi_dev[i];
        vi_config.dev_info[vi_dev[i]].sensor_type = sensor_type[i];
        vi_config.pipe_info[vi_pipe[i]].pipe_en = XMEDIA_TRUE;
        vi_config.pipe_info[vi_pipe[i]].pipe_no = vi_pipe[i];
        vi_config.pipe_info[vi_pipe[i]].chn_info[vi_chn].chn_en = XMEDIA_TRUE;
        vi_config.pipe_info[vi_pipe[i]].chn_info[vi_chn].chn_no = vi_chn;
        vi_config.dev_bind_pipe[vi_dev[i]].pipe[0] = vi_pipe[i];
        vi_config.dev_bind_pipe[vi_dev[i]].pipe[1] = -1;
        
        if (sensor_type[i] == SMART_SC235HAI_MIPI_2M_2LANE_30FPS_10BIT ||
                sensor_type[i] == SMART_SC485SL_MIPI_4M_30FPS_2LANE_12BIT) {
            mirror = XMEDIA_TRUE;
            flip = XMEDIA_TRUE;
        }

        isp_config.fps = framerate;
        isp_config.mode_config.work_mode = XMEDIA_ISP_WORK_MODE_MASTER;
        isp_config.mode_config.master_mode.blend_stat_enable = XMEDIA_FALSE;
        isp_config.mode_config.master_mode.slave_num = 0;
        isp_config.pixel_fmt = sensor_info[i].pixel_format;
        isp_config.size.height = sensor_info[i].height;
        isp_config.size.width = sensor_info[i].width;
        isp_config.wdr_mode = sensor_info[i].wdr_mode;

        // isp pipe init
        isp_param.pipe[vi_pipe[i]] = vi_pipe[i];
        isp_param.isp_info[vi_pipe[i]].isp_pipe_en = XMEDIA_TRUE;
        isp_param.isp_info[vi_pipe[i]].isp_sensor_en = XMEDIA_TRUE;
        isp_param.isp_info[vi_pipe[i]].sensor_type = sensor_type[i];
        isp_param.isp_info[vi_pipe[i]].disable_dynamic_fps = XMEDIA_TRUE;
        isp_param.isp_info[vi_pipe[i]].mirror = mirror;
        isp_param.isp_info[vi_pipe[i]].flip = flip;
        memcpy(&(isp_param.isp_info[vi_pipe[i]].isp_config), &isp_config, sizeof(xmedia_isp_config));
    }

    // isp init
    ret = sample_comm_isp_init(&isp_param, &vi_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("isp init failed!\n");
        goto exit0;
    }

    // vi start
    ret = sample_comm_vi_start(&vi_config, &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("start vi failed!\n");
        goto exit1;
    }

    // isp start
    ret = sample_comm_isp_start(&isp_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("start isp failed!\n");
        goto exit2;
    }

    // vpss config
    for(i = 0; i < media_pipe_num; i ++) {
        ret = sample_comm_vpss_get_default_pipe_cfg(&vpss_config.pipe_info[vpss_pipe[i]].pipe_config, pic_size[i], &video_param);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("get default pipe cfg failed !\n");
            goto exit2;
        }

        vpss_ochn_size[vpss_pipe[i]][0].width = sensor_info[i].width;
        vpss_ochn_size[vpss_pipe[i]][0].height = sensor_info[i].height;
        vpss_ochn_size[vpss_pipe[i]][1].width = sub_pic_size.width;
        vpss_ochn_size[vpss_pipe[i]][1].height = sub_pic_size.height;
        vpss_ochn_size[vpss_pipe[i]][2].width = npu_pic_size.width;
        vpss_ochn_size[vpss_pipe[i]][2].height = npu_pic_size.height;

        vpss_config.pipe_info[vpss_pipe[i]].pipe_en = XMEDIA_TRUE;
        vpss_config.pipe_info[vpss_pipe[i]].pipe_no = vpss_pipe[i];

        vpss_chnl = vpss_ochn[0];
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_no = vpss_ochn[0];
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].wrap_attr.buf_line = wrap_buffer_line[i];
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].wrap_attr.enable = XMEDIA_TRUE;
        ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config,
                vpss_ochn_size[vpss_pipe[i]][vpss_chnl], &video_param);
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config.cmp_mode = XMEDIA_VIDEO_COMPRESS_MODE_TILE_COMPACT;
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config.video_fmt = XMEDIA_VIDEO_FMT_TILE_32x4;
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("get default pipe cfg failed !\n");
            goto exit2;
        }
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config.depth = 0;

        vpss_chnl = vpss_ochn[1];
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_no = vpss_ochn[1];
        ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config,
                vpss_ochn_size[vpss_pipe[i]][vpss_chnl], &video_param);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("get default pipe cfg failed !\n");
            goto exit2;
        }
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config.depth = 0;

        vpss_chnl = vpss_ochn[2];
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_no = vpss_ochn[2];
        ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config,
                vpss_ochn_size[vpss_pipe[i]][vpss_chnl], &video_param);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("get default pipe cfg failed !\n");
            goto exit2;
        }
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config.depth = 4;
    }

    // set wrap buf share
    if(wrap_share_cfg.enable == XMEDIA_TRUE) {
        ret = xmedia_vpss_set_wrap_buf_share_config(&wrap_share_cfg);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("set wrap buf share failed !\n");
            goto exit2;
        }
    }

    // vpss start
    ret = sample_comm_vpss_start(&vpss_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("vpss start failed !\n");
        goto exit2;
    }

    ret = sample_aov_create_npu_person_detect(&svp_handle);
    if (XMEDIA_SUCCESS != ret) {
        SAMPLE_ERR("npu init fail \n");
        goto exit3;
    }

    npu_work_sapce_info.npu_modle_num = 2;
    npu_work_sapce_info.npu_model_path[0] = ISP_AINR_MODEL_PATH;
    npu_work_sapce_info.npu_model_path[1] = AOV_NPU_MODEL_PATH;

    ret = sample_comm_sys_npu_acquire_workspace(&npu_work_sapce_info);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("acquire workspace fail\n");
        goto exit4;
    }

    for(i = 0; i < media_pipe_num; i ++) {
        isp_param.ainr_model_path[vi_pipe[i]] = npu_work_sapce_info.npu_model_path[0];
    }

    ret = sample_comm_isp_ainr_init(&isp_param, &vi_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("isp init failed!\n");
        goto exit5;
    } else {
        SAMPLE_PRT("isp load ainr success!\n");
    }

    for(i = 0; i < media_pipe_num; i ++) {
        pq_info.vi.enable = XMEDIA_TRUE;
        pq_info.vi.pipe = vi_pipe[i];
        pq_info.mcf.enable = XMEDIA_FALSE;
        pq_info.vpss.enable = XMEDIA_FALSE;

        ret = sample_comm_pq_bin_import(&pq_info, i == 0? SENSOR0_PQ_BIN_PATH:SENSOR1_PQ_BIN_PATH);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("isp load pq bin failed!\n");
            goto exit6;
        } else {
            SAMPLE_PRT("isp load pq success!\n");
        }

        ret = sample_comm_isp_enable_fpn(isp_param.pipe[vi_pipe[i]], i == 0? SENSOR0_FPN_BIN_PATH:SENSOR1_FPN_BIN_PATH);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("isp load fpn bin failed!\n");
            goto exit6;
        } else {
            SAMPLE_PRT("isp load fpn success!\n");
        }
    }

    // venc init
    for(i = 0; i < media_pipe_num; i ++) {
        venc_chnl = venc_chn[2*i+0];
        venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
        venc_config.chn_info[venc_chnl].payload_type = PT_H265;
        venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265AVBR;
        //venc_config.chn_info[venc_chnl].str_buffer_size = ALIGN_UP(pic_size[i].width * pic_size[i].height * 2, 64);
        venc_config.chn_info[venc_chnl].str_buffer_size = ALIGN_UP(pic_size[i].width * pic_size[i].height, 64);
        venc_config.chn_info[venc_chnl].vui_info.vui_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].vui_info.stream_display_fps = frame_rate;
        sample_comm_venc_get_default_chn_info(vpss_ochn_size[vpss_pipe[i]][0], framerate, &venc_config.chn_info[venc_chnl]);

        venc_chnl = venc_chn[2*i+1];
        venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
        venc_config.chn_info[venc_chnl].payload_type = PT_H265;
        venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265AVBR;
        //venc_config.chn_info[venc_chnl].str_buffer_size = ALIGN_UP(sub_pic_size.width * sub_pic_size.height * 2, 64);
        venc_config.chn_info[venc_chnl].str_buffer_size = ALIGN_UP(sub_pic_size.width * sub_pic_size.height, 64);
        venc_config.chn_info[venc_chnl].vui_info.vui_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].vui_info.stream_display_fps = frame_rate;
        sample_comm_venc_get_default_chn_info(vpss_ochn_size[vpss_pipe[i]][1], framerate, &venc_config.chn_info[venc_chnl]);
    }

    ret = sample_comm_venc_start(&venc_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("venc start failed !\n");
        goto exit7;
    }

    for(i = 0; i < media_pipe_num; i ++) {
        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe[i], vpss_ochn[0], venc_chn[2*i+0]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("vpss bind venc failed !\n");
            goto exit8;
        }

        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe[i], vpss_ochn[1], venc_chn[2*i+1]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("vpss bind venc failed !\n");
            goto exit9;
        }
    }

    for(i = 0; i < venc_chn_cnt; i++) {
        ret = xmedia_venc_get_chn_param(venc_chn[i], &venc_chn_param);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("xmedia_venc_get_chn_param failed(0x%x), venc_chn[%d]!\n", ret, venc_chn[i]);
            goto exit9;
        }
        
        venc_chn_param.max_strm_cnt = VENC_STR_BUF_DEFAULT_MAX_FRAME_COUNT;
        
        ret = xmedia_venc_set_chn_param(venc_chn[i], &venc_chn_param);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("xmedia_venc_set_chn_param failed(0x%x), venc_chn[%d]!\n", ret, venc_chn[i]);
            goto exit9;
        }
    }

    aov_init_param.isp_param.isp_pipe_num = 2;
    aov_init_param.vpss_param.vpss_pipe_num = 2;
    aov_init_param.npu_param.npu_detect_num = 2;
    aov_init_param.venc_param.venc_chn_num = venc_chn_cnt;
    
    for(i = 0; i < media_pipe_num; i ++) {
        aov_init_param.isp_param.isp_pipe[i] = isp_param.pipe[vi_pipe[i]];

        //vpss normal, aov should disable ochn
        aov_init_param.vpss_param.vpss_pipe[i] = vpss_pipe[i];
        aov_init_param.vpss_param.vpss_ochn_num[i] = 1;
        aov_init_param.vpss_param.vpss_ochn[i][0] = vpss_ochn[1];

        aov_init_param.npu_param.vpss_pipe[i] = vpss_pipe[i];
        aov_init_param.npu_param.vpss_ochn[i] = vpss_ochn[2];
        aov_init_param.npu_param.npu_svp_handle[i] = svp_handle;
        aov_init_param.npu_param.en_switch_mode[i] = XMEDIA_TRUE;

        aov_init_param.venc_param.venc_chn[2*i] = venc_chn[2*i];
        aov_init_param.venc_param.en_aov[2*i] = XMEDIA_TRUE;
        aov_init_param.venc_param.chn_max_size[2*i] = venc_config.chn_info[venc_chn[2*i]].str_buffer_size;
        aov_init_param.venc_param.chn_threshold_size[2*i] = aov_init_param.venc_param.chn_max_size[2*i] / 5;
        
        aov_init_param.venc_param.venc_chn[2*i+1] = venc_chn[2*i+1];
        aov_init_param.venc_param.en_aov[2*i+1] = XMEDIA_FALSE;
        aov_init_param.venc_param.chn_max_size[2*i+1] = venc_config.chn_info[2*i+1].str_buffer_size;
        aov_init_param.venc_param.chn_threshold_size[2*i+1] = aov_init_param.venc_param.chn_max_size[2*i+1] / 5;
    }

    aov_init_param.suspend_time = suspend_time_ms;
    aov_init_param.npu_notice_callback = sample_aov_set_work_mode_state;
    aov_init_param.normal_resume_callback = sample_aov_normal_resume_func;
    aov_init_param.normal_suspend_callback = sample_aov_normal_suspend_func;

    ret = sample_aov_media_start(&aov_init_param);
    if (XMEDIA_SUCCESS != ret) {
        SAMPLE_ERR("media start fail \n");
        goto exit9;
    }

#ifdef SUPPORT_OSD_TIME
    sample_osd_time_init();

    mpp_chn.mod_id = MOD_ID_VENC;
    mpp_chn.dev_id = 0;
    mpp_chn.chn_id = venc_chn[0];

    rgn_chn_attr.is_show = XMEDIA_TRUE;
    rgn_chn_attr.type = XMEDIA_RGN_TYPE_OVERLAY;

    overlay_chn = &rgn_chn_attr.attr.overlay_chn;
    overlay_chn->alpha0 = 128;
    overlay_chn->alpha1 = 128;
    overlay_chn->qp_info.disable = XMEDIA_TRUE;
    overlay_chn->qp_info.is_abs_qp = XMEDIA_FALSE;
    overlay_chn->qp_info.qp_val = -2;
    overlay_chn->dst = XMEDIA_RGN_ATTACH_JPEG_MAIN;
    overlay_chn->point.x = 0;
    overlay_chn->point.y = 0;
    overlay_chn->layer = 0;

    sample_osd_time_attach_to_chn(&mpp_chn, &rgn_chn_attr);

    mpp_chn.mod_id = MOD_ID_VENC;
    mpp_chn.dev_id = 0;
    mpp_chn.chn_id = venc_chn[1];
    sample_osd_time_attach_to_chn(&mpp_chn, &rgn_chn_attr);

    mpp_chn.mod_id = MOD_ID_VENC;
    mpp_chn.dev_id = 0;
    mpp_chn.chn_id = venc_chn[2];
    sample_osd_time_attach_to_chn(&mpp_chn, &rgn_chn_attr);

    mpp_chn.mod_id = MOD_ID_VENC;
    mpp_chn.dev_id = 0;
    mpp_chn.chn_id = venc_chn[3];
    sample_osd_time_attach_to_chn(&mpp_chn, &rgn_chn_attr);

    sample_osd_time_start();
#endif

    sample_aov_work_mode work_mode = MEDIA_WORK_NORMAL;
    sample_aov_work_mode last_work_mode = MEDIA_WORK_NORMAL;

    while(g_force_exit != XMEDIA_TRUE) {
        sample_aov_get_work_mode_state(&work_mode);
        if (work_mode != last_work_mode) {
            if (work_mode == MEDIA_WORK_AOV) {
                sample_comm_aov_set_work_mode(MEDIA_WORK_AOV);
                last_work_mode = MEDIA_WORK_AOV;
            }

            if (work_mode == MEDIA_WORK_NORMAL) {
                last_work_mode = MEDIA_WORK_NORMAL;
            }
        }

#ifdef SUPPORT_OSD_TIME
        sample_osd_time_update(sleep_time, work_mode);
#endif
        usleep(sleep_time);
    }

    sample_aov_media_stop();
    sample_comm_aov_npu_destroy(svp_handle);

#ifdef SUPPORT_OSD_TIME
    sample_osd_time_destroy();
#endif

exit9:
    for(i = 0; i < media_pipe_num; i ++) {
        sample_comm_sys_vpss_unbind_venc(vpss_pipe[i], vpss_ochn[0], venc_chn[2*i+0]);
        sample_comm_sys_vpss_unbind_venc(vpss_pipe[i], vpss_ochn[1], venc_chn[2*i+1]);
    }
exit8:
    sample_comm_venc_stop(&venc_config);

exit7:
    for(i = 0; i < media_pipe_num; i ++) {
        sample_comm_isp_disable_fpn(isp_param.pipe[vi_pipe[i]]);
    }

exit6:
    sample_comm_isp_ainr_exit(&isp_param);

exit5:
    sample_comm_sys_npu_release_workspace();

exit4:
    sample_comm_aov_npu_destroy(svp_handle);

exit3:
    sample_comm_vpss_stop(&vpss_config);

exit2:
    sample_comm_isp_stop(&isp_param);

exit1:
    sample_comm_vi_stop(&vi_config);

exit0:
    sample_comm_isp_exit(&isp_param);
    sample_aov_sys_exit();

    return ret;
}


xmedia_void sample_aov_usage(xmedia_char *args)
{
    printf("\n"
           "***************************************************************\n"
           "Usage: %s [work_mode] [frame_rate] [suspend_time]\n"
           "work_mode: \n"
           "    0:  aov single sensor stnr olny. \n"
           "    1:  aov single sensor ainr. \n"
           "    2:  aov dual sensor stnr olny. \n"
           "    3:  aov dual sensor ainr. \n"
           "frame_rate: (12 - 25fps). \n"
           "suspend_time: ms (1000). \n"
           "e.g: ./sample_aov 0 25 1000. \n"
           "***************************************************************\n"
           "\n",
           args);
    return;
}

xmedia_void sample_aov_handle_sig(xmedia_s32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo || SIGTERM == signo) {
        g_force_exit = XMEDIA_TRUE;
    }
}

xmedia_s32 main(xmedia_s32 argc, xmedia_char **argv)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_pm_init_param pm_param = {0};
    xmedia_s32 work_mode = 0;
    xmedia_s32 frame_rate = 0;
    xmedia_s32 suspend_time = 0;

    signal(SIGINT, sample_aov_handle_sig);
    signal(SIGTERM, sample_aov_handle_sig);

    if (argc < 4) {
        sample_aov_usage(argv[0]);
        return 0;
    }

    work_mode = atoi(argv[1]);
    frame_rate = atoi(argv[2]);
    suspend_time = atoi(argv[3]);

    pm_param.type = PM_INIT_TYPE_STR;

    ret = xmedia_pm_init_ext(&pm_param);
    if (XMEDIA_SUCCESS != ret) {
        SAMPLE_PRT("pm init fail \n");
    }

    switch (work_mode) {
        case 0:
            ret = sample_aov_single_sensor_stnr_only(frame_rate, suspend_time);
            break;
        case 1:
            ret = sample_aov_single_sensor_ainr(frame_rate, suspend_time);
            break;
        case 2:
            ret = sample_aov_dual_sensor_stnr_only(frame_rate, suspend_time);
            break;
        case 3:
            ret = sample_aov_dual_sensor_ainr(frame_rate, suspend_time);
            break;

        default:
            sample_aov_usage(argv[0]);
            return 0;
    }

    if (XMEDIA_SUCCESS == ret) {
        SAMPLE_PRT("program exit normally!\n");
    } else {
        SAMPLE_PRT("program exit abnormally!\n");
    }

    return 0;
}


