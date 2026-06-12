#ifndef __XMEDIA_VI_H__
#define __XMEDIA_VI_H__

#include "xmedia_video_common.h"
#include "xmedia_intf_common.h"
#include "xmedia_type.h"

#define VI_MIPI_DATA_LANE_MAX_NUM     4

typedef enum {
    XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_1_LANE = 0,
    XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE,
    XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE,
    XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_MAX
} xmedia_vi_mipi_lane_divide_mode;

typedef struct {
    xmedia_bool enable;
    xmedia_u32 lane_cfg[VI_MIPI_DATA_LANE_MAX_NUM];
} xmedia_vi_mipi_lane_config;

typedef struct {
    xmedia_vi_mipi_lane_divide_mode mipi_lane_num;
    xmedia_intf_mipi_csi_data_type data_type;
    xmedia_u32 mipi_rate; // 单位：Mbps
    xmedia_vi_mipi_lane_config lane_config;
} xmedia_vi_dev_config;

typedef struct {
    xmedia_video_data_width bit_width;
    xmedia_u32 width;
    xmedia_u32 height;
    xmedia_u32 vi_work_mode; // 0:在线 1:离线
    xmedia_video_wdr_mode wdr_mode;
} xmedia_vi_pipe_config;

#endif