/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __SAMPLE_AUDIO_H__
#define __SAMPLE_AUDIO_H__

#include <stdbool.h>
#include "xmedia_audio_common.h"

int sample_audio_ai_config(unsigned int channels,
        unsigned int rate,
        unsigned int period_time,
        audio_bit_depth format,
        bool is_interleaved);
int sample_audio_ai_startup(void);
int sample_audio_ai_shutdown(void);
int sample_audio_get_frame_from_ai(char* buf, int size, int time_out);

int sample_audio_ao_config(unsigned int channels,
        unsigned int rate,
        unsigned int period_time,
        audio_bit_depth format,
        bool is_interleaved);
int sample_audio_ao_startup(void);
int sample_audio_ao_shutdown(void);
int sample_audio_send_frame_to_ao(char* buf, int size, int time_out);

#endif
