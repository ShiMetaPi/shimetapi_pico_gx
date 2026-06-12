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
#include "xmedia_audio_vqe_bcd.h"
#include "xmedia_audio_vqe_detect.h"

#include "sample_comm_audio.h"
#include "config_parameter/config_load_parameter.h"

static ai_vqe_attr_detect detect_attr;

#define SAMPLE_AUDIO_DBG  printf
#define MAX_FILE_NAME_LEN 128
#define FRAME_TIME        10  //ms

typedef struct user_input_args {
    xmedia_char* sample;
    audio_sample_rate sample_rate;
    audio_sample_rate usr_sample_rate;
    audio_channel channels;
    xmedia_bool binder_mode;
    xmedia_s32 codec_input_vol;
    xmedia_s32 ai_vqe_vol;

    char output_file_name[MAX_FILE_NAME_LEN];
    char config_file_name[MAX_FILE_NAME_LEN];
    FILE *output_fd[2];
} user_input_args;

static user_input_args g_user_input_args;
static sample_ai_config g_ai_config;
static user_base_config base_config;

static xmedia_s32 cry_callback(xmedia_void *attr)
{
    static xmedia_s32 count = 0;
    printf("cry callback count(%d)\n" ,count++);
    return XMEDIA_SUCCESS;
}

static xmedia_s32 gbd_callback(xmedia_void *attr)
{
    static xmedia_s32 count = 0;
    printf("gbd callback count(%d)\n" ,count++);
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

static xmedia_s32 sample_ai_config_init(xmedia_void)
{
    xmedia_s32 ret;
    g_ai_config.dev = XMEDIA_AI_DEV_ADC0;

    sample_comm_ai_get_default_attr(g_ai_config.dev, &g_ai_config.dev_attr);

    ret = load_base_parameter(g_user_input_args.config_file_name, &base_config);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }
    g_user_input_args.codec_input_vol = base_config.codec_input_vol;
    g_user_input_args.ai_vqe_vol = base_config.ai_vqe_vol;
    g_ai_config.dev_attr.channels = base_config.in_chn_cnt;
    g_ai_config.dev_attr.sample_rate = base_config.work_samplerate;
    g_ai_config.dev_attr.pcm_samples_per_frame = base_config.sample_per_frame;
    g_ai_config.dev_attr.pcm_frame_num = base_config.frame_nums;
    g_ai_config.dev_attr.mode = AUDIO_DEV_WORK_MODE_QUEUE;
    g_ai_config.chn_attr.sample_rate = base_config.work_samplerate;
    g_ai_config.source = AI_EC_SOURCE_AO_DAC0;
    g_ai_config.ec_chn_id = 0;
    g_ai_config.res_enable = XMEDIA_FALSE;
    g_ai_config.chn_attr.interleaved = XMEDIA_TRUE;
    if ((g_ai_config.dev_attr.sample_rate != AUDIO_SAMPLE_RATE_8000 && g_ai_config.dev_attr.sample_rate != AUDIO_SAMPLE_RATE_16000)
        || (g_ai_config.chn_attr.sample_rate != g_ai_config.dev_attr.sample_rate)) {
        g_ai_config.res_enable = XMEDIA_TRUE;
    }

    g_ai_config.vqe_enable = XMEDIA_TRUE;
    if (base_config.vqe_version == AI_VQE_DETECT_ATTR_VERSION_1) {
        g_ai_config.vqe_attr.attr = &detect_attr;
    }
    g_ai_config.vqe_attr.version = base_config.vqe_version;
    g_ai_config.vqe_attr.work_sample_rate = base_config.work_samplerate;
    g_ai_config.vqe_attr.in_channels = base_config.in_chn_cnt;
    ret = load_aivqe_parameter(g_user_input_args.config_file_name, &g_ai_config.vqe_attr);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    detect_attr.bcd_attr.bcd_model.load_type = XMEDIA_CL_LOAD_FILE;
    detect_attr.bcd_attr.bcd_model.model_info.file.pathname = "../audio_xmm/bcd_neuron_network.xmm";
    detect_attr.bcd_attr.callback = (fn_bcd_callback)cry_callback;

    detect_attr.gbd_attr.gbd_model.load_type = XMEDIA_CL_LOAD_FILE;
    detect_attr.gbd_attr.gbd_model.model_info.file.pathname = "../audio_xmm/gbd_neuron_network.xmm";
    detect_attr.gbd_attr.callback = (fn_detect_gbd_callback)gbd_callback;

    detect_attr.ssl_attr.callback = (fn_detect_ssl_callback)ssl_callback;
    detect_attr.ved_attr.callback = (fn_detect_ved_callback)ved_callback;
    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_init_audio()
{
    return sample_comm_audio_init();
}

static xmedia_s32 sample_start_ai()
{
    xmedia_s32 ret;
    xmedia_u32 mask = 0;

    ret = sample_ai_config_init();
    CHECK_RET(ret, "sample_ai_config_init");
    mask = detect_attr.mask;

    ret = sample_comm_ai_register(mask, g_ai_config.vqe_attr.version, g_ai_config.res_enable);
    CHECK_RET(ret, "sample_comm_ai_register");

    ret = sample_comm_ai_start(&g_ai_config);
    CHECK_RET(ret, "sample_comm_ai_start");

    if (g_ai_config.dev == XMEDIA_AI_DEV_ADC0) {
        ret = sample_comm_ai_set_dev_param(&g_ai_config, AI_PARAM_SET_CODEC_ADC_GAIN, (xmedia_void*)&g_user_input_args.codec_input_vol);
        if (ret != XMEDIA_SUCCESS) {
            printf("sample_comm_ai_set_dev_param failed(0x%x)\n", ret);
            goto ERR;
        }
    } else if (g_ai_config.dev == XMEDIA_AI_DEV_PDM0) {
        ret = sample_comm_ai_set_dev_param(&g_ai_config, AI_PARAM_SET_CODEC_MIC_GAIN, (xmedia_void*)&g_user_input_args.codec_input_vol);
        if (ret != XMEDIA_SUCCESS) {
            printf("sample_comm_ai_set_dev_param failed(0x%x)\n", ret);
            goto ERR;
        }
    }

    ret = sample_comm_ai_set_volume(&g_ai_config, g_user_input_args.ai_vqe_vol);
    if (ret != XMEDIA_SUCCESS) {
        printf("sample_comm_ai_set_volume failed(0x%x)\n", ret);
        goto ERR;
    }
    ret = sample_comm_ai_create_output_thread_to_file(&g_ai_config, g_user_input_args.output_fd);
    if (ret != XMEDIA_SUCCESS) {
        printf("sample_comm_ai_create_output_thread_to_file failed(0x%x)\n", ret);
        goto ERR;
    }

    return XMEDIA_SUCCESS;
ERR:
    sample_comm_ai_stop(&g_ai_config);
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

xmedia_s32 sample_audio_detect_vqe(xmedia_void)
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

    while(1) {
        int val;
        xmedia_char input_cmd[32];
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
    SAMPLE_AUDIO_DBG("Usage:./aq_utils_test_detect [Config] [OutFile]\n");
    SAMPLE_AUDIO_DBG("Parameter comment as follows:\n");
    SAMPLE_AUDIO_DBG("  Config      : alg parameters config file\n");
    SAMPLE_AUDIO_DBG("  OutFile     : vqe out file\n");

    SAMPLE_AUDIO_DBG("Examples as follows:\n");
    SAMPLE_AUDIO_DBG("    ./aq_utils_test_detect ./config_parameter/record_ai_detect.ini /tmp/vqe_out.pcm\n");
}

static xmedia_s32 user_args_init(int argc, char* argv[], user_input_args* user_in_args)
{
    user_in_args->sample = argv[0];

    if ((argc >= 3) && (strcmp(argv[1], "-h")) == 0) {
        user_args_usage(user_in_args);
        return XMEDIA_FAILURE;
    }

    if (argc != 3) {
        printf("Invalid argc:%d\n", argc);
        user_args_usage(user_in_args);
        return XMEDIA_FAILURE;
    }
    user_in_args->binder_mode = XMEDIA_FALSE;

    memcpy(user_in_args->config_file_name, argv[1], strlen(argv[1]));

    memcpy(user_in_args->output_file_name, argv[2], strlen(argv[2]));
    user_in_args->output_fd[0] = fopen(user_in_args->output_file_name, "w+");
    if (user_in_args->output_fd[0] == NULL) {
        printf("Open %s failed\n", user_in_args->output_file_name);
        return XMEDIA_FAILURE;
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

    ret = sample_audio_detect_vqe();
    if (ret != XMEDIA_SUCCESS) {
        printf("sample_audio_record_vqev1 failed(0x%x)\n", ret);
    }

    user_args_deinit(&g_user_input_args);
    SAMPLE_AUDIO_DBG("exit %s\n", g_user_input_args.sample);
    return ret;
}

