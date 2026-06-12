/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdio.h>

#include "uac_hal.h"
#include "audio_stream.h"
#include "sample_audio.h"

#include "uac_log.h"

struct audio_format_map {
    audio_stream_format_t hal_format;
    audio_bit_depth       bit_format;
};

struct audio_format_map format_group[] = {
    {AUDIO_STREAM_FORMAT_S8, AUDIO_BIT_DEPTH_8},
    {AUDIO_STREAM_FORMAT_S16_LE, AUDIO_BIT_DEPTH_16},
    {AUDIO_STREAM_FORMAT_S24_LE, AUDIO_BIT_DEPTH_24},
    {AUDIO_STREAM_FORMAT_S32_LE, AUDIO_BIT_DEPTH_32},
};

static int format_hal_to_bit(audio_stream_format_t format)
{
    int i;
    int nums = sizeof(format_group) / sizeof(struct audio_format_map);

    for (i = 0; i < nums; i++) {
        if (format_group[i].hal_format == format) {
            return format_group[i].bit_format;
        }
    }

    return -1;
}

static int playback_stream_config(unsigned int channels,
        unsigned int rate,
        unsigned int period_time,
        audio_stream_format_t format,
        bool is_interleaved)
{
    audio_bit_depth bit_format = format_hal_to_bit(format);

    uac_logd("channels: %u\n", channels);
    uac_logd("rate: %u\n", rate);
    uac_logd("period_time: %ums\n", period_time);
    uac_logd("format: %d\n", bit_format);
    uac_logd("interleaved: %d\n", is_interleaved);

    return sample_audio_ai_config(channels, rate, period_time, bit_format, is_interleaved);
}

static int playback_stream_startup(void)
{
    return sample_audio_ai_startup();
}

static int playback_stream_shutdown(void)
{
    return sample_audio_ai_shutdown();
}

/* Get stream from AI */
static int playback_recv_from_stream(char* buf, int size, int timeout_ms)
{
    return sample_audio_get_frame_from_ai(buf, size, timeout_ms);
}

static int capture_stream_config(unsigned int channels,
        unsigned int rate,
        unsigned int period_time,
        audio_stream_format_t format,
        bool is_interleaved)
{
    audio_bit_depth bit_format = format_hal_to_bit(format);

    uac_logd("channels: %u\n", channels);
    uac_logd("rate: %u\n", rate);
    uac_logd("period_time: %ums\n", period_time);
    uac_logd("format: %d\n", bit_format);
    uac_logd("interleaved: %d\n", is_interleaved);

    return sample_audio_ao_config(channels, rate, period_time, bit_format, is_interleaved);
}

static int capture_stream_startup(void)
{
    return sample_audio_ao_startup();
}

static int capture_stream_shutdown(void)
{
    return sample_audio_ao_shutdown();
}

/* Send stream to AO */
static int capture_send_to_stream(char* buf, int size, int timeout_ms)
{
    return sample_audio_send_frame_to_ao(buf, size, timeout_ms);
}

static audio_stream_dev_t playback_stream_dev = {
    .config   = playback_stream_config,
    .startup  = playback_stream_startup,
    .shutdown = playback_stream_shutdown,
    .recv_from_stream = playback_recv_from_stream,
};

static audio_stream_dev_t capture_stream_dev = {
    .config   = capture_stream_config,
    .startup  = capture_stream_startup,
    .shutdown = capture_stream_shutdown,
    .send_to_stream = capture_send_to_stream,
};

static audio_stream_operate_t strem_operate = {
    .playback_stream = &playback_stream_dev,
    .capture_stream  = &capture_stream_dev,
};

audio_stream_operate_t* get_audio_stream(void)
{
    return &strem_operate;
}
