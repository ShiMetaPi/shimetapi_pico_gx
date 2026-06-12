/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __HAL_VO_CSC_H__
#define __HAL_VO_CSC_H__

#include "drv_vo_comm.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    xmedia_s32 csc_coef0;
    xmedia_s32 csc_coef1;
    xmedia_s32 csc_coef2;

    xmedia_s32 csc_coef3;
    xmedia_s32 csc_coef4;
    xmedia_s32 csc_coef5;

    xmedia_s32 csc_coef6;
    xmedia_s32 csc_coef7;
    xmedia_s32 csc_coef8;

    xmedia_s32 csc_offset_in0;
    xmedia_s32 csc_offset_in1;
    xmedia_s32 csc_offset_in2;
    xmedia_s32 csc_offset_out0;
    xmedia_s32 csc_offset_out1;
    xmedia_s32 csc_offset_out2;
} vo_csc_coef;

xmedia_s32 hal_vo_csc_coef_convert(vo_pic_info *src_pic_info, vo_csc_coef *pst_csc_matrix);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_VO_IP_CSC_H__ */
