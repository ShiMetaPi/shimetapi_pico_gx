/*
 * Copyright (c) XMEDIA. All rights reserved.
 */


#ifndef __DEMO_COMMON_H__
#define __DEMO_COMMON_H__

//#include "type.h"
#include "common.h"
//#include "comm_isp.h"
//#include "comm_vi.h"
//#include "mipi.h"
//#include "sns_ctrl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define DEMO_PRINT(fmt, ...) \
do { \
    printf("%s-%d: "fmt"", __FUNCTION__, __LINE__, ##__VA_ARGS__); \
} while(0)

#define CHECK_RET_SUCCESS(ret) \
do { \
    if (ret != XMEDIA_SUCCESS) { \
        DEMO_PRINT("failed, ret=%#x\n", ret); \
    } \
} while(0)

typedef enum {
    SONY_IMX327_MIPI_2M_30FPS_12BIT,
    SONY_IMX327_MIPI_2M_30FPS_12BIT_WDR2TO1,
    SONY_IMX327_2L_MIPI_2M_30FPS_12BIT,
    SONY_IMX327_2L_MIPI_2M_30FPS_12BIT_WDR2TO1,
    SONY_IMX307_MIPI_2M_30FPS_12BIT,
    SONY_IMX307_MIPI_2M_30FPS_12BIT_WDR2TO1,
    SONY_IMX307_2L_MIPI_2M_30FPS_12BIT,
    SONY_IMX307_2L_MIPI_2M_30FPS_12BIT_WDR2TO1,
    SONY_IMX335_MIPI_5M_30FPS_12BIT,
    SONY_IMX335_MIPI_5M_30FPS_10BIT_WDR2TO1,
    SONY_IMX335_MIPI_4M_30FPS_12BIT,
    SONY_IMX335_MIPI_4M_30FPS_10BIT_WDR2TO1,
    SMART_SC4236_MIPI_3M_30FPS_10BIT,
    SMART_SC4236_MIPI_3M_20FPS_10BIT,
    SMART_SC2231_MIPI_2M_30FPS_10BIT,
    SOI_JXF37_MIPI_2M_30FPS_10BIT,
    SMART_SC2235_DC_2M_30FPS_10BIT,
    SMART_SC3235_MIPI_3M_30FPS_10BIT,
    GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT,
    GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT_FORCAR,
    GALAXYCORE_GC4653_2L_MIPI_4M_30FPS_10BIT,
    GALAXYCORE_GC4663_2L_MIPI_4M_30FPS_10BIT,
    GALAXYCORE_GC4663_2L_MIPI_4M_25FPS_10BIT_WDR2TO1,
    OMNIVISION_OS05A_MIPI_4M_30FPS_12BIT,
    OMNIVISION_OS05A_MIPI_5M_30FPS_12BIT,
    OMNIVISION_OS05A_MIPI_4M_30FPS_10BIT_WDR2TO1,
    SMART_SC530AI_2L_MIPI_5M_25FPS_10BIT,
    SMART_SC530AI_MIPI_5M_30FPS_10BIT,
    SMART_SC530AI_MIPI_5M_30FPS_10BIT_WDR2TO1,
    BT1120_2M_30FPS_8BIT,
    BT656_2M_30FPS_8BIT,
    BT601_2M_30FPS_8BIT,
    DEMO_SNS_TYPE_MAX,
} demo_sensor_type;

void demo_get_vi_dev_default_attr(demo_sensor_type sns, VI_DEV_ATTR_S *attr);

void demo_get_vi_pipe_default_attr(demo_sensor_type sns, VI_PIPE_ATTR_S *attr);

void demo_get_vi_chn_default_attr(demo_sensor_type sns, VI_CHN_ATTR_S *attr);

void demo_get_mipi_default_attr(demo_sensor_type sns, combo_dev_attr_t *attr);

void demo_get_isp_pub_default_attr(demo_sensor_type sns, ISP_PUB_ATTR_S *attr);

ISP_SNS_OBJ_S *demo_get_sns_obj(demo_sensor_type sns);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

