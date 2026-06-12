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

#include "sample_comm_audio.h"
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "drv_codec_es7210.h"
#include "drv_i2c.h"
#include "drv_led_ctl.h"

#define ES7210_CODEC_I2C_NODE   "/dev/i2c-2"
#define ES7210_CODEC_I2C_ADDR   0x40
#define ES7210B_CODEC_I2C_ADDR   0x41

static codec_handle g_h_codec;

#define SAMPLE_AUDIO_DBG  printf
#define MAX_FILE_NAME_LEN 128
#define FRAME_TIME        10  //ms

typedef struct user_input_args {
    xmedia_char* sample;
    ai_dev dev;
    audio_sample_rate sample_rate;
    audio_sample_rate usr_sample_rate;
    audio_bit_depth bit_depth;
    audio_channel channels;
    xmedia_s32 workmode;
    xmedia_u32 samples_per_frm;

    char output_file_name[MAX_FILE_NAME_LEN];
    FILE *output_fd[2];
} user_input_args;

static user_input_args g_user_input_args;
static sample_ai_config g_ai_config;

static xmedia_s32 g_volume;


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

xmedia_void es7210b_config(xmedia_void)
{
    ES7210_CODEC_CFG_S codec_cfg = {0};

    strcpy(codec_cfg.i2c_cfg.dev_node, ES7210_CODEC_I2C_NODE);
    codec_cfg.i2c_cfg.addr = ES7210B_CODEC_I2C_ADDR;
    codec_cfg.i2c_cfg.i2c_gpio_map_cb = (fn_i2c_gpio_map_cb)gpio_map_to_i2c;

    codec_cfg.micbias = ES7210_MICBIAS_2V87;
    codec_cfg.vdda = ES7210_VDDA_3V3;
    codec_cfg.chip_flag = ES7210_LAST;
    codec_cfg.chip_number = ES7210_CHIP_NUMBER_1;

    codec_cfg.channels = ES7210_CHANNELS_4;
    codec_cfg.samplerate = g_user_input_args.sample_rate;
    codec_cfg.bitwidth = ES7210_BIT_WIDTH_16;

    codec_cfg.track_mode.adc12 = ES7210_TRACK_NORMAL;
    codec_cfg.track_mode.adc34 = ES7210_TRACK_NORMAL;

    codec_cfg.i2s_cfg.mode = ES7210_SLAVER;
    codec_cfg.i2s_cfg.format = ES7210_TDM_DSPA;
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
    es7210_set_volume(g_h_codec, g_volume);
}

xmedia_void es7210_config(xmedia_void)
{
    ES7210_CODEC_CFG_S codec_cfg = {0};

    strcpy(codec_cfg.i2c_cfg.dev_node, ES7210_CODEC_I2C_NODE);
    codec_cfg.i2c_cfg.addr = ES7210_CODEC_I2C_ADDR;
    codec_cfg.i2c_cfg.i2c_gpio_map_cb = (fn_i2c_gpio_map_cb)gpio_map_to_i2c;

    codec_cfg.micbias = ES7210_MICBIAS_2V87;
    codec_cfg.vdda = ES7210_VDDA_3V3;
    codec_cfg.chip_flag = ES7210_LAST;
    codec_cfg.chip_number = ES7210_CHIP_NUMBER_0;

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
    es7210_set_volume(g_h_codec, g_volume);
}

xmedia_void es7210_destroy(xmedia_void)
{
    es7210_deinit(g_h_codec);
}

static xmedia_s32 sample_ai_config_init(xmedia_void)
{
    g_ai_config.dev = g_user_input_args.dev;

    sample_comm_ai_get_default_attr(g_ai_config.dev, &g_ai_config.dev_attr);
    g_ai_config.dev_attr.channels = g_user_input_args.channels;
    g_ai_config.dev_attr.sample_rate = g_user_input_args.sample_rate;
    g_ai_config.dev_attr.bit_depth = g_user_input_args.bit_depth;
    g_ai_config.dev_attr.pcm_samples_per_frame = g_user_input_args.samples_per_frm;
    g_ai_config.dev_attr.pcm_frame_num = 200;
    g_ai_config.dev_attr.mode = g_user_input_args.workmode;
    g_ai_config.chn_attr.sample_rate = g_user_input_args.usr_sample_rate;
    g_ai_config.chn_attr.interleaved = XMEDIA_TRUE;
    g_ai_config.source = AI_EC_SOURCE_AO_DAC0;
    g_ai_config.ec_chn_id = 0;
    g_ai_config.vqe_enable = XMEDIA_FALSE;

    g_ai_config.dev_attr.un_attr.i2s_attr.master = XMEDIA_TRUE;
    g_ai_config.dev_attr.un_attr.i2s_attr.i2s_mode = AUDIO_I2S_STD_MODE;
    g_ai_config.dev_attr.un_attr.i2s_attr.mclk_sel = AUDIO_I2S_MCLK_256_FS;
    g_ai_config.dev_attr.un_attr.i2s_attr.bclk_sel = AUDIO_I2S_BCLK_4_DIV;
    g_ai_config.dev_attr.un_attr.i2s_attr.pcm_sample_rise_edge = XMEDIA_TRUE;
    g_ai_config.dev_attr.un_attr.i2s_attr.pcm_delay_cycle = AUDIO_I2S_PCM_1_DELAY;
    return XMEDIA_SUCCESS;
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

static xmedia_s32 sample_stop_ai()
{
    xmedia_s32 ret;

    ret = sample_comm_ai_destroy_output_thread_to_file(&g_ai_config);
    CHECK_RET(ret, "sample_comm_ai_destroy_output_thread_to_file");

    ret = sample_comm_ai_stop(&g_ai_config);
    CHECK_RET(ret, "sample_comm_ai_stop");

    return ret;
}

xmedia_s32 sample_audio_ai(xmedia_void)
{
    xmedia_s32 ret;

    ret = xmedia_ai_init();
    CHECK_RET(ret, "xmedia_ai_init");

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
        SAMPLE_AUDIO_DBG("Input 'dso' to dump stream open\n");
        SAMPLE_AUDIO_DBG("Input 'dsc' to dump stream close\n");

        fgets(input_cmd, (sizeof(input_cmd) - 1), stdin);
        if (input_cmd[0] == 's' && input_cmd[1] == 'v') {
            val = atoi(input_cmd + 2);
            ret = sample_comm_ai_set_volume(&g_ai_config, val);
            SAMPLE_AUDIO_DBG("\nSet ai volume %d return 0x%x\n", val, ret);
        } else if (input_cmd[0] == 'g' && input_cmd[1] == 'v') {
            ret = sample_comm_ai_get_volume(&g_ai_config, &val);
            SAMPLE_AUDIO_DBG("\nGet ai volume %d return 0x%x\n", val, ret);
        } else if (input_cmd[0] == 'd' && input_cmd[1] == 's' && input_cmd[2] == 'o') {
            system("echo setnode 1 > /proc/umap/ai");
            system("echo dump start > /proc/umap/ai");
            SAMPLE_AUDIO_DBG("\nSet dump orgin and ai post stream start\n");
        } else if (input_cmd[0] == 'd' && input_cmd[1] == 's' && input_cmd[2] == 'c') {
            system("echo dump stop > /proc/umap/ai");
            SAMPLE_AUDIO_DBG("\nSet dump stream stop \n");
        } else if ((input_cmd[0] == 'q') || (input_cmd[0] == 'Q')) {
            SAMPLE_AUDIO_DBG("Ready to exit\n");
            break;
        }
    }

ERR_START_AI:
    sample_stop_ai();
    xmedia_ai_exit();
    CHECK_RET(ret, "xmedia_ai_exit");
    return ret;
}

static xmedia_void user_args_usage(user_input_args* args)
{
    SAMPLE_AUDIO_DBG("usage: %s [dev] [samplerate] [bit_depth] [channels] [outfile] [volume]\n", args->sample);
    SAMPLE_AUDIO_DBG("dev: dev id\n");
    SAMPLE_AUDIO_DBG("samplerate: dev samplerate\n");
    SAMPLE_AUDIO_DBG("channels: dev chn cnt\n");
    SAMPLE_AUDIO_DBG("outfile: save file path\n");
    SAMPLE_AUDIO_DBG("volume: [0,68]\n");

    SAMPLE_AUDIO_DBG("examples:\n");
    SAMPLE_AUDIO_DBG("    %s 0 8000 16 2 out.pcm 40\n", args->sample);
    SAMPLE_AUDIO_DBG("    %s 1 8000 16 4 out.pcm 40\n", args->sample);
    SAMPLE_AUDIO_DBG("    %s 1 16000 16 4 out.pcm 40\n", args->sample);
}

static xmedia_s32 user_args_init(int argc, char* argv[], user_input_args* user_in_args)
{
	char file_name[128] = {0};
	xmedia_s32 i = 0;

    user_in_args->sample = argv[0];

    if ((argc >= 2) && (strcmp(argv[1], "-h")) == 0) {
        user_args_usage(user_in_args);
        return XMEDIA_FAILURE;
    }

    if (argc != 7) {
        printf("Invalid argc:%d\n", argc);
        user_args_usage(user_in_args);
        return XMEDIA_FAILURE;
    }

    user_in_args->dev = atoi(argv[1]);
    user_in_args->sample_rate = atoi(argv[2]);
    user_in_args->usr_sample_rate = user_in_args->sample_rate;
    user_in_args->bit_depth = atoi(argv[3]);
    user_in_args->channels = atoi(argv[4]);
    user_in_args->workmode = 1;
    user_in_args->samples_per_frm = user_in_args->sample_rate / 100;
	g_volume = atoi(argv[6]);

    memcpy(user_in_args->output_file_name, argv[5], strlen(argv[5]));
    if (strcmp(user_in_args->output_file_name, "null") == 0) {
        while (1) {
            memset(file_name, 0 , sizeof(file_name));
            if (user_in_args->sample_rate == 16000) {
                sprintf((void *)file_name,"audio_8ch_16k_file%d.pcm", i++);
            } else {
                sprintf((void *)file_name,"audio_8ch_8k_file%d.pcm", i++);
            }
            if (access(file_name, F_OK) == -1) {
                break;
            }
        }
        user_in_args->output_fd[0] = fopen(file_name, "w+");
        if (user_in_args->output_fd[0] == NULL) {
            printf("Open %s failed\n", user_in_args->output_file_name);
            return XMEDIA_FAILURE;
            }
    } else {
        user_in_args->output_fd[0] = fopen(user_in_args->output_file_name, "w+");
        if (user_in_args->output_fd[0] == NULL) {
            printf("Open %s failed\n", user_in_args->output_file_name);
            return XMEDIA_FAILURE;
        }
    }

    SAMPLE_AUDIO_DBG("get user args: \n");
    SAMPLE_AUDIO_DBG("  samplerate: %d\n", user_in_args->sample_rate);
    SAMPLE_AUDIO_DBG("  bit_depth: %d\n", user_in_args->bit_depth);
    SAMPLE_AUDIO_DBG("  chnnels: %d\n", user_in_args->channels);
    SAMPLE_AUDIO_DBG("  workmode: %d\n", user_in_args->workmode);
    SAMPLE_AUDIO_DBG("  samples_per_frm: %d\n", user_in_args->samples_per_frm);
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
    es7210_destroy();
    es7210b_config();
    es7210_destroy();

    ret = sample_audio_ai();
    if (ret != XMEDIA_SUCCESS) {
        printf("sample_auido_ai failed(0x%x)\n", ret);
    }
    user_args_deinit(&g_user_input_args);
    printf("exit sample ai\n");
    return ret;
}
