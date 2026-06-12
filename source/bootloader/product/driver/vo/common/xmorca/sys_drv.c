/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include "common.h"
#include "sys_drv.h"
#include "sys_hal.h"
#include "drv_sys.h"
#include "xmedia_vo.h"
#include "drv_vo_comm.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

#define PHASE         32
#define SYS_MULTI_V   1048576
#define SYS_MULTI_H   1048576



xmedia_s32 sys_mod_init(xmedia_void)
{
    xmedia_s32 ret;
    ret = sys_hal_init();
    return ret;
}

xmedia_s32 sys_mod_exit(xmedia_void)
{
    sys_hal_exit();
    return XMEDIA_SUCCESS;
}


xmedia_s32 sys_mod_ctrl(xmedia_chn_info *chn_info, sys_func_id func_id, xmedia_void *io_args)
{
    xmedia_s32 dev;

    dev = chn_info->dev_id;
    switch (func_id) {
        case SYS_VO_VOU_HD_CKSEL:{
            xmedia_u32 *vou_hd_sel = XMEDIA_NULL;
            vou_hd_sel = (xmedia_u32 *)io_args;
            sys_hal_vo_vou_hd_sel(dev, *vou_hd_sel);
            break;
        }

        case SYS_VO_VOU_PPC_CKSEL: {
            xmedia_u32 *vou_ppc_cksel = XMEDIA_NULL;
            vou_ppc_cksel = (xmedia_u32 *)io_args;
            sys_hal_vo_vou_ppc_cksel(*vou_ppc_cksel);
            break;
        }

        case SYS_VO_LVDS_POR_SRST: {
            xmedia_bool *lvd_por_srst = XMEDIA_NULL;
            lvd_por_srst = (xmedia_bool *)io_args;
            sys_hal_vo_lvds_por_srst(*lvd_por_srst);
            break;
        }

        case SYS_VO_MIPITX_PLL_REF_CKSEL: {
            xmedia_bool *mipitx_pll_ref_cksel = XMEDIA_NULL;
            mipitx_pll_ref_cksel = (xmedia_bool *)io_args;
            sys_hal_vo_mipitx_pll_ref_cksel(*mipitx_pll_ref_cksel);
            break;
        }

        case SYS_VO_LVDS_CKSEL:{
            xmedia_bool *lvds_cksel = XMEDIA_NULL;
            lvds_cksel = (xmedia_bool *)io_args;
            sys_hal_vo_lvds_cksel(dev, *lvds_cksel);
            break;
        }

        case SYS_VO_LCD_DIV_CFG:{
            xmedia_u32 *lcd_div_cfg = XMEDIA_NULL;
            lcd_div_cfg = (xmedia_u32 *)io_args;
            sys_hal_vo_lcd_div_cfg(*lcd_div_cfg);
            break;
        }

        case SYS_VO_PLL_DIV_CFG:{
            xmedia_u32 *pll_div_cfg = XMEDIA_NULL;
            pll_div_cfg = (xmedia_u32 *)io_args;
            if(sys_hal_vo_pll_div_cfg(*pll_div_cfg) != XMEDIA_SUCCESS) {
                return XMEDIA_FAILURE;
            }
            break;
        }

        case SYS_VO_VOU_HD_CKEN:{
            xmedia_bool *vou_hd_cken = XMEDIA_NULL;
            vou_hd_cken = (xmedia_bool *)io_args;
            sys_hal_vo_vou_hd_cken(dev, *vou_hd_cken);
            break;
        }

        case SYS_VO_VOU_SRST_REQ:{
            xmedia_bool *vou_srst_req = XMEDIA_NULL;
            vou_srst_req = (xmedia_bool *)io_args;
            sys_hal_vo_vou_srst_req(*vou_srst_req);
            break;
        }

        case SYS_VO_VOU_SRST_GET:{
            xmedia_bool *vou_srst = XMEDIA_NULL;
            vou_srst = (xmedia_bool *)io_args;
            sys_hal_vo_vou_srst_get(vou_srst);
            break;
        }

        case SYS_VO_LCD_DIV_CKEN:{
            xmedia_bool *lcd_div_cken = XMEDIA_NULL;
            lcd_div_cken = (xmedia_bool *)io_args;
            sys_hal_vo_lcd_div_cken(*lcd_div_cken);
            break;
        }

        case SYS_VO_PLL_DIV_CKEN:{
            xmedia_bool *pll_div_cken = XMEDIA_NULL;
            pll_div_cken = (xmedia_bool *)io_args;
            sys_hal_vo_pll_div_cken(*pll_div_cken);
            break;
        }

        case SYS_VO_BT_HD_CKSEL:{
            xmedia_bool *bt_hd_cksel = XMEDIA_NULL;
            bt_hd_cksel = (xmedia_bool *)io_args;
            sys_hal_vo_bt_hd_cksel(*bt_hd_cksel);
            break;
        }

        case SYS_VO_LCD_HD_CKSEL:{
            xmedia_bool *lcd_hd_cksel = XMEDIA_NULL;
            lcd_hd_cksel = (xmedia_bool *)io_args;
            sys_hal_vo_lcd_hd_cksel(*lcd_hd_cksel);
            break;
        }

        case SYS_VO_MIPITX_HD_CKSEL:{
            xmedia_bool *mipitx_hd_cksel = XMEDIA_NULL;
            mipitx_hd_cksel = (xmedia_bool *)io_args;
            sys_hal_vo_mipitx_hd_cksel(*mipitx_hd_cksel);
            break;
        }

        case SYS_VO_LVDSTX_HD_CKSEL:{
            xmedia_bool *lvdstx_hd_cksel = XMEDIA_NULL;
            lvdstx_hd_cksel = (xmedia_bool *)io_args;
            sys_hal_vo_lvdstx_hd_cksel(*lvdstx_hd_cksel);
            break;
        }

        case SYS_VO_BT_PCTRL:{
            xmedia_bool *bt_pctrl = XMEDIA_NULL;
            bt_pctrl = (xmedia_bool *)io_args;
            sys_hal_vo_bt_pctrl(*bt_pctrl);
            break;
        }

        case SYS_VO_LCD_PCTRL:{
            xmedia_bool *lcd_pctrl = XMEDIA_NULL;
            lcd_pctrl = (xmedia_bool *)io_args;
            sys_hal_vo_lcd_pctrl(*lcd_pctrl);
            break;
        }

        case SYS_VO_MIPITX_PCTRL:{
            xmedia_bool *mipitx_pctrl = XMEDIA_NULL;
            mipitx_pctrl = (xmedia_bool *)io_args;
            sys_hal_vo_mipitx_pctrl(*mipitx_pctrl);
            break;
        }

        case SYS_VO_LVDSTX_PCTRL:{
            xmedia_bool *lvdstx_pctrl = XMEDIA_NULL;
            lvdstx_pctrl = (xmedia_bool *)io_args;
            sys_hal_vo_lvdstx_pctrl(*lvdstx_pctrl);
            break;
        }

        case SYS_VO_BT_CKEN:{
            xmedia_bool *bt_cken = XMEDIA_NULL;
            bt_cken = (xmedia_bool *)io_args;
            sys_hal_vo_bt_cken(*bt_cken);
            break;
        }

        case SYS_VO_LCD_CKEN:{
            xmedia_bool *lcd_cken = XMEDIA_NULL;
            lcd_cken = (xmedia_bool *)io_args;
            sys_hal_vo_lcd_cken(*lcd_cken);
            break;
        }

        case SYS_VO_MIPITX_CKEN:{
            xmedia_bool *mipitx_cken = XMEDIA_NULL;
            mipitx_cken = (xmedia_bool *)io_args;
            sys_hal_vo_mipitx_cken(*mipitx_cken);
            break;
        }

        case SYS_VO_LVDSTX_CKEN:{
            xmedia_bool *lvdstx_cken = XMEDIA_NULL;
            lvdstx_cken = (xmedia_bool *)io_args;
            sys_hal_vo_lvdstx_cken(*lvdstx_cken);
            break;
        }


        default: {
            return XMEDIA_FAILURE;
        }
    }
    return XMEDIA_SUCCESS;
}

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */
