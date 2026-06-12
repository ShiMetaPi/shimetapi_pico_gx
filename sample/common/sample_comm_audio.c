#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "defines.h"
#include "xmedia_sys.h"
#include "xmedia_audio_ai.h"
#include "xmedia_audio_ao.h"
#include "xmedia_audio_adec.h"
#include "xmedia_audio_aenc.h"
#include "xmedia_audio_vqe.h"
#include "xmedia_audio_codec.h"
#include "xmedia_audio_codec_voice.h"
#include "xmedia_audio_codec_mp3.h"
#include "xmedia_audio_codec_aac.h"
#include "xmedia_audio_vqe_enhance_v1.h"
#include "xmedia_audio_vqe_enhance_v2.h"
#include "xmedia_audio_vqe_enhance_v3.h"
#include "xmedia_audio_vqe_detect.h"

#include "sample_comm_audio.h"

#define MAX_AI_CREATE_CHN_CNT 3
typedef struct {
    ai_dev dev;
    ai_chn chn;
    pthread_t data_thd;
    xmedia_bool running_flag;
    xmedia_bool out_to_aenc;
    xmedia_bool out_to_ao;
    xmedia_bool out_to_file;
    ao_dev dev_ao;
    ao_chn chn_ao;
    FILE** file;
} comm_ai_ctx;

typedef struct {
    ao_dev_attr* attr;
    ao_dev dev;
    ao_chn chn;
    sample_ao_frame* frm_info;
    pthread_t data_thd;
    xmedia_bool running_flag;
    FILE** file;
} comm_ao_ctx;

typedef struct {
    ao_dev dev;
    ao_chn chn_ao;
    adec_chn chn;
    pthread_t input_thd;
    pthread_t output_thd;
    xmedia_bool input_running_flag;
    xmedia_bool output_running_flag;
    xmedia_bool send_to_ao;
    audio_codec_type codec_type;
    FILE** input_file;
    FILE** output_file;
} comm_adec_ctx;

typedef struct {
    ao_chn chn;
    audio_channel channels;
    audio_sample_rate samplerate;
    audio_bit_depth bit_depth;
    pthread_t input_thd;
    pthread_t output_thd;
    xmedia_bool input_running_flag;
    xmedia_bool output_running_flag;
    audio_codec_type codec_type;
    FILE** input_file;
    FILE** output_file;
} comm_aenc_ctx;

static comm_ai_ctx g_ai_ctx[3] = {0};
static comm_ao_ctx g_ao_ctx[3] = {0};
static comm_adec_ctx g_adec_ctx[ADEC_MAX_CHN_NUM] = {0};
static comm_aenc_ctx g_aenc_ctx[AENC_MAX_CHN_NUM] = {0};
static xmedia_bool g_share_mode_enable = XMEDIA_FALSE;

static xmedia_s32 regist_vqe_lib(xmedia_u32 mask, xmedia_u32 version, xmedia_bool res_enable)
{
    xmedia_s32 ret = XMEDIA_FAILURE;

    if (version == AI_VQE_ENHANCE_ATTR_VERSION_1) {
        vqe_handle_v1  libv1 = {0};
        if (mask & VQE_V1_MASK_HPF) {
            libv1.hpf_handle = xmedia_vqe_v1_get_hpf_handle();
        }

        if (mask & VQE_V1_MASK_AEC) {
            libv1.aec_handle = xmedia_vqe_v1_get_aec_handle();
        }

        if (mask & VQE_V1_MASK_ANR) {
            libv1.anr_handle = xmedia_vqe_v1_get_anr_handle();
        }

        if (mask & VQE_V1_MASK_AGC) {
            libv1.agc_handle = xmedia_vqe_v1_get_agc_handle();
        }

        if (mask & VQE_V1_MASK_VAD) {
            libv1.vad_handle = xmedia_vqe_v1_get_vad_handle();
        }

        if (mask & VQE_V1_MASK_WNS) {
            libv1.wns_handle = xmedia_vqe_v1_get_wns_handle();
        }

        if (mask & VQE_V1_MASK_EQ) {
            libv1.eq_handle = xmedia_vqe_v1_get_eq_handle();
        }

        if (mask & VQE_V1_MASK_DEREVERB) {
            libv1.dereverb_handle = xmedia_vqe_v1_get_dereverb_handle();
        }

        if(res_enable) {
            libv1.resampler_handle = xmedia_vqe_v1_get_resampler_handle();
        }
        ret = xmedia_vqe_register_module(VQE_V1_LIB_MASK, &libv1);
    } else if (version == AI_VQE_ENHANCE_ATTR_VERSION_2) {
        vqe_handle_v2  libv2 = {0};
        if (mask & VQE_V2_MASK_HPF) {
            libv2.hpf_handle = xmedia_vqe_v2_get_hpf_handle();
        }
        if (mask & VQE_V2_MASK_AEC) {
            libv2.aec_handle = xmedia_vqe_v2_get_aec_handle();
        }

        if (mask & VQE_V2_MASK_ANR) {
            libv2.anr_handle = xmedia_vqe_v2_get_anr_handle();
        }

        if (mask & VQE_V2_MASK_AGC) {
            libv2.agc_handle = xmedia_vqe_v2_get_agc_handle();
        }

        if (mask & VQE_V2_MASK_BF) {
            libv2.bf_handle = xmedia_vqe_v2_get_bf_handle();
        }

        if (mask & VQE_V2_MASK_VAD) {
            libv2.vad_handle = xmedia_vqe_v2_get_vad_handle();
        }

        if (mask & VQE_V2_MASK_WNS) {
            libv2.wns_handle = xmedia_vqe_v2_get_wns_handle();
        }

        if (mask & VQE_V2_MASK_EQ) {
            libv2.eq_handle = xmedia_vqe_v2_get_eq_handle();
        }

        if (mask & VQE_V2_MASK_DEREVERB) {
            libv2.dereverb_handle = xmedia_vqe_v2_get_dereverb_handle();
        }

        if(res_enable) {
            libv2.resampler_handle = xmedia_vqe_v2_get_resampler_handle();
        }
        ret = xmedia_vqe_register_module(VQE_V2_LIB_MASK, &libv2);
    } else if (version == AI_VQE_ENHANCE_ATTR_VERSION_3) {
        vqe_handle_v3  libv3 = {0};
        if (mask & VQE_V3_MASK_HPF) {
            libv3.hpf_handle = xmedia_vqe_v3_get_hpf_handle();
        }

        if (mask & VQE_V3_MASK_ANR) {
            libv3.anr_handle = xmedia_vqe_v3_get_anr_handle();
        }

        if (mask & VQE_V3_MASK_AGC) {
            libv3.agc_handle = xmedia_vqe_v3_get_agc_handle();
        }

        if (mask & VQE_V3_MASK_EQ) {
            libv3.eq_handle = xmedia_vqe_v3_get_eq_handle();
        }

        if(res_enable) {
            libv3.resampler_handle = xmedia_vqe_v3_get_resampler_handle();
        }
        ret = xmedia_vqe_register_module(VQE_V3_LIB_MASK, &libv3);
    } else if (version == AI_VQE_DETECT_ATTR_VERSION_1) {
        vqe_handle_detect libdet = {0};
        if (mask & VQE_DETECT_MASK_HPF) {
            libdet.hpf_handle = xmedia_vqe_detect_get_hpf_handle();
        }

        if (mask & VQE_DETECT_MASK_BCD) {
            libdet.bcd_handle = xmedia_vqe_detect_get_bcd_handle();
        }

        if (mask & VQE_DETECT_MASK_VED) {
            libdet.ved_handle = xmedia_vqe_detect_get_ved_handle();
        }

        if (mask & VQE_DETECT_MASK_SSL) {
            libdet.ssl_handle = xmedia_vqe_detect_get_ssl_handle();
        }

        if (mask & VQE_DETECT_MASK_GBD) {
            libdet.gbd_handle = xmedia_vqe_detect_get_gbd_handle();
        }

        ret = xmedia_vqe_register_module(VQE_DETECT_LIB_MASK, &libdet);
    }

    return ret;
}

static xmedia_s32 get_adec_attr(adec_attr* attr, sample_adec_config* ctx)
{
    attr->in_buffer_size = 8096;
    attr->out_frame_num = 8;
    attr->package_mode = XMEDIA_FALSE;
    attr->type = ctx->type;
    attr->param = (decoder_param*)malloc(sizeof(decoder_param));
    if (attr->param == XMEDIA_NULL) {
        return XMEDIA_FAILURE;
    }
    memset(attr->param, 0, sizeof(decoder_param));

    if (ctx->type == CODEC_TYPE_AAC) {
        aac_decoder_config* decoder_config = (aac_decoder_config*)malloc(sizeof(aac_decoder_config));
        if (decoder_config == XMEDIA_NULL) {
            return XMEDIA_FAILURE;
        }
        memset(decoder_config, 0, sizeof(aac_decoder_config));

        aac_dec_get_default_config(decoder_config);
        aac_dec_get_default_param(attr->param, decoder_config);
    } else if (ctx->type == CODEC_TYPE_MP3) {
        mp3_decoder_config* decoder_config = (mp3_decoder_config*)malloc(sizeof(mp3_decoder_config));
        if (decoder_config == XMEDIA_NULL) {
            return XMEDIA_FAILURE;
        }
        memset(decoder_config, 0, sizeof(mp3_decoder_config));

        mp3_dec_get_default_config(decoder_config);
        mp3_dec_get_default_param(attr->param, decoder_config);
    } else if (ctx->type == CODEC_TYPE_LPCM) {
        voice_dec_get_default_param(attr->param, XMEDIA_NULL);

        attr->param->desired_out_samplerate = ctx->sample_rate;
    } else {
        voice_decoder_config* decoder_config = (voice_decoder_config*)malloc(sizeof(voice_decoder_config));
        if (decoder_config == XMEDIA_NULL) {
            return XMEDIA_FAILURE;
        }
        memset(decoder_config, 0, sizeof(voice_decoder_config));

        voice_dec_get_default_config(decoder_config);
        voice_dec_get_default_param(attr->param, decoder_config);

        decoder_config->format = ctx->format;
        decoder_config->sample_rate = ctx->sample_rate;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 release_adec_attr(adec_attr* attr)
{
    if (!attr->param) {
        return XMEDIA_SUCCESS;
    }

    if (attr->param->decoder_config) {
        free(attr->param->decoder_config);
        attr->param->decoder_config = XMEDIA_NULL;
    }

    free(attr->param);
    attr->param = XMEDIA_NULL;

    return XMEDIA_SUCCESS;
}

static xmedia_s32 get_recommend_bit_rate(xmedia_u32 sample_rate)
{
    xmedia_s32 recommend_rate = 0;

    if ((sample_rate == 8000) || (sample_rate == 11025) || (sample_rate == 12000)) {
        recommend_rate = 32000;
    } else {
        recommend_rate = 128000;
    }

    return recommend_rate;
}

static xmedia_s32 get_aenc_attr(aenc_attr* attr, sample_aenc_config* ctx)
{
    attr->in_buffer_size = 8096;
    attr->out_frame_num = 8;
    attr->type = ctx->type;
    attr->param = (encoder_param*)malloc(sizeof(encoder_param));
    if (attr->param == XMEDIA_NULL) {
        return XMEDIA_FAILURE;
    }
    memset(attr->param, 0, sizeof(encoder_param));

    if (ctx->type == CODEC_TYPE_AAC) {
        aac_encoder_config* encoder_config = (aac_encoder_config*)malloc(sizeof(aac_encoder_config));
        if (encoder_config == XMEDIA_NULL) {
            return XMEDIA_FAILURE;
        }
        memset(encoder_config, 0, sizeof(aac_encoder_config));

        aac_enc_get_default_config(encoder_config);

        encoder_config->sample_rate = ctx->sample_rate;
        encoder_config->channels = ctx->channels;
        encoder_config->bit_width = ctx->bit_depth;
        encoder_config->band_width = ctx->sample_rate / 2;
        encoder_config->bit_rate = 48000;

        aac_enc_get_default_param(attr->param, encoder_config);
    } else if (ctx->type == CODEC_TYPE_MP3) {
        mp3_encoder_config* encoder_config = (mp3_encoder_config*)malloc(sizeof(mp3_encoder_config));
        if (encoder_config == XMEDIA_NULL) {
            return XMEDIA_FAILURE;
        }
        memset(encoder_config, 0, sizeof(mp3_encoder_config));

        mp3_enc_get_default_config(encoder_config);

        encoder_config->sample_rate = ctx->sample_rate;
        encoder_config->channels = ctx->channels;
        encoder_config->bit_per_sample = ctx->bit_depth;
        encoder_config->bit_rate = get_recommend_bit_rate(ctx->sample_rate);

        mp3_enc_get_default_param(attr->param, encoder_config);

    } else if (ctx->type == CODEC_TYPE_LPCM) {
                voice_encoder_config* encoder_config = (voice_encoder_config*)malloc(sizeof(voice_encoder_config));
        if (encoder_config == XMEDIA_NULL) {
            return XMEDIA_FAILURE;
        }
        memset(encoder_config, 0, sizeof(voice_encoder_config));

        voice_enc_get_default_config(encoder_config);
        voice_enc_get_default_param(attr->param, encoder_config);
        attr->param->sample_rate = ctx->sample_rate;
        attr->param->channels = ctx->channels;
        attr->param->bit_per_sample = ctx->bit_depth;
    } else {
        voice_encoder_config* encoder_config = (voice_encoder_config*)malloc(sizeof(voice_encoder_config));
        if (encoder_config == XMEDIA_NULL) {
            return XMEDIA_FAILURE;
        }
        memset(encoder_config, 0, sizeof(voice_encoder_config));

        voice_enc_get_default_config(encoder_config);
        encoder_config->format = ctx->format;
        voice_enc_get_default_param(attr->param, encoder_config);
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 release_aenc_attr(aenc_attr* attr)
{
    if (!attr->param) {
        return XMEDIA_SUCCESS;
    }

    if (attr->param->encoder_config) {
        free(attr->param->encoder_config);
        attr->param->encoder_config = XMEDIA_NULL;
    }

    free(attr->param);
    attr->param = XMEDIA_NULL;

    return XMEDIA_SUCCESS;
}

xmedia_s32 ai_get_default_attr(ai_dev dev, ai_dev_attr* dev_attr)
{
    xmedia_s32 ret;

    ret = xmedia_ai_get_default_attr(dev, dev_attr);
    CHECK_RET(ret, "xmedia_ai_get_default_attr");

    return ret;
}

static xmedia_s32 ai_start_dev(sample_ai_config* config)
{
    xmedia_s32 ret;

    ret = xmedia_ai_set_dev_attr(config->dev, &config->dev_attr);
    CHECK_RET(ret, "xmedia_ai_set_dev_attr");

    ret = xmedia_ai_enable_dev(config->dev);
    CHECK_RET(ret, "xmedia_ai_enable_dev");

    return ret;
}

static xmedia_s32 ai_start_chn(sample_ai_config* config)
{
    xmedia_s32 i;
    xmedia_s32 ret;
    xmedia_u32 chn_cnt;

    chn_cnt =  (config->dev_attr.mode == AUDIO_DEV_WORK_MODE_QUEUE) ? 1 : config->dev_attr.channels;
    chn_cnt = (chn_cnt > MAX_AI_CREATE_CHN_CNT) ? MAX_AI_CREATE_CHN_CNT : chn_cnt;
    for (i = 0; i < chn_cnt; i++) {
        ret = xmedia_ai_create_chn(config->dev, i, &config->chn_attr);
        CHECK_RET(ret, "xmedia_ai_create_chn");

        if (config->vqe_enable && g_share_mode_enable == XMEDIA_FALSE) {
            ret = xmedia_ai_set_ec_source(config->dev, i, config->source, config->ec_chn_id);
            CHECK_RET(ret, "xmedia_ai_set_ec_source");

            ret = xmedia_ai_set_vqe_attr(config->dev, i, &config->vqe_attr);
            CHECK_RET(ret, "xmedia_ai_create_chn");

            ret = xmedia_ai_enable_vqe(config->dev, i);
            CHECK_RET(ret, "xmedia_ai_enable_vqe");
        }

        ret = xmedia_ai_enable_chn(config->dev, i);
        CHECK_RET(ret, "xmedia_ai_enable_chn");
    }

    return ret;
}

static xmedia_s32 ai_stop_dev(sample_ai_config* config)
{
    xmedia_s32 ret;

    ret = xmedia_ai_disable_dev(config->dev);
    CHECK_RET(ret, "xmedia_ai_disable_dev");

    return ret;
}

static xmedia_s32 ai_stop_chn(sample_ai_config* config)
{
    xmedia_s32 i;
    xmedia_s32 ret;
    xmedia_u32 chn_cnt;

    chn_cnt =  (config->dev_attr.mode == AUDIO_DEV_WORK_MODE_QUEUE) ? 1 : config->dev_attr.channels;
    chn_cnt = (chn_cnt > MAX_AI_CREATE_CHN_CNT) ? MAX_AI_CREATE_CHN_CNT : chn_cnt;
    for (i = 0; i < chn_cnt; i++) {
        if (config->vqe_enable && g_share_mode_enable == XMEDIA_FALSE) {
            ret = xmedia_ai_disable_vqe(config->dev, i);
            CHECK_RET(ret, "xmedia_ai_disable_vqe");
        }

        ret = xmedia_ai_disable_chn(config->dev, i);
        CHECK_RET(ret, "xmedia_ai_disable_chn");

        ret = xmedia_ai_destroy_chn(config->dev, i);
        CHECK_RET(ret, "xmedia_ai_create_chn");
    }

    return ret;
}

static xmedia_s32 ai_set_volume(sample_ai_config* config, xmedia_s32 volume)
{
    xmedia_s32 i;
    xmedia_s32 ret;
    xmedia_u32 chn_cnt;

    chn_cnt =  (config->dev_attr.mode == AUDIO_DEV_WORK_MODE_QUEUE) ? 1 : config->dev_attr.channels;
    for (i = 0; i < chn_cnt; i++) {
        ret = xmedia_ai_set_volume(config->dev, i, volume);
        CHECK_RET(ret, "xmedia_ai_set_volume");
    }

    return ret;
}

static xmedia_s32 ai_get_volume(sample_ai_config* config, xmedia_s32* volume)
{
    xmedia_s32 i;
    xmedia_s32 ret;
    xmedia_u32 chn_cnt;

    chn_cnt =  (config->dev_attr.mode == AUDIO_DEV_WORK_MODE_QUEUE) ? 1 : config->dev_attr.channels;
    for (i = 0; i < chn_cnt; i++) {
        ret = xmedia_ai_get_volume(config->dev, i, volume);
        CHECK_RET(ret, "xmedia_ai_get_volume");
    }

    return ret;
}

static xmedia_s32 ai_set_ec_source(sample_ai_config* config)
{
    xmedia_s32 i;
    xmedia_s32 ret;
    xmedia_u32 chn_cnt;

    chn_cnt =  (config->dev_attr.mode == AUDIO_DEV_WORK_MODE_QUEUE) ? 1 : config->dev_attr.channels;
    for (i = 0; i < chn_cnt; i++) {
        ret = xmedia_ai_set_ec_source(config->dev, i, config->source, config->ec_chn_id);
        CHECK_RET(ret, "xmedia_ai_set_ec_source");
    }

    return ret;
}

static xmedia_s32 ai_enable_org_frame(sample_ai_config* config, xmedia_bool enable)
{
    xmedia_s32 i;
    xmedia_s32 ret;
    xmedia_u32 chn_cnt;

    chn_cnt =  (config->dev_attr.mode == AUDIO_DEV_WORK_MODE_QUEUE) ? 1 : config->dev_attr.channels;
    for (i = 0; i < chn_cnt; i++) {
        if (enable) {
            ret = xmedia_ai_enable_origin_frame(config->dev, i);
            CHECK_RET(ret, "xmedia_ai_enable_origin_frame");
        } else {
            ret = xmedia_ai_disable_origin_frame(config->dev, i);
            CHECK_RET(ret, "xmedia_ai_disable_origin_frame");
        }
    }

    return ret;
}

static xmedia_s32 ai_enable_ec_frame(sample_ai_config* config, xmedia_bool enable)
{
    xmedia_s32 i;
    xmedia_s32 ret;
    xmedia_u32 chn_cnt;

    chn_cnt =  (config->dev_attr.mode == AUDIO_DEV_WORK_MODE_QUEUE) ? 1 : config->dev_attr.channels;
    for (i = 0; i < chn_cnt; i++) {
        if (enable) {
            ret = xmedia_ai_enable_ec_frame(config->dev, i);
            CHECK_RET(ret, "xmedia_ai_enable_ec_frame");
        } else {
            ret = xmedia_ai_disable_ec_frame(config->dev, i);
            CHECK_RET(ret, "xmedia_ai_disable_ec_frame");
        }
    }

    return ret;
}

static xmedia_void *ai_data_thread(void *arg)
{
    xmedia_s32 ret;
    xmedia_s32 i;
    xmedia_s32 write_size = 0;
    xmedia_s32 time_out = 1000;
    audio_frame frame;
    audio_ext_frame ext_frame;
    comm_ai_ctx* ctx = (comm_ai_ctx*)arg;
    xmedia_u32 count = 0;
    int ffd = -1;

    while (ctx->running_flag) {
        ret = xmedia_ai_acquire_frame(ctx->dev, ctx->chn, &frame, &ext_frame, time_out);
        if (ret != XMEDIA_SUCCESS) {
            continue;
        }

        if (ctx->file && ctx->file[ctx->chn]) {
            write_size = ((frame.channels > 1) && (frame.interleaved == XMEDIA_FALSE)) ? (frame.size / frame.channels) : frame.size;
            if (frame.bit_depth == AUDIO_BIT_DEPTH_24) {
                xmedia_void* write_data;
                for (i = 0; i < write_size; i++) {
                    write_data = frame.data + i;
                    if (i % 4 == 0) {
                        continue;
                    }
                    fwrite(write_data, 1, 1, ctx->file[ctx->chn]);
                }
            } else {
                fwrite(frame.data, 1, write_size, ctx->file[ctx->chn]);
                if ((count % 10) == 0) {
                    ffd = fileno(ctx->file[ctx->chn]);
                    if (ffd != -1)
                    fsync(ffd);
                }
            }
        }

        if (ctx->out_to_aenc) {
            xmedia_aenc_send_frame(ctx->chn, &frame, -1);
        }

        if (ctx->out_to_ao) {
            xmedia_ao_send_frame(ctx->dev_ao, ctx->chn_ao, &frame, -1);
        }

        xmedia_ai_release_frame(ctx->dev, ctx->chn, &frame, &ext_frame);
    }

    return (xmedia_void*)XMEDIA_NULL;
}

static xmedia_s32 ai_create_output_thread_to_aenc(sample_ai_config* config)
{
    xmedia_s32 i;
    xmedia_s32 ret;
    xmedia_u32 chn_cnt;

    chn_cnt =  (config->dev_attr.mode == AUDIO_DEV_WORK_MODE_QUEUE) ? 1 : config->dev_attr.channels;
    chn_cnt = (chn_cnt > MAX_AI_CREATE_CHN_CNT) ? MAX_AI_CREATE_CHN_CNT : chn_cnt;
    for (i = 0; i < chn_cnt; i++) {
        g_ai_ctx[i].dev = config->dev;
        g_ai_ctx[i].chn = i;
        g_ai_ctx[i].out_to_aenc = XMEDIA_TRUE;
        g_ai_ctx[i].running_flag = XMEDIA_TRUE;
        ret = pthread_create(&g_ai_ctx[i].data_thd, 0, ai_data_thread, &g_ai_ctx[i]);
        CHECK_RET(ret, "pthread_create");
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 ai_create_output_thread_to_ao(sample_ai_config* config, ao_dev dev, ao_chn chn)
{
    xmedia_s32 i;
    xmedia_s32 ret;
    xmedia_u32 chn_cnt;

    chn_cnt =  (config->dev_attr.mode == AUDIO_DEV_WORK_MODE_QUEUE) ? 1 : config->dev_attr.channels;
    chn_cnt = (chn_cnt > MAX_AI_CREATE_CHN_CNT) ? MAX_AI_CREATE_CHN_CNT : chn_cnt;
    for (i = 0; i < chn_cnt; i++) {
        g_ai_ctx[i].dev = config->dev;
        g_ai_ctx[i].chn = i;
        g_ai_ctx[i].out_to_ao = XMEDIA_TRUE;
        g_ai_ctx[i].dev_ao = dev;
        g_ai_ctx[i].chn_ao = chn;
        g_ai_ctx[i].running_flag = XMEDIA_TRUE;
        ret = pthread_create(&g_ai_ctx[i].data_thd, 0, ai_data_thread, &g_ai_ctx[i]);
        CHECK_RET(ret, "pthread_create");
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 ai_create_output_thread_to_file(sample_ai_config* config, FILE** file)
{
    xmedia_s32 i;
    xmedia_s32 ret;
    xmedia_u32 chn_cnt;

    chn_cnt =  (config->dev_attr.mode == AUDIO_DEV_WORK_MODE_QUEUE) ? 1 : config->dev_attr.channels;
    chn_cnt = (chn_cnt > MAX_AI_CREATE_CHN_CNT) ? MAX_AI_CREATE_CHN_CNT : chn_cnt;
    for (i = 0; i < chn_cnt; i++) {
        g_ai_ctx[i].chn = i;
        g_ai_ctx[i].file = file;
        g_ai_ctx[i].running_flag = XMEDIA_TRUE;
        g_ai_ctx[i].dev = config->dev;
        ret = pthread_create(&g_ai_ctx[i].data_thd, 0, ai_data_thread, &g_ai_ctx[i]);
        CHECK_RET(ret, "pthread_create");
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 ai_destroy_output_thread_to_aenc(sample_ai_config* config)
{
    xmedia_s32 i;
    xmedia_u32 chn_cnt;

    chn_cnt =  (config->dev_attr.mode == AUDIO_DEV_WORK_MODE_QUEUE) ? 1 : config->dev_attr.channels;
    chn_cnt = (chn_cnt > MAX_AI_CREATE_CHN_CNT) ? MAX_AI_CREATE_CHN_CNT : chn_cnt;
    for (i = 0; i < chn_cnt; i++) {
        if (g_ai_ctx[i].data_thd) {
            g_ai_ctx[i].running_flag = XMEDIA_FALSE;
            g_ai_ctx[i].out_to_aenc = XMEDIA_FALSE;
            pthread_join(g_ai_ctx[i].data_thd, XMEDIA_NULL);
        }
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 ai_destroy_output_thread_to_ao(sample_ai_config* config)
{
    xmedia_s32 i;
    xmedia_u32 chn_cnt;

    chn_cnt =  (config->dev_attr.mode == AUDIO_DEV_WORK_MODE_QUEUE) ? 1 : config->dev_attr.channels;
    chn_cnt = (chn_cnt > MAX_AI_CREATE_CHN_CNT) ? MAX_AI_CREATE_CHN_CNT : chn_cnt;
    for (i = 0; i < chn_cnt; i++) {
        if (g_ai_ctx[i].data_thd) {
            g_ai_ctx[i].running_flag = XMEDIA_FALSE;
            g_ai_ctx[i].out_to_ao = XMEDIA_FALSE;
            pthread_join(g_ai_ctx[i].data_thd, XMEDIA_NULL);
        }
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 ai_destroy_output_thread_to_file(sample_ai_config* config)
{
    xmedia_s32 i;
    xmedia_u32 chn_cnt;

    chn_cnt =  (config->dev_attr.mode == AUDIO_DEV_WORK_MODE_QUEUE) ? 1 : config->dev_attr.channels;
    chn_cnt = (chn_cnt > MAX_AI_CREATE_CHN_CNT) ? MAX_AI_CREATE_CHN_CNT : chn_cnt;
    for (i = 0; i < chn_cnt; i++) {
        if (g_ai_ctx[i].data_thd) {
            g_ai_ctx[i].running_flag = XMEDIA_FALSE;
            pthread_join(g_ai_ctx[i].data_thd, XMEDIA_NULL);
            g_ai_ctx[i].file = XMEDIA_NULL;
        }
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 ao_get_default_attr(ao_dev dev, ao_dev_attr* dev_attr)
{
    xmedia_s32 ret;

    ret = xmedia_ao_get_default_attr(dev, dev_attr);
    CHECK_RET(ret, "xmedia_ao_get_default_attr");

    return ret;
}


static xmedia_s32 ao_start_dev(sample_ao_config* config)
{
    xmedia_s32 ret;

    ret = xmedia_ao_set_dev_attr(config->dev, &config->dev_attr);
    CHECK_RET(ret, "xmedia_ao_set_dev_attr");

    ret = xmedia_ao_enable_dev(config->dev);
    CHECK_RET(ret, "xmedia_ao_set_dev_attr");

    return ret;
}

static xmedia_s32 ao_start_chn(sample_ao_config* config)
{
    xmedia_s32 i;
    xmedia_s32 ret;
    xmedia_u32 chn_cnt;

    chn_cnt =  (config->dev_attr.mode == AUDIO_DEV_WORK_MODE_QUEUE) ? 1 : config->dev_attr.channels;
    for (i = 0; i < chn_cnt; i++) {
        ret = xmedia_ao_create_chn(config->dev, i);
        CHECK_RET(ret, "xmedia_ao_create_chn");

        if (config->vqe_enable) {
            ret = xmedia_ao_set_vqe_attr(config->dev, i, &config->vqe_attr);
            CHECK_RET(ret, "xmedia_ao_set_vqe_attr");

            ret = xmedia_ao_enable_vqe(config->dev, i);
            CHECK_RET(ret, "xmedia_ao_enable_vqe");
        }

        if (config->binder_src == MOD_ID_AI || config->binder_src == MOD_ID_ADEC) {
            ret = xmedia_ao_bind(config->dev, i, config->binder_src, i);
            CHECK_RET(ret, "xmedia_ao_bind");
        }

        ret = xmedia_ao_enable_chn(config->dev, i);
        CHECK_RET(ret, "xmedia_ao_enable_chn");
    }

    return ret;
}

static xmedia_s32 ao_stop_dev(sample_ao_config* config)
{
    xmedia_s32 ret;

    ret = xmedia_ao_disable_dev(config->dev);
    CHECK_RET(ret, "xmedia_ai_disable_dev");

    return ret;
}

static xmedia_s32 ao_stop_chn(sample_ao_config* config)
{
    xmedia_s32 i;
    xmedia_s32 ret;
    xmedia_u32 chn_cnt;

    chn_cnt =  (config->dev_attr.mode == AUDIO_DEV_WORK_MODE_QUEUE) ? 1 : config->dev_attr.channels;
    for (i = 0; i < chn_cnt; i++) {
        if (config->vqe_enable) {
            ret = xmedia_ao_disable_vqe(config->dev, i);
            CHECK_RET(ret, "xmedia_ao_disable_vqe");
        }

        ret = xmedia_ao_disable_chn(config->dev, i);
        CHECK_RET(ret, "xmedia_ao_disable_vqe");

        ret = xmedia_ao_destroy_chn(config->dev, i);
        CHECK_RET(ret, "xmedia_ao_disable_vqe");
    }

    return ret;
}

static xmedia_s32 ao_set_volume(sample_ao_config* config, xmedia_s32 volume)
{
    xmedia_s32 i;
    xmedia_s32 ret;
    xmedia_u32 chn_cnt;

    chn_cnt =  (config->dev_attr.mode == AUDIO_DEV_WORK_MODE_QUEUE) ? 1 : config->dev_attr.channels;
    for (i = 0; i < chn_cnt; i++) {
        ret = xmedia_ao_set_volume(config->dev, i, volume);
        CHECK_RET(ret, "xmedia_ao_set_volume");
    }

    return ret;
}

static xmedia_s32 ao_get_volume(sample_ao_config* config, xmedia_s32* volume)
{
    xmedia_s32 i;
    xmedia_s32 ret;
    xmedia_u32 chn_cnt;

    chn_cnt =  (config->dev_attr.mode == AUDIO_DEV_WORK_MODE_QUEUE) ? 1 : config->dev_attr.channels;
    for (i = 0; i < chn_cnt; i++) {
        ret = xmedia_ao_get_volume(config->dev, i, volume);
        CHECK_RET(ret, "xmedia_ao_get_volume");
    }

    return ret;
}

static xmedia_s32 ao_set_dev_params(sample_ao_config* config, ao_dev_param_cmd cmd, xmedia_void* param)
{
    xmedia_s32 ret;

    ret = xmedia_ao_set_dev_param(config->dev, cmd, param);
    CHECK_RET(ret, "xmedia_ao_set_dev_param");

    return ret;
}

static xmedia_void *ao_data_thread(void *arg)
{
    xmedia_s32 ret;
    xmedia_s32 time_out = -1;
    audio_frame frame;
    comm_ao_ctx* ctx = (comm_ao_ctx*)arg;
    xmedia_s32 send_size;
    xmedia_char* send_buffer = XMEDIA_NULL;

    frame.bit_depth = ctx->frm_info->frm_bit;
    frame.channels = ctx->frm_info->frm_chn;
    frame.interleaved = XMEDIA_TRUE;
    frame.sample_rate = ctx->frm_info->frm_rate;
    frame.timestamp = -1;
    send_size = frame.sample_rate * 10 * frame.channels * sizeof(xmedia_s16) / 1000;//10ms
    send_buffer = malloc(send_size);
    if(send_buffer == XMEDIA_NULL) {
        return (xmedia_void*)XMEDIA_NULL;
    }

    frame.data = (xmedia_void*)send_buffer;

    while (ctx->running_flag) {
        ret = fread(send_buffer, 1, send_size, ctx->file[ctx->chn]);
        if (ret < send_size) {
            rewind(ctx->file[ctx->chn]);
        }

        frame.size = ret;

        xmedia_ao_send_frame(ctx->dev, ctx->chn, &frame, time_out);
    }
    free(send_buffer);
    return (xmedia_void*)XMEDIA_NULL;
}

static xmedia_s32 ao_create_input_thread_from_file(sample_ao_config* config, FILE** file)
{
    xmedia_s32 i;
    xmedia_s32 ret;
    xmedia_u32 chn_cnt;

    chn_cnt =  (config->dev_attr.mode == AUDIO_DEV_WORK_MODE_QUEUE) ? 1 : config->dev_attr.channels;
    for (i = 0; i < chn_cnt; i++) {
        g_ao_ctx[i].dev = config->dev;
        g_ao_ctx[i].chn = i;
        g_ao_ctx[i].file = file;
        g_ao_ctx[i].attr = &config->dev_attr;
        g_ao_ctx[i].frm_info = &config->frame_info;
        g_ao_ctx[i].running_flag = XMEDIA_TRUE;
        ret = pthread_create(&g_ao_ctx[i].data_thd, 0, ao_data_thread, &g_ao_ctx[i]);
        CHECK_RET(ret, "pthread_create");
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 ao_destroy_input_thread_from_file(sample_ao_config* config)
{
    xmedia_s32 i;
    xmedia_u32 chn_cnt;

    chn_cnt =  (config->dev_attr.mode == AUDIO_DEV_WORK_MODE_QUEUE) ? 1 : config->dev_attr.channels;
    for (i = 0; i < chn_cnt; i++) {
        if (g_ao_ctx[i].data_thd) {
            g_ao_ctx[i].running_flag = XMEDIA_FALSE;
            pthread_join(g_ao_ctx[i].data_thd, XMEDIA_NULL);
            g_ao_ctx[i].file[i] = XMEDIA_NULL;
        }
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 adec_register_lib(audio_codec_type type)
{
    xmedia_s32 ret = XMEDIA_FAILURE;

    switch (type) {
        case CODEC_TYPE_G711:
            ret = xmedia_adec_register(CODEC_MASK_G711, xmedia_g711_dec_get_handle());
            break;
        case CODEC_TYPE_G726:
            ret = xmedia_adec_register(CODEC_MASK_G726, xmedia_g726_dec_get_handle());
            break;
        case CODEC_TYPE_ADPCM:
            ret = xmedia_adec_register(CODEC_MASK_ADPCM, xmedia_adpcm_dec_get_handle());
            break;
        case CODEC_TYPE_AAC:
            ret = xmedia_adec_register(CODEC_MASK_AAC, xmedia_aac_dec_get_handle());
            break;
        case CODEC_TYPE_MP3:
            ret = xmedia_adec_register(CODEC_MASK_MP3, xmedia_mp3_dec_get_handle());
            break;
        case CODEC_TYPE_LPCM:
            ret = xmedia_adec_register(CODEC_MASK_LPCM, xmedia_lpcm_dec_get_handle());
            break;
        default:
            break;
    }

    return ret;
}

static xmedia_s32 adec_unregister_lib(audio_codec_type type)
{
    xmedia_s32 ret = XMEDIA_FAILURE;

    switch (type) {
        case CODEC_TYPE_G711:
            ret = xmedia_adec_unregister(CODEC_MASK_G711);
            break;
        case CODEC_TYPE_G726:
            ret = xmedia_adec_unregister(CODEC_MASK_G726);
            break;
        case CODEC_TYPE_ADPCM:
            ret = xmedia_adec_unregister(CODEC_MASK_ADPCM);
            break;
        case CODEC_TYPE_AAC:
            ret = xmedia_adec_unregister(CODEC_MASK_AAC);
            break;
        case CODEC_TYPE_MP3:
            ret = xmedia_adec_unregister(CODEC_MASK_MP3);
            break;
        case CODEC_TYPE_LPCM:
            ret = xmedia_adec_unregister(CODEC_MASK_LPCM);
            break;
        default:
            break;
    }

    return ret;
}

static xmedia_s32 adec_start_chn(sample_adec_config* config)
{
    xmedia_s32 ret;
    adec_attr attr;

    ret = get_adec_attr(&attr, config);
    CHECK_RET(ret, "get_adec_attr");

    ret = xmedia_adec_create_chn(config->chn, (const adec_attr*)&attr);
    CHECK_RET(ret, "xmedia_adec_create_chn");

    ret = release_adec_attr(&attr);
    CHECK_RET(ret, "release_adec_attr");

    return ret;
}

static xmedia_s32 adec_stop_chn(sample_adec_config* config)
{
    xmedia_s32 ret;

    ret = xmedia_adec_destroy_chn(config->chn);
    CHECK_RET(ret, "xmedia_adec_destroy_chn");

    return ret;
}

static xmedia_void *adec_input_data_thread(void *arg)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 time_out = 100;
    xmedia_u32 send_cnt = 0;
    xmedia_s32 read_size = 0;
    xmedia_u32 head_size = 0;
    xmedia_bool voice_type = XMEDIA_FALSE;
    audio_stream stream;
    comm_adec_ctx* ctx = (comm_adec_ctx*)arg;
    xmedia_s32 send_size;
    xmedia_uchar send_buffer[4096];

    send_size = 1024;
    //for voice type, we need add 4 bytes head
    if ((ctx->codec_type == CODEC_TYPE_G711) || (ctx->codec_type == CODEC_TYPE_G726)) {
        voice_type = XMEDIA_TRUE;
        head_size = 4;
        //send size must be multiples of 80 for voice type
        send_size = 320;
    }

    while (ctx->input_running_flag) {
        if (ret == XMEDIA_SUCCESS) {
            read_size = fread(send_buffer + head_size, 1, send_size, ctx->input_file[ctx->chn]);
            if (read_size <= 0) {
                rewind(ctx->input_file[ctx->chn]);
                continue;
            }

            if (voice_type) {
                send_buffer[0] = 0x00; //sync byte
                send_buffer[1] = 0x01; //sync byte
                send_buffer[2] = (xmedia_uchar)((read_size / 2) & 0x00ff); //samples count low bit(0 ~ 7)
                send_buffer[3] = (xmedia_uchar)(((read_size / 2) & 0xff00) >> 8); //samples count high bit(8 ~ 15)
            }
        }

        stream.len = read_size + head_size;
        stream.index = send_cnt;
        stream.data = (xmedia_void*)send_buffer;
        stream.timestamp = ((send_cnt == 0) ? 0 : -1);

        ret = xmedia_adec_send_stream(ctx->chn, &stream, time_out);
        if (ret == XMEDIA_SUCCESS) {
            send_cnt++;
        }
    }

    return (xmedia_void*)XMEDIA_NULL;
}

static xmedia_void *adec_output_data_thread(void *arg)
{
    xmedia_s32 ret;
    xmedia_s32 time_out = 1000;
    audio_frame frame;
    comm_adec_ctx* ctx = (comm_adec_ctx*)arg;

    while (ctx->output_running_flag) {
        ret = xmedia_adec_get_frame(ctx->chn, &frame, time_out);
        if (ret != XMEDIA_SUCCESS) {
            continue;
        }

        if (ctx->output_file && ctx->output_file[ctx->chn]) {
            fwrite(frame.data, 1, frame.size, ctx->output_file[ctx->chn]);
        }

        if (ctx->send_to_ao) {
            xmedia_ao_send_frame(ctx->dev, ctx->chn_ao, &frame, time_out);
        }

        xmedia_adec_release_frame(ctx->chn, &frame);
    }

    return (xmedia_void*)XMEDIA_NULL;
}

static xmedia_s32 adec_create_input_thread_from_file(sample_adec_config* config, FILE** file)
{
    xmedia_s32 ret;

    g_adec_ctx[config->chn].chn = config->chn;
    g_adec_ctx[config->chn].input_file = file;
    g_adec_ctx[config->chn].input_running_flag = XMEDIA_TRUE;
    g_adec_ctx[config->chn].codec_type = config->type;
    ret = pthread_create(&g_adec_ctx[config->chn].input_thd, 0, adec_input_data_thread, &g_adec_ctx[config->chn]);
    CHECK_RET(ret, "pthread_create");

    return XMEDIA_SUCCESS;
}

static xmedia_s32 adec_destroy_input_thread_from_file(sample_adec_config* config)
{
    g_adec_ctx[config->chn].input_running_flag = XMEDIA_FALSE;
    pthread_join(g_adec_ctx[config->chn].input_thd, XMEDIA_NULL);
    g_adec_ctx[config->chn].input_file = XMEDIA_NULL;

    return XMEDIA_SUCCESS;
}

static xmedia_s32 adec_create_output_thread_to_ao(sample_adec_config* config, ao_dev dev, ao_chn chn)
{
    xmedia_s32 ret;

    g_adec_ctx[config->chn].chn = config->chn;
    g_adec_ctx[config->chn].dev = dev;
    g_adec_ctx[config->chn].chn_ao = chn;
    g_adec_ctx[config->chn].send_to_ao = XMEDIA_TRUE;
    g_adec_ctx[config->chn].output_running_flag = XMEDIA_TRUE;
    ret = pthread_create(&g_adec_ctx[config->chn].output_thd, 0, adec_output_data_thread, &g_adec_ctx[config->chn]);
    CHECK_RET(ret, "pthread_create");
    return XMEDIA_SUCCESS;
}

static xmedia_s32 adec_destroy_output_thread_to_ao(sample_adec_config* config)
{
    g_adec_ctx[config->chn].output_running_flag = XMEDIA_FALSE;
    g_adec_ctx[config->chn].send_to_ao = XMEDIA_FALSE;
    pthread_join(g_adec_ctx[config->chn].output_thd, XMEDIA_NULL);

    return XMEDIA_SUCCESS;
}

static xmedia_s32 adec_create_output_thread_to_file(sample_adec_config* config, FILE** output_file)
{
    xmedia_s32 ret;

    g_adec_ctx[config->chn].chn = config->chn;
    g_adec_ctx[config->chn].output_file = output_file;
    g_adec_ctx[config->chn].output_running_flag = XMEDIA_TRUE;
    ret = pthread_create(&g_adec_ctx[config->chn].output_thd, 0, adec_output_data_thread, &g_adec_ctx[config->chn]);
    CHECK_RET(ret, "pthread_create");

    return XMEDIA_SUCCESS;
}

static xmedia_s32 adec_destroy_output_thread_to_file(sample_adec_config* config)
{
    g_adec_ctx[config->chn].output_running_flag = XMEDIA_FALSE;
    g_adec_ctx[config->chn].send_to_ao = XMEDIA_FALSE;
    pthread_join(g_adec_ctx[config->chn].output_thd, XMEDIA_NULL);
    g_adec_ctx[config->chn].output_file = XMEDIA_NULL;

    return XMEDIA_SUCCESS;
}


static xmedia_s32 aenc_register_lib(audio_codec_type type)
{
    xmedia_s32 ret = XMEDIA_FAILURE;

    switch (type) {
        case CODEC_TYPE_G711:
            ret = xmedia_aenc_register(CODEC_MASK_G711, xmedia_g711_enc_get_handle());
            break;
        case CODEC_TYPE_G726:
            ret = xmedia_aenc_register(CODEC_MASK_G726, xmedia_g726_enc_get_handle());
            break;
        case CODEC_TYPE_ADPCM:
            ret = xmedia_aenc_register(CODEC_MASK_ADPCM, xmedia_adpcm_enc_get_handle());
            break;
        case CODEC_TYPE_AAC:
            ret = xmedia_aenc_register(CODEC_MASK_AAC, xmedia_aac_enc_get_handle());
            break;
        case CODEC_TYPE_MP3:
            ret = xmedia_aenc_register(CODEC_MASK_MP3, xmedia_mp3_enc_get_handle());
            break;
        case CODEC_TYPE_LPCM:
            ret = xmedia_aenc_register(CODEC_MASK_LPCM, xmedia_lpcm_enc_get_handle());
            break;
        default:
            break;
    }

    return ret;
}

static xmedia_s32 aenc_unregister_lib(audio_codec_type type)
{
    xmedia_s32 ret = XMEDIA_FAILURE;

    switch (type) {
        case CODEC_TYPE_G711:
            ret = xmedia_aenc_unregister(CODEC_MASK_G711);
            break;
        case CODEC_TYPE_G726:
            ret = xmedia_aenc_unregister(CODEC_MASK_G726);
            break;
        case CODEC_TYPE_ADPCM:
            ret = xmedia_aenc_unregister(CODEC_MASK_ADPCM);
            break;
        case CODEC_TYPE_AAC:
            ret = xmedia_aenc_unregister(CODEC_MASK_AAC);
            break;
        case CODEC_TYPE_MP3:
            ret = xmedia_aenc_unregister(CODEC_MASK_MP3);
            break;
        case CODEC_TYPE_LPCM:
            ret = xmedia_aenc_unregister(CODEC_MASK_LPCM);
            break;
        default:
            break;
    }

    return ret;
}


static xmedia_s32 aenc_start_chn(sample_aenc_config* config)
{
    xmedia_s32 ret;
    aenc_attr attr;

    ret = get_aenc_attr(&attr, config);
    CHECK_RET(ret, "get_aenc_attr");

    ret = xmedia_aenc_create_chn(config->chn, (const aenc_attr*)&attr);
    CHECK_RET(ret, "xmedia_aenc_create_chn");

    if (config->binder_src == MOD_ID_AI) {
        ret = xmedia_aenc_bind(config->chn, config->binder_dev, 0);
        CHECK_RET(ret, "xmedia_aenc_binder");
    }

    ret = release_aenc_attr(&attr);
    CHECK_RET(ret, "release_aenc_attr");

    return ret;
}

static xmedia_s32 aenc_stop_chn(sample_aenc_config* config)
{
    xmedia_s32 ret;

    if (config->binder_src == MOD_ID_AI) {
        ret = xmedia_aenc_unbind(config->chn, config->binder_dev, 0);
        CHECK_RET(ret, "xmedia_aenc_binder");
    }

    ret = xmedia_aenc_destroy_chn(config->chn);
    CHECK_RET(ret, "xmedia_aenc_destroy_chn");

    return ret;
}

static xmedia_void *aenc_input_data_thread(void *arg)
{
    xmedia_s32 ret;
    xmedia_s32 time_out = -1;
    audio_frame frame;
    comm_aenc_ctx* ctx = (comm_aenc_ctx*)arg;
    xmedia_s32 send_size;
    xmedia_char send_buffer[4096];

    frame.bit_depth = ctx->bit_depth;
    frame.channels = ctx->channels;
    frame.interleaved = XMEDIA_TRUE;
    frame.sample_rate = ctx->samplerate;
    frame.timestamp = -1;
    frame.data = (xmedia_void*)send_buffer;
    send_size = ctx->samplerate * frame.channels * sizeof(xmedia_s16) * 10 / 1000;

    while (ctx->input_running_flag) {
        ret = fread(send_buffer, 1, send_size, ctx->input_file[ctx->chn]);
        if (ret < send_size) {
            rewind(ctx->input_file[ctx->chn]);
        }

        frame.size = ret;

        xmedia_aenc_send_frame(ctx->chn, &frame, time_out);
    }

    return (xmedia_void*)XMEDIA_NULL;
}

static xmedia_void *aenc_output_data_thread(void *arg)
{
    xmedia_s32 ret;
    xmedia_s32 time_out = -1;
    xmedia_u32 head_size = 0;
    audio_stream stream;
    comm_aenc_ctx* ctx = (comm_aenc_ctx*)arg;

    //for voice type stream, it contains 4 bytes head, we drop it to get origin stream
    if ((ctx->codec_type == CODEC_TYPE_G711) || (ctx->codec_type == CODEC_TYPE_G726)) {
        head_size = 4;
    }

    while (ctx->output_running_flag) {
        ret = xmedia_aenc_get_stream(ctx->chn, &stream, time_out);
        if (ret != XMEDIA_SUCCESS) {
            continue;
        }

        if (ctx->output_file[ctx->chn]) {
            fwrite(stream.data + head_size, 1, stream.len - head_size, ctx->output_file[ctx->chn]);
        }

        xmedia_aenc_release_stream(ctx->chn, &stream);
    }

    return (xmedia_void*)XMEDIA_NULL;
}



static xmedia_s32 aenc_create_input_thread_from_file(sample_aenc_config* config, FILE** file)
{
    xmedia_s32 ret;

    g_aenc_ctx[config->chn].chn = config->chn;
    g_aenc_ctx[config->chn].input_file = file;
    g_aenc_ctx[config->chn].samplerate = config->sample_rate;
    g_aenc_ctx[config->chn].channels = config->channels;
    g_aenc_ctx[config->chn].bit_depth = config->bit_depth;
    g_aenc_ctx[config->chn].input_running_flag = XMEDIA_TRUE;
    ret = pthread_create(&g_aenc_ctx[config->chn].input_thd, 0, aenc_input_data_thread, &g_aenc_ctx[config->chn]);
    CHECK_RET(ret, "pthread_create");

    return XMEDIA_SUCCESS;
}

static xmedia_s32 aenc_destroy_input_thread_from_file(sample_aenc_config* config)
{

    g_aenc_ctx[config->chn].input_running_flag = XMEDIA_FALSE;
    pthread_join(g_aenc_ctx[config->chn].input_thd, XMEDIA_NULL);
    g_aenc_ctx[config->chn].input_file = XMEDIA_NULL;

    return XMEDIA_SUCCESS;
}

static xmedia_s32 aenc_create_output_thread_to_file(sample_aenc_config* config, FILE** file)
{
    xmedia_s32 ret;

    g_aenc_ctx[config->chn].chn = config->chn;
    g_aenc_ctx[config->chn].output_file = file;
    g_aenc_ctx[config->chn].output_running_flag = XMEDIA_TRUE;
    g_aenc_ctx[config->chn].codec_type = config->type;
    ret = pthread_create(&g_aenc_ctx[config->chn].output_thd, 0, aenc_output_data_thread, &g_aenc_ctx[config->chn]);
    CHECK_RET(ret, "pthread_create");

    return XMEDIA_SUCCESS;
}

static xmedia_s32 aenc_destroy_output_thread_to_file(sample_aenc_config* config)
{

    g_aenc_ctx[config->chn].output_running_flag = XMEDIA_FALSE;
    pthread_join(g_aenc_ctx[config->chn].output_thd, XMEDIA_NULL);
    g_aenc_ctx[config->chn].output_file = XMEDIA_NULL;

    return XMEDIA_SUCCESS;
}


static xmedia_s32 comm_audio_init()
{
    xmedia_s32 ret;

    ret = xmedia_ai_init();
    CHECK_RET(ret, "xmedia_ai_init");

    ret = xmedia_ao_init();
    CHECK_RET(ret, "xmedia_ao_init");

    ret = xmedia_adec_init();
    CHECK_RET(ret, "xmedia_adec_init");

    ret = xmedia_aenc_init();
    CHECK_RET(ret, "xmedia_aenc_init");

    return ret;
}

static xmedia_s32 comm_audio_exit()
{
    xmedia_s32 ret;

    ret = xmedia_ai_exit();
    CHECK_RET(ret, "xmedia_ai_exit");

    ret = xmedia_ao_exit();
    CHECK_RET(ret, "xmedia_ao_exit");

    ret = xmedia_adec_exit();
    CHECK_RET(ret, "xmedia_adec_exit");

    ret = xmedia_aenc_exit();
    CHECK_RET(ret, "xmedia_aenc_exit");

    return ret;
}

xmedia_s32 sample_comm_audio_init()
{
    xmedia_s32 ret;
    xmedia_sys_config sys_config = {0};

    ret = xmedia_sys_init(&sys_config);
    CHECK_RET(ret, "xmedia_sys_init");

    ret = comm_audio_init();
    CHECK_RET(ret, "comm_audio_init");

    return ret;
}

xmedia_s32 sample_comm_audio_exit()
{
    xmedia_s32 ret;

    ret = comm_audio_exit();
    CHECK_RET(ret, "comm_audio_exit");

    ret = xmedia_sys_exit();
    CHECK_RET(ret, "xmedia_sys_exit");

    return ret;
}

xmedia_s32 sample_comm_ai_register(xmedia_u32 mask, ai_vqe_attr_version version, xmedia_bool res_enable)
{
    xmedia_s32 ret;

    ret = regist_vqe_lib(mask, version, res_enable);
    CHECK_RET(ret, "regist_vqe_lib");

    return ret;
}

xmedia_s32 sample_comm_ai_get_default_attr(ai_dev dev, ai_dev_attr* dev_attr)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(dev_attr);

    ret = ai_get_default_attr(dev, dev_attr);
    CHECK_RET(ret, "ai_get_default_attr");

    return ret;
}

xmedia_s32 sample_comm_ai_set_share_mode(xmedia_bool enable)
{
    xmedia_s32 ret;

    ret = xmedia_ai_set_share_mode(enable);
    CHECK_RET(ret, "xmedia_ai_set_share_mode");

    g_share_mode_enable = enable;
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_ai_start(sample_ai_config* config)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = ai_start_dev(config);
    CHECK_RET(ret, "ai_start_dev");

    ret = ai_start_chn(config);
    CHECK_RET(ret, "ai_start_chn");

    return ret;
}

xmedia_s32 sample_comm_ai_stop(sample_ai_config* config)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = ai_stop_chn(config);
    CHECK_RET(ret, "ai_stop_chn");

    ret = ai_stop_dev(config);
    CHECK_RET(ret, "ai_stop_dev");

    return ret;
}

xmedia_s32 sample_comm_ai_set_volume(sample_ai_config* config, xmedia_s32 volume)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = ai_set_volume(config, volume);
    CHECK_RET(ret, "ai_set_volume");

    return ret;
}

xmedia_s32 sample_comm_ai_get_volume(sample_ai_config* config, xmedia_s32* volume)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);
    CHECK_NULL_PTR(volume);

    ret = ai_get_volume(config, volume);
    CHECK_RET(ret, "ai_get_volume");

    return ret;
}

xmedia_s32 sample_comm_ai_set_dev_param(sample_ai_config* config, ai_dev_param_cmd cmd, xmedia_void* param)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);
    CHECK_NULL_PTR(param);

    ret = xmedia_ai_set_dev_param(config->dev, cmd, param);
    CHECK_RET(ret, "xmedia_ai_set_dev_param");

    return ret;
}

xmedia_s32 sample_comm_ai_set_trackmode(sample_ai_config* config, audio_track_mode trackmode)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = xmedia_ai_set_trackmode(config->dev, trackmode);
    CHECK_RET(ret, "xmedia_ai_set_trackmode");

    return ret;
}

xmedia_s32 sample_comm_ai_get_trackmode(sample_ai_config* config, audio_track_mode* trackmode)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);
    CHECK_NULL_PTR(trackmode);

    ret = xmedia_ai_get_trackmode(config->dev, trackmode);
    CHECK_RET(ret, "xmedia_ai_get_trackmode");

    return ret;
}

xmedia_s32 sample_comm_ai_set_ec_source(sample_ai_config* config)
{
    xmedia_s32 ret;
    CHECK_NULL_PTR(config);

    ret = ai_set_ec_source(config);
    CHECK_RET(ret, "ai_set_ec_source");

    return ret;
}

xmedia_s32 sample_comm_ai_enable_org_frame(sample_ai_config* config, xmedia_bool enable)
{
    xmedia_s32 ret;
    CHECK_NULL_PTR(config);

    ret = ai_enable_org_frame(config, enable);
    CHECK_RET(ret, "ai_enable_org_frame");

    return ret;
}

xmedia_s32 sample_comm_ai_enable_ec_frame(sample_ai_config* config, xmedia_bool enable)
{
    xmedia_s32 ret;
    CHECK_NULL_PTR(config);

    ret = ai_enable_ec_frame(config, enable);
    CHECK_RET(ret, "ai_enable_ec_frame");

    return ret;
}

xmedia_s32 sample_comm_ai_create_output_thread_to_aenc(sample_ai_config* config)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = ai_create_output_thread_to_aenc(config);
    CHECK_RET(ret, "ai_create_data_thread");

    return ret;
}

xmedia_s32 sample_comm_ai_destroy_output_thread_to_aenc(sample_ai_config* config)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = ai_destroy_output_thread_to_aenc(config);
    CHECK_RET(ret, "ai_create_data_thread");

    return ret;
}
xmedia_s32 sample_comm_ai_create_output_thread_to_ao(sample_ai_config* config, ao_dev dev, ao_chn chn)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = ai_create_output_thread_to_ao(config, dev, chn);
    CHECK_RET(ret, "ai_create_data_thread");

    return ret;
}
xmedia_s32 sample_comm_ai_destroy_output_thread_to_ao(sample_ai_config* config)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = ai_destroy_output_thread_to_ao(config);
    CHECK_RET(ret, "ai_create_data_thread");

    return ret;
}
xmedia_s32 sample_comm_ai_create_output_thread_to_file(sample_ai_config* config, FILE** output_file)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = ai_create_output_thread_to_file(config, output_file);
    CHECK_RET(ret, "ai_create_data_thread");

    return ret;
}
xmedia_s32 sample_comm_ai_destroy_output_thread_to_file(sample_ai_config* config)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = ai_destroy_output_thread_to_file(config);
    CHECK_RET(ret, "ai_create_data_thread");

    return ret;
}

xmedia_s32 sample_comm_detect_open(ai_dev dev, ai_vqe_attr* vqe_attr)
{
    xmedia_s32 ret;

    ret = xmedia_ai_disable_vqe(dev, 0);
    CHECK_RET(ret, "xmedia_ai_disable_vqe");

    ret = xmedia_ai_set_vqe_attr(dev, 0, vqe_attr);
    CHECK_RET(ret, "xmedia_ai_set_vqe_attr");
    ret = xmedia_ai_enable_vqe(dev, 0);
    CHECK_RET(ret, "xmedia_ai_enable_vqe");

    return XMEDIA_SUCCESS;
}
xmedia_s32 sample_comm_ao_register(xmedia_u32 mask, ao_vqe_attr_version version, xmedia_bool res_enable)
{
    xmedia_s32 ret;

    ret = regist_vqe_lib(mask, version, res_enable);
    CHECK_RET(ret, "vqe_register_lib");

    return ret;
}

xmedia_s32 sample_comm_ao_get_default_attr(ao_dev dev, ao_dev_attr* dev_attr)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(dev_attr);

    ret = ao_get_default_attr(dev, dev_attr);
    CHECK_RET(ret, "ao_get_default_attr");

    return ret;
}

xmedia_s32 sample_comm_ao_start(sample_ao_config* config)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = ao_start_dev(config);
    CHECK_RET(ret, "ao_start_dev");

    ret = ao_start_chn(config);
    CHECK_RET(ret, "ao_start_chn");

    return ret;
}

xmedia_s32 sample_comm_ao_stop(sample_ao_config* config)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = ao_stop_chn(config);
    CHECK_RET(ret, "ao_stop_chn");

    ret = ao_stop_dev(config);
    CHECK_RET(ret, "ao_stop_dev");

    return ret;
}

xmedia_s32 sample_comm_ao_set_volume(sample_ao_config* config, xmedia_s32 volume)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = ao_set_volume(config, volume);
    CHECK_RET(ret, "ao_set_volume");

    return ret;
}

xmedia_s32 sample_comm_ao_get_volume(sample_ao_config* config, xmedia_s32* volume)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = ao_get_volume(config, volume);
    CHECK_RET(ret, "ao_set_volume");

    return ret;
}

xmedia_s32 sample_comm_ao_set_dev_param(sample_ao_config* config, ao_dev_param_cmd cmd, xmedia_void* param)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);
    CHECK_NULL_PTR(param);

    ret = ao_set_dev_params(config, cmd, param);
    CHECK_RET(ret, "ai_set_dev_params");

    return ret;
}

xmedia_s32 sample_comm_ao_create_input_thread_from_file(sample_ao_config* config, FILE** input_file)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);
    CHECK_NULL_PTR(input_file);

    ret = ao_create_input_thread_from_file(config, input_file);
    CHECK_RET(ret, "ao_create_input_thread_from_file");

    return ret;

}
xmedia_s32 sample_comm_ao_destroy_input_thread_from_file(sample_ao_config* config)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = ao_destroy_input_thread_from_file(config);
    CHECK_RET(ret, "ao_destroy_input_thread_from_file");

    return ret;
}

xmedia_s32 sample_comm_adec_register(audio_codec_type type)
{
    xmedia_s32 ret;

    ret = adec_register_lib(type);
    CHECK_RET(ret, "adec_register_lib");

    return ret;
}

xmedia_s32 sample_comm_adec_unregister(audio_codec_type type)
{
    xmedia_s32 ret;

    ret = adec_unregister_lib(type);
    CHECK_RET(ret, "adec_unregister_lib");

    return ret;
}

xmedia_s32 sample_comm_adec_start(sample_adec_config* config)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = adec_start_chn(config);
    CHECK_RET(ret, "ai_start_chn");

    return ret;
}

xmedia_s32 sample_comm_adec_stop(sample_adec_config* config)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = adec_stop_chn(config);
    CHECK_RET(ret, "adec_stop_chn");

    return ret;
}

xmedia_s32 sample_comm_adec_create_input_thread_from_file(sample_adec_config* config, FILE** input_file)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);
    CHECK_NULL_PTR(input_file);

    ret = adec_create_input_thread_from_file(config, input_file);
    CHECK_RET(ret, "adec_create_input_thread_from_file");

    return ret;
}

xmedia_s32 sample_comm_adec_destroy_input_thread_from_file(sample_adec_config* config)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = adec_destroy_input_thread_from_file(config);
    CHECK_RET(ret, "adec_destroy_input_thread_from_file");

    return ret;
}

xmedia_s32 sample_comm_adec_create_output_thread_to_ao(sample_adec_config* config, ao_dev dev, ao_chn chn)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = adec_create_output_thread_to_ao(config, dev, chn);
    CHECK_RET(ret, "adec_create_output_thread_to_ao");

    return ret;
}

xmedia_s32 sample_comm_adec_destroy_output_thread_to_ao(sample_adec_config* config)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = adec_destroy_output_thread_to_ao(config);
    CHECK_RET(ret, "adec_destroy_output_thread_to_ao");

    return ret;
}
xmedia_s32 sample_comm_adec_create_output_thread_to_file(sample_adec_config* config, FILE** output_file)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);
    CHECK_NULL_PTR(output_file);

    ret = adec_create_output_thread_to_file(config, output_file);
    CHECK_RET(ret, "adec_create_output_thread_to_file");

    return ret;
}
xmedia_s32 sample_comm_adec_destroy_output_thread_to_file(sample_adec_config* config)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = adec_destroy_output_thread_to_file(config);
    CHECK_RET(ret, "adec_destroy_output_thread_to_file");

    return ret;
}

xmedia_s32 sample_comm_aenc_register(audio_codec_type type)
{
    xmedia_s32 ret;

    ret = aenc_register_lib(type);
    CHECK_RET(ret, "aenc_register_lib");

    return ret;
}

xmedia_s32 sample_comm_aenc_unregister(audio_codec_type type)
{
    xmedia_s32 ret;

    ret = aenc_unregister_lib(type);
    CHECK_RET(ret, "aenc_unregister_lib");

    return ret;
}


xmedia_s32 sample_comm_aenc_start(sample_aenc_config* config)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = aenc_start_chn(config);
    CHECK_RET(ret, "ai_start_chn");

    return ret;
}

xmedia_s32 sample_comm_aenc_stop(sample_aenc_config* config)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = aenc_stop_chn(config);
    CHECK_RET(ret, "adec_stop_chn");

    return ret;
}

xmedia_s32 sample_comm_aenc_create_input_thread_from_file(sample_aenc_config* config, FILE** input_file)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = aenc_create_input_thread_from_file(config, input_file);
    CHECK_RET(ret, "aenc_create_input_thread_from_file");

    return ret;
}

xmedia_s32 sample_comm_aenc_destroy_input_thread_from_file(sample_aenc_config* config)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = aenc_destroy_input_thread_from_file(config);
    CHECK_RET(ret, "aenc_destroy_output_thread_to_file");

    return ret;
}

xmedia_s32 sample_comm_aenc_create_output_thread_to_file(sample_aenc_config* config, FILE** output_file)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = aenc_create_output_thread_to_file(config, output_file);
    CHECK_RET(ret, "aenc_create_output_thread_to_file");

    return ret;
}

xmedia_s32 sample_comm_aenc_destroy_output_thread_to_file(sample_aenc_config* config)
{
    xmedia_s32 ret;

    CHECK_NULL_PTR(config);

    ret = aenc_destroy_output_thread_to_file(config);
    CHECK_RET(ret, "aenc_destroy_output_thread_to_file");

    return ret;
}


xmedia_s32 sample_comm_get_codec_info(xmedia_char* file_name, audio_codec_type* type, voice_format* format)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    CHECK_NULL_PTR(file_name);
    CHECK_NULL_PTR(type);

    if (strstr(file_name,".adpcm")) {
        *type = CODEC_TYPE_ADPCM;
         *format = VOICE_FORMAT_ADPCM_DVI4;
    } else if (strstr(file_name,".lpcm") || strstr(file_name,".pcm")) {
        *type = CODEC_TYPE_LPCM;
    } else if (strstr(file_name,".g711a") || strstr(file_name,".alaw")) {
        *type = CODEC_TYPE_G711;
        *format = VOICE_FORMAT_G711_A;
    } else if (strstr(file_name,".g711u") || strstr(file_name,".ulaw")) {
        *type = CODEC_TYPE_G711;
        *format = VOICE_FORMAT_G711_U;
    } else if (strstr(file_name,".g726")) {
        *type = CODEC_TYPE_G726;
        *format = VOICE_FORMAT_G726_16KBPS;
    } else if (strstr(file_name,".aac")) {
        *type = CODEC_TYPE_AAC;
    } else if (strstr(file_name,".mp3")) {
        *type = CODEC_TYPE_MP3;
    } else {
        printf("invalid file extension\n");
        ret = XMEDIA_FAILURE;
    }

    CHECK_RET(ret, "get_codec_info");

    return ret;
}



