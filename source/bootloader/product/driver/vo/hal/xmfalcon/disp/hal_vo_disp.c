/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include "hal_vo_disp.h"
#include "hal_vo_reg.h"

extern s_vout_regs_type *g_p_vout_reg;

xmedia_void hal_vo_set_top_regup(xmedia_bool enable)
{
    hal_vout_set_cfg_regup_en(g_p_vout_reg, enable);
    return;
}

xmedia_void hal_vo_get_top_regup(xmedia_bool *enable)
{
    *enable = hal_vout_get_top_regup_sta(g_p_vout_reg);
     return;
}

xmedia_void hal_vo_get_video_regup(xmedia_vo_layer layer, xmedia_bool *enable)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        *enable = hal_vout_get_v0_regup_sta(g_p_vout_reg);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        *enable = hal_vout_get_v1_regup_sta(g_p_vout_reg);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_get_intf_regup(xmedia_vo_dev dev, xmedia_bool *state)
{
    if (dev == XMEDIA_VO_DEV_0) {
        *state = hal_vout_get_intf_regup_sta(g_p_vout_reg);
    } else if (dev == XMEDIA_VO_DEV_1) {
        *state = hal_vout_get_intf1_regup_sta(g_p_vout_reg);
    } else {
        ;
    }
    return;
}

xmedia_void  hal_vo_get_version0(xmedia_u32 *version)
{
    *version = hal_vout_get_vout_ver_0(g_p_vout_reg);
    return;
}

xmedia_void  hal_vo_get_version1(xmedia_u32 *version)
{
    *version = hal_vout_get_vout_ver_1(g_p_vout_reg);
    return;
}

xmedia_void  hal_vo_set_mem_ctrl(xmedia_u32 speed)
{
    hal_vout_set_cfg_mem_ctrl(g_p_vout_reg, speed);
    return;
}

xmedia_u32 hal_vo_get_mask_interrupt_state(xmedia_void)
{
    xmedia_u32 mask_state;
    mask_state = hal_vout_reg_read((xmedia_uintptr_t)&(g_p_vout_reg->vout_int_sta.u32));
    return mask_state;
}

xmedia_void hal_vo_clear_mask_interrupt_state(vo_isr_interrupt_type int_mask)
{
    if (int_mask & VO_ISR_INTERRUPT_TYPE_DHD0_VTT0) {
        hal_vout_set_cfg_intf_vt_int_0_clr(g_p_vout_reg, XMEDIA_TRUE);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_DHD0_VTT1) {
        hal_vout_set_cfg_intf_vt_int_1_clr(g_p_vout_reg, XMEDIA_TRUE);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_DHD0_VTT2) {
        hal_vout_set_cfg_intf_vt_int_2_clr(g_p_vout_reg, XMEDIA_TRUE);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_DHD0_VTT3) {
        hal_vout_set_cfg_intf_vt_int_3_clr(g_p_vout_reg, XMEDIA_TRUE);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_DHD1_VTT0) {
        hal_vout_set_cfg_intf1_vt_int_0_clr(g_p_vout_reg, XMEDIA_TRUE);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_DHD1_VTT1) {
        hal_vout_set_cfg_intf1_vt_int_1_clr(g_p_vout_reg, XMEDIA_TRUE);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_DHD1_VTT2) {
        hal_vout_set_cfg_intf1_vt_int_2_clr(g_p_vout_reg, XMEDIA_TRUE);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_DHD1_VTT3) {
        hal_vout_set_cfg_intf1_vt_int_3_clr(g_p_vout_reg, XMEDIA_TRUE);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_DHD0_UFINT) {
        hal_vout_set_cfg_intf_underflow_clr(g_p_vout_reg, XMEDIA_TRUE);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_DHD1_UFINT) {
        hal_vout_set_cfg_intf1_underflow_clr(g_p_vout_reg, XMEDIA_TRUE);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_LOW_DELAY_CERR) {
        hal_vout_set_cfg_low_delay_c_err_clr(g_p_vout_reg, XMEDIA_TRUE);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_LOW_DELAY_YERR) {
        hal_vout_set_cfg_low_delay_y_err_clr(g_p_vout_reg, XMEDIA_TRUE);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_BUSR_ERR) {
        hal_vout_set_cfg_axi_bus_rerr_clr(g_p_vout_reg, XMEDIA_TRUE);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_BUSW_ERR) {
        hal_vout_set_cfg_axi_bus_werr_clr(g_p_vout_reg, XMEDIA_TRUE);
    }

    return;
}

xmedia_void hal_vo_set_mask_interrupt_enable(vo_isr_interrupt_type int_mask,xmedia_bool enable)
{
    if (int_mask & VO_ISR_INTERRUPT_TYPE_DHD0_VTT0) {
        hal_vout_set_cfg_intf_vt_int_0_msk(g_p_vout_reg, enable);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_DHD0_VTT1) {
        hal_vout_set_cfg_intf_vt_int_1_msk(g_p_vout_reg, enable);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_DHD0_VTT2) {
        hal_vout_set_cfg_intf_vt_int_2_msk(g_p_vout_reg, enable);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_DHD0_VTT3) {
        hal_vout_set_cfg_intf_vt_int_3_msk(g_p_vout_reg, enable);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_DHD1_VTT0) {
        hal_vout_set_cfg_intf1_vt_int_0_msk(g_p_vout_reg, enable);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_DHD1_VTT1) {
        hal_vout_set_cfg_intf1_vt_int_1_msk(g_p_vout_reg, enable);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_DHD1_VTT2) {
        hal_vout_set_cfg_intf1_vt_int_2_msk(g_p_vout_reg, enable);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_DHD1_VTT3) {
        hal_vout_set_cfg_intf1_vt_int_3_msk(g_p_vout_reg, enable);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_DHD0_UFINT) {
        hal_vout_set_cfg_intf_underflow_msk(g_p_vout_reg, enable);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_DHD1_UFINT) {
        hal_vout_set_cfg_intf1_underflow_msk(g_p_vout_reg, enable);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_LOW_DELAY_CERR) {
        hal_vout_set_cfg_low_delay_c_err_msk(g_p_vout_reg, enable);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_LOW_DELAY_YERR) {
        hal_vout_set_cfg_low_delay_y_err_msk(g_p_vout_reg, enable);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_BUSR_ERR) {
        hal_vout_set_cfg_axi_bus_rerr_msk(g_p_vout_reg, enable);
    }
    if (int_mask & VO_ISR_INTERRUPT_TYPE_BUSW_ERR) {
        hal_vout_set_cfg_axi_bus_werr_msk(g_p_vout_reg, enable);
    }

    return;
}

xmedia_u32 hal_vo_get_unmask_interrupt_state(xmedia_void)
{
    xmedia_u32 unmask_state;
    unmask_state = hal_vout_reg_read((xmedia_uintptr_t)&(g_p_vout_reg->vout_int_src_sta.u32));
    return unmask_state;
}

xmedia_void hal_vo_clear_lowband_state(xmedia_bool enable)
{
    hal_vout_set_cfg_uf_clr(g_p_vout_reg, enable);
    return;
}

xmedia_void hal_vo_set_cbm_mixer_prio(xmedia_vo_dev dev, vo_mix_prio prio_id)
{
    if (dev == XMEDIA_VO_DEV_1) {
        if (prio_id == VO_MIX_PRIO_VIDEO) {
            hal_vout_set_cfg_mixer1_prio0(g_p_vout_reg, 2); //0层图形
            hal_vout_set_cfg_mixer1_prio1(g_p_vout_reg, 1); //1层视频
        } else if (prio_id == VO_MIX_PRIO_GFX) {
            hal_vout_set_cfg_mixer1_prio0(g_p_vout_reg, 1); //0层视频
            hal_vout_set_cfg_mixer1_prio1(g_p_vout_reg, 2); //1层图形
        } else {
            hal_vout_set_cfg_mixer1_prio0(g_p_vout_reg, 0); //无驱动层
            hal_vout_set_cfg_mixer1_prio1(g_p_vout_reg, 0); //无驱动层
        }
    } else {
         if (prio_id == VO_MIX_PRIO_VIDEO) {
            hal_vout_set_cfg_mixer_prio0(g_p_vout_reg, 2); //0层图形
            hal_vout_set_cfg_mixer_prio1(g_p_vout_reg, 1); //1层视频
        } else if (prio_id == VO_MIX_PRIO_GFX) {
            hal_vout_set_cfg_mixer_prio0(g_p_vout_reg, 1); //0层视频
            hal_vout_set_cfg_mixer_prio1(g_p_vout_reg, 2); //1层图形
        } else {
            hal_vout_set_cfg_mixer_prio0(g_p_vout_reg, 0); //无驱动层
            hal_vout_set_cfg_mixer_prio1(g_p_vout_reg, 0); //无驱动层
        }
    }
    return;
}

xmedia_void hal_vo_set_cbm_bgcolor(xmedia_vo_dev dev, xmedia_u32 bgcolor)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_cfg_bk_color(g_p_vout_reg, bgcolor);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_cfg_bk1_color(g_p_vout_reg, bgcolor);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_get_cbm_bgcolor(xmedia_vo_dev dev, xmedia_u32 *bgcolor)
{
    if (dev == XMEDIA_VO_DEV_0) {
        *bgcolor = hal_vout_get_cfg_bk_color(g_p_vout_reg);
    } else if (dev == XMEDIA_VO_DEV_1) {
        *bgcolor = hal_vout_get_cfg_bk1_color(g_p_vout_reg);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_dma_standing(xmedia_vo_dev dev, xmedia_u8 value)
{
    hal_vout_set_cfg_axi_rd_ost_num(g_p_vout_reg, value);
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_cfg_rd_ost_num_v0_y(g_p_vout_reg, value);
        hal_vout_set_cfg_rd_ost_num_v0_c(g_p_vout_reg, value);
        hal_vout_set_cfg_rd_ost_num_v0_y_1(g_p_vout_reg, value);
        hal_vout_set_cfg_rd_ost_num_v0_c_1(g_p_vout_reg, value);
        hal_vout_set_cfg_rd_ost_num_g0(g_p_vout_reg, value);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_cfg_rd_ost_num_v1_y(g_p_vout_reg, value);
        hal_vout_set_cfg_rd_ost_num_v1_c(g_p_vout_reg, value);
        hal_vout_set_cfg_rd_ost_num_g1(g_p_vout_reg, value);
        hal_vout_set_cfg_rd_ost_num_v1_c_1(g_p_vout_reg, value);
        hal_vout_set_cfg_rd_ost_num_v1_y_1(g_p_vout_reg, value);
    } else {
        ;
    }
    return;
}

xmedia_u8 hal_vo_get_dma_standing(xmedia_void)
{
    return hal_vout_get_cfg_axi_rd_ost_num(g_p_vout_reg);
}

//v0 层的封装寄存器
xmedia_void hal_vo_set_enable(xmedia_vo_layer layer, xmedia_bool enable)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_layer_en(g_p_vout_reg, enable);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_1ctrl_cfg_layer_en(g_p_vout_reg, enable);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_get_enable(xmedia_vo_layer layer, xmedia_bool *enable)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        *enable = hal_vout_get_cfg_layer_en(g_p_vout_reg);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        *enable = hal_vout_get_1ctrl_cfg_layer_en(g_p_vout_reg);
    } else {
        ;
    }
    return;
}

//两个及时寄存器
xmedia_void hal_vo_set_regup_mode(xmedia_vo_layer layer, xmedia_bool filed_en)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_regup_mode(g_p_vout_reg, filed_en);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_1ctrl_cfg_regup_mode(g_p_vout_reg, filed_en);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_regup_field(xmedia_vo_layer layer, xmedia_bool bottom_en)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_regup_field(g_p_vout_reg, bottom_en);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_1ctrl_cfg_regup_field(g_p_vout_reg, bottom_en);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_alpha(xmedia_vo_layer layer, xmedia_u8 alpha)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_galpha(g_p_vout_reg, alpha);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_1ctrl_cfg_galpha(g_p_vout_reg, alpha);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_alpha_enable(xmedia_vo_layer layer, xmedia_bool alpha_en)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_galpha_en(g_p_vout_reg, alpha_en);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_1ctrl_cfg_galpha_en(g_p_vout_reg, alpha_en);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_alpha_blend_mode(xmedia_vo_layer layer, xmedia_bool alpha_blend_en)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_blend_mode(g_p_vout_reg, alpha_blend_en);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_1ctrl_cfg_blend_mode(g_p_vout_reg, alpha_blend_en);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_src_pixel_format(xmedia_vo_layer layer, vo_pixel_format format, xmedia_bool uv_order)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_src_fmt(g_p_vout_reg, format);
        hal_vout_set_cfg_src_uv_order(g_p_vout_reg, uv_order);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_1ctrl_cfg_src_fmt(g_p_vout_reg, format);
        hal_vout_set_1ctrl_cfg_src_uv_order(g_p_vout_reg, uv_order);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_multi_enable(xmedia_vo_layer layer, xmedia_bool enable)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_mrg_en(g_p_vout_reg, enable);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_1ctrl_cfg_mrg_en(g_p_vout_reg, enable);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_get_multi_enable(xmedia_vo_layer layer, xmedia_bool *enable)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        *enable = hal_vout_get_cfg_mrg_en(g_p_vout_reg);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        *enable = hal_vout_get_1ctrl_cfg_mrg_en(g_p_vout_reg);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_video_regup_enable(xmedia_vo_layer layer, xmedia_bool enable)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_v0upd_cfg_regup_en(g_p_vout_reg, enable);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_v1upd_cfg_regup_en(g_p_vout_reg, enable);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_bgcolor(xmedia_vo_layer layer, xmedia_u32 bgcolor)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_v0bk_cfg_bk_color(g_p_vout_reg, bgcolor);
        hal_vout_set_cfg_mute_color(g_p_vout_reg, bgcolor);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_v1bk_cfg_bk_color(g_p_vout_reg, bgcolor);
        hal_vout_set_voutv1_cfg_mute_color(g_p_vout_reg, bgcolor);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_get_bgcolor(xmedia_vo_layer layer, xmedia_u32 *bgcolor)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        *bgcolor = hal_vout_get_v0bk_cfg_bk_color(g_p_vout_reg);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        *bgcolor = hal_vout_get_v1bk_cfg_bk_color(g_p_vout_reg);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_mute_enable(xmedia_vo_layer layer, xmedia_bool enable)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_mute_en(g_p_vout_reg, enable);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_voutv1_cfg_mute_en(g_p_vout_reg, enable);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_get_mute_enable(xmedia_vo_layer layer, xmedia_bool *enable)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        *enable = hal_vout_get_cfg_mute_en(g_p_vout_reg);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        *enable = hal_vout_get_voutv1_cfg_mute_en(g_p_vout_reg);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_mute_color(xmedia_vo_layer layer, xmedia_u32 bgcolor)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_mute_color(g_p_vout_reg, bgcolor);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_voutv1_cfg_mute_color(g_p_vout_reg, bgcolor);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_mute_pattern_mode(xmedia_vo_layer layer, xmedia_bool value)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
       hal_vout_set_cfg_mute_pat(g_p_vout_reg, value);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_voutv1_cfg_mute_pat(g_p_vout_reg, value);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_mute_data_mode(xmedia_vo_layer layer,  xmedia_bool value)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
       hal_vout_set_cfg_mute_mode(g_p_vout_reg, value);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_voutv1_cfg_mute_mode(g_p_vout_reg, value);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_checkbar_size(xmedia_vo_layer layer, xmedia_u8 width, xmedia_u8 height)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_checker_h(g_p_vout_reg, width);
        hal_vout_set_cfg_checker_v(g_p_vout_reg, height);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_voutv1_cfg_checker_h(g_p_vout_reg, width);
        hal_vout_set_voutv1_cfg_checker_v(g_p_vout_reg, height);
    } else {
        ;
    }
    return;
}
/*
 * 层相对窗口 的rect
 */
xmedia_void  hal_vo_set_disp_rect     (xmedia_vo_layer layer, xmedia_video_rect *disp_rect)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_disp_xpos(g_p_vout_reg, disp_rect->x);
        hal_vout_set_cfg_disp_ypos(g_p_vout_reg, disp_rect->y);
        hal_vout_set_cfg_disp_v(g_p_vout_reg, disp_rect->height - 1);
        hal_vout_set_cfg_disp_h(g_p_vout_reg, disp_rect->width - 1);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_voutv1_cfg_disp_xpos(g_p_vout_reg, disp_rect->x);
        hal_vout_set_voutv1_cfg_disp_ypos(g_p_vout_reg, disp_rect->y);
        hal_vout_set_1disp_cfg_disp_v(g_p_vout_reg, disp_rect->height - 1);
        hal_vout_set_1disp_cfg_disp_h(g_p_vout_reg, disp_rect->width - 1);
    } else {
        ;
    }
    return;
}

xmedia_void  hal_vo_get_disp_rect     (xmedia_vo_layer layer, xmedia_video_rect *disp_rect)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
       disp_rect->x = hal_vout_get_cfg_disp_xpos(g_p_vout_reg);
       disp_rect->y = hal_vout_get_cfg_disp_ypos(g_p_vout_reg);
       disp_rect->height= hal_vout_get_cfg_disp_v(g_p_vout_reg);
       disp_rect->width = hal_vout_get_cfg_disp_h(g_p_vout_reg);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        disp_rect->x = hal_vout_get_voutv1_cfg_disp_xpos(g_p_vout_reg);
        disp_rect->y = hal_vout_get_voutv1_cfg_disp_ypos(g_p_vout_reg);
        disp_rect->height = hal_vout_get_1disp_cfg_disp_v(g_p_vout_reg);
        disp_rect->width = hal_vout_get_1disp_cfg_disp_h(g_p_vout_reg);
    } else {
        ;
    }
    return;
}

xmedia_void  hal_vo_set_src_rect     (xmedia_vo_layer layer, xmedia_video_rect *src_rect)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_src_xpos(g_p_vout_reg, src_rect->x);
        hal_vout_set_cfg_src_ypos(g_p_vout_reg, src_rect->y);
        hal_vout_set_cfg_src_v(g_p_vout_reg, src_rect->height - 1);
        hal_vout_set_cfg_src_h(g_p_vout_reg, src_rect->width - 1);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_voutv1_cfg_src_xpos(g_p_vout_reg, src_rect->x);
        hal_vout_set_voutv1_cfg_src_ypos(g_p_vout_reg, src_rect->y);
        hal_vout_set_1src_cfg_src_v(g_p_vout_reg, src_rect->height - 1);
        hal_vout_set_1src_cfg_src_h(g_p_vout_reg, src_rect->width - 1);
    } else {
        ;
    }
    return ;
}

xmedia_void  hal_vo_get_src_rect     (xmedia_vo_layer layer, xmedia_video_rect *src_rect)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        src_rect->x = hal_vout_get_cfg_src_xpos(g_p_vout_reg);
        src_rect->y = hal_vout_get_cfg_src_ypos(g_p_vout_reg);
        src_rect->width = hal_vout_get_cfg_src_v(g_p_vout_reg);
        src_rect->height = hal_vout_get_cfg_src_h(g_p_vout_reg);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        src_rect->x = hal_vout_get_voutv1_cfg_src_xpos(g_p_vout_reg);
        src_rect->y = hal_vout_get_voutv1_cfg_src_ypos(g_p_vout_reg);
        src_rect->width = hal_vout_get_1src_cfg_src_v(g_p_vout_reg);
        src_rect->height = hal_vout_get_1src_cfg_src_h(g_p_vout_reg);
    } else {
        ;
    }
    return ;
}

xmedia_void  hal_vo_set_dst_rect     (xmedia_vo_layer layer, xmedia_video_rect *dst_rect)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_disp_xpos(g_p_vout_reg, dst_rect->x);
        hal_vout_set_cfg_disp_ypos(g_p_vout_reg, dst_rect->y);
        hal_vout_set_cfg_disp_v(g_p_vout_reg, dst_rect->height - 1);
        hal_vout_set_cfg_disp_h(g_p_vout_reg, dst_rect->width - 1);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_voutv1_cfg_disp_xpos(g_p_vout_reg, dst_rect->x);
        hal_vout_set_voutv1_cfg_disp_ypos(g_p_vout_reg, dst_rect->y);
        hal_vout_set_1disp_cfg_disp_v(g_p_vout_reg, dst_rect->height - 1);
        hal_vout_set_1disp_cfg_disp_h(g_p_vout_reg, dst_rect->width - 1);
    } else {
        ;
    }
    return;
}

xmedia_void  hal_vo_set_addr     (xmedia_vo_layer layer, vo_addr *addr)
{
    xmedia_u32 addr_y_l = addr->lum_addr & 0xFFFFFFFF;
    xmedia_u32 addr_y_h = (addr->lum_addr & 0xFFFFFFFF00000000) >> 32;
    xmedia_u32 addr_c_l = addr->chm_addr & 0xFFFFFFFF;
    xmedia_u32 addr_c_h = (addr->chm_addr & 0xFFFFFFFF00000000) >> 32;

    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_src_stride_c(g_p_vout_reg, addr->chm_stride);
        hal_vout_set_cfg_src_addr_c_l(g_p_vout_reg, addr_c_l);
        hal_vout_set_cfg_src_addr_c_h(g_p_vout_reg, addr_c_h);
        hal_vout_set_cfg_src_stride_y(g_p_vout_reg, addr->lum_stride);
        hal_vout_set_cfg_src_addr_y_l(g_p_vout_reg, addr_y_l);
        hal_vout_set_cfg_src_addr_y_h(g_p_vout_reg, addr_y_h);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_voutv1_cfg_src_stride_c(g_p_vout_reg, addr->chm_stride);
        hal_vout_set_voutv1_cfg_src_addr_c_l(g_p_vout_reg, addr_c_l);
        hal_vout_set_voutv1_cfg_src_addr_c_h(g_p_vout_reg, addr_c_h);
        hal_vout_set_voutv1_cfg_src_stride_y(g_p_vout_reg, addr->lum_stride);
        hal_vout_set_voutv1_cfg_src_addr_y_l(g_p_vout_reg, addr_y_l);
        hal_vout_set_voutv1_cfg_src_addr_y_h(g_p_vout_reg, addr_y_h);
    } else {
        ;
    }
    return ;
}

xmedia_void hal_vo_set_mrg_mixer_prio(xmedia_vo_layer layer, vo_mrg_mix_prio prio_id)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_mixer_prio(g_p_vout_reg, prio_id);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_voutv1_cfg_mixer_prio(g_p_vout_reg, prio_id);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_mrg_rdma1_link(xmedia_vo_layer layer, xmedia_u8 rgion_id)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_mrg_link_rdma1(g_p_vout_reg, rgion_id);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_voutv1_cfg_mrg_link_rdma1(g_p_vout_reg, rgion_id);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_mrg_rdma1_en(xmedia_vo_layer layer, xmedia_bool enable)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_rdma1_en(g_p_vout_reg, enable);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_voutv1_cfg_rdma1_en(g_p_vout_reg, enable);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_mrg_enable(xmedia_vo_layer layer, xmedia_u32 region_id, xmedia_bool enable)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        if (region_id == 0) {
            hal_vout_set_cfg_mrg0_en(g_p_vout_reg, enable);
        } else if (region_id == 1) {
            hal_vout_set_cfg_mrg1_en(g_p_vout_reg, enable);
        } else if (region_id == 2) {
            hal_vout_set_cfg_mrg2_en(g_p_vout_reg, enable);
        } else if (region_id == 3) {
            hal_vout_set_cfg_mrg3_en(g_p_vout_reg, enable);
        } else {
           ;
        }
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        if (region_id == 0) {
            hal_vout_set_voutv1_cfg_mrg0_en(g_p_vout_reg, enable);
        } else if (region_id == 1) {
            hal_vout_set_voutv1_cfg_mrg1_en(g_p_vout_reg, enable);
        } else if (region_id == 2) {
            hal_vout_set_voutv1_cfg_mrg2_en(g_p_vout_reg, enable);
        } else if (region_id == 3) {
            hal_vout_set_voutv1_cfg_mrg3_en(g_p_vout_reg, enable);
        } else {
           ;
        }
    } else {
        ;
    }
    return ;
}

xmedia_void  hal_vo_set_mrg_mute_enable(xmedia_vo_layer layer,   xmedia_u32 region_id, xmedia_bool enable)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        if (region_id == 0) {
            hal_vout_set_cfg_mrg0_mute_en(g_p_vout_reg, enable);
        } else if (region_id == 1) {
            hal_vout_set_cfg_mrg1_mute_en(g_p_vout_reg, enable);
        } else if (region_id == 2) {
            hal_vout_set_cfg_mrg2_mute_en(g_p_vout_reg, enable);
        } else if (region_id == 3) {
            hal_vout_set_cfg_mrg3_mute_en(g_p_vout_reg, enable);
        } else {
            ;
        }
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        if (region_id == 0) {
            hal_vout_set_voutv1_cfg_mrg0_mute_en(g_p_vout_reg, enable);
        } else if (region_id == 1) {
            hal_vout_set_voutv1_cfg_mrg1_mute_en(g_p_vout_reg, enable);
        } else if (region_id == 2) {
            hal_vout_set_voutv1_cfg_mrg2_mute_en(g_p_vout_reg, enable);
        } else if (region_id == 3) {
            hal_vout_set_voutv1_cfg_mrg3_mute_en(g_p_vout_reg, enable);
        } else {
           ;
        }
    } else {
        ;
    }
    return ;
}

xmedia_void hal_vo_get_mrg_enable(xmedia_vo_layer layer, xmedia_u32 region_id, xmedia_bool *enable)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        if (region_id == 0) {
            *enable = hal_vout_get_cfg_mrg0_en(g_p_vout_reg);
        } else if (region_id == 1) {
            *enable = hal_vout_get_cfg_mrg1_en(g_p_vout_reg);
        } else if (region_id == 2) {
            *enable = hal_vout_get_cfg_mrg2_en(g_p_vout_reg);
        } else if (region_id == 3) {
            *enable = hal_vout_get_cfg_mrg3_en(g_p_vout_reg);
        } else {
           ;
        }
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        if (region_id == 0) {
            *enable = hal_vout_get_voutv1_cfg_mrg0_en(g_p_vout_reg);
        } else if (region_id == 1) {
            *enable = hal_vout_get_voutv1_cfg_mrg1_en(g_p_vout_reg);
        } else if (region_id == 2) {
            *enable = hal_vout_get_voutv1_cfg_mrg2_en(g_p_vout_reg);
        } else if (region_id == 3) {
            *enable = hal_vout_get_voutv1_cfg_mrg3_en(g_p_vout_reg);
        } else {
            ;
        }
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_mrg_sort(xmedia_vo_layer layer, xmedia_u16 sort)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_mixer_sort(g_p_vout_reg, sort);

    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_voutv1_cfg_mixer_sort(g_p_vout_reg, sort);
    } else {
        ;
    }
    return;
}

xmedia_void  hal_vo_set_mrg_src_rect     (xmedia_vo_layer layer,   xmedia_u32 region_id, xmedia_video_rect *src_rect)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        if (region_id == 0) {
            hal_vout_set_cfg_mrg0_src_xpos(g_p_vout_reg, src_rect->x);
            hal_vout_set_cfg_mrg0_src_ypos(g_p_vout_reg, src_rect->y);
            hal_vout_set_cfg_mrg0_src_h(g_p_vout_reg, src_rect->width - 1);
            hal_vout_set_cfg_mrg0_src_v(g_p_vout_reg, src_rect->height - 1);
        }
        else if (region_id == 1) {
            hal_vout_set_cfg_mrg1_src_xpos(g_p_vout_reg, src_rect->x);
            hal_vout_set_cfg_mrg1_src_ypos(g_p_vout_reg, src_rect->y);
            hal_vout_set_cfg_mrg1_src_h(g_p_vout_reg, src_rect->width - 1);
            hal_vout_set_cfg_mrg1_src_v(g_p_vout_reg, src_rect->height - 1);
        }
        else if (region_id == 2) {
            hal_vout_set_cfg_mrg2_src_xpos(g_p_vout_reg, src_rect->x);
            hal_vout_set_cfg_mrg2_src_ypos(g_p_vout_reg, src_rect->y);
            hal_vout_set_cfg_mrg2_src_h(g_p_vout_reg, src_rect->width - 1);
            hal_vout_set_cfg_mrg2_src_v(g_p_vout_reg, src_rect->height - 1);
        }
        else if (region_id == 3) {
            hal_vout_set_cfg_mrg3_src_xpos(g_p_vout_reg, src_rect->x);
            hal_vout_set_cfg_mrg3_src_ypos(g_p_vout_reg, src_rect->y);
            hal_vout_set_cfg_mrg3_src_h(g_p_vout_reg, src_rect->width - 1);
            hal_vout_set_cfg_mrg3_src_v(g_p_vout_reg, src_rect->height - 1);
        } else {
            ;
        }
     } else if (layer == XMEDIA_VO_LAYER_V1) {
        if (region_id == 0) {
            hal_vout_set_voutv1_cfg_mrg0_src_xpos(g_p_vout_reg, src_rect->x);
            hal_vout_set_voutv1_cfg_mrg0_src_ypos(g_p_vout_reg, src_rect->y);
            hal_vout_set_voutv1_cfg_mrg0_src_h(g_p_vout_reg, src_rect->width - 1);
            hal_vout_set_voutv1_cfg_mrg0_src_v(g_p_vout_reg, src_rect->height - 1);
        }
        else if (region_id == 1) {
            hal_vout_set_voutv1_cfg_mrg1_src_xpos(g_p_vout_reg, src_rect->x);
            hal_vout_set_voutv1_cfg_mrg1_src_ypos(g_p_vout_reg, src_rect->y);
            hal_vout_set_voutv1_cfg_mrg1_src_h(g_p_vout_reg, src_rect->width - 1);
            hal_vout_set_voutv1_cfg_mrg1_src_v(g_p_vout_reg, src_rect->height - 1);
        }
        else if (region_id == 2) {
            hal_vout_set_voutv1_cfg_mrg2_src_xpos(g_p_vout_reg, src_rect->x);
            hal_vout_set_voutv1_cfg_mrg2_src_ypos(g_p_vout_reg, src_rect->y);
            hal_vout_set_voutv1_cfg_mrg2_src_h(g_p_vout_reg, src_rect->width - 1);
            hal_vout_set_voutv1_cfg_mrg2_src_v(g_p_vout_reg, src_rect->height - 1);
        }
        else if (region_id == 3) {
            hal_vout_set_voutv1_cfg_mrg3_src_xpos(g_p_vout_reg, src_rect->x);
            hal_vout_set_voutv1_cfg_mrg3_src_ypos(g_p_vout_reg, src_rect->y);
            hal_vout_set_voutv1_cfg_mrg3_src_h(g_p_vout_reg, src_rect->width - 1);
            hal_vout_set_voutv1_cfg_mrg3_src_v(g_p_vout_reg, src_rect->height - 1);
        } else {
            ;
        }
    } else {
        ;
    }
    return ;
}

xmedia_void  hal_vo_set_mrg_dst_rect     (xmedia_vo_layer layer,   xmedia_u32 region_id, xmedia_video_rect *dst_rect)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        if (region_id == 0) {
            hal_vout_set_cfg_mrg0_src_xpos(g_p_vout_reg, dst_rect->x);
            hal_vout_set_cfg_mrg0_src_ypos(g_p_vout_reg, dst_rect->y);
            hal_vout_set_cfg_mrg0_src_h(g_p_vout_reg, dst_rect->width - 1);
            hal_vout_set_cfg_mrg0_src_v(g_p_vout_reg, dst_rect->height - 1);
        }
        else if (region_id == 1) {
            hal_vout_set_cfg_mrg1_src_xpos(g_p_vout_reg, dst_rect->x);
            hal_vout_set_cfg_mrg1_src_ypos(g_p_vout_reg, dst_rect->y);
            hal_vout_set_cfg_mrg1_src_h(g_p_vout_reg, dst_rect->width - 1);
            hal_vout_set_cfg_mrg1_src_v(g_p_vout_reg, dst_rect->height - 1);
        }
        else if (region_id == 2) {
            hal_vout_set_cfg_mrg2_src_xpos(g_p_vout_reg, dst_rect->x);
            hal_vout_set_cfg_mrg2_src_ypos(g_p_vout_reg, dst_rect->y);
            hal_vout_set_cfg_mrg2_src_h(g_p_vout_reg, dst_rect->width - 1);
            hal_vout_set_cfg_mrg2_src_v(g_p_vout_reg, dst_rect->height - 1);
        }
        else if (region_id == 3) {
            hal_vout_set_cfg_mrg3_src_xpos(g_p_vout_reg, dst_rect->x);
            hal_vout_set_cfg_mrg3_src_ypos(g_p_vout_reg, dst_rect->y);
            hal_vout_set_cfg_mrg3_src_h(g_p_vout_reg, dst_rect->width - 1);
            hal_vout_set_cfg_mrg3_src_v(g_p_vout_reg, dst_rect->height - 1);
        } else {
             ;
        }
    }  else if (layer == XMEDIA_VO_LAYER_V1) {
        if (region_id == 0) {
            hal_vout_set_voutv1_cfg_mrg0_src_xpos(g_p_vout_reg, dst_rect->x);
            hal_vout_set_voutv1_cfg_mrg0_src_ypos(g_p_vout_reg, dst_rect->y);
            hal_vout_set_voutv1_cfg_mrg0_src_h(g_p_vout_reg, dst_rect->width - 1);
            hal_vout_set_voutv1_cfg_mrg0_src_v(g_p_vout_reg, dst_rect->height - 1);
        }
        else if (region_id == 1) {
            hal_vout_set_voutv1_cfg_mrg1_src_xpos(g_p_vout_reg, dst_rect->x);
            hal_vout_set_voutv1_cfg_mrg1_src_ypos(g_p_vout_reg, dst_rect->y);
            hal_vout_set_voutv1_cfg_mrg1_src_h(g_p_vout_reg, dst_rect->width - 1);
            hal_vout_set_voutv1_cfg_mrg1_src_v(g_p_vout_reg, dst_rect->height - 1);
        }
        else if (region_id == 2) {
            hal_vout_set_voutv1_cfg_mrg2_src_xpos(g_p_vout_reg, dst_rect->x);
            hal_vout_set_voutv1_cfg_mrg2_src_ypos(g_p_vout_reg, dst_rect->y);
            hal_vout_set_voutv1_cfg_mrg2_src_h(g_p_vout_reg, dst_rect->width - 1);
            hal_vout_set_voutv1_cfg_mrg2_src_v(g_p_vout_reg, dst_rect->height - 1);
        }
        else if (region_id == 3) {
            hal_vout_set_voutv1_cfg_mrg3_src_xpos(g_p_vout_reg, dst_rect->x);
            hal_vout_set_voutv1_cfg_mrg3_src_ypos(g_p_vout_reg, dst_rect->y);
            hal_vout_set_voutv1_cfg_mrg3_src_h(g_p_vout_reg, dst_rect->width - 1);
            hal_vout_set_voutv1_cfg_mrg3_src_v(g_p_vout_reg, dst_rect->height - 1);
        } else {
            ;
        }
    } else {
        ;
    }
    return ;
}

xmedia_void  hal_vo_set_mrg0_addr(xmedia_vo_layer layer,      vo_addr *addr)
{
    xmedia_u32 addr_y_l = addr->lum_addr & 0xFFFFFFFF;
    xmedia_u32 addr_y_h = (addr->lum_addr & 0xFFFFFFFF00000000) >> 32;
    xmedia_u32 addr_c_l = addr->chm_addr & 0xFFFFFFFF;
    xmedia_u32 addr_c_h = (addr->chm_addr & 0xFFFFFFFF00000000) >> 32;

    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_mrg0_src_stride_c(g_p_vout_reg, addr->chm_stride);
        hal_vout_set_cfg_mrg0_src_addr_c_l(g_p_vout_reg, addr_c_l);
        hal_vout_set_cfg_mrg0_src_addr_c_h(g_p_vout_reg, addr_c_h);
        hal_vout_set_cfg_mrg0_src_stride_y(g_p_vout_reg, addr->lum_stride);
        hal_vout_set_cfg_mrg0_src_addr_y_l(g_p_vout_reg, addr_y_l);
        hal_vout_set_cfg_mrg0_src_addr_y_h(g_p_vout_reg, addr_y_h);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_voutv1_cfg_mrg0_src_stride_c(g_p_vout_reg, addr->chm_stride);
        hal_vout_set_voutv1_cfg_mrg0_src_addr_c_l(g_p_vout_reg, addr_c_l);
        hal_vout_set_voutv1_cfg_mrg0_src_addr_c_h(g_p_vout_reg, addr_c_h);
        hal_vout_set_voutv1_cfg_mrg0_src_stride_y(g_p_vout_reg, addr->lum_stride);
        hal_vout_set_voutv1_cfg_mrg0_src_addr_y_l(g_p_vout_reg, addr_y_l);
        hal_vout_set_voutv1_cfg_mrg0_src_addr_y_h(g_p_vout_reg, addr_y_h);
    } else {
        ;
    }

    return ;
}

xmedia_void  hal_vo_set_mrg1_addr(xmedia_vo_layer layer,      vo_addr *addr)
{
    xmedia_u32 addr_y_l = addr->lum_addr & 0xFFFFFFFF;
    xmedia_u32 addr_y_h = (addr->lum_addr & 0xFFFFFFFF00000000) >> 32;
    xmedia_u32 addr_c_l = addr->chm_addr & 0xFFFFFFFF;
    xmedia_u32 addr_c_h = (addr->chm_addr & 0xFFFFFFFF00000000) >> 32;

    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_mrg1_src_stride_c(g_p_vout_reg, addr->chm_stride);
        hal_vout_set_cfg_mrg1_src_addr_c_l(g_p_vout_reg, addr_c_l);
        hal_vout_set_cfg_mrg1_src_addr_c_h(g_p_vout_reg, addr_c_h);
        hal_vout_set_cfg_mrg1_src_stride_y(g_p_vout_reg, addr->lum_stride);
        hal_vout_set_cfg_mrg1_src_addr_y_l(g_p_vout_reg, addr_y_l);
        hal_vout_set_cfg_mrg1_src_addr_y_h(g_p_vout_reg, addr_y_h);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_voutv1_cfg_mrg1_src_stride_c(g_p_vout_reg, addr->chm_stride);
        hal_vout_set_voutv1_cfg_mrg1_src_addr_c_l(g_p_vout_reg, addr_c_l);
        hal_vout_set_voutv1_cfg_mrg1_src_addr_c_h(g_p_vout_reg, addr_c_h);
        hal_vout_set_voutv1_cfg_mrg1_src_stride_y(g_p_vout_reg, addr->lum_stride);
        hal_vout_set_voutv1_cfg_mrg1_src_addr_y_l(g_p_vout_reg, addr_y_l);
        hal_vout_set_voutv1_cfg_mrg1_src_addr_y_h(g_p_vout_reg, addr_y_h);
    } else {
        ;
    }
    return ;
}

xmedia_void  hal_vo_set_mrg2_addr(xmedia_vo_layer layer,      vo_addr *addr)
{
    xmedia_u32 addr_y_l = addr->lum_addr & 0xFFFFFFFF;
    xmedia_u32 addr_y_h = (addr->lum_addr & 0xFFFFFFFF00000000) >> 32;
    xmedia_u32 addr_c_l = addr->chm_addr & 0xFFFFFFFF;
    xmedia_u32 addr_c_h = (addr->chm_addr & 0xFFFFFFFF00000000) >> 32;

    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_mrg2_src_stride_c(g_p_vout_reg, addr->chm_stride);
        hal_vout_set_cfg_mrg2_src_addr_c_l(g_p_vout_reg, addr_c_l);
        hal_vout_set_cfg_mrg2_src_addr_c_h(g_p_vout_reg, addr_c_h);
        hal_vout_set_cfg_mrg2_src_stride_y(g_p_vout_reg, addr->lum_stride);
        hal_vout_set_cfg_mrg2_src_addr_y_l(g_p_vout_reg, addr_y_l);
        hal_vout_set_cfg_mrg2_src_addr_y_h(g_p_vout_reg, addr_y_h);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_voutv1_cfg_mrg2_src_stride_c(g_p_vout_reg, addr->chm_stride);
        hal_vout_set_voutv1_cfg_mrg2_src_addr_c_l(g_p_vout_reg, addr_c_l);
        hal_vout_set_voutv1_cfg_mrg2_src_addr_c_h(g_p_vout_reg, addr_c_h);
        hal_vout_set_voutv1_cfg_mrg2_src_stride_y(g_p_vout_reg, addr->lum_stride);
        hal_vout_set_voutv1_cfg_mrg2_src_addr_y_l(g_p_vout_reg, addr_y_l);
        hal_vout_set_voutv1_cfg_mrg2_src_addr_y_h(g_p_vout_reg, addr_y_h);
    } else {
        ;
    }

    return ;
}

xmedia_void  hal_vo_set_mrg3_addr(xmedia_vo_layer layer,      vo_addr *addr)
{
    xmedia_u32 addr_y_l = addr->lum_addr & 0xFFFFFFFF;
    xmedia_u32 addr_y_h = (addr->lum_addr & 0xFFFFFFFF00000000) >> 32;
    xmedia_u32 addr_c_l = addr->chm_addr & 0xFFFFFFFF;
    xmedia_u32 addr_c_h = (addr->chm_addr & 0xFFFFFFFF00000000) >> 32;

    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_mrg3_src_stride_c(g_p_vout_reg, addr->chm_stride);
        hal_vout_set_cfg_mrg3_src_addr_c_l(g_p_vout_reg, addr_c_l);
        hal_vout_set_cfg_mrg3_src_addr_c_h(g_p_vout_reg, addr_c_h);
        hal_vout_set_cfg_mrg3_src_stride_y(g_p_vout_reg, addr->lum_stride);
        hal_vout_set_cfg_mrg3_src_addr_y_l(g_p_vout_reg, addr_y_l);
        hal_vout_set_cfg_mrg3_src_addr_y_h(g_p_vout_reg, addr_y_h);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_voutv1_cfg_mrg3_src_stride_c(g_p_vout_reg, addr->chm_stride);
        hal_vout_set_voutv1_cfg_mrg3_src_addr_c_l(g_p_vout_reg, addr_c_l);
        hal_vout_set_voutv1_cfg_mrg3_src_addr_c_h(g_p_vout_reg, addr_c_h);
        hal_vout_set_voutv1_cfg_mrg3_src_stride_y(g_p_vout_reg, addr->lum_stride);
        hal_vout_set_voutv1_cfg_mrg3_src_addr_y_l(g_p_vout_reg, addr_y_l);
        hal_vout_set_voutv1_cfg_mrg3_src_addr_y_h(g_p_vout_reg, addr_y_h);
    } else {
        ;
    }

    return ;
}

xmedia_void  hal_vo_set_mrg_addr(xmedia_vo_layer layer,       xmedia_u32 region_id,vo_addr *addr)
{
    if (region_id == 0) {
        hal_vo_set_mrg0_addr(layer, addr);
    } else if (region_id == 1) {
        hal_vo_set_mrg1_addr(layer, addr);
    } else if (region_id == 2) {
        hal_vo_set_mrg2_addr(layer, addr);
    } else if (region_id == 3) {
        hal_vo_set_mrg3_addr(layer, addr);
    } else {
        ;
    }
    return ;
}

xmedia_void  hal_vo_set_mrg_mute_color(xmedia_vo_layer layer,         xmedia_u32 region_id, xmedia_u32 bgcolor)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        if (region_id == 0) {
            hal_vout_set_cfg_mrg0_mute_color(g_p_vout_reg, bgcolor);
        } else if (region_id == 1) {
            hal_vout_set_cfg_mrg1_mute_color(g_p_vout_reg, bgcolor);
        } else if (region_id == 2) {
            hal_vout_set_cfg_mrg2_mute_color(g_p_vout_reg, bgcolor);
        } else if (region_id == 3) {
            hal_vout_set_cfg_mrg3_mute_color(g_p_vout_reg, bgcolor);
        }
        else {
            ;
        }
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        if (region_id == 0) {
            hal_vout_set_voutv1_cfg_mrg0_mute_color(g_p_vout_reg, bgcolor);
        } else if (region_id == 1) {
            hal_vout_set_voutv1_cfg_mrg1_mute_color(g_p_vout_reg, bgcolor);
        } else if (region_id == 2) {
            hal_vout_set_voutv1_cfg_mrg2_mute_color(g_p_vout_reg, bgcolor);
        } else if (region_id == 3) {
            hal_vout_set_voutv1_cfg_mrg3_mute_color(g_p_vout_reg, bgcolor);
        } else {
             ;
        }
    } else {
        ;
    }
    return ;
}

xmedia_void hal_vo_set_cus(xmedia_vo_layer layer, vo_video_cus *cus)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_vcus_mode(g_p_vout_reg, cus->vcus_mode);
        hal_vout_set_cfg_hcus_mode(g_p_vout_reg, cus->hcus_mode);
        hal_vout_set_cfg_vcus_coef_0(g_p_vout_reg, cus->vcus_coef0);
        hal_vout_set_cfg_vcus_coef_1(g_p_vout_reg, cus->vcus_coef1);
        hal_vout_set_cfg_vcus_coef_2(g_p_vout_reg, cus->vcus_coef2);
        hal_vout_set_cfg_vcus_coef_3(g_p_vout_reg, cus->vcus_coef3);
        hal_vout_set_cfg_hcus_coef_0(g_p_vout_reg, cus->hcus_coef0);
        hal_vout_set_cfg_hcus_coef_1(g_p_vout_reg, cus->hcus_coef1);
        hal_vout_set_cfg_hcus_coef_2(g_p_vout_reg, cus->hcus_coef2);
        hal_vout_set_cfg_hcus_coef_3(g_p_vout_reg, cus->hcus_coef3);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_voutv1_cfg_vcus_mode(g_p_vout_reg, cus->vcus_mode);
        hal_vout_set_voutv1_cfg_hcus_mode(g_p_vout_reg, cus->hcus_mode);
        hal_vout_set_voutv1_cfg_vcus_coef_0(g_p_vout_reg, cus->vcus_coef0);
        hal_vout_set_voutv1_cfg_vcus_coef_1(g_p_vout_reg, cus->vcus_coef1);
        hal_vout_set_voutv1_cfg_vcus_coef_2(g_p_vout_reg, cus->vcus_coef2);
        hal_vout_set_voutv1_cfg_vcus_coef_3(g_p_vout_reg, cus->vcus_coef3);
        hal_vout_set_voutv1_cfg_hcus_coef_0(g_p_vout_reg, cus->hcus_coef0);
        hal_vout_set_voutv1_cfg_hcus_coef_1(g_p_vout_reg, cus->hcus_coef1);
        hal_vout_set_voutv1_cfg_hcus_coef_2(g_p_vout_reg, cus->hcus_coef2);
        hal_vout_set_voutv1_cfg_hcus_coef_3(g_p_vout_reg, cus->hcus_coef3);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_csc_en(xmedia_vo_layer layer, xmedia_bool enable)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_csc_en(g_p_vout_reg,enable);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_voutv1_cfg_csc_en(g_p_vout_reg,enable);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_csc(xmedia_vo_layer layer,vo_csc_coef *csc)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_mrg0_csc_offset_in_2(g_p_vout_reg,csc->csc_offset_in2);
        hal_vout_set_cfg_mrg0_csc_offset_in_1(g_p_vout_reg,csc->csc_offset_in1);
        hal_vout_set_cfg_mrg0_csc_offset_in_0(g_p_vout_reg,csc->csc_offset_in0);
        hal_vout_set_cfg_mrg0_csc_offset_out_2(g_p_vout_reg,csc->csc_offset_out2);
        hal_vout_set_cfg_mrg0_csc_offset_out_1(g_p_vout_reg,csc->csc_offset_out1);
        hal_vout_set_cfg_mrg0_csc_offset_out_0(g_p_vout_reg,csc->csc_offset_out0);
        hal_vout_set_cfg_mrg0_csc_matrix_0(g_p_vout_reg,csc->csc_coef0);
        hal_vout_set_cfg_mrg0_csc_matrix_1(g_p_vout_reg,csc->csc_coef1);
        hal_vout_set_cfg_mrg0_csc_matrix_2(g_p_vout_reg,csc->csc_coef2);
        hal_vout_set_cfg_mrg0_csc_matrix_3(g_p_vout_reg,csc->csc_coef3);
        hal_vout_set_cfg_mrg0_csc_matrix_4(g_p_vout_reg,csc->csc_coef4);
        hal_vout_set_cfg_mrg0_csc_matrix_5(g_p_vout_reg,csc->csc_coef5);
        hal_vout_set_cfg_mrg0_csc_matrix_6(g_p_vout_reg,csc->csc_coef6);
        hal_vout_set_cfg_mrg0_csc_matrix_7(g_p_vout_reg,csc->csc_coef7);
        hal_vout_set_cfg_mrg0_csc_matrix_8(g_p_vout_reg,csc->csc_coef8);

    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_voutv1_cfg_mrg0_csc_offset_in_2(g_p_vout_reg,csc->csc_offset_in2);
        hal_vout_set_voutv1_cfg_mrg0_csc_offset_in_1(g_p_vout_reg,csc->csc_offset_in1);
        hal_vout_set_voutv1_cfg_mrg0_csc_offset_in_0(g_p_vout_reg,csc->csc_offset_in0);
        hal_vout_set_voutv1_cfg_mrg0_csc_offset_out_2(g_p_vout_reg,csc->csc_offset_out2);
        hal_vout_set_voutv1_cfg_mrg0_csc_offset_out_1(g_p_vout_reg,csc->csc_offset_out1);
        hal_vout_set_voutv1_cfg_mrg0_csc_offset_out_0(g_p_vout_reg,csc->csc_offset_out0);
        hal_vout_set_voutv1_cfg_mrg0_csc_matrix_0(g_p_vout_reg,csc->csc_coef0);
        hal_vout_set_voutv1_cfg_mrg0_csc_matrix_1(g_p_vout_reg,csc->csc_coef1);
        hal_vout_set_voutv1_cfg_mrg0_csc_matrix_2(g_p_vout_reg,csc->csc_coef2);
        hal_vout_set_voutv1_cfg_mrg0_csc_matrix_3(g_p_vout_reg,csc->csc_coef3);
        hal_vout_set_voutv1_cfg_mrg0_csc_matrix_4(g_p_vout_reg,csc->csc_coef4);
        hal_vout_set_voutv1_cfg_mrg0_csc_matrix_5(g_p_vout_reg,csc->csc_coef5);
        hal_vout_set_voutv1_cfg_mrg0_csc_matrix_6(g_p_vout_reg,csc->csc_coef6);
        hal_vout_set_voutv1_cfg_mrg0_csc_matrix_7(g_p_vout_reg,csc->csc_coef7);
        hal_vout_set_voutv1_cfg_mrg0_csc_matrix_8(g_p_vout_reg,csc->csc_coef8);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_mrg0_csc(xmedia_vo_layer layer,vo_csc_coef *csc)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_mrg0_csc_offset_in_2(g_p_vout_reg,csc->csc_offset_in2);
        hal_vout_set_cfg_mrg0_csc_offset_in_1(g_p_vout_reg,csc->csc_offset_in1);
        hal_vout_set_cfg_mrg0_csc_offset_in_0(g_p_vout_reg,csc->csc_offset_in0);
        hal_vout_set_cfg_mrg0_csc_offset_out_2(g_p_vout_reg,csc->csc_offset_out2);
        hal_vout_set_cfg_mrg0_csc_offset_out_1(g_p_vout_reg,csc->csc_offset_out1);
        hal_vout_set_cfg_mrg0_csc_offset_out_0(g_p_vout_reg,csc->csc_offset_out0);
        hal_vout_set_cfg_mrg0_csc_matrix_0(g_p_vout_reg,csc->csc_coef0);
        hal_vout_set_cfg_mrg0_csc_matrix_1(g_p_vout_reg,csc->csc_coef1);
        hal_vout_set_cfg_mrg0_csc_matrix_2(g_p_vout_reg,csc->csc_coef2);
        hal_vout_set_cfg_mrg0_csc_matrix_3(g_p_vout_reg,csc->csc_coef3);
        hal_vout_set_cfg_mrg0_csc_matrix_4(g_p_vout_reg,csc->csc_coef4);
        hal_vout_set_cfg_mrg0_csc_matrix_5(g_p_vout_reg,csc->csc_coef5);
        hal_vout_set_cfg_mrg0_csc_matrix_6(g_p_vout_reg,csc->csc_coef6);
        hal_vout_set_cfg_mrg0_csc_matrix_7(g_p_vout_reg,csc->csc_coef7);
        hal_vout_set_cfg_mrg0_csc_matrix_8(g_p_vout_reg,csc->csc_coef8);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_voutv1_cfg_mrg0_csc_offset_in_2(g_p_vout_reg,csc->csc_offset_in2);
        hal_vout_set_voutv1_cfg_mrg0_csc_offset_in_1(g_p_vout_reg,csc->csc_offset_in1);
        hal_vout_set_voutv1_cfg_mrg0_csc_offset_in_0(g_p_vout_reg,csc->csc_offset_in0);
        hal_vout_set_voutv1_cfg_mrg0_csc_offset_out_2(g_p_vout_reg,csc->csc_offset_out2);
        hal_vout_set_voutv1_cfg_mrg0_csc_offset_out_1(g_p_vout_reg,csc->csc_offset_out1);
        hal_vout_set_voutv1_cfg_mrg0_csc_offset_out_0(g_p_vout_reg,csc->csc_offset_out0);
        hal_vout_set_voutv1_cfg_mrg0_csc_matrix_0(g_p_vout_reg,csc->csc_coef0);
        hal_vout_set_voutv1_cfg_mrg0_csc_matrix_1(g_p_vout_reg,csc->csc_coef1);
        hal_vout_set_voutv1_cfg_mrg0_csc_matrix_2(g_p_vout_reg,csc->csc_coef2);
        hal_vout_set_voutv1_cfg_mrg0_csc_matrix_3(g_p_vout_reg,csc->csc_coef3);
        hal_vout_set_voutv1_cfg_mrg0_csc_matrix_4(g_p_vout_reg,csc->csc_coef4);
        hal_vout_set_voutv1_cfg_mrg0_csc_matrix_5(g_p_vout_reg,csc->csc_coef5);
        hal_vout_set_voutv1_cfg_mrg0_csc_matrix_6(g_p_vout_reg,csc->csc_coef6);
        hal_vout_set_voutv1_cfg_mrg0_csc_matrix_7(g_p_vout_reg,csc->csc_coef7);
        hal_vout_set_voutv1_cfg_mrg0_csc_matrix_8(g_p_vout_reg,csc->csc_coef8);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_mrg1_csc(xmedia_vo_layer layer,vo_csc_coef *csc)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_mrg1_csc_offset_in_2(g_p_vout_reg,csc->csc_offset_in2);
        hal_vout_set_cfg_mrg1_csc_offset_in_1(g_p_vout_reg,csc->csc_offset_in1);
        hal_vout_set_cfg_mrg1_csc_offset_in_0(g_p_vout_reg,csc->csc_offset_in0);
        hal_vout_set_cfg_mrg1_csc_offset_out_2(g_p_vout_reg,csc->csc_offset_out2);
        hal_vout_set_cfg_mrg1_csc_offset_out_1(g_p_vout_reg,csc->csc_offset_out1);
        hal_vout_set_cfg_mrg1_csc_offset_out_0(g_p_vout_reg,csc->csc_offset_out0);
        hal_vout_set_cfg_mrg1_csc_matrix_0(g_p_vout_reg,csc->csc_coef0);
        hal_vout_set_cfg_mrg1_csc_matrix_1(g_p_vout_reg,csc->csc_coef1);
        hal_vout_set_cfg_mrg1_csc_matrix_2(g_p_vout_reg,csc->csc_coef2);
        hal_vout_set_cfg_mrg1_csc_matrix_3(g_p_vout_reg,csc->csc_coef3);
        hal_vout_set_cfg_mrg1_csc_matrix_4(g_p_vout_reg,csc->csc_coef4);
        hal_vout_set_cfg_mrg1_csc_matrix_5(g_p_vout_reg,csc->csc_coef5);
        hal_vout_set_cfg_mrg1_csc_matrix_6(g_p_vout_reg,csc->csc_coef6);
        hal_vout_set_cfg_mrg1_csc_matrix_7(g_p_vout_reg,csc->csc_coef7);
        hal_vout_set_cfg_mrg1_csc_matrix_8(g_p_vout_reg,csc->csc_coef8);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_voutv1_cfg_mrg1_csc_offset_in_2(g_p_vout_reg,csc->csc_offset_in2);
        hal_vout_set_voutv1_cfg_mrg1_csc_offset_in_1(g_p_vout_reg,csc->csc_offset_in1);
        hal_vout_set_voutv1_cfg_mrg1_csc_offset_in_0(g_p_vout_reg,csc->csc_offset_in0);
        hal_vout_set_voutv1_cfg_mrg1_csc_offset_out_2(g_p_vout_reg,csc->csc_offset_out2);
        hal_vout_set_voutv1_cfg_mrg1_csc_offset_out_1(g_p_vout_reg,csc->csc_offset_out1);
        hal_vout_set_voutv1_cfg_mrg1_csc_offset_out_0(g_p_vout_reg,csc->csc_offset_out0);
        hal_vout_set_voutv1_cfg_mrg1_csc_matrix_0(g_p_vout_reg,csc->csc_coef0);
        hal_vout_set_voutv1_cfg_mrg1_csc_matrix_1(g_p_vout_reg,csc->csc_coef1);
        hal_vout_set_voutv1_cfg_mrg1_csc_matrix_2(g_p_vout_reg,csc->csc_coef2);
        hal_vout_set_voutv1_cfg_mrg1_csc_matrix_3(g_p_vout_reg,csc->csc_coef3);
        hal_vout_set_voutv1_cfg_mrg1_csc_matrix_4(g_p_vout_reg,csc->csc_coef4);
        hal_vout_set_voutv1_cfg_mrg1_csc_matrix_5(g_p_vout_reg,csc->csc_coef5);
        hal_vout_set_voutv1_cfg_mrg1_csc_matrix_6(g_p_vout_reg,csc->csc_coef6);
        hal_vout_set_voutv1_cfg_mrg1_csc_matrix_7(g_p_vout_reg,csc->csc_coef7);
        hal_vout_set_voutv1_cfg_mrg1_csc_matrix_8(g_p_vout_reg,csc->csc_coef8);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_mrg2_csc(xmedia_vo_layer layer,vo_csc_coef *csc)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_mrg2_csc_offset_in_2(g_p_vout_reg,csc->csc_offset_in2);
        hal_vout_set_cfg_mrg2_csc_offset_in_1(g_p_vout_reg,csc->csc_offset_in1);
        hal_vout_set_cfg_mrg2_csc_offset_in_0(g_p_vout_reg,csc->csc_offset_in0);
        hal_vout_set_cfg_mrg2_csc_offset_out_2(g_p_vout_reg,csc->csc_offset_out2);
        hal_vout_set_cfg_mrg2_csc_offset_out_1(g_p_vout_reg,csc->csc_offset_out1);
        hal_vout_set_cfg_mrg2_csc_offset_out_0(g_p_vout_reg,csc->csc_offset_out0);
        hal_vout_set_cfg_mrg2_csc_matrix_0(g_p_vout_reg,csc->csc_coef0);
        hal_vout_set_cfg_mrg2_csc_matrix_1(g_p_vout_reg,csc->csc_coef1);
        hal_vout_set_cfg_mrg2_csc_matrix_2(g_p_vout_reg,csc->csc_coef2);
        hal_vout_set_cfg_mrg2_csc_matrix_3(g_p_vout_reg,csc->csc_coef3);
        hal_vout_set_cfg_mrg2_csc_matrix_4(g_p_vout_reg,csc->csc_coef4);
        hal_vout_set_cfg_mrg2_csc_matrix_5(g_p_vout_reg,csc->csc_coef5);
        hal_vout_set_cfg_mrg2_csc_matrix_6(g_p_vout_reg,csc->csc_coef6);
        hal_vout_set_cfg_mrg2_csc_matrix_7(g_p_vout_reg,csc->csc_coef7);
        hal_vout_set_cfg_mrg2_csc_matrix_8(g_p_vout_reg,csc->csc_coef8);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_voutv1_cfg_mrg2_csc_offset_in_2(g_p_vout_reg,csc->csc_offset_in2);
        hal_vout_set_voutv1_cfg_mrg2_csc_offset_in_1(g_p_vout_reg,csc->csc_offset_in1);
        hal_vout_set_voutv1_cfg_mrg2_csc_offset_in_0(g_p_vout_reg,csc->csc_offset_in0);
        hal_vout_set_voutv1_cfg_mrg2_csc_offset_out_2(g_p_vout_reg,csc->csc_offset_out2);
        hal_vout_set_voutv1_cfg_mrg2_csc_offset_out_1(g_p_vout_reg,csc->csc_offset_out1);
        hal_vout_set_voutv1_cfg_mrg2_csc_offset_out_0(g_p_vout_reg,csc->csc_offset_out0);
        hal_vout_set_voutv1_cfg_mrg2_csc_matrix_0(g_p_vout_reg,csc->csc_coef0);
        hal_vout_set_voutv1_cfg_mrg2_csc_matrix_1(g_p_vout_reg,csc->csc_coef1);
        hal_vout_set_voutv1_cfg_mrg2_csc_matrix_2(g_p_vout_reg,csc->csc_coef2);
        hal_vout_set_voutv1_cfg_mrg2_csc_matrix_3(g_p_vout_reg,csc->csc_coef3);
        hal_vout_set_voutv1_cfg_mrg2_csc_matrix_4(g_p_vout_reg,csc->csc_coef4);
        hal_vout_set_voutv1_cfg_mrg2_csc_matrix_5(g_p_vout_reg,csc->csc_coef5);
        hal_vout_set_voutv1_cfg_mrg2_csc_matrix_6(g_p_vout_reg,csc->csc_coef6);
        hal_vout_set_voutv1_cfg_mrg2_csc_matrix_7(g_p_vout_reg,csc->csc_coef7);
        hal_vout_set_voutv1_cfg_mrg2_csc_matrix_8(g_p_vout_reg,csc->csc_coef8);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_mrg3_csc(xmedia_vo_layer layer,vo_csc_coef *csc)
{
    if (layer == XMEDIA_VO_LAYER_V0) {
        hal_vout_set_cfg_mrg3_csc_offset_in_2(g_p_vout_reg,csc->csc_offset_in2);
        hal_vout_set_cfg_mrg3_csc_offset_in_1(g_p_vout_reg,csc->csc_offset_in1);
        hal_vout_set_cfg_mrg3_csc_offset_in_0(g_p_vout_reg,csc->csc_offset_in0);
        hal_vout_set_cfg_mrg3_csc_offset_out_2(g_p_vout_reg,csc->csc_offset_out2);
        hal_vout_set_cfg_mrg3_csc_offset_out_1(g_p_vout_reg,csc->csc_offset_out1);
        hal_vout_set_cfg_mrg3_csc_offset_out_0(g_p_vout_reg,csc->csc_offset_out0);
        hal_vout_set_cfg_mrg3_csc_matrix_0(g_p_vout_reg,csc->csc_coef0);
        hal_vout_set_cfg_mrg3_csc_matrix_1(g_p_vout_reg,csc->csc_coef1);
        hal_vout_set_cfg_mrg3_csc_matrix_2(g_p_vout_reg,csc->csc_coef2);
        hal_vout_set_cfg_mrg3_csc_matrix_3(g_p_vout_reg,csc->csc_coef3);
        hal_vout_set_cfg_mrg3_csc_matrix_4(g_p_vout_reg,csc->csc_coef4);
        hal_vout_set_cfg_mrg3_csc_matrix_5(g_p_vout_reg,csc->csc_coef5);
        hal_vout_set_cfg_mrg3_csc_matrix_6(g_p_vout_reg,csc->csc_coef6);
        hal_vout_set_cfg_mrg3_csc_matrix_7(g_p_vout_reg,csc->csc_coef7);
        hal_vout_set_cfg_mrg3_csc_matrix_8(g_p_vout_reg,csc->csc_coef8);
    } else if (layer == XMEDIA_VO_LAYER_V1) {
        hal_vout_set_voutv1_cfg_mrg3_csc_offset_in_2(g_p_vout_reg,csc->csc_offset_in2);
        hal_vout_set_voutv1_cfg_mrg3_csc_offset_in_1(g_p_vout_reg,csc->csc_offset_in1);
        hal_vout_set_voutv1_cfg_mrg3_csc_offset_in_0(g_p_vout_reg,csc->csc_offset_in0);
        hal_vout_set_voutv1_cfg_mrg3_csc_offset_out_2(g_p_vout_reg,csc->csc_offset_out2);
        hal_vout_set_voutv1_cfg_mrg3_csc_offset_out_1(g_p_vout_reg,csc->csc_offset_out1);
        hal_vout_set_voutv1_cfg_mrg3_csc_offset_out_0(g_p_vout_reg,csc->csc_offset_out0);
        hal_vout_set_voutv1_cfg_mrg3_csc_matrix_0(g_p_vout_reg,csc->csc_coef0);
        hal_vout_set_voutv1_cfg_mrg3_csc_matrix_1(g_p_vout_reg,csc->csc_coef1);
        hal_vout_set_voutv1_cfg_mrg3_csc_matrix_2(g_p_vout_reg,csc->csc_coef2);
        hal_vout_set_voutv1_cfg_mrg3_csc_matrix_3(g_p_vout_reg,csc->csc_coef3);
        hal_vout_set_voutv1_cfg_mrg3_csc_matrix_4(g_p_vout_reg,csc->csc_coef4);
        hal_vout_set_voutv1_cfg_mrg3_csc_matrix_5(g_p_vout_reg,csc->csc_coef5);
        hal_vout_set_voutv1_cfg_mrg3_csc_matrix_6(g_p_vout_reg,csc->csc_coef6);
        hal_vout_set_voutv1_cfg_mrg3_csc_matrix_7(g_p_vout_reg,csc->csc_coef7);
        hal_vout_set_voutv1_cfg_mrg3_csc_matrix_8(g_p_vout_reg,csc->csc_coef8);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_mrg_csc(xmedia_vo_layer layer, xmedia_u32 region_id, vo_csc_coef *csc)
{
    if (region_id == 0) {
        hal_vo_set_mrg0_csc(layer, csc);
    }
    else if (region_id == 1) {
        hal_vo_set_mrg1_csc(layer, csc);
    }
    else if (region_id == 2) {
        hal_vo_set_mrg2_csc(layer, csc);
    }
    else if (region_id == 3) {
        hal_vo_set_mrg3_csc(layer, csc);
    } else {
        ;
    }
    return;
}

