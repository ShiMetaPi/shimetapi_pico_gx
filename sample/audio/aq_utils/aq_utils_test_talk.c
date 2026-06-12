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
#include "xmedia_audio_vqe_enhance_v2.h"
#include "sample_comm_audio.h"
#include "config_parameter/config_load_parameter.h"

#define SAMPLE_AUDIO_DBG  printf
#define MAX_FILE_NAME_LEN 128
#define FRAME_TIME        10  //ms

typedef struct user_input_args {
    xmedia_char* sample;
    audio_sample_rate sample_rate;
    audio_sample_rate usr_sample_rate;
    audio_channel channels;
    xmedia_bool binder_mode;
    audio_sample_rate file_samplerate;

    char input_file_name[MAX_FILE_NAME_LEN];
    char output_file_name[MAX_FILE_NAME_LEN];
    char aivqe_config_file_name[MAX_FILE_NAME_LEN];
    char aovqe_config_file_name[MAX_FILE_NAME_LEN];
    FILE *input_fd[2];
    FILE *output_fd[2];
} user_input_args;

static user_input_args g_user_input_args;
static sample_ai_config g_ai_config;
static sample_ao_config g_ao_config;
static sample_adec_config g_adec_config;
static sample_aenc_config g_aenc_config;

static user_base_config base_config;
static ai_vqe_attr_v1 ai_v1_attr;
static ai_vqe_attr_v2 ai_v2_attr;
static ao_vqe_attr_v1 ao_v1_attr;
static ao_vqe_attr_v2 ao_v2_attr;

static xmedia_s32 sample_ai_config_init(xmedia_void)
{
    xmedia_s32 ret;
    g_ai_config.dev = XMEDIA_AI_DEV_ADC0;
    sample_comm_ai_get_default_attr(g_ai_config.dev, &g_ai_config.dev_attr);
    g_ai_config.dev_attr.channels = base_config.in_chn_cnt;
    g_ai_config.dev_attr.sample_rate = base_config.work_samplerate;
    g_ai_config.dev_attr.pcm_samples_per_frame = base_config.sample_per_frame;
    g_ai_config.dev_attr.pcm_frame_num = base_config.frame_nums;
    g_ai_config.dev_attr.mode = AUDIO_DEV_WORK_MODE_QUEUE;
    g_ai_config.chn_attr.sample_rate = base_config.work_samplerate;
    g_ai_config.source = AI_EC_SOURCE_AO_DAC0;
    g_ai_config.ec_chn_id = 0;
    g_ai_config.res_enable = XMEDIA_FALSE;
    if ((g_ai_config.dev_attr.sample_rate != AUDIO_SAMPLE_RATE_8000 && g_ai_config.dev_attr.sample_rate != AUDIO_SAMPLE_RATE_16000)
        || (g_ai_config.chn_attr.sample_rate != g_ai_config.dev_attr.sample_rate)) {
        g_ai_config.res_enable = XMEDIA_TRUE;
    }

    g_ai_config.vqe_enable = XMEDIA_TRUE;
    if (base_config.vqe_version == AI_VQE_ENHANCE_ATTR_VERSION_1) {
        g_ai_config.vqe_attr.attr = &ai_v1_attr;
    } else if (base_config.vqe_version == AI_VQE_ENHANCE_ATTR_VERSION_2) {
        g_ai_config.vqe_attr.attr = &ai_v2_attr;
    }
    g_ai_config.vqe_attr.version = base_config.vqe_version;
    g_ai_config.vqe_attr.work_sample_rate = base_config.work_samplerate;
    g_ai_config.vqe_attr.in_channels = base_config.in_chn_cnt;
    ret = load_aivqe_parameter(g_user_input_args.aivqe_config_file_name, &g_ai_config.vqe_attr);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }
    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_ao_config_init(xmedia_void)
{
    xmedia_s32 ret;
    g_ao_config.dev = XMEDIA_AO_DEV_DAC0;

    sample_comm_ao_get_default_attr(g_ao_config.dev, &g_ao_config.dev_attr);
    g_ao_config.dev_attr.channels = base_config.out_chn_cnt;
    g_ao_config.dev_attr.sample_rate = base_config.work_samplerate;
    g_ao_config.dev_attr.pcm_samples_per_frame = base_config.sample_per_frame;
    g_ao_config.dev_attr.pcm_frame_max_num = base_config.frame_nums;
    g_ao_config.dev_attr.mode = AUDIO_DEV_WORK_MODE_QUEUE;
    //AAC/MP3不知道输入采样率，默认注册重采样，其他格式根据输入采样率决定是否注册
    g_ao_config.res_enable = XMEDIA_FALSE;
    if ((g_adec_config.type == CODEC_TYPE_AAC || g_adec_config.type == CODEC_TYPE_MP3) ||
        (g_ao_config.dev_attr.sample_rate != AUDIO_SAMPLE_RATE_8000 && g_ao_config.dev_attr.sample_rate != AUDIO_SAMPLE_RATE_16000)
        || (g_ao_config.frame_info.frm_rate != g_ao_config.dev_attr.sample_rate)) {
        g_ao_config.res_enable = XMEDIA_TRUE;
    }

    g_ao_config.vqe_enable = XMEDIA_TRUE;
    g_ao_config.vqe_attr.version = base_config.vqe_version;
    if (base_config.vqe_version == AI_VQE_ENHANCE_ATTR_VERSION_1) {
        g_ao_config.vqe_attr.attr = &ao_v1_attr;
    } else if (base_config.vqe_version == AI_VQE_ENHANCE_ATTR_VERSION_2) {
        g_ao_config.vqe_attr.attr = &ao_v2_attr;
    }
    ret = load_aovqe_parameter(g_user_input_args.aovqe_config_file_name, &g_ao_config.vqe_attr);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

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

    g_adec_config.sample_rate = base_config.work_samplerate;
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
    g_aenc_config.sample_rate = base_config.work_samplerate;
    g_aenc_config.channels = base_config.out_chn_cnt;
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

static xmedia_s32 sample_start_ai()
{
    xmedia_s32 ret;

    ret = sample_ai_config_init();
    CHECK_RET(ret, "sample_ai_config_init");

    ret = sample_comm_ai_register((g_ai_config.vqe_attr.version == AI_VQE_ENHANCE_ATTR_VERSION_1) ? ai_v1_attr.mask : ai_v2_attr.mask, g_ai_config.vqe_attr.version, g_ai_config.res_enable);
    CHECK_RET(ret, "sample_comm_ai_register");

    ret = sample_comm_ai_start(&g_ai_config);
    CHECK_RET(ret, "sample_comm_ai_start");

    if (g_user_input_args.binder_mode == XMEDIA_FALSE) {
        ret = sample_comm_ai_create_output_thread_to_aenc(&g_ai_config);
        CHECK_RET(ret, "sample_comm_ai_create_data_thread");
    }

    if (g_ai_config.dev == XMEDIA_AI_DEV_ADC0) {
        ret = sample_comm_ai_set_dev_param(&g_ai_config, AI_PARAM_SET_CODEC_ADC_GAIN, (xmedia_void*)&base_config.codec_input_vol);
        if (ret != XMEDIA_SUCCESS) {
            printf("sample_comm_ai_set_dev_param failed(0x%x)\n", ret);
            goto ERR;
        }
    } else if (g_ai_config.dev == XMEDIA_AI_DEV_PDM0) {
        ret = sample_comm_ai_set_dev_param(&g_ai_config, AI_PARAM_SET_CODEC_MIC_GAIN, (xmedia_void*)&base_config.codec_input_vol);
        if (ret != XMEDIA_SUCCESS) {
            printf("sample_comm_ai_set_dev_param failed(0x%x)\n", ret);
            goto ERR;
        }
    }

    ret = sample_comm_ai_set_volume(&g_ai_config, base_config.ai_vqe_vol);
    if (ret != XMEDIA_SUCCESS) {
        printf("sample_comm_ai_set_volume failed(0x%x)\n", ret);
        goto ERR;
    }
    return XMEDIA_SUCCESS;
ERR:
    sample_stop_ai();
    return ret;
}

static xmedia_s32 sample_start_ao()
{
    xmedia_s32 ret;

    ret = sample_ao_config_init();
    CHECK_RET(ret, "sample_ao_config_init");

    ret = sample_comm_ao_register((g_ao_config.vqe_attr.version == AO_VQE_ENHANCE_ATTR_VERSION_1) ? ao_v1_attr.mask : ao_v2_attr.mask, g_ao_config.vqe_attr.version, g_ao_config.res_enable);
    CHECK_RET(ret, "sample_comm_ai_register");

    ret = sample_comm_ao_start(&g_ao_config);
    CHECK_RET(ret, "sample_comm_ao_start");

    ret = sample_comm_ao_set_volume(&g_ao_config, base_config.ao_vqe_vol);
    CHECK_RET(ret, "sample_comm_ao_set_volume");
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
        int val;
        xmedia_char input_cmd[32];
        SAMPLE_AUDIO_DBG("\nInput 'q' ENTER to exit\n");
        SAMPLE_AUDIO_DBG("Input 'sivXX' to set input vol to xx dB\n");
        SAMPLE_AUDIO_DBG("Input 'giv' to get input vol\n");
        SAMPLE_AUDIO_DBG("Input 'sovXX' to set ao vqe vol to xx\n");
        SAMPLE_AUDIO_DBG("Input 'gov' to get ao vqe vol to xx\n");

        fgets(input_cmd, (sizeof(input_cmd) - 1), stdin);
        if (input_cmd[0] == 's' && input_cmd[1] == 'i' && input_cmd[2] == 'v') {
            val = atoi(input_cmd + 3);
            ret = sample_comm_ai_set_dev_param(&g_ai_config, AI_PARAM_SET_CODEC_ADC_GAIN, (xmedia_void*)&val);
            SAMPLE_AUDIO_DBG("\nSet codec input volume %d return 0x%x\n", val, ret);
        } else if (input_cmd[0] == 'g' && input_cmd[1] == 'i' && input_cmd[2] == 'v') {
            ret = sample_comm_ai_set_dev_param(&g_ai_config, AI_PARAM_GET_CODEC_ADC_GAIN, (xmedia_void*)&val);
            SAMPLE_AUDIO_DBG("\nGet codec input volume(%d) return 0x%x\n", val, ret);
        } else if (input_cmd[0] == 's' && input_cmd[1] == 'o' && input_cmd[2] == 'v') {
            val = atoi(input_cmd + 3);
            ret = sample_comm_ao_set_volume(&g_ao_config, val);
            SAMPLE_AUDIO_DBG("\nSet ao vqe volume %d return 0x%x\n", val, ret);
        } else if (input_cmd[0] == 'g' && input_cmd[1] == 'o' && input_cmd[2] == 'v') {
            ret = sample_comm_ao_get_volume(&g_ao_config, &val);
            SAMPLE_AUDIO_DBG("\nGet ao vqe volume(%d) return 0x%x\n", val, ret);
        } else if (input_cmd[0] == 'q') {
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
    SAMPLE_AUDIO_DBG("Usage:./aq_utils_test_talk [AIvqeConfig] [AOvqeConfig] [RemoteFile] [OutFile]\n");
    SAMPLE_AUDIO_DBG("Parameter comment as follows:\n");
    SAMPLE_AUDIO_DBG("  [AIvqeConfig]:  AI VQE config file\n");
    SAMPLE_AUDIO_DBG("  [AOvqeConfig]:  AO VQE config file\n");
    SAMPLE_AUDIO_DBG("  [RemoteFile]:   speaker playback file\n");
    SAMPLE_AUDIO_DBG("  [OutFile]:      vqe out file\n");

    SAMPLE_AUDIO_DBG("Examples as follows:\n");
    SAMPLE_AUDIO_DBG("    ./aq_utils_test_talk ./config_parameter/talk_ai_vqev2_16k_2ch.ini ./config_parameter/talk_ao_vqev2_16k_1ch.ini spk_play.pcm vqe_out.pcm\n");
}

static xmedia_s32 user_args_init(int argc, char* argv[], user_input_args* user_in_args)
{
    xmedia_s32 ret;
    user_in_args->sample = argv[0];

    if ((argc >= 5) && (strcmp(argv[1], "-h")) == 0) {
        user_args_usage(user_in_args);
        return XMEDIA_FAILURE;
    }

    if (argc != 5) {
        printf("Invalid argc:%d\n", argc);
        user_args_usage(user_in_args);
        return XMEDIA_FAILURE;
    }

    memcpy(user_in_args->aivqe_config_file_name, argv[1], strlen(argv[1]));
    memcpy(user_in_args->aovqe_config_file_name, argv[2], strlen(argv[2]));

    user_in_args->binder_mode = XMEDIA_TRUE;
     memcpy(user_in_args->input_file_name, argv[3], strlen(argv[3]));
    if (strcmp(user_in_args->input_file_name, "null")) {
        user_in_args->input_fd[0] = fopen(user_in_args->input_file_name, "rb");
        if (user_in_args->input_fd[0] == NULL) {
            printf("Open %s failed\n", user_in_args->input_file_name);
            return XMEDIA_FAILURE;
        }
    }
    memcpy(user_in_args->output_file_name, argv[4], strlen(argv[4]));
    if (strcmp(user_in_args->output_file_name, "null")) {
        user_in_args->output_fd[0] = fopen(user_in_args->output_file_name, "w+");
        if (user_in_args->output_fd[0] == NULL) {
            fclose(user_in_args->input_fd[0]);
            printf("Open %s failed\n", user_in_args->output_file_name);
            return XMEDIA_FAILURE;
        }
    }

    if (user_in_args->output_fd[0] != NULL) {
        SAMPLE_AUDIO_DBG("  mic0_file_name:%s\n", user_in_args->output_file_name);
    }

    ret = load_base_parameter(g_user_input_args.aivqe_config_file_name, &base_config);
    if (ret != XMEDIA_SUCCESS) {
        fclose(user_in_args->input_fd[0]);
        fclose(user_in_args->output_fd[0]);
        SAMPLE_AUDIO_DBG("load_base_parameter failed(0x%x)\n", ret);
        return ret;
    }
    return ret;
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
