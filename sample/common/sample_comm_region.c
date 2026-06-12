/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include "common.h"
#include "sample_comm.h"
#include "sample_comm_region.h"
#include "loadbmp.h"

#define OVERLAY_MIN_HANDLE   (0)
#define OVERLAYEX_MIN_HANDLE (15)
#define COVER_MIN_HANDLE     (30)
#define COVEREX_MIN_HANDLE   (45)
#define MOSAIC_MIN_HANDLE    (60)
#define MOSAICEX_MIN_HANDLE  (75)
#define CORNER_RECT_MIN_HANDLE (90)
#define CORNER_RECTEX_MIN_HANDLE (105)

#define DEFAULT_CANVAS_NUM (2)

#define BITS_NUM_PER_BYTE  (8)
#define BYTE_PER_PIX_1555  (2)
#define BYTE_PER_PIX_8888  (4)
#define PIX_PER_BYTE_CLUT2 (4)
#define PIX_PER_BYTE_CLUT4 (2)
#define OFFSET_X (40)
#define OFFSET_Y (40)

#define MAX_BIT_COUNT (32)

#define RGN_DEFAULT_WIDTH  (160)
#define RGN_DEFAULT_HEIGHT (160)
#define RGN_DEFAULT_CORNER_LENGTH (16)
#define RGN_ALPHA          (128)
#define RGN_ARGB8888_BLUE  (0xff0000ff)
#define RGN_RGB888_BLUE    (0x0000ff)
#define RGN_RGB888_RED     (0xff0000)

#define SAMPLE_RGN_HANDLE_NUM_MAX (16)
#define SAMPLE_RGN_HANDLE_NUM_MIN (1)

#define rgn_check_handle_num_return(handle_num) \
    do { \
        if (((handle_num) < SAMPLE_RGN_HANDLE_NUM_MIN) || ((handle_num) > SAMPLE_RGN_HANDLE_NUM_MAX)) { \
            SAMPLE_PRT("handle_num(%d) should be in [%d, %d].\n", \
                (handle_num), SAMPLE_RGN_HANDLE_NUM_MIN, SAMPLE_RGN_HANDLE_NUM_MAX); \
            return XMEDIA_FAILURE; \
        } \
    } while (0)

typedef struct {
    xmedia_video_pixel_format pixel_format;  /* bitmap's pixel format */
    xmedia_u32 width;               /* bitmap's width */
    xmedia_u32 height;              /* bitmap's height */
    xmedia_void* ATTRIBUTE data;      /* address of bitmap's data */
} bmp_info;

typedef struct {
    const xmedia_char *filename;
    bmp_info bmp;
    xmedia_bool fil;
    xmedia_u32 fil_color;
    xmedia_video_pixel_format pixel_fmt;
} rgn_load_bmp_info;

static osd_color_format region_mst_get_color_format_by_pixel_format(xmedia_video_pixel_format pixel_format)
{
    switch (pixel_format) {
        case XMEDIA_VIDEO_PIXEL_FMT_CLUT2:
        /* is the same with argb1555 */
        case XMEDIA_VIDEO_PIXEL_FMT_CLUT4:
        /* is the same with argb1555 */
        case XMEDIA_VIDEO_PIXEL_FMT_ARGB_1555:
            return OSD_COLOR_FORMAT_RGB1555;
        case XMEDIA_VIDEO_PIXEL_FMT_ARGB_4444:
            return OSD_COLOR_FORMAT_RGB4444;
        case XMEDIA_VIDEO_PIXEL_FMT_ARGB_8888:
            return OSD_COLOR_FORMAT_RGB8888;
        default:
            printf("pixel format is not support!\n");
            return OSD_COLOR_FORMAT_BUTT;
    }
}

static xmedia_u8 *region_mst_get_clut2_data_from_bmp_data(bmp_info *bmp)
{
    xmedia_u8 *clut_data = NULL;
    xmedia_u16 *temp = XMEDIA_NULL;
    xmedia_u8 *c_temp = XMEDIA_NULL;
    xmedia_u32 i, j, k;
    xmedia_s32 value;
    xmedia_u8 value_temp;

    temp = (xmedia_u16 *)bmp->data;

    clut_data = malloc(bmp->height * bmp->width / PIX_PER_BYTE_CLUT2);
    if (clut_data == NULL) {
        printf("malloc osd memory err!\n");
        return XMEDIA_NULL;
    }

    c_temp = (xmedia_u8 *)clut_data;
    for (i = 0; i < bmp->height; i++) {
        for (j = 0; j < bmp->width / PIX_PER_BYTE_CLUT2; j++) {
            value = 0;
            for (k = 0; k < PIX_PER_BYTE_CLUT2; k++) {
                value_temp = ((*temp & 0x001f) * 28 + ((*temp >> 5) & 0x001f) * 58 + /* 0x001f:28:5:58:color modulus */
                    ((*temp >> 10) & 0x001f) * 14) / 800; /* 10:0x001f:14:800:color modulus */
                value_temp = value_temp << (2 * (PIX_PER_BYTE_CLUT2 - k - 1)); /* 2:color modulus */
                value += value_temp;
                temp++;
            }
            *c_temp = value;
            c_temp++;
        }
    }

    return clut_data;
}

static xmedia_u8 *region_mst_get_clut4_data_from_bmp_data(bmp_info *bmp)
{
    xmedia_u8 *clut_data = NULL;
    xmedia_u16 *temp = XMEDIA_NULL;
    xmedia_u8 *c_temp = XMEDIA_NULL;
    xmedia_u32 i, j, k;
    xmedia_s32 value;
    xmedia_u8 value_temp;

    temp = (xmedia_u16 *)bmp->data;

    clut_data = malloc(bmp->height * bmp->width / PIX_PER_BYTE_CLUT4);
    if (clut_data == NULL) {
        printf("malloc osd memory err!\n");
        return XMEDIA_NULL;
    }

    c_temp = (xmedia_u8 *)clut_data;
    for (i = 0; i < bmp->height; i++) {
        for (j = 0; j < bmp->width / PIX_PER_BYTE_CLUT4; j++) {
            value = 0;
            for (k = j; k < j + PIX_PER_BYTE_CLUT4; k++) {
                value_temp = ((*temp & 0x001f) * 28 + ((*temp >> 5) & 0x001f) * 58 + /* 0x001f:28:5:58:color modulus */
                    ((*temp >> 10) & 0x001f) * 14) / 200; /* 10:0x001f:14:200:color modulus */
                temp++;
                value = (value_temp << 4) + value_temp; /* 4:color modulus */
            }
            *c_temp = value;
            c_temp++;
        }
    }

    return clut_data;
}

xmedia_void fil_bmp(bmp_info *bmp, xmedia_bool fil, xmedia_u32 fil_color)
{
    xmedia_u16 *temp = XMEDIA_NULL;
    xmedia_u32 i, j;

    if (!fil) {
        return;
    }

    temp = (xmedia_u16 *)bmp->data;
    for (i = 0; i < bmp->height; i++) {
        for (j = 0; j < bmp->width; j++) {
            if (*temp == fil_color) {
                *temp &= 0x7FFF;
            }
            temp++;
        }
    }
    return;
}

xmedia_s32 region_mst_load_bmp(rgn_load_bmp_info *load_bmp_info)
{
    osd_surface surface;
    osd_bit_map_file_header bmp_file_header;
    osd_bit_map_info bmp_info;
    xmedia_u32 bpp;
    xmedia_u8 *clut_data = NULL;
    if (get_bmp_info(load_bmp_info->filename, &bmp_file_header, &bmp_info) < 0) {
        printf("get_bmp_info err!\n");
        return XMEDIA_FAILURE;
    }

    if (bmp_info.bmp_header.bi_bit_count > MAX_BIT_COUNT || bmp_info.bmp_header.bi_width > RGN_OVERLAY_MAX_WIDTH ||
        bmp_info.bmp_header.bi_height > RGN_OVERLAY_MAX_HEIGHT) {
        printf("bmp info error!");
        return XMEDIA_FAILURE;
    }
    surface.color_format = region_mst_get_color_format_by_pixel_format(load_bmp_info->pixel_fmt);
    if (surface.color_format == OSD_COLOR_FORMAT_BUTT) {
        return XMEDIA_FAILURE;
    }
    bpp = bmp_info.bmp_header.bi_bit_count / BITS_NUM_PER_BYTE;
    load_bmp_info->bmp.data = malloc(bmp_info.bmp_header.bi_width * bpp * abs(bmp_info.bmp_header.bi_height));
    if (load_bmp_info->bmp.data == NULL) {
        printf("malloc osd memory err!\n");
        return XMEDIA_FAILURE;
    }

    create_surface_by_bit_map(load_bmp_info->filename, &surface, (xmedia_u8 *)(load_bmp_info->bmp.data));
    load_bmp_info->bmp.width = surface.width;
    load_bmp_info->bmp.height = surface.height;
    load_bmp_info->bmp.pixel_format = load_bmp_info->pixel_fmt;

    if (load_bmp_info->pixel_fmt == XMEDIA_VIDEO_PIXEL_FMT_CLUT2) {
        clut_data = region_mst_get_clut2_data_from_bmp_data(&load_bmp_info->bmp);
    } else if (load_bmp_info->pixel_fmt == XMEDIA_VIDEO_PIXEL_FMT_CLUT4) {
        clut_data = region_mst_get_clut4_data_from_bmp_data(&load_bmp_info->bmp);
    } else {
        fil_bmp(&load_bmp_info->bmp, load_bmp_info->fil, load_bmp_info->fil_color);
    }
    if (clut_data != NULL) {
        free(load_bmp_info->bmp.data);
        load_bmp_info->bmp.data = clut_data;
    }
    return XMEDIA_SUCCESS;
}

xmedia_s32 region_mst_update_canvas(rgn_load_bmp_info *load_bmp_info, xmedia_video_size *size, xmedia_u32 stride)
{
    osd_surface surface;
    osd_bit_map_file_header bmp_file_header;
    osd_bit_map_info bmp_info;
    canvas_size_info canvas_size;

    if (get_bmp_info(load_bmp_info->filename, &bmp_file_header, &bmp_info) < 0) {
        printf("get_bmp_info err!\n");
        return XMEDIA_FAILURE;
    }

    if (XMEDIA_VIDEO_PIXEL_FMT_ARGB_1555 == load_bmp_info->pixel_fmt) {
        surface.color_format = OSD_COLOR_FORMAT_RGB1555;
    } else if (XMEDIA_VIDEO_PIXEL_FMT_ARGB_4444 == load_bmp_info->pixel_fmt) {
        surface.color_format = OSD_COLOR_FORMAT_RGB4444;
    } else if (XMEDIA_VIDEO_PIXEL_FMT_ARGB_8888 == load_bmp_info->pixel_fmt) {
        surface.color_format = OSD_COLOR_FORMAT_RGB8888;
    } else if (XMEDIA_VIDEO_PIXEL_FMT_CLUT2 == load_bmp_info->pixel_fmt) {
        surface.color_format = OSD_COLOR_FORMAT_CLUT2;
    } else if (XMEDIA_VIDEO_PIXEL_FMT_CLUT4 == load_bmp_info->pixel_fmt) {
        surface.color_format = OSD_COLOR_FORMAT_CLUT4;
    } else {
        printf("pixel format is not support!\n");
        return XMEDIA_FAILURE;
    }

    if (load_bmp_info->bmp.data == NULL) {
        printf("malloc osd memory err!\n");
        return XMEDIA_FAILURE;
    }

    canvas_size.width = size->width;
    canvas_size.height = size->height;
    canvas_size.stride = stride;
    create_surface_by_canvas(load_bmp_info->filename, &surface, (xmedia_u8 *)(load_bmp_info->bmp.data), &canvas_size);

    load_bmp_info->bmp.width = surface.width;
    load_bmp_info->bmp.height = surface.height;

    if (XMEDIA_VIDEO_PIXEL_FMT_ARGB_1555 == load_bmp_info->pixel_fmt) {
        load_bmp_info->bmp.pixel_format = XMEDIA_VIDEO_PIXEL_FMT_ARGB_1555;
    } else if (XMEDIA_VIDEO_PIXEL_FMT_ARGB_4444 == load_bmp_info->pixel_fmt) {
        load_bmp_info->bmp.pixel_format = XMEDIA_VIDEO_PIXEL_FMT_ARGB_4444;
    } else if (XMEDIA_VIDEO_PIXEL_FMT_ARGB_8888 == load_bmp_info->pixel_fmt) {
        load_bmp_info->bmp.pixel_format = XMEDIA_VIDEO_PIXEL_FMT_ARGB_8888;
    } else if (XMEDIA_VIDEO_PIXEL_FMT_CLUT2 == load_bmp_info->pixel_fmt) {
        load_bmp_info->bmp.pixel_format = XMEDIA_VIDEO_PIXEL_FMT_CLUT2;
    } else if (XMEDIA_VIDEO_PIXEL_FMT_CLUT4 == load_bmp_info->pixel_fmt) {
        load_bmp_info->bmp.pixel_format = XMEDIA_VIDEO_PIXEL_FMT_CLUT4;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_region_get_min_handle(xmedia_rgn_type type)
{
    xmedia_s32 min_handle;
    switch (type) {
        case XMEDIA_RGN_TYPE_OVERLAY:
            min_handle = OVERLAY_MIN_HANDLE;
            break;
        case XMEDIA_RGN_TYPE_OVERLAYEX:
            min_handle = OVERLAYEX_MIN_HANDLE;
            break;
        case XMEDIA_RGN_TYPE_COVER:
            min_handle = COVER_MIN_HANDLE;
            break;
        case XMEDIA_RGN_TYPE_COVEREX:
            min_handle = COVEREX_MIN_HANDLE;
            break;
            break;
        case XMEDIA_RGN_TYPE_MOSAIC:
            min_handle = MOSAIC_MIN_HANDLE;
            break;
        case XMEDIA_RGN_TYPE_MOSAICEX:
            min_handle = MOSAICEX_MIN_HANDLE;
            break;
        case XMEDIA_RGN_TYPE_CORNER_RECT:
            min_handle = CORNER_RECT_MIN_HANDLE;
            break;
        case XMEDIA_RGN_TYPE_CORNER_RECTEX:
            min_handle = CORNER_RECTEX_MIN_HANDLE;
            break;
        default:
            min_handle = -1;
            break;
    }
    return min_handle;
}

xmedia_s32 sample_region_create_overlay(xmedia_s32 handle_num)
{
    xmedia_s32 ret;
    xmedia_s32 i;
    xmedia_rgn_attr region;

    region.type = XMEDIA_RGN_TYPE_OVERLAY;
    region.attr.overlay.pixel_format = XMEDIA_VIDEO_PIXEL_FMT_CLUT2;
    region.attr.overlay.size.height = RGN_DEFAULT_WIDTH;
    region.attr.overlay.size.width = RGN_DEFAULT_HEIGHT;
    region.attr.overlay.bg_color = 1;
    region.attr.overlay.canvas_num = DEFAULT_CANVAS_NUM;

    if (handle_num == 2) { /* 2 :two region */
        for (i = OVERLAY_MIN_HANDLE; i < OVERLAY_MIN_HANDLE + handle_num; i++) {
            region.attr.overlay.pixel_format = XMEDIA_VIDEO_PIXEL_FMT_ARGB_1555;
            ret = xmedia_rgn_create(i, &region);
        }
    }
    if (handle_num == 8) {                                                         /* 8:eight region */
        for (i = OVERLAY_MIN_HANDLE; i < OVERLAY_MIN_HANDLE + handle_num - 6; i++) { /* 6:ARGBCLUT2 */
            ret = xmedia_rgn_create(i, &region);
        }
        for (i = OVERLAY_MIN_HANDLE + 2; i < OVERLAY_MIN_HANDLE + handle_num - 4; i++) { /* 2:4:ARGBCLUT4 */
            region.attr.overlay.pixel_format = XMEDIA_VIDEO_PIXEL_FMT_CLUT4;
            region.attr.overlay.bg_color = 1;
            ret = xmedia_rgn_create(i, &region);
        }
        for (i = OVERLAY_MIN_HANDLE + 4; i < OVERLAY_MIN_HANDLE + handle_num - 2; i++) { /* 4:2:ARGB1555 */
            region.attr.overlay.pixel_format = XMEDIA_VIDEO_PIXEL_FMT_ARGB_1555;
            region.attr.overlay.bg_color = 0xffff;
            ret = xmedia_rgn_create(i, &region);
        }
        for (i = OVERLAY_MIN_HANDLE + 6; i < OVERLAY_MIN_HANDLE + handle_num; i++) { /* 6:ARGB4444 */
            region.attr.overlay.pixel_format = XMEDIA_VIDEO_PIXEL_FMT_ARGB_4444;
            region.attr.overlay.bg_color = 0xffff;
            ret = xmedia_rgn_create(i, &region);
        }
    } else {
        ret = XMEDIA_FALSE;
    }
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_rgn_create failed with %#x!\n", ret);
        return XMEDIA_FAILURE;
    }
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_region_create_overlayex(xmedia_s32 handle_num)
{
    xmedia_s32 ret;
    xmedia_s32 i;
    xmedia_rgn_attr region;

    region.type = XMEDIA_RGN_TYPE_OVERLAYEX;
    region.attr.overlayex.pixel_format = XMEDIA_VIDEO_PIXEL_FMT_ARGB_1555;
    region.attr.overlayex.size.height = RGN_DEFAULT_HEIGHT;
    region.attr.overlayex.size.width = RGN_DEFAULT_WIDTH;
    region.attr.overlayex.bg_color = 0x00ff;
    region.attr.overlayex.canvas_num = DEFAULT_CANVAS_NUM;
    for (i = OVERLAYEX_MIN_HANDLE; i < OVERLAYEX_MIN_HANDLE + handle_num; i++) {
        ret = xmedia_rgn_create(i, &region);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("xmedia_rgn_create failed with %#x!\n", ret);
            return XMEDIA_FAILURE;
        }
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_region_create_cover(xmedia_s32 handle_num)
{
    xmedia_s32 ret;
    xmedia_s32 i;
    xmedia_rgn_attr region;

    region.type = XMEDIA_RGN_TYPE_COVER;

    for (i = COVER_MIN_HANDLE; i < COVER_MIN_HANDLE + handle_num; i++) {
        ret = xmedia_rgn_create(i, &region);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("xmedia_rgn_create failed with %#x!\n", ret);
            return XMEDIA_FAILURE;
        }
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_region_create_coverex(xmedia_s32 handle_num)
{
    xmedia_s32 ret;
    xmedia_s32 i;
    xmedia_rgn_attr region;

    region.type = XMEDIA_RGN_TYPE_COVEREX;

    for (i = COVEREX_MIN_HANDLE; i < COVEREX_MIN_HANDLE + handle_num; i++) {
        ret = xmedia_rgn_create(i, &region);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("xmedia_rgn_create failed with %#x!\n", ret);
            return XMEDIA_FAILURE;
        }
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_region_create_mosaic(xmedia_s32 handle_num)
{
    xmedia_s32 ret;
    xmedia_s32 i;
    xmedia_rgn_attr region;

    region.type = XMEDIA_RGN_TYPE_MOSAIC;

    for (i = MOSAIC_MIN_HANDLE; i < MOSAIC_MIN_HANDLE + handle_num; i++) {
        ret = xmedia_rgn_create(i, &region);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("xmedia_rgn_create failed with %#x!\n", ret);
            return XMEDIA_FAILURE;
        }
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_region_create_mosaicex(xmedia_s32 handle_num)
{
    xmedia_s32 ret;
    xmedia_s32 i;
    xmedia_rgn_attr region;

    region.type = XMEDIA_RGN_TYPE_MOSAICEX;

    for (i = MOSAICEX_MIN_HANDLE; i < MOSAICEX_MIN_HANDLE + handle_num; i++) {
        ret = xmedia_rgn_create(i, &region);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("xmedia_rgn_create failed with %#x!\n", ret);
            return XMEDIA_FAILURE;
        }
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_region_create_corner_rect(xmedia_s32 handle_num)
{
    xmedia_s32 ret;
    xmedia_s32 i;
    xmedia_rgn_attr region;

    region.type = XMEDIA_RGN_TYPE_CORNER_RECT;

    for (i = CORNER_RECT_MIN_HANDLE; i < CORNER_RECT_MIN_HANDLE + handle_num; i++) {
        ret = xmedia_rgn_create(i, &region);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("xmedia_rgn_create failed with %#x!\n", ret);
            return XMEDIA_FAILURE;
        }
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_region_create_corner_rectex(xmedia_s32 handle_num)
{
    xmedia_s32 ret;
    xmedia_s32 i;
    xmedia_rgn_attr region;

    region.type = XMEDIA_RGN_TYPE_CORNER_RECTEX;

    for (i = CORNER_RECTEX_MIN_HANDLE; i < CORNER_RECTEX_MIN_HANDLE + handle_num; i++) {
        ret = xmedia_rgn_create(i, &region);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("xmedia_rgn_create failed with %#x!\n", ret);
            return XMEDIA_FAILURE;
        }
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_region_destroy(xmedia_u32 handle)
{
    xmedia_s32 ret;

    ret = xmedia_rgn_destroy(handle);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_rgn_destroy failed with %#x!\n", ret);
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_region_attach_to_chn(xmedia_u32 handle, xmedia_chn_info *chn, xmedia_rgn_chn_attr *chn_attr)
{
    xmedia_s32 ret;

    ret = xmedia_rgn_attach_to_chn(handle, chn, chn_attr);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_rgn_attach_to_chn failed with %#x!\n", ret);
        return XMEDIA_FAILURE;
    }
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_region_detach_from_chn(xmedia_u32 handle, xmedia_chn_info *chn)
{
    xmedia_s32 ret;
    ret = xmedia_rgn_detach_from_chn(handle, chn);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_rgn_detach_from_chn failed with %#x!\n", ret);
        return XMEDIA_FAILURE;
    }
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_region_get_up_canvas_info(xmedia_u32 handle, xmedia_rgn_canvas_info *canvas_info)
{
    xmedia_s32 ret;
    ret = xmedia_rgn_get_canvas_info(handle, canvas_info);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_rgn_get_canvas_info failed with %#x!\n", ret);
        return XMEDIA_FAILURE;
    }

    ret = xmedia_rgn_update_canvas(handle);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_rgn_update_canvas failed with %#x!\n", ret);
        return XMEDIA_FAILURE;
    }
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_region_create(xmedia_s32 handle_num, xmedia_rgn_type type)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    if (handle_num <= 0 || handle_num > 16) { /* 16:max_num */
        SAMPLE_PRT("handle_num is illegal %d!\n", handle_num);
        return XMEDIA_FAILURE;
    }
    if (type < 0 || type >= XMEDIA_RGN_TYPE_MAX) {
        SAMPLE_PRT("type is illegal %d!\n", type);
        return XMEDIA_FAILURE;
    }
    switch (type) {
        case XMEDIA_RGN_TYPE_OVERLAY:
            ret = sample_region_create_overlay(handle_num);
            break;
        case XMEDIA_RGN_TYPE_OVERLAYEX:
            ret = sample_region_create_overlayex(handle_num);
            break;
        case XMEDIA_RGN_TYPE_COVER:
            ret = sample_region_create_cover(handle_num);
            break;
        case XMEDIA_RGN_TYPE_COVEREX:
            ret = sample_region_create_coverex(handle_num);
            break;
        case XMEDIA_RGN_TYPE_MOSAIC:
            ret = sample_region_create_mosaic(handle_num);
            break;
        case XMEDIA_RGN_TYPE_MOSAICEX:
            ret = sample_region_create_mosaicex(handle_num);
            break;
        case XMEDIA_RGN_TYPE_CORNER_RECT:
            ret = sample_region_create_corner_rect(handle_num);
            break;
        case XMEDIA_RGN_TYPE_CORNER_RECTEX:
            ret = sample_region_create_corner_rectex(handle_num);
            break;
        default:
            break;
    }
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_region_create failed! handle_num:%d,type:%d!\n", handle_num, type);
        return XMEDIA_FAILURE;
    }
    return ret;
}

xmedia_s32 sample_comm_region_destroy(xmedia_s32 handle_num, xmedia_rgn_type type)
{
    xmedia_s32 i;
    xmedia_s32 ret;
    xmedia_s32 min_handle;

    if (handle_num <= 0 || handle_num > 16) { /* 16 max_num */
        SAMPLE_PRT("handle_num is illegal %d!\n", handle_num);
        return XMEDIA_FAILURE;
    }
    if (type < 0 || type >= XMEDIA_RGN_TYPE_MAX) {
        SAMPLE_PRT("type is illegal %d!\n", type);
        return XMEDIA_FAILURE;
    }
    min_handle = sample_comm_region_get_min_handle(type);
    for (i = min_handle; i < min_handle + handle_num; i++) {
        ret = sample_region_destroy(i);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("sample_comm_region_destroy failed!\n");
        }
    }
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_region_attach_check(xmedia_s32 handle_num, xmedia_rgn_type type, xmedia_chn_info *mpp_chn)
{
    if (handle_num <= 0 || handle_num > 16) { /* 16 max_num */
        SAMPLE_PRT("handle_num is illegal %d!\n", handle_num);
        return XMEDIA_FAILURE;
    }
    if (type < 0 || type >= XMEDIA_RGN_TYPE_MAX) {
        SAMPLE_PRT("type is illegal %d!\n", type);
        return XMEDIA_FAILURE;
    }
    if (mpp_chn == XMEDIA_NULL) {
        SAMPLE_PRT("mpp_chn is NULL !\n");
        return XMEDIA_FAILURE;
    }
    return XMEDIA_SUCCESS;
}

#define rgn_check_handle_min_ret(handle, min_value) \
    do { \
        if ((handle) < (min_value)) { \
            SAMPLE_PRT("handle(%d) invalid\n", (handle)); \
            return; \
        } \
    } while (0)

static xmedia_void sample_region_get_overlay_chn_attr(xmedia_s32 handle, xmedia_rgn_overlay_chn_attr *overlay_chn)
{
    rgn_check_handle_min_ret(handle, OVERLAY_MIN_HANDLE);

    overlay_chn->alpha0 = RGN_ALPHA;
    overlay_chn->alpha1 = RGN_ALPHA;
    overlay_chn->qp_info.disable = XMEDIA_TRUE;
    overlay_chn->qp_info.is_abs_qp = XMEDIA_FALSE;
    overlay_chn->qp_info.qp_val = -2;
    overlay_chn->dst = XMEDIA_RGN_ATTACH_JPEG_MAIN;
    overlay_chn->point.x = (RGN_DEFAULT_WIDTH + OFFSET_X) * (handle - OVERLAY_MIN_HANDLE);
    overlay_chn->point.y = (RGN_DEFAULT_HEIGHT + OFFSET_Y) * (handle - OVERLAY_MIN_HANDLE);
    overlay_chn->layer = handle - OVERLAY_MIN_HANDLE;
}

xmedia_s32 sample_region_set_overlay_chn_attr(xmedia_s32 handle_num,
    xmedia_rgn_chn_attr *chn_attr, xmedia_chn_info *mpp_chn)
{
    xmedia_s32 i;
    xmedia_s32 ret;

    rgn_check_handle_num_return(handle_num);

    chn_attr->type = XMEDIA_RGN_TYPE_OVERLAY;

    for (i = OVERLAY_MIN_HANDLE; i < OVERLAY_MIN_HANDLE + handle_num; i++) {
        sample_region_get_overlay_chn_attr(i, &chn_attr->attr.overlay_chn);
        ret = sample_region_attach_to_chn(i, mpp_chn, chn_attr);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("sample_region_attach_to_chn failed!\n");
            sample_comm_region_detach_frm_chn(i - OVERLAY_MIN_HANDLE + 1, XMEDIA_RGN_TYPE_OVERLAY, mpp_chn);
            return ret;
        }
    }
    return XMEDIA_SUCCESS;
}

static xmedia_void sample_region_get_overlayex_chn_attr(xmedia_s32 handle,
    xmedia_rgn_overlayex_chn_attr *overlayex_chn)
{
    rgn_check_handle_min_ret(handle, OVERLAYEX_MIN_HANDLE);

    overlayex_chn->alpha0 = RGN_ALPHA;
    overlayex_chn->alpha1 = RGN_ALPHA;
    overlayex_chn->point.x = (RGN_DEFAULT_WIDTH + OFFSET_X) * (handle - OVERLAYEX_MIN_HANDLE);
    overlayex_chn->point.y = (RGN_DEFAULT_HEIGHT + OFFSET_Y) * (handle - OVERLAYEX_MIN_HANDLE);
    overlayex_chn->layer = handle - OVERLAYEX_MIN_HANDLE;
}

xmedia_s32 sample_region_set_overlayex_chn_attr(xmedia_s32 handle_num,
    xmedia_rgn_chn_attr *chn_attr, xmedia_chn_info *mpp_chn)
{
    xmedia_s32 i;
    xmedia_s32 ret;

    rgn_check_handle_num_return(handle_num);

    chn_attr->type = XMEDIA_RGN_TYPE_OVERLAYEX;

    for (i = OVERLAYEX_MIN_HANDLE; i < OVERLAYEX_MIN_HANDLE + handle_num; i++) {
        sample_region_get_overlayex_chn_attr(i, &chn_attr->attr.overlayex_chn);
        ret = sample_region_attach_to_chn(i, mpp_chn, chn_attr);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("sample_region_attach_to_chn failed!\n");
            sample_comm_region_detach_frm_chn(i - OVERLAYEX_MIN_HANDLE + 1, XMEDIA_RGN_TYPE_OVERLAYEX, mpp_chn);
            return ret;
        }
    }
    return XMEDIA_SUCCESS;
}

static xmedia_void sample_region_get_rect_cover_chn_attr(xmedia_s32 handle, xmedia_rgn_cover_chn_attr *cover_chn)
{
    rgn_check_handle_min_ret(handle, COVER_MIN_HANDLE);

    cover_chn->layer = handle - COVER_MIN_HANDLE;

    cover_chn->cover_type = XMEDIA_RGN_COVER_TYPE_RECT;
    cover_chn->color = RGN_RGB888_BLUE;
    cover_chn->rect.height = RGN_DEFAULT_HEIGHT;
    cover_chn->rect.width = RGN_DEFAULT_WIDTH;
    cover_chn->rect.x = (RGN_DEFAULT_WIDTH + OFFSET_X) * (handle - COVER_MIN_HANDLE);
    cover_chn->rect.y = (RGN_DEFAULT_HEIGHT + OFFSET_Y) * (handle - COVER_MIN_HANDLE);
}

xmedia_s32 sample_region_set_cover_chn_attr(xmedia_s32 handle_num,
    xmedia_rgn_chn_attr *chn_attr, xmedia_chn_info *mpp_chn)
{
    xmedia_s32 i;
    xmedia_s32 ret;

    rgn_check_handle_num_return(handle_num);

    chn_attr->type = XMEDIA_RGN_TYPE_COVER;
    for (i = COVER_MIN_HANDLE; i < COVER_MIN_HANDLE + handle_num; i++) {
        sample_region_get_rect_cover_chn_attr(i, &chn_attr->attr.cover_chn);
        ret = sample_region_attach_to_chn(i, mpp_chn, chn_attr);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("sample_region_attach_to_chn failed!\n");
            sample_comm_region_detach_frm_chn(i - COVER_MIN_HANDLE + 1, XMEDIA_RGN_TYPE_COVER, mpp_chn);
            return ret;
        }
    }
    return XMEDIA_SUCCESS;
}

static xmedia_void sample_region_get_rect_coverex_chn_attr(xmedia_s32 handle,
    xmedia_rgn_coverex_chn_attr *coverex_chn)
{
    rgn_check_handle_min_ret(handle, COVEREX_MIN_HANDLE);

    coverex_chn->layer = handle - COVEREX_MIN_HANDLE;
    coverex_chn->coverex_type = XMEDIA_RGN_COVER_TYPE_RECT;
    coverex_chn->color = RGN_RGB888_BLUE;
    coverex_chn->rect.height = RGN_DEFAULT_HEIGHT;
    coverex_chn->rect.width = RGN_DEFAULT_WIDTH;
    coverex_chn->rect.x = (RGN_DEFAULT_WIDTH + OFFSET_X) * (handle - COVEREX_MIN_HANDLE);
    coverex_chn->rect.y = (RGN_DEFAULT_HEIGHT + OFFSET_Y) * (handle - COVEREX_MIN_HANDLE);
}

xmedia_s32 sample_region_set_coverex_chn_attr(xmedia_s32 handle_num,
    xmedia_rgn_chn_attr *chn_attr, xmedia_chn_info *mpp_chn)
{
    xmedia_s32 i;
    xmedia_s32 ret;

    rgn_check_handle_num_return(handle_num);

    chn_attr->type = XMEDIA_RGN_TYPE_COVEREX;

    for (i = COVEREX_MIN_HANDLE; i < COVEREX_MIN_HANDLE + handle_num; i++) {

        sample_region_get_rect_coverex_chn_attr(i, &chn_attr->attr.coverex_chn);

        ret = sample_region_attach_to_chn(i, mpp_chn, chn_attr);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("sample_region_attach_to_chn failed!\n");
            sample_comm_region_detach_frm_chn(i - COVEREX_MIN_HANDLE + 1, XMEDIA_RGN_TYPE_COVEREX, mpp_chn);
            return ret;
        }
    }
    return XMEDIA_SUCCESS;
}

static xmedia_void sample_region_get_corner_rect_chn_attr(xmedia_s32 handle,
    xmedia_rgn_corner_rect_chn_attr *corner_rect_chn)
{
    rgn_check_handle_min_ret(handle, CORNER_RECT_MIN_HANDLE);

    corner_rect_chn->rect.height = RGN_DEFAULT_HEIGHT;
    corner_rect_chn->rect.width = RGN_DEFAULT_WIDTH;
    corner_rect_chn->thick = RGN_CORNER_RECT_MIN_THICK;
    corner_rect_chn->hor_len = RGN_DEFAULT_CORNER_LENGTH;
    corner_rect_chn->ver_len = RGN_DEFAULT_CORNER_LENGTH;
    corner_rect_chn->color = RGN_RGB888_RED;
    corner_rect_chn->layer = handle - CORNER_RECT_MIN_HANDLE;
    corner_rect_chn->rect.x = (RGN_DEFAULT_WIDTH + OFFSET_X) * (handle - CORNER_RECT_MIN_HANDLE);
    corner_rect_chn->rect.y = (RGN_DEFAULT_HEIGHT + OFFSET_Y) * (handle - CORNER_RECT_MIN_HANDLE);
}

xmedia_s32 sample_region_set_corner_rect_chn_attr(xmedia_s32 handle_num,
    xmedia_rgn_chn_attr *chn_attr, xmedia_chn_info *mpp_chn)
{
    xmedia_s32 i;
    xmedia_s32 ret;

    rgn_check_handle_num_return(handle_num);

    chn_attr->type = XMEDIA_RGN_TYPE_CORNER_RECT;

    for (i = CORNER_RECT_MIN_HANDLE; i < CORNER_RECT_MIN_HANDLE + handle_num; i++) {
        sample_region_get_corner_rect_chn_attr(i, &chn_attr->attr.corner_rect_chn);
        ret = sample_region_attach_to_chn(i, mpp_chn, chn_attr);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("sample_region_attach_to_chn failed!\n");
            sample_comm_region_detach_frm_chn(i - CORNER_RECT_MIN_HANDLE + 1, XMEDIA_RGN_TYPE_CORNER_RECT, mpp_chn);
            return ret;
        }
    }
    return XMEDIA_SUCCESS;
}


static xmedia_void sample_region_get_corner_rectex_chn_attr(xmedia_s32 handle,
    xmedia_rgn_corner_rectex_chn_attr *corner_rectex_chn)
{
    rgn_check_handle_min_ret(handle, CORNER_RECTEX_MIN_HANDLE);

    corner_rectex_chn->rect.height = RGN_DEFAULT_HEIGHT;
    corner_rectex_chn->rect.width = RGN_DEFAULT_WIDTH;
    corner_rectex_chn->thick = RGN_CORNER_RECTEX_MIN_THICK;
    corner_rectex_chn->hor_len = RGN_DEFAULT_CORNER_LENGTH;
    corner_rectex_chn->ver_len = RGN_DEFAULT_CORNER_LENGTH;
    corner_rectex_chn->color = RGN_RGB888_RED;
    corner_rectex_chn->layer = handle - CORNER_RECTEX_MIN_HANDLE;
    corner_rectex_chn->rect.x = (RGN_DEFAULT_WIDTH + OFFSET_X) * (handle - CORNER_RECTEX_MIN_HANDLE);
    corner_rectex_chn->rect.y = (RGN_DEFAULT_HEIGHT + OFFSET_Y) * (handle - CORNER_RECTEX_MIN_HANDLE);
}

xmedia_s32 sample_region_set_corner_rectex_chn_attr(xmedia_s32 handle_num,
    xmedia_rgn_chn_attr *chn_attr, xmedia_chn_info *mpp_chn)
{
    xmedia_s32 i;
    xmedia_s32 ret;

    rgn_check_handle_num_return(handle_num);

    chn_attr->type = XMEDIA_RGN_TYPE_CORNER_RECTEX;

    for (i = CORNER_RECTEX_MIN_HANDLE; i < CORNER_RECTEX_MIN_HANDLE + handle_num; i++) {
        sample_region_get_corner_rectex_chn_attr(i, &chn_attr->attr.corner_rectex_chn);
        ret = sample_region_attach_to_chn(i, mpp_chn, chn_attr);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("sample_region_attach_to_chn failed!\n");
            sample_comm_region_detach_frm_chn(i - CORNER_RECTEX_MIN_HANDLE + 1, XMEDIA_RGN_TYPE_CORNER_RECTEX, mpp_chn);
            return ret;
        }
    }
    return XMEDIA_SUCCESS;
}

static xmedia_void sample_region_get_mosaic_chn_attr(xmedia_s32 handle, xmedia_rgn_mosaic_chn_attr *mosaic_chn)
{
    rgn_check_handle_min_ret(handle, MOSAIC_MIN_HANDLE);

    mosaic_chn->blk_size = XMEDIA_RGN_MOSAIC_BLK_SIZE_32;
    mosaic_chn->rect.height = RGN_DEFAULT_WIDTH;
    mosaic_chn->rect.width = RGN_DEFAULT_HEIGHT;
    mosaic_chn->rect.x = (RGN_DEFAULT_WIDTH + OFFSET_X) * (handle - MOSAIC_MIN_HANDLE);
    mosaic_chn->rect.y = (RGN_DEFAULT_HEIGHT + OFFSET_Y) * (handle - MOSAIC_MIN_HANDLE);
    mosaic_chn->layer = handle - MOSAIC_MIN_HANDLE;
}

static xmedia_s32 sample_region_set_mosaic_chn_attr(xmedia_s32 handle_num,
    xmedia_rgn_chn_attr *chn_attr, xmedia_chn_info *mpp_chn)
{
    xmedia_s32 i;
    xmedia_s32 ret;

    rgn_check_handle_num_return(handle_num);

    chn_attr->type = XMEDIA_RGN_TYPE_MOSAIC;

    for (i = MOSAIC_MIN_HANDLE; i < MOSAIC_MIN_HANDLE + handle_num; i++) {
        sample_region_get_mosaic_chn_attr(i, &chn_attr->attr.mosaic_chn);
        ret = sample_region_attach_to_chn(i, mpp_chn, chn_attr);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("sample_region_attach_to_chn failed!\n");
            sample_comm_region_detach_frm_chn(i - MOSAIC_MIN_HANDLE + 1, XMEDIA_RGN_TYPE_MOSAIC, mpp_chn);
            return ret;
        }
    }
    return XMEDIA_SUCCESS;
}

static xmedia_void sample_region_get_mosaicex_chn_attr(xmedia_s32 handle, xmedia_rgn_mosaicex_chn_attr *mosaicex_chn)
{
    rgn_check_handle_min_ret(handle, MOSAICEX_MIN_HANDLE);

    mosaicex_chn->blk_size = XMEDIA_RGN_MOSAIC_BLK_SIZE_16;
    mosaicex_chn->rect.height = RGN_DEFAULT_WIDTH;
    mosaicex_chn->rect.width = RGN_DEFAULT_HEIGHT;
    mosaicex_chn->rect.x = (RGN_DEFAULT_WIDTH + OFFSET_X) * (handle - MOSAICEX_MIN_HANDLE);
    mosaicex_chn->rect.y = (RGN_DEFAULT_HEIGHT + OFFSET_Y) * (handle - MOSAICEX_MIN_HANDLE);
    mosaicex_chn->layer = handle - MOSAICEX_MIN_HANDLE;
}

xmedia_s32 sample_region_set_mosaicex_chn_attr(xmedia_s32 handle_num,
    xmedia_rgn_chn_attr *chn_attr, xmedia_chn_info *mpp_chn)
{
    xmedia_s32 i;
    xmedia_s32 ret;

    rgn_check_handle_num_return(handle_num);

    chn_attr->type = XMEDIA_RGN_TYPE_MOSAICEX;

    for (i = MOSAICEX_MIN_HANDLE; i < MOSAICEX_MIN_HANDLE + handle_num; i++) {
        sample_region_get_mosaicex_chn_attr(i, &chn_attr->attr.mosaicex_chn);
        ret = sample_region_attach_to_chn(i, mpp_chn, chn_attr);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("sample_region_attach_to_chn failed!\n");
            sample_comm_region_detach_frm_chn(i - MOSAICEX_MIN_HANDLE + 1, MOSAICEX_MIN_HANDLE, mpp_chn);
            return ret;
        }
    }
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_region_set_chn_attr(xmedia_s32 handle_num, xmedia_rgn_type type, xmedia_rgn_chn_attr *chn_attr,
    xmedia_chn_info *chn)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    switch (type) {
        case XMEDIA_RGN_TYPE_OVERLAY:
            ret = sample_region_set_overlay_chn_attr(handle_num, chn_attr, chn);
            break;
        case XMEDIA_RGN_TYPE_OVERLAYEX:
            ret = sample_region_set_overlayex_chn_attr(handle_num, chn_attr, chn);
            break;
        case XMEDIA_RGN_TYPE_COVER:
            ret = sample_region_set_cover_chn_attr(handle_num, chn_attr, chn);
            break;
        case XMEDIA_RGN_TYPE_COVEREX:
            ret = sample_region_set_coverex_chn_attr(handle_num, chn_attr, chn);
            break;
        case XMEDIA_RGN_TYPE_MOSAIC:
            ret = sample_region_set_mosaic_chn_attr(handle_num, chn_attr, chn);
            break;
        case XMEDIA_RGN_TYPE_MOSAICEX:
            ret = sample_region_set_mosaicex_chn_attr(handle_num, chn_attr, chn);
            break;
        case XMEDIA_RGN_TYPE_CORNER_RECT:
            ret = sample_region_set_corner_rect_chn_attr(handle_num, chn_attr, chn);
            break;
        case XMEDIA_RGN_TYPE_CORNER_RECTEX:
            ret = sample_region_set_corner_rectex_chn_attr(handle_num, chn_attr, chn);
            break;
        default:
            break;
    }
    return ret;
}

xmedia_s32 sample_comm_region_attach_to_chn(xmedia_s32 handle_num, xmedia_rgn_type type, xmedia_chn_info *mpp_chn)
{
    xmedia_s32 ret;
    xmedia_rgn_chn_attr chn_attr = {0};
    ret = sample_comm_region_attach_check(handle_num, type, mpp_chn);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }
    /* set the chn config */
    chn_attr.is_show = XMEDIA_TRUE;
    ret = sample_region_set_chn_attr(handle_num, type, &chn_attr, mpp_chn);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_region_attach_to_chn failed!\n");
    }
    return ret;
}

xmedia_s32 sample_comm_region_check_min(xmedia_s32 min_handle)
{
    if ((min_handle < 0) || (min_handle > RGN_HANDLE_MAX)) {
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_region_detach_frm_chn(xmedia_s32 handle_num, xmedia_rgn_type type, xmedia_chn_info *mpp_chn)
{
    xmedia_s32 i;
    xmedia_s32 ret;
    xmedia_s32 min_handle;

    if (handle_num <= 0 || handle_num > 16) { /* 16:max region num */
        SAMPLE_PRT("handle_num is illegal %d!\n", handle_num);
        return XMEDIA_FAILURE;
    }
    if (type < 0 || type >= XMEDIA_RGN_TYPE_MAX) {
        SAMPLE_PRT("type is illegal %d!\n", type);
        return XMEDIA_FAILURE;
    }
    if (mpp_chn == XMEDIA_NULL) {
        SAMPLE_PRT("mpp_chn is NULL !\n");
        return XMEDIA_FAILURE;
    }
    min_handle = sample_comm_region_get_min_handle(type);
    if (sample_comm_region_check_min(min_handle) != XMEDIA_SUCCESS) {
        SAMPLE_PRT("min_handle(%d) should be in [0, %d).\n", min_handle, RGN_HANDLE_MAX);
        return XMEDIA_FAILURE;
    }
    for (i = min_handle; i < min_handle + handle_num; i++) {
        ret = sample_region_detach_from_chn(i, mpp_chn);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("sample_region_detach_from_chn failed! handle:%d\n", i);
        }
    }
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_region_get_up_canvas(xmedia_u32 handle, const xmedia_char *bmp_path)
{
    xmedia_s32 ret;
    xmedia_video_size size;
    xmedia_rgn_canvas_info canvas_info;
    rgn_load_bmp_info load_bmp_info = {0};

    ret = xmedia_rgn_get_canvas_info(handle, &canvas_info);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_rgn_get_canvas_info failed with %#x!\n", ret);
        return XMEDIA_FAILURE;
    }

    size.width = canvas_info.size.width;
    size.height = canvas_info.size.height;
    load_bmp_info.bmp.data = (xmedia_void*)(xmedia_uintptr_t)canvas_info.virt_addr;
    load_bmp_info.pixel_fmt = canvas_info.pixel_format;
    load_bmp_info.fil = XMEDIA_FALSE;
    load_bmp_info.fil_color = 0;
    load_bmp_info.filename = bmp_path;
    ret = region_mst_update_canvas(&load_bmp_info, &size, canvas_info.stride);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("region_mst_update_canvas failed with %#x!\n", ret);
        return XMEDIA_FAILURE;
    }

    ret = xmedia_rgn_update_canvas(handle);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_rgn_update_canvas failed with %#x!\n", ret);
        return XMEDIA_FAILURE;
    }

    return ret;
}

xmedia_s32 sample_comm_region_init(xmedia_void)
{
    return xmedia_rgn_init();
}

xmedia_void sample_comm_region_exit(xmedia_void)
{
    return xmedia_rgn_exit();
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

