/* Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __HAL_VO_H__
#define __HAL_VO_H__

#include "drv_vo_comm.h"
#include "hal_vo_chip_cfg.h"
#include "hal_vo_disp.h"
#include "hal_vo_intf.h"
#include "hal_vo_clock.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    VO_INT_STATE_TYPE_MASK = 0,
    VO_INT_STATE_TYPE_UNMASK,
    VO_INT_STATE_TYPE_MAX,
} vo_int_state_type;

typedef enum {
    VO_FIELD_FRAME = 0,
    VO_FIELD_TOP,
    VO_FIELD_BOTTOM,
    VO_FIELD_TOP_BOTTOM,
    VO_FIELD_MAX
} vo_field_mode;

typedef enum {
    VO_PATTERN_MODE_FULLCOLOR = 0,
    VO_PATTERN_MODE_CHECKER,
    VO_PATTERN_MODE_COLORBAR,
    VO_PATTERN_MODE_BLACKBOARD,
    VO_PATTERN_MODE_MAX
} vo_pattern_mode;

typedef enum {
    VO_PATTERN_DATA_MODE_DYNAMAIC = 0,
    VO_PATTERN_DATA_MODE_VIDEO,
    VO_PATTERN_DATA_MODE_MAX
} vo_pattern_data_mode;

typedef enum {
    VO_PATTERN_POSTION_VIDEO = 0,
    VO_PATTERN_POSTION_INTF,
    VO_PATTERN_POSTION_MAX
} vo_pattern_postion;

typedef struct {
    xmedia_vo_intf_sync intf_sync;
    xmedia_vo_user_sync_timing sync_timing;
    xmedia_bool clk_reverse_en;
} vo_intf_timing;

//定义固定分频相关属性
typedef struct {
    vo_intf_special_timing special_timing[XMEDIA_VO_DEV_MAX];//固定频点 和 实际需求频点 纠正得到的特殊时序
#ifdef xmfalcon
    mipi_phy_config mipi_phy_cfg;
#endif
    vo_clock_config clock_config;                            //hal层vo时钟配置
} vo_hal_context;

typedef struct {
    vo_pattern_postion pos;
    vo_pattern_mode pattern_mode;
    vo_pattern_data_mode data_mode;
    xmedia_bool pattern_en;
    xmedia_bool rgb;
    xmedia_u32 color;
    xmedia_u32 width;
    xmedia_u32 height;
} vo_pattern_info;

//初始化和去初始化
xmedia_s32 hal_vo_init(xmedia_void);
xmedia_void hal_vo_deinit(xmedia_void);
vo_base_cap* hal_vo_get_capacity(xmedia_void);
vo_layer_cap* hal_get_layer_capacity(xmedia_u32 layer);
vo_dev_cap* hal_get_dev_capacity(xmedia_vo_dev dev);
xmedia_void hal_vo_set_mipi_htotal_adjust_value(xmedia_s32 htotal_adjust_value);
xmedia_void hal_vo_set_mipi_vsync_adjust_value(xmedia_s32 vsync_adjust_value);

//时钟和寄存器生效
xmedia_void hal_vo_set_clock_gate_enable(xmedia_bool clk_en);
xmedia_void hal_vo_set_top_regup_enable(xmedia_bool enable);
xmedia_bool hal_vo_get_top_regup_enable(xmedia_void);
xmedia_void hal_vo_set_dev_intf_regup_enable(xmedia_vo_dev dev, xmedia_bool enable);
xmedia_bool hal_vo_get_dev_intf_regup_state(xmedia_vo_dev dev);
xmedia_void hal_vo_set_layer_regup_enable(xmedia_vo_layer layer, xmedia_bool enable);
xmedia_bool hal_vo_get_layer_regup_enable(xmedia_vo_layer layer);

// 中断相关:state表示中断的状态，比如vtt1,vvt2
xmedia_void hal_vo_get_interrupt_state(vo_int_state_type type, xmedia_u32 *state);
xmedia_void hal_vo_clean_interrupt_state(vo_int_state_type type, xmedia_u32 state);
xmedia_void hal_vo_set_interrupt_enable(vo_isr_interrupt_type int_type, xmedia_bool enable);
xmedia_void hal_vo_set_int_mode(xmedia_vo_dev dev, xmedia_u32 vtt_id, vo_isr_field_flag int_mode);
xmedia_void hal_vo_set_dev_thd(xmedia_vo_dev dev, xmedia_u32 vtt_id, xmedia_u32 thd);
xmedia_void hal_vo_get_dev_state(xmedia_vo_dev dev, xmedia_bool *btm, xmedia_u16 *vcnt, xmedia_u8 *vstate);

xmedia_void hal_vo_set_dev_mixer_prio(xmedia_vo_dev dev, vo_mix_prio prio_id);
xmedia_void hal_vo_set_dev_bgcolor(xmedia_vo_dev dev, xmedia_u32 bgcolor);
xmedia_void hal_vo_get_dev_bgcolor(xmedia_vo_dev dev, xmedia_u32 *bgcolor);
xmedia_void hal_vo_set_dev_csc_en(xmedia_vo_dev dev, xmedia_bool csc_en);
xmedia_void hal_vo_get_dev_csc_en(xmedia_vo_dev dev, xmedia_bool *csc_en);
xmedia_s32 hal_vo_set_dev_csc_coef(xmedia_vo_dev dev, vo_pic_info* pic_info);

//设备接口时序包含相关时钟配置
xmedia_void hal_vo_set_crg_clock_reset(xmedia_bool reset_en);
xmedia_bool hal_vo_get_crg_clock_reset_state(xmedia_void);
xmedia_s32 hal_vo_set_crg_clock(xmedia_vo_dev dev, xmedia_u32 clk_hz, xmedia_bool clk_reverse_en,
    xmedia_intf_type intf_type);
xmedia_void hal_vo_set_crg_clock_enable(xmedia_vo_dev dev, xmedia_intf_type intf_type, xmedia_bool clk_en);
xmedia_void hal_vo_set_crg_intf_detach_dev(xmedia_vo_dev dev, xmedia_intf_type intf_type);
xmedia_void hal_vo_set_dev_intf_timing(xmedia_vo_dev dev, xmedia_u32 frame_rate,
    xmedia_vo_intf_config *intf_config, xmedia_vo_user_sync_timing *sync_timing);
xmedia_void hal_vo_set_clock_sel_priority(xmedia_vo_dev dev, vo_clock_priority_sel vo_lcd_divider_priority);
vo_hal_context* hal_vo_get_hal_context(xmedia_void);

xmedia_void hal_vo_set_dev_intf_enable(xmedia_vo_dev dev, xmedia_bool enable);
xmedia_void hal_vo_get_dev_intf_enable(xmedia_vo_dev dev, xmedia_bool *enable);
xmedia_s32 hal_vo_enable_dev_phy_cfg(xmedia_vo_dev dev, xmedia_u32 clk_hz, xmedia_u32 frame_rate,
                                            xmedia_vo_user_sync_timing *sync_timing, xmedia_intf_type intf_type);
xmedia_void hal_vo_disable_dev_phy_cfg(xmedia_intf_type intf_type);
xmedia_s32 vo_calc_dev_phy_clk_cfg(xmedia_vo_dev dev, xmedia_u32 clk_hz, xmedia_vo_intf_config *intf_config);

//视频层相关配置
xmedia_void hal_vo_set_layer_enable(xmedia_vo_layer layer, xmedia_bool enable);
xmedia_void hal_vo_get_layer_enable(xmedia_vo_layer layer, xmedia_bool *enable);
xmedia_void hal_vo_set_layer_bg_color(xmedia_vo_layer layer, xmedia_u32 bgcolor);
xmedia_void hal_vo_set_layer_mute_enable(xmedia_vo_layer layer, xmedia_bool enable);
xmedia_void hal_vo_set_layer_mute_color(xmedia_vo_layer layer, xmedia_u32 bgcolor);
xmedia_void hal_vo_set_layer_csc_en(xmedia_vo_layer layer, xmedia_bool csc_en);
xmedia_s32 hal_vo_set_layer_csc_coef(xmedia_vo_layer layer, vo_pic_info* pic_info);

//设置输出的宽和高
xmedia_void hal_vo_set_layer_disp_rect(xmedia_vo_layer layer, xmedia_video_rect *disp_rect);
xmedia_void hal_vo_get_layer_disp_rect(xmedia_vo_layer layer, xmedia_video_rect *disp_rect);
xmedia_void hal_vo_set_layer_reso_rect(xmedia_vo_layer layer, xmedia_video_rect *reso_rect);
xmedia_void hal_vo_get_layer_reso_rect(xmedia_vo_layer layer, xmedia_video_rect *reso_rect);
xmedia_void hal_vo_set_layer_pixel_format(xmedia_vo_layer layer, xmedia_video_pixel_format pix_format);

//单区域和多区域控制
xmedia_void hal_vo_set_layer_multi_enable(xmedia_vo_layer layer, xmedia_bool enable);
xmedia_void hal_vo_get_layer_multi_enable(xmedia_vo_layer layer, xmedia_bool *enable);
xmedia_void hal_vo_set_layer_region_enable(xmedia_vo_layer layer, xmedia_u32 region_id, xmedia_bool enable);
xmedia_void hal_vo_get_layer_region_enable(xmedia_vo_layer layer, xmedia_u32 region_id, xmedia_bool *enable);
xmedia_void hal_vo_set_layer_region_mrg_enable(xmedia_vo_layer layer, xmedia_u32 region_id,
    xmedia_bool enable);
xmedia_void hal_vo_set_layer_region_mute_color(xmedia_vo_layer layer, xmedia_u32 region_id,
    xmedia_u32 bgcolor);
xmedia_void hal_vo_set_layer_single_region_cfg(xmedia_vo_layer layer, vo_layer_single_cfg *single_cfg);
xmedia_void hal_vo_set_layer_multi_region_cfg(xmedia_vo_layer layer, vo_layer_multi_cfg *multi_cfg);

xmedia_void hal_vo_set_dev_debug_pattern(xmedia_vo_dev dev, vo_pattern_info *pattern_info);
xmedia_void hal_vo_set_layer_debug_pattern(xmedia_vo_layer layer, vo_pattern_info *pattern_info);
xmedia_void hal_vo_set_debug_ink(xmedia_vo_dev dev, vo_csc_ink_info *ink_info);
xmedia_void hal_vo_set_debug_mask(xmedia_vo_dev dev, vo_mix_prio mix_pro);
xmedia_void hal_vo_get_debug_checksum(xmedia_vo_dev dev, xmedia_u32 *y, xmedia_u32 *u, xmedia_u32 *v);

xmedia_void hal_vo_set_gamma_addr(xmedia_vo_dev dev);
xmedia_void hal_vo_set_gamma_enable(xmedia_vo_dev dev, xmedia_bool enable);
xmedia_void hal_vo_set_gamma_table(xmedia_vo_dev dev, xmedia_vo_gamma_table *gamma_table);
xmedia_void hal_vo_get_gamma_table(xmedia_vo_dev dev, xmedia_vo_gamma_table *gamma_table);
xmedia_s32 hal_vo_gamma_init(xmedia_vo_dev dev);
xmedia_s32 hal_vo_gamma_deinit(xmedia_vo_dev dev);

xmedia_s32 hal_vo_reg_record(xmedia_void);
xmedia_s32 hal_vo_reg_restore(xmedia_void);
xmedia_u32 hal_vo_get_cyc_pre_pixel(xmedia_vo_intf_config *intf_config);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_VO_H__ */
