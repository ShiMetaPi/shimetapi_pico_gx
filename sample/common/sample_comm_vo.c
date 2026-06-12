/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include "sample_comm.h"
#include "sample_comm_vo.h"

typedef enum {
    SAMMPLE_SCREEN_SYNC_TIMING_MIPI_800X1280_SAT935BO30I30R13 = 0,
    SAMMPLE_SCREEN_SYNC_TIMING_MAX,
} sample_screen_sync_timing;

xmedia_vo_user_sync_timing sample_screen_user_sync_timing[SAMPLE_COMM_SCREEN_TYPE_INVALID] = {
    {480,  23, 12, 800,  210, 46, 1, 1,  1,  1, 2,  1,  0, 0, 0, 0, 1, 1},
    {1280, 24, 20, 800,  40,  40, 1, 1,  1,  1, 20, 4,  0, 0, 0, 0, 1, 1},
    {1080, 41, 4,  1920, 192, 88, 1, 1,  1,  1, 44, 5,  0, 0, 0, 0, 1, 1},
    {1280, 25, 16, 800,  88,  72, 1, 1,  1,  1, 24, 4,  0, 0, 0, 0, 1, 1},
    {800,  23, 15, 1280, 88,  72, 1, 1,  1,  1, 24, 2,  0, 0, 0, 0, 1, 1},
    {720,  25, 5,  1280, 260, 110,1, 1,  1,  1, 40, 5,  0, 0, 0, 0, 1, 1},
    {720,  45, 22, 1920, 136, 40, 1, 1,  1,  1, 20, 20, 0, 0, 0, 0, 1, 1},
    {1080, 41, 4,  1920, 192, 88, 1, 1,  1,  1, 44, 5,  0, 0, 0, 0, 1, 1},
    {320,  8,  8,  240,  14,  15, 1, 1,  1,  1, 4,  2,  0, 0, 0, 0, 1, 1},
    {2160, 0,  0,  3840, 0,   0,  0, 0,  0,  0, 0,  0,  0, 0, 0, 0, 0, 0},
    {2160, 82, 8,  3840, 384, 176,1, 1,  1,  1, 88, 10, 0, 0, 0, 0, 1, 1},
    {384,  8,  8,  1920, 27,  27, 1, 1,  1,  1, 13, 4,  1, 1, 1, 0, 1, 1},
    {240,  8,  8,  320,  130, 38, 1, 1,  1,  1, 2,  1,  0, 0, 0, 0, 1, 1},
    {480,  4,  8,  320,  8,   38, 1, 1,  1,  1, 1,  1,  0, 0, 0, 0, 1, 1},
    {240,  8,  8,  320,  293, 38, 1, 1,  1,  1, 2,  1,  0, 0, 0, 0, 1, 1},
    {1080, 41, 4,  1920, 192, 88, 1, 1,  1,  1, 44, 5,  0, 0, 0, 0, 1, 1},
    {720,  25, 5,  1280, 260, 110,1, 1,  1,  1, 40, 5,  0, 0, 0, 0, 1, 1},
    {720,  25, 5,  1280, 260, 110,1, 1,  1,  1, 40, 5,  0, 0, 0, 0, 1, 1},
};

xmedia_s32 sample_comm_vo_get_syncinfo(xmedia_vo_intf_sync intf_sync, xmedia_u32 *pst_wdith,
                                        xmedia_u32 *pst_height,xmedia_u32 *pst_freq)
{
    xmedia_u32 freq, wdith, height;

    switch (intf_sync) {
        case XMEDIA_VO_INTF_SYNC_PAL:
            wdith = 720;
            height = 576;
            freq = 25;
            break;
        case XMEDIA_VO_INTF_SYNC_NTSC:
            wdith = 720;
            height = 480;
            freq = 30;
            break;
        case XMEDIA_VO_INTF_SYNC_1080P24:
            wdith = 1920;
            height = 1080;
            freq = 24;
            break;
        case XMEDIA_VO_INTF_SYNC_1080P25:
            wdith = 1920;
            height = 1080;
            freq = 25;
            break;
        case XMEDIA_VO_INTF_SYNC_1080P30:
            wdith = 1920;
            height = 1080;
            freq = 30;
            break;
        case XMEDIA_VO_INTF_SYNC_1080I50:
            wdith = 1920;
            height = 1080;
            freq = 50;
            break;
        case XMEDIA_VO_INTF_SYNC_1080I60:
            wdith = 1920;
            height = 1080;
            freq = 60;
            break;
        case XMEDIA_VO_INTF_SYNC_1080P50:
            wdith = 1920;
            height = 1080;
            freq = 50;
            break;
        case XMEDIA_VO_INTF_SYNC_1080P60:
            wdith = 1920;
            height = 1080;
            freq = 60;
            break;
        case XMEDIA_VO_INTF_SYNC_720P60:
            wdith = 1280;
            height = 720;
            freq = 60;
            break;
        case XMEDIA_VO_INTF_SYNC_720P50:
            wdith = 1280;
            height = 720;
            freq = 50;
            break;
        case XMEDIA_VO_INTF_SYNC_576P50:
            wdith = 720;
            height = 576;
            freq = 50;
            break;
        case XMEDIA_VO_INTF_SYNC_480P60:
            wdith = 720;
            height = 480;
            freq = 60;
            break;
        case XMEDIA_VO_INTF_SYNC_800x600_60:
            wdith = 800;
            height = 600;
            freq = 60;
            break;
        case XMEDIA_VO_INTF_SYNC_1024x768_60:
            wdith = 1024;
            height = 768;
            freq = 60;
            break;
        case XMEDIA_VO_INTF_SYNC_1280x1024_60:
            wdith = 1280;
            height = 1024;
            freq = 60;
            break;
        case XMEDIA_VO_INTF_SYNC_1366x768_60:
            wdith = 1366;
            height = 768;
            freq = 60;
            break;
        case XMEDIA_VO_INTF_SYNC_1440x900_60:
            wdith = 1440;
            height = 900;
            freq = 60;
            break;
        case XMEDIA_VO_INTF_SYNC_1280x800_60:
            wdith = 1280;
            height = 800;
            freq = 60;
            break;
        case XMEDIA_VO_INTF_SYNC_1600x1200_60:
            wdith = 1600;
            height = 1200;
            freq = 60;
            break;
        case XMEDIA_VO_INTF_SYNC_1680x1050_60:
            wdith = 1680;
            height = 1050;
            freq = 60;
            break;
        case XMEDIA_VO_INTF_SYNC_1920x1200_60:
            wdith = 1920;
            height = 1200;
            freq = 60;
            break;
        case XMEDIA_VO_INTF_SYNC_640x480_60:
            wdith = 640;
            height = 480;
            freq = 60;
            break;
        case XMEDIA_VO_INTF_SYNC_960H_PAL:
            wdith = 960;
            height = 576;
            freq = 25;
            break;
        case XMEDIA_VO_INTF_SYNC_960H_NTSC:
            wdith = 960;
            height = 480;
            freq = 30;
            break;
        case XMEDIA_VO_INTF_SYNC_1920x2160_30:
            wdith = 1920;
            height = 2160;
            freq = 30;
            break;
        case XMEDIA_VO_INTF_SYNC_2560x1440_30:
            wdith = 2560;
            height = 1440;
            freq = 30;
            break;
        case XMEDIA_VO_INTF_SYNC_2560x1440_60:
            wdith = 2560;
            height = 1440;
            freq = 60;
            break;
        case XMEDIA_VO_INTF_SYNC_2560x1600_60:
            wdith = 2560;
            height = 1600;
            freq = 60;
            break;
        case XMEDIA_VO_INTF_SYNC_3840x2160_24:
            wdith = 3840;
            height = 2160;
            freq = 24;
            break;
        case XMEDIA_VO_INTF_SYNC_3840x2160_25:
            wdith = 3840;
            height = 2160;
            freq = 25;
            break;
        case XMEDIA_VO_INTF_SYNC_3840x2160_30:
            wdith = 3840;
            height = 2160;
            freq = 30;
            break;
        case XMEDIA_VO_INTF_SYNC_3840x2160_50:
            wdith = 3840;
            height = 2160;
            freq = 50;
            break;
        case XMEDIA_VO_INTF_SYNC_3840x2160_60:
            wdith = 3840;
            height = 2160;
            freq = 60;
            break;
        case XMEDIA_VO_INTF_SYNC_4096x2160_24:
            wdith = 4096;
            height = 2160;
            freq = 24;
            break;
        case XMEDIA_VO_INTF_SYNC_4096x2160_25:
            wdith = 4096;
            height = 2160;
            freq = 25;
            break;
        case XMEDIA_VO_INTF_SYNC_4096x2160_30:
            wdith = 4096;
            height = 2160;
            freq = 30;
            break;
        case XMEDIA_VO_INTF_SYNC_4096x2160_50:
            wdith = 4096;
            height = 2160;
            freq = 50;
            break;
        case XMEDIA_VO_INTF_SYNC_4096x2160_60:
            wdith = 4096;
            height = 2160;
            freq = 60;
            break;
        case XMEDIA_VO_INTF_SYNC_320x240_60:
            wdith = 320;
            height = 240;
            freq = 60;
            break;
        case XMEDIA_VO_INTF_SYNC_320x240_50:
            wdith = 320;
            height = 240;
            freq = 50;
            break;
        case XMEDIA_VO_INTF_SYNC_240x320_50:
            wdith = 240;
            height = 320;
            freq = 50;
            break;

        case XMEDIA_VO_INTF_SYNC_240x320_60:
            wdith = 240;
            height = 320;
            freq = 60;
            break;
        case XMEDIA_VO_INTF_SYNC_800x600_50:
            wdith = 800;
            height = 600;
            freq = 50;
            break;
        case XMEDIA_VO_INTF_SYNC_800x480_50:
            wdith = 800;
            height = 480;
            freq = 50;
            break;
        case XMEDIA_VO_INTF_SYNC_720x1280_60:
            wdith = 720;
            height = 1280;
            freq = 60;
            break;
        case XMEDIA_VO_INTF_SYNC_1080x1920_60:
            wdith = 1080;
            height = 1920;
            freq = 60;
            break;
        case XMEDIA_VO_INTF_SYNC_7680x4320_30:
            wdith = 7680;
            height = 4320;
            freq = 30;
            break;
        case XMEDIA_VO_INTF_SYNC_USER:
            wdith = 720;
            height = 576;
            freq = 25;
            break;
        default:
            return XMEDIA_FAILURE;
    }

    if (pst_wdith != XMEDIA_NULL) {
        *pst_wdith = wdith;
    }

    if (pst_height != XMEDIA_NULL) {
        *pst_height = height;
    }

    if (pst_freq != XMEDIA_NULL) {
        *pst_freq = freq;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_vo_get_dev_config(xmedia_s32 screen_type, sample_vo_config *vo_config,
    xmedia_u32 *pst_wdith, xmedia_u32 *pst_height,xmedia_u32 *pst_freq)
{
    vo_config->dev_config.bg_color = 0x00;
    switch(screen_type) {
        case MICROTECH_MTF050_LCD_800X480:
            vo_config->dev_config.intf_config.intf_type = XMEDIA_INTF_TYPE_LCD;
            vo_config->dev_config.intf_config.lcd_attr.data_mode = XMEDIA_VO_LCD_DATA_MODE_PARA;
            vo_config->dev_config.intf_config.lcd_attr.data_seq = XMEDIA_INTF_LCD_DATA_SEQ_RGB;
            vo_config->dev_config.intf_config.lcd_attr.data_type = XMEDIA_INTF_LCD_DATA_TYPE_RGB888_24BIT;
            vo_config->dev_csc.color_info.color_space = XMEDIA_VIDEO_COLOR_SPACE_RGB;
            vo_config->dev_config.user_sync_config.clk_reverse_en = 0;
            vo_config->dev_config.user_sync_config.frame_rate = 60;
            vo_config->dev_config.intf_sync = XMEDIA_VO_INTF_SYNC_USER;
            break;
        case SATOZ_SAT935_MIPI_800X1280_RGB8888_24BIT:
            vo_config->dev_config.intf_config.intf_type = XMEDIA_INTF_TYPE_MIPI_DSI;
            vo_config->dev_config.intf_config.mipi_attr.data_type = XMEDIA_INTF_MIPI_DSI_DATA_TYPE_RGB888_24BIT;
            vo_config->dev_config.intf_config.mipi_attr.lane_sel[0] = XMEDIA_VO_MIPI_LANE_DATA0;
            vo_config->dev_config.intf_config.mipi_attr.lane_sel[1] = XMEDIA_VO_MIPI_LANE_DATA1;
            vo_config->dev_config.intf_config.mipi_attr.lane_sel[2] = XMEDIA_VO_MIPI_LANE_CLK;
            vo_config->dev_config.intf_config.mipi_attr.lane_sel[3] = XMEDIA_VO_MIPI_LANE_DATA2;
            vo_config->dev_config.intf_config.mipi_attr.lane_sel[4] = XMEDIA_VO_MIPI_LANE_DATA3;
            vo_config->dev_config.intf_config.mipi_attr.lane_pn_swap[0] = 0;
            vo_config->dev_config.intf_config.mipi_attr.lane_pn_swap[1] = 0;
            vo_config->dev_config.intf_config.mipi_attr.lane_pn_swap[2] = 0;
            vo_config->dev_config.intf_config.mipi_attr.lane_pn_swap[3] = 0;
            vo_config->dev_config.intf_config.mipi_attr.lane_pn_swap[4] = 0;
            vo_config->dev_config.intf_config.mipi_attr.mipi_htotal_adjust = 1;
            vo_config->dev_config.intf_config.mipi_attr.mipi_vsync_adjust = 2;
            vo_config->dev_csc.color_info.color_space = XMEDIA_VIDEO_COLOR_SPACE_RGB;
            vo_config->dev_config.user_sync_config.clk_reverse_en = 0;
            vo_config->dev_config.user_sync_config.frame_rate = 60;
            vo_config->dev_config.intf_sync = XMEDIA_VO_INTF_SYNC_USER;
            break;
        case SIL9024_BT1120_1920X1080P60:
            vo_config->dev_config.intf_config.intf_type = XMEDIA_INTF_TYPE_BT1120;
            vo_config->dev_config.intf_config.bt1120_attr.data_seq = XMEDIA_INTF_BT1120_DATA_SEQ_UVUV;
            vo_config->dev_config.intf_config.bt1120_attr.data_type = XMEDIA_INTF_BT_DATA_TYPE_YUV422_8BIT;
            vo_config->dev_csc.color_info.color_space = XMEDIA_VIDEO_COLOR_SPACE_YUV;
            vo_config->dev_config.user_sync_config.clk_reverse_en = 1;
            vo_config->dev_config.user_sync_config.frame_rate = 60;
            vo_config->dev_config.intf_sync = XMEDIA_VO_INTF_SYNC_USER;
            break;
        case MICROTECH_MTF101_FT01_LVDS_800X1280_RGB666_18BIT:
            vo_config->dev_config.intf_config.intf_type = XMEDIA_INTF_TYPE_LVDS;
            vo_config->dev_config.intf_config.lvds_attr.format = XMEDIA_INTF_LVDS_FORMAT_VESA;
            vo_config->dev_config.intf_config.lvds_attr.data_type = XMEDIA_INTF_LVDS_DATA_TYPE_RGB666_18BIT;
            vo_config->dev_config.intf_config.lvds_attr.lane_sel[0] = XMEDIA_VO_LVDS_LANE_DATA0;
            vo_config->dev_config.intf_config.lvds_attr.lane_sel[1] = XMEDIA_VO_LVDS_LANE_DATA1;
            vo_config->dev_config.intf_config.lvds_attr.lane_sel[2] = XMEDIA_VO_LVDS_LANE_DATA2;
            vo_config->dev_config.intf_config.lvds_attr.lane_sel[3] = XMEDIA_VO_LVDS_LANE_CLK;
            vo_config->dev_config.intf_config.lvds_attr.lane_sel[4] = XMEDIA_VO_LVDS_LANE_DATA3;
            vo_config->dev_config.intf_config.lvds_attr.lane_pn_swap[0] = 0;
            vo_config->dev_config.intf_config.lvds_attr.lane_pn_swap[1] = 0;
            vo_config->dev_config.intf_config.lvds_attr.lane_pn_swap[2] = 0;
            vo_config->dev_config.intf_config.lvds_attr.lane_pn_swap[3] = 0;
            vo_config->dev_config.intf_config.lvds_attr.lane_pn_swap[4] = 0;
            vo_config->dev_csc.color_info.color_space = XMEDIA_VIDEO_COLOR_SPACE_RGB;
            vo_config->dev_config.user_sync_config.clk_reverse_en = 0;
            vo_config->dev_config.user_sync_config.frame_rate = 60;
            vo_config->dev_config.intf_sync = XMEDIA_VO_INTF_SYNC_USER;
            break;
        case MICROTECH_MTF101_FS16_LVDS_1280X800_RGB888_24BIT:
            vo_config->dev_config.intf_config.intf_type = XMEDIA_INTF_TYPE_LVDS;
            vo_config->dev_config.intf_config.lvds_attr.format = XMEDIA_INTF_LVDS_FORMAT_VESA;
            vo_config->dev_config.intf_config.lvds_attr.data_type = XMEDIA_INTF_LVDS_DATA_TYPE_RGB888_24BIT;
            vo_config->dev_config.intf_config.lvds_attr.lane_sel[0] = XMEDIA_VO_LVDS_LANE_DATA0;
            vo_config->dev_config.intf_config.lvds_attr.lane_sel[1] = XMEDIA_VO_LVDS_LANE_DATA1;
            vo_config->dev_config.intf_config.lvds_attr.lane_sel[2] = XMEDIA_VO_LVDS_LANE_DATA2;
            vo_config->dev_config.intf_config.lvds_attr.lane_sel[3] = XMEDIA_VO_LVDS_LANE_CLK;
            vo_config->dev_config.intf_config.lvds_attr.lane_sel[4] = XMEDIA_VO_LVDS_LANE_DATA3;
            vo_config->dev_config.intf_config.lvds_attr.lane_pn_swap[0] = 0;
            vo_config->dev_config.intf_config.lvds_attr.lane_pn_swap[1] = 0;
            vo_config->dev_config.intf_config.lvds_attr.lane_pn_swap[2] = 0;
            vo_config->dev_config.intf_config.lvds_attr.lane_pn_swap[3] = 0;
            vo_config->dev_config.intf_config.lvds_attr.lane_pn_swap[4] = 0;
            vo_config->dev_csc.color_info.color_space = XMEDIA_VIDEO_COLOR_SPACE_RGB;
            vo_config->dev_config.user_sync_config.clk_reverse_en = 0;
            vo_config->dev_config.user_sync_config.frame_rate = 60;
            vo_config->dev_config.intf_sync = XMEDIA_VO_INTF_SYNC_USER;
            break;
        case C3200B_EVB_BT656_1280X720_UYVY422:
            vo_config->dev_config.intf_config.intf_type = XMEDIA_INTF_TYPE_BT656;
            vo_config->dev_config.intf_config.bt656_attr.data_seq = XMEDIA_INTF_BT601_BT656_DATA_SEQ_UYVY;
            vo_config->dev_config.intf_config.bt656_attr.data_type = XMEDIA_INTF_BT_DATA_TYPE_YUV422_8BIT;
            vo_config->dev_csc.color_info.color_space = XMEDIA_VIDEO_COLOR_SPACE_YUV;
            vo_config->dev_config.user_sync_config.clk_reverse_en = 0;
            vo_config->dev_config.user_sync_config.frame_rate = 30;
            vo_config->dev_config.intf_sync = XMEDIA_VO_INTF_SYNC_USER;
            break;
        case DEMO_LVDS_1920X720_RGB888_24BIT:
            vo_config->dev_config.intf_config.intf_type = XMEDIA_INTF_TYPE_LVDS;
            vo_config->dev_config.intf_config.lvds_attr.format = XMEDIA_INTF_LVDS_FORMAT_VESA;
            vo_config->dev_config.intf_config.lvds_attr.data_type = XMEDIA_INTF_LVDS_DATA_TYPE_RGB888_24BIT;
            vo_config->dev_config.intf_config.lvds_attr.lane_sel[0] = XMEDIA_VO_LVDS_LANE_DATA0;
            vo_config->dev_config.intf_config.lvds_attr.lane_sel[1] = XMEDIA_VO_LVDS_LANE_DATA1;
            vo_config->dev_config.intf_config.lvds_attr.lane_sel[2] = XMEDIA_VO_LVDS_LANE_DATA2;
            vo_config->dev_config.intf_config.lvds_attr.lane_sel[3] = XMEDIA_VO_LVDS_LANE_DATA3;
            vo_config->dev_config.intf_config.lvds_attr.lane_sel[4] = XMEDIA_VO_LVDS_LANE_CLK;
            vo_config->dev_csc.color_info.color_space = XMEDIA_VIDEO_COLOR_SPACE_RGB;
            vo_config->dev_config.user_sync_config.clk_reverse_en = 0;
            vo_config->dev_config.user_sync_config.frame_rate = 60;
            vo_config->dev_config.intf_sync = XMEDIA_VO_INTF_SYNC_USER;
            break;
        case MIPI_TO_HDMI_1920X1080_RGB888_24BIT:
            vo_config->dev_config.intf_config.intf_type = XMEDIA_INTF_TYPE_MIPI_DSI;
            vo_config->dev_config.intf_config.mipi_attr.data_type = XMEDIA_INTF_MIPI_DSI_DATA_TYPE_RGB888_24BIT;
            vo_config->dev_config.intf_config.mipi_attr.lane_sel[0] = XMEDIA_VO_MIPI_LANE_DATA3;
            vo_config->dev_config.intf_config.mipi_attr.lane_sel[1] = XMEDIA_VO_MIPI_LANE_DATA2;
            vo_config->dev_config.intf_config.mipi_attr.lane_sel[2] = XMEDIA_VO_MIPI_LANE_CLK;
            vo_config->dev_config.intf_config.mipi_attr.lane_sel[3] = XMEDIA_VO_MIPI_LANE_DATA1;
            vo_config->dev_config.intf_config.mipi_attr.lane_sel[4] = XMEDIA_VO_MIPI_LANE_DATA0;
            vo_config->dev_config.intf_config.mipi_attr.lane_pn_swap[0] = 1;
            vo_config->dev_config.intf_config.mipi_attr.lane_pn_swap[1] = 1;
            vo_config->dev_config.intf_config.mipi_attr.lane_pn_swap[2] = 1;
            vo_config->dev_config.intf_config.mipi_attr.lane_pn_swap[3] = 1;
            vo_config->dev_config.intf_config.mipi_attr.lane_pn_swap[4] = 1;
            vo_config->dev_config.intf_config.mipi_attr.mipi_htotal_adjust = 0;
            vo_config->dev_config.intf_config.mipi_attr.mipi_vsync_adjust = 0;
            vo_config->dev_csc.color_info.color_space = XMEDIA_VIDEO_COLOR_SPACE_RGB;
            vo_config->dev_config.user_sync_config.clk_reverse_en = 0;
            vo_config->dev_config.user_sync_config.frame_rate = 60;
            vo_config->dev_config.intf_sync = XMEDIA_VO_INTF_SYNC_USER;
            break;
        case MTF024QV33B_SLCD_240X320_18BIT:
            vo_config->dev_config.intf_config.intf_type = XMEDIA_INTF_TYPE_LCD;
            vo_config->dev_config.intf_config.lcd_attr.data_mode = XMEDIA_VO_LCD_DATA_MODE_SERIAL;
            vo_config->dev_config.intf_config.lcd_attr.data_seq = XMEDIA_INTF_LCD_DATA_SEQ_RGB;
            vo_config->dev_config.intf_config.lcd_attr.data_type = XMEDIA_INTF_LCD_DATA_TYPE_RGB666_18BIT;
            vo_config->dev_csc.color_info.color_space = XMEDIA_VIDEO_COLOR_SPACE_RGB;
            vo_config->dev_config.user_sync_config.clk_reverse_en = 1;
            vo_config->dev_config.user_sync_config.frame_rate = 60;
            vo_config->dev_config.intf_sync = XMEDIA_VO_INTF_SYNC_USER;
            break;
        case VIRTUAL_SCREEN_MIPI_3840X2160P30:
            vo_config->dev_config.intf_config.intf_type = XMEDIA_INTF_TYPE_MIPI_DSI;
            vo_config->dev_config.intf_config.mipi_attr.data_type = XMEDIA_INTF_MIPI_DSI_DATA_TYPE_RGB888_24BIT;
            vo_config->dev_csc.color_info.color_space = XMEDIA_VIDEO_COLOR_SPACE_RGB;
            vo_config->dev_config.user_sync_config.clk_reverse_en = 0;
            vo_config->dev_config.user_sync_config.frame_rate = 30;
            vo_config->dev_config.intf_sync = XMEDIA_VO_INTF_SYNC_USER;
            break;
        case MIPI_TO_HDMI_3840X2160_RGB888_24BIT:
            vo_config->dev_config.intf_config.intf_type = XMEDIA_INTF_TYPE_MIPI_DSI;
            vo_config->dev_config.intf_config.mipi_attr.data_type = XMEDIA_INTF_MIPI_DSI_DATA_TYPE_RGB888_24BIT;
            vo_config->dev_config.intf_config.mipi_attr.lane_sel[0] = XMEDIA_VO_MIPI_LANE_DATA3;
            vo_config->dev_config.intf_config.mipi_attr.lane_sel[1] = XMEDIA_VO_MIPI_LANE_DATA2;
            vo_config->dev_config.intf_config.mipi_attr.lane_sel[2] = XMEDIA_VO_MIPI_LANE_CLK;
            vo_config->dev_config.intf_config.mipi_attr.lane_sel[3] = XMEDIA_VO_MIPI_LANE_DATA1;
            vo_config->dev_config.intf_config.mipi_attr.lane_sel[4] = XMEDIA_VO_MIPI_LANE_DATA0;
            vo_config->dev_config.intf_config.mipi_attr.lane_pn_swap[0] = 1;
            vo_config->dev_config.intf_config.mipi_attr.lane_pn_swap[1] = 1;
            vo_config->dev_config.intf_config.mipi_attr.lane_pn_swap[2] = 1;
            vo_config->dev_config.intf_config.mipi_attr.lane_pn_swap[3] = 1;
            vo_config->dev_config.intf_config.mipi_attr.lane_pn_swap[4] = 1;
            vo_config->dev_config.intf_config.mipi_attr.mipi_htotal_adjust = 0;
            vo_config->dev_config.intf_config.mipi_attr.mipi_vsync_adjust = 0;
            vo_config->dev_csc.color_info.color_space = XMEDIA_VIDEO_COLOR_SPACE_RGB;
            vo_config->dev_config.user_sync_config.clk_reverse_en = 0;
            vo_config->dev_config.user_sync_config.frame_rate = 30;
            vo_config->dev_config.intf_sync = XMEDIA_VO_INTF_SYNC_USER;
            break;
        case HX_M090P24349A_LVDS_VESA_1920X384_RGB888_24BIT:
            vo_config->dev_config.intf_config.intf_type = XMEDIA_INTF_TYPE_LVDS;
            vo_config->dev_config.intf_config.lvds_attr.format = XMEDIA_INTF_LVDS_FORMAT_VESA;
            vo_config->dev_config.intf_config.lvds_attr.data_type = XMEDIA_INTF_LVDS_DATA_TYPE_RGB888_24BIT;
            vo_config->dev_config.intf_config.lvds_attr.lane_sel[0] = XMEDIA_VO_LVDS_LANE_DATA0;
            vo_config->dev_config.intf_config.lvds_attr.lane_sel[1] = XMEDIA_VO_LVDS_LANE_DATA1;
            vo_config->dev_config.intf_config.lvds_attr.lane_sel[2] = XMEDIA_VO_LVDS_LANE_DATA2;
            vo_config->dev_config.intf_config.lvds_attr.lane_sel[3] = XMEDIA_VO_LVDS_LANE_DATA3;
            vo_config->dev_config.intf_config.lvds_attr.lane_sel[4] = XMEDIA_VO_LVDS_LANE_CLK;
            vo_config->dev_csc.color_info.color_space = XMEDIA_VIDEO_COLOR_SPACE_RGB;
            vo_config->dev_config.user_sync_config.clk_reverse_en = 1;
            vo_config->dev_config.user_sync_config.frame_rate = 60;
            vo_config->dev_config.intf_sync = XMEDIA_VO_INTF_SYNC_USER;
            break;
        case MCU_KD026_320X240P6_4LINE_RGB565:
            vo_config->dev_config.intf_config.intf_type = XMEDIA_INTF_TYPE_MCU;
            vo_config->dev_config.intf_config.mcu_attr.te_mode_en = XMEDIA_FALSE;
            vo_config->dev_config.intf_config.mcu_attr.data_type = XMEDIA_VO_MCU_DATA_TYPE_4LINE_RGB565;
            vo_config->dev_csc.color_info.color_space = XMEDIA_VIDEO_COLOR_SPACE_RGB;
            vo_config->dev_config.user_sync_config.frame_rate = 10;
            vo_config->dev_config.intf_sync = XMEDIA_VO_INTF_SYNC_USER;
            break;
        case MCU_KD035_320X480P6_3LINE_RGB666_OPTION1:
            vo_config->dev_config.intf_config.intf_type = XMEDIA_INTF_TYPE_MCU;
            vo_config->dev_config.intf_config.mcu_attr.te_mode_en = XMEDIA_FALSE;
            vo_config->dev_config.intf_config.mcu_attr.data_type = XMEDIA_VO_MCU_DATA_TYPE_3LINE_RGB666_OPTION1;
            vo_config->dev_csc.color_info.color_space = XMEDIA_VIDEO_COLOR_SPACE_RGB;
            vo_config->dev_config.user_sync_config.frame_rate = 6;
            vo_config->dev_config.intf_sync = XMEDIA_VO_INTF_SYNC_USER;
            break;
        case MCU_KD026_320X240P60_18BIT_18BPP_RGB666_8080I:
            vo_config->dev_config.intf_config.intf_type = XMEDIA_INTF_TYPE_MCU;
            vo_config->dev_config.intf_config.mcu_attr.te_mode_en = XMEDIA_FALSE;
            vo_config->dev_config.intf_config.mcu_attr.data_type = XMEDIA_VO_MCU_DATA_TYPE_18BIT_18BPP_RGB666_8080I;
            vo_config->dev_csc.color_info.color_space = XMEDIA_VIDEO_COLOR_SPACE_RGB;
            vo_config->dev_config.user_sync_config.frame_rate = 60;
            vo_config->dev_config.intf_sync = XMEDIA_VO_INTF_SYNC_USER;
            break;
        case SIL9024_BT1120_1920X1080P30:
            vo_config->dev_config.intf_config.intf_type = XMEDIA_INTF_TYPE_BT1120;
            vo_config->dev_config.intf_config.bt1120_attr.data_seq = XMEDIA_INTF_BT1120_DATA_SEQ_UVUV;
            vo_config->dev_config.intf_config.bt1120_attr.data_type = XMEDIA_INTF_BT_DATA_TYPE_YUV422_8BIT;
            vo_config->dev_csc.color_info.color_space = XMEDIA_VIDEO_COLOR_SPACE_YUV;
            vo_config->dev_config.user_sync_config.clk_reverse_en = 1;
            vo_config->dev_config.user_sync_config.frame_rate = 30;
            vo_config->dev_config.intf_sync = XMEDIA_VO_INTF_SYNC_USER;
            break;
        case SIL9024_BT1120_1280X720P60:
            vo_config->dev_config.intf_config.intf_type = XMEDIA_INTF_TYPE_BT1120;
            vo_config->dev_config.intf_config.bt1120_attr.data_seq = XMEDIA_INTF_BT1120_DATA_SEQ_UVUV;
            vo_config->dev_config.intf_config.bt1120_attr.data_type = XMEDIA_INTF_BT_DATA_TYPE_YUV422_8BIT;
            vo_config->dev_csc.color_info.color_space = XMEDIA_VIDEO_COLOR_SPACE_YUV;
            vo_config->dev_config.user_sync_config.clk_reverse_en = 1;
            vo_config->dev_config.user_sync_config.frame_rate = 60;
            vo_config->dev_config.intf_sync = XMEDIA_VO_INTF_SYNC_USER;
            break;
        case SIL9024_BT656_1280X720P30_UYVY422:
            vo_config->dev_config.intf_config.intf_type = XMEDIA_INTF_TYPE_BT656;
            vo_config->dev_config.intf_config.bt656_attr.data_seq = XMEDIA_INTF_BT601_BT656_DATA_SEQ_UYVY;
            vo_config->dev_config.intf_config.bt656_attr.data_type = XMEDIA_INTF_BT_DATA_TYPE_YUV422_8BIT;
            vo_config->dev_csc.color_info.color_space = XMEDIA_VIDEO_COLOR_SPACE_YUV;
            vo_config->dev_config.user_sync_config.clk_reverse_en = 1;
            vo_config->dev_config.user_sync_config.frame_rate = 30;
            vo_config->dev_config.intf_sync = XMEDIA_VO_INTF_SYNC_USER;
            break;
        default:
             SAMPLE_PRT("screen_type err !\n");
             return XMEDIA_FAILURE;
    }
    memcpy(&vo_config->dev_config.user_sync_config.timing_info, &sample_screen_user_sync_timing[screen_type],
        sizeof(xmedia_vo_user_sync_timing));
    *pst_wdith = sample_screen_user_sync_timing[screen_type].hact;
    *pst_height = sample_screen_user_sync_timing[screen_type].vact;
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_vo_get_default_config(sample_vo_config_input config_in, sample_vo_config *vo_config)
{
    xmedia_u32 freq, width, height, j;
    if (vo_config != XMEDIA_NULL) {
        if (sample_comm_vo_get_dev_config(config_in.screen_type, vo_config, &width, &height, &freq) != XMEDIA_SUCCESS) {
            SAMPLE_PRT("can not get dev config information!\n");
            return XMEDIA_FAILURE;
        }

        //csc info
        vo_config->dev_csc.color_info.color_gamut = XMEDIA_VIDEO_COLOR_GAMUT_BT709;
        vo_config->dev_csc.color_info.quantify_range = XMEDIA_VIDEO_COLOR_FULL_RANGE;
        vo_config->dev_csc.mode = XMEDIA_VO_DEV_MODE_VIDEO_GRAPHIC;
        vo_config->dev_csc.hue = 50;
        vo_config->dev_csc.luma = 50;
        vo_config->dev_csc.saturation = 50;
        vo_config->dev_csc.contrast = 50;

        //layer info
        vo_config->layer_config.img_size.width = width;
        vo_config->layer_config.img_size.height = height;
        vo_config->layer_config.pix_format = XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_420;
        vo_config->layer_config.bit_width = XMEDIA_VIDEO_DATA_WIDTH_8;
        vo_config->layer_config.part_mode = XMEDIA_VO_PARTITION_MODE_SINGLE;
        vo_config->layer_config.disp_buf_len = 0;

        //chn info
         for (j = 0; j < (config_in.square_sort); j++) {
            if (j == 0 ){
                vo_config->chn_attr[j].out_rect.x = 0;
                vo_config->chn_attr[j].out_rect.y = 0;
            } else if (j == 1) {
                vo_config->chn_attr[j].out_rect.x = width / 2;
                vo_config->chn_attr[j].out_rect.y = 0;
            } else if (j == 2) {
                vo_config->chn_attr[j].out_rect.x = 0;
                vo_config->chn_attr[j].out_rect.y = height / 2;
            } else {
                vo_config->chn_attr[j].out_rect.x = width / 2;
                vo_config->chn_attr[j].out_rect.y = height / 2;
            }

            if (config_in.square_sort > 1) {
                if (config_in.square_sort == 2) {
                    vo_config->chn_attr[j].out_rect.width = width / 2;
                    vo_config->chn_attr[j].out_rect.height = height;
                } else {
                    vo_config->chn_attr[j].out_rect.width = width / 2;
                    vo_config->chn_attr[j].out_rect.height = height / 2;
                }
            } else {
                vo_config->chn_attr[j].out_rect.width = width;
                vo_config->chn_attr[j].out_rect.height = height;
            }
            vo_config->chn_attr[j].region_rect.x = vo_config->chn_attr[j].out_rect.x;
            vo_config->chn_attr[j].region_rect.y = vo_config->chn_attr[j].out_rect.y;
            vo_config->chn_attr[j].region_rect.width = vo_config->chn_attr[j].out_rect.width;
            vo_config->chn_attr[j].region_rect.height = vo_config->chn_attr[j].out_rect.height;
            vo_config->chn_attr[j].bg_color = 0xff;
            vo_config->chn_attr[j].zorder = j;
        }
    }else {
        SAMPLE_PRT("vo_config is null!\n");
        return XMEDIA_FAILURE;
    }
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_vo_start(xmedia_vo_dev dev, xmedia_vo_layer layer, xmedia_vo_chn chn[],xmedia_s32 chn_num,
                                    xmedia_bool buf_len_set,sample_vo_config *vo_config)
{
    xmedia_s32 i;

    if (vo_config != XMEDIA_NULL) {
        CHECK_RET(xmedia_vo_set_dev_config(dev, &(vo_config->dev_config)), "xmedia_vo_set_dev_config");
        CHECK_RET(xmedia_vo_set_dev_csc(dev, &(vo_config->dev_csc)), "xmedia_vo_set_dev_csc");
        CHECK_RET(xmedia_vo_enable_dev(dev), "xmedia_vo_enable_dev");

        layer = (xmedia_vo_layer)dev;
        if (buf_len_set) {
            vo_config->layer_config.disp_buf_len = 3;
        }

        CHECK_RET(xmedia_vo_set_layer_config(layer, &(vo_config->layer_config)), "xmedia_vo_set_layer_config");
        CHECK_RET(xmedia_vo_enable_layer(layer), "xmedia_vo_enable_layer");

        for (i = 0; i < chn_num; i++) {
            SAMPLE_PRT("dev:%d chn:%d chn_rect[%d-%d-%d-%d]\n", dev, chn[i], vo_config->chn_attr[i].out_rect.x,
                vo_config->chn_attr[i].out_rect.y,
                vo_config->chn_attr[i].out_rect.width, vo_config->chn_attr[i].out_rect.height);

            if (vo_config->lowdelay_attr.enable == XMEDIA_TRUE) {
                CHECK_RET(xmedia_vo_set_chn_low_delay(layer, chn[i], vo_config->lowdelay_attr.mode), "set chn rotate");
            }

            CHECK_RET(xmedia_vo_set_chn_attr(layer, chn[i], &(vo_config->chn_attr[i])), "xmedia_vo_set_chn_attr");
            CHECK_RET(xmedia_vo_enable_chn(layer, chn[i]), "xmedia_vo_enable_chn");
        }
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_vo_stop(xmedia_vo_dev dev, xmedia_vo_layer layer, xmedia_vo_chn chn[], xmedia_s32 chn_num)
{
    xmedia_s32 i;

    for (i = 0; i < chn_num; i++){
        CHECK_RET(xmedia_vo_disable_chn(layer, chn[i]), "xmedia_vo_disable_chn");
    }
    CHECK_RET(xmedia_vo_disable_layer(layer), "xmedia_vo_disable_layer");

    CHECK_RET(xmedia_vo_disable_dev(dev), "xmedia_vo_disable_dev");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_vo_init(xmedia_void)
{
    return xmedia_vo_init();
}

xmedia_s32 sample_comm_vo_exit(xmedia_void)
{
    return xmedia_vo_exit();
}

