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
#include "xmedia_audio_vqe_detect.h"

#include "sample_comm_audio.h"

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "drv_codec_es7210.h"
#include "drv_i2c.h"

#define ES7210_CODEC_I2C_NODE   "/dev/i2c-2"
#define ES7210_CODEC_I2C_ADDR   0x40
#define ES7210B_CODEC_I2C_ADDR   0x41
static codec_handle g_h_codec;
static xmedia_u32 mic_distance = 50;

#define SAMPLE_AUDIO_DBG  printf
#define MAX_FILE_NAME_LEN 128
#define FRAME_TIME        10  //ms

#define CRY_ALARMLIMIT 80      // probability greater than which to count as one cry frame
#define CRY_TIMELIMITCNT 3     // number of cry frame count as one cry event
#define CRY_TIMELIMIT 5        // time interval to report cry event
#define CRY_ALARMINTERVAL 3    // alarm interval time to unfreeze

#define GBD_TIMELIMIT     100 // time interval to report gbd event
#define GBD_TIMELIMITCNT  1    // number of gbd frame count as one gbd event
#define GBD_ALARMLIMIT    70   // probability greater than which to count as one gbd frame
#define GBD_ALARMINTERVAL 3    // alarm interval time to unfreeze

typedef struct user_input_args {
    xmedia_char* sample;
    audio_sample_rate sample_rate;
    audio_sample_rate usr_sample_rate;
    audio_channel channels;
    xmedia_s32 codec_input_vol;

    char output_file_name[MAX_FILE_NAME_LEN];
    FILE *output_fd[2];
} user_input_args;

static user_input_args g_user_input_args;
static sample_ai_config g_ai_config;
static ai_vqe_attr_detect ai_detect_attr;
static xmedia_u32 g_vqe_type;

static xmedia_void gpio_map_to_i2s(xmedia_void)
{
    char cmds[128];

    sprintf(cmds, "xmmm 0x100c0010 0x100b");   // I2S_MCLK
    system(cmds);
    sprintf(cmds, "xmmm 0x100c0000 0x100b");   // I2S_BCLK_TX
    system(cmds);
    sprintf(cmds, "xmmm 0x100c0004 0x100b");   // I2S_WS_TX
    system(cmds);
    sprintf(cmds, "xmmm 0x100c000c 0x100b");   // I2S_DATA_RX
    system(cmds);
}

void gpio_map_to_i2c(void)
{
    char cmds[128];

    sprintf(cmds, "xmmm 0x11980014 0x1002");//I2C2_SCL
    system(cmds);
    sprintf(cmds, "xmmm 0x11980018 0x1002");//I2C2_SDA
    system(cmds);
}

static xmedia_void es7210_config(xmedia_void)
{
    ES7210_CODEC_CFG_S codec_cfg = {0};

    strcpy(codec_cfg.i2c_cfg.dev_node, ES7210_CODEC_I2C_NODE);
    codec_cfg.i2c_cfg.addr = ES7210_CODEC_I2C_ADDR;
    codec_cfg.i2c_cfg.i2c_gpio_map_cb = (fn_i2c_gpio_map_cb)gpio_map_to_i2c;

    codec_cfg.micbias = ES7210_MICBIAS_2V87;
    codec_cfg.vdda = ES7210_VDDA_3V3;
    codec_cfg.chip_flag = ES7210_LAST;
    codec_cfg.chip_flag = ES7210_CHIP_NUMBER_0;

    codec_cfg.channels = ES7210_CHANNELS_2;
    codec_cfg.samplerate = g_user_input_args.sample_rate;
    codec_cfg.bitwidth = ES7210_BIT_WIDTH_16;

    codec_cfg.track_mode.adc12 = ES7210_TRACK_NORMAL;
    codec_cfg.track_mode.adc34 = ES7210_TRACK_NORMAL;

    codec_cfg.i2s_cfg.mode = ES7210_SLAVER;
    codec_cfg.i2s_cfg.format = ES7210_I2S_STD;
    codec_cfg.i2s_cfg.bclk_inv_mode = ES7210_BCLK_NORMAL;
    codec_cfg.i2s_cfg.mclk_source = ES7210_MCLK_SOURCE_DEFAULT;
    codec_cfg.i2s_cfg.i2s_gpio_map_cb = (fn_i2s_gpio_map_cb)gpio_map_to_i2s;
    switch (g_user_input_args.sample_rate) {
         case 8000:
             codec_cfg.i2s_cfg.mclk = 2048000;
             break;
         case 16000:
             codec_cfg.i2s_cfg.mclk = 4096000;
             break;
         case 32000:
             codec_cfg.i2s_cfg.mclk = 8192000;
             break;
         case 64000:
             codec_cfg.i2s_cfg.mclk = 8192000;
             break;
         case 48000:
             codec_cfg.i2s_cfg.mclk = 12288000;
             break;
         case 44100:
             codec_cfg.i2s_cfg.mclk = 11289600;
             break;
         default:
             break;
    }
    es7210_init(&g_h_codec, &codec_cfg);
    es7210_set_volume(g_h_codec, 50);
}

xmedia_void es7210_destroy(xmedia_void)
{
    es7210_deinit(g_h_codec);
}

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

static xmedia_void load_ai_detect_attr(ai_vqe_attr* attr, xmedia_u32 sample_rate)
{
    ai_vqe_attr_detect* detect_attr = &ai_detect_attr;

    attr->version = AI_VQE_DETECT_ATTR_VERSION_1;
    attr->work_sample_rate = sample_rate;
    attr->in_channels = g_ai_config.dev_attr.channels;

    if (g_vqe_type == 1) {
        detect_attr->mask =VQE_DETECT_MASK_BCD;
    } else if (g_vqe_type == 2) {
        if (attr->work_sample_rate == AUDIO_SAMPLE_RATE_16000) {
            g_ai_config.res_enable = XMEDIA_TRUE;
        }
        detect_attr->mask = VQE_DETECT_MASK_GBD;
    } else if (g_vqe_type == 3) {
        detect_attr->mask = VQE_DETECT_MASK_SSL;
    } else if (g_vqe_type == 4) {
        detect_attr->mask = VQE_DETECT_MASK_VED;
    } else if (g_vqe_type == 5) {
        detect_attr->mask = VQE_DETECT_MASK_BCD | VQE_DETECT_MASK_HPF | VQE_DETECT_MASK_VED | VQE_DETECT_MASK_SSL;
    }
    detect_attr->hpf_attr.mode = VQE_DET_USR_MODE_AUTO;
    detect_attr->hpf_attr.freq = VQE_DETECT_HPF_FREQ_80;

    detect_attr->bcd_attr.usr_mode = XMEDIA_TRUE;
    detect_attr->bcd_attr.bypass = XMEDIA_FALSE;
    detect_attr->bcd_attr.alarm_threshold = CRY_ALARMLIMIT;
    detect_attr->bcd_attr.time_limit = CRY_TIMELIMIT;
    detect_attr->bcd_attr.time_limit_threshold_count = CRY_TIMELIMITCNT;
    detect_attr->bcd_attr.interval_time = CRY_ALARMINTERVAL;
    /*just for npu bcd lib*/
    detect_attr->bcd_attr.bcd_model.load_type = XMEDIA_CL_LOAD_FILE;
    detect_attr->bcd_attr.bcd_model.model_info.file.pathname = "../audio_xmm/bcd_neuron_network.xmm";
    detect_attr->bcd_attr.callback = (fn_bcd_callback)cry_callback;

    detect_attr->ssl_attr.mic_distance = mic_distance;
    detect_attr->ssl_attr.shape = 0;
    detect_attr->ssl_attr.sensitivity_level = 5;
    detect_attr->ssl_attr.callback = (fn_detect_ssl_callback)ssl_callback;
    detect_attr->ssl_attr.private_data = (xmedia_void*)0x12345678;

    detect_attr->ved_attr.mode = VQE_DET_USR_MODE_AUTO;
    detect_attr->ved_attr.volume_threshold = -15;
    detect_attr->ved_attr.mutation_flag = 1;
    detect_attr->ved_attr.fast_change_threshold = 15;
    detect_attr->ved_attr.slow_change_threshold = 8;
    detect_attr->ved_attr.callback = (fn_detect_ved_callback)ved_callback;

    detect_attr->gbd_attr.usr_mode = XMEDIA_TRUE;
    detect_attr->gbd_attr.bypass = XMEDIA_FALSE;
    detect_attr->gbd_attr.alarm_threshold = GBD_ALARMLIMIT;
    detect_attr->gbd_attr.time_limit = GBD_TIMELIMIT;
    detect_attr->gbd_attr.time_limit_threshold_count = GBD_TIMELIMITCNT;
    detect_attr->gbd_attr.interval_time = GBD_ALARMINTERVAL;
    /*just for npu gbd lib*/
    detect_attr->gbd_attr.gbd_model.load_type = XMEDIA_CL_LOAD_FILE;
    detect_attr->gbd_attr.gbd_model.model_info.file.pathname = "../audio_xmm/gbd_neuron_network.xmm";
    detect_attr->gbd_attr.callback = (fn_detect_gbd_callback)gbd_callback;
    attr->attr = (xmedia_void*)&ai_detect_attr;
}

static xmedia_s32 sample_ai_config_init(xmedia_void)
{
    g_ai_config.dev = XMEDIA_AI_DEV_I2S0;

    sample_comm_ai_get_default_attr(g_ai_config.dev, &g_ai_config.dev_attr);
    g_ai_config.dev_attr.channels = g_user_input_args.channels;
    g_ai_config.dev_attr.sample_rate = g_user_input_args.sample_rate;
    g_ai_config.dev_attr.pcm_samples_per_frame = FRAME_TIME * g_user_input_args.sample_rate / 1000;
    g_ai_config.dev_attr.mode = AUDIO_DEV_WORK_MODE_QUEUE;
    g_ai_config.chn_attr.sample_rate = g_user_input_args.usr_sample_rate;
    g_ai_config.chn_attr.interleaved = XMEDIA_TRUE;
    g_ai_config.source = AI_EC_SOURCE_AO_DAC0;
    g_ai_config.ec_chn_id = 0;
    g_ai_config.vqe_enable = XMEDIA_FALSE;
    g_ai_config.res_enable = XMEDIA_FALSE;

    g_ai_config.dev_attr.un_attr.i2s_attr.master = XMEDIA_TRUE;
    g_ai_config.dev_attr.un_attr.i2s_attr.i2s_mode = AUDIO_I2S_STD_MODE;
    g_ai_config.dev_attr.un_attr.i2s_attr.mclk_sel = AUDIO_I2S_MCLK_256_FS;
    g_ai_config.dev_attr.un_attr.i2s_attr.bclk_sel = AUDIO_I2S_BCLK_4_DIV;
    g_ai_config.dev_attr.un_attr.i2s_attr.pcm_sample_rise_edge = XMEDIA_TRUE;
    g_ai_config.dev_attr.un_attr.i2s_attr.pcm_delay_cycle = AUDIO_I2S_PCM_1_DELAY;
    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_init_audio()
{
    return sample_comm_audio_init();
}

static xmedia_s32 sample_start_ai()
{
    xmedia_s32 ret;
    ai_vqe_attr vqe_detect_attr;

    ret = sample_ai_config_init();
    CHECK_RET(ret, "sample_ai_config_init");

    ret = sample_comm_ai_start(&g_ai_config);
    CHECK_RET(ret, "sample_comm_ai_start");

    //开启检测算法
    if (g_vqe_type != 0) {
        load_ai_detect_attr(&vqe_detect_attr, g_user_input_args.sample_rate);
        ret = sample_comm_ai_register(ai_detect_attr.mask, vqe_detect_attr.version, g_ai_config.res_enable);
        CHECK_RET(ret, "sample_comm_ai_register");
        ret = sample_comm_detect_open(g_ai_config.dev, &vqe_detect_attr);
        if (ret != XMEDIA_SUCCESS) {
            sample_comm_ai_stop(&g_ai_config);
            CHECK_RET(ret, "sample_comm_bcd_open");
        }
    }

    ret = sample_comm_ai_create_output_thread_to_file(&g_ai_config, g_user_input_args.output_fd);
    CHECK_RET(ret, "sample_comm_ai_create_output_thread_to_file");

    if (g_ai_config.dev == XMEDIA_AI_DEV_ADC0) {
        ret = sample_comm_ai_set_dev_param(&g_ai_config, AI_PARAM_SET_CODEC_ADC_GAIN, (xmedia_void*)&g_user_input_args.codec_input_vol);
        CHECK_RET(ret, "sample_comm_ai_set_dev_param");
    }

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
    CHECK_RET(ret, "sample_comm_ai_destroy_output_thread_to_file");

    ret = sample_comm_ai_stop(&g_ai_config);
    CHECK_RET(ret, "sample_comm_ai_stop");

    return ret;
}

xmedia_s32 sample_audio_record_detect(xmedia_void)
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
        xmedia_s32 val;
        xmedia_char input_cmd[32];
        SAMPLE_AUDIO_DBG("\nInput 'q' ENTER to exit\n");
        SAMPLE_AUDIO_DBG("Input 'svXX' to set ai vol to xx dB\n");
        SAMPLE_AUDIO_DBG("Input 'gv' to get ai vol\n");
        SAMPLE_AUDIO_DBG("Input 'saXX' to set adc gain to xx dB\n");

        fgets(input_cmd, (sizeof(input_cmd) - 1), stdin);
        if (input_cmd[0] == 's' && input_cmd[1] == 'v' ) {
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
        } else if ((input_cmd[0] == 'q') || (input_cmd[0] == 'Q')) {
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
    SAMPLE_AUDIO_DBG("usage:     %s [samplerate] [usr_samplerate] [channels] [codec_input_vol] [outfile] [vqe_type] [mic_distance]\n", args->sample);
    SAMPLE_AUDIO_DBG("dev_rate:  dev samplerate(8000/16000/32000/48000)\n");
    SAMPLE_AUDIO_DBG("usr_rate:  out file samplerate(8000/16000/32000/48000)\n");
    SAMPLE_AUDIO_DBG("dev_chn:   dev chn cnt(1/2/8)\n");
    SAMPLE_AUDIO_DBG("out_file:  pcm out file(.pcm)\n");
    SAMPLE_AUDIO_DBG("vqe_type: 0:not open detect, 1:open bcd, 2:open gbd, 3:open ssl, 4:open ved, 5:open bcd + ssl + ved\n");

    SAMPLE_AUDIO_DBG("examples:\n");
    SAMPLE_AUDIO_DBG("           %s 8000 8000 1 20 out.pcm 0\n", args->sample);
    SAMPLE_AUDIO_DBG("           %s 16000 16000 1 20 out.pcm 1\n", args->sample);
    SAMPLE_AUDIO_DBG("           %s 8000 8000 1 20 out.pcm 2\n", args->sample);
    SAMPLE_AUDIO_DBG("           %s 16000 16000 2 20 out.pcm 3 50\n", args->sample);
    SAMPLE_AUDIO_DBG("           %s 16000 16000 2 20 out.pcm 4\n", args->sample);
    SAMPLE_AUDIO_DBG("           %s 16000 16000 2 20 out.pcm 5\n", args->sample);
}

static xmedia_s32 user_args_init(int argc, char* argv[], user_input_args* user_in_args)
{
    user_in_args->sample = argv[0];

    if ((argc >= 2) && (strcmp(argv[1], "-h")) == 0) {
        user_args_usage(user_in_args);
        return XMEDIA_FAILURE;
    }

    if (argc < 7) {
        printf("Invalid argc:%d\n", argc);
        user_args_usage(user_in_args);
        return XMEDIA_FAILURE;
    }

    user_in_args->sample_rate = atoi(argv[1]);
    user_in_args->usr_sample_rate = atoi(argv[2]);
    user_in_args->channels = atoi(argv[3]);
    user_in_args->codec_input_vol = atoi(argv[4]);
    memcpy(user_in_args->output_file_name, argv[5], strlen(argv[5]));
    if (strcmp(user_in_args->output_file_name, "null")) {
        user_in_args->output_fd[0] = fopen(user_in_args->output_file_name, "w+");
        if (user_in_args->output_fd[0] == NULL) {
            printf("Open %s failed\n", user_in_args->output_file_name);
            return XMEDIA_FAILURE;
         }
    }

    g_vqe_type = atoi(argv[6]);
    if (g_vqe_type < 0 || g_vqe_type > 5) {
        printf("Invalid det_type:%d\n", g_vqe_type);
        user_args_usage(user_in_args);
        return XMEDIA_FAILURE;
    }
    mic_distance = atoi(argv[7]);

    SAMPLE_AUDIO_DBG("get user args: \n");
    SAMPLE_AUDIO_DBG("  samplerate: %d\n", user_in_args->sample_rate);
    SAMPLE_AUDIO_DBG("  user samplerate: %d\n", user_in_args->usr_sample_rate);
    SAMPLE_AUDIO_DBG("  chnnels: %d\n", user_in_args->channels);
    SAMPLE_AUDIO_DBG("  codec_input_vol: %d\n", user_in_args->codec_input_vol);
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
    es7210_config();

    ret = sample_audio_record_detect();
    if (ret != XMEDIA_SUCCESS) {
        printf("sample_audio_record_vqev1 failed(0x%x)\n", ret);
    }
    es7210_destroy();

    user_args_deinit(&g_user_input_args);
    SAMPLE_AUDIO_DBG("exit %s\n", g_user_input_args.sample);
    return ret;
}
