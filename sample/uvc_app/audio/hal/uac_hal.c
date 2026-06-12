/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include <alsa/asoundlib.h>

#include "uac_hal.h"
#include "uac_log.h"

#define UAC_HOST_END 0
#define PCM_WAIT_TIME_MS 1000

struct data_buffer {
    char* addr;
    unsigned int size; // The unit is bytes, and the size is equal to period_stize.
};

struct user_args {
    char id[16];
    unsigned int rate;
    unsigned int channels;
    unsigned int buffer_time;
    unsigned int period_time;
    snd_pcm_format_t format;
    snd_pcm_sframes_t buffer_size; // number of samples, When calculating memory space, it is necessary to convert it into bytes.
    snd_pcm_sframes_t period_size; // number of samples, When calculating memory space, it is necessary to convert it into bytes.
};

static struct data_buffer g_playback_data = {0};
static snd_pcm_t* g_handle_playback = NULL;
static struct user_args p_args = {
    .id          = "playback",                /* playback */
    .rate        = 16000,                     /* stream rate */
    .channels    = 1,                         /* count of channels */
    .buffer_time = 40000,                     /* ring buffer length in us */
    .period_time = 10000,                     /* period time in us */
    .format      = SND_PCM_FORMAT_S16_LE,     /* sample format */
};

static struct data_buffer g_capture_data = {0};
static snd_pcm_t* g_handle_capture = NULL;
static struct user_args c_args = {
    .id          = "capture",                 /* capture */
    .rate        = 16000,                     /* stream rate */
    .channels    = 1,                         /* count of channels */
    .buffer_time = 40000,                     /* ring buffer length in us */
    .period_time = 10000,                     /* period time in us */
    .format      = SND_PCM_FORMAT_S16_LE,     /* sample format */
};

#if UAC_HOST_END
static char *device = "hw:0,0";              /* playback and capture device */
#else
static char *device = "default";              /* playback and capture device */
#endif

static int g_playback_run = 0;
static int g_capture_run  = 0;

static audio_stream_operate_t* uac_stream = NULL;

/******************************* UAC Stream *******************************************/

static int uac_stream_config_playback(unsigned int channels,
        unsigned int rate,
        unsigned int period_time,
        audio_stream_format_t format,
        int is_interleaved)
{
    if (uac_stream && uac_stream->playback_stream && uac_stream->playback_stream->config) {
        return uac_stream->playback_stream->config(channels, rate, period_time, format, is_interleaved);
    } else {
        uac_loge("Invalid config function for playback stream.\n");
        return -1;
    }
}

static int uac_stream_startup_playback(void)
{
    if (uac_stream && uac_stream->playback_stream && uac_stream->playback_stream->startup) {
        return uac_stream->playback_stream->startup();
    } else {
        uac_loge("Invalid startup function for playback stream.\n");
        return -1;
    }
}

static int uac_stream_shutdown_playback(void)
{
    if (uac_stream && uac_stream->playback_stream && uac_stream->playback_stream->shutdown) {
        return uac_stream->playback_stream->shutdown();
    } else {
        uac_loge("Invalid shutdown function for playback stream.\n");
        return -1;
    }
}

static int uac_stream_recv_from_ai(char* buf, int size, int timeout_ms) {
    if (uac_stream && uac_stream->playback_stream
            && uac_stream->playback_stream->recv_from_stream) {
        return uac_stream->playback_stream->recv_from_stream(buf, size, timeout_ms);
    } else {
        uac_loge("Invalid recv function for playback stream.\n");
        return -1;
    }
}

static int uac_stream_config_capture(unsigned int channels,
        unsigned int rate,
        unsigned int period_time,
        audio_stream_format_t format,
        int is_interleaved)
{
    if (uac_stream && uac_stream->capture_stream && uac_stream->capture_stream->config) {
        return uac_stream->capture_stream->config(channels, rate, period_time, format, is_interleaved);
    } else {
        uac_loge("Invalid contig function for capture stream.\n");
        return -1;
    }
}

static int uac_stream_startup_capture(void)
{
    if (uac_stream && uac_stream->capture_stream && uac_stream->capture_stream->startup) {
        return uac_stream->capture_stream->startup();
    } else {
        uac_loge("Invalid startup function for capture stream.\n");
        return -1;
    }
}

static int uac_stream_shutdown_capture(void)
{
    if (uac_stream && uac_stream->capture_stream && uac_stream->capture_stream->shutdown) {
        return uac_stream->capture_stream->shutdown();
    } else {
        uac_loge("Invalid shutdown function for capture.\n");
        return -1;
    }
}

static int uac_stream_send_to_ao(char* buf, int size, int timeout_ms)
{
    if (uac_stream && uac_stream->capture_stream
            && uac_stream->capture_stream->send_to_stream) {
        return uac_stream->capture_stream->send_to_stream(buf, size, timeout_ms);
    } else {
        uac_loge("Invalid send function for capture stream.\n");
        return -1;
    }
}

/**************************************************************************************/

/******************************* UAC Device *******************************************/
static int set_hwparams(snd_pcm_t* handle, snd_pcm_hw_params_t* params, struct user_args* args)
{
    int err;
    unsigned int val, sample_rate;
    unsigned int channels = args->channels;
    char* id = (char*)args->id;
    snd_pcm_format_t format = args->format;
    snd_pcm_uframes_t frames = 0;

    /* Fill it in with default values. */
    err = snd_pcm_hw_params_any(handle, params);
    if (err < 0) {
        uac_loge("Broken configuration for %s: no configurations available: %s\n", id, snd_strerror(err));
        return err;
    }

    /* set the interleaved read/write format */
    err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) {
        uac_loge("Access type not available for %s: %s\n", id, snd_strerror(err));
        return err;
    }

    /* set the sample format */
    err = snd_pcm_hw_params_set_format(handle, params, format);
    if (err < 0) {
        uac_loge("Sample format not available for %s: %s\n", id, snd_strerror(err));
        return err;
    }

    /* set the count of channels */
    err = snd_pcm_hw_params_set_channels(handle, params, channels);
    if (err < 0) {
        uac_loge("Channels count (%i) not available for %s: %s\n", channels, id, snd_strerror(err));
        return err;
    }

    /* set the stream rate */
    val = args->rate;
    err = snd_pcm_hw_params_set_rate_near(handle, params, &val, 0);
    if (err < 0) {
        uac_loge("Rate %iHz not available for %s: %s\n", args->rate, id, snd_strerror(err));
        return err;
    }

    if (val != args->rate) {
        uac_logw("Rate doesn't match for %s: (requested %iHz, get %iHz)\n", id, args->rate, val);
    }

    err = snd_pcm_hw_params_get_rate(params, &sample_rate, 0);
    if (err < 0) {
        uac_loge("Unable to get rate for %s: %s.\n", id, snd_strerror(err));
        return err;
    }

    args->rate = sample_rate;

    /* set set the period size */
    frames = sample_rate / 1000 * (args->period_time / 1000);
    err = snd_pcm_hw_params_set_period_size(handle, params, frames, 0);
    if (err < 0) {
        uac_loge("Unable to set period size %lu for %s: %s.\n", frames, id, snd_strerror(err));
        return err;
    }

    err = snd_pcm_hw_params_get_period_size(params, &frames, NULL);
    if (err < 0) {
        uac_loge("Unable to get period size for %s: %s.\n", id, snd_strerror(err));
        return err;
    }

    args->period_size = frames;

    /* set the buffer size */
    frames = (args->buffer_time / args->period_time) * args->period_size;
    err = snd_pcm_hw_params_set_buffer_size(handle, params, frames);
    if (err < 0) {
        uac_loge("Unable to set buffer size %lu for %s: %s.\n", frames, id, snd_strerror(err));
        return err;
    }

    err = snd_pcm_hw_params_get_buffer_size(params, &frames);
    if (err < 0) {
        uac_loge("Unable to get buffer size for %s: %s.\n", id, snd_strerror(err));
        return err;
    }

    args->buffer_size = frames;

    /* write the parameters to device */
    err = snd_pcm_hw_params(handle, params);
    if (err < 0) {
        uac_loge("Unable to set hw params for %s: %s\n", id, snd_strerror(err));
        return err;
    }

    return 0;
}

// For playback
static int set_swparams(snd_pcm_t* handle, snd_pcm_sw_params_t* swparams, struct user_args* args)
{
    int err;
    int period_event = 0;                /* produce poll event after each period */
    char* id = (char*)args->id;
    snd_pcm_sframes_t buffer_size = args->buffer_size;
    snd_pcm_sframes_t period_size = args->period_size;


    /* get the current swparams */
    err = snd_pcm_sw_params_current(handle, swparams);
    if (err < 0) {
        uac_loge("Unable to determine current swparams for %s: %s\n", id, snd_strerror(err));
        return err;
    }

    /* start the transfer when the buffer is almost full: */
    /* (buffer_size / avail_min) * avail_min */
    err = snd_pcm_sw_params_set_start_threshold(handle, swparams, (buffer_size / period_size) * period_size);
    if (err < 0) {
        uac_loge("Unable to set start threshold mode for %s: %s\n", id, snd_strerror(err));
        return err;
    }

    /* allow the transfer when at least period_size samples can be processed */
    /* or disable this mechanism when period event is enabled (aka interrupt like style processing) */
    err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_event ? buffer_size : period_size);
    if (err < 0) {
        uac_loge("Unable to set avail min for %s: %s\n", id, snd_strerror(err));
        return err;
    }

    /* enable period events when requested */
    if (period_event) {
        err = snd_pcm_sw_params_set_period_event(handle, swparams, 1);
        if (err < 0) {
            uac_loge("Unable to set period event %s: %s\n", id, snd_strerror(err));
            return err;
        }
    }

    /* write the parameters to the playback device */
    err = snd_pcm_sw_params(handle, swparams);
    if (err < 0) {
        uac_loge("Unable to set sw params for %s: %s\n", id, snd_strerror(err));
        return err;
    }

    return 0;
}

static int alsa_playback_init(void)
{
    // init p_args

    return 0;
}

static int alsa_playback_deinit(void)
{
    return 0;
}

static int alsa_playback_open(void)
{
    int err;
    int buf_size;
    snd_output_t *output = NULL;
    snd_pcm_hw_params_t *hwparams = NULL;
    snd_pcm_sw_params_t *swparams = NULL;

    err = snd_output_stdio_attach(&output, stdout, 0);
    if (err < 0) {
        uac_logw("Output failed: %s\n", snd_strerror(err));
    }

    err = snd_pcm_open(&g_handle_playback, device, SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        uac_loge("snd_pcm_open playback failed: %s.\n", snd_strerror(err));
        return -1;
    }

    snd_pcm_hw_params_alloca(&hwparams);
    snd_pcm_sw_params_alloca(&swparams);

    err = set_hwparams(g_handle_playback, hwparams, &p_args);
    if (err < 0) {
        goto hw_err;
    }

    err = set_swparams(g_handle_playback, swparams, &p_args);
    if (err < 0) {
        goto hw_err;
    }

    // p_args.period_size : Number of samples per cycle
    // (p_args.channels * snd_pcm_format_physical_width(p_args.format) / 8) : The number of bytes per sample.
    buf_size = p_args.period_size * (p_args.channels * snd_pcm_format_physical_width(p_args.format) / 8);
    g_playback_data.addr = malloc(buf_size);
    if (g_playback_data.addr == NULL) {
        uac_loge("No enough memory\n");
        goto sw_err;
    }

    g_playback_data.size = buf_size;

    uac_stream_config_playback(p_args.channels,
            p_args.rate,
            p_args.period_time / 1000,
            AUDIO_STREAM_FORMAT_S16_LE,
            1);


    /* show the PCM setup parameters */
    snd_pcm_dump(g_handle_playback, output);

    return 0;

hw_err:
sw_err:
    snd_pcm_close(g_handle_playback);
    g_handle_playback = NULL;

    return -1;
}

static int alsa_playback_close(void)
{
    if (g_playback_data.addr != NULL) {
        free(g_playback_data.addr);
        g_playback_data.addr = NULL;
    }

    if (g_handle_playback != NULL) {
        snd_pcm_close(g_handle_playback);
        g_handle_playback = NULL;
    }

    return 0;
}

/* Recv audio frame from AI, and send to ALSA */
static int alsa_playback_run(int timeout_ms)
{
    int err, cptr;
    char* buffer = g_playback_data.addr;
    unsigned int buf_len  = g_playback_data.size;
    unsigned int period_size = p_args.period_size;
    snd_pcm_t* handle = g_handle_playback;

    uac_logd("Playback stream start\n");
    memset(buffer, 0, buf_len);
    err = uac_stream_recv_from_ai(buffer, buf_len, timeout_ms);
    if (err < 0) {
        uac_loge("recv stream from ai failed.\n");
        //return -1;
    }

    cptr = period_size;
    while (cptr > 0 && g_playback_run) {
        err = snd_pcm_wait(handle, timeout_ms);
        if (err == 0) {
            // If the other end keeps not taking data, the buffer will be continuously timed out after being consumed.
            uac_logd("Playback pcm wait timeout(%dms)\n", timeout_ms);
            break;
        }

        err = snd_pcm_writei(handle, buffer, cptr);
#if 1
        if (err == -EPIPE) {
            /* EPIPE means underrun */
            uac_loge("Playback underrun occurred, %s\n", snd_strerror(err));
            snd_pcm_prepare(handle);
            snd_pcm_start(handle);
            continue;
        } else if (err < 0) {
            uac_loge("Playback writei error: %s\n", snd_strerror(err));
            break;
        } else if (err != cptr) {
            uac_logi("Short write (expected %u, wrote %d)\n", cptr, err);
            break;
        }
#else
        if (err < 0) {
            err = snd_pcm_recover(handle, err, 0);
        }

        if (err < 0) {
            printf("snd_pcm_writei failed: %s\n", snd_strerror(err));
            break;
        }
#endif

        cptr -= err;
    }
    uac_logd("Playback stream ok\n");

    return 0;
}

static int alsa_playback_stop(void)
{
    return 0;
}

static int alsa_capture_init(void)
{
    // init c_args
    return 0;
}

static int alsa_capture_deinit(void)
{
    return 0;
}

static int alsa_capture_open(void)
{
    int err;
    int buf_size;
    snd_output_t *output = NULL;
    snd_pcm_hw_params_t *hwparams = NULL;

    err = snd_output_stdio_attach(&output, stdout, 0);
    if (err < 0) {
        uac_logw("Output failed: %s\n", snd_strerror(err));
    }

    err = snd_pcm_open(&g_handle_capture, device, SND_PCM_STREAM_CAPTURE, 0);
    if (err < 0) {
        uac_loge("snd_pcm_open capture failed: %s.\n", snd_strerror(err));
        return -1;
    }

    snd_pcm_hw_params_alloca(&hwparams);

    err = set_hwparams(g_handle_capture, hwparams, &c_args);
    if (err < 0) {
        goto hw_err;
    }

    buf_size = (c_args.period_size * c_args.channels * snd_pcm_format_physical_width(c_args.format)) / 8;
    g_capture_data.addr = malloc(buf_size);
    if (g_capture_data.addr == NULL) {
        uac_loge("No enough memory\n");
        goto hw_err;
    }

    g_capture_data.size = buf_size;

    uac_stream_config_capture(c_args.channels,
            c_args.rate,
            c_args.period_time / 1000,
            AUDIO_STREAM_FORMAT_S16_LE,
            1);

    /* show the PCM setup parameters */
    snd_pcm_dump(g_handle_capture, output);

    err = snd_pcm_start(g_handle_capture);
    if (err < 0) {
        uac_loge("snd_pcm_start capture failed: %s.\n", snd_strerror(err));
        goto sw_err;
    }

    return 0;

sw_err:
    if (g_capture_data.addr != NULL) {
        free(g_capture_data.addr);
        g_capture_data.addr = NULL;
        g_capture_data.size = 0;
    }

hw_err:
    snd_pcm_close(g_handle_capture);
    g_handle_capture = NULL;

    return -1;
}

static int alsa_capture_close(void)
{
    if (g_capture_data.addr != NULL) {
        free(g_capture_data.addr);
        g_capture_data.addr = NULL;
    }

    if (g_handle_capture != NULL) {
        snd_pcm_close(g_handle_capture);
        g_handle_capture = NULL;
    }

    return 0;
}

/* Recv sample frame from ALSA, and send to AO */
static int alsa_capture_run(int timeout_ms)
{
    int err = 0;
    char* buffer = g_capture_data.addr;
    int size = g_capture_data.size;
    unsigned int period_size = c_args.period_size;
    snd_pcm_t* handle = g_handle_capture;

    uac_logd("Capture stream send start.\n");
    while (err != period_size && g_capture_run) {
        err = snd_pcm_wait(handle, timeout_ms);
        if (err == 0) {
            uac_logd("Capture pcm wait timeout(%dms).\n", timeout_ms);
            continue;
        }

        err = snd_pcm_readi(handle, buffer, period_size);
        if (err == -EPIPE) {
            /* EPIPE means overrun */
            uac_logw("Capture overrun occurred: %s.\n", snd_strerror(err));
            snd_pcm_prepare(handle);
            snd_pcm_start(handle);
            continue;
        } else if (err < 0) {
            uac_logw("Capture read failed: %s.\n", snd_strerror(err));
            continue;
        } else if (err != (int)period_size) {
            uac_logw("Short read (expected %u, rrote %d).\n", period_size, err);
            continue;
        }

        if (uac_stream_send_to_ao(buffer, size, timeout_ms) < 0) {
            uac_loge("Send to ao failed.\n");
        } else {
            uac_logd("Send to ao ok.\n");
        }
    }

    uac_logd("Capture stream send ok.\n");

    return 0;
}

static int alsa_capture_stop(void)
{
    return 0;
}
/**************************************************************************************/

static pthread_t pid_playback = -1;
static pthread_t pid_capture = -1;

static void* playback_process(void* args)
{
    uac_logt("UAC playbcak process start.\n");
    uac_stream_startup_playback();
    // Waiting for AI to cache data, avoiding the issue of also underrun.
    usleep(20 * 1000);
    while (g_playback_run) {
        //alsa_playback_run(p_args.period_time / 1000);
        alsa_playback_run(PCM_WAIT_TIME_MS);
    }

    alsa_playback_stop();
    uac_stream_shutdown_playback();
    uac_logt("UAC playback process exit.\n");

    return NULL;
}

static void* capture_process(void* args)
{
    uac_logt("UAC capture process start.\n");
    uac_stream_startup_capture();
    while (g_capture_run) {
        //alsa_capture_run(c_args.period_time / 1000);
        alsa_capture_run(PCM_WAIT_TIME_MS);
    }

    alsa_capture_stop();
    uac_stream_shutdown_capture();
    uac_logt("UAC capture process exit.\n");

    return NULL;
}

/****************************** UAC Interface *****************************************/
static int __uac_dev_init(void)
{
    int err;

    err = alsa_playback_init();
    if (err < 0) {
        uac_loge("Playback init failed.\n");
        return -1;
    }

    err = alsa_capture_init();
    if (err < 0) {
        uac_loge("Capture init failed.\n");
        return -1;
    }

    return 0;
}

static int __uac_dev_deinit(void)
{
    alsa_playback_deinit();
    alsa_capture_deinit();

    return 0;
}

static int __uac_dev_open(void)
{
    alsa_playback_open();
    alsa_capture_open();

    return 0;
}

static int __uac_dev_close(void)
{
    alsa_playback_close();
    alsa_capture_close();

    return 0;
}

static int __uac_dev_run(void)
{
    int err;

    uac_logt("UAC run entry.\n");
    g_playback_run = 1;
    err = pthread_create(&pid_playback, NULL, &playback_process, NULL);
    if (err != 0) {
        uac_loge("Create playback_process failed.\n");
    }

    g_capture_run = 1;
    err = pthread_create(&pid_capture, NULL, &capture_process, NULL);
    if (err != 0) {
        uac_loge("Creat capture_process failed.\n");
    }
    uac_logt("UAC run ok.\n");

    return 0;
}

static int __uac_dev_stop(void)
{
    uac_logt("UAC stop entry.\n");
    g_playback_run = 0;
    g_capture_run = 0;
    pthread_join(pid_playback, NULL);
    pthread_join(pid_capture, NULL);
    uac_logt("UAC  stop ok.\n");

    return 0;
}

static uac_dev_t uac_dev = {
    .init = &__uac_dev_init,
    .deinit = &__uac_dev_deinit,
    .open = &__uac_dev_open,
    .close = &__uac_dev_close,
    .run = &__uac_dev_run,
    .stop = &__uac_dev_stop,
};

uac_dev_t* get_uac_dev(void)
{
    return &uac_dev;
}

#define check_func(func_ptr, name) do { \
    if (func_ptr == NULL) { \
        uac_loge("Invalid %s function.\n", name); \
        return -1; \
    } \
} while(0)

static int uac_check_strem_func(audio_stream_dev_t* func, int is_playback)
{
    check_func(func->config, "config");
    check_func(func->startup, "startup");
    check_func(func->shutdown, "shutdown");
    if (is_playback) {
        check_func(func->recv_from_stream, "recv stream");
    } else {
        check_func(func->send_to_stream, "send stream");
    }

    return 0;
}

int uac_stream_register(audio_stream_operate_t* hand)
{
    if (hand == NULL) {
        uac_loge("Invalid stream operate function.\n");
        return -1;
    }

    if (hand->playback_stream == NULL) {
        uac_loge("Invalid playback_stream.\n");
        return -1;
    }

    if (hand->capture_stream == NULL) {
        uac_loge("Invalid capture_stream.\n");
        return -1;
    }

    if (uac_check_strem_func(hand->playback_stream, 1) < 0) {
        uac_loge("Invalid playback operate function.\n");
        return -1;
    }

    if (uac_check_strem_func(hand->capture_stream, 0) < 0) {
        uac_loge("Invalid capture operate function.\n");
        return -1;
    }

    uac_stream = hand;

    return 0;
}

int uac_stream_unregister(void)
{
    uac_stream = NULL;

    return 0;
}

/**************************************************************************************/

/****************************** main **************************************************/
#if 0
int main(int argc, char* argv[])
{
    uac_dev_t* uac = get_uac_dev();

    if (uac) {
        uac_dev->init();
        uac_dev->open();
        uac_dev->run();
        uac_dev->stop();
        uac_dev->close();
        uac_dev->deinit();
    }

    return 0;
}
#endif
/**************************************************************************************/
