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
//#include "xmedia_audio_vqe_common.h"
#include "xmedia_audio_vqe.h"
#include "xmedia_audio_vqe_enhance_v1.h"

#include "sample_comm_audio.h"

#include "sample_audio.h"
#include "uac_log.h"

#define SAMPLE_AUDIO_DBG  printf
#define FRAME_TIME        10  //ms

// XMEDIA_AI_DEV_ADC0
// XMEDIA_AI_DEV_I2S0
// XMEDIA_AI_DEV_PDM0
// XMEDIA_AI_DEV_PDM_I2S0
#define AI_DEV XMEDIA_AI_DEV_ADC0
#define AI_CHANNELS  1

// XMEDIA_AO_DEV_DAC0
// XMEDIA_AO_DEV_I2S0
#define AO_DEV XMEDIA_AO_DEV_DAC0
#define AO_CHANNELS  1

static pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;
#define sample_audio_lock() pthread_mutex_lock(&mutex_lock)
#define sample_audio_unlock() pthread_mutex_unlock(&mutex_lock)

#define MIN(val1, val2) ((val1) > (val2) ? (val2) : (val1))

struct user_args {
    unsigned int channels;
    unsigned int sample_rate;
    unsigned int period_time; /* ms */
    unsigned int format;      /* audio_bit_depth */
    bool is_interleaved;
};

static unsigned int g_audio_init = 0;

static sample_ai_config g_ai_config = {
    .dev = AI_DEV,
};
static ai_vqe_attr_v1 ai_vqev1_attr = {0};
static struct user_args g_ai_args = {
    .channels = AI_CHANNELS,
    .sample_rate = AUDIO_SAMPLE_RATE_8000,
    .period_time = FRAME_TIME,
    .format = AUDIO_BIT_DEPTH_16,
    .is_interleaved = 1,
};

static sample_ao_config g_ao_config = {
    .dev = AO_DEV,
};
static ao_vqe_attr_v1 ao_vqev1_attr = {0};
static struct user_args g_ao_args = {
    .channels = AO_CHANNELS,
    .sample_rate = AUDIO_SAMPLE_RATE_8000,
    .period_time = FRAME_TIME,
    .format = AUDIO_BIT_DEPTH_16,
    .is_interleaved = 1,
};

static xmedia_void load_ai_vqev1_attr(ai_vqe_attr* attr)
{
    ai_vqe_attr_v1 *vqev1_attr = &ai_vqev1_attr;

    attr->version = AI_VQE_ENHANCE_ATTR_VERSION_1;
    attr->work_sample_rate = (g_ai_args.sample_rate == AUDIO_SAMPLE_RATE_8000) ?
        AUDIO_SAMPLE_RATE_8000 : AUDIO_SAMPLE_RATE_16000;
    attr->in_channels = g_ai_args.channels;
    attr->attr = (xmedia_void*)&ai_vqev1_attr;

    vqev1_attr->mask = VQE_V1_MASK_AGC | VQE_V1_MASK_ANR | VQE_V1_MASK_HPF;

    vqev1_attr->agc_attr.mode = VQE_V1_USR_MODE_AUTO;
    vqev1_attr->agc_attr.target_level = -6;
    vqev1_attr->agc_attr.max_boost_gain = 20;
    vqev1_attr->agc_attr.noise_floor = -100;
    vqev1_attr->agc_attr.ratio = 12;
    vqev1_attr->agc_attr.attack_time = 8;
    vqev1_attr->agc_attr.release_time =20;

    vqev1_attr->anr_attr.mode = VQE_V1_USR_MODE_AUTO;
    vqev1_attr->anr_attr.usr_scene = VQE_V1_ANR_SCENE_NORMAL;
    vqev1_attr->anr_attr.nr_mode = VQE_V1_NR_MODE_SPEECH;
    vqev1_attr->anr_attr.max_suppress_gain = 6;
    vqev1_attr->anr_attr.suppress_level = 0x2;
    vqev1_attr->anr_attr.nonstationary_suppress_level = 0x0;

    vqev1_attr->vad_attr.mode = VQE_V1_USR_MODE_AUTO;
    vqev1_attr->vad_attr.sensitivity_level = 4;

    vqev1_attr->hpf_attr.mode = VQE_V1_USR_MODE_AUTO;
    vqev1_attr->hpf_attr.freq = VQE_V1_HPF_FREQ_80;

    memset(&vqev1_attr->eq_attr, 0, sizeof(vqe_eq_attr_v1));

    vqev1_attr->eq_attr.gain[0] = 0;
    vqev1_attr->eq_attr.gain[1] = 0;
    vqev1_attr->eq_attr.gain[2] = 0;
    vqev1_attr->eq_attr.gain[3] = 0;
    vqev1_attr->eq_attr.gain[4] = 0;
    vqev1_attr->eq_attr.gain[5] = 0;
    vqev1_attr->eq_attr.gain[6] = 0;
    vqev1_attr->eq_attr.gain[7] = 0;
    vqev1_attr->eq_attr.gain[8] = 0;
    vqev1_attr->eq_attr.gain[9] = 0;
    vqev1_attr->eq_attr.gain[10] = 0;
    vqev1_attr->eq_attr.gain[11] = 0;
    vqev1_attr->eq_attr.gain[12] = 0;
    vqev1_attr->eq_attr.gain[13] = 0;
    vqev1_attr->eq_attr.gain[14] = 0;
    vqev1_attr->eq_attr.gain[15] = 0;
    vqev1_attr->eq_attr.gain[16] = 0;
    vqev1_attr->eq_attr.gain[17] = 0;
    vqev1_attr->eq_attr.gain[18] = 0;
    vqev1_attr->eq_attr.gain[19] = 0;
}

static xmedia_s32 sample_ai_config_init(xmedia_void)
{
    g_ai_config.dev = AI_DEV;

    sample_comm_ai_get_default_attr(g_ai_config.dev, &g_ai_config.dev_attr);
    g_ai_config.dev_attr.mode        = AUDIO_DEV_WORK_MODE_MONO;
    g_ai_config.dev_attr.channels    = g_ai_args.channels;
    g_ai_config.dev_attr.bit_depth   = g_ai_args.format;
    g_ai_config.dev_attr.sample_rate = g_ai_args.sample_rate;
    //g_ai_config.dev_attr.pcm_frame_num = ;
    g_ai_config.dev_attr.pcm_samples_per_frame = g_ai_args.period_time * g_ai_args.sample_rate / 1000;

    g_ai_config.chn_attr.sample_rate = g_ai_args.sample_rate;
    g_ai_config.chn_attr.interleaved = g_ai_args.is_interleaved;

    g_ai_config.res_enable = XMEDIA_FALSE;
    if ((g_ai_config.dev_attr.sample_rate != AUDIO_SAMPLE_RATE_8000 &&
                g_ai_config.dev_attr.sample_rate != AUDIO_SAMPLE_RATE_16000) ||
            (g_ai_config.chn_attr.sample_rate != g_ai_config.dev_attr.sample_rate)) {
        g_ai_config.res_enable = XMEDIA_TRUE;
    }

    g_ai_config.vqe_enable = XMEDIA_TRUE;
    load_ai_vqev1_attr(&g_ai_config.vqe_attr);

    g_ai_config.source = AI_EC_SOURCE_AO_DAC0;
    g_ai_config.ec_chn_id = 0;

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_start_ai()
{
    xmedia_s32 ret, vol;

    ret = sample_ai_config_init();
    CHECK_RET(ret, "sample_ai_config_init");

    ret = sample_comm_ai_register(ai_vqev1_attr.mask, g_ai_config.vqe_attr.version, g_ai_config.res_enable);
    CHECK_RET(ret, "sample_comm_ai_register");

    ret = sample_comm_ai_start(&g_ai_config);
    CHECK_RET(ret, "sample_comm_ai_start");

    vol = 10;
    ret = sample_comm_ai_set_volume(&g_ai_config, vol);
    if (ret != XMEDIA_SUCCESS) {
        uac_logw("Set ai volume %d return 0x%x.\n", vol, ret);
    }

    return ret;
}

static xmedia_s32 sample_stop_ai(xmedia_void)
{
    xmedia_s32 ret;

    ret = sample_comm_ai_stop(&g_ai_config);
    CHECK_RET(ret, "sample_comm_ai_stop");

    return ret;
}

static xmedia_s32 sample_recv_frame_from_ai(char* buf, int size, int time_out)
{
    xmedia_s32 ret;
    audio_frame frame;
    audio_ext_frame ext_frame;
    xmedia_u32 copy_size;
    int i;

    for (i = 0; i < g_ai_config.dev_attr.channels; i++) {
        ret = xmedia_ai_acquire_frame(g_ai_config.dev, i, &frame, &ext_frame, time_out);
        if (ret != XMEDIA_SUCCESS) {
            uac_loge("Acquire audio frame from AI failed, error=%d.\n", ret);
            return -1;
        }

        copy_size = MIN(size, frame.size);

        if (size != frame.size) {
            uac_logw("frame.size(%d) != buf.size(%d).\n", frame.size, size);
        }

        memcpy(buf, frame.data, copy_size);

        xmedia_ai_release_frame(g_ai_config.dev, i, &frame, &ext_frame);
    }

    return 0;
}

static xmedia_void load_ao_vqev1_attr(ao_vqe_attr* attr)
{
    ao_vqe_attr_v1 *vqev1_attr = &ao_vqev1_attr;

    attr->version = AO_VQE_ENHANCE_ATTR_VERSION_1;
    attr->attr = (xmedia_void*)&ao_vqev1_attr;

    vqev1_attr->mask = VQE_V1_MASK_AGC | VQE_V1_MASK_ANR | VQE_V1_MASK_HPF;

    vqev1_attr->agc_attr.mode = VQE_V1_USR_MODE_AUTO;
    vqev1_attr->agc_attr.target_level = -6;
    vqev1_attr->agc_attr.max_boost_gain = 20;
    vqev1_attr->agc_attr.noise_floor = -65;
    vqev1_attr->agc_attr.ratio = 10;
    vqev1_attr->agc_attr.attack_time = 20;
    vqev1_attr->agc_attr.release_time =20;

    vqev1_attr->anr_attr.mode = VQE_V1_USR_MODE_AUTO;
    vqev1_attr->anr_attr.usr_scene = VQE_V1_ANR_SCENE_NORMAL;
    vqev1_attr->anr_attr.nr_mode = VQE_V1_NR_MODE_SPEECH;
    vqev1_attr->anr_attr.max_suppress_gain = 6;
    vqev1_attr->anr_attr.suppress_level = 0x2;
    vqev1_attr->anr_attr.nonstationary_suppress_level = 0x2;

    vqev1_attr->hpf_attr.mode = VQE_V1_USR_MODE_AUTO;
    vqev1_attr->hpf_attr.freq = VQE_V1_HPF_FREQ_80;

    memset(&vqev1_attr->eq_attr, 0, sizeof(vqe_eq_attr_v1));

    vqev1_attr->eq_attr.gain[0] = 0;
    vqev1_attr->eq_attr.gain[1] = 0;
    vqev1_attr->eq_attr.gain[2] = 0;
    vqev1_attr->eq_attr.gain[3] = 0;
    vqev1_attr->eq_attr.gain[4] = 0;
    vqev1_attr->eq_attr.gain[5] = 0;
    vqev1_attr->eq_attr.gain[6] = 0;
    vqev1_attr->eq_attr.gain[7] = 0;
    vqev1_attr->eq_attr.gain[8] = 0;
    vqev1_attr->eq_attr.gain[9] = 0;
    vqev1_attr->eq_attr.gain[10] = 0;
    vqev1_attr->eq_attr.gain[11] = 0;
    vqev1_attr->eq_attr.gain[12] = 0;
    vqev1_attr->eq_attr.gain[13] = 0;
    vqev1_attr->eq_attr.gain[14] = 0;
    vqev1_attr->eq_attr.gain[15] = 0;
    vqev1_attr->eq_attr.gain[16] = 0;
    vqev1_attr->eq_attr.gain[17] = 0;
    vqev1_attr->eq_attr.gain[18] = 0;
    vqev1_attr->eq_attr.gain[19] = 0;
}

static xmedia_s32 sample_ao_config_init(xmedia_void)
{
    g_ao_config.dev = AO_DEV;

    sample_comm_ao_get_default_attr(g_ao_config.dev, &g_ao_config.dev_attr);
    g_ao_config.dev_attr.mode        = AUDIO_DEV_WORK_MODE_MONO;
    g_ao_config.dev_attr.channels    = g_ao_args.channels;
    g_ao_config.dev_attr.bit_depth   = g_ao_args.format;
    g_ao_config.dev_attr.sample_rate = g_ao_args.sample_rate;
    //g_ao_config.dev_attr.pcm_frame_max_num     = ;
    g_ao_config.dev_attr.pcm_samples_per_frame = g_ao_args.period_time * g_ao_args.sample_rate / 1000;

    g_ao_config.res_enable = XMEDIA_FALSE;
    if ((g_ao_config.dev_attr.sample_rate != AUDIO_SAMPLE_RATE_8000 &&
                g_ao_config.dev_attr.sample_rate != AUDIO_SAMPLE_RATE_16000) ||
            (g_ao_config.frame_info.frm_rate != g_ao_config.dev_attr.sample_rate)) {
        g_ao_config.res_enable = XMEDIA_TRUE;
    }

    g_ao_config.vqe_enable = XMEDIA_TRUE;
    load_ao_vqev1_attr(&g_ao_config.vqe_attr);
    //g_ao_config.binder_src = ;

    g_ao_config.frame_info.frm_rate = g_ao_args.sample_rate;
    g_ao_config.frame_info.frm_chn  = g_ao_args.channels;
    g_ao_config.frame_info.frm_bit  = g_ao_args.format;

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_start_ao()
{
    xmedia_s32 ret, vol;

    ret = sample_ao_config_init();
    CHECK_RET(ret, "sample_ao_config_init");

    ret = sample_comm_ao_register(ao_vqev1_attr.mask, g_ao_config.vqe_attr.version, g_ao_config.res_enable);
    CHECK_RET(ret, "sample_comm_ao_register");

    ret = sample_comm_ao_start(&g_ao_config);
    CHECK_RET(ret, "sample_comm_ao_start");

    vol = 10;
    ret = sample_comm_ao_set_volume(&g_ao_config, vol);
    if (ret != XMEDIA_SUCCESS) {
        uac_logw("Set ao volume %d return 0x%x.\n", vol, ret);
    }

    return ret;
}

static xmedia_s32 sample_stop_ao(xmedia_void)
{
    xmedia_s32 ret;

    ret = sample_comm_ao_stop(&g_ao_config);
    CHECK_RET(ret, "sample_comm_ao_stop");

    return ret;
}

static unsigned int get_sample_bytes(void)
{
    switch (g_ao_args.format) {
        case AUDIO_BIT_DEPTH_8:
            return 1;

        case AUDIO_BIT_DEPTH_16:
            return 2;

        case AUDIO_BIT_DEPTH_24:
            return 3;

        case AUDIO_BIT_DEPTH_32:
            return 4;

        case AUDIO_BIT_DEPTH_UNKNOWN:
        default:
            return 0;
    }
}

static xmedia_s32 sample_send_frame_to_ao(char* buf, int size, int time_out)
{
    int ret, i;
    audio_frame frame;
    xmedia_u32 send_size;
    unsigned int period_time = g_ao_args.period_time;
    unsigned int sample_bytes = get_sample_bytes();

    frame.bit_depth = g_ao_config.frame_info.frm_bit;
    frame.channels  = g_ao_config.frame_info.frm_chn;
    frame.interleaved = g_ao_args.is_interleaved;
    frame.sample_rate = g_ao_config.frame_info.frm_rate;
    frame.timestamp = -1;

    // frame.sample_rate / 1000 : Number of samples per millisecond
    // period_time : Milliseconds of one cycle
    // frame.channels * sample_bytes : The byte size of a sample.
    send_size = (frame.sample_rate  / 1000) * (frame.channels * sample_bytes) * period_time;

    frame.data = (xmedia_void*)buf;
    frame.size = MIN(send_size, size);

    if (send_size != size) {
        uac_logw("send_size(%d) != buf.size(%d).\n", send_size, size);
    }

    for (i = 0; i < g_ao_config.dev_attr.channels; i++) {
        ret = xmedia_ao_send_frame(g_ao_config.dev, i, &frame, time_out);
        if (ret != XMEDIA_SUCCESS) {
            uac_loge("xmedia_ao_send_frame failed, error=%d\n", ret);
            return -1;
        }
    }

    return 0;
}

static xmedia_s32 comm_audio_init()
{
    xmedia_s32 ret;

    ret = xmedia_ai_init();
    CHECK_RET(ret, "xmedia_ai_init");

    ret = xmedia_ao_init();
    CHECK_RET(ret, "xmedia_ao_init");

    return ret;
}

static xmedia_s32 comm_audio_exit()
{
    xmedia_s32 ret;

    ret = xmedia_ai_exit();
    CHECK_RET(ret, "xmedia_ai_exit");

    ret = xmedia_ao_exit();
    CHECK_RET(ret, "xmedia_ao_exit");

    return ret;
}

static xmedia_s32 sample_init_audio(void)
{
    int err = 0;

    sample_audio_lock();
    if (g_audio_init) {
        g_audio_init++;
        sample_audio_unlock();
        return 0;
    }

    err = comm_audio_init();
    if (err < 0) {
        uac_loge("sample_comm_audio_init failed.\n");
        sample_audio_unlock();
        return err;
    }

    g_audio_init++;
    sample_audio_unlock();

    return 0;
}

static xmedia_s32 sample_deinit_audio(void)
{
    int err = 0;

    sample_audio_lock();
    if (!g_audio_init) {
        sample_audio_unlock();
        return 0;
    } else {
        g_audio_init--;
    }

    if (g_audio_init == 0) {
        err = comm_audio_exit();
        if (err < 0) {
            uac_loge("comm_audio_exit failed.\n");
        }
    }

    sample_audio_unlock();

    return err;
}

int sample_audio_ai_config(unsigned int channels,
        unsigned int rate,
        unsigned int period_time,
        unsigned int format,
        bool is_interleaved)
{
    g_ai_args.channels = channels;
    g_ai_args.sample_rate = rate;
    g_ai_args.period_time = period_time;
    g_ai_args.format = format;
    g_ai_args.is_interleaved = is_interleaved;

    return 0;
}

int sample_audio_ai_startup(void)
{
    if (sample_init_audio() < 0) {
        return -1;
    }

    if (sample_start_ai() < 0) {
        sample_deinit_audio();
        return -1;
    }

    return 0;
}

int sample_audio_ai_shutdown(void)
{
    sample_stop_ai();

    sample_deinit_audio();

    return 0;
}

int sample_audio_get_frame_from_ai(char* buf, int size, int time_out)
{
    if (buf == NULL || size <= 0) {
        uac_loge("Invalid param.\n");
        return -1;
    }

    return sample_recv_frame_from_ai(buf, size, time_out);
}

int sample_audio_ao_config(unsigned int channels,
        unsigned int rate,
        unsigned int period_time,
        unsigned int format,
        bool is_interleaved)
{
    g_ao_args.channels = channels;
    g_ao_args.sample_rate = rate;
    g_ao_args.period_time = period_time;
    g_ao_args.format = format;
    g_ao_args.is_interleaved = is_interleaved;

    return 0;
}

int sample_audio_ao_startup(void)
{
    if (sample_init_audio() < 0) {
        return -1;
    }

    if (sample_start_ao() < 0) {
        sample_deinit_audio();
        return -1;
    }

    return 0;
}

int sample_audio_ao_shutdown(void)
{
    sample_stop_ao();

    sample_deinit_audio();
    return 0;
}

int sample_audio_send_frame_to_ao(char* buf, int size, int time_out)
{
    if (buf == NULL || size <= 0) {
        uac_loge("Invlaid params.\n");
        return -1;
    }

    return sample_send_frame_to_ao(buf, size, time_out);
}

