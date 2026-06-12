/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include "xmedia_audio_common.h"
#include "xmedia_audio_ai.h"
#include "xmedia_audio_vqe.h"
#include "xmedia_audio_vqe_enhance_v1.h"
#include "xmedia_audio_vqe_enhance_v2.h"
#include "xmedia_audio_vqe_enhance_v3.h"
#include "xmedia_audio_vqe_detect.h"
#include "sample_comm_audio.h"

#define SAMPLE_AUDIO_DBG  printf
#define MAX_FILE_NAME_LEN 128
#define CRY_ALARMLIMIT 80  // probability greater than which to count as one cry frame
#define CRY_TIMELIMITCNT 3 // number of cry frame count as one cry event
#define CRY_TIMELIMIT 5 // time interval to report cry event
#define CRY_ALARMINTERVAL 3 //alarm interval time to unfreeze

#define AI_DEFAULT_MIC_VOL 20

typedef struct user_input_args {
    xmedia_char* sample;
    ai_dev dev;
    audio_sample_rate sample_rate;
    audio_sample_rate usr_sample_rate;
    audio_bit_depth bit_depth;
    audio_channel channels;
    xmedia_s32 workmode;
    xmedia_u32 samples_per_frm;
    xmedia_s32 codec_input_vol;
    xmedia_bool binder_mode;
    xmedia_bool share_mode_enable;

    xmedia_u32 send_object;
    ao_dev send_object_dev;
    char output_file_name[MAX_FILE_NAME_LEN];
    FILE *output_fd[2];
} user_input_args;

static user_input_args g_user_input_args;
static sample_ai_config g_ai_config;
static sample_ao_config g_ao_config;
static sample_aenc_config g_aenc_config;
static ai_vqe_attr_v1 ai_vqev1_attr;
static ai_vqe_attr_v2 ai_vqev2_attr;
static ai_vqe_attr_v3 ai_vqev3_attr;
static ai_vqe_attr_detect ai_detect_attr;
static xmedia_u32 g_vqe_type;

static xmedia_s32 cry_callback(xmedia_void *attr)
{
    static xmedia_s32 count = 0;
    printf("cry callback count(%d)\n" ,count++);
    return XMEDIA_SUCCESS;
}

static xmedia_s32 ssl_callback(xmedia_void* attr, xmedia_s32 out)
{
    static xmedia_s32 ssl_out = 0;
    static xmedia_s32 call_times = 0;
    call_times++;

    if (ssl_out != out) {
        printf("attr(0x%x), call_times(%d), out_angle(%d)\n", (xmedia_u32)attr, call_times, out);
    }
    ssl_out = out;

    return XMEDIA_SUCCESS;
}

static xmedia_s32 ved_callback(xmedia_void* private, xmedia_bool is_event, xmedia_bool is_mutation, xmedia_s32 out_db)
{
    if (is_event) {
        printf("volume event detect out_db = %d\n",out_db);
    }
    if (is_mutation) {
        printf("volume mutation detect out_db = %d\n", out_db);
    }

    return XMEDIA_SUCCESS;
}

static xmedia_void load_ai_vqev1_attr(ai_vqe_attr* attr)
{
    ai_vqe_attr_v1 *vqev1_attr = &ai_vqev1_attr;

    attr->version = AI_VQE_ENHANCE_ATTR_VERSION_1;
    attr->work_sample_rate = (g_user_input_args.sample_rate == AUDIO_SAMPLE_RATE_8000) ? AUDIO_SAMPLE_RATE_8000 : AUDIO_SAMPLE_RATE_16000;
    attr->in_channels = 1;
    attr->attr = (xmedia_void*)&ai_vqev1_attr;
    vqev1_attr->mask = VQE_V1_MASK_AGC | VQE_V1_MASK_ANR | VQE_V1_MASK_HPF;


    vqev1_attr->agc_attr.mode = VQE_V1_USR_MODE_AUTO;
    vqev1_attr->agc_attr.target_level = -6;
    vqev1_attr->agc_attr.max_boost_gain = 20;
    vqev1_attr->agc_attr.noise_floor = -100;
    vqev1_attr->agc_attr.ratio = 12;
    vqev1_attr->agc_attr.attack_time = 8;
    vqev1_attr->agc_attr.release_time =20;

    vqev1_attr->anr_attr.mode = VQE_V1_USR_MODE_AUTO;
    vqev1_attr->anr_attr.usr_scene = VQE_V1_ANR_SCENE_NORMAL;
    vqev1_attr->anr_attr.nr_mode = VQE_V1_NR_MODE_SPEECH;
    vqev1_attr->anr_attr.max_suppress_gain = 6;
    vqev1_attr->anr_attr.suppress_level = 0x2;
    vqev1_attr->anr_attr.nonstationary_suppress_level = 0x0;

    vqev1_attr->vad_attr.mode = VQE_V1_USR_MODE_AUTO;
    vqev1_attr->vad_attr.sensitivity_level = 4;

    vqev1_attr->hpf_attr.mode = VQE_V1_USR_MODE_AUTO;
    vqev1_attr->hpf_attr.freq = VQE_V1_HPF_FREQ_80;

    memset(&vqev1_attr->eq_attr, 0, sizeof(vqe_eq_attr_v1));
}

static xmedia_void load_ai_vqev2_attr(ai_vqe_attr* attr)
{
    ai_vqe_attr_v2 *vqev2_attr = &ai_vqev2_attr;

    attr->version = AI_VQE_ENHANCE_ATTR_VERSION_2;
    attr->work_sample_rate = (g_user_input_args.sample_rate == AUDIO_SAMPLE_RATE_8000) ? AUDIO_SAMPLE_RATE_8000 : AUDIO_SAMPLE_RATE_16000;
    attr->in_channels = 2;
    attr->attr = (xmedia_void*)&ai_vqev2_attr;
    vqev2_attr->mask = VQE_V2_MASK_AGC | VQE_V2_MASK_ANR | VQE_V2_MASK_HPF | VQE_V2_MASK_BF;
    vqev2_attr->aec_attr.mode = VQE_V2_USR_MODE_AUTO;
    vqev2_attr->aec_attr.cng_enable = XMEDIA_TRUE;
    vqev2_attr->aec_attr.freq_boundary = 1400;
    vqev2_attr->aec_attr.low_suppress_level = 0x1;
    vqev2_attr->aec_attr.high_suppress_level = 0x1;

    vqev2_attr->agc_attr.mode = VQE_V2_USR_MODE_AUTO;
    vqev2_attr->agc_attr.target_level = -6;
    vqev2_attr->agc_attr.max_boost_gain = 20;
    vqev2_attr->agc_attr.noise_floor = -65;
    vqev2_attr->agc_attr.ratio = 10;
    vqev2_attr->agc_attr.attack_time = 20;
    vqev2_attr->agc_attr.release_time = 20;

    vqev2_attr->anr_attr.mode = VQE_V2_USR_MODE_AUTO;
    vqev2_attr->anr_attr.usr_scene = VQE_V2_ANR_SCENE_NORMAL;
    vqev2_attr->anr_attr.nr_mode = VQE_V2_NR_MODE_SPEECH;
    vqev2_attr->anr_attr.max_suppress_gain = 6;
    vqev2_attr->anr_attr.suppress_level = 0x2;
    vqev2_attr->anr_attr.nonstationary_suppress_level = 0x2;

    vqev2_attr->bf_attr.reserved = 0;

    vqev2_attr->hpf_attr.mode = VQE_V2_USR_MODE_AUTO;
    vqev2_attr->hpf_attr.freq = VQE_V2_HPF_FREQ_80;

    vqev2_attr->vad_attr.mode = VQE_V2_USR_MODE_AUTO;
    vqev2_attr->vad_attr.sensitivity_level = 5;

    memset(&vqev2_attr->eq_attr, 0, sizeof(vqe_eq_attr_v2));
}

static xmedia_void load_ai_vqev3_attr(ai_vqe_attr* attr)
{
    ai_vqe_attr_v3 *vqev3_attr = &ai_vqev3_attr;

    attr->version = AI_VQE_ENHANCE_ATTR_VERSION_3;
    attr->work_sample_rate = (g_user_input_args.sample_rate == AUDIO_SAMPLE_RATE_8000) ? AUDIO_SAMPLE_RATE_8000 : AUDIO_SAMPLE_RATE_16000;
    attr->in_channels = 2;
    attr->attr = (xmedia_void*)&ai_vqev3_attr;
    vqev3_attr->mask = VQE_V3_MASK_AGC | VQE_V3_MASK_ANR | VQE_V3_MASK_HPF | VQE_V3_MASK_EQ;

    vqev3_attr->agc_attr.mode = VQE_V3_USR_MODE_AUTO;
    vqev3_attr->agc_attr.target_level = -6;
    vqev3_attr->agc_attr.max_boost_gain = 20;
    vqev3_attr->agc_attr.noise_floor = -65;
    vqev3_attr->agc_attr.ratio = 10;
    vqev3_attr->agc_attr.attack_time = 20;
    vqev3_attr->agc_attr.release_time = 20;

    vqev3_attr->anr_attr.mode = VQE_V3_USR_MODE_AUTO;
    vqev3_attr->anr_attr.level = 0;

    vqev3_attr->hpf_attr.mode = VQE_V3_USR_MODE_AUTO;
    vqev3_attr->hpf_attr.freq = VQE_V3_HPF_FREQ_80;

    memset(&vqev3_attr->eq_attr, 0, sizeof(vqe_eq_attr_v3));
}

static xmedia_void load_ai_detect_attr(ai_vqe_attr* attr)
{
    ai_vqe_attr_detect* detect_attr = &ai_detect_attr;

    attr->version = AI_VQE_DETECT_ATTR_VERSION_1;
    attr->work_sample_rate = AUDIO_SAMPLE_RATE_16000;
    attr->in_channels = g_ai_config.dev_attr.channels;

    detect_attr->mask = VQE_DETECT_MASK_HPF | VQE_DETECT_MASK_BCD | VQE_DETECT_MASK_VED;
    detect_attr->hpf_attr.mode = VQE_DET_USR_MODE_AUTO;
    detect_attr->hpf_attr.freq = VQE_DETECT_HPF_FREQ_80;

    detect_attr->bcd_attr.usr_mode = XMEDIA_TRUE;
    detect_attr->bcd_attr.bypass = XMEDIA_FALSE;
    detect_attr->bcd_attr.alarm_threshold = CRY_ALARMLIMIT;
    detect_attr->bcd_attr.time_limit = CRY_TIMELIMIT;
    detect_attr->bcd_attr.time_limit_threshold_count = CRY_TIMELIMITCNT;
    detect_attr->bcd_attr.interval_time = CRY_ALARMINTERVAL;
    detect_attr->bcd_attr.callback = (fn_bcd_callback)cry_callback;
    detect_attr->bcd_attr.bcd_model.load_type = XMEDIA_CL_LOAD_FILE;
    detect_attr->bcd_attr.bcd_model.model_info.file.pathname = "./audio_xmm/bcd_neuron_network.xmm";

    detect_attr->ssl_attr.mic_distance = 50;
    detect_attr->ssl_attr.sensitivity_level = 5;
    detect_attr->ssl_attr.shape = 0;
    detect_attr->ssl_attr.callback = (fn_detect_ssl_callback)ssl_callback;
    detect_attr->ssl_attr.private_data = (xmedia_void*)0x12345678;

    detect_attr->ved_attr.mode = VQE_DET_USR_MODE_AUTO;
    detect_attr->ved_attr.volume_threshold = -15;
    detect_attr->ved_attr.mutation_flag = 1;
    detect_attr->ved_attr.fast_change_threshold = 15;
    detect_attr->ved_attr.slow_change_threshold = 8;
    detect_attr->ved_attr.callback = (fn_detect_ved_callback)ved_callback;
    attr->attr = (xmedia_void*)&ai_detect_attr;
}

static xmedia_s32 sample_ai_config_init(xmedia_void)
{
    g_ai_config.dev = g_user_input_args.dev;

    sample_comm_ai_get_default_attr(g_ai_config.dev, &g_ai_config.dev_attr);
    g_ai_config.dev_attr.channels = g_user_input_args.channels;
    g_ai_config.dev_attr.sample_rate = g_user_input_args.sample_rate;
    g_ai_config.dev_attr.bit_depth = AUDIO_BIT_DEPTH_16;
    if (g_ai_config.dev_attr.sample_rate % AUDIO_SAMPLE_RATE_11025 == 0) {
        g_ai_config.dev_attr.pcm_samples_per_frame = 128 * (g_ai_config.dev_attr.sample_rate / AUDIO_SAMPLE_RATE_11025);
    } else {
        g_ai_config.dev_attr.pcm_samples_per_frame = g_ai_config.dev_attr.sample_rate / 100;    // 10ms
    }
    g_ai_config.dev_attr.mode = g_user_input_args.workmode;
    g_ai_config.chn_attr.sample_rate = g_user_input_args.usr_sample_rate;
    g_ai_config.chn_attr.interleaved = XMEDIA_TRUE;
    if (g_ai_config.dev == XMEDIA_AI_DEV_PDM_I2S0) {
        g_ai_config.source = AI_EC_SOURCE_PDM_REF;
        g_ai_config.ec_chn_id = 0;
    } else {
        g_ai_config.source = AI_EC_SOURCE_AO_DAC0;
        g_ai_config.ec_chn_id = 0;
    }
    if (g_vqe_type == 1 || g_vqe_type == 3) {
        g_ai_config.vqe_enable = XMEDIA_TRUE;
        load_ai_vqev1_attr(&g_ai_config.vqe_attr);
    } else if (g_vqe_type == 2) {
        g_ai_config.vqe_enable = XMEDIA_TRUE;
        load_ai_vqev2_attr(&g_ai_config.vqe_attr);
        g_ai_config.vqe_attr.in_channels = g_ai_config.dev_attr.channels;
    } else if (g_vqe_type == 6) {
        g_ai_config.vqe_enable = XMEDIA_TRUE;
        load_ai_vqev3_attr(&g_ai_config.vqe_attr);
        g_ai_config.vqe_attr.in_channels = g_ai_config.dev_attr.channels;
    } else {
        g_ai_config.vqe_enable = XMEDIA_FALSE;
    }

    g_ai_config.res_enable = XMEDIA_FALSE;
    if ((g_ai_config.vqe_enable && g_ai_config.dev_attr.sample_rate != AUDIO_SAMPLE_RATE_8000 && g_ai_config.dev_attr.sample_rate != AUDIO_SAMPLE_RATE_16000)
        || (g_ai_config.chn_attr.sample_rate != g_ai_config.dev_attr.sample_rate)) {
        g_ai_config.res_enable = XMEDIA_TRUE;
    }
    if (g_ai_config.dev == XMEDIA_AI_DEV_I2S0 && g_ai_config.dev_attr.channels == 16) {
        g_ai_config.dev_attr.un_attr.i2s_attr.i2s_mode = AUDIO_I2S_TDM_MODE;
        g_ai_config.dev_attr.un_attr.i2s_attr.bclk_sel = AUDIO_I2S_BCLK_1_DIV;
    }
    if (g_user_input_args.share_mode_enable) {
        g_ai_config.dev_attr.pcm_frame_num = 500;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_ao_config_init(xmedia_void)
{
    g_ao_config.dev = g_user_input_args.send_object_dev;

    sample_comm_ao_get_default_attr(g_ao_config.dev, &g_ao_config.dev_attr);
    g_ao_config.dev_attr.channels = g_user_input_args.channels;
    g_ao_config.dev_attr.sample_rate = g_user_input_args.sample_rate;
    g_ao_config.dev_attr.bit_depth = AUDIO_BIT_DEPTH_16;
    g_ao_config.dev_attr.pcm_samples_per_frame = g_ai_config.dev_attr.pcm_samples_per_frame;
    g_ao_config.dev_attr.mode = g_user_input_args.workmode;

    g_ao_config.vqe_enable = XMEDIA_FALSE;

    if (g_user_input_args.binder_mode == XMEDIA_TRUE) {
        g_ao_config.binder_src = MOD_ID_AI;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_aenc_config_init(xmedia_void)
{
    voice_format format = VOICE_FORMAT_G711_A;
    audio_codec_type type = CODEC_TYPE_G711;

    sample_comm_get_codec_info(g_user_input_args.output_file_name, &type, &format);

    g_aenc_config.sample_rate = g_user_input_args.usr_sample_rate;
    g_aenc_config.channels = (g_vqe_type == 2 || g_vqe_type == 5 || g_ai_config.dev_attr.mode == AUDIO_DEV_WORK_MODE_MONO) ? 1 : g_user_input_args.channels;    // ai open vqev2 or ai 1 channel
    g_aenc_config.bit_depth = AUDIO_BIT_DEPTH_16;
    g_aenc_config.chn = 0;
    g_aenc_config.type = type;
    g_aenc_config.format = format;
    g_aenc_config.binder_dev = g_user_input_args.dev;
    if (g_user_input_args.binder_mode == XMEDIA_TRUE) {
        g_aenc_config.binder_src = MOD_ID_AI;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_init_audio()
{
    return sample_comm_audio_init();
}

static xmedia_s32 sample_start_ai()
{
    xmedia_s32 ret;
    xmedia_s32 vol = AI_DEFAULT_MIC_VOL;
    ai_vqe_attr vqe_detect_attr;

    ret = sample_ai_config_init();
    CHECK_RET(ret, "sample_ai_config_init");

    if (g_user_input_args.share_mode_enable) {
        ret = sample_comm_ai_set_share_mode(g_user_input_args.share_mode_enable);
        CHECK_RET(ret, "sample_comm_ai_set_share_mode");
    }

    if (g_vqe_type == 1 || g_vqe_type == 3) {
        ret = sample_comm_ai_register(ai_vqev1_attr.mask, g_ai_config.vqe_attr.version, g_ai_config.res_enable);
        CHECK_RET(ret, "sample_comm_ai_register");
    } else if (g_vqe_type == 2) {
        ret = sample_comm_ai_register(ai_vqev2_attr.mask, g_ai_config.vqe_attr.version, g_ai_config.res_enable);
        CHECK_RET(ret, "sample_comm_ai_register");
    } else if (g_vqe_type == 6) {
        ret = sample_comm_ai_register(ai_vqev3_attr.mask, g_ai_config.vqe_attr.version, g_ai_config.res_enable);
        CHECK_RET(ret, "sample_comm_ai_register");
    }else { //可能存在只有重采样场景
        if (g_ai_config.res_enable) {
            ret = sample_comm_ao_register(0, AI_VQE_ENHANCE_ATTR_VERSION_1, g_ai_config.res_enable);
            CHECK_RET(ret, "sample_comm_ai_register");
        }
    }

    ret = sample_comm_ai_start(&g_ai_config);
    CHECK_RET(ret, "sample_comm_ai_start");

    if (g_ai_config.dev == XMEDIA_AI_DEV_ADC0) {
        ret = sample_comm_ai_set_dev_param(&g_ai_config, AI_PARAM_SET_CODEC_ADC_GAIN, (xmedia_void*)&vol);
        if (ret != XMEDIA_SUCCESS) {
            sample_comm_ai_stop(&g_ai_config);
            CHECK_RET(ret, "sample_comm_ai_set_dev_param");
        }
    }

    if (g_user_input_args.share_mode_enable) {
        return XMEDIA_SUCCESS;
    }

    if (g_vqe_type == 3 || g_vqe_type == 4) {
        load_ai_detect_attr(&vqe_detect_attr);
        ret = sample_comm_ai_register(ai_detect_attr.mask, vqe_detect_attr.version, g_ai_config.res_enable);
        CHECK_RET(ret, "sample_comm_ai_register");
        ret = sample_comm_detect_open(g_ai_config.dev, &vqe_detect_attr);
        if (ret != XMEDIA_SUCCESS) {
            sample_comm_ai_stop(&g_ai_config);
            CHECK_RET(ret, "sample_comm_bcd_open");
        }
    }

    if (g_user_input_args.send_object == 0) {
        ret = sample_comm_ai_create_output_thread_to_file(&g_ai_config, g_user_input_args.output_fd);
        CHECK_RET(ret, "sample_comm_ai_create_output_thread_to_file");
    }

    if ((g_user_input_args.send_object == 1) && (g_user_input_args.binder_mode == XMEDIA_FALSE)) {
        ret = sample_comm_ai_create_output_thread_to_aenc(&g_ai_config);
        CHECK_RET(ret, "sample_comm_ai_create_output_thread_to_aenc");
    }

    if ((g_user_input_args.send_object == 2) && (g_user_input_args.binder_mode == XMEDIA_FALSE)) {
        ret = sample_comm_ai_create_output_thread_to_ao(&g_ai_config, g_user_input_args.send_object_dev, 0);
        CHECK_RET(ret, "sample_comm_ai_create_output_thread_to_ao");
    }

    return ret;
}

static xmedia_s32 sample_start_ao()
{
    xmedia_s32 ret;

    ret = sample_ao_config_init();
    CHECK_RET(ret, "sample_ao_config_init");

    ret = sample_comm_ao_start(&g_ao_config);

    return ret;
}

static xmedia_s32 sample_start_aenc()
{
    xmedia_s32 ret;

    ret = sample_aenc_config_init();
    CHECK_RET(ret, "sample_aenc_config_init");

    ret = sample_comm_aenc_register(g_aenc_config.type);
    CHECK_RET(ret, "sample_comm_adec_register");

    ret = sample_comm_aenc_start(&g_aenc_config);
    CHECK_RET(ret, "sample_comm_aenc_start");

    ret = sample_comm_aenc_create_output_thread_to_file(&g_aenc_config, g_user_input_args.output_fd);
    CHECK_RET(ret, "sample_comm_aenc_create_data_thread");

    return ret;
}

static xmedia_s32 sample_deinit_audio()
{
    return sample_comm_audio_exit();
}

static xmedia_s32 sample_stop_ai()
{
    xmedia_s32 ret;

    if (g_user_input_args.send_object == 0) {
        ret = sample_comm_ai_destroy_output_thread_to_file(&g_ai_config);
        CHECK_RET(ret, "sample_comm_ai_destroy_output_thread_to_file");
    }

    if ((g_user_input_args.send_object == 1) && (g_user_input_args.binder_mode == XMEDIA_FALSE)) {
        ret = sample_comm_ai_destroy_output_thread_to_aenc(&g_ai_config);
        CHECK_RET(ret, "sample_comm_ai_destroy_output_thread_to_aenc");
    }

    if ((g_user_input_args.send_object == 2) && (g_user_input_args.binder_mode == XMEDIA_FALSE)) {
        ret = sample_comm_ai_destroy_output_thread_to_ao(&g_ai_config);
        CHECK_RET(ret, "sample_comm_ai_destroy_output_thread_to_ao");
    }

    ret = sample_comm_ai_stop(&g_ai_config);
    CHECK_RET(ret, "sample_comm_ai_stop");

    if (g_user_input_args.share_mode_enable) {
        sample_comm_ai_set_share_mode(XMEDIA_FALSE);
    }

    return ret;
}

static xmedia_s32 sample_stop_ao()
{
    xmedia_s32 ret;

    ret = sample_comm_ao_stop(&g_ao_config);
    CHECK_RET(ret, "sample_comm_ao_stop");

    return ret;
}

static xmedia_s32 sample_stop_aenc()
{
    xmedia_s32 ret;

    ret = sample_comm_aenc_destroy_output_thread_to_file(&g_aenc_config);
    CHECK_RET(ret, "sample_comm_aenc_create_out_to_file_thread");

    ret = sample_comm_aenc_stop(&g_aenc_config);
    CHECK_RET(ret, "sample_comm_aenc_stop");

    ret = sample_comm_aenc_unregister(g_aenc_config.type);
    CHECK_RET(ret, "sample_comm_aenc_unregister");

    return ret;
}


xmedia_s32 sample_audio_ai(xmedia_void)
{
    xmedia_s32 ret;

    ret = sample_init_audio();
    if (ret != XMEDIA_SUCCESS) {
        goto ERR_INIT_AUDIO;
    }

    ret = sample_start_ai();
    if (ret != XMEDIA_SUCCESS) {
        goto ERR_START_AI;
    }

    if (g_user_input_args.send_object == 2) {
        ret = sample_start_ao();
        if (ret != XMEDIA_SUCCESS) {
            goto ERR_START_AO;
        }
    }

    if (g_user_input_args.send_object == 1) {
        ret = sample_start_aenc();
        if (ret != XMEDIA_SUCCESS) {
            goto ERR_START_AENC;
        }
    }

    while(1) {
        xmedia_s32 val;
        xmedia_char input_cmd[32];
        SAMPLE_AUDIO_DBG("\nInput 'q' ENTER to exit\n");
        SAMPLE_AUDIO_DBG("Input 'svXX' to set ai vol to xx dB\n");
        SAMPLE_AUDIO_DBG("Input 'gv' to get ai vol\n");
        SAMPLE_AUDIO_DBG("Input 'saXX' to set adc gain to xx dB\n");
        SAMPLE_AUDIO_DBG("Input 'ga' to get adc gain\n");

        SAMPLE_AUDIO_DBG("Input 'se' to set ec source\n");
        SAMPLE_AUDIO_DBG("Input 'eo' to enable org frame\n");
        SAMPLE_AUDIO_DBG("Input 'do' to disable org frame\n");
        SAMPLE_AUDIO_DBG("Input 'ee' to enable ec frame\n");
        SAMPLE_AUDIO_DBG("Input 'de' to disable ec frame\n");
        SAMPLE_AUDIO_DBG("Input 'dso' to dump stream open\n");
        SAMPLE_AUDIO_DBG("Input 'dsc' to dump stream close\n");
        SAMPLE_AUDIO_DBG("Input 'pow' to test suspend and resume\n");

        fgets(input_cmd, (sizeof(input_cmd) - 1), stdin);
        if (input_cmd[0] == 's' && input_cmd[1] == 'v') {
            val = atoi(input_cmd + 2);
            ret = sample_comm_ai_set_volume(&g_ai_config, val);
            SAMPLE_AUDIO_DBG("\nSet ai volume %d return 0x%x\n", val, ret);
        } else if (input_cmd[0] == 'g' && input_cmd[1] == 'v') {
            ret = sample_comm_ai_get_volume(&g_ai_config, &val);
            SAMPLE_AUDIO_DBG("\nGet ai volume %d return 0x%x\n", val, ret);
        } else if (input_cmd[0] == 's' && input_cmd[1] == 'a') {
            val = atoi(input_cmd + 2);
            ret = sample_comm_ai_set_dev_param(&g_ai_config, AI_PARAM_SET_CODEC_ADC_GAIN, (xmedia_void*)&val);
            SAMPLE_AUDIO_DBG("\nSet adc gain %d return 0x%x\n", val, ret);
        } else if (input_cmd[0] == 'g' && input_cmd[1] == 'a') {
            ret = sample_comm_ai_set_dev_param(&g_ai_config, AI_PARAM_GET_CODEC_ADC_GAIN, (xmedia_void*)&val);
            SAMPLE_AUDIO_DBG("\nGet adc gain %d return 0x%x\n", val, ret);

        } else if (input_cmd[0] == 's' && input_cmd[1] == 'e') {
            g_ai_config.source = AI_EC_SOURCE_AO_DAC0;
            g_ai_config.ec_chn_id = 0;
            ret = sample_comm_ai_set_ec_source(&g_ai_config);
            SAMPLE_AUDIO_DBG("\nSet ec source return 0x%x\n", ret);
        } else if (input_cmd[0] == 'e' && input_cmd[1] == 'o') {
            ret = sample_comm_ai_enable_org_frame(&g_ai_config, XMEDIA_TRUE);
            SAMPLE_AUDIO_DBG("\nSet enable org frame return 0x%x\n", ret);
        } else if (input_cmd[0] == 'd' && input_cmd[1] == 'o') {
            ret = sample_comm_ai_enable_org_frame(&g_ai_config, XMEDIA_FALSE);
            SAMPLE_AUDIO_DBG("\nSet disable org frame return 0x%x\n", ret);
        } else if (input_cmd[0] == 'e' && input_cmd[1] == 'e') {
            ret = sample_comm_ai_enable_ec_frame(&g_ai_config, XMEDIA_TRUE);
            SAMPLE_AUDIO_DBG("\nSet enable ec frame return 0x%x\n", ret);
        } else if (input_cmd[0] == 'd' && input_cmd[1] == 'e') {
            ret = sample_comm_ai_enable_ec_frame(&g_ai_config, XMEDIA_FALSE);
            SAMPLE_AUDIO_DBG("\nSet disable ec frame return 0x%x\n", ret);
        } else if (input_cmd[0] == 'd' && input_cmd[1] == 's' && input_cmd[2] == 'o') {
            system("echo setnode 1 > /proc/umap/ai");
            system("echo dump start > /proc/umap/ai");
            SAMPLE_AUDIO_DBG("\nSet dump orgin and ai post stream start\n");
        } else if (input_cmd[0] == 'd' && input_cmd[1] == 's' && input_cmd[2] == 'c') {
            system("echo dump stop > /proc/umap/ai");
            SAMPLE_AUDIO_DBG("\nSet dump stream stop \n");
        } else if (input_cmd[0] == 'p' && input_cmd[1] == 'o' && input_cmd[2] == 'w') {
            system("echo mem > /sys/power/state"); // 进入休眠状态 之后自动唤醒
            SAMPLE_AUDIO_DBG("\nSuspend and resume test ok\n");
        } else if ((input_cmd[0] == 'q') || (input_cmd[0] == 'Q')) {
            SAMPLE_AUDIO_DBG("Ready to exit\n");
            break;
        }
    }

    if (g_user_input_args.send_object == 1) {
        sample_stop_aenc();
    }
ERR_START_AENC:
    if (g_user_input_args.send_object == 2) {
        sample_stop_ao();
    }
ERR_START_AO:
    sample_stop_ai();
ERR_START_AI:
ERR_INIT_AUDIO:
    sample_deinit_audio();
    return ret;
}

static xmedia_void user_args_usage(user_input_args* args)
{
    SAMPLE_AUDIO_DBG("usage: %s [dev] [send_object] [binder_mode] [samplerate] [usr_samplerate] [channels] [workmode] [outfile/send_object_dev] [vqe_type/share_mode]\n", args->sample);
    SAMPLE_AUDIO_DBG("dev: dev id\n");
    SAMPLE_AUDIO_DBG("send_object: 0:not send data and save ai file, 1:ai send data to aenc, 2:ai send data to ao\n");
    SAMPLE_AUDIO_DBG("binder_mode: use binder mode\n");
    SAMPLE_AUDIO_DBG("samplerate: dev samplerate\n");
    SAMPLE_AUDIO_DBG("usr_samplerate: ai user data samplerate\n");
    SAMPLE_AUDIO_DBG("channels: dev chn cnt\n");
    SAMPLE_AUDIO_DBG("workmode: mono:0, queue:1\n");
    SAMPLE_AUDIO_DBG("outfile/send_object_dev: ai out file or aenc encoder out file(.pcm/.g711a/.g711u/.g726/.aac/.mp3/.adpcm) or ao dev\n");
    SAMPLE_AUDIO_DBG("vqe_type/share_mode: 0: not open vqe, 1: open vqe v1, 2: open vqe v2, 3: open vqe v1 and detect, 4: open vqe detect, 5: open share mode, 6: open vqe v3\n");

    SAMPLE_AUDIO_DBG("examples:\n");
    SAMPLE_AUDIO_DBG("    %s 0 1 1 8000 8000 1 1 out.g711a 1\n", args->sample);
    SAMPLE_AUDIO_DBG("    %s 0 2 1 8000 8000 1 1    0      1\n", args->sample);
    SAMPLE_AUDIO_DBG("    %s 0 0 0 8000 8000 2 1 out.pcm   2\n", args->sample);
    SAMPLE_AUDIO_DBG("    %s 0 0 0 16000 16000 1 1 out.pcm 3\n", args->sample);
    SAMPLE_AUDIO_DBG("    %s 0 0 0 16000 16000 1 1 out.pcm 4\n", args->sample);
    SAMPLE_AUDIO_DBG("    %s 0 0 0 8000 8000 2 1 out.pcm   5\n", args->sample);
    SAMPLE_AUDIO_DBG("    %s 0 0 0 16000 16000 1 1 out.pcm 6\n", args->sample);
}

static xmedia_s32 user_args_init(int argc, char* argv[], user_input_args* user_in_args)
{
    user_in_args->sample = argv[0];

    if ((argc >= 2) && (strcmp(argv[1], "-h")) == 0) {
        user_args_usage(user_in_args);
        return XMEDIA_FAILURE;
    }

    if (argc != 10) {
        printf("Invalid argc:%d\n", argc);
        user_args_usage(user_in_args);
        return XMEDIA_FAILURE;
    }

    user_in_args->dev = atoi(argv[1]);
    user_in_args->send_object = atoi(argv[2]);
    if (user_in_args->send_object == 0) {
        printf("ai not send data and save ai file\n");
    } else if (user_in_args->send_object == 1) {
        printf("ai send data to aenc\n");
    } else if (user_in_args->send_object == 2) {
        printf("ai send data to ao\n");
    } else {
        printf("Invalid send_object:%d\n", user_in_args->send_object);
        user_args_usage(user_in_args);
        return XMEDIA_FAILURE;
    }
    user_in_args->binder_mode = (atoi(argv[3]) == 0) ? XMEDIA_FALSE : XMEDIA_TRUE;
    user_in_args->sample_rate = atoi(argv[4]);
    user_in_args->usr_sample_rate = atoi(argv[5]);
    user_in_args->channels = atoi(argv[6]);
    user_in_args->workmode = atoi(argv[7]);

    g_vqe_type = atoi(argv[9]);
    if (g_vqe_type < 0 || g_vqe_type > 6) {
        printf("Invalid vqe_type/share_mode:%d\n", g_vqe_type);
        user_args_usage(user_in_args);
        return XMEDIA_FAILURE;
    }

    if (user_in_args->send_object != 2) {
        memcpy(user_in_args->output_file_name, argv[8], strlen(argv[8]));
        if (strcmp(user_in_args->output_file_name, "null")) {
            user_in_args->output_fd[0] = fopen(user_in_args->output_file_name, "w+");
            if (user_in_args->output_fd[0] == NULL) {
                printf("Open %s failed\n", user_in_args->output_file_name);
                return XMEDIA_FAILURE;
             }
        } else {
            user_in_args->output_fd[0] = NULL;
        }
    }
    user_in_args->share_mode_enable = (g_vqe_type == 5) ? XMEDIA_TRUE : XMEDIA_FALSE;

    SAMPLE_AUDIO_DBG("get user args: \n");
    SAMPLE_AUDIO_DBG("  dev: %d\n", user_in_args->dev);
    SAMPLE_AUDIO_DBG("  send_object: %d\n", user_in_args->send_object);
    SAMPLE_AUDIO_DBG("  binder_mode: %d\n", user_in_args->binder_mode);
    SAMPLE_AUDIO_DBG("  samplerate: %d\n", user_in_args->sample_rate);
    SAMPLE_AUDIO_DBG("  user samplerate: %d\n", user_in_args->usr_sample_rate);
    SAMPLE_AUDIO_DBG("  chnnels: %d\n", user_in_args->channels);
    SAMPLE_AUDIO_DBG("  workmode: %d\n", user_in_args->workmode);

    SAMPLE_AUDIO_DBG("  vqe_type/share_mode: %d\n", g_vqe_type);
    SAMPLE_AUDIO_DBG("  share_mode: %d\n", user_in_args->share_mode_enable);
    if (user_in_args->output_fd[0] != NULL) {
        SAMPLE_AUDIO_DBG("  mic0_file_name:%s\n", user_in_args->output_file_name);
    }

    return XMEDIA_SUCCESS;
}

static xmedia_void user_args_deinit(user_input_args* user_in_args)
{
    if (user_in_args->output_fd[1] != NULL) {
        fclose(user_in_args->output_fd[1]);
        user_in_args->output_fd[1] = NULL;
    }

    if (user_in_args->output_fd[0] != NULL) {
        fclose(user_in_args->output_fd[0]);
        user_in_args->output_fd[0] = NULL;
    }
}

xmedia_s32 main(int argc, char* argv[])
{
    xmedia_s32 ret;

    ret = user_args_init(argc, argv, &g_user_input_args);
    if (ret != XMEDIA_SUCCESS) {
        printf("user_args_init failed(0x%x)\n\n", ret);
        return ret;
    }
    SAMPLE_AUDIO_DBG("user_args_init ok\n");

    ret = sample_audio_ai();
    if (ret != XMEDIA_SUCCESS) {
        printf("sample_auido_ai failed(0x%x)\n", ret);
    }

    user_args_deinit(&g_user_input_args);
    printf("exit sample ai\n");
    return ret;
}

