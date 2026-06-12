/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#include <string.h>
#include "hal_vo.h"
#include "hal_vo_chip_cfg.h"
#include "hal_vo_reg.h"
#include "hal_vo_disp.h"
#include "hal_vo_intf.h"
#include "hal_vo_clock.h"
#include "hal_vo_gamma.h"
#include "drv_vo_comm.h"

static xmedia_s32 vo_mipi_htotal_adjust_value = 0;
static xmedia_s32 vo_mipi_vsync_adjust_value = 0;

xmedia_void hal_vo_set_mipi_htotal_adjust_value(xmedia_s32 htotal_adjust_value)
{
    vo_mipi_htotal_adjust_value = htotal_adjust_value;
}

xmedia_void hal_vo_set_mipi_vsync_adjust_value(xmedia_s32 vsync_adjust_value)
{
    vo_mipi_vsync_adjust_value = vsync_adjust_value;
}

vo_hal_context vo_hal_ctx;
vo_hal_context *hal_vo_get_hal_context(xmedia_void)
{
    return &vo_hal_ctx;
}

#define vo_reg_addr_offset(REG) ((xmedia_ulong) & (((s_vout_regs_type *)0)->REG))
#define OFFSET_ELE_NUM 2
#ifdef VO_KERNEL
static xmedia_ulong g_vo_reg_offset[][OFFSET_ELE_NUM] = {
    { vo_reg_addr_offset(vout_top_ctrl), 0x320 },
    { vo_reg_addr_offset(vout_lvds_phy_update), 0x50 },
    { vo_reg_addr_offset(vout_v0_ctrl), 0x300 },
    { vo_reg_addr_offset(vout_intf_ctrl), 0x230 },
    { vo_reg_addr_offset(vout_intf_csc_offset_in), 0x70 },
    { vo_reg_addr_offset(vout_v1_ctrl), 0x300 },
    { vo_reg_addr_offset(vout_intf1_ctrl), 0x230 },
    { vo_reg_addr_offset(vout_intf1_csc_offset_in), 0x70 },
};
#endif
#ifdef VO_KERNEL
static xmedia_u8 *g_vo_reg_record = XMEDIA_NULL;
#endif
typedef struct {
    xmedia_u64 gamma_phy_addr;
    xmedia_void *gamma_virtual_addr;
    xmedia_u32 gamma_mem_len;
} vo_dev_gamma_buf;

#define VO_BASE_ADDR 0x11280000
#define VO_MIPI_TX_CTRL_BASE 0x11960000
#define VO_MIPI_TX_PHY_BASE  0x11970000

s_vout_regs_type *g_p_vout_reg = (s_vout_regs_type *)VO_BASE_ADDR;
void *g_p_vout_mipi_tx_ctrl = (void *)VO_MIPI_TX_CTRL_BASE;
void *g_p_vout_mipi_tx_phy = (void *)VO_MIPI_TX_PHY_BASE;

#define VO_MIPI_TX_PHY_SIZE 0x1000
#define VO_FIFO_DEPTH 1919
#define VO_MMZ_NAME_SIZE                    128

#ifdef VO_KERNEL
static vo_dev_gamma_buf g_vo_gamma_buf[XMEDIA_VO_DEV_MAX];
#endif
static xmedia_s32 vo_reg_map(xmedia_void)
{
#ifdef VO_KERNEL
    if (g_p_vout_reg == XMEDIA_NULL) {
        g_p_vout_reg = (s_vout_regs_type *)osal_ioremap_nocache(VO_BASE_ADDR, sizeof(s_vout_regs_type));
        if (g_p_vout_reg == XMEDIA_NULL) {
            return XMEDIA_FAILURE;
        }
    }

    if (g_p_vout_mipi_tx_phy == XMEDIA_NULL) {
        g_p_vout_mipi_tx_phy = osal_ioremap_nocache(VO_MIPI_TX_PHY_BASE, VO_MIPI_TX_PHY_SIZE);
        if (g_p_vout_mipi_tx_phy == XMEDIA_NULL) {
            osal_iounmap((xmedia_void *)g_p_vout_reg);
            g_p_vout_reg = XMEDIA_NULL;
            return XMEDIA_FAILURE;
        }
    }

    if (g_p_vout_mipi_tx_phy_2 == XMEDIA_NULL) {
        g_p_vout_mipi_tx_phy_2 = osal_ioremap_nocache(VO_MIPI_TX_PHY_BASE_2, VO_MIPI_TX_PHY_SIZE);
        if (g_p_vout_mipi_tx_phy_2 == XMEDIA_NULL) {
            osal_iounmap((xmedia_void *)g_p_vout_reg);
            osal_iounmap((xmedia_void *)g_p_vout_mipi_tx_phy);
            g_p_vout_reg = XMEDIA_NULL;
            g_p_vout_mipi_tx_phy = XMEDIA_NULL;
            return XMEDIA_FAILURE;
        }
    }
#endif
    return XMEDIA_SUCCESS;
}

static xmedia_void vo_reg_unmap(xmedia_void)
{
#ifdef VO_KERNEL
    if (g_p_vout_reg != XMEDIA_NULL) {
        osal_iounmap((xmedia_void *)g_p_vout_reg);
        g_p_vout_reg = XMEDIA_NULL;
    }

    if (g_p_vout_mipi_tx_phy != XMEDIA_NULL) {
        osal_iounmap(g_p_vout_mipi_tx_phy);
        g_p_vout_mipi_tx_phy = XMEDIA_NULL;
    }

    if (g_p_vout_mipi_tx_phy_2 != XMEDIA_NULL) {
        osal_iounmap(g_p_vout_mipi_tx_phy_2);
        g_p_vout_mipi_tx_phy_2 = XMEDIA_NULL;
    }
#endif
    return;
}

xmedia_s32 hal_vo_gamma_init(xmedia_vo_dev dev)
{
#ifdef VO_KERNEL
    xmedia_char mmz_user_name[VO_MMZ_NAME_SIZE];
    xmedia_vo_gamma_table gamma_table;

    g_vo_gamma_buf[dev].gamma_mem_len = hal_vo_gamma_get_table_size();
    osal_snprintf(mmz_user_name, sizeof(mmz_user_name), "vo_gamma_dev%u", dev);
    g_vo_gamma_buf[dev].gamma_phy_addr = drv_vo_mmz_alloc(mmz_user_name,
        g_vo_gamma_buf[dev].gamma_mem_len, XMEDIA_NULL);
    if (g_vo_gamma_buf[dev].gamma_phy_addr == XMEDIA_NULL) {
        VO_TRACE(MODULE_DBG_ERR,"drv_vo_mmz_alloc error!\n");
        goto err_out;
    }

    g_vo_gamma_buf[dev].gamma_virtual_addr = drv_vo_mmz_map(g_vo_gamma_buf[dev].gamma_phy_addr,
        g_vo_gamma_buf[dev].gamma_mem_len);
    if (g_vo_gamma_buf[dev].gamma_virtual_addr == XMEDIA_NULL) {
        drv_vo_mmz_free(g_vo_gamma_buf[dev].gamma_phy_addr);
       g_vo_gamma_buf[dev].gamma_phy_addr = XMEDIA_NULL;
        VO_TRACE(MODULE_DBG_ERR,"drv_vo_mmz_map return NULL!\n");
        goto err_out;
    }

    gamma_table.gamma_table[dev] = dev;
    hal_vo_set_gamma_table(dev, &gamma_table);
    return XMEDIA_SUCCESS;
err_out:
    if (g_vo_gamma_buf[dev].gamma_virtual_addr != XMEDIA_NULL) {
        drv_vo_mmz_unmap(g_vo_gamma_buf[dev].gamma_virtual_addr);
        g_vo_gamma_buf[dev].gamma_virtual_addr = XMEDIA_NULL;
    }

    if (g_vo_gamma_buf[dev].gamma_phy_addr != XMEDIA_NULL) {
        drv_vo_mmz_free(g_vo_gamma_buf[dev].gamma_phy_addr);
        g_vo_gamma_buf[dev].gamma_phy_addr = XMEDIA_NULL;
    }
#endif
    return XMEDIA_FAILURE;
}

xmedia_s32 hal_vo_gamma_deinit(xmedia_vo_dev dev)
{
#ifdef VO_KERNEL
    if (g_vo_gamma_buf[dev].gamma_virtual_addr != XMEDIA_NULL) {
        drv_vo_mmz_unmap(g_vo_gamma_buf[dev].gamma_virtual_addr);
        g_vo_gamma_buf[dev].gamma_virtual_addr = XMEDIA_NULL;
    }

    if (g_vo_gamma_buf[dev].gamma_phy_addr != XMEDIA_NULL) {
        drv_vo_mmz_free(g_vo_gamma_buf[dev].gamma_phy_addr);
        g_vo_gamma_buf[dev].gamma_phy_addr = XMEDIA_NULL;
    }
#endif
    return XMEDIA_SUCCESS;
}

xmedia_s32 hal_vo_init(xmedia_void)
{
    xmedia_u32 ret;
    vo_hal_context *hal_ctx = hal_vo_get_hal_context();
    memset(hal_ctx, 0, sizeof(vo_hal_context));

    hal_vo_init_chip_capaciblity();
    ret = vo_reg_map();
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }
    hal_vo_clock_init(&hal_ctx->clock_config);
    return XMEDIA_SUCCESS;
}

xmedia_void hal_vo_deinit(xmedia_void)
{
    //关闭所有的时钟todo
    hal_vo_clock_deinit();
    vo_reg_unmap();
    return;
}

vo_base_cap* hal_vo_get_capacity(xmedia_void)
{
    return hal_vo_get_chip_capacity();
}

vo_layer_cap* hal_get_layer_capacity(xmedia_u32 layer)
{
    return hal_get_layer_chip_capacity(layer);
}

vo_dev_cap* hal_get_dev_capacity(xmedia_vo_dev dev)
{
    return hal_get_dev_chip_capacity(dev);
}

xmedia_void hal_vo_set_clock_gate_enable(xmedia_bool clk_en)
{
    vo_hal_context *hal_ctx = hal_vo_get_hal_context();
    hal_vo_set_clock_gate(&hal_ctx->clock_config, clk_en); 
}

xmedia_void hal_vo_set_crg_clock_reset(xmedia_bool reset_en)
{
    vo_hal_context *hal_ctx = hal_vo_get_hal_context();
    hal_vo_set_clock_reset_enable(&hal_ctx->clock_config, reset_en);
}

xmedia_bool hal_vo_get_crg_clock_reset_state(xmedia_void)
{
    return hal_vo_get_clock_reset_sta();
}

xmedia_void hal_vo_set_clock_sel_priority(xmedia_vo_dev dev, vo_clock_priority_sel vo_clock_priority_sel)
{
    vo_hal_context *hal_ctx = hal_vo_get_hal_context();
    hal_ctx->special_timing[dev].vo_clock_priority_sel = vo_clock_priority_sel;
}

xmedia_s32 hal_vo_set_crg_clock(xmedia_vo_dev dev, xmedia_u32 clk_hz, xmedia_bool clk_reverse_en,
    xmedia_intf_type intf_type)
{
    xmedia_bool vo_lcd_divider_priority = XMEDIA_FALSE;
    vo_hal_context *hal_ctx = hal_vo_get_hal_context();
    xmedia_s32 ret = XMEDIA_SUCCESS;
    //根据时钟判断是否需要使用特殊时序配置
    if (hal_ctx->special_timing[dev].vo_clock_priority_sel == VO_CLOCK_PRIORITY_SEL_LCD) {
        vo_lcd_divider_priority = XMEDIA_TRUE;
    } else if (hal_ctx->special_timing[dev].vo_clock_priority_sel == VO_CLOCK_PRIORITY_SEL_FIX) {
        vo_lcd_divider_priority = XMEDIA_FALSE;
    } else {
        //debug没有设置lcd分频器时，对接口类型做二次判断，lcd/lvds优先使用LCD分频器
        if (intf_type == XMEDIA_INTF_TYPE_LCD || intf_type == XMEDIA_INTF_TYPE_LVDS) {
            vo_lcd_divider_priority = XMEDIA_TRUE;
        }
    }
    ret = hal_vo_set_dev_div_clock(&hal_ctx->clock_config, dev, clk_hz,
        &hal_ctx->special_timing[dev].vo_special_timing_sel, vo_lcd_divider_priority);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }
    ret = hal_vo_set_intf_clock(&hal_ctx->clock_config, dev, clk_reverse_en, intf_type);
    return ret;
}

xmedia_void hal_vo_set_crg_clock_enable(xmedia_vo_dev dev, xmedia_intf_type intf_type, xmedia_bool clk_en)
{
    vo_hal_context *hal_ctx = hal_vo_get_hal_context();
    hal_vo_set_dev_div_clock_enable(&hal_ctx->clock_config, dev, clk_en);
    hal_vo_set_dev_clock_enable(&hal_ctx->clock_config, dev, clk_en);
    hal_vo_set_intf_clock_enable(&hal_ctx->clock_config, intf_type, clk_en);
    return;
}

xmedia_void hal_vo_set_crg_intf_detach_dev(xmedia_vo_dev dev, xmedia_intf_type intf_type)
{
    vo_hal_context *hal_ctx = hal_vo_get_hal_context();
    hal_vo_set_intf_detach_dev(&hal_ctx->clock_config, dev, intf_type);
}

xmedia_void hal_vo_set_top_regup_enable(xmedia_bool enable)
{
    hal_vo_set_top_regup(enable);
    return;
}

xmedia_bool hal_vo_get_top_regup_enable(xmedia_void)
{
    xmedia_bool enable = XMEDIA_FALSE;
    hal_vo_get_top_regup(&enable);
    return enable;
}

xmedia_void hal_vo_set_dev_intf_regup_enable(xmedia_vo_dev dev, xmedia_bool enable)
{
    hal_vo_set_intf_regup(dev, enable);
    return;
}

xmedia_bool hal_vo_get_dev_intf_regup_state(xmedia_vo_dev dev)
{
    xmedia_bool state = XMEDIA_FALSE;
    hal_vo_get_intf_regup(dev, &state);
    return state;
}

xmedia_void hal_vo_set_layer_regup_enable(xmedia_vo_layer layer, xmedia_bool enable)
{
    hal_vo_set_video_regup_enable(layer, enable);
    return;
}

xmedia_bool hal_vo_get_layer_regup_enable(xmedia_vo_layer layer)
{
    xmedia_bool enable = XMEDIA_FALSE;
    hal_vo_get_video_regup(layer, &enable);
    return enable;
}

xmedia_void hal_vo_get_interrupt_state(vo_int_state_type type, xmedia_u32 *state)
{
   if (type == VO_INT_STATE_TYPE_MASK) {
        *state = hal_vo_get_mask_interrupt_state();
   } else if (type == VO_INT_STATE_TYPE_UNMASK) {
        *state = hal_vo_get_unmask_interrupt_state();
   } else {
        *state = hal_vo_get_mask_interrupt_state();
   }
   return;
}

xmedia_void hal_vo_clean_interrupt_state(vo_int_state_type type, vo_isr_interrupt_type state)
{
    if (type == VO_INT_STATE_TYPE_UNMASK) {
        hal_vo_clear_lowband_state(XMEDIA_TRUE);
    } else {
        hal_vo_clear_mask_interrupt_state(state);
    }
    return;
}

xmedia_void hal_vo_set_interrupt_enable(vo_isr_interrupt_type int_type, xmedia_bool enable)
{
    hal_vo_set_mask_interrupt_enable(int_type, enable);
    return;
}

xmedia_void hal_vo_set_int_mode(xmedia_vo_dev dev, xmedia_u32 vtt_id, vo_isr_field_flag int_mode)
{
    xmedia_bool frame_mode;

    if ((int_mode == VO_ISR_FIELD_FLAG_TOP) || (int_mode == VO_ISR_FIELD_FLAG_BOTTOM) ||
       (int_mode == VO_ISR_FIELD_FLAG_TOP_BOTTOM)) {
        frame_mode = XMEDIA_TRUE;
    } else {
        frame_mode = XMEDIA_FALSE;
    }
    hal_vo_set_intf_vtt_mode(dev, vtt_id, frame_mode);
    return;
}

xmedia_void hal_vo_set_dev_thd(xmedia_vo_dev dev, xmedia_u32 vtt_id, xmedia_u32 thd)
{
    //设置中断有效区计数
    hal_vo_set_intf_vtt_sel(dev, vtt_id, 1);
    hal_vo_set_intf_vtt_thd(dev, vtt_id, thd);
    return;
}

xmedia_void hal_vo_get_dev_state(xmedia_vo_dev dev, xmedia_bool *btm, xmedia_u16 *vcnt, xmedia_u8 *vstate)
{
    hal_vo_get_intf_state(dev, btm, vcnt, vstate);
    return;
}

xmedia_void hal_vo_set_dev_mixer_prio(xmedia_vo_dev dev, vo_mix_prio prio_id)
{
    hal_vo_set_cbm_mixer_prio(dev, prio_id);
    return;
}

xmedia_void hal_vo_set_dev_bgcolor(xmedia_vo_dev dev, xmedia_u32 bgcolor)
{
    hal_vo_set_cbm_bgcolor(dev, bgcolor);
    return;
}

xmedia_void hal_vo_get_dev_bgcolor(xmedia_vo_dev dev, xmedia_u32 *bgcolor)
{
    xmedia_u32 color;
    hal_vo_get_cbm_bgcolor(dev, &color);
    *bgcolor = color;
    return;
}

xmedia_void hal_vo_set_dev_csc_en(xmedia_vo_dev dev, xmedia_bool csc_en)
{
    hal_vo_set_intf_csc_en(dev, csc_en);
    return;
}

xmedia_void hal_vo_get_dev_csc_en(xmedia_vo_dev dev, xmedia_bool *csc_en)
{
    hal_vo_get_intf_csc_en(dev, csc_en);
    return;
}

xmedia_s32 hal_vo_set_dev_csc_coef(xmedia_vo_dev dev, vo_pic_info* pic_info)
{
    vo_csc_coef csc_matrix;

    if (hal_vo_csc_coef_convert(pic_info, &csc_matrix) != XMEDIA_SUCCESS) {
        return XMEDIA_FAILURE;
    }
    hal_vo_set_intf_csc(dev, &csc_matrix);

    return XMEDIA_SUCCESS;
}

/*以下关于特殊时序计算均参考 “VOUT_MIPI 时序配置说明.excel”,
 *以下计算得到的值都是非-1 计算，而excel中的寄存器值都是-1运算
 */
static xmedia_void hal_vo_calc_special_timing(xmedia_vo_dev dev, xmedia_u32 frame_rate,
    xmedia_vo_intf_config *intf_config, xmedia_vo_user_sync_timing *sync_timing, mipi_phy_config *mipi_phy_cfg)
{
    xmedia_s32 cycle_n = 0;
    vo_hal_context *hal_ctx = hal_vo_get_hal_context();
    vo_intf_special_timing * special_timing = &hal_ctx->special_timing[dev];
    xmedia_u32 fixed_clock_hz;
    xmedia_s32 htotal_adjust_value;
    xmedia_s32 vsync_adjust_value;

    switch(intf_config->intf_type) {
        case XMEDIA_INTF_TYPE_BT656:
        case XMEDIA_INTF_TYPE_BT1120:
        case XMEDIA_INTF_TYPE_LVDS:
        case XMEDIA_INTF_TYPE_LCD:
            //cycle_n表示单个像素时钟倍数，lcd串行配置为3，并行为1，bt656并行yuv为2
            if ((intf_config->intf_type == XMEDIA_INTF_TYPE_LCD) &&
                (intf_config->lcd_attr.data_mode == XMEDIA_VO_LCD_DATA_MODE_SERIAL)) {
                cycle_n = 3;
            } else if (intf_config->intf_type == XMEDIA_INTF_TYPE_BT656) {
                cycle_n = 2;
            } else {
                cycle_n = 1;
            }
            if (hal_vo_get_intf_clock(&hal_ctx->clock_config, dev, &fixed_clock_hz) != XMEDIA_SUCCESS) {
                return;
            }
            /* =INT(1/I4/(I9+I10+I11+I12)/(1/(I26/I23)/10^6))
             * (1/(frame_rate * (vbb + vfb + vact)))   /   (1/((fixed_clock_Mz/cycle_n) * 10^6))
             * ((fixed_clock_Mz/cycle_n) * 10^6) / (frame_rate * (vbb + vfb + vact))
             * (fixed_clock_hz/cycle_n) / (frame_rate * (vbb + vfb + vact))
             */
            special_timing->fixed_clk_htotal = (fixed_clock_hz / cycle_n) / (frame_rate * (sync_timing->vbb +
                sync_timing->vfb + sync_timing->vact));
            /* (I27-I31-1)/2-1
             * (special_timing->fixed_clk_htotal - sync_timing->hact) / 2
             * 表格运算会出现小数，和逻辑沟通，向下取整忽略小数部分
             */
            special_timing->fixed_clk_hbb = (special_timing->fixed_clk_htotal - sync_timing->hact) / 2;
            /* I27-(I31+1)-(I29+1)-1
             * fixed_clk_hfb - hact - hbb
             */
            special_timing->fixed_clk_hfb = special_timing->fixed_clk_htotal - sync_timing->hact -
                special_timing->fixed_clk_hbb;

            special_timing->vs_st_v_filed = sync_timing->vfb;
            special_timing->vs_end_v_filed = special_timing->vs_st_v_filed + sync_timing->vpw;
            VO_TRACE(MODULE_DBG_DEBUG,
                "fixed_clk_htotal %d fixed_clock_hz %d  fixed_clk_hbb %d fixed_clk_hfb %d\n",
                special_timing->fixed_clk_htotal, fixed_clock_hz,
                special_timing->fixed_clk_hbb, special_timing->fixed_clk_hfb);
            VO_TRACE(MODULE_DBG_DEBUG, "vs_st_v_filed %d vs_end_v_filed %d\n",
                special_timing->vs_st_v_filed, special_timing->vs_end_v_filed);
             break;
        case XMEDIA_INTF_TYPE_MIPI_DSI:
            if (hal_vo_get_intf_clock(&hal_ctx->clock_config, dev, &fixed_clock_hz) != XMEDIA_SUCCESS) {
                return;
            }
            htotal_adjust_value = (vo_mipi_htotal_adjust_value != 0) ? vo_mipi_htotal_adjust_value :
                mipi_phy_cfg->phy_htotal_adjust;
            vsync_adjust_value = (vo_mipi_vsync_adjust_value != 0) ? vo_mipi_vsync_adjust_value :
                mipi_phy_cfg->phy_vsync_adjust;
            /*
             * INT(D20/D15*D26) + FIXCLOCK_HTOTAL_AJUST_VALUE
             * hal_ctx->mipi_phy_cfg.phy_vid_hline_time * fixed_clock_Mhz / phy_lanebyte_clock_hzMhz + AJUST
             * hal_ctx->mipi_phy_cfg.phy_vid_hline_time * fixed_clock_hz / phy_lanebyte_clock_hzhz + AJUST
             * hal_ctx->mipi_phy_cfg.phy_vid_hline_time * (fixed_clock_hz/1000) / (phy_lanebyte_clock_hzhz/1000) + AJUST
             */
            special_timing->fixed_clk_htotal = (hal_ctx->mipi_phy_cfg.phy_vid_hline_time * (fixed_clock_hz/1000)) /
                (hal_ctx->mipi_phy_cfg.phy_lanebyte_clock_hz/1000) + htotal_adjust_value;

            // = D7 => hbp => hbb - hpw
            special_timing->fixed_clk_hbb = sync_timing->hbb - sync_timing->hpw;

            // D29-(D33+1)-(D31+1) => fixed_clk_htotal - vact - fixed_clk_hbb
            special_timing->fixed_clk_hfb = special_timing->fixed_clk_htotal - sync_timing->hact -
                special_timing->fixed_clk_hbb;

            // D34 + 1 + D28 => vfb + 1 + FIXCLOCK_VSYNC_AJUST_VALUE
            special_timing->vs_st_v_filed = sync_timing->vfb + 1 + vsync_adjust_value - 1;

            // D39 + D21 => vs_st_v_filed + vpw
            special_timing->vs_end_v_filed = special_timing->vs_st_v_filed + sync_timing->vpw;

            VO_TRACE(MODULE_DBG_DEBUG,
                "fixed_clk_htotal %d phy_vid_hline_time %d fixed_clock_hz %d phy_lanebyte_clock_hz %d! fixed_clk_hbb %d fixed_clk_hfb %d\n",
                special_timing->fixed_clk_htotal, hal_ctx->mipi_phy_cfg.phy_vid_hline_time, fixed_clock_hz,
                hal_ctx->mipi_phy_cfg.phy_lanebyte_clock_hz, special_timing->fixed_clk_hbb, special_timing->fixed_clk_hfb);
            VO_TRACE(MODULE_DBG_DEBUG, "vs_st_v_filed %d vs_end_v_filed %d\n",
                special_timing->vs_st_v_filed, special_timing->vs_end_v_filed);
            break;
        default:
            break;
    }
}

xmedia_void hal_vo_set_dev_intf_timing(xmedia_vo_dev dev, xmedia_u32 frame_rate,
                                              xmedia_vo_intf_config *intf_config, xmedia_vo_user_sync_timing *sync_timing)
{

    vo_hal_context *hal_ctx = hal_vo_get_hal_context();
#ifdef VO_KERNEL
    ddr_ost_info ost_value;
#endif
    hal_vo_set_intf_cnt_pos(dev);

    if (hal_ctx->special_timing[dev].vo_special_timing_sel) {
        hal_vo_calc_special_timing(dev, frame_rate, intf_config, sync_timing, &hal_ctx->mipi_phy_cfg);
    }
    hal_vo_set_intf_timing(dev, frame_rate, intf_config, sync_timing, &hal_ctx->special_timing[dev]);
    hal_vo_set_intf_fifo(dev, VO_FIFO_DEPTH);
#ifdef VO_KERNEL
    drv_ddr_get_ost(XMEDIA_MOD_ID_VO, &ost_value);
#endif

    hal_vo_set_dma_standing(dev, 8);

    switch(intf_config->intf_type){
        case XMEDIA_INTF_TYPE_BT656:
             hal_vo_set_bt656_intf_cfg(dev, &(intf_config->bt656_attr));
             break;
        case XMEDIA_INTF_TYPE_BT1120:
             hal_vo_set_bt1120_intf_cfg(dev, &(intf_config->bt1120_attr));
             break;
        case XMEDIA_INTF_TYPE_LCD:
             hal_vo_set_lcd_intf_cfg(dev, &(intf_config->lcd_attr));
             break;
        case XMEDIA_INTF_TYPE_MIPI_DSI:
             hal_vo_set_mipi_intf_cfg(dev, &(intf_config->mipi_attr));
             break;
        case XMEDIA_INTF_TYPE_LVDS:
             hal_vo_set_lvds_intf_cfg(dev, &(intf_config->lvds_attr));
             break;
        default:
            break;
    }

    //intf 时钟相关绑定及门控 在前级set crg中做了
    return;
}



xmedia_void hal_vo_set_dev_intf_enable(xmedia_vo_dev dev, xmedia_bool enable)
{
    xmedia_bool intf_en[XMEDIA_VO_DEV_MAX];
    xmedia_vo_dev i;

    hal_vo_set_intf_enable(dev, enable);

    for (i = 0; i < XMEDIA_VO_DEV_MAX; i++) {
        hal_vo_get_intf_enable(i, &intf_en[i]);
    }
    return;
}

xmedia_void hal_vo_get_dev_intf_enable(xmedia_vo_dev dev, xmedia_bool *enable)
{
    hal_vo_get_intf_enable(dev, enable);
    return;
}

xmedia_void hal_vo_set_layer_enable(xmedia_vo_layer layer, xmedia_bool enable)
{
    hal_vo_set_enable(layer, enable);
    return;
}

xmedia_void hal_vo_get_layer_enable(xmedia_vo_layer layer, xmedia_bool *enable)
{
    hal_vo_get_enable(layer,enable);
    return;
}

xmedia_void hal_vo_set_layer_bg_color(xmedia_vo_layer layer, xmedia_u32 bgcolor)
{
    hal_vo_set_bgcolor(layer, bgcolor);
    return;
}

xmedia_void hal_vo_set_layer_mute_enable(xmedia_vo_layer layer, xmedia_bool enable)
{
    hal_vo_set_mute_enable(layer, enable);
    return;
}

xmedia_void hal_vo_set_layer_mute_color(xmedia_vo_layer layer, xmedia_u32 bgcolor)
{
    hal_vo_set_mute_enable(layer, bgcolor);
    return;
}

xmedia_void hal_vo_set_layer_csc_en(xmedia_vo_layer layer, xmedia_bool csc_en)
{
    hal_vo_set_csc_en(layer, csc_en);
    return;
}

xmedia_s32 hal_vo_set_layer_csc_coef(xmedia_vo_layer layer, vo_pic_info* pic_info)
{
    vo_csc_coef csc_matrix;
    xmedia_bool multi_en;

    hal_vo_set_alpha(layer, 0xFF);

    if (hal_vo_csc_coef_convert(pic_info, &csc_matrix) != XMEDIA_SUCCESS) {
        return XMEDIA_FAILURE;
    }

    hal_vo_get_multi_enable(layer, &multi_en);
    if (multi_en == XMEDIA_TRUE) {
        hal_vo_set_mrg0_csc(layer, &csc_matrix);
        hal_vo_set_mrg1_csc(layer, &csc_matrix);
        hal_vo_set_mrg2_csc(layer, &csc_matrix);
        hal_vo_set_mrg3_csc(layer, &csc_matrix);
    } else {
        //单区域的csc复用多区域0的csc
        hal_vo_set_csc(layer, &csc_matrix);
    }
    return XMEDIA_SUCCESS;
}

xmedia_void hal_vo_set_layer_disp_rect(xmedia_vo_layer layer, xmedia_video_rect *disp_rect)
{
    hal_vo_set_disp_rect(layer, disp_rect);
    return;
}

xmedia_void hal_vo_get_layer_disp_rect(xmedia_vo_layer layer, xmedia_video_rect *disp_rect)
{
    hal_vo_get_disp_rect(layer, disp_rect);
    return;
}

//单区域模式 设定真实内容相对于层的rect
xmedia_void hal_vo_set_layer_reso_rect(xmedia_vo_layer layer, xmedia_video_rect *reso_rect)
{
    hal_vo_set_src_rect(layer, reso_rect);
    return;
}

xmedia_void hal_vo_get_layer_reso_rect(xmedia_vo_layer layer, xmedia_video_rect *reso_rect)
{
    hal_vo_get_src_rect(layer, reso_rect);
    return;
}

xmedia_void hal_vo_set_layer_pixel_format(xmedia_vo_layer layer, xmedia_video_pixel_format pix_format)
{
    vo_pixel_format format = VO_PIXEL_FMT_YUV_420;
    xmedia_bool uv_order = 0;

    //注意下逻辑的排序与应用理解的UV是方向的
    if (pix_format == XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_420) {
        format = VO_PIXEL_FMT_YUV_420;
        uv_order = 1;
    } else if (pix_format == XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420) {
        format = VO_PIXEL_FMT_YUV_420;
        uv_order = 0;
    } else if (pix_format == XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_422) {
        format = VO_PIXEL_FMT_YUV_422;
        uv_order = 1;
    } else if (pix_format == XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_422) {
        format = VO_PIXEL_FMT_YUV_422;
        uv_order = 0;
    } else if (pix_format == XMEDIA_VIDEO_PIXEL_FMT_YUV_400) {
        format = VO_PIXEL_FMT_YUV_400;
        uv_order = 0;
    } else {
        format = VO_PIXEL_FMT_YUV_420;
        uv_order = 1;
    }
    hal_vo_set_src_pixel_format(layer, format, uv_order);
    return;
}


//单区域和多区域控制
xmedia_void hal_vo_set_layer_multi_enable(xmedia_vo_layer layer, xmedia_bool enable)
{
    hal_vo_set_multi_enable(layer, enable);
    return;
}

xmedia_void hal_vo_get_layer_multi_enable(xmedia_vo_layer layer, xmedia_bool *enable)
{
    hal_vo_get_multi_enable(layer, enable);
    return;
}

xmedia_void hal_vo_set_layer_region_enable(xmedia_vo_layer layer, xmedia_u32 region_id, xmedia_bool enable)
{
    hal_vo_set_mrg_enable(layer, region_id, enable);
    return;
}

xmedia_void hal_vo_get_layer_region_enable(xmedia_vo_layer layer, xmedia_u32 region_id, xmedia_bool *enable)
{
    hal_vo_get_mrg_enable(layer, region_id, enable);
    return;
}

xmedia_void hal_vo_set_layer_region_mrg_enable(xmedia_vo_layer layer, xmedia_u32 region_id,
    xmedia_bool enable)
{
    hal_vo_set_mrg_enable(layer, region_id, enable);
    return;
}

xmedia_void hal_vo_set_layer_region_mute_color(xmedia_vo_layer layer, xmedia_u32 region_id,
    xmedia_u32 bgcolor)
{
    hal_vo_set_mrg_mute_color(layer, region_id, bgcolor);
    return;
}

xmedia_void hal_vo_set_layer_single_region_cfg(xmedia_vo_layer layer, vo_layer_single_cfg *single_cfg)
{
    //真实内容 相对于显示窗口的rect
    hal_vo_set_src_rect(layer, &(single_cfg->frame_rect));
    //层 相对于显示窗口的rect
    hal_vo_set_dst_rect(layer, &(single_cfg->video_rect));
    hal_vo_set_addr(layer, &(single_cfg->frame_addr));
    hal_vo_set_mute_enable(layer, single_cfg->mute_en);
    hal_vo_set_mrg_rdma1_en(layer, XMEDIA_FALSE);
    return;
}

xmedia_void hal_vo_set_layer_multi_region_cfg(xmedia_vo_layer layer, vo_layer_multi_cfg *multi_cfg)
{
    xmedia_u32 id;

    id = multi_cfg->region_id;
    hal_vo_set_mrg_enable(layer, id, multi_cfg->region[id].enable);
    hal_vo_set_mrg_src_rect(layer,id, &(multi_cfg->region[id].src_rect));
    hal_vo_set_mrg_dst_rect(layer,id, &(multi_cfg->region[id].dst_rect));
    hal_vo_set_mrg_addr(layer,id, &(multi_cfg->region[id].region_addr));

    if (multi_cfg->region_overlap) {
        hal_vo_set_mrg_rdma1_en(layer, XMEDIA_TRUE);
        //rdma0+rdma1 rdma1绑定优先级最高的
        hal_vo_set_mrg_mixer_prio(layer, VO_MRG_MIX_PRIO_RDMA0);
        hal_vo_set_mrg_rdma1_link(layer, multi_cfg->overlap_id);
    } else {
        hal_vo_set_mrg_rdma1_en(layer, XMEDIA_FALSE);
        hal_vo_set_mrg_mixer_prio(layer, VO_MRG_MIX_PRIO_RDMA0);
    }
    //sort的配置会影响画面的输出
    hal_vo_set_mrg_sort(layer, multi_cfg->region_sort);

    return;
}

xmedia_void hal_vo_set_dev_debug_pattern(xmedia_vo_dev dev, vo_pattern_info *pattern_info)
{
    hal_vo_set_intf_mute_enable(dev, pattern_info->pattern_en);
    hal_vo_set_intf_mute_pattern_mode(dev, pattern_info->pattern_mode);
    hal_vo_set_intf_mute_data_mode(dev, pattern_info->data_mode);
    hal_vo_set_intf_mute_color(dev, pattern_info->color, pattern_info->color);
    hal_vo_set_intf_cbar_sel(dev, pattern_info->rgb);
    if (pattern_info->pattern_mode == VO_PATTERN_MODE_CHECKER) {
        hal_vo_set_intf_checkbar_size(dev, pattern_info->width, pattern_info->height);
        hal_vo_set_intf_mute_color(dev, pattern_info->color, !(pattern_info->color));
    }
    return;
}

xmedia_void hal_vo_set_layer_debug_pattern(xmedia_vo_layer layer, vo_pattern_info *pattern_info)
{
    vo_pattern_mode mode = VO_PATTERN_MODE_FULLCOLOR;

    mode = pattern_info->pattern_mode;
    if ((mode != VO_PATTERN_MODE_FULLCOLOR)  && (mode != VO_PATTERN_MODE_CHECKER)) {
        mode = VO_PATTERN_MODE_FULLCOLOR;
    }
    hal_vo_set_mute_enable(layer, pattern_info->pattern_en);
    hal_vo_set_mute_pattern_mode(layer, pattern_info->pattern_mode);
    hal_vo_set_mute_data_mode(layer, pattern_info->data_mode);
    hal_vo_set_mute_color(layer, pattern_info->color);
    if (mode == VO_PATTERN_MODE_CHECKER) {
        hal_vo_set_checkbar_size(layer, pattern_info->width, pattern_info->height);
    }
    return;
}

xmedia_void hal_vo_set_debug_ink(xmedia_vo_dev dev, vo_csc_ink_info *ink_info)
{
    hal_vo_set_intf_ink(dev, ink_info);
    return;
}

xmedia_void hal_vo_get_debug_checksum(xmedia_vo_dev dev, xmedia_u32 *y, xmedia_u32 *u, xmedia_u32 *v)
{
    hal_vo_get_intf_checksum_value(dev, y, u,v);
    return;
}

xmedia_s32 hal_vo_enable_dev_phy_cfg(xmedia_vo_dev dev, xmedia_u32 clk_hz, xmedia_u32 frame_rate,
                                            xmedia_vo_user_sync_timing *sync_timing,
                                            xmedia_intf_type intf_type)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    vo_hal_context * hal_ctx = hal_vo_get_hal_context();

    switch(intf_type){
        case XMEDIA_INTF_TYPE_MIPI_DSI:
            ret = hal_vo_set_mipi_tx_phy_cfg(dev, frame_rate, sync_timing, &hal_ctx->mipi_phy_cfg,
                hal_ctx->special_timing[dev].vo_special_timing_sel);
            break;
        case XMEDIA_INTF_TYPE_LVDS:
            ret = hal_vo_enable_lvds_phy_cfg(clk_hz);
            break;
        case XMEDIA_INTF_TYPE_BT656:
        case XMEDIA_INTF_TYPE_BT1120:
        case XMEDIA_INTF_TYPE_LCD:
        default:
            break;
    }
    return ret;
}

xmedia_void hal_vo_disable_dev_phy_cfg(xmedia_intf_type intf_type)
{
    switch(intf_type){
    case XMEDIA_INTF_TYPE_MIPI_DSI:
        hal_vo_set_mipi_tx_phy_reset();
        break;
    case XMEDIA_INTF_TYPE_LVDS:
        hal_vo_disable_lvds_phy_cfg();
        break;
    case XMEDIA_INTF_TYPE_BT656:
    case XMEDIA_INTF_TYPE_BT1120:
    case XMEDIA_INTF_TYPE_LCD:
    default:
        break;
    }
    return;
}

xmedia_s32 vo_calc_dev_phy_clk_cfg(xmedia_vo_dev dev, xmedia_u32 clk_hz, xmedia_vo_intf_config *intf_config)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    vo_hal_context *hal_ctx = hal_vo_get_hal_context();

    switch(intf_config->intf_type){
        case XMEDIA_INTF_TYPE_MIPI_DSI:
            ret = vo_calculate_mipi_phy_cfg(dev, clk_hz, &intf_config->mipi_attr, &hal_ctx->mipi_phy_cfg);
            break;
        case XMEDIA_INTF_TYPE_LVDS:
        case XMEDIA_INTF_TYPE_BT656:
        case XMEDIA_INTF_TYPE_BT1120:
        case XMEDIA_INTF_TYPE_LCD:
        default:
            break;
    }
    return ret;
}

#ifdef VO_KERNEL
xmedia_void hal_vo_set_gamma_addr(xmedia_vo_dev dev)
{
    hal_vo_gamma_set_addr(dev, g_vo_gamma_buf[dev].gamma_phy_addr);
}

xmedia_void hal_vo_set_gamma_enable(xmedia_vo_dev dev, xmedia_bool enable)
{
    hal_vo_gamma_enable(dev, enable);
}

xmedia_void hal_vo_set_gamma_table(xmedia_vo_dev dev, xmedia_vo_gamma_table *gamma_table)
{
    hal_vo_gamma_set_table(dev, g_vo_gamma_buf[dev].gamma_virtual_addr, g_vo_gamma_buf[dev].gamma_mem_len, gamma_table);
}

xmedia_void hal_vo_get_gamma_table(xmedia_vo_dev dev, xmedia_vo_gamma_table *gamma_table)
{
    hal_vo_gamma_get_table(dev, g_vo_gamma_buf[dev].gamma_virtual_addr, g_vo_gamma_buf[dev].gamma_mem_len, gamma_table);
}

xmedia_s32 hal_vo_reg_record(xmedia_void)
{

    xmedia_u32 idx, section_num;
    xmedia_ulong offset = 0;
    xmedia_u32 malloc_size = 0;

    section_num = sizeof(g_vo_reg_offset) / sizeof(xmedia_ulong) / OFFSET_ELE_NUM;

    for (idx = 0; idx < section_num; idx++) {
        malloc_size += g_vo_reg_offset[idx][1];
    }

    if (g_vo_reg_record != XMEDIA_NULL) {
        VO_TRACE(MODULE_DBG_ERR,"g_vo_reg_record is not null,please check!\n");
        return XMEDIA_FAILURE;
    } else {
        g_vo_reg_record = (xmedia_u8 *)osal_kmalloc(malloc_size, osal_gfp_kernel);
    }
    for (idx = 0; idx < section_num; idx++) {
        osal_memcpy((xmedia_void *)((xmedia_uintptr_t)g_vo_reg_record + offset),
            (xmedia_void *)((xmedia_uintptr_t)g_p_vout_reg + g_vo_reg_offset[idx][0]), g_vo_reg_offset[idx][1]);
            offset += g_vo_reg_offset[idx][1];
    }
    return XMEDIA_SUCCESS;
}

xmedia_s32 hal_vo_reg_restore(xmedia_void)
{
    xmedia_u32 idx;
    xmedia_u32 section_num;
    xmedia_ulong offset = 0;

    if (g_vo_reg_record == XMEDIA_NULL) {
        VO_TRACE(MODULE_DBG_ERR,"g_vo_reg_record is null,please check!\n");
        return XMEDIA_FAILURE;
    }
    section_num = sizeof(g_vo_reg_offset) / sizeof(xmedia_ulong) / OFFSET_ELE_NUM;

    for (idx = 0; idx < section_num; idx++) {
        osal_memcpy((xmedia_void *)(xmedia_uintptr_t)((xmedia_ulong)(xmedia_uintptr_t)g_p_vout_reg + g_vo_reg_offset[idx][0]),
            (xmedia_void *)(xmedia_uintptr_t)((xmedia_ulong)(xmedia_uintptr_t)g_vo_reg_record + offset),
            g_vo_reg_offset[idx][1]);
        offset += g_vo_reg_offset[idx][1];
    }
    osal_kfree(g_vo_reg_record);
    g_vo_reg_record = XMEDIA_NULL;
    return XMEDIA_SUCCESS;
}
#endif
