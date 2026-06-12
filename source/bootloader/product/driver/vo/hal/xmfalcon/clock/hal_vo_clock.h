/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __HAL_VO_CLOCK_H__
#define __HAL_VO_CLOCK_H__

#include "drv_vo_comm.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
    VO_MIPITX_PLL_REF_CKSEL_24M = 0,
    VO_MIPITX_PLL_REF_CKSEL_36M,
    VO_MIPITX_PLL_REF_CKSEL_MAX
} vo_mipitx_pll_ref_cksel;

#define LCD_FREQUENCY_DIVIDER_MAX (148.5 * 1000 * 1000)
#define PLL_FREQUENCY_DIVIDER_MAX (297 * 1000 * 1000)

#define LCD_DIV_2_POWER_27 (1 << 27)        //2的27次方
#define LCD_DIV_2_POWER_24 (1 << 24)        //2的24次方
#define LCD_DIV_SORCE_FRE  (1375*1000*1000) //lcd源频率

#define VO_CLOCK_SRC_13_5MHZ 13500000
#define VO_CLOCK_SRC_27MHZ   27000000
#define VO_CLOCK_SRC_37_125MHZ 37125000
#define VO_CLOCK_SRC_74_25MHZ   74250000
#define VO_CLOCK_SRC_148_5MHZ   148500000
#define VO_CLOCK_SRC_297MHZ     297000000
#define VO_CLOCK_SRC_343_75MHZ     343750000

typedef enum {
    VO_CLOCK_SRC_SEL_297MHZ,
    VO_CLOCK_SRC_SEL_148_5MHZ,
    VO_CLOCK_SRC_SEL_74_25MHZ,
    VO_CLOCK_SRC_SEL_37_125MHZ,
    VO_CLOCK_SRC_SEL_27MHZ,
    VO_CLOCK_SRC_SEL_13_5MHZ,
    VO_CLOCK_SRC_SEL_LCD,
    VO_CLOCK_SRC_SEL_APLL_FOUT1,
    VO_CLOCK_SRC_SEL_343_75MHZ,
    VO_CLOCK_SRC_SEL_MAX,
}vo_clock_src_sel;

typedef enum {
    VO_CLOCK_SRC_PPC_SEL_343MHZ,
    VO_CLOCK_SRC_PPC_SEL_297MHZ,
    VO_CLOCK_SRC_PPC_SEL_148_5MHZ,
    VO_CLOCK_SRC_PPC_SEL_74_25MHZ,
    VO_CLOCK_SRC_PPC_SEL_MAX,
}vo_clock_ppc_sel;

typedef enum {
    VO_DIV_CLOCK_TYPE_LCD,
    VO_DIV_CLOCK_TYPE_MAX,
} vo_div_clock_type;

typedef enum {
    VO_CRG_CLOCK_TYPE_MIPI,
    VO_CRG_CLOCK_TYPE_LVDS,
    VO_CRG_CLOCK_TYPE_BT,
    VO_CRG_CLOCK_TYPE_LCD,
    VO_CRG_CLOCK_TYPE_MAX,
} vo_crg_intf_type;

typedef enum {
    VO_CLOCK_PRIORITY_SEL_NORMAL,
    VO_CLOCK_PRIORITY_SEL_LCD,
    VO_CLOCK_PRIORITY_SEL_FIX,
    VO_CLOCK_PRIORITY_SEL_MAX,
} vo_clock_priority_sel;

typedef struct {
    xmedia_bool div_clock_en;       //分频器使能状态 dev0 dev1 共用同一分频器时 需判断两个dev的使能状态来使能分频器
    xmedia_u32 div_clock_hz;        //分频器频率
} vo_div_clock_info;

typedef struct {
    vo_clock_src_sel clock_sel;                         //dev 时钟源选择
    xmedia_bool vo_lvds_sel;                            //lvds时需要配1 选择LVDS TXPHY PLL_CLKOUT，否则配0 选择SOC时钟
    xmedia_u32 clock_hz;
    xmedia_u32 fixed_clock_hz;                          //固定频点时钟
    vo_div_clock_info freq_clock[VO_DIV_CLOCK_TYPE_MAX];//dev 分频器信息
    xmedia_bool dev_clock_gate;                         //dev时钟门控
} vo_dev_clock_info;

typedef struct {
    xmedia_bool intf_clock_gate;        //intf时钟门控
    xmedia_vo_dev intf_sel_dev;         //接口时钟通道选择
    xmedia_bool intf_reverse_en;
} vo_intf_clock_info;


typedef struct {
    xmedia_bool vo_top_clock_gate;                              //vo top层动态时钟门控
    vo_dev_clock_info dev_clock_info[XMEDIA_VO_DEV_MAX];        //dev时钟信息
    vo_intf_clock_info intf_clock_info[VO_CRG_CLOCK_TYPE_MAX];  //intf时钟信息
} vo_clock_config;

#define VO_MIPI_PLL_FREF VO_MIPITX_PLL_REF_CKSEL_24M  //24MHz pll原始频率

xmedia_void hal_vo_clock_init(vo_clock_config * vo_clock);
xmedia_void hal_vo_clock_deinit(xmedia_void);
xmedia_void hal_vo_set_clock_gate(vo_clock_config * vo_clock, xmedia_bool enable);
xmedia_s32 hal_vo_set_dev_div_clock(vo_clock_config * vo_clock, xmedia_vo_dev dev, xmedia_u32 clk_hz,
    xmedia_bool *vo_special_timing_sel, xmedia_bool vo_lcd_divider_priority);

xmedia_void hal_vo_set_dev_div_clock_enable(vo_clock_config * vo_clock, xmedia_vo_dev dev,
    xmedia_bool clk_en);
xmedia_void hal_vo_set_clock_reset_enable(vo_clock_config * vo_clock, xmedia_bool reset_en);
xmedia_bool hal_vo_get_clock_reset_sta(xmedia_void);
xmedia_void hal_vo_set_dev_clock_enable(vo_clock_config * vo_clock, xmedia_vo_dev dev,
    xmedia_bool clk_en);
xmedia_s32 hal_vo_set_intf_clock(vo_clock_config * vo_clock, xmedia_vo_dev dev, xmedia_bool clk_reverse_en,
    xmedia_intf_type intf_type);
xmedia_void hal_vo_set_intf_clock_enable(vo_clock_config * vo_clock, xmedia_intf_type intf_type,
    xmedia_bool clock_enable);
xmedia_void hal_vo_set_intf_detach_dev(vo_clock_config * vo_clock, xmedia_vo_dev dev,
    xmedia_intf_type intf_type);
xmedia_s32 hal_vo_get_intf_clock(vo_clock_config * vo_clock, xmedia_vo_dev dev, xmedia_u32 *clock_hz);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_VO_IP_CSC_H__ */

