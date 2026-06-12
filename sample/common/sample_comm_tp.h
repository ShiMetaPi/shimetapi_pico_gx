#ifndef __TP9963_MIPI_H__
#define __TP9963_MIPI_H__

#include "xmedia_type.h"
#include "sample_comm_tp_i2c.h"

#define TP_MAX_CHN 2

typedef enum {
    TP_CH_1 = 0,
    TP_CH_2 = 1,
    TP_CH_ALL = 4,
    TP_CH_MIPI_PAGE = 8,
    TP_CH_MAX,
} tp9963_chn;

typedef enum {
    TP_FMT_PAL = 0,    // 720x288i
    TP_FMT_NTSC,       // 720x240i
    TP_FMT_HD25,       // 1280x720
    TP_FMT_HD30,       // 1280x720
    TP_FMT_FHD25,      // 1920x1080
    TP_FMT_FHD30,      // 1920x1080
    TP_FMT_QHD25,      // 2560x1440
    TP_FMT_QHD30,      // 2560x1440
    TP_FMT_QHD275,     // 2560X1440 27.5fps
    TP_FMT_FHD50,      // 1920x1080
    TP_FMT_FHD60,      // 1920x1080
    TP_FMT_5M20,       //
    TP_FMT_FHD275,     // 1080P27.5
    TP_FMT_FHD28,      // 1920X1080, 2200X1205
    TP_FMT_F_UVGA30,   // FH 1280x960p30, must use with MIPI_2CH2LANE_432M
    TP_FMT_QHD25_4A10, // total
    TP_FMT_FHD59,      // 1920x1080,Nonsatandard total 2200x1144
    TP_FMT_UVGA30,     // 1280x960p30, must use with MIPI_2CH2LANE_445M
    TP_FMT_HD50,
    TP_FMT_HD60,
    TP_FMT_MAX,
} tp9963_fmt;

typedef enum {
    TP_STD_TVI = 0, // TVI
    TP_STD_HDA,     // AHD
    TP_STD_MAX,
} tp9963_std;

typedef enum {
    TP_MIPI_MODE_2CH2LANE_594M = 0, // up to 2x1080p25/30
    TP_MIPI_MODE_2CH2LANE_297M,     // up to 2x720p25/30
    TP_MIPI_MODE_2CH4LANE_594M,     // up to 2x1080p50/60
    TP_MIPI_MODE_2CH4LANE_297M,     // up to 2x1080p25/30
    TP_MIPI_MODE_2CH2LANE_432M,     // only for 2xF_UVGA30
    TP_MIPI_MODE_1CH2LANE_594M,     // up to 1xQHD30
    TP_MIPI_MODE_1CH2LANE_297M,     // up to 1x1080P30
    TP_MIPI_MODE_1CH4LANE_297M,     // up to 1xQHD30
    TP_MIPI_MODE_2CH4LANE_216M,     // only for 2xF_UVGA30
    TP_MIPI_MODE_2CH2LANE_445M,     // only for 2xUVGA30
    TP_MIPI_MODE_MAX,
} tp9963_mipi_mode;

typedef struct {
    xmedia_u32 i2c_dev;
    tp9963_chn chn[TP_MAX_CHN];
    tp9963_fmt fmt[TP_MAX_CHN];
    tp9963_std std[TP_MAX_CHN];
    tp9963_mipi_mode mipi_mode;
} sample_comm_tp9963_cfg;

xmedia_s32 sample_comm_tp9963_init(sample_comm_tp9963_cfg *cfg);
xmedia_s32 sample_comm_tp9963_deinit(xmedia_void);

#endif