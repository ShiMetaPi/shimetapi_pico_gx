/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/select.h>

#include <linux/usb/ch9.h>
#include <linux/videodev2.h>
#include <linux/usb/g_uvc.h>

#include "uvc_hal.h"
#include "video_stream.h"
#include "sample_video.h"
#include "config_svc.h"
#include "frame_cache.h"
#include "events.h"
#include "uvc_log.h"
#include "uvc_formats.h"
#include "uvc_adapter.h"

#include "debug.h"

#define v4l2_fourcc(a, b, c, d)\
    ((__u32)(a) | ((__u32)(b) << 8) | ((__u32)(c) << 16) | ((__u32)(d) << 24))

#define V4L2_PIX_FMT_H265 v4l2_fourcc('H', '2', '6', '5')   /* H265 with start codes */

#define clamp(val, min, max) ( \
        { \
        typeof(val)__val = (val);              \
        typeof(min)__min = (min);              \
        typeof(max)__max = (max);              \
        (void) (&__val == &__min);             \
        (void) (&__val == &__max);             \
        __val = __val < __min ? __min : __val; \
        __val > __max ? __max : __val;         \
        })

#define ARRAY_SIZE(a) ((sizeof(a) / sizeof(a[0])))

#define CLEAR(x) memset (&(x), 0, sizeof (x))

typedef enum {
    IO_METHOD_MMAP = 0x1,
    IO_METHOD_USERPTR = 0x2,
} io_method;

#define WAITED_NODE_SIZE (2)
static frame_node_t* __waited_node[WAITED_NODE_SIZE];

extern unsigned int g_bulk;

static void clear_waited_node()
{
    int i;
    uvc_cache_t *uvc_cache = uvc_cache_get();

    for (i = 0; i < WAITED_NODE_SIZE; i++) {
        if ((__waited_node[i] != 0) && uvc_cache) {
            put_node_to_queue(uvc_cache->free_queue, __waited_node[i]);
            __waited_node[i] = 0;
        }
    }
}

static struct uvc_device* uvc_open(const char* devname)
{
    int ret, fd;
    struct uvc_device* dev;
    struct v4l2_capability cap;

    func_entry();

    fd = open(devname, O_RDWR | O_NONBLOCK);

    if (fd == -1) {
        uvc_loge("v4l2 open failed(%s): %s (%d)\n",devname, strerror(errno), errno);
        return NULL;
    }

    ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
    if (ret < 0) {
        uvc_loge("unable to query device: %s (%d)\n", strerror(errno), errno);
        close(fd);
        return NULL;
    }

    uvc_logd("open succeeded (%s:caps=0x%04x)\n", devname, cap.capabilities);

    /*
       V4L2_CAP_VIDEO_CAPTURE 0x00000001 support video Capture interface.
       V4L2_CAP_VIDEO_OUTPUT 0x00000002 support video output interface.
       V4L2_CAP_VIDEO_OVERLAY 0x00000004 support video cover interface.
       V4L2_CAP_VBI_CAPTURE 0x00000010 Original VBI Capture interface.
       V4L2_CAP_VBI_OUTPUT 0x00000020 Original VBI Output interface.
       V4L2_CAP_SLICED_VBI_CAPTURE 0x00000040 Sliced VBI Capture interface.
       V4L2_CAP_SLICED_VBI_OUTPUT 0x00000080 Sliced VBI Output interface.
       V4L2_CAP_RDS_CAPTURE 0x00000100 undefined
     */
    if (!(cap.capabilities & V4L2_CAP_VIDEO_OUTPUT)) {
        close(fd);
        return NULL;
    }

    uvc_logd("device is %s on bus %s\n", cap.card, cap.bus_info);

    dev = (struct uvc_device*)malloc(sizeof * dev);

    if (dev == NULL) {
        close(fd);
        return NULL;
    }

    memset(dev, 0, sizeof * dev);
    dev->fd = fd;

    CLEAR(__waited_node);

    func_success();

    return dev;
}

static void uvc_close(struct uvc_device* dev)
{
    close(dev->fd);
    free(dev);
}

/* ---------------------------------------------------------------------------
 * Video streaming
 */
static void fill_buffer(struct uvc_device* dev, struct v4l2_buffer* buf)
{
    int retry_count = 5;
    frame_node_t *node = NULL;
    frame_queue_t *q   = NULL;
    frame_queue_t *fq  = NULL;
    uvc_cache_t *uvc_cache = uvc_cache_get();

retry:
    node_wait_timeout_ms(1000);

    if (uvc_cache) {
        q  = uvc_cache->ok_queue;
        fq = uvc_cache->free_queue;
        get_node_from_queue(q, &node);
    }

    if ((__waited_node[buf->index] != 0) && uvc_cache) {
        put_node_to_queue(fq, __waited_node[buf->index]);
        __waited_node[buf->index] = 0;
    }

    if (node != NULL) {
        buf->bytesused = node->used;
        buf->m.userptr = (unsigned long)node->mem;
        buf->length = node->length;
        __waited_node[buf->index] = node;
    } else if (retry_count != 0){
        retry_count--;
        //uvc_logw("Not found stream buffer, retry.\n");
        goto retry;
    } else {
        uvc_logw("Get stream fail.\n");
        return;
    }
}

static void uvc_video_fill_buffer_userptr(struct uvc_device* dev, struct v4l2_buffer* buf)
{
    buf->bytesused = 0;

    switch (dev->fcc) {
        case V4L2_PIX_FMT_MJPEG:
        case V4L2_PIX_FMT_H264:
        case V4L2_PIX_FMT_H265:
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_NV21:
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_YVU420:
            fill_buffer(dev, buf);
            break;
        default:
            uvc_logw("what's up....\n");
            break;
    }
}

static int uvc_video_process_userptr(struct uvc_device* dev)
{
    int ret;
    struct v4l2_buffer buf;

    //uvc_logd("uvc_video_process_userptr entry.\n");

    memset(&buf, 0, sizeof buf);
    buf.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    buf.memory = V4L2_MEMORY_USERPTR;

    ret = ioctl(dev->fd, VIDIOC_DQBUF, &buf);
    if (ret < 0) {
        //uvc_loge("Unable to dequeue buffer: %s (%d).\n", strerror(errno), errno);
        return ret;
    }

    uvc_video_fill_buffer_userptr(dev, &buf);

    debug_dump_stream((char*)buf.m.userptr, buf.bytesused, "stream_qbuf.bin");

    ret = ioctl(dev->fd, VIDIOC_QBUF, &buf);
    if (ret < 0) {
        uvc_loge("Unable to queue buffer: %s (%d).\n", strerror(errno), errno);
        return ret;
    }

    //uvc_logd("uvc_video_process_userptr ok.\n");

    return 0;
}

static int uvc_video_reqbufs_userptr(struct uvc_device* dev, int nbufs)
{
    int ret;
    struct v4l2_requestbuffers rb;

    dev->nbufs = 0;

    memset(&rb, 0, sizeof rb);
    rb.count = nbufs;
    rb.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    rb.memory = V4L2_MEMORY_USERPTR;

    ret = ioctl(dev->fd, VIDIOC_REQBUFS, &rb);
    if (ret < 0) {
        uvc_loge("Unable to allocate buffers: %s (%d).\n", strerror(errno), errno);
        return ret;
    }

    dev->nbufs = rb.count;

    uvc_logi("reqbufs[%d] ok.\n", rb.count);

    return 0;
}

static int uvc_video_stream_userptr(struct uvc_device* dev, int enable)
{
    int ret = 0;
    int type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    unsigned int i;
    struct v4l2_buffer buf;

    for (i = 0; i < (dev->nbufs); ++i) {
        memset(&buf, 0, sizeof buf);

        buf.index = i;
        buf.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        buf.memory = V4L2_MEMORY_USERPTR;

        uvc_video_fill_buffer_userptr(dev, &buf);

        if ((ret = ioctl(dev->fd, VIDIOC_QBUF, &buf)) < 0) {
            uvc_loge("Unable to queue buffer(%d): %s (%d).\n", i, strerror(errno), errno);
            break;
        }
    }

    ioctl(dev->fd, VIDIOC_STREAMON, &type);
    dev->streaming = 1;

    return ret;
}

static int uvc_video_set_format(struct uvc_device* dev)
{
    int ret;
    struct v4l2_format fmt;

    memset(&fmt, 0, sizeof fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    fmt.fmt.pix.width  = dev->width;
    fmt.fmt.pix.height = dev->height;
    fmt.fmt.pix.pixelformat = dev->fcc;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if ((dev->fcc == V4L2_PIX_FMT_MJPEG) || (dev->fcc == V4L2_PIX_FMT_H264) || (dev->fcc == V4L2_PIX_FMT_H265)) {
        if (dev->bulk) {
            fmt.fmt.pix.sizeimage = dev->bulk_size;
        } else {
            fmt.fmt.pix.sizeimage = dev->imgsize;
        }
    }

    ret = ioctl(dev->fd, VIDIOC_S_FMT, &fmt);
    if (ret < 0) {
        uvc_loge("Unable to set format: %s (%d).\n", strerror(errno), errno);
    }

    return ret;
}

static int uvc_video_init(struct uvc_device* dev __attribute__ ((__unused__)))
{
    return 0;
}

/* ---------------------------------------------------------------------------
 * Request processing
 */

static void uvc_streamoff(struct uvc_device *dev)
{
    int type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

    dev->streaming = 0;
    video_stream_shutdown();
    ioctl(dev->fd, VIDIOC_STREAMOFF, &type);
}

static void disable_uvc_video(struct uvc_device* dev)
{
    uvc_streamoff(dev);
    clear_waited_node();
}

static void enable_uvc_video(struct uvc_device* dev)
{
    struct encoder_property p;

    disable_uvc_video(dev);
    clear_ok_queue();
    clear_waited_node();

    uvc_video_reqbufs_userptr(dev, WAITED_NODE_SIZE);

    p.format = dev->fcc;
    p.width  = dev->width;
    p.height = dev->height;
    p.fps    = dev->fps;

    video_stream_set_enc_property(&p);

    video_stream_shutdown();
    video_stream_startup();

    uvc_video_stream_userptr(dev, 1);
}

static void uvc_fill_streaming_control(struct uvc_device* dev,
        struct uvc_streaming_control* ctrl,
        int iframe, int iformat)
{
    struct uvc_frame_info frame = {0};
    int fmt_index, frm_index, fcc;

    fmt_index = iformat;
    frm_index = iframe;
    fcc = 0;

    if (find_frame(&fmt_index, &frm_index, &fcc, &frame) < 0) {
        return;
    }

    memset(ctrl, 0, sizeof * ctrl);

    ctrl->bmHint = 1;
    ctrl->bFormatIndex = fmt_index;
    ctrl->bFrameIndex = frm_index;
    ctrl->dwFrameInterval = frame.intervals[0];

    switch (fcc) {
        case V4L2_PIX_FMT_YUYV:
            ctrl->dwMaxVideoFrameSize = frame.width * frame.height * 2;
            break;

        case V4L2_PIX_FMT_NV21:
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_YVU420:
            ctrl->dwMaxVideoFrameSize = frame.width * frame.height * 3 / 2;
            break;

        case V4L2_PIX_FMT_MJPEG:
        case V4L2_PIX_FMT_H264:
        case V4L2_PIX_FMT_H265:
            if (dev->bulk) {
                ctrl->dwMaxVideoFrameSize = dev->bulk_size;
            } else {
                ctrl->dwMaxVideoFrameSize = dev->imgsize;
            }
            break;

        default:
            uvc_loge("Unkonw pixel format.\n");
            return;
    }

    if (dev->bulk) {
        ctrl->dwMaxPayloadTransferSize = dev->bulk_size;   /* TODO this should be filled by the driver. */
    } else {
        /* Read /sys/kernel/config/usb_gadget/camera/functions/uvc.usb0/streaming_maxpacket */
        ctrl->dwMaxPayloadTransferSize = dev->max_payload_size;
    }

    uvc_logi("Set MaxPayloadTransferSize = %d.\n", ctrl->dwMaxPayloadTransferSize);

    ctrl->bmFramingInfo = 3;
    ctrl->bPreferedVersion = 1;
    ctrl->bMaxVersion = 1;
}

static void uvc_events_process_standard_setup(struct uvc_device* dev, struct usb_ctrlrequest* ctrl,
        struct uvc_request_data* resp)
{
    uvc_logi("standard request\n");
    (void)dev;
    (void)ctrl;
    (void)resp;
}

static void vc_interface_request_error_code_cs(struct uvc_device *dev,
        uint8_t req,
        struct uvc_request_data *resp)
{
    switch (req) {
        case UVC_GET_CUR:
            resp->length = dev->request_error_code.length;
            resp->data[0] = dev->request_error_code.data[0];
            uvc_logd("dev->request_error_code.data[0] = %d\n",dev->request_error_code.data[0]);
            break;

        case UVC_GET_INFO:
            resp->length  = 1;
            resp->data[0] = 1; /* GET */
            break;

        default:
            uvc_loge("Invalid request.\n");
            break;
    }
}

static void uvc_event_vc_interface_setup(struct uvc_device *dev,
        uint8_t req,
        uint8_t cs,
        struct uvc_request_data *resp)
{
    switch (cs) {
        case UVC_VC_REQUEST_ERROR_CODE_CONTROL:
            vc_interface_request_error_code_cs(dev, req, resp);
            break;
        case UVC_VC_VIDEO_POWER_MODE_CONTROL:
        default:
            dev->request_error_code.length = 1;
            dev->request_error_code.data[0] = UVC_REC_INVALID_CONTROL;
            break;
    }
}

static void uvc_events_process_vc_setup(struct uvc_device*       dev,
        uint8_t                  req,
        uint8_t                  unit_id,
        uint8_t                  cs,
        struct uvc_request_data* resp)
{
    switch (unit_id) {
        case UVC_VC_INTERFACE_ID:
            uvc_event_vc_interface_setup(dev, req, cs,  resp);
            break;
        case UVC_CAMERA_TERMINAL_ID:
            stream_event_it_setup(dev, req, unit_id, cs, resp);
            break;
        case UVC_PROCESSING_UNIT_ID:
            stream_event_pu_setup(dev, req, unit_id, cs, resp);
            break;
        case UVC_EXTENSION_UNIT_H264_ID:
            stream_event_eu_h264_setup(req, unit_id, cs, resp);
            break;
        case UVC_EXTENSION_UNIT_VENDOR1_ID:
            stream_event_eu_vendor1_setup(dev, req, unit_id, cs, resp);
            break;
        default:
            dev->request_error_code.length = 1;
            dev->request_error_code.data[0] = UVC_REC_INVALID_UNIT;
            break;
    }
}

static void uvc_events_process_vs_setup(struct uvc_device* dev, uint8_t req, uint8_t cs,
        struct uvc_request_data* resp)
{
    struct uvc_streaming_control* ctrl;

    if ((cs != UVC_VS_PROBE_CONTROL) && (cs != UVC_VS_COMMIT_CONTROL)) {
        return;
    }

    ctrl = (struct uvc_streaming_control*)&resp->data;
    resp->length = sizeof(struct uvc_streaming_control);

    switch (req) {
        case UVC_SET_CUR:
            dev->control = cs;
            resp->length = 34; /* uvc1.1 sizeof(struct uvc_streaming_control) */
            break;

        case UVC_GET_CUR:
            if (cs == UVC_VS_PROBE_CONTROL) {
                memcpy(ctrl, &dev->probe, resp->length);
            } else {
                memcpy(ctrl, &dev->commit, resp->length);
            }
            break;

        case UVC_GET_MIN:
            uvc_fill_streaming_control(dev, ctrl, 0, 0);
            break;

        case UVC_GET_MAX:
            uvc_fill_streaming_control(dev, ctrl, -1, -1);
            break;

        case UVC_GET_DEF:
            uvc_fill_streaming_control(dev, ctrl, 0, 0);
            break;

        case UVC_GET_RES:
            memset(ctrl, 0, sizeof(struct uvc_streaming_control));
            break;

        case UVC_GET_LEN:
            resp->data[0] = 34; /* uvc1.1 */
            resp->length = 1;
            break;

        case UVC_GET_INFO:
            resp->data[0] = 0x03;
            resp->length = 1;
            break;

        default:
            break;
    }
}

static void set_probe_status(struct uvc_device* dev, int cs, int req)
{
    if (cs == UVC_VS_INTERFACE_ID) {
        switch (req) {
            case UVC_SET_CUR:
                dev->probe_status.set = 1;
                break;
            case UVC_GET_CUR:
                dev->probe_status.get = 1;
                break;
            case UVC_GET_MIN:
                dev->probe_status.min = 1;
                break;
            case UVC_GET_MAX:
                dev->probe_status.max = 1;
                break;
            case UVC_GET_RES:
            case UVC_GET_LEN:
            case UVC_GET_INFO:
            default:
                break;
        }
    }
}

#if 0
static int check_probe_status(struct uvc_device* dev)
{
    unsigned char status = 1;

    status = (status & dev->probe_status.set);
    status = (status & dev->probe_status.get);
    status = (status & dev->probe_status.min);
    status = (status & dev->probe_status.max);

    if (status) {
        return 1;
    }

    uvc_loge("the probe status is not correct...\n");

    return 0;
}
#endif

static void uvc_events_process_class_setup(struct uvc_device* dev, struct usb_ctrlrequest* ctrl,
        struct uvc_request_data* resp)
{
    unsigned char cs_code, req_id;
    unsigned char probe_status = 1;
    unsigned char type = ctrl->bRequestType & USB_RECIP_MASK;

    if (probe_status) {
        switch (type) {
            case USB_RECIP_INTERFACE:
                uvc_requests_infos(ctrl);
                cs_code = MASK_CS_CODE(ctrl);
                req_id  = MASK_REQ_CODE(ctrl);
                set_probe_status(dev, cs_code, req_id);
                break;
            case USB_RECIP_DEVICE:
                uvc_logd("request type :DEVICE\n");
                break;
            case USB_RECIP_ENDPOINT:
                uvc_logd("request type :ENDPOINT\n");
                break;
            case USB_RECIP_OTHER:
                uvc_logd("request type :OTHER\n");
                break;
            default:
                uvc_logw("unkonw request type.\n");
                break;
        }
    }

    if (type != USB_RECIP_INTERFACE) {
        uvc_loge("unsupported request type.\n");
        return;
    }

    //save unit id, interface id and control selector
    dev->req     = MASK_REQ_CODE(ctrl);
    dev->control = MASK_CS_CODE(ctrl);
    dev->unit_id = MASK_ENTITY_ID(ctrl);
    dev->interface_id = MASK_INTF_ID(ctrl);

    if (dev->unit_id != 0) {
        uvc_events_process_vc_setup(dev, dev->req, dev->unit_id, dev->control, resp);
    } else if (dev->interface_id == UVC_VC_INTERFACE_ID) {
        uvc_events_process_vc_setup(dev, dev->req, dev->interface_id, dev->control, resp);
    } else if (dev->interface_id == UVC_VS_INTERFACE_ID) {
        uvc_events_process_vs_setup(dev, dev->req, dev->control, resp);
    } else {
        uvc_logw("unknow unit(%u) and interface(%u).\n", dev->unit_id, dev->interface_id);
    }
}

static void uvc_events_process_setup(struct uvc_device* dev, struct usb_ctrlrequest* ctrl,
        struct uvc_request_data* resp)
{
    unsigned char type = ctrl->bRequestType & USB_TYPE_MASK;

    dev->req = 0;
    dev->control = 0;
    dev->unit_id = 0;
    dev->interface_id = 0;

    switch (type) {
        case USB_TYPE_STANDARD:
            uvc_events_process_standard_setup(dev, ctrl, resp);
            break;

        case USB_TYPE_CLASS:
            uvc_events_process_class_setup(dev, ctrl, resp);
            break;

        default:
            break;
    }
}

static const char* to_string(unsigned int format)
{
    switch (format) {
        case V4L2_PIX_FMT_H265:
            return "H265";

        case V4L2_PIX_FMT_H264:
            return "H264";

        case V4L2_PIX_FMT_MJPEG:
            return "MJPEG";

        case V4L2_PIX_FMT_YUYV:
            return "YUYV";

        case V4L2_PIX_FMT_NV21:
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_YVU420:
            return "YUV420";

        default:
            return "unknown format";
            break;
    }
}

static void handle_control_interface_data(struct uvc_device *dev, struct uvc_request_data *data)
{
    switch (dev->unit_id) {
        case UVC_CAMERA_TERMINAL_ID:
            stream_event_it_data(dev->unit_id, dev->control, data);
            break;
        case UVC_PROCESSING_UNIT_ID:
            stream_event_pu_data(dev->unit_id, dev->control, data);
            break;
        case UVC_EXTENSION_UNIT_H264_ID:
            stream_event_eu_h264_data(dev->unit_id, dev->control, data);
            break;
        case UVC_EXTENSION_UNIT_VENDOR1_ID:
            stream_event_eu_vendor1_data(dev->unit_id, dev->control, data);
            break;
        default:
            break;
    }
}

static void uvc_events_process_data(struct uvc_device* dev, struct uvc_request_data* data)
{
    int fcc;
    unsigned int iformat, iframe;
    const unsigned int* interval;
    struct uvc_streaming_control* target;
    struct uvc_streaming_control* ctrl;
    struct uvc_frame_info frame = {0};

    if ((dev->unit_id != 0) && (dev->interface_id == UVC_VC_INTERFACE_ID)) {
        return handle_control_interface_data(dev, data);
    }

    /* handle_streaming_interface_data */
    switch (dev->control) {
        case UVC_VS_PROBE_CONTROL:
            uvc_logd("setting probe control, length = %d\n", data->length);
            target = &dev->probe;
            break;

        case UVC_VS_COMMIT_CONTROL:
            uvc_logd("setting commit control, length = %d\n", data->length);
            target = &dev->commit;
            break;

        default:
            uvc_logw("setting unknown control, length = %d\n", data->length);
            return;
    }

    ctrl = (struct uvc_streaming_control*)&data->data;

    iformat = (unsigned int)ctrl->bFormatIndex;
    iframe  = (unsigned int)ctrl->bFrameIndex;

    if (find_frame((int*)&iformat, (int*)&iframe, &fcc, &frame) < 0) {
        uvc_loge("Device find {format %u, frame %u} fail\n", iformat, iframe);
        return;
    }

    if (iformat != (unsigned int)ctrl->bFormatIndex ||
        iframe  != (unsigned int)ctrl->bFrameIndex) {
        uvc_logw("Expect {foramt %u, farme %u}, Actual {format %u, farme %u}\n",
                (unsigned int)ctrl->bFormatIndex, (unsigned int)ctrl->bFrameIndex, iformat, iframe);
    }

    fflush(stdout);
    interval = frame.intervals;

    while ((interval[0] < ctrl->dwFrameInterval) && interval[1]) {
        ++interval;
    }

    target->bFormatIndex = iformat;
    target->bFrameIndex = iframe;

    switch (fcc) {
        case V4L2_PIX_FMT_YUYV:
            target->dwMaxVideoFrameSize = frame.width * frame.height * 2;
            break;

        case V4L2_PIX_FMT_NV21:
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_YVU420:
            target->dwMaxVideoFrameSize = frame.width * frame.height * 3 / 2;
            break;

        case V4L2_PIX_FMT_MJPEG:
        case V4L2_PIX_FMT_H264:
        case V4L2_PIX_FMT_H265:
            if (dev->imgsize == 0){
                uvc_logw("WARNING: MJPEG/H264 requested and no image loaded.\n");
            }
            if (dev->bulk) {
                target->dwMaxVideoFrameSize = dev->bulk_size;
            } else {
                target->dwMaxVideoFrameSize = dev->imgsize;
            }
            break;

        default:
            uvc_loge("Unkonw pixel format.\n");
            return;
    }

    target->dwFrameInterval = *interval;

    uvc_logd("set interval=%d format=%d frame=%d MaxPayloadTransferSize=%d MaxPayloadTransferSize = %d\n",
            target->dwFrameInterval,
            target->bFormatIndex,
            target->bFrameIndex,
            target->dwMaxPayloadTransferSize,
            ctrl->dwMaxPayloadTransferSize);

    if (dev->control == UVC_VS_COMMIT_CONTROL) {
        dev->fcc    = fcc;
        dev->width  = frame.width;
        dev->height = frame.height;
        dev->fps    = 10000000 / target->dwFrameInterval;

        uvc_video_set_format(dev);
        uvc_logi("\nset device format=%s width=%d height=%d fps=%d\n", to_string(dev->fcc), dev->width, dev->height, dev->fps);
        fflush(stdout);

        if (dev->bulk != 0){
            disable_uvc_video(dev);
            enable_uvc_video(dev);
        }

        memset(&dev->probe_status, 0, sizeof (dev->probe_status));
    }
}

static void uvc_events_process(struct uvc_device* dev)
{
    struct v4l2_event v4l2_event;
    struct uvc_event* uvc_event = (struct uvc_event*)(void*)&v4l2_event.u.data;
    struct uvc_request_data resp;
    int ret;

    uvc_logd("#############uvc_events_process\n");

    ret = ioctl(dev->fd, VIDIOC_DQEVENT, &v4l2_event);
    if (ret < 0) {
        uvc_loge("VIDIOC_DQEVENT failed: %s (%d)\n", strerror(errno), errno);
        return;
    }

    memset(&resp, 0, sizeof resp);
    resp.length = -EL2HLT;

    switch (v4l2_event.type) {
        case UVC_EVENT_CONNECT:
            uvc_logd("handle connect event \n");
            return;

        case UVC_EVENT_DISCONNECT:
            uvc_logd("handle disconnect event\n");
            return;

        case UVC_EVENT_SETUP:
            uvc_logd("handle setup event\n");
            uvc_events_process_setup(dev, &uvc_event->req, &resp);
            break;

        case UVC_EVENT_DATA:
            uvc_logd("handle data event\n");
            uvc_events_process_data(dev, &uvc_event->data);
            return;

        case UVC_EVENT_STREAMON:
            uvc_logi("UVC_EVENT_STREAMON\n");
            if (!dev->bulk) {
                enable_uvc_video(dev);
            }
            return;

        case UVC_EVENT_STREAMOFF:
            uvc_logi("UVC_EVENT_STREAMOFF\n");
            if (!dev->bulk) {
                disable_uvc_video(dev);
            }
            return;

        default:
            uvc_logw("Unsupport event.\n");
            return;
    }

    ret = ioctl(dev->fd, UVCIOC_SEND_RESPONSE, &resp);
    if (ret < 0) {
        uvc_loge("UVCIOC_SEND_RESPONSE failed: %s (%d)\n", strerror(errno), errno);
        return;
    }
}

static void uvc_events_init(struct uvc_device* dev)
{
    int ret;
    struct v4l2_event_subscription sub;

    uvc_fill_streaming_control(dev, &dev->probe, 0, 0);
    uvc_fill_streaming_control(dev, &dev->commit, 0, 0);

    if (dev->bulk) {
        /* FIXME Crude hack, must be negotiated with the driver. */
        dev->probe.dwMaxPayloadTransferSize  = dev->bulk_size;
        dev->commit.dwMaxPayloadTransferSize = dev->bulk_size;
    }

    memset(&sub, 0, sizeof(sub));
    sub.type = UVC_EVENT_SETUP;
    ret = ioctl(dev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    if (ret < 0) {
        uvc_loge("UVC_EVENT_SETUP failed: %s (%d)\n", strerror(errno), errno);
    }

    sub.type = UVC_EVENT_DATA;
    ret = ioctl(dev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    if (ret < 0) {
        uvc_loge("UVC_EVENT_DATA failed: %s (%d)\n", strerror(errno), errno);
    }

    sub.type = UVC_EVENT_STREAMON;
    ret = ioctl(dev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    if (ret < 0) {
        uvc_loge("UVC_EVENT_STREAMON failed: %s (%d)\n", strerror(errno), errno);
    }

    sub.type = UVC_EVENT_STREAMOFF;
    ret = ioctl(dev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    if (ret < 0) {
        uvc_loge("UVC_EVENT_STREAMOFF failed: %s (%d)\n", strerror(errno), errno);
    }

    sub.type = UVC_EVENT_CONNECT;
    ret = ioctl(dev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    if (ret < 0) {
        uvc_logw("UVC_EVENT_CONNECT failed: %s (%d)\n", strerror(errno), errno);
    }

    sub.type = UVC_EVENT_DISCONNECT;
    ret = ioctl(dev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    if (ret < 0) {
        uvc_logw("UVC_EVENT_DISCONNECT failed: %s (%d)\n", strerror(errno), errno);
    }
}

/* ---------------------------------------------------------------------------
 * main
 */

static struct uvc_device *__uvc_device = NULL;

int open_uvc_device()
{
    struct uvc_device *dev;
    const char *devpath = "/dev/video0";

    func_entry();
    dev = uvc_open(devpath);
    if (dev == 0) {
        return -1;
    }

    dev->bulk = g_bulk;
    dev->bulk_size        = get_config_value("uvc:bulksize", 1843200);
    dev->imgsize          = get_config_value("uvc:imagesize", 1843200);
    dev->max_payload_size = get_config_value("uvc:maxpacket", 1023);

    uvc_formats_init();
    uvc_events_init(dev);
    uvc_video_init(dev);

    __uvc_device = dev;

    func_success();

    return 0;
}

int close_uvc_device()
{
    if (__uvc_device != 0) {
        disable_uvc_video(__uvc_device);
        uvc_close(__uvc_device);
    }

    uvc_formats_deinit();
    __uvc_device = 0;

    return 0;
}

#if 1
int run_uvc_data()
{
    int r;
    struct timeval tv;
    fd_set wfds;

    if (!__uvc_device) {
        return -1;
    }

    FD_ZERO(&wfds);

    if (__uvc_device->streaming) {
        FD_SET(__uvc_device->fd, &wfds);
        tv.tv_sec  = 1;
        tv.tv_usec = 0;
    } else {
        tv.tv_sec  = 0;
        tv.tv_usec = 100 * 1000; /* 100ms */
    }

    r = select(__uvc_device->fd + 1, NULL, &wfds, NULL, &tv);
    if (r > 0) {
        if (FD_ISSET(__uvc_device->fd, &wfds)) {
            uvc_video_process_userptr(__uvc_device);
        }
    }

    return r;
}

int run_uvc_device()
{
    int r;
    struct timeval tv;
    fd_set efds;

    if (!__uvc_device) {
        return -1;
    }

    tv.tv_sec  = 1;
    tv.tv_usec = 0;

    FD_ZERO(&efds);

    FD_SET(__uvc_device->fd, &efds);

    r = select(__uvc_device->fd + 1, NULL, NULL, &efds, &tv);
    if (r > 0) {
        if (FD_ISSET(__uvc_device->fd, &efds)) {
            uvc_events_process(__uvc_device);
        }
    }

    return r;
}
#else

int run_uvc_device()
{
    int r;
    struct timeval tv;
    fd_set efds, wfds;

    if (!__uvc_device) {
        return -1;
    }

    tv.tv_sec  = 1;
    tv.tv_usec = 0;

    FD_ZERO(&efds);
    FD_ZERO(&wfds);

    FD_SET(__uvc_device->fd, &efds);
    if (__uvc_device->streaming) {
        FD_SET(__uvc_device->fd, &wfds);
    }

    r = select(__uvc_device->fd + 1, NULL, &wfds, &efds, &tv);
    if (r > 0) {
        if (FD_ISSET(__uvc_device->fd, &efds)) {
            uvc_events_process(__uvc_device);
        } else if (FD_ISSET(__uvc_device->fd, &wfds)) {
            uvc_video_process_userptr(__uvc_device);
        }
    }

    return r;
}
#endif
