/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#ifndef __DRV_VO_COMMON_H__
#define __DRV_VO_COMMON_H__
#include <net.h>
#include <exports.h>
#include <serial.h>
#include <xyzModem.h>
#include "xmedia_vo.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VO_MIN_CHN_WIDTH               32
#define VO_MIN_CHN_HEIGHT              32
#define VO_MAX_ZOOM_RATIO              1000
#define VO_MAX_DEV_NUM                 4
#define VO_MAX_LAYER_NUM               4
#define VO_MAX_CHN_NUM                 4
#define VO_MIN_TOLERATE                1
#define VO_MAX_TOLERATE                100000

#define VO_INVALID_CHN_ID VO_MAX_CHN_NUM
#define VO_INVALID_NODE_INDEX 0

#ifdef xmorca
#define XMEDIA_VO_REAL_DEV_NUM XMEDIA_VO_DEV_1
#define XMEDIA_VO_REAL_LAYER_NUM XMEDIA_VO_LAYER_V1
#define XMEDIA_VO_VIRT_DEV_NUM 1
#define XMEDIA_VO_VIRT_LAYER_NUM 1
#endif

#ifdef xmfalcon
#define XMEDIA_VO_REAL_DEV_NUM   XMEDIA_VO_DEV_VIRT_0
#define XMEDIA_VO_REAL_LAYER_NUM XMEDIA_VO_LAYER_VIRT_V0
#define XMEDIA_VO_VIRT_DEV_NUM 2
#define XMEDIA_VO_VIRT_LAYER_NUM 2
#endif
#define VO_CHECK_VAILD_DEV(dev) ((((dev) >= (XMEDIA_VO_DEV_0)) && ((dev) < (XMEDIA_VO_REAL_DEV_NUM))) || \
    (((dev) >= (XMEDIA_VO_DEV_VIRT_0)) && ((dev) < ((XMEDIA_VO_DEV_VIRT_0) + (XMEDIA_VO_VIRT_DEV_NUM)))))

#define VO_CHECK_REAL_DEV(dev) (((dev) >= (XMEDIA_VO_DEV_0)) && ((dev) < (XMEDIA_VO_REAL_DEV_NUM)))

#define VO_CHECK_VIRT_DEV(dev) (((dev) >= XMEDIA_VO_DEV_VIRT_0) && \
    ((dev) < ((XMEDIA_VO_DEV_VIRT_0) + (XMEDIA_VO_VIRT_DEV_NUM))))

#define VO_CHECK_VAILD_LAYER(layer) ((((layer) >= (XMEDIA_VO_LAYER_V0)) && ((layer) < (XMEDIA_VO_REAL_LAYER_NUM))) || \
    (((layer) >= (XMEDIA_VO_LAYER_VIRT_V0)) && ((layer) < ((XMEDIA_VO_LAYER_VIRT_V0) + (XMEDIA_VO_VIRT_LAYER_NUM)))))


#define VO_TRACE(level, fmt, ...)                                                                            \
    do {                                                                                                     \
        if (level <= MODULE_DBG_ERR) { \
            printf("[Func]:%s [Line]:%d [Info]:" fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)

#define CHECK_VO_PTR_RETURN(pointer)                     \
    do {                                           \
           if ((pointer) == XMEDIA_NULL) {             \
               VO_TRACE(MODULE_DBG_ERR,"vo para is null ptr.\n");  \
               return XMEDIA_ERRCODE_NULL_PTR;        \
           }                                       \
    } while (0)

#define CHECK_VO_DEV_ID_RETURN(dev)                              \
    do {                                                  \
           if (dev >= XMEDIA_VO_DEV_MAX) {       \
               VO_TRACE(MODULE_DBG_ERR,"vo dev %d is invalid.\n",dev);     \
               return XMEDIA_ERRCODE_INVALID_DEV_ID;           \
           }                                              \
    } while (0)

#define CHECK_VO_DEV_ID(dev)                              \
   do {                                                  \
          if (dev >= XMEDIA_VO_DEV_MAX) {       \
              VO_TRACE(MODULE_DBG_ERR,"vo dev %d is invalid.\n",dev);     \
              return;           \
          }                                              \
   } while (0)


#define CHECK_VO_LAYER_ID_RETURN(layer)                              \
    do {                                                  \
          if (layer >= XMEDIA_VO_LAYER_MAX){       \
              VO_TRACE(MODULE_DBG_ERR,"vo layer %d is invalid.\n",layer);     \
              return XMEDIA_ERRCODE_INVALID_GRP_ID;           \
          }                                              \
    } while (0)

#define CHECK_VO_CHN_ID_RETURN(chn)                              \
    do {                                                  \
           if ((chn < 0) || (chn >= VO_MAX_CHN_NUM)) {      \
               VO_TRACE(MODULE_DBG_ERR,"vo chn %d is invalid.\n",chn);     \
               return XMEDIA_ERRCODE_INVALID_CHN_ID;           \
           }                                              \
    } while (0)

#define CHECK_VO_ZERO_RETURN(value)                              \
      do {                                                  \
             if (value == 0) {      \
                 VO_TRACE(MODULE_DBG_ERR,"a denominator of 0 is illegal\n");     \
                 return XMEDIA_ERRCODE_NOT_PERMITTED;           \
             }                                              \
      } while (0)

#define IS_ALIGN(x, bitw) (((x % bitw) == 0) ? XMEDIA_TRUE : XMEDIA_FALSE)

#define osal_memset(ptr, value, len) memset(ptr, value, len)
#define osal_mdelay(msecs)           mdelay(msecs)
#define osal_memcpy(dst, src, size)  memcpy(dst, src, size)
#define osal_udelay(usecs)           udelay(usecs)
#define osal_vmalloc(size)           malloc(size)
#define osal_vfree(addr)             free(addr)

typedef enum {
   VO_ISR_INTERRUPT_TYPE = 0,
   VO_ISR_INTERRUPT_TYPE_DHD0_VTT0 = 0x1,
   VO_ISR_INTERRUPT_TYPE_DHD0_VTT1 = 0x2,
   VO_ISR_INTERRUPT_TYPE_DHD0_VTT2 = 0x4,
   VO_ISR_INTERRUPT_TYPE_DHD0_VTT3 = 0x8,
   VO_ISR_INTERRUPT_TYPE_DHD0_UFINT = 0x10000, //low band width

   VO_ISR_INTERRUPT_TYPE_DHD1_VTT0 = 0x10,
   VO_ISR_INTERRUPT_TYPE_DHD1_VTT1 = 0x20,
   VO_ISR_INTERRUPT_TYPE_DHD1_VTT2 = 0x40,
   VO_ISR_INTERRUPT_TYPE_DHD1_VTT3 = 0x80,
   VO_ISR_INTERRUPT_TYPE_DHD1_UFINT = 0x20000,

   VO_ISR_INTERRUPT_TYPE_LOW_DELAY_CERR = 0x100000,
   VO_ISR_INTERRUPT_TYPE_LOW_DELAY_YERR = 0x200000,

   VO_ISR_INTERRUPT_TYPE_BUSR_ERR = 0x1000000,
   VO_ISR_INTERRUPT_TYPE_BUSW_ERR = 0x2000000,

   VO_ISR_INTERRUPT_TYPE_ALL = 0xffffffff,
} vo_isr_interrupt_type;

typedef enum {
    VO_ISR_FIELD_FLAG_FRAME = 0,
    VO_ISR_FIELD_FLAG_TOP,
    VO_ISR_FIELD_FLAG_BOTTOM,
    VO_ISR_FIELD_FLAG_TOP_BOTTOM,
    VO_ISR_FIELD_FLAG_MAX
} vo_isr_field_flag;

#ifdef xmorca
typedef enum {
    VO_CSC_YUV2RGB_601_LIMIT2FULL = 0,
    VO_CSC_YUV2RGB_709_LIMIT2FULL,
    VO_CSC_YUV2RGB_601_FULL2LIMIT,
    VO_CSC_YUV2RGB_709_FULL2LIMIT,
    VO_CSC_YUV2RGB_601_FULL2FULL,
    VO_CSC_YUV2RGB_709_FULL2FULL,
    VO_CSC_YUV2RGB_601_LIMIT2LIMIT,
    VO_CSC_YUV2RGB_709_LIMIT2LIMIT,

    VO_CSC_YUV2YUV_601_601_LIMIT2LIMIT = 8,
    VO_CSC_YUV2YUV_601_709_LIMIT2LIMIT,
    VO_CSC_YUV2YUV_709_601_LIMIT2LIMIT,
    VO_CSC_YUV2YUV_709_709_LIMIT2LIMIT,

    VO_CSC_YUV2YUV_601_601_FULL2FULL = 12,
    VO_CSC_YUV2YUV_601_709_FULL2FULL,
    VO_CSC_YUV2YUV_709_601_FULL2FULL,
    VO_CSC_YUV2YUV_709_709_FULL2FULL,

    VO_CSC_YUV2YUV_601_601_LIMIT2FULL = 16,
    VO_CSC_YUV2YUV_601_709_LIMIT2FULL,
    VO_CSC_YUV2YUV_709_601_LIMIT2FULL,
    VO_CSC_YUV2YUV_709_709_LIMIT2FULL,

    VO_CSC_YUV2YUV_601_601_FULL2LIMIT = 20,
    VO_CSC_YUV2YUV_601_709_FULL2LIMIT,
    VO_CSC_YUV2YUV_709_601_FULL2LIMIT,
    VO_CSC_YUV2YUV_709_709_FULL2LIMIT,
    VO_CSC_MAX
} vo_csc_type;
#endif

#ifdef xmfalcon
typedef enum {
    VO_CSC_YUV2RGB_601_LIMIT2FULL = 0,
    VO_CSC_YUV2RGB_709_LIMIT2FULL,
    VO_CSC_YUV2RGB_2020_LIMIT2FULL,
    VO_CSC_YUV2RGB_601_FULL2LIMIT,
    VO_CSC_YUV2RGB_709_FULL2LIMIT,
    VO_CSC_YUV2RGB_2020_FULL2LIMIT,
    VO_CSC_YUV2RGB_601_FULL2FULL,
    VO_CSC_YUV2RGB_709_FULL2FULL,
    VO_CSC_YUV2RGB_2020_FULL2FULL,
    VO_CSC_YUV2RGB_601_LIMIT2LIMIT,
    VO_CSC_YUV2RGB_709_LIMIT2LIMIT,
    VO_CSC_YUV2RGB_2020_LIMIT2LIMIT,

    VO_CSC_RGB2YUV_601_LIMIT2FULL = 12,
    VO_CSC_RGB2YUV_709_LIMIT2FULL,
    VO_CSC_RGB2YUV_2020_LIMIT2FULL,
    VO_CSC_RGB2YUV_601_FULL2LIMIT,
    VO_CSC_RGB2YUV_709_FULL2LIMIT,
    VO_CSC_RGB2YUV_2020_FULL2LIMIT,
    VO_CSC_RGB2YUV_601_FULL2FULL,
    VO_CSC_RGB2YUV_709_FULL2FULL,
    VO_CSC_RGB2YUV_2020_FULL2FULL,
    VO_CSC_RGB2YUV_601_LIMIT2LIMIT,
    VO_CSC_RGB2YUV_709_LIMIT2LIMIT,
    VO_CSC_RGB2YUV_2020_LIMIT2LIMIT,

    VO_CSC_YUV2YUV_601_601_LIMIT2LIMIT = 24,
    VO_CSC_YUV2YUV_601_709_LIMIT2LIMIT,
    VO_CSC_YUV2YUV_601_2020_LIMIT2LIMIT,
    VO_CSC_YUV2YUV_709_601_LIMIT2LIMIT,
    VO_CSC_YUV2YUV_709_709_LIMIT2LIMIT,
    VO_CSC_YUV2YUV_709_2020_LIMIT2LIMIT,
    VO_CSC_YUV2YUV_2020_601_LIMIT2LIMIT,
    VO_CSC_YUV2YUV_2020_709_LIMIT2LIMIT,
    VO_CSC_YUV2YUV_2020_2020_LIMIT2LIMIT,

    VO_CSC_YUV2YUV_601_601_FULL2FULL = 33,
    VO_CSC_YUV2YUV_601_709_FULL2FULL,
    VO_CSC_YUV2YUV_601_2020_FULL2FULL,
    VO_CSC_YUV2YUV_709_601_FULL2FULL,
    VO_CSC_YUV2YUV_709_709_FULL2FULL,
    VO_CSC_YUV2YUV_709_2020_FULL2FULL,
    VO_CSC_YUV2YUV_2020_601_FULL2FULL,
    VO_CSC_YUV2YUV_2020_709_FULL2FULL,
    VO_CSC_YUV2YUV_2020_2020_FULL2FULL,

    VO_CSC_YUV2YUV_601_601_LIMIT2FULL = 42,
    VO_CSC_YUV2YUV_601_709_LIMIT2FULL,
    VO_CSC_YUV2YUV_601_2020_LIMIT2FULL,
    VO_CSC_YUV2YUV_709_601_LIMIT2FULL,
    VO_CSC_YUV2YUV_709_709_LIMIT2FULL,
    VO_CSC_YUV2YUV_709_2020_LIMIT2FULL,
    VO_CSC_YUV2YUV_2020_601_LIMIT2FULL,
    VO_CSC_YUV2YUV_2020_709_LIMIT2FULL,
    VO_CSC_YUV2YUV_2020_2020_LIMIT2FULL,

    VO_CSC_YUV2YUV_601_601_FULL2LIMIT = 51,
    VO_CSC_YUV2YUV_601_709_FULL2LIMIT,
    VO_CSC_YUV2YUV_601_2020_FULL2LIMIT,
    VO_CSC_YUV2YUV_709_601_FULL2LIMIT,
    VO_CSC_YUV2YUV_709_709_FULL2LIMIT,
    VO_CSC_YUV2YUV_709_2020_FULL2LIMIT,
    VO_CSC_YUV2YUV_2020_601_FULL2LIMIT,
    VO_CSC_YUV2YUV_2020_709_FULL2LIMIT,
    VO_CSC_YUV2YUV_2020_2020_FULL2LIMIT,

    VO_CSC_RGB2RGB_LIMIT2LIMIT = 60,
    VO_CSC_RGB2RGB_FULL2FULL,
    VO_CSC_RGB2RGB_FULL2LIMIT,
    VO_CSC_RGB2RGB_LIMIT2FULL,

    VO_CSC_MAX
} vo_csc_type;
#endif

typedef struct {
    vo_csc_type csc_type;
    xmedia_u32 luma;
    xmedia_u32 contrast;
    xmedia_u32 hue;
    xmedia_u32 saturation;
} vo_pic_info;

typedef struct {
    xmedia_u64 lum_addr;
    xmedia_u64 chm_addr;
    xmedia_u32 lum_stride;
    xmedia_u32 chm_stride;
} vo_addr;

typedef struct {
    xmedia_bool config;
    xmedia_bool enable;
    xmedia_u32 chn;                 //硬件region区域 所对应的软件chn号
    xmedia_video_rect src_rect;
    xmedia_video_rect dst_rect;
    xmedia_video_rect lba_rect;
    xmedia_u32 lba_color;
    vo_addr region_addr;
} vo_region_info;

typedef struct {
    xmedia_bool mute_en;
    xmedia_video_rect frame_rect;   //真实内容 相对于显示窗口的rect
    xmedia_video_rect video_rect;   //层        相对于显示窗口的rect
    xmedia_video_rect lba_rect;
    xmedia_u32 lba_color;
    vo_addr frame_addr;
} vo_layer_single_cfg;

typedef struct {
    xmedia_video_rect reso_rect;
    xmedia_video_rect disp_rect;
    xmedia_u32 region_id;
    xmedia_bool region_overlap;
    xmedia_u32 overlap_id;
    xmedia_u32 region_sort;
    vo_region_info region[VO_MAX_CHN_NUM]; //硬件region区域配置
} vo_layer_multi_cfg;

typedef struct {
    xmedia_bool bt656_intf_en;
    xmedia_bool bt1120_intf_en;
    xmedia_bool lcd_intf_en;
    xmedia_bool mipi_intf_en;
    xmedia_bool lvds_intf_en;
    xmedia_vo_dev dev;
} vo_dev_intf_type_mux;

#ifdef __cplusplus
}
#endif

#endif
