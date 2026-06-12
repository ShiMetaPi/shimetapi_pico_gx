/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __HAL_VO_INTF_H__
#define __HAL_VO_INTF_H__

#include "hal_vo_csc.h"
#include "hal_vo_clock.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
    VO_INTF_HS_MODE_RF = 0, //vsyn对应的上升沿和下降沿都有效
    VO_INTF_HS_MODE_RF_NONE, //vsyn对应的上升沿和下降沿都无效
    VO_INTF_HS_MODE_FALLING,
    VO_INTF_HS_MODE_RISING,
    VO_INTF_HS_MODE_MAX,
}vo_intf_hs_mode;

typedef struct{
    xmedia_bool hcds_en;
    xmedia_bool hcds_mode;
    xmedia_u8 hcds_coef_0;
    xmedia_u8 hcds_coef_1;
    xmedia_u8 hcds_coef_2;
    xmedia_u8 hcds_coef_3;
}vo_intf_hcds;

typedef struct{
    xmedia_bool clip_en;
    xmedia_u32 clip_l;
    xmedia_u32 clip_h;
}vo_intf_clip;

typedef struct{
    xmedia_u16 top_start;
    xmedia_u16 top_end;
    xmedia_u16 bottom_start;
    xmedia_u16 bottom_end;
}vo_intf_bt_sync;

typedef struct {
    xmedia_bool ink_en;
    xmedia_bool cross_en;
    xmedia_bool color_reverse;
    xmedia_u32 color;
    xmedia_u32 x;
    xmedia_u32 y;
} vo_csc_ink_info;

typedef struct {
    vo_clock_priority_sel vo_clock_priority_sel;  //debug 接口 1：lcd分频器优先，0：固定频点优先
    xmedia_bool vo_special_timing_sel;    //true表示使用固定分频，需要配置特殊时序,否则使用默认时序
    xmedia_u32 fixed_clk_htotal;
    xmedia_u32 fixed_clk_hbb;
    xmedia_u32 fixed_clk_hfb;
    xmedia_u32 vs_st_v_filed;
    xmedia_u32 vs_end_v_filed;
} vo_intf_special_timing;

typedef enum{
    VO_DTS_LCD_TYPE_MIPI = 0,
    VO_DTS_LCD_TYPE_MCU,
    VO_DTS_LCD_TYPE_MAX,
}vo_dts_lcd_type;

typedef struct {
    xmedia_s32 size;
    xmedia_u8 *array;
} vo_dts_lcd_config;

xmedia_void hal_vo_set_intf_fifo(xmedia_vo_dev dev, xmedia_u32 fifo);
xmedia_void hal_vo_set_intf_enable(xmedia_vo_dev dev, xmedia_bool enable);
xmedia_void hal_vo_get_intf_enable(xmedia_vo_dev dev, xmedia_bool *enable);
xmedia_void hal_vo_set_intf_chk_sum_enable(xmedia_vo_dev dev, xmedia_bool enable);
xmedia_void hal_vo_set_intf_hs_mode(xmedia_vo_dev dev, vo_intf_hs_mode hs_mode);
xmedia_void hal_vo_set_intf_mux(xmedia_vo_dev dev, xmedia_intf_type intf);
xmedia_void hal_vo_set_intf_regup(xmedia_vo_dev dev, xmedia_bool enable);
xmedia_void hal_vo_set_intf_cnt_pos(xmedia_vo_dev dev);
xmedia_void hal_vo_set_intf_timing(xmedia_vo_dev dev, xmedia_u32 frame_rate,
    xmedia_vo_intf_config *intf_config, xmedia_vo_user_sync_timing *sync_timing, vo_intf_special_timing *special_timing);
xmedia_void hal_vo_set_intf_sync_inverse(xmedia_vo_dev dev);
xmedia_void hal_vo_set_intf_vtt_sel(xmedia_vo_dev dev, xmedia_u32 vtt_id, xmedia_bool cnt_mode);
xmedia_void hal_vo_set_intf_vtt_mode(xmedia_vo_dev dev, xmedia_u32 vtt_id, xmedia_bool frame_mode);
xmedia_void hal_vo_set_intf_vtt_thd(xmedia_vo_dev dev, xmedia_u32 vtt_id, xmedia_u32 vtt_thd);
xmedia_void hal_vo_get_intf_state(xmedia_vo_dev dev, xmedia_bool *btm, xmedia_u16 *vcnt, xmedia_u8 *vstate);
xmedia_void hal_vo_get_intf_checksum_value(xmedia_vo_dev dev, xmedia_u32 *y, xmedia_u32 *u, xmedia_u32 *v);
xmedia_void hal_vo_set_intf_bt_mode(xmedia_vo_dev dev, xmedia_bool value);
xmedia_void hal_vo_set_intf_bt_yc_order(xmedia_vo_dev dev, xmedia_bool value);
xmedia_void hal_vo_set_intf_bt_uv_order(xmedia_vo_dev dev, xmedia_bool value);
xmedia_void hal_vo_set_intf_bt_vbit_mode(xmedia_vo_dev dev, xmedia_bool sync_mode);
xmedia_void hal_vo_set_intf_bt_bit_inverse(xmedia_vo_dev dev, xmedia_bool enable);
xmedia_void hal_vo_set_intf_bt_user_sync(xmedia_vo_dev dev, vo_intf_bt_sync *sync);
xmedia_void hal_vo_set_intf_lcd_mode(xmedia_vo_dev dev, xmedia_bool value);
xmedia_void hal_vo_set_intf_lcd_rgb_order(xmedia_vo_dev dev, xmedia_u8 order0, xmedia_u8 order1);
xmedia_void hal_vo_set_intf_lcd_bit_inverse(xmedia_vo_dev dev, xmedia_bool enable);
xmedia_void hal_vo_set_intf_mute_enable(xmedia_vo_dev dev, xmedia_bool enable);
xmedia_void hal_vo_get_intf_mute_enable(xmedia_vo_dev dev, xmedia_bool *enable);
xmedia_void hal_vo_set_intf_mute_pattern_mode(xmedia_vo_dev dev, xmedia_u8 value);
xmedia_void hal_vo_set_intf_mute_data_mode(xmedia_vo_dev dev, xmedia_u8 value);
xmedia_void hal_vo_set_intf_checkbar_size(xmedia_vo_dev dev, xmedia_u8 width, xmedia_u8 height);
xmedia_void hal_vo_set_intf_cbar_sel(xmedia_vo_dev dev, xmedia_u8 value);
xmedia_void hal_vo_set_intf_mute_color(xmedia_vo_dev dev, xmedia_u32 white_color, xmedia_u32 black_color);
xmedia_void hal_vo_set_intf_ink(xmedia_vo_dev dev, vo_csc_ink_info *ink_info);
xmedia_void hal_vo_set_intf_clip(xmedia_vo_dev dev, vo_intf_clip *intf_clip);
xmedia_void hal_vo_set_intf_csc_en(xmedia_vo_dev dev, xmedia_bool enable);
xmedia_void hal_vo_get_intf_csc_en(xmedia_vo_dev dev, xmedia_bool *enable);
xmedia_void hal_vo_set_intf_csc(xmedia_vo_dev dev, vo_csc_coef *intf_csc);
xmedia_void hal_vo_set_intf_hcds(xmedia_vo_dev dev, vo_intf_hcds *intf_hcds);
xmedia_void hal_vo_set_intf_dither_enable(xmedia_vo_dev dev, xmedia_bool enable);
xmedia_void hal_vo_set_intf_dither_thd(xmedia_vo_dev dev, xmedia_u8 thd_h, xmedia_u8 thd_l);
xmedia_void hal_vo_set_bt656_intf_cfg(xmedia_vo_dev dev, xmedia_vo_bt656_attr *bt656_attr);
xmedia_void hal_vo_set_bt1120_intf_cfg(xmedia_vo_dev dev, xmedia_vo_bt1120_attr *bt1120_attr);
xmedia_void hal_vo_set_lcd_intf_cfg(xmedia_vo_dev dev, xmedia_vo_lcd_attr *lcd_attr);
xmedia_void hal_vo_set_mcu_intf_cfg(xmedia_vo_dev dev, xmedia_vo_mcu_attr *mcu_attr);

#ifdef __cplusplus
}
#endif

#endif
