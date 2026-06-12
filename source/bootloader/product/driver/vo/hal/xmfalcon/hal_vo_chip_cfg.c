/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include "hal_vo_chip_cfg.h"
#define VO_IRQ_NAME "intr_vo"
#define VO_INTF_NOT_MIPI_MAX_WHIDTH 1920
#define VO_INTF_NOT_MIPI_MAX_CLK 148500000
#define VO_INTF_MIPI_MAX_CLK 297000000
#define VO_IRQ_NUM_7606V1 47

xmedia_s32 g_vo_irq = VO_IRQ_NUM_7606V1;

static vo_base_cap g_vo_cap;
static xmedia_bool g_vo_cap_is_inited = XMEDIA_FALSE;
static vo_layer_cap g_layer_capability[XMEDIA_VO_LAYER_MAX];
static vo_dev_cap g_dev_capability[XMEDIA_VO_DEV_MAX];

static xmedia_void vo_init_chip_capacity(xmedia_void)
{
    g_vo_cap.irq_name = VO_IRQ_NAME;
    g_vo_cap.irq_num = g_vo_irq;
    g_vo_cap.irq_cpumask = 0;
    g_vo_cap.vo_phy_addr = 0x11420000;
    g_vo_cap.chip_name = "xm7606v1";
    g_vo_cap.chip_revision = 0;
    g_vo_cap.chip_type = 0;
    g_vo_cap.board_id =0;
    return;
}

static xmedia_void vo_init_chip_dev_capacity(xmedia_void)
{
     int i = 0;
     for (i = 0; i < XMEDIA_VO_DEV_MAX; i++) {
         g_dev_capability[i].support_video_layer_num = 1;
         g_dev_capability[i].support_gfx_layer_num = 1;
         g_dev_capability[i].support_max_layer_num = 2;
         g_dev_capability[i].support_intf_num = 1;
         g_dev_capability[i].support_csc_num = 3;
         if(i == XMEDIA_VO_DEV_1) {
             g_dev_capability[i].max_size.width = 1920;
             g_dev_capability[i].max_size.height = 1080;
         } else {
             g_dev_capability[i].max_size.width = 3840;
             g_dev_capability[i].max_size.height = 2160;
         }
         g_dev_capability[i].max_frame_rate = 60;
         g_dev_capability[i].support_ink = XMEDIA_TRUE;
         g_dev_capability[i].support_attach_layer = XMEDIA_FALSE;
    }
    return;
}

static xmedia_void vo_init_chip_layer_capacity(xmedia_void)
{
     int i = 0;
     for (i = 0; i < XMEDIA_VO_LAYER_MAX; i++) {
         g_layer_capability[i].support_hdr = XMEDIA_FALSE;
         g_layer_capability[i].support_flip = XMEDIA_FALSE;
         g_layer_capability[i].support_mirror = XMEDIA_FALSE;
         g_layer_capability[i].support_zme = XMEDIA_FALSE;
         g_layer_capability[i].support_dcmp = XMEDIA_FALSE;

         g_layer_capability[i].support_region = XMEDIA_TRUE;
         g_layer_capability[i].region_num = 4;
         if(i == XMEDIA_VO_DEV_1) {
             g_layer_capability[i].max_read_size.width = 1920;
             g_layer_capability[i].max_read_size.height = 1080;
         } else {
             g_layer_capability[i].max_read_size.width = 3840;
             g_layer_capability[i].max_read_size.height = 2160;
         }
         g_layer_capability[i].min_out_size.width = 32;
         g_layer_capability[i].min_out_size.height = 32;

         g_layer_capability[i].support_max_bit_width = 8;
         g_layer_capability[i].support_pixformat_num = 5;
    }
    return;
}

static xmedia_s32 vo_check_bt656_attr(xmedia_vo_dev dev, xmedia_vo_bt656_attr *bt656_attr,
    xmedia_vo_intf_sync intf_sync)
{
    if (bt656_attr->data_type != XMEDIA_INTF_BT_DATA_TYPE_YUV422_8BIT) {
        VO_TRACE(MODULE_DBG_ERR,"dev %d bt656 intf not support data_type(%d) !\n", dev, bt656_attr->data_type);
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    if ((bt656_attr->data_seq != XMEDIA_INTF_BT601_BT656_DATA_SEQ_UYVY) &&
        (bt656_attr->data_seq != XMEDIA_INTF_BT601_BT656_DATA_SEQ_VYUY) &&
        (bt656_attr->data_seq != XMEDIA_INTF_BT601_BT656_DATA_SEQ_YUYV) &&
        (bt656_attr->data_seq != XMEDIA_INTF_BT601_BT656_DATA_SEQ_YVYU)){
        VO_TRACE(MODULE_DBG_ERR,"dev %d bt656 intf not support data_seq(%d) !\n", dev, bt656_attr->data_seq);
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    if ((intf_sync != XMEDIA_VO_INTF_SYNC_USER) &&
        (intf_sync != XMEDIA_VO_INTF_SYNC_PAL) && (intf_sync != XMEDIA_VO_INTF_SYNC_NTSC) &&
        (intf_sync != XMEDIA_VO_INTF_SYNC_960H_PAL) && (intf_sync != XMEDIA_VO_INTF_SYNC_960H_NTSC)) {
            VO_TRACE(MODULE_DBG_ERR,"dev %d bt656 intf, intfsync %d illegal!\n", dev, intf_sync);
            return XMEDIA_ERRCODE_NOT_SUPPORT;
   }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 vo_check_bt1120_attr(xmedia_vo_dev dev, xmedia_vo_bt1120_attr *bt1120_attr,
    xmedia_vo_intf_sync intf_sync)
{
    if (bt1120_attr->data_type != XMEDIA_INTF_BT_DATA_TYPE_YUV422_8BIT) {
        VO_TRACE(MODULE_DBG_ERR,"dev %d bt1120 intf not support data_type(%d) !\n", dev, bt1120_attr->data_type);
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    if ((bt1120_attr->data_seq != XMEDIA_INTF_BT1120_DATA_SEQ_UVUV) &&
        (bt1120_attr->data_seq != XMEDIA_INTF_BT1120_DATA_SEQ_VUVU)){
        VO_TRACE(MODULE_DBG_ERR,"dev %d bt1120 intf not support data_seq(%d) !\n", dev, bt1120_attr->data_seq);
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    if ((intf_sync != XMEDIA_VO_INTF_SYNC_USER) &&
        (intf_sync != XMEDIA_VO_INTF_SYNC_1080P24) && (intf_sync != XMEDIA_VO_INTF_SYNC_1080P25) &&
        (intf_sync != XMEDIA_VO_INTF_SYNC_1080P30) && (intf_sync != XMEDIA_VO_INTF_SYNC_1080I50) &&
        (intf_sync != XMEDIA_VO_INTF_SYNC_1080I60) && (intf_sync != XMEDIA_VO_INTF_SYNC_1080P50) &&
        (intf_sync != XMEDIA_VO_INTF_SYNC_1080P60) && (intf_sync != XMEDIA_VO_INTF_SYNC_720P50) &&
        (intf_sync != XMEDIA_VO_INTF_SYNC_720P60) && (intf_sync != XMEDIA_VO_INTF_SYNC_576P50) &&
        (intf_sync != XMEDIA_VO_INTF_SYNC_480P60) && (intf_sync != XMEDIA_VO_INTF_SYNC_960H_PAL) &&
        (intf_sync != XMEDIA_VO_INTF_SYNC_960H_NTSC)) {
        VO_TRACE(MODULE_DBG_ERR,"dev %d bt1120 intf, intfsync %d illegal!\n", dev, intf_sync);
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 vo_check_lcd_sync(xmedia_vo_dev dev, xmedia_vo_lcd_attr *lcd_attr,
    xmedia_vo_intf_sync intf_sync)
{
    xmedia_intf_lcd_data_type data_type = lcd_attr->data_type;
    xmedia_vo_lcd_data_mode data_mode = lcd_attr->data_mode;
    //XMEDIA_VO_INTF_SYNC_USER lcd 用户自定义时序 是否会有问题????
    if ((data_mode == XMEDIA_VO_LCD_DATA_MODE_SERIAL) && (data_type == XMEDIA_INTF_LCD_DATA_TYPE_RGB666_18BIT)) {
        if ((intf_sync != XMEDIA_VO_INTF_SYNC_320x240_50) && (intf_sync != XMEDIA_VO_INTF_SYNC_240x320_50) && 
            (intf_sync != XMEDIA_VO_INTF_SYNC_USER)) {
            VO_TRACE(MODULE_DBG_ERR,"For LCD 6bit intface,dev %d intfsync %d illegal!\n", dev, intf_sync);
            return XMEDIA_ERRCODE_NOT_SUPPORT;
        }
    }

    if ((data_mode == XMEDIA_VO_LCD_DATA_MODE_SERIAL) && (data_type == XMEDIA_INTF_LCD_DATA_TYPE_RGB888_24BIT)) {
        if ((intf_sync != XMEDIA_VO_INTF_SYNC_320x240_60) && (intf_sync != XMEDIA_VO_INTF_SYNC_USER)) {
            VO_TRACE(MODULE_DBG_ERR,"For LCD 8bit intface,dev %d intfsync %d illegal!\n", dev, intf_sync);
            return XMEDIA_ERRCODE_NOT_SUPPORT;
        }
    }

    if ((data_mode == XMEDIA_VO_LCD_DATA_MODE_PARA) && (data_type == XMEDIA_INTF_LCD_DATA_TYPE_RGB565_16BIT)) {
        if ((intf_sync != XMEDIA_VO_INTF_SYNC_240x320_60) && (intf_sync != XMEDIA_VO_INTF_SYNC_640x480_60) &&
            (intf_sync != XMEDIA_VO_INTF_SYNC_USER)) {
            VO_TRACE(MODULE_DBG_ERR,"For LCD 16bit intface,dev %d intfsync %d illegal!\n", dev, intf_sync);
            return XMEDIA_ERRCODE_NOT_SUPPORT;
        }
    }

    if ((data_mode == XMEDIA_VO_LCD_DATA_MODE_PARA) && (data_type == XMEDIA_INTF_LCD_DATA_TYPE_RGB666_18BIT)) {
        if ((intf_sync != XMEDIA_VO_INTF_SYNC_240x320_60) && (intf_sync != XMEDIA_VO_INTF_SYNC_640x480_60) &&
            (intf_sync != XMEDIA_VO_INTF_SYNC_USER)) {
            VO_TRACE(MODULE_DBG_ERR,"For LCD 18bit intface,dev %d intfsync %d illegal!\n", dev, intf_sync);
            return XMEDIA_ERRCODE_NOT_SUPPORT;
        }
    }

    if ((data_mode == XMEDIA_VO_LCD_DATA_MODE_PARA) && (data_type == XMEDIA_INTF_LCD_DATA_TYPE_RGB888_24BIT)) {
        if ((intf_sync != XMEDIA_VO_INTF_SYNC_240x320_60) && (intf_sync != XMEDIA_VO_INTF_SYNC_640x480_60) &&
            (intf_sync != XMEDIA_VO_INTF_SYNC_800x480_50) && (intf_sync != XMEDIA_VO_INTF_SYNC_USER)) {
            VO_TRACE(MODULE_DBG_ERR,"For LCD 24bit intface,dev %d intfsync %d illegal!\n", dev, intf_sync);
            return XMEDIA_ERRCODE_NOT_SUPPORT;
        }
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 vo_check_lcd_attr(xmedia_vo_dev dev, xmedia_vo_lcd_attr *lcd_attr,
    xmedia_vo_intf_sync intf_sync)
{
    xmedia_intf_lcd_data_type data_type = lcd_attr->data_type;
    xmedia_vo_lcd_data_mode data_mode = lcd_attr->data_mode;

    if ((data_type != XMEDIA_INTF_LCD_DATA_TYPE_RGB565_16BIT) && (data_type != XMEDIA_INTF_LCD_DATA_TYPE_RGB666_18BIT) &&
        (data_type != XMEDIA_INTF_LCD_DATA_TYPE_RGB888_24BIT)) {
        VO_TRACE(MODULE_DBG_ERR,"dev %d lcd intf not support data_type(%d) !\n", dev, data_type);
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    if ((data_mode != XMEDIA_VO_LCD_DATA_MODE_SERIAL) && (data_mode != XMEDIA_VO_LCD_DATA_MODE_PARA)) {
        VO_TRACE(MODULE_DBG_ERR,"dev %d lcd intf not support data_mode(%d) !\n", dev, data_mode);
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    if ((lcd_attr->data_seq != XMEDIA_INTF_LCD_DATA_SEQ_RGB) && (lcd_attr->data_seq != XMEDIA_INTF_LCD_DATA_SEQ_RBG) &&
        (lcd_attr->data_seq != XMEDIA_INTF_LCD_DATA_SEQ_GRB) && (lcd_attr->data_seq != XMEDIA_INTF_LCD_DATA_SEQ_GBR) &&
        (lcd_attr->data_seq != XMEDIA_INTF_LCD_DATA_SEQ_BRG) && (lcd_attr->data_seq != XMEDIA_INTF_LCD_DATA_SEQ_BGR)) {
        VO_TRACE(MODULE_DBG_ERR,"dev %d lcd intf not support data_seq(%d) !\n", dev, lcd_attr->data_seq);
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    if (data_mode == XMEDIA_VO_LCD_DATA_MODE_SERIAL) {
        if ((data_type != XMEDIA_INTF_LCD_DATA_TYPE_RGB666_18BIT) && (data_type != XMEDIA_INTF_LCD_DATA_TYPE_RGB888_24BIT)) {
            VO_TRACE(MODULE_DBG_ERR,"dev %d lcd intf support 18&24 bit in serial mode(%d) !\n", dev, data_type);
            return XMEDIA_ERRCODE_NOT_SUPPORT;
        }
    }
    return vo_check_lcd_sync(dev, lcd_attr, intf_sync);
}

static xmedia_s32 vo_check_mipi_attr(xmedia_vo_dev dev, xmedia_vo_mipi_attr *mipi_attr,
    xmedia_vo_intf_sync intf_sync)
{
#ifdef VO_KERNEL
    if (!(CHIP_ONLY_SUPPORT_MIPITX() || CHIP_BOTH_SUPPORT_MIPITX_LVDSTX())) {
        VO_TRACE(MODULE_DBG_ERR,"dev %d mipi intf not support, CHIP_TYPE err!\n", dev);
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }
#endif
    xmedia_s32 i, j;
    for (i = XMEDIA_VO_MIPI_LANE_DATA0; i < XMEDIA_VO_MIPI_LANE_MAX - 1; i++) {
        for (j = i + 1; j < XMEDIA_VO_MIPI_LANE_MAX; j++) {
            if (mipi_attr->lane_sel[i] == mipi_attr->lane_sel[j]) {
                VO_TRACE(MODULE_DBG_ERR,"dev %d mipi intf not support duplicate (lane sel[%d] and lane_sel[%d])!\n",
                    dev, mipi_attr->lane_sel[i], mipi_attr->lane_sel[j]);
                    return XMEDIA_ERRCODE_NOT_SUPPORT;
            }
            if (mipi_attr->lane_sel[i] >= XMEDIA_VO_MIPI_LANE_MAX) {
                VO_TRACE(MODULE_DBG_ERR,"dev %d mipi intf, mipi lane sel is out of support(%d) !\n",
                    dev, mipi_attr->lane_sel[i]);
                    return XMEDIA_ERRCODE_NOT_SUPPORT;
            }
            if (mipi_attr->lane_sel[j] >= XMEDIA_VO_MIPI_LANE_MAX) {
                VO_TRACE(MODULE_DBG_ERR,"dev %d mipi intf, mipi lane sel is out of support(%d) !\n",
                    dev, mipi_attr->lane_sel[j]);
                    return XMEDIA_ERRCODE_NOT_SUPPORT;
            }
        }
    }

    for (i = XMEDIA_VO_MIPI_LANE_DATA0; i < XMEDIA_VO_MIPI_LANE_MAX; i++) {
        if ((mipi_attr->lane_pn_swap[i] != XMEDIA_TRUE) && (mipi_attr->lane_pn_swap[i] != XMEDIA_FALSE)) {
            VO_TRACE(MODULE_DBG_ERR,"dev %d mipi intf, mipi lane swap is out of support(%d) !\n",
                 dev, mipi_attr->lane_pn_swap[j]);
            return XMEDIA_ERRCODE_NOT_SUPPORT;
        }
    }
    if ((mipi_attr->data_type != XMEDIA_INTF_MIPI_DSI_DATA_TYPE_YUV422_16BIT) &&
        (mipi_attr->data_type != XMEDIA_INTF_MIPI_DSI_DATA_TYPE_RGB666_18BIT) &&
        (mipi_attr->data_type != XMEDIA_INTF_MIPI_DSI_DATA_TYPE_RGB888_24BIT)){
        VO_TRACE(MODULE_DBG_ERR,"dev %d lcd intf not support data_type(%d) !\n", dev, mipi_attr->data_type);
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    //需要测试是否支持所有的这些分辨率
    if ((intf_sync != XMEDIA_VO_INTF_SYNC_USER) &&
        (((intf_sync >= XMEDIA_VO_INTF_SYNC_3840x2160_50) && (intf_sync <= XMEDIA_VO_INTF_SYNC_4096x2160_60)) ||
        (intf_sync == XMEDIA_VO_INTF_SYNC_7680x4320_30) || (intf_sync == XMEDIA_VO_INTF_SYNC_PAL) ||
        (intf_sync == XMEDIA_VO_INTF_SYNC_NTSC) || (intf_sync == XMEDIA_VO_INTF_SYNC_960H_PAL) ||
        (intf_sync == XMEDIA_VO_INTF_SYNC_960H_NTSC) || (intf_sync == XMEDIA_VO_INTF_SYNC_1080I50) ||
        (intf_sync == XMEDIA_VO_INTF_SYNC_1080I60))) {
        VO_TRACE(MODULE_DBG_ERR,"dev %d mipi intf, intfsync %d illegal!\n", dev, intf_sync);
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    if (mipi_attr->mipi_lane_num < XMEDIA_VO_MIPI_LANE_DEFAULT || mipi_attr->mipi_lane_num >= XMEDIA_VO_MIPI_LANE_NUM_MAX) {
        VO_TRACE(MODULE_DBG_ERR, "dev %d not support mipi_lane_num %d", dev, mipi_attr->mipi_lane_num);
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    if (mipi_attr->mipi_lane_num == XMEDIA_VO_MIPI_LANE_DEFAULT) {
        mipi_attr->mipi_lane_num = XMEDIA_VO_MIPI_LANE_NUM4;
        VO_TRACE(MODULE_DBG_INFO, "mipi_lane_num %d\n", mipi_attr->mipi_lane_num);
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 vo_check_lvds_attr(xmedia_vo_dev dev, xmedia_vo_lvds_attr *lvds_attr,
    xmedia_vo_intf_sync intf_sync)
{
    xmedia_s32 i, j;
#ifdef VO_KERNEL
    if (!(CHIP_ONLY_SUPPORT_LVDSTX() || CHIP_BOTH_SUPPORT_MIPITX_LVDSTX())) {
        VO_TRACE(MODULE_DBG_ERR,"dev %d lvds intf not support, CHIP_TYPE err!\n", dev);
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }
#endif
    if ((lvds_attr->data_type != XMEDIA_INTF_LVDS_DATA_TYPE_RGB666_18BIT) &&
        (lvds_attr->data_type != XMEDIA_INTF_LVDS_DATA_TYPE_RGB888_24BIT)) {
        VO_TRACE(MODULE_DBG_ERR,"dev %d lvds intf not support data_type(%d) !\n", dev, lvds_attr->data_type);
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    if ((lvds_attr->format != XMEDIA_INTF_LVDS_FORMAT_VESA) && (lvds_attr->format != XMEDIA_INTF_LVDS_FORMAT_JEIDA)) {
        VO_TRACE(MODULE_DBG_ERR,"dev %d lvds intf not support format(%d) !\n", dev, lvds_attr->format);
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    for (i = XMEDIA_VO_LVDS_LANE_DATA0; i < XMEDIA_VO_LVDS_LANE_MAX - 1; i++) {
        for (j = i + 1; j < XMEDIA_VO_LVDS_LANE_MAX; j++) {
            if (lvds_attr->lane_sel[i] == lvds_attr->lane_sel[j]) {
                VO_TRACE(MODULE_DBG_ERR,"dev %d lvds intf not support duplicate (lane sel[%d] and lane_sel[%d])!\n",
                    dev, lvds_attr->lane_sel[i], lvds_attr->lane_sel[j]);
                    return XMEDIA_ERRCODE_NOT_SUPPORT;
            }
            if (lvds_attr->lane_sel[i] >= XMEDIA_VO_LVDS_LANE_MAX) {
                VO_TRACE(MODULE_DBG_ERR,"dev %d lvds intf, lvds lane sel is out of support(%d) !\n",
                    dev, lvds_attr->lane_sel[i]);
                    return XMEDIA_ERRCODE_NOT_SUPPORT;
            }
            if (lvds_attr->lane_sel[j] >= XMEDIA_VO_LVDS_LANE_MAX) {
                VO_TRACE(MODULE_DBG_ERR,"dev %d lvds intf, lvds lane sel is out of support(%d) !\n",
                    dev, lvds_attr->lane_sel[j]);
                    return XMEDIA_ERRCODE_NOT_SUPPORT;
            }
        }
    }

    //需要测试是否支持所有的这些分辨率
    if ((intf_sync != XMEDIA_VO_INTF_SYNC_USER) &&
        (((intf_sync >= XMEDIA_VO_INTF_SYNC_1920x2160_30) && (intf_sync <= XMEDIA_VO_INTF_SYNC_4096x2160_60)) ||
        (intf_sync == XMEDIA_VO_INTF_SYNC_7680x4320_30) || (intf_sync == XMEDIA_VO_INTF_SYNC_1920x1200_60) ||
        (intf_sync == XMEDIA_VO_INTF_SYNC_PAL) || (intf_sync == XMEDIA_VO_INTF_SYNC_NTSC) ||
        (intf_sync == XMEDIA_VO_INTF_SYNC_960H_PAL) || (intf_sync == XMEDIA_VO_INTF_SYNC_960H_NTSC) ||
        (intf_sync == XMEDIA_VO_INTF_SYNC_1080I50) || (intf_sync == XMEDIA_VO_INTF_SYNC_1080I60))) {
        VO_TRACE(MODULE_DBG_ERR,"dev %d lvds intf, intfsync %d illegal!\n", dev, intf_sync);
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 vo_check_dev_size(xmedia_vo_dev dev, xmedia_intf_type intf_type,
    xmedia_video_size *dev_size) {
    if (VO_CHECK_VIRT_DEV(dev) == XMEDIA_TRUE) {
        if (dev_size->width > g_dev_capability[dev].max_size.width ||
            dev_size->height > g_dev_capability[dev].max_size.height) {
            VO_TRACE(MODULE_DBG_ERR, "dev%d not support dev_size_width %d\n", dev, dev_size->width);
            return XMEDIA_ERRCODE_NOT_SUPPORT;
        }
    } else {
        if (dev_size->width > g_dev_capability[dev].max_size.width) {
            VO_TRACE(MODULE_DBG_ERR, "dev%d not support dev_size_width %d\n", dev, dev_size->width);
            return XMEDIA_ERRCODE_NOT_SUPPORT;
        }
        //只有mipi支持 width>1920以上时序
        if ((dev_size->width > VO_INTF_NOT_MIPI_MAX_WHIDTH) && (intf_type != XMEDIA_INTF_TYPE_MIPI_DSI)) {
            VO_TRACE(MODULE_DBG_ERR, "only mipi width support 1920+\n");
            return XMEDIA_ERRCODE_NOT_SUPPORT;
        }
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 hal_vo_check_dev_intf(xmedia_vo_dev dev, xmedia_vo_dev_config *config, xmedia_video_size *dev_size)
{
    xmedia_s32 ret;
    if (vo_check_dev_size(dev, config->intf_config.intf_type, dev_size) != XMEDIA_SUCCESS) {
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    //虚拟设备不需要后续的check
    if (VO_CHECK_VIRT_DEV(dev) == XMEDIA_TRUE) {
        return XMEDIA_SUCCESS;
    }

    switch (config->intf_config.intf_type){
        case XMEDIA_INTF_TYPE_BT656:
             ret = vo_check_bt656_attr(dev, &(config->intf_config.bt656_attr), config->intf_sync);
             break;
        case XMEDIA_INTF_TYPE_BT1120:
             ret = vo_check_bt1120_attr(dev, &(config->intf_config.bt1120_attr), config->intf_sync);
             break;
        case XMEDIA_INTF_TYPE_LCD:
             ret = vo_check_lcd_attr(dev, &(config->intf_config.lcd_attr), config->intf_sync);
             break;
        case XMEDIA_INTF_TYPE_MIPI_DSI:
             ret = vo_check_mipi_attr(dev, &(config->intf_config.mipi_attr), config->intf_sync);
             break;
        case XMEDIA_INTF_TYPE_LVDS:
             ret = vo_check_lvds_attr(dev, &(config->intf_config.lvds_attr), config->intf_sync);
             break;
        default:
            ret = XMEDIA_ERRCODE_NOT_SUPPORT;
            break;
    }

    return ret;
}

xmedia_s32 hal_vo_check_clk(xmedia_vo_dev dev, xmedia_vo_dev_config *config, xmedia_u32 clk)
{
    //dev=0 且 mipi 端子才支持297M时钟，否则只支持148.5M
    if ((dev == XMEDIA_VO_DEV_0) && (config->intf_config.intf_type == XMEDIA_INTF_TYPE_MIPI_DSI)) {
        if (clk > VO_INTF_MIPI_MAX_CLK) {
            VO_TRACE(MODULE_DBG_ERR,"hal_vo_check_clk err, clk %d\n", clk);
            return XMEDIA_ERRCODE_NOT_SUPPORT;
        }
    } else {
        if (clk > VO_INTF_NOT_MIPI_MAX_CLK) {
            VO_TRACE(MODULE_DBG_ERR,"hal_vo_check_clk err, clk %d\n", clk);
            return XMEDIA_ERRCODE_NOT_SUPPORT;
        }
    }
    return XMEDIA_SUCCESS;
}

xmedia_s32 hal_vo_check_dev_csc_cap(xmedia_vo_dev dev, xmedia_vo_intf_config *intf_config,
    xmedia_vo_dev_csc *dev_csc)
{
    xmedia_bool mipi_rgb_en;

    if (dev_csc->color_info.color_gamut == XMEDIA_VIDEO_COLOR_GAMUT_BT2020) {
        VO_TRACE(MODULE_DBG_ERR,"Vo dev(%d) color gamut should be bt601 or 709\n", dev);
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    if (((intf_config->intf_type == XMEDIA_INTF_TYPE_LCD) || (intf_config->intf_type == XMEDIA_INTF_TYPE_LVDS)) &&
        (dev_csc->color_info.color_space == XMEDIA_VIDEO_COLOR_SPACE_YUV)) {
        VO_TRACE(MODULE_DBG_ERR,"Vo dev(%d) lcd/lvds should be output rgb\n", dev);
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    if (((intf_config->intf_type == XMEDIA_INTF_TYPE_BT656) || (intf_config->intf_type == XMEDIA_INTF_TYPE_BT1120)) &&
        (dev_csc->color_info.color_space == XMEDIA_VIDEO_COLOR_SPACE_RGB)) {
        VO_TRACE(MODULE_DBG_ERR,"Vo dev(%d) bt656/bt1120 should be output yuv\n", dev);
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    if (intf_config->intf_type == XMEDIA_INTF_TYPE_MIPI_DSI) {
        if ((intf_config->mipi_attr.data_type == XMEDIA_INTF_MIPI_DSI_DATA_TYPE_RGB666_18BIT) ||
            (intf_config->mipi_attr.data_type == XMEDIA_INTF_MIPI_DSI_DATA_TYPE_RGB888_24BIT)){
            mipi_rgb_en = XMEDIA_TRUE;
        } else {
            mipi_rgb_en = XMEDIA_FALSE;
        }

        if ((mipi_rgb_en == XMEDIA_TRUE) && (dev_csc->color_info.color_space == XMEDIA_VIDEO_COLOR_SPACE_YUV)){
           VO_TRACE(MODULE_DBG_ERR,"dev %d color space should be same as mipi output(%d,%d)\n",
               dev, mipi_rgb_en, dev_csc->color_info.color_space);
           return XMEDIA_ERRCODE_NOT_SUPPORT;
        }

        if ((mipi_rgb_en == XMEDIA_FALSE) && (dev_csc->color_info.color_space == XMEDIA_VIDEO_COLOR_SPACE_RGB)){
           VO_TRACE(MODULE_DBG_ERR,"dev %d color space should be same as mipi output(%d,%d)\n",
               dev, mipi_rgb_en, dev_csc->color_info.color_space);
           return XMEDIA_ERRCODE_NOT_SUPPORT;
        }
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 hal_vo_check_dev_gamma_cap(xmedia_vo_dev dev, xmedia_vo_intf_config *intf_config)
{
    if ((intf_config->intf_type == XMEDIA_INTF_TYPE_BT656) || (intf_config->intf_type == XMEDIA_INTF_TYPE_BT1120)) {
        VO_TRACE(MODULE_DBG_DEBUG,"Vo dev(%d) yuv output not support gamma set\n", dev);
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }
    if ((intf_config->intf_type == XMEDIA_INTF_TYPE_MIPI_DSI) &&
        (intf_config->mipi_attr.data_type == XMEDIA_INTF_MIPI_DSI_DATA_TYPE_YUV422_16BIT)) {
        VO_TRACE(MODULE_DBG_DEBUG,"Vo dev(%d) yuv output not support gamma set\n", dev);
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }
    return XMEDIA_SUCCESS;
}

xmedia_bool hal_vo_check_chip_type(vo_chip_type board_id)
{
    if (board_id == VO_CHIP_TYPE_XM7606V1) {
        return XMEDIA_TRUE;
    } else {
        return XMEDIA_FALSE;
    }
}

xmedia_s32 hal_vo_check_layer_cap(xmedia_vo_layer layer, xmedia_u32 max_width, xmedia_u32 max_height,
    xmedia_vo_layer_config *layer_config)
{
    if ((layer_config->img_size.width < g_layer_capability[layer].min_out_size.width) ||
        (layer_config->img_size.height < g_layer_capability[layer].min_out_size.height)) {
        VO_TRACE(MODULE_DBG_ERR,"layer %d image size(%d, %d) is illegaled, should biger than (%d,%d)!\n",
            layer, layer_config->img_size.width, layer_config->img_size.height,
            g_layer_capability[layer].min_out_size.width, g_layer_capability[layer].min_out_size.height);
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    if (layer_config->img_size.width > max_width || layer_config->img_size.height > max_height) {
        VO_TRACE(MODULE_DBG_ERR,"layer %d image size width(%d %d) is illegaled, should less than(%d %d)!\n",
            layer, layer_config->img_size.width, layer_config->img_size.height, max_width, max_height);
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    if ((IS_ALIGN(layer_config->img_size.width, 2) != XMEDIA_TRUE) ||
        (IS_ALIGN(layer_config->img_size.height, 2) != XMEDIA_TRUE)) {
        VO_TRACE(MODULE_DBG_ERR,"layer %d img size(%d, %d) dosen't aligned by 2 pixel!\n",
            layer, layer_config->img_size.width, layer_config->img_size.height);
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    if ((layer_config->pix_format != XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_420) &&
        (layer_config->pix_format != XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_422) &&
        (layer_config->pix_format != XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420) &&
        (layer_config->pix_format != XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_422) &&
        (layer_config->pix_format != XMEDIA_VIDEO_PIXEL_FMT_YUV_400)) {
        VO_TRACE(MODULE_DBG_ERR,"layer %d pix_format %d doesn't support!\n", layer, layer_config->pix_format);
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }
    if (layer_config->bit_width != XMEDIA_VIDEO_DATA_WIDTH_8) {
        VO_TRACE(MODULE_DBG_ERR,"layer %d bit_width %d doesn't support!\n", layer, layer_config->bit_width);
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }
    if (layer_config->part_mode >= XMEDIA_VO_PARTITION_MODE_MAX) {
        VO_TRACE(MODULE_DBG_ERR,"layer %d part mode %d is illegal!\n", layer, layer_config->part_mode);
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 hal_vo_check_layer_frame(xmedia_vo_layer layer, xmedia_video_frame_info *frame_info)
{
    xmedia_u32 valid_width;
    xmedia_u32 valid_height;

    valid_width = g_layer_capability[layer].min_out_size.width;
    valid_height = g_layer_capability[layer].min_out_size.height;
    if ((frame_info->frame.pixel_fmt != XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_422) &&
       (frame_info->frame.pixel_fmt != XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_422) &&
       (frame_info->frame.pixel_fmt != XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_420) &&
       (frame_info->frame.pixel_fmt != XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420) &&
       (frame_info->frame.pixel_fmt != XMEDIA_VIDEO_PIXEL_FMT_YUV_400)) {
         VO_TRACE(MODULE_DBG_ERR,"Unsupport input pixel format %d!\n", frame_info->frame.pixel_fmt);
         return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    if ((frame_info->frame.width < valid_width) || (frame_info->frame.height < valid_height) ||
       (IS_ALIGN(frame_info->frame.width, 2) != XMEDIA_TRUE) ||
       (IS_ALIGN(frame_info->frame.width, 2) != XMEDIA_TRUE)) {
        VO_TRACE(MODULE_DBG_ERR,"pic width(%d) is smaller than min width 32 or pic height(%d) is smaller than"
            "min height 32 or not align2!!\n", frame_info->frame.width, frame_info->frame.height);
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    if (frame_info->frame.dynamic_range != XMEDIA_VIDEO_DYNAMIC_RANGE_SDR) {
        VO_TRACE(MODULE_DBG_ERR,"Unsupport input dynamic range%d!\n", frame_info->frame.dynamic_range);
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    if (frame_info->frame.compress_mode != XMEDIA_VIDEO_COMPRESS_MODE_NONE) {
        VO_TRACE(MODULE_DBG_ERR,"Unsupport input compress mode%d!\n", frame_info->frame.compress_mode);
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    if (frame_info->frame.video_fmt != XMEDIA_VIDEO_FMT_LINEAR) {
        VO_TRACE(MODULE_DBG_ERR,"Unsupport input video format%d!\n", frame_info->frame.video_fmt);
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    if (frame_info->frame.bit_width != XMEDIA_VIDEO_DATA_WIDTH_8) {
        VO_TRACE(MODULE_DBG_ERR,"Unsupport input bit width%d!\n", frame_info->frame.bit_width);
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    if ((frame_info->frame.color_info.color_space != XMEDIA_VIDEO_COLOR_SPACE_YUV) ||
       (frame_info->frame.color_info.color_gamut == XMEDIA_VIDEO_COLOR_GAMUT_BT2020) ||
       (frame_info->frame.color_info.color_gamut >= XMEDIA_VIDEO_COLOR_GAMUT_MAX) ||
       (frame_info->frame.color_info.quantify_range >= XMEDIA_VIDEO_COLOR_RANGE_MAX)) {
        VO_TRACE(MODULE_DBG_ERR,"Unsupport input color info:space-gamut-range(%d-%d-%d)!\n",
            frame_info->frame.color_info.color_space,
            frame_info->frame.color_info.color_gamut,frame_info->frame.color_info.quantify_range);
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    return XMEDIA_SUCCESS;
}

xmedia_void hal_vo_init_chip_capaciblity(xmedia_void)
{
    if (g_vo_cap_is_inited == XMEDIA_TRUE) {
        return;
    }
    g_vo_cap_is_inited = XMEDIA_TRUE;

    vo_init_chip_capacity();
    vo_init_chip_dev_capacity();
    vo_init_chip_layer_capacity();
}

xmedia_u32 hal_vo_get_chip_revision(xmedia_void)
{
    return g_vo_cap.chip_revision;
}

vo_base_cap *hal_vo_get_chip_capacity(xmedia_void)
{
    return &g_vo_cap;
}

vo_layer_cap *hal_get_layer_chip_capacity(xmedia_vo_layer layer)
{
    return &g_layer_capability[layer];
}

vo_dev_cap *hal_get_dev_chip_capacity(xmedia_vo_dev dev)
{
    return &g_dev_capability[dev];
}

