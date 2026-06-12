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
#include "sample_comm_audio.h"

#define SAMPLE_AUDIO_DBG  printf
#define MAX_FILE_NAME_LEN 128
#define FRAME_TIME        10  //ms

typedef enum {
    MODE_SAVE_FILE,
    MODE_BINDER_AI,
    MODE_DROP
} sample_encoder_mode;

typedef struct user_input_args {
    xmedia_char* sample;
    audio_sample_rate file_samplerate;
    audio_channel file_channel;
    xmedia_u32 chn_cnt;
    xmedia_char output_file_name[AENC_MAX_CHN_NUM][MAX_FILE_NAME_LEN];
    FILE *input_fd[AENC_MAX_CHN_NUM];
    FILE *output_fd[AENC_MAX_CHN_NUM];
} user_input_args;

static user_input_args g_user_input_args;
static sample_aenc_config g_aenc_config[AENC_MAX_CHN_NUM];

static xmedia_s32 sample_aenc_config_init(aenc_chn chn)
{
    voice_format format = VOICE_FORMAT_G711_A;
    audio_codec_type type = CODEC_TYPE_G711;

    sample_comm_get_codec_info(g_user_input_args.output_file_name[chn], &type, &format);

    g_aenc_config[chn].sample_rate = g_user_input_args.file_samplerate;
    g_aenc_config[chn].chn = chn;
    g_aenc_config[chn].type = type;
    g_aenc_config[chn].format = format;
    g_aenc_config[chn].bit_depth = 16;
    g_aenc_config[chn].channels = g_user_input_args.file_channel;

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_init_audio()
{
    return sample_comm_audio_init();
}

static xmedia_s32 sample_start_aenc()
{
    xmedia_s32 i;
    xmedia_s32 ret;

    for (i = 0; i < g_user_input_args.chn_cnt; i++) {
        ret = sample_aenc_config_init(i);
        CHECK_RET(ret, "sample_aenc_config_init");

        ret = sample_comm_aenc_register(g_aenc_config[i].type);
        //CHECK_RET(ret, "sample_comm_aenc_register");

        ret = sample_comm_aenc_start(&g_aenc_config[i]);
        CHECK_RET(ret, "sample_comm_aenc_start");

        ret = sample_comm_aenc_create_input_thread_from_file(&g_aenc_config[i], g_user_input_args.input_fd);
        CHECK_RET(ret, "sample_comm_aenc_create_input_thread_from_file");

        ret = sample_comm_aenc_create_output_thread_to_file(&g_aenc_config[i], g_user_input_args.output_fd);
        CHECK_RET(ret, "sample_comm_aenc_create_input_thread_from_file");
    }

    return ret;
}

static xmedia_s32 sample_deinit_audio()
{
    return sample_comm_audio_exit();
}

static xmedia_s32 sample_stop_aenc()
{
    xmedia_s32 i;
    xmedia_s32 ret;

    for (i = 0; i < g_user_input_args.chn_cnt; i++) {
        ret = sample_comm_aenc_stop(&g_aenc_config[i]);
        CHECK_RET(ret, "sample_comm_aenc_stop");

        ret = sample_comm_aenc_destroy_input_thread_from_file(&g_aenc_config[i]);
        CHECK_RET(ret, "sample_comm_aenc_create_input_thread_from_file");

        ret = sample_comm_aenc_destroy_output_thread_to_file(&g_aenc_config[i]);
        CHECK_RET(ret, "sample_comm_aenc_destroy_output_thread_to_file");
    }

    for (i = 0; i < g_user_input_args.chn_cnt; i++) {
        ret = sample_comm_aenc_unregister(g_aenc_config[i].type);
        CHECK_RET(ret, "sample_comm_aenc_unregister");
    }

    return ret;
}

xmedia_s32 sample_audio_doubletalk(xmedia_void)
{
    xmedia_s32 ret;

    ret = sample_init_audio();
    if (ret != XMEDIA_SUCCESS) {
        goto ERR_INIT_AUDIO;
    }

    ret = sample_start_aenc();
    if (ret != XMEDIA_SUCCESS) {
        goto ERR_START_AENC;
    }

    while(1) {
        xmedia_char input_cmd[32];
        SAMPLE_AUDIO_DBG("\nInput 'q' ENTER to exit\n");

        fgets(input_cmd, (sizeof(input_cmd) - 1), stdin);
        if ((input_cmd[0] == 'q') || (input_cmd[0] == 'Q')) {
            SAMPLE_AUDIO_DBG("Ready to exit\n");
            break;
        }
    }
    sample_stop_aenc();
ERR_START_AENC:
ERR_INIT_AUDIO:
    sample_deinit_audio();
    return ret;
}

static xmedia_void user_args_usage(user_input_args* args)
{
    SAMPLE_AUDIO_DBG("usage: %s [infile samplerate] [infile channels] [infile] [outfile]\n", args->sample);
    SAMPLE_AUDIO_DBG("infile samplerate: input file samplerate\n");
    SAMPLE_AUDIO_DBG("infile channels: input file channels\n");
    SAMPLE_AUDIO_DBG("infile: input file path for aenc\n");
    SAMPLE_AUDIO_DBG("outfile: .g711a/.g711u/.g726/.aac/.mp3/.adpcm\n");

    SAMPLE_AUDIO_DBG("examples:\n");
    SAMPLE_AUDIO_DBG("one aenc chn:  %s 16000 1 in.pcm out.g711a \n", args->sample);
    SAMPLE_AUDIO_DBG("multi aenc chn:  %s 16000 1 in1.pcm out1.g711a in2.pcm out2.aac\n", args->sample);
}

static xmedia_s32 user_args_init(int argc, char* argv[], user_input_args* user_in_args)
{
    xmedia_s32 i;
    user_in_args->sample = argv[0];

    if ((argc >= 2) && (strcmp(argv[1], "-h")) == 0) {
        user_args_usage(user_in_args);
        return XMEDIA_FAILURE;
    }

    if (argc < 5 || ((argc - 1) % 2 != 0)) {
        SAMPLE_AUDIO_DBG("Invalid argc:%d\n", argc);
        user_args_usage(user_in_args);
        return XMEDIA_FAILURE;
    }

    user_in_args->file_samplerate = atoi(argv[1]);
    user_in_args->file_channel = atoi(argv[2]);

    SAMPLE_AUDIO_DBG("get user args: \n");
    SAMPLE_AUDIO_DBG("  file samplerate: %d\n", user_in_args->file_samplerate);
    SAMPLE_AUDIO_DBG("  file channels: %d\n", user_in_args->file_channel);

    for (i = 0; i < argc - 3; i += 2) {
        if (i >= AENC_MAX_CHN_NUM * 2) {
            break;
        }

        SAMPLE_AUDIO_DBG("  in file: %s\n", argv[3 + i]);
        user_in_args->input_fd[user_in_args->chn_cnt] = fopen(argv[3 + i], "rb");
        if (user_in_args->input_fd[user_in_args->chn_cnt] == NULL) {
            SAMPLE_AUDIO_DBG("Open %s failed\n", argv[3 + i]);
            return XMEDIA_FAILURE;
        }

        memcpy(user_in_args->output_file_name[user_in_args->chn_cnt], argv[3 + i + 1], strlen(argv[3 + i + 1]));
        SAMPLE_AUDIO_DBG("  out file: %s\n", user_in_args->output_file_name[user_in_args->chn_cnt]);
        user_in_args->output_fd[user_in_args->chn_cnt] = fopen(user_in_args->output_file_name[user_in_args->chn_cnt], "wb");
        if (user_in_args->output_fd[user_in_args->chn_cnt] == NULL) {
            SAMPLE_AUDIO_DBG("Open %s failed\n", user_in_args->output_file_name[user_in_args->chn_cnt]);
            return XMEDIA_FAILURE;
        }

        user_in_args->chn_cnt++;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_void user_args_deinit(user_input_args* user_in_args)
{
    xmedia_s32 i;

    for (i = 0; i < AENC_MAX_CHN_NUM; i++) {
        if (user_in_args->input_fd[i] != NULL) {
            fclose(user_in_args->input_fd[i]);
            user_in_args->input_fd[i] = NULL;
        }

        if (user_in_args->output_fd[i] != NULL) {
            fclose(user_in_args->output_fd[i]);
            user_in_args->output_fd[i] = NULL;
        }
    }
}

xmedia_s32 main(int argc, char* argv[])
{
    xmedia_s32 ret;

    ret = user_args_init(argc, argv, &g_user_input_args);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_AUDIO_DBG("user_args_init failed(0x%x)\n\n", ret);
        user_args_deinit(&g_user_input_args);
        return ret;
    }
    SAMPLE_AUDIO_DBG("user_args_init ok\n");

    ret = sample_audio_doubletalk();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_AUDIO_DBG("sample_auido_ai failed(0x%x)\n", ret);
    }

    user_args_deinit(&g_user_input_args);
    SAMPLE_AUDIO_DBG("exit %s\n", g_user_input_args.sample);
    return ret;
}



