/*
  Copyright (c), 2001-2021, Shenshu Tech. Co., Ltd.
 */

#ifndef __LOAD_BMP_H__
#define __LOAD_BMP_H__

#include "xmedia_type.h"

/* the color format OSD supported */
typedef enum {
    OSD_COLOR_FORMAT_RGB444 = 0,
    OSD_COLOR_FORMAT_RGB4444 = 1,
    OSD_COLOR_FORMAT_RGB555 = 2,
    OSD_COLOR_FORMAT_RGB565 = 3,
    OSD_COLOR_FORMAT_RGB1555 = 4,
    OSD_COLOR_FORMAT_RGB888 = 6,
    OSD_COLOR_FORMAT_RGB8888 = 7,
    OSD_COLOR_FORMAT_CLUT2 = 8,
    OSD_COLOR_FORMAT_CLUT4 = 9,
    OSD_COLOR_FORMAT_BUTT
} osd_color_format;

typedef struct {
    xmedia_u8 r;
    xmedia_u8 g;
    xmedia_u8 b;
    xmedia_u8 reserved;
} osd_rgb;

typedef struct {
    osd_color_format color_format; /* color format */
    xmedia_u16 height;                 /* operation height */
    xmedia_u16 width;                  /* operation width */
    xmedia_u16 stride;                 /* surface stride */
    xmedia_u16 reserved;
    xmedia_u8 *virt_addr; /* virtual address */
} osd_surface;

typedef struct {
    xmedia_u32 width;   /* out */
    xmedia_u32 height;  /* out */
    xmedia_u32 stride;  /* in */
    xmedia_u8 *rgb_buf; /* in/out */
} osd_logo;

typedef struct {
    xmedia_u16 bi_size;
    xmedia_u32 bi_width;
    xmedia_s32 bi_height;
    xmedia_u16 bi_planes;
    xmedia_u16 bi_bit_count;
    xmedia_u32 bi_compression;
    xmedia_u32 bi_size_image;
    xmedia_u32 bi_x_pels_per_meter;
    xmedia_u32 bi_y_pels_per_meter;
    xmedia_u32 bi_clr_used;
    xmedia_u32 bi_clr_important;
} osd_bit_map_info_header;

typedef struct {
    xmedia_u32 bf_size;
    xmedia_u16 bf_reserved1;
    xmedia_u16 bf_reserved2;
    xmedia_u32 bf_off_bits;
} osd_bit_map_file_header;

typedef struct {
    xmedia_u8 blue;
    xmedia_u8 green;
    xmedia_u8 red;
    xmedia_u8 reserved;
} osd_rgb_quad;

typedef struct {
    osd_bit_map_info_header bmp_header;
    osd_rgb_quad bmp_colors[1];
} osd_bit_map_info;

typedef struct {
    xmedia_u8 a_len;
    xmedia_u8 r_len;
    xmedia_u8 g_len;
    xmedia_u8 b_len;
} osd_component_info;

typedef struct {
    xmedia_u32 width;
    xmedia_u32 height;
    xmedia_u32 stride;
} canvas_size_info;

typedef canvas_size_info bmp_data_size_info;

#ifdef __cplusplus
extern "C" {
#endif

xmedia_s32 load_image(const xmedia_char *filename, osd_logo *video_logo);
xmedia_s32 load_bit_map_to_surface(const xmedia_char *file_name, const osd_surface *surface, xmedia_u8 *virt);
xmedia_s32 create_surface_by_bit_map(const xmedia_char *file_name, osd_surface *surface, xmedia_u8 *virt);
xmedia_s32 create_surface_by_canvas(const xmedia_char *file_name, osd_surface *surface, xmedia_u8 *virt,
    const canvas_size_info *canvas_size);
xmedia_s32 get_bmp_info(const xmedia_char *filename, osd_bit_map_file_header *bmp_file_header, osd_bit_map_info *bmp_info);

#ifdef __cplusplus
}
#endif

#endif /* End of #ifndef __LOAD_BMP_H__ */
