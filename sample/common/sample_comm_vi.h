/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __SAMPLE_COMM_VI_H__
#define __SAMPLE_COMM_VI_H__

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "xmedia_sys.h"
#include "xmedia_type.h"
#include "xmedia_vi.h"
#include "xmedia_video_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VI_DEV_BIND_PIPE_NUM       2
#define VI_ALIGN                   16
#define VI_MAX_STITCH_WDR_PIPE_NUM 4

typedef struct {
    xmedia_u32 width;
    xmedia_u32 height;
    xmedia_video_pixel_format pixel_format;
    xmedia_video_wdr_mode wdr_mode;
    xmedia_video_wdr_format wdr_format;
    xmedia_video_data_width bit_width;
    xmedia_intf_type intf_type;
    xmedia_vi_mipi_lane_divide_mode lane_mode;
} vi_sensor_info;

typedef struct {
    xmedia_s32 pipe[VI_DEV_BIND_PIPE_NUM];
} vi_dev_bind_pipe_info;

typedef struct {
    xmedia_bool grp_en;
    xmedia_s32 grp;
    xmedia_vi_grp_config grp_config;
} vi_grp_info;

typedef struct {
    xmedia_bool chn_en;
    xmedia_s32 chn_no;
    xmedia_vi_lowdelay_attr lowdelay_attr;
    xmedia_vi_chn_config chn_config;
    xmedia_vi_ext_chn_config ext_chn_config;
} vi_chn_info;

typedef struct {
    xmedia_bool dev_en;
    xmedia_bool dev_i2c_en;
    xmedia_u32 dev_i2c_addr; // 多路相同sensor，设置的i2c地址
    xmedia_s32 dev_no;
    xmedia_u32 i2c_id; // sensor使用的i2c编号
    xmedia_u32 sns_clk_id; // sensor使用的clk编号
    xmedia_vi_dev_config dev_config;
    xmedia_bool mipi_phy_en; // mipi phy手动速率配置使能
    xmedia_vi_dev_mipi_phy_config mipi_phy_config;
    sample_comm_sensor_type sensor_type;
} vi_dev_info;

typedef struct {
    xmedia_bool pipe_en;
    xmedia_bool vc_en;
    xmedia_s32 pipe_no;
    xmedia_u32 vc_num;
    xmedia_vi_frame_dump_attr dump_attr;
    xmedia_vi_pipe_config pipe_config;
    xmedia_vi_lowdelay_attr lowdelay_attr;
    vi_chn_info chn_info[VI_MAX_CHN_NUM];
    xmedia_bool ainr_en;
    xmedia_video_data_width ainr_model_bitwidth;
} vi_pipe_info;

typedef struct {
    xmedia_s32 pipe;
    xmedia_char *in_file_name[VI_MAX_WDR_NUM];
    vi_sensor_info sensor_info;
    xmedia_video_wdr_mode wdr_mode;
} vi_pipe_send_info;

typedef struct {
    xmedia_s32 dev[VI_MAX_STITCH_NUM];
    xmedia_s32 wdr_grp[VI_MAX_STITCH_NUM];
    xmedia_bool stitch_en;
    xmedia_s32 stitch_grp;
    xmedia_s32 vi_pipe[VI_MAX_STITCH_WDR_PIPE_NUM];
    xmedia_s32 vi_chn;
    xmedia_bool gdc_en;
    sample_comm_sensor_type sensor_type;
    xmedia_video_wdr_mode wdr_mode;
} sample_vi_stitch_param;

typedef struct sample_vi_config {
    vi_dev_info dev_info[VI_MAX_DEV_NUM];
    vi_pipe_info pipe_info[VI_MAX_PIPE_NUM];
    vi_dev_bind_pipe_info dev_bind_pipe[VI_MAX_DEV_NUM];
    vi_grp_info grp_info[VI_MAX_GRP_NUM];
} sample_vi_config;

typedef struct {
    xmedia_s32 vi_dev;
    xmedia_s32 vi_pipe[VI_MAX_WDR_NUM];
    xmedia_s32 vi_grp;
    xmedia_s32 vi_chn;
    xmedia_u32 cache_line;
    sample_comm_sensor_type sensor_type;
    xmedia_video_wdr_mode wdr_mode;
} sample_vi_wdr_param;

xmedia_s32 sample_comm_vi_get_sensor_info(sample_comm_sensor_type sensor_type, vi_sensor_info *sensor_info);

xmedia_s32 sample_comm_vi_get_default_dev_info(vi_sensor_info *sensor_info, xmedia_vi_dev_config *dev_cfg);

xmedia_void sample_comm_vi_get_default_chn_info(vi_sensor_info *sensor_info, xmedia_vi_chn_config *chn_config,
                                                sample_comm_video_param *video_param);

xmedia_void sample_comm_vi_get_default_pipe_info(vi_sensor_info *sensor_info, xmedia_vi_pipe_config *pipe_config,
                                                 xmedia_video_compress_mode compress_mode);

xmedia_s32 sample_comm_vi_get_default_pipe_chn_info(xmedia_s32 dev, vi_sensor_info *sensor_info,
                                                    sample_vi_config *vi_cfg, sample_comm_video_param *video_parame);

xmedia_s32 sample_comm_vi_get_default_cfg(vi_sensor_info *sensor_info, sample_vi_config *vi_cfg,
                                          sample_comm_video_param *video_param);

xmedia_s32 sample_comm_vi_get_framerate_by_sensor(sample_comm_sensor_type sensor_type, xmedia_u32 *framerate);

xmedia_s32 sample_comm_vi_get_def_dev_pipe_chn_by_sensor(sample_comm_sensor_type sensor_type,
                                                         xmedia_vi_dev_config *dev_cfg,
                                                         xmedia_vi_pipe_config *pipe_config,
                                                         xmedia_vi_chn_config *chn_config);

xmedia_s32 sample_comm_vi_start(sample_vi_config *vi_cfg, sample_comm_video_param *video_param);

xmedia_s32 sample_comm_vi_stop(sample_vi_config *vi_cfg);

xmedia_s32 sample_comm_vi_init(xmedia_void);

xmedia_s32 sample_comm_vi_exit(xmedia_void);

sample_comm_sensor_type sample_comm_vi_get_wdr_sensor_type(sample_comm_sensor_type sensor_type);

xmedia_void sample_comm_vi_sticth_config_linear_init(sample_vi_stitch_param *vi_stitch_param, sample_vi_config *vi_config);

xmedia_void sample_comm_vi_sticth_config_wdr_init(sample_vi_stitch_param *vi_stitch_param, sample_vi_config *vi_config);

xmedia_void sample_comm_vi_sticth_config_init(sample_vi_stitch_param *vi_stitch_param, sample_vi_config *vi_config);

xmedia_void sample_comm_vi_wdr_config_init(sample_vi_wdr_param *vi_wdr_param, sample_vi_config *vi_config);

#ifdef __cplusplus
}
#endif

#endif
