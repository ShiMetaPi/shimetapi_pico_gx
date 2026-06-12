/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef DRV_SYS_H
#define DRV_SYS_H

#include "drv_vo_comm.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

typedef enum {
    SYS_VO_VOU_HD_CKSEL,
    SYS_VO_VOU_PPC_CKSEL,
    SYS_VO_LVDS_POR_SRST,
    SYS_VO_MIPITX_PLL_REF_CKSEL,
    SYS_VO_LVDS_CKSEL,
    SYS_VO_LCD_DIV_CFG,
    SYS_VO_PLL_DIV_CFG,
    SYS_VO_VOU_HD_CKEN,
    SYS_VO_VOU_SRST_REQ,
    SYS_VO_VOU_SRST_GET,
    SYS_VO_LCD_DIV_CKEN,
    SYS_VO_PLL_DIV_CKEN,
    SYS_VO_BT_HD_CKSEL,
    SYS_VO_LCD_HD_CKSEL,
    SYS_VO_MIPITX_HD_CKSEL,
    SYS_VO_LVDSTX_HD_CKSEL,
    SYS_VO_BT_PCTRL,
    SYS_VO_LCD_PCTRL,
    SYS_VO_MIPITX_PCTRL,
    SYS_VO_LVDSTX_PCTRL,
    SYS_VO_BT_CKEN,
    SYS_VO_LCD_CKEN,
    SYS_VO_MIPITX_CKEN,
    SYS_VO_LVDSTX_CKEN,
    SYS_MAX
} sys_func_id;

#ifndef xmedia_chn_info
typedef struct {
    xmedia_mod_id mod_id;
    xmedia_s32 dev_id;
    xmedia_s32 chn_id;
} xmedia_chn_info;
#endif

xmedia_s32 drv_sys_mod_init(xmedia_void);
xmedia_void drv_sys_mod_exit(xmedia_void);

xmedia_s32 drv_sys_mod_ctrl(xmedia_chn_info *chn_info, sys_func_id func_id, xmedia_void *io_args);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif /* DRV_SYS_H */

