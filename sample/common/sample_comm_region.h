/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __SAMPLE_COMM_REGION_H__
#define __SAMPLE_COMM_REGION_H__

#include "xmedia_type.h"
#include "xmedia_region.h"

#ifdef __cplusplus
extern "C" {
#endif

xmedia_s32 sample_comm_region_check_min(xmedia_s32 min_handle);
xmedia_s32 sample_comm_region_get_min_handle(xmedia_rgn_type type);
xmedia_s32 sample_comm_region_get_up_canvas(xmedia_u32 handle, const xmedia_char *bmp_path);
xmedia_s32 sample_comm_region_init(xmedia_void);
xmedia_void sample_comm_region_exit(xmedia_void);
xmedia_s32 sample_comm_region_create(xmedia_s32 handle_num, xmedia_rgn_type type);
xmedia_s32 sample_comm_region_attach_to_chn(xmedia_s32 handle_num, xmedia_rgn_type type, xmedia_chn_info *mpp_chn);
xmedia_s32 sample_comm_region_detach_frm_chn(xmedia_s32 handle_num, xmedia_rgn_type type, xmedia_chn_info *mpp_chn);
xmedia_s32 sample_comm_region_get_min_handle(xmedia_rgn_type type);
xmedia_s32 sample_comm_region_destroy(xmedia_s32 handle_num, xmedia_rgn_type type);

#ifdef __cplusplus
}
#endif
#endif /*__SAMPLE_COMM_REGION_H__*/
