#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "xmedia_vb.h"
#include "sample_comm.h"
#include "sample_comm_sys.h"
#include "xmedia_vgs.h"
#include "xmedia_mmz.h"
#include "loadbmp.h"

#define VGS_DEFAULT_ALIGN 16
#define SAMPLE_COMPONENT_NUM  3

#define ALIGN_UP(x, a)  ( ( ((x) + ((a) - 1) ) / (a) ) * (a) )

typedef enum {
    SAMPLE_BUFFER_TYPE_VB_ALLOC = 0,
    SAMPLE_BUFFER_TYPE_MMZ_ALLOC,
    SAMPLE_BUFFER_TYPE_MAX,
} sample_buffer_type;

typedef struct sample_private_vb {
    xmedia_bool alloc_from_private_vb; // 指定分配vb从私有pool获取
    xmedia_u32 pool_id; // 私有vb池子id
} sample_private_vb;

typedef struct sample_buffer_base {
    xmedia_u32 width;
    xmedia_u32 height;
    xmedia_video_pixel_format pixel_fmt;
    xmedia_video_format video_fmt;
    xmedia_video_compress_mode compress_mode;
    xmedia_video_data_width bit_width;
    xmedia_u32 stride_align;
} sample_buffer_base;

typedef struct sample_vb_cfg {
    xmedia_handle vb_handle;
    xmedia_u32 pool_id;
    xmedia_u32 vb_size;
    xmedia_u64 y_head_phy_addr;
    xmedia_u64 c_head_phy_addr;
    xmedia_u64 y_phy_addr;
    xmedia_u64 c_phy_addr;
    xmedia_u64 y_head_vir_addr;
    xmedia_u64 c_head_vir_addr;
    xmedia_u64 y_vir_addr;
    xmedia_u64 c_vir_addr;
    xmedia_u32 y_head_stride;
    xmedia_u32 c_head_stride;
    xmedia_u32 y_stride;
    xmedia_u32 c_stride;
    xmedia_video_private_info priv_info;
} sample_vb_cfg;

typedef struct sample_mmz_cfg {
    xmedia_u64 phy_addr[SAMPLE_COMPONENT_NUM];
    xmedia_u64 vir_addr[SAMPLE_COMPONENT_NUM];
    xmedia_u64 stride[SAMPLE_COMPONENT_NUM];
    xmedia_u32 mmz_size;
} sample_mmz_cfg;

typedef struct sample_buffer_cfg {
    sample_buffer_type buffer_type;
    union {
        sample_vb_cfg vb_cfg;
        sample_mmz_cfg mmz_cfg;
    };
} sample_buffer_cfg;

typedef struct sample_buffer_source {
    sample_buffer_type buffer_type;
    union {
        sample_private_vb vb_private;
        xmedia_u64 mmz_private; /* reserve */
    };
} sample_buffer_source;

typedef struct vgs_sample_buffer {
    sample_buffer_source buffer_source;
    sample_buffer_base buffer_base;
    sample_buffer_cfg buffer_cfg;
} vgs_sample_buffer;

xmedia_s32 sample_vgs_set_vb_param(sample_sys_config *sys_config)
{
    xmedia_vb_base_info vb_base_info = {0};
    xmedia_vb_cal_cfg vb_cal_cfg = {0};

    vb_base_info.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_420;
    vb_base_info.cmp_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;
    vb_base_info.width = 4096;
    vb_base_info.height = 4096;
    vb_base_info.bit_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    vb_base_info.align = VGS_DEFAULT_ALIGN;

    xmedia_vb_get_buffer_config(&vb_base_info, &vb_cal_cfg);
    sys_config->vb_conf.common_pool[0].block_size = vb_cal_cfg.vb_size;
    sys_config->vb_conf.common_pool[0].block_cnt  = 1;

    vb_base_info.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_422;
    vb_base_info.cmp_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;
    vb_base_info.width = 1920;
    vb_base_info.height = 1080;
    vb_base_info.bit_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    vb_base_info.align = VGS_DEFAULT_ALIGN;

    xmedia_vb_get_buffer_config(&vb_base_info, &vb_cal_cfg);
    sys_config->vb_conf.common_pool[1].block_size = vb_cal_cfg.vb_size;
    sys_config->vb_conf.common_pool[1].block_cnt  = 1;

    return XMEDIA_SUCCESS;
}

static xmedia_s32 alloc_vb_linear_buffer(sample_buffer_source *buffer_source, sample_buffer_base *buffer_base,
    sample_buffer_cfg *buffer_cfg)
{
    xmedia_vb_base_info base_info = {0};
    xmedia_vb_cal_cfg vb_cal_cfg = {0};
    xmedia_handle vb_handle;
    xmedia_u32 pool_id = VB_INVALID_POOLID;
    xmedia_video_private_info private_info = {0};

    base_info.width = buffer_base->width;
    base_info.height = buffer_base->height;
    base_info.pixel_fmt = buffer_base->pixel_fmt;
    base_info.bit_width = buffer_base->bit_width;
    base_info.cmp_mode = buffer_base->compress_mode;
    base_info.align = buffer_base->stride_align;

    xmedia_vb_get_buffer_config(&base_info, &vb_cal_cfg);

    vb_handle = xmedia_vb_get_block(pool_id, vb_cal_cfg.vb_size, XMEDIA_NULL);
    if (vb_handle == VB_INVALID_HANDLE) {
        SAMPLE_PRT("xmedia_vb_get_block(size:%d) failed!\n", vb_cal_cfg.vb_size);
        return XMEDIA_FAILURE;
    }

    xmedia_vb_get_supplement_info(vb_handle, &private_info);

    buffer_cfg->vb_cfg.vb_handle = vb_handle;
    buffer_cfg->vb_cfg.pool_id = xmedia_vb_handle_to_pool_id(vb_handle);
    buffer_cfg->vb_cfg.vb_size = vb_cal_cfg.vb_size;
    buffer_cfg->vb_cfg.y_head_phy_addr = xmedia_vb_handle_to_phy_addr(vb_handle);
    buffer_cfg->vb_cfg.c_head_phy_addr = buffer_cfg->vb_cfg.y_head_phy_addr + vb_cal_cfg.head_y_size;
    buffer_cfg->vb_cfg.y_phy_addr  = buffer_cfg->vb_cfg.y_head_phy_addr + vb_cal_cfg.head_size;
    buffer_cfg->vb_cfg.c_phy_addr  = buffer_cfg->vb_cfg.y_phy_addr + vb_cal_cfg.main_y_size;
    buffer_cfg->vb_cfg.y_head_stride = vb_cal_cfg.head_y_stride;
    buffer_cfg->vb_cfg.c_head_stride = vb_cal_cfg.head_c_stride;
    buffer_cfg->vb_cfg.y_stride = vb_cal_cfg.main_stride;
    buffer_cfg->vb_cfg.c_stride = vb_cal_cfg.main_stride;
    buffer_cfg->vb_cfg.priv_info.isp_info_phy_addr = private_info.isp_info_phy_addr;
    buffer_cfg->vb_cfg.priv_info.isp_info_vir_addr = private_info.isp_info_vir_addr;

    buffer_cfg->vb_cfg.y_head_vir_addr = (xmedia_u64)(xmedia_ulong)xmedia_mmz_map(buffer_cfg->vb_cfg.y_head_phy_addr,
        vb_cal_cfg.vb_size, XMEDIA_FALSE);
    buffer_cfg->vb_cfg.c_head_vir_addr = buffer_cfg->vb_cfg.y_head_vir_addr + vb_cal_cfg.head_y_size;
    buffer_cfg->vb_cfg.y_vir_addr = buffer_cfg->vb_cfg.y_head_vir_addr + vb_cal_cfg.head_size;
    buffer_cfg->vb_cfg.c_vir_addr = buffer_cfg->vb_cfg.y_vir_addr + vb_cal_cfg.main_y_size;

    buffer_cfg->buffer_type = SAMPLE_BUFFER_TYPE_VB_ALLOC;

    memset((xmedia_u8*)(xmedia_ulong)buffer_cfg->vb_cfg.y_head_vir_addr, 0, vb_cal_cfg.vb_size);

    return XMEDIA_SUCCESS;
}

static xmedia_s32 alloc_vb_buffer(sample_buffer_source *buffer_source, sample_buffer_base *buffer_base,
    sample_buffer_cfg *buffer_cfg)
{
    xmedia_s32 ret = 0;

    if (buffer_source == XMEDIA_NULL || buffer_base == XMEDIA_NULL || buffer_cfg == XMEDIA_NULL) {
        SAMPLE_PRT("input param is NULL!\n");
        return XMEDIA_FAILURE;
    }

    if (buffer_base->video_fmt != XMEDIA_VIDEO_FMT_LINEAR) {
        SAMPLE_PRT("wrong video fmt!\n");
        return XMEDIA_FAILURE;
    }

    ret = alloc_vb_linear_buffer(buffer_source, buffer_base, buffer_cfg);

    return ret;
}

static xmedia_s32 alloc_mmz_linear_buffer(sample_buffer_base *buffer_base, sample_buffer_cfg *buffer_cfg)
{
    xmedia_u32 stride = 0;
    xmedia_u32 size = 0;

    if (buffer_base->pixel_fmt == XMEDIA_VIDEO_PIXEL_FMT_ARGB_1555) {
        stride = ALIGN_UP(2 * buffer_base->width, buffer_base->stride_align);
    } else {
        SAMPLE_PRT("invalid pixel fmt, actual:%d\n", buffer_base->pixel_fmt);
        return XMEDIA_FAILURE;
    }

    size = stride * buffer_base->height;
    buffer_cfg->mmz_cfg.phy_addr[0] = xmedia_mmz_alloc(XMEDIA_NULL, "sample_rgb_buf", size);
    if (buffer_cfg->mmz_cfg.phy_addr[0] == 0) {
        SAMPLE_PRT("xmedia_mmz_alloc failed\n");
        return XMEDIA_FAILURE;
    }

    buffer_cfg->mmz_cfg.vir_addr[0] = (xmedia_u64)(xmedia_uintptr_t)xmedia_mmz_map(buffer_cfg->mmz_cfg.phy_addr[0],
        size, XMEDIA_FALSE);
    if (buffer_cfg->mmz_cfg.vir_addr[0] == 0) {
        SAMPLE_PRT("xmedia_mmz_map failed\n");
        xmedia_mmz_free(buffer_cfg->mmz_cfg.phy_addr[0]);
        return XMEDIA_FAILURE;
    }

    buffer_cfg->mmz_cfg.stride[0] = stride;
    buffer_cfg->buffer_type = SAMPLE_BUFFER_TYPE_MMZ_ALLOC;
    buffer_cfg->mmz_cfg.mmz_size = size;

    return XMEDIA_SUCCESS;
}

static xmedia_s32 alloc_mmz_buffer(sample_buffer_base *buffer_base, sample_buffer_cfg *buffer_cfg)
{
    xmedia_s32 ret = 0;

    if (buffer_base == XMEDIA_NULL || buffer_cfg == XMEDIA_NULL) {
        SAMPLE_PRT("input param is NULL!\n");
        return XMEDIA_FAILURE;
    }

    if (buffer_base->video_fmt != XMEDIA_VIDEO_FMT_LINEAR) {
        SAMPLE_PRT("wrong video fmt!\n");
        return XMEDIA_FAILURE;
    }

    ret = alloc_mmz_linear_buffer(buffer_base, buffer_cfg);
    return ret;
}

xmedia_s32 sample_buffer_alloc(vgs_sample_buffer *sample_buffer)
{
    xmedia_s32 ret;

    if (sample_buffer == XMEDIA_NULL) {
        SAMPLE_PRT("input param is NULL!\n");
        return XMEDIA_FAILURE;
    }

    switch (sample_buffer->buffer_source.buffer_type) {
        case SAMPLE_BUFFER_TYPE_VB_ALLOC:
            ret = alloc_vb_buffer(&sample_buffer->buffer_source, &sample_buffer->buffer_base, &sample_buffer->buffer_cfg);
            break;
        case SAMPLE_BUFFER_TYPE_MMZ_ALLOC:
            ret = alloc_mmz_buffer(&sample_buffer->buffer_base, &sample_buffer->buffer_cfg);
            break;
        default:
            SAMPLE_PRT("buffer_type only support vb or mmz, actual:%d!\n", sample_buffer->buffer_source.buffer_type);
            return XMEDIA_FAILURE;
    }
    return ret;
}

static xmedia_void vgs_sample_fill_video_frame(xmedia_video_frame_info *frame_info,
    vgs_sample_buffer *buffer)
{
    frame_info->pool_id = buffer->buffer_cfg.vb_cfg.pool_id;
    frame_info->frame.width = buffer->buffer_base.width;
    frame_info->frame.height = buffer->buffer_base.height;
    frame_info->frame.dynamic_range = XMEDIA_VIDEO_DYNAMIC_RANGE_SDR;
    frame_info->frame.color_info.color_gamut = XMEDIA_VIDEO_COLOR_GAMUT_BT709;
    frame_info->frame.pixel_fmt = buffer->buffer_base.pixel_fmt;
    frame_info->frame.pts  = 0;
    frame_info->frame.compress_mode = buffer->buffer_base.compress_mode;
    frame_info->frame.video_fmt  = buffer->buffer_base.video_fmt;
    frame_info->frame.bit_width = buffer->buffer_base.bit_width;
    frame_info->mod_id = MOD_ID_USER;
    frame_info->frame.addr.y_head_phy_addr = buffer->buffer_cfg.vb_cfg.y_head_phy_addr;
    frame_info->frame.addr.c_head_phy_addr = buffer->buffer_cfg.vb_cfg.c_head_phy_addr;
    frame_info->frame.stride.y_head_stride  = buffer->buffer_cfg.vb_cfg.y_head_stride;
    frame_info->frame.stride.c_head_stride  = buffer->buffer_cfg.vb_cfg.c_head_stride;
    frame_info->frame.addr.y_phy_addr  = buffer->buffer_cfg.vb_cfg.y_phy_addr;
    frame_info->frame.addr.c_phy_addr  = buffer->buffer_cfg.vb_cfg.c_phy_addr;
    frame_info->frame.stride.y_stride = buffer->buffer_cfg.vb_cfg.y_stride;
    frame_info->frame.stride.c_stride = buffer->buffer_cfg.vb_cfg.c_stride;
    return;
}

static xmedia_s32 buffer_read_yuv(xmedia_char *file_name, vgs_sample_buffer *buffer_cfg)
{
    FILE *fp = XMEDIA_NULL;
    xmedia_u32 size;

    if (file_name == XMEDIA_NULL || buffer_cfg == XMEDIA_NULL) {
        SAMPLE_PRT("input param is NULL!\n");
        return XMEDIA_FAILURE;
    }

    if (buffer_cfg->buffer_base.pixel_fmt != XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_420 &&
        buffer_cfg->buffer_base.pixel_fmt != XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420) {
        SAMPLE_PRT("pixel_fmt not support!\n");
        return XMEDIA_FAILURE;
    }

    fp = fopen(file_name, "r");
    if (fp == XMEDIA_NULL) {
        SAMPLE_PRT("fopen in file %s failure!\n", file_name);
        return XMEDIA_FAILURE;
    }

    size = buffer_cfg->buffer_base.width * buffer_cfg->buffer_base.height * 3 / 2;

    fread((xmedia_u8 *)(xmedia_ulong)buffer_cfg->buffer_cfg.vb_cfg.y_vir_addr, size, 1, fp);

    fclose(fp);
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_read_bmp_to_buffer(xmedia_char *bmp_name, vgs_sample_buffer *buffer_cfg_osd)
{
    xmedia_u8 *bit_map = XMEDIA_NULL;
    osd_surface surface;
    osd_bit_map_file_header bmp_file_header;
    osd_bit_map_info bmp_info;
    xmedia_u32 stride;
    canvas_size_info canvas_size = {0};

    bit_map = buffer_cfg_osd->buffer_cfg.buffer_type == SAMPLE_BUFFER_TYPE_VB_ALLOC ?
        (xmedia_u8 *)(xmedia_ulong)buffer_cfg_osd->buffer_cfg.vb_cfg.y_head_vir_addr :
        (xmedia_u8 *)(xmedia_ulong)buffer_cfg_osd->buffer_cfg.mmz_cfg.vir_addr[0];
    stride = buffer_cfg_osd->buffer_cfg.buffer_type == SAMPLE_BUFFER_TYPE_VB_ALLOC ?
        buffer_cfg_osd->buffer_cfg.vb_cfg.y_head_stride : buffer_cfg_osd->buffer_cfg.mmz_cfg.stride[0];

    if (get_bmp_info(bmp_name, &bmp_file_header, &bmp_info) < 0) {
        SAMPLE_PRT("GetBmpInfo err!\n");
        return XMEDIA_FAILURE;
    }

    switch (buffer_cfg_osd->buffer_base.pixel_fmt) {
        case XMEDIA_VIDEO_PIXEL_FMT_ARGB_8888:
            surface.color_format = OSD_COLOR_FORMAT_RGB8888;
            break;
        case XMEDIA_VIDEO_PIXEL_FMT_ARGB_4444:
            surface.color_format = OSD_COLOR_FORMAT_RGB4444;
            break;
        case XMEDIA_VIDEO_PIXEL_FMT_ARGB_1555:
            surface.color_format = OSD_COLOR_FORMAT_RGB1555;
            break;
        default:
            SAMPLE_PRT("Pixel format is not support!\n");
            return XMEDIA_FAILURE;
    }

    if (bit_map == XMEDIA_NULL) {
        SAMPLE_PRT("malloc osd memroy err!\n");
        return XMEDIA_FAILURE;
    }

    canvas_size.height = buffer_cfg_osd->buffer_base.height;
    canvas_size.width = buffer_cfg_osd->buffer_base.width;
    canvas_size.stride = stride;

    create_surface_by_canvas(bmp_name, &surface, bit_map, &canvas_size);

    return XMEDIA_SUCCESS;
}

xmedia_s32 buffer_write_yuv(char *file_name, vgs_sample_buffer *buffer_cfg)
{
    FILE *fp = XMEDIA_NULL;
    xmedia_u32 size;

    if (file_name == XMEDIA_NULL || buffer_cfg == XMEDIA_NULL) {
        SAMPLE_PRT("input param is NULL!\n");
        return XMEDIA_FAILURE;
    }

    if (buffer_cfg->buffer_base.pixel_fmt != XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_420 &&
        buffer_cfg->buffer_base.pixel_fmt != XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420) {
        SAMPLE_PRT("pixel_fmt not support!\n");
        return XMEDIA_FAILURE;
    }

    fp = fopen(file_name, "w+");
    if (fp == XMEDIA_NULL) {
        SAMPLE_PRT("fopen out file %s failure!\n", file_name);
        return XMEDIA_FAILURE;
    }

    size = buffer_cfg->buffer_base.width * buffer_cfg->buffer_base.height * 3 / 2;

    fwrite((xmedia_u8 *)(xmedia_ulong)buffer_cfg->buffer_cfg.vb_cfg.y_vir_addr, size, 1, fp);

    fclose(fp);
    return XMEDIA_SUCCESS;
}

static xmedia_s32 free_vb_buffer(sample_buffer_cfg *buffer_cfg)
{
    xmedia_s32 ret;

    ret = xmedia_mmz_unmap((xmedia_void *)(xmedia_ulong)buffer_cfg->vb_cfg.y_head_vir_addr);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("Unmap vb failed\n");
        return ret;
    }

    ret = xmedia_vb_release_block(buffer_cfg->vb_cfg.vb_handle);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("Info:xmedia_vb_release_block failed\n");
        return ret;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 free_mmz_buffer(sample_buffer_cfg *buffer_cfg)
{
    xmedia_mmz_unmap((void *)(xmedia_uintptr_t)buffer_cfg->mmz_cfg.vir_addr[0]);
    xmedia_mmz_free(buffer_cfg->mmz_cfg.phy_addr[0]);

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_buffer_free(sample_buffer_cfg *buffer_cfg)
{
    if (buffer_cfg == XMEDIA_NULL) {
        SAMPLE_PRT("buffer_cfg is NULL!\n");
        return XMEDIA_FAILURE;
    }

    switch (buffer_cfg->buffer_type) {
        case SAMPLE_BUFFER_TYPE_VB_ALLOC:
            return free_vb_buffer(buffer_cfg);
        case SAMPLE_BUFFER_TYPE_MMZ_ALLOC:
            return free_mmz_buffer(buffer_cfg);
        default:
            SAMPLE_PRT("buffer_type only support vb or mmz, actual:%d!\n", buffer_cfg->buffer_type);
            return XMEDIA_FAILURE;
    }
}

xmedia_void sample_vgs_exit(void)
{
    xmedia_vgs_exit();
    sample_comm_sys_exit();
}

xmedia_s32 sample_vgs_main_mode()
{
    xmedia_s32 handle;
    sample_sys_config sys_config = {0};
    xmedia_vgs_frame_info task_info = {0};
    vgs_sample_buffer buffer_cfg_in = {0};
    vgs_sample_buffer buffer_cfg_out = {0};
    vgs_sample_buffer buffer_cfg_osd = {0};
    xmedia_vgs_osd_attr osd_attr = {0};
    xmedia_vgs_cover_attr cover_attr = {0};
    xmedia_video_rect luma_rect = {0};
    xmedia_u64 luma_data;
    xmedia_char in_file_name[100] = "./res/car.yuv";
    xmedia_char out_file_name[100] = "./output_main_mode.yuv";
    xmedia_char osd_file_name[100] = "./res/girl.bmp";
    xmedia_s32 ret;

    ret = sample_vgs_set_vb_param(&sys_config);
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_vgs_set_vb_param failed!\n");
        return ret;
    }

    ret = sample_comm_sys_init(&sys_config);
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_sys_init failed!\n");
        return ret;
    }

    buffer_cfg_in.buffer_base.width = 1920;
    buffer_cfg_in.buffer_base.height = 1080;
    buffer_cfg_in.buffer_base.bit_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    buffer_cfg_in.buffer_base.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_420;
    buffer_cfg_in.buffer_base.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    buffer_cfg_in.buffer_base.stride_align = VGS_DEFAULT_ALIGN;
    buffer_cfg_in.buffer_base.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;
    buffer_cfg_in.buffer_source.buffer_type = SAMPLE_BUFFER_TYPE_VB_ALLOC;
    ret = sample_buffer_alloc(&buffer_cfg_in);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_buffer_alloc failed!\n");
        return XMEDIA_FAILURE;
    }

    ret = buffer_read_yuv(in_file_name, &buffer_cfg_in);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("buffer_read_yuv failed!\n");
        goto FAIL1;
    }

    buffer_cfg_out.buffer_base.width = 1280;
    buffer_cfg_out.buffer_base.height = 720;
    buffer_cfg_out.buffer_base.bit_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    buffer_cfg_out.buffer_base.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_420;
    buffer_cfg_out.buffer_base.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    buffer_cfg_out.buffer_base.stride_align = VGS_DEFAULT_ALIGN;
    buffer_cfg_out.buffer_base.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;
    buffer_cfg_out.buffer_source.buffer_type = SAMPLE_BUFFER_TYPE_VB_ALLOC;
    ret = sample_buffer_alloc(&buffer_cfg_out);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_buffer_alloc failed!\n");
        goto FAIL0;
    }

    osd_attr.osd_rect.x = 0;
    osd_attr.osd_rect.y = 0;
    osd_attr.osd_rect.width = 180;
    osd_attr.osd_rect.height = 144;
    osd_attr.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_ARGB_1555;
    osd_attr.alpha0 = 0;
    osd_attr.alpha1 = 0xff;
    osd_attr.is_osd_revert = XMEDIA_FALSE;

    buffer_cfg_osd.buffer_base.width = osd_attr.osd_rect.width;
    buffer_cfg_osd.buffer_base.height = osd_attr.osd_rect.height;
    buffer_cfg_osd.buffer_base.bit_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    buffer_cfg_osd.buffer_base.pixel_fmt = osd_attr.pixel_fmt;
    buffer_cfg_osd.buffer_base.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    buffer_cfg_osd.buffer_base.stride_align = VGS_DEFAULT_ALIGN;
    buffer_cfg_osd.buffer_base.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;
    buffer_cfg_osd.buffer_source.buffer_type = SAMPLE_BUFFER_TYPE_MMZ_ALLOC;

    ret = sample_buffer_alloc(&buffer_cfg_osd);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_buffer_alloc failed!\n");
        goto FAIL1;
    }

    ret = sample_read_bmp_to_buffer(osd_file_name, &buffer_cfg_osd);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_read_bmp_to_buffer failed!\n");
        goto FAIL2;
    }

    osd_attr.stride = buffer_cfg_osd.buffer_cfg.mmz_cfg.stride[0];
    osd_attr.phys_addr = buffer_cfg_osd.buffer_cfg.mmz_cfg.phy_addr[0];

    vgs_sample_fill_video_frame(&task_info.img_in, &buffer_cfg_in);
    vgs_sample_fill_video_frame(&task_info.img_out, &buffer_cfg_in);

    ret = xmedia_vgs_init();
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_vgs_init failed!\n");
        goto FAIL2;
    }

    ret = xmedia_vgs_create_job(&handle);
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_vgs_create_job failed!\n");
        goto FAIL2;
    }

    cover_attr.cover_type = XMEDIA_VGS_COVER_TYPE_RECT;
    cover_attr.dst_rect.x = 500;
    cover_attr.dst_rect.y = 100;
    cover_attr.dst_rect.width = 600;
    cover_attr.dst_rect.height = 600;
    cover_attr.color = 0xff;
    ret = xmedia_vgs_add_task_cover(handle, &task_info, &cover_attr, 1);
    if(ret != XMEDIA_SUCCESS) {
        xmedia_vgs_cancel_job(handle);
        SAMPLE_PRT("xmedia_vgs_add_task_cover failed!\n");
        goto FAIL2;
    }

    ret = xmedia_vgs_add_task_osd(handle, &task_info, &osd_attr, 1);
    if(ret != XMEDIA_SUCCESS) {
        xmedia_vgs_cancel_job(handle);
        SAMPLE_PRT("xmedia_vgs_add_task_osd failed!\n");
        goto FAIL2;
    }

    luma_rect.x = 100;
    luma_rect.y = 800;
    luma_rect.width = 200;
    luma_rect.height = 200;
    ret = xmedia_vgs_add_task_luma(handle, &task_info, &luma_rect, 1, &luma_data);
    if(ret != XMEDIA_SUCCESS) {
        xmedia_vgs_cancel_job(handle);
        SAMPLE_PRT("xmedia_vgs_add_task_luma failed!\n");
        goto FAIL2;
    }

    vgs_sample_fill_video_frame(&task_info.img_out, &buffer_cfg_out);
    ret = xmedia_vgs_add_task_scale(handle, &task_info, XMEDIA_VIDEO_SCALE_MODE_NORMAL);
    if(ret != XMEDIA_SUCCESS) {
        xmedia_vgs_cancel_job(handle);
        SAMPLE_PRT("xmedia_vgs_add_task_scale failed!\n");
        goto FAIL2;
    }

    ret = xmedia_vgs_submit_job(handle);
    if(ret != XMEDIA_SUCCESS) {
        xmedia_vgs_cancel_job(handle);
        SAMPLE_PRT("xmedia_vgs_submit_job failed!\n");
        goto FAIL2;
    }

    ret = xmedia_vgs_wait_job(handle, 2000);
    if(ret != XMEDIA_SUCCESS) {
        xmedia_vgs_cancel_job(handle);
        SAMPLE_PRT("xmedia_vgs_wait_job failed!\n");
        goto FAIL2;
    }

    printf("luma data = %lld\n", luma_data);

    ret = buffer_write_yuv(out_file_name, &buffer_cfg_out);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("buffer_write_yuv failed!\n");
        goto FAIL2;
    }

    sample_buffer_free(&buffer_cfg_osd.buffer_cfg);
    sample_buffer_free(&buffer_cfg_out.buffer_cfg);
    sample_buffer_free(&buffer_cfg_in.buffer_cfg);
    sample_vgs_exit();

    return XMEDIA_SUCCESS;
FAIL2:
    sample_buffer_free(&buffer_cfg_osd.buffer_cfg);
FAIL1:
    sample_buffer_free(&buffer_cfg_out.buffer_cfg);
FAIL0:
    sample_buffer_free(&buffer_cfg_in.buffer_cfg);
    sample_vgs_exit();
    return ret;
}

xmedia_s32 sample_vgs_rotate_mode()
{
    xmedia_s32 handle;
    sample_sys_config sys_config = {0};
    xmedia_vgs_frame_info task_info = {0};
    vgs_sample_buffer buffer_cfg_in = {0};
    vgs_sample_buffer buffer_cfg_out = {0};
    xmedia_video_rotation rot = XMEDIA_VIDEO_ROTATION_180;
    xmedia_char in_file_name[100] = "./res/car.yuv";
    xmedia_char out_file_name[100] = "./output_rotate_mode.yuv";
    xmedia_s32 ret;

    ret = sample_vgs_set_vb_param(&sys_config);
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_vgs_set_vb_param failed!\n");
        return ret;
    }

    ret = sample_comm_sys_init(&sys_config);
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_sys_init failed!\n");
        return ret;
    }

    buffer_cfg_in.buffer_base.width = 1920;
    buffer_cfg_in.buffer_base.height = 1080;
    buffer_cfg_in.buffer_base.bit_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    buffer_cfg_in.buffer_base.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_420;
    buffer_cfg_in.buffer_base.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    buffer_cfg_in.buffer_base.stride_align = VGS_DEFAULT_ALIGN;
    buffer_cfg_in.buffer_base.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;
    buffer_cfg_in.buffer_source.buffer_type = SAMPLE_BUFFER_TYPE_VB_ALLOC;
    ret = sample_buffer_alloc(&buffer_cfg_in);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_buffer_alloc failed!\n");
        return XMEDIA_FAILURE;
    }

    buffer_cfg_out.buffer_base.width = 1920;
    buffer_cfg_out.buffer_base.height = 1080;
    buffer_cfg_out.buffer_base.bit_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    buffer_cfg_out.buffer_base.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_420;
    buffer_cfg_out.buffer_base.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    buffer_cfg_out.buffer_base.stride_align = VGS_DEFAULT_ALIGN;
    buffer_cfg_out.buffer_base.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;
    buffer_cfg_out.buffer_source.buffer_type = SAMPLE_BUFFER_TYPE_VB_ALLOC;
    ret = sample_buffer_alloc(&buffer_cfg_out);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_buffer_alloc failed!\n");
        goto FAIL0;
    }

    vgs_sample_fill_video_frame(&task_info.img_in, &buffer_cfg_in);
    vgs_sample_fill_video_frame(&task_info.img_out, &buffer_cfg_out);

    ret = buffer_read_yuv(in_file_name, &buffer_cfg_in);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("buffer_read_yuv failed!\n");
        goto FAIL1;
    }

    ret = xmedia_vgs_init();
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_vgs_init failed!\n");
        goto FAIL1;
    }

    ret = xmedia_vgs_create_job(&handle);
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_vgs_create_job failed!\n");
        goto FAIL1;
    }

    ret = xmedia_vgs_add_task_rotation(handle, &task_info, rot);
    if(ret != XMEDIA_SUCCESS) {
        xmedia_vgs_cancel_job(handle);
        SAMPLE_PRT("xmedia_vgs_add_task_rotation failed!\n");
        goto FAIL1;
    }

    ret = xmedia_vgs_submit_job(handle);
    if(ret != XMEDIA_SUCCESS) {
        xmedia_vgs_cancel_job(handle);
        SAMPLE_PRT("xmedia_vgs_submit_job failed!\n");
        goto FAIL1;
    }

    ret = xmedia_vgs_wait_job(handle, 2000);
    if(ret != XMEDIA_SUCCESS) {
        xmedia_vgs_cancel_job(handle);
        SAMPLE_PRT("xmedia_vgs_wait_job failed!\n");
        goto FAIL1;
    }

    ret = buffer_write_yuv(out_file_name, &buffer_cfg_out);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("buffer_write_yuv failed!\n");
        goto FAIL1;
    }

    sample_buffer_free(&buffer_cfg_out.buffer_cfg);
    sample_buffer_free(&buffer_cfg_in.buffer_cfg);
    sample_vgs_exit();

    return XMEDIA_SUCCESS;
FAIL1:
    sample_buffer_free(&buffer_cfg_out.buffer_cfg);
FAIL0:
    sample_buffer_free(&buffer_cfg_in.buffer_cfg);
    sample_vgs_exit();
    return ret;
}

void sample_vgs_handle_sig(xmedia_s32 signo)
{
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);

    if (signo == SIGINT || signo == SIGTERM || signo == SIGTSTP) {
        sample_vgs_exit();
        SAMPLE_PRT("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

void sample_vgs_usage(char* prg_name)
{
    printf("Usage : %s <index> \n", prg_name);
    printf("index:\n");
    printf("\t  0) cover + osd + luma + scale\n");
    printf("\t  1) rotate \n");
}

int main(int argc, char *argv[])

{
    if (argc < 2) {
        sample_vgs_usage(argv[0]);
        return XMEDIA_FAILURE;
    }

    signal(SIGINT, sample_vgs_handle_sig);
    signal(SIGTERM, sample_vgs_handle_sig);

    switch (*argv[1]) {
        case '0':
            sample_vgs_main_mode();
            break;
        case '1':
            sample_vgs_rotate_mode();
            break;
        default :
            sample_vgs_usage(argv[0]);
            break;
    }

    return XMEDIA_SUCCESS;
}

