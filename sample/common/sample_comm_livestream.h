/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __SAMPLE_COMM_LIVESTREAM_H__
#define __SAMPLE_COMM_LIVESTREAM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "sample_comm.h"
#include "sample_comm_sys.h"
#include "xcam_log.h"
#include "xcam_livestream_rtsp_server.h"
#include "xcam_liveserver.h"

xmedia_s32 app_liveserver_init(xmedia_void);

xmedia_void app_liveserver_deinit(xmedia_void);

xmedia_s32 app_liveserver_get_video_src_info(int index, xcam_track_video_source_info *video_src_info,
                                     xmedia_video_size *chn_size, xmedia_u32 frame_rate);

xmedia_s32 app_liveserver_video_chn_request_key_frame(xcam_handle venc_handle, xmedia_void* param);

xmedia_s32 app_liveserver_start(xmedia_video_size *chn_size,xmedia_u32 chn_cnt,xmedia_u32 frame_rate);

xmedia_s32 app_liveserver_stop(xmedia_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

