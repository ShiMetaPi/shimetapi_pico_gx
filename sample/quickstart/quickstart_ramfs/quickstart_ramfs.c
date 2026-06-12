#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "quickstart_comm.h"

#define MCU_CPU_SYNC_REG      0x1202014c
#define MCU_END_VALUE         0xbeef0002
#define MCU_ALREADY_END_VALUE 0xbeef0003

#ifdef SMART_SC485SL_MIPI_4M_30FPS_4LANE_12BIT
#define SENSOR_MAX_WIDTH      2688
#define SENSOR_MAX_HEIGHT     1520
#endif

#ifdef SMART_SC485SL_MIPI_4M_30FPS_2LANE_12BIT
#define SENSOR_MAX_WIDTH      2560
#define SENSOR_MAX_HEIGHT     1440
#endif

#ifdef SMART_SC485SL_MIPI_4M_60FPS_4LANE_12BIT
#define SENSOR_MAX_WIDTH      2560
#define SENSOR_MAX_HEIGHT     1440
#endif

#ifdef SMART_SC235HAI_MIPI_2M_2LANE_30FPS_10BIT
#define SENSOR_MAX_WIDTH      1920
#define SENSOR_MAX_HEIGHT     1080
#endif

#define VENC_SUBCHN_WIDTH     640
#define VENC_SUBCHN_HEIGHT    360
#define NPU_CHN_WIDTH       640
#define NPU_CHN_HEIGHT       360
//#define SCENE_AUTO_SUPPORT
#define SUBCHN_SUPPORT

xmedia_void quickstart_media_start(xmedia_void)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_s32 pool_num = 0;
    xmedia_s32 wrap_blk_size = 0;
    xmedia_s32 vi_dev = 0;
    xmedia_s32 vi_pipe = 0;
    xmedia_s32 vpss_ichn = 0;
    xmedia_s32 vpss_ochn[3] = {0, 1, 2};
    xmedia_s32 vpss_pipe = 0;
    xmedia_s32 venc_chn[2] = {0,1};
    media_comm_sys_config sys_cfg = {0};
    xmedia_vb_base_info vb_base_info = {0};
    xmedia_vb_cal_cfg vb_cfg = {0};
    media_comm_isp_cfg isp_cfg = {0};
    media_comm_vi_cfg vi_cfg = {0};
    media_comm_vpss_cfg vpss_cfg = {0};
    media_comm_venc_cfg venc_cfg = {0};

    /*sys init*/
    for (xmedia_s32 i = 0; i < VI_MAX_PIPE_NUM; i++)
    {
        sys_cfg.sys_conf.pipe_mode[i].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
        sys_cfg.sys_conf.pipe_mode[i].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
        sys_cfg.sys_conf.pipe_mode[i].viproc_vpss_mode = XMEDIA_WORK_MODE_ONLINE;
    }
    sys_cfg.sys_conf.pipe_mode[0].vicap_viproc_mode = XMEDIA_WORK_MODE_ONLINE;

    vb_base_info.width = SENSOR_MAX_WIDTH;
    vb_base_info.height = SENSOR_MAX_HEIGHT;
    vb_base_info.cmp_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;
    vb_base_info.bit_width = XMEDIA_VIDEO_DATA_WIDTH_12;//XMEDIA_VIDEO_DATA_WIDTH_12;
    vb_base_info.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_RAW;
    vb_base_info.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    vb_base_info.align = DEFAULT_ALIGN;
    vb_base_info.ainr_attr.ainr_en = XMEDIA_FALSE;

    xmedia_vb_get_buffer_config(&vb_base_info, &vb_cfg);
    sys_cfg.vb_conf.common_pool[pool_num].block_size = vb_cfg.vb_size;
    sys_cfg.vb_conf.common_pool[pool_num].block_cnt = 2;
    pool_num ++;

    vb_base_info.width = SENSOR_MAX_WIDTH;
    vb_base_info.height = SENSOR_MAX_HEIGHT;
    vb_base_info.cmp_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;
    vb_base_info.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    vb_base_info.align = DEFAULT_ALIGN;
    //vpss
    vb_base_info.bit_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    vb_base_info.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    vb_base_info.ainr_attr.ainr_en = XMEDIA_FALSE;

#if 0//wrap disable
    xmedia_vb_get_buffer_config(&vb_base_info, &vb_cfg);
    sys_cfg.vb_conf.common_pool[pool_num].block_size = vb_cfg.vb_size;
    sys_cfg.vb_conf.common_pool[pool_num].block_cnt = 2;
    pool_num ++;
#else
    wrap_blk_size = xmedia_vb_get_wrap_buffer_size(&vb_base_info, SENSOR_MAX_WIDTH / 2);
    sys_cfg.vb_conf.common_pool[pool_num].block_size = wrap_blk_size;
    sys_cfg.vb_conf.common_pool[pool_num].block_cnt = 1;
    pool_num ++;
#endif

    vb_base_info.width = VENC_SUBCHN_WIDTH;
    vb_base_info.height = VENC_SUBCHN_HEIGHT;

    xmedia_vb_get_buffer_config(&vb_base_info, &vb_cfg);
    sys_cfg.vb_conf.common_pool[pool_num].block_size = vb_cfg.vb_size;
    sys_cfg.vb_conf.common_pool[pool_num].block_cnt = 2;
    pool_num ++;

    vb_base_info.width = NPU_CHN_WIDTH;
    vb_base_info.height = NPU_CHN_HEIGHT;

    xmedia_vb_get_buffer_config(&vb_base_info, &vb_cfg);
    sys_cfg.vb_conf.common_pool[pool_num].block_size = vb_cfg.vb_size;
    sys_cfg.vb_conf.common_pool[pool_num].block_cnt = 4;
    pool_num ++;

    sys_cfg.vb_conf.max_pool_cnt = pool_num;
    sys_cfg.vb_conf.supplement_config = 1;

    s32Ret = media_comm_sys_init(&sys_cfg);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_PRT("sys init fail \n");
        goto error0;
    }

    /*media init*/
    s32Ret = media_comm_media_init();
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_PRT("media init fail \n");
        goto error1;
    }

    /*isp init*/
    isp_cfg.en_quickstart = XMEDIA_TRUE;
    isp_cfg.sns_num = MEDIA_SENSOR0;
    isp_cfg.isp_pipe = vi_pipe;
    isp_cfg.vi_dev = vi_dev;

    s32Ret = media_comm_isp_init(isp_cfg);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_PRT("isp init fail \n");
        goto error2;
    }

    /*vi init*/
    vi_cfg.vi_dev_work_num = 1;
    vi_cfg.vi_dev[0] = vi_dev;
    vi_cfg.en_quickstart[0] = XMEDIA_TRUE;
    vi_cfg.vi_grp_param.vi_grp_work_num = 0;
    vi_cfg.vi_pipe_param[0].vi_pipe_work_num = 1;
    vi_cfg.vi_pipe_param[0].vi_pipe[0] = vi_pipe;
    vi_cfg.mipi_cfg[0].mipi_rate = media_comm_isp_get_mipi_rate(isp_cfg.isp_pipe);

#if 0
    
    xmedia_vi_lowdelay_attr vi_cap_lowdelay = {0};
    xmedia_vi_lowdelay_attr vi_proc_lowdelay = {0};
    vi_cap_lowdelay.enable = XMEDIA_TRUE;
    vi_cap_lowdelay.line_cnt = SENSOR_MAX_HEIGHT / 2;
    vi_cap_lowdelay.one_buf_en = XMEDIA_TRUE;

    vi_proc_lowdelay.enable = XMEDIA_TRUE;
    vi_proc_lowdelay.line_cnt = SENSOR_MAX_HEIGHT / 2;
    vi_proc_lowdelay.one_buf_en = XMEDIA_TRUE;

    memcpy(&vi_cfg.vi_pipe_param[0].vi_pipe_info[0].vi_cap_lowdelay, &vi_cap_lowdelay, sizeof(xmedia_vi_lowdelay_attr));
    memcpy(&vi_cfg.vi_pipe_param[0].vi_pipe_info[0].vi_proc_lowdelay, &vi_proc_lowdelay, sizeof(xmedia_vi_lowdelay_attr));
    vi_cfg.vi_pipe_param[0].vi_pipe_info[0].en_vi_cap_lowdelay = XMEDIA_TRUE; //开启 会丢1fps
    //vi_cfg.vi_pipe_param[0].vi_pipe_info[0].en_vi_proc_lowdelay = XMEDIA_TRUE; // vi 离线 vpss在线不支持
#endif

    s32Ret = media_comm_vi_start(&vi_cfg);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_PRT("vi start fail \n");
        goto error3;
    }

    s32Ret = media_comm_isp_start(vi_pipe, XMEDIA_FALSE);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_PRT("isp start fail \n");
        goto error4;
    }

    /*vpss init*/
    vpss_cfg.vpss_pipe_work_num = 1;
    vpss_cfg.vpss_pipe[0] = vpss_pipe;

    vpss_cfg.pipe_cfg[0].max_w = SENSOR_MAX_WIDTH;
    vpss_cfg.pipe_cfg[0].max_h = SENSOR_MAX_HEIGHT;
    vpss_cfg.pipe_cfg[0].dync_range = XMEDIA_VIDEO_DYNAMIC_RANGE_SDR;
    vpss_cfg.pipe_cfg[0].frm_rate_ctrl.src_frm_rate = -1;
    vpss_cfg.pipe_cfg[0].frm_rate_ctrl.dst_frm_rate = -1;
    vpss_cfg.pipe_cfg[0].pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    vpss_cfg.pipe_cfg[0].nr_attr.nr_en = XMEDIA_FALSE;
    vpss_cfg.pipe_cfg[0].nr_attr.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;
    vpss_cfg.pipe_cfg[0].stitch_blend_cfg.stitch_blend_en = XMEDIA_FALSE;

    vpss_cfg.vpss_ochn_info[0].vpss_ochn_work_num = 1;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn[0] = vpss_ochn[0];
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[0].cmp_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[0].depth = 1;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[0].dync_range = XMEDIA_VIDEO_DYNAMIC_RANGE_SDR;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[0].flip_en = XMEDIA_FALSE;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[0].mirror_en = XMEDIA_FALSE;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[0].frm_rate_ctrl.src_frm_rate = -1;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[0].frm_rate_ctrl.dst_frm_rate = -1;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[0].width = SENSOR_MAX_WIDTH;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[0].height = SENSOR_MAX_HEIGHT;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[0].pix_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[0].video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[0].mode = XMEDIA_VPSS_OCHN_MODE_USER;

    xmedia_vpss_low_delay_attr vpss_lowdelay_cfg = {0};
    vpss_lowdelay_cfg.enable = XMEDIA_FALSE;
    vpss_lowdelay_cfg.line_cnt = 16;
    vpss_lowdelay_cfg.one_buf_en = XMEDIA_FALSE;

    xmedia_video_wrap_attr vpss_wrap_cfg = {0};
    vpss_wrap_cfg.enable = XMEDIA_TRUE;
    vpss_wrap_cfg.buf_line = SENSOR_MAX_HEIGHT/2;
    vpss_wrap_cfg.buf_size = wrap_blk_size;

    //memcpy(&vpss_cfg.vpss_ochn_info[0].vpss_lowdelay_cfg[0], &vpss_lowdelay_cfg, sizeof(xmedia_vpss_low_delay_attr));
    memcpy(&vpss_cfg.vpss_ochn_info[0].vpss_wrap_cfg[0], &vpss_wrap_cfg, sizeof(xmedia_video_wrap_attr));

#ifdef SUBCHN_SUPPORT
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_work_num += 1;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn[1] = vpss_ochn[1];
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[1].cmp_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[1].depth = 0;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[1].dync_range = XMEDIA_VIDEO_DYNAMIC_RANGE_SDR;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[1].flip_en = XMEDIA_FALSE;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[1].mirror_en = XMEDIA_FALSE;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[1].frm_rate_ctrl.src_frm_rate = -1;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[1].frm_rate_ctrl.dst_frm_rate = -1;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[1].width = VENC_SUBCHN_WIDTH;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[1].height = VENC_SUBCHN_HEIGHT;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[1].pix_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[1].video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[1].mode = XMEDIA_VPSS_OCHN_MODE_USER;


    vpss_cfg.vpss_ochn_info[0].vpss_ochn_work_num += 1;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn[2] = vpss_ochn[2];
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[2].cmp_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[2].depth = 1;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[2].dync_range = XMEDIA_VIDEO_DYNAMIC_RANGE_SDR;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[2].flip_en = XMEDIA_FALSE;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[2].mirror_en = XMEDIA_FALSE;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[2].frm_rate_ctrl.src_frm_rate = -1;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[2].frm_rate_ctrl.dst_frm_rate = -1;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[2].width = NPU_CHN_WIDTH;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[2].height = NPU_CHN_HEIGHT;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[2].pix_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[2].video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    vpss_cfg.vpss_ochn_info[0].vpss_ochn_cfg[2].mode = XMEDIA_VPSS_OCHN_MODE_USER;
#endif
    vpss_lowdelay_cfg.enable = XMEDIA_FALSE;
    memcpy(&vpss_cfg.vpss_ochn_info[0].vpss_lowdelay_cfg[1], &vpss_lowdelay_cfg, sizeof(xmedia_vpss_low_delay_attr));
    vpss_lowdelay_cfg.enable = XMEDIA_FALSE;
    memcpy(&vpss_cfg.vpss_ochn_info[0].vpss_lowdelay_cfg[2], &vpss_lowdelay_cfg, sizeof(xmedia_vpss_low_delay_attr));


    s32Ret = media_comm_vpss_start(&vpss_cfg);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_PRT("vpss start fail \n");
        goto error5;
    }

    s32Ret = media_comm_vi_bind_vpss(vi_pipe, 0, vpss_pipe, vpss_ichn);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_PRT("vi bind vpss fail \n");
        goto error6;
    }

    /*venc init*/
    venc_cfg.src_frame_rate = 30;
    venc_cfg.dst_frame_rate = 30;
    venc_cfg.max_pic_width = SENSOR_MAX_WIDTH;
    venc_cfg.max_pic_height = SENSOR_MAX_HEIGHT;
    venc_cfg.en_type = PT_H265;
    venc_cfg.profile = 0;
    venc_cfg.rcn_ref_share_buf = XMEDIA_FALSE;
    venc_cfg.rc_mode = VENC_RC_MODE_H265CBR;

    venc_cfg.venc_gop_attr.gop_mode = VENC_GOPMODE_NORMALP;
    venc_cfg.venc_gop_attr.normal_p.ip_qp_delta = 2;

    s32Ret = media_comm_venc_start(venc_chn[0], &venc_cfg);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_PRT("venc start fail \n");
        goto error7;
    }

    s32Ret = media_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_PRT("vpss bind venc fail \n");
        goto error8;
    }

#ifdef SUBCHN_SUPPORT
    /*venc init*/
    venc_cfg.max_pic_width = VENC_SUBCHN_WIDTH;
    venc_cfg.max_pic_height = VENC_SUBCHN_HEIGHT;

    s32Ret = media_comm_venc_start(venc_chn[1], &venc_cfg);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_PRT("venc start fail \n");
        goto error9;
    }

    s32Ret = media_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_PRT("vpss bind venc fail \n");
        goto error10;
    }
#endif
    
#ifdef SCENE_AUTO_SUPPORT
        bin_module_info scene_auto_info = {0};
        scene_auto_info.vi.enable = XMEDIA_TRUE;
        scene_auto_info.vi.pipe = vi_pipe;
    
        media_comm_scene_auto_enable(&scene_auto_info);
#endif

#if 0
    s32Ret = quickstart_comm_venc_start_get_stream(venc_chn, 2);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_PRT("media init fail \n");
    }
#endif

    sleep(30);

#ifdef SUBCHN_SUPPORT
error10:
    media_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
error9:
    media_comm_venc_stop(venc_chn[1]);
#endif

error8:
    media_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
error7:
    media_comm_venc_stop(venc_chn[0]);
error6:
    media_comm_vi_unbind_vpss(vi_pipe, 0, vpss_pipe, vpss_ichn);
error5:
    media_comm_vpss_stop(&vpss_cfg);
error4:
    media_comm_isp_stop(vi_pipe, XMEDIA_FALSE);
error3:
    media_comm_vi_stop(&vi_cfg);
error2:
    media_comm_isp_exit(isp_cfg.isp_pipe, isp_cfg.sns_num);
error1:
    media_comm_media_exit();
error0:
    media_comm_sys_exit();

    return;
}

xmedia_s32 main(int argc, char **argv)
{
    media_comm_mem_open();
    media_comm_set_reg(MCU_CPU_SYNC_REG, MCU_END_VALUE);
    while(1)
    {
        if (MCU_ALREADY_END_VALUE == media_comm_get_reg(MCU_CPU_SYNC_REG))
        {
            break;
        }
        usleep(1000*2);
    }

    quickstart_media_start();

    media_comm_mem_close();
}
