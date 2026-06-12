#ifndef _DRV_ISP_H_
#define _DRV_ISP_H_

#include "xmedia_isp.h"

typedef enum {
    ISP_IRQ_TYPE_PT_FRAME_START,
    ISP_IRQ_TYPE_PT_FRAME_END,
    ISP_IRQ_TYPE_FE_FRAME_START,
    ISP_IRQ_TYPE_FE_FRAME_END,
    ISP_IRQ_TYPE_BE_FRAME_START,
    ISP_IRQ_TYPE_BE_FRAME_END,
    ISP_IRQ_TYPE_MAX,
} isp_irq_type;

xmedia_s32 drv_sensor_init(xmedia_u32 pipe, xmedia_sensor_config *cfg);
xmedia_s32 drv_sensor_exit(xmedia_u32 pipe);
xmedia_s32 drv_sensor_resume(xmedia_u32 pipe);
xmedia_s32 drv_sensor_start(xmedia_u32 pipe);
xmedia_s32 drv_sensor_stop(xmedia_u32 pipe);
xmedia_s32 drv_sensor_set_mipi_lanes(xmedia_u32 pipe, const xmedia_sensor_mipi_lanes mipi_lanes);
xmedia_s32 drv_sensor_set_attr(xmedia_u32 pipe, const xmedia_sensor_attr *sns_attr);
xmedia_s32 drv_sensor_set_init_param(xmedia_u32 pipe, const xmedia_sensor_init_param *init_param);
xmedia_s32 drv_isp_init(xmedia_u32 pipe, xmedia_isp_config *cfg);
xmedia_s32 drv_isp_exit(xmedia_u32 pipe);
xmedia_s32 drv_isp_start(xmedia_u32 pipe);
xmedia_s32 drv_isp_stop(xmedia_u32 pipe);
xmedia_s32 drv_isp_notifier(xmedia_u32 pipe, isp_irq_type irq_type);
xmedia_s32 drv_isp_process(xmedia_u32 pipe);
xmedia_s32 drv_isp_config(xmedia_u32 pipe);


#endif

