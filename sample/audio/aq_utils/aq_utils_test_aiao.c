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
#define DEFAULT_AI_FRAME_NUM 50

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
static xmedia_s32 g_codec_vol;
static xmedia_s32 g_ao_vol;

static xmedia_s32 sample_ai_config_init(xmedia_void)
{
    g_ai_config.dev = XMEDIA_AI_DEV_ADC0;
    sample_comm_ai_get_default_attr(g_ai_config.dev, &g_ai_config.dev_attr);
    g_ai_config.dev_attr.channels = g_user_input_args.channels;
    g_ai_config.dev_attr.sample_rate = g_user_input_args.sample_rate;
    g_ai_config.dev_attr.pcm_samples_per_frame = g_user_input_args.sample_rate / 100;
    g_ai_config.dev_attr.pcm_frame_num = DEFAULT_AI_FRAME_NUM;
    g_ai_config.dev_attr.mode = AUDIO_DEV_WORK_MODE_QUEUE;
    g_ai_config.chn_attr.sample_rate = g_user_input_args.sample_rate;
    g_ai_config.chn_attr.interleaved = XMEDIA_TRUE;
    g_ai_config.source = AI_EC_SOURCE_AO_DAC0;
    g_ai_config.ec_chn_id = 0;
    g_ai_config.vqe_enable = XMEDIA_FALSE;
    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_ao_config_init(xmedia_void)
{
    g_ao_config.dev = XMEDIA_AO_DEV_DAC0;

    sample_comm_ao_get_default_attr(g_ao_config.dev, &g_ao_config.dev_attr);
    g_ao_config.dev_attr.channels = g_user_input_args.channels;
    g_ao_config.dev_attr.sample_rate = g_user_input_args.sample_rate;
    g_ao_config.dev_attr.pcm_samples_per_frame = g_user_input_args.sample_rate / 100;
    g_ao_config.dev_attr.mode = AUDIO_DEV_WORK_MODE_QUEUE;
    g_ao_config.vqe_enable = XMEDIA_FALSE;

    g_ao_config.frame_info.frm_rate = g_user_input_args.sample_rate;
    g_ao_config.frame_info.frm_chn = g_user_input_args.channels;
    g_ao_config.frame_info.frm_bit = AUDIO_BIT_DEPTH_16;
    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_init_audio()
{
    return sample_comm_audio_init();
}

static xmedia_s32 sample_start_ai()
{
    xmedia_s32 ret;

    ret = sample_ai_config_init();
    CHECK_RET(ret, "sample_ai_config_init");

    ret = sample_comm_ai_start(&g_ai_config);
    CHECK_RET(ret, "sample_comm_ai_start");

    ret = sample_comm_ai_create_output_thread_to_file(&g_ai_config, g_user_input_args.output_fd);
    CHECK_RET(ret, "sample_comm_ai_create_output_thread_to_file");
    return ret;
}

static xmedia_s32 sample_start_ao()
{
    xmedia_s32 ret;

    ret = sample_ao_config_init();
    CHECK_RET(ret, "sample_ao_config_init");

    ret = sample_comm_ao_start(&g_ao_config);

    sample_comm_ao_create_input_thread_from_file(&g_ao_config, g_user_input_args.input_fd);
    return ret;
}

static xmedia_s32 sample_deinit_audio()
{
    return sample_comm_audio_exit();
}

static xmedia_s32 sample_stop_ai()
{
    xmedia_s32 ret;
    ret = sample_comm_ai_destroy_output_thread_to_file(&g_ai_config);
    CHECK_RET(ret, "sample_comm_ai_destroy_data_thread");

    ret = sample_comm_ai_stop(&g_ai_config);
    CHECK_RET(ret, "sample_comm_ai_stop");

    return ret;
}

static xmedia_s32 sample_stop_ao()
{
    xmedia_s32 ret;
    ret = sample_comm_ao_destroy_input_thread_from_file(&g_ao_config);
    CHECK_RET(ret, "sample_comm_ao_create_input_thread_from_file");

    ret = sample_comm_ao_stop(&g_ao_config);
    CHECK_RET(ret, "sample_comm_ao_stop");

    return ret;
}

static xmedia_s32 sample_audio_aiao(xmedia_void)
{
    xmedia_s32 ret;

    ret = sample_init_audio();
    if (ret != XMEDIA_SUCCESS) {
        goto ERR_INIT_AUDIO;
    }

    ret = sample_start_ao();
    if (ret != XMEDIA_SUCCESS) {
        goto ERR_START_AO;
    }

    ret = sample_comm_ao_set_volume(&g_ao_config, g_ao_vol);
    if (ret != XMEDIA_SUCCESS) {
        goto ERR_START_AO;
    }

    ret = sample_start_ai();
    if (ret != XMEDIA_SUCCESS) {
        goto ERR_START_AI;
    }

    ret = sample_comm_ai_set_dev_param(&g_ai_config, AI_PARAM_SET_CODEC_ADC_GAIN, (xmedia_void*)&g_codec_vol);
    if (ret != XMEDIA_SUCCESS) {
        goto ERR_SET_GAIN;
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
ERR_SET_GAIN:
    sample_stop_ai();
ERR_START_AI:
    sample_stop_ao();
ERR_START_AO:
ERR_INIT_AUDIO:
    sample_deinit_audio();
    return ret;
}

static xmedia_void user_args_usage(user_input_args* args)
{
    SAMPLE_AUDIO_DBG("Usage:./aq_utils_test_aiao [Samplerate] [Channel] [CodecInputVol] [AoVol] [MicFile] [SpkFile]\n");
    SAMPLE_AUDIO_DBG("Parameter comment as follows:\n");
    SAMPLE_AUDIO_DBG("  [Samplerate]: Work Samplerate, 8000 or 16000 \n");
    SAMPLE_AUDIO_DBG("  [Channel]: dev chn, 1 or 2 \n");
    SAMPLE_AUDIO_DBG("  [CodecInputVol]: audiocodec mic input volume, range:[-95~48] \n");
    SAMPLE_AUDIO_DBG("  [AoVol]: ao vqe volume, range:[-81~18] \n");
    SAMPLE_AUDIO_DBG("  [MicFile]: file for saving mic data\n");
    SAMPLE_AUDIO_DBG("  [SpkFile]: file for speaker playback\n");

    SAMPLE_AUDIO_DBG("Examples as follows:\n");
    SAMPLE_AUDIO_DBG("    ./aq_utils_test_aiao 16000 1 20 0 /tmp/mic.pcm /tmp/test_signal_loopback_16k_1ch_16bit.pcm\n");
    SAMPLE_AUDIO_DBG("    ./aq_utils_test_aiao 8000  2 20 0 /tmp/mic.pcm /tmp/test_signal_loopback_8k_1ch_16bit.pcm\n");
}

static xmedia_s32 user_args_init(int argc, char* argv[], user_input_args* user_in_args)
{
    user_in_args->sample = argv[0];

    if ((argc >= 2) && (strcmp(argv[1], "-h")) == 0) {
        user_args_usage(user_in_args);
        return XMEDIA_FAILURE;
    }

    if (argc != 7) {
        SAMPLE_AUDIO_DBG("Invalid argc:%d\n", argc);
        user_args_usage(user_in_args);
        return XMEDIA_FAILURE;
    }
    user_in_args->sample_rate = atoi(argv[1]);
    user_in_args->channels = atoi(argv[2]);
    g_codec_vol = atoi(argv[3]);
    g_ao_vol = atoi(argv[4]);
    user_in_args->binder_mode = XMEDIA_TRUE;
    memcpy(user_in_args->output_file_name, argv[5], strlen(argv[5]));
    if (strcmp(user_in_args->output_file_name, "null")) {
        user_in_args->output_fd[0] = fopen(user_in_args->output_file_name, "w+");
        if (user_in_args->output_fd[0] == NULL) {
            SAMPLE_AUDIO_DBG("Open %s failed\n", user_in_args->output_file_name);
            return XMEDIA_FAILURE;
         }
    }

    memcpy(user_in_args->input_file_name, argv[6], strlen(argv[6]));
    if (strcmp(user_in_args->input_file_name, "null")) {
        user_in_args->input_fd[0] = fopen(user_in_args->input_file_name, "rb");
        if (user_in_args->input_fd[0] == NULL) {
            SAMPLE_AUDIO_DBG("Open %s failed\n", user_in_args->input_file_name);
            fclose(user_in_args->output_fd[0]);
            return XMEDIA_FAILURE;
        }
    }

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
        SAMPLE_AUDIO_DBG("user_args_init failed(0x%x)\n\n", ret);
        return ret;
    }
    SAMPLE_AUDIO_DBG("user_args_init ok\n");

    ret = sample_audio_aiao();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_AUDIO_DBG("sample_auido_ai failed(0x%x)\n", ret);
    }

    user_args_deinit(&g_user_input_args);
    SAMPLE_AUDIO_DBG("exit %s\n", g_user_input_args.sample);
    return ret;
}
