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
#include "xmedia_audio_ao.h"
#include "xmedia_audio_vqe.h"
#include "xmedia_audio_vqe_enhance_v1.h"
#include "xmedia_audio_vqe_enhance_v2.h"
#include "sample_comm_audio.h"

#define SAMPLE_AUDIO_DBG  printf
#define SLEEP_TIME 10000 /* 5ms */
#define INPUT_CMD_LENGTH 32

#define SEED_FRAME_TIME 10 /*10ms*/
#define ACQUIRE_FRAME_STATUS 1
#define SEND_FRAME_STATUS 0
#define DEFAULT_AO_FRAME_NUM 30

#define SAMPLE_GET_INPUTCMD(input_cmd) fgets((char *)(input_cmd), (sizeof(input_cmd) - 1), stdin)
static sample_ao_config g_ao_config = {0};
static xmedia_s32 gain = 0;

typedef struct {
    xmedia_char *sample;
    xmedia_char *dev_rate;
    xmedia_char *dev_chn;
    xmedia_char *gain;
    xmedia_char *in_stream;
} input_arg;

typedef struct {
    FILE *in_stream[2];
    ao_dev dev;
    ao_chn chn;
    xmedia_u32 vqe;
    xmedia_u32 frm_rate;
    xmedia_u32 frm_chn;
    xmedia_u32 dev_rate;
    xmedia_u32 dev_chn;
    xmedia_u32 frame_pcm_sample;
    audio_dev_work_mode mode;
    xmedia_u32 pcm_samples;
} sample_ctx;

static xmedia_s32 sample_ctx_init(const input_arg *arg, sample_ctx *ctx)
{
    xmedia_u32 i;

    ctx->dev = XMEDIA_AO_DEV_DAC0;
    ctx->dev_rate = atoi(arg->dev_rate);
    ctx->dev_chn = atoi(arg->dev_chn);
    ctx->frm_rate = atoi(arg->dev_rate);
    ctx->frm_chn = atoi(arg->dev_chn);
    ctx->mode = AUDIO_DEV_WORK_MODE_QUEUE;
    ctx->frame_pcm_sample = ctx->dev_rate / 100;
    ctx->vqe = 0;
    gain = atoi(arg->gain);

    for(i = 0; i < 2; i++) {
        ctx->in_stream[i] = fopen(arg->in_stream, "rb");
        if (ctx->in_stream[i] == XMEDIA_NULL) {
            SAMPLE_AUDIO_DBG("open file %s error!\n", arg->in_stream);
            return XMEDIA_FAILURE;
        }
    }
    return XMEDIA_SUCCESS;

}

static xmedia_s32 sample_ctx_deinit(sample_ctx *ctx)
{
    xmedia_u32 i;

    for(i = 0; i < 2; i++) {
        if (ctx->in_stream[i] != XMEDIA_NULL) {
            fclose(ctx->in_stream[i]);
            ctx->in_stream[i] = XMEDIA_NULL;
        }
    }
    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_ao_config_init(sample_ctx* ctx)
{
    g_ao_config.dev = ctx->dev;

    sample_comm_ao_get_default_attr(g_ao_config.dev, &g_ao_config.dev_attr);
    g_ao_config.dev_attr.pcm_frame_max_num = 10;
    g_ao_config.dev_attr.channels = ctx->dev_chn;
    g_ao_config.dev_attr.sample_rate = ctx->dev_rate;
    g_ao_config.dev_attr.pcm_samples_per_frame = ctx->frame_pcm_sample;
    g_ao_config.dev_attr.pcm_frame_max_num = DEFAULT_AO_FRAME_NUM;
    g_ao_config.dev_attr.mode = ctx->mode;

    g_ao_config.frame_info.frm_rate = ctx->frm_rate;
    g_ao_config.frame_info.frm_chn = ctx->frm_chn;
    g_ao_config.frame_info.frm_bit = AUDIO_BIT_DEPTH_16;

    g_ao_config.vqe_enable = ctx->vqe;
    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_start_ao(sample_ctx* ctx)
{
    xmedia_s32 ret;

    ret = sample_ao_config_init(ctx);
    CHECK_RET(ret, "sample_ao_config_init");

    ret = sample_comm_ao_start(&g_ao_config);
    CHECK_RET(ret, "sample_comm_ao_start");

    sample_comm_ao_create_input_thread_from_file(&g_ao_config, ctx->in_stream);

    return ret;
}

static xmedia_s32 sample_stop_ao()
{
    xmedia_s32 ret;

    sample_comm_ao_destroy_input_thread_from_file(&g_ao_config);

    ret = sample_comm_ao_stop(&g_ao_config);
    CHECK_RET(ret, "sample_comm_ao_stop");

    return ret;
}

static xmedia_void usage(input_arg *arg)
{
    SAMPLE_AUDIO_DBG("Usage:./aq_utils_test_speaker_vol [Samplerate] [Chn] [AoVol] [SpkFile]\n");
    SAMPLE_AUDIO_DBG("Parameter comment as follows:\n");
    SAMPLE_AUDIO_DBG("  [Samplerate]: Work Samplerate, 8000 or 16000\n");
    SAMPLE_AUDIO_DBG("  [Chn]: channel, 1 or 2\n");
    SAMPLE_AUDIO_DBG("  [AoVol]: ao vqe volume, range:[-81~18]\n");
    SAMPLE_AUDIO_DBG("  [SpkFile]: file for speaker playback\n");

    SAMPLE_AUDIO_DBG("Examples as follows:\n");
    SAMPLE_AUDIO_DBG("    ./aq_utils_test_speaker_vol 16000 1 0 test_signal_spk_volumn_16k_1ch_16bit.pcm\n");
    SAMPLE_AUDIO_DBG("    ./aq_utils_test_speaker_vol 8000  1 0 test_signal_spk_volumn_8k_1ch_16bit.pcm\n");
}

xmedia_s32 main(int argc, char* argv[])
{
    xmedia_s32 ret;
    sample_ctx ctx;
    xmedia_s32 val = 0;

    input_arg *arg = (input_arg *)argv;
    xmedia_char input_cmd[INPUT_CMD_LENGTH];

    if (argc != sizeof(input_arg) / sizeof(xmedia_char *)) {
        usage(arg);
        return -1;
    }
    memset(&ctx, 0, sizeof(sample_ctx));

    ret = sample_ctx_init(arg, &ctx);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_AUDIO_DBG("call sample_ctx_init failed(0x%x)\n", ret);
        return ret;
    }

    sample_comm_audio_init();

    ret = sample_start_ao(&ctx);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_AUDIO_DBG("call sample_start_ao failed(0x%x)\n", ret);
        goto START_AO_ERR;
    }
    ret = sample_comm_ao_set_volume(&g_ao_config, gain);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_AUDIO_DBG("call sample_start_ao failed(0x%x)\n", ret);
        goto SET_VOL_ERR;
    }

    while(1) {
        SAMPLE_AUDIO_DBG("\nInput 'q' ENTER to exit\n");
        SAMPLE_AUDIO_DBG("Input 'svXX' to set ao vqe volume to xx\n");
        SAMPLE_AUDIO_DBG("Input 'gv' to get ao vqe volume\n");

        fgets(input_cmd, (sizeof(input_cmd) - 1), stdin);
        if (input_cmd[0] == 's' && input_cmd[1] == 'v' ) {
            val = atoi(input_cmd + 2);
            ret = sample_comm_ao_set_volume(&g_ao_config, val);
            SAMPLE_AUDIO_DBG("\nSet ao vqe volume(%d) return 0x%x\n", val, ret);
        } else if (input_cmd[0] == 'g' && input_cmd[1] == 'v' ) {
            ret = sample_comm_ao_get_volume(&g_ao_config, &val);
            SAMPLE_AUDIO_DBG("\nGet ao vqe volume(%d) return 0x%x\n", val, ret);
        } else if (input_cmd[0] == 'q') {
            SAMPLE_AUDIO_DBG("Ready to exit\n");
            break;
        }
    }
SET_VOL_ERR:
    sample_stop_ao();
START_AO_ERR:
    sample_comm_audio_exit();
    sample_ctx_deinit(&ctx);
    SAMPLE_AUDIO_DBG("exit %s\n", arg->sample);
    return ret;
}
