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
    MODE_SEND_AO,
    MODE_BINDER_AO,
    MODE_DROP
} sample_decoder_mode;

typedef struct user_input_args {
    xmedia_char* sample;
    sample_decoder_mode mode;
    audio_sample_rate file_samplerate;
    xmedia_u32 chn_cnt;
    xmedia_char input_file_name[ADEC_MAX_CHN_NUM][MAX_FILE_NAME_LEN];
    FILE *input_fd[ADEC_MAX_CHN_NUM];
    FILE *output_fd[ADEC_MAX_CHN_NUM];
} user_input_args;

static user_input_args g_user_input_args;
static sample_ao_config g_ao_config;
static sample_adec_config g_adec_config[ADEC_MAX_CHN_NUM];

static xmedia_s32 sample_ao_config_init(xmedia_void)
{
    g_ao_config.dev = XMEDIA_AO_DEV_DAC0;

    sample_comm_ao_get_default_attr(g_ao_config.dev, &g_ao_config.dev_attr);
    g_ao_config.dev_attr.channels = 1;
    g_ao_config.dev_attr.sample_rate = 16000;
    g_ao_config.dev_attr.pcm_samples_per_frame = FRAME_TIME * g_ao_config.dev_attr.sample_rate / 1000;
    g_ao_config.dev_attr.mode = AUDIO_DEV_WORK_MODE_MONO;
    g_ao_config.dev_attr.pcm_frame_max_num = 12;
    g_ao_config.vqe_enable = XMEDIA_FALSE;
    if ((g_ao_config.dev_attr.sample_rate != AUDIO_SAMPLE_RATE_8000 && g_ao_config.dev_attr.sample_rate != AUDIO_SAMPLE_RATE_16000)
        || (g_ao_config.frame_info.frm_rate != g_ao_config.dev_attr.sample_rate)) {
        g_ao_config.res_enable = XMEDIA_TRUE;
    }
    if (g_user_input_args.mode == MODE_BINDER_AO) {
        g_ao_config.binder_src = MOD_ID_ADEC;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_adec_config_init(adec_chn chn)
{
    voice_format format = VOICE_FORMAT_G711_A;
    audio_codec_type type = CODEC_TYPE_G711;

    sample_comm_get_codec_info(g_user_input_args.input_file_name[chn], &type, &format);

    g_adec_config[chn].sample_rate = g_user_input_args.file_samplerate;
    g_adec_config[chn].chn = chn;
    g_adec_config[chn].type = type;
    g_adec_config[chn].format = format;

    //AAC/MP3不知道输入采样率，默认注册重采样
    g_ao_config.res_enable = XMEDIA_FALSE;
    if (g_adec_config[chn].type == CODEC_TYPE_AAC || g_adec_config[chn].type == CODEC_TYPE_MP3) {
        g_ao_config.res_enable = XMEDIA_TRUE;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_init_audio()
{
    return sample_comm_audio_init();
}

static xmedia_s32 sample_start_ao()
{
    xmedia_s32 ret;

    if (g_user_input_args.mode == MODE_SAVE_FILE || g_user_input_args.mode == MODE_DROP) {
        return XMEDIA_SUCCESS;
    }

    ret = sample_ao_config_init();
    CHECK_RET(ret, "sample_ao_config_init");

    if (g_ao_config.res_enable) {
        ret = sample_comm_ao_register(0, AO_VQE_ENHANCE_ATTR_VERSION_1, g_ao_config.res_enable);
        CHECK_RET(ret, "sample_comm_ao_register");
    }

    ret = sample_comm_ao_start(&g_ao_config);
    CHECK_RET(ret, "sample_comm_ao_start");

    return ret;
}

static xmedia_s32 sample_start_adec()
{
    xmedia_s32 i;
    xmedia_s32 ret;

    for (i = 0; i < g_user_input_args.chn_cnt; i++) {
        ret = sample_adec_config_init(i);
        CHECK_RET(ret, "sample_adec_config_init");

        ret = sample_comm_adec_register(g_adec_config[i].type);
        //CHECK_RET(ret, "sample_comm_adec_register");

        ret = sample_comm_adec_start(&g_adec_config[i]);
        CHECK_RET(ret, "sample_comm_adec_start");
    }

    return ret;
}

static xmedia_s32 sample_deinit_audio()
{
    return sample_comm_audio_exit();
}

static xmedia_s32 sample_stop_ao()
{
    xmedia_s32 ret;

    if (g_user_input_args.mode == MODE_SAVE_FILE || g_user_input_args.mode == MODE_DROP) {
        return XMEDIA_SUCCESS;
    }

    ret = sample_comm_ao_stop(&g_ao_config);
    CHECK_RET(ret, "sample_comm_ao_stop");

    return ret;
}

static xmedia_s32 sample_stop_adec()
{
    xmedia_s32 i;
    xmedia_s32 ret;

    for (i = 0; i < g_user_input_args.chn_cnt; i++) {
        ret = sample_comm_adec_stop(&g_adec_config[i]);
        CHECK_RET(ret, "sample_comm_adec_stop");
    }

    for (i = 0; i < g_user_input_args.chn_cnt; i++) {
        ret = sample_comm_adec_unregister(g_adec_config[i].type);
        CHECK_RET(ret, "sample_comm_adec_unregister");
    }

    return ret;
}

static xmedia_s32 sample_start_data_thread()
{
    xmedia_s32 i;
    xmedia_s32 ret = XMEDIA_SUCCESS;
    for (i = 0; i < g_user_input_args.chn_cnt; i++) {
        ret = sample_comm_adec_create_input_thread_from_file(&g_adec_config[i], g_user_input_args.input_fd);
        CHECK_RET(ret, "sample_comm_adec_create_out_to_ao_thread");

        if (g_user_input_args.mode == MODE_SEND_AO) {
            ret = sample_comm_adec_create_output_thread_to_ao(&g_adec_config[i], g_ao_config.dev, g_adec_config[i].chn);
            CHECK_RET(ret, "sample_comm_adec_create_out_to_ao_thread");
        } else if (g_user_input_args.mode == MODE_SAVE_FILE || g_user_input_args.mode == MODE_DROP) {
            ret = sample_comm_adec_create_output_thread_to_file(&g_adec_config[i], g_user_input_args.output_fd);
             CHECK_RET(ret, "sample_comm_adec_create_output_thread_to_file");
        }
    }

    return ret;
}

static xmedia_s32 sample_stop_data_thread()
{
    xmedia_s32 i;
    xmedia_s32 ret = XMEDIA_SUCCESS;

    for (i = 0; i < g_user_input_args.chn_cnt; i++) {
        ret = sample_comm_adec_destroy_input_thread_from_file(&g_adec_config[i]);
        CHECK_RET(ret, "sample_comm_adec_destroy_input_thread_from_file");
        if (g_user_input_args.mode == MODE_SEND_AO) {
            ret = sample_comm_adec_destroy_output_thread_to_ao(&g_adec_config[i]);
            CHECK_RET(ret, "sample_comm_adec_destroy_output_thread_to_ao");
        } else if (g_user_input_args.mode == MODE_SAVE_FILE || g_user_input_args.mode == MODE_DROP) {
            ret = sample_comm_adec_destroy_output_thread_to_file(&g_adec_config[i]);
            CHECK_RET(ret, "sample_comm_adec_destroy_output_thread_to_file");
        }
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

    ret = sample_start_adec();
    if (ret != XMEDIA_SUCCESS) {
        goto ERR_START_ADEC;
    }

    ret = sample_start_ao();
    if (ret != XMEDIA_SUCCESS) {
        goto ERR_START_AO;
    }

    ret = sample_start_data_thread();
    if (ret != XMEDIA_SUCCESS) {
        goto ERR_START_DATA_TRHEAD;;
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
    sample_stop_data_thread();

ERR_START_DATA_TRHEAD:
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
    SAMPLE_AUDIO_DBG("usage:   %s [mode] [infile samplerate] [infile]\n", args->sample);
    SAMPLE_AUDIO_DBG("mode :   0:save file 1:send ao 2:bind ao 3: drop\n");
    SAMPLE_AUDIO_DBG("in_rate: input file samplerate\n");
    SAMPLE_AUDIO_DBG("in_file: input file for adec(.g711a/.g711u/.g726/.aac/.mp3/.adpcm)\n");

    SAMPLE_AUDIO_DBG("examples:\n");
    SAMPLE_AUDIO_DBG("one chn:    %s 0 16000 in.aac \n", args->sample);
    SAMPLE_AUDIO_DBG("multi chn:  %s 0 16000 in1.aac in2.aac in3.g711a in4.mp3\n", args->sample);
}

static xmedia_s32 user_args_init(int argc, char* argv[], user_input_args* user_in_args)
{
    xmedia_s32 i;
    xmedia_char* pos;
    xmedia_char* in_file_path;
    xmedia_char out_file_path[MAX_FILE_NAME_LEN];
    user_in_args->sample = argv[0];

    if ((argc >= 2) && (strcmp(argv[1], "-h")) == 0) {
        user_args_usage(user_in_args);
        return XMEDIA_FAILURE;
    }

    if (argc < 4) {
        SAMPLE_AUDIO_DBG("Invalid argc:%d\n", argc);
        user_args_usage(user_in_args);
        return XMEDIA_FAILURE;
    }

    user_in_args->mode = atoi(argv[1]);
    user_in_args->file_samplerate = atoi(argv[2]);

    SAMPLE_AUDIO_DBG("get user args: \n");
    SAMPLE_AUDIO_DBG("  mode: %d\n", user_in_args->mode);
    SAMPLE_AUDIO_DBG("  file samplerate: %d\n", user_in_args->file_samplerate);

    for (i = 0; i < argc - 3; i++) {
        if (i >= ADEC_MAX_CHN_NUM) {
            break;
        }

        in_file_path = argv[3 + i];
        SAMPLE_AUDIO_DBG("  in file: %s\n", in_file_path);
        memcpy(user_in_args->input_file_name[i], in_file_path, strlen(in_file_path));
        user_in_args->input_fd[i] = fopen(in_file_path, "rb");
        if (user_in_args->input_fd[i] == NULL) {
            SAMPLE_AUDIO_DBG("Open %s failed\n", in_file_path);
            return XMEDIA_FAILURE;
        }
        user_in_args->chn_cnt++;

        if (user_in_args->mode != MODE_SAVE_FILE) {
            continue;
        }

        pos = strstr(in_file_path, ".");
        if (pos == XMEDIA_NULL) {
            SAMPLE_AUDIO_DBG("Open %s failed\n", in_file_path);
            return XMEDIA_FAILURE;
        }

        memset(out_file_path, 0, MAX_FILE_NAME_LEN);
        memcpy(out_file_path, in_file_path, pos - in_file_path);
        snprintf(out_file_path + (pos - in_file_path), 64, "_chn%d.pcm", i);

        SAMPLE_AUDIO_DBG("  out file: %s\n", out_file_path);
        user_in_args->output_fd[i] = fopen(out_file_path, "wb");
        if (user_in_args->output_fd[i] == NULL) {
            SAMPLE_AUDIO_DBG("Open %s failed\n", out_file_path);
            return XMEDIA_FAILURE;
        }
    }

    return XMEDIA_SUCCESS;
}

static xmedia_void user_args_deinit(user_input_args* user_in_args)
{
    xmedia_s32 i;
    for (i = 0; i < ADEC_MAX_CHN_NUM; i++) {
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

