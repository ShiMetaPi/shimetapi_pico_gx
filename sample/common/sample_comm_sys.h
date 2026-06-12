/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __SAMPLE_COMM_SYS_H__
#define __SAMPLE_COMM_SYS_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "xmedia_type.h"
#include "xmedia_sys.h"
#include "xmedia_vb.h"
#include "xmedia_vo.h"
#include "xmedia_mmz.h"
#include "xmedia_cl.h"

#include "sample_comm.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NPU_WORKSPACE_MAX_MODEL_NUM     4//The maximum scenario may include up to 4 models

typedef struct sample_sys_config {
    xmedia_sys_config sys_conf;
    xmedia_vb_config vb_conf;
    xmedia_sys_compress_level compress_level;//压缩级别
} sample_sys_config;

typedef struct sample_comm_npu_workspace_cfg {
    xmedia_cl_context    cl_ctx;
    xmedia_u32           cl_device_num;
    xmedia_cl_device_id *cl_device;
    xmedia_u64           work_space_phyaddr;
    xmedia_void         *work_space_viraddr;
} sample_comm_npu_workspace_cfg;

typedef struct sample_comm_npu_workspace_info {
    xmedia_s32  npu_modle_num;
    xmedia_char *npu_model_path[NPU_WORKSPACE_MAX_MODEL_NUM];
} sample_comm_npu_workspace_info;

xmedia_s32 sample_comm_sys_init(sample_sys_config *sys_conifg);
xmedia_s32 sample_comm_sys_cmp_init(sample_sys_config *sys_conifg);
xmedia_s32 sample_comm_vb_init(sample_sys_config *sys_conifg);
xmedia_s32 sample_comm_sys_exit(xmedia_void);
xmedia_s32 sample_comm_sys_vi_bind_vo(xmedia_s32 vi_pipe, xmedia_s32 vi_chn, xmedia_vo_layer vo_layer,
                                      xmedia_s32 vo_chn);
xmedia_s32 sample_comm_sys_vi_unbind_vo(xmedia_s32 vi_pipe, xmedia_s32 vi_chn, xmedia_vo_layer vo_layer,
                                        xmedia_s32 vo_chn);
xmedia_s32 sample_comm_sys_vi_bind_vpss(xmedia_s32 vi_pipe, xmedia_s32 vi_chn, xmedia_s32 vpss_pipe,
                                        xmedia_s32 vpss_ichn);
xmedia_s32 sample_comm_sys_vi_unbind_vpss(xmedia_s32 vi_pipe, xmedia_s32 vi_chn, xmedia_s32 vpss_pipe,
                                          xmedia_s32 vpss_ichn);
xmedia_s32 sample_comm_sys_vi_bind_mcf(xmedia_s32 vi_pipe, xmedia_s32 vi_chn, xmedia_s32 mcf_pipe,
                                        xmedia_s32 mcf_ichn);
xmedia_s32 sample_comm_sys_vi_unbind_mcf(xmedia_s32 vi_pipe, xmedia_s32 vi_chn, xmedia_s32 mcf_pipe,
                                          xmedia_s32 mcf_ichn);
xmedia_s32 sample_comm_sys_vi_bind_venc(xmedia_s32 vi_pipe, xmedia_s32 vi_chn, xmedia_s32 venc_chn);
xmedia_s32 sample_comm_sys_vi_unbind_venc(xmedia_s32 vi_pipe, xmedia_s32 vi_chn, xmedia_s32 venc_chn);
xmedia_s32 sample_comm_sys_mcf_bind_vpss(xmedia_s32 mcf_pipe, xmedia_s32 mcf_chn, xmedia_s32 vpss_pipe,
                                        xmedia_s32 vpss_ichn);
xmedia_s32 sample_comm_sys_mcf_unbind_vpss(xmedia_s32 mcf_pipe, xmedia_s32 mcf_chn, xmedia_s32 vpss_pipe,
                                          xmedia_s32 vpss_ichn);
xmedia_s32 sample_comm_sys_vpss_bind_vo(xmedia_s32 vpss_pipe, xmedia_s32 vpss_ochn, xmedia_vo_layer vo_layer,
                                        xmedia_s32 vo_chn);
xmedia_s32 sample_comm_sys_vpss_unbind_vo(xmedia_s32 vpss_pipe, xmedia_s32 vpss_ochn, xmedia_vo_layer vo_layer,
                                          xmedia_s32 vo_chn);
xmedia_s32 sample_comm_sys_vpss_bind_venc(xmedia_s32 vpss_pipe, xmedia_s32 vpss_ochn, xmedia_s32 venc_chn);
xmedia_s32 sample_comm_sys_vpss_unbind_venc(xmedia_s32 vpss_pipe, xmedia_s32 vpss_ochn, xmedia_s32 venc_chn);
xmedia_s32 sample_comm_sys_vdec_bind_vpss(xmedia_s32 vdec_chn, xmedia_s32 vpss_pipe, xmedia_s32 vpss_ichn);
xmedia_s32 sample_comm_sys_vdec_unbind_vpss(xmedia_s32 vdec_chn, xmedia_s32 vpss_pipe, xmedia_s32 vpss_ichn);
xmedia_s32 sample_comm_sys_vdec_bind_vo(xmedia_vo_dev vo_dev, xmedia_vo_chn vo_chn, xmedia_s32 vdec_chn);
xmedia_s32 sample_comm_sys_vdec_unbind_vo(xmedia_vo_dev vo_dev, xmedia_vo_chn vo_chn, xmedia_s32 vdec_chn);
xmedia_s32 sample_comm_sys_vo_bind_venc(xmedia_vo_dev vo_dev, xmedia_s32 venc_chn);
xmedia_s32 sample_comm_sys_vo_unbind_venc(xmedia_vo_dev vo_dev, xmedia_s32 venc_chn);
pic_size_e sample_comm_sys_get_picszie_by_wxh(xmedia_u32 width, xmedia_u32 height);
xmedia_s32 sample_comm_sys_get_buffer_size(xmedia_video_size pic_size, xmedia_video_format video_fmt,
                                           xmedia_video_pixel_format pixel_fmt, xmedia_video_data_width data_width,
                                           xmedia_video_compress_mode compress_mode);
xmedia_s32 sample_comm_sys_get_wrap_buffer_size(xmedia_video_size pic_size, xmedia_video_format video_fmt,
                                                xmedia_video_pixel_format pixel_fmt, xmedia_video_data_width data_width,
                                                xmedia_video_compress_mode compress_mode, xmedia_u32 *wrap_buffer_line);
xmedia_s32 sample_comm_sys_getchar_timeout(xmedia_u32 timeout_ms);
xmedia_s32 sample_comm_sys_npu_acquire_workspace(sample_comm_npu_workspace_info *npu_info);
xmedia_s32 sample_comm_sys_npu_release_workspace();

xmedia_s32 sample_comm_sys_mem_open(xmedia_void);
xmedia_void sample_comm_sys_mem_close(xmedia_void);
xmedia_void* sample_comm_sys_mem_map(xmedia_u32 phy_addr, xmedia_u32 size);
xmedia_void sample_comm_sys_mem_unmap(xmedia_void* vir_addr, xmedia_u32 size);
xmedia_s32 sample_comm_sys_set_reg(xmedia_u32 addr, xmedia_u32 value);
xmedia_s32 sample_comm_sys_get_reg(xmedia_u32 addr, xmedia_u32 *value);

#ifdef __cplusplus
}
#endif

#endif
