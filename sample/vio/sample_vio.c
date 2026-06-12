/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

#include "sample_comm.h"
#include "sample_comm_sys.h"
#include "sample_comm_vi.h"
#include "sample_comm_vpss.h"
#include "sample_comm_vo.h"
#include "sample_comm_isp.h"
#include "sample_comm_vgs.h"
#include "sample_comm_venc.h"
#include "sample_comm_ircut.h"
#include "sample_comm_livestream.h"
#include "sample_comm_tp.h"

#define SAMPLE_STITCH_DEBUG         0
#define SAMPLE_STITCH_LUMA_ADJ_TIME 50
#define SAMPLE_STITCH_MIN_LUMA_DIFF 2
#define SAMPLE_STITCH_MAX_LUMA_GAIN 128
#define ISP_DYNAMIC_SWITCH_SLEEP_TIME 5000

static sample_comm_sensor_type sample_vio_sensor_type[MAX_SENSOR_NUM] = { SENSOR0_TYPE, SENSOR1_TYPE, SENSOR2_TYPE,
                                                                          SENSOR3_TYPE, SENSOR4_TYPE };
static sample_comm_screen_type sample_vo_screen_type[MAX_SCREEN_NUM] = { SCREEN0_TYPE, SCREEN0_TYPE};
static xmedia_bool g_force_exit = XMEDIA_FALSE;

static xmedia_bool g_venc_use = XMEDIA_TRUE;

xmedia_void sample_vio_usage(xmedia_char *args)
{
    printf("\n"
           "***************************************************************\n"
           "Usage: %s [work_mode] [stream_mode]\n"
           "work_mode: \n"
           "    0:  vi online vpss online\n"
           "    1:  vi offline vpss online (can use for 4K sensor)\n"
           "    2:  vi offline vpss offline\n"
           "    3:  vi online/offline vpss online wdr (default vi online)\n"
           "    4:  vi offline vpss offline switch linear and wdr\n"
           "    5:  vi offline vpss offline switch resolution\n"
           "    6:  vi offline vpss online double sensor\n"
           "    7:  vi offline vpss online double sensor splicing \n"
           "    8:  vi online vpss online double sensor(switch)\n"
           "    9:  vi offline vpss offline preroll switch to normal and then switch to readback\n"
           "   10:  vi offline vpss online ldc\n"
           "   11:  vi offline vpss online ahd mipi\n"
           "stream_mode: (default 1)\n"
           "    0: VO output\n"
           "    1: VENC output\n"
           "e.g: ./sample_vio 0 || ./sample_vio 0 0\n"
           "***************************************************************\n"
           "\n",
           args);
    return;
}

xmedia_s32 sample_vio_sys_init(sample_sys_config *sys_config)
{
    xmedia_s32 ret = 0;

    ret = sample_comm_vb_init(sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vb_init failed!\n");
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

    ret = sample_comm_vo_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vo_init failed!\n");
        return ret;
    }

    ret = sample_comm_venc_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_venc_init failed!\n");
        return ret;
    }

    return XMEDIA_SUCCESS;
}

xmedia_void sample_vio_sys_exit(void)
{
    sample_comm_venc_stop_get_stream();
    sample_comm_venc_exit();
    sample_comm_vo_exit();
    sample_comm_vpss_exit();
    sample_comm_vi_exit();
    sample_comm_vgs_exit();
    sample_comm_sys_exit();
}

xmedia_void sample_vio_handle_sig(xmedia_s32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo || SIGTERM == signo) {
        g_force_exit = XMEDIA_TRUE;
    }
}

xmedia_s32 sample_vio_vi_online_vpss_online()
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 blk_size = 0;
    xmedia_video_size pic_size = { 0 };
    xmedia_video_size sub_pic_size = { 0 };
    sample_comm_video_param video_param = { 0 };
    sample_sys_config sys_config = { 0 };

    xmedia_s32 vi_dev = 0;
    xmedia_s32 vi_pipe = 0;
    xmedia_s32 vi_chn = 0;
    sample_vi_config vi_config = { 0 };

    xmedia_s32 sensor_type = sample_vio_sensor_type[0];
    xmedia_u32 framerate = 0;
    vi_sensor_info sensor_info = { 0 };
    sample_isp_param isp_param = { 0 };
    xmedia_isp_config isp_config = { 0 };
    xmedia_bool mirror = XMEDIA_FALSE;
    xmedia_bool flip = XMEDIA_FALSE;

    xmedia_s32 vpss_chnl = 0;
    xmedia_s32 vpss_pipe = 0;
    xmedia_s32 vpss_ochn[2] = { 0, 1 };
    sample_vpss_config vpss_config = { 0 };
    xmedia_video_size vpss_ochn_size[VPSS_MAX_OCHN_NUM] = { 0 };
    xmedia_u32 wrap_buffer_line = 0;

    xmedia_s32 venc_chnl = 0;
    xmedia_s32 venc_chn[2] = { 0, 1 };
    xmedia_s32 venc_chn_cnt = 2;
    sample_venc_config venc_config = { 0 };

    xmedia_s32 vo_dev = 0;
    xmedia_s32 vo_layer = vo_dev;
    xmedia_s32 vo_chn[1] = { 0 };
    xmedia_s32 vo_chn_num = 1;
    sample_vo_config_input vo_input = { 0 };
    sample_vo_config vo_config = { 0 };
    vo_input.square_sort = 1;
    vo_input.screen_type = sample_vo_screen_type[0];

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

    sys_config.compress_level = XMEDIA_SYS_COMPRESS_LEVEL_0;

    ret = sample_comm_sys_cmp_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("common sys failed !\n");
        return ret;
    }

    sys_config.vb_conf.max_pool_cnt = 20;

    pic_size.width = sensor_info.width;
    pic_size.height = sensor_info.height;

    /* wrap VB */
    blk_size = sample_comm_sys_get_wrap_buffer_size(pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                                    video_param.data_width, video_param.compress_mode, &wrap_buffer_line);
    sys_config.vb_conf.common_pool[0].block_size = blk_size;
    sys_config.vb_conf.common_pool[0].block_cnt = 1;
    SAMPLE_PRT("wrap vb size : %d \n", blk_size);

    sub_pic_size.width = 640;
    sub_pic_size.height = 480;
    blk_size = sample_comm_sys_get_buffer_size(sub_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[1].block_size = blk_size;
    sys_config.vb_conf.common_pool[1].block_cnt = 3;

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_vio_sys_init(&sys_config);
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

    if (g_venc_use == XMEDIA_TRUE) {
        // venc init
        app_liveserver_init();
        app_liveserver_start(vpss_ochn_size, venc_chn_cnt, framerate);

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

        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind venc failed !\n");
            goto exit5;
        }

        ret = sample_comm_venc_start_get_stream(venc_chn, venc_chn_cnt, XMEDIA_FALSE, XMEDIA_FALSE, ".");
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("venc start get stream failed !\n");
            goto exit6;
        }
    } else {
        // vo init
        ret = sample_comm_vo_get_default_config(vo_input, &vo_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get vo default config failed !\n");
            goto exit3;
        }

        ret = sample_comm_vo_start(vo_dev, vo_layer, vo_chn, vo_chn_num, XMEDIA_TRUE, &vo_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vo start failed !\n");
            goto exit3;
        }

        ret = sample_comm_sys_vpss_bind_vo(vpss_pipe, vpss_ochn[0], vo_layer, vo_chn[0]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind vo failed !\n");
            goto exit4;
        }
    }

    PAUSE(g_force_exit);

    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_venc_stop_get_stream();
    }

exit6:
    if (g_venc_use == XMEDIA_TRUE) {
        app_liveserver_stop();
        app_liveserver_deinit();
        sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
    }

exit5:
    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
    } else {
        sample_comm_sys_vpss_unbind_vo(vpss_pipe, vpss_ochn[0], vo_layer, vo_chn[0]);
    }

exit4:
    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_venc_stop(&venc_config);
    } else {
        sample_comm_vo_stop(vo_dev, vo_layer, vo_chn, vo_chn_num);
    }

exit3:
    sample_comm_vpss_stop(&vpss_config);

exit2:
    sample_comm_isp_stop(&isp_param);

exit1:
    sample_comm_vi_stop(&vi_config);

exit0:
    sample_comm_isp_exit(&isp_param);
    sample_vio_sys_exit();

    return ret;
}

xmedia_s32 sample_vio_vi_offline_vpss_online()
{
    xmedia_s32 i = 0;
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 blk_size = 0;
    xmedia_video_size pic_size = { 0 };
    xmedia_video_size sub_pic_size = { 0 };
    sample_comm_video_param video_param = { 0 };
    sample_sys_config sys_config = { 0 };

    xmedia_s32 vi_dev = 0;
    xmedia_s32 vi_pipe = 0;
    xmedia_s32 vi_chn = 0;
    sample_vi_config vi_config = { 0 };

    xmedia_s32 sensor_type = sample_vio_sensor_type[0];
    xmedia_u32 framerate = 0;
    vi_sensor_info sensor_info = { 0 };
    sample_isp_param isp_param = { 0 };
    xmedia_isp_config isp_config = { 0 };
    xmedia_bool mirror = XMEDIA_FALSE;
    xmedia_bool flip = XMEDIA_FALSE;

    xmedia_s32 vpss_chnl = 0;
    xmedia_s32 vpss_pipe = 0;
    xmedia_s32 vpss_ochn[2] = { 0, 1 };
    sample_vpss_config vpss_config = { 0 };
    xmedia_video_size vpss_ochn_size[VPSS_MAX_OCHN_NUM] = { 0 };

    xmedia_s32 venc_chnl = 0;
    xmedia_s32 venc_chn[2] = { 0, 1 };
    xmedia_s32 venc_chn_cnt = 2;
    sample_venc_config venc_config = { 0 };

    xmedia_s32 vo_dev = 0;
    xmedia_s32 vo_layer = vo_dev;
    xmedia_s32 vo_chn[1] = { 0 };
    xmedia_s32 vo_chn_num = 1;
    sample_vo_config_input vo_input = { 0 };
    sample_vo_config vo_config = { 0 };
    vo_input.square_sort = 1;
    vo_input.screen_type = sample_vo_screen_type[0];

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);

    // sys init
    for(i = 0; i < VI_MAX_PIPE_NUM; i ++)
    {
        sys_config.sys_conf.pipe_mode[i].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
        sys_config.sys_conf.pipe_mode[i].viproc_vpss_mode = XMEDIA_WORK_MODE_ONLINE;
        sys_config.sys_conf.pipe_mode[i].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    }

    sys_config.compress_level = XMEDIA_SYS_COMPRESS_LEVEL_0;

    ret = sample_comm_sys_cmp_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("common sys failed !\n");
        return ret;
    }

    sys_config.vb_conf.max_pool_cnt = 20;

    pic_size.width = sensor_info.width;
    pic_size.height = sensor_info.height;

    /* vicap-viproc offline VB */
    blk_size = sample_comm_sys_get_buffer_size(pic_size, video_param.video_fmt, sensor_info.pixel_format,
                                               sensor_info.bit_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[0].block_size = blk_size;
    sys_config.vb_conf.common_pool[0].block_cnt = 1;
    SAMPLE_PRT("offline vb size : %d \n", blk_size);

    /* VPSS-VENC VB */
    blk_size = sample_comm_sys_get_buffer_size(pic_size, XMEDIA_VIDEO_FMT_TILE_32x4, video_param.pixel_fmt,
                                               video_param.data_width, XMEDIA_VIDEO_COMPRESS_MODE_TILE);
    sys_config.vb_conf.common_pool[1].block_size = blk_size;
    sys_config.vb_conf.common_pool[1].block_cnt = 1;
    SAMPLE_PRT("vpss-venc vb size : %d \n", blk_size);

    sub_pic_size.width = 640;
    sub_pic_size.height = 480;
    blk_size = sample_comm_sys_get_buffer_size(sub_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[2].block_size = blk_size;
    sys_config.vb_conf.common_pool[2].block_cnt = 2;

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_vio_sys_init(&sys_config);
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
    vi_config.pipe_info[vi_pipe].lowdelay_attr.enable = XMEDIA_TRUE;
    vi_config.pipe_info[vi_pipe].lowdelay_attr.line_cnt = sensor_info.height / 2;
    vi_config.pipe_info[vi_pipe].lowdelay_attr.one_buf_en = XMEDIA_TRUE;
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
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].wrap_attr.enable = XMEDIA_TRUE;
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].wrap_attr.buf_line = sensor_info.height;
    ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
                                                vpss_ochn_size[vpss_chnl], &video_param);
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.depth = 0;

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

    if (g_venc_use == XMEDIA_TRUE) {
        // venc init
        app_liveserver_init();
        app_liveserver_start(vpss_ochn_size, venc_chn_cnt, framerate);

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

        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind venc failed !\n");
            goto exit5;
        }

        ret = sample_comm_venc_start_get_stream(venc_chn, venc_chn_cnt, XMEDIA_FALSE, XMEDIA_FALSE, ".");
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("venc start get stream failed !\n");
            goto exit6;
        }
    } else {
        // vo init
        ret = sample_comm_vo_get_default_config(vo_input, &vo_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get vo default config failed !\n");
            goto exit3;
        }

        ret = sample_comm_vo_start(vo_dev, vo_layer, vo_chn, vo_chn_num, XMEDIA_TRUE, &vo_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vo start failed !\n");
            goto exit3;
        }

        ret = sample_comm_sys_vpss_bind_vo(vpss_pipe, vpss_ochn[0], vo_layer, vo_chn[0]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind vo failed !\n");
            goto exit4;
        }
    }

    PAUSE(g_force_exit);

    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_venc_stop_get_stream();
    }

exit6:
    if (g_venc_use == XMEDIA_TRUE) {
        app_liveserver_stop();
        app_liveserver_deinit();
        sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
    }

exit5:
    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
    } else {
        sample_comm_sys_vpss_unbind_vo(vpss_pipe, vpss_ochn[0], vo_layer, vo_chn[0]);
    }

exit4:
    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_venc_stop(&venc_config);
    } else {
        sample_comm_vo_stop(vo_dev, vo_layer, vo_chn, vo_chn_num);
    }

exit3:
    sample_comm_vpss_stop(&vpss_config);

exit2:
    sample_comm_isp_stop(&isp_param);

exit1:
    sample_comm_vi_stop(&vi_config);

exit0:
    sample_comm_isp_exit(&isp_param);
    sample_vio_sys_exit();

    return ret;
}

xmedia_s32 sample_vio_vi_offline_vpss_offline()
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 blk_size = 0;
    xmedia_video_size pic_size = { 0 };
    xmedia_video_size sub_pic_size = { 0 };
    sample_comm_video_param video_param = { 0 };
    sample_sys_config sys_config = { 0 };

    xmedia_s32 vi_dev = 0;
    xmedia_s32 vi_pipe = 0;
    xmedia_s32 vi_chn = 0;
    sample_vi_config vi_config = { 0 };

    xmedia_s32 sensor_type = sample_vio_sensor_type[0];
    xmedia_u32 framerate = 0;
    vi_sensor_info sensor_info = { 0 };
    sample_isp_param isp_param = { 0 };
    xmedia_isp_config isp_config = { 0 };
    xmedia_bool mirror = XMEDIA_FALSE;
    xmedia_bool flip = XMEDIA_FALSE;

    xmedia_s32 vpss_chnl = 0;
    xmedia_s32 vpss_pipe = 0;
    xmedia_s32 vpss_ichn = 0;
    xmedia_s32 vpss_ochn[2] = { 0, 1 };
    sample_vpss_config vpss_config = { 0 };
    xmedia_video_size vpss_ochn_size[VPSS_MAX_OCHN_NUM] = { 0 };

    xmedia_s32 venc_chnl = 0;
    xmedia_s32 venc_chn[2] = { 0, 1 };
    xmedia_s32 venc_chn_cnt = 2;
    sample_venc_config venc_config = { 0 };

    xmedia_s32 vo_dev = 0;
    xmedia_s32 vo_layer = vo_dev;
    xmedia_s32 vo_chn[1] = { 0 };
    xmedia_s32 vo_chn_num = 1;
    sample_vo_config_input vo_input = { 0 };
    sample_vo_config vo_config = { 0 };
    vo_input.screen_type = sample_vo_screen_type[0];
    vo_input.square_sort = 1;

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);

    // sys init
    sys_config.sys_conf.pipe_mode[vi_pipe].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[vi_pipe].viproc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[vi_pipe].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;

    sys_config.compress_level = XMEDIA_SYS_COMPRESS_LEVEL_0;

    ret = sample_comm_sys_cmp_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("common sys failed !\n");
        return ret;
    }

    sys_config.vb_conf.max_pool_cnt = 25;

    pic_size.width = sensor_info.width;
    pic_size.height = sensor_info.height;
    blk_size = sample_comm_sys_get_buffer_size(pic_size, video_param.video_fmt, sensor_info.pixel_format,
                                               sensor_info.bit_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[0].block_size = blk_size;
    sys_config.vb_conf.common_pool[0].block_cnt = 2;

    blk_size = sample_comm_sys_get_buffer_size(pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[1].block_size = blk_size;
    sys_config.vb_conf.common_pool[1].block_cnt = 3;

    sub_pic_size.width = 640;
    sub_pic_size.height = 480;
    blk_size = sample_comm_sys_get_buffer_size(sub_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[2].block_size = blk_size;
    sys_config.vb_conf.common_pool[2].block_cnt = 3;

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_vio_sys_init(&sys_config);
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

    framerate = 15;  // 全离线vb占用过多，默认用15fps
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
    ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
                                                vpss_ochn_size[vpss_chnl], &video_param);
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.depth = 0;
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

    ret = sample_comm_sys_vi_bind_vpss(vi_pipe, vi_chn, vpss_pipe, vpss_ichn);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vi bind vpss failed !\n");
        goto exit3;
    }

    if (g_venc_use == XMEDIA_TRUE) {
        // venc init
        app_liveserver_init();
        app_liveserver_start(vpss_ochn_size, venc_chn_cnt, framerate);

        venc_chnl = venc_chn[0];
        venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
        venc_config.chn_info[venc_chnl].payload_type = PT_H265;
        venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
        sample_comm_venc_get_default_chn_info(vpss_ochn_size[0], framerate, &venc_config.chn_info[venc_chnl]);
        venc_config.chn_info[venc_chnl].rcn_ref_share_buf = XMEDIA_TRUE;

        venc_chnl = venc_chn[1];
        venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
        venc_config.chn_info[venc_chnl].payload_type = PT_H265;
        venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
        sample_comm_venc_get_default_chn_info(vpss_ochn_size[1], framerate, &venc_config.chn_info[venc_chnl]);
        venc_config.chn_info[venc_chnl].rcn_ref_share_buf = XMEDIA_TRUE;

        ret = sample_comm_venc_start(&venc_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("venc start failed !\n");
            goto exit4;
        }

        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind venc failed !\n");
            goto exit5;
        }

        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind venc failed !\n");
            goto exit6;
        }

        ret = sample_comm_venc_start_get_stream(venc_chn, venc_chn_cnt, XMEDIA_FALSE, XMEDIA_FALSE, ".");
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("venc start get stream failed !\n");
            goto exit7;
        }
    } else {
        // vo init
        ret = sample_comm_vo_get_default_config(vo_input, &vo_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get vo default config failed !\n");
            goto exit4;
        }

        ret = sample_comm_vo_start(vo_dev, vo_layer, vo_chn, vo_chn_num, XMEDIA_TRUE, &vo_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vo start failed !\n");
            goto exit4;
        }

        ret = sample_comm_sys_vpss_bind_vo(vpss_pipe, vpss_ochn[0], vo_layer, vo_chn[0]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind vo failed !\n");
            goto exit5;
        }
    }

    PAUSE(g_force_exit);

    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_venc_stop_get_stream();
    }

exit7:
    if (g_venc_use == XMEDIA_TRUE) {
        app_liveserver_stop();
        app_liveserver_deinit();
        sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
    }

exit6:
    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
    } else {
        sample_comm_sys_vpss_unbind_vo(vpss_pipe, vpss_ochn[0], vo_layer, vo_chn[0]);
    }

exit5:
    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_venc_stop(&venc_config);
    } else {
        sample_comm_vo_stop(vo_dev, vo_layer, vo_chn, vo_chn_num);
    }

exit4:
    sample_comm_sys_vi_unbind_vpss(vi_pipe, vi_chn, vpss_pipe, vpss_ichn);

exit3:
    sample_comm_vpss_stop(&vpss_config);

exit2:
    sample_comm_isp_stop(&isp_param);

exit1:
    sample_comm_vi_stop(&vi_config);

exit0:
    sample_comm_isp_exit(&isp_param);
    sample_vio_sys_exit();

    return ret;
}

xmedia_s32 sample_vio_vi_offline_vpss_offline_wdr()
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 blk_size = 0;
    xmedia_video_size pic_size = { 0 };
    xmedia_video_size sub_pic_size = { 0 };
    sample_comm_video_param video_param = { 0 };
    sample_sys_config sys_config = { 0 };

    xmedia_s32 vi_dev = 0;
    xmedia_s32 vi_pipe0 = 0;
    xmedia_s32 vi_pipe1 = 1;
    xmedia_s32 vi_grp = 0;
    xmedia_s32 vi_chn = 0;
    sample_vi_config vi_config = { 0 };
    sample_vi_wdr_param wdr_param = { 0 };

    xmedia_s32 sensor_type = sample_vio_sensor_type[0];
    xmedia_u32 framerate = 0;
    vi_sensor_info sensor_info = { 0 };
    sample_isp_param isp_param = { 0 };
    xmedia_isp_config isp_config = { 0 };
    xmedia_bool mirror = XMEDIA_FALSE;
    xmedia_bool flip = XMEDIA_FALSE;

    xmedia_s32 vpss_chnl = 0;
    xmedia_s32 vpss_pipe = 0;
    xmedia_s32 vpss_ichn = 0;
    xmedia_s32 vpss_ochn[2] = { 0, 1 };
    sample_vpss_config vpss_config = { 0 };
    xmedia_video_size vpss_ochn_size[VPSS_MAX_OCHN_NUM] = { 0 };
    xmedia_u32 wrap_buffer_line = 0;

    xmedia_s32 venc_chnl = 0;
    xmedia_s32 venc_chn[2] = { 0, 1 };
    xmedia_s32 venc_chn_cnt = 2;
    sample_venc_config venc_config = { 0 };

    xmedia_s32 vo_dev = 0;
    xmedia_s32 vo_layer = vo_dev;
    xmedia_s32 vo_chn[1] = { 0 };
    xmedia_s32 vo_chn_num = 1;
    sample_vo_config_input vo_input = { 0 };
    sample_vo_config vo_config = { 0 };

    vo_input.intf_sync = XMEDIA_VO_INTF_SYNC_800x480_50;
    vo_input.intf_type = XMEDIA_INTF_TYPE_LCD;
    vo_input.square_sort = 1;

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    sensor_type = sample_comm_vi_get_wdr_sensor_type(sensor_type);

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);

    // sys init
    for(int i = 0; i < VI_MAX_PIPE_NUM; i ++) {
        sys_config.sys_conf.pipe_mode[i].vicap_viproc_mode = (i == 0) ? XMEDIA_WORK_MODE_ONLINE : XMEDIA_WORK_MODE_OFFLINE;
        sys_config.sys_conf.pipe_mode[i].viproc_vpss_mode = XMEDIA_WORK_MODE_ONLINE;
        sys_config.sys_conf.pipe_mode[i].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    }

    sys_config.compress_level = XMEDIA_SYS_COMPRESS_LEVEL_0;

    ret = sample_comm_sys_cmp_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("common sys failed !\n");
        return ret;
    }

    sys_config.vb_conf.max_pool_cnt = 25;

    pic_size.width = sensor_info.width;
    pic_size.height = sensor_info.height;

    if(sys_config.sys_conf.pipe_mode[0].vicap_viproc_mode == XMEDIA_WORK_MODE_OFFLINE)
    {
        blk_size = sample_comm_sys_get_buffer_size(pic_size, video_param.video_fmt, sensor_info.pixel_format,
                                                sensor_info.bit_width, video_param.compress_mode);
        sys_config.vb_conf.common_pool[0].block_size = blk_size;
        sys_config.vb_conf.common_pool[0].block_cnt = 4;
    }

    blk_size = sample_comm_sys_get_wrap_buffer_size(pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                            video_param.data_width, video_param.compress_mode, &wrap_buffer_line);
    sys_config.vb_conf.common_pool[1].block_size = blk_size;
    sys_config.vb_conf.common_pool[1].block_cnt = 1;

    sub_pic_size.width = 640;
    sub_pic_size.height = 480;
    blk_size = sample_comm_sys_get_buffer_size(sub_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[2].block_size = blk_size;
    sys_config.vb_conf.common_pool[2].block_cnt = 3;

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_vio_sys_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sys init failed !\n");
        return ret;
    }

    wdr_param.vi_dev      = vi_dev;
    wdr_param.vi_pipe[0]  = vi_pipe0;
    wdr_param.vi_pipe[1]  = vi_pipe1;
    wdr_param.vi_grp      = vi_grp;
    wdr_param.vi_chn      = vi_chn;
    wdr_param.sensor_type = sensor_type;
    wdr_param.wdr_mode    = sensor_info.wdr_mode;

    if(sys_config.sys_conf.pipe_mode[0].vicap_viproc_mode == XMEDIA_WORK_MODE_ONLINE)
    {
        wdr_param.cache_line  = pic_size.height / 2;
    }
    sample_comm_vi_wdr_config_init(&wdr_param, &vi_config);

    isp_config.fps = framerate;
    isp_config.mode_config.work_mode = XMEDIA_ISP_WORK_MODE_MASTER;
    isp_config.mode_config.master_mode.blend_stat_enable = XMEDIA_FALSE;
    isp_config.mode_config.master_mode.slave_num = 0;
    isp_config.pixel_fmt = sensor_info.pixel_format;
    isp_config.size.height = sensor_info.height;
    isp_config.size.width = sensor_info.width;
    isp_config.wdr_mode = sensor_info.wdr_mode;

    // isp pipe init
    isp_param.pipe[vi_pipe0] = vi_pipe0;
    isp_param.isp_info[vi_pipe0].isp_pipe_en = XMEDIA_TRUE;
    isp_param.isp_info[vi_pipe0].isp_sensor_en = XMEDIA_TRUE;
    isp_param.isp_info[vi_pipe0].sensor_type = sensor_type;
    isp_param.isp_info[vi_pipe0].mirror = mirror;
    isp_param.isp_info[vi_pipe0].flip = flip;
    memcpy(&(isp_param.isp_info[vi_pipe0].isp_config), &isp_config, sizeof(xmedia_isp_config));

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
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.depth = 0;

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

    ret = sample_comm_sys_vi_bind_vpss(vi_pipe0, vi_chn, vpss_pipe, vpss_ichn);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vi bind vpss failed !\n");
        goto exit3;
    }

    if (g_venc_use == XMEDIA_TRUE) {
        // venc init
        app_liveserver_init();
        app_liveserver_start(vpss_ochn_size, venc_chn_cnt, framerate);

        venc_chnl = venc_chn[0];
        venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
        venc_config.chn_info[venc_chnl].payload_type = PT_H265;
        venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
        sample_comm_venc_get_default_chn_info(vpss_ochn_size[0], framerate, &venc_config.chn_info[venc_chnl]);
        venc_config.chn_info[venc_chnl].rcn_ref_share_buf = XMEDIA_TRUE;

        venc_chnl = venc_chn[1];
        venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
        venc_config.chn_info[venc_chnl].payload_type = PT_H265;
        venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
        sample_comm_venc_get_default_chn_info(vpss_ochn_size[1], framerate, &venc_config.chn_info[venc_chnl]);
        venc_config.chn_info[venc_chnl].rcn_ref_share_buf = XMEDIA_TRUE;

        ret = sample_comm_venc_start(&venc_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("venc start failed !\n");
            goto exit4;
        }

        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind venc failed !\n");
            goto exit5;
        }

        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind venc failed !\n");
            goto exit6;
        }

        ret = sample_comm_venc_start_get_stream(venc_chn, venc_chn_cnt, XMEDIA_FALSE, XMEDIA_FALSE, ".");
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("venc start get stream failed !\n");
            goto exit7;
        }
    } else {
        // vo init
        ret = sample_comm_vo_get_default_config(vo_input, &vo_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get vo default config failed !\n");
            goto exit4;
        }

        ret = sample_comm_vo_start(vo_dev, vo_layer, vo_chn, vo_chn_num, XMEDIA_TRUE, &vo_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vo start failed !\n");
            goto exit4;
        }

        ret = sample_comm_sys_vpss_bind_vo(vpss_pipe, vpss_ochn[0], vo_layer, vo_chn[0]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind vo failed !\n");
            goto exit5;
        }
    }

    PAUSE(g_force_exit);

    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_venc_stop_get_stream();
    }

exit7:
    if (g_venc_use == XMEDIA_TRUE) {
        app_liveserver_stop();
        app_liveserver_deinit();
        sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
    }

exit6:
    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
    } else {
        sample_comm_sys_vpss_unbind_vo(vpss_pipe, vpss_ochn[0], vo_layer, vo_chn[0]);
    }

exit5:
    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_venc_stop(&venc_config);
    } else {
        sample_comm_vo_stop(vo_dev, vo_layer, vo_chn, vo_chn_num);
    }

exit4:
    sample_comm_sys_vi_unbind_vpss(vi_pipe0, vi_chn, vpss_pipe, vpss_ichn);

exit3:
    sample_comm_vpss_stop(&vpss_config);

exit2:
    sample_comm_isp_stop(&isp_param);

exit1:
    sample_comm_vi_stop(&vi_config);

exit0:
    sample_comm_isp_exit(&isp_param);
    sample_vio_sys_exit();

    return ret;
}

static xmedia_s32 sample_vio_switch_to_linear(sample_vi_config *vi_config, sample_comm_video_param *video_param,
                                                                                                xmedia_s32 venc_chn[2])
{
    vi_sensor_info     sensor_info  = { 0 };
    xmedia_isp_attr    isp_attr     = { 0 };
    xmedia_s32         ret          = XMEDIA_SUCCESS;
    xmedia_s32         sensor_type  = sample_vio_sensor_type[0];
    xmedia_s32         vi_dev       = 0;
    xmedia_s32         vi_pipe      = 0;
    xmedia_s32         vi_chn       = 0;
    xmedia_u32         fps          = 0;
    xmedia_u8          mipi_lane[4] = {1, 2, 4 ,8};
    xmedia_u64         start, end;
    xmedia_sensor_attr attr         = {0};

    xmedia_venc_recv_pic_param recv_param = { 0 };
    recv_param.recv_pic_num               = -1;

    sample_comm_vi_stop(vi_config);
    xmedia_sensor_stop(vi_pipe);
    sample_comm_vi_exit();
    xmedia_venc_stop_recv_frame(venc_chn[0]);
    xmedia_venc_stop_recv_frame(venc_chn[1]);
    xmedia_venc_reset_chn(venc_chn[0]);
    xmedia_venc_reset_chn(venc_chn[1]);
    xmedia_venc_start_recv_frame(venc_chn[0], &recv_param);
    xmedia_venc_start_recv_frame(venc_chn[1], &recv_param);
    sample_comm_vi_init();

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &fps);

    ret = xmedia_sensor_set_mipi_lanes(vi_pipe, mipi_lane[sensor_info.lane_mode]);
    CHECK_RET(ret, "sensor set mipi_lanes");

    attr.width    = sensor_info.width;
    attr.height   = sensor_info.height;
    attr.wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
    ret = xmedia_sensor_set_attr(vi_pipe, &attr);
    CHECK_RET(ret, "sensor set attr");

    xmedia_sys_get_cur_pts(&start);
    xmedia_isp_get_attr(vi_pipe, &isp_attr);
    isp_attr.wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
    isp_attr.fps      = fps;
    xmedia_isp_set_attr(vi_pipe, &isp_attr);
    do {
        usleep(ISP_DYNAMIC_SWITCH_SLEEP_TIME);
        xmedia_isp_get_attr(vi_pipe, &isp_attr);
    } while (isp_attr.wdr_mode != XMEDIA_VIDEO_WDR_MODE_NONE);
    xmedia_sys_get_cur_pts(&end);
    SAMPLE_PRT("switch to linear cost time: %llu us.\n", end - start);

    memset(vi_config, 0, sizeof(sample_vi_config));
    vi_config->dev_info[vi_dev].dev_en                    = XMEDIA_TRUE;
    vi_config->dev_info[vi_dev].dev_no                    = vi_dev;
    vi_config->dev_info[vi_dev].sensor_type               = sensor_type;
    vi_config->pipe_info[vi_pipe].pipe_en                 = XMEDIA_TRUE;
    vi_config->pipe_info[vi_pipe].pipe_no                 = vi_pipe;
    vi_config->pipe_info[vi_pipe].chn_info[vi_chn].chn_en = XMEDIA_TRUE;
    vi_config->pipe_info[vi_pipe].chn_info[vi_chn].chn_no = vi_chn;
    vi_config->dev_bind_pipe[vi_dev].pipe[0]              = vi_pipe;
    vi_config->dev_bind_pipe[vi_dev].pipe[1]              = -1;

    ret = sample_comm_vi_start(vi_config, video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vi_start failed !\n");
        return ret;
    }

    xmedia_sensor_start(vi_pipe);

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_vio_switch_to_wdr(sample_vi_config *vi_config, sample_comm_video_param *video_param,
                                                                                                xmedia_s32 venc_chn[2])

{
    vi_sensor_info     sensor_info  = { 0 };
    xmedia_isp_attr    isp_attr     = { 0 };
    xmedia_s32         ret          = XMEDIA_SUCCESS;
    xmedia_s32         sensor_type  = sample_vio_sensor_type[0];
    xmedia_s32         vi_dev       = 0;
    xmedia_s32         vi_chn       = 0;
    xmedia_s32         vi_pipe0     = 0;
    xmedia_s32         vi_pipe1     = 1;
    xmedia_s32         vi_grp       = 0;
    xmedia_u32         fps          = 0;
    xmedia_u8          mipi_lane[4] = {1, 2, 4 ,8};
    xmedia_u64         start, end;
    xmedia_sensor_attr attr         = {0};

    xmedia_venc_recv_pic_param recv_param = { 0 };
    recv_param.recv_pic_num               = -1;

    sample_comm_vi_stop(vi_config);
    xmedia_sensor_stop(vi_pipe0);
    sample_comm_vi_exit();
    xmedia_venc_stop_recv_frame(venc_chn[0]);
    xmedia_venc_stop_recv_frame(venc_chn[1]);
    xmedia_venc_reset_chn(venc_chn[0]);
    xmedia_venc_reset_chn(venc_chn[1]);
    xmedia_venc_start_recv_frame(venc_chn[0], &recv_param);
    xmedia_venc_start_recv_frame(venc_chn[1], &recv_param);
    sample_comm_vi_init();

    sensor_type = sample_comm_vi_get_wdr_sensor_type(sensor_type);
    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &fps);

    ret = xmedia_sensor_set_mipi_lanes(vi_pipe0, mipi_lane[sensor_info.lane_mode]);
    CHECK_RET(ret, "sensor set mipi_lanes");

    attr.width    = sensor_info.width;
    attr.height   = sensor_info.height;
    attr.wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
    ret = xmedia_sensor_set_attr(vi_pipe0, &attr);
    CHECK_RET(ret, "sensor set attr");

    xmedia_sys_get_cur_pts(&start);
    xmedia_isp_get_attr(vi_pipe0, &isp_attr);
    isp_attr.wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
    isp_attr.fps      = fps;
    xmedia_isp_set_attr(vi_pipe0, &isp_attr);
    do {
        usleep(ISP_DYNAMIC_SWITCH_SLEEP_TIME);
        xmedia_isp_get_attr(vi_pipe0, &isp_attr);
    } while (isp_attr.wdr_mode != XMEDIA_VIDEO_WDR_MODE_2TO1_LINE);
    xmedia_sys_get_cur_pts(&end);
    SAMPLE_PRT("switch to wdr cost time: %llu us.\n", end - start);

    memset(vi_config, 0, sizeof(sample_vi_config));
    vi_config->dev_info[vi_dev].dev_en                               = XMEDIA_TRUE;
    vi_config->dev_info[vi_dev].dev_no                               = vi_dev;
    vi_config->dev_info[vi_dev].sensor_type                          = sensor_type;
    vi_config->pipe_info[vi_pipe0].pipe_en                           = XMEDIA_TRUE;
    vi_config->pipe_info[vi_pipe0].pipe_no                           = vi_pipe0;
    vi_config->pipe_info[vi_pipe0].chn_info[vi_chn].chn_en           = XMEDIA_TRUE;
    vi_config->pipe_info[vi_pipe0].chn_info[vi_chn].chn_no           = vi_chn;
    vi_config->pipe_info[vi_pipe1].pipe_en                           = XMEDIA_TRUE;
    vi_config->pipe_info[vi_pipe1].pipe_no                           = vi_pipe1;
    vi_config->pipe_info[vi_pipe1].chn_info[vi_chn].chn_en           = XMEDIA_FALSE;
    vi_config->pipe_info[vi_pipe1].chn_info[vi_chn].chn_no           = vi_chn;
    vi_config->dev_bind_pipe[vi_dev].pipe[0]                         = vi_pipe0;
    vi_config->dev_bind_pipe[vi_dev].pipe[1]                         = vi_pipe1;
    vi_config->grp_info[vi_grp].grp                                  = vi_grp;
    vi_config->grp_info[vi_grp].grp_en                               = XMEDIA_TRUE;
    vi_config->grp_info[vi_grp].grp_config.grp_type                  = XMEDIA_VI_GRP_TYPE_WDR;
    vi_config->grp_info[vi_grp].grp_config.wdr_grp_config.pipe_id[0] = vi_pipe0;
    vi_config->grp_info[vi_grp].grp_config.wdr_grp_config.pipe_id[1] = vi_pipe1;
    vi_config->grp_info[vi_grp].grp_config.wdr_grp_config.wdr_mode   = sensor_info.wdr_mode;
    vi_config->grp_info[vi_grp].grp_config.wdr_grp_config.cache_line = sensor_info.height >> 1;

    ret = sample_comm_vi_start(vi_config, video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("start vi failed!\n");
        return ret;
    }

    xmedia_sensor_start(vi_pipe0);

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_vio_vi_offline_vpss_offline_switch_linear_wdr()
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 blk_size = 0;
    xmedia_video_size pic_size = { 0 };
    xmedia_video_size sub_pic_size = { 0 };
    sample_comm_video_param video_param = { 0 };
    sample_sys_config sys_config = { 0 };

    xmedia_s32 vi_dev = 0;
    xmedia_s32 vi_pipe = 0;
    xmedia_s32 vi_chn = 0;
    sample_vi_config vi_config = { 0 };

    xmedia_s32 sensor_type = sample_vio_sensor_type[0];
    xmedia_u32 framerate = 0;
    vi_sensor_info sensor_info = { 0 };
    sample_isp_param isp_param = { 0 };
    xmedia_isp_config isp_config = { 0 };
    xmedia_bool mirror = XMEDIA_FALSE;
    xmedia_bool flip = XMEDIA_FALSE;

    xmedia_s32 vpss_chnl = 0;
    xmedia_s32 vpss_pipe = 0;
    xmedia_s32 vpss_ichn = 0;
    xmedia_s32 vpss_ochn[2] = { 0, 1 };
    sample_vpss_config vpss_config = { 0 };
    xmedia_video_size vpss_ochn_size[VPSS_MAX_OCHN_NUM] = { 0 };

    xmedia_s32 venc_chnl = 0;
    xmedia_s32 venc_chn[2] = { 0, 1 };
    xmedia_s32 venc_chn_cnt = 2;
    sample_venc_config venc_config = { 0 };

    xmedia_s32 vo_dev = 0;
    xmedia_s32 vo_layer = vo_dev;
    xmedia_s32 vo_chn[1] = { 0 };
    xmedia_s32 vo_chn_num = 1;
    sample_vo_config_input vo_input = { 0 };
    sample_vo_config vo_config = { 0 };
    vo_input.screen_type = sample_vo_screen_type[0];
    vo_input.square_sort = 1;

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);

    // sys init
    sys_config.sys_conf.pipe_mode[vi_pipe].vicap_viproc_mode = XMEDIA_WORK_MODE_ONLINE;
    sys_config.sys_conf.pipe_mode[vi_pipe].viproc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[vi_pipe].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;

    sys_config.compress_level = XMEDIA_SYS_COMPRESS_LEVEL_0;

    ret = sample_comm_sys_cmp_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("common sys failed !\n");
        return ret;
    }

    sys_config.vb_conf.max_pool_cnt = 25;

    pic_size.width = sensor_info.width;
    pic_size.height = sensor_info.height;
    blk_size = sample_comm_sys_get_buffer_size(pic_size, video_param.video_fmt, sensor_info.pixel_format,
                                               sensor_info.bit_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[0].block_size = blk_size;
    sys_config.vb_conf.common_pool[0].block_cnt = 4;

    blk_size = sample_comm_sys_get_buffer_size(pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[1].block_size = blk_size;
    sys_config.vb_conf.common_pool[1].block_cnt = 5;

    sub_pic_size.width = 640;
    sub_pic_size.height = 480;
    blk_size = sample_comm_sys_get_buffer_size(sub_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[2].block_size = blk_size;
    sys_config.vb_conf.common_pool[2].block_cnt = 3;

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_vio_sys_init(&sys_config);
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
    ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
                                                vpss_ochn_size[vpss_chnl], &video_param);
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

    ret = sample_comm_sys_vi_bind_vpss(vi_pipe, vi_chn, vpss_pipe, vpss_ichn);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vi bind vpss failed !\n");
        goto exit3;
    }

    if (g_venc_use == XMEDIA_TRUE) {
        // venc init
        app_liveserver_init();
        app_liveserver_start(vpss_ochn_size, venc_chn_cnt, framerate);

        venc_chnl = venc_chn[0];
        venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
        venc_config.chn_info[venc_chnl].payload_type = PT_H265;
        venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
        sample_comm_venc_get_default_chn_info(vpss_ochn_size[0], framerate, &venc_config.chn_info[venc_chnl]);
        venc_config.chn_info[venc_chnl].rcn_ref_share_buf = XMEDIA_TRUE;

        venc_chnl = venc_chn[1];
        venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
        venc_config.chn_info[venc_chnl].payload_type = PT_H265;
        venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
        sample_comm_venc_get_default_chn_info(vpss_ochn_size[1], framerate, &venc_config.chn_info[venc_chnl]);
        venc_config.chn_info[venc_chnl].rcn_ref_share_buf = XMEDIA_TRUE;

        ret = sample_comm_venc_start(&venc_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("venc start failed !\n");
            goto exit4;
        }

        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind venc failed !\n");
            goto exit5;
        }

        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind venc failed !\n");
            goto exit6;
        }

        ret = sample_comm_venc_start_get_stream(venc_chn, venc_chn_cnt, XMEDIA_TRUE, XMEDIA_FALSE, ".");
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("venc start get stream failed !\n");
            goto exit7;
        }
    } else {
        // vo init
        ret = sample_comm_vo_get_default_config(vo_input, &vo_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get vo default config failed !\n");
            goto exit4;
        }

        ret = sample_comm_vo_start(vo_dev, vo_layer, vo_chn, vo_chn_num, XMEDIA_TRUE, &vo_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vo start failed !\n");
            goto exit4;
        }

        ret = sample_comm_sys_vpss_bind_vo(vpss_pipe, vpss_ochn[0], vo_layer, vo_chn[0]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind vo failed !\n");
            goto exit5;
        }
    }

    /* ----------------------------------------------------------------- */
    /* ---------- dynamic switch between linear and wdr begin ---------- */
    /* ----------------------------------------------------------------- */
    printf("---------- press enter to switch linear to 2to1wdr. ----------\n");
    PAUSE(g_force_exit);
    sample_vio_switch_to_wdr(&vi_config, &video_param, venc_chn);

    printf("---------- press enter to switch 2to1wdr to linear. ----------\n");
    PAUSE(g_force_exit);
    sample_vio_switch_to_linear(&vi_config, &video_param, venc_chn);
    /* --------------------------------------------------------------- */
    /* ---------- dynamic switch between linear and wdr end ---------- */
    /* --------------------------------------------------------------- */

    PAUSE(g_force_exit);

    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_venc_stop_get_stream();
    }

exit7:
    if (g_venc_use == XMEDIA_TRUE) {
        app_liveserver_stop();
        app_liveserver_deinit();
        sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
    }

exit6:
    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
    } else {
        sample_comm_sys_vpss_unbind_vo(vpss_pipe, vpss_ochn[0], vo_layer, vo_chn[0]);
    }

exit5:
    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_venc_stop(&venc_config);
    } else {
        sample_comm_vo_stop(vo_dev, vo_layer, vo_chn, vo_chn_num);
    }

exit4:
    sample_comm_sys_vi_unbind_vpss(vi_pipe, vi_chn, vpss_pipe, vpss_ichn);

exit3:
    sample_comm_vpss_stop(&vpss_config);

exit2:
    sample_comm_isp_stop(&isp_param);

exit1:
    sample_comm_vi_stop(&vi_config);

exit0:
    sample_comm_isp_exit(&isp_param);
    sample_vio_sys_exit();

    return ret;
}

static xmedia_s32 sample_vio_set_isp_nr_param(xmedia_s32 pipe)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_vi_pipe_config vi_pipe_config = { 0 };
    xmedia_isp_bnr_attr bnr_attr = { 0 };
    xmedia_isp_stnr_attr stnr_attr = { 0 };

    ret = xmedia_vi_get_pipe_config(pipe, &vi_pipe_config);
    CHECK_RET(ret, "get vi pipe attr");

    ret = xmedia_isp_get_bnr_attr(pipe, &bnr_attr);
    CHECK_RET(ret, "get bnr attr");

    bnr_attr.enable = vi_pipe_config.bnr_attr.bnr_en;
    bnr_attr.tnr_enable = vi_pipe_config.bnr_attr.bnr_en;
    ret = xmedia_isp_set_bnr_attr(pipe, &bnr_attr);
    CHECK_RET(ret, "set bnr attr");

    // STNR
    ret = xmedia_isp_get_stnr_attr(pipe, &stnr_attr);
    CHECK_RET(ret, "get stnr attr");

    stnr_attr.stnr_enable = vi_pipe_config.stnr_attr.stnr_en;
    stnr_attr.tnr_enable = vi_pipe_config.stnr_attr.stnr_en;
    stnr_attr.cnr_enable = vi_pipe_config.stnr_attr.stnr_en;
    ret = xmedia_isp_set_stnr_attr(pipe, &stnr_attr);
    CHECK_RET(ret, "set stnr attr");

    return ret;
}

static xmedia_s32 sample_vio_switch_to_2M(sample_vi_config *vi_config, sample_comm_video_param *video_param)
{
    xmedia_s32         ret          = XMEDIA_SUCCESS;
    xmedia_s32         vi_dev       = 0;
    xmedia_s32         vi_pipe      = 0;
    xmedia_s32         vi_chn       = 0;
    xmedia_u32         fps          = 0;
    vi_sensor_info     sensor_info  = { 0 };
    xmedia_s32         sensor_type  = OMNIVISION_OS08A20_MIPI_2M_30FPS_10BIT;
    xmedia_isp_attr    isp_attr     = { 0 };
    xmedia_u8          mipi_lane[4] = {1, 2, 4 ,8};
    xmedia_u64         start, end;
    xmedia_sensor_attr attr = {0};

    sample_comm_vi_stop(vi_config);
    xmedia_sensor_stop(vi_pipe);

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &fps);

    ret = xmedia_sensor_set_mipi_lanes(vi_pipe, mipi_lane[sensor_info.lane_mode]);
    CHECK_RET(ret, "sensor set mipi_lanes");

    attr.width    = sensor_info.width;
    attr.height   = sensor_info.height;
    attr.wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
    ret = xmedia_sensor_set_attr(vi_pipe, &attr);
    CHECK_RET(ret, "sensor set attr");

    xmedia_sys_get_cur_pts(&start);
    xmedia_isp_get_attr(vi_pipe, &isp_attr);
    isp_attr.size.width  = sensor_info.width;
    isp_attr.size.height = sensor_info.height;
    isp_attr.fps         = fps;
    xmedia_isp_set_attr(vi_pipe, &isp_attr);
    do {
        usleep(ISP_DYNAMIC_SWITCH_SLEEP_TIME);
        xmedia_isp_get_attr(vi_pipe, &isp_attr);
    } while (isp_attr.size.width != sensor_info.width || isp_attr.size.height != sensor_info.height);
    xmedia_sys_get_cur_pts(&end);
    SAMPLE_PRT("switch to 2M cost time: %llu us.\n", end - start);

    memset(vi_config, 0, sizeof(sample_vi_config));
    vi_config->dev_info[vi_dev].dev_en                    = XMEDIA_TRUE;
    vi_config->dev_info[vi_dev].dev_no                    = vi_dev;
    vi_config->dev_info[vi_dev].sensor_type               = sensor_type;
    vi_config->pipe_info[vi_pipe].pipe_en                 = XMEDIA_TRUE;
    vi_config->pipe_info[vi_pipe].pipe_no                 = vi_pipe;
    vi_config->pipe_info[vi_pipe].chn_info[vi_chn].chn_en = XMEDIA_TRUE;
    vi_config->pipe_info[vi_pipe].chn_info[vi_chn].chn_no = vi_chn;
    vi_config->dev_bind_pipe[vi_dev].pipe[0]              = vi_pipe;
    vi_config->dev_bind_pipe[vi_dev].pipe[1]              = -1;

    ret = sample_comm_vi_start(vi_config, video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("start vi failed!\n");
        return ret;
    }

    sample_vio_set_isp_nr_param(vi_pipe);

    xmedia_sensor_start(vi_pipe);

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_vio_switch_to_8M(sample_vi_config *vi_config, sample_comm_video_param *video_param)
{
    xmedia_s32         ret          = XMEDIA_SUCCESS;
    xmedia_s32         vi_dev       = 0;
    xmedia_s32         vi_pipe      = 0;
    xmedia_s32         vi_chn       = 0;
    xmedia_u32         fps          = 0;
    vi_sensor_info     sensor_info  = { 0 };
    xmedia_s32         sensor_type  = OMNIVISION_OS08A20_MIPI_8M_20FPS_10BIT;
    xmedia_isp_attr    isp_attr     = { 0 };
    xmedia_u8          mipi_lane[4] = {1, 2, 4 ,8};
    xmedia_u64         start, end;
    xmedia_sensor_attr attr = {0};

    sample_comm_vi_stop(vi_config);
    xmedia_sensor_stop(vi_pipe);

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &fps);

    ret = xmedia_sensor_set_mipi_lanes(vi_pipe, mipi_lane[sensor_info.lane_mode]);
    CHECK_RET(ret, "sensor set mipi_lanes");

    attr.width    = sensor_info.width;
    attr.height   = sensor_info.height;
    attr.wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
    ret = xmedia_sensor_set_attr(vi_pipe, &attr);
    CHECK_RET(ret, "sensor set attr");

    xmedia_sys_get_cur_pts(&start);
    xmedia_isp_get_attr(vi_pipe, &isp_attr);
    isp_attr.size.width  = sensor_info.width;
    isp_attr.size.height = sensor_info.height;
    isp_attr.fps         = fps;
    xmedia_isp_set_attr(vi_pipe, &isp_attr);
    do {
        usleep(ISP_DYNAMIC_SWITCH_SLEEP_TIME);
        xmedia_isp_get_attr(vi_pipe, &isp_attr);
    } while (isp_attr.size.width != sensor_info.width || isp_attr.size.height != sensor_info.height);
    xmedia_sys_get_cur_pts(&end);
    SAMPLE_PRT("switch to 8M cost time: %llu us.\n", end - start);

    memset(vi_config, 0, sizeof(sample_vi_config));
    vi_config->dev_info[vi_dev].dev_en                    = XMEDIA_TRUE;
    vi_config->dev_info[vi_dev].dev_no                    = vi_dev;
    vi_config->dev_info[vi_dev].sensor_type               = sensor_type;
    vi_config->pipe_info[vi_pipe].pipe_en                 = XMEDIA_TRUE;
    vi_config->pipe_info[vi_pipe].pipe_no                 = vi_pipe;
    vi_config->pipe_info[vi_pipe].chn_info[vi_chn].chn_en = XMEDIA_TRUE;
    vi_config->pipe_info[vi_pipe].chn_info[vi_chn].chn_no = vi_chn;
    vi_config->dev_bind_pipe[vi_dev].pipe[0]              = vi_pipe;
    vi_config->dev_bind_pipe[vi_dev].pipe[1]              = -1;

    // vi start
    ret = sample_comm_vi_start(vi_config, video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("start vi failed!\n");
        return ret;
    }

    sample_vio_set_isp_nr_param(vi_pipe);

    xmedia_sensor_start(vi_pipe);

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_vio_switch_to_readback(sample_vi_config *vi_config, sample_comm_video_param *video_param)
{
    xmedia_s32             ret          = XMEDIA_SUCCESS;
    xmedia_s32             vi_dev       = 0;
    xmedia_s32             vi_pipe      = 0;
    xmedia_s32             vi_chn       = 0;
    vi_sensor_info         sensor_info  = { 0 };
    xmedia_s32             sensor_type  = CVSENS_CV4003_MIPI_1M_120FPS_10BIT_PREROLL_READBACK;
    xmedia_sensor_property property     = { 0 };
    xmedia_isp_attr        isp_attr     = { 0 };
    xmedia_u8              mipi_lane[4] = {1, 2, 4 ,8};
    xmedia_sensor_attr     attr         = {0};
    xmedia_u64             start, end;

    sample_comm_vi_stop(vi_config);
    xmedia_sensor_stop(vi_pipe);
    usleep(1000000);

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);

    ret = xmedia_sensor_set_mipi_lanes(vi_pipe, mipi_lane[sensor_info.lane_mode]);
    CHECK_RET(ret, "sensor set mipi_lanes");

    attr.width    = sensor_info.width;
    attr.height   = sensor_info.height;
    attr.wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
    ret = xmedia_sensor_set_attr(vi_pipe, &attr);
    CHECK_RET(ret, "sensor set attr");

    xmedia_sys_get_cur_pts(&start);
    xmedia_sensor_get_property(vi_pipe, &property);
    xmedia_isp_get_attr(vi_pipe, &isp_attr);
    isp_attr.size.width  = sensor_info.width;
    isp_attr.size.height = sensor_info.height;
    isp_attr.fps         = property.max_fps;

    xmedia_isp_set_attr(vi_pipe, &isp_attr);
    do {
        usleep(ISP_DYNAMIC_SWITCH_SLEEP_TIME);
        xmedia_isp_get_attr(vi_pipe, &isp_attr);
    } while (isp_attr.size.width != sensor_info.width || isp_attr.size.height != sensor_info.height);
    xmedia_sys_get_cur_pts(&end);
    SAMPLE_PRT("switch to readback cost time: %llu us.\n", end - start);

    memset(vi_config, 0, sizeof(sample_vi_config));
    vi_config->dev_info[vi_dev].dev_en                    = XMEDIA_TRUE;
    vi_config->dev_info[vi_dev].dev_no                    = vi_dev;
    vi_config->dev_info[vi_dev].sensor_type               = sensor_type;
    vi_config->pipe_info[vi_pipe].pipe_en                 = XMEDIA_TRUE;
    vi_config->pipe_info[vi_pipe].pipe_no                 = vi_pipe;
    vi_config->pipe_info[vi_pipe].chn_info[vi_chn].chn_en = XMEDIA_TRUE;
    vi_config->pipe_info[vi_pipe].chn_info[vi_chn].chn_no = vi_chn;
    vi_config->dev_bind_pipe[vi_dev].pipe[0]              = vi_pipe;
    vi_config->dev_bind_pipe[vi_dev].pipe[1]              = -1;

    // vi start
    ret = sample_comm_vi_start(vi_config, video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("start vi failed!\n");
        return ret;
    }

    sample_vio_set_isp_nr_param(vi_pipe);

    xmedia_sensor_start(vi_pipe);

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_vio_vi_offline_vpss_offline_switch_resolution()
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 blk_size = 0;
    xmedia_video_size pic_size = { 0 };
    xmedia_video_size sub_pic_size = { 0 };
    sample_comm_video_param video_param = { 0 };
    sample_sys_config sys_config = { 0 };

    xmedia_s32 vi_dev = 0;
    xmedia_s32 vi_pipe = 0;
    xmedia_s32 vi_chn = 0;
    sample_vi_config vi_config = { 0 };

    xmedia_s32 sensor_type = OMNIVISION_OS08A20_MIPI_2M_30FPS_10BIT;
    xmedia_u32 framerate = 0;
    vi_sensor_info sensor_info = { 0 };
    sample_isp_param isp_param = { 0 };
    xmedia_isp_config isp_config = { 0 };
    xmedia_bool mirror = XMEDIA_FALSE;
    xmedia_bool flip = XMEDIA_FALSE;

    xmedia_s32 vpss_chnl = 0;
    xmedia_s32 vpss_pipe = 0;
    xmedia_s32 vpss_ichn = 0;
    xmedia_s32 vpss_ochn[2] = { 0, 1 };
    sample_vpss_config vpss_config = { 0 };
    xmedia_video_size vpss_ochn_size[VPSS_MAX_OCHN_NUM] = { 0 };

    xmedia_s32 venc_chnl = 0;
    xmedia_s32 venc_chn[2] = { 0, 1 };
    xmedia_s32 venc_chn_cnt = 2;
    sample_venc_config venc_config = { 0 };

    xmedia_s32 vo_dev = 0;
    xmedia_s32 vo_layer = vo_dev;
    xmedia_s32 vo_chn[1] = { 0 };
    xmedia_s32 vo_chn_num = 1;
    sample_vo_config_input vo_input = { 0 };
    sample_vo_config vo_config = { 0 };
    vo_input.screen_type = sample_vo_screen_type[0];
    vo_input.square_sort = 1;

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);

    // sys init
    sys_config.sys_conf.pipe_mode[vi_pipe].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[vi_pipe].viproc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[vi_pipe].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;

    sys_config.compress_level = XMEDIA_SYS_COMPRESS_LEVEL_0;

    ret = sample_comm_sys_cmp_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("common sys failed !\n");
        return ret;
    }

    sys_config.vb_conf.max_pool_cnt = 25;

    pic_size.width = 3840;//sensor_info.width;
    pic_size.height = 2160;//sensor_info.height;
    blk_size = sample_comm_sys_get_buffer_size(pic_size, video_param.video_fmt, sensor_info.pixel_format,
                                               sensor_info.bit_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[0].block_size = blk_size;
    sys_config.vb_conf.common_pool[0].block_cnt = 2;

    blk_size = sample_comm_sys_get_buffer_size(pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[1].block_size = blk_size;
    sys_config.vb_conf.common_pool[1].block_cnt = 3;

    sub_pic_size.width = 640;
    sub_pic_size.height = 480;
    blk_size = sample_comm_sys_get_buffer_size(sub_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[2].block_size = blk_size;
    sys_config.vb_conf.common_pool[2].block_cnt = 3;

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_vio_sys_init(&sys_config);
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

    framerate = 15;  // 全离线vb占用过多，默认用15fps
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
    ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
                                                vpss_ochn_size[vpss_chnl], &video_param);
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.depth = 0;
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

    ret = sample_comm_sys_vi_bind_vpss(vi_pipe, vi_chn, vpss_pipe, vpss_ichn);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vi bind vpss failed !\n");
        goto exit3;
    }

    if (g_venc_use == XMEDIA_TRUE) {
        // venc init
        app_liveserver_init();
        app_liveserver_start(vpss_ochn_size, venc_chn_cnt, framerate);

        venc_chnl = venc_chn[0];
        venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
        venc_config.chn_info[venc_chnl].payload_type = PT_H265;
        venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
        sample_comm_venc_get_default_chn_info(vpss_ochn_size[0], framerate, &venc_config.chn_info[venc_chnl]);
        venc_config.chn_info[venc_chnl].rcn_ref_share_buf = XMEDIA_TRUE;

        venc_chnl = venc_chn[1];
        venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
        venc_config.chn_info[venc_chnl].payload_type = PT_H265;
        venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
        sample_comm_venc_get_default_chn_info(vpss_ochn_size[1], framerate, &venc_config.chn_info[venc_chnl]);
        venc_config.chn_info[venc_chnl].rcn_ref_share_buf = XMEDIA_TRUE;

        ret = sample_comm_venc_start(&venc_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("venc start failed !\n");
            goto exit4;
        }

        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind venc failed !\n");
            goto exit5;
        }

        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind venc failed !\n");
            goto exit6;
        }

        ret = sample_comm_venc_start_get_stream(venc_chn, venc_chn_cnt, XMEDIA_FALSE, XMEDIA_FALSE, ".");
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("venc start get stream failed !\n");
            goto exit7;
        }
    } else {
        // vo init
        ret = sample_comm_vo_get_default_config(vo_input, &vo_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get vo default config failed !\n");
            goto exit4;
        }

        ret = sample_comm_vo_start(vo_dev, vo_layer, vo_chn, vo_chn_num, XMEDIA_TRUE, &vo_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vo start failed !\n");
            goto exit4;
        }

        ret = sample_comm_sys_vpss_bind_vo(vpss_pipe, vpss_ochn[0], vo_layer, vo_chn[0]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind vo failed !\n");
            goto exit5;
        }
    }

    /* ------------------------------------------------------------ */
    /* ---------- dynamic switch between 2M and 8M begin ---------- */
    /* ------------------------------------------------------------ */
    printf("---------- press enter to switch 2M to 8M. ----------\n");
    PAUSE(g_force_exit);
    sample_vio_switch_to_8M(&vi_config, &video_param);

    printf("---------- press enter to switch 8M to 2M. ----------\n");
    PAUSE(g_force_exit);
    sample_vio_switch_to_2M(&vi_config, &video_param);
    /* --------------------------------------------------------------- */
    /* ---------- dynamic switch between linear and wdr end ---------- */
    /* --------------------------------------------------------------- */
    PAUSE(g_force_exit);

    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_venc_stop_get_stream();
    }

exit7:
    if (g_venc_use == XMEDIA_TRUE) {
        app_liveserver_stop();
        app_liveserver_deinit();
        sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
    }

exit6:
    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
    } else {
        sample_comm_sys_vpss_unbind_vo(vpss_pipe, vpss_ochn[0], vo_layer, vo_chn[0]);
    }

exit5:
    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_venc_stop(&venc_config);
    } else {
        sample_comm_vo_stop(vo_dev, vo_layer, vo_chn, vo_chn_num);
    }

exit4:
    sample_comm_sys_vi_unbind_vpss(vi_pipe, vi_chn, vpss_pipe, vpss_ichn);

exit3:
    sample_comm_vpss_stop(&vpss_config);

exit2:
    sample_comm_isp_stop(&isp_param);

exit1:
    sample_comm_vi_stop(&vi_config);

exit0:
    sample_comm_isp_exit(&isp_param);
    sample_vio_sys_exit();

    return ret;
}

xmedia_s32 sample_vio_vi_offline_vpss_offline_preroll_normal_readback()
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 blk_size = 0;
    xmedia_video_size pic_size = { 0 };
    xmedia_video_size sub_pic_size = { 0 };
    sample_comm_video_param video_param = { 0 };
    sample_sys_config sys_config = { 0 };

    xmedia_s32 vi_dev = 0;
    xmedia_s32 vi_pipe = 0;
    xmedia_s32 vi_chn = 0;
    sample_vi_config vi_config = { 0 };

    xmedia_s32 sensor_type = CVSENS_CV4003_MIPI_4M_30FPS_10BIT;
    xmedia_u32 framerate = 0;
    vi_sensor_info sensor_info = { 0 };
    sample_isp_param isp_param = { 0 };
    xmedia_isp_config isp_config = { 0 };
    xmedia_bool mirror = XMEDIA_FALSE;
    xmedia_bool flip = XMEDIA_FALSE;

    xmedia_s32 vpss_chnl = 0;
    xmedia_s32 vpss_pipe = 0;
    xmedia_s32 vpss_ichn = 0;
    xmedia_s32 vpss_ochn[2] = { 0, 1 };
    sample_vpss_config vpss_config = { 0 };
    xmedia_video_size vpss_ochn_size[VPSS_MAX_OCHN_NUM] = { 0 };

    xmedia_s32 venc_chnl = 0;
    xmedia_s32 venc_chn[2] = { 0, 1 };
    xmedia_s32 venc_chn_cnt = 2;
    sample_venc_config venc_config = { 0 };

    xmedia_s32 vo_dev = 0;
    xmedia_s32 vo_layer = vo_dev;
    xmedia_s32 vo_chn[1] = { 0 };
    xmedia_s32 vo_chn_num = 1;
    sample_vo_config_input vo_input = { 0 };
    sample_vo_config vo_config = { 0 };
    vo_input.screen_type = sample_vo_screen_type[0];
    vo_input.square_sort = 1;

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);

    // sys init
    sys_config.sys_conf.pipe_mode[vi_pipe].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[vi_pipe].viproc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[vi_pipe].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;

    sys_config.compress_level = XMEDIA_SYS_COMPRESS_LEVEL_0;

    ret = sample_comm_sys_cmp_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("common sys failed !\n");
        return ret;
    }

    sys_config.vb_conf.max_pool_cnt = 25;

    pic_size.width = sensor_info.width;//sensor_info.width;
    pic_size.height = sensor_info.width;//sensor_info.height;
    blk_size = sample_comm_sys_get_buffer_size(pic_size, video_param.video_fmt, sensor_info.pixel_format,
                                               sensor_info.bit_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[0].block_size = blk_size;
    sys_config.vb_conf.common_pool[0].block_cnt = 2;

    blk_size = sample_comm_sys_get_buffer_size(pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[1].block_size = blk_size;
    sys_config.vb_conf.common_pool[1].block_cnt = 3;

    sub_pic_size.width = 640;
    sub_pic_size.height = 480;
    blk_size = sample_comm_sys_get_buffer_size(sub_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[2].block_size = blk_size;
    sys_config.vb_conf.common_pool[2].block_cnt = 3;

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_vio_sys_init(&sys_config);
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

    //isp_init完成之后，sensor进入预录阶段
    PAUSE(g_force_exit);

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
    ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
                                                vpss_ochn_size[vpss_chnl], &video_param);
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.depth = 0;
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

    ret = sample_comm_sys_vi_bind_vpss(vi_pipe, vi_chn, vpss_pipe, vpss_ichn);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vi bind vpss failed !\n");
        goto exit3;
    }

    if (g_venc_use == XMEDIA_TRUE) {
        // venc init
        app_liveserver_init();
        app_liveserver_start(vpss_ochn_size, venc_chn_cnt, framerate);

        venc_chnl = venc_chn[0];
        venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
        venc_config.chn_info[venc_chnl].payload_type = PT_H265;
        venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
        sample_comm_venc_get_default_chn_info(vpss_ochn_size[0], framerate, &venc_config.chn_info[venc_chnl]);
        venc_config.chn_info[venc_chnl].rcn_ref_share_buf = XMEDIA_TRUE;

        venc_chnl = venc_chn[1];
        venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
        venc_config.chn_info[venc_chnl].payload_type = PT_H265;
        venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
        sample_comm_venc_get_default_chn_info(vpss_ochn_size[1], framerate, &venc_config.chn_info[venc_chnl]);
        venc_config.chn_info[venc_chnl].rcn_ref_share_buf = XMEDIA_TRUE;

        ret = sample_comm_venc_start(&venc_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("venc start failed !\n");
            goto exit4;
        }

        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind venc failed !\n");
            goto exit5;
        }

        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind venc failed !\n");
            goto exit6;
        }

        ret = sample_comm_venc_start_get_stream(venc_chn, venc_chn_cnt, XMEDIA_TRUE, XMEDIA_FALSE, ".");
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("venc start get stream failed !\n");
            goto exit7;
        }
    } else {
        // vo init
        ret = sample_comm_vo_get_default_config(vo_input, &vo_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get vo default config failed !\n");
            goto exit4;
        }

        ret = sample_comm_vo_start(vo_dev, vo_layer, vo_chn, vo_chn_num, XMEDIA_TRUE, &vo_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vo start failed !\n");
            goto exit4;
        }

        ret = sample_comm_sys_vpss_bind_vo(vpss_pipe, vpss_ochn[0], vo_layer, vo_chn[0]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind vo failed !\n");
            goto exit5;
        }
    }

    /* ------------------------------------------------------------ */
    /* ---------- dynamic switch between normal and readback -------*/
    /* ------------------------------------------------------------ */
    PAUSE(g_force_exit);
    sample_comm_venc_stop_get_stream();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start get stream failed !\n");
        goto exit7;
    }
    ret = sample_comm_venc_start_get_stream(venc_chn, venc_chn_cnt, XMEDIA_TRUE, XMEDIA_FALSE, ".");

    xmedia_awb_info wb_info;
    xmedia_sensor_init_param init_param = { 0 };
    xmedia_awb_query_info(vi_pipe, &wb_info);

    init_param.wb_bgain = wb_info.bgain;
    init_param.wb_ggain = wb_info.grgain;
    init_param.wb_rgain = wb_info.rgain;
    xmedia_sensor_set_init_param(vi_pipe, &init_param);
    sample_vio_switch_to_readback(&vi_config, &video_param);

    PAUSE(g_force_exit);

    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_venc_stop_get_stream();
    }

exit7:
    if (g_venc_use == XMEDIA_TRUE) {
        app_liveserver_stop();
        app_liveserver_deinit();
        sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
    }

exit6:
    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
    } else {
        sample_comm_sys_vpss_unbind_vo(vpss_pipe, vpss_ochn[0], vo_layer, vo_chn[0]);
    }

exit5:
    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_venc_stop(&venc_config);
    } else {
        sample_comm_vo_stop(vo_dev, vo_layer, vo_chn, vo_chn_num);
    }

exit4:
    sample_comm_sys_vi_unbind_vpss(vi_pipe, vi_chn, vpss_pipe, vpss_ichn);

exit3:
    sample_comm_vpss_stop(&vpss_config);

exit2:
    sample_comm_isp_stop(&isp_param);

exit1:
    sample_comm_vi_stop(&vi_config);

exit0:
    sample_comm_isp_exit(&isp_param);
    sample_vio_sys_exit();

    return ret;
}

xmedia_s32 sample_vio_vi_offline_vpss_online_double()
{
    xmedia_s32 i = 0;
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 blk_size = 0;
    xmedia_video_size pic_size[2] = { 0 };
    xmedia_video_size sub_pic_size = { 0 };
    sample_comm_video_param video_param = { 0 };
    sample_sys_config sys_config = { 0 };
    xmedia_s32 total_input_capability = 0;
    xmedia_s32 pool_count = 0;

    xmedia_s32 vi_dev[2] = { 0, 1 };
    xmedia_s32 vi_pipe[2] = { 0, 1 };
    xmedia_s32 vi_chn = 0;
    sample_vi_config vi_config = { 0 };

    xmedia_s32 sensor_type[2] = {sample_vio_sensor_type[0], sample_vio_sensor_type[1]};
    xmedia_u32 framerate[2] = { 0 };
    vi_sensor_info sensor_info[2] = { 0 };
    sample_isp_param isp_param = { 0 };
    xmedia_isp_config isp_config = { 0 };
    xmedia_bool mirror = XMEDIA_FALSE;
    xmedia_bool flip = XMEDIA_FALSE;

    xmedia_s32 vpss_pipe[2] = { 0, 1 };
    xmedia_s32 vpss_chnl = 0;
    xmedia_s32 vpss_ochn[2] = { 0, 1 };
    sample_vpss_config vpss_config = { 0 };
    xmedia_video_size vpss_ochn_size[2][VPSS_MAX_OCHN_NUM] = { 0 };
    xmedia_u32 wrap_buffer_line[2] = { 0 };
    xmedia_vpss_wrap_buf_share_config wrap_share_cfg = { 0 };

    xmedia_s32 venc_chnl = 0;
    xmedia_s32 venc_chn[4] = { 0, 1, 2, 3 };
    xmedia_s32 venc_chn_cnt = 4;
    sample_venc_config venc_config = { 0 };

    xmedia_video_size rtsp_chn_size[4] = { 0 };

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    // sys init
    for(i = 0; i < VI_MAX_PIPE_NUM; i ++)
    {
        sys_config.sys_conf.pipe_mode[i].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
        sys_config.sys_conf.pipe_mode[i].viproc_vpss_mode = XMEDIA_WORK_MODE_ONLINE;
        sys_config.sys_conf.pipe_mode[i].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    }

    sys_config.compress_level = XMEDIA_SYS_COMPRESS_LEVEL_0;

    ret = sample_comm_sys_cmp_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("common sys failed !\n");
        return ret;
    }

    sys_config.vb_conf.max_pool_cnt = 20;

    for(i = 0; i < 2; i ++)
    {
        sample_comm_vi_get_sensor_info(sensor_type[i], &sensor_info[i]);
        sample_comm_vi_get_framerate_by_sensor(sensor_type[i], &framerate[i]);

        pic_size[i].width = sensor_info[i].width;
        pic_size[i].height = sensor_info[i].height;

        /*vi-viproc offline raw VB*/
        blk_size = sample_comm_sys_get_buffer_size(pic_size[i], video_param.video_fmt, sensor_info[i].pixel_format,
                                                sensor_info[i].bit_width, video_param.compress_mode);
        sys_config.vb_conf.common_pool[pool_count].block_size = blk_size;
        sys_config.vb_conf.common_pool[pool_count].block_cnt = ((i == 0) ? 1 : 2);

        pool_count ++;
    }

    /* wrap VB */
    wrap_share_cfg.enable = XMEDIA_TRUE;
    wrap_share_cfg.wrap_pos = XMEDIA_VPSS_WRAP_POS_PHY_OCHN;

    for(i = 0; i < 2; i ++)
    {
        blk_size = sample_comm_sys_get_wrap_buffer_size(pic_size[i], video_param.video_fmt, video_param.pixel_fmt,
                                            video_param.data_width, video_param.compress_mode, &wrap_buffer_line[i]);

        sys_config.vb_conf.common_pool[pool_count].block_size = wrap_share_cfg.buf_size;
        sys_config.vb_conf.common_pool[pool_count].block_cnt = 1;

        if(wrap_share_cfg.enable == XMEDIA_TRUE)
        {
            wrap_share_cfg.buf_size = (wrap_share_cfg.buf_size > blk_size) ? wrap_share_cfg.buf_size : blk_size;
            if(i == 1)
            {
                pool_count ++;
                SAMPLE_PRT("wrap share vb size : %d \n", wrap_share_cfg.buf_size);
            }
        }
        else
        {
            pool_count ++;
            SAMPLE_PRT("sensor%d wrap vb size : %d \n", i, blk_size);
        }
    }

    sub_pic_size.width = 640;
    sub_pic_size.height = 480;
    blk_size = sample_comm_sys_get_buffer_size(sub_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                            video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[pool_count].block_size = blk_size;
    sys_config.vb_conf.common_pool[pool_count].block_cnt = 6;

    total_input_capability = (sensor_info[0].width * sensor_info[0].height * framerate[0]) + (sensor_info[1].width * sensor_info[1].height * framerate[1]);
    if(total_input_capability > 3840 * 2160 * 20) {
        framerate[0] = 20;
        framerate[1] = 20;
    }

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_vio_sys_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sys init failed !\n");
        return ret;
    }

    for(i = 0; i < 2; i ++)
    {
        vi_config.dev_info[vi_dev[i]].dev_en = XMEDIA_TRUE;
        vi_config.dev_info[vi_dev[i]].dev_no = vi_dev[i];
        vi_config.dev_info[vi_dev[i]].sensor_type = sensor_type[i];
        vi_config.pipe_info[vi_pipe[i]].pipe_en = XMEDIA_TRUE;
        vi_config.pipe_info[vi_pipe[i]].pipe_no = vi_pipe[i];
        vi_config.pipe_info[vi_pipe[i]].chn_info[vi_chn].chn_en = XMEDIA_TRUE;
        vi_config.pipe_info[vi_pipe[i]].chn_info[vi_chn].chn_no = vi_chn;
        vi_config.pipe_info[vi_pipe[i]].lowdelay_attr.enable = ((i == 0) ? XMEDIA_TRUE : XMEDIA_FALSE);
        vi_config.pipe_info[vi_pipe[i]].lowdelay_attr.line_cnt = sensor_info[i].height / 2;
        vi_config.pipe_info[vi_pipe[i]].lowdelay_attr.one_buf_en = ((i == 0) ? XMEDIA_TRUE : XMEDIA_FALSE);
        vi_config.dev_bind_pipe[vi_dev[i]].pipe[0] = vi_pipe[i];
        vi_config.dev_bind_pipe[vi_dev[i]].pipe[1] = -1;

        isp_config.fps = framerate[i];
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
        isp_param.isp_info[vi_pipe[i]].mirror = mirror;
        isp_param.isp_info[vi_pipe[i]].flip = flip;
        memcpy(&(isp_param.isp_info[vi_pipe[i]].isp_config), &isp_config, sizeof(xmedia_isp_config));
    }

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
    for(i = 0; i < 2; i ++)
    {
        ret = sample_comm_vpss_get_default_pipe_cfg(&vpss_config.pipe_info[vpss_pipe[i]].pipe_config, pic_size[i], &video_param);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get default pipe cfg failed !\n");
            goto exit2;
        }

        vpss_ochn_size[vpss_pipe[i]][0].width = sensor_info[i].width;
        vpss_ochn_size[vpss_pipe[i]][0].height = sensor_info[i].height;
        vpss_ochn_size[vpss_pipe[i]][1].width = sub_pic_size.width;
        vpss_ochn_size[vpss_pipe[i]][1].height = sub_pic_size.height;

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
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config.depth = 0;

        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get default pipe cfg failed !\n");
            goto exit2;
        }

        vpss_chnl = vpss_ochn[1];
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_no = vpss_ochn[1];
        ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config,
                                                    vpss_ochn_size[vpss_pipe[i]][vpss_chnl], &video_param);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get default pipe cfg failed !\n");
            goto exit2;
        }
    }

    // set wrap buf share
    ret = xmedia_vpss_set_wrap_buf_share_config(&wrap_share_cfg);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("set wrap buf share failed !\n");
        goto exit2;
    }

    // vpss start
    ret = sample_comm_vpss_start(&vpss_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vpss start failed !\n");
        goto exit2;
    }

    for(i = 0; i < 2; i ++)
    {
        memcpy(&rtsp_chn_size[2 * i], &vpss_ochn_size[i][0], sizeof(xmedia_video_size));
        memcpy(&rtsp_chn_size[2 * i + 1], &vpss_ochn_size[i][1], sizeof(xmedia_video_size));
    }

    app_liveserver_init();
    app_liveserver_start(rtsp_chn_size, 4, framerate[0]);

    for(i = 0; i < 2; i ++)
    {
        venc_chnl = venc_chn[2 * i];
        venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
        venc_config.chn_info[venc_chnl].payload_type = PT_H265;
        venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
        sample_comm_venc_get_default_chn_info(vpss_ochn_size[i][0], framerate[i], &venc_config.chn_info[venc_chnl]);

        venc_chnl = venc_chn[2 * i + 1];
        venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
        venc_config.chn_info[venc_chnl].payload_type = PT_H265;
        venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
        sample_comm_venc_get_default_chn_info(vpss_ochn_size[i][1], framerate[i], &venc_config.chn_info[venc_chnl]);
    }

    ret = sample_comm_venc_start(&venc_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start failed !\n");
        goto exit3;
    }

    for(i = 0; i < 2; i ++)
    {
        xmedia_venc_chn_param chn_param;
        xmedia_venc_get_chn_param(venc_chn[2 * i], &chn_param);
        chn_param.priority = 1;
        xmedia_venc_set_chn_param(venc_chn[2 * i], &chn_param);

        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe[i], vpss_ochn[0], venc_chn[2 * i]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind venc failed !\n");
            goto exit4;
        }

        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe[i], vpss_ochn[1], venc_chn[2 * i + 1]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind venc failed !\n");
            goto exit5;
        }
    }

    ret = sample_comm_venc_start_get_stream(venc_chn, venc_chn_cnt, XMEDIA_FALSE, XMEDIA_FALSE, ".");
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start get stream failed !\n");
        goto exit6;
    }

    PAUSE(g_force_exit);

    sample_comm_venc_stop_get_stream();

exit6:
    app_liveserver_stop();
    app_liveserver_deinit();
    sample_comm_sys_vpss_unbind_venc(vpss_pipe[0], vpss_ochn[1], venc_chn[1]);
    sample_comm_sys_vpss_unbind_venc(vpss_pipe[1], vpss_ochn[1], venc_chn[3]);

exit5:
    sample_comm_sys_vpss_unbind_venc(vpss_pipe[0], vpss_ochn[0], venc_chn[0]);
    sample_comm_sys_vpss_unbind_venc(vpss_pipe[1], vpss_ochn[0], venc_chn[2]);

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
    sample_vio_sys_exit();

    return ret;
}

xmedia_s32 sample_vio_vi_offline_vpss_online_double_splicing()
{
    xmedia_s32 i = 0;
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 blk_size = 0;
    xmedia_video_size pic_size[2] = { 0 };
    xmedia_video_size splicing_pic_size[2] = { 0 };
    xmedia_video_size sub_pic_size = { 0 };
    sample_comm_video_param video_param = { 0 };
    sample_sys_config sys_config = { 0 };
    xmedia_s32 pool_count = 0;

    xmedia_s32 vi_dev[2] = { 0, 1 };
    xmedia_s32 vi_pipe[2] = { 0, 1 };
    xmedia_s32 vi_chn = 0;
    sample_vi_config vi_config = { 0 };

    xmedia_s32 sensor_type[2] = {sample_vio_sensor_type[0], sample_vio_sensor_type[1]};
    xmedia_u32 framerate[2] = { 0 };
    vi_sensor_info sensor_info[2] = { 0 };
    sample_isp_param isp_param = { 0 };
    xmedia_isp_config isp_config = { 0 };
    xmedia_bool mirror = XMEDIA_FALSE;
    xmedia_bool flip = XMEDIA_TRUE;

    xmedia_s32 vpss_pipe[2] = { 0, 1 };
    xmedia_s32 vpss_chnl = 0;
    xmedia_s32 vpss_ochn[2] = { 0, 1 };
    sample_vpss_config vpss_config = { 0 };
    xmedia_video_size vpss_ochn_size[2][VPSS_MAX_OCHN_NUM] = { 0 };
    xmedia_vpss_output_mode vpss_output_mode[2] = { XMEDIA_VPSS_OUTPUT_MODE_V_SPLICING, XMEDIA_VPSS_OUTPUT_MODE_INDEPENDENT};
    xmedia_vpss_splicing_grp_config splicing_grp_cfg = { 0 };

    xmedia_s32 venc_chnl = 0;
    xmedia_s32 venc_chn[4] = { 0, 1, 2, 3 };
    xmedia_s32 venc_chn_cnt = 2;
    sample_venc_config venc_config = { 0 };

    xmedia_video_size rtsp_chn_size[4] = { 0 };

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    // sys init
    for(i = 0; i < VI_MAX_PIPE_NUM; i ++)
    {
        sys_config.sys_conf.pipe_mode[i].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
        sys_config.sys_conf.pipe_mode[i].viproc_vpss_mode = XMEDIA_WORK_MODE_ONLINE;
        sys_config.sys_conf.pipe_mode[i].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    }

    sys_config.compress_level = XMEDIA_SYS_COMPRESS_LEVEL_0;

    ret = sample_comm_sys_cmp_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("common sys failed !\n");
        return ret;
    }

    sys_config.vb_conf.max_pool_cnt = 20;

    for(i = 0; i < 2; i ++)
    {
        sample_comm_vi_get_sensor_info(sensor_type[i], &sensor_info[i]);
        sample_comm_vi_get_framerate_by_sensor(sensor_type[i], &framerate[i]);

        pic_size[i].width = sensor_info[i].width;
        pic_size[i].height = sensor_info[i].height;

        /*vi-viproc offline raw VB*/
        blk_size = sample_comm_sys_get_buffer_size(pic_size[i], video_param.video_fmt, sensor_info[i].pixel_format,
                                                sensor_info[i].bit_width, video_param.compress_mode);
        sys_config.vb_conf.common_pool[pool_count].block_size = blk_size;
        sys_config.vb_conf.common_pool[pool_count].block_cnt = ((i == 0) ? 1 : 2);

        pool_count ++;
    }

    if (vpss_output_mode[0] == XMEDIA_VPSS_OUTPUT_MODE_H_SPLICING && pic_size[0].height == pic_size[1].height) {
        splicing_pic_size[0].width = pic_size[0].width + pic_size[1].width;
        splicing_pic_size[0].height = pic_size[0].height;
    } else if (vpss_output_mode[0] == XMEDIA_VPSS_OUTPUT_MODE_V_SPLICING && pic_size[0].width == pic_size[1].width) {
        splicing_pic_size[0].width = pic_size[0].width;
        splicing_pic_size[0].height = pic_size[0].height + pic_size[1].height;
    } else {
        SAMPLE_ERR("unsupport H/V splicing, please check the sensor info\n");
        return XMEDIA_FAILURE;
    }

    blk_size = sample_comm_sys_get_buffer_size(splicing_pic_size[0], video_param.video_fmt, video_param.pixel_fmt,
                                            video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[pool_count].block_size = blk_size;
    sys_config.vb_conf.common_pool[pool_count].block_cnt = 2;
    pool_count ++;
    SAMPLE_PRT("splicing vb size : %u \n", blk_size);

    sub_pic_size.width = 640;
    sub_pic_size.height = 480;
    splicing_pic_size[1].width = (vpss_output_mode[1] == XMEDIA_VPSS_OUTPUT_MODE_H_SPLICING) ? (sub_pic_size.width * 2) : sub_pic_size.width;
    splicing_pic_size[1].height = (vpss_output_mode[1] == XMEDIA_VPSS_OUTPUT_MODE_V_SPLICING) ? (sub_pic_size.height * 2) : sub_pic_size.height;
    blk_size = sample_comm_sys_get_buffer_size(splicing_pic_size[1], video_param.video_fmt, video_param.pixel_fmt,
                                            video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[pool_count].block_size = blk_size;
    sys_config.vb_conf.common_pool[pool_count].block_cnt = (vpss_output_mode[1] == XMEDIA_VPSS_OUTPUT_MODE_INDEPENDENT) ? 4 : 2;

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_vio_sys_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sys init failed !\n");
        return ret;
    }

    for(i = 0; i < 2; i ++)
    {
        vi_config.dev_info[vi_dev[i]].dev_en = XMEDIA_TRUE;
        vi_config.dev_info[vi_dev[i]].dev_no = vi_dev[i];
        vi_config.dev_info[vi_dev[i]].sensor_type = sensor_type[i];
        vi_config.pipe_info[vi_pipe[i]].pipe_en = XMEDIA_TRUE;
        vi_config.pipe_info[vi_pipe[i]].pipe_no = vi_pipe[i];
        vi_config.pipe_info[vi_pipe[i]].chn_info[vi_chn].chn_en = XMEDIA_TRUE;
        vi_config.pipe_info[vi_pipe[i]].chn_info[vi_chn].chn_no = vi_chn;
        vi_config.pipe_info[vi_pipe[i]].lowdelay_attr.enable = ((i == 0) ? XMEDIA_TRUE : XMEDIA_FALSE);
        vi_config.pipe_info[vi_pipe[i]].lowdelay_attr.line_cnt = sensor_info[i].height / 2;
        vi_config.pipe_info[vi_pipe[i]].lowdelay_attr.one_buf_en = ((i == 0) ? XMEDIA_TRUE : XMEDIA_FALSE);
        vi_config.dev_bind_pipe[vi_dev[i]].pipe[0] = vi_pipe[i];
        vi_config.dev_bind_pipe[vi_dev[i]].pipe[1] = -1;

        framerate[i] = 15;
        isp_config.fps = framerate[i];
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
        isp_param.isp_info[vi_pipe[i]].mirror = mirror;
        isp_param.isp_info[vi_pipe[i]].flip = flip;
        isp_param.isp_info[vi_pipe[i]].disable_dynamic_fps = XMEDIA_TRUE;
        memcpy(&(isp_param.isp_info[vi_pipe[i]].isp_config), &isp_config, sizeof(xmedia_isp_config));
    }

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

    // set vpss splicing config
    splicing_grp_cfg.pipe_num = 2;
    splicing_grp_cfg.pipe_id[0] = 0;
    splicing_grp_cfg.pipe_id[1] = 1;
    splicing_grp_cfg.master_pipe_id = 0;
    splicing_grp_cfg.output_mode[0] = vpss_output_mode[0];
    splicing_grp_cfg.output_mode[1] = vpss_output_mode[1];
    splicing_grp_cfg.output_mode[2] = XMEDIA_VPSS_OUTPUT_MODE_INDEPENDENT;

    ret = xmedia_vpss_set_splicing_grp_config(0, &splicing_grp_cfg);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("set vpss splicing grp cfg failed !\n");
        goto exit2;
    }

    // vpss config
    for(i = 0; i < 2; i ++)
    {
        ret = sample_comm_vpss_get_default_pipe_cfg(&vpss_config.pipe_info[vpss_pipe[i]].pipe_config, pic_size[i], &video_param);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get default pipe cfg failed !\n");
            goto exit2;
        }

        vpss_ochn_size[vpss_pipe[i]][0].width = sensor_info[i].width;
        vpss_ochn_size[vpss_pipe[i]][0].height = sensor_info[i].height;
        vpss_ochn_size[vpss_pipe[i]][1].width = sub_pic_size.width;
        vpss_ochn_size[vpss_pipe[i]][1].height = sub_pic_size.height;

        vpss_config.pipe_info[vpss_pipe[i]].pipe_en = XMEDIA_TRUE;
        vpss_config.pipe_info[vpss_pipe[i]].pipe_no = vpss_pipe[i];

        vpss_chnl = vpss_ochn[0];
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_no = vpss_ochn[0];
        ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config,
                                                    vpss_ochn_size[vpss_pipe[i]][vpss_chnl], &video_param);
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config.depth = 0;

        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get default pipe cfg failed !\n");
            goto exit2;
        }

        vpss_chnl = vpss_ochn[1];
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_no = vpss_ochn[1];
        ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config,
                                                    vpss_ochn_size[vpss_pipe[i]][vpss_chnl], &video_param);
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config.depth = 0;
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get default pipe cfg failed !\n");
            goto exit2;
        }
    }

    // vpss start
    ret = sample_comm_vpss_start(&vpss_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vpss start failed !\n");
        goto exit2;
    }

    memcpy(&rtsp_chn_size[0], &splicing_pic_size[0], sizeof(xmedia_video_size));

    if(vpss_output_mode[1] == XMEDIA_VPSS_OUTPUT_MODE_INDEPENDENT) {
        memcpy(&rtsp_chn_size[1], &sub_pic_size, sizeof(xmedia_video_size));
        memcpy(&rtsp_chn_size[2], &sub_pic_size, sizeof(xmedia_video_size));
        venc_chn_cnt = 3;
    } else {
        memcpy(&rtsp_chn_size[1], &splicing_pic_size[1], sizeof(xmedia_video_size));
        venc_chn_cnt = 2;
    }

    app_liveserver_init();
    app_liveserver_start(rtsp_chn_size, venc_chn_cnt, framerate[0]);

    venc_chnl = venc_chn[0];
    venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
    venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
    venc_config.chn_info[venc_chnl].payload_type = PT_H265;
    venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
    sample_comm_venc_get_default_chn_info(splicing_pic_size[0], framerate[0], &venc_config.chn_info[venc_chnl]);

    if(vpss_output_mode[1] == XMEDIA_VPSS_OUTPUT_MODE_INDEPENDENT)
    {
        venc_chnl = venc_chn[1];
        venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
        venc_config.chn_info[venc_chnl].payload_type = PT_H265;
        venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
        sample_comm_venc_get_default_chn_info(sub_pic_size, framerate[0], &venc_config.chn_info[venc_chnl]);

        venc_chnl = venc_chn[2];
        venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
        venc_config.chn_info[venc_chnl].payload_type = PT_H265;
        venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
        sample_comm_venc_get_default_chn_info(sub_pic_size, framerate[1], &venc_config.chn_info[venc_chnl]);
    }
    else
    {
        venc_chnl = venc_chn[1];
        venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
        venc_config.chn_info[venc_chnl].payload_type = PT_H265;
        venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
        sample_comm_venc_get_default_chn_info(splicing_pic_size[1], framerate[0], &venc_config.chn_info[venc_chnl]);
    }


    ret = sample_comm_venc_start(&venc_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start failed !\n");
        goto exit3;
    }

    ret = sample_comm_sys_vpss_bind_venc(vpss_pipe[0], vpss_ochn[0], venc_chn[0]);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vpss bind venc failed !\n");
        goto exit4;
    }

    if(vpss_output_mode[1] == XMEDIA_VPSS_OUTPUT_MODE_INDEPENDENT)
    {
        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe[0], vpss_ochn[1], venc_chn[1]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind venc failed !\n");
            goto exit5;
        }

        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe[1], vpss_ochn[1], venc_chn[2]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind venc failed !\n");
            goto exit5;
        }
    }
    else
    {
        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe[0], vpss_ochn[1], venc_chn[1]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind venc failed !\n");
            goto exit5;
        }
    }

    ret = sample_comm_venc_start_get_stream(venc_chn, venc_chn_cnt, XMEDIA_FALSE, XMEDIA_FALSE, ".");
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start get stream failed !\n");
        goto exit6;
    }

    PAUSE(g_force_exit);

    sample_comm_venc_stop_get_stream();

exit6:
    app_liveserver_stop();
    app_liveserver_deinit();
    if(vpss_output_mode[1] == XMEDIA_VPSS_OUTPUT_MODE_INDEPENDENT) {
        sample_comm_sys_vpss_unbind_venc(vpss_pipe[0], vpss_ochn[1], venc_chn[1]);
        sample_comm_sys_vpss_unbind_venc(vpss_pipe[1], vpss_ochn[1], venc_chn[2]);
    } else {
        sample_comm_sys_vpss_unbind_venc(vpss_pipe[0], vpss_ochn[1], venc_chn[1]);
    }

exit5:
    sample_comm_sys_vpss_unbind_venc(vpss_pipe[0], vpss_ochn[0], venc_chn[0]);

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
    sample_vio_sys_exit();

    return ret;
}

xmedia_s32 sample_vio_vi_online_vpss_online_double()
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 i;
    xmedia_s32 blk_size = 0;
    xmedia_video_size pic_size = { 0 };
    xmedia_video_size sub_pic_size = { 0 };
    sample_comm_video_param video_param = { 0 };
    sample_sys_config sys_config = { 0 };
    xmedia_s32 pool_count = 0;

    xmedia_s32 vi_dev[2] = {0, 1};
    xmedia_s32 vi_pipe[2] = {0, 1};
    xmedia_s32 vi_chn = 0;
    sample_vi_config vi_config = { 0 };

    xmedia_sys_switch_config switch_config = { 0 };

    xmedia_s32 sensor_type = sample_vio_sensor_type[0];
    xmedia_u32 framerate = 0;
    vi_sensor_info sensor_info = { 0 };
    sample_isp_param isp_param = { 0 };
    xmedia_isp_config isp_config = { 0 };
    xmedia_bool mirror = XMEDIA_FALSE;
    xmedia_bool flip = XMEDIA_FALSE;

    xmedia_s32 vpss_pipe[2] = {0, 1};
    xmedia_s32 vpss_chnl = 0;
    xmedia_s32 vpss_ochn[2] = { 0, 1 };
    sample_vpss_config vpss_config = { 0 };
    xmedia_video_size vpss_ochn_size[2][VPSS_MAX_OCHN_NUM] = { 0 };
    xmedia_u32 wrap_buffer_line = 0;

    xmedia_s32 venc_chnl = 0;
    xmedia_s32 venc_chn[4] = { 0, 1, 2, 3 };
    xmedia_s32 venc_chn_cnt = 4;
    sample_venc_config venc_config = { 0 };
    xmedia_video_size rtsp_chn_size[4] = { 0 };

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);
    framerate = 15; // 双路15fps

    for (i = 0; i < 2; i++) {
        // sys init
        sys_config.sys_conf.pipe_mode[i].vicap_viproc_mode = XMEDIA_WORK_MODE_ONLINE;
        sys_config.sys_conf.pipe_mode[i].viproc_vpss_mode = XMEDIA_WORK_MODE_ONLINE;
        sys_config.sys_conf.pipe_mode[i].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    }

    sys_config.compress_level = XMEDIA_SYS_COMPRESS_LEVEL_0;

    ret = sample_comm_sys_cmp_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("common sys failed !\n");
        return ret;
    }

    sys_config.vb_conf.max_pool_cnt = 20;

    for (i = 0; i < 2; i++) {
        pic_size.width = sensor_info.width;
        pic_size.height = sensor_info.height;

        /* wrap VB */
        blk_size = sample_comm_sys_get_wrap_buffer_size(pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                                        video_param.data_width, video_param.compress_mode, &wrap_buffer_line);
        sys_config.vb_conf.common_pool[pool_count].block_size = blk_size;
        sys_config.vb_conf.common_pool[pool_count].block_cnt = 1;
        SAMPLE_PRT("wrap vb size : %d \n", blk_size);
        pool_count++;

        sub_pic_size.width = 640;
        sub_pic_size.height = 480;
        blk_size = sample_comm_sys_get_buffer_size(sub_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                                   video_param.data_width, video_param.compress_mode);
        sys_config.vb_conf.common_pool[pool_count].block_size = blk_size;
        sys_config.vb_conf.common_pool[pool_count].block_cnt = 3;
        pool_count++;
    }

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_vio_sys_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sys init failed !\n");
        return ret;
    }

    // switch init
    switch_config.ext_switch_en = XMEDIA_FALSE;
    switch_config.master_pipe_id = 0;
    switch_config.multi_pipe_mode = XMEDIA_SYS_MULTI_PIPE_MODE_DUAL_SWITCH;
    switch_config.pipe_id[0] = 0;
    switch_config.pipe_id[1] = 1;
    switch_config.pipe_num = 2;
    switch_config.pipe_sel[0] = 0;
    switch_config.pipe_sel[1] = 1;
    switch_config.sync_mode = XMEDIA_SYS_SWITCH_SYNC_MODE_SOC_MASTER_SLAVE;
    ret = xmedia_sys_set_switch_config(0, &switch_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_sys_set_switch_config failed!\n");
        sample_vio_sys_exit();
        return ret;
    }

    for (i = 0; i < 2; i++) {
        vi_config.dev_info[vi_dev[i]].dev_en = XMEDIA_TRUE;
        vi_config.dev_info[vi_dev[i]].dev_no = vi_dev[i];
        vi_config.dev_info[vi_dev[i]].sensor_type = sensor_type;
        vi_config.pipe_info[vi_pipe[i]].pipe_en = XMEDIA_TRUE;
        vi_config.pipe_info[vi_pipe[i]].pipe_no = vi_pipe[i];
        vi_config.pipe_info[vi_pipe[i]].chn_info[vi_chn].chn_en = XMEDIA_TRUE;
        vi_config.pipe_info[vi_pipe[i]].chn_info[vi_chn].chn_no = vi_chn;
        vi_config.dev_bind_pipe[vi_dev[i]].pipe[0] = vi_pipe[i];
        vi_config.dev_bind_pipe[vi_dev[i]].pipe[1] = -1;
    }

    isp_config.fps = framerate;
    isp_config.mode_config.work_mode = XMEDIA_ISP_WORK_MODE_MASTER;
    isp_config.mode_config.master_mode.blend_stat_enable = XMEDIA_FALSE;
    isp_config.mode_config.master_mode.slave_num = 0;
    isp_config.pixel_fmt = sensor_info.pixel_format;
    isp_config.size.height = sensor_info.height;
    isp_config.size.width = sensor_info.width;
    isp_config.wdr_mode = sensor_info.wdr_mode;

    // isp pipe init
    for (i = 0; i < 2; i++) {
        isp_param.pipe[vi_pipe[i]] = vi_pipe[i];
        isp_param.isp_info[vi_pipe[i]].isp_pipe_en = XMEDIA_TRUE;
        isp_param.isp_info[vi_pipe[i]].isp_sensor_en = XMEDIA_TRUE;
        isp_param.isp_info[vi_pipe[i]].sensor_type = sensor_type;
        isp_param.isp_info[vi_pipe[i]].mirror = mirror;
        isp_param.isp_info[vi_pipe[i]].flip = flip;
        memcpy(&(isp_param.isp_info[vi_pipe[i]].isp_config), &isp_config, sizeof(xmedia_isp_config));
    }

    // switch 主从:sensor0默认，sensor1从
    isp_param.isp_info[0].sns_work_mode = XMEDIA_SENSOR_WORK_MODE_NORMAL;
    isp_param.isp_info[1].sns_work_mode = XMEDIA_SENSOR_WORK_MODE_SLAVE;
    isp_param.isp_info[0].disable_dynamic_fps = XMEDIA_TRUE;
    isp_param.isp_info[1].disable_dynamic_fps = XMEDIA_TRUE;

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
    for (i = 0; i < 2; i++) {
        ret = sample_comm_vpss_get_default_pipe_cfg(&vpss_config.pipe_info[vpss_pipe[i]].pipe_config, pic_size, &video_param);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get default pipe cfg failed !\n");
            goto exit2;
        }

        vpss_ochn_size[vpss_pipe[i]][0].width = sensor_info.width;
        vpss_ochn_size[vpss_pipe[i]][0].height = sensor_info.height;
        vpss_ochn_size[vpss_pipe[i]][1].width = sub_pic_size.width;
        vpss_ochn_size[vpss_pipe[i]][1].height = sub_pic_size.height;

        vpss_config.pipe_info[vpss_pipe[i]].pipe_en = XMEDIA_TRUE;
        vpss_config.pipe_info[vpss_pipe[i]].pipe_no = vpss_pipe[i];

        vpss_chnl = vpss_ochn[0];
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_no = vpss_ochn[0];
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].wrap_attr.buf_line = wrap_buffer_line;
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].wrap_attr.enable = XMEDIA_TRUE;
        ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config,
                                                    vpss_ochn_size[vpss_pipe[i]][vpss_chnl], &video_param);
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config.cmp_mode = XMEDIA_VIDEO_COMPRESS_MODE_TILE_COMPACT;
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config.video_fmt = XMEDIA_VIDEO_FMT_TILE_32x4;
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config.depth = 0;

        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get default pipe cfg failed !\n");
            goto exit2;
        }

        vpss_chnl = vpss_ochn[1];
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
        vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_no = vpss_ochn[1];
        ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe[i]].chn_info[vpss_chnl].chn_config,
                                                    vpss_ochn_size[vpss_pipe[i]][vpss_chnl], &video_param);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get default pipe cfg failed !\n");
            goto exit2;
        }
    }

    // vpss start
    ret = sample_comm_vpss_start(&vpss_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vpss start failed !\n");
        goto exit2;
    }

    for(i = 0; i < 2; i ++)
    {
        memcpy(&rtsp_chn_size[2 * i], &vpss_ochn_size[i][0], sizeof(xmedia_video_size));
        memcpy(&rtsp_chn_size[2 * i + 1], &vpss_ochn_size[i][1], sizeof(xmedia_video_size));
    }

    app_liveserver_init();
    app_liveserver_start(rtsp_chn_size, 4, framerate);

    for (i = 0; i < 2; i++) {
        venc_chnl = venc_chn[2 * i];
        venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
        venc_config.chn_info[venc_chnl].payload_type = PT_H265;
        venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
        sample_comm_venc_get_default_chn_info(vpss_ochn_size[i][0], framerate, &venc_config.chn_info[venc_chnl]);

        venc_chnl = venc_chn[2 * i + 1];
        venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
        venc_config.chn_info[venc_chnl].payload_type = PT_H265;
        venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
        sample_comm_venc_get_default_chn_info(vpss_ochn_size[i][1], framerate, &venc_config.chn_info[venc_chnl]);
    }

    ret = sample_comm_venc_start(&venc_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start failed !\n");
        goto exit3;
    }

    for (i = 0; i < 2; i++) {
        xmedia_venc_chn_param chn_param;
        xmedia_venc_get_chn_param(venc_chn[2 * i], &chn_param);
        chn_param.priority = 1;
        xmedia_venc_set_chn_param(venc_chn[2 * i], &chn_param);

        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe[i], vpss_ochn[0], venc_chn[2 * i]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind venc failed !\n");
            goto exit4;
        }

        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe[i], vpss_ochn[1], venc_chn[2 * i + 1]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind venc failed !\n");
            goto exit5;
        }
    }

    ret = sample_comm_venc_start_get_stream(venc_chn, venc_chn_cnt, XMEDIA_FALSE, XMEDIA_FALSE, ".");
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start get stream failed !\n");
        goto exit6;
    }

    PAUSE(g_force_exit);
    sample_comm_venc_stop_get_stream();
exit6:
    app_liveserver_stop();
    app_liveserver_deinit();

    sample_comm_sys_vpss_unbind_venc(vpss_pipe[0], vpss_ochn[1], venc_chn[1]);
    sample_comm_sys_vpss_unbind_venc(vpss_pipe[1], vpss_ochn[1], venc_chn[3]);
exit5:
    sample_comm_sys_vpss_unbind_venc(vpss_pipe[0], vpss_ochn[0], venc_chn[0]);
    sample_comm_sys_vpss_unbind_venc(vpss_pipe[1], vpss_ochn[0], venc_chn[2]);
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
    sample_vio_sys_exit();

    return ret;
}

xmedia_s32 sample_vio_vi_offline_vpss_online_ldc()
{
    xmedia_s32 i = 0;
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 blk_size = 0;
    xmedia_video_size pic_size = { 0 };
    xmedia_video_size sub_pic_size = { 0 };
    sample_comm_video_param video_param = { 0 };
    sample_sys_config sys_config = { 0 };

    xmedia_s32 vi_dev = 0;
    xmedia_s32 vi_pipe = 0;
    xmedia_s32 vi_chn = 0;
    sample_vi_config vi_config = { 0 };
    xmedia_video_ldc_attr vi_ldc_attr = { 0 };

    xmedia_s32 sensor_type = sample_vio_sensor_type[0];
    xmedia_u32 framerate = 0;
    vi_sensor_info sensor_info = { 0 };
    sample_isp_param isp_param = { 0 };
    xmedia_isp_config isp_config = { 0 };
    xmedia_bool mirror = XMEDIA_FALSE;
    xmedia_bool flip = XMEDIA_FALSE;

    xmedia_s32 vpss_chnl = 0;
    xmedia_s32 vpss_pipe = 0;
    xmedia_s32 vpss_ochn[2] = { 0, 1 };
    sample_vpss_config vpss_config = { 0 };
    xmedia_video_size vpss_ochn_size[VPSS_MAX_OCHN_NUM] = { 0 };

    xmedia_s32 venc_chnl = 0;
    xmedia_s32 venc_chn[2] = { 0, 1 };
    xmedia_s32 venc_chn_cnt = 2;
    sample_venc_config venc_config = { 0 };

    xmedia_s32 vo_dev = 0;
    xmedia_s32 vo_layer = vo_dev;
    xmedia_s32 vo_chn[1] = { 0 };
    xmedia_s32 vo_chn_num = 1;
    sample_vo_config_input vo_input = { 0 };
    sample_vo_config vo_config = { 0 };
    vo_input.square_sort = 1;
    vo_input.screen_type = sample_vo_screen_type[0];

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);

    // sys init
    for(i = 0; i < VI_MAX_PIPE_NUM; i ++)
    {
        sys_config.sys_conf.pipe_mode[i].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
        sys_config.sys_conf.pipe_mode[i].viproc_vpss_mode = XMEDIA_WORK_MODE_ONLINE;
        sys_config.sys_conf.pipe_mode[i].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    }

    sys_config.compress_level = XMEDIA_SYS_COMPRESS_LEVEL_0;

    ret = sample_comm_sys_cmp_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("common sys failed !\n");
        return ret;
    }

    sys_config.vb_conf.max_pool_cnt = 20;

    pic_size.width = sensor_info.width;
    pic_size.height = sensor_info.height;

    /* vicap-viproc offline VB */
    blk_size = sample_comm_sys_get_buffer_size(pic_size, video_param.video_fmt, sensor_info.pixel_format,
                                               sensor_info.bit_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[0].block_size = blk_size;
    sys_config.vb_conf.common_pool[0].block_cnt = 2;
    SAMPLE_PRT("offline vb size : %d \n", blk_size);

    /* VPSS-VENC VB */
    blk_size = sample_comm_sys_get_buffer_size(pic_size, XMEDIA_VIDEO_FMT_LINEAR, video_param.pixel_fmt,
                                               video_param.data_width, XMEDIA_VIDEO_COMPRESS_MODE_NONE);
    sys_config.vb_conf.common_pool[1].block_size = blk_size;
    sys_config.vb_conf.common_pool[1].block_cnt = 2;
    SAMPLE_PRT("vpss-venc vb size : %d \n", blk_size);

    sub_pic_size.width = 640;
    sub_pic_size.height = 480;
    blk_size = sample_comm_sys_get_buffer_size(sub_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[2].block_size = blk_size;
    sys_config.vb_conf.common_pool[2].block_cnt = 2;

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_vio_sys_init(&sys_config);
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

    vi_ldc_attr.enable = XMEDIA_TRUE;
    vi_ldc_attr.version = XMEDIA_VIDEO_LDC_V1;

    if (vi_ldc_attr.version == XMEDIA_VIDEO_LDC_V1) {
        // LDC_V1需手动调整参数以达到最佳的矫正效果.
        vi_ldc_attr.ldcv1_attr.aspect = 1; // 0；仅开水平LDC 1: 水平和垂直LDC都开.
        vi_ldc_attr.ldcv1_attr.x_ratio = 0;
        vi_ldc_attr.ldcv1_attr.y_ratio = 0;
        vi_ldc_attr.ldcv1_attr.xy_ratio = 0;
        vi_ldc_attr.ldcv1_attr.center_xoffset = 0;
        vi_ldc_attr.ldcv1_attr.center_yoffset = 0;
        vi_ldc_attr.ldcv1_attr.distortion_ratio = 300;
    } else {
        // LDC_V2参数通过标定得到.
        vi_ldc_attr.ldcv2_attr.focal_len_x = 53272;
        vi_ldc_attr.ldcv2_attr.focal_len_y = 53310;
        vi_ldc_attr.ldcv2_attr.coor_shift_x = 40362;
        vi_ldc_attr.ldcv2_attr.coor_shift_y = 24146;
        vi_ldc_attr.ldcv2_attr.src_calibration_ratio[0] = -53082;
        vi_ldc_attr.ldcv2_attr.src_calibration_ratio[1] = 23062;
        vi_ldc_attr.ldcv2_attr.src_calibration_ratio[2] = -5125;
    }

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

    ret = xmedia_vi_set_chn_ldc_attr(vi_pipe, vi_chn, &vi_ldc_attr);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vi set chn ldc attr failed!\n");
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
    ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
                                                vpss_ochn_size[vpss_chnl], &video_param);
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.depth = 0;

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

    if (g_venc_use == XMEDIA_TRUE) {
        // venc init
        app_liveserver_init();
        app_liveserver_start(vpss_ochn_size, venc_chn_cnt, framerate);

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

        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind venc failed !\n");
            goto exit5;
        }

        ret = sample_comm_venc_start_get_stream(venc_chn, venc_chn_cnt, XMEDIA_FALSE, XMEDIA_FALSE, ".");
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("venc start get stream failed !\n");
            goto exit6;
        }
    } else {
        // vo init
        ret = sample_comm_vo_get_default_config(vo_input, &vo_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get vo default config failed !\n");
            goto exit3;
        }

        ret = sample_comm_vo_start(vo_dev, vo_layer, vo_chn, vo_chn_num, XMEDIA_TRUE, &vo_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vo start failed !\n");
            goto exit3;
        }

        ret = sample_comm_sys_vpss_bind_vo(vpss_pipe, vpss_ochn[0], vo_layer, vo_chn[0]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind vo failed !\n");
            goto exit4;
        }
    }

    PAUSE(g_force_exit);

    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_venc_stop_get_stream();
    }

exit6:
    if (g_venc_use == XMEDIA_TRUE) {
        app_liveserver_stop();
        app_liveserver_deinit();
        sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
    }

exit5:
    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
    } else {
        sample_comm_sys_vpss_unbind_vo(vpss_pipe, vpss_ochn[0], vo_layer, vo_chn[0]);
    }

exit4:
    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_venc_stop(&venc_config);
    } else {
        sample_comm_vo_stop(vo_dev, vo_layer, vo_chn, vo_chn_num);
    }

exit3:
    sample_comm_vpss_stop(&vpss_config);

exit2:
    sample_comm_isp_stop(&isp_param);

exit1:
    sample_comm_vi_stop(&vi_config);

exit0:
    sample_comm_isp_exit(&isp_param);
    sample_vio_sys_exit();

    return ret;
}

xmedia_s32 sample_vio_vi_offline_vpss_online_ahd_mipi()
{
    xmedia_s32 i = 0;
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 blk_size = 0;
    xmedia_video_size pic_size = { 0 };
    xmedia_video_size sub_pic_size = { 0 };
    sample_comm_video_param video_param = { 0 };
    sample_sys_config sys_config = { 0 };

    xmedia_s32 vi_dev = 0;
    xmedia_s32 vi_pipe = 0;
    xmedia_s32 vi_chn = 0;
    sample_vi_config vi_config = { 0 };
    sample_comm_tp9963_cfg tp9963_cfg = { 0 };

    xmedia_s32 sensor_type = VI_AHD_MIPI_2M_YUV;
    xmedia_u32 framerate = 0;
    vi_sensor_info sensor_info = { 0 };
    sample_isp_param isp_param = { 0 };
    xmedia_isp_config isp_config = { 0 };
    xmedia_bool mirror = XMEDIA_FALSE;
    xmedia_bool flip = XMEDIA_FALSE;

    xmedia_s32 vpss_chnl = 0;
    xmedia_s32 vpss_pipe = 0;
    xmedia_s32 vpss_ochn[2] = { 0, 1 };
    sample_vpss_config vpss_config = { 0 };
    xmedia_video_size vpss_ochn_size[VPSS_MAX_OCHN_NUM] = { 0 };

    xmedia_s32 venc_chnl = 0;
    xmedia_s32 venc_chn[2] = { 0, 1 };
    xmedia_s32 venc_chn_cnt = 2;
    sample_venc_config venc_config = { 0 };

    xmedia_s32 vo_dev = 0;
    xmedia_s32 vo_layer = vo_dev;
    xmedia_s32 vo_chn[1] = { 0 };
    xmedia_s32 vo_chn_num = 1;
    sample_vo_config_input vo_input = { 0 };
    sample_vo_config vo_config = { 0 };
    vo_input.square_sort = 1;
    vo_input.screen_type = sample_vo_screen_type[0];

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);

    // sys init
    for(i = 0; i < VI_MAX_PIPE_NUM; i ++)
    {
        sys_config.sys_conf.pipe_mode[i].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
        sys_config.sys_conf.pipe_mode[i].viproc_vpss_mode = XMEDIA_WORK_MODE_ONLINE;
        sys_config.sys_conf.pipe_mode[i].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    }

    sys_config.compress_level = XMEDIA_SYS_COMPRESS_LEVEL_0;

    ret = sample_comm_sys_cmp_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("common sys failed !\n");
        return ret;
    }

    sys_config.vb_conf.max_pool_cnt = 20;

    pic_size.width = sensor_info.width;
    pic_size.height = sensor_info.height;

    /* vicap-viproc offline VB */
    blk_size = sample_comm_sys_get_buffer_size(pic_size, video_param.video_fmt, sensor_info.pixel_format,
                                               sensor_info.bit_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[0].block_size = blk_size;
    sys_config.vb_conf.common_pool[0].block_cnt = 2;
    SAMPLE_PRT("offline vb size : %d \n", blk_size);

    /* VPSS-VENC VB */
    blk_size = sample_comm_sys_get_buffer_size(pic_size, XMEDIA_VIDEO_FMT_TILE_32x4, video_param.pixel_fmt,
                                               video_param.data_width, XMEDIA_VIDEO_COMPRESS_MODE_TILE);
    sys_config.vb_conf.common_pool[1].block_size = blk_size;
    sys_config.vb_conf.common_pool[1].block_cnt = 2;
    SAMPLE_PRT("vpss-venc vb size : %d \n", blk_size);

    sub_pic_size.width = 640;
    sub_pic_size.height = 480;
    blk_size = sample_comm_sys_get_buffer_size(sub_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[2].block_size = blk_size;
    sys_config.vb_conf.common_pool[2].block_cnt = 2;

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_vio_sys_init(&sys_config);
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
    vi_config.dev_info[vi_dev].mipi_phy_en = XMEDIA_TRUE;
    vi_config.dev_info[vi_dev].mipi_phy_config.mipi_rate = 594;

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
    isp_param.isp_info[vi_pipe].isp_sensor_en = XMEDIA_FALSE;
    isp_param.isp_info[vi_pipe].sensor_type = sensor_type;
    isp_param.isp_info[vi_pipe].mirror = mirror;
    isp_param.isp_info[vi_pipe].flip = flip;
    memcpy(&(isp_param.isp_info[vi_pipe].isp_config), &isp_config, sizeof(xmedia_isp_config));

    ret = sample_comm_isp_sensor_reset(vi_config.dev_info[vi_dev].sns_clk_id);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("isp sensor reset failed!\n");
        sample_vio_sys_exit();
        return ret;
    }

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

    tp9963_cfg.i2c_dev = vi_config.dev_info[vi_dev].i2c_id;
    tp9963_cfg.chn[0] = TP_CH_1;
    tp9963_cfg.fmt[0] = TP_FMT_FHD25;
    tp9963_cfg.std[0] = TP_STD_TVI;
    tp9963_cfg.chn[1] = TP_CH_MAX;
    tp9963_cfg.mipi_mode = TP_MIPI_MODE_1CH2LANE_297M;
    ret = sample_comm_tp9963_init(&tp9963_cfg);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("tp9963 init failed!\n");
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
    ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
                                                vpss_ochn_size[vpss_chnl], &video_param);
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.cmp_mode = XMEDIA_VIDEO_COMPRESS_MODE_TILE;
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.video_fmt = XMEDIA_VIDEO_FMT_TILE_32x4;
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.depth = 0;

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

    if (g_venc_use == XMEDIA_TRUE) {
        // venc init
        app_liveserver_init();
        app_liveserver_start(vpss_ochn_size, venc_chn_cnt, framerate);

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

        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind venc failed !\n");
            goto exit5;
        }

        ret = sample_comm_venc_start_get_stream(venc_chn, venc_chn_cnt, XMEDIA_FALSE, XMEDIA_FALSE, ".");
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("venc start get stream failed !\n");
            goto exit6;
        }
    } else {
        // vo init
        ret = sample_comm_vo_get_default_config(vo_input, &vo_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get vo default config failed !\n");
            goto exit3;
        }

        ret = sample_comm_vo_start(vo_dev, vo_layer, vo_chn, vo_chn_num, XMEDIA_TRUE, &vo_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vo start failed !\n");
            goto exit3;
        }

        ret = sample_comm_sys_vpss_bind_vo(vpss_pipe, vpss_ochn[0], vo_layer, vo_chn[0]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind vo failed !\n");
            goto exit4;
        }
    }

    PAUSE(g_force_exit);

    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_venc_stop_get_stream();
    }

exit6:
    if (g_venc_use == XMEDIA_TRUE) {
        app_liveserver_stop();
        app_liveserver_deinit();
        sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
    }

exit5:
    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
    } else {
        sample_comm_sys_vpss_unbind_vo(vpss_pipe, vpss_ochn[0], vo_layer, vo_chn[0]);
    }

exit4:
    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_venc_stop(&venc_config);
    } else {
        sample_comm_vo_stop(vo_dev, vo_layer, vo_chn, vo_chn_num);
    }

exit3:
    sample_comm_vpss_stop(&vpss_config);

exit2:
    sample_comm_tp9963_deinit();
    sample_comm_isp_stop(&isp_param);

exit1:
    sample_comm_vi_stop(&vi_config);

exit0:
    sample_comm_isp_exit(&isp_param);
    sample_vio_sys_exit();

    return ret;
}

int main(int argc, char **argv)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 work_mode = 0;
    xmedia_s32 stream_mode = 0;

    signal(SIGINT, sample_vio_handle_sig);
    signal(SIGTERM, sample_vio_handle_sig);

    if (argc == 2) {
        if (!strncmp(argv[1], "-h", 2)) {
            sample_vio_usage(argv[0]);
            return 0;
        }

        g_venc_use = XMEDIA_TRUE;
    } else if (argc == 3) {
        stream_mode = atoi(argv[2]);
        if (stream_mode == 1) {
            g_venc_use = XMEDIA_TRUE;
        } else if (stream_mode == 0) {
            g_venc_use = XMEDIA_FALSE;
        } else {
            sample_vio_usage(argv[0]);
            return 0;
        }
    } else {
        sample_vio_usage(argv[0]);
        return 0;
    }

    work_mode = atoi(argv[1]);

    switch (work_mode) {
        case 0:
            ret = sample_vio_vi_online_vpss_online();
            break;

        case 1:
            ret = sample_vio_vi_offline_vpss_online();
            break;

        case 2:
            ret = sample_vio_vi_offline_vpss_offline();
            break;

        case 3:
            ret = sample_vio_vi_offline_vpss_offline_wdr();
            break;

        case 4:
            ret = sample_vio_vi_offline_vpss_offline_switch_linear_wdr();
            break;

        case 5:
            ret = sample_vio_vi_offline_vpss_offline_switch_resolution();
            break;

        case 6:
            ret = sample_vio_vi_offline_vpss_online_double();
            break;

        case 7:
            ret = sample_vio_vi_offline_vpss_online_double_splicing();
            break;

        case 8:
            ret = sample_vio_vi_online_vpss_online_double();
            break;

        case 9:
            ret = sample_vio_vi_offline_vpss_offline_preroll_normal_readback();
            break;

        case 10:
            ret = sample_vio_vi_offline_vpss_online_ldc();
            break;

        case 11:
            ret = sample_vio_vi_offline_vpss_online_ahd_mipi();
            break;

        default:
            sample_vio_usage(argv[0]);
            return 0;
    }

    if (XMEDIA_SUCCESS == ret) {
        SAMPLE_PRT("program exit normally!\n");
    } else {
        SAMPLE_PRT("program exit abnormally!\n");
    }

    return 0;
}
