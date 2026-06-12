#ifndef __MEDIA_PARAM_H__
#define __MEDIA_PARAM_H__

#include "media_comm.h"


/****************************************************
***************support sensor list*******************
SMART_SC485SL_MIPI_4M_30FPS_2LANE_12BIT
SMART_SC485SL_MIPI_4M_30FPS_4LANE_12BIT
SMART_SC485SL_MIPI_4M_60FPS_4LANE_12BIT
SMART_SC235HAI_MIPI_2M_2LANE_30FPS_10BIT
*****************************************************/

#ifdef SMART_SC485SL_MIPI_4M_30FPS_2LANE_12BIT

static xmedia_isp_config MEDIA_COMM_ISP_CFG_SENSOR0 = {
    .fps = 30,
    .size = {
        .width = 2560,
        .height = 1440,
    },
    .bayer_fmt = XMEDIA_VIDEO_BAYER_FMT_BGGR,
    .pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_RAW,
    .wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE,
    .mode_config = {
        .work_mode = XMEDIA_ISP_WORK_MODE_MASTER,
        .master_mode = {
            .blend_stat_enable = XMEDIA_FALSE,
            .slave_num = 0,
        },
    },
};

static xmedia_vi_dev_config MEDIA_COMM_VI_DEV_CFG_SENSOR0 = {
    .intf_type = XMEDIA_INTF_TYPE_MIPI_CSI,
    .mipi_attr = {
        .input_data_type = XMEDIA_INTF_MIPI_CSI_DATA_TYPE_RAW_12BIT,
        .lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE,
        .wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE,
    },
};

static xmedia_vi_pipe_config MEDIA_COMM_VI_PIPE_CFG_SENSOR0 = {
    .bit_width = XMEDIA_VIDEO_DATA_WIDTH_12,
    .compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE,
    .frm_rate_ctrl = {
        .src_frm_rate = -1,
        .dst_frm_rate = -1,
    },
    .isp_bypass = XMEDIA_FALSE,
    .pipe_bypass_mode = XMEDIA_VI_PIPE_BYPASS_NONE,
    .pix_fmt = XMEDIA_VIDEO_PIXEL_FMT_RAW,
    .gdc_en = XMEDIA_FALSE, //default false
    .in_size = {
        .width = 2560,
        .height = 1440,
    },
    .bnr_attr = {
        .bnr_en = XMEDIA_TRUE,
        .compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_LINE,
    },
    .stnr_attr = {
        .stnr_en = XMEDIA_TRUE,
        .compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_LINE,
    },
};

static xmedia_vi_chn_config MEDIA_COMM_VI_CHN_CFG_SENSOR0 = {
    .compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE,
    .dynamic_range = XMEDIA_VIDEO_DYNAMIC_RANGE_SDR,
    .flip_en = XMEDIA_FALSE,
    .mirror_en = XMEDIA_FALSE,
    .pix_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420,
    .video_fmt = XMEDIA_VIDEO_FMT_LINEAR,
    .depth = 0,
    .frm_rate_ctrl = {
        .src_frm_rate = -1,
        .dst_frm_rate = -1,
    },
    .out_size = {
        .width = 2560,
        .height = 1440,
    },
};

#endif

#ifdef SMART_SC485SL_MIPI_4M_30FPS_4LANE_12BIT

static const xmedia_s32 sc485sl_width  = 2688;
static const xmedia_s32 sc485sl_height = 1520;

static xmedia_isp_config MEDIA_COMM_ISP_CFG_SENSOR0 = {
    .fps = 12,
    .size = {
        .width = sc485sl_width,
        .height = sc485sl_height,
    },
    .bayer_fmt = XMEDIA_VIDEO_BAYER_FMT_BGGR,
    .pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_RAW,
    .wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE,
    .mode_config = {
        .work_mode = XMEDIA_ISP_WORK_MODE_MASTER,
        .master_mode = {
            .blend_stat_enable = XMEDIA_FALSE,
            .slave_num = 0,
        },
    },
};

static xmedia_vi_dev_config MEDIA_COMM_VI_DEV_CFG_SENSOR0 = {
    .intf_type = XMEDIA_INTF_TYPE_MIPI_CSI,
    .mipi_attr = {
        .input_data_type = XMEDIA_INTF_MIPI_CSI_DATA_TYPE_RAW_12BIT,
        .lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE,
        .wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE,
    },
};

static xmedia_vi_pipe_config MEDIA_COMM_VI_PIPE_CFG_SENSOR0 = {
    .bit_width = XMEDIA_VIDEO_DATA_WIDTH_12,
    .compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE,
    .frm_rate_ctrl = {
        .src_frm_rate = -1,
        .dst_frm_rate = -1,
    },
    .isp_bypass = XMEDIA_FALSE,
    .pipe_bypass_mode = XMEDIA_VI_PIPE_BYPASS_NONE,
    .pix_fmt = XMEDIA_VIDEO_PIXEL_FMT_RAW,
    .gdc_en = XMEDIA_FALSE, //default false
    .in_size = {
        .width = sc485sl_width,
        .height = sc485sl_height,
    },
    .bnr_attr = {
        .bnr_en = XMEDIA_TRUE,
        .compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_LINE,
    },
    .stnr_attr = {
        .stnr_en = XMEDIA_TRUE,
        .compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_LINE,
    },
};

static xmedia_vi_chn_config MEDIA_COMM_VI_CHN_CFG_SENSOR0 = {
    .compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE,
    .dynamic_range = XMEDIA_VIDEO_DYNAMIC_RANGE_SDR,
    .flip_en = XMEDIA_FALSE,
    .mirror_en = XMEDIA_FALSE,
    .pix_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420,
    .video_fmt = XMEDIA_VIDEO_FMT_LINEAR,
    .depth = 0,
    .frm_rate_ctrl = {
        .src_frm_rate = -1,
        .dst_frm_rate = -1,
    },
    .out_size = {
        .width = sc485sl_width,
        .height = sc485sl_height,
    },
};

#endif

#ifdef SMART_SC485SL_MIPI_4M_60FPS_4LANE_12BIT

static const xmedia_s32 sc485sl_width  = 2560;
static const xmedia_s32 sc485sl_height = 1440;

static xmedia_isp_config MEDIA_COMM_ISP_CFG_SENSOR0 = {
    .fps = 30,
    .size = {
        .width = sc485sl_width,
        .height = sc485sl_height,
    },
    .bayer_fmt = XMEDIA_VIDEO_BAYER_FMT_BGGR,
    .pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_RAW,
    .wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE,
    .mode_config = {
        .work_mode = XMEDIA_ISP_WORK_MODE_MASTER,
        .master_mode = {
            .blend_stat_enable = XMEDIA_FALSE,
            .slave_num = 0,
        },
    },
};

static xmedia_vi_dev_config MEDIA_COMM_VI_DEV_CFG_SENSOR0 = {
    .intf_type = XMEDIA_INTF_TYPE_MIPI_CSI,
    .mipi_attr = {
        .input_data_type = XMEDIA_INTF_MIPI_CSI_DATA_TYPE_RAW_12BIT,
        .lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE,
        .wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE,
    },
};

static xmedia_vi_pipe_config MEDIA_COMM_VI_PIPE_CFG_SENSOR0 = {
    .bit_width = XMEDIA_VIDEO_DATA_WIDTH_12,
    .compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE,
    .frm_rate_ctrl = {
        .src_frm_rate = -1,
        .dst_frm_rate = -1,
    },
    .isp_bypass = XMEDIA_FALSE,
    .pipe_bypass_mode = XMEDIA_VI_PIPE_BYPASS_NONE,
    .pix_fmt = XMEDIA_VIDEO_PIXEL_FMT_RAW,
    .gdc_en = XMEDIA_FALSE, //default false
    .in_size = {
        .width = sc485sl_width,
        .height = sc485sl_height,
    },
    .bnr_attr = {
        .bnr_en = XMEDIA_TRUE,
        .compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_LINE,
    },
    .stnr_attr = {
        .stnr_en = XMEDIA_TRUE,
        .compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_LINE,
    },
};

static xmedia_vi_chn_config MEDIA_COMM_VI_CHN_CFG_SENSOR0 = {
    .compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE,
    .dynamic_range = XMEDIA_VIDEO_DYNAMIC_RANGE_SDR,
    .flip_en = XMEDIA_FALSE,
    .mirror_en = XMEDIA_FALSE,
    .pix_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420,
    .video_fmt = XMEDIA_VIDEO_FMT_LINEAR,
    .depth = 0,
    .frm_rate_ctrl = {
        .src_frm_rate = -1,
        .dst_frm_rate = -1,
    },
    .out_size = {
        .width = sc485sl_width,
        .height = sc485sl_height,
    },
};
#endif

#ifdef SMART_SC235HAI_MIPI_2M_2LANE_30FPS_10BIT
static xmedia_isp_config MEDIA_COMM_ISP_CFG_SENSOR0 = {
    .fps = 30,
    .size = {
        .width = 1920,
        .height = 1080,
    },
    .bayer_fmt = XMEDIA_VIDEO_BAYER_FMT_BGGR,
    .pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_RAW,
    .wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE,
    .mode_config = {
        .work_mode = XMEDIA_ISP_WORK_MODE_MASTER,
        .master_mode = {
            .blend_stat_enable = XMEDIA_FALSE,
            .slave_num = 0,
        },
    },
};

static xmedia_vi_dev_config MEDIA_COMM_VI_DEV_CFG_SENSOR0 = {
    .intf_type = XMEDIA_INTF_TYPE_MIPI_CSI,
    .mipi_attr = {
        .input_data_type = XMEDIA_INTF_MIPI_CSI_DATA_TYPE_RAW_10BIT,
        .lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE,
        .wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE,
    },
};

static xmedia_vi_pipe_config MEDIA_COMM_VI_PIPE_CFG_SENSOR0 = {
    .bit_width = XMEDIA_VIDEO_DATA_WIDTH_10,
    .compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE,
    .frm_rate_ctrl = {
        .src_frm_rate = -1,
        .dst_frm_rate = -1,
    },
    .isp_bypass = XMEDIA_FALSE,
    .pipe_bypass_mode = XMEDIA_VI_PIPE_BYPASS_NONE,
    .pix_fmt = XMEDIA_VIDEO_PIXEL_FMT_RAW,
    .gdc_en = XMEDIA_FALSE, //default false
    .in_size = {
        .width = 1920,
        .height = 1080,
    },
    .bnr_attr = {
        .bnr_en = XMEDIA_TRUE,
        .compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_LINE,
    },
    .stnr_attr = {
        .stnr_en = XMEDIA_TRUE,
        .compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_LINE,
    },
};

static xmedia_vi_chn_config MEDIA_COMM_VI_CHN_CFG_SENSOR0 = {
    .compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE,
    .dynamic_range = XMEDIA_VIDEO_DYNAMIC_RANGE_SDR,
    .flip_en = XMEDIA_FALSE,
    .mirror_en = XMEDIA_FALSE,
    .pix_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420,
    .video_fmt = XMEDIA_VIDEO_FMT_LINEAR,
    .depth = 0,
    .frm_rate_ctrl = {
        .src_frm_rate = -1,
        .dst_frm_rate = -1,
    },
    .out_size = {
        .width = 1920,
        .height = 1080,
    },
};

#endif

#ifdef OMNIVISION_OS04D10_MIPI_4M_30FPS_10BIT

static xmedia_isp_config MEDIA_COMM_ISP_CFG_SENSOR0 = {
    .fps = 25,
    .size = {
        .width = 2560,
        .height = 1440,
    },
    .bayer_fmt = XMEDIA_VIDEO_BAYER_FMT_BGGR,
    .pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_RAW,
    .wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE,
    .mode_config = {
        .work_mode = XMEDIA_ISP_WORK_MODE_MASTER,
        .master_mode = {
            .blend_stat_enable = XMEDIA_FALSE,
            .slave_num = 0,
        },
    },
};

static xmedia_vi_dev_config MEDIA_COMM_VI_DEV_CFG_SENSOR0 = {
    .intf_type = XMEDIA_INTF_TYPE_MIPI_CSI,
    .mipi_attr = {
        .input_data_type = XMEDIA_INTF_MIPI_CSI_DATA_TYPE_RAW_10BIT,
        .lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE,
        .wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE,
    },
};

static xmedia_vi_pipe_config MEDIA_COMM_VI_PIPE_CFG_SENSOR0 = {
    .bit_width = XMEDIA_VIDEO_DATA_WIDTH_10,
    .compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE,
    .frm_rate_ctrl = {
        .src_frm_rate = -1,
        .dst_frm_rate = -1,
    },
    .isp_bypass = XMEDIA_FALSE,
    .pipe_bypass_mode = XMEDIA_VI_PIPE_BYPASS_NONE,
    .pix_fmt = XMEDIA_VIDEO_PIXEL_FMT_RAW,
    .gdc_en = XMEDIA_FALSE, //default false
    .in_size = {
        .width = 2560,
        .height = 1440,
    },
    .bnr_attr = {
        .bnr_en = XMEDIA_TRUE,
        .compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_LINE,
    },
    .stnr_attr = {
        .stnr_en = XMEDIA_TRUE,
        .compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_LINE,
    },
};

static xmedia_vi_chn_config MEDIA_COMM_VI_CHN_CFG_SENSOR0 = {
    .compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE,
    .dynamic_range = XMEDIA_VIDEO_DYNAMIC_RANGE_SDR,
    .flip_en = XMEDIA_FALSE,
    .mirror_en = XMEDIA_FALSE,
    .pix_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420,
    .video_fmt = XMEDIA_VIDEO_FMT_LINEAR,
    .depth = 0,
    .frm_rate_ctrl = {
        .src_frm_rate = -1,
        .dst_frm_rate = -1,
    },
    .out_size = {
        .width = 2560,
        .height = 1440,
    },
};

#endif

#endif
