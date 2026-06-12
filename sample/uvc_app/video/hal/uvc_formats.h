/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
/*
 * uvc formats
 */

#ifndef __UVC_FORMATS_H__
#define __UVC_FORMATS_H__

#include <linux/videodev2.h>

#if 1
struct uvc_frame_info {
    unsigned int width;
    unsigned int height;
    unsigned int intervals[8];
};

struct uvc_format_info {
    unsigned int                 fcc;
    unsigned int                 frm_nums;
    const struct uvc_frame_info* frames;
};

void uvc_formats_init(void);

void uvc_formats_deinit(void);

int find_frame(int* fmt_index, int* frm_index, int* fcc, struct uvc_frame_info* frame);

#else
struct uvc_frame_info {
    unsigned int width;
    unsigned int height;
    unsigned int intervals[8];
};

struct uvc_format_info {
    unsigned int                 fcc;
    const struct uvc_frame_info* frames;
};

static const struct uvc_frame_info uvc_frames_yuv[] = {
    {  640,  360, {333333,       0 }, },
    { 1280,  720, {333333,       0 }, },
    { 1920, 1080, {333333,       0 }, },
    { 3840, 2160, {333333,       0 }, },
    {    0,    0, {     0,         }, },
};

static const struct uvc_frame_info uvc_frames_mjpeg[] = {
    {  640,  360, {333333,       0 }, },
    { 1280,  720, {333333,       0 }, },
    { 1920, 1080, {333333,       0 }, },
    { 3840, 2160, {333333,       0 }, },
    {    0,    0, {     0,         }, },
};

static const struct uvc_frame_info uvc_frames_h264[] = {
    {  640,  360, {333333,       0 }, },
    { 1280,  720, {333333,       0 }, },
    { 1920, 1080, {333333,       0 }, },
    { 3840, 2160, {333333,       0 }, },
    {    0,    0, {     0,         }, },
};

static const struct uvc_format_info uvc_formats[] = {
    {V4L2_PIX_FMT_YUYV, uvc_frames_yuv},
    {V4L2_PIX_FMT_MJPEG, uvc_frames_mjpeg},
    {V4L2_PIX_FMT_H264,  uvc_frames_h264},
};
#endif

#endif
