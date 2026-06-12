/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#ifndef __DRV_VO_SYS_H__
#define __DRV_VO_SYS_H__


#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char           xmedia_uchar;
typedef unsigned char           xmedia_u8;
typedef unsigned short          xmedia_u16;
typedef unsigned int            xmedia_u32;
typedef unsigned long long      xmedia_u64;
typedef unsigned long           xmedia_ulong;

typedef char                    xmedia_char;
typedef signed char             xmedia_s8;
typedef short                   xmedia_s16;
typedef int                     xmedia_s32;
typedef long long               xmedia_s64;
typedef long                    xmedia_slong;

typedef float                   xmedia_float;
typedef double                  xmedia_double;

typedef void                    xmedia_void;

typedef xmedia_u32              xmedia_handle;

typedef xmedia_ulong            xmedia_uintptr_t;
typedef xmedia_slong            xmedia_intptr_t;

typedef enum {
    XMEDIA_FALSE    = 0,
    XMEDIA_TRUE     = 1,
} xmedia_bool;

#define XMEDIA_NULL                 0L

#define XMEDIA_SUCCESS              0
#define XMEDIA_FAILURE              (-1)

#define XMEDIA_INVALID_HANDLE       (0xffffffff)

#define XMEDIA_UNUSED(x)            ((x) = (x))

#define XMEDIA_ERRCODE_INVALID_PARAM            (xmedia_s32)(0x80010001)
#define XMEDIA_ERRCODE_NULL_PTR                 (xmedia_s32)(0x80010002)
#define XMEDIA_ERRCODE_NOT_ENOUGH_MEMORY        (xmedia_s32)(0x80010003)    // memory allocation fails
#define XMEDIA_ERRCODE_NOT_INIT                 (xmedia_s32)(0x80010004)
#define XMEDIA_ERRCODE_NOT_SUPPORT              (xmedia_s32)(0x80010005)
#define XMEDIA_ERRCODE_TIMEOUT                  (xmedia_s32)(0x80010006)
#define XMEDIA_ERRCODE_BAD_ADDRESS              (xmedia_s32)(0x80010007)
#define XMEDIA_ERRCODE_OPEN_FAILED              (xmedia_s32)(0x80010008)
#define XMEDIA_ERRCODE_CLOSE_FAILED             (xmedia_s32)(0x80010009)
#define XMEDIA_ERRCODE_CREATE_FAILED            (xmedia_s32)(0x8001000A)
#define XMEDIA_ERRCODE_DESTROY_FAILED           (xmedia_s32)(0x8001000B)
#define XMEDIA_ERRCODE_BUFFER_EMPTY             (xmedia_s32)(0x8001000C)
#define XMEDIA_ERRCODE_BUFFER_FULL              (xmedia_s32)(0x8001000D)
#define XMEDIA_ERRCODE_NO_BUFFER_FREE           (xmedia_s32)(0x8001000E)
#define XMEDIA_ERRCODE_NOT_PERMITTED            (xmedia_s32)(0x8001000F)    // Operation not permitted
#define XMEDIA_ERRCODE_EXIST                    (xmedia_s32)(0x80010010)    // Target exists
#define XMEDIA_ERRCODE_NOT_EXIST                (xmedia_s32)(0x80010011)    // Target does not exist
#define XMEDIA_ERRCODE_NOT_READY                (xmedia_s32)(0x80010012)    // Target not ready
#define XMEDIA_ERRCODE_BUSY                     (xmedia_s32)(0x80010013)    // Device or resource busy
#define XMEDIA_ERRCODE_COPY_DATA_ERROR          (xmedia_s32)(0x80010014)
#define XMEDIA_ERRCODE_TYPE_ERROR               (xmedia_s32)(0x80010015)
#define XMEDIA_ERRCODE_CHN_FULL                 (xmedia_s32)(0x80010016)
#define XMEDIA_ERRCODE_INVALID_CHN_ID           (xmedia_s32)(0x80010017)
#define XMEDIA_ERRCODE_INVALID_DEV_ID           (xmedia_s32)(0x80010018)
#define XMEDIA_ERRCODE_INVALID_PIPE_ID          (xmedia_s32)(0x80010019)
#define XMEDIA_ERRCODE_INVALID_GRP_ID           (xmedia_s32)(0x8001001A)
#define XMEDIA_ERRCODE_NOT_CONFIG               (xmedia_s32)(0x8001001B)
#define XMEDIA_ERRCODE_NOT_ENABLE               (xmedia_s32)(0x8001001C)
#define XMEDIA_ERRCODE_NOT_DISABLE              (xmedia_s32)(0x8001001D)
#define XMEDIA_ERRCODE_NOT_BIND                 (xmedia_s32)(0x8001001E)
#define XMEDIA_ERRCODE_BINDED                   (xmedia_s32)(0x8001001F)

#define MODULE_DBG_EMERG      0
#define MODULE_DBG_ALERT      1
#define MODULE_DBG_CRIT       2
#define MODULE_DBG_ERR        3
#define MODULE_DBG_WARN       4
#define MODULE_DBG_NOTICE     5
#define MODULE_DBG_INFO       6
#define MODULE_DBG_DEBUG      7
#define ALIGN_NUM             4

#define MODULE_USR_DEFAULT_DBG      MODULE_DBG_ERR       // 用户态日志管理初始化失败时各模块的打印等级
#define ATTRIBUTE __attribute__((aligned (ALIGN_NUM)))

typedef enum {
    MOD_ID_VB      = 0,
    MOD_ID_SYS     = 1,
    MOD_ID_LOG     = 2,

    MOD_ID_RGN     = 3,
    MOD_ID_CHNL    = 4,
    MOD_ID_VI      = 5,
    MOD_ID_VO      = 6,
    MOD_ID_DIS     = 7,
    MOD_ID_AVS     = 8,
    MOD_ID_VPSS    = 9,
    MOD_ID_ISP     = 10,
    MOD_ID_FB      = 11,
    MOD_ID_GDC     = 12,
    MOD_ID_TDE     = 13,
    MOD_ID_VGS     = 14,
    MOD_ID_MCF     = 15,
    MOD_ID_HDMI    = 16,

    MOD_ID_VDEC    = 17,
    MOD_ID_VENC    = 18,
    MOD_ID_VPU     = 19,
    MOD_ID_VALG    = 20,
    MOD_ID_RC      = 21,
    MOD_ID_JPEGE   = 22,
    MOD_ID_JPEGD   = 23,
    MOD_ID_H264E   = 24,
    MOD_ID_H264D   = 25,
    MOD_ID_H265E   = 26,

    MOD_ID_AIO     = 29,
    MOD_ID_AI      = 30,
    MOD_ID_AO      = 31,
    MOD_ID_AENC    = 32,
    MOD_ID_ADEC    = 33,
    MOD_ID_ACOMM   = 34,

    MOD_ID_NPU     = 35,
    MOD_ID_IVE     = 36,
    MOD_ID_USER    = 37,
    MOD_ID_PM      = 38,
    MOD_ID_GYRODIS = 39,
    MOD_ID_IR      = 40,
    MOD_ID_NPU_SVP = 41,
    MOD_ID_DDR_OST_QOS = 42,
    MOD_ID_CIPHER  = 43,

    MOD_ID_MAX,
} xmedia_mod_id;

typedef struct {
    xmedia_u32 width;
    xmedia_u32 height;
} xmedia_video_size;

typedef struct {
    xmedia_s32 x;
    xmedia_s32 y;
    xmedia_u32 width;
    xmedia_u32 height;
} xmedia_video_rect;

typedef enum {
    /* Semi-Planner */
    XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_420 = 0,
    XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420,
    XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_422,
    XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_422,
    XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_444,
    XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_444,

    /* Package */
    XMEDIA_VIDEO_PIXEL_FMT_YUYV_PACKAGE_422 = 0x40,
    XMEDIA_VIDEO_PIXEL_FMT_YVYU_PACKAGE_422,
    XMEDIA_VIDEO_PIXEL_FMT_UYVY_PACKAGE_422,
    XMEDIA_VIDEO_PIXEL_FMT_VYUY_PACKAGE_422,
    XMEDIA_VIDEO_PIXEL_FMT_YYUV_PACKAGE_422,
    XMEDIA_VIDEO_PIXEL_FMT_YYVU_PACKAGE_422,
    XMEDIA_VIDEO_PIXEL_FMT_UVYY_PACKAGE_422,
    XMEDIA_VIDEO_PIXEL_FMT_VUYY_PACKAGE_422,
    XMEDIA_VIDEO_PIXEL_FMT_VY1UY0_PACKAGE_422,

    /*yuv 400 */
    XMEDIA_VIDEO_PIXEL_FMT_YUV_400 = 0x90,

    /*RGB*/
    XMEDIA_VIDEO_PIXEL_FMT_ARGB_1555 = 0xC0,
    XMEDIA_VIDEO_PIXEL_FMT_ARGB_4444,
    XMEDIA_VIDEO_PIXEL_FMT_ARGB_8888,
    XMEDIA_VIDEO_PIXEL_FMT_ABGR_1555,
    XMEDIA_VIDEO_PIXEL_FMT_ABGR_4444,
    XMEDIA_VIDEO_PIXEL_FMT_ABGR_8888,
    XMEDIA_VIDEO_PIXEL_FMT_RGBA_1555,
    XMEDIA_VIDEO_PIXEL_FMT_RGBA_4444,
    XMEDIA_VIDEO_PIXEL_FMT_RGBA_8888,
    XMEDIA_VIDEO_PIXEL_FMT_BGRA_1555,
    XMEDIA_VIDEO_PIXEL_FMT_BGRA_4444,
    XMEDIA_VIDEO_PIXEL_FMT_BGRA_8888,

    XMEDIA_VIDEO_PIXEL_FMT_RGB_565,
    XMEDIA_VIDEO_PIXEL_FMT_RGB_888,
    XMEDIA_VIDEO_PIXEL_FMT_BGR_565,
    XMEDIA_VIDEO_PIXEL_FMT_BGR_888,

    /*CLUT*/
    XMEDIA_VIDEO_PIXEL_FMT_CLUT2 = 0x100,
    XMEDIA_VIDEO_PIXEL_FMT_CLUT4,
    XMEDIA_VIDEO_PIXEL_FMT_CLUT8,

    /*RAW*/
    XMEDIA_VIDEO_PIXEL_FMT_RAW = 0x110,

    XMEDIA_VIDEO_PIXEL_FMT_MAX = 0x120
} xmedia_video_pixel_format;

typedef enum {
    XMEDIA_VIDEO_FMT_LINEAR = 0x0,
    XMEDIA_VIDEO_FMT_TILE_16x4,
    XMEDIA_VIDEO_FMT_TILE_32x4,
    XMEDIA_VIDEO_FMT_TILE_64x4,
    XMEDIA_VIDEO_FMT_TILE_64x64,
    XMEDIA_VIDEO_FMT_MAX
} xmedia_video_format;

/*维度：
 *yuv、RGB、RAW
 *有损/无损,
 *行、帧、段紧凑、段非紧凑
 */
typedef enum {
    XMEDIA_VIDEO_COMPRESS_MODE_NONE = 0x0,
    XMEDIA_VIDEO_COMPRESS_MODE_LINE, // 3DNR 使用行压缩
    XMEDIA_VIDEO_COMPRESS_MODE_FRAME,
    XMEDIA_VIDEO_COMPRESS_MODE_SEG,
    XMEDIA_VIDEO_COMPRESS_MODE_TILE,
    XMEDIA_VIDEO_COMPRESS_MODE_SLICE,

    XMEDIA_VIDEO_COMPRESS_MODE_MAX
} xmedia_video_compress_mode;

typedef enum {
    XMEDIA_VIDEO_COLOR_SPACE_YUV = 0x0,
    XMEDIA_VIDEO_COLOR_SPACE_RGB,
    XMEDIA_VIDEO_COLOR_SPACE_MAX
} xmedia_video_color_space;

typedef enum {
    XMEDIA_VIDEO_COLOR_GAMUT_BT601 = 0x0,
    XMEDIA_VIDEO_COLOR_GAMUT_BT709,
    XMEDIA_VIDEO_COLOR_GAMUT_BT2020,
    XMEDIA_VIDEO_COLOR_GAMUT_USER,
    XMEDIA_VIDEO_COLOR_GAMUT_MAX
} xmedia_video_color_gamut;

typedef enum {
    XMEDIA_VIDEO_COLOR_LIMITED_RANGE = 0x0,
    XMEDIA_VIDEO_COLOR_FULL_RANGE,
    XMEDIA_VIDEO_COLOR_RANGE_MAX
} xmedia_video_color_quantify_range;

typedef struct {
    xmedia_video_color_space color_space;
    xmedia_video_color_gamut color_gamut;
    xmedia_video_color_quantify_range quantify_range;
} xmedia_video_color_descript;

typedef enum {
    XMEDIA_VIDEO_DYNAMIC_RANGE_SDR = 0x0,
    XMEDIA_VIDEO_DYNAMIC_RANGE_HDR10,
    XMEDIA_VIDEO_DYNAMIC_RANGE_HLG,
    XMEDIA_VIDEO_DYNAMIC_RANGE_MAX
} xmedia_video_dynamic_range;

typedef enum {
    XMEDIA_VIDEO_DATA_WIDTH_8 = 0x0,
    XMEDIA_VIDEO_DATA_WIDTH_10,
    XMEDIA_VIDEO_DATA_WIDTH_12,
    XMEDIA_VIDEO_DATA_WIDTH_14,
    XMEDIA_VIDEO_DATA_WIDTH_16,
    XMEDIA_VIDEO_DATA_WIDTH_MAX
} xmedia_video_data_width;

typedef struct {
    xmedia_u64 isp_info_phy_addr;
    xmedia_u64 lowdelay_info_phy_addr;
    xmedia_u64 vdec_info_phy_addr;

    xmedia_void *ATTRIBUTE isp_info_vir_addr;
    xmedia_void *ATTRIBUTE lowdelay_info_vir_addr;
    xmedia_void *ATTRIBUTE vdec_info_vir_addr;
} xmedia_video_private_info;

typedef struct {
    xmedia_u64 y_head_phy_addr;
    xmedia_u64 c_head_phy_addr;
    xmedia_u64 y_phy_addr;
    xmedia_u64 c_phy_addr;
    // xmedia_u64 y_buf_phy_addr; // luma buffer start phy addr
    // xmedia_u64 c_buf_phy_addr; // chroma buffer start phy addr
    // xmedia_u32 y_size; // luma buffer size
    // xmedia_u32 c_size; // chroma buffer size
} xmedia_video_frame_address;

typedef struct {
    xmedia_u32 y_head_stride;
    xmedia_u32 c_head_stride;
    xmedia_u32 y_stride;
    xmedia_u32 c_stride;
} xmedia_video_frame_stride;

typedef struct {
    xmedia_u32 index;

    xmedia_u32 width;
    xmedia_u32 height;
    xmedia_video_frame_address addr;
    xmedia_video_frame_stride stride;

    xmedia_video_data_width bit_width;
    xmedia_video_pixel_format pixel_fmt;
    xmedia_video_compress_mode compress_mode;
    xmedia_video_format video_fmt;

    xmedia_video_dynamic_range dynamic_range;
    xmedia_video_color_descript color_info;

    xmedia_u64 pts;
    xmedia_u32 time_ref;
    xmedia_u32 frame_flag;
    xmedia_video_private_info priv_info;
} xmedia_video_frame;

typedef struct {
    xmedia_video_frame frame;
    xmedia_u32 pool_id;
    xmedia_mod_id mod_id;
} xmedia_video_frame_info;

typedef enum {
    XMEDIA_INTF_MIPI_CSI_DATA_TYPE_RAW_8BIT = 0,
    XMEDIA_INTF_MIPI_CSI_DATA_TYPE_RAW_10BIT,
    XMEDIA_INTF_MIPI_CSI_DATA_TYPE_RAW_12BIT,
    XMEDIA_INTF_MIPI_CSI_DATA_TYPE_RAW_14BIT,
    XMEDIA_INTF_MIPI_CSI_DATA_TYPE_RAW_16BIT,
    XMEDIA_INTF_MIPI_CSI_DATA_TYPE_RAW_20BIT,
    XMEDIA_INTF_MIPI_CSI_DATA_TYPE_RAW_24BIT,
    XMEDIA_INTF_MIPI_CSI_DATA_TYPE_YUV420_8BIT,
    XMEDIA_INTF_MIPI_CSI_DATA_TYPE_YUV420_LEGACY_8BIT,
    XMEDIA_INTF_MIPI_CSI_DATA_TYPE_YUV422_8BIT,
    XMEDIA_INTF_MIPI_CSI_DATA_TYPE_MAX
} xmedia_intf_mipi_csi_data_type;

typedef enum {
    XMEDIA_INTF_MIPI_DSI_DATA_TYPE_YUV422_16BIT = 0,
    XMEDIA_INTF_MIPI_DSI_DATA_TYPE_RGB666_18BIT,
    XMEDIA_INTF_MIPI_DSI_DATA_TYPE_RGB888_24BIT,
    XMEDIA_INTF_MIPI_DSI_DATA_TYPE_MAX
} xmedia_intf_mipi_dsi_data_type;

typedef enum {
    XMEDIA_INTF_BT_DATA_TYPE_YUV422_8BIT = 0,
    XMEDIA_INTF_BT_DATA_TYPE_YUV422_10BIT,
    XMEDIA_INTF_BT_DATA_TYPE_MAX
} xmedia_intf_bt_data_type;

typedef enum {
    XMEDIA_INTF_LCD_DATA_TYPE_RGB565_16BIT = 0,
    XMEDIA_INTF_LCD_DATA_TYPE_RGB666_18BIT,
    XMEDIA_INTF_LCD_DATA_TYPE_RGB888_24BIT,
    XMEDIA_INTF_LCD_DATA_TYPE_MAX
} xmedia_intf_lcd_data_type;

typedef enum {
    XMEDIA_INTF_LVDS_DATA_TYPE_RGB666_18BIT = 0,
    XMEDIA_INTF_LVDS_DATA_TYPE_RGB666_36BIT,
    XMEDIA_INTF_LVDS_DATA_TYPE_RGB888_24BIT,
    XMEDIA_INTF_LVDS_DATA_TYPE_RGB888_48BIT,
    XMEDIA_INTF_LVDS_DATA_TYPE_MAX
} xmedia_intf_lvds_data_type;

typedef enum {
    XMEDIA_INTF_DC_DATA_TYPE_RAW_8BIT = 0,
    XMEDIA_INTF_DC_DATA_TYPE_RAW_10BIT,
    XMEDIA_INTF_DC_DATA_TYPE_RAW_12BIT,
    XMEDIA_INTF_DC_DATA_TYPE_RAW_14BIT,
    XMEDIA_INTF_DC_DATA_TYPE_RAW_16BIT,
    XMEDIA_INTF_DC_DATA_TYPE_MAX
} xmedia_intf_dc_data_type;

typedef enum {
    XMEDIA_INTF_TYPE_BT601 = 0,
    XMEDIA_INTF_TYPE_BT656,
    XMEDIA_INTF_TYPE_BT1120,
    XMEDIA_INTF_TYPE_MIPI_CSI,
    XMEDIA_INTF_TYPE_MIPI_DSI,
    XMEDIA_INTF_TYPE_LCD,
    XMEDIA_INTF_TYPE_LVDS,
    XMEDIA_INTF_TYPE_DC,
    XMEDIA_INTF_TYPE_MCU,
    XMEDIA_INTF_TYPE_MAX
} xmedia_intf_type;

/* bt601&bt656 */
typedef enum {
    XMEDIA_INTF_BT601_BT656_DATA_SEQ_UYVY = 0,
    XMEDIA_INTF_BT601_BT656_DATA_SEQ_VYUY,
    XMEDIA_INTF_BT601_BT656_DATA_SEQ_YUYV,
    XMEDIA_INTF_BT601_BT656_DATA_SEQ_YVYU,
    XMEDIA_INTF_BT601_BT656_DATA_SEQ_MAX
} xmedia_intf_bt601_bt656_data_seq;

typedef enum {
    XMEDIA_INTF_BT1120_DATA_SEQ_VUVU = 0,
    XMEDIA_INTF_BT1120_DATA_SEQ_UVUV,
    XMEDIA_INTF_BT1120_DATA_SEQ_MAX,
} xmedia_intf_bt1120_data_seq;

typedef enum {
    XMEDIA_INTF_LCD_DATA_SEQ_RGB = 0,
    XMEDIA_INTF_LCD_DATA_SEQ_RBG,
    XMEDIA_INTF_LCD_DATA_SEQ_GRB,
    XMEDIA_INTF_LCD_DATA_SEQ_GBR,
    XMEDIA_INTF_LCD_DATA_SEQ_BRG,
    XMEDIA_INTF_LCD_DATA_SEQ_BGR,
    XMEDIA_INTF_LCD_DATA_SEQ_MAX,
} xmedia_intf_lcd_data_seq;

typedef enum {
    XMEDIA_INTF_LVDS_FORMAT_VESA = 0,
    XMEDIA_INTF_LVDS_FORMAT_JEIDA,
    XMEDIA_INTF_LVDS_FORMAT_MAX
} xmedia_intf_lvds_format;


#ifdef __cplusplus
}
#endif

#endif
