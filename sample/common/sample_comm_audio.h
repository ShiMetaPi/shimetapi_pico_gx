/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __SAMPLE_COMM_AUDIO_H__
#define __SAMPLE_COMM_AUDIO_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "xmedia_type.h"
#include "sample_comm.h"

#include "xmedia_audio_ai.h"
#include "xmedia_audio_ao.h"
#include "xmedia_audio_adec.h"
#include "xmedia_audio_aenc.h"
#include "xmedia_audio_vqe.h"
#include "xmedia_audio_codec.h"
#include "xmedia_audio_codec_voice.h"
#include "xmedia_audio_vqe_bcd.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    audio_sample_rate frm_rate;
    audio_channel frm_chn;
    audio_bit_depth frm_bit;
} sample_ao_frame;

typedef struct {
    ai_dev dev;
    ai_dev_attr dev_attr;
    ai_user_chn_attr chn_attr;
    xmedia_bool res_enable;
    xmedia_bool vqe_enable;
    ai_vqe_attr vqe_attr;
    ai_ec_source source;
    xmedia_u32 ec_chn_id;
} sample_ai_config;

typedef struct {
    ao_dev dev;
    ao_dev_attr dev_attr;
    xmedia_bool res_enable;
    xmedia_bool vqe_enable;
    ao_vqe_attr vqe_attr;
    xmedia_mod_id binder_src;
    sample_ao_frame frame_info;
} sample_ao_config;

typedef struct {
    adec_chn chn;
    audio_codec_type type;
    voice_format format;
    xmedia_s32 sample_rate;
} sample_adec_config;

typedef struct {
    aenc_chn chn;
    audio_sample_rate sample_rate;
    audio_channel channels;
    audio_bit_depth bit_depth;
    voice_format format;
    audio_codec_type type;
    ai_dev binder_dev;
    xmedia_mod_id binder_src;
} sample_aenc_config;

xmedia_s32 sample_comm_audio_init();
xmedia_s32 sample_comm_audio_exit();

xmedia_s32 sample_comm_ai_register(xmedia_u32 mask, ai_vqe_attr_version version, xmedia_bool res_enable);
xmedia_s32 sample_comm_ai_get_default_attr(ai_dev dev, ai_dev_attr* dev_attr);
xmedia_s32 sample_comm_ai_set_share_mode(xmedia_bool enable);
xmedia_s32 sample_comm_ai_start(sample_ai_config* config);
xmedia_s32 sample_comm_ai_stop(sample_ai_config* config);
xmedia_s32 sample_comm_ai_set_codec_volume(ai_dev dev, xmedia_s32 volume);
xmedia_s32 sample_comm_ai_set_volume(sample_ai_config* config, xmedia_s32 volume);
xmedia_s32 sample_comm_ai_get_volume(sample_ai_config* config, xmedia_s32* volume);
xmedia_s32 sample_comm_ai_set_ec_source(sample_ai_config* config);
xmedia_s32 sample_comm_ai_enable_org_frame(sample_ai_config* config, xmedia_bool enable);
xmedia_s32 sample_comm_ai_enable_ec_frame(sample_ai_config* config, xmedia_bool enable);
xmedia_s32 sample_comm_ai_set_dev_param(sample_ai_config* config, ai_dev_param_cmd cmd, xmedia_void* param);
xmedia_s32 sample_comm_ai_set_trackmode(sample_ai_config* config, audio_track_mode trackmode);
xmedia_s32 sample_comm_ai_get_trackmode(sample_ai_config* config, audio_track_mode* trackmode);

xmedia_s32 sample_comm_ai_create_output_thread_to_aenc(sample_ai_config* config);
xmedia_s32 sample_comm_ai_destroy_output_thread_to_aenc(sample_ai_config* config);
xmedia_s32 sample_comm_ai_create_output_thread_to_ao(sample_ai_config* config, ao_dev dev, ao_chn chn);
xmedia_s32 sample_comm_ai_destroy_output_thread_to_ao(sample_ai_config* config);
xmedia_s32 sample_comm_ai_create_output_thread_to_file(sample_ai_config* config, FILE** output_file);
xmedia_s32 sample_comm_ai_destroy_output_thread_to_file(sample_ai_config* config);
xmedia_s32 sample_comm_detect_open(ai_dev dev, ai_vqe_attr* vqe_attr);

xmedia_s32 sample_comm_ao_register(xmedia_u32 mask, ao_vqe_attr_version version, xmedia_bool res_enable);
xmedia_s32 sample_comm_ao_get_default_attr(ao_dev dev, ao_dev_attr* dev_attr);
xmedia_s32 sample_comm_ao_start(sample_ao_config* config);
xmedia_s32 sample_comm_ao_stop(sample_ao_config* config);
xmedia_s32 sample_comm_ao_set_volume(sample_ao_config* config, xmedia_s32 volume);
xmedia_s32 sample_comm_ao_get_volume(sample_ao_config* config, xmedia_s32* volume);
xmedia_s32 sample_comm_ao_set_dev_param(sample_ao_config* config, ao_dev_param_cmd cmd, xmedia_void* param);
xmedia_s32 sample_comm_ao_create_input_thread_from_file(sample_ao_config* config, FILE** input_file);
xmedia_s32 sample_comm_ao_destroy_input_thread_from_file(sample_ao_config* config);

xmedia_s32 sample_comm_adec_register(audio_codec_type type);
xmedia_s32 sample_comm_adec_unregister(audio_codec_type type);
xmedia_s32 sample_comm_adec_start(sample_adec_config* config);
xmedia_s32 sample_comm_adec_stop(sample_adec_config* config);
xmedia_s32 sample_comm_adec_create_input_thread_from_file(sample_adec_config* config, FILE** input_file);
xmedia_s32 sample_comm_adec_destroy_input_thread_from_file(sample_adec_config* config);
xmedia_s32 sample_comm_adec_create_output_thread_to_ao(sample_adec_config* config, ao_dev dev, ao_chn chn);
xmedia_s32 sample_comm_adec_destroy_output_thread_to_ao(sample_adec_config* config);
xmedia_s32 sample_comm_adec_create_output_thread_to_file(sample_adec_config* config, FILE** output_file);
xmedia_s32 sample_comm_adec_destroy_output_thread_to_file(sample_adec_config* config);

xmedia_s32 sample_comm_aenc_register(audio_codec_type type);
xmedia_s32 sample_comm_aenc_unregister(audio_codec_type type);
xmedia_s32 sample_comm_aenc_start(sample_aenc_config* config);
xmedia_s32 sample_comm_aenc_stop(sample_aenc_config* config);
xmedia_s32 sample_comm_aenc_create_input_thread_from_file(sample_aenc_config* config, FILE** input_file);
xmedia_s32 sample_comm_aenc_destroy_input_thread_from_file(sample_aenc_config* config);
xmedia_s32 sample_comm_aenc_create_output_thread_to_file(sample_aenc_config* config, FILE** output_file);
xmedia_s32 sample_comm_aenc_destroy_output_thread_to_file(sample_aenc_config* config);

xmedia_s32 sample_comm_get_codec_info(xmedia_char* file_name, audio_codec_type* type, voice_format* format);

#ifdef __cplusplus
}
#endif

#endif

