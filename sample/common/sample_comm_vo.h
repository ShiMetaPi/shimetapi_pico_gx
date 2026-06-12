/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __SAMPLE_COMM_VO_H__
#define __SAMPLE_COMM_VO_H__

#include <pthread.h>

#include "common.h"
#include "defines.h"
#include "xmedia_sys.h"
#include "xmedia_vb.h"
#include "xmedia_vo.h"
#include "xmedia_type.h"
#include "xmedia_mmz.h"
#include "math_fun.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct{
    xmedia_bool enable;
    xmedia_vo_low_delay_mode mode;
}sample_vo_lowdelay_attr;

typedef struct{
    xmedia_vo_intf_sync intf_sync;
    xmedia_intf_type intf_type;
    xmedia_s32 square_sort;
    xmedia_s32 screen_type;
}sample_vo_config_input;

typedef struct{
    xmedia_vo_dev_config dev_config;
    xmedia_vo_layer_config layer_config;
    xmedia_vo_chn_attr chn_attr[VO_MAX_CHN_NUM];
    xmedia_vo_dev_csc dev_csc;
    sample_vo_lowdelay_attr lowdelay_attr;
}sample_vo_config;

typedef enum {
    MICROTECH_MTF050_LCD_800X480 = 0,
    SATOZ_SAT935_MIPI_800X1280_RGB8888_24BIT,
    SIL9024_BT1120_1920X1080P60,
    MICROTECH_MTF101_FT01_LVDS_800X1280_RGB666_18BIT,
    MICROTECH_MTF101_FS16_LVDS_1280X800_RGB888_24BIT,
    C3200B_EVB_BT656_1280X720_UYVY422,
    DEMO_LVDS_1920X720_RGB888_24BIT,
    MIPI_TO_HDMI_1920X1080_RGB888_24BIT,
    MTF024QV33B_SLCD_240X320_18BIT,
    VIRTUAL_SCREEN_MIPI_3840X2160P30,
    MIPI_TO_HDMI_3840X2160_RGB888_24BIT = 10,
    HX_M090P24349A_LVDS_VESA_1920X384_RGB888_24BIT,
    MCU_KD026_320X240P6_4LINE_RGB565,
    MCU_KD035_320X480P6_3LINE_RGB666_OPTION1,
    MCU_KD026_320X240P60_18BIT_18BPP_RGB666_8080I,
    SIL9024_BT1120_1920X1080P30,
    SIL9024_BT1120_1280X720P60,
    SIL9024_BT656_1280X720P30_UYVY422,
    SAMPLE_COMM_SCREEN_TYPE_INVALID,
} sample_comm_screen_type;

xmedia_s32 sample_comm_vo_get_dev_config(xmedia_s32 screen_type, sample_vo_config *vo_config, xmedia_u32 *pst_wdith,
                                            xmedia_u32 *pst_height,xmedia_u32 *pst_freq);

xmedia_s32 sample_comm_vo_get_default_config(sample_vo_config_input config_in, sample_vo_config *vo_config);

xmedia_s32 sample_comm_vo_start(xmedia_vo_dev dev, xmedia_vo_layer layer, xmedia_vo_chn chn[],xmedia_s32 chn_num,
                                    xmedia_bool buf_len_set,sample_vo_config *vo_config);

xmedia_s32 sample_comm_vo_stop(xmedia_vo_dev dev, xmedia_vo_layer layer, xmedia_vo_chn chn[], xmedia_s32 chn_num);

xmedia_s32 sample_comm_vo_init(xmedia_void);

xmedia_s32 sample_comm_vo_exit(xmedia_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* End of #ifndef __SAMPLE_COMM_VO_H__ */


