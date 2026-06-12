#ifndef _ISP_DEF_H_
#define _ISP_DEF_H_

#include "compile.h"

#define ISP_SUPPORT_PIPE_NUM 1

#if CONFIG_MEDIA_PIPE_NUM > ISP_SUPPORT_PIPE_NUM
#error "NOT supportted CONFIG_MEDIA_PIPE_NUM > 1.\n"
#endif

#define ISP_PIPE_MAX_NUM   CONFIG_MEDIA_PIPE_NUM

#define ISP_COST_TIME_DEBUG  0
#define ISP_COUNT_SIZE_DEBUG 0

typedef struct {
    xmedia_u32                             frame_cnt;
    xmedia_isp_awb_stat_global             awb_stat_global;
    // xmedia_isp_awb_stat_local_avg_r        awb_stat_local_avg_r;
    // xmedia_isp_awb_stat_local_avg_g        awb_stat_local_avg_g;
    // xmedia_isp_awb_stat_local_avg_b        awb_stat_local_avg_b;
    // xmedia_isp_awb_stat_local_valid_count  stat_local_valid_count;

    xmedia_u32 *awb_zone_avg_r;
    xmedia_u32 *awb_zone_avg_g;
    xmedia_u32 *awb_zone_avg_b;
    xmedia_u32 *awb_zone_valid_count;

    xmedia_u8  awb_gain_switch;
    xmedia_u8  awb_stat_switch;
    xmedia_u32 wdr_wb_gain[XMEDIA_ISP_BAYER_PATTERN_NUM];
    xmedia_u8  awb_zone_row_num;
    xmedia_u8  awb_zone_col_num;
    xmedia_u16 awb_width;
    xmedia_u16 awb_height;
} xmedia_isp_awb_stat;

typedef xmedia_isp_be_ae_stat_hist       xmedia_isp_ae_stat_hist;
typedef xmedia_isp_be_ae_stat_global_avg xmedia_isp_ae_stat_global_avg;
typedef xmedia_isp_be_ae_stat_zone_avg   xmedia_isp_ae_stat_zone_avg;

typedef struct {
    xmedia_u64 frame_cnt;
    // xmedia_u8  ae_zone_row_num;
    // xmedia_u8  ae_zone_col_num;

    xmedia_isp_ae_stat_hist        ae_stat_hist;
    xmedia_isp_ae_stat_global_avg  ae_stat_global_avg;
    // xmedia_isp_ae_stat_zone_avg    ae_stat_zone_avg;
} xmedia_isp_ae_stat;

typedef struct{
    xmedia_isp_ae_stat  ae_stat;
    xmedia_isp_awb_stat awb_stat;
}xmedia_isp_statistics;

typedef struct{
    xmedia_isp_ae_result  ae_result;
    xmedia_isp_awb_result awb_result;
}xmedia_isp_result;


#endif

