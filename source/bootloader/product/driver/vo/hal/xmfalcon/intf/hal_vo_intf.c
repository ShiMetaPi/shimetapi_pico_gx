/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <common.h>
#include <bootretry.h>
#include <cli.h>
#include <command.h>
#include <console.h>
#include <dm.h>
#include <edid.h>
#include <errno.h>
#include <i2c.h>
#include <malloc.h>
#include <asm/byteorder.h>
#include <linux/compiler.h>
#include <u-boot/crc.h>

#include "hal_vo.h"
#include "hal_vo_intf.h"
#include "hal_vo_reg.h"
#include "hal_vo_clock.h"

extern s_vout_regs_type *g_p_vout_reg;
extern void *g_p_vout_mipi_tx_ctrl;
extern void *g_p_vout_mipi_tx_phy;
extern vo_mipi_lcd_config g_vo_mipi_lcd;

#define MIPI_PHY_PLL_CLK_35MHZ 35
#define MIPI_PHY_PLL_CLK_75MHZ 75
#define MIPI_PHY_PLL_CLK_150MHZ 150
#define MIPI_PHY_PLL_CLK_300MHZ 300
#define MIPI_PHY_PLL_CLK_600MHZ 600
#define MIPI_PHY_PLL_CLK_1000MHZ 1000
#define MIPI_PHY_PLL_CLK_2400MHZ 2400
#define MIPI_PHY_PLL_CLK_4000MHZ 4000
#define LVDS_PHY_PLL_CLK_20MHZ 20000000
#define LVDS_PHY_PLL_CLK_32MHZ 32000000
#define LVDS_PHY_PLL_CLK_48MHZ 48000000
#define LVDS_PHY_PLL_CLK_66MHZ 66000000
#define LVDS_PHY_PLL_CLK_88MHZ 88000000
#define LVDS_PHY_PLL_CLK_110MHZ 110000000
#define LVDS_PHY_PLL_CLK_132MHZ 132000000
#define LVDS_PHY_PLL_CLK_154MHZ 154000000
#define LVDS_PHY_PLL_CLK_200MHZ 200000000

xmedia_void hal_vo_set_intf_fifo(xmedia_vo_dev dev, xmedia_u32 fifo)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_cfg_intf_sfifo_thd(g_p_vout_reg, fifo);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_voutintf1_cfg_intf_sfifo_thd(g_p_vout_reg, fifo);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_enable(xmedia_vo_dev dev, xmedia_bool enable)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_cfg_chk_sum_en(g_p_vout_reg, enable);
        hal_vout_set_cfg_intf_en(g_p_vout_reg, enable);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_1ctrl_cfg_chk_sum_en(g_p_vout_reg, enable);
        hal_vout_set_1ctrl_cfg_intf_en(g_p_vout_reg, enable);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_get_intf_enable(xmedia_vo_dev dev, xmedia_bool *enable)
{
    if (dev == XMEDIA_VO_DEV_0) {
        *enable = hal_vout_get_cfg_intf_en(g_p_vout_reg);
    } else if (dev == XMEDIA_VO_DEV_1) {
        *enable = hal_vout_get_1ctrl_cfg_intf_en(g_p_vout_reg);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_chk_sum_enable(xmedia_vo_dev dev, xmedia_bool enable)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_cfg_chk_sum_en(g_p_vout_reg, enable);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_1ctrl_cfg_chk_sum_en(g_p_vout_reg, enable);
    } else {
        ;
    }
    return;
}

//for mipi
xmedia_void hal_vo_set_intf_hs_mode(xmedia_vo_dev dev, vo_intf_hs_mode hs_mode)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_cfg_hs_mode(g_p_vout_reg, hs_mode);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_1ctrl_cfg_hs_mode(g_p_vout_reg, hs_mode);
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
               hal_vout_set_cfg_intf_sel(g_p_vout_reg, 1);
               break;
           case XMEDIA_INTF_TYPE_BT656:
           case XMEDIA_INTF_TYPE_BT1120:
               hal_vout_set_cfg_intf_sel(g_p_vout_reg, 0);
               if (intf == XMEDIA_INTF_TYPE_BT1120) {
                   hal_vout_set_cfg_bt_mode(g_p_vout_reg, 1);
               } else {
                   hal_vout_set_cfg_bt_mode(g_p_vout_reg, 0);
               }
               break;
           case XMEDIA_INTF_TYPE_MIPI_DSI:
               hal_vout_set_cfg_intf_sel(g_p_vout_reg, 2);
               break;
           case XMEDIA_INTF_TYPE_LVDS:
               hal_vout_set_cfg_intf_sel(g_p_vout_reg, 3);
               break;
           default:
               break;
       }
    } else if (dev == XMEDIA_VO_DEV_1) {
        switch (intf) {
           case XMEDIA_INTF_TYPE_LCD:
               hal_vout_set_1ctrl_cfg_intf_sel(g_p_vout_reg, 1);
               break;
           case XMEDIA_INTF_TYPE_BT656:
           case XMEDIA_INTF_TYPE_BT1120:
               hal_vout_set_1ctrl_cfg_intf_sel(g_p_vout_reg, 0);
               if (intf == XMEDIA_INTF_TYPE_BT1120) {
                   hal_vout_set_1btctrl_cfg_bt_mode(g_p_vout_reg, 1);
               } else {
                   hal_vout_set_1btctrl_cfg_bt_mode(g_p_vout_reg, 0);
               }
               break;
           case XMEDIA_INTF_TYPE_MIPI_DSI:
               hal_vout_set_1ctrl_cfg_intf_sel(g_p_vout_reg, 2);
               break;
           case XMEDIA_INTF_TYPE_LVDS:
               hal_vout_set_1ctrl_cfg_intf_sel(g_p_vout_reg, 3);
               break;
           default:
               break;
       }
    }
    return ;
}

xmedia_void hal_vo_set_intf_regup(xmedia_vo_dev dev, xmedia_bool enable)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_intfupd_cfg_regup_en(g_p_vout_reg, enable);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_intf1upd_cfg_regup_en(g_p_vout_reg, enable);
    } else {
        ;
    }
    return;
}

//默认从消隐区开始计数
xmedia_void hal_vo_set_intf_cnt_pos(xmedia_vo_dev dev)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_cfg_regup_pos(g_p_vout_reg, 1);
        hal_vout_set_cfg_para_pos(g_p_vout_reg, 2);
        hal_vout_set_cfg_prefetch_pos(g_p_vout_reg, 3);
        hal_vout_set_cfg_start_pos(g_p_vout_reg, 4);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_voutintf1_cfg_regup_pos(g_p_vout_reg, 1);
        hal_vout_set_voutintf1_cfg_para_pos(g_p_vout_reg, 2);
        hal_vout_set_voutintf1_cfg_prefetch_pos(g_p_vout_reg, 3);
        hal_vout_set_voutintf1_cfg_start_pos(g_p_vout_reg, 4);
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
        hal_vout_set_cfg_interlaced(g_p_vout_reg, !(sync_timing->iop)); //逐隔行 配置
        hal_vout_set_cfg_de_inv(g_p_vout_reg,sync_timing->idv);         //数据有效信号的极性
        hal_vout_set_cfg_hs_inv(g_p_vout_reg,sync_timing->ihs);         //水平有效信号的极性
        hal_vout_set_cfg_vs_inv(g_p_vout_reg,sync_timing->ivs);         //垂直有效信号的极性
        if (sync_timing->iop == 0) {
            hal_vout_set_cfg_vact(g_p_vout_reg, sync_timing->vact * 2 - 1);//垂直有效区
        } else {
            hal_vout_set_cfg_vact(g_p_vout_reg, sync_timing->vact - 1);
        }
        hal_vout_set_cfg_vbb(g_p_vout_reg,sync_timing->vbb - 1);        //垂直消隐后肩
        hal_vout_set_cfg_vfb(g_p_vout_reg,sync_timing->vfb - 1);        //垂直消隐前肩
        hal_vout_set_cfg_hact(g_p_vout_reg,sync_timing->hact - 1);      //水平有效区
        hal_vout_set_cfg_bvbb(g_p_vout_reg,sync_timing->bvbb - 1);      //底场垂直消隐后肩
        hal_vout_set_cfg_bvfb(g_p_vout_reg,sync_timing->bvfb - 1);      //底场垂直消隐前肩
        hal_vout_set_cfg_hs_width(g_p_vout_reg,sync_timing->hpw);       //水平脉冲宽度
        if (special_timing->vo_special_timing_sel) {
            hal_vout_set_cfg_hbb(g_p_vout_reg,special_timing->fixed_clk_hbb - 1);        //水平消隐后肩
            hal_vout_set_cfg_hfb(g_p_vout_reg,special_timing->fixed_clk_hfb - 1);        //水平消隐前肩
            //vsync lcd mipi lvds 都需要使用，vsync的起始点和结束点
            hal_vout_set_cfg_vs_st_h_field0(g_p_vout_reg, special_timing->fixed_clk_hfb - 1);
            hal_vout_set_cfg_vs_end_h_field0(g_p_vout_reg, special_timing->fixed_clk_hfb - 1);
            hal_vout_set_cfg_vs_st_v_field0(g_p_vout_reg, special_timing->vs_st_v_filed);
            hal_vout_set_cfg_vs_end_v_field0(g_p_vout_reg, special_timing->vs_end_v_filed);
        } else {
            hal_vout_set_cfg_hbb(g_p_vout_reg,sync_timing->hbb - 1);        //水平消隐后肩
            hal_vout_set_cfg_hfb(g_p_vout_reg,sync_timing->hfb - 1);        //水平消隐前肩
            //vsync lcd mipi lvds 都需要使用，vsync的起始点和结束点
            hal_vout_set_cfg_vs_st_h_field0(g_p_vout_reg, sync_timing->hbb - 1);
            hal_vout_set_cfg_vs_end_h_field0(g_p_vout_reg, sync_timing->hbb - 1);
            hal_vout_set_cfg_vs_st_v_field0(g_p_vout_reg, sync_timing->vfb);
            hal_vout_set_cfg_vs_end_v_field0(g_p_vout_reg, sync_timing->vfb + sync_timing->vpw);
        }

        hal_vout_set_cfg_field_tog(g_p_vout_reg, 0x0);  //场模式用的  暂时不管
        hal_vout_set_cfg_bfield_tog(g_p_vout_reg, 0x0); //场模式用的  暂时不管
        hal_vout_set_cfg_vs_st_h_field1(g_p_vout_reg, 0x0);     //场模式用的  暂时不管
        hal_vout_set_cfg_vs_end_h_field1(g_p_vout_reg, 0x0);    //场模式用的  暂时不管
        hal_vout_set_cfg_vs_st_v_field1(g_p_vout_reg, 0x0);     //场模式用的  暂时不管
        hal_vout_set_cfg_vs_end_v_field1(g_p_vout_reg, 0x0);    //场模式用的  暂时不管

        /*
         * 目前还有以下参数没用到
         *  xmedia_u16 hmid;  //底场同步水平有效区域
         *  xmedia_bool synm; //同步模式(0 表示比如BT656时序，1表示像LCD那样的信号)
         *  xmedia_u8 intfb;  //隔行输出的位宽y
         */
   } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_1ctrl_cfg_interlaced(g_p_vout_reg,!(sync_timing->iop));
        hal_vout_set_cfg_de_inv(g_p_vout_reg,sync_timing->idv);
        hal_vout_set_cfg_hs_inv(g_p_vout_reg,sync_timing->ihs);
        hal_vout_set_cfg_vs_inv(g_p_vout_reg,sync_timing->ivs);
        if (sync_timing->iop == 0) {
            hal_vout_set_1act_cfg_vact(g_p_vout_reg, sync_timing->vact * 2 - 1);
        } else {
            hal_vout_set_1act_cfg_vact(g_p_vout_reg, sync_timing->vact - 1);
        }
        hal_vout_set_1vfb_cfg_vbb(g_p_vout_reg,sync_timing->vbb - 1);
        hal_vout_set_1vfb_cfg_vfb(g_p_vout_reg,sync_timing->vfb - 1);
        hal_vout_set_1act_cfg_hact(g_p_vout_reg,sync_timing->hact - 1);
        hal_vout_set_1bvfb_cfg_bvbb(g_p_vout_reg,sync_timing->bvbb -1);
        hal_vout_set_1bvfb_cfg_bvfb(g_p_vout_reg,sync_timing->bvfb - 1);
        hal_vout_set_1hsw_cfg_hs_width(g_p_vout_reg,sync_timing->hpw);       //水平脉冲宽度
        if (special_timing->vo_special_timing_sel) {
            hal_vout_set_1hfb_cfg_hbb(g_p_vout_reg,special_timing->fixed_clk_hbb - 1);
            hal_vout_set_1hfb_cfg_hfb(g_p_vout_reg,special_timing->fixed_clk_hfb - 1);
            hal_vout_set_1vsh_cfg_vs_st_h_field0(g_p_vout_reg, special_timing->fixed_clk_hfb - 1);
            hal_vout_set_1vsh_cfg_vs_end_h_field0(g_p_vout_reg, special_timing->fixed_clk_hfb - 1);
            hal_vout_set_1vsv_cfg_vs_st_v_field0(g_p_vout_reg, special_timing->vs_st_v_filed);
            hal_vout_set_1vsv_cfg_vs_end_v_field0(g_p_vout_reg, special_timing->vs_end_v_filed);
        } else {
            hal_vout_set_1hfb_cfg_hbb(g_p_vout_reg,sync_timing->hbb - 1);
            hal_vout_set_1hfb_cfg_hfb(g_p_vout_reg,sync_timing->hfb - 1);
            hal_vout_set_1vsh_cfg_vs_st_h_field0(g_p_vout_reg, sync_timing->hbb - 1);
            hal_vout_set_1vsh_cfg_vs_end_h_field0(g_p_vout_reg, sync_timing->hbb - 1);
            hal_vout_set_1vsv_cfg_vs_st_v_field0(g_p_vout_reg, sync_timing->vfb);
            hal_vout_set_1vsv_cfg_vs_end_v_field0(g_p_vout_reg, sync_timing->vfb + sync_timing->vpw);
        }

        hal_vout_set_1field_cfg_field_tog(g_p_vout_reg, 0x0);
        hal_vout_set_1field_cfg_bfield_tog(g_p_vout_reg, 0x0);
        hal_vout_set_1bvsh_cfg_vs_st_h_field1(g_p_vout_reg, 0x0);
        hal_vout_set_1bvsh_cfg_vs_end_h_field1(g_p_vout_reg, 0x0);
        hal_vout_set_1bvsv_cfg_vs_st_v_field1(g_p_vout_reg, 0x0);
        hal_vout_set_1bvsv_cfg_vs_end_v_field1(g_p_vout_reg, 0x0);
   } else {
        ;
    }
   return;
}

//主要给LCD用，BT用内同步
xmedia_void hal_vo_set_intf_sync_inverse(xmedia_vo_dev dev)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_cfg_field_tog(g_p_vout_reg, 0x0); //(0->vbb+bvbb+1)
        hal_vout_set_cfg_bfield_tog(g_p_vout_reg, 0x0);
        hal_vout_set_cfg_vs_st_h_field0(g_p_vout_reg, 0x0);
        hal_vout_set_cfg_vs_end_h_field0(g_p_vout_reg, 0x1);
        hal_vout_set_cfg_vs_st_v_field0(g_p_vout_reg, 0x0); //行翻转点起始
        hal_vout_set_cfg_vs_end_v_field0(g_p_vout_reg, 0x1); //(0,vfb+vbb+1)
        hal_vout_set_cfg_vs_st_v_field1(g_p_vout_reg, 0x0);
        hal_vout_set_cfg_vs_end_v_field1(g_p_vout_reg, 0x1);
        hal_vout_set_cfg_vs_st_h_field1(g_p_vout_reg, 0x0);
        hal_vout_set_cfg_vs_end_h_field1(g_p_vout_reg, 0x1);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_1field_cfg_field_tog(g_p_vout_reg, 0x0); //场翻转点
        hal_vout_set_1field_cfg_bfield_tog(g_p_vout_reg, 0x0);
        hal_vout_set_1vsh_cfg_vs_st_h_field0(g_p_vout_reg, 0x0); //行翻转点起始
        hal_vout_set_1vsh_cfg_vs_end_h_field0(g_p_vout_reg, 0x1); //行翻转点结束
        hal_vout_set_1vsv_cfg_vs_st_v_field0(g_p_vout_reg, 0x0);
        hal_vout_set_1vsv_cfg_vs_end_v_field0(g_p_vout_reg, 0x1);
        hal_vout_set_1bvsh_cfg_vs_st_h_field1(g_p_vout_reg, 0x0);
        hal_vout_set_1bvsh_cfg_vs_end_h_field1(g_p_vout_reg, 0x1);
        hal_vout_set_1bvsv_cfg_vs_st_v_field1(g_p_vout_reg, 0x0);
        hal_vout_set_1bvsv_cfg_vs_end_v_field1(g_p_vout_reg, 0x1);
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
            hal_vout_set_cfg_vt_int_cnt_sel_0(g_p_vout_reg, cnt_mode);
        }
        if (vtt_id == 1) {
           hal_vout_set_cfg_vt_int_cnt_sel_1(g_p_vout_reg, cnt_mode);
        }
        if (vtt_id == 2) {
            hal_vout_set_cfg_vt_int_cnt_sel_2(g_p_vout_reg, cnt_mode);
        }
        if (vtt_id == 3) {
            hal_vout_set_cfg_vt_int_cnt_sel_3(g_p_vout_reg, cnt_mode);
        }
    } else if (dev == XMEDIA_VO_DEV_1) {
        if (vtt_id == 0) {
            hal_vout_set_1vtint_cfg_vt_int_cnt_sel_0(g_p_vout_reg, cnt_mode);
        }
        if (vtt_id == 1) {
           hal_vout_set_1vtint_cfg_vt_int_cnt_sel_1(g_p_vout_reg, cnt_mode);
        }
        if (vtt_id == 2) {
            hal_vout_set_1vtint_cfg_vt_int_cnt_sel_2(g_p_vout_reg, cnt_mode);
        }
        if (vtt_id == 3) {
            hal_vout_set_1vtint_cfg_vt_int_cnt_sel_3(g_p_vout_reg, cnt_mode);
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
            hal_vout_set_cfg_vt_int_mode_0(g_p_vout_reg, frame_mode);
        }
        if (vtt_id == 1) {
            hal_vout_set_cfg_vt_int_mode_1(g_p_vout_reg, frame_mode);
        }
        if (vtt_id == 2) {
            hal_vout_set_cfg_vt_int_mode_2(g_p_vout_reg, frame_mode);
        }
        if (vtt_id == 3) {
            hal_vout_set_cfg_vt_int_mode_3(g_p_vout_reg, frame_mode);
        }
    } else if (dev == XMEDIA_VO_DEV_1) {
        if (vtt_id == 0) {
            hal_vout_set_1vtint_cfg_vt_int_mode_0(g_p_vout_reg, frame_mode);
        }
        if (vtt_id == 1) {
            hal_vout_set_1vtint_cfg_vt_int_mode_1(g_p_vout_reg, frame_mode);
        }
        if (vtt_id == 2) {
            hal_vout_set_1vtint_cfg_vt_int_mode_2(g_p_vout_reg, frame_mode);
        }
        if (vtt_id == 3) {
            hal_vout_set_1vtint_cfg_vt_int_mode_3(g_p_vout_reg, frame_mode);
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
            hal_vout_set_cfg_vt_int_thd_0(g_p_vout_reg, vtt_thd);
        }
        if (vtt_id == 1) {
            hal_vout_set_cfg_vt_int_thd_1(g_p_vout_reg, vtt_thd);
        }
        if (vtt_id == 2) {
            hal_vout_set_cfg_vt_int_thd_2(g_p_vout_reg, vtt_thd);
        }
        if (vtt_id == 3) {
            hal_vout_set_cfg_vt_int_thd_3(g_p_vout_reg, vtt_thd);
        }
    } else if (dev == XMEDIA_VO_DEV_1) {
        if (vtt_id == 0) {
            hal_vout_set_1vtintf1thd0_cfg_vt_int_thd_0(g_p_vout_reg, vtt_thd);
        }
        if (vtt_id == 1) {
            hal_vout_set_1vtintf1thd0_cfg_vt_int_thd_1(g_p_vout_reg, vtt_thd);
        }
        if (vtt_id == 2) {
            hal_vout_set_1vtintf1thd2_cfg_vt_int_thd_2(g_p_vout_reg, vtt_thd);
        }
        if (vtt_id == 3) {
            hal_vout_set_1vtintf1thd2_cfg_vt_int_thd_3(g_p_vout_reg, vtt_thd);
        }
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_get_intf_state(xmedia_vo_dev dev, xmedia_bool *btm, xmedia_u16 *vcnt, xmedia_u8 *vstate)
{
    if (dev == XMEDIA_VO_DEV_0) {
        *btm = hal_vout_get_field_flag_ppc(g_p_vout_reg);
        *vstate = hal_vout_get_vstate(g_p_vout_reg);
        if (hal_vout_get_cfg_vt_int_cnt_sel_0(g_p_vout_reg) == 0) {
            *vcnt = hal_vout_get_vcnt_st_vblk(g_p_vout_reg);
        } else if (hal_vout_get_cfg_vt_int_cnt_sel_0(g_p_vout_reg) == 1) {
            *vcnt = hal_vout_get_vcnt_st_vact(g_p_vout_reg);
        } else {
            *vcnt = hal_vout_get_vcnt_st_vblk(g_p_vout_reg);
        }

    } else if (dev == XMEDIA_VO_DEV_1) {
        *btm = hal_vout_get_1sta_field_flag_ppc(g_p_vout_reg);
        *vstate = hal_vout_get_1sta_vstate(g_p_vout_reg);
        if (hal_vout_get_1vtint_cfg_vt_int_cnt_sel_0(g_p_vout_reg) == 0) {
            *vcnt = hal_vout_get_1sta2_vcnt_st_vblk(g_p_vout_reg);
        } else if (hal_vout_get_1vtint_cfg_vt_int_cnt_sel_0(g_p_vout_reg) == 1) {
            *vcnt = hal_vout_get_1sta2_vcnt_st_vact(g_p_vout_reg);
        } else {
            *vcnt = hal_vout_get_1sta2_vcnt_st_vblk(g_p_vout_reg);
        }
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_get_intf_checksum_value(xmedia_vo_dev dev, xmedia_u32 *y, xmedia_u32 *u, xmedia_u32 *v)
{
    if (dev == XMEDIA_VO_DEV_0) {
        *y = hal_vout_get_intf_y_sum(g_p_vout_reg);
        *u = hal_vout_get_intf_u_sum(g_p_vout_reg);
        *v = hal_vout_get_intf_v_sum(g_p_vout_reg);
    } else if (dev == XMEDIA_VO_DEV_1) {
        *y = hal_vout_get_1ysum_intf_y_sum(g_p_vout_reg);
        *u = hal_vout_get_1usum_intf_u_sum(g_p_vout_reg);
        *v = hal_vout_get_1vsum_intf_v_sum(g_p_vout_reg);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_bt_mode(xmedia_vo_dev dev, xmedia_bool value)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_cfg_bt_mode(g_p_vout_reg, value);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_1btctrl_cfg_bt_mode(g_p_vout_reg, value);
    } else {
        ;
    }
    return ;
}

xmedia_void hal_vo_set_intf_bt_yc_order(xmedia_vo_dev dev, xmedia_bool value)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_cfg_bt_yc_order(g_p_vout_reg, value);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_1btctrl_cfg_bt_yc_order(g_p_vout_reg, value);
    } else {
        ;
    }
    return ;
}

xmedia_void hal_vo_set_intf_bt_uv_order(xmedia_vo_dev dev, xmedia_bool value)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_cfg_bt_uv_order(g_p_vout_reg, value);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_1btctrl_cfg_bt_uv_order(g_p_vout_reg, value);
    } else {
        ;
    }
    return ;
}

//场消隐还是自定义
xmedia_void hal_vo_set_intf_bt_vbit_mode(xmedia_vo_dev dev, xmedia_bool sync_mode)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_cfg_bt_bit_inv(g_p_vout_reg, sync_mode);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_1btctrl_cfg_vbit_mode(g_p_vout_reg, sync_mode);
    } else {
        ;
    }
    return ;
}

xmedia_void hal_vo_set_intf_bt_bit_inverse(xmedia_vo_dev dev, xmedia_bool enable)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_cfg_bt_bit_inv(g_p_vout_reg, enable);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_1btctrl_cfg_bt_bit_inv(g_p_vout_reg, enable);
    } else {
        ;
    }
    return ;
}

xmedia_void hal_vo_set_intf_bt_user_sync(xmedia_vo_dev dev, vo_intf_bt_sync *sync)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_cfg_vbit_st(g_p_vout_reg, sync->top_start);
        hal_vout_set_cfg_vbit_end(g_p_vout_reg, sync->top_end);
        hal_vout_set_cfg_bvbit_st(g_p_vout_reg, sync->bottom_start);
        hal_vout_set_cfg_bvbit_end(g_p_vout_reg, sync->bottom_end);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_1btvbit_cfg_vbit_st(g_p_vout_reg, sync->top_start);
        hal_vout_set_1btvbit_cfg_vbit_end(g_p_vout_reg, sync->top_end);
        hal_vout_set_1btvbit_cfg_vbit_st(g_p_vout_reg, sync->bottom_start);
        hal_vout_set_1btvbit_cfg_vbit_end(g_p_vout_reg, sync->bottom_end);
    } else {
        ;
    }
    return ;
}

xmedia_void hal_vo_set_intf_lcd_mode(xmedia_vo_dev dev, xmedia_bool value)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_cfg_lcd_mode(g_p_vout_reg, value);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_1lcdctrl_cfg_lcd_mode(g_p_vout_reg, value);
    } else {
        ;
    }
    return ;
}

xmedia_void hal_vo_set_intf_lcd_rgb_order(xmedia_vo_dev dev, xmedia_u8 order0, xmedia_u8 order1)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_cfg_lcd_rgb_order_0(g_p_vout_reg, order0);
        hal_vout_set_cfg_lcd_rgb_order_1(g_p_vout_reg, order1);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_1lcdctrl_cfg_lcd_rgb_order_0(g_p_vout_reg, order0);
        hal_vout_set_1lcdctrl_cfg_lcd_rgb_order_1(g_p_vout_reg, order1);
    } else {
        ;
    }
    return ;
}

xmedia_void hal_vo_set_intf_lcd_bit_inverse(xmedia_vo_dev dev, xmedia_bool enable)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_cfg_lcd_bit_inv(g_p_vout_reg, enable);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_1lcdctrl_cfg_lcd_bit_inv(g_p_vout_reg, enable);
    } else {
        ;
    }
    return ;
}

xmedia_void hal_vo_set_intf_mute_enable(xmedia_vo_dev dev, xmedia_bool enable)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_voutintf_cfg_mute_en(g_p_vout_reg, enable);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_voutintf1_cfg_mute_en(g_p_vout_reg, enable);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_get_intf_mute_enable(xmedia_vo_dev dev, xmedia_bool *enable)
{
    if (dev == XMEDIA_VO_DEV_0) {
        *enable = hal_vout_get_voutintf_cfg_mute_en(g_p_vout_reg);
    } else if (dev == XMEDIA_VO_DEV_1) {
        *enable = hal_vout_get_voutintf1_cfg_mute_en(g_p_vout_reg);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_mute_pattern_mode(xmedia_vo_dev dev, xmedia_u8 value)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_voutintf_cfg_mute_pat(g_p_vout_reg, value);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_voutintf1_cfg_mute_pat(g_p_vout_reg, value);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_mute_data_mode(xmedia_vo_dev dev, xmedia_u8 value)
{
    if (dev == XMEDIA_VO_DEV_0) {
       hal_vout_set_voutintf_cfg_mute_mode(g_p_vout_reg, value);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_voutintf1_cfg_mute_mode(g_p_vout_reg, value);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_checkbar_size(xmedia_vo_dev dev, xmedia_u8 width, xmedia_u8 height)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_voutintf_cfg_checker_h(g_p_vout_reg, width);
        hal_vout_set_voutintf_cfg_checker_v(g_p_vout_reg, height);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_voutintf1_cfg_checker_h(g_p_vout_reg, width);
        hal_vout_set_voutintf1_cfg_checker_v(g_p_vout_reg, height);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_cbar_sel(xmedia_vo_dev dev, xmedia_u8 value)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_cfg_cbar_sel(g_p_vout_reg, value);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_voutintf1_cfg_cbar_sel(g_p_vout_reg, value);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_mute_color(xmedia_vo_dev dev, xmedia_u32 white_color, xmedia_u32 black_color)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_voutintf_cfg_mute_color(g_p_vout_reg, white_color);
        hal_vout_set_intfmutebk_cfg_bk_color(g_p_vout_reg, black_color);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_voutintf1_cfg_mute_color(g_p_vout_reg, white_color);
        hal_vout_set_intf1mutebk_cfg_bk_color(g_p_vout_reg, black_color);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_ink(xmedia_vo_dev dev, vo_csc_ink_info *ink_info)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_cfg_ink_en(g_p_vout_reg, ink_info->ink_en);
        hal_vout_set_cfg_cross_en(g_p_vout_reg, ink_info->cross_en);
        hal_vout_set_cfg_cross_color_mode(g_p_vout_reg, ink_info->color_reverse);
        hal_vout_set_cfg_cross_color(g_p_vout_reg, ink_info->color);
        hal_vout_set_cfg_y_pos(g_p_vout_reg, ink_info->y);
        hal_vout_set_cfg_x_pos(g_p_vout_reg, ink_info->x);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_1inkctrl_cfg_ink_en(g_p_vout_reg, ink_info->ink_en);
        hal_vout_set_1inkctrl_cfg_cross_en(g_p_vout_reg, ink_info->cross_en);
        hal_vout_set_1inkctrl_cfg_cross_color_mode(g_p_vout_reg, ink_info->color_reverse);
        hal_vout_set_voutintf1_cfg_cross_color(g_p_vout_reg, ink_info->color);
        hal_vout_set_1inkpos_cfg_y_pos(g_p_vout_reg, ink_info->y);
        hal_vout_set_1inkpos_cfg_x_pos(g_p_vout_reg, ink_info->x);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_clip(xmedia_vo_dev dev, vo_intf_clip *intf_clip)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_cfg_clip_yuv_l(g_p_vout_reg, intf_clip->clip_l);
        hal_vout_set_cfg_clip_yuv_h(g_p_vout_reg, intf_clip->clip_h);
        hal_vout_set_cfg_clip_en(g_p_vout_reg, intf_clip->clip_en);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_1clipl_cfg_clip_yuv_l(g_p_vout_reg, intf_clip->clip_l);
        hal_vout_set_1cliph_cfg_clip_yuv_h(g_p_vout_reg, intf_clip->clip_h);
        hal_vout_set_1clipl_cfg_clip_en(g_p_vout_reg, intf_clip->clip_en);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_csc_en(xmedia_vo_dev dev, xmedia_bool enable)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_voutintf_cfg_csc_en(g_p_vout_reg, enable);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_voutintf1_cfg_csc_en(g_p_vout_reg, enable);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_get_intf_csc_en(xmedia_vo_dev dev, xmedia_bool *enable)
{
    if (dev == XMEDIA_VO_DEV_0) {
        *enable = hal_vout_get_voutintf_cfg_csc_en(g_p_vout_reg);
    } else if (dev == XMEDIA_VO_DEV_1) {
        *enable = hal_vout_get_voutintf1_cfg_csc_en(g_p_vout_reg);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_csc(xmedia_vo_dev dev, vo_csc_coef *intf_csc)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_voutintf_cfg_csc_offset_in_2(g_p_vout_reg, intf_csc->csc_offset_in2);
        hal_vout_set_voutintf_cfg_csc_offset_in_1(g_p_vout_reg, intf_csc->csc_offset_in1);
        hal_vout_set_voutintf_cfg_csc_offset_in_0(g_p_vout_reg, intf_csc->csc_offset_in0);
        hal_vout_set_voutintf_cfg_csc_matrix_0(g_p_vout_reg, intf_csc->csc_coef0);
        hal_vout_set_voutintf_cfg_csc_matrix_1(g_p_vout_reg, intf_csc->csc_coef1);
        hal_vout_set_voutintf_cfg_csc_matrix_2(g_p_vout_reg, intf_csc->csc_coef2);
        hal_vout_set_voutintf_cfg_csc_matrix_3(g_p_vout_reg, intf_csc->csc_coef3);
        hal_vout_set_voutintf_cfg_csc_matrix_4(g_p_vout_reg, intf_csc->csc_coef4);
        hal_vout_set_voutintf_cfg_csc_matrix_5(g_p_vout_reg, intf_csc->csc_coef5);
        hal_vout_set_voutintf_cfg_csc_matrix_6(g_p_vout_reg, intf_csc->csc_coef6);
        hal_vout_set_voutintf_cfg_csc_matrix_7(g_p_vout_reg, intf_csc->csc_coef7);
        hal_vout_set_voutintf_cfg_csc_matrix_8(g_p_vout_reg, intf_csc->csc_coef8);
        hal_vout_set_voutintf_cfg_csc_offset_out_2(g_p_vout_reg, intf_csc->csc_offset_out2);
        hal_vout_set_voutintf_cfg_csc_offset_out_1(g_p_vout_reg, intf_csc->csc_offset_out1);
        hal_vout_set_voutintf_cfg_csc_offset_out_0(g_p_vout_reg, intf_csc->csc_offset_out0);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_voutintf1_cfg_csc_offset_in_2(g_p_vout_reg, intf_csc->csc_offset_in2);
        hal_vout_set_voutintf1_cfg_csc_offset_in_1(g_p_vout_reg, intf_csc->csc_offset_in1);
        hal_vout_set_voutintf1_cfg_csc_offset_in_0(g_p_vout_reg, intf_csc->csc_offset_in0);
        hal_vout_set_voutintf1_cfg_csc_matrix_0(g_p_vout_reg, intf_csc->csc_coef0);
        hal_vout_set_voutintf1_cfg_csc_matrix_1(g_p_vout_reg, intf_csc->csc_coef1);
        hal_vout_set_voutintf1_cfg_csc_matrix_2(g_p_vout_reg, intf_csc->csc_coef2);
        hal_vout_set_voutintf1_cfg_csc_matrix_3(g_p_vout_reg, intf_csc->csc_coef3);
        hal_vout_set_voutintf1_cfg_csc_matrix_4(g_p_vout_reg, intf_csc->csc_coef4);
        hal_vout_set_voutintf1_cfg_csc_matrix_5(g_p_vout_reg, intf_csc->csc_coef5);
        hal_vout_set_voutintf1_cfg_csc_matrix_6(g_p_vout_reg, intf_csc->csc_coef6);
        hal_vout_set_voutintf1_cfg_csc_matrix_7(g_p_vout_reg, intf_csc->csc_coef7);
        hal_vout_set_voutintf1_cfg_csc_matrix_8(g_p_vout_reg, intf_csc->csc_coef8);
        hal_vout_set_voutintf1_cfg_csc_offset_out_2(g_p_vout_reg, intf_csc->csc_offset_out2);
        hal_vout_set_voutintf1_cfg_csc_offset_out_1(g_p_vout_reg, intf_csc->csc_offset_out1);
        hal_vout_set_voutintf1_cfg_csc_offset_out_0(g_p_vout_reg, intf_csc->csc_offset_out0);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_hcds(xmedia_vo_dev dev, vo_intf_hcds *intf_hcds)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_cfg_hcds_coef_0(g_p_vout_reg, intf_hcds->hcds_coef_0);
        hal_vout_set_cfg_hcds_coef_1(g_p_vout_reg, intf_hcds->hcds_coef_1);
        hal_vout_set_cfg_hcds_coef_2(g_p_vout_reg, intf_hcds->hcds_coef_2);
        hal_vout_set_cfg_hcds_coef_3(g_p_vout_reg, intf_hcds->hcds_coef_3);
        // 0:丢点模式， 1：二阶滤波
        hal_vout_set_cfg_hcds_mode(g_p_vout_reg, intf_hcds->hcds_mode);
        hal_vout_set_cfg_hcds_en(g_p_vout_reg, intf_hcds->hcds_en);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_voutintf1_cfg_hcds_coef_0(g_p_vout_reg, intf_hcds->hcds_coef_0);
        hal_vout_set_voutintf1_cfg_hcds_coef_1(g_p_vout_reg, intf_hcds->hcds_coef_1);
        hal_vout_set_voutintf1_cfg_hcds_coef_2(g_p_vout_reg, intf_hcds->hcds_coef_2);
        hal_vout_set_voutintf1_cfg_hcds_coef_3(g_p_vout_reg, intf_hcds->hcds_coef_3);
        // 0:丢点模式， 1：二阶滤波
        hal_vout_set_voutintf1_cfg_hcds_mode(g_p_vout_reg, intf_hcds->hcds_mode);
        hal_vout_set_voutintf1_cfg_hcds_en(g_p_vout_reg, intf_hcds->hcds_en);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_set_intf_dither_enable(xmedia_vo_dev dev, xmedia_bool enable)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_cfg_dither_en(g_p_vout_reg, enable);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_voutintf1_cfg_dither_en(g_p_vout_reg, enable);
    } else {
        ;
    }
    return ;
}

xmedia_void hal_vo_set_intf_dither_thd(xmedia_vo_dev dev, xmedia_u8 thd_h, xmedia_u8 thd_l)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_vout_set_cfg_dither_thd_h(g_p_vout_reg, thd_h);
        hal_vout_set_cfg_dither_thd_l(g_p_vout_reg, thd_l);
    } else if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_voutintf1_cfg_dither_thd_h(g_p_vout_reg, thd_h);
        hal_vout_set_voutintf1_cfg_dither_thd_l(g_p_vout_reg, thd_l);
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

//dpi需要根据寄存器进行更新 如果接666 屏是否要设置dither？？？
xmedia_void hal_vo_set_mipi_intf_cfg(xmedia_vo_dev dev, xmedia_vo_mipi_attr *mipi_attr)
{
    xmedia_u8 value;
    hal_vo_set_intf_mux(dev, XMEDIA_INTF_TYPE_MIPI_DSI);

    switch(mipi_attr->data_type){
        case XMEDIA_INTF_MIPI_DSI_DATA_TYPE_YUV422_16BIT:
             value = 0;
             break;
        case XMEDIA_INTF_MIPI_DSI_DATA_TYPE_RGB666_18BIT:
             value = 1;
             break;
        case XMEDIA_INTF_MIPI_DSI_DATA_TYPE_RGB888_24BIT:
             value = 3;
             break;
        default:
            value = 0;
            break;
    }
    //mipi目前没有uv vu type, 类型定为uv
    if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_voutintf1_cfg_dpi_mode(g_p_vout_reg, value);
        hal_vout_set_voutintf1_cfg_dpi_uv_order(g_p_vout_reg, 0);
    } else {
        hal_vout_set_cfg_dpi_mode(g_p_vout_reg, value);
        hal_vout_set_cfg_dpi_uv_order(g_p_vout_reg, 0);
    }
    return;
}

//data lane需要根据寄存器进行更新 如果接666 屏是否要设置dither？？？
xmedia_void hal_vo_set_lvds_intf_cfg(xmedia_vo_dev dev, xmedia_vo_lvds_attr *lvds_attr)
{
    xmedia_u8 value;
    xmedia_u32 lvds_pn_inv;

    hal_vo_set_intf_mux(dev, XMEDIA_INTF_TYPE_LVDS);
    lvds_pn_inv =
        ((lvds_attr->lane_pn_swap[XMEDIA_VO_LVDS_LANE_DATA0]) << 0) |
        ((lvds_attr->lane_pn_swap[XMEDIA_VO_LVDS_LANE_DATA1]) << 1) |
        ((lvds_attr->lane_pn_swap[XMEDIA_VO_LVDS_LANE_DATA2]) << 2) |
        ((lvds_attr->lane_pn_swap[XMEDIA_VO_LVDS_LANE_DATA3]) << 3) |
        ((lvds_attr->lane_pn_swap[XMEDIA_VO_LVDS_LANE_CLK]) << 4);

    if ((lvds_attr->data_type == XMEDIA_INTF_LVDS_DATA_TYPE_RGB666_18BIT) &&
       (lvds_attr->format == XMEDIA_INTF_LVDS_FORMAT_VESA)) {
       value = 0;
    } else if ((lvds_attr->data_type == XMEDIA_INTF_LVDS_DATA_TYPE_RGB888_24BIT) &&
       (lvds_attr->format == XMEDIA_INTF_LVDS_FORMAT_VESA)) {
       value = 1;
    } else if ((lvds_attr->data_type == XMEDIA_INTF_LVDS_DATA_TYPE_RGB666_18BIT) &&
       (lvds_attr->format == XMEDIA_INTF_LVDS_FORMAT_JEIDA)) {
       value = 2;
    } else if ((lvds_attr->data_type == XMEDIA_INTF_LVDS_DATA_TYPE_RGB888_24BIT) &&
       (lvds_attr->format == XMEDIA_INTF_LVDS_FORMAT_JEIDA)) {
       value = 3;
    } else {
       value = 0;
    }
    if (dev == XMEDIA_VO_DEV_1) {
        hal_vout_set_voutintf1_cfg_lvds_mode(g_p_vout_reg, value);
        hal_vout_set_voutintf1_cfg_lvds_lane_4_sel(g_p_vout_reg, lvds_attr->lane_sel[4]);
        hal_vout_set_voutintf1_cfg_lvds_lane_3_sel(g_p_vout_reg, lvds_attr->lane_sel[3]);
        hal_vout_set_voutintf1_cfg_lvds_lane_2_sel(g_p_vout_reg, lvds_attr->lane_sel[2]);
        hal_vout_set_voutintf1_cfg_lvds_lane_1_sel(g_p_vout_reg, lvds_attr->lane_sel[1]);
        hal_vout_set_voutintf1_cfg_lvds_lane_0_sel(g_p_vout_reg, lvds_attr->lane_sel[0]);
        hal_vout_set_cfg_lvds_phy_pn_inv(g_p_vout_reg, lvds_pn_inv);
        hal_vout_set_cfg_lvds_phy_ldo(g_p_vout_reg, 0x3); // Select LDO output voltage
        hal_vout_set_cfg_lvds_phy_update(g_p_vout_reg, XMEDIA_TRUE); //Enable lvds phy
    } else {
        hal_vout_set_cfg_lvds_mode(g_p_vout_reg, value);
        hal_vout_set_cfg_lvds_lane_4_sel(g_p_vout_reg, lvds_attr->lane_sel[4]);
        hal_vout_set_cfg_lvds_lane_3_sel(g_p_vout_reg, lvds_attr->lane_sel[3]);
        hal_vout_set_cfg_lvds_lane_2_sel(g_p_vout_reg, lvds_attr->lane_sel[2]);
        hal_vout_set_cfg_lvds_lane_1_sel(g_p_vout_reg, lvds_attr->lane_sel[1]);
        hal_vout_set_cfg_lvds_lane_0_sel(g_p_vout_reg, lvds_attr->lane_sel[0]);
        hal_vout_set_cfg_lvds_phy_pn_inv(g_p_vout_reg, lvds_pn_inv);
        hal_vout_set_cfg_lvds_phy_ldo(g_p_vout_reg, 0x3); // Select LDO output voltage
        hal_vout_set_cfg_lvds_phy_update(g_p_vout_reg, XMEDIA_TRUE); //Enable lvds phy
    }

    return;
}

static xmedia_s32 vo_check_mipi_phy_pll_clk_step(xmedia_u32 pll_outclk)
{
    if (pll_outclk <= MIPI_PHY_PLL_CLK_35MHZ)
        return 7;
    else if (pll_outclk <= MIPI_PHY_PLL_CLK_75MHZ)
        return 6;
    else if (pll_outclk <= MIPI_PHY_PLL_CLK_150MHZ)
        return 5;
    else if (pll_outclk <= MIPI_PHY_PLL_CLK_300MHZ)
        return 4;
    else if (pll_outclk <= MIPI_PHY_PLL_CLK_600MHZ)
        return 3;
    else if (pll_outclk <= MIPI_PHY_PLL_CLK_1000MHZ)
        return 2;
    else if (pll_outclk <= MIPI_PHY_PLL_CLK_2400MHZ)
        return 1;
    else
        return 0;
}

static xmedia_s32 vo_mipi_pll_g8_to_f8(xmedia_s32 g8)
{
    switch (g8) {
        case 1 :
            return 0;
        case 2 :
            return 1;
        case 3 :
            return 2;
        default:
            return 3;
    }
}
static xmedia_s32 vo_mipi_pll_i8_to_h8(xmedia_s32 i8)
{
    switch (i8) {
    case 11:
        return 7;
    case 10:
        return 6;
    case 9:
        return 5;
    case 8:
        return 4;
    case 7:
        return 3;
    case 6:
        return 2;
    case 5:
        return 1;
    default:
       return 0;
    }
}

static xmedia_void vo_set_mipi_lcd_cfg(xmedia_void)
{
    xmedia_s32 index = 0;
    xmedia_u32 value;
    xmedia_s32 i, j, len;
    xmedia_s32 offset, cycle_times, tmp;

    while(1) {
        value = 0;
        len = g_vo_mipi_lcd.array[index];
        VO_TRACE(MODULE_DBG_DEBUG, "len %u\n", len);
        if (len > 0 && len <= 2) {
            //mipi_phy发短包 6c低8位置0x15
            value = 0x15;
            for (i = 1, offset = 8; i <= len; i++, offset += 8) {
                value |= (((xmedia_u32)g_vo_mipi_lcd.array[index + i] & 0xff) << offset);
            }
            hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x6c), value);
            VO_TRACE(MODULE_DBG_DEBUG, "reg_value_6c 0x%x\n", value);
            index = index + len + 1;
        }else if (len > 2 && len <= 4) {
            //mipi_phy一次下发长包
            for (i = 1, offset = 0; i <= len; i++, offset += 8) {
                value |= (((xmedia_u32)g_vo_mipi_lcd.array[index + i] & 0xff) << offset);
            }
            hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x70), value);
            VO_TRACE(MODULE_DBG_DEBUG, "reg_value_70 0x%x\n", value);
            //mipi_phy下发长包     70低8位置0x39 中8位置数据长度
            value = 0x39 | (len << 8);
            hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x6c), value);
            VO_TRACE(MODULE_DBG_DEBUG, "reg_value_6c 0x%x\n", value);
            index = index + len + 1;
        } else if (len > 4) {
            //mipi_phy多次下发长包
            cycle_times = len / 4;
            tmp = len % 4;
            for (i = 0; i < cycle_times; i++) {
                for (j = 1, offset = 0; j <= 4; j++, offset += 8) {
                    value |= (((xmedia_u32)g_vo_mipi_lcd.array[index + j] & 0xff) << offset);
                }
                hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x70), value);
                VO_TRACE(MODULE_DBG_DEBUG, "reg_value_70 0x%x\n", value);
                index = index + 4;
                value = 0;
            }
            if (tmp != 0) {
                for (i = 1, offset = 0; i <= tmp; i++, offset += 8) {
                    value |= (((xmedia_u32)g_vo_mipi_lcd.array[index + i] & 0xff) << offset);
                }
                hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x70), value);
                VO_TRACE(MODULE_DBG_DEBUG, "reg_value_70 0x%x\n", value);
            }
            // mipi_phy下发长包     70低8位置0x39 中8位置数据长度
            value = 0x39 | (len << 8);
            hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x6c), value);
            VO_TRACE(MODULE_DBG_DEBUG, "reg_value_6c 0x%x\n", value);
            index = index + tmp + 1;
        }else {
            value = g_vo_mipi_lcd.array[index + 1];
            mdelay(value);
            VO_TRACE(MODULE_DBG_DEBUG, "sleep %ums\n", value);
            index += 2;
        }
        if (index == g_vo_mipi_lcd.size) {
            break;
        }
    }

    return;
}


//以下计算依照MIPI PLL Frequency Calculator.excel文档换算而来
xmedia_s32 vo_calculate_mipi_phy_cfg(xmedia_vo_dev dev, xmedia_u32 clk_hz,
    xmedia_vo_mipi_attr *mipi_attr, mipi_phy_config* mipi_phy_cfg)
{
    xmedia_u32 b5_expect_pll_outclk;//预期输出
    xmedia_u32 b8_actual_pll_outclk;//实际输出 excel表格能分出来的频率
    xmedia_s32 d7, r8, p9, p10, p8, g9, g8, o8, e8, i9, i8, m8, k9, k8= 0;
    xmedia_s32 p9_roundup;
    xmedia_s32 expect_lanebyte_clock_hz;//单lane每秒传输的像素字节数
    xmedia_s32 f8_rg_pll_dr_p0, d8_rg_pll_dr_p1, q8_rg_pll_dr_p2, h8_rg_pll_dr_n0, j8_rg_pll_dr_n, l8_rg_pll_dr_s,
        n8_rg_pll_psdiv_en;
    xmedia_u32 b4_vo_pll_fref;
    b4_vo_pll_fref = (VO_MIPI_PLL_FREF == VO_MIPITX_PLL_REF_CKSEL_24M) ? 24 : 36;

    //由于RGB有三个像素分量，因此需要乘以3;再除以lane_num，得到期望的单lane时钟;lanebyte_clock_hz实际上无法达到3/4理论时钟值，因此需要添加10%的余量
    expect_lanebyte_clock_hz = ((clk_hz * 3) / mipi_attr->mipi_lane_num) / 100 * 110;
    VO_TRACE(MODULE_DBG_DEBUG, "expect_lanebyte_clock_hz%d\n", expect_lanebyte_clock_hz);
    //期望pll时钟Mhz，像素时钟*8，转换为bit量级，向上取整
    if (((expect_lanebyte_clock_hz * 8) % 1000000) == 0) {
        b5_expect_pll_outclk = (expect_lanebyte_clock_hz * 8) / 1000000;
    } else {
        b5_expect_pll_outclk = (expect_lanebyte_clock_hz * 8) / 1000000 + 1;
    }
    e8 = b4_vo_pll_fref / 19;
    d8_rg_pll_dr_p1 = e8;

    if (d8_rg_pll_dr_p1 == 0) {
        VO_TRACE(MODULE_DBG_ERR,"d8_rg_pll_dr_p1 err %d \n", d8_rg_pll_dr_p1);
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }
    q8_rg_pll_dr_p2 = vo_check_mipi_phy_pll_clk_step(b5_expect_pll_outclk);

    r8 = 1 << q8_rg_pll_dr_p2;
    d7 = b4_vo_pll_fref / d8_rg_pll_dr_p1;
    p9 = (int)((b5_expect_pll_outclk * r8) / d7);
    p9_roundup = (((b5_expect_pll_outclk * r8) % d7) == 0) ? p9 : (p9 + 1);
    p10 = (p9 % 2 == 0) ? p9 : p9_roundup;
    p8 = (b5_expect_pll_outclk <= MIPI_PHY_PLL_CLK_4000MHZ) ? p9 : p10;

    //实际的pll时钟,向下取整
    b8_actual_pll_outclk = (d7 * p8) / r8;
    g9 = ((p8 % 2) == 0) ? 2 : 1;
    g8 = (b5_expect_pll_outclk <= MIPI_PHY_PLL_CLK_4000MHZ) ? g9 : ((p8 % 3 == 0) ? 3 : 2);
    f8_rg_pll_dr_p0 = vo_mipi_pll_g8_to_f8(g8);

    if ((p8 % 10) == 0 || (p8 % 8) == 0 || (p8 % 15) == 0 || (p8 % 12 == 0)) {
        o8 = 0;
    } else {
        o8 = 1;
    }

    i9 = (o8 == 1) ? (((p9 % 2) == 0) ? 4 : 8) : (((p9 / g8) % 5 == 0) ? 5 : 4);
    i8 = (b5_expect_pll_outclk <= MIPI_PHY_PLL_CLK_4000MHZ) ? i9 :
        ((o8 == 0) ? (((p8 % 8) == 0) ? 4 : (((p8 % 12) == 0) ? 4 : 5)): 4);

    h8_rg_pll_dr_n0 = vo_mipi_pll_i8_to_h8(i8);
    m8 = ((o8 == 1) ? ((p8 / g8) % i8) : 1);
    k9 = (p9 / g8 - m8 * o8) / i8;
    k8 = (b5_expect_pll_outclk <= MIPI_PHY_PLL_CLK_4000MHZ) ? k9 : ((p8 / g8 - m8 * o8) / i8);

    j8_rg_pll_dr_n = k8;
    l8_rg_pll_dr_s = ((m8 - 1) < 0) ? 0 : (m8 - 1);
    n8_rg_pll_psdiv_en = (o8 > 0) ? 1 : 0;

    mipi_phy_cfg->phy_clk_cfg = f8_rg_pll_dr_p0 | ((d8_rg_pll_dr_p1 & 0x3F) << 0x2) | ((q8_rg_pll_dr_p2 & 0x7) << 8) |
        ((h8_rg_pll_dr_n0 & 0x7) << 11) | ((j8_rg_pll_dr_n & 0x3F) << 14) | ((l8_rg_pll_dr_s & 7) << 20) |
        ((n8_rg_pll_psdiv_en & 0x1) << 23);
    //除以8转换为Byte量级
    mipi_phy_cfg->phy_lanebyte_clock_hz = b8_actual_pll_outclk * 1000000 / 8;
    VO_TRACE(MODULE_DBG_DEBUG,"b8_actual_pll_outclk %d b5_expect_pll_outclk %d mipi_phy_cfg->phy_clk_cfg %x phy_lanebyte_clock_hz %d\n",
        b8_actual_pll_outclk, b5_expect_pll_outclk, mipi_phy_cfg->phy_clk_cfg, mipi_phy_cfg->phy_lanebyte_clock_hz);

    mipi_phy_cfg->phy_master_swap =
        ((mipi_attr->lane_pn_swap[XMEDIA_VO_MIPI_LANE_DATA0]) << 0) |
        ((mipi_attr->lane_pn_swap[XMEDIA_VO_MIPI_LANE_DATA1]) << 1) |
        ((mipi_attr->lane_pn_swap[XMEDIA_VO_MIPI_LANE_CLK]) << 2) |
        ((mipi_attr->lane_pn_swap[XMEDIA_VO_MIPI_LANE_DATA2]) << 3) |
        ((mipi_attr->lane_pn_swap[XMEDIA_VO_MIPI_LANE_DATA3]) << 4);
    mipi_phy_cfg->phy_master_swap |=
        ((mipi_attr->lane_sel[XMEDIA_VO_MIPI_LANE_DATA0]) << 5) |
        ((mipi_attr->lane_sel[XMEDIA_VO_MIPI_LANE_DATA1]) << 8) |
        ((mipi_attr->lane_sel[XMEDIA_VO_MIPI_LANE_CLK]) << 11) |
        ((mipi_attr->lane_sel[XMEDIA_VO_MIPI_LANE_DATA2]) << 14) |
        ((mipi_attr->lane_sel[XMEDIA_VO_MIPI_LANE_DATA3]) << 17);

    mipi_phy_cfg->phy_htotal_adjust = mipi_attr->mipi_htotal_adjust;
    mipi_phy_cfg->phy_vsync_adjust = mipi_attr->mipi_vsync_adjust;
    mipi_phy_cfg->phy_lane_num = mipi_attr->mipi_lane_num;
    VO_TRACE(MODULE_DBG_DEBUG,"phy_master_swap %d phy_htotal_adjust %d phy_vsync_adjust %x\n",
        mipi_phy_cfg->phy_master_swap, mipi_phy_cfg->phy_vsync_adjust, mipi_phy_cfg->phy_htotal_adjust);

    return XMEDIA_SUCCESS;
}

xmedia_s32 hal_vo_set_mipi_tx_phy_cfg(xmedia_vo_dev dev, xmedia_u32 frame_rate,
    xmedia_vo_user_sync_timing *sync_timing, mipi_phy_config *mipi_phy_cfg, xmedia_bool special_timing_sel)
{
    xmedia_u32 read_reg;
    xmedia_u32 num = 0;

    //reset controller
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x04), 0x0);
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x04), 0x1);

    //PHY CONFIG
    //phy_rstz =1/phy_shutdownz = 1
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0xa0), 0x7);
    //phy_rstz =0/phy_shutdownz = 0/clk_en = 0
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0xa0), 0);
    //phy_rstz =1/phy_shutdownz = 1
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0xa0), 0x7);
    //n lanes enale/phy_stop_wait_time = 8
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0xa4), (0x800 | (mipi_phy_cfg->phy_lane_num - 1)));
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_phy + 0x4), mipi_phy_cfg->phy_master_swap);

    //MIPI_MASTER_GLOBAL_CTRL
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_phy + 0x138), 0x0);//20241121 0x7 -> 0x1
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_phy + 0x174), 0x1);//20241121 add
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_phy + 0x100), 0xa);
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_phy + 0x108), 0x0);
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_phy + 0x114), 0x3);
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_phy + 0x120), 0x400);
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_phy + 0x124), 0x2);
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_phy + 0x134), 0x8);
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_phy + 0x158), 0x3);
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_phy + 0x204), mipi_phy_cfg->phy_clk_cfg);
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_phy + 0x200), 0x88000041);
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_phy + 0x90), 0x20);
    hal_vout_reg_bit_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_phy), 0x1, 0x1c00, 10); // activate to intensify
    hal_vout_reg_bit_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_phy + 0x8), 0x2, 0xc00, 10); // increase driving force of HS

    /*需要确保phy状态正常以后才继续后面的工作，通过读0x11970200 bit2拉高获取phy正常状态*/
    read_reg = hal_vout_reg_read((xmedia_uintptr_t)(g_p_vout_mipi_tx_phy + 0x200));
    while(1) {
        if(num == 10) {
            VO_TRACE(MODULE_DBG_ERR,"read_reg 0x11970200 failed\n");
            return XMEDIA_FAILURE;
        }
        if((read_reg & 0x2) != 0) {
            VO_TRACE(MODULE_DBG_DEBUG,"read_reg %x \n", read_reg);
            break;
        }
        VO_TRACE(MODULE_DBG_DEBUG,"read_reg %x \n", read_reg);
        num++;
        udelay(4);
        read_reg = hal_vout_reg_read((xmedia_uintptr_t)(g_p_vout_mipi_tx_phy + 0x200));
    }
    //200us 是逻辑mipi phy仿真得到的经验值 后续存在mipi点不亮需注意此处延时是否不够
    udelay(200);

    //power up to controller
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x4), 0x1);
    //clk_division = 90/tx_esc_clk_division = 3
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x8), 0x5a03);
    //24BIT RGB
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x10), 0x5);
    //eotp_tx_lp_en = 1/crc_rx_en = 1/ecc_rx_en = 1/ecc_tx_en = 1
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x2c), 0x3);
    //mode_cfg: 0:video_mode, 1:command mode
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x34), 0x1);
    /* vid_mode_cfg [1:0]
     * 0x0 vidmode0: Non-burst with sync pulses
     * 0x1 vidmode1: Non-burst with sync events
     * 0x2 vidmode2: burst_mode
     * 0x3 vidmode3: burst_mode
     */
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x38), 0x2);
    //vid_pkt_size = 1024
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x3c), sync_timing->hact);
    //vid_num_chunks = 1
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x40), 0x1);
    //vid_hsa_time (clock cycle)
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x48), sync_timing->hpw);
    //vid_hbp_time (clock cycle) = hbb - hpw
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x4c), sync_timing->hbb - sync_timing->hpw);

    /*
     * =INT(1/D4/(D9+D10+D11+D12)/(1/D15/10^6)+0.5)
     * (1/(D4*(D9+D10+D11+D12))) / (1/(D15 * 10^6)) + 0.5
     * (D15 * 10^6) / (D4*(D9+D10+D11+D12)) + 0.5
     * mipi_phy_cfg->phy_lanebyte_clock_hz / (frame_rate * vtotal) + 0.5
     * 4向上对齐
     */
    mipi_phy_cfg->phy_vid_hline_time =
        (((mipi_phy_cfg->phy_lanebyte_clock_hz * 10) /
        (frame_rate * (sync_timing->vbb + sync_timing->vfb + sync_timing->vact)) + 5)) / 10;
    mipi_phy_cfg->phy_vid_hline_time = (mipi_phy_cfg->phy_vid_hline_time / 4 ) * 4 +
        ((mipi_phy_cfg->phy_vid_hline_time % 4) ? 4 : 0);

    VO_TRACE(MODULE_DBG_DEBUG,"mipi_phy_cfg->phy_vid_hline_time %d \n", mipi_phy_cfg->phy_vid_hline_time);
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x50), mipi_phy_cfg->phy_vid_hline_time);
    //vid_vsa_time(vsa_period)
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x54), sync_timing->vpw);
    //vid_vbp_time(vbp_period)) vbp = vbb - vpw
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x58), sync_timing->vbb - sync_timing->vpw);
    //vid_vfp_time(vfp_period)
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x5c), sync_timing->vfb);
    //vid_vactive_time(vact_period)
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x60), sync_timing->vact);
    //phy_txrequestclkhs = 1
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x94), 0x1);
    if (g_vo_mipi_lcd.array != XMEDIA_NULL &&  g_vo_mipi_lcd.size > 0) {
        //下发mipi_lcd屏参 命令模式选择 HS/LP
        hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x68), 0x10f5700);
        vo_set_mipi_lcd_cfg();
        hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x68), 0x0);
    } else {
        hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x68), 0x1001200);//点屏
        hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x6c), 0x1113);
        hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x70), 0x0000);
        hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x6c), 0x2913);
        hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x70), 0x0000);
        hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x68), 0x0);
    }
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x34), 0x0);

    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x38), 0x00002);
    return XMEDIA_SUCCESS;
}

xmedia_void hal_vo_set_mipi_tx_phy_reset(xmedia_void)
{
    //reset controller
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x04), 0x0);
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0x04), 0x1);

    //phy_rstz =0/phy_shutdownz = 0/clk_en = 0
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0xa0), 0x0);
    hal_vout_reg_write((xmedia_uintptr_t)(g_p_vout_mipi_tx_ctrl + 0xa0), 0x7);
}

xmedia_s32 hal_vo_enable_lvds_phy_cfg(xmedia_u32 clk_hz)
{
    xmedia_s32 value;

    hal_vout_set_cfg_lvds_phy_voc_sw(g_p_vout_reg, 0xa);
    if ((clk_hz > LVDS_PHY_PLL_CLK_20MHZ) && (clk_hz <= LVDS_PHY_PLL_CLK_32MHZ)) {
        value = 0x0;
    } else if ((clk_hz > LVDS_PHY_PLL_CLK_32MHZ) && (clk_hz <= LVDS_PHY_PLL_CLK_48MHZ)) {
        value = 0x1;
    } else if ((clk_hz > LVDS_PHY_PLL_CLK_48MHZ) && (clk_hz <= LVDS_PHY_PLL_CLK_66MHZ)) {
        value = 0x2;
    } else if ((clk_hz > LVDS_PHY_PLL_CLK_66MHZ) && (clk_hz <= LVDS_PHY_PLL_CLK_88MHZ)) {
        value = 0x3;
    } else if ((clk_hz > LVDS_PHY_PLL_CLK_88MHZ) && (clk_hz <= LVDS_PHY_PLL_CLK_110MHZ)) {
        value = 0x4;
    } else if ((clk_hz > LVDS_PHY_PLL_CLK_110MHZ) && (clk_hz <= LVDS_PHY_PLL_CLK_132MHZ)) {
        value = 0x5;
    } else if ((clk_hz > LVDS_PHY_PLL_CLK_132MHZ) && (clk_hz <= LVDS_PHY_PLL_CLK_154MHZ)) {
        value = 0x6;
    } else if ((clk_hz > LVDS_PHY_PLL_CLK_154MHZ) && (clk_hz <= LVDS_PHY_PLL_CLK_200MHZ)) {
        value = 0x7;
    } else {
        VO_TRACE(MODULE_DBG_ERR,"%d is out of lvds pll frange(20~200Mhz),default use 154M~200MHZ frange!\n", clk_hz);
        value = 0x7;
    }
    hal_vout_set_cfg_lvds_pll_frange(g_p_vout_reg, value);
    /***********************
     * PLL SSC modulation frequency
     * 2’b00: 30KHz
     * 2’b01: 50KHz
     * 2’b10: 70KHz
     * 2’b11: 100KHz
     **************************/
    hal_vout_set_cfg_lvds_pll_ssc_freq(g_p_vout_reg, 0x2);

    /***********************
     * PLL SSC frequency spread ppm
     * 2’b00: -5000ppm
     * 2’b01: -15500ppm
     * 2’b10: +31000ppm
     * 2’b11: -31000ppm
     **************************/
    hal_vout_set_cfg_lvds_pll_ssc_spread(g_p_vout_reg, 0x2);
    hal_vout_set_cfg_lvds_pll_en(g_p_vout_reg, 1);
    hal_vout_set_cfg_lvds_pll_option(g_p_vout_reg, 0x4);
    hal_vout_set_cfg_lvds_pll_cpi2(g_p_vout_reg, 0xb);
    /*phy电气指标的功能，屏能显示的话可以不设置*/
    hal_vout_set_cfg_lvds_rterm_en(g_p_vout_reg, 1);
    hal_vout_set_cfg_lvds_lane_en(g_p_vout_reg, 0x1f);
    hal_vout_set_cfg_lvds_pd_en(g_p_vout_reg, 0);
    hal_vout_set_cfg_lvds_phy_update(g_p_vout_reg, 0x1);

    return XMEDIA_SUCCESS;
}

xmedia_s32 hal_vo_disable_lvds_phy_cfg(xmedia_void)
{
    hal_vout_set_cfg_lvds_rterm_en(g_p_vout_reg, 0);

    /*Shut down the PLL and the bias circuit*/
    hal_vout_set_cfg_lvds_pd_en(g_p_vout_reg, 1);
    hal_vout_set_cfg_lvds_lane_en(g_p_vout_reg, 0);
    hal_vout_set_cfg_lvds_phy_update(g_p_vout_reg, 0x1);

    return XMEDIA_SUCCESS;
}

