/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#ifdef VO_BOOT
#include <string.h>
#include "xmedia_vo.h"
#endif
#include "hal_vo.h"
#include "hal_vo_disp.h"
#include "hal_vo_reg.h"
#include "sys_drv.h"
#include "hal_vo_clock.h"

extern s_vout_regs_type *g_p_vout_reg;

xmedia_void hal_vo_clock_init(vo_clock_config * vo_clock)
{
    xmedia_u32 i;
    osal_memset(vo_clock, 0, sizeof(vo_clock_config));
    for(i = XMEDIA_VO_DEV_0;i < XMEDIA_VO_REAL_DEV_NUM; i++) {
        vo_clock->dev_clock_info[i].clock_sel = VO_CLOCK_SRC_SEL_MAX;
    }
    for(i = VO_CRG_CLOCK_TYPE_MCU;i < VO_CRG_CLOCK_TYPE_MAX; i++) {
        vo_clock->intf_clock_info[i].intf_sel_dev = XMEDIA_VO_REAL_DEV_NUM;
    }
}

xmedia_void hal_vo_clock_deinit(xmedia_void)
{
    return;
}

//vo top 门控使能
xmedia_void hal_vo_set_clock_gate(vo_clock_config * vo_clock, xmedia_bool enable)
{
    hal_reg_vout_set_cfg_ck_dyn_gt_en_dma(g_p_vout_reg, enable);
    hal_reg_vout_set_cfg_ck_dyn_gt_en(g_p_vout_reg, enable);
    vo_clock->vo_top_clock_gate = enable;
    return;
}

static vo_clock_src_sel vo_fix_clk_sel(vo_clock_config * vo_clock, xmedia_vo_dev dev, xmedia_u32 clk_hz)
{
    if ((0 < clk_hz) && (clk_hz <= VO_CLOCK_SRC_13_5MHZ)) {
        vo_clock->dev_clock_info[dev].fixed_clock_hz = VO_CLOCK_SRC_13_5MHZ;
        return VO_CLOCK_SRC_SEL_13_5MHZ;
    } else if ((VO_CLOCK_SRC_13_5MHZ < clk_hz) && (clk_hz <= VO_CLOCK_SRC_27MHZ)) {
        vo_clock->dev_clock_info[dev].fixed_clock_hz = VO_CLOCK_SRC_27MHZ;
        return VO_CLOCK_SRC_SEL_27MHZ;
    } else if ((VO_CLOCK_SRC_27MHZ < clk_hz) && (clk_hz <= VO_CLOCK_SRC_37_125MHZ)) {
        vo_clock->dev_clock_info[dev].fixed_clock_hz = VO_CLOCK_SRC_37_125MHZ;
        return VO_CLOCK_SRC_SEL_37_125MHZ;
    } else if ((VO_CLOCK_SRC_37_125MHZ < clk_hz) && (clk_hz <= VO_CLOCK_SRC_74_25MHZ)) {
        vo_clock->dev_clock_info[dev].fixed_clock_hz = VO_CLOCK_SRC_74_25MHZ;
        return VO_CLOCK_SRC_SEL_74_25MHZ;
    } else if ((VO_CLOCK_SRC_74_25MHZ < clk_hz) && (clk_hz <= VO_CLOCK_SRC_148_5MHZ)) {
        vo_clock->dev_clock_info[dev].fixed_clock_hz = VO_CLOCK_SRC_148_5MHZ;
        return VO_CLOCK_SRC_SEL_148_5MHZ;
    } else {
        VO_TRACE(MODULE_DBG_DEBUG, "not match normal Hz! Use LCD or PLL Hz!\n");
        return VO_CLOCK_SRC_SEL_MAX;
    }
}

static vo_clock_ppc_sel vo_get_ppc_clock(vo_clock_config *vo_clock)
{
    xmedia_u32 i;
    xmedia_u32 intf_clk_hz = 0;
    xmedia_u32 ppc_clk_hz = 0;

    for(i = XMEDIA_VO_DEV_0;i < XMEDIA_VO_DEV_1; i++) {
        hal_vo_get_intf_clock(vo_clock, i, &intf_clk_hz);
        ppc_clk_hz = (intf_clk_hz > ppc_clk_hz) ? intf_clk_hz : ppc_clk_hz;
    }

    if (ppc_clk_hz <= 50000000) {
        return VO_CLOCK_SRC_PPC_SEL_50MHZ;
    } else if (ppc_clk_hz <= 100000000) {
        return VO_CLOCK_SRC_PPC_SEL_100MHZ;
    } else if (ppc_clk_hz <= 150000000) {
        return VO_CLOCK_SRC_PPC_SEL_150MHZ;
    } else if (ppc_clk_hz <= 200000000) {
        return VO_CLOCK_SRC_PPC_SEL_200MHZ;
    } else {
        return VO_CLOCK_SRC_PPC_SEL_MAX;
    }
}

static vo_clock_src_sel vo_div_clk_sel(vo_clock_config *vo_clock, xmedia_vo_dev dev,
    xmedia_u32 clk_hz)
{
    xmedia_bool lcd_use_flag = XMEDIA_TRUE;
    xmedia_u32 i;

    //判断lcd分频能否使用
    if (clk_hz <= LCD_FREQUENCY_DIVIDER_MAX) {
        for(i = XMEDIA_VO_DEV_0;i < XMEDIA_VO_REAL_DEV_NUM; i++) {
            //lcd分频已使能&&不是本dev使用&&分频频率不同 则无法使用lcd分频
            if ((vo_clock->dev_clock_info[i].clock_sel == VO_CLOCK_SRC_SEL_LCD) &&
                (i != dev) && (vo_clock->dev_clock_info[i].freq_clock[VO_DIV_CLOCK_TYPE_LCD].div_clock_hz != clk_hz)) {
                lcd_use_flag = XMEDIA_FALSE;
            }
        }
        //配置lcd分频
        if (lcd_use_flag == XMEDIA_TRUE) {
            return VO_CLOCK_SRC_SEL_LCD;
        }
    }
    return VO_CLOCK_SRC_SEL_MAX;
}

static vo_div_clock_type vo_clk_sel_to_div_type(vo_clock_src_sel clk_sel)
{
    switch (clk_sel) {
        case VO_CLOCK_SRC_SEL_LCD:
            return VO_DIV_CLOCK_TYPE_LCD;
        default :
            return VO_DIV_CLOCK_TYPE_MAX;
    }
}

//crg 分频时钟配置
xmedia_s32 hal_vo_set_dev_div_clock(vo_clock_config * vo_clock, xmedia_vo_dev dev, xmedia_u32 clk_hz,
    xmedia_bool *vo_special_timing_sel, xmedia_bool vo_lcd_divider_priority)
{
    vo_clock_src_sel clk_sel;
    vo_clock_ppc_sel vo_ppc_sel;
    vo_div_clock_type div_type;

    xmedia_chn_info vo_chn_info = {0};
    vo_chn_info.mod_id = MOD_ID_VO;
    vo_chn_info.dev_id = dev;
    vo_chn_info.chn_id = 0;
    //debug状态位检测，优先使用lcd分频器 or 优先使用固定频点
    if (vo_lcd_divider_priority == XMEDIA_TRUE) {
        clk_sel = vo_div_clk_sel(vo_clock, dev, clk_hz);
        if (clk_sel == VO_CLOCK_SRC_SEL_MAX) {
            //分频器无法使用，尝试使用固定频点
            clk_sel = vo_fix_clk_sel(vo_clock, dev, clk_hz);
            if (clk_sel == VO_CLOCK_SRC_SEL_MAX) {
                return XMEDIA_ERRCODE_NOT_SUPPORT;
            }
            vo_clock->dev_clock_info[dev].clock_sel = clk_sel;
        } else  {
            div_type = vo_clk_sel_to_div_type(clk_sel);
            if (div_type == VO_DIV_CLOCK_TYPE_MAX) {
                return XMEDIA_ERRCODE_NOT_SUPPORT;
            }
            //使用lcd分频器
            vo_clock->dev_clock_info[dev].clock_sel = clk_sel;
            vo_clock->dev_clock_info[dev].fixed_clock_hz = 0;
            vo_clock->dev_clock_info[dev].freq_clock[div_type].div_clock_hz = clk_hz;
            drv_sys_mod_ctrl(&vo_chn_info, SYS_VO_LCD_DIV_CFG, &clk_hz);
        }
    } else {
        clk_sel = vo_fix_clk_sel(vo_clock, dev, clk_hz);
        if (clk_sel != VO_CLOCK_SRC_SEL_MAX) {
            //使用固定频点
            vo_clock->dev_clock_info[dev].clock_sel = clk_sel;
        } else {
            clk_sel = vo_div_clk_sel(vo_clock, dev, clk_hz);
            if (clk_sel == VO_CLOCK_SRC_SEL_MAX) {
                return XMEDIA_ERRCODE_NOT_SUPPORT;
            }
            div_type = vo_clk_sel_to_div_type(clk_sel);
            if (div_type == VO_DIV_CLOCK_TYPE_MAX) {
                return XMEDIA_ERRCODE_NOT_SUPPORT;
            }
            //使用lcd分频
            vo_clock->dev_clock_info[dev].clock_sel = clk_sel;
            vo_clock->dev_clock_info[dev].fixed_clock_hz = 0;
            vo_clock->dev_clock_info[dev].freq_clock[div_type].div_clock_hz = clk_hz;
            drv_sys_mod_ctrl(&vo_chn_info, SYS_VO_LCD_DIV_CFG, &clk_hz);
        }
    }

    //都使用校正后的特殊时序,不同的是依照 分频器时钟值/固定频点值 计算出不同的矫正时序
    *vo_special_timing_sel = XMEDIA_TRUE;

    //时钟源选择配置
    drv_sys_mod_ctrl(&vo_chn_info, SYS_VO_VOU_HD_CKSEL, &vo_clock->dev_clock_info[dev].clock_sel);

    vo_clock->dev_clock_info[dev].clock_hz = clk_hz;

    //ppc 时钟配置
    vo_ppc_sel = vo_get_ppc_clock(vo_clock);
    if (vo_ppc_sel != VO_CLOCK_SRC_PPC_SEL_MAX) {
        drv_sys_mod_ctrl(&vo_chn_info, SYS_VO_VOU_PPC_CKSEL, &vo_ppc_sel);
    }else {
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }
    return XMEDIA_SUCCESS;
}

//crg时钟 分频器使能
xmedia_void hal_vo_set_dev_div_clock_enable(vo_clock_config * vo_clock,
    xmedia_vo_dev dev, xmedia_bool clk_en)
{
    xmedia_bool lcd_div_freq_en = XMEDIA_FALSE;
    xmedia_u32 i;
    xmedia_chn_info vo_chn_info = {0};
    vo_chn_info.mod_id = MOD_ID_VO;
    vo_chn_info.dev_id = dev;
    vo_chn_info.chn_id = 0;

    if (vo_clock->dev_clock_info[dev].clock_sel == VO_CLOCK_SRC_SEL_LCD) {
        vo_clock->dev_clock_info[dev].freq_clock[VO_DIV_CLOCK_TYPE_LCD].div_clock_en = clk_en;
    }
    //有可能两个dev用同一个分屏器的频点，判断是否是所有dev分频器的使能状态
    for(i = XMEDIA_VO_DEV_0;i < XMEDIA_VO_REAL_DEV_NUM; i++) {
        lcd_div_freq_en |= vo_clock->dev_clock_info[i].freq_clock[VO_DIV_CLOCK_TYPE_LCD].div_clock_en;
    }
    drv_sys_mod_ctrl(&vo_chn_info, SYS_VO_LCD_DIV_CKEN, &lcd_div_freq_en);
    return;
}

//crg时钟 vou软复位请求
xmedia_void hal_vo_set_clock_reset_enable(vo_clock_config * vo_clock, xmedia_bool reset_en)
{
    xmedia_chn_info vo_chn_info = {0};
    xmedia_u32 i;
    vo_chn_info.mod_id = MOD_ID_VO;
    vo_chn_info.dev_id = 0;
    vo_chn_info.chn_id = 0;

    drv_sys_mod_ctrl(&vo_chn_info, SYS_VO_VOU_SRST_REQ, &reset_en);
    //软复位 暂定状态将会改变 其他配置呢 div配置及门控使能 intf门控使能等????
    for(i = XMEDIA_VO_DEV_0;i < XMEDIA_VO_REAL_DEV_NUM; i++) {
        vo_clock->dev_clock_info[i].dev_clock_gate = XMEDIA_FALSE;
    }
}

xmedia_bool hal_vo_get_clock_reset_sta(xmedia_void)
{
    xmedia_bool state;
    xmedia_chn_info vo_chn_info = {0};
    vo_chn_info.mod_id = MOD_ID_VO;
    vo_chn_info.dev_id = 0;
    vo_chn_info.chn_id = 0;
    drv_sys_mod_ctrl(&vo_chn_info, SYS_VO_VOU_SRST_GET, &state);
    return state;
}

//crg时钟 vou dev0/dev1门控使能
xmedia_void hal_vo_set_dev_clock_enable(vo_clock_config * vo_clock,
    xmedia_vo_dev dev, xmedia_bool clk_en)
{
    xmedia_chn_info vo_chn_info = {0};
    vo_chn_info.mod_id = MOD_ID_VO;
    vo_chn_info.dev_id = dev;
    vo_chn_info.chn_id = 0;

    drv_sys_mod_ctrl(&vo_chn_info, SYS_VO_VOU_HD_CKEN, &clk_en);
    vo_clock->dev_clock_info[dev].dev_clock_gate = clk_en;

    return;
}
xmedia_bool vo_check_intf_sel_dev(vo_clock_config * vo_clock,
    vo_crg_intf_type crg_int_type, xmedia_vo_dev dev)
{
    xmedia_u32 i;
    for(i = VO_CRG_CLOCK_TYPE_MCU;i < VO_CRG_CLOCK_TYPE_MAX; i++) {
        //dev是否已经被其他intf选定了
        if ((vo_clock->intf_clock_info[i].intf_sel_dev == dev) && (crg_int_type != i)) {
            return XMEDIA_FAILURE;
        }
    }
    return XMEDIA_SUCCESS;
}

static vo_crg_intf_type hal_vo_swap_intf_type(xmedia_intf_type intf_type)
{
    switch(intf_type) {
        case XMEDIA_INTF_TYPE_BT656:
        case XMEDIA_INTF_TYPE_BT1120:
            return VO_CRG_CLOCK_TYPE_BT;
        case XMEDIA_INTF_TYPE_LCD:
            return VO_CRG_CLOCK_TYPE_LCD;
        case XMEDIA_INTF_TYPE_MCU:
            return VO_CRG_CLOCK_TYPE_MCU;
        default:
            return VO_CRG_CLOCK_TYPE_MAX;
    }
}

xmedia_void hal_vo_set_intf_detach_dev(vo_clock_config * vo_clock,
    xmedia_vo_dev dev, xmedia_intf_type intf_type)
{
    xmedia_u32 i;

    for(i = VO_CRG_CLOCK_TYPE_MCU;i < VO_CRG_CLOCK_TYPE_MAX; i++) {
        if ((vo_clock->intf_clock_info[i].intf_sel_dev == dev) && (hal_vo_swap_intf_type(intf_type) == i)) {
            vo_clock->intf_clock_info[i].intf_sel_dev = XMEDIA_VO_REAL_DEV_NUM;
        }
    }
    return;
}

//crg intf 时钟 通道选择
xmedia_s32 hal_vo_set_intf_clock(vo_clock_config * vo_clock,
    xmedia_vo_dev dev, xmedia_bool clk_reverse_en, xmedia_intf_type intf_type)
{
    xmedia_chn_info vo_chn_info = {0};
    vo_chn_info.mod_id = MOD_ID_VO;
    vo_chn_info.dev_id = dev;
    vo_chn_info.chn_id = 0;

    switch(intf_type) {
        case XMEDIA_INTF_TYPE_BT656:
        case XMEDIA_INTF_TYPE_BT1120:
            if (vo_check_intf_sel_dev(vo_clock, VO_CRG_CLOCK_TYPE_BT, dev) == XMEDIA_SUCCESS) {
                drv_sys_mod_ctrl(&vo_chn_info, SYS_VO_BT_HD_CKSEL, &dev);                //bt接口时钟通道选择 hd0 hd1
                drv_sys_mod_ctrl(&vo_chn_info, SYS_VO_BT_PCTRL, &clk_reverse_en);        //bt接口时钟反向
                vo_clock->intf_clock_info[VO_CRG_CLOCK_TYPE_BT].intf_sel_dev = dev;
                vo_clock->intf_clock_info[VO_CRG_CLOCK_TYPE_BT].intf_reverse_en = clk_reverse_en;
                break;
            } else {
                return XMEDIA_ERRCODE_NOT_SUPPORT;
            }
        case XMEDIA_INTF_TYPE_LCD:
            if (vo_check_intf_sel_dev(vo_clock, VO_CRG_CLOCK_TYPE_LCD, dev) == XMEDIA_SUCCESS) {
                drv_sys_mod_ctrl(&vo_chn_info, SYS_VO_LCD_HD_CKSEL, &dev);               //lcd接口时钟通道选择 hd0 hd1
                drv_sys_mod_ctrl(&vo_chn_info, SYS_VO_LCD_PCTRL, &clk_reverse_en);       //lcd接口时钟反向
                vo_clock->intf_clock_info[VO_CRG_CLOCK_TYPE_LCD].intf_sel_dev = dev;
                vo_clock->intf_clock_info[VO_CRG_CLOCK_TYPE_LCD].intf_reverse_en = clk_reverse_en;
                break;
            } else {
                return XMEDIA_ERRCODE_NOT_SUPPORT;
            }
        case XMEDIA_INTF_TYPE_MCU:
            //todo ???? mcu 接口时钟
            break;
        default:
            VO_TRACE(MODULE_DBG_ERR, "intf type error!\n");
            return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    return XMEDIA_SUCCESS;
}

//crg intf 接口时钟门控使能
xmedia_void hal_vo_set_intf_clock_enable(vo_clock_config * vo_clock,
    xmedia_intf_type intf_type, xmedia_bool clock_enable)
{
    xmedia_chn_info vo_chn_info = {0};
    vo_chn_info.mod_id = MOD_ID_VO;
    vo_chn_info.dev_id = 0;
    vo_chn_info.chn_id = 0;

    switch(intf_type) {
        case XMEDIA_INTF_TYPE_BT656:
        case XMEDIA_INTF_TYPE_BT1120:
            drv_sys_mod_ctrl(&vo_chn_info, SYS_VO_BT_CKEN, &clock_enable);           //bt时钟使能
            break;
        case XMEDIA_INTF_TYPE_LCD:
            drv_sys_mod_ctrl(&vo_chn_info, SYS_VO_LCD_CKEN, &clock_enable);          //lcd时钟使能
            break;
        case XMEDIA_INTF_TYPE_MCU:
            break;
        default:
            VO_TRACE(MODULE_DBG_ERR, "intf type error!\n");
            return;
    }
    vo_clock->intf_clock_info[hal_vo_swap_intf_type(intf_type)].intf_clock_gate = clock_enable;
    return;
}

//获取接口实际配置时钟
xmedia_s32 hal_vo_get_intf_clock(vo_clock_config * vo_clock, xmedia_vo_dev dev, xmedia_u32 *clock_hz)
{
    vo_div_clock_type div_type;
    if (vo_clock->dev_clock_info[dev].clock_sel == VO_CLOCK_SRC_SEL_MAX) {
        VO_TRACE(MODULE_DBG_ERR, "clock_sel error!\n");
        return XMEDIA_FAILURE;
    }

    if (vo_clock->dev_clock_info[dev].clock_sel != VO_CLOCK_SRC_SEL_LCD) {
         *clock_hz = vo_clock->dev_clock_info[dev].fixed_clock_hz;
    } else {
        div_type = vo_clk_sel_to_div_type(vo_clock->dev_clock_info[dev].clock_sel);
        *clock_hz = vo_clock->dev_clock_info[dev].freq_clock[div_type].div_clock_hz;
    }
    return XMEDIA_SUCCESS;
}
