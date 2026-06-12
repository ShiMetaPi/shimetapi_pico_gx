/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __HAL_VO_GAMMA_H__
#define __HAL_VO_GAMMA_H__

#include "drv_vo_comm.h"

#ifdef __cplusplus
extern "C" {
#endif

xmedia_void hal_vo_gamma_enable(xmedia_vo_dev dev, xmedia_bool enable);
xmedia_s32 hal_vo_gamma_get_table_size(xmedia_void);
xmedia_void hal_vo_gamma_set_table(xmedia_vo_dev dev, xmedia_void *gamma_virtual_addr,
    xmedia_u32 gamma_mem_len, xmedia_vo_gamma_table *gamma_table);
xmedia_void hal_vo_gamma_get_table(xmedia_vo_dev dev, xmedia_void *gamma_virtual_addr,
    xmedia_u32 gamma_mem_len, xmedia_vo_gamma_table *gamma_table);
xmedia_void hal_vo_gamma_set_addr(xmedia_vo_dev dev, xmedia_u64 addr);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_VO_IP_CSC_H__ */

