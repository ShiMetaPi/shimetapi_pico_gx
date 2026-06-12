/*
 * Copyright (c) XCAM. All rights reserved.
 */

#include <string.h>
#include <stdio.h>

#include "xcam_type.h"
#include "xcam_track_source.h"
#include "xcam_livestream_rtsp_server.h"
#include "xcam_liveserver.h"

#define LIVESVR_RTSP_PACKET_LEN      1500  /* packet length, value[500,5000], recommend 1500, unit:byte */
#define LIVESVR_RTSP_DEF_LISTEN_PORT 554   /* value[1,65535], normal 554 */
#define LIVESVR_RTSP_MAX_PAYLOAD_CNT 8     /* max payload num in mbuffer, value[1,254] */
#define LIVESVR_RTSP_TIMEOUT_SEC     -1     /* <0: not use timeout, unit:s */
#define LIVESVR_RTSP_MAX_STREAM_CNT  4
#define LIVESVR_VIDEO_MBUFFER_FRAME  5
#define LIVESVR_PATH_MAX             64
#define LIVESVR_AUDIO_MBUFFER_SIZE   (16 << 10)  /* 16K */
#define RTSP_MIN_MBUFFER_SIZE        (216 << 10) /* 216K */
#define RTSP_MAX_MBUFFER_SIZE        (3 << 20)  /* 3M */
#define LIVESVR_DEF_PATTERN_NAME    "livestream"

#define INVALID_TRACK    (-1)

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif


typedef struct {
    xcam_bool is_used;
    xcam_char stream_name[LIVESVR_PATH_MAX];
    xcam_track_source track_video_src;
    xcam_track_source track_audio_src;
} xcam_liveserver_rtsp_stream_ctx;

typedef struct {
    xcam_bool is_initialized;
    xcam_void* rtspserv_obj;
    xcam_liveserver_rtsp_stream_ctx rtsp_stream_ctx[LIVESVR_RTSP_MAX_STREAM_CNT];
    xcam_char pattern_path[PATTERN_NAME_LEN];
} xcam_liveserver_context;

static xcam_liveserver_context g_xcam_liveserver_ctx;

static xcam_void client_connect_callback(xcam_server_state_listener* listener, xcam_char* ipaddr)
{
    if (listener == XCAM_NULL) {
        LOG_LIVESERVER_ERROR("Null pointer [listener]!\n");
    }
    if (ipaddr == XCAM_NULL) {
        LOG_LIVESERVER_ERROR("Null pointer [ipaddr]!\n");
        return;
    }

    LOG_LIVESERVER_INFO("client connect, ipaddr(%s).\n", ipaddr);
    return;
}

static xcam_void client_disconnect_callback(const xcam_server_state_listener* listener, const xcam_char* ipaddr)
{
    if (listener == XCAM_NULL) {
        LOG_LIVESERVER_ERROR("Null pointer [listener]!\n");
    }
    if (ipaddr == XCAM_NULL) {
        LOG_LIVESERVER_ERROR("Null pointer [ipaddr]!\n");
        return;
    }

    LOG_LIVESERVER_INFO("client disconnect, ipaddr(%s)!\n", ipaddr);
    return;
}

static xcam_void server_error_callback(xcam_server_state_listener* listener, xcam_s32 err_id, xcam_char* err_msg)
{
    if (listener == XCAM_NULL) {
        LOG_LIVESERVER_ERROR("Null pointer [listener]!\n");
    }
    if (err_msg == XCAM_NULL) {
        LOG_LIVESERVER_ERROR("Null pointer [err_msg]!\n");
        return;
    }

    LOG_LIVESERVER_INFO("server error, err_id(%d) err_msg(%s)!\n", err_id, err_msg);
    return;
}

static xcam_s32 init_rtsp_server(const xcam_s32 max_conn_num, const xcam_u16 lsn_port)
{
    xcam_server_state_listener listener;
    xcam_livestream_rtsp_config rtsp_cfg;
    xcam_s32 ret;

    rtsp_cfg.listen_port = (xcam_s32)lsn_port;
    rtsp_cfg.max_conn_num = max_conn_num;
    rtsp_cfg.max_payload = LIVESVR_RTSP_MAX_PAYLOAD_CNT;
    rtsp_cfg.packet_len = LIVESVR_RTSP_PACKET_LEN;
    rtsp_cfg.timeout = LIVESVR_RTSP_TIMEOUT_SEC;
    ret = xcam_livestream_rtsp_server_create(&g_xcam_liveserver_ctx.rtspserv_obj, &rtsp_cfg);
    if (ret != XCAM_SUCCESS) {
        LOG_LIVESERVER_ERROR("Create rtsp server failed, ret:0x%x!\n", ret);
        return XCAM_FAILURE;
    }

    listener.func_client_connect = client_connect_callback;
    listener.func_client_disconnect = client_disconnect_callback;
    listener.func_server_error = server_error_callback;
    ret = xcam_livestream_rtsp_server_set_listener(g_xcam_liveserver_ctx.rtspserv_obj, &listener);
    if (ret != XCAM_SUCCESS) {
        LOG_LIVESERVER_ERROR("Set rtsp server listener failed, ret:0x%x!\n", ret);
        (xcam_void)xcam_livestream_rtsp_server_destroy(g_xcam_liveserver_ctx.rtspserv_obj);
        return XCAM_FAILURE;
    }

    LOG_LIVESERVER_INFO("Start rtsp server, listen port[%u] max conn count[%d]!\n", 
        lsn_port, max_conn_num);

    ret = xcam_livestream_rtsp_server_start(g_xcam_liveserver_ctx.rtspserv_obj);
    if (ret != XCAM_SUCCESS) {
        LOG_LIVESERVER_ERROR("Start rtsp server failed, ret:0x%x!\n", ret);
        (xcam_void)xcam_livestream_rtsp_server_destroy(g_xcam_liveserver_ctx.rtspserv_obj);
        return XCAM_FAILURE;
    }

    return XCAM_SUCCESS;
}

static xcam_void deinit_rtsp_server(xcam_void)
{
    xcam_s32 ret;

    ret = xcam_livestream_rtsp_server_stop(g_xcam_liveserver_ctx.rtspserv_obj);
    if (ret != XCAM_SUCCESS) {
        LOG_LIVESERVER_ERROR("rtsp server stop failed, ret:0x%X!\n", ret);
    }
    ret = xcam_livestream_rtsp_server_destroy(g_xcam_liveserver_ctx.rtspserv_obj);
    if (ret != XCAM_SUCCESS) {
        LOG_LIVESERVER_ERROR("rtsp server destroy failed, ret:0x%X!\n", ret);
    }
}

static xcam_s32 get_available_stream_context(const xcam_char* stream_name, xcam_liveserver_rtsp_stream_ctx** stream_ctx)
{
    xcam_u32 idx;
    for (idx = 0; idx < LIVESVR_RTSP_MAX_STREAM_CNT; ++idx) {
        if (g_xcam_liveserver_ctx.rtsp_stream_ctx[idx].is_used) {
            if (strncmp(g_xcam_liveserver_ctx.rtsp_stream_ctx[idx].stream_name, stream_name, LIVESVR_PATH_MAX) == 0) {
                LOG_LIVESERVER_ERROR("stream_name[%s] already exists!\n", stream_name);
                return XCAM_LIVESERVER_ERR_EXISTED;
            }
        }
    }

    for (idx = 0; idx < LIVESVR_RTSP_MAX_STREAM_CNT; ++idx) {
        if (!g_xcam_liveserver_ctx.rtsp_stream_ctx[idx].is_used) {
            *stream_ctx = &g_xcam_liveserver_ctx.rtsp_stream_ctx[idx];
            break;
        }
    }

    if (*stream_ctx == XCAM_NULL) {
        LOG_LIVESERVER_ERROR("Beyond max stream count!\n");
        return XCAM_LIVESERVER_ERR_NO_FREE_RESOURCE;
    }

    return XCAM_SUCCESS;
}

static xcam_s32 get_rtsp_stream_context(xcam_handle venc_handle, xcam_track_video_source_info *video_src_info,
                    xcam_handle aenc_handle, xcam_track_audio_source_info *audio_src_info,
                    const xcam_char* stream_name, xcam_liveserver_rtsp_stream_ctx* stream_ctx)
{
    xcam_s32 ret;

    if (venc_handle != (xcam_handle)INVALID_TRACK) {
        memcpy(&stream_ctx->track_video_src.track_source_attr.video_info, video_src_info, 
            sizeof(xcam_track_video_source_info));
        stream_ctx->track_video_src.private_handle = venc_handle;
        stream_ctx->track_video_src.track_type = XCAM_TRACK_SOURCE_TYPE_VIDEO;
    } else {
        stream_ctx->track_video_src.private_handle = INVALID_TRACK;
    }

    if (aenc_handle != (xcam_handle)INVALID_TRACK) {
        memcpy(&stream_ctx->track_audio_src.track_source_attr.audio_info, audio_src_info,
            sizeof(xcam_track_audio_source_info));
        stream_ctx->track_audio_src.private_handle = aenc_handle;
        stream_ctx->track_audio_src.track_type = XCAM_TRACK_SOURCE_TYPE_AUDIO;
    } else {
        stream_ctx->track_audio_src.private_handle = INVALID_TRACK;
    }

    ret = snprintf(stream_ctx->stream_name, LIVESVR_PATH_MAX - 1, "%s", stream_name);
    if (ret < 0) {
        LOG_LIVESERVER_ERROR("snprintf failed, ret:0x%x!\n", ret);
        return XCAM_FAILURE;
    }

    return XCAM_SUCCESS;
}

static xcam_void get_rtsp_source(xcam_liveserver_rtsp_stream_ctx* stream_ctx,
                    xcam_livestream_rtsp_source* rtsp_src)
{
    if (stream_ctx->track_video_src.private_handle != INVALID_TRACK) {
        rtsp_src->video_src_handle = &stream_ctx->track_video_src;
    } else {
        rtsp_src->video_src_handle = XCAM_NULL;
    }

    if (stream_ctx->track_audio_src.private_handle != INVALID_TRACK) {
        rtsp_src->audio_src_handle = &stream_ctx->track_audio_src;
    } else {
        rtsp_src->audio_src_handle = XCAM_NULL;
    }
}

static xcam_void get_mbuffer_size(const xcam_liveserver_rtsp_stream_ctx* stream_ctx, xcam_u32* mbuf_size)
{
    *mbuf_size = RTSP_MIN_MBUFFER_SIZE;
    if (stream_ctx->track_video_src.private_handle != INVALID_TRACK) {
        *mbuf_size += (stream_ctx->track_video_src.track_source_attr.video_info.bit_rate /
            stream_ctx->track_video_src.track_source_attr.video_info.frame_rate * LIVESVR_VIDEO_MBUFFER_FRAME);
    }

    if (stream_ctx->track_audio_src.private_handle != INVALID_TRACK) {
        *mbuf_size += LIVESVR_AUDIO_MBUFFER_SIZE;
    }

    *mbuf_size = MAX(*mbuf_size, RTSP_MIN_MBUFFER_SIZE);
    *mbuf_size = MIN(*mbuf_size, RTSP_MAX_MBUFFER_SIZE);
}

xcam_s32 xcam_liveserver_send_video_data(xcam_handle venc_handle, xcam_livestream_rtsp_data *rtsp_data)
{
    if (rtsp_data == XCAM_NULL) {
        LOG_LIVESERVER_ERROR("NuLL pointer [aenc_data]\n");
        return XCAM_LIVESERVER_ERR_ILLEGAL_PARAM;
    }

    xcam_track_source_handle track_video = &(g_xcam_liveserver_ctx.rtsp_stream_ctx[venc_handle].track_video_src);
    xcam_s32 ret;

    ret = xcam_livestream_rtsp_server_write_frame(g_xcam_liveserver_ctx.rtspserv_obj,
                (xcam_track_source_handle)track_video, rtsp_data);
    return ret;
}

xcam_s32 xcam_liveserver_send_audio_data(xcam_handle aenc_handle, xcam_livestream_rtsp_data *rtsp_data)
{
    if (rtsp_data == XCAM_NULL) {
        LOG_LIVESERVER_ERROR("NuLL pointer [aenc_data]\n");
        return XCAM_LIVESERVER_ERR_ILLEGAL_PARAM;
    }

    xcam_track_source_handle track_audio = &(g_xcam_liveserver_ctx.rtsp_stream_ctx[aenc_handle].track_audio_src);
    xcam_s32 ret;
    
    ret = xcam_livestream_rtsp_server_write_frame(g_xcam_liveserver_ctx.rtspserv_obj,
                (xcam_track_source_handle)track_audio, rtsp_data);

    return ret;
}

// xcam_livestream_rtsp_data rtsp_data;
// xcam_u32 offset = 0;
// rtsp_data.data_ptr[0] = aenc_data->pStream + offset;
// rtsp_data.data_len[0] = aenc_data->u32Len - offset;
// rtsp_data.pts_us = aenc_data->u64TimeStamp;
// rtsp_data.seq_num = aenc_data->u32Seq;
// rtsp_data.block_cnt = 1;
// rtsp_data.is_key_frame = XCAM_FALSE;


xcam_s32 xcam_liveserver_add_stream(xcam_handle venc_handle, xcam_track_video_source_info *video_src_info,
            xcam_handle aenc_handle, xcam_track_audio_source_info *audio_src_info,
            const xcam_char* stream_name,
            func_xcam_track_source_request_key_frame func_request_key_frame)
{
    xcam_liveserver_rtsp_stream_ctx* stream_ctx = XCAM_NULL;
    xcam_char stream_path[LIVESVR_PATH_MAX];
    xcam_livestream_rtsp_source rtsp_src;
    xcam_u32 mbuf_size;
    xcam_s32 ret;

    if (!g_xcam_liveserver_ctx.is_initialized) {
        LOG_LIVESERVER_ERROR("liveserver is not inited yet!\n");
        return XCAM_LIVESERVER_ERR_NOT_INIT;
    }
    if (stream_name == XCAM_NULL) {
        LOG_LIVESERVER_ERROR("NuLL pointer [stream_name]!\n");
        return XCAM_LIVESERVER_ERR_ILLEGAL_PARAM;
    }
    if ((venc_handle == (xcam_handle)INVALID_TRACK) 
      && (aenc_handle == (xcam_handle)INVALID_TRACK)) {
        LOG_LIVESERVER_ERROR("venc_handle[%d] aenc_handle[%d] is invalid!\n", venc_handle, aenc_handle);
        return XCAM_LIVESERVER_ERR_ILLEGAL_PARAM;
    }

    if (strlen(stream_name) == 0) {
        LOG_LIVESERVER_ERROR("stream_name is null!\n");
        return XCAM_LIVESERVER_ERR_ILLEGAL_PARAM;
    }

    ret = get_available_stream_context(stream_name, &stream_ctx);
    if (ret != XCAM_SUCCESS) {
        LOG_LIVESERVER_ERROR("Get one available stream context failed, ret:0x%x!\n", ret);
        return ret;
    }

    memset(&stream_ctx->track_video_src, 0x0, sizeof(xcam_track_source));
    memset(&stream_ctx->track_audio_src, 0x0, sizeof(xcam_track_source));
    ret = get_rtsp_stream_context(venc_handle, video_src_info, aenc_handle, audio_src_info, stream_name, stream_ctx);
    if (ret != XCAM_SUCCESS) {
        LOG_LIVESERVER_ERROR("Get specific stream context failed, ret:0x%x!\n", ret);
        return ret;
    }

    if (venc_handle != (xcam_handle)INVALID_TRACK && func_request_key_frame) {
        stream_ctx->track_video_src.func_request_key_frame = func_request_key_frame;
    }

    get_rtsp_source(stream_ctx, &rtsp_src);
    get_mbuffer_size(stream_ctx, &mbuf_size);
    ret = snprintf(stream_path, LIVESVR_PATH_MAX - 1, "%s/%s", g_xcam_liveserver_ctx.pattern_path, stream_name);
    if (ret < 0) {
        LOG_LIVESERVER_ERROR("snprintf failed, ret:0x%x!\n", ret);
        return XCAM_FAILURE;
    }

    LOG_LIVESERVER_INFO("Add stream_path[%s] venc_hdl[%d] aenc_hdl[%d] mbuf_size[%u byte]!\n",
        stream_path, venc_handle, aenc_handle, mbuf_size);

    ret = xcam_livestream_rtsp_server_add_media_stream(g_xcam_liveserver_ctx.rtspserv_obj, &rtsp_src, stream_path, mbuf_size);
    if (ret != XCAM_SUCCESS) {
        LOG_LIVESERVER_ERROR("Add media stream failed, ret:0x%x!\n", ret);
        return XCAM_FAILURE;
    }

    stream_ctx->is_used = XCAM_TRUE;

    return XCAM_SUCCESS;
}

xcam_s32 xcam_liveserver_remove_stream(const xcam_char* stream_name)
{
    xcam_s32 ret;
    xcam_s32 idx;

    if (g_xcam_liveserver_ctx.is_initialized == XCAM_FALSE) {
        LOG_LIVESERVER_ERROR("liveserver is not init yet!\n");
        return XCAM_LIVESERVER_ERR_NOT_INIT;
    }
    if (stream_name == XCAM_NULL) {
        LOG_LIVESERVER_ERROR("NuLL pointer [stream_name]!\n");
        return XCAM_LIVESERVER_ERR_ILLEGAL_PARAM;
    }
    if (strlen(stream_name) == 0) {
        LOG_LIVESERVER_ERROR("valid length of stream_name is 0!\n");
        return XCAM_LIVESERVER_ERR_ILLEGAL_PARAM;
    }

    for (idx = 0; idx < LIVESVR_RTSP_MAX_STREAM_CNT; ++idx) {
        if ( (g_xcam_liveserver_ctx.rtsp_stream_ctx[idx].is_used == XCAM_TRUE)
            && (strncmp(g_xcam_liveserver_ctx.rtsp_stream_ctx[idx].stream_name, 
                    stream_name, LIVESVR_PATH_MAX) == 0) ) {
            xcam_char stream_path[LIVESVR_PATH_MAX];
            memset(stream_path, '\0', sizeof(stream_path));
            ret = snprintf(stream_path, LIVESVR_PATH_MAX - 1, "%s/%s", g_xcam_liveserver_ctx.pattern_path, stream_name);
            if (ret < 0) {
                LOG_LIVESERVER_ERROR("snprintf failed, ret:0x%x.\n", ret);
                return XCAM_FAILURE;
            }

            ret = xcam_livestream_rtsp_server_remove_media_stream(g_xcam_liveserver_ctx.rtspserv_obj, (const xcam_char*)stream_path);
            if (ret != XCAM_SUCCESS) {
                LOG_LIVESERVER_ERROR("Remove media stream failed, ret:0x%x.\n", ret);
                return XCAM_FAILURE;
            }

            g_xcam_liveserver_ctx.rtsp_stream_ctx[idx].is_used = XCAM_FALSE;

            return XCAM_SUCCESS;
        }
    }

    LOG_LIVESERVER_ERROR("could not find stream[%s].\n", stream_name);
    return XCAM_LIVESERVER_ERR_NOT_FIND;
}

xcam_s32 xcam_liveserver_remove_all_stream(xcam_void)
{
    xcam_s32 ret;
    xcam_s32 idx;

    if (!g_xcam_liveserver_ctx.is_initialized) {
        LOG_LIVESERVER_ERROR("liveserver is not init yet!\n");
        return XCAM_LIVESERVER_ERR_NOT_INIT;
    }

    for (idx = 0; idx < LIVESVR_RTSP_MAX_STREAM_CNT; ++idx) {
        if (g_xcam_liveserver_ctx.rtsp_stream_ctx[idx].is_used == XCAM_TRUE) {
            xcam_char stream_path[LIVESVR_PATH_MAX];
            memset(stream_path, '\0', sizeof(stream_path));
            ret = snprintf(stream_path, LIVESVR_PATH_MAX - 1, "%s/%s",
                    g_xcam_liveserver_ctx.pattern_path,
                    g_xcam_liveserver_ctx.rtsp_stream_ctx[idx].stream_name);
            if (ret < 0) {
                LOG_LIVESERVER_ERROR("snprintf failed, ret:0x%x!\n", ret);
                continue;
            }

            ret = xcam_livestream_rtsp_server_remove_media_stream(g_xcam_liveserver_ctx.rtspserv_obj, stream_path);
            if (ret != XCAM_SUCCESS) {
                LOG_LIVESERVER_ERROR("remove media stream failed, ret:0x%x!\n", ret);
            }

            g_xcam_liveserver_ctx.rtsp_stream_ctx[idx].is_used = XCAM_FALSE;
        }
    }

    return XCAM_SUCCESS;
}

xcam_bool xcam_liveserver_is_init(xcam_void)
{
    return g_xcam_liveserver_ctx.is_initialized;
}

xcam_s32 xcam_liveserver_init(const xcam_liveserver_init_param param)
{
    if (g_xcam_liveserver_ctx.is_initialized) {
        LOG_LIVESERVER_ERROR("Liveserver had been initialized!\n");
        return XCAM_LIVESERVER_ERR_WAS_INITED;
    }
    if ((param.max_conn_num < XCAM_LIVESERVER_CONN_CNT_MIN)
       || (param.max_conn_num > XCAM_LIVESERVER_CONN_CNT_MAX)) {
        LOG_LIVESERVER_ERROR("max conn num[%d] is beyond range[%d, %d]!\n", param.max_conn_num,
            XCAM_LIVESERVER_CONN_CNT_MIN, XCAM_LIVESERVER_CONN_CNT_MAX);
        return XCAM_LIVESERVER_ERR_ILLEGAL_PARAM;
    }

    xcam_u16 serv_port = param.listen_port;
    if (serv_port == 0) {
        serv_port = LIVESVR_RTSP_DEF_LISTEN_PORT;
        LOG_LIVESERVER_INFO("Use default listen port(%u)!\n", serv_port);
    }

    memset(&g_xcam_liveserver_ctx, 0x0, sizeof(xcam_liveserver_context));
    xcam_s32 ret = init_rtsp_server(param.max_conn_num, serv_port);
    if (ret != XCAM_SUCCESS) {
        LOG_LIVESERVER_ERROR("Init rtsp server failed, ret:0x%x!\n", ret);
        return XCAM_FAILURE;
    }

    if (strlen(param.pattern_name) > 0) {
        strncpy(g_xcam_liveserver_ctx.pattern_path, param.pattern_name, PATTERN_NAME_LEN);
    } else {
        LOG_LIVESERVER_INFO("Use default pattern name(%s)!\n", LIVESVR_DEF_PATTERN_NAME);
        strncpy(g_xcam_liveserver_ctx.pattern_path, LIVESVR_DEF_PATTERN_NAME, PATTERN_NAME_LEN);
    }

    g_xcam_liveserver_ctx.is_initialized = XCAM_TRUE;
    return XCAM_SUCCESS;
}

xcam_s32 xcam_liveserver_deinit(xcam_void)
{
    xcam_s32 ret;

    if (!g_xcam_liveserver_ctx.is_initialized) {
        LOG_LIVESERVER_ERROR("liveserver is not init yet!\n");
        return XCAM_LIVESERVER_ERR_NOT_INIT;
    }
    ret = xcam_liveserver_remove_all_stream();
    if (ret != XCAM_SUCCESS) {
        LOG_LIVESERVER_ERROR("Remove all stream failed, ret:0x%x!\n", ret);
    }

    deinit_rtsp_server();
    g_xcam_liveserver_ctx.is_initialized = XCAM_FALSE;

    return XCAM_SUCCESS;
}
