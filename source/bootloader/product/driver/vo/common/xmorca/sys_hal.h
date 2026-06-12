/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#ifndef SYS_HAL_H
#define SYS_HAL_H

#include "drv_vo_comm.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

xmedia_s32 sys_hal_init(xmedia_void);
xmedia_void sys_hal_exit(xmedia_void);

xmedia_void sys_hal_vo_vou_hd_sel(xmedia_s32 vo_dev, xmedia_u32 vou_hd_sel);
xmedia_void sys_hal_vo_vou_ppc_cksel(xmedia_u32 vou_ppc_cksel);
xmedia_void sys_hal_vo_lvds_por_srst(xmedia_bool lvd_por_srst);
xmedia_void sys_hal_vo_mipitx_pll_ref_cksel(xmedia_bool mipitx_pll_ref_cksel);
xmedia_void sys_hal_vo_lvds_cksel(xmedia_s32 vo_dev, xmedia_bool lvds_cksel);
xmedia_void sys_hal_vo_lcd_div_cfg(xmedia_u32 lcd_div_cfg);
xmedia_s32 sys_hal_vo_pll_div_cfg(xmedia_u32 pll_div_cfg);
xmedia_void sys_hal_vo_vou_hd_cken(xmedia_s32 vo_dev, xmedia_bool vou_hd_cken);
xmedia_void sys_hal_vo_vou_srst_req(xmedia_bool vou_srst_req);
xmedia_s32 sys_hal_vo_vou_srst_get(xmedia_bool* vou_srst_req);
xmedia_void sys_hal_vo_lcd_div_cken(xmedia_bool lcd_div_cken);
xmedia_void sys_hal_vo_pll_div_cken(xmedia_bool pll_div_cken);
xmedia_void sys_hal_vo_bt_hd_cksel(xmedia_bool bt_hd_cksel);
xmedia_void sys_hal_vo_lcd_hd_cksel(xmedia_bool lcd_hd_cksel);
xmedia_void sys_hal_vo_mipitx_hd_cksel(xmedia_bool mipitx_hd_cksel);
xmedia_void sys_hal_vo_lvdstx_hd_cksel(xmedia_bool lvdstx_hd_cksel);
xmedia_void sys_hal_vo_bt_pctrl(xmedia_bool bt_pctrl);
xmedia_void sys_hal_vo_lcd_pctrl(xmedia_bool lcd_pctrl);
xmedia_void sys_hal_vo_mipitx_pctrl(xmedia_bool mipitx_pctrl);
xmedia_void sys_hal_vo_lvdstx_pctrl(xmedia_bool lvdstx_pctrl);
xmedia_void sys_hal_vo_bt_cken(xmedia_bool bt_cken);
xmedia_void sys_hal_vo_lcd_cken(xmedia_bool lcd_cken);
xmedia_void sys_hal_vo_mipitx_cken(xmedia_bool mipitx_cken);
xmedia_void sys_hal_vo_lvdstx_cken(xmedia_bool lvdstx_cken);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef SYS_HAL_H */
