/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#ifndef __CONFIG_LOAD_PARAMETER_H__
#define __CONFIG_LOAD_PARAMETER_H__

#include "xmedia_audio_common.h"
#include "xmedia_audio_ao.h"
#include "xmedia_audio_ai.h"

typedef struct USER_BASE_CONFIG
{
    audio_sample_rate work_samplerate;
    xmedia_u32 in_chn_cnt;
    xmedia_u32 out_chn_cnt;
    xmedia_u32 sample_per_frame;
    xmedia_u32 frame_nums;
    xmedia_u32 vqe_version;

    xmedia_s32 codec_input_vol;
    xmedia_s32 ai_vqe_vol;
    xmedia_s32 ao_vqe_vol;
} user_base_config;

xmedia_s32 load_base_parameter(xmedia_char* ini_name, user_base_config * base_config);
xmedia_s32 load_aivqe_parameter(xmedia_char* ini_name, ai_vqe_attr* vqe_config);
xmedia_s32 load_aovqe_parameter(xmedia_char* ini_name, ao_vqe_attr* vqe_config);
#endif
