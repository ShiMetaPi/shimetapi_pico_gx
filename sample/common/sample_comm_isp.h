/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __SAMPLE_COMM_ISP_H__
#define __SAMPLE_COMM_ISP_H__

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "xmedia_sys.h"
#include "xmedia_type.h"
#include "xmedia_isp.h"
#include "xmedia_ae.h"
#include "xmedia_awb.h"
#include "sample_comm_vi.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ISP_MAX_PIPE_NUM VI_MAX_PIPE_NUM

typedef struct {
    xmedia_bool isp_sensor_en;
    xmedia_bool isp_pipe_en;
    xmedia_bool i2c_addr_en;
    xmedia_u32 i2c_addr;
    sample_comm_sensor_type sensor_type;
    xmedia_u32 sns_clk_ch;
    xmedia_u32 sns_rst_ch;
    xmedia_sensor_trig_signal signal;
    xmedia_sensor_config sns_cfg;
    xmedia_sensor_work_mode sns_work_mode;
    xmedia_isp_config isp_config;
    xmedia_bool mirror;
    xmedia_bool flip;
    xmedia_bool serdes_enable;
    xmedia_bool disable_dynamic_fps;
} sample_isp_info;

typedef struct {
    xmedia_s32 pipe[ISP_MAX_PIPE_NUM]; /*vi master pipe number*/
    sample_isp_info isp_info[ISP_MAX_PIPE_NUM];
    xmedia_char *ainr_model_path[ISP_MAX_PIPE_NUM];
} sample_isp_param;

// for load ainr model and pqbin
#define AINR_LIST_NUM       (16)

typedef struct {
    sample_comm_sensor_type sensor_type;
    xmedia_char *load_file;
} sample_isp_sensor_effect_list;

extern sample_isp_sensor_effect_list g_ainr_pqbin_list[AINR_LIST_NUM];
extern sample_isp_sensor_effect_list g_ainr_model_list[AINR_LIST_NUM];


xmedia_s32 sample_comm_isp_init(sample_isp_param *isp_param, sample_vi_config *vi_config);

xmedia_s32 sample_comm_isp_exit(sample_isp_param *isp_param);

xmedia_s32 sample_comm_isp_start(sample_isp_param *isp_param);

xmedia_s32 sample_comm_isp_stop(sample_isp_param *isp_param);

xmedia_s32 sample_comm_isp_get_sensor_bayer_fmt(xmedia_u32 pipe, xmedia_video_size size, xmedia_video_wdr_mode wdr_mode,
                                                xmedia_bool mirror, xmedia_bool flip,
                                                xmedia_video_bayer_format *bayer_fmt);

xmedia_s32 sample_comm_isp_get_sensor_bit_rate(xmedia_u32 pipe, xmedia_u32 *bit_rate);

xmedia_void sample_comm_isp_set_multi_pipe_mode(xmedia_bool multi_pipe_mode,
    xmedia_s32 master_pipe_id, xmedia_s32 slave_pipe_id);

xmedia_bool sample_comm_isp_is_wdr_mode(sample_comm_sensor_type sensor_type);

xmedia_s32 sample_comm_isp_ainr_init(sample_isp_param *isp_param, sample_vi_config *vi_config);
xmedia_void sample_comm_isp_ainr_exit(sample_isp_param *isp_param);

xmedia_s32 sample_comm_isp_enable_fpn(xmedia_u32 isp_pipe, const xmedia_char *fpn_path);
xmedia_s32 sample_comm_isp_disable_fpn(xmedia_u32 isp_pipe);

xmedia_s32 sample_comm_isp_get_sensor_effect_index(sample_isp_sensor_effect_list list[], xmedia_s32 list_num,
                                                    sample_comm_sensor_type sensor_type);
xmedia_s32 sample_comm_isp_sensor_reset(xmedia_s32 sns_id);

#ifdef __cplusplus
}
#endif
#endif /*__SAMPLE_COMM_ISP_H__*/