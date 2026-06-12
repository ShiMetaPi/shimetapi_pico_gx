/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "video_stream.h"

#include "sample_video.h"
#include "sample_video_yuv.h"
#include "sample_video_mjpeg.h"
#include "sample_video_h264.h"
#include "sample_video_mjpeg_h264.h"

#include "venc_log.h"

#include "frame_cache.h"

#define XMEDIA_ACCESS_ENABLE 1

#define EMBEDDING_H24_IN_MJPEG 0

#if !XMEDIA_ACCESS_ENABLE
#define STREAM_FILE_PATH "./stream.bin"
#endif

#define v4l2_fourcc(a, b, c, d) \
    ((uint32_t)(a) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

#define V4L2_PIX_FMT_YUYV v4l2_fourcc('Y', 'U', 'Y', 'V')   /* 16  YUV 4:2:2     */
#define V4L2_PIX_FMT_NV21 v4l2_fourcc('N', 'V', '2', '1')   /* 12  Y/CrCb 4:2:0  */
#define V4L2_PIX_FMT_NV12 v4l2_fourcc('N', 'V', '1', '2')   /* 12  Y/CbCr 4:2:0  */
#define V4L2_PIX_FMT_YUV420 v4l2_fourcc('Y', 'U', '1', '2') /* 12  YUV 4:2:0     */
#define V4L2_PIX_FMT_YVU420 v4l2_fourcc('Y', 'V', '1', '2') /* 12  YUV 4:2:0     */
#define V4L2_PIX_FMT_MJPEG v4l2_fourcc('M', 'J', 'P', 'G')  /* Motion-JPEG   */
#define V4L2_PIX_FMT_H264 v4l2_fourcc('H', '2', '6', '4')   /* H264 with start codes */
#define V4L2_PIX_FMT_H265 v4l2_fourcc('H', '2', '6', '5')   /* H265 with start codes */

static int g_start = 0;
static struct encoder_property g_encoder_property;

static xmedia_payload_type format_v4l2_to_mpp(uint32_t fcc)
{
    xmedia_payload_type t;

    switch (fcc) {
        case V4L2_PIX_FMT_YVU420:
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_NV21:
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_MJPEG:
            t = PT_MJPEG;
            break;

        case V4L2_PIX_FMT_H264:
            t = PT_H264;
            break;

        case V4L2_PIX_FMT_H265:
            t = PT_H265;
            break;

        default:
            t = PT_MJPEG;
            break;
    }

    return t;
}

static void pic_format_convert(xmedia_payload_type *format)
{
    *format = format_v4l2_to_mpp(g_encoder_property.format);
}

static void pic_size_convert(xmedia_video_size *wh)
{
    wh->width = g_encoder_property.width;
    wh->height = g_encoder_property.height;
}

void get_user_format(xmedia_payload_type *format, xmedia_video_size *wh, unsigned int *framerate)
{
    if (wh != NULL) {
        pic_size_convert(wh);
    }

    if (format != NULL) {
        pic_format_convert(format);
    }

    if (framerate != NULL) {
        *framerate = g_encoder_property.fps;
    }
}

/*******************************************************
 * Processing Unit Operation Functions
 *******************************************************/

/* Processing Unit Operation Functions End */

/*******************************************************
 * Input Terminal Operation Functions
 *******************************************************/

/* Input Terminal Operation Functions End */

/*******************************************************
 * Stream Control Operation Functions
 *******************************************************/
static int sample_video_stream_set_idr(void)
{
    return 0;
}

static int sample_video_stream_init(void)
{
    return 0;
}

static int sample_video_stream_deinit(void)
{
    return 0;
}

#if !XMEDIA_ACCESS_ENABLE
static int g_stop = 0;
static pthread_t send_pid = -1;
static void* send_process(void* args)
{
    FILE *fp = fopen(STREAM_FILE_PATH, "r");
    uvc_cache_t *uvc_cache = NULL;
    frame_node_t *fnode = NULL;

    g_stop = 0;
    while (!g_stop && fp != NULL) {
        uvc_cache = uvc_cache_get();
        if (uvc_cache) {
            get_node_from_queue(uvc_cache->free_queue, &fnode);
        }

        if (!fnode) {
            usleep(100000);
        } else {
            fseek(fp, 0L, SEEK_SET);
            fnode->used = fread((char*)fnode->mem, 1, fnode->length, fp);
            venc_logi("send %d\n", fnode->used);
            put_node_to_queue(uvc_cache->ok_queue, fnode);
        }
    }
    fclose(fp);

    return NULL;
}

static void start_send_file(int fcc)
{
    pthread_create(&send_pid, NULL, &send_process, NULL);
}

static void stop_send_file(int fcc)
{
    g_stop = 1;

    if (send_pid != -1) {
        pthread_join(send_pid, NULL);
    }
}
#endif

static int sample_video_stream_startup(void)
{
    int fcc = g_encoder_property.format;

    venc_logi("sample stream startup.\n");
    g_start = 1;

#if !XMEDIA_ACCESS_ENABLE
    start_send_file(fcc);
#else
    switch (fcc) {
        case V4L2_PIX_FMT_MJPEG:
#if !EMBEDDING_H24_IN_MJPEG
            sample_mjpeg_init();
            sample_mjpeg_startup();
#else
            sample_mjpeg_h264_init();
            sample_mjpeg_h264_startup();
#endif
            break;

        case V4L2_PIX_FMT_H264:
        case V4L2_PIX_FMT_H265:
            sample_h264_init();
            sample_h264_startup();
            break;

        case V4L2_PIX_FMT_NV21:
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_YVU420:
            sample_yuv_init();
            sample_yuv_startup(fcc);
            break;

        default:
            venc_loge("Unsupported stream type.\n");
            return -1;
    }
#endif

    venc_logi("sample stream startup ok.\n");
    return 0;
}

static int sample_video_stream_shutdown(void)
{
    int fcc = g_encoder_property.format;

    if (!g_start) {
        return 0;
    }

#if !XMEDIA_ACCESS_ENABLE
    stop_send_file(fcc);
#else
    switch (fcc) {
        case V4L2_PIX_FMT_MJPEG:
#if !EMBEDDING_H24_IN_MJPEG
            sample_mjpeg_shutdown();
#else
            sample_mjpeg_h264_shutdown();
#endif
            break;

        case V4L2_PIX_FMT_H264:
        case V4L2_PIX_FMT_H265:
            sample_h264_shutdown();
            break;

        case V4L2_PIX_FMT_NV21:
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_YVU420:
            sample_yuv_shutdown();
            break;

        default:
            venc_loge("Unsupported stream type.\n");
            return -1;
    }
#endif
    g_start = 0;

    return 0;
}

static int sample_video_stream_set_property(struct encoder_property *p)
{
    g_encoder_property = *p;

    return 0;
}

/* Stream Control Operation Functions End */

static struct stream_control_ops venc_sc_ops = {
    .init = &sample_video_stream_init,
    .deinit = &sample_video_stream_deinit,
    .startup = &sample_video_stream_startup,
    .shutdown = &sample_video_stream_shutdown,
    .set_idr = &sample_video_stream_set_idr,
    .set_property = &sample_video_stream_set_property,
};

static struct processing_unit_ops venc_pu_ops = {
    /* get */
    .brightness_get = NULL,
    .contrast_get = NULL,
    .hue_get = NULL,
    .power_line_frequency_get = NULL,
    .saturation_get = NULL,
    .white_balance_temperature_auto_get = NULL,
    .white_balance_temperature_get = NULL,
    /* set */
    .brightness_set = NULL,
    .contrast_set = NULL,
    .hue_set = NULL,
    .power_line_frequency_set = NULL,
    .saturation_set = NULL,
    .white_balance_temperature_auto_set = NULL,
    .white_balance_temperature_set = NULL,
};

static struct input_terminal_ops venc_it_ops = {
    /* get */
    .exposure_ansolute_time_get = NULL,
    .exposure_auto_mode_get = NULL,

    /* set */
    .exposure_ansolute_time_set = NULL,
    .exposure_auto_mode_set = NULL,
};

//static struct extension_unit_ops venc_xu_ops;

void video_stream_register(void)
{
    stream_register_mpi_ops(&venc_sc_ops, &venc_pu_ops, &venc_it_ops, NULL);
}

void video_stream_unregister(void)
{
    stream_register_mpi_ops(NULL, NULL, NULL, NULL);
}
