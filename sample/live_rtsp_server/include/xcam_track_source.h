/*
 * Copyright (c) XCAM. All rights reserved.
 */

#ifndef _XCAM_TRACK_SOURCE_H_
#define _XCAM_TRACK_SOURCE_H_

#include "xcam_type.h"

typedef enum {
    XCAM_TRACK_SOURCE_TYPE_PRIV = 0,
    XCAM_TRACK_SOURCE_TYPE_VIDEO,
    XCAM_TRACK_SOURCE_TYPE_AUDIO,
    XCAM_TRACK_SOURCE_TYPE_BUTT
} xcam_track_source_type;

typedef enum {
    XCAM_TRACK_VIDEO_CODEC_H264 = 96,
    XCAM_TRACK_VIDEO_CODEC_H265 = 98,
    XCAM_TRACK_VIDEO_CODEC_MJPEG = 102,
    XCAM_TRACK_VIDEO_CODEC_BUTT
} xcam_track_video_codec;

typedef enum {
    XCAM_TRACK_AUDIO_CODEC_G711U = 0,   /* G.711 U */
    XCAM_TRACK_AUDIO_CODEC_G711A = 8,   /* G.711 A */
    XCAM_TRACK_AUDIO_CODEC_LPCM = 23,   /* LPCM see cam_common.h file from sdk */
    XCAM_TRACK_AUDIO_CODEC_G726 = 97,   /* G.726 */
    XCAM_TRACK_AUDIO_CODEC_AMR = 101,   /* AMR encoder format */
    XCAM_TRACK_AUDIO_CODEC_ADPCM = 104, /* ADPCM */
    XCAM_TRACK_AUDIO_CODEC_AAC = 105,
    XCAM_TRACK_AUDIO_CODEC_WAV = 108, /* WAV encoder */
    XCAM_TRACK_AUDIO_CODEC_MP3 = 109,
    XCAM_TRACK_AUDIO_CODEC_BUTT
} xcam_track_audio_codec;

typedef struct {
    xcam_track_video_codec codec_type;
    xcam_u32   width;
    xcam_u32   height;
    xcam_u32   bit_rate;
    xcam_float frame_rate;
    xcam_u32   gop;
    xcam_float speed;
} xcam_track_video_source_info;

typedef struct {
    xcam_track_audio_codec codec_type;
    xcam_u32 chn_cnt;
    xcam_u32 sample_rate;
    xcam_u32 avg_bytes_per_sec;
    xcam_u32 samples_per_frame;
    xcam_u16 sample_bit_width;
} xcam_track_audio_source_info;

typedef struct {
    xcam_u32   private_data;
    xcam_float frame_rate;
    xcam_u32   bytes_per_sec;
    xcam_bool  is_strict_sync;
} xcam_track_private_source_info;

typedef struct cam_track_source* xcam_track_source_handle;
typedef xcam_s32 (*func_xcam_track_source_request_key_frame)(xcam_handle venc_handle, xcam_void* param);

typedef struct cam_track_source {
    intptr_t private_handle;  // venc or aenc handle
    func_xcam_track_source_request_key_frame func_request_key_frame;

    xcam_track_source_type track_type;
    union {
        xcam_track_video_source_info video_info;  /** video track info */
        xcam_track_audio_source_info audio_info;  /** audio track info */
        xcam_track_private_source_info priv_info; /** private track info */
    } track_source_attr;
} xcam_track_source;

#endif // _XCAM_TRACK_SOURCE_H_
