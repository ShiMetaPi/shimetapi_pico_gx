/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include "sample_comm.h"
#include "sample_comm_isp.h"
#include "sample_comm_sys.h"
#include "sample_comm_vi.h"
#include "sample_comm_vpss.h"
#include "sample_comm_vo.h"
#include "sample_comm_venc.h"
#include "sample_comm_region.h"
#include "sample_comm_livestream.h"

#define  vi_chn_0_bmp  "./res/vi_chn_0.bmp"
#define  mm_bmp        "./res/mm.bmp"
#define  mm2_bmp       "./res/mm2.bmp"
#define RGN_SAMPLE_WIDTH  (200)
#define RGN_SAMPLE_HEIGHT (200)
#define RGN_CALC_HANDLE_NUM(width, height) \
    ((((width) / RGN_SAMPLE_WIDTH) < ((height) / RGN_SAMPLE_HEIGHT)) ? \
     ((width) / RGN_SAMPLE_WIDTH) : ((height) / RGN_SAMPLE_HEIGHT))


static sample_comm_sensor_type sample_region_sensor_type[MAX_SENSOR_NUM] = {
    SENSOR0_TYPE,SENSOR1_TYPE,SENSOR2_TYPE,SENSOR3_TYPE,SENSOR4_TYPE
};

static sample_comm_screen_type sample_vo_screen_type[MAX_SCREEN_NUM] = { SCREEN0_TYPE, SCREEN0_TYPE};

xmedia_s32 g_vi_dev = 0;
xmedia_s32 g_vi_pipe = 0;
xmedia_s32 g_vi_chn = 0;

xmedia_s32 g_vpss_chn = 0;
xmedia_s32 g_vpss_pipe = 0;
xmedia_s32 g_vpss_ichn = 0;
xmedia_s32 g_vpss_ochn[2] = { 0, 1 };

xmedia_s32 g_vo_dev = 0;
xmedia_s32 g_vo_layer = 0;
xmedia_s32 g_vo_chn[1] = { 0 };
xmedia_s32 g_vo_chn_num = 1;

xmedia_s32 g_venc_chnl = 0;
xmedia_s32 g_venc_chn[2] = { 0, 1 };
xmedia_s32 g_venc_chn_cnt = 2;
static sample_venc_config g_venc_config = { 0 };
static xmedia_bool g_venc_use = XMEDIA_TRUE; // vpss test 默认开启venc 看流

static sample_vi_config g_vi_config = { 0 };
static sample_isp_param g_isp_param = { 0 };
static sample_vpss_config g_vpss_config = { 0 };

static xmedia_char* g_path_bmp;
static xmedia_bool g_vo_display = XMEDIA_FALSE; // VO 打osd, 需要申请vb
static sample_vo_config g_vo_config = { 0 };

/******************************************************************************
* function : show usage
******************************************************************************/
static xmedia_void sample_region_usage(char* args)
{
    printf("usage : %s <index> \n", args);
    printf("index:\n");
    printf("\t 0)VPSS OVERLAYEX.\n");
    printf("\t 1)VPSS COVEREX.\n");
    printf("\t 2)VPSS MOSAICEX.\n");
    printf("\t 3)VPSS CORNER RECTEX.\n");
    printf("\t 4)VO OVERLAYEX.\n");
    printf("\t 5)VO COVEREX.\n");
    printf("\t 6)VO CORNER RECTEX.\n");
    printf("\t 7)VENC OVERLAY.\n");
    printf("\t 8)VENC COVER.\n");
    printf("\t 9)VENC MOSAIC.\n");
    printf("\t 10)VENC CORNER RECT.\n");
    return;
}

xmedia_s32 sample_region_sys_init(sample_sys_config *sys_config)
{
    xmedia_s32 ret = 0;

    ret = sample_comm_sys_init(sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_sys_init failed!\n");
        return ret;
    }

    ret = sample_comm_region_init();
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
        SAMPLE_PRT("sample_comm_vo_init failed!\n");
        return ret;
    }

    return XMEDIA_SUCCESS;
}

xmedia_void sample_region_sys_exit(void)
{
    sample_comm_venc_exit();
    sample_comm_vo_exit();
    sample_comm_vpss_exit();
    sample_comm_vi_exit();
    sample_comm_region_exit();
    sample_comm_sys_exit();
}

xmedia_void sample_region_handle_sig(xmedia_s32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo || SIGTERM == signo) {
        sample_comm_venc_stop_get_stream();
        sample_comm_venc_stop(&g_venc_config);
        sample_comm_isp_stop(&g_isp_param);
        sample_comm_isp_exit(&g_isp_param);
        sample_region_sys_exit();
        SAMPLE_PRT("\033[0;31mprogram exit abnormally!\033[0;39m\n");
    }

    exit(0);
}

xmedia_s32 sample_comm_media_vi_vpss_vo_start(xmedia_void)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 blk_size = 0;
    xmedia_video_size pic_size = { 0 };
    xmedia_video_size sub_pic_size = { 0 };
    sample_comm_video_param video_param = { 0 };
    sample_sys_config sys_config = { 0 };

    xmedia_s32 sensor_type = sample_region_sensor_type[0];
    xmedia_u32 framerate = 0;
    vi_sensor_info sensor_info = { 0 };
    xmedia_isp_config isp_config = { 0 };
    xmedia_bool mirror = XMEDIA_FALSE;
    xmedia_bool flip = XMEDIA_FALSE;
    xmedia_video_size g_vpss_ochn_size[VPSS_MAX_OCHN_NUM] = { 0 };

    sample_vo_config_input vo_input = { 0 };

    vo_input.square_sort = 1;
    vo_input.screen_type = sample_vo_screen_type[0]; // 显示器

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);

    // sys init
    sys_config.sys_conf.pipe_mode[g_vi_pipe].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[g_vi_pipe].viproc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[g_vi_pipe].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;

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
    sys_config.vb_conf.common_pool[1].block_cnt = 5;

    sub_pic_size.width = 640;
    sub_pic_size.height = 480;
    blk_size = sample_comm_sys_get_buffer_size(sub_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[2].block_size = blk_size;
    sys_config.vb_conf.common_pool[2].block_cnt = 3;

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_region_sys_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sys init failed !\n");
        return ret;
    }
    memset(&g_vi_config, 0 , sizeof(sample_vi_config));
    g_vi_config.dev_info[g_vi_dev].dev_en = XMEDIA_TRUE;
    g_vi_config.dev_info[g_vi_dev].dev_no = g_vi_dev;
    g_vi_config.dev_info[g_vi_dev].sensor_type = sensor_type;
    g_vi_config.pipe_info[g_vi_pipe].pipe_en = XMEDIA_TRUE;
    g_vi_config.pipe_info[g_vi_pipe].pipe_no = g_vi_pipe;
    g_vi_config.pipe_info[g_vi_pipe].chn_info[g_vi_chn].chn_en = XMEDIA_TRUE;
    g_vi_config.pipe_info[g_vi_pipe].chn_info[g_vi_chn].chn_no = g_vi_chn;
    g_vi_config.dev_bind_pipe[g_vi_dev].pipe[0] = g_vi_pipe;
    g_vi_config.dev_bind_pipe[g_vi_dev].pipe[1] = -1;

    isp_config.bayer_fmt = XMEDIA_VIDEO_BAYER_FMT_RGGB;
    isp_config.fps = framerate;
    isp_config.mode_config.work_mode = XMEDIA_ISP_WORK_MODE_MASTER;
    isp_config.mode_config.master_mode.blend_stat_enable = XMEDIA_FALSE;
    isp_config.mode_config.master_mode.slave_num = 0;
    isp_config.pixel_fmt = sensor_info.pixel_format;
    isp_config.size.height = sensor_info.height;
    isp_config.size.width = sensor_info.width;
    isp_config.wdr_mode = sensor_info.wdr_mode;

    // isp pipe init
    memset(&g_isp_param, 0 , sizeof(sample_isp_param));
    g_isp_param.pipe[g_vi_pipe] = g_vi_pipe;
    g_isp_param.isp_info[g_vi_pipe].isp_pipe_en = XMEDIA_TRUE;
    g_isp_param.isp_info[g_vi_pipe].isp_sensor_en = XMEDIA_TRUE;
    g_isp_param.isp_info[g_vi_pipe].sensor_type = sensor_type;
    g_isp_param.isp_info[g_vi_pipe].mirror = mirror;
    g_isp_param.isp_info[g_vi_pipe].flip = flip;

    memcpy(&(g_isp_param.isp_info[g_vi_pipe].isp_config), &isp_config, sizeof(xmedia_isp_config));

    // isp init
    ret = sample_comm_isp_init(&g_isp_param, &g_vi_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("isp init failed!\n");
        goto exit0;
    }

    // vi start
    ret = sample_comm_vi_start(&g_vi_config, &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("start vi failed!\n");
        goto exit1;
    }

    // isp start
    ret = sample_comm_isp_start(&g_isp_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("start isp failed!\n");
        goto exit2;
    }

    // vpss config
    memset(&g_vpss_config, 0 , sizeof(sample_vpss_config));
    ret = sample_comm_vpss_get_default_pipe_cfg(&g_vpss_config.pipe_info[g_vpss_pipe].pipe_config, pic_size, &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get default pipe cfg failed !\n");
        goto exit3;
    }

    g_vpss_ochn_size[0].width = sensor_info.width;
    g_vpss_ochn_size[0].height = sensor_info.height;
    g_vpss_ochn_size[1].width = 640;
    g_vpss_ochn_size[1].height = 480;

    g_vpss_config.pipe_info[g_vpss_pipe].pipe_en = XMEDIA_TRUE;
    g_vpss_config.pipe_info[g_vpss_pipe].pipe_no = g_vpss_pipe;

    g_vpss_chn = g_vpss_ochn[0];
    g_vpss_config.pipe_info[g_vpss_pipe].chn_info[g_vpss_chn].chn_en = XMEDIA_TRUE;
    g_vpss_config.pipe_info[g_vpss_pipe].chn_info[g_vpss_chn].chn_no = g_vpss_ochn[0];
    ret = sample_comm_vpss_get_default_ochn_cfg(&g_vpss_config.pipe_info[g_vpss_pipe].chn_info[g_vpss_chn].chn_config,
                                                g_vpss_ochn_size[g_vpss_chn], &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get default pipe cfg failed !\n");
        goto exit3;
    }

    g_vpss_chn = g_vpss_ochn[1];
    g_vpss_config.pipe_info[g_vpss_pipe].chn_info[g_vpss_chn].chn_en = XMEDIA_TRUE;
    g_vpss_config.pipe_info[g_vpss_pipe].chn_info[g_vpss_chn].chn_no = g_vpss_ochn[1];
    ret = sample_comm_vpss_get_default_ochn_cfg(&g_vpss_config.pipe_info[g_vpss_pipe].chn_info[g_vpss_chn].chn_config,
                                                g_vpss_ochn_size[g_vpss_chn], &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get default pipe cfg failed !\n");
        goto exit3;
    }

    // vpss start
    ret = sample_comm_vpss_start(&g_vpss_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vpss start failed !\n");
        goto exit3;
    }

    ret = sample_comm_sys_vi_bind_vpss(g_vi_pipe, g_vi_chn, g_vpss_pipe, g_vpss_ichn);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vpss bind vo failed !\n");
        goto exit4;
    }

    // vo init
    ret = sample_comm_vo_get_default_config(vo_input, &g_vo_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get vo default config failed !\n");
        goto exit5;
    }

    ret = sample_comm_vo_start(g_vo_dev, g_vo_layer, g_vo_chn, g_vo_chn_num, g_vo_display, &g_vo_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vo start failed !\n");
        goto exit5;
    }

    ret = sample_comm_sys_vpss_bind_vo(g_vpss_pipe, g_vpss_ochn[0], g_vo_layer, g_vo_chn[0]);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vpss bind vo failed !\n");
        goto exit6;
    }

    return XMEDIA_SUCCESS;

exit6:
    sample_comm_vo_stop(g_vo_dev, g_vo_layer, g_vo_chn, g_vo_chn_num);
exit5:
    sample_comm_sys_vi_unbind_vpss(g_vi_pipe, g_vi_chn, g_vpss_pipe, g_vpss_ichn);
exit4:
    sample_comm_vpss_stop(&g_vpss_config);
exit3:
    sample_comm_isp_stop(&g_isp_param);
exit2:
    sample_comm_vi_stop(&g_vi_config);
exit1:
    sample_comm_isp_exit(&g_isp_param);
exit0:
    sample_region_sys_exit();

    return ret;
}

xmedia_void sample_comm_media_vi_vpss_vo_stop(xmedia_void)
{
    sample_comm_sys_vpss_unbind_vo(g_vpss_pipe, g_vpss_ochn[0], g_vo_layer, g_vo_chn[0]);
    sample_comm_vo_stop(g_vo_dev, g_vo_layer, g_vo_chn, g_vo_chn_num);
    sample_comm_sys_vi_unbind_vpss(g_vi_pipe, g_vi_chn, g_vpss_pipe, g_vpss_ichn);
    sample_comm_vpss_stop(&g_vpss_config);
    sample_comm_isp_stop(&g_isp_param);
    sample_comm_vi_stop(&g_vi_config);
    sample_comm_isp_exit(&g_isp_param);
    sample_region_sys_exit();
    return ;
}

xmedia_s32 sample_comm_media_vi_vpss_venc_start(xmedia_void)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 blk_size = 0;
    xmedia_video_size pic_size = { 0 };
    xmedia_video_size sub_pic_size = { 0 };
    sample_comm_video_param video_param = { 0 };
    sample_sys_config sys_config = { 0 };

    xmedia_s32 sensor_type = sample_region_sensor_type[0];
    xmedia_u32 framerate = 0;
    vi_sensor_info sensor_info = { 0 };
    xmedia_isp_config isp_config = { 0 };
    xmedia_bool mirror = XMEDIA_FALSE;
    xmedia_bool flip = XMEDIA_FALSE;
    xmedia_video_size g_vpss_ochn_size[VPSS_MAX_OCHN_NUM] = { 0 };

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);

    // sys init
    sys_config.sys_conf.pipe_mode[g_vi_pipe].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[g_vi_pipe].viproc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[g_vi_pipe].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;

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
    sys_config.vb_conf.common_pool[1].block_cnt = 5;

    sub_pic_size.width = 640;
    sub_pic_size.height = 480;
    blk_size = sample_comm_sys_get_buffer_size(sub_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[2].block_size = blk_size;
    sys_config.vb_conf.common_pool[2].block_cnt = 3;

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_region_sys_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sys init failed !\n");
        return ret;
    }
    memset(&g_vi_config, 0 , sizeof(sample_vi_config));
    g_vi_config.dev_info[g_vi_dev].dev_en = XMEDIA_TRUE;
    g_vi_config.dev_info[g_vi_dev].dev_no = g_vi_dev;
    g_vi_config.dev_info[g_vi_dev].sensor_type = sensor_type;
    g_vi_config.pipe_info[g_vi_pipe].pipe_en = XMEDIA_TRUE;
    g_vi_config.pipe_info[g_vi_pipe].pipe_no = g_vi_pipe;
    g_vi_config.pipe_info[g_vi_pipe].chn_info[g_vi_chn].chn_en = XMEDIA_TRUE;
    g_vi_config.pipe_info[g_vi_pipe].chn_info[g_vi_chn].chn_no = g_vi_chn;
    g_vi_config.dev_bind_pipe[g_vi_dev].pipe[0] = g_vi_pipe;
    g_vi_config.dev_bind_pipe[g_vi_dev].pipe[1] = -1;

    isp_config.bayer_fmt = XMEDIA_VIDEO_BAYER_FMT_RGGB;
    isp_config.fps = framerate;
    isp_config.mode_config.work_mode = XMEDIA_ISP_WORK_MODE_MASTER;
    isp_config.mode_config.master_mode.blend_stat_enable = XMEDIA_FALSE;
    isp_config.mode_config.master_mode.slave_num = 0;
    isp_config.pixel_fmt = sensor_info.pixel_format;
    isp_config.size.height = sensor_info.height;
    isp_config.size.width = sensor_info.width;
    isp_config.wdr_mode = sensor_info.wdr_mode;

    // isp pipe init
    memset(&g_isp_param, 0 , sizeof(sample_isp_param));
    g_isp_param.pipe[g_vi_pipe] = g_vi_pipe;
    g_isp_param.isp_info[g_vi_pipe].isp_pipe_en = XMEDIA_TRUE;
    g_isp_param.isp_info[g_vi_pipe].isp_sensor_en = XMEDIA_TRUE;
    g_isp_param.isp_info[g_vi_pipe].sensor_type = sensor_type;
    g_isp_param.isp_info[g_vi_pipe].mirror = mirror;
    g_isp_param.isp_info[g_vi_pipe].flip = flip;

    memcpy(&(g_isp_param.isp_info[g_vi_pipe].isp_config), &isp_config, sizeof(xmedia_isp_config));

    // isp init
    ret = sample_comm_isp_init(&g_isp_param, &g_vi_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("isp init failed!\n");
        goto exit0;
    }

    // vi start
    ret = sample_comm_vi_start(&g_vi_config, &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("start vi failed!\n");
        goto exit1;
    }

    // isp start
    ret = sample_comm_isp_start(&g_isp_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("start isp failed!\n");
        goto exit2;
    }

    // vpss config
    memset(&g_vpss_config, 0 , sizeof(sample_vpss_config));
    ret = sample_comm_vpss_get_default_pipe_cfg(&g_vpss_config.pipe_info[g_vpss_pipe].pipe_config, pic_size, &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get default pipe cfg failed !\n");
        goto exit3;
    }

    g_vpss_ochn_size[0].width = sensor_info.width;
    g_vpss_ochn_size[0].height = sensor_info.height;
    g_vpss_ochn_size[1].width = 640;
    g_vpss_ochn_size[1].height = 480;

    g_vpss_config.pipe_info[g_vpss_pipe].pipe_en = XMEDIA_TRUE;
    g_vpss_config.pipe_info[g_vpss_pipe].pipe_no = g_vpss_pipe;

    g_vpss_chn = g_vpss_ochn[0];
    g_vpss_config.pipe_info[g_vpss_pipe].chn_info[g_vpss_chn].chn_en = XMEDIA_TRUE;
    g_vpss_config.pipe_info[g_vpss_pipe].chn_info[g_vpss_chn].chn_no = g_vpss_ochn[0];
    ret = sample_comm_vpss_get_default_ochn_cfg(&g_vpss_config.pipe_info[g_vpss_pipe].chn_info[g_vpss_chn].chn_config,
                                                g_vpss_ochn_size[g_vpss_chn], &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get default pipe cfg failed !\n");
        goto exit3;
    }

    g_vpss_chn = g_vpss_ochn[1];
    g_vpss_config.pipe_info[g_vpss_pipe].chn_info[g_vpss_chn].chn_en = XMEDIA_TRUE;
    g_vpss_config.pipe_info[g_vpss_pipe].chn_info[g_vpss_chn].chn_no = g_vpss_ochn[1];
    ret = sample_comm_vpss_get_default_ochn_cfg(&g_vpss_config.pipe_info[g_vpss_pipe].chn_info[g_vpss_chn].chn_config,
                                                g_vpss_ochn_size[g_vpss_chn], &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get default pipe cfg failed !\n");
        goto exit3;
    }

    // vpss start
    ret = sample_comm_vpss_start(&g_vpss_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vpss start failed !\n");
        goto exit3;
    }

    ret = sample_comm_sys_vi_bind_vpss(g_vi_pipe, g_vi_chn, g_vpss_pipe, g_vpss_ichn);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vpss bind vo failed !\n");
        goto exit4;
    }

    app_liveserver_init();
    app_liveserver_start(g_vpss_ochn_size, g_venc_chn_cnt, framerate);

    // venc config
    memset(&g_venc_config, 0 , sizeof(sample_venc_config));
    g_venc_chnl = g_venc_chn[0];
    g_venc_config.chn_info[g_venc_chnl].venc_en = XMEDIA_TRUE;
    g_venc_config.chn_info[g_venc_chnl].venc_chn = g_venc_chnl;
    g_venc_config.chn_info[g_venc_chnl].payload_type = PT_H265;
    g_venc_config.chn_info[g_venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
    sample_comm_venc_get_default_chn_info(g_vpss_ochn_size[0], framerate, &g_venc_config.chn_info[g_venc_chnl]);

    g_venc_chnl = g_venc_chn[1];
    g_venc_config.chn_info[g_venc_chnl].venc_en = XMEDIA_TRUE;
    g_venc_config.chn_info[g_venc_chnl].venc_chn = g_venc_chnl;
    g_venc_config.chn_info[g_venc_chnl].payload_type = PT_H265;
    g_venc_config.chn_info[g_venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
    sample_comm_venc_get_default_chn_info(g_vpss_ochn_size[1], framerate, &g_venc_config.chn_info[g_venc_chnl]);

    ret = sample_comm_venc_start(&g_venc_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start failed !\n");
        goto exit5;
    }

    ret = sample_comm_sys_vpss_bind_venc(g_vpss_pipe, g_vpss_ochn[0], g_venc_chn[0]);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vpss bind venc failed !\n");
        goto exit6;
    }

    ret = sample_comm_sys_vpss_bind_venc(g_vpss_pipe, g_vpss_ochn[1], g_venc_chn[1]);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vpss bind venc failed !\n");
        goto exit7;
    }

    ret = sample_comm_venc_start_get_stream(g_venc_chn, g_venc_chn_cnt, XMEDIA_TRUE, XMEDIA_FALSE, ".");
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start get stream failed !\n");
        goto exit8;
    }

    return XMEDIA_SUCCESS;

exit8:
    app_liveserver_stop();
    app_liveserver_deinit();
    sample_comm_sys_vpss_unbind_venc(g_vpss_pipe, g_vpss_ochn[1], g_venc_chn[1]);
exit7:
    sample_comm_sys_vpss_unbind_venc(g_vpss_pipe, g_vpss_ochn[0], g_venc_chn[0]);
exit6:
    sample_comm_venc_stop(&g_venc_config);
exit5:
    sample_comm_sys_vi_unbind_vpss(g_vi_pipe, g_vi_chn, g_vpss_pipe, g_vpss_ichn);
exit4:
    sample_comm_vpss_stop(&g_vpss_config);
exit3:
    sample_comm_isp_stop(&g_isp_param);
exit2:
    sample_comm_vi_stop(&g_vi_config);
exit1:
    sample_comm_isp_exit(&g_isp_param);
exit0:
    sample_region_sys_exit();

    return ret;
}

xmedia_void sample_comm_media_vi_vpss_venc_stop(xmedia_void)
{
    app_liveserver_stop();
    app_liveserver_deinit();
    sample_comm_sys_vpss_unbind_venc(g_vpss_pipe, g_vpss_ochn[1], g_venc_chn[1]);
    sample_comm_sys_vpss_unbind_venc(g_vpss_pipe, g_vpss_ochn[0], g_venc_chn[0]);
    sample_comm_venc_stop(&g_venc_config);
    sample_comm_sys_vi_unbind_vpss(g_vi_pipe, g_vi_chn, g_vpss_pipe, g_vpss_ichn);
    sample_comm_vpss_stop(&g_vpss_config);
    sample_comm_isp_stop(&g_isp_param);
    sample_comm_vi_stop(&g_vi_config);
    sample_comm_isp_exit(&g_isp_param);
    sample_region_sys_exit();
    return ;
}

static xmedia_s32 sample_region_set_bmp(xmedia_s32 handle_num, xmedia_rgn_type type)
{
    xmedia_s32 min_handle, i, ret;

    min_handle = sample_comm_region_get_min_handle(type);
    ret = sample_comm_region_check_min(min_handle);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("min_handle(%d) should be in [0, %d).\n", min_handle, RGN_HANDLE_MAX);
        return ret;
    }

    if (type == XMEDIA_RGN_TYPE_OVERLAY || type == XMEDIA_RGN_TYPE_OVERLAYEX) {
        for (i = min_handle; i < min_handle + handle_num; i++) {
            ret = sample_comm_region_get_up_canvas(i, g_path_bmp);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("sample_comm_region_get_up_canvas failed!\n");
                return ret;
            }
        }
    }
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_region_media_start(xmedia_s32 handle_num,xmedia_rgn_type  type, xmedia_chn_info *chn)
{
    xmedia_s32 ret;
    xmedia_s32 min_handle;

    if (g_venc_use == XMEDIA_TRUE) {
        ret = sample_comm_media_vi_vpss_venc_start();
    }  else {
        ret = sample_comm_media_vi_vpss_vo_start();
    }
    if(XMEDIA_SUCCESS != ret) {
        return ret;
    }

    ret = sample_comm_region_create(handle_num, type);
    if(XMEDIA_SUCCESS != ret) {
        SAMPLE_PRT("sample_comm_region_create failed!\n");
        goto EXIT1;
    }

    ret = sample_comm_region_attach_to_chn(handle_num, type, chn);
    if(XMEDIA_SUCCESS != ret) {
        SAMPLE_PRT("sample_comm_region_attach_to_chn failed!\n");
        goto EXIT2;
    }

    min_handle = sample_comm_region_get_min_handle(type);
    if(min_handle == -1) {
        SAMPLE_PRT("sample_comm_region_get_min_handle failed!\n");
        return XMEDIA_FAILURE;
    }

    ret = sample_region_set_bmp(handle_num, type);
    if(XMEDIA_SUCCESS != ret) {
        SAMPLE_PRT("sample_comm_region_attach_to_chn failed!\n");
        goto EXIT2;
    }

    PAUSE(0);
EXIT2:
    ret = sample_comm_region_detach_frm_chn(handle_num, type, chn);
    if(XMEDIA_SUCCESS!= ret) {
        SAMPLE_PRT("sample_comm_region_detach_frm_chn failed!\n");
    }
EXIT1:
    ret = sample_comm_region_destroy(handle_num, type);
    if(XMEDIA_SUCCESS != ret) {
        SAMPLE_PRT("sample_comm_region_destroy failed!\n");
    }

    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_venc_stop_get_stream();
        sample_comm_media_vi_vpss_venc_stop();
    }  else {
        sample_comm_media_vi_vpss_vo_stop();
    }

    return ret;
}

xmedia_s32 sample_region_vpss_overlayex(xmedia_void)
{
    xmedia_s32 ret;
    xmedia_s32 handle_num;
    xmedia_rgn_type type;
    xmedia_chn_info chn;

    handle_num = 4;
    type = XMEDIA_RGN_TYPE_OVERLAYEX;
    chn.mod_id = MOD_ID_VPSS;
    chn.dev_id = 0;
    chn.chn_id = 0;
    g_path_bmp = mm_bmp;

    ret = sample_region_media_start(handle_num, type, &chn);
    return ret;
}

xmedia_s32 sample_region_vpss_coverex(xmedia_void)
{
    xmedia_s32 ret;
    xmedia_s32 handle_num;
    xmedia_rgn_type type;
    xmedia_chn_info chn;

    handle_num = 4;
    type = XMEDIA_RGN_TYPE_COVEREX;
    chn.mod_id = MOD_ID_VPSS;
    chn.dev_id = 0;
    chn.chn_id = 0;

    ret = sample_region_media_start(handle_num, type, &chn);
    return ret;
}

xmedia_s32 sample_region_vpss_mosaicex(xmedia_void)
{
    xmedia_s32 ret;
    xmedia_s32 handle_num;
    xmedia_rgn_type type;
    xmedia_chn_info chn;

    handle_num = 4;
    type = XMEDIA_RGN_TYPE_MOSAICEX;
    chn.mod_id = MOD_ID_VPSS;
    chn.dev_id = 0;
    chn.chn_id = 0;

    ret = sample_region_media_start(handle_num, type, &chn);
    return ret;
}

xmedia_s32 sample_region_vpss_corner_rectex(xmedia_void)
{
    xmedia_s32 ret;
    xmedia_s32 handle_num;
    xmedia_rgn_type type;
    xmedia_chn_info chn;

    handle_num = 4;
    type = XMEDIA_RGN_TYPE_CORNER_RECTEX;
    chn.mod_id = MOD_ID_VPSS;
    chn.dev_id = 0;
    chn.chn_id = 0;

    ret = sample_region_media_start(handle_num, type, &chn);
    return ret;
}

xmedia_s32 sample_region_vo_overlayex(xmedia_void)
{
    xmedia_s32 ret;
    xmedia_s32 handle_num;
    xmedia_rgn_type type;
    xmedia_chn_info chn;

    handle_num = 1;
    type = XMEDIA_RGN_TYPE_OVERLAYEX;
    chn.mod_id = MOD_ID_VO;
    chn.dev_id = 0;
    chn.chn_id = 0;
    g_path_bmp = mm_bmp;
    g_vo_display = XMEDIA_TRUE;
    g_venc_use = XMEDIA_FALSE;
    ret = sample_region_media_start(handle_num, type, &chn);
    g_vo_display = XMEDIA_FALSE;
    g_venc_use = XMEDIA_TRUE;
    return ret;
}

xmedia_s32 sample_region_vo_coverex(xmedia_void)
{
    xmedia_s32 ret;
    xmedia_s32 handle_num;
    xmedia_rgn_type type;
    xmedia_chn_info chn;

    handle_num = 1;
    type = XMEDIA_RGN_TYPE_COVEREX;
    chn.mod_id = MOD_ID_VO;
    chn.dev_id = 0;
    chn.chn_id = 0;
    g_vo_display = XMEDIA_TRUE;
    g_venc_use = XMEDIA_FALSE;
    ret = sample_region_media_start(handle_num, type, &chn);
    g_vo_display = XMEDIA_FALSE;
    g_venc_use = XMEDIA_TRUE;
    return ret;
}

xmedia_s32 sample_region_vo_corner_rectex(xmedia_void)
{
    xmedia_s32 ret;
    xmedia_s32 handle_num;
    xmedia_rgn_type type;
    xmedia_chn_info chn;
    sample_vo_config_input vo_input = { 0 };

    vo_input.square_sort = 1;
    vo_input.screen_type = sample_vo_screen_type[0];
    ret = sample_comm_vo_get_default_config(vo_input, &g_vo_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vo_get_default_config failed !\n");
        return ret;
    }

    handle_num = RGN_CALC_HANDLE_NUM(g_vo_config.chn_attr[0].out_rect.width, g_vo_config.chn_attr[0].out_rect.height);
    type = XMEDIA_RGN_TYPE_CORNER_RECTEX;
    chn.mod_id = MOD_ID_VO;
    chn.dev_id = 0;
    chn.chn_id = 0;
    g_vo_display = XMEDIA_TRUE;
    g_venc_use = XMEDIA_FALSE;
    ret = sample_region_media_start(handle_num, type, &chn);
    g_vo_display = XMEDIA_FALSE;
    g_venc_use = XMEDIA_TRUE;
    return ret;
}

xmedia_s32 sample_region_venc_overlay(xmedia_void)
{
    xmedia_s32 ret;
    xmedia_s32 handle_num;
    xmedia_rgn_type type;
    xmedia_chn_info chn;

    handle_num = 2;
    type = XMEDIA_RGN_TYPE_OVERLAY;
    chn.mod_id = MOD_ID_VENC;
    chn.dev_id = 0;
    chn.chn_id = 0;
    g_path_bmp = mm_bmp;
    g_vo_display = XMEDIA_FALSE;
    g_venc_use = XMEDIA_TRUE;
    ret = sample_region_media_start(handle_num, type, &chn);
    g_vo_display = XMEDIA_FALSE;
    g_venc_use = XMEDIA_TRUE;
    return ret;
}

xmedia_s32 sample_region_venc_cover(xmedia_void)
{
    xmedia_s32 ret;
    xmedia_s32 handle_num;
    xmedia_rgn_type type;
    xmedia_chn_info chn;

    handle_num = 2;
    type = XMEDIA_RGN_TYPE_COVER;
    chn.mod_id = MOD_ID_VENC;
    chn.dev_id = 0;
    chn.chn_id = 0;
    g_path_bmp = mm_bmp;
    g_vo_display = XMEDIA_FALSE;
    g_venc_use = XMEDIA_TRUE;
    ret = sample_region_media_start(handle_num, type, &chn);
    g_vo_display = XMEDIA_FALSE;
    g_venc_use = XMEDIA_TRUE;
    return ret;
}

xmedia_s32 sample_region_venc_mosaic(xmedia_void)
{
    xmedia_s32 ret;
    xmedia_s32 handle_num;
    xmedia_rgn_type type;
    xmedia_chn_info chn;

    handle_num = 2;
    type = XMEDIA_RGN_TYPE_MOSAIC;
    chn.mod_id = MOD_ID_VENC;
    chn.dev_id = 0;
    chn.chn_id = 0;
    g_path_bmp = mm_bmp;
    g_vo_display = XMEDIA_FALSE;
    g_venc_use = XMEDIA_TRUE;
    ret = sample_region_media_start(handle_num, type, &chn);
    g_vo_display = XMEDIA_FALSE;
    g_venc_use = XMEDIA_TRUE;
    return ret;
}

xmedia_s32 sample_region_venc_corner_rect(xmedia_void)
{
    xmedia_s32 ret;
    xmedia_s32 handle_num;
    xmedia_rgn_type type;
    xmedia_chn_info chn;

    handle_num = 2;
    type = XMEDIA_RGN_TYPE_CORNER_RECT;
    chn.mod_id = MOD_ID_VENC;
    chn.dev_id = 0;
    chn.chn_id = 0;
    g_path_bmp = mm_bmp;
    g_vo_display = XMEDIA_FALSE;
    g_venc_use = XMEDIA_TRUE;
    ret = sample_region_media_start(handle_num, type, &chn);
    g_vo_display = XMEDIA_FALSE;
    g_venc_use = XMEDIA_TRUE;
    return ret;
}

int main(int argc, char* argv[])
{
    xmedia_s32 ret = XMEDIA_FAILURE;
    xmedia_s32 index;

    if (argc != 2) {
        sample_region_usage(argv[0]);
        return XMEDIA_FAILURE;
    }

    if (!strncmp(argv[1], "-h", 2)) { /* 2:arg num */
        sample_region_usage(argv[0]);
        return XMEDIA_FAILURE;
    }

    signal(SIGINT, sample_region_handle_sig);
    signal(SIGTERM, sample_region_handle_sig);

    index = atoi(argv[1]);
    switch (index) {
        case 0:
            ret = sample_region_vpss_overlayex();
            break;
        case 1:
            ret = sample_region_vpss_coverex();
            break;
        case 2:
            ret = sample_region_vpss_mosaicex();
            break;
        case 3:
            ret = sample_region_vpss_corner_rectex();
            break;
        case 4:
            ret = sample_region_vo_overlayex();
            break;
        case 5:
            ret = sample_region_vo_coverex();
            break;
        case 6:
            ret = sample_region_vo_corner_rectex();
            break;
        case 7:
            ret = sample_region_venc_overlay();
            break;
        case 8:
            ret = sample_region_venc_cover();
            break;
        case 9:
            ret = sample_region_venc_mosaic();
            break;
        case 10:
            ret = sample_region_venc_corner_rect();
            break;
        default:
            SAMPLE_PRT("the index %d is invaild!\n",index);
            sample_region_usage(argv[0]);
            ret = XMEDIA_FAILURE;
            break;
    }

    if (XMEDIA_SUCCESS == ret) {
        SAMPLE_PRT("program exit normally!\n");
    } else {
        SAMPLE_PRT("program exit abnormally!\n");
    }

    return (ret);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

