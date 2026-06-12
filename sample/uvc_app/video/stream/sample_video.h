/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __SAMPLE_VIDEO_H__
#define __SAMPLE_VIDEO_H__

#include "sample_comm.h"

struct encoder_property {
    unsigned int format;
    unsigned int width;
    unsigned int height;
    unsigned int fps;
};

void video_stream_register(void);
void video_stream_unregister(void);

void get_user_format(xmedia_payload_type *format, xmedia_video_size *wh, unsigned int *framerate);

#endif
