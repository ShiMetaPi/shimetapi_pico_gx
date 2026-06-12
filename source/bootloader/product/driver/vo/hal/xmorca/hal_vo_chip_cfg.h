 /* Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __HAL_VO_CHIP_CFG_H__
#define __HAL_VO_CHIP_CFG_H__

#include "drv_vo_comm.h"
#ifdef VO_BOOT
#include "xmedia_vo.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    VO_CHIP_INTF_TYPE_BT656 = 1 << 0,
    VO_CHIP_INTF_TYPE_BT1120 = 1 << 1,
    VO_CHIP_INTF_TYPE_LCD = 1 << 2,
    VO_CHIP_INTF_TYPE_LVDS = 1 << 3,
    VO_CHIP_INTF_TYPE_MIPI_DSI = 1 << 4,
    VO_CHIP_INTF_TYPE_MAX
} vo_chip_intf_type;

typedef enum {
    VO_CHIP_TYPE_XM7606V1,
    VO_CHIP_TYPE_MAX
} vo_chip_type;

typedef struct {
    xmedia_bool support;
    xmedia_bool support_hdr;
    xmedia_bool support_flip;
    xmedia_bool support_mirror;
    xmedia_bool support_zme;
    xmedia_bool support_dcmp;
    xmedia_bool support_region;
    xmedia_u32 region_num;
    xmedia_video_size max_read_size;
    xmedia_video_size min_out_size;
    xmedia_u32 support_max_bit_width;
    xmedia_u32 support_pixformat_num;
} vo_layer_cap;

typedef struct {
    xmedia_bool support;
    xmedia_u32 support_video_layer_num;
    xmedia_u32 support_gfx_layer_num;
    xmedia_u32 support_max_layer_num;
    xmedia_u32 support_intf_num;
    xmedia_u32 support_csc_num;
    xmedia_video_size max_size;
    xmedia_bool support_ink;
    xmedia_bool support_attach_layer;
    xmedia_u32 max_frame_rate;
} vo_dev_cap;

typedef struct {
    xmedia_u32 board_id;
    xmedia_u32 irq_num;
    xmedia_u32 irq_cpumask;
    xmedia_u32 chip_revision;
    xmedia_u32 chip_type;
    xmedia_char *chip_name;
    xmedia_char *irq_name;
} vo_base_cap;

xmedia_bool hal_vo_check_chip_type(vo_chip_type board_id);
xmedia_void hal_vo_init_chip_capaciblity(xmedia_void);
xmedia_u32 hal_vo_get_chip_revision(xmedia_void);
xmedia_s32 hal_vo_check_layer_cap(xmedia_vo_layer layer, xmedia_u32 max_width, xmedia_u32 max_height,
    xmedia_vo_layer_config *layer_config);
xmedia_s32 hal_vo_check_dev_intf(xmedia_vo_dev dev, xmedia_vo_dev_config *config, xmedia_video_size *dev_size);
xmedia_s32 hal_vo_check_clk(xmedia_vo_dev dev, xmedia_vo_dev_config *config, xmedia_u32 clk);
xmedia_s32 hal_vo_check_dev_size(xmedia_vo_dev dev, xmedia_u32 width, xmedia_u32 height);
xmedia_s32 hal_vo_check_dev_csc_cap(xmedia_vo_dev dev, xmedia_vo_intf_config *intf_config,
    xmedia_vo_dev_csc *dev_csc);
xmedia_s32 hal_vo_check_dev_gamma_cap(xmedia_vo_dev dev, xmedia_vo_intf_config *intf_config);
xmedia_s32 hal_vo_check_layer_frame(xmedia_vo_layer layer, xmedia_video_frame_info *frame_info);
vo_base_cap *hal_vo_get_chip_capacity(xmedia_void);
vo_layer_cap* hal_get_layer_chip_capacity(xmedia_vo_layer layer);
vo_dev_cap* hal_get_dev_chip_capacity(xmedia_vo_dev dev);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_VO__CHIP_CFG_H__ */
