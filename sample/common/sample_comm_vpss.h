/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __SAMPLE_COMM_VPSS_H__
#define __SAMPLE_COMM_VPSS_H__

#include "xmedia_type.h"
#include "xmedia_sys.h"
#include "xmedia_vb.h"
#include "xmedia_vpss.h"
#include "xmedia_region.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    xmedia_bool ext_chn_en;
    xmedia_vpss_ext_ochn_config ext_chn;
}vpss_ext_chn_info;

typedef struct {
    xmedia_bool chn_en;
    xmedia_s32 chn_no;
    xmedia_vpss_ochn_config chn_config;
    xmedia_video_crop_attr crop_attr;
    xmedia_video_rotation rot_attr;
    xmedia_vpss_low_delay_attr lowdelay_attr;
    xmedia_video_wrap_attr wrap_attr;
}vpss_chn_info;

typedef struct {
    xmedia_bool stitch_en;
    xmedia_vpss_stitch_blend_attr stitch_blend_attr;
}vpss_pipe_stitch_info;

typedef struct {
    xmedia_bool pipe_en;
    xmedia_s32 pipe_no;
    xmedia_vpss_pipe_config pipe_config;
    vpss_chn_info chn_info[VPSS_MAX_PHY_OCHN_NUM];
    xmedia_video_crop_attr crop_attr;
    xmedia_isp_stnr_attr stnr_attr;
    xmedia_isp_sharpen_attr sharpen_attr;
    vpss_pipe_stitch_info stitch_attr;
    vpss_ext_chn_info ext_chn_info[VPSS_MAX_EXT_OCHN_NUM];
} vpss_pipe_info;

typedef struct {
    vpss_pipe_info pipe_info[VPSS_MAX_PIPE_NUM];
} sample_vpss_config;

xmedia_s32 sample_comm_vpss_create_pipe(xmedia_s32 pipe_id, vpss_pipe_info* pipe_info);

xmedia_s32 sample_comm_vpss_destory_pipe(xmedia_s32 pipe_id);

xmedia_s32 sample_comm_vpss_set_chn_attr(xmedia_s32 pipe_id, xmedia_s32 chn_id, vpss_chn_info *chn_info);

xmedia_s32 sample_comm_vpss_get_default_pipe_cfg(xmedia_vpss_pipe_config *pipe_cfg, xmedia_video_size in_size, sample_comm_video_param *video_param);

xmedia_s32 sample_comm_vpss_get_default_ochn_cfg(xmedia_vpss_ochn_config *ochn_cfg, xmedia_video_size out_size, sample_comm_video_param *video_param);

xmedia_s32 sample_comm_vpss_set_ochn_wrap(vpss_chn_info *ochn_info);

xmedia_s32 sample_comm_vpss_pipe_chn_start(sample_vpss_config *vpss_cfg);

xmedia_s32 sample_comm_vpss_pipe_chn_stop(sample_vpss_config *vpss_cfg);

xmedia_s32 sample_comm_vpss_start(sample_vpss_config *vpss_cfg);

xmedia_s32 sample_comm_vpss_stop(sample_vpss_config *vpss_cfg);

xmedia_s32 sample_comm_vpss_init();

xmedia_s32 sample_comm_vpss_exit();

#ifdef __cplusplus
}
#endif

#endif
