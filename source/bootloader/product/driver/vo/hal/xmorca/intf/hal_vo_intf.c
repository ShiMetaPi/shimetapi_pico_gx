/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include "hal_vo.h"
#include "hal_vo_intf.h"
#include "hal_vo_reg.h"
#include "hal_vo_clock.h"
#ifdef VO_BOOT
#include "drv_vo_comm.h"
#endif
#ifdef VO_KERNEL
#include "osal.h"
#endif
extern s_vout_regs_type *g_p_vout_reg;
extern vo_dts_lcd_config g_vo_dts_lcd_config[VO_DTS_LCD_TYPE_MAX];
#define MCU_CMD_PACKAGE_MAX_SIZE (64 * 4)

#define VO_REGUP_POS_VALUE 1
#define VO_PARA_POS_VALUE  2
#define VO_PREFETCH_POS_VALUE 3
#define VO_START_POS_VALUE 4

xmedia_void hal_vo_set_intf_fifo(xmedia_vo_dev dev, xmedia_u32 fifo)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_cfg_intf_sfifo_thd(g_p_vout_reg, fifo);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_enable(xmedia_vo_dev dev, xmedia_bool enable)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_cfg_chk_sum_en(g_p_vout_reg, enable);
        hal_reg_vout_set_cfg_intf_en(g_p_vout_reg, enable);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_get_intf_enable(xmedia_vo_dev dev, xmedia_bool *enable)
{
    if (dev == XMEDIA_VO_DEV_0) {
        *enable = hal_reg_vout_get_cfg_intf_en(g_p_vout_reg);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_chk_sum_enable(xmedia_vo_dev dev, xmedia_bool enable)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_cfg_chk_sum_en(g_p_vout_reg, enable);
    } else {
        ;
    }
    return;
}

//for mipi
xmedia_void hal_vo_set_intf_hs_mode(xmedia_vo_dev dev, vo_intf_hs_mode hs_mode)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_cfg_hs_mode(g_p_vout_reg, hs_mode);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_mux(xmedia_vo_dev dev, xmedia_intf_type intf)
{
    if (dev == XMEDIA_VO_DEV_0) {
        switch (intf) {
           case XMEDIA_INTF_TYPE_LCD:
               hal_reg_vout_set_cfg_intf_sel(g_p_vout_reg, 1);
               break;
           case XMEDIA_INTF_TYPE_BT656:
           case XMEDIA_INTF_TYPE_BT1120:
               hal_reg_vout_set_cfg_intf_sel(g_p_vout_reg, 0);
               if (intf == XMEDIA_INTF_TYPE_BT1120) {
                   hal_reg_vout_set_cfg_bt_mode(g_p_vout_reg, 1);
               } else {
                   hal_reg_vout_set_cfg_bt_mode(g_p_vout_reg, 0);
               }
               break;
           case XMEDIA_INTF_TYPE_MCU:
               hal_reg_vout_set_cfg_intf_sel(g_p_vout_reg, 4);
               break;
           default:
               break;
       }
    } else {
    }
    return ;
}

xmedia_void hal_vo_set_intf_regup(xmedia_vo_dev dev, xmedia_bool enable)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_intfupd_cfg_regup_en(g_p_vout_reg, enable);
    }
    return;
}

//默认从消隐区开始计数
xmedia_void hal_vo_set_intf_cnt_pos(xmedia_vo_dev dev)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_cfg_regup_pos(g_p_vout_reg, VO_REGUP_POS_VALUE);
        hal_reg_vout_set_cfg_para_pos(g_p_vout_reg, VO_PARA_POS_VALUE);
        hal_reg_vout_set_cfg_prefetch_pos(g_p_vout_reg, VO_PREFETCH_POS_VALUE);
        hal_reg_vout_set_cfg_start_pos(g_p_vout_reg, VO_START_POS_VALUE);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_timing(xmedia_vo_dev dev, xmedia_u32 frame_rate,
    xmedia_vo_intf_config *intf_config, xmedia_vo_user_sync_timing *sync_timing,
    vo_intf_special_timing *special_timing)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_cfg_interlaced(g_p_vout_reg, !(sync_timing->iop)); //逐隔行 配置
        hal_reg_vout_set_cfg_de_inv(g_p_vout_reg,sync_timing->idv);         //数据有效信号的极性
        hal_reg_vout_set_cfg_hs_inv(g_p_vout_reg,sync_timing->ihs);         //水平有效信号的极性
        hal_reg_vout_set_cfg_vs_inv(g_p_vout_reg,sync_timing->ivs);         //垂直有效信号的极性
        if (sync_timing->iop == 0) {
            hal_reg_vout_set_cfg_vact(g_p_vout_reg, sync_timing->vact * 2 - 1);//垂直有效区
        } else {
            hal_reg_vout_set_cfg_vact(g_p_vout_reg, sync_timing->vact - 1);
        }
        hal_reg_vout_set_cfg_vbb(g_p_vout_reg,sync_timing->vbb - 1);        //垂直消隐后肩
        hal_reg_vout_set_cfg_vfb(g_p_vout_reg,sync_timing->vfb - 1);        //垂直消隐前肩
        hal_reg_vout_set_cfg_hact(g_p_vout_reg,sync_timing->hact - 1);      //水平有效区
        hal_reg_vout_set_cfg_bvbb(g_p_vout_reg,sync_timing->bvbb - 1);      //底场垂直消隐后肩
        hal_reg_vout_set_cfg_bvfb(g_p_vout_reg,sync_timing->bvfb - 1);      //底场垂直消隐前肩
        hal_reg_vout_set_cfg_hs_width(g_p_vout_reg,sync_timing->hpw);       //水平脉冲宽度
        if (special_timing->vo_special_timing_sel) {
             hal_reg_vout_set_cfg_hbb(g_p_vout_reg,special_timing->fixed_clk_hbb - 1);        //水平消隐后肩
            hal_reg_vout_set_cfg_hfb(g_p_vout_reg,special_timing->fixed_clk_hfb - 1);        //水平消隐前肩
            //vsync lcd mipi lvds 都需要使用，vsync的起始点和结束点
            hal_reg_vout_set_cfg_vs_st_h_field0(g_p_vout_reg, special_timing->fixed_clk_hfb - 1);
            hal_reg_vout_set_cfg_vs_end_h_field0(g_p_vout_reg, special_timing->fixed_clk_hfb - 1);
            hal_reg_vout_set_cfg_vs_st_v_field0(g_p_vout_reg, special_timing->vs_st_v_filed);
            hal_reg_vout_set_cfg_vs_end_v_field0(g_p_vout_reg, special_timing->vs_end_v_filed);
        } else {
            hal_reg_vout_set_cfg_hbb(g_p_vout_reg,sync_timing->hbb - 1);        //水平消隐后肩
            hal_reg_vout_set_cfg_hfb(g_p_vout_reg,sync_timing->hfb - 1);        //水平消隐前肩
            //vsync lcd mipi lvds 都需要使用，vsync的起始点和结束点
            hal_reg_vout_set_cfg_vs_st_h_field0(g_p_vout_reg, sync_timing->hbb - 1);
            hal_reg_vout_set_cfg_vs_end_h_field0(g_p_vout_reg, sync_timing->hbb - 1);
            hal_reg_vout_set_cfg_vs_st_v_field0(g_p_vout_reg, sync_timing->vfb);
            hal_reg_vout_set_cfg_vs_end_v_field0(g_p_vout_reg, sync_timing->vfb + sync_timing->vpw);
        }

        hal_reg_vout_set_cfg_field_tog(g_p_vout_reg, 0x0);  //场模式用的  暂时不管
        hal_reg_vout_set_cfg_bfield_tog(g_p_vout_reg, 0x0); //场模式用的  暂时不管
        hal_reg_vout_set_cfg_vs_st_h_field1(g_p_vout_reg, 0x0);     //场模式用的  暂时不管
        hal_reg_vout_set_cfg_vs_end_h_field1(g_p_vout_reg, 0x0);    //场模式用的  暂时不管
        hal_reg_vout_set_cfg_vs_st_v_field1(g_p_vout_reg, 0x0);     //场模式用的  暂时不管
        hal_reg_vout_set_cfg_vs_end_v_field1(g_p_vout_reg, 0x0);    //场模式用的  暂时不管

        /*
         * 目前还有以下参数没用到
         *  xmedia_u16 hmid;  //底场同步水平有效区域
         *  xmedia_bool synm; //同步模式(0 表示比如BT656时序，1表示像LCD那样的信号)
         *  xmedia_u8 intfb;  //隔行输出的位宽y
         */
    } else {
        ;
    }
   return;
}

//主要给LCD用，BT用内同步
xmedia_void hal_vo_set_intf_sync_inverse(xmedia_vo_dev dev)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_cfg_field_tog(g_p_vout_reg, 0x0); //(0->vbb+bvbb+1)
        hal_reg_vout_set_cfg_bfield_tog(g_p_vout_reg, 0x0);
        hal_reg_vout_set_cfg_vs_st_h_field0(g_p_vout_reg, 0x0);
        hal_reg_vout_set_cfg_vs_end_h_field0(g_p_vout_reg, 0x1);
        hal_reg_vout_set_cfg_vs_st_v_field0(g_p_vout_reg, 0x0); //行翻转点起始
        hal_reg_vout_set_cfg_vs_end_v_field0(g_p_vout_reg, 0x1); //(0,vfb+vbb+1)
        hal_reg_vout_set_cfg_vs_st_v_field1(g_p_vout_reg, 0x0);
        hal_reg_vout_set_cfg_vs_end_v_field1(g_p_vout_reg, 0x1);
        hal_reg_vout_set_cfg_vs_st_h_field1(g_p_vout_reg, 0x0);
        hal_reg_vout_set_cfg_vs_end_h_field1(g_p_vout_reg, 0x1);
    } else {
        ;
    }
    return;
}

//垂直中断起始行配置：有效区或者消隐区计数
xmedia_void hal_vo_set_intf_vtt_sel(xmedia_vo_dev dev, xmedia_u32 vtt_id, xmedia_bool cnt_mode)
{
    if (dev == XMEDIA_VO_DEV_0) {
        if (vtt_id == 0) {
            hal_reg_vout_set_cfg_vt_int_cnt_sel_0(g_p_vout_reg, cnt_mode);
        }
        if (vtt_id == 1) {
           hal_reg_vout_set_cfg_vt_int_cnt_sel_1(g_p_vout_reg, cnt_mode);
        }
        if (vtt_id == 2) {
            hal_reg_vout_set_cfg_vt_int_cnt_sel_2(g_p_vout_reg, cnt_mode);
        }
        if (vtt_id == 3) {
            hal_reg_vout_set_cfg_vt_int_cnt_sel_3(g_p_vout_reg, cnt_mode);
        }
    } else {
        ;
    }
    return;
}

//帧模式还是场模式
xmedia_void hal_vo_set_intf_vtt_mode(xmedia_vo_dev dev, xmedia_u32 vtt_id, xmedia_bool frame_mode)
{
    if (dev == XMEDIA_VO_DEV_0) {
        if (vtt_id == 0) {
            hal_reg_vout_set_cfg_vt_int_mode_0(g_p_vout_reg, frame_mode);
        }
        if (vtt_id == 1) {
            hal_reg_vout_set_cfg_vt_int_mode_1(g_p_vout_reg, frame_mode);
        }
        if (vtt_id == 2) {
            hal_reg_vout_set_cfg_vt_int_mode_2(g_p_vout_reg, frame_mode);
        }
        if (vtt_id == 3) {
            hal_reg_vout_set_cfg_vt_int_mode_3(g_p_vout_reg, frame_mode);
        }
    } else {
        ;
    }
    return;
}

//中断阈值
xmedia_void hal_vo_set_intf_vtt_thd(xmedia_vo_dev dev, xmedia_u32 vtt_id, xmedia_u32 vtt_thd)
{
    if (dev == XMEDIA_VO_DEV_0) {
        if (vtt_id == 0) {
            hal_reg_vout_set_cfg_vt_int_thd_0(g_p_vout_reg, vtt_thd);
        }
        if (vtt_id == 1) {
            hal_reg_vout_set_cfg_vt_int_thd_1(g_p_vout_reg, vtt_thd);
        }
        if (vtt_id == 2) {
            hal_reg_vout_set_cfg_vt_int_thd_2(g_p_vout_reg, vtt_thd);
        }
        if (vtt_id == 3) {
            hal_reg_vout_set_cfg_vt_int_thd_3(g_p_vout_reg, vtt_thd);
        }
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_get_intf_state(xmedia_vo_dev dev, xmedia_bool *btm, xmedia_u16 *vcnt, xmedia_u8 *vstate)
{
    if (dev == XMEDIA_VO_DEV_0) {
        *btm = hal_reg_vout_get_field_flag_ppc(g_p_vout_reg);
        *vstate = hal_reg_vout_get_vstate(g_p_vout_reg);
        if (hal_reg_vout_get_cfg_vt_int_cnt_sel_0(g_p_vout_reg) == 0) {
            *vcnt = hal_reg_vout_get_vcnt_st_vblk(g_p_vout_reg);
        } else if (hal_reg_vout_get_cfg_vt_int_cnt_sel_0(g_p_vout_reg) == 1) {
            *vcnt = hal_reg_vout_get_vcnt_st_vact(g_p_vout_reg);
        } else {
            *vcnt = hal_reg_vout_get_vcnt_st_vblk(g_p_vout_reg);
        }
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_get_intf_checksum_value(xmedia_vo_dev dev, xmedia_u32 *y, xmedia_u32 *u, xmedia_u32 *v)
{
    if (dev == XMEDIA_VO_DEV_0) {
        *y = hal_reg_vout_get_intf_y_sum(g_p_vout_reg);
        *u = hal_reg_vout_get_intf_u_sum(g_p_vout_reg);
        *v = hal_reg_vout_get_intf_v_sum(g_p_vout_reg);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_bt_mode(xmedia_vo_dev dev, xmedia_bool value)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_cfg_bt_mode(g_p_vout_reg, value);
    } else {
        ;
    }
    return ;
}

xmedia_void hal_vo_set_intf_bt_yc_order(xmedia_vo_dev dev, xmedia_bool value)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_cfg_bt_yc_order(g_p_vout_reg, value);
    } else {
        ;
    }
    return ;
}

xmedia_void hal_vo_set_intf_bt_uv_order(xmedia_vo_dev dev, xmedia_bool value)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_cfg_bt_uv_order(g_p_vout_reg, value);
    } else {
        ;
    }
    return ;
}

//场消隐还是自定义
xmedia_void hal_vo_set_intf_bt_vbit_mode(xmedia_vo_dev dev, xmedia_bool sync_mode)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_cfg_vbit_mode(g_p_vout_reg, sync_mode);
    } else {
        ;
    }
    return ;
}

xmedia_void hal_vo_set_intf_bt_bit_inverse(xmedia_vo_dev dev, xmedia_bool enable)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_cfg_bt_bit_inv(g_p_vout_reg, enable);
    } else {
        ;
    }
    return ;
}

xmedia_void hal_vo_set_intf_bt_user_sync(xmedia_vo_dev dev, vo_intf_bt_sync *sync)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_cfg_vbit_st(g_p_vout_reg, sync->top_start);
        hal_reg_vout_set_cfg_vbit_end(g_p_vout_reg, sync->top_end);
        hal_reg_vout_set_cfg_bvbit_st(g_p_vout_reg, sync->bottom_start);
        hal_reg_vout_set_cfg_bvbit_end(g_p_vout_reg, sync->bottom_end);
    } else {
        ;
    }
    return ;
}

xmedia_void hal_vo_set_intf_lcd_mode(xmedia_vo_dev dev, xmedia_bool value)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_cfg_lcd_mode(g_p_vout_reg, value);
    } else {
        ;
    }
    return ;
}

xmedia_void hal_vo_set_intf_lcd_rgb_order(xmedia_vo_dev dev, xmedia_u8 order0, xmedia_u8 order1)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_cfg_lcd_rgb_order_0(g_p_vout_reg, order0);
        hal_reg_vout_set_cfg_lcd_rgb_order_1(g_p_vout_reg, order1);
    } else {
        ;
    }
    return ;
}

xmedia_void hal_vo_set_intf_lcd_bit_inverse(xmedia_vo_dev dev, xmedia_bool enable)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_cfg_lcd_bit_inv(g_p_vout_reg, enable);
    } else {
        ;
    }
    return ;
}

xmedia_void hal_vo_set_intf_mute_enable(xmedia_vo_dev dev, xmedia_bool enable)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_voutintf_cfg_mute_en(g_p_vout_reg, enable);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_get_intf_mute_enable(xmedia_vo_dev dev, xmedia_bool *enable)
{
    if (dev == XMEDIA_VO_DEV_0) {
        *enable = hal_reg_vout_get_voutintf_cfg_mute_en(g_p_vout_reg);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_mute_pattern_mode(xmedia_vo_dev dev, xmedia_u8 value)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_voutintf_cfg_mute_pat(g_p_vout_reg, value);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_mute_data_mode(xmedia_vo_dev dev, xmedia_u8 value)
{
    if (dev == XMEDIA_VO_DEV_0) {
       hal_reg_vout_set_voutintf_cfg_mute_mode(g_p_vout_reg, value);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_checkbar_size(xmedia_vo_dev dev, xmedia_u8 width, xmedia_u8 height)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_voutintf_cfg_checker_h(g_p_vout_reg, width);
        hal_reg_vout_set_voutintf_cfg_checker_v(g_p_vout_reg, height);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_cbar_sel(xmedia_vo_dev dev, xmedia_u8 value)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_cfg_cbar_sel(g_p_vout_reg, value);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_mute_color(xmedia_vo_dev dev, xmedia_u32 white_color, xmedia_u32 black_color)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_voutintf_cfg_mute_color(g_p_vout_reg, white_color);
        hal_reg_vout_set_intfmutebk_cfg_bk_color(g_p_vout_reg, black_color);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_ink(xmedia_vo_dev dev, vo_csc_ink_info *ink_info)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_cfg_ink_en(g_p_vout_reg, ink_info->ink_en);
        hal_reg_vout_set_cfg_cross_en(g_p_vout_reg, ink_info->cross_en);
        hal_reg_vout_set_cfg_cross_color_mode(g_p_vout_reg, ink_info->color_reverse);
        hal_reg_vout_set_cfg_cross_color(g_p_vout_reg, ink_info->color);
        hal_reg_vout_set_cfg_y_pos(g_p_vout_reg, ink_info->y);
        hal_reg_vout_set_cfg_x_pos(g_p_vout_reg, ink_info->x);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_clip(xmedia_vo_dev dev, vo_intf_clip *intf_clip)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_cfg_clip_yuv_l(g_p_vout_reg, intf_clip->clip_l);
        hal_reg_vout_set_cfg_clip_yuv_h(g_p_vout_reg, intf_clip->clip_h);
        hal_reg_vout_set_cfg_clip_en(g_p_vout_reg, intf_clip->clip_en);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_hcds(xmedia_vo_dev dev, vo_intf_hcds *intf_hcds)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_cfg_hcds_coef_0(g_p_vout_reg, intf_hcds->hcds_coef_0);
        hal_reg_vout_set_cfg_hcds_coef_1(g_p_vout_reg, intf_hcds->hcds_coef_1);
        hal_reg_vout_set_cfg_hcds_coef_2(g_p_vout_reg, intf_hcds->hcds_coef_2);
        hal_reg_vout_set_cfg_hcds_coef_3(g_p_vout_reg, intf_hcds->hcds_coef_3);
        // 0:丢点模式， 1：二阶滤波
        hal_reg_vout_set_cfg_hcds_mode(g_p_vout_reg, intf_hcds->hcds_mode);
        hal_reg_vout_set_cfg_hcds_en(g_p_vout_reg, intf_hcds->hcds_en);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_dither_enable(xmedia_vo_dev dev, xmedia_bool enable)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_cfg_dither_en(g_p_vout_reg, enable);
    } else {
        ;
    }
    return ;
}

xmedia_void hal_vo_set_intf_dither_thd(xmedia_vo_dev dev, xmedia_u8 thd_h, xmedia_u8 thd_l)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_cfg_dither_thd_h(g_p_vout_reg, thd_h);
        hal_reg_vout_set_cfg_dither_thd_l(g_p_vout_reg, thd_l);
    } else {
        ;
    }
    return ;
}
//接口数据顺序要要注意下视频格式输出的顺序
xmedia_void hal_vo_set_bt656_intf_cfg(xmedia_vo_dev dev, xmedia_vo_bt656_attr *bt656_attr)
{
    vo_intf_hcds intf_hcds;
    vo_intf_clip intf_clip;

    hal_vo_set_intf_mux(dev, XMEDIA_INTF_TYPE_BT656);
    intf_hcds.hcds_en = XMEDIA_TRUE;
    intf_hcds.hcds_mode = 0;
    intf_hcds.hcds_coef_0 = 127;
    intf_hcds.hcds_coef_1 = 1;
    intf_hcds.hcds_coef_2 = 32;
    intf_hcds.hcds_coef_3 = 96;
    hal_vo_set_intf_hcds(dev, &intf_hcds);
    intf_clip.clip_en = XMEDIA_TRUE;
    intf_clip.clip_l = 0x101010;
    intf_clip.clip_h = 0xEBF0F0;
    hal_vo_set_intf_clip(dev, &intf_clip);
    switch(bt656_attr->data_seq){
        case XMEDIA_INTF_BT601_BT656_DATA_SEQ_YUYV:
            hal_vo_set_intf_bt_yc_order(dev, 1);
            hal_vo_set_intf_bt_uv_order(dev, 0);
            break;
        case XMEDIA_INTF_BT601_BT656_DATA_SEQ_YVYU:
            hal_vo_set_intf_bt_yc_order(dev, 1);
            hal_vo_set_intf_bt_uv_order(dev, 1);
            break;
        case XMEDIA_INTF_BT601_BT656_DATA_SEQ_UYVY:
            hal_vo_set_intf_bt_yc_order(dev, 0);
            hal_vo_set_intf_bt_uv_order(dev, 0);
            break;
        case XMEDIA_INTF_BT601_BT656_DATA_SEQ_VYUY:
            hal_vo_set_intf_bt_yc_order(dev, 0);
            hal_vo_set_intf_bt_uv_order(dev, 1);
            break;
        default:
            break;
    }
    return;
}

xmedia_void hal_vo_set_bt1120_intf_cfg(xmedia_vo_dev dev, xmedia_vo_bt1120_attr *bt1120_attr)
{
    vo_intf_hcds intf_hcds;
    vo_intf_clip intf_clip;

    hal_vo_set_intf_mux(dev, XMEDIA_INTF_TYPE_BT1120);
    hal_vo_set_intf_dither_enable(dev, XMEDIA_FALSE);
    intf_hcds.hcds_en = XMEDIA_TRUE;
    intf_hcds.hcds_mode = 0;
    intf_hcds.hcds_coef_0 = 127;
    intf_hcds.hcds_coef_1 = 1;
    intf_hcds.hcds_coef_2 = 32;
    intf_hcds.hcds_coef_3 = 96;
    hal_vo_set_intf_hcds(dev, &intf_hcds);
    intf_clip.clip_en = XMEDIA_TRUE;
    intf_clip.clip_l = 0x101010;
    intf_clip.clip_h = 0xEBF0F0;
    hal_vo_set_intf_clip(dev, &intf_clip);
    hal_vo_set_intf_bt_yc_order(dev, 0);
    switch(bt1120_attr->data_seq){
        case XMEDIA_INTF_BT1120_DATA_SEQ_VUVU:
            hal_vo_set_intf_bt_uv_order(dev, 1);
            break;
        case XMEDIA_INTF_BT1120_DATA_SEQ_UVUV:
            hal_vo_set_intf_bt_uv_order(dev, 0);
            break;
        default:
            break;
    }
    return;
}

xmedia_void hal_vo_set_lcd_intf_cfg(xmedia_vo_dev dev, xmedia_vo_lcd_attr *lcd_attr)
{
    xmedia_u8 data_seq = 0;

    hal_vo_set_intf_mux(dev, XMEDIA_INTF_TYPE_LCD);
    switch(lcd_attr->data_mode){
        case XMEDIA_VO_LCD_DATA_MODE_SERIAL:
             if (lcd_attr->data_type == XMEDIA_INTF_LCD_DATA_TYPE_RGB666_18BIT) {
                hal_vo_set_intf_lcd_mode(dev, 0);
             } else {
                hal_vo_set_intf_lcd_mode(dev, 1);
             }
             break;
        case XMEDIA_VO_LCD_DATA_MODE_PARA:
             if (lcd_attr->data_type == XMEDIA_INTF_LCD_DATA_TYPE_RGB565_16BIT) {
                hal_vo_set_intf_lcd_mode(dev, 2);
             } else if (lcd_attr->data_type == XMEDIA_INTF_LCD_DATA_TYPE_RGB666_18BIT) {
                hal_vo_set_intf_lcd_mode(dev, 3);
             } else {
                hal_vo_set_intf_lcd_mode(dev, 4);
             }
             break;
        default:
            break;
    }

    /*
     *逻辑和实际lcd的颜色是不一致的
     * PLCD:
     * LCD_RGB = LOGIC_BGR  LCD_RBG = LOGIC_GBR  LCD_BGR = LOGIC_RGB
     * LCD_GRB = LOGIC_BRG  LCD_GBR = LOGIC_GRB  LCD_BRG = LOGIC_RBG
     * SLCD:
     * LCD_RGB = LOGIC_RGB  LCD_RBG = LOGIC_RBG  LCD_BGR = LOGIC_BGR
     * LCD_GRB = LOGIC_GRB  LCD_GBR = LOGIC_BRG  LCD_BRG = LOGIC_GBR
     */
    switch(lcd_attr->data_seq){
        case XMEDIA_INTF_LCD_DATA_SEQ_RGB:
             data_seq = lcd_attr->data_mode == XMEDIA_VO_LCD_DATA_MODE_PARA ? 5 : 0;
             break;
        case XMEDIA_INTF_LCD_DATA_SEQ_RBG:
             data_seq = lcd_attr->data_mode == XMEDIA_VO_LCD_DATA_MODE_PARA ? 3 : 1;
             break;
        case XMEDIA_INTF_LCD_DATA_SEQ_GRB:
             data_seq = lcd_attr->data_mode == XMEDIA_VO_LCD_DATA_MODE_PARA ? 4 : 2;
             break;
        case XMEDIA_INTF_LCD_DATA_SEQ_GBR:
             data_seq = lcd_attr->data_mode == XMEDIA_VO_LCD_DATA_MODE_PARA ? 2 : 4;
             break;
        case XMEDIA_INTF_LCD_DATA_SEQ_BRG:
             data_seq = lcd_attr->data_mode == XMEDIA_VO_LCD_DATA_MODE_PARA ? 1 : 3;
             break;
        case XMEDIA_INTF_LCD_DATA_SEQ_BGR:
             data_seq = lcd_attr->data_mode == XMEDIA_VO_LCD_DATA_MODE_PARA ? 0 : 5;
             break;
        default:
            break;
    }

    hal_vo_set_intf_lcd_rgb_order(dev, data_seq, data_seq);
    hal_vo_set_intf_dither_enable(dev, XMEDIA_TRUE);
    hal_vo_set_intf_dither_thd(dev, 255, 0);
    return;
}

static xmedia_void vo_data_type_to_hal_type(xmedia_intf_mcu_data_type data_type, xmedia_u32 *hal_data_type,
    xmedia_u32 *hal_wr_mode)
{
    switch (data_type) {
        case XMEDIA_VO_MCU_DATA_TYPE_8BIT_16BPP_RGB565_8080I:
            *hal_data_type = data_type;
            *hal_wr_mode = 1;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_8BIT_18BPP_RGB666_8080I:
            *hal_data_type = data_type;
            *hal_wr_mode = 2;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_8BIT_16BPP_RGB565_8080II:
            *hal_data_type = data_type;
            *hal_wr_mode = 1;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_8BIT_18BPP_RGB666_8080II:
            *hal_data_type = data_type;
            *hal_wr_mode = 2;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_9BIT_16BPP_RGB565_8080I:
            *hal_data_type = data_type;
            *hal_wr_mode = 1;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_9BIT_18BPP_RGB666_8080I_OPTION1:
            *hal_data_type = data_type;
            *hal_wr_mode = 2;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_9BIT_18BPP_RGB666_8080I_OPTION2:
            *hal_data_type = data_type;
            *hal_wr_mode = 2;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_9BIT_16BPP_RGB565_8080II:
            *hal_data_type = data_type;
            *hal_wr_mode = 1;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_9BIT_18BPP_RGB666_8080II_OPTION1:
            *hal_data_type = data_type;
            *hal_wr_mode = 2;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_9BIT_18BPP_RGB666_8080II_OPTION2:
            *hal_data_type = data_type;
            *hal_wr_mode = 2;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_16BIT_16BPP_RGB565_8080I:
            *hal_data_type = data_type;
            *hal_wr_mode = 1;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_16BIT_18BPP_RGB666_8080I_OPTION1:
            *hal_data_type = data_type;
            *hal_wr_mode = 2;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_16BIT_18BPP_RGB666_8080I_OPTION2:
            *hal_data_type = data_type;
            *hal_wr_mode = 2;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_16BIT_18BPP_RGB666_8080I_OPTION3:
            *hal_data_type = data_type;
            *hal_wr_mode = 2;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_16BIT_18BPP_RGB666_8080I_OPTION4:
            *hal_data_type = data_type;
            *hal_wr_mode = 2;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_16BIT_18BPP_RGB666_8080I_OPTION5:
            *hal_data_type = data_type;
            *hal_wr_mode = 2;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_16BIT_16BPP_RGB565_8080II:
            *hal_data_type = data_type;
            *hal_wr_mode = 1;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_16BIT_18BPP_RGB666_8080II_OPTION1:
            *hal_data_type = data_type;
            *hal_wr_mode = 2;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_16BIT_18BPP_RGB666_8080II_OPTION2:
            *hal_data_type = data_type;
            *hal_wr_mode = 2;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_16BIT_18BPP_RGB666_8080II_OPTION3:
            *hal_data_type = data_type;
            *hal_wr_mode = 2;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_16BIT_18BPP_RGB666_8080II_OPTION4:
            *hal_data_type = data_type;
            *hal_wr_mode = 2;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_18BIT_16BPP_RGB565_8080I:
            *hal_data_type = data_type;
            *hal_wr_mode = 1;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_18BIT_18BPP_RGB666_8080I:
            *hal_data_type = data_type;
            *hal_wr_mode = 2;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_18BIT_16BPP_RGB565_8080II:
            *hal_data_type = data_type;
            *hal_wr_mode = 1;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_18BIT_18BPP_RGB666_8080II:
            *hal_data_type = data_type;
            *hal_wr_mode = 2;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_3LINE_RGB565_OPTION1:
            *hal_data_type = 26;
            *hal_wr_mode = 1;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_3LINE_RGB666_OPTION1:
            *hal_data_type = 27;
            *hal_wr_mode = 2;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_3LINE_RGB565_OPTION2:
            *hal_data_type = 29;
            *hal_wr_mode = 1;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_3LINE_RGB666_OPTION2:
            *hal_data_type = 30;
            *hal_wr_mode = 2;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_4LINE_RGB565:
            *hal_data_type = 32;
            *hal_wr_mode = 1;
            break;
        case XMEDIA_VO_MCU_DATA_TYPE_4LINE_RGB666:
            *hal_data_type = 33;
            *hal_wr_mode = 2;
            break;
        default:
            *hal_data_type = 0;
            *hal_wr_mode = 2;
            break;
    }
    return;
}

// MCU 屏写参数
static xmedia_s32 vo_mcu_write_cmd(xmedia_u8 lut_mcu[], xmedia_u32 para_size)
{
    // 查询等待
    while((hal_reg_vout_get_mcu_wr_cmd_state(g_p_vout_reg) != 0) ||
        (hal_reg_vout_get_mcu_rd_cmd_state(g_p_vout_reg) == 1) ||
        (hal_reg_vout_get_mcu_rd_pixel_state(g_p_vout_reg) == 1));

    // 配置屏命令类型 写参数
    hal_reg_vout_set_cfg_mcu_cmd_mode(g_p_vout_reg, 0x1);
    // 参数个数 减一配置
    hal_reg_vout_set_cfg_mcu_wrcmd_size(g_p_vout_reg, para_size - 1);
    // 配置写参数寄存器
    osal_memcpy((xmedia_void*)&(g_p_vout_reg->vout_mcu_screen_wr_para), lut_mcu, para_size * 4);
    // 等待参数全部写入参数RAM中
    while(hal_reg_vout_get_mcu_wr_cmd_state(g_p_vout_reg) != 1) ;
    // 使能参数更新到MCU屏
    hal_reg_vout_set_cfg_mcu_wrcmd_start(g_p_vout_reg, XMEDIA_TRUE);
    while(hal_reg_vout_get_mcu_wr_cmd_state(g_p_vout_reg) != 0); 
    return 0;
}

static xmedia_s32 vo_dts_array_to_cmd_package(xmedia_u8 *cmd_buf, xmedia_u32 *cmd_index)
{
    xmedia_s32 index = *cmd_index;
    xmedia_u8 *cmd_buf_tmp = cmd_buf;
    xmedia_u32 one_cmd_len = 0;
    xmedia_u32 one_cmd_len_4byte_align = 0;
    xmedia_u32 cmd_buf_fill_size = 0;
    xmedia_u32 i = 0;
    if (*cmd_index >= g_vo_dts_lcd_config[VO_DTS_LCD_TYPE_MCU].size) {
        VO_TRACE(MODULE_DBG_DEBUG, "convert end *cmd_index %d\n", *cmd_index);
        return cmd_buf_fill_size;
    }

    while(1) {
        one_cmd_len = g_vo_dts_lcd_config[VO_DTS_LCD_TYPE_MCU].array[index];
        //碰到延时指令 直接跳出 进入下次循环的延时处理
        if (one_cmd_len == 0) {
            VO_TRACE(MODULE_DBG_DEBUG, "need delay %d ms break!!!\n", g_vo_dts_lcd_config[VO_DTS_LCD_TYPE_MCU].array[index + 1]);
            break;
        }
        //命令长度对4取整
        one_cmd_len_4byte_align = ((one_cmd_len / 4) + ((one_cmd_len % 4) ? 1 : 0)) * 4;
        //超出单次传送命令包长度
        if((cmd_buf_fill_size + one_cmd_len_4byte_align) >= MCU_CMD_PACKAGE_MAX_SIZE) {
            *cmd_index = index;
            break;
        }
        //单个命令起始位置
        index++;
        osal_memcpy(cmd_buf_tmp, (g_vo_dts_lcd_config[VO_DTS_LCD_TYPE_MCU].array + index), one_cmd_len);
        //索引按实际命令累加
        index += one_cmd_len;
        //按4byte对齐累加
        cmd_buf_tmp += one_cmd_len_4byte_align;
        cmd_buf_fill_size += one_cmd_len_4byte_align;
        *cmd_index = index;

        if (index == g_vo_dts_lcd_config[VO_DTS_LCD_TYPE_MCU].size) {
            VO_TRACE(MODULE_DBG_DEBUG, "read end success\n");
            break;
        } else if (index > g_vo_dts_lcd_config[VO_DTS_LCD_TYPE_MCU].size) {
            VO_TRACE(MODULE_DBG_ERR, "size error please check!!!\n");
            break;
        } else {
        }
    }

    for (i = 0;i < (cmd_buf_fill_size / 4); i++) {
        VO_TRACE(MODULE_DBG_DEBUG, "0x%02x 0x%02x 0x%02x 0x%02x \n", cmd_buf[i*4], cmd_buf[i*4 + 1],cmd_buf[i*4 + 2],cmd_buf[i*4 + 3]);
    }

    return cmd_buf_fill_size;
}

static xmedia_s32 vo_dts_do_delay(xmedia_u8 *cmd_buf, xmedia_u32 *cmd_index)
{
    xmedia_s32 index = *cmd_index;
    xmedia_u8 *cmd_buf_tmp = cmd_buf;
    xmedia_u32 one_cmd_len = 0;
    if (*cmd_index >= g_vo_dts_lcd_config[VO_DTS_LCD_TYPE_MCU].size) {
        VO_TRACE(MODULE_DBG_DEBUG, "convert end *cmd_index %d\n", *cmd_index);
        return XMEDIA_FALSE;
    }

    one_cmd_len = g_vo_dts_lcd_config[VO_DTS_LCD_TYPE_MCU].array[index];
    if (one_cmd_len == 0) {
        VO_TRACE(MODULE_DBG_DEBUG, "need delay %d ms!!!\n", g_vo_dts_lcd_config[VO_DTS_LCD_TYPE_MCU].array[index + 1]);
        cmd_buf_tmp[0] = 0;
        cmd_buf_tmp[1] = g_vo_dts_lcd_config[VO_DTS_LCD_TYPE_MCU].array[index + 1];
        index +=2;
        *cmd_index = index;
        return XMEDIA_TRUE;
    } else {
        return XMEDIA_FALSE;
    }
}

//mcu屏参初始化 todo ????
xmedia_void hal_vo_set_mcu_intf_cfg(xmedia_vo_dev dev, xmedia_vo_mcu_attr *mcu_attr)
{
    vo_intf_hcds intf_hcds;
    xmedia_u32 hal_data_type;
    xmedia_u32 hal_wr_mode;
    xmedia_u8 *cmd_buf;
    xmedia_s32 cmd_buf_fill_size = 0;
    xmedia_u32 cmd_index = 0;
    xmedia_s32 ret = XMEDIA_FALSE;
    hal_vo_set_intf_mux(dev, XMEDIA_INTF_TYPE_MCU);

    vo_data_type_to_hal_type(mcu_attr->data_type, &hal_data_type, &hal_wr_mode);
    hal_reg_vout_set_cfg_mcu_mode(g_p_vout_reg, hal_data_type);

    //mcu屏软复位
    hal_reg_vout_set_cfg_mcu_rst_ctrl(g_p_vout_reg, XMEDIA_TRUE);
    osal_udelay(1*1000);
    hal_reg_vout_set_cfg_mcu_rst_ctrl(g_p_vout_reg, XMEDIA_FALSE);
    osal_udelay(10*1000);
    hal_reg_vout_set_cfg_mcu_rst_ctrl(g_p_vout_reg, XMEDIA_TRUE);
    osal_udelay(120*1000);

    cmd_buf = (xmedia_u8 *)osal_vmalloc(MCU_CMD_PACKAGE_MAX_SIZE);
    while(1) {
        osal_memset(cmd_buf, 0, MCU_CMD_PACKAGE_MAX_SIZE);
        //处理屏参中穿插的延时指令 可能有连续延时指令 所以while循环处理
        while(1) {
            ret = vo_dts_do_delay(cmd_buf, &cmd_index);
            if (ret == XMEDIA_TRUE) {
                VO_TRACE(MODULE_DBG_DEBUG, "do delay %d ms!!!\n", cmd_buf[1]);
                osal_udelay(cmd_buf[1] * 1000);
                cmd_buf[1] = 0;
            } else {
                VO_TRACE(MODULE_DBG_DEBUG, "do delay success!!!\n");
                break;
            }
        }
        //传屏参
        cmd_buf_fill_size = vo_dts_array_to_cmd_package(cmd_buf, &cmd_index);
        if (cmd_buf_fill_size > 0) {
            vo_mcu_write_cmd(cmd_buf, (cmd_buf_fill_size / 4));
        } else {
            VO_TRACE(MODULE_DBG_DEBUG, "set mcu param end!!!\n");
            break;
        }
    }
    osal_vfree(cmd_buf);

    osal_udelay(10*1000);

    //565 666
    hal_reg_vout_set_cfg_mcu_wr_fmt(g_p_vout_reg, hal_wr_mode);

    //rgb888 下采样
    hal_vo_set_intf_dither_enable(dev, XMEDIA_TRUE);
    hal_vo_set_intf_dither_thd(dev, 255, 0);

    // 使用TE帧同步
    hal_reg_vout_set_cfg_mcu_sync_type(g_p_vout_reg, mcu_attr->te_mode_en);
    // Write memory start
    hal_reg_vout_set_cfg_mcu_wrram_cmd(g_p_vout_reg, 0x2c);
    // Write memory continue
    hal_reg_vout_set_cfg_mcu_wrramc_cmd(g_p_vout_reg, 0x3c);

    intf_hcds.hcds_en = XMEDIA_TRUE;
    intf_hcds.hcds_mode = 0;
    intf_hcds.hcds_coef_0 = 127;
    intf_hcds.hcds_coef_1 = 1;
    intf_hcds.hcds_coef_2 = 32;
    intf_hcds.hcds_coef_3 = 96;
    hal_vo_set_intf_hcds(dev, &intf_hcds);

    hal_reg_vout_set_cfg_mcu_wrram_en(g_p_vout_reg, 0x1); // 当前帧写display data到屏幕frame ram命令使能
    return;
}

