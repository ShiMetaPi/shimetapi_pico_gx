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

#define SAMPLE_GET_INPUTCMD(input_cmd) fgets((char *)(input_cmd), (sizeof(input_cmd) - 1), stdin)
static sample_ao_config g_ao_config = {0};
static ao_vqe_attr_v2 ao_vqev2_attr = {0};
static ao_vqe_attr_v1 ao_vqev1_attr = {0};

typedef struct {
    xmedia_char *sample;
    xmedia_char *in_stream;
    xmedia_char *dev;
    xmedia_char *dev_rate;
    xmedia_char *dev_chn;
    xmedia_char *frm_rate;
    xmedia_char *frm_chn;
    xmedia_char *mode;
    xmedia_char *frame_pcm_sample;
    xmedia_char *vqe;
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

    ctx->dev = atoi(arg->dev);
    ctx->dev_rate = atoi(arg->dev_rate);
    ctx->dev_chn = atoi(arg->dev_chn);
    ctx->frm_rate = atoi(arg->frm_rate);
    ctx->frm_chn = atoi(arg->frm_chn);
    ctx->mode = atoi(arg->mode);
    ctx->frame_pcm_sample = atoi(arg->frame_pcm_sample);
    ctx->vqe = atoi(arg->vqe);

    for(i = 0; i < 2; i++) {
        ctx->in_stream[i] = fopen(arg->in_stream, "rb");
        if (ctx->in_stream[i] == XMEDIA_NULL) {
            SAMPLE_AUDIO_DBG("open file %s error!\n", arg->in_stream);
            return XMEDIA_FAILURE;
        }
    }

    //SAMPLE_AUDIO_DBG("dev_rate=%d,frm_rate=%d,ctx->vqe=%d,mode=%d\n", ctx->dev_rate, ctx->frm_rate, ctx->vqe, ctx->mode);

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

static xmedia_void sample_ao_load_vqe_v1_attr(ao_vqe_attr *vqe_attr)
{
    ao_vqe_attr_v1 *vqev1_attr = &ao_vqev1_attr;

    vqe_attr->version = AO_VQE_ENHANCE_ATTR_VERSION_1;
    vqe_attr->attr = (xmedia_void*)&ao_vqev1_attr;

    vqev1_attr->mask = VQE_V1_MASK_AGC | VQE_V1_MASK_ANR | VQE_V1_MASK_HPF | VQE_V1_MASK_EQ;
    vqev1_attr->agc_attr.mode = VQE_V1_USR_MODE_AUTO;
    vqev1_attr->agc_attr.target_level = -8;
    vqev1_attr->agc_attr.max_boost_gain = 25;
    vqev1_attr->agc_attr.noise_floor = -120;
    vqev1_attr->agc_attr.ratio = 12;
    vqev1_attr->agc_attr.attack_time = 2;
    vqev1_attr->agc_attr.release_time = 20;

    vqev1_attr->anr_attr.mode = VQE_V1_USR_MODE_AUTO;
    vqev1_attr->anr_attr.usr_scene = VQE_V1_ANR_SCENE_NORMAL;
    vqev1_attr->anr_attr.nr_mode = VQE_V1_NR_MODE_SPEECH;
    vqev1_attr->anr_attr.max_suppress_gain = 6;
    vqev1_attr->anr_attr.suppress_level = 0x0;
    vqev1_attr->anr_attr.nonstationary_suppress_level = 0x2;

    vqev1_attr->hpf_attr.mode = VQE_V1_USR_MODE_AUTO;
    vqev1_attr->hpf_attr.freq = VQE_V1_HPF_FREQ_80;

    memset(&vqev1_attr->eq_attr, 0, sizeof(vqe_eq_attr_v1));
}

static xmedia_void sample_ao_load_vqe_v2_attr(ao_vqe_attr *vqe_attr)
{
    ao_vqe_attr_v2 *vqev2_attr = &ao_vqev2_attr;

    vqe_attr->version = AO_VQE_ENHANCE_ATTR_VERSION_2;
    vqe_attr->attr = (xmedia_void*)&ao_vqev2_attr;
    vqev2_attr->mask = VQE_V2_MASK_AGC | VQE_V2_MASK_ANR | VQE_V2_MASK_HPF | VQE_V2_MASK_EQ;


    vqev2_attr->agc_attr.mode = VQE_V2_USR_MODE_AUTO;
    vqev2_attr->agc_attr.target_level = -6;
    vqev2_attr->agc_attr.max_boost_gain = 20;
    vqev2_attr->agc_attr.noise_floor = -65;
    vqev2_attr->agc_attr.ratio = 10;
    vqev2_attr->agc_attr.attack_time = 20;
    vqev2_attr->agc_attr.release_time = 20;

    vqev2_attr->anr_attr.mode = VQE_V2_USR_MODE_AUTO;
    vqev2_attr->anr_attr.usr_scene = VQE_V2_ANR_SCENE_NORMAL;
    vqev2_attr->anr_attr.nr_mode = VQE_V2_NR_MODE_SPEECH;
    vqev2_attr->anr_attr.max_suppress_gain = 6;
    vqev2_attr->anr_attr.suppress_level = 0x2;
    vqev2_attr->anr_attr.nonstationary_suppress_level = 0x2;

    vqev2_attr->hpf_attr.mode = VQE_V2_USR_MODE_AUTO;
    vqev2_attr->hpf_attr.freq = VQE_V2_HPF_FREQ_80;

    memset(&vqev2_attr->eq_attr, 0, sizeof(vqe_eq_attr_v2));
}

static xmedia_void sample_ao_load_vqe_attr(ao_vqe_attr *vqe_attr, xmedia_u32 vqe_enable)
{
    if(vqe_enable == 1) {
        sample_ao_load_vqe_v1_attr(vqe_attr);
    } else if(vqe_enable == 2) {
        sample_ao_load_vqe_v2_attr(vqe_attr);
    }
}

static xmedia_s32 sample_ao_config_init(sample_ctx* ctx)
{
    g_ao_config.dev = ctx->dev;

    sample_comm_ao_get_default_attr(g_ao_config.dev, &g_ao_config.dev_attr);
    g_ao_config.dev_attr.pcm_frame_max_num = 10;
    g_ao_config.dev_attr.channels = ctx->dev_chn;
    g_ao_config.dev_attr.sample_rate = ctx->dev_rate;
    g_ao_config.dev_attr.pcm_samples_per_frame = ctx->frame_pcm_sample;
    g_ao_config.dev_attr.mode = ctx->mode;

    g_ao_config.frame_info.frm_rate = ctx->frm_rate;
    g_ao_config.frame_info.frm_chn = ctx->frm_chn;
    g_ao_config.frame_info.frm_bit = AUDIO_BIT_DEPTH_16;

    g_ao_config.res_enable = XMEDIA_FALSE;
    if ((g_ao_config.dev_attr.sample_rate != AUDIO_SAMPLE_RATE_8000 && g_ao_config.dev_attr.sample_rate != AUDIO_SAMPLE_RATE_16000)
        || (g_ao_config.frame_info.frm_rate != g_ao_config.dev_attr.sample_rate)) {
        g_ao_config.res_enable = XMEDIA_TRUE;
    }
    g_ao_config.vqe_enable = ctx->vqe;
    sample_ao_load_vqe_attr(&g_ao_config.vqe_attr, ctx->vqe);

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_start_ao(sample_ctx* ctx)
{
    xmedia_s32 ret;

    ret = sample_ao_config_init(ctx);
    CHECK_RET(ret, "sample_ao_config_init");

    if(ctx->vqe == 1) {
        ret = sample_comm_ao_register(ao_vqev1_attr.mask, g_ao_config.vqe_attr.version, g_ao_config.res_enable);
        CHECK_RET(ret, "sample_comm_ao_register");
    } else if(ctx->vqe == 2) {
        ret = sample_comm_ao_register(ao_vqev2_attr.mask, g_ao_config.vqe_attr.version, g_ao_config.res_enable);
        CHECK_RET(ret, "sample_comm_ao_register");
    } else { //可能存在只有重采样场景
        if (g_ao_config.res_enable) {
            ret = sample_comm_ao_register(0, AO_VQE_ENHANCE_ATTR_VERSION_1, g_ao_config.res_enable);
            CHECK_RET(ret, "sample_comm_ao_register");
        }
    }

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
    SAMPLE_AUDIO_DBG(" usage:      %s infile dev dev_rate dev_chn frm_rate frm_chn mode pcm_sample vqe\n", arg->sample);
    SAMPLE_AUDIO_DBG(" infile:     input file name,only support pcm file\n");
    SAMPLE_AUDIO_DBG(" dev:        (0/1)->(DAC0/I2S0)\n");
    SAMPLE_AUDIO_DBG(" dev_rate:   (8000/16000/32000/48000) -> dev attr chn\n");
    SAMPLE_AUDIO_DBG(" dev_chn:    (1/2)-> dev chn\n");
    SAMPLE_AUDIO_DBG(" frm_rate:   (8000/16000/32000/48000) -> user frame chn\n");
    SAMPLE_AUDIO_DBG(" frm_chn:    (1/2)-> user data chn\n");
    SAMPLE_AUDIO_DBG(" mode:       (0/1)->(mono/queue)\n");
    SAMPLE_AUDIO_DBG(" pcm_sample: frame_pcm_samples\n");
    SAMPLE_AUDIO_DBG(" vqe:        (0/1/2)-> vqe disable/enable v1/enable v2\n");
    SAMPLE_AUDIO_DBG(" examples:  \n");
    SAMPLE_AUDIO_DBG("             %s infile 0 8000 1 16000 1 0 80 0\n", arg->sample);
}

xmedia_s32 main(int argc, char* argv[])
{
    xmedia_s32 ret;
    sample_ctx ctx;

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
        goto out;
    }
    while (1) {
        SAMPLE_AUDIO_DBG("\ninput q to quit!\n");
        SAMPLE_AUDIO_DBG("input 'sm 0/1' to set codec unmute/mute!\n");
        SAMPLE_AUDIO_DBG("input 'sg xx' to set codec gain to xx db!\n");
        SAMPLE_AUDIO_DBG("input 'gg' to get codec gain xxdb!\n");
        SAMPLE_AUDIO_DBG("input 'sv xx' to set ao chn volume to xx db!\n");
        SAMPLE_GET_INPUTCMD(input_cmd);

        if (input_cmd[0] == 'q') {
            SAMPLE_AUDIO_DBG("prepare to quit!\n");
            break;
        }

        if (input_cmd[0] == 's' && input_cmd[1] == 'm') {
            xmedia_bool mute = atoi(input_cmd + 2);
            ret = sample_comm_ao_set_dev_param(&g_ao_config, AO_PARAM_CODEC_SET_MUTE, &mute);
            SAMPLE_AUDIO_DBG("\nset ao codec mute(%d) return 0x%x\n", mute, ret);
        }

        if (input_cmd[0] == 's' && input_cmd[1] == 'g') {
            xmedia_s32 gain = atoi(input_cmd + 2);
            ret = sample_comm_ao_set_dev_param(&g_ao_config, AO_PARAM_CODEC_SET_DAC_GAIN, &gain);
            SAMPLE_AUDIO_DBG("\nset ao codec gain(%d) return 0x%x\n", gain, ret);
        }

        if (input_cmd[0] == 'g' && input_cmd[1] == 'g') {
            xmedia_s32 gain;
            ret = sample_comm_ao_set_dev_param(&g_ao_config, AO_PARAM_CODEC_GET_DAC_GAIN, &gain);
            SAMPLE_AUDIO_DBG("\nget ao codec gain(%d) return 0x%x\n", gain, ret);
        }

        if (input_cmd[0] == 's' && input_cmd[1] == 'v') {
            xmedia_s32 vol = atoi(input_cmd + 2);
            ret = sample_comm_ao_set_volume(&g_ao_config, vol);
            SAMPLE_AUDIO_DBG("\nset ao chn volume(%d) return 0x%x\n", vol, ret);
        }
    }

    sample_stop_ao();
out:
    sample_comm_audio_exit();
    sample_ctx_deinit(&ctx);
    SAMPLE_AUDIO_DBG("exit %s\n", arg->sample);
    return ret;
}
