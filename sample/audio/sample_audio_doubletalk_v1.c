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

//#include "sample_comm.h"
#include "xmedia_audio_common.h"
#include "xmedia_audio_ai.h"
#include "xmedia_audio_vqe.h"
#include "xmedia_audio_vqe_enhance_v1.h"

#include "sample_comm_audio.h"

#define SAMPLE_AUDIO_DBG  printf
#define MAX_FILE_NAME_LEN 128
#define FRAME_TIME        10  //ms
#define CRY_ALARMLIMIT 80  // probability greater than which to count as one cry frame
#define CRY_TIMELIMITCNT 3 // number of cry frame count as one cry event
#define CRY_TIMELIMIT 5 // time interval to report cry event
#define CRY_ALARMINTERVAL 3 //alarm interval time to unfreeze

#define AI_DEFAULT_MIC_VOL 20

typedef struct user_input_args {
    xmedia_char* sample;
    audio_sample_rate sample_rate;
    audio_sample_rate usr_sample_rate;
    audio_channel channels;
    xmedia_bool binder_mode;
    audio_sample_rate file_samplerate;

    char input_file_name[MAX_FILE_NAME_LEN];
    char output_file_name[MAX_FILE_NAME_LEN];
    FILE *input_fd[2];
    FILE *output_fd[2];
} user_input_args;

static user_input_args g_user_input_args;
static sample_ai_config g_ai_config;
static sample_ao_config g_ao_config;
static sample_adec_config g_adec_config;
static sample_aenc_config g_aenc_config;
static ai_vqe_attr_v1 ai_vqev1_attr;
static ao_vqe_attr_v1 ao_vqev1_attr;

static xmedia_void load_ai_vqev1_attr(ai_vqe_attr* attr)
{
    ai_vqe_attr_v1 *vqev1_attr = &ai_vqev1_attr;

    attr->version = AI_VQE_ENHANCE_ATTR_VERSION_1;
    attr->work_sample_rate = (g_user_input_args.sample_rate == AUDIO_SAMPLE_RATE_8000) ? AUDIO_SAMPLE_RATE_8000 : AUDIO_SAMPLE_RATE_16000;
    attr->in_channels = 1;
    attr->attr = (xmedia_void*)&ai_vqev1_attr;
    vqev1_attr->mask = VQE_V1_MASK_AGC | VQE_V1_MASK_ANR | VQE_V1_MASK_AEC | VQE_V1_MASK_HPF;

    vqev1_attr->aec_attr.mode = VQE_V1_USR_MODE_AUTO;
    vqev1_attr->aec_attr.cng_enable = XMEDIA_TRUE;
    vqev1_attr->aec_attr.freq_boundary = 1800;
    vqev1_attr->aec_attr.low_suppress_level = 0x7;
    vqev1_attr->aec_attr.high_suppress_level = 0x5;
    vqev1_attr->aec_attr.low_enhance_voice_protect_level = 0;
    vqev1_attr->aec_attr.high_enhance_voice_protect_level = 0;

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

static xmedia_void load_ao_vqev1_attr(ao_vqe_attr* attr)
{
    ao_vqe_attr_v1 *vqev1_attr = &ao_vqev1_attr;

    attr->version = AO_VQE_ENHANCE_ATTR_VERSION_1;
    attr->attr = (xmedia_void*)&ao_vqev1_attr;
    vqev1_attr->mask = VQE_V1_MASK_AGC | VQE_V1_MASK_ANR | VQE_V1_MASK_HPF;

    vqev1_attr->agc_attr.mode = VQE_V1_USR_MODE_AUTO;
    vqev1_attr->agc_attr.target_level = -6;
    vqev1_attr->agc_attr.max_boost_gain = 20;
    vqev1_attr->agc_attr.noise_floor = -65;
    vqev1_attr->agc_attr.ratio = 10;
    vqev1_attr->agc_attr.attack_time = 20;
    vqev1_attr->agc_attr.release_time =20;

    vqev1_attr->anr_attr.mode = VQE_V1_USR_MODE_AUTO;
    vqev1_attr->anr_attr.usr_scene = VQE_V1_ANR_SCENE_NORMAL;
    vqev1_attr->anr_attr.nr_mode = VQE_V1_NR_MODE_SPEECH;
    vqev1_attr->anr_attr.max_suppress_gain = 6;
    vqev1_attr->anr_attr.suppress_level = 0x2;
    vqev1_attr->anr_attr.nonstationary_suppress_level = 0x2;

    vqev1_attr->hpf_attr.mode = VQE_V1_USR_MODE_AUTO;
    vqev1_attr->hpf_attr.freq = VQE_V1_HPF_FREQ_80;

    memset(&vqev1_attr->eq_attr, 0, sizeof(vqe_eq_attr_v1));
}

static xmedia_s32 sample_ai_config_init(xmedia_void)
{
    g_ai_config.dev = XMEDIA_AI_DEV_ADC0;

    sample_comm_ai_get_default_attr(g_ai_config.dev, &g_ai_config.dev_attr);
    g_ai_config.dev_attr.channels = g_user_input_args.channels;
    g_ai_config.dev_attr.sample_rate = g_user_input_args.sample_rate;
    g_ai_config.dev_attr.pcm_samples_per_frame = FRAME_TIME * g_user_input_args.sample_rate / 1000;
    g_ai_config.dev_attr.mode = AUDIO_DEV_WORK_MODE_QUEUE;
    g_ai_config.chn_attr.sample_rate = g_user_input_args.usr_sample_rate;
    g_ai_config.source = AI_EC_SOURCE_AO_DAC0;
    g_ai_config.ec_chn_id = 0;
    g_ai_config.vqe_enable = XMEDIA_TRUE;
    g_ai_config.res_enable = XMEDIA_FALSE;
    if ((g_ai_config.dev_attr.sample_rate != AUDIO_SAMPLE_RATE_8000 && g_ai_config.dev_attr.sample_rate != AUDIO_SAMPLE_RATE_16000)
        || (g_ai_config.chn_attr.sample_rate != g_ai_config.dev_attr.sample_rate)) {
        g_ai_config.res_enable = XMEDIA_TRUE;
    }
    load_ai_vqev1_attr(&g_ai_config.vqe_attr);

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_ao_config_init(xmedia_void)
{
    g_ao_config.dev = XMEDIA_AO_DEV_DAC0;

    sample_comm_ao_get_default_attr(g_ao_config.dev, &g_ao_config.dev_attr);
    g_ao_config.dev_attr.channels = 1;
    g_ao_config.dev_attr.sample_rate = g_user_input_args.sample_rate;
    g_ao_config.dev_attr.pcm_samples_per_frame = FRAME_TIME * g_user_input_args.sample_rate / 1000;
    g_ao_config.dev_attr.mode = AUDIO_DEV_WORK_MODE_MONO;

    //AAC/MP3不知道输入采样率，默认注册重采样，其他格式根据输入采样率决定是否注册
    g_ao_config.res_enable = XMEDIA_FALSE;
    if ((g_adec_config.type == CODEC_TYPE_AAC || g_adec_config.type == CODEC_TYPE_MP3) ||
        (g_ao_config.dev_attr.sample_rate != AUDIO_SAMPLE_RATE_8000 && g_ao_config.dev_attr.sample_rate != AUDIO_SAMPLE_RATE_16000)
        || (g_ao_config.frame_info.frm_rate != g_ao_config.dev_attr.sample_rate)) {
        g_ao_config.res_enable = XMEDIA_TRUE;
    }
    g_ao_config.vqe_enable = XMEDIA_TRUE;
    load_ao_vqev1_attr(&g_ao_config.vqe_attr);

    if (g_user_input_args.binder_mode == XMEDIA_TRUE) {
        g_ao_config.binder_src = MOD_ID_ADEC;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_adec_config_init(xmedia_void)
{
    voice_format format = VOICE_FORMAT_G711_A;
    audio_codec_type type = CODEC_TYPE_G711;

    sample_comm_get_codec_info(g_user_input_args.input_file_name, &type, &format);

    g_adec_config.sample_rate = g_user_input_args.file_samplerate;
    g_adec_config.chn = 0;
    g_adec_config.type = type;
    g_adec_config.format = format;

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_aenc_config_init(xmedia_void)
{
    voice_format format = VOICE_FORMAT_G711_A;
    audio_codec_type type = CODEC_TYPE_G711;

    sample_comm_get_codec_info(g_user_input_args.output_file_name, &type, &format);

    g_aenc_config.sample_rate = g_user_input_args.usr_sample_rate;
    g_aenc_config.channels = g_user_input_args.channels;
    g_aenc_config.bit_depth = 16;
    g_aenc_config.chn = 0;
    g_aenc_config.type = type;
    g_aenc_config.format = format;
    g_aenc_config.binder_dev = g_ai_config.dev;
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

    ret = sample_ai_config_init();
    CHECK_RET(ret, "sample_ai_config_init");

    ret = sample_comm_ai_register(ai_vqev1_attr.mask, g_ai_config.vqe_attr.version, g_ai_config.res_enable);
    CHECK_RET(ret, "sample_comm_ai_register");

    ret = sample_comm_ai_start(&g_ai_config);
    CHECK_RET(ret, "sample_comm_ai_start");

    if (g_ai_config.dev == XMEDIA_AI_DEV_ADC0) {
        ret = sample_comm_ai_set_dev_param(&g_ai_config, AI_PARAM_SET_CODEC_ADC_GAIN, (xmedia_void*)&vol);
        CHECK_RET(ret, "sample_comm_ai_set_dev_param");
    }

    if (g_user_input_args.binder_mode == XMEDIA_FALSE) {
        ret = sample_comm_ai_create_output_thread_to_aenc(&g_ai_config);
        CHECK_RET(ret, "sample_comm_ai_create_data_thread");
    }
    return ret;
}

static xmedia_s32 sample_start_ao()
{
    xmedia_s32 ret;

    ret = sample_ao_config_init();
    CHECK_RET(ret, "sample_ao_config_init");

    ret = sample_comm_ao_register(ao_vqev1_attr.mask, g_ao_config.vqe_attr.version, g_ao_config.res_enable);
    CHECK_RET(ret, "sample_comm_ai_register");

    ret = sample_comm_ao_start(&g_ao_config);

    return ret;
}

static xmedia_s32 sample_start_adec()
{
    xmedia_s32 ret;

    ret = sample_adec_config_init();
    CHECK_RET(ret, "sample_adec_config_init");

    ret = sample_comm_adec_register(g_adec_config.type);
    CHECK_RET(ret, "sample_comm_adec_register");

    ret = sample_comm_adec_start(&g_adec_config);
    CHECK_RET(ret, "sample_comm_adec_start");

    ret = sample_comm_adec_create_input_thread_from_file(&g_adec_config, g_user_input_args.input_fd);
    CHECK_RET(ret, "sample_comm_adec_create_out_to_ao_thread");

    if (g_user_input_args.binder_mode == XMEDIA_FALSE) {
        ret = sample_comm_adec_create_output_thread_to_ao(&g_adec_config, g_ao_config.dev, g_adec_config.chn);
        CHECK_RET(ret, "sample_comm_adec_create_out_to_ao_thread");
    }

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
    if (g_user_input_args.binder_mode == XMEDIA_FALSE) {
        ret = sample_comm_ai_destroy_output_thread_to_aenc(&g_ai_config);
        CHECK_RET(ret, "sample_comm_ai_destroy_data_thread");
    }

    ret = sample_comm_ai_stop(&g_ai_config);
    CHECK_RET(ret, "sample_comm_ai_stop");

    return ret;
}

static xmedia_s32 sample_stop_ao()
{
    xmedia_s32 ret;

    ret = sample_comm_ao_stop(&g_ao_config);
    CHECK_RET(ret, "sample_comm_ao_stop");

    return ret;
}

static xmedia_s32 sample_stop_adec()
{
    xmedia_s32 ret;

    ret = sample_comm_adec_destroy_input_thread_from_file(&g_adec_config);
    CHECK_RET(ret, "sample_comm_adec_destroy_input_thread_from_file");

    if (g_user_input_args.binder_mode == XMEDIA_FALSE) {
        ret = sample_comm_adec_destroy_output_thread_to_ao(&g_adec_config);
        CHECK_RET(ret, "sample_comm_adec_destroy_output_thread_to_ao");
    }

    ret = sample_comm_adec_stop(&g_adec_config);
    CHECK_RET(ret, "sample_comm_adec_stop");

    ret = sample_comm_adec_unregister(g_adec_config.type);
    CHECK_RET(ret, "sample_comm_adec_unregister");

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


xmedia_s32 sample_audio_doubletalk(xmedia_void)
{
    xmedia_s32 ret;

    ret = sample_init_audio();
    if (ret != XMEDIA_SUCCESS) {
        goto ERR_INIT_AUDIO;
    }

    ret = sample_start_adec();
    if (ret != XMEDIA_SUCCESS) {
        goto ERR_START_ADEC;
    }

    ret = sample_start_ao();
    if (ret != XMEDIA_SUCCESS) {
        goto ERR_START_AO;
    }

    ret = sample_start_ai();
    if (ret != XMEDIA_SUCCESS) {
        goto ERR_START_AI;
    }

    ret = sample_start_aenc();
    if (ret != XMEDIA_SUCCESS) {
        goto ERR_START_AENC;
    }

    while(1) {
        xmedia_s32 val;
        xmedia_char input_cmd[32];
        SAMPLE_AUDIO_DBG("\nInput 'q' ENTER to exit\n");
        SAMPLE_AUDIO_DBG("Input 'svXX' to set ai vol to xx dB\n");
        SAMPLE_AUDIO_DBG("Input 'soXX' to set ao vol to xx dB\n");
        SAMPLE_AUDIO_DBG("Input 'gv' to get ai vol\n");

        fgets(input_cmd, (sizeof(input_cmd) - 1), stdin);
        if (input_cmd[0] == 's' && input_cmd[1] == 'v' ) {
            val = atoi(input_cmd + 2);
            ret = sample_comm_ai_set_volume(&g_ai_config, val);
            SAMPLE_AUDIO_DBG("\nSet ai volume %d return 0x%x\n", val, ret);
        } else if (input_cmd[0] == 's' && input_cmd[1] == 'o') {
            xmedia_s32 vol = atoi(input_cmd + 2);
            ret = sample_comm_ao_set_volume(&g_ao_config, vol);
            SAMPLE_AUDIO_DBG("\nset ao chn volume(%d) return 0x%x\n", vol, ret);
        } else if ((input_cmd[0] == 'q') || (input_cmd[0] == 'Q')) {
            SAMPLE_AUDIO_DBG("Ready to exit\n");
            break;
        }
    }
    sample_stop_aenc();
ERR_START_AENC:
    sample_stop_ai();
ERR_START_AI:
    sample_stop_ao();
ERR_START_AO:
    sample_stop_adec();
ERR_START_ADEC:
ERR_INIT_AUDIO:
    sample_deinit_audio();
    return ret;
}

static xmedia_void user_args_usage(user_input_args* args)
{
    SAMPLE_AUDIO_DBG("usage:     %s [samplerate] [usr_samplerate] [channels] [binder_mode] [outfile] [infile] [infile samplerate]\n", args->sample);
    SAMPLE_AUDIO_DBG("dev_rate:  dev samplerate(8000/16000/32000/48000)\n");
    SAMPLE_AUDIO_DBG("usr_rate:  out file samplerate(8000/16000/32000/48000)\n");
    SAMPLE_AUDIO_DBG("dev_chn:   dev chn cnt(1/2/8)\n");
    SAMPLE_AUDIO_DBG("mode:      binder mode(0/1)\n");
    SAMPLE_AUDIO_DBG("out_file:  aenc encoder out file(.g711a/.g711u/.g726/.aac/.mp3/.pcm/.adpcm)\n");
    SAMPLE_AUDIO_DBG("in_file:   input file for adec(.g711a/.g711u/.g726/.aac/.mp3/.pcm/.adpcm)\n");
    SAMPLE_AUDIO_DBG("file_rate: input file samplerate for voice\n");

    SAMPLE_AUDIO_DBG("examples:\n");
    SAMPLE_AUDIO_DBG("           %s 8000 8000 1 1 out.g711a in.aac 8000\n", args->sample);
    SAMPLE_AUDIO_DBG("           %s 16000 16000 1 1 out.g711a in.aac 16000\n", args->sample);
}

static xmedia_s32 user_args_init(int argc, char* argv[], user_input_args* user_in_args)
{
    user_in_args->sample = argv[0];

    if ((argc >= 2) && (strcmp(argv[1], "-h")) == 0) {
        user_args_usage(user_in_args);
        return XMEDIA_FAILURE;
    }

    if (argc != 8) {
        printf("Invalid argc:%d\n", argc);
        user_args_usage(user_in_args);
        return XMEDIA_FAILURE;
    }

    user_in_args->sample_rate = atoi(argv[1]);
    user_in_args->usr_sample_rate = atoi(argv[2]);
    user_in_args->channels = atoi(argv[3]);
    user_in_args->binder_mode = (atoi(argv[4]) == 0 ? XMEDIA_FALSE : XMEDIA_TRUE);
    memcpy(user_in_args->output_file_name, argv[5], strlen(argv[5]));
    if (strcmp(user_in_args->output_file_name, "null")) {
        user_in_args->output_fd[0] = fopen(user_in_args->output_file_name, "w+");
        if (user_in_args->output_fd[0] == NULL) {
            printf("Open %s failed\n", user_in_args->output_file_name);
            return XMEDIA_FAILURE;
         }
    }

    memcpy(user_in_args->input_file_name, argv[6], strlen(argv[6]));
    if (strcmp(user_in_args->input_file_name, "null")) {
        user_in_args->input_fd[0] = fopen(user_in_args->input_file_name, "rb");
        if (user_in_args->input_fd[0] == NULL) {
            printf("Open %s failed\n", user_in_args->input_file_name);
            fclose(user_in_args->output_fd[0]);
            return XMEDIA_FAILURE;
        }
    }

    user_in_args->file_samplerate = atoi(argv[7]);

    SAMPLE_AUDIO_DBG("get user args: \n");
    SAMPLE_AUDIO_DBG("  samplerate: %d\n", user_in_args->sample_rate);
    SAMPLE_AUDIO_DBG("  user samplerate: %d\n", user_in_args->usr_sample_rate);
    SAMPLE_AUDIO_DBG("  chnnels: %d\n", user_in_args->channels);
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

    if (user_in_args->input_fd[1] != NULL) {
        fclose(user_in_args->input_fd[1]);
        user_in_args->input_fd[1] = NULL;
    }

    if (user_in_args->input_fd[0] != NULL) {
        fclose(user_in_args->input_fd[0]);
        user_in_args->input_fd[0] = NULL;
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

    ret = sample_audio_doubletalk();
    if (ret != XMEDIA_SUCCESS) {
        printf("sample_auido_ai failed(0x%x)\n", ret);
    }

    user_args_deinit(&g_user_input_args);
    SAMPLE_AUDIO_DBG("exit %s\n", g_user_input_args.sample);
    return ret;
}


