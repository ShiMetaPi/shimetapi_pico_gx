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

#include "sample_aov_audio.h"
#ifdef NORMAL_SUPPORT_AUDIO
#include "xmedia_audio_vqe_enhance_v1.h"
#include "xmedia_audio_vqe_enhance_v2.h"

#define FRAME_TIME        10  //ms

#define SAMPLE_AOV_AUDIO_ADEC_CHN 1

#define SAMPLE_AOV_AUDIO_ADEC_RATE              16000
#define SAMPLE_AOV_AUDIO_AO_RATE                16000
#define SAMPLE_AOV_AUDIO_AI_RATE                16000
#define SAMPLE_AOV_AUDIO_AENC_RATE              16000


#define SAMPLE_AOV_AUDIO_AENC_VOL               20
#define SAMPLE_AOV_AUDIO_ADEC_BASE_FILE_PATH         "/usr/param/chn"   //chn0.mp3
#define SAMPLE_AOV_AUDIO_AENC_FILE_PATH              "/tmp/sd/aov_audio.aac"

static sample_ao_config   g_ao_config = {0};
static ao_vqe_attr_v2 ao_vqev2_attr = {0};
static ao_vqe_attr_v1 ao_vqev1_attr = {0};
static sample_adec_config g_adec_config[SAMPLE_AOV_AUDIO_ADEC_CHN] = {0};
static FILE *adec_input_fd[SAMPLE_AOV_AUDIO_ADEC_CHN];
static FILE *aenc_output_fd = XMEDIA_NULL;

static sample_ai_config g_ai_config = {0};
static sample_aenc_config g_aenc_config;
static ai_vqe_attr_v2 ai_vqev2_attr = {0};

static xmedia_s32 sample_aov_audio_play_thread_start()
{
    xmedia_s32 i = 0;
    xmedia_char buf[256] = {0};
    xmedia_s32 s32Ret = XMEDIA_SUCCESS;

    for (i = 0; i < SAMPLE_AOV_AUDIO_ADEC_CHN; i++) {
        memset(buf, 0 , sizeof(buf));
        snprintf(buf, 256, "%s%d%s", SAMPLE_AOV_AUDIO_ADEC_BASE_FILE_PATH,i,".mp3");

        adec_input_fd[i] = fopen(buf, "rb");
        if (adec_input_fd[i] == XMEDIA_NULL) {
            for (;i >= 0; i --) {
                sample_comm_adec_destroy_input_thread_from_file(&g_adec_config[i]);
                fclose(adec_input_fd[i]);
            }
            return XMEDIA_FAILURE;
        }

        s32Ret |= sample_comm_adec_create_input_thread_from_file(&g_adec_config[i], &adec_input_fd[i]);
    }

    return s32Ret;
}

static xmedia_s32 sample_aov_audio_play_thread_stop()
{
    xmedia_s32 i = 0;
    xmedia_s32 s32Ret = XMEDIA_SUCCESS;

    for (i = 0; i < SAMPLE_AOV_AUDIO_ADEC_CHN; i++) {
        s32Ret |= sample_comm_adec_destroy_input_thread_from_file(&g_adec_config[i]);
        fclose(adec_input_fd[i]);
        adec_input_fd[i] = XMEDIA_NULL;
    }

    return s32Ret;
}

xmedia_s32 sample_aov_audio_adec_start()
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_s32 i = 0;
    xmedia_char buf[256] = {0};
    voice_format format = VOICE_FORMAT_G711_A;
    audio_codec_type type = CODEC_TYPE_G711;

    for (i = 0; i < SAMPLE_AOV_AUDIO_ADEC_CHN; i++) {
        memset(buf, 0 , sizeof(buf));
        snprintf(buf, 256, "%s%d%s", SAMPLE_AOV_AUDIO_ADEC_BASE_FILE_PATH,i,".mp3");
        s32Ret = sample_comm_get_codec_info(buf, &type, &format);
        if (s32Ret != XMEDIA_SUCCESS) {
            goto ADEC_ERR;
        }

        g_adec_config[i].sample_rate = SAMPLE_AOV_AUDIO_ADEC_RATE;
        g_adec_config[i].chn = i;
        g_adec_config[i].type = type;
        g_adec_config[i].format = format;

        s32Ret = sample_comm_adec_register(g_adec_config[i].type);
        if (s32Ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("sample_comm_adec_register fail\n");
            goto ADEC_ERR;
        }

        s32Ret = sample_comm_adec_start(&g_adec_config[i]);
        if (s32Ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("sample_comm_adec_start fail\n");
            goto ADEC_ERR;
        }
    }

    return XMEDIA_SUCCESS;

ADEC_ERR:
    for (;i >= 0; i --) {
        sample_comm_adec_stop(&g_adec_config[i]);
    }

    for (i = 0; i < SAMPLE_AOV_AUDIO_ADEC_CHN; i++) {
        sample_comm_adec_unregister(g_adec_config[i].type);
    }

    return XMEDIA_FAILURE;
}

xmedia_s32 sample_aov_audio_adec_stop()
{
    xmedia_s32 i = 0;

    for (i = 0; i < SAMPLE_AOV_AUDIO_ADEC_CHN; i++) {
        sample_comm_adec_stop(&g_adec_config[i]);
    }

    for (i = 0; i < SAMPLE_AOV_AUDIO_ADEC_CHN; i++) {
        sample_comm_adec_unregister(g_adec_config[i].type);
    }

    return XMEDIA_SUCCESS;
}

xmedia_void sample_aov_audio_ao_load_vqe_v1(ao_vqe_attr *vqe_attr)
{
    vqe_attr->attr = (xmedia_void*)&ao_vqev1_attr;
    vqe_attr->version = AO_VQE_ENHANCE_ATTR_VERSION_1;

    ao_vqev1_attr.mask = VQE_V1_MASK_AGC | VQE_V1_MASK_ANR | VQE_V1_MASK_HPF | VQE_V1_MASK_EQ;
    ao_vqev1_attr.agc_attr.mode = VQE_V1_USR_MODE_AUTO;
    ao_vqev1_attr.agc_attr.target_level = -8;
    ao_vqev1_attr.agc_attr.max_boost_gain = 25;
    ao_vqev1_attr.agc_attr.noise_floor = -120;
    ao_vqev1_attr.agc_attr.ratio = 12;
    ao_vqev1_attr.agc_attr.attack_time = 2;
    ao_vqev1_attr.agc_attr.release_time = 20;

    ao_vqev1_attr.anr_attr.mode = VQE_V1_USR_MODE_AUTO;
    ao_vqev1_attr.anr_attr.usr_scene = VQE_V1_ANR_SCENE_NORMAL;
    ao_vqev1_attr.anr_attr.nr_mode = VQE_V1_NR_MODE_SPEECH;
    ao_vqev1_attr.anr_attr.max_suppress_gain = 6;
    ao_vqev1_attr.anr_attr.suppress_level = 0x0;
    ao_vqev1_attr.anr_attr.nonstationary_suppress_level = 0x2;

    ao_vqev1_attr.hpf_attr.mode = VQE_V1_USR_MODE_AUTO;
    ao_vqev1_attr.hpf_attr.freq = VQE_V1_HPF_FREQ_80;

    memset(&ao_vqev1_attr.eq_attr, 0, sizeof(vqe_eq_attr_v1));
}

xmedia_void sample_aov_audio_ao_load_vqe_v2(ao_vqe_attr *vqe_attr)
{
    vqe_attr->attr = (xmedia_void*)&ao_vqev2_attr;
    vqe_attr->version = AO_VQE_ENHANCE_ATTR_VERSION_2;

    ao_vqev2_attr.mask = VQE_V2_MASK_AGC | VQE_V2_MASK_ANR | VQE_V2_MASK_HPF | VQE_V2_MASK_EQ;

    ao_vqev2_attr.agc_attr.mode = VQE_V2_USR_MODE_AUTO;
    ao_vqev2_attr.agc_attr.target_level = -6;
    ao_vqev2_attr.agc_attr.max_boost_gain = 20;
    ao_vqev2_attr.agc_attr.noise_floor = -65;
    ao_vqev2_attr.agc_attr.ratio = 10;
    ao_vqev2_attr.agc_attr.attack_time = 20;
    ao_vqev2_attr.agc_attr.release_time = 20;

    ao_vqev2_attr.anr_attr.mode = VQE_V2_USR_MODE_AUTO;
    ao_vqev2_attr.anr_attr.usr_scene = VQE_V2_ANR_SCENE_NORMAL;
    ao_vqev2_attr.anr_attr.nr_mode = VQE_V2_NR_MODE_SPEECH;
    ao_vqev2_attr.anr_attr.max_suppress_gain = 6;
    ao_vqev2_attr.anr_attr.suppress_level = 0x2;
    ao_vqev2_attr.anr_attr.nonstationary_suppress_level = 0x2;

    ao_vqev2_attr.hpf_attr.mode = VQE_V2_USR_MODE_AUTO;
    ao_vqev2_attr.hpf_attr.freq = VQE_V2_HPF_FREQ_80;

    memset(&ao_vqev2_attr.eq_attr, 0, sizeof(vqe_eq_attr_v2));
}

xmedia_void sample_aov_audio_ai_load_vqe_v2(ai_vqe_attr *vqe_attr)
{
    vqe_attr->attr = (xmedia_void*)&ai_vqev2_attr;

    vqe_attr->version = AI_VQE_ENHANCE_ATTR_VERSION_2;
    vqe_attr->work_sample_rate = AUDIO_SAMPLE_RATE_16000;
    vqe_attr->in_channels = 2;
    vqe_attr->attr = (xmedia_void*)&ai_vqev2_attr;
    ai_vqev2_attr.mask = VQE_V2_MASK_AGC | VQE_V2_MASK_ANR | VQE_V2_MASK_HPF | VQE_V2_MASK_BF;

    ai_vqev2_attr.aec_attr.mode = VQE_V2_USR_MODE_AUTO;
    ai_vqev2_attr.aec_attr.cng_enable = XMEDIA_TRUE;
    ai_vqev2_attr.aec_attr.freq_boundary = 1800;
    ai_vqev2_attr.aec_attr.low_suppress_level = 0x7;
    ai_vqev2_attr.aec_attr.high_suppress_level = 0x5;

    ai_vqev2_attr.agc_attr.mode = VQE_V2_USR_MODE_AUTO;
    ai_vqev2_attr.agc_attr.target_level = -6;
    ai_vqev2_attr.agc_attr.max_boost_gain = 20;
    ai_vqev2_attr.agc_attr.noise_floor = -100;
    ai_vqev2_attr.agc_attr.ratio = 12;
    ai_vqev2_attr.agc_attr.attack_time = 8;
    ai_vqev2_attr.agc_attr.release_time = 20;

    ai_vqev2_attr.anr_attr.mode = VQE_V2_USR_MODE_AUTO;
    ai_vqev2_attr.anr_attr.usr_scene = VQE_V2_ANR_SCENE_NORMAL;
    ai_vqev2_attr.anr_attr.nr_mode = VQE_V2_NR_MODE_SPEECH;
    ai_vqev2_attr.anr_attr.max_suppress_gain = 6;
    ai_vqev2_attr.anr_attr.suppress_level = 0x2;
    ai_vqev2_attr.anr_attr.nonstationary_suppress_level = 0x0;

    ai_vqev2_attr.bf_attr.reserved = 0;

    ai_vqev2_attr.vad_attr.mode = VQE_V2_USR_MODE_AUTO;
    ai_vqev2_attr.vad_attr.sensitivity_level = 5;

    ai_vqev2_attr.hpf_attr.mode = VQE_V2_USR_MODE_AUTO;
    ai_vqev2_attr.hpf_attr.freq = VQE_V2_HPF_FREQ_80;

    memset(&ai_vqev2_attr.eq_attr, 0, sizeof(vqe_eq_attr_v2));
}

xmedia_s32 sample_aov_audio_ao_start()
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    g_ao_config.dev = XMEDIA_AO_DEV_DAC0;

    sample_comm_ao_get_default_attr(g_ao_config.dev, &g_ao_config.dev_attr);
    g_ao_config.dev_attr.channels = 1;
    g_ao_config.dev_attr.sample_rate = 16000;
    g_ao_config.dev_attr.pcm_samples_per_frame = FRAME_TIME * g_ao_config.dev_attr.sample_rate / 1000;
    g_ao_config.dev_attr.mode = AUDIO_DEV_WORK_MODE_MONO;
    g_ao_config.dev_attr.pcm_frame_max_num = 12;
    g_ao_config.vqe_enable = XMEDIA_TRUE;
    sample_aov_audio_ao_load_vqe_v2(&g_ao_config.vqe_attr);
    if ((g_ao_config.dev_attr.sample_rate != AUDIO_SAMPLE_RATE_8000 && g_ao_config.dev_attr.sample_rate != AUDIO_SAMPLE_RATE_16000)
        || (g_ao_config.frame_info.frm_rate != g_ao_config.dev_attr.sample_rate)) {
        g_ao_config.res_enable = XMEDIA_TRUE;
    }

    g_ao_config.binder_src = MOD_ID_ADEC;

    s32Ret = sample_comm_ao_register(ao_vqev2_attr.mask, AO_VQE_ENHANCE_ATTR_VERSION_2, g_ao_config.res_enable);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("sample_comm_ao_register fail\n");
        return XMEDIA_FAILURE;
    }

    s32Ret = sample_comm_ao_start(&g_ao_config);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("sample_comm_ao_start fail\n");
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_aov_audio_ao_stop()
{
    return sample_comm_ao_stop(&g_ao_config);
}

xmedia_s32 sample_aov_audio_ai_start()
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_s32 codec_vol = SAMPLE_AOV_AUDIO_AENC_VOL;
    g_ai_config.dev = XMEDIA_AI_DEV_ADC0;

    sample_comm_ai_get_default_attr(g_ai_config.dev, &g_ai_config.dev_attr);
    g_ai_config.dev_attr.channels = AUDIO_CHANNEL_2;
    g_ai_config.dev_attr.sample_rate = SAMPLE_AOV_AUDIO_AI_RATE;
    g_ai_config.dev_attr.pcm_samples_per_frame = FRAME_TIME * SAMPLE_AOV_AUDIO_AI_RATE / 1000;
    g_ai_config.dev_attr.mode = AUDIO_DEV_WORK_MODE_QUEUE;
    g_ai_config.chn_attr.sample_rate = SAMPLE_AOV_AUDIO_AI_RATE;
    g_ai_config.source = AI_EC_SOURCE_AO_DAC0;
    g_ai_config.ec_chn_id = 0;
    g_ai_config.vqe_enable = XMEDIA_TRUE;
    g_ai_config.res_enable = XMEDIA_FALSE;
    if ((g_ai_config.dev_attr.sample_rate != AUDIO_SAMPLE_RATE_8000 && g_ai_config.dev_attr.sample_rate != AUDIO_SAMPLE_RATE_16000)
        || (g_ai_config.chn_attr.sample_rate != g_ai_config.dev_attr.sample_rate)) {
        g_ai_config.res_enable = XMEDIA_TRUE;
    }

    sample_aov_audio_ai_load_vqe_v2(&g_ai_config.vqe_attr);

    s32Ret = sample_comm_ai_register(ai_vqev2_attr.mask, g_ai_config.vqe_attr.version, g_ai_config.res_enable);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("sample_comm_ai_register fail\n");
        return XMEDIA_FAILURE;
    }

    s32Ret = sample_comm_ai_start(&g_ai_config);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("sample_comm_ai_start fail\n");
        return XMEDIA_FAILURE;
    }

    s32Ret = sample_comm_ai_set_dev_param(&g_ai_config, AI_PARAM_SET_CODEC_ADC_GAIN, (xmedia_void*)&codec_vol);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("sample_comm_ai_set_dev_param fail\n");
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_aov_audio_ai_stop()
{
    return sample_comm_ai_stop(&g_ai_config);
}

xmedia_s32 sample_aov_audio_aenc_start()
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    voice_format format = VOICE_FORMAT_G711_A;
    audio_codec_type type = CODEC_TYPE_G711;

    s32Ret = sample_comm_get_codec_info(SAMPLE_AOV_AUDIO_AENC_FILE_PATH, &type, &format);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("sample_comm_get_codec_info fail\n");
        return XMEDIA_FAILURE;
    }

    aenc_output_fd = fopen(SAMPLE_AOV_AUDIO_AENC_FILE_PATH, "ab");
    if (aenc_output_fd == XMEDIA_NULL) {
        SAMPLE_ERR("open aenc output file fail errno %s\n",strerror(errno));
        return XMEDIA_FAILURE;
    }

    g_aenc_config.sample_rate = SAMPLE_AOV_AUDIO_AENC_RATE;
    g_aenc_config.channels = 1;     // ai open vqev2
    g_aenc_config.bit_depth = 16;
    g_aenc_config.chn = 0;
    g_aenc_config.type = type;
    g_aenc_config.format = format;
    g_aenc_config.binder_dev = g_ai_config.dev;
    g_aenc_config.binder_src = MOD_ID_AI;

    s32Ret = sample_comm_aenc_register(g_aenc_config.type);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("sample_comm_aenc_register fail\n");
        return XMEDIA_FAILURE;
    }

    s32Ret = sample_comm_aenc_start(&g_aenc_config);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("sample_comm_aenc_start fail\n");
        return XMEDIA_FAILURE;
    }

    s32Ret = sample_comm_aenc_create_output_thread_to_file(&g_aenc_config, &aenc_output_fd);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("sample_comm_aenc_create_output_thread_to_file fail\n");
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_aov_audio_aenc_stop()
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;

    s32Ret = sample_comm_aenc_destroy_output_thread_to_file(&g_aenc_config);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("sample_comm_aenc_destroy_output_thread_to_file fail\n");
        return XMEDIA_FAILURE;
    }

    fflush(aenc_output_fd);
    fsync(fileno(aenc_output_fd));
    fclose(aenc_output_fd);
    aenc_output_fd = XMEDIA_NULL;

    s32Ret = sample_comm_aenc_stop(&g_aenc_config);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("sample_comm_aenc_stop fail\n");
        return XMEDIA_FAILURE;
    }

    s32Ret = sample_comm_aenc_unregister(g_aenc_config.type);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("sample_comm_aenc_unregister fail\n");
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_aov_audio_init()
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;

    s32Ret = xmedia_ai_init();
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_ai_init fail\n");
    }

    s32Ret = xmedia_ao_init();
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_ao_init fail\n");
    }

    s32Ret = xmedia_adec_init();
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_adec_init fail\n");
    }

    s32Ret = xmedia_aenc_init();
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_aenc_init fail\n");
    }

    return s32Ret;
}

xmedia_s32 sample_aov_audio_exit()
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;

    s32Ret = xmedia_ai_exit();
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_ai_exit fail\n");
    }

    s32Ret = xmedia_ao_exit();
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_ao_exit fail\n");
    }

    s32Ret = xmedia_adec_exit();
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_adec_exit fail\n");
    }

    s32Ret = xmedia_aenc_exit();
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_aenc_exit fail\n");
    }

    return s32Ret;
}


xmedia_s32 sample_aov_audio_start_play()
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;

    s32Ret = sample_aov_audio_adec_start();
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("audio adec start fail\n");
        goto ERR_ADEC_START;
    }

    s32Ret = sample_aov_audio_ao_start();
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("audio ao start fail\n");
        goto ERR_AO_START;
    }

    sample_aov_audio_play_thread_start();

    return XMEDIA_SUCCESS;

ERR_AO_START:
    sample_aov_audio_ao_stop();

ERR_ADEC_START:
    sample_aov_audio_adec_stop();

    return XMEDIA_FAILURE;
}

xmedia_s32 sample_aov_audio_stop_play()
{
    sample_aov_audio_play_thread_stop();

    sample_aov_audio_ao_stop();

    sample_aov_audio_adec_stop();

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_aov_audio_start_record()
{
    sample_aov_audio_ai_start();

    sample_aov_audio_aenc_start();

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_aov_audio_stop_record()
{
    sample_aov_audio_aenc_stop();

    sample_aov_audio_ai_stop();

    return XMEDIA_SUCCESS;
}
#endif
