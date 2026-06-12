/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __HAL_VO_DISP_H__
#define __HAL_VO_DISP_H__

#include "hal_vo_csc.h"
#include "drv_vo_comm.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    VO_MRG_MIX_PRIO_RDMA0 = 0,
    VO_MRG_MIX_PRIO_RDMA1,
    VO_MRG_MIX_PRIO_MAX,
} vo_mrg_mix_prio;

typedef enum {
    VO_MIX_PRIO_VIDEO = 0,
    VO_MIX_PRIO_GFX,
    VO_MIX_PRIO_MAX,
} vo_mix_prio;

typedef enum {
    VO_PIXEL_FMT_YUV_400 = 0,
    VO_PIXEL_FMT_YUV_420,
    VO_PIXEL_FMT_YUV_422,
    VO_PIXEL_FMT_YUV_MAX
} vo_pixel_format;

typedef struct {
    xmedia_bool vcus_mode;
    xmedia_bool hcus_mode;
    xmedia_u8 hcus_coef0;
    xmedia_u8 hcus_coef1;
    xmedia_u8 hcus_coef2;
    xmedia_u8 hcus_coef3;
    xmedia_u8 vcus_coef0;
    xmedia_u8 vcus_coef1;
    xmedia_u8 vcus_coef2;
    xmedia_u8 vcus_coef3;
} vo_video_cus;

xmedia_void hal_vo_set_top_regup(xmedia_bool enable);
xmedia_void hal_vo_get_top_regup(xmedia_bool *enable);
xmedia_void hal_vo_get_video_regup(xmedia_vo_layer layer, xmedia_bool *enable);
xmedia_void hal_vo_get_intf_regup(xmedia_vo_dev dev, xmedia_bool *enable);
xmedia_void hal_vo_get_version0(xmedia_u32 *version);
xmedia_void hal_vo_get_version1(xmedia_u32 *version);
xmedia_void hal_vo_set_mem_ctrl(xmedia_u32 speed);
xmedia_u32 hal_vo_get_mask_interrupt_state(xmedia_void);
xmedia_void hal_vo_clear_mask_interrupt_state(vo_isr_interrupt_type int_mask);
xmedia_void hal_vo_set_mask_interrupt_enable(vo_isr_interrupt_type int_mask,xmedia_bool enable);
xmedia_u32 hal_vo_get_unmask_interrupt_state(xmedia_void);
xmedia_void hal_vo_clear_lowband_state(xmedia_bool enable);
xmedia_bool hal_vo_disp_get_dma_busy_state(xmedia_void);
xmedia_void hal_vo_set_cbm_mixer_prio(xmedia_vo_dev dev, vo_mix_prio prio_id);
xmedia_void hal_vo_set_cbm_bgcolor(xmedia_vo_dev dev, xmedia_u32 bgcolor);
xmedia_void hal_vo_get_cbm_bgcolor(xmedia_vo_dev dev, xmedia_u32 *bgcolor);
xmedia_void hal_vo_set_dma_standing(xmedia_vo_dev dev, xmedia_u8 value);
xmedia_u8 hal_vo_get_dma_standing(xmedia_void);
xmedia_void hal_vo_set_enable(xmedia_vo_layer layer, xmedia_bool enable);
xmedia_void hal_vo_get_enable(xmedia_vo_layer layer, xmedia_bool *enable);
//两个及时寄存器
xmedia_void hal_vo_set_regup_mode(xmedia_vo_layer layer, xmedia_bool filed_en);
xmedia_void hal_vo_set_regup_field(xmedia_vo_layer layer, xmedia_bool bottom_en);
xmedia_void hal_vo_set_alpha(xmedia_vo_layer layer, xmedia_u8 alpha);
xmedia_void hal_vo_set_alpha_enable(xmedia_vo_layer layer, xmedia_bool alpha_en);
xmedia_void hal_vo_set_alpha_blend_mode(xmedia_vo_layer layer, xmedia_bool alpha_blend_en);
xmedia_void hal_vo_set_src_pixel_format(xmedia_vo_layer layer, vo_pixel_format format, xmedia_bool uv_order);
xmedia_void hal_vo_set_multi_enable(xmedia_vo_layer layer, xmedia_bool enable);
xmedia_void hal_vo_get_multi_enable(xmedia_vo_layer layer, xmedia_bool *enable);
xmedia_void hal_vo_set_bgcolor(xmedia_vo_layer layer, xmedia_u32 bgcolor);
xmedia_void hal_vo_get_bgcolor(xmedia_vo_layer layer, xmedia_u32 *bgcolor);
xmedia_void hal_vo_set_mute_enable(xmedia_vo_layer layer, xmedia_bool enable);
xmedia_void hal_vo_get_mute_enable(xmedia_vo_layer layer, xmedia_bool *enable);
xmedia_void hal_vo_set_video_regup_enable(xmedia_vo_layer layer, xmedia_bool enable);
xmedia_void hal_vo_set_mute_color(xmedia_vo_layer layer, xmedia_u32 bgcolor);
xmedia_void hal_vo_set_mute_pattern_mode(xmedia_vo_layer layer, xmedia_bool value);
xmedia_void hal_vo_set_mute_data_mode(xmedia_vo_layer layer,  xmedia_bool value);
xmedia_void hal_vo_set_checkbar_size(xmedia_vo_layer layer, xmedia_u8 width, xmedia_u8 height);
xmedia_void hal_vo_set_disp_rect     (xmedia_vo_layer layer, xmedia_video_rect *disp_rect);
xmedia_void hal_vo_get_disp_rect     (xmedia_vo_layer layer, xmedia_video_rect *disp_rect);
xmedia_void hal_vo_set_src_rect     (xmedia_vo_layer layer, xmedia_video_rect *src_rect);
xmedia_void hal_vo_get_src_rect     (xmedia_vo_layer layer, xmedia_video_rect *src_rect);
xmedia_void hal_vo_set_addr     (xmedia_vo_layer layer, vo_addr *addr);
xmedia_void hal_vo_set_mrg_mixer_prio(xmedia_vo_layer layer, vo_mrg_mix_prio prio_id);
xmedia_void hal_vo_set_mrg_rdma1_en(xmedia_vo_layer layer, xmedia_bool enable);
xmedia_void hal_vo_set_mrg_enable(xmedia_vo_layer layer,   xmedia_u32 region_id, xmedia_bool enable);
xmedia_void hal_vo_set_mrg_mute_enable(xmedia_vo_layer layer,   xmedia_u32 region_id, xmedia_bool enable);
xmedia_void hal_vo_get_mrg_enable(xmedia_vo_layer layer, xmedia_u32 region_id, xmedia_bool *enable);
xmedia_void hal_vo_set_mrg_sort(xmedia_vo_layer layer, xmedia_u16 sort);
xmedia_void hal_vo_set_mrg_src_rect     (xmedia_vo_layer layer,   xmedia_u32 region_id, xmedia_video_rect *src_rect);
xmedia_void hal_vo_set_mrg_dst_rect     (xmedia_vo_layer layer,   xmedia_u32 region_id, xmedia_video_rect *dst_rect);
xmedia_void hal_vo_set_mrg0_addr(xmedia_vo_layer layer,      vo_addr *addr);
xmedia_void hal_vo_set_mrg1_addr(xmedia_vo_layer layer,      vo_addr *addr);
xmedia_void hal_vo_set_mrg_addr(xmedia_vo_layer layer,       xmedia_u32 region_id,vo_addr *addr);
xmedia_void hal_vo_set_mrg_mute_color(xmedia_vo_layer layer,         xmedia_u32 region_id, xmedia_u32 bgcolor);
xmedia_void hal_vo_set_cus(xmedia_vo_layer layer, vo_video_cus *cus);
xmedia_void hal_vo_set_csc_en(xmedia_vo_layer layer, xmedia_bool enable);
xmedia_void hal_vo_set_mrg0_csc(xmedia_vo_layer layer,vo_csc_coef *csc);
xmedia_void hal_vo_set_mrg1_csc(xmedia_vo_layer layer,vo_csc_coef *csc);
xmedia_void hal_vo_set_mrg_csc(xmedia_vo_layer layer, xmedia_u32 region_id, vo_csc_coef *csc);
xmedia_void hal_vo_set_csc_mode(xmedia_vo_layer layer, xmedia_bool csc_mode);
xmedia_void hal_vo_set_lba_color(xmedia_vo_layer layer, xmedia_u32 region_id, xmedia_u32 color);
xmedia_void hal_vo_set_lba_rect(xmedia_vo_layer layer, xmedia_u32 region_id, xmedia_video_rect *lba_rect);

#ifdef __cplusplus
}
#endif

#endif
