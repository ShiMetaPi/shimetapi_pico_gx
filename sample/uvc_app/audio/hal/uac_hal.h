/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __UAC_HAL_H__
#define __UAC_HAL_H__

#include <stdbool.h>

typedef enum audio_stream_format {
    AUDIO_STREAM_FORMAT_UNKNOWN = -1,
    /** Signed 8 bit */
    AUDIO_STREAM_FORMAT_S8 = 0,
    /** Unsigned 8 bit */
    AUDIO_STREAM_FORMAT_U8,
    /** Signed 16 bit Little Endian */
    AUDIO_STREAM_FORMAT_S16_LE,
    /** Signed 16 bit Big Endian */
    AUDIO_STREAM_FORMAT_S16_BE,
    /** Unsigned 16 bit Little Endian */
    AUDIO_STREAM_FORMAT_U16_LE,
    /** Unsigned 16 bit Big Endian */
    AUDIO_STREAM_FORMAT_U16_BE,
    /** Signed 24 bit Little Endian using low three bytes in 32-bit word */
    AUDIO_STREAM_FORMAT_S24_LE,
    /** Signed 24 bit Big Endian using low three bytes in 32-bit word */
    AUDIO_STREAM_FORMAT_S24_BE,
    /** Unsigned 24 bit Little Endian using low three bytes in 32-bit word */
    AUDIO_STREAM_FORMAT_U24_LE,
    /** Unsigned 24 bit Big Endian using low three bytes in 32-bit word */
    AUDIO_STREAM_FORMAT_U24_BE,
    /** Signed 32 bit Little Endian */
    AUDIO_STREAM_FORMAT_S32_LE,
    /** Signed 32 bit Big Endian */
    AUDIO_STREAM_FORMAT_S32_BE,
    /** Unsigned 32 bit Little Endian */
    AUDIO_STREAM_FORMAT_U32_LE,
    /** Unsigned 32 bit Big Endian */
    AUDIO_STREAM_FORMAT_U32_BE,
} audio_stream_format_t;

typedef struct {
    int (*config)(unsigned int channels, unsigned int rate, unsigned int period_time,
            audio_stream_format_t format, bool is_interleaved);
    int (*startup)(void);
    int (*shutdown)(void);
    int (*recv_from_stream)(char* buf, int size, int timeout_ms);
    int (*send_to_stream)(char* buf, int size, int timeout_ms);
} audio_stream_dev_t;

typedef struct {
    /* Get data from stream dev(AI) and send to uac dev */
    audio_stream_dev_t* playback_stream;
    /* Get data from uac dev and send to stream dev(AO) */
    audio_stream_dev_t* capture_stream;
} audio_stream_operate_t;

typedef struct uac_dev {
    int (*init)(void);
    int (*deinit)(void);
    int (*open)(void);
    int (*close)(void);
    int (*run)(void);
    int (*stop)(void);
} uac_dev_t;

uac_dev_t* get_uac_dev(void);
int uac_stream_register(audio_stream_operate_t* hand);
int uac_stream_unregister(void);

#endif
