/*
 * Copyright (c) XCAM. All rights reserved.
 */

#ifndef _XCAM_LIVESERVER_H_
#define _XCAM_LIVESERVER_H_

#include "xcam_define.h"
#include "xcam_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* name pattern that used in webserver distrib link */
#define XCAM_LIVESERVER_CONN_CNT_MIN  1
#define XCAM_LIVESERVER_CONN_CNT_MAX  32

enum {
    ERR_CODE_INVALID_PARAM = 0x10,
    ERR_CODE_NOT_INIT = 0x11,
    ERR_CODE_WAS_INITED = 0x12,
    ERR_CODE_ALREADY_EXISTED = 0x13,
    ERR_CODE_NOT_FIND = 0x14,
    ERR_CODE_NO_FREE_RESOURCE = 0x15,
    ERR_CODE_BUTT
};

/* general error code */
#define XCAM_LIVESERVER_ERR_ILLEGAL_PARAM  XCAM_BUILD_ERRNO(XCAM_MOD_LIVESERVER, ERR_CODE_INVALID_PARAM)
#define XCAM_LIVESERVER_ERR_NOT_INIT  XCAM_BUILD_ERRNO(XCAM_MOD_LIVESERVER, ERR_CODE_NOT_INIT)
#define XCAM_LIVESERVER_ERR_WAS_INITED  XCAM_BUILD_ERRNO(XCAM_MOD_LIVESERVER, ERR_CODE_WAS_INITED)
#define XCAM_LIVESERVER_ERR_EXISTED  XCAM_BUILD_ERRNO(XCAM_MOD_LIVESERVER, ERR_CODE_ALREADY_EXISTED)
#define XCAM_LIVESERVER_ERR_NOT_FIND  XCAM_BUILD_ERRNO(XCAM_MOD_LIVESERVER, ERR_CODE_NOT_FIND)
#define XCAM_LIVESERVER_ERR_NO_FREE_RESOURCE  XCAM_BUILD_ERRNO(XCAM_MOD_LIVESERVER, ERR_CODE_NO_FREE_RESOURCE)


#define LOG_LIVESERVER_INFO(fmt...)  XCAM_LOG_INFO_PRINT("liveserver", fmt);
#define LOG_LIVESERVER_WARN(fmt...)  XCAM_LOG_WARN_PRINT("liveserver", fmt);
#define LOG_LIVESERVER_ERROR(fmt...) XCAM_LOG_ERR_PRINT("liveserver", fmt);
#define LOG_LIVESERVER_FATAL(fmt...) XCAM_LOG_FATAL_PRINT("liveserver", fmt);


#define PATTERN_NAME_LEN 16

typedef struct {
    xcam_s32 max_conn_num;
    xcam_u16 listen_port;
    xcam_char pattern_name[PATTERN_NAME_LEN];
} xcam_liveserver_init_param;

xcam_s32 xcam_liveserver_add_stream(xcam_handle venc_handle, xcam_track_video_source_info *video_src_info,
            xcam_handle aenc_handle, xcam_track_audio_source_info *audio_src_info,
            const xcam_char* stream_name,
            func_xcam_track_source_request_key_frame func_request_key_frame);

xcam_s32 xcam_liveserver_remove_stream(const xcam_char* stream_name);

xcam_s32 xcam_liveserver_remove_all_stream(xcam_void);

xcam_bool xcam_liveserver_is_init(xcam_void);

xcam_s32 xcam_liveserver_init(const xcam_liveserver_init_param param);

xcam_s32 xcam_liveserver_deinit(xcam_void);

xcam_s32 xcam_liveserver_send_video_data(xcam_handle venc_handle, xcam_livestream_rtsp_data *rtsp_data);

xcam_s32 xcam_liveserver_send_audio_data(xcam_handle aenc_handle, xcam_livestream_rtsp_data *rtsp_data);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* _XCAM_LIVESERVER_H_ */

