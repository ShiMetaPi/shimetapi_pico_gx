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

#include "sample_comm_audio.h"

#define SAMPLE_AUDIO_DBG  printf
#define MAX_FILE_NAME_LEN 128
#define FRAME_TIME        10  //ms
#define DEFAULT_AI_FRAME_NUM 50

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

    xmedia_u32 send_object;
    ao_dev send_object_dev;
    char output_file_name[MAX_FILE_NAME_LEN];
    FILE *output_fd[2];
} user_input_args;

static user_input_args g_user_input_args;
static sample_ai_config g_ai_config;

static xmedia_s32 sample_ai_config_init(xmedia_void)
{
    g_ai_config.dev = g_user_input_args.dev;

    sample_comm_ai_get_default_attr(g_ai_config.dev, &g_ai_config.dev_attr);
    g_ai_config.dev_attr.channels = g_user_input_args.channels;
    g_ai_config.dev_attr.sample_rate = g_user_input_args.sample_rate;
    g_ai_config.dev_attr.bit_depth = g_user_input_args.bit_depth;
    g_ai_config.dev_attr.pcm_samples_per_frame = g_user_input_args.samples_per_frm;
    g_ai_config.dev_attr.pcm_frame_num = DEFAULT_AI_FRAME_NUM;
    g_ai_config.dev_attr.mode = g_user_input_args.workmode;
    g_ai_config.chn_attr.sample_rate = g_user_input_args.usr_sample_rate;
    g_ai_config.chn_attr.interleaved = XMEDIA_TRUE;
    g_ai_config.source = AI_EC_SOURCE_AO_DAC0;
    g_ai_config.ec_chn_id = 0;
    g_ai_config.vqe_enable = XMEDIA_FALSE;

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

    if (g_user_input_args.send_object == 0) {
        ret = sample_comm_ai_destroy_output_thread_to_file(&g_ai_config);
        CHECK_RET(ret, "sample_comm_ai_destroy_output_thread_to_file");
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

    ret = sample_comm_ai_start(&g_ai_config);
    CHECK_RET(ret, "sample_comm_ai_start");

    if (g_user_input_args.send_object == 0) {
        ret = sample_comm_ai_create_output_thread_to_file(&g_ai_config, g_user_input_args.output_fd);
        CHECK_RET(ret, "sample_comm_ai_create_output_thread_to_file");
    }

    if (g_user_input_args.dev == XMEDIA_AI_DEV_ADC0) {
        ret = sample_comm_ai_set_dev_param(&g_ai_config, AI_PARAM_SET_CODEC_ADC_GAIN, (xmedia_void*)&g_user_input_args.codec_input_vol);
        if (ret != XMEDIA_SUCCESS) {
            printf("sample_comm_ai_set_dev_param failed(0x%x)\n", ret);
            goto ERR;
        }
    }
    return XMEDIA_SUCCESS;
ERR:
    sample_stop_ai();
    return ret;
}

xmedia_s32 sample_audio_ai(xmedia_void)
{
    xmedia_s32 ret;
    xmedia_s32 val;
    xmedia_char input_cmd[32];

    ret = sample_init_audio();
    if (ret != XMEDIA_SUCCESS) {
        goto ERR_INIT_AUDIO;
    }

    ret = sample_start_ai();
    if (ret != XMEDIA_SUCCESS) {
        goto ERR_START_AI;
    }

    while(1) {
        SAMPLE_AUDIO_DBG("\nInput 'q' ENTER to exit\n");
        SAMPLE_AUDIO_DBG(">>> Input 'scvXX' to set input vol to xx dB\n");
        SAMPLE_AUDIO_DBG(">>> Input 'gcv' to get input vol\n");

        fgets(input_cmd, (sizeof(input_cmd) - 1), stdin);
        if (input_cmd[0] == 's' && input_cmd[1] == 'c' && input_cmd[2] == 'v') {
            val = atoi(input_cmd + 3);
            ret = sample_comm_ai_set_dev_param(&g_ai_config, AI_PARAM_SET_CODEC_ADC_GAIN, (xmedia_void*)&val);
            SAMPLE_AUDIO_DBG("\nSet codec input volume %d return 0x%x\n", val, ret);
        } else if (input_cmd[0] == 'g' && input_cmd[1] == 'c' && input_cmd[2] == 'v') {
            ret = sample_comm_ai_set_dev_param(&g_ai_config, AI_PARAM_GET_CODEC_ADC_GAIN, (xmedia_void*)&val);
            SAMPLE_AUDIO_DBG("\nGet codec input volume(%d) return 0x%x\n", val, ret);
        } else if (input_cmd[0] == 'q') {
            SAMPLE_AUDIO_DBG("Ready to exit\n");
            break;
        }
    }
    sample_stop_ai();
ERR_START_AI:
ERR_INIT_AUDIO:
    sample_deinit_audio();
    return ret;
}

static xmedia_void user_args_usage(user_input_args* args)
{
    SAMPLE_AUDIO_DBG("Usage:./aq_utils_test_mic_vol [Samplerate] [Chn] [CodecInputVol] [MicFile]\n");
    SAMPLE_AUDIO_DBG("Parameter comment as follows:\n");
    SAMPLE_AUDIO_DBG("  [Samplerate]: Work Samplerate, 8000 or 16000\n");
    SAMPLE_AUDIO_DBG("  [Chn]: Channel, 1 or 2\n");
    SAMPLE_AUDIO_DBG("  [CodecInputVol]: audiocodec mic input volume, range:[-95~48]\n");
    SAMPLE_AUDIO_DBG("  [MicFile]: file for saving mic data\n");

    SAMPLE_AUDIO_DBG("Examples as follows:\n");
    SAMPLE_AUDIO_DBG("    ./aq_utils_test_mic_vol 16000 1 20 /tmp/mic_gain.pcm\n");
}

static xmedia_s32 user_args_init(int argc, char* argv[], user_input_args* user_in_args)
{
    user_in_args->sample = argv[0];

    if ((argc >= 2) && (strcmp(argv[1], "-h")) == 0) {
        user_args_usage(user_in_args);
        return XMEDIA_FAILURE;
    }

    if (argc != 5) {
        printf("Invalid argc:%d\n", argc);
        user_args_usage(user_in_args);
        return XMEDIA_FAILURE;
    }

    user_in_args->dev = XMEDIA_AI_DEV_ADC0;
    user_in_args->sample_rate = atoi(argv[1]);
    user_in_args->usr_sample_rate = atoi(argv[1]);
    user_in_args->bit_depth = AUDIO_BIT_DEPTH_16;
    user_in_args->channels = atoi(argv[2]);
    user_in_args->workmode = 1;     /*queue mode*/
    user_in_args->samples_per_frm = user_in_args->usr_sample_rate / 100;
    user_in_args->codec_input_vol = atoi(argv[3]);
    user_in_args->send_object = 0;
    user_in_args->binder_mode = XMEDIA_TRUE;

    memcpy(user_in_args->output_file_name, argv[4], strlen(argv[4]));
    if (strcmp(user_in_args->output_file_name, "null")) {
        user_in_args->output_fd[0] = fopen(user_in_args->output_file_name, "w+");
        if (user_in_args->output_fd[0] == NULL) {
            printf("Open %s failed\n", user_in_args->output_file_name);
            return XMEDIA_FAILURE;
         }
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
