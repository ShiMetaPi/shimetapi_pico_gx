/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include "hal_vo_reg.h"
#include "hal_vo_gamma.h"

extern s_vout_regs_type *g_p_vout_reg;

xmedia_void hal_vo_gamma_enable(xmedia_vo_dev dev, xmedia_bool enable)
{
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_cfg_gamma_en(g_p_vout_reg, enable);
    } else {
        ;
    }
    return;
}

xmedia_s32 hal_vo_gamma_get_table_size(xmedia_void)
{
    return VO_GAMMA_MAP_SIZE * sizeof(xmedia_u8);
}

xmedia_void hal_vo_gamma_set_table(xmedia_vo_dev dev, xmedia_void *gamma_virtual_addr,
    xmedia_u32 gamma_mem_len, xmedia_vo_gamma_table *gamma_table)
{
    xmedia_s32 i;
    for (i = 0;i < VO_GAMMA_MAP_SIZE; i++) {
        ((xmedia_u8 *)gamma_virtual_addr)[i] = gamma_table->gamma_table[i];
    }

    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_cfg_para_2_update(g_p_vout_reg, XMEDIA_TRUE);
    } else {
        ;
    }
    return;
}

xmedia_void hal_vo_gamma_get_table(xmedia_vo_dev dev, xmedia_void *gamma_virtual_addr,
    xmedia_u32 gamma_mem_len, xmedia_vo_gamma_table *gamma_table)
{
    xmedia_s32 i;

    for (i = 0;i < VO_GAMMA_MAP_SIZE; i++) {
        gamma_table->gamma_table[i] = ((xmedia_u8 *)gamma_virtual_addr)[i];
    }
    return;
}

xmedia_void hal_vo_gamma_set_addr(xmedia_vo_dev dev, xmedia_u64 addr)
{
    xmedia_u32 addr_l = addr & 0xFFFFFFFF;
    xmedia_u32 addr_h = (addr & 0xFFFFFFFF00000000) >> 32;
    if (dev == XMEDIA_VO_DEV_0) {
        hal_reg_vout_set_cfg_para_2_addr_h(g_p_vout_reg, addr_h);
        hal_reg_vout_set_cfg_para_2_addr_l(g_p_vout_reg, addr_l);
    } else {
        ;
    }
    return;
}
