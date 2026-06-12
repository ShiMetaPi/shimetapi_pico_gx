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

#include "sample_comm.h"
#include "sample_comm_sys.h"
#include "sample_comm_vi.h"
#include "sample_comm_vpss.h"
#include "sample_comm_vgs.h"
#include "sample_comm_venc.h"
#include "sample_comm_isp.h"
#include "sample_comm_livestream.h"

static sample_comm_sensor_type sample_venc_sensor_type[MAX_SENSOR_NUM] = {
    SENSOR0_TYPE,SENSOR1_TYPE,SENSOR2_TYPE,SENSOR3_TYPE,SENSOR4_TYPE
};
static xmedia_bool g_force_exit = XMEDIA_FALSE;
extern xmedia_bool g_user_start;

xmedia_void sample_venc_usage(xmedia_char* args)
{
    printf("Usage1 : %s [index] \n", args);
    printf("index:\n");
    printf("\t  0) h265(large stream)+h264(small stream)+jpeg lowdelay encode with ringbuf.\n");
    printf("\t  1) h265 + h264 (large stream + small stream).\n");
    printf("\t  2) qpmap (h265 + h264).\n");
    printf("\t  3) roi   (h265 + h264).\n");
    printf("\t  4) jpeg  (Mjpeg + jpeg snap).\n");
    printf("\t  5) jpeg  snap split amplification encode.\n");
    printf("Usage2 : [nv21 yuv file] + [width] + [height] + [target format]\n");
    printf("\t  target format = 265, h265, H265, 264, h264, H264, jpg, jpeg, mjpg, mjpeg\n");
    printf("\t  Example: ./sample_venc input.yuv 1920 1080 h264\n");

    return;
}

// 获取码率控制模式
sample_rc sample_venc_get_rc_mode(void)
{
    char c;
    sample_rc rc_mode = 0;

begin_get:

    printf("please input choose rc mode!\n");
    printf("\t c) cbr.\n");
    printf("\t v) vbr.\n");
    printf("\t a) avbr.\n");
    printf("\t f) fixQp\n");

    c = getchar();//getchar(stdio.h)-从标准输入读取一个字符
    while(getchar() != '\n' && c != EOF);//EOF-文件结束标志
    switch(c)
    {
        case 'c':
            rc_mode = SAMPLE_RC_CBR;
            break;
        case 'v':
            rc_mode = SAMPLE_RC_VBR;
            break;
        case 'a':
            rc_mode = SAMPLE_RC_AVBR;
            break;
        case 'f':
            rc_mode = SAMPLE_RC_FIXQP;
            break;
        default:
            SAMPLE_PRT("input rcmode: %c, is invaild!\n",c);
            goto begin_get;
    }
    return rc_mode;
}

xmedia_venc_rc_mode sample_venc_set_rc_mode(sample_rc rc_mode, xmedia_payload_type type)
{
    xmedia_venc_rc_mode venc_rc_mode = 0;

    if (type == PT_H264) {
        switch (rc_mode) {
            case SAMPLE_RC_CBR:
                venc_rc_mode = VENC_RC_MODE_H264CBR;
                break;
            case SAMPLE_RC_VBR:
                venc_rc_mode = VENC_RC_MODE_H264VBR;
                break;
            case SAMPLE_RC_AVBR:
                venc_rc_mode = VENC_RC_MODE_H264AVBR;
                break;
            case SAMPLE_RC_QPMAP:
                venc_rc_mode = VENC_RC_MODE_H264QPMAP;
                break;
            case SAMPLE_RC_FIXQP:
                venc_rc_mode = VENC_RC_MODE_H264FIXQP;
                break;
        }
    } else if (type == PT_H265) {
        switch (rc_mode) {
            case SAMPLE_RC_CBR:
                venc_rc_mode = VENC_RC_MODE_H265CBR;
                break;
            case SAMPLE_RC_VBR:
                venc_rc_mode = VENC_RC_MODE_H265VBR;
                break;
            case SAMPLE_RC_AVBR:
                venc_rc_mode = VENC_RC_MODE_H265AVBR;
                break;
            case SAMPLE_RC_QPMAP:
                venc_rc_mode = VENC_RC_MODE_H265QPMAP;
                break;
            case SAMPLE_RC_FIXQP:
                venc_rc_mode = VENC_RC_MODE_H265FIXQP;
                break;
        }
    } else if (type == PT_MJPEG) {
        switch (rc_mode) {
            case SAMPLE_RC_CBR:
                venc_rc_mode = VENC_RC_MODE_MJPEGCBR;
                break;
            case SAMPLE_RC_VBR:
                venc_rc_mode = VENC_RC_MODE_MJPEGVBR;
                break;
            case SAMPLE_RC_FIXQP:
                venc_rc_mode = VENC_RC_MODE_MJPEGFIXQP;
                break;
            default:
                SAMPLE_PRT("input rcmode: %c, Mjpeg not support, set to CBR !\n", rc_mode);
                break;
        }
    }

    return venc_rc_mode;
}

xmedia_venc_gop_mode sample_venc_get_gop_mode(void)
{
    char c;
    xmedia_venc_gop_mode gop_mode = 0;

begin_get:

    printf("please input choose gop mode!\n");
    printf("\t 0) NORMALP.\n");
    printf("\t 1) SMARTP.\n");

    c = getchar();
    while(getchar() != '\n' && c != EOF);
    switch(c)
    {
        case '0':
            gop_mode = VENC_GOPMODE_NORMALP;
            break;
        case '1':
            gop_mode = VENC_GOPMODE_SMARTP;
            break;
        case '2':
            gop_mode = VENC_GOPMODE_ADVSMARTP;
            break;
        default:
            SAMPLE_PRT("input rcmode: %c, is invaild!\n",c);
            goto begin_get;
    }

    return gop_mode;
}

// 系统初始化
xmedia_s32 sample_venc_sys_init(sample_sys_config *sys_config)
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

xmedia_void sample_venc_sys_exit(void)
{
    sample_comm_venc_exit();
    sample_comm_vpss_exit();
    sample_comm_vi_exit();
    sample_comm_sys_exit();
}

xmedia_void sample_venc_handle_sig(xmedia_s32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo || SIGTERM == signo) {
        g_force_exit = XMEDIA_TRUE;
    }
}

xmedia_s32 sample_venc_ring()
{
    xmedia_s32 ret = XMEDIA_SUCCESS;//XMEDIA_SUCCESS-成功
    xmedia_s32 ch;//ch-字符
    xmedia_s32 blk_size = 0;//blk_size-块大小
    xmedia_video_size pic_size = { 0 };//pic_size-图片大小
    xmedia_video_size sub_pic_size = { 0 };//sub_pic_size-子图片大小
    sample_comm_video_param video_param = { 0 };//摄像头参数
    sample_sys_config sys_config = { 0 };//系统配置

    xmedia_s32 vi_dev = 0;
    xmedia_s32 vi_pipe = 0;//vi_pipe-视频管道
    xmedia_s32 vi_chn = 0;//vi_chn-视频通道 
    sample_vi_config vi_config = { 0 };//vi_config-视频配置

    xmedia_s32 sensor_type = sample_venc_sensor_type[0];//摄像头类型
    xmedia_u32 framerate = 0;//framerate-帧率
    vi_sensor_info sensor_info = { 0 };//摄像头信息
    sample_isp_param isp_param = { 0 };//isp参数
    xmedia_isp_config isp_config = { 0 };//isp配置
    xmedia_bool mirror = XMEDIA_FALSE;//mirror-镜像
    xmedia_bool flip = XMEDIA_FALSE;//flip-翻转

    xmedia_s32 vpss_chnl = 0;//vpss_chnl-视频通道
    xmedia_s32 vpss_pipe = 0;//vpss_pipe-视频管道
    xmedia_s32 vpss_ichn = 0;//vpss_ichn-视频通道
    xmedia_s32 vpss_ochn[3] = { 0, 1, 2};
    sample_vpss_config vpss_config = { 0 };//vpss_config-视频配置
    xmedia_video_size vpss_ochn_size[VPSS_MAX_OCHN_NUM] = { 0 };//vpss_ochn_size-视频通道大小

    xmedia_s32 venc_chnl = 0;//venc_chnl-视频通道
    xmedia_s32 venc_chn[3] = { 0, 1, 2}; //venc_chn-视频通道
    xmedia_s32 venc_chn_cnt = 3;//venc_chn_cnt-视频通道数量
    sample_venc_config venc_config = { 0 };//venc_config-视频配置
    sample_rc rc_mode;//rc_mode-码率控制模式
    xmedia_venc_gop_mode gop_mode;//gop_mode-gop模式

    // 原始图像数据格式设置
    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;//视频存储格式-线性
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;//像素格式-YVU半平面420
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;//数据宽度-8位
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;//压缩模式-无压缩

    // 获取摄像头信息
    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    // 获取摄像头帧率
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);

    rc_mode = sample_venc_get_rc_mode();//从标准输入获取码率控制模式
    gop_mode = sample_venc_get_gop_mode();//从输入获取gop模式

    // sys init
    sys_config.sys_conf.pipe_mode[vi_pipe].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[vi_pipe].viproc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[vi_pipe].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;

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
    sys_config.vb_conf.common_pool[2].block_cnt = 2;

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_venc_sys_init(&sys_config);
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
    vpss_ochn_size[1].width = 640;
    vpss_ochn_size[1].height = 480;

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
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.depth = 0;

    ret = sample_comm_vpss_set_ochn_wrap(&vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl]);
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
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.depth = 0;

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

    //venc init
    app_liveserver_init();
    app_liveserver_start(vpss_ochn_size,venc_chn_cnt - 1,framerate);

    venc_chnl = venc_chn[0];
    venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
    venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
    venc_config.chn_info[venc_chnl].payload_type = PT_H265;
    venc_config.chn_info[venc_chnl].rc_mode = sample_venc_set_rc_mode(rc_mode, venc_config.chn_info[venc_chnl].payload_type);
    venc_config.chn_info[venc_chnl].venc_gop_attr.gop_mode = gop_mode;
    sample_comm_venc_get_default_chn_info(vpss_ochn_size[0], framerate, &venc_config.chn_info[venc_chnl]);

    venc_chnl = venc_chn[1];
    venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
    venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
    venc_config.chn_info[venc_chnl].payload_type = PT_H264;
    venc_config.chn_info[venc_chnl].rc_mode = sample_venc_set_rc_mode(rc_mode, venc_config.chn_info[venc_chnl].payload_type);
    venc_config.chn_info[venc_chnl].venc_gop_attr.gop_mode = gop_mode;
    sample_comm_venc_get_default_chn_info(vpss_ochn_size[1], framerate, &venc_config.chn_info[venc_chnl]);

    venc_chnl = venc_chn[2];
    venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
    venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
    venc_config.chn_info[venc_chnl].payload_type = PT_JPEG;
    sample_comm_venc_get_default_chn_info(vpss_ochn_size[0], framerate, &venc_config.chn_info[venc_chnl]);

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

    ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[0], venc_chn[2]);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vpss bind venc failed !\n");
        goto exit7;
    }

    ret = sample_comm_venc_start_get_stream(venc_chn, venc_chn_cnt - 1, XMEDIA_TRUE, XMEDIA_FALSE, ".");
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start get stream failed !\n");
        goto exit8;
    }

    printf("---press 'q' to exit or any other key to capture pic---\n");
    while (!g_force_exit) {
        ch = sample_comm_sys_getchar_timeout(200);
        if (ch == 'q') {
            break;
        } else if (ch == XMEDIA_FAILURE) {
            continue;
        }

        ret = sample_comm_venc_capture_jpeg(venc_chn[2], 1, XMEDIA_TRUE, XMEDIA_FALSE, ".");
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("capture jpeg failed!\n");
            break;
        }
    }
    sample_comm_venc_stop_get_stream();

exit8:
    app_liveserver_stop();
    app_liveserver_deinit();
    sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[0], venc_chn[2]);
exit7:
    sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
exit6:
    sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
exit5:
    sample_comm_venc_stop(&venc_config);
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
    sample_venc_sys_exit();

    return ret;
}


xmedia_s32 sample_venc_h265_h264(xmedia_bool set_roi)
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

    xmedia_s32 sensor_type = sample_venc_sensor_type[0];
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
    sample_rc rc_mode;
    xmedia_venc_gop_mode gop_mode;

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);

    rc_mode = sample_venc_get_rc_mode();
    gop_mode = sample_venc_get_gop_mode();

    // sys init
    sys_config.sys_conf.pipe_mode[vi_pipe].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[vi_pipe].viproc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[vi_pipe].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;

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
    sys_config.vb_conf.common_pool[1].block_cnt = 4;

    sub_pic_size.width = 640;
    sub_pic_size.height = 480;
    blk_size = sample_comm_sys_get_buffer_size(sub_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[2].block_size = blk_size;
    sys_config.vb_conf.common_pool[2].block_cnt = 2;

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_venc_sys_init(&sys_config);
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
    vpss_ochn_size[1].width = 640;
    vpss_ochn_size[1].height = 480;

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
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.depth = 0;

    vpss_chnl = vpss_ochn[1];
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_no = vpss_ochn[1];
    ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
                                                vpss_ochn_size[vpss_chnl], &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get default pipe cfg failed !\n");
        goto exit2;
    }
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.depth = 0;

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

    //venc init
    app_liveserver_init();
    app_liveserver_start(vpss_ochn_size,venc_chn_cnt,framerate);

    venc_chnl = venc_chn[0];
    venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
    venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
    venc_config.chn_info[venc_chnl].payload_type = PT_H265;
    venc_config.chn_info[venc_chnl].rc_mode = sample_venc_set_rc_mode(rc_mode, venc_config.chn_info[venc_chnl].payload_type);
    venc_config.chn_info[venc_chnl].venc_gop_attr.gop_mode = gop_mode;
    sample_comm_venc_get_default_chn_info(vpss_ochn_size[0], framerate, &venc_config.chn_info[venc_chnl]);

    venc_chnl = venc_chn[1];
    venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
    venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
    venc_config.chn_info[venc_chnl].payload_type = PT_H264;
    venc_config.chn_info[venc_chnl].rc_mode = sample_venc_set_rc_mode(rc_mode, venc_config.chn_info[venc_chnl].payload_type);
    venc_config.chn_info[venc_chnl].venc_gop_attr.gop_mode = gop_mode;
    sample_comm_venc_get_default_chn_info(vpss_ochn_size[1], framerate, &venc_config.chn_info[venc_chnl]);

    ret = sample_comm_venc_start(&venc_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start failed !\n");
        goto exit4;
    }

    if (set_roi == XMEDIA_TRUE) {
        ret = sample_comm_venc_set_roi(venc_chn, venc_chn_cnt);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("venc start get stream failed !\n");
            goto exit4;
        }
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

    PAUSE(g_force_exit);
    sample_comm_venc_stop_get_stream();

exit7:
    app_liveserver_stop();
    app_liveserver_deinit();
    sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
exit6:
    sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
exit5:
    sample_comm_venc_stop(&venc_config);
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
    sample_venc_sys_exit();

    return ret;
}

xmedia_s32 sample_venc_qpmap()
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

    xmedia_s32 sensor_type = sample_venc_sensor_type[0];
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
    xmedia_venc_gop_mode gop_mode;

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);

    gop_mode = sample_venc_get_gop_mode();

    // sys init
    sys_config.sys_conf.pipe_mode[vi_pipe].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[vi_pipe].viproc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[vi_pipe].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;

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
    sys_config.vb_conf.common_pool[1].block_cnt = 4;

    sub_pic_size.width = 640;
    sub_pic_size.height = 480;
    blk_size = sample_comm_sys_get_buffer_size(sub_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[2].block_size = blk_size;
    sys_config.vb_conf.common_pool[2].block_cnt = 2;

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_venc_sys_init(&sys_config);
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
    vpss_ochn_size[1].width = 640;
    vpss_ochn_size[1].height = 480;

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
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.depth = 0;

    vpss_chnl = vpss_ochn[1];
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_no = vpss_ochn[1];
    ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
                                                vpss_ochn_size[vpss_chnl], &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get default pipe cfg failed !\n");
        goto exit2;
    }
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.depth = 0;

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

    //venc init
    app_liveserver_init();
    app_liveserver_start(vpss_ochn_size, venc_chn_cnt, framerate);

    venc_chnl = venc_chn[0];
    venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
    venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
    venc_config.chn_info[venc_chnl].payload_type = PT_H265;
    venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265QPMAP;
    venc_config.chn_info[venc_chnl].venc_gop_attr.gop_mode = gop_mode;
    sample_comm_venc_get_default_chn_info(vpss_ochn_size[0], framerate, &venc_config.chn_info[venc_chnl]);

    venc_chnl = venc_chn[1];
    venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
    venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
    venc_config.chn_info[venc_chnl].payload_type = PT_H264;
    venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H264QPMAP;
    venc_config.chn_info[venc_chnl].venc_gop_attr.gop_mode = gop_mode;
    sample_comm_venc_get_default_chn_info(vpss_ochn_size[1], framerate, &venc_config.chn_info[venc_chnl]);

    ret = sample_comm_venc_start(&venc_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start failed !\n");
        goto exit4;
    }

    ret = sample_comm_venc_qpmap_sendfrm(vpss_pipe, vpss_ochn, venc_chn, venc_chn_cnt, vpss_ochn_size);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start failed !\n");
        goto exit4;
    }

    ret = sample_comm_venc_start_get_stream(venc_chn, venc_chn_cnt, XMEDIA_TRUE, XMEDIA_FALSE, ".");
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start get stream failed !\n");
        goto exit5;
    }

    PAUSE(g_force_exit);
    sample_comm_venc_stop_send_qpmapfrm();
    sample_comm_venc_stop_get_stream();

exit5:
    app_liveserver_stop();
    app_liveserver_deinit();
    sample_comm_venc_stop(&venc_config);
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
    sample_venc_sys_exit();

    return ret;
}


xmedia_s32 sample_venc_jpeg()
{
    xmedia_s32 ret = XMEDIA_SUCCESS;//成功

    xmedia_s32 blk_size = 0;//块大小
    xmedia_video_size pic_size = { 0 };//主图片大小
    xmedia_video_size sub_pic_size = { 0 };//子图片大小 
    xmedia_video_size dcf_pic_size = { 0 };//DCF图片大小
    sample_comm_video_param video_param = { 0 };
    sample_sys_config sys_config = { 0 };//系统配置

    xmedia_s32 vi_dev = 0;//vi_dev-视频设备
    xmedia_s32 vi_pipe = 0;//vi_pipe-视频管道
    xmedia_s32 vi_chn = 0;//vi_chn-视频通道
    sample_vi_config vi_config = { 0 };//vi_config-视频配置

    xmedia_s32 sensor_type = sample_venc_sensor_type[0];//摄像头类型
    xmedia_u32 framerate = 0;//帧率
    vi_sensor_info sensor_info = { 0 };//摄像头信息
    sample_isp_param isp_param = { 0 };//isp参数
    xmedia_isp_config isp_config = { 0 };//isp配置
    xmedia_bool mirror = XMEDIA_FALSE;//镜像
    xmedia_bool flip = XMEDIA_FALSE;//翻转

    xmedia_s32 vpss_chnl = 0;//vpss_chnl-视频通道
    xmedia_s32 vpss_pipe = 0;//vpss_pipe-视频管道
    xmedia_s32 vpss_ichn = 0;//vpss_ichn-视频通道
    xmedia_s32 vpss_ochn[2] = { 0, 1 };//vpss_ochn-视频通道
    sample_vpss_config vpss_config = { 0 };//vpss_config-视频配置
    xmedia_video_size vpss_ochn_size[VPSS_MAX_OCHN_NUM] = { 0 };//vpss_ochn_size-视频通道大小
    xmedia_s32 ch;//ch-字符
    xmedia_s32 chnl = 0;//chnl-通道
    xmedia_s32 venc_chn[2] = { 0, 1 };//venc_chn-视频通道
    xmedia_s32 venc_chn_cnt = 2;//venc_chn_cnt-视频通道数量
    sample_venc_config venc_config = { 0 };//venc_config-视频配置
    xmedia_bool support_dcf = XMEDIA_TRUE;//support_dcf-支持DCF     
    sample_rc rc_mode;//rc_mode-码率控制模式

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;//视频存储格式-线性
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;//像素格式-YVU半平面420
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;//数据宽度-8位
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;//压缩模式-无压缩

    // 获取摄像头信息
    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    // 获取摄像头帧率
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);

    rc_mode = sample_venc_get_rc_mode();//从标准输入获取码率控制模式
    
    // sys init-系统初始化-离线工作模式-每个模块“独立处理 + 通过内存传递数据”
    sys_config.sys_conf.pipe_mode[vi_pipe].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;//vi-isp
    sys_config.sys_conf.pipe_mode[vi_pipe].viproc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;//isp-vpss
    sys_config.sys_conf.pipe_mode[vi_pipe].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;//gdc-vpss

    sys_config.vb_conf.max_pool_cnt = 25;//最大内存缓冲池数量-25

    pic_size.width = sensor_info.width;//图片宽度
    pic_size.height = sensor_info.height;//图片高度
    blk_size = sample_comm_sys_get_buffer_size(pic_size, video_param.video_fmt, sensor_info.pixel_format,
                                               sensor_info.bit_width, video_param.compress_mode);//获取缓冲区大小
    sys_config.vb_conf.common_pool[0].block_size = blk_size;//缓冲区大小
    sys_config.vb_conf.common_pool[0].block_cnt = 2;//缓冲区数量-2

    blk_size = sample_comm_sys_get_buffer_size(pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);//获取缓冲区大小  
    sys_config.vb_conf.common_pool[1].block_size = blk_size;//视频缓冲区大小
    sys_config.vb_conf.common_pool[1].block_cnt = 5;//视频缓冲区数量-5

    //保持子码流和主码流大小一致
    sub_pic_size.width = sensor_info.width;;
    sub_pic_size.height = sensor_info.height;
    //Pool Size=block_size×block_cnt
    blk_size = sample_comm_sys_get_buffer_size(sub_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[2].block_size = blk_size;
    sys_config.vb_conf.common_pool[2].block_cnt = 3;

    dcf_pic_size.width = 160;
    dcf_pic_size.height = 120;
    blk_size = sample_comm_sys_get_buffer_size(dcf_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[3].block_size = blk_size;
    sys_config.vb_conf.common_pool[3].block_cnt = 1;

    sys_config.vb_conf.supplement_config = 1;//补充配置-1

    ret = sample_venc_sys_init(&sys_config);
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
    vpss_ochn_size[1].width = 640;
    vpss_ochn_size[1].height = 480;

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
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.depth = 0;

    vpss_chnl = vpss_ochn[1];
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_no = vpss_ochn[1];
    ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
                                                vpss_ochn_size[vpss_chnl], &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get default pipe cfg failed !\n");
        goto exit2;
    }
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.depth = 0;

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

    //venc init
    sample_comm_vi_get_framerate_by_sensor(sensor_type,&framerate);

    chnl = venc_chn[0];
    venc_config.chn_info[chnl].venc_en = XMEDIA_TRUE;
    venc_config.chn_info[chnl].venc_chn = chnl;
    venc_config.chn_info[chnl].payload_type = PT_JPEG;
    venc_config.chn_info[chnl].support_dcf = support_dcf;
    venc_config.chn_info[chnl].mpf_cnt = 2;
    sample_comm_venc_get_default_chn_info(vpss_ochn_size[0],framerate,&venc_config.chn_info[chnl]);

    chnl = venc_chn[1];
    venc_config.chn_info[chnl].venc_en = XMEDIA_TRUE;
    venc_config.chn_info[chnl].venc_chn = chnl;
    venc_config.chn_info[chnl].payload_type = PT_MJPEG;
    venc_config.chn_info[chnl].rc_mode = sample_venc_set_rc_mode(rc_mode, venc_config.chn_info[chnl].payload_type);
    sample_comm_venc_get_default_chn_info(vpss_ochn_size[1],framerate,&venc_config.chn_info[chnl]);

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

    ret = sample_comm_venc_start_get_stream(&venc_chn[1], venc_chn_cnt - 1, XMEDIA_TRUE, XMEDIA_FALSE, ".");
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start get stream failed !\n");
        goto exit7;
    }

    ret = sample_comm_venc_set_jpeg_exif(venc_chn[0]);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start get stream failed !\n");
        goto exit7;
    }

    printf("---press 'q' to exit or any other key to capture pic---\n");
    while (!g_force_exit) {
        ch = sample_comm_sys_getchar_timeout(200);
        if (ch == 'q') {
            break;
        } else if (ch == XMEDIA_FAILURE) {
            continue;
        }

        ret = sample_comm_venc_capture_jpeg(venc_chn[0], 1, XMEDIA_TRUE, support_dcf, ".");
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("capture jpeg failed!\n");
            break;
        }
    }

    sample_comm_venc_stop_get_stream();
exit7:
    sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
exit6:
    sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
exit5:
    sample_comm_venc_stop(&venc_config);
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
    sample_venc_sys_exit();

    return ret;
}

xmedia_s32 sample_venc_jpeg_splicing()
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 blk_size = 0;
    xmedia_video_size pic_size = { 0 };
    xmedia_video_size block_size = { 0 };
    sample_comm_video_param video_param = { 0 };
    sample_sys_config sys_config = { 0 };
    xmedia_vb_pool_config user_vb_pool;
    xmedia_u32 pool  = VB_INVALID_POOLID;

    xmedia_s32 vi_dev = 0;
    xmedia_s32 vi_pipe = 0;
    xmedia_s32 vi_chn = 0;
    sample_vi_config vi_config = { 0 };

    xmedia_s32 sensor_type = sample_venc_sensor_type[0];
    xmedia_u32 framerate = 0;
    vi_sensor_info sensor_info = { 0 };
    sample_isp_param isp_param = { 0 };
    xmedia_isp_config isp_config = { 0 };
    xmedia_bool mirror = XMEDIA_FALSE;
    xmedia_bool flip = XMEDIA_FALSE;

    xmedia_s32 vpss_chnl = 0;
    xmedia_s32 vpss_pipe = 0;
    xmedia_s32 vpss_ichn = 0;
    xmedia_s32 vpss_ochn[1] = { 0 };
    sample_vpss_config vpss_config = { 0 };
    xmedia_video_size vpss_ochn_size[VPSS_MAX_OCHN_NUM] = { 0 };

    xmedia_s32 venc_chnl = 0;
    xmedia_s32 venc_chn[1] = { 0 };
    sample_venc_config venc_config = { 0 };
    xmedia_s32 ch;

    sample_venc_splicing_para splicing_para = { 0 };

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    /* src resolution */
    splicing_para.splicing_info.src_width = 1920;
    splicing_para.splicing_info.src_height = 1080;
    /* dst resolution */
    splicing_para.splicing_info.dst_width = 7680;
    splicing_para.splicing_info.dst_height = 4320;
    /* every block resolution */
    splicing_para.splicing_info.enc_width = 3840;
    splicing_para.splicing_info.enc_height = 256;
    splicing_para.splicing_info.row_cnt = splicing_para.splicing_info.dst_height /
                                          splicing_para.splicing_info.enc_height;
    if (splicing_para.splicing_info.dst_height %
        splicing_para.splicing_info.enc_height > 0) {
        splicing_para.splicing_info.row_cnt++;
    }
    /* attention: the more vertical blocks there are, the slower the splicing speed */
    splicing_para.splicing_info.col_cnt = splicing_para.splicing_info.dst_width /
                                          splicing_para.splicing_info.enc_width;
    if (splicing_para.splicing_info.dst_width %
        splicing_para.splicing_info.enc_width > 0) {
        SAMPLE_PRT("the width of each vertically divided block must be equal\n");
        return XMEDIA_FAILURE;
    }
    if (splicing_para.splicing_info.col_cnt > VENC_JPEG_SPLICING_MAX_VER_CNT) {
        SAMPLE_PRT("the cnt of vertically divided block must less than %d\n", VENC_JPEG_SPLICING_MAX_VER_CNT);
        return XMEDIA_FAILURE;
    }

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);

    // sys init
    sys_config.sys_conf.pipe_mode[vi_pipe].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[vi_pipe].viproc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[vi_pipe].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;

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
    sys_config.vb_conf.common_pool[1].block_cnt = 4;

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_venc_sys_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sys init failed !\n");
        return ret;
    }

    /* get scale block vb */
    memset(&user_vb_pool, 0, sizeof(xmedia_vb_pool_config));
    block_size.width = splicing_para.splicing_info.enc_width;
    block_size.height = splicing_para.splicing_info.enc_height;
    user_vb_pool.block_size  = sample_comm_sys_get_buffer_size(block_size, video_param.video_fmt, video_param.pixel_fmt,
                                                               video_param.data_width, video_param.compress_mode);
    user_vb_pool.block_cnt   = 1;
    user_vb_pool.map_mode    = XMEDIA_VB_MAP_MODE_NONE;
    pool = xmedia_vb_create_pool(&user_vb_pool);
    if (pool == VB_INVALID_POOLID) {
        SAMPLE_PRT("xmedia_vb_create_pool err:0x%x",ret);
        goto exit0;
    }
    splicing_para.splicing_info.vb_size = user_vb_pool.block_size;
    splicing_para.splicing_info.pool_id = pool;
    splicing_para.splicing_info.vb_block = xmedia_vb_get_block(splicing_para.splicing_info.pool_id,
                                                               splicing_para.splicing_info.vb_size, XMEDIA_NULL);
    splicing_para.splicing_info.phy_addr = xmedia_vb_handle_to_phy_addr(splicing_para.splicing_info.vb_block);
    splicing_para.splicing_info.vir_addr = (xmedia_u8 *)xmedia_mmz_map(splicing_para.splicing_info.phy_addr,
                                                                       splicing_para.splicing_info.vb_size, 0);
    if (splicing_para.splicing_info.vir_addr == NULL) {
        SAMPLE_PRT("mem dev may not open\n");
        xmedia_vb_release_block(splicing_para.splicing_info.vb_block);
        goto exit0;
    }
    ret = xmedia_vb_get_supplement_info(splicing_para.splicing_info.vb_block, &splicing_para.splicing_info.priv_vb_info);
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_vb_get_supplement_info error\n");
        xmedia_vb_release_block(splicing_para.splicing_info.vb_block);
        goto exit0;
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

    // vgs start
    ret = sample_comm_vgs_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("start vgs failed!\n");
        goto exit3;
    }

    // vpss config
    ret = sample_comm_vpss_get_default_pipe_cfg(&vpss_config.pipe_info[vpss_pipe].pipe_config, pic_size, &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get default pipe cfg failed !\n");
        goto exit3;
    }

    vpss_ochn_size[0].width = splicing_para.splicing_info.src_width;
    vpss_ochn_size[0].height = splicing_para.splicing_info.src_height;

    vpss_config.pipe_info[vpss_pipe].pipe_en = XMEDIA_TRUE;
    vpss_config.pipe_info[vpss_pipe].pipe_no = vpss_pipe;

    vpss_chnl = vpss_ochn[0];
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_no = vpss_ochn[0];
    ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
                                                vpss_ochn_size[vpss_chnl], &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get default pipe cfg failed !\n");
        goto exit3;
    }

    // vpss start
    ret = sample_comm_vpss_start(&vpss_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vpss start failed !\n");
        goto exit3;
    }

    ret = sample_comm_sys_vi_bind_vpss(vi_pipe, vi_chn, vpss_pipe, vpss_ichn);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vi bind vpss failed !\n");
        goto exit4;
    }

    venc_chnl = venc_chn[0];
    venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
    venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
    venc_config.chn_info[venc_chnl].payload_type = PT_JPEG;
    sample_comm_venc_get_default_chn_info(block_size, framerate, &venc_config.chn_info[venc_chnl]);

    ret = sample_comm_venc_start(&venc_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start failed !\n");
        goto exit5;
    }

    ret = sample_comm_venc_set_jpeg_exif(venc_chn[0]);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start get stream failed !\n");
        goto exit5;
    }

    printf("---press 'q' to exit or any other key to capture pic---\n");
    while (!g_force_exit) {
        ch = sample_comm_sys_getchar_timeout(200);
        if (ch == 'q') {
            break;
        } else if (ch == XMEDIA_FAILURE) {
            continue;
        }

        splicing_para.vpss_pipe = vpss_pipe;
        splicing_para.venc_chn = venc_chn[0];
        splicing_para.vpss_chn = vpss_ochn[0];
        snprintf(splicing_para.save_path, sizeof(splicing_para.save_path), ".");
        ret = sample_comm_venc_splicing_proc(&splicing_para);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("capture jpeg failed!\n");
            break;
        }
    }

    sample_comm_venc_stop(&venc_config);
exit5:
    sample_comm_sys_vi_unbind_vpss(vi_pipe, vi_chn, vpss_pipe, vpss_ichn);
exit4:
    sample_comm_vpss_stop(&vpss_config);
exit3:
    sample_comm_vgs_exit();
exit2:
    sample_comm_isp_stop(&isp_param);
exit1:
    sample_comm_vi_stop(&vi_config);
exit0:
    xmedia_mmz_unmap(splicing_para.splicing_info.vir_addr);
    xmedia_vb_release_block(splicing_para.splicing_info.vb_block);
    sample_comm_isp_exit(&isp_param);
    sample_venc_sys_exit();

    return ret;
}


/****  User send yuv + venc only  ****/
xmedia_s32 sample_venc_only(xmedia_video_size res, xmedia_payload_type payload_type, FILE *file_input)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 blk_size = 0;
    xmedia_u32 framerate = 30;
    xmedia_video_size pic_size = { 0 };
    sample_sys_config sys_config = { 0 };
    xmedia_s32 venc_chnl = 0;
    xmedia_s32 venc_chn_cnt = 1;
    sample_venc_config venc_config = { 0 };
    sample_venc_yuv_thread_para yuv_thread_para = { 0 };
    sample_rc rc_mode;
    xmedia_venc_gop_mode gop_mode;
    xmedia_vb_pool_config user_vb_pool;

    rc_mode = sample_venc_get_rc_mode();
    gop_mode = sample_venc_get_gop_mode();

    // init yuv_thread_para
    yuv_thread_para.user_width = res.width;
    yuv_thread_para.user_height = res.height;
    yuv_thread_para.user_video_format = XMEDIA_VIDEO_FMT_LINEAR;
    yuv_thread_para.user_pixel_format = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    yuv_thread_para.user_compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;
    yuv_thread_para.max_send_yuv_cnt = 300;
    yuv_thread_para.yuv_num = 1;
    yuv_thread_para.user_send_yuv_delay_ms = 0;
    yuv_thread_para.input_yuv[0] = file_input;

    // sys init
    sys_config.vb_conf.supplement_config = 1;
    ret = sample_comm_sys_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sys init failed !\n");
        return ret;
    }

    //venc init
    ret = sample_comm_venc_init();
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_venc_init failed!\n");
        goto exit0;
    }

    g_user_start = XMEDIA_TRUE;
    venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
    venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
    venc_config.chn_info[venc_chnl].payload_type = payload_type;
    venc_config.chn_info[venc_chnl].rc_mode = sample_venc_set_rc_mode(rc_mode, venc_config.chn_info[venc_chnl].payload_type);
    venc_config.chn_info[venc_chnl].venc_gop_attr.gop_mode = gop_mode;
    sample_comm_venc_get_default_chn_info(res, framerate, &venc_config.chn_info[venc_chnl]);


    pic_size.width = ((res.width + 15) >> 4) << 4;
    pic_size.height = ((res.height + 15) >> 4) << 4;
    blk_size = sample_comm_sys_get_buffer_size(pic_size, yuv_thread_para.user_video_format, yuv_thread_para.user_pixel_format,
                                               XMEDIA_VIDEO_DATA_WIDTH_8, yuv_thread_para.user_compress_mode);
    memset(&user_vb_pool, 0, sizeof(xmedia_vb_pool_config));
    user_vb_pool.block_size = blk_size;
    user_vb_pool.block_cnt  = 3;
    user_vb_pool.map_mode   = XMEDIA_VB_MAP_MODE_NONE;
    yuv_thread_para.pool = xmedia_vb_create_pool(&user_vb_pool);

    printf("%s,%d, creat vb pool size:%d, cnt:%d\n", __FUNCTION__, __LINE__, blk_size, user_vb_pool.block_cnt);
    XMEDIA_ASSERT(yuv_thread_para.pool != VB_INVALID_POOLID);


    // create venc chnl
    ret = sample_comm_venc_create_chn(&venc_config.chn_info[venc_chnl]);
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_venc_init failed!\n");
        goto exit2;
    }
    if (payload_type == PT_H264) {
        yuv_thread_para.h264e_cnt = 1;
        yuv_thread_para.h264e_chnl[0] = venc_chnl;
    } if (payload_type == PT_H265) {
        yuv_thread_para.h265e_cnt = 1;
        yuv_thread_para.h265e_chnl[0] = venc_chnl;
    }else if (payload_type == PT_MJPEG) {
        yuv_thread_para.mjpeg_cnt = 1;
        yuv_thread_para.mjpeg_chnl[0] = venc_chnl;
    } else if (payload_type == PT_JPEG) {
        yuv_thread_para.jpege_cnt = 1;
        yuv_thread_para.jpege_chnl[0] = venc_chnl;
    }

    // start yuv input
    ret = pthread_create(&(yuv_thread_para.user_send_yuv_thread), 0, sample_comm_venc_user_send_thread, (xmedia_void *)&yuv_thread_para);
    if (ret) {
        printf("%s,%d,pthread_create err!!!!\n",__FUNCTION__,__LINE__);
        goto exit2;
    }

    if (payload_type == PT_JPEG) {
        yuv_thread_para.max_send_yuv_cnt = 30; // for jpeg only send 30 frame
    }
    sample_comm_venc_start_chn(venc_chnl);
    ret = sample_comm_venc_start_get_stream(&venc_chnl, venc_chn_cnt, XMEDIA_TRUE, XMEDIA_TRUE, ".");
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start get stream failed !\n");
        goto exit3;
    }
    PAUSE(g_force_exit);
    (xmedia_void)sample_comm_venc_stop_get_stream();
    g_user_start = XMEDIA_FALSE;
exit3:
    pthread_join(yuv_thread_para.user_send_yuv_thread, 0);
exit2:
    sample_comm_venc_stop(&venc_config);
    xmedia_vb_destroy_pool(yuv_thread_para.pool);
exit0:
    sample_venc_sys_exit();

    return ret;
}

int main(int argc,char **argv)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 index = 0;
    xmedia_bool set_roi = XMEDIA_FALSE;
    xmedia_video_size res;
    xmedia_payload_type payload_type = PT_MAX;

    signal(SIGINT, sample_venc_handle_sig);
    signal(SIGTERM, sample_venc_handle_sig);

    if (argc < 2) {
        sample_venc_usage(argv[0]);
        return 0;
    }

    index = atoi(argv[1]);//atoi(stdlib.h)-将字符串转换为整数
    if (argc == 2) {
        switch (index) {
            case 0:
                ret = sample_venc_ring();
                break;
            case 1:
                ret = sample_venc_h265_h264(set_roi);
                break;
            case 2:
                ret = sample_venc_qpmap();
                break;
            case 3:
                set_roi = XMEDIA_TRUE;
                ret = sample_venc_h265_h264(set_roi);
                break;
            case 4:
                ret = sample_venc_jpeg();
                break;
            case 5:
                ret = sample_venc_jpeg_splicing();
                break;
            default:
                sample_venc_usage(argv[0]);
                return 0;
        }
    } else if (argc == 5) {
            FILE *file_input = fopen(argv[1], "rb");
            if (file_input == NULL)
            {
                printf("ERROR:open file :%s failed!!\n", argv[1]);
                return 0;
            }
            res.width  = atoi(argv[2]);
            res.height = atoi(argv[3]);
            if (!(strcmp("h264", argv[4]) && strcmp("H264", argv[4]) && strcmp("264", argv[4]))) {
                payload_type = PT_H264;
            } else if (!(strcmp("h265", argv[4]) && strcmp("H265", argv[4]) && strcmp("265", argv[4]))) {
                payload_type = PT_H265;
            } else if (!(strcmp("jpeg", argv[4]) && strcmp("jpg", argv[4]))) {
                payload_type = PT_JPEG;
            } else if (!(strcmp("mjpg", argv[4]) && strcmp("mjpeg", argv[4]))) {
                payload_type = PT_MJPEG;
            }
            ret = sample_venc_only(res, payload_type, file_input);
            fclose(file_input);
    }

    if (XMEDIA_SUCCESS == ret) {
        SAMPLE_PRT("program exit normally!\n");
    } else {
        SAMPLE_PRT("program exit abnormally!\n");
    }

    return 0;
}



