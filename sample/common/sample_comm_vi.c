/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <fcntl.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <string.h>
#include "sample_comm.h"
#include "sample_comm_vi.h"
#include "sample_comm_isp.h"
#include "xmedia_vb.h"
#include "xmedia_vpss.h"
#include "sample_comm_tp.h"

#define VI_DEV_NAME   "/dev/vi"
#define VI_INVALID_FD (-1)

xmedia_vi_stitch_attr g_vi_stitch_attr = {
    XMEDIA_VIDEO_STITCH_MODE_CYLINDER_PROJECTION,
    {
        {
            {-34848, -23168},
            {377631, 0, 346633, 0, 377399, 191845, 0, 0, 256},
            {2172, 1492},
        }
    }
};

xmedia_void vi_set_1M_sensor_info(sample_comm_sensor_type sensor_type, vi_sensor_info *sensor_info)
{
    sensor_info->width = 1280;
    sensor_info->height = 720;
    sensor_info->intf_type = XMEDIA_INTF_TYPE_MIPI_CSI;
    sensor_info->pixel_format = XMEDIA_VIDEO_PIXEL_FMT_RAW;
    sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE;

    if (sensor_type == OMNIVISION_OX03F10_MIPI_1M_10FPS_14BIT_BUILT_IN) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_BUILT_IN;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_BUILTIN;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_14;
    } else if (sensor_type == OMNIVISION_OX05B1S_MIPI_1M_30FPS_10BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    } else if (sensor_type == CVSENS_CV4003_MIPI_1M_120FPS_10BIT_PREROLL_READBACK) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == SONY_IMX179_MIPI_640_480_120FPS_10BIT) {
        sensor_info->width = 640;
        sensor_info->height = 480;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE;
    }
}

xmedia_void vi_set_2M_sensor_info(sample_comm_sensor_type sensor_type, vi_sensor_info *sensor_info)
{
    sensor_info->width = 1920;
    sensor_info->height = 1080;
    sensor_info->intf_type = XMEDIA_INTF_TYPE_MIPI_CSI;
    sensor_info->pixel_format = XMEDIA_VIDEO_PIXEL_FMT_RAW;
    sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE;

    if (sensor_type == SONY_IMX290_MIPI_2M_30FPS_10BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == SONY_IMX290_MIPI_2M_30FPS_16BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_16;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == SONY_IMX290_MIPI_2M_30FPS_10BIT_WDR2TO1) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_VC;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == SMART_SC8238_MIPI_2M_30FPS_10BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    } else if (sensor_type == SMART_SC8238_MIPI_2M_30FPS_10BIT_WDR2TO1) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_VC;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    } else if (sensor_type == GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT_WDR2TO1) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_FRAME;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_VC;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == GALAXYCORE_GC20C3_MIPI_2M_30FPS_10BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == GALAXYCORE_GC2083_MIPI_2M_30FPS_10BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == OMNIVISION_OS08A20_MIPI_2M_30FPS_10BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    } else if (sensor_type == OMNIVISION_OS08A20_MIPI_2M_30FPS_10BIT_WDR2TO1) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_VC;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    } else if (sensor_type == SMART_SC200AI_MIPI_2M_30FPS_10BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == SMART_SC200AI_MIPI_2M_30FPS_10BIT_WDR2TO1) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_VC;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == OMNIVISION_OS02K10_MIPI_2M_30FPS_10BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == OMNIVISION_OS02K10_MIPI_2M_30FPS_12BIT_BUILT_IN) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_BUILT_IN;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_BUILTIN;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_12;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE;
    } else if (sensor_type == SONY_IMX307_MIPI_2M_2L_30FPS_10BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == SONY_IMX307_MIPI_2M_2L_30FPS_10BIT_WDR2TO1) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_DOL;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == SMART_SC231HAI_MIPI_2M_2LANE_30FPS_10BIT) {
        sensor_info->wdr_mode   = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width  = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode  = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == SMART_SC235HAI_MIPI_2M_2LANE_30FPS_10BIT) {
        sensor_info->wdr_mode   = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width  = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode  = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == SMART_SC2337P_MIPI_2M_30FPS_10BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == IMAGEDESIGN_MIS20S1_MIPI_2M_2LANE_30FPS_10BIT) {
        sensor_info->wdr_mode   = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width  = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode  = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == CVSENS_CV2005_MIPI_2M_30FPS_10BIT) {
        sensor_info->wdr_mode   = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width  = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode  = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == VI_AHD_MIPI_2M_YUV) {
        sensor_info->wdr_mode   = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width  = XMEDIA_VIDEO_DATA_WIDTH_8;
        sensor_info->lane_mode  = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
        sensor_info->pixel_format = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_422;
    } else if (sensor_type == SMART_SC285SL_MIPI_2M_4LANE_30FPS_12BIT) {
        sensor_info->wdr_mode   = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width  = XMEDIA_VIDEO_DATA_WIDTH_12;
        sensor_info->lane_mode  = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE;
    } else if (sensor_type == SMART_SC285SL_MIPI_2M_2LANE_30FPS_12BIT) {
        sensor_info->wdr_mode   = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width  = XMEDIA_VIDEO_DATA_WIDTH_12;
        sensor_info->lane_mode  = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    }
}

xmedia_void vi_set_3M_sensor_info(sample_comm_sensor_type sensor_type, vi_sensor_info *sensor_info)
{
    sensor_info->width = 2048;
    sensor_info->height = 1536;
    sensor_info->intf_type = XMEDIA_INTF_TYPE_MIPI_CSI;
    sensor_info->pixel_format = XMEDIA_VIDEO_PIXEL_FMT_RAW;
    sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE;

    if (sensor_type == SMART_SC8238_MIPI_3M_30FPS_10BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    } else if (sensor_type == SMART_SC8238_MIPI_3M_30FPS_10BIT_WDR2TO1) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_VC;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    } else if (sensor_type == SMART_SC3338_MIPI_3M_30FPS_10BIT) {
        sensor_info->width = 2304;
        sensor_info->height = 1296;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == OMNIVISION_OX03F10_MIPI_3M_60FPS_14BIT_BUILT_IN) {
        sensor_info->width = 1920;
        sensor_info->height = 1536;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_BUILT_IN;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_BUILTIN;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_14;
    } else if (sensor_type == OMNIVISION_OX03F10_MIPI_3M_60FPS_10BIT) {
        sensor_info->width = 1920;
        sensor_info->height = 1536;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    }
}

xmedia_void vi_set_4M_sensor_info(sample_comm_sensor_type sensor_type, vi_sensor_info *sensor_info)
{
    sensor_info->width = 2688;
    sensor_info->height = 1520;
    sensor_info->intf_type = XMEDIA_INTF_TYPE_MIPI_CSI;
    sensor_info->pixel_format = XMEDIA_VIDEO_PIXEL_FMT_RAW;
    sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE;

    if (sensor_type == SMART_SC8238_MIPI_4M_30FPS_10BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    } else if (sensor_type == SMART_SC8238_MIPI_4M_30FPS_10BIT_WDR2TO1) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_VC;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    } else if (sensor_type == OMNIVISION_OS08A20_MIPI_4M_30FPS_10BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    } else if (sensor_type == OMNIVISION_OS08A20_MIPI_4M_30FPS_10BIT_WDR2TO1) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_VC;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    } else if (sensor_type == OMNIVISION_OS04A10_MIPI_4M_30FPS_12BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_12;
    } else if (sensor_type == OMNIVISION_OS04D10_MIPI_4M_30FPS_10BIT) {
        sensor_info->width  = 2560;
        sensor_info->height = 1440;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == OMNIVISION_OS04A10_MIPI_4M_30FPS_10BIT_WDR2TO1) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_VC;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    } else if (sensor_type == OMNIVISION_OS04C10_MIPI_4M_30FPS_10BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == OMNIVISION_OS04J10_MIPI_4M_30FPS_12BIT) {
        sensor_info->width = 2560;
        sensor_info->height = 1440;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_12;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE;
    } else if (sensor_type == OMNIVISION_OS05A10_MIPI_4M_30FPS_12BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_12;
        sensor_info->width = 2688;
        sensor_info->height = 1520;
    } else if (sensor_type == OMNIVISION_OS05A10_MIPI_4M_30FPS_10BIT_WDR2TO1) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_VC;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->width = 2688;
        sensor_info->height = 1520;
    } else if (sensor_type == SMART_SC850SL_MIPI_4M_30FPS_12BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_12;
    } else if (sensor_type == SMART_SC465SL_MIPI_4M_4LANE_30FPS_12BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_12;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE;
        sensor_info->width = 2560;
        sensor_info->height = 1440;
    } else if (sensor_type == SMART_SC465SL_MIPI_4M_2LANE_30FPS_12BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_12;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
        sensor_info->width = 2560;
        sensor_info->height = 1440;
    } else if (sensor_type == SMART_SC465SL_MIPI_4M_30FPS_4LANE_10BIT_WDR2TO1) {
        sensor_info->width = 2560;
        sensor_info->height = 1440;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_VC;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE;
    } else if (sensor_type == CVSENS_CV4003_MIPI_4M_30FPS_10BIT) {
        sensor_info->width = 2560;
        sensor_info->height = 1440;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == SMART_SC850SL_MIPI_4M_20FPS_10BIT_WDR2TO1) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_VC;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    } else if (sensor_type == SMART_SC450AI_MIPI_4M_30FPS_10BIT) {
        sensor_info->width = 2560;
        sensor_info->height = 1440;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == GALAXYCORE_GC4663_MIPI_4M_30FPS_10BIT) {
        sensor_info->width = 2560;
        sensor_info->height = 1440;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == GALAXYCORE_GC4653_MIPI_4M_30FPS_10BIT) {
        sensor_info->width = 2560;
        sensor_info->height = 1440;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == GALAXYCORE_GC4663_MIPI_4M_25FPS_10BIT_WDR2TO1) {
        sensor_info->width = 2560;
        sensor_info->height = 1440;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_VC;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == GALAXYCORE_OV4689_MIPI_4M_30FPS_10BIT) {
        sensor_info->width = 2688;
        sensor_info->height = 1520;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE;
    } else if (sensor_type == SMART_SC4336_MIPI_4M_30FPS_10BIT) {
        sensor_info->width = 2560;
        sensor_info->height = 1440;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == SMART_SC4336P_MIPI_4M_30FPS_10BIT) {
        sensor_info->width = 2560;
        sensor_info->height = 1440;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == SMART_SC485SL_MIPI_4M_2688_1520_30FPS_4LANE_12BIT) {
        sensor_info->width = 2688;
        sensor_info->height = 1520;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_12;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE;
    } else if (sensor_type == SMART_SC485SL_MIPI_4M_2560_1440_30FPS_4LANE_12BIT) {
        sensor_info->width = 2560;
        sensor_info->height = 1440;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_12;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE;
    } else if (sensor_type == SMART_SC485SL_MIPI_4M_30FPS_4LANE_10BIT_WDR2TO1) {
        sensor_info->width = 2560;
        sensor_info->height = 1440;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_VC;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE;
    } else if (sensor_type == SMART_SC485SL_MIPI_4M_30FPS_2LANE_12BIT) {
        sensor_info->width = 2560;
        sensor_info->height = 1440;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_12;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == IMAGEDESIGN_MIS40H1_MIPI_4M_2LANE_30FPS_10BIT) {
        sensor_info->width = 2688;
        sensor_info->height = 1520;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == SMART_SC431HAI_MIPI_4M_30FPS_10BIT) {
        sensor_info->width = 2560;
        sensor_info->height = 1440;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == SMART_SC435HAI_MIPI_4M_30FPS_10BIT) {
        sensor_info->width = 2560;
        sensor_info->height = 1440;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    }
}

xmedia_void vi_set_5M_sensor_info(sample_comm_sensor_type sensor_type, vi_sensor_info *sensor_info)
{
    sensor_info->width = 3072;
    sensor_info->height = 1728;

    sensor_info->intf_type = XMEDIA_INTF_TYPE_MIPI_CSI;
    sensor_info->pixel_format = XMEDIA_VIDEO_PIXEL_FMT_RAW;
    sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE;

    if (sensor_type == SMART_SC8238_MIPI_5M_30FPS_10BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->width = 2560;
        sensor_info->height = 1920;
    } else if (sensor_type == SMART_SC8238_MIPI_5M_30FPS_10BIT_WDR2TO1) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_VC;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->width = 2560;
        sensor_info->height = 1920;
    } else if (sensor_type == OMNIVISION_OS08A20_MIPI_5M_30FPS_10BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->width = 2560;
        sensor_info->height = 1920;
    } else if (sensor_type == OMNIVISION_OS08A20_MIPI_5M_30FPS_10BIT_WDR2TO1) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_VC;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->width = 2560;
        sensor_info->height = 1920;
    } else if (sensor_type == OMNIVISION_OS05A10_MIPI_5M_30FPS_12BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_12;
        sensor_info->width = 2688;
        sensor_info->height = 1944;
    } else if (sensor_type == OMNIVISION_OS05A10_MIPI_5M_30FPS_10BIT_WDR2TO1) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_VC;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->width = 2688;
        sensor_info->height = 1944;
    } else if (sensor_type == SMART_SC850SL_MIPI_5M_30FPS_12BIT) {
        sensor_info->width = 2560;
        sensor_info->height = 1920;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_12;
    } else if (sensor_type == SMART_SC850SL_MIPI_5M_20FPS_10BIT_WDR2TO1) {
        sensor_info->width = 2560;
        sensor_info->height = 1920;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_VC;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    } else if (sensor_type == SMART_SC530AI_MIPI_5M_4L_30FPS_10BIT) {
        sensor_info->width = 2880;
        sensor_info->height = 1620;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE;
    } else if (sensor_type == SMART_SC530AI_MIPI_5M_2L_30FPS_10BIT) {
        sensor_info->width = 2880;
        sensor_info->height = 1620;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == SMART_SC530AI_MIPI_5M_4L_30FPS_10BIT_WDR2TO1) {
        sensor_info->width = 2880;
        sensor_info->height = 1620;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_VC;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    } else if (sensor_type == SMART_SC500AI_MIPI_5M_30FPS_10BIT) {
        sensor_info->width = 2880;
        sensor_info->height = 1620;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE;
    } else if (sensor_type == SMART_SC500AI_MIPI_5M_30FPS_10BIT_WDR2TO1) {
        sensor_info->width = 2880;
        sensor_info->height = 1620;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_VC;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE;
    } else if (sensor_type == GALAXYCORE_GC5603_MIPI_5M_30FPS_10BIT) {
        sensor_info->width = 2960;
        sensor_info->height = 1620;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == OMNIVISION_OX05B1S_MIPI_5M_60FPS_10BIT) {
        sensor_info->width = 2592;
        sensor_info->height = 1944;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    } else if (sensor_type == SMART_SC533HAI_MIPI_5M_2L_30FPS_10BIT) {
        sensor_info->width = 2880;
        sensor_info->height = 1620;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == GALAXYCORE_GC6603_MIPI_5M_30FPS_10BIT) {
        sensor_info->width = 2688;
        sensor_info->height = 2048;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    }
}

xmedia_void vi_set_8M_sensor_info(sample_comm_sensor_type sensor_type, vi_sensor_info *sensor_info)
{
    sensor_info->width = 3840;
    sensor_info->height = 2160;
    sensor_info->intf_type = XMEDIA_INTF_TYPE_MIPI_CSI;
    sensor_info->pixel_format = XMEDIA_VIDEO_PIXEL_FMT_RAW;
    sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE;

    if (sensor_type == SMART_SC8238_MIPI_8M_20FPS_10BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    } else if (sensor_type == SMART_SC8238_MIPI_8M_20FPS_10BIT_WDR2TO1) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_VC;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    } else if (sensor_type == OMNIVISION_OS08A20_MIPI_8M_20FPS_10BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    } else if (sensor_type == OMNIVISION_OS08A20_MIPI_8M_20FPS_10BIT_WDR2TO1) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_VC;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    } else if (sensor_type == SMART_SC850SL_MIPI_8M_20FPS_12BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_12;
    } else if (sensor_type == SMART_SC850SL_MIPI_8M_20FPS_10BIT_WDR2TO1) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_VC;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    } else if (sensor_type == GALAXYCORE_GC8613_MIPI_8M_20FPS_10BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
        sensor_info->lane_mode = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    } else if (sensor_type == GALAXYCORE_GC8613_MIPI_8M_20FPS_14BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_14;
    } else if (sensor_type == SONY_IMX415_MIPI_8M_20FPS_10BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    } else if (sensor_type == SONY_IMX415_MIPI_8M_20FPS_10BIT_WDR2TO1) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_VC;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    }else if (sensor_type == SONY_IMX678_MIPI_8M_20FPS_12BIT) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_12;
    } else if (sensor_type == SONY_IMX678_MIPI_8M_20FPS_10BIT_WDR2TO1) {
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_2TO1_LINE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_VC;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    } else if (sensor_type == SONY_IMX179_MIPI_8M_20FPS_10BIT) {
        sensor_info->width = 3280;
        sensor_info->height = 2464;
        sensor_info->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
        sensor_info->wdr_format = XMEDIA_VIDEO_WDR_FMT_NONE;
        sensor_info->bit_width = XMEDIA_VIDEO_DATA_WIDTH_10;
    }
}

xmedia_s32 sample_comm_vi_get_framerate_by_sensor(sample_comm_sensor_type sensor_type, xmedia_u32 *framerate)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;

    if (!framerate) {
        return XMEDIA_FAILURE;
    }

    switch (sensor_type) {
        case SONY_IMX290_MIPI_2M_30FPS_10BIT:
        case SONY_IMX290_MIPI_2M_30FPS_16BIT:
        case SONY_IMX290_MIPI_2M_30FPS_10BIT_WDR2TO1:
        case SONY_IMX327_MIPI_2M_30FPS_12BIT:
        case SMART_SC8238_MIPI_2M_30FPS_10BIT:
        case SMART_SC8238_MIPI_2M_30FPS_10BIT_WDR2TO1:
        case SMART_SC8238_MIPI_3M_30FPS_10BIT:
        case SMART_SC8238_MIPI_3M_30FPS_10BIT_WDR2TO1:
        case SMART_SC8238_MIPI_4M_30FPS_10BIT:
        case SMART_SC8238_MIPI_4M_30FPS_10BIT_WDR2TO1:
        case SMART_SC8238_MIPI_5M_30FPS_10BIT:
        case SMART_SC8238_MIPI_5M_30FPS_10BIT_WDR2TO1:
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT:
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT_WDR2TO1:
        case GALAXYCORE_GC20C3_MIPI_2M_30FPS_10BIT:
        case GALAXYCORE_GC2083_MIPI_2M_30FPS_10BIT:
        case OMNIVISION_OS08A20_MIPI_2M_30FPS_10BIT:
        case OMNIVISION_OS08A20_MIPI_2M_30FPS_10BIT_WDR2TO1:
        case OMNIVISION_OS08A20_MIPI_4M_30FPS_10BIT:
        case OMNIVISION_OS08A20_MIPI_4M_30FPS_10BIT_WDR2TO1:
        case OMNIVISION_OS08A20_MIPI_5M_30FPS_10BIT:
        case OMNIVISION_OS08A20_MIPI_5M_30FPS_10BIT_WDR2TO1:
        case OMNIVISION_OS04A10_MIPI_4M_30FPS_12BIT:
        case OMNIVISION_OS04D10_MIPI_4M_30FPS_10BIT:
        case OMNIVISION_OS04A10_MIPI_4M_30FPS_10BIT_WDR2TO1:
        case OMNIVISION_OS04C10_MIPI_4M_30FPS_10BIT:
        case OMNIVISION_OS04J10_MIPI_4M_30FPS_12BIT:
        case OMNIVISION_OS05A10_MIPI_4M_30FPS_12BIT:
        case OMNIVISION_OS05A10_MIPI_4M_30FPS_10BIT_WDR2TO1:
        case OMNIVISION_OS05A10_MIPI_5M_30FPS_12BIT:
        case OMNIVISION_OS05A10_MIPI_5M_30FPS_10BIT_WDR2TO1:
        case SMART_SC850SL_MIPI_5M_30FPS_12BIT:
        case SMART_SC850SL_MIPI_4M_30FPS_12BIT:
        case SMART_SC485SL_MIPI_4M_2688_1520_30FPS_4LANE_12BIT:
        case SMART_SC485SL_MIPI_4M_2560_1440_30FPS_4LANE_12BIT:
        case SMART_SC485SL_MIPI_4M_30FPS_4LANE_10BIT_WDR2TO1:
        case SMART_SC485SL_MIPI_4M_30FPS_2LANE_12BIT:
        case IMAGEDESIGN_MIS40H1_MIPI_4M_2LANE_30FPS_10BIT:
        case SMART_SC530AI_MIPI_5M_4L_30FPS_10BIT:
        case SMART_SC530AI_MIPI_5M_2L_30FPS_10BIT:
        case SMART_SC530AI_MIPI_5M_4L_30FPS_10BIT_WDR2TO1:
        case SMART_SC533HAI_MIPI_5M_2L_30FPS_10BIT:
        case SMART_SC200AI_MIPI_2M_30FPS_10BIT:
        case SMART_SC200AI_MIPI_2M_30FPS_10BIT_WDR2TO1:
        case SMART_SC500AI_MIPI_5M_30FPS_10BIT:
        case SMART_SC500AI_MIPI_5M_30FPS_10BIT_WDR2TO1:
        case SMART_SC450AI_MIPI_4M_30FPS_10BIT:
        case SMART_SC3338_MIPI_3M_30FPS_10BIT:
        case GALAXYCORE_GC4663_MIPI_4M_30FPS_10BIT:
        case GALAXYCORE_GC4653_MIPI_4M_30FPS_10BIT:
        case GALAXYCORE_OV4689_MIPI_4M_30FPS_10BIT:
        case GALAXYCORE_GC6603_MIPI_5M_30FPS_10BIT:
        case OMNIVISION_OS02K10_MIPI_2M_30FPS_10BIT:
        case OMNIVISION_OS02K10_MIPI_2M_30FPS_12BIT_BUILT_IN:
        case SONY_IMX307_MIPI_2M_2L_30FPS_10BIT:
        case SONY_IMX307_MIPI_2M_2L_30FPS_10BIT_WDR2TO1:
        case GALAXYCORE_GC5603_MIPI_5M_30FPS_10BIT:
        case OMNIVISION_OX03F10_MIPI_1M_10FPS_14BIT_BUILT_IN:
        case SMART_SC4336_MIPI_4M_30FPS_10BIT:
        case SMART_SC4336P_MIPI_4M_30FPS_10BIT:
        case SMART_SC431HAI_MIPI_4M_30FPS_10BIT:
        case SMART_SC435HAI_MIPI_4M_30FPS_10BIT:
        case OMNIVISION_OX05B1S_MIPI_1M_30FPS_10BIT:
        case SMART_SC231HAI_MIPI_2M_2LANE_30FPS_10BIT:
        case IMAGEDESIGN_MIS20S1_MIPI_2M_2LANE_30FPS_10BIT:
        case SMART_SC235HAI_MIPI_2M_2LANE_30FPS_10BIT:
        case SMART_SC285SL_MIPI_2M_4LANE_30FPS_12BIT:
        case SMART_SC285SL_MIPI_2M_2LANE_30FPS_12BIT:
        case SMART_SC465SL_MIPI_4M_4LANE_30FPS_12BIT:
        case SMART_SC465SL_MIPI_4M_2LANE_30FPS_12BIT:
        case CVSENS_CV4003_MIPI_4M_30FPS_10BIT:
        case CVSENS_CV2005_MIPI_2M_30FPS_10BIT:
        case SMART_SC465SL_MIPI_4M_30FPS_4LANE_10BIT_WDR2TO1:
        case SMART_SC2337P_MIPI_2M_30FPS_10BIT:
            *framerate = 30;
            break;
        case SONY_IMX179_MIPI_640_480_120FPS_10BIT:
            *framerate = 120;
            break;
        case CVSENS_CV4003_MIPI_1M_120FPS_10BIT_PREROLL_READBACK:
            *framerate = 120;
            break;
        case SMART_SC850SL_MIPI_5M_20FPS_10BIT_WDR2TO1:
        case SMART_SC850SL_MIPI_4M_20FPS_10BIT_WDR2TO1:
            *framerate = 20;
            break;
        case SMART_SC8238_MIPI_8M_20FPS_10BIT:
        case SONY_IMX334_MIPI_8M_20FPS_12BIT:
        case SONY_IMX415_MIPI_8M_20FPS_10BIT:
        case SONY_IMX179_MIPI_8M_20FPS_10BIT:
        case SONY_IMX678_MIPI_8M_20FPS_12BIT:
        case GALAXYCORE_GC8613_MIPI_8M_20FPS_10BIT:
        case GALAXYCORE_GC8613_MIPI_8M_20FPS_14BIT:
        case SMART_SC850SL_MIPI_8M_20FPS_12BIT:
        case OMNIVISION_OS08A20_MIPI_8M_20FPS_10BIT:
            *framerate = 20;
            break;
        case SMART_SC8238_MIPI_8M_20FPS_10BIT_WDR2TO1:
        case SONY_IMX415_MIPI_8M_20FPS_10BIT_WDR2TO1:
        case SONY_IMX678_MIPI_8M_20FPS_10BIT_WDR2TO1:
        case SMART_SC850SL_MIPI_8M_20FPS_10BIT_WDR2TO1:
        case OMNIVISION_OS08A20_MIPI_8M_20FPS_10BIT_WDR2TO1:
            *framerate = 15;
            break;
        case OMNIVISION_OX03F10_MIPI_3M_60FPS_14BIT_BUILT_IN:
        case OMNIVISION_OX03F10_MIPI_3M_60FPS_10BIT:
        case OMNIVISION_OX05B1S_MIPI_5M_60FPS_10BIT:
            *framerate = 60;
            break;
        case GALAXYCORE_GC4663_MIPI_4M_25FPS_10BIT_WDR2TO1:
            *framerate = 25;
            break;
        default:
            *framerate = 30;
            break;
    }

    return ret;
}

xmedia_s32 sample_comm_vi_get_sensor_info(sample_comm_sensor_type sensor_type, vi_sensor_info *sensor_info)
{
    memset(sensor_info, 0, sizeof(vi_sensor_info));

    switch (sensor_type) {
        case OMNIVISION_OX03F10_MIPI_1M_10FPS_14BIT_BUILT_IN:
        case OMNIVISION_OX05B1S_MIPI_1M_30FPS_10BIT:
        case CVSENS_CV4003_MIPI_1M_120FPS_10BIT_PREROLL_READBACK:
        case SONY_IMX179_MIPI_640_480_120FPS_10BIT:
            vi_set_1M_sensor_info(sensor_type, sensor_info);
            break;
        case SONY_IMX290_MIPI_2M_30FPS_10BIT:
        case SONY_IMX290_MIPI_2M_30FPS_10BIT_WDR2TO1:
        case SMART_SC8238_MIPI_2M_30FPS_10BIT:
        case SMART_SC8238_MIPI_2M_30FPS_10BIT_WDR2TO1:
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT:
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT_WDR2TO1:
        case GALAXYCORE_GC20C3_MIPI_2M_30FPS_10BIT:
        case GALAXYCORE_GC2083_MIPI_2M_30FPS_10BIT:
        case OMNIVISION_OS08A20_MIPI_2M_30FPS_10BIT:
        case OMNIVISION_OS08A20_MIPI_2M_30FPS_10BIT_WDR2TO1:
        case SMART_SC200AI_MIPI_2M_30FPS_10BIT:
        case SMART_SC200AI_MIPI_2M_30FPS_10BIT_WDR2TO1:
        case OMNIVISION_OS02K10_MIPI_2M_30FPS_10BIT:
        case OMNIVISION_OS02K10_MIPI_2M_30FPS_12BIT_BUILT_IN:
        case SONY_IMX307_MIPI_2M_2L_30FPS_10BIT:
        case SONY_IMX307_MIPI_2M_2L_30FPS_10BIT_WDR2TO1:
        case SMART_SC231HAI_MIPI_2M_2LANE_30FPS_10BIT:
        case IMAGEDESIGN_MIS20S1_MIPI_2M_2LANE_30FPS_10BIT:
        case SMART_SC235HAI_MIPI_2M_2LANE_30FPS_10BIT:
        case SMART_SC2337P_MIPI_2M_30FPS_10BIT:
        case SMART_SC285SL_MIPI_2M_4LANE_30FPS_12BIT:
        case SMART_SC285SL_MIPI_2M_2LANE_30FPS_12BIT:
        case CVSENS_CV2005_MIPI_2M_30FPS_10BIT:
        case VI_AHD_MIPI_2M_YUV:
            vi_set_2M_sensor_info(sensor_type, sensor_info);
            break;

        case SMART_SC8238_MIPI_3M_30FPS_10BIT:
        case SMART_SC8238_MIPI_3M_30FPS_10BIT_WDR2TO1:
        case SMART_SC3338_MIPI_3M_30FPS_10BIT:
        case OMNIVISION_OX03F10_MIPI_3M_60FPS_14BIT_BUILT_IN:
        case OMNIVISION_OX03F10_MIPI_3M_60FPS_10BIT:
            vi_set_3M_sensor_info(sensor_type, sensor_info);
            break;

        case SMART_SC8238_MIPI_4M_30FPS_10BIT:
        case SMART_SC8238_MIPI_4M_30FPS_10BIT_WDR2TO1:
        case OMNIVISION_OS08A20_MIPI_4M_30FPS_10BIT:
        case OMNIVISION_OS08A20_MIPI_4M_30FPS_10BIT_WDR2TO1:
        case OMNIVISION_OS04A10_MIPI_4M_30FPS_12BIT:
        case OMNIVISION_OS04D10_MIPI_4M_30FPS_10BIT:
        case OMNIVISION_OS04A10_MIPI_4M_30FPS_10BIT_WDR2TO1:
        case OMNIVISION_OS04C10_MIPI_4M_30FPS_10BIT:
        case OMNIVISION_OS04J10_MIPI_4M_30FPS_12BIT:
        case OMNIVISION_OS05A10_MIPI_4M_30FPS_12BIT:
        case OMNIVISION_OS05A10_MIPI_4M_30FPS_10BIT_WDR2TO1:
        case SMART_SC850SL_MIPI_4M_30FPS_12BIT:
        case SMART_SC850SL_MIPI_4M_20FPS_10BIT_WDR2TO1:
        case SMART_SC485SL_MIPI_4M_2688_1520_30FPS_4LANE_12BIT:
        case SMART_SC485SL_MIPI_4M_2560_1440_30FPS_4LANE_12BIT:
        case SMART_SC485SL_MIPI_4M_30FPS_4LANE_10BIT_WDR2TO1:
        case SMART_SC485SL_MIPI_4M_30FPS_2LANE_12BIT:
        case IMAGEDESIGN_MIS40H1_MIPI_4M_2LANE_30FPS_10BIT:
        case SMART_SC450AI_MIPI_4M_30FPS_10BIT:
        case GALAXYCORE_GC4663_MIPI_4M_30FPS_10BIT:
        case GALAXYCORE_GC4653_MIPI_4M_30FPS_10BIT:
        case GALAXYCORE_OV4689_MIPI_4M_30FPS_10BIT:
        case GALAXYCORE_GC4663_MIPI_4M_25FPS_10BIT_WDR2TO1:
        case SMART_SC4336_MIPI_4M_30FPS_10BIT:
        case SMART_SC4336P_MIPI_4M_30FPS_10BIT:
        case SMART_SC431HAI_MIPI_4M_30FPS_10BIT:
        case SMART_SC435HAI_MIPI_4M_30FPS_10BIT:
        case SMART_SC465SL_MIPI_4M_4LANE_30FPS_12BIT:
        case SMART_SC465SL_MIPI_4M_2LANE_30FPS_12BIT:
        case SMART_SC465SL_MIPI_4M_30FPS_4LANE_10BIT_WDR2TO1:
        case CVSENS_CV4003_MIPI_4M_30FPS_10BIT:
            vi_set_4M_sensor_info(sensor_type, sensor_info);
            break;

        case SMART_SC8238_MIPI_5M_30FPS_10BIT:
        case SMART_SC8238_MIPI_5M_30FPS_10BIT_WDR2TO1:
        case OMNIVISION_OS08A20_MIPI_5M_30FPS_10BIT:
        case OMNIVISION_OS08A20_MIPI_5M_30FPS_10BIT_WDR2TO1:
        case OMNIVISION_OS05A10_MIPI_5M_30FPS_12BIT:
        case OMNIVISION_OS05A10_MIPI_5M_30FPS_10BIT_WDR2TO1:
        case SMART_SC850SL_MIPI_5M_30FPS_12BIT:
        case SMART_SC850SL_MIPI_5M_20FPS_10BIT_WDR2TO1:
        case SMART_SC530AI_MIPI_5M_4L_30FPS_10BIT:
        case SMART_SC530AI_MIPI_5M_2L_30FPS_10BIT:
        case SMART_SC533HAI_MIPI_5M_2L_30FPS_10BIT:
        case SMART_SC530AI_MIPI_5M_4L_30FPS_10BIT_WDR2TO1:
        case SMART_SC500AI_MIPI_5M_30FPS_10BIT:
        case SMART_SC500AI_MIPI_5M_30FPS_10BIT_WDR2TO1:
        case GALAXYCORE_GC5603_MIPI_5M_30FPS_10BIT:
        case GALAXYCORE_GC6603_MIPI_5M_30FPS_10BIT:
        case OMNIVISION_OX05B1S_MIPI_5M_60FPS_10BIT:
            vi_set_5M_sensor_info(sensor_type, sensor_info);
            break;

        case SMART_SC8238_MIPI_8M_20FPS_10BIT:
        case SMART_SC8238_MIPI_8M_20FPS_10BIT_WDR2TO1:
        case OMNIVISION_OS08A20_MIPI_8M_20FPS_10BIT:
        case OMNIVISION_OS08A20_MIPI_8M_20FPS_10BIT_WDR2TO1:
        case SMART_SC850SL_MIPI_8M_20FPS_12BIT:
        case SMART_SC850SL_MIPI_8M_20FPS_10BIT_WDR2TO1:
        case GALAXYCORE_GC8613_MIPI_8M_20FPS_10BIT:
        case GALAXYCORE_GC8613_MIPI_8M_20FPS_14BIT:
        case SONY_IMX179_MIPI_8M_20FPS_10BIT:
        case SONY_IMX415_MIPI_8M_20FPS_10BIT:
        case SONY_IMX415_MIPI_8M_20FPS_10BIT_WDR2TO1:
        case SONY_IMX678_MIPI_8M_20FPS_12BIT:
        case SONY_IMX678_MIPI_8M_20FPS_10BIT_WDR2TO1:
            vi_set_8M_sensor_info(sensor_type, sensor_info);
            break;

        default:
            SAMPLE_PRT("sensor %d type not supported\n", sensor_type);
            return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_vi_get_default_dev_info(vi_sensor_info *sensor_info, xmedia_vi_dev_config *dev_cfg)
{
    CHECK_NULL_PTR(sensor_info);
    CHECK_NULL_PTR(dev_cfg);

    dev_cfg->intf_type = sensor_info->intf_type;
    if (dev_cfg->intf_type == XMEDIA_INTF_TYPE_MIPI_CSI) {
        dev_cfg->mipi_attr.lane_mode = sensor_info->lane_mode;
        dev_cfg->mipi_attr.wdr_format = sensor_info->wdr_format;
        if (sensor_info->pixel_format == XMEDIA_VIDEO_PIXEL_FMT_RAW) {
            if (sensor_info->bit_width == XMEDIA_VIDEO_DATA_WIDTH_8) {
                dev_cfg->mipi_attr.input_data_type = XMEDIA_INTF_MIPI_CSI_DATA_TYPE_RAW_8BIT;
            } else if (sensor_info->bit_width == XMEDIA_VIDEO_DATA_WIDTH_10) {
                dev_cfg->mipi_attr.input_data_type = XMEDIA_INTF_MIPI_CSI_DATA_TYPE_RAW_10BIT;
            } else if (sensor_info->bit_width == XMEDIA_VIDEO_DATA_WIDTH_12) {
                dev_cfg->mipi_attr.input_data_type = XMEDIA_INTF_MIPI_CSI_DATA_TYPE_RAW_12BIT;
            } else if (sensor_info->bit_width == XMEDIA_VIDEO_DATA_WIDTH_14) {
                dev_cfg->mipi_attr.input_data_type = XMEDIA_INTF_MIPI_CSI_DATA_TYPE_RAW_14BIT;
            } else if (sensor_info->bit_width == XMEDIA_VIDEO_DATA_WIDTH_16) {
                dev_cfg->mipi_attr.input_data_type = XMEDIA_INTF_MIPI_CSI_DATA_TYPE_RAW_16BIT;
            } else {
                SAMPLE_PRT("dev raw invalid bit width %d\n", sensor_info->bit_width);
            }
        } else {
            dev_cfg->mipi_attr.input_data_type = XMEDIA_INTF_MIPI_CSI_DATA_TYPE_YUV422_8BIT;
        }
    } else if (dev_cfg->intf_type == XMEDIA_INTF_TYPE_DC) {
        dev_cfg->dc_attr.data_connect.enable = XMEDIA_FALSE;
        if (sensor_info->bit_width == XMEDIA_VIDEO_DATA_WIDTH_8) {
            dev_cfg->dc_attr.data_type = XMEDIA_INTF_DC_DATA_TYPE_RAW_8BIT;
        } else if (sensor_info->bit_width == XMEDIA_VIDEO_DATA_WIDTH_10) {
            dev_cfg->dc_attr.data_type = XMEDIA_INTF_DC_DATA_TYPE_RAW_10BIT;
        } else if (sensor_info->bit_width == XMEDIA_VIDEO_DATA_WIDTH_12) {
            dev_cfg->dc_attr.data_type = XMEDIA_INTF_DC_DATA_TYPE_RAW_12BIT;
        } else if (sensor_info->bit_width == XMEDIA_VIDEO_DATA_WIDTH_14) {
            dev_cfg->dc_attr.data_type = XMEDIA_INTF_DC_DATA_TYPE_RAW_14BIT;
        } else if (sensor_info->bit_width == XMEDIA_VIDEO_DATA_WIDTH_16) {
            dev_cfg->dc_attr.data_type = XMEDIA_INTF_DC_DATA_TYPE_RAW_16BIT;
        } else {
            SAMPLE_PRT("dev raw invalid bit width %d\n", sensor_info->bit_width);
        }
    } else if (dev_cfg->intf_type == XMEDIA_INTF_TYPE_BT601) {
        dev_cfg->bt601_attr.data_connect.enable = XMEDIA_FALSE;
        dev_cfg->bt601_attr.data_seq = XMEDIA_INTF_BT601_BT656_DATA_SEQ_YUYV;
        dev_cfg->bt601_attr.sync_cfg.hsync = XMEDIA_VI_SYNC_PULSE_POSITIVE;
        dev_cfg->bt601_attr.sync_cfg.vsync = XMEDIA_VI_SYNC_PULSE_POSITIVE;
        dev_cfg->bt601_attr.sync_cfg.timing_blank.hsync_hfb = 210;
        dev_cfg->bt601_attr.sync_cfg.timing_blank.hsync_act = sensor_info->width;
    } else if (dev_cfg->intf_type == XMEDIA_INTF_TYPE_BT656) {
        dev_cfg->bt656_attr.data_connect.enable = XMEDIA_FALSE;
        dev_cfg->bt656_attr.data_seq = XMEDIA_INTF_BT601_BT656_DATA_SEQ_YUYV;
        dev_cfg->bt656_attr.multiplex_mode = XMEDIA_VI_MULTIPLEX_MODE_1;
        dev_cfg->bt656_attr.clk_egde = XMEDIA_VI_CLK_EDGE_SINGLE;
    } else if (dev_cfg->intf_type == XMEDIA_INTF_TYPE_BT1120) {
        dev_cfg->bt1120_attr.data_connect.enable = XMEDIA_FALSE;
        dev_cfg->bt1120_attr.data_seq = XMEDIA_INTF_BT1120_DATA_SEQ_VUVU;
        dev_cfg->bt1120_attr.multiplex_mode = XMEDIA_VI_MULTIPLEX_MODE_1;
    } else {
        SAMPLE_PRT("dev intf type %d invalid\n", dev_cfg->intf_type);
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_vi_get_default_grp_info(vi_sensor_info *sensor_info, sample_vi_config *vi_cfg)
{
    xmedia_s32 grp;

    CHECK_NULL_PTR(sensor_info);
    CHECK_NULL_PTR(vi_cfg);

    for (grp = 0; grp < VI_MAX_GRP_NUM; grp++) {
        if (vi_cfg->grp_info[grp].grp_en == XMEDIA_FALSE) {
            continue;
        }

        if (vi_cfg->grp_info[grp].grp_config.grp_type == XMEDIA_VI_GRP_TYPE_STITCH) {
            memcpy(&vi_cfg->grp_info[grp].grp_config.stitch_grp_config.vi_stitch_attr, &g_vi_stitch_attr,
                sizeof(xmedia_vi_stitch_attr));
        }
    }

    return XMEDIA_SUCCESS;
}

xmedia_void sample_comm_vi_get_default_chn_info(vi_sensor_info *sensor_info, xmedia_vi_chn_config *chn_config,
                                                sample_comm_video_param *video_param)
{
    chn_config->compress_mode = video_param->compress_mode;
    chn_config->depth = 1;
    chn_config->dynamic_range = XMEDIA_VIDEO_DYNAMIC_RANGE_SDR;
    chn_config->flip_en = XMEDIA_FALSE;
    chn_config->mirror_en = XMEDIA_FALSE;
    chn_config->frm_rate_ctrl.src_frm_rate = -1;
    chn_config->frm_rate_ctrl.dst_frm_rate = -1;
    chn_config->out_size.width = sensor_info->width;
    chn_config->out_size.height = sensor_info->height;
    chn_config->pix_fmt = video_param->pixel_fmt;
    chn_config->video_fmt = video_param->video_fmt;

    return;
}

xmedia_void sample_comm_vi_get_default_pipe_info(vi_sensor_info *sensor_info, xmedia_vi_pipe_config *pipe_config,
                                                 xmedia_video_compress_mode compress_mode)
{
    pipe_config->bit_width = sensor_info->bit_width;
    pipe_config->frm_rate_ctrl.src_frm_rate = -1;
    pipe_config->frm_rate_ctrl.dst_frm_rate = -1;
    pipe_config->in_size.width = sensor_info->width;
    pipe_config->in_size.height = sensor_info->height;
    pipe_config->isp_bypass = XMEDIA_FALSE;
    pipe_config->pipe_bypass_mode = XMEDIA_VI_PIPE_BYPASS_NONE;
    pipe_config->pix_fmt = sensor_info->pixel_format;
    // 默认开3dnr参考帧，默认开B3D压缩
    if(sensor_info->width > 3200 || pipe_config->pix_fmt != XMEDIA_VIDEO_PIXEL_FMT_RAW) {
        pipe_config->bnr_attr.bnr_en = XMEDIA_FALSE;  // 分块场景不开bnr，内存占用过大
    } else {
        pipe_config->bnr_attr.bnr_en = XMEDIA_TRUE;
    }
    pipe_config->bnr_attr.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_LINE;
    pipe_config->stnr_attr.stnr_en = XMEDIA_TRUE;
    pipe_config->stnr_attr.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_LINE;
}

xmedia_s32 sample_comm_vi_get_default_pipe_chn_info(xmedia_s32 dev, vi_sensor_info *sensor_info,
                                                    sample_vi_config *vi_cfg, sample_comm_video_param *video_param)
{
    xmedia_s32 i, j;
    xmedia_s32 pipe;
    xmedia_vi_pipe_config *pipe_config = XMEDIA_NULL;
    xmedia_vi_chn_config *chn_config = XMEDIA_NULL;

    CHECK_NULL_PTR(sensor_info);
    CHECK_NULL_PTR(vi_cfg);
    CHECK_NULL_PTR(video_param);

    for (i = 0; i < VI_DEV_BIND_PIPE_NUM; i++) {
        pipe = vi_cfg->dev_bind_pipe[dev].pipe[i];
        if (pipe == -1) {
            continue;
        }

        if (vi_cfg->pipe_info[pipe].pipe_en == XMEDIA_FALSE) {
            continue;
        }

        pipe_config = &vi_cfg->pipe_info[pipe].pipe_config;
        sample_comm_vi_get_default_pipe_info(sensor_info, pipe_config, video_param->compress_mode);

        for (j = 0; j < VI_MAX_CHN_NUM; j++) {
            if (vi_cfg->pipe_info[pipe].chn_info[j].chn_en == XMEDIA_FALSE) {
                continue;
            }

            chn_config = &vi_cfg->pipe_info[pipe].chn_info[j].chn_config;
            sample_comm_vi_get_default_chn_info(sensor_info, chn_config, video_param);
            if (vi_cfg->pipe_info[pipe].pipe_config.gdc_en == XMEDIA_TRUE) {
                chn_config->video_fmt = XMEDIA_VIDEO_FMT_TILE_32x4;
            }
        }
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_vi_start_dev(sample_vi_config *vi_info)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 i, j;
    vi_grp_info *grp_info = XMEDIA_NULL;
    xmedia_vi_dev_mipi_phy_config mipi_phy_config = { 0 };

    for (i = 0; i < VI_MAX_DEV_NUM; i++) {
        if (vi_info->dev_info[i].dev_en == XMEDIA_FALSE) {
            continue;
        }

        ret = xmedia_vi_set_dev_config(vi_info->dev_info[i].dev_no, &vi_info->dev_info[i].dev_config);
        CHECK_RET(ret, "set vi dev config");

        if (vi_info->dev_info[i].mipi_phy_en == XMEDIA_TRUE) {
            if (vi_info->dev_info[i].sensor_type == VI_AHD_MIPI_2M_YUV) {
                mipi_phy_config.mipi_rate = vi_info->dev_info[i].mipi_phy_config.mipi_rate;
            } else {
                // 根据sensor接口获取速率
                ret = sample_comm_isp_get_sensor_bit_rate(vi_info->dev_bind_pipe[i].pipe[0], &mipi_phy_config.mipi_rate);
                if (ret != XMEDIA_SUCCESS) {
                    mipi_phy_config.mipi_rate = 400;
                }
            }

            SAMPLE_PRT("mipi_rate: %u.\n", mipi_phy_config.mipi_rate);

            ret = xmedia_vi_set_dev_mipi_phy_config(vi_info->dev_info[i].dev_no, &mipi_phy_config);
            CHECK_RET(ret, "set vi dev mipi phy config");
        }

        ret = xmedia_vi_enable_dev(vi_info->dev_info[i].dev_no);
        CHECK_RET(ret, "enable vi dev");

        for (j = 0; j < VI_DEV_BIND_PIPE_NUM; j++) {
            if (vi_info->dev_bind_pipe[i].pipe[j] == -1) {
                continue;
            }

            ret = xmedia_vi_set_dev_bind_pipe(vi_info->dev_info[i].dev_no, vi_info->dev_bind_pipe[i].pipe[j]);
            CHECK_RET(ret, "vi dev bind pipe");
        }
    }

    for (i = 0; i < VI_MAX_GRP_NUM; i++) {
        grp_info = &vi_info->grp_info[i];
        if (grp_info->grp_en == XMEDIA_FALSE) {
            continue;
        }

        ret = xmedia_vi_set_grp_config(grp_info->grp, &grp_info->grp_config);
        CHECK_RET(ret, "set vi grp config");
    }

    return ret;
}

xmedia_s32 sample_comm_vi_stop_dev(sample_vi_config *vi_cfg)
{
    xmedia_s32 ret;
    xmedia_s32 i, j;

    for (i = 0; i < VI_MAX_DEV_NUM; i++) {
        if (vi_cfg->dev_info[i].dev_en == XMEDIA_FALSE) {
            continue;
        }

        for (j = 0; j < VI_DEV_BIND_PIPE_NUM; j++) {
            if (vi_cfg->dev_bind_pipe[i].pipe[j] == -1) {
                continue;
            }
        }

        ret = xmedia_vi_disable_dev(vi_cfg->dev_info[i].dev_no);
        CHECK_RET(ret, "disable vi dev");
    }
    return ret;
}

xmedia_s32 sample_comm_vi_start_pipe_chn(sample_vi_config *vi_cfg)
{
    xmedia_s32 ret;
    xmedia_s32 i, j;

    for (i = 0; i < VI_MAX_PIPE_NUM; i++) {
        if (vi_cfg->pipe_info[i].pipe_en == XMEDIA_FALSE) {
            continue;
        }

        if ( vi_cfg->pipe_info[i].ainr_en == XMEDIA_TRUE) {
            // when ainr enable, bit width should be configed 12bit or 10bit
            vi_cfg->pipe_info[i].pipe_config.bit_width = vi_cfg->pipe_info[i].ainr_model_bitwidth;
        }

        ret = xmedia_vi_create_pipe(vi_cfg->pipe_info[i].pipe_no, &vi_cfg->pipe_info[i].pipe_config);
        CHECK_RET(ret, "create vi pipe");

        if (vi_cfg->pipe_info[i].vc_en == XMEDIA_TRUE) {
            ret =xmedia_vi_set_pipe_vc_number(vi_cfg->pipe_info[i].pipe_no, vi_cfg->pipe_info[i].vc_num);
            CHECK_RET(ret, "set vi pipe vc num");
        }

        if (vi_cfg->pipe_info[i].lowdelay_attr.enable == XMEDIA_TRUE) {
            ret = xmedia_vi_set_pipe_low_delay_attr(vi_cfg->pipe_info[i].pipe_no, &vi_cfg->pipe_info[i].lowdelay_attr);
            CHECK_RET(ret, "set vi pipe low delay");
        }

        ret = xmedia_vi_start_pipe(vi_cfg->pipe_info[i].pipe_no);
        CHECK_RET(ret, "start vi pipe");

        if (vi_cfg->pipe_info[i].dump_attr.enable == XMEDIA_TRUE) {
            xmedia_vi_set_pipe_frame_dump_attr(vi_cfg->pipe_info[i].pipe_no, &vi_cfg->pipe_info[i].dump_attr);
        }

        for (j = 0; j < VI_MAX_PHY_CHN_NUM; j++) {
            if (vi_cfg->pipe_info[i].chn_info[j].chn_en == XMEDIA_FALSE) {
                continue;
            }

            ret = xmedia_vi_set_chn_config(vi_cfg->pipe_info[i].pipe_no, vi_cfg->pipe_info[i].chn_info[j].chn_no,
                                           &vi_cfg->pipe_info[i].chn_info[j].chn_config);
            CHECK_RET(ret, "set vi chn config");

            if (vi_cfg->pipe_info[i].chn_info[j].lowdelay_attr.enable == XMEDIA_TRUE) {
                ret = xmedia_vi_set_chn_low_delay_attr(vi_cfg->pipe_info[i].pipe_no,
                                                       vi_cfg->pipe_info[i].chn_info[j].chn_no,
                                                       &vi_cfg->pipe_info[i].chn_info[j].lowdelay_attr);
                CHECK_RET(ret, "set vi chn low delay");
            }

            ret = xmedia_vi_enable_chn(vi_cfg->pipe_info[i].pipe_no, vi_cfg->pipe_info[i].chn_info[j].chn_no);
            CHECK_RET(ret, "enable vi chn");
        }

        //扩展通道，通道号从1开始
        for (j = 1; j < VI_MAX_CHN_NUM; j ++) {
            if (vi_cfg->pipe_info[i].chn_info[j].chn_en == XMEDIA_FALSE) {
                continue;
            }

            ret = xmedia_vi_set_ext_chn_config(vi_cfg->pipe_info[i].pipe_no, vi_cfg->pipe_info[i].chn_info[j].chn_no,
                                           &vi_cfg->pipe_info[i].chn_info[j].ext_chn_config);
            CHECK_RET(ret, "set vi ext chn config");

            ret = xmedia_vi_enable_chn(vi_cfg->pipe_info[i].pipe_no, vi_cfg->pipe_info[i].chn_info[j].chn_no);
            CHECK_RET(ret, "enable vi ext chn");
        }
    }

    return ret;
}

xmedia_s32 sample_comm_vi_stop_pipe_chn(sample_vi_config *vi_cfg)
{
    xmedia_s32 ret;
    xmedia_s32 i, j;

    for (i = 0; i < VI_MAX_PIPE_NUM; i++) {
        if (vi_cfg->pipe_info[i].pipe_en == XMEDIA_FALSE) {
            continue;
        }

        for (j = 0; j < VI_MAX_PHY_CHN_NUM; j++) {
            if (vi_cfg->pipe_info[i].chn_info[j].chn_en == XMEDIA_FALSE) {
                continue;
            }

            ret = xmedia_vi_disable_chn(vi_cfg->pipe_info[i].pipe_no, vi_cfg->pipe_info[i].chn_info[j].chn_no);
            CHECK_RET(ret, "disable vi chn");
        }

        for (j = 1; j < VI_MAX_CHN_NUM; j++) {
            if (vi_cfg->pipe_info[i].chn_info[j].chn_en == XMEDIA_FALSE) {
                continue;
            }

            ret = xmedia_vi_disable_chn(vi_cfg->pipe_info[i].pipe_no, vi_cfg->pipe_info[i].chn_info[j].chn_no);
            CHECK_RET(ret, "disable vi ext chn");
        }

        ret = xmedia_vi_stop_pipe(vi_cfg->pipe_info[i].pipe_no);
        CHECK_RET(ret, "stop vi pipe");

        ret = xmedia_vi_destroy_pipe(vi_cfg->pipe_info[i].pipe_no);
        CHECK_RET(ret, "destroy vi pipe");
    }

    return ret;
}

xmedia_s32 sample_comm_vi_start(sample_vi_config *vi_cfg, sample_comm_video_param *video_param)
{
    xmedia_s32 ret;
    xmedia_s32 i;
    vi_sensor_info sensor_info[VI_MAX_DEV_NUM] = { 0 };

    for (i = 0; i < VI_MAX_DEV_NUM; i++) {
        if (vi_cfg->dev_info[i].dev_en == XMEDIA_FALSE) {
            continue;
        }

        ret = sample_comm_vi_get_sensor_info(vi_cfg->dev_info[i].sensor_type, &sensor_info[i]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get sensor%d info failed!\n", i);
            return ret;
        }

        ret = sample_comm_vi_get_default_dev_info(&sensor_info[i], &vi_cfg->dev_info[i].dev_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get vi dev %d dev info failed!\n", i);
            return ret;
        }

        /* get mipi rate for isp */
        vi_cfg->dev_info[i].mipi_phy_en = XMEDIA_TRUE;

        ret = sample_comm_vi_get_default_grp_info(&sensor_info[i], vi_cfg);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get vi dev %d grp info failed!\n", i);
            return ret;
        }

        ret = sample_comm_vi_get_default_pipe_chn_info(vi_cfg->dev_info[i].dev_no, &sensor_info[i], vi_cfg, video_param);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get pipe chnl info failed!\n");
            return ret;
        }
    }

    ret = sample_comm_vi_start_dev(vi_cfg);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vi_start_dev fail!\n");
        goto exit0;
    }

    ret = sample_comm_vi_start_pipe_chn(vi_cfg);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vi_start_pipe_chn fail!\n");
        goto exit1;
    }

    return ret;

exit1:
    sample_comm_vi_stop_pipe_chn(vi_cfg);
exit0:
    sample_comm_vi_stop_dev(vi_cfg);

    return ret;
}

xmedia_s32 sample_comm_vi_stop(sample_vi_config *vi_cfg)
{
    xmedia_s32 ret;

    ret = sample_comm_vi_stop_pipe_chn(vi_cfg);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vi_stop_pipe_chn fail!\n");
        return ret;
    }

    ret = sample_comm_vi_stop_dev(vi_cfg);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vi_stop_dev fail!\n");
        return ret;
    }

    return ret;
}

xmedia_u32 sample_comm_vi_bit_width_convert(xmedia_video_pixel_format pix_fmt, xmedia_video_data_width data_width)
{
    xmedia_u32 bit_width;

    if (pix_fmt != XMEDIA_VIDEO_PIXEL_FMT_RAW) {
        bit_width = 8;
        return bit_width;
    }

    switch (data_width) {
        case XMEDIA_VIDEO_DATA_WIDTH_8:
            bit_width = 8;
            break;

        case XMEDIA_VIDEO_DATA_WIDTH_10:
            bit_width = 10;
            break;

        case XMEDIA_VIDEO_DATA_WIDTH_12:
            bit_width = 12;
            break;

        case XMEDIA_VIDEO_DATA_WIDTH_14:
            bit_width = 14;
            break;

        case XMEDIA_VIDEO_DATA_WIDTH_16:
            bit_width = 16;
            break;

        default:
            return 0;
    }

    return bit_width;
}

xmedia_s32 sample_comm_vi_init(xmedia_void)
{
    return xmedia_vi_init();
}

xmedia_s32 sample_comm_vi_exit(xmedia_void)
{
    return xmedia_vi_exit();
}

sample_comm_sensor_type sample_comm_vi_get_wdr_sensor_type(sample_comm_sensor_type sensor_type)
{
    sample_comm_sensor_type type = sensor_type;

    switch (sensor_type) {
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT:
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT_WDR2TO1:
            type = GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT_WDR2TO1;
            break;

        case SMART_SC8238_MIPI_2M_30FPS_10BIT:
        case SMART_SC8238_MIPI_2M_30FPS_10BIT_WDR2TO1:
            type = SMART_SC8238_MIPI_2M_30FPS_10BIT_WDR2TO1;
            break;

        case SMART_SC8238_MIPI_3M_30FPS_10BIT:
        case SMART_SC8238_MIPI_3M_30FPS_10BIT_WDR2TO1:
            type = SMART_SC8238_MIPI_3M_30FPS_10BIT_WDR2TO1;
            break;

        case SMART_SC8238_MIPI_4M_30FPS_10BIT:
        case SMART_SC8238_MIPI_4M_30FPS_10BIT_WDR2TO1:
            type = SMART_SC8238_MIPI_4M_30FPS_10BIT_WDR2TO1;
            break;

        case SMART_SC8238_MIPI_5M_30FPS_10BIT:
        case SMART_SC8238_MIPI_5M_30FPS_10BIT_WDR2TO1:
            type = SMART_SC8238_MIPI_5M_30FPS_10BIT_WDR2TO1;
            break;

        case SMART_SC8238_MIPI_8M_20FPS_10BIT:
        case SMART_SC8238_MIPI_8M_20FPS_10BIT_WDR2TO1:
            type = SMART_SC8238_MIPI_8M_20FPS_10BIT_WDR2TO1;
            break;

        case OMNIVISION_OS08A20_MIPI_2M_30FPS_10BIT:
        case OMNIVISION_OS08A20_MIPI_2M_30FPS_10BIT_WDR2TO1:
            type = OMNIVISION_OS08A20_MIPI_2M_30FPS_10BIT_WDR2TO1;
            break;

        case OMNIVISION_OS08A20_MIPI_4M_30FPS_10BIT:
        case OMNIVISION_OS08A20_MIPI_4M_30FPS_10BIT_WDR2TO1:
            type = OMNIVISION_OS08A20_MIPI_4M_30FPS_10BIT_WDR2TO1;
            break;

        case OMNIVISION_OS08A20_MIPI_5M_30FPS_10BIT:
        case OMNIVISION_OS08A20_MIPI_5M_30FPS_10BIT_WDR2TO1:
            type = OMNIVISION_OS08A20_MIPI_5M_30FPS_10BIT_WDR2TO1;
            break;

        case OMNIVISION_OS08A20_MIPI_8M_20FPS_10BIT:
        case OMNIVISION_OS08A20_MIPI_8M_20FPS_10BIT_WDR2TO1:
            type = OMNIVISION_OS08A20_MIPI_8M_20FPS_10BIT_WDR2TO1;
            break;

        case OMNIVISION_OS04A10_MIPI_4M_30FPS_12BIT:
        case OMNIVISION_OS04A10_MIPI_4M_30FPS_10BIT_WDR2TO1:
            type = OMNIVISION_OS04A10_MIPI_4M_30FPS_10BIT_WDR2TO1;
            break;
        case OMNIVISION_OS05A10_MIPI_4M_30FPS_12BIT:
        case OMNIVISION_OS05A10_MIPI_4M_30FPS_10BIT_WDR2TO1:
            type = OMNIVISION_OS05A10_MIPI_4M_30FPS_10BIT_WDR2TO1;
            break;
        case OMNIVISION_OS05A10_MIPI_5M_30FPS_12BIT:
        case OMNIVISION_OS05A10_MIPI_5M_30FPS_10BIT_WDR2TO1:
            type = OMNIVISION_OS05A10_MIPI_5M_30FPS_10BIT_WDR2TO1;
            break;
        case SMART_SC850SL_MIPI_8M_20FPS_12BIT:
        case SMART_SC850SL_MIPI_8M_20FPS_10BIT_WDR2TO1:
            type = SMART_SC850SL_MIPI_8M_20FPS_10BIT_WDR2TO1;
            break;
        case SMART_SC850SL_MIPI_5M_30FPS_12BIT:
        case SMART_SC850SL_MIPI_5M_20FPS_10BIT_WDR2TO1:
            type = SMART_SC850SL_MIPI_5M_20FPS_10BIT_WDR2TO1;
            break;
        case SMART_SC850SL_MIPI_4M_30FPS_12BIT:
        case SMART_SC850SL_MIPI_4M_20FPS_10BIT_WDR2TO1:
            type = SMART_SC850SL_MIPI_4M_20FPS_10BIT_WDR2TO1;
            break;
        case SMART_SC530AI_MIPI_5M_4L_30FPS_10BIT:
        case SMART_SC530AI_MIPI_5M_4L_30FPS_10BIT_WDR2TO1:
            type = SMART_SC530AI_MIPI_5M_4L_30FPS_10BIT_WDR2TO1;
            break;
        case GALAXYCORE_GC4663_MIPI_4M_30FPS_10BIT:
        case GALAXYCORE_GC4663_MIPI_4M_25FPS_10BIT_WDR2TO1:
            type = GALAXYCORE_GC4663_MIPI_4M_25FPS_10BIT_WDR2TO1;
            break;
        case SONY_IMX415_MIPI_8M_20FPS_10BIT:
        case SONY_IMX415_MIPI_8M_20FPS_10BIT_WDR2TO1:
            type = SONY_IMX415_MIPI_8M_20FPS_10BIT_WDR2TO1;
            break;
        case SONY_IMX678_MIPI_8M_20FPS_12BIT:
        case SONY_IMX678_MIPI_8M_20FPS_10BIT_WDR2TO1:
            type = SONY_IMX678_MIPI_8M_20FPS_10BIT_WDR2TO1;
            break;
        case SONY_IMX307_MIPI_2M_2L_30FPS_10BIT:
        case SONY_IMX307_MIPI_2M_2L_30FPS_10BIT_WDR2TO1:
            type = SONY_IMX307_MIPI_2M_2L_30FPS_10BIT_WDR2TO1;
            break;
        case OMNIVISION_OS02K10_MIPI_2M_30FPS_12BIT_BUILT_IN:
            type = OMNIVISION_OS02K10_MIPI_2M_30FPS_12BIT_BUILT_IN;
            break;
        case SMART_SC200AI_MIPI_2M_30FPS_10BIT:
        case SMART_SC200AI_MIPI_2M_30FPS_10BIT_WDR2TO1:
            type = SMART_SC200AI_MIPI_2M_30FPS_10BIT_WDR2TO1;
            break;
        case SMART_SC500AI_MIPI_5M_30FPS_10BIT:
        case SMART_SC500AI_MIPI_5M_30FPS_10BIT_WDR2TO1:
            type = SMART_SC500AI_MIPI_5M_30FPS_10BIT_WDR2TO1;
            break;
        case SMART_SC485SL_MIPI_4M_2688_1520_30FPS_4LANE_12BIT:
        case SMART_SC485SL_MIPI_4M_2560_1440_30FPS_4LANE_12BIT:
        case SMART_SC485SL_MIPI_4M_30FPS_2LANE_12BIT:
        case SMART_SC485SL_MIPI_4M_30FPS_4LANE_10BIT_WDR2TO1:
            type = SMART_SC485SL_MIPI_4M_30FPS_4LANE_10BIT_WDR2TO1;
            break;
        case SMART_SC465SL_MIPI_4M_4LANE_30FPS_12BIT:
        case SMART_SC465SL_MIPI_4M_2LANE_30FPS_12BIT:
        case SMART_SC465SL_MIPI_4M_30FPS_4LANE_10BIT_WDR2TO1:
            type = SMART_SC465SL_MIPI_4M_30FPS_4LANE_10BIT_WDR2TO1;
            break;
        default:
            SAMPLE_PRT("sensor type %d get wdr sensor type failed!\n", sensor_type);
            break;
    }

    return type;
}

xmedia_void sample_comm_vi_stitch_config_linear_init(sample_vi_stitch_param *vi_stitch_param, sample_vi_config *vi_config)
{
    xmedia_s32 vi_dev0 = vi_stitch_param->dev[0];
    xmedia_s32 vi_dev1 = vi_stitch_param->dev[1];
    xmedia_s32 vi_grp = vi_stitch_param->stitch_grp;
    xmedia_s32 vi_pipe0 = vi_stitch_param->vi_pipe[0];
    xmedia_s32 vi_pipe1 = vi_stitch_param->vi_pipe[2];
    xmedia_s32 vi_chn = vi_stitch_param->vi_chn;

    vi_config->dev_info[vi_dev0].dev_en = XMEDIA_TRUE;
    vi_config->dev_info[vi_dev0].dev_no = vi_dev0;
    vi_config->dev_info[vi_dev0].sensor_type = vi_stitch_param->sensor_type;
    vi_config->pipe_info[vi_pipe0].pipe_en = XMEDIA_TRUE;
    vi_config->pipe_info[vi_pipe0].pipe_no = vi_pipe0;
    vi_config->pipe_info[vi_pipe0].pipe_config.gdc_en = vi_stitch_param->gdc_en;
    vi_config->pipe_info[vi_pipe0].chn_info[vi_chn].chn_en = XMEDIA_TRUE;
    vi_config->pipe_info[vi_pipe0].chn_info[vi_chn].chn_no = vi_chn;
    vi_config->dev_bind_pipe[vi_dev0].pipe[0] = vi_pipe0;
    vi_config->dev_bind_pipe[vi_dev0].pipe[1] = -1;

    vi_config->dev_info[vi_dev1].dev_en = XMEDIA_TRUE;
    vi_config->dev_info[vi_dev1].dev_no = vi_dev1;
    vi_config->dev_info[vi_dev1].sensor_type = vi_stitch_param->sensor_type;
    vi_config->pipe_info[vi_pipe1].pipe_en = XMEDIA_TRUE;
    vi_config->pipe_info[vi_pipe1].pipe_no = vi_pipe1;
    vi_config->pipe_info[vi_pipe1].pipe_config.gdc_en = vi_stitch_param->gdc_en;
    vi_config->pipe_info[vi_pipe1].chn_info[vi_chn].chn_en = XMEDIA_TRUE;
    vi_config->pipe_info[vi_pipe1].chn_info[vi_chn].chn_no = vi_chn;
    vi_config->dev_bind_pipe[vi_dev1].pipe[0] = vi_pipe1;
    vi_config->dev_bind_pipe[vi_dev1].pipe[1] = -1;

    vi_config->grp_info[vi_grp].grp = vi_grp;
    vi_config->grp_info[vi_grp].grp_en = vi_stitch_param->stitch_en;
    vi_config->grp_info[vi_grp].grp_config.grp_type = XMEDIA_VI_GRP_TYPE_STITCH;
    vi_config->grp_info[vi_grp].grp_config.stitch_grp_config.pipe_num = 2;
    vi_config->grp_info[vi_grp].grp_config.stitch_grp_config.pipe_id[0] = vi_pipe0;
    vi_config->grp_info[vi_grp].grp_config.stitch_grp_config.pipe_id[1] = vi_pipe1;
}

xmedia_void sample_comm_vi_stitch_config_wdr_init(sample_vi_stitch_param *vi_stitch_param, sample_vi_config *vi_config)
{
    xmedia_s32 vi_dev0 = vi_stitch_param->dev[0];
    xmedia_s32 vi_dev1 = vi_stitch_param->dev[1];
    xmedia_s32 vi_wdr_grp0 = vi_stitch_param->wdr_grp[0];
    xmedia_s32 vi_wdr_grp1 = vi_stitch_param->wdr_grp[1];
    xmedia_s32 vi_stitch_grp = vi_stitch_param->stitch_grp;
    xmedia_s32 vi_pipe0 = vi_stitch_param->vi_pipe[0];
    xmedia_s32 vi_pipe1 = vi_stitch_param->vi_pipe[1];
    xmedia_s32 vi_pipe2 = vi_stitch_param->vi_pipe[2];
    xmedia_s32 vi_pipe3 = vi_stitch_param->vi_pipe[3];
    xmedia_s32 vi_chn = vi_stitch_param->vi_chn;

    vi_config->dev_info[vi_dev0].dev_en = XMEDIA_TRUE;
    vi_config->dev_info[vi_dev0].dev_no = vi_dev0;
    vi_config->dev_info[vi_dev0].sensor_type = vi_stitch_param->sensor_type;
    vi_config->pipe_info[vi_pipe0].pipe_en = XMEDIA_TRUE;
    vi_config->pipe_info[vi_pipe0].pipe_no = vi_pipe0;
    vi_config->pipe_info[vi_pipe0].pipe_config.gdc_en = vi_stitch_param->gdc_en;
    vi_config->pipe_info[vi_pipe0].chn_info[vi_chn].chn_en = XMEDIA_TRUE;
    vi_config->pipe_info[vi_pipe0].chn_info[vi_chn].chn_no = vi_chn;
    vi_config->pipe_info[vi_pipe1].pipe_en = XMEDIA_TRUE;
    vi_config->pipe_info[vi_pipe1].pipe_no = vi_pipe1;
    vi_config->pipe_info[vi_pipe1].chn_info[vi_chn].chn_en = XMEDIA_FALSE;
    vi_config->pipe_info[vi_pipe1].chn_info[vi_chn].chn_no = vi_chn;
    vi_config->dev_bind_pipe[vi_dev0].pipe[0] = vi_pipe0;
    vi_config->dev_bind_pipe[vi_dev0].pipe[1] = vi_pipe1;

    vi_config->grp_info[vi_wdr_grp0].grp = vi_wdr_grp0;
    vi_config->grp_info[vi_wdr_grp0].grp_en = XMEDIA_TRUE;
    vi_config->grp_info[vi_wdr_grp0].grp_config.grp_type = XMEDIA_VI_GRP_TYPE_WDR;
    vi_config->grp_info[vi_wdr_grp0].grp_config.wdr_grp_config.pipe_id[0] = vi_pipe0;
    vi_config->grp_info[vi_wdr_grp0].grp_config.wdr_grp_config.pipe_id[1] = vi_pipe1;
    vi_config->grp_info[vi_wdr_grp0].grp_config.wdr_grp_config.wdr_mode = vi_stitch_param->wdr_mode;

    vi_config->dev_info[vi_dev1].dev_en = XMEDIA_TRUE;
    vi_config->dev_info[vi_dev1].dev_no = vi_dev1;
    vi_config->dev_info[vi_dev1].sensor_type = vi_stitch_param->sensor_type;
    vi_config->pipe_info[vi_pipe2].pipe_en = XMEDIA_TRUE;
    vi_config->pipe_info[vi_pipe2].pipe_no = vi_pipe2;
    vi_config->pipe_info[vi_pipe2].pipe_config.gdc_en = vi_stitch_param->gdc_en;
    vi_config->pipe_info[vi_pipe2].chn_info[vi_chn].chn_en = XMEDIA_TRUE;
    vi_config->pipe_info[vi_pipe2].chn_info[vi_chn].chn_no = vi_chn;
    vi_config->pipe_info[vi_pipe3].pipe_en = XMEDIA_TRUE;
    vi_config->pipe_info[vi_pipe3].pipe_no = vi_pipe3;
    vi_config->pipe_info[vi_pipe3].chn_info[vi_chn].chn_en = XMEDIA_FALSE;
    vi_config->pipe_info[vi_pipe3].chn_info[vi_chn].chn_no = vi_chn;
    vi_config->dev_bind_pipe[vi_dev1].pipe[0] = vi_pipe2;
    vi_config->dev_bind_pipe[vi_dev1].pipe[1] = vi_pipe3;

    vi_config->grp_info[vi_wdr_grp1].grp = vi_wdr_grp1;
    vi_config->grp_info[vi_wdr_grp1].grp_en = XMEDIA_TRUE;
    vi_config->grp_info[vi_wdr_grp1].grp_config.grp_type = XMEDIA_VI_GRP_TYPE_WDR;
    vi_config->grp_info[vi_wdr_grp1].grp_config.wdr_grp_config.pipe_id[0] = vi_pipe2;
    vi_config->grp_info[vi_wdr_grp1].grp_config.wdr_grp_config.pipe_id[1] = vi_pipe3;
    vi_config->grp_info[vi_wdr_grp1].grp_config.wdr_grp_config.wdr_mode = vi_stitch_param->wdr_mode;

    vi_config->grp_info[vi_stitch_grp].grp = vi_stitch_grp;
    vi_config->grp_info[vi_stitch_grp].grp_en = vi_stitch_param->stitch_en;
    vi_config->grp_info[vi_stitch_grp].grp_config.grp_type = XMEDIA_VI_GRP_TYPE_STITCH;
    vi_config->grp_info[vi_stitch_grp].grp_config.stitch_grp_config.pipe_num = 2;
    vi_config->grp_info[vi_stitch_grp].grp_config.stitch_grp_config.pipe_id[0] = vi_pipe0;
    vi_config->grp_info[vi_stitch_grp].grp_config.stitch_grp_config.pipe_id[1] = vi_pipe2;
}

xmedia_void sample_comm_vi_sticth_config_init(sample_vi_stitch_param *vi_stitch_param, sample_vi_config *vi_config)
{
    if (sample_comm_isp_is_wdr_mode(vi_stitch_param->sensor_type) == XMEDIA_TRUE) {
        sample_comm_vi_stitch_config_wdr_init(vi_stitch_param, vi_config);
    } else {
        sample_comm_vi_stitch_config_linear_init(vi_stitch_param, vi_config);
    }
}

xmedia_void sample_comm_vi_wdr_config_init(sample_vi_wdr_param *wdr_param, sample_vi_config *vi_config)
{
    vi_config->dev_info[wdr_param->vi_dev].dev_en = XMEDIA_TRUE;
    vi_config->dev_info[wdr_param->vi_dev].dev_no = wdr_param->vi_dev;
    vi_config->dev_info[wdr_param->vi_dev].sensor_type = wdr_param->sensor_type;

    vi_config->pipe_info[wdr_param->vi_pipe[0]].pipe_en = XMEDIA_TRUE;
    vi_config->pipe_info[wdr_param->vi_pipe[0]].pipe_no = wdr_param->vi_pipe[0];
    vi_config->pipe_info[wdr_param->vi_pipe[0]].chn_info[wdr_param->vi_chn].chn_en = XMEDIA_TRUE;
    vi_config->pipe_info[wdr_param->vi_pipe[0]].chn_info[wdr_param->vi_chn].chn_no = wdr_param->vi_chn;

    vi_config->grp_info[wdr_param->vi_grp].grp    = wdr_param->vi_grp;
    vi_config->grp_info[wdr_param->vi_grp].grp_en = XMEDIA_TRUE;
    vi_config->grp_info[wdr_param->vi_grp].grp_config.grp_type = XMEDIA_VI_GRP_TYPE_WDR;
    vi_config->grp_info[wdr_param->vi_grp].grp_config.wdr_grp_config.pipe_id[0] = wdr_param->vi_pipe[0];
    vi_config->grp_info[wdr_param->vi_grp].grp_config.wdr_grp_config.wdr_mode   = wdr_param->wdr_mode;
    vi_config->grp_info[wdr_param->vi_grp].grp_config.wdr_grp_config.cache_line = wdr_param->cache_line;

    if (wdr_param->wdr_mode == XMEDIA_VIDEO_WDR_MODE_BUILT_IN) {
        vi_config->dev_bind_pipe[wdr_param->vi_dev].pipe[1] = -1;
        vi_config->grp_info[wdr_param->vi_grp].grp_config.wdr_grp_config.pipe_id[1] = -1;
    } else {
        vi_config->pipe_info[wdr_param->vi_pipe[1]].pipe_en = XMEDIA_TRUE;
        vi_config->pipe_info[wdr_param->vi_pipe[1]].pipe_no = wdr_param->vi_pipe[1];
        vi_config->pipe_info[wdr_param->vi_pipe[1]].chn_info[wdr_param->vi_chn].chn_en = XMEDIA_FALSE;
        vi_config->pipe_info[wdr_param->vi_pipe[1]].chn_info[wdr_param->vi_chn].chn_no = wdr_param->vi_chn;

        vi_config->dev_bind_pipe[wdr_param->vi_dev].pipe[0] = wdr_param->vi_pipe[0];
        vi_config->dev_bind_pipe[wdr_param->vi_dev].pipe[1] = wdr_param->vi_pipe[1];

        vi_config->grp_info[wdr_param->vi_grp].grp_config.wdr_grp_config.pipe_id[1] = wdr_param->vi_pipe[1];
        vi_config->grp_info[wdr_param->vi_grp].grp_config.wdr_grp_config.wdr_mode   = wdr_param->wdr_mode;
    }
}
