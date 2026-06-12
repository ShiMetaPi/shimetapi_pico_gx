/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __UVC_HAL_H__
#define __UVC_HAL_H__

#include <stdint.h>
#include <linux/usb/g_uvc.h>
#include <linux/usb/video.h>

typedef struct uvc_probe_t {
    unsigned char set;
    unsigned char get;
    unsigned char max;
    unsigned char min;
} uvc_probe_t;

struct uvc_device {
    int fd;
    int streaming;

    unsigned int req;
    unsigned int control;
    unsigned int unit_id;
    unsigned int interface_id;

    unsigned int fcc;
    unsigned int width;
    unsigned int height;
    unsigned int fps;
    unsigned int bulk;
    unsigned int bulk_size;
    unsigned int nbufs;
    unsigned int imgsize;
    unsigned int max_payload_size;
    unsigned char color;

    /* USB speed specific */
    int mult;
    int burst;
    int maxpkt;
    enum usb_device_speed speed;

    uvc_probe_t  probe_status;
    struct uvc_streaming_control probe;
    struct uvc_streaming_control commit;

    struct uvc_request_data request_error_code;
};

int open_uvc_device();
int close_uvc_device();
int run_uvc_device();
int run_uvc_data();

#endif //__UVC_HAL_H__
