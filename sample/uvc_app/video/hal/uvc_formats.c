#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "uvc_formats.h"
#include "config_svc.h"
#include "uvc_log.h"

#define FORMAT_NV12  "nv12"
#define FORMAT_NV21  "nv21"
#define FORMAT_YUYV  "yuyv"
#define FORMAT_H264  "h264"
#define FORMAT_H265  "h265"
#define FORMAT_MJPEG "mjpeg"

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#define v4l2_fourcc(a, b, c, d)\
    ((__u32)(a) | ((__u32)(b) << 8) | ((__u32)(c) << 16) | ((__u32)(d) << 24))

#define V4L2_PIX_FMT_H265 v4l2_fourcc('H', '2', '6', '5')   /* H265 with start codes */


struct node {
    void* data;
    struct node* row;
    struct node* column;
};

static struct node g_formats_list_head = {
    .data = NULL,
    .row  = NULL,
    .column = NULL,
};

static int g_malloc_count = 0;

static char* fcc_to_string(int fcc)
{
    switch (fcc) {
        case V4L2_PIX_FMT_YUYV:
            return "yuyv";

        case V4L2_PIX_FMT_NV21:
            return "nv21";

        case V4L2_PIX_FMT_NV12:
            return "nv12";

        case V4L2_PIX_FMT_MJPEG:
            return "mjpeg";

        case V4L2_PIX_FMT_H264:
            return "h264";

        case V4L2_PIX_FMT_H265:
            return "h265";

        default:
            return "Unknow";
    }
}

static int string_to_fcc(char* str)
{
    if (memcmp(str, FORMAT_YUYV, strlen(str)) == 0) {
        return V4L2_PIX_FMT_YUYV;
    }

    if (memcmp(str, FORMAT_NV21, strlen(str)) == 0) {
        return V4L2_PIX_FMT_NV21;
    }

    if (memcmp(str, FORMAT_NV12, strlen(str)) == 0) {
        return V4L2_PIX_FMT_NV12;
    }

    if (memcmp(str, FORMAT_MJPEG, strlen(str)) == 0) {
        return V4L2_PIX_FMT_MJPEG;
    }

    if (memcmp(str, FORMAT_H264, strlen(str)) == 0) {
        return V4L2_PIX_FMT_H264;
    }

    if (memcmp(str, FORMAT_H265, strlen(str)) == 0) {
        return V4L2_PIX_FMT_H265;
    }

    return 0;
}

static void show_formats_info(void)
{
    int i, j;
    struct node* list_head = &g_formats_list_head;
    struct node* fmt_n = NULL;
    struct node* frm_n = NULL;
    struct uvc_format_info* fmt_info = NULL;
    struct uvc_frame_info*  frm_info = NULL;

    printf("\n");
    printf("Formats info:\n");
    fmt_n = list_head->row;
    i = 1;
    while (fmt_n) {
        fmt_info = (struct uvc_format_info*)fmt_n->data;
        printf("\tFormat %d: %s\n", i, fcc_to_string(fmt_info->fcc));
        frm_n = fmt_n->column;
        j = 1;
        while (frm_n) {
            frm_info = (struct uvc_frame_info*)frm_n->data;
            printf("\tFrame %d: \t%d x %d \t@ %d.\n",
                    j, frm_info->width, frm_info->height, frm_info->intervals[0]);
            frm_n = frm_n->column;
            j++;
        }

        fmt_n = fmt_n->row;
        i++;
        printf("\n");
    }
}

static struct node* new_node(void* data)
{
    struct node* new = malloc(sizeof(struct node));

    if (new == NULL) {
        uvc_loge("Out of memory\n");
    } else {
        g_malloc_count++;
        new->data = data;
        new->row  = NULL;
        new->column = NULL;
    }

    return new;
}

static struct node* new_format(int fcc)
{
    struct node* new_fmt = NULL;
    struct uvc_format_info* fmt = malloc(sizeof(struct uvc_format_info));

    if (fmt == NULL) {
        uvc_loge("Out of memory.\n");
    } else {
        g_malloc_count++;
        fmt->fcc      = fcc;
        fmt->frames   = NULL;
        fmt->frm_nums = 0;
        new_fmt = new_node(fmt);
        if (new_fmt == NULL) {
            free(fmt);
            g_malloc_count--;
            fmt = NULL;
        }
    }

    return new_fmt;
}

static struct node* new_frame(int w, int h, int interval)
{
    struct node* new_frm = NULL;
    struct uvc_frame_info* frm = malloc(sizeof(struct uvc_frame_info));

    if (frm == NULL) {
        uvc_loge("Out of memory.\n");
    } else {
        g_malloc_count++;
        memset(frm, 0, sizeof(struct uvc_frame_info));
        frm->width  = w;
        frm->height = h;
        frm->intervals[0] = interval;

        new_frm = new_node(frm);
        if (new_frm == NULL) {
            free(frm);
            g_malloc_count--;
            frm = NULL;
        }
    }

    return new_frm;
}

static void add_format_tail(struct node* list_head, struct node* new_fmt)
{
    struct node* cur = list_head;

    while (cur->row) {
        cur = cur->row;
    }

    cur->row = new_fmt;
}

static void add_frame_tail(struct node* list_head, int fcc, struct node* new_frm)
{
    struct node* frm = NULL;
    struct node* fmt = list_head->row;
    struct uvc_format_info* fmt_info = NULL;

    while (fmt) {
        fmt_info = (struct uvc_format_info*)fmt->data;
        if (fmt_info->fcc == fcc) {
            fmt_info->frm_nums++;
            break;
        } else {
            fmt = fmt->row;
        }
    }

    if (fmt == NULL) {
        uvc_loge("Not found format: %d.\n", fcc);
        return;
    }

    frm = fmt;
    while (frm->column) {
        frm = frm->column;
    }

    frm->column = new_frm;
}

static void add_format(struct node* list_head, int fcc)
{
    struct node* new_fmt = new_format(fcc);

    add_format_tail(list_head, new_fmt);
}

static void add_frame(struct node* list_head, int fcc, struct uvc_frame_info frm)
{
    struct node* new_frm = new_frame(frm.width, frm.height, frm.intervals[0]);

    add_frame_tail(list_head, fcc, new_frm);
}

static char* parser_sub(char* in, char* out, char k)
{
    char* tmp = in;

    while (1) {
        if (*tmp == '\0') {
            tmp = NULL;
            break;
        } else if (*tmp == k) {
            /* step k */
            tmp++;
            break;
        } else {
            *out++ = *tmp++;
        }
    }

    *out = '\0';

    return tmp;
}

static char* parser_format(char* str, char* format)
{
    char k = ',';
    return parser_sub(str, format, k);
}

static char* parser_frame(char* formats, char* frame)
{
    char k = ',';
    return parser_sub(formats, frame, k);
}

static int frame_interval(int fps)
{
    return ((1000 * 10000) / fps);
}

static void parser_whf(char* frame, struct uvc_frame_info* frm_info)
{
    int i;
    char param[16];
    char* tmp = frame;
    char k = '-';

    /* width */
    bzero(param, sizeof(param));
    tmp = parser_sub(tmp, param, k);
    frm_info->width = atoi(param);

    /* height */
    bzero(param, sizeof(param));
    tmp = parser_sub(tmp, param, k);
    frm_info->height = atoi(param);

    /* fps */
    i = 0;
    do {
        bzero(param, sizeof(param));
        tmp = parser_sub(tmp, param, k);

        if (i == ARRAY_SIZE(frm_info->intervals)) {
            break;
        }

        frm_info->intervals[i++] = frame_interval(atoi(param));
    } while (tmp != NULL);
}

static int init_format(char* key)
{
    int rc, fcc;
    char str[256];
    char frame[256];
    char format[16];
    char stream[32] = "stream:";
    char* tmp = NULL;
    struct uvc_frame_info frm_info= {0};

    strcat(stream, key);
    bzero(str, sizeof(str));
    rc = get_config_string(stream, str, sizeof(str));
    if (rc < 0) {
        uvc_logd("Not found %s\n", stream);
        return -1;
    }

    tmp = (char*)str;

    bzero(format, sizeof(format));
    tmp = parser_format(tmp, format);
    if (tmp == NULL) {
        uvc_loge("parser_format fail\n");
        return 0;
    }

    uvc_logd("Format %s: %s\n", format, tmp);

    fcc = string_to_fcc(format);
    add_format(&g_formats_list_head, fcc);
    do {
        bzero(frame, sizeof(frame));
        tmp = parser_frame(tmp, frame);

        parser_whf(frame, &frm_info);

        add_frame(&g_formats_list_head, fcc, frm_info);
    } while (tmp != NULL);

    return 0;
}

static int frame_min(int* fmt_index, int* frm_index)
{
    *fmt_index = 1;
    *frm_index = 1;

    uvc_logd("Format min {format %d, frame %d}\n", *fmt_index, *frm_index);
    return 0;
}

static int frame_max(int* fmt_index, int* frm_index)
{
    struct node* list_head = &g_formats_list_head;
    struct node* tmp = list_head;
    int index;

    index = 0;
    while (tmp->row) {
        tmp = tmp->row;
        index++;
    }
    *fmt_index = index;

    index = 0;
    while (tmp->column) {
        tmp = tmp->column;
        index++;
    }
    *frm_index = index;

    uvc_logd("Format max {format %d, frame %d}\n", *fmt_index, *frm_index);
    return 0;
}

int find_frame(int* fmt_index, int* frm_index, int* fcc, struct uvc_frame_info* frame)
{
    struct node* list_head = &g_formats_list_head;
    struct node* tmp = list_head;
    struct uvc_format_info* fmt = NULL;
    struct uvc_frame_info* frm = NULL;
    int index;

    if (list_head->row == NULL) {
        uvc_loge("UVC formats is not initialized.\n");
        return -1;
    }

    if (*fmt_index == 0) {
        frame_min(fmt_index, frm_index);
    } else if (*fmt_index < 0) {
        frame_max(fmt_index, frm_index);
    }

    if (frame == NULL || fcc == NULL) {
        return 0;
    }

    /* Find format */
    index = *fmt_index;
    while (index-- && tmp) {
        tmp = tmp->row;
    }

    if (tmp == NULL) {
        uvc_logw("Not found {format %d , frame %d }, Use {format 1, frame 1}\n", *fmt_index, *frm_index);
        show_formats_info();
        fflush(stdout);

        *fmt_index = 1;
        *frm_index = 1;

        return find_frame(fmt_index, frm_index, fcc, frame);
    }

    fmt = (struct uvc_format_info*)tmp->data;
    *fcc = fmt->fcc;

    /* Find frame */
    index = *frm_index;
    while (index-- && tmp) {
        tmp = tmp->column;
    }

    if (tmp == NULL) {
        uvc_logw("Not found {format %d, frame %d}, Use {format %d, frame 1}\n", *fmt_index, *frm_index, *fmt_index);
        show_formats_info();
        fflush(stdout);

        *frm_index = 1;

        return find_frame(fmt_index, frm_index, fcc, frame);
    }

    frm = (struct uvc_frame_info*)tmp->data;
    memcpy(frame, frm, sizeof(struct uvc_frame_info));

    uvc_logd("Find {format %d, frame %d} : %s, %dx%d @ %d\n", *fmt_index, *frm_index,
            fcc_to_string(*fcc), frame->width, frame->height, frame->intervals[0]);

    return 0;
}



void uvc_formats_init(void)
{
    int i, ret;
    char key[16];

    i = 0;
    do {
        bzero(key, sizeof(key));
        i++;
        snprintf(key, sizeof(key), "fmt%d", i);
        ret = init_format(key);
    } while (ret == 0);
}

void uvc_formats_deinit(void)
{
    struct node* list_head = &g_formats_list_head;
    struct node* row = list_head->row;
    struct node* tmp = NULL;
    struct node* column = NULL;

    while (row) {
        list_head->row = row->row;

        /* release resources */
        column = row;
        while (column) {
            tmp = column->column;
            free(column->data);
            g_malloc_count--;
            free(column);
            g_malloc_count--;
            column = tmp;
        }

        row = list_head->row;
    }

    if (g_malloc_count) {
        uvc_logw("Resource leakage.\n");
    } else {
        uvc_logi("Release resources ok.\n");
    }
}

