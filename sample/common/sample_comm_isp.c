/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <fcntl.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdint.h>
#include "sample_comm.h"
#include "sample_comm_isp.h"
#include "sample_comm_sys.h"
#include "xmedia_mmz.h"
#include "xmedia_npu.h"
#include "xmedia_cl.h"

#define SAMPLE_CHECK_RET_RETURN(ret)                                                                                   \
    do {                                                                                                               \
        if (ret != XMEDIA_SUCCESS) {                                                                                   \
            SAMPLE_PRT("%s%d-call func failed!\n", __FUNCTION__, __LINE__);                                            \
            return ret;                                                                                                \
        }                                                                                                              \
    } while (0)

#define SAMPLE_CHECK_RET_GOTO(ret, tag)                                                                                \
    do {                                                                                                               \
        if (ret != XMEDIA_SUCCESS) {                                                                                   \
            SAMPLE_PRT("%s%d-call func failed!\n", __FUNCTION__, __LINE__);                                            \
            goto tag;                                                                                                  \
        }                                                                                                              \
    } while (0)

#define SENSOR_CRL_BASE_ADDR 0x120100F0

static xmedia_u64 g_isp_fpn_buff[ISP_MAX_PIPE_NUM] = { 0 };
static xmedia_bool g_isp_task_exit[ISP_MAX_PIPE_NUM];
static pthread_t   g_isp_task_id[ISP_MAX_PIPE_NUM] = { 0 };
static xmedia_bool g_multi_pipe_mode = XMEDIA_FALSE;
static xmedia_s32 g_multi_pipe_id[2];   // [0]-master, [1]-slave

typedef struct {
    xmedia_s32 pipe;
    xmedia_video_wdr_mode wdr_mode;
    xmedia_s32 sensor_type;
} sample_comm_isp_task_param;

typedef struct {
    xmedia_u64 last_exp;
    xmedia_u32 last_fps;
    xmedia_u32 last_level;
    xmedia_u32 stable_count;
} sampel_comm_isp_dynamic_fps;


#define AINR_PQ_BIN_DIR     "../scene_auto"
#define AINR_MODEL_DIR      "../../tools/linux/pq_board/arm-gcc12.2.0-linux-uclibceabi/configs"

sample_isp_sensor_effect_list g_ainr_pqbin_list[AINR_LIST_NUM] = {
    {SMART_SC235HAI_MIPI_2M_2LANE_30FPS_10BIT,      ""AINR_PQ_BIN_DIR"/sc235hai/sc235hai_linear_isp_vpss_ainr.bin"},
    {SMART_SC231HAI_MIPI_2M_2LANE_30FPS_10BIT,      ""AINR_PQ_BIN_DIR"/sc231hai/sc231hai_linear_isp_vpss_ainr.bin"},
    {SMART_SC465SL_MIPI_4M_2LANE_30FPS_12BIT,       ""AINR_PQ_BIN_DIR"/sc465sl/sc465sl_linear_isp_vpss_ainr.bin"},
    {SMART_SC465SL_MIPI_4M_4LANE_30FPS_12BIT,       ""AINR_PQ_BIN_DIR"/sc465sl/sc465sl_linear_isp_vpss_ainr.bin"},
    {SMART_SC485SL_MIPI_4M_30FPS_2LANE_12BIT,       ""AINR_PQ_BIN_DIR"/sc485sl/sc485sl_linear_isp_vpss_ainr.bin"},
    {SMART_SC485SL_MIPI_4M_2688_1520_30FPS_4LANE_12BIT, ""AINR_PQ_BIN_DIR"/sc485sl/sc485sl_linear_isp_vpss_ainr.bin"},
    {SMART_SC4336P_MIPI_4M_30FPS_10BIT,             ""AINR_PQ_BIN_DIR"/sc4336p/sc4336p_linear_isp_vpss_ainr.bin"},
    {SMART_SC431HAI_MIPI_4M_30FPS_10BIT,            ""AINR_PQ_BIN_DIR"/sc431hai/sc431hai_linear_isp_vpss_ainr.bin"},
    {CVSENS_CV4003_MIPI_4M_30FPS_10BIT,             ""AINR_PQ_BIN_DIR"/cv4003/sc485sl_linear_isp_vpss_ainr.bin"},

    {SENSOR_TYPE_INVALID,  NULL},
};

sample_isp_sensor_effect_list g_ainr_model_list[AINR_LIST_NUM] = {
    {SMART_SC235HAI_MIPI_2M_2LANE_30FPS_10BIT,      ""AINR_MODEL_DIR"/sc235hai/neuron_network.xmm"},
    {SMART_SC231HAI_MIPI_2M_2LANE_30FPS_10BIT,      ""AINR_MODEL_DIR"/sc231hai/neuron_network.xmm"},
    {SMART_SC465SL_MIPI_4M_2LANE_30FPS_12BIT,       ""AINR_MODEL_DIR"/sc465sl/neuron_network.xmm"},
    {SMART_SC465SL_MIPI_4M_4LANE_30FPS_12BIT,       ""AINR_MODEL_DIR"/sc465sl/neuron_network.xmm"},
    {SMART_SC485SL_MIPI_4M_30FPS_2LANE_12BIT,       ""AINR_MODEL_DIR"/sc485sl/neuron_network_2560_1440.xmm"},
    {SMART_SC485SL_MIPI_4M_2688_1520_30FPS_4LANE_12BIT, ""AINR_MODEL_DIR"/sc485sl/neuron_network_2688_1520.xmm"},
    {SMART_SC4336P_MIPI_4M_30FPS_10BIT,             ""AINR_MODEL_DIR"/sc4336p/neuron_network.xmm"},
    {SMART_SC431HAI_MIPI_4M_30FPS_10BIT,            ""AINR_MODEL_DIR"/sc431hai/neuron_network.xmm"},
    {CVSENS_CV4003_MIPI_4M_30FPS_10BIT,             ""AINR_MODEL_DIR"/cv4003/neuron_network_2560_1440.xmm"},

    {SENSOR_TYPE_INVALID,  NULL},
};

xmedia_s32 sample_comm_isp_sensor_init(xmedia_s32 pipe, sample_isp_info *isp_info)
{
    xmedia_s32 ret;
    xmedia_sensor_mipi_lanes mipi_lanes = 0;
    xmedia_sensor_attr       attr       = {0};

    CHECK_NULL_PTR(isp_info);

    if (XMEDIA_FALSE == isp_info->isp_pipe_en || -1 == pipe) {
        return XMEDIA_FAILURE;
    }

    switch (isp_info->sensor_type) {
        case SMART_SC8238_MIPI_2M_30FPS_10BIT:
        case SMART_SC8238_MIPI_2M_30FPS_10BIT_WDR2TO1:
        case SMART_SC8238_MIPI_3M_30FPS_10BIT:
        case SMART_SC8238_MIPI_3M_30FPS_10BIT_WDR2TO1:
        case SMART_SC8238_MIPI_4M_30FPS_10BIT:
        case SMART_SC8238_MIPI_4M_30FPS_10BIT_WDR2TO1:
        case SMART_SC8238_MIPI_5M_30FPS_10BIT:
        case SMART_SC8238_MIPI_5M_30FPS_10BIT_WDR2TO1:
        case SMART_SC8238_MIPI_8M_20FPS_10BIT:
        case SMART_SC8238_MIPI_8M_20FPS_10BIT_WDR2TO1:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_4L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc8238);
            break;
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT:
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT_WDR2TO1:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, gc2053);
            break;
        case GALAXYCORE_GC20C3_MIPI_2M_30FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, gc20c3);
            break;
        case GALAXYCORE_GC2083_MIPI_2M_30FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, gc2083);
            break;
        case OMNIVISION_OS08A20_MIPI_2M_30FPS_10BIT:
        case OMNIVISION_OS08A20_MIPI_2M_30FPS_10BIT_WDR2TO1:
        case OMNIVISION_OS08A20_MIPI_4M_30FPS_10BIT:
        case OMNIVISION_OS08A20_MIPI_4M_30FPS_10BIT_WDR2TO1:
        case OMNIVISION_OS08A20_MIPI_5M_30FPS_10BIT:
        case OMNIVISION_OS08A20_MIPI_5M_30FPS_10BIT_WDR2TO1:
        case OMNIVISION_OS08A20_MIPI_8M_20FPS_10BIT:
        case OMNIVISION_OS08A20_MIPI_8M_20FPS_10BIT_WDR2TO1:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_4L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, os08a20);
            break;
        case OMNIVISION_OS05A10_MIPI_4M_30FPS_12BIT:
        case OMNIVISION_OS05A10_MIPI_4M_30FPS_10BIT_WDR2TO1:
        case OMNIVISION_OS05A10_MIPI_5M_30FPS_12BIT:
        case OMNIVISION_OS05A10_MIPI_5M_30FPS_10BIT_WDR2TO1:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_4L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, os05a10);
            break;
        case SMART_SC850SL_MIPI_8M_20FPS_12BIT:
        case SMART_SC850SL_MIPI_8M_20FPS_10BIT_WDR2TO1:
        case SMART_SC850SL_MIPI_5M_30FPS_12BIT:
        case SMART_SC850SL_MIPI_5M_20FPS_10BIT_WDR2TO1:
        case SMART_SC850SL_MIPI_4M_30FPS_12BIT:
        case SMART_SC850SL_MIPI_4M_20FPS_10BIT_WDR2TO1:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_4L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc850sl);
            break;
        case SMART_SC465SL_MIPI_4M_4LANE_30FPS_12BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_4L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc465sl);
            break;
        case SMART_SC465SL_MIPI_4M_2LANE_30FPS_12BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc465sl);
            break;
        case SMART_SC465SL_MIPI_4M_30FPS_4LANE_10BIT_WDR2TO1:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_4L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc465sl);
            break;
        case CVSENS_CV4003_MIPI_4M_30FPS_10BIT:
        case CVSENS_CV4003_MIPI_1M_120FPS_10BIT_PREROLL_READBACK:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, cv4003);
            break;
        case CVSENS_CV2005_MIPI_2M_30FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, cv2005);
            break;
        case SMART_SC231HAI_MIPI_2M_2LANE_30FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc231hai);
            break;
        case IMAGEDESIGN_MIS20S1_MIPI_2M_2LANE_30FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, mis20s1);
            break;
        case SMART_SC2337P_MIPI_2M_30FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc2337p);
            break;
        case SMART_SC235HAI_MIPI_2M_2LANE_30FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc235hai);
            break;
        case SMART_SC285SL_MIPI_2M_4LANE_30FPS_12BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_4L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc285sl);
            break;
        case SMART_SC285SL_MIPI_2M_2LANE_30FPS_12BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc285sl);
            break;
        case SMART_SC485SL_MIPI_4M_2688_1520_30FPS_4LANE_12BIT:
        case SMART_SC485SL_MIPI_4M_2560_1440_30FPS_4LANE_12BIT:
        case SMART_SC485SL_MIPI_4M_30FPS_4LANE_10BIT_WDR2TO1:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_4L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc485sl);
            break;
        case SMART_SC485SL_MIPI_4M_30FPS_2LANE_12BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc485sl);
            break;
        case IMAGEDESIGN_MIS40H1_MIPI_4M_2LANE_30FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, mis40h1);
            break;
        case SMART_SC530AI_MIPI_5M_4L_30FPS_10BIT:
        case SMART_SC530AI_MIPI_5M_4L_30FPS_10BIT_WDR2TO1:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_4L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc530ai);
            break;
        case SMART_SC533HAI_MIPI_5M_2L_30FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc533hai);
            break;
        case SMART_SC530AI_MIPI_5M_2L_30FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc530ai);
            break;
        case SMART_SC200AI_MIPI_2M_30FPS_10BIT:
        case SMART_SC200AI_MIPI_2M_30FPS_10BIT_WDR2TO1:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc200ai);
            break;
        case SMART_SC500AI_MIPI_5M_30FPS_10BIT:
        case SMART_SC500AI_MIPI_5M_30FPS_10BIT_WDR2TO1:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_4L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc500ai);
            break;
        case SMART_SC450AI_MIPI_4M_30FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc450ai);
            break;
        case SMART_SC3338_MIPI_3M_30FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc3338);
            break;
        case OMNIVISION_OS04A10_MIPI_4M_30FPS_12BIT:
        case OMNIVISION_OS04A10_MIPI_4M_30FPS_10BIT_WDR2TO1:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_4L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, os04a10);
            break;
        case OMNIVISION_OS04D10_MIPI_4M_30FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, os04d10);
            break;
        case OMNIVISION_OS04C10_MIPI_4M_30FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, os04c10);
            break;
        case OMNIVISION_OS04J10_MIPI_4M_30FPS_12BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_4L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, os04j10);
            break;
        case GALAXYCORE_GC4663_MIPI_4M_30FPS_10BIT:
        case GALAXYCORE_GC4663_MIPI_4M_25FPS_10BIT_WDR2TO1:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, gc4663);
            break;
        case GALAXYCORE_GC4653_MIPI_4M_30FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, gc4653);
            break;
        case GALAXYCORE_OV4689_MIPI_4M_30FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_4L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, ov4689);
            break;
        case GALAXYCORE_GC8613_MIPI_8M_20FPS_14BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_4L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, gc8613);
            break;
        case GALAXYCORE_GC8613_MIPI_8M_20FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, gc8613);
            break;
        case OMNIVISION_OS02K10_MIPI_2M_30FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, os02k10);
            break;
        case OMNIVISION_OS02K10_MIPI_2M_30FPS_12BIT_BUILT_IN:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_4L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, os02k10);
            break;
        case SONY_IMX415_MIPI_8M_20FPS_10BIT:
        case SONY_IMX415_MIPI_8M_20FPS_10BIT_WDR2TO1:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_4L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, imx415);
            break;
        case SONY_IMX678_MIPI_8M_20FPS_12BIT:
        case SONY_IMX678_MIPI_8M_20FPS_10BIT_WDR2TO1:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_4L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, imx678);
            break;
        case SONY_IMX179_MIPI_8M_20FPS_10BIT:
        case SONY_IMX179_MIPI_640_480_120FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_4L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, imx179);
            break;
        case SONY_IMX307_MIPI_2M_2L_30FPS_10BIT:
        case SONY_IMX307_MIPI_2M_2L_30FPS_10BIT_WDR2TO1:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, imx307);
            break;
        case GALAXYCORE_GC5603_MIPI_5M_30FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, gc5603);
            break;
        case GALAXYCORE_GC6603_MIPI_5M_30FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, gc6603);
            break;
        case OMNIVISION_OX03F10_MIPI_1M_10FPS_14BIT_BUILT_IN:
        case OMNIVISION_OX03F10_MIPI_3M_60FPS_14BIT_BUILT_IN:
        case OMNIVISION_OX03F10_MIPI_3M_60FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_4L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, ox03f10);
            break;
        case SMART_SC4336_MIPI_4M_30FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc4336);
            break;
        case SMART_SC4336P_MIPI_4M_30FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc4336p);
            break;
        case SMART_SC431HAI_MIPI_4M_30FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc431hai);
            break;
        case SMART_SC435HAI_MIPI_4M_30FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc435hai);
            break;
        case OMNIVISION_OX05B1S_MIPI_1M_30FPS_10BIT:
        case OMNIVISION_OX05B1S_MIPI_5M_60FPS_10BIT:
            mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_4L;
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, ox05b1s);
            break;
        default:
            SAMPLE_ERR("sensor %d type not supported\n", isp_info->sensor_type);
            break;
    }

    ret = xmedia_sensor_init(pipe, &isp_info->sns_cfg);
    CHECK_RET(ret, "sensor init");

    ret = xmedia_sensor_set_mipi_lanes(pipe, mipi_lanes);
    CHECK_RET(ret, "sensor set mipi lanes");

    if (isp_info->i2c_addr_en == XMEDIA_TRUE) {
        ret= xmedia_sensor_set_dev_addr(pipe, isp_info->i2c_addr);
        CHECK_RET(ret, "isp set i2c addr");
    }

    attr.width    = isp_info->isp_config.size.width;
    attr.height   = isp_info->isp_config.size.height;
    attr.wdr_mode = isp_info->isp_config.wdr_mode;

    ret = xmedia_sensor_set_attr(pipe, &attr);
    CHECK_RET(ret, "sensor set attr");

    SAMPLE_PRT("sensor i2c_id = %u, clk_id = %u\n", isp_info->sns_cfg.comm_bus.i2c_dev, isp_info->sns_cfg.ctrl_sig.clk_ch);

    ret = xmedia_sensor_set_work_mode(pipe, isp_info->sns_work_mode);
    CHECK_RET(ret, "sensor set work mode");

    if (isp_info->sns_work_mode == XMEDIA_SENSOR_WORK_MODE_SLAVE) {
        ret= xmedia_sensor_set_trig_signal(pipe, &isp_info->signal);
        CHECK_RET(ret, "isp set trig signal");
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_isp_sensor_exit(xmedia_s32 pipe, sample_comm_sensor_type sensor_type)
{
    xmedia_s32 ret;

    if (-1 == pipe) {
        return XMEDIA_FAILURE;
    }

    ret = xmedia_sensor_exit(pipe);
    CHECK_RET(ret, "sensor exit");

    switch (sensor_type) {
        case SMART_SC8238_MIPI_2M_30FPS_10BIT:
        case SMART_SC8238_MIPI_2M_30FPS_10BIT_WDR2TO1:
        case SMART_SC8238_MIPI_3M_30FPS_10BIT:
        case SMART_SC8238_MIPI_3M_30FPS_10BIT_WDR2TO1:
        case SMART_SC8238_MIPI_4M_30FPS_10BIT:
        case SMART_SC8238_MIPI_4M_30FPS_10BIT_WDR2TO1:
        case SMART_SC8238_MIPI_5M_30FPS_10BIT:
        case SMART_SC8238_MIPI_5M_30FPS_10BIT_WDR2TO1:

        case SMART_SC8238_MIPI_8M_20FPS_10BIT:
        case SMART_SC8238_MIPI_8M_20FPS_10BIT_WDR2TO1:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, sc8238);
            break;
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT:
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT_WDR2TO1:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, gc2053);
            break;
        case GALAXYCORE_GC20C3_MIPI_2M_30FPS_10BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, gc20c3);
            break;
        case GALAXYCORE_GC2083_MIPI_2M_30FPS_10BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, gc2083);
            break;
        case OMNIVISION_OS08A20_MIPI_2M_30FPS_10BIT:
        case OMNIVISION_OS08A20_MIPI_2M_30FPS_10BIT_WDR2TO1:
        case OMNIVISION_OS08A20_MIPI_4M_30FPS_10BIT:
        case OMNIVISION_OS08A20_MIPI_4M_30FPS_10BIT_WDR2TO1:
        case OMNIVISION_OS08A20_MIPI_5M_30FPS_10BIT:
        case OMNIVISION_OS08A20_MIPI_5M_30FPS_10BIT_WDR2TO1:
        case OMNIVISION_OS08A20_MIPI_8M_20FPS_10BIT:
        case OMNIVISION_OS08A20_MIPI_8M_20FPS_10BIT_WDR2TO1:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, os08a20);
            break;
        case OMNIVISION_OS05A10_MIPI_4M_30FPS_12BIT:
        case OMNIVISION_OS05A10_MIPI_4M_30FPS_10BIT_WDR2TO1:
        case OMNIVISION_OS05A10_MIPI_5M_30FPS_12BIT:
        case OMNIVISION_OS05A10_MIPI_5M_30FPS_10BIT_WDR2TO1:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, os05a10);
            break;
        case SMART_SC850SL_MIPI_8M_20FPS_12BIT:
        case SMART_SC850SL_MIPI_8M_20FPS_10BIT_WDR2TO1:
        case SMART_SC850SL_MIPI_5M_30FPS_12BIT:
        case SMART_SC850SL_MIPI_5M_20FPS_10BIT_WDR2TO1:
        case SMART_SC850SL_MIPI_4M_30FPS_12BIT:
        case SMART_SC850SL_MIPI_4M_20FPS_10BIT_WDR2TO1:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, sc850sl);
            break;
        case SMART_SC231HAI_MIPI_2M_2LANE_30FPS_10BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, sc231hai);
            break;
        case IMAGEDESIGN_MIS20S1_MIPI_2M_2LANE_30FPS_10BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, mis20s1);
            break;
        case SMART_SC2337P_MIPI_2M_30FPS_10BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, sc2337p);
            break;
        case SMART_SC235HAI_MIPI_2M_2LANE_30FPS_10BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, sc235hai);
            break;
        case SMART_SC285SL_MIPI_2M_4LANE_30FPS_12BIT:
        case SMART_SC285SL_MIPI_2M_2LANE_30FPS_12BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, sc285sl);
            break;
        case SMART_SC465SL_MIPI_4M_4LANE_30FPS_12BIT:
        case SMART_SC465SL_MIPI_4M_2LANE_30FPS_12BIT:
        case SMART_SC465SL_MIPI_4M_30FPS_4LANE_10BIT_WDR2TO1:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, sc465sl);
            break;
        case CVSENS_CV4003_MIPI_4M_30FPS_10BIT:
        case CVSENS_CV4003_MIPI_1M_120FPS_10BIT_PREROLL_READBACK:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, cv4003);
            break;
        case CVSENS_CV2005_MIPI_2M_30FPS_10BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, cv2005);
            break;
        case SMART_SC485SL_MIPI_4M_2688_1520_30FPS_4LANE_12BIT:
        case SMART_SC485SL_MIPI_4M_2560_1440_30FPS_4LANE_12BIT:
        case SMART_SC485SL_MIPI_4M_30FPS_2LANE_12BIT:
        case SMART_SC485SL_MIPI_4M_30FPS_4LANE_10BIT_WDR2TO1:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, sc485sl);
            break;
        case IMAGEDESIGN_MIS40H1_MIPI_4M_2LANE_30FPS_10BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, mis40h1);
            break;
        case SMART_SC530AI_MIPI_5M_4L_30FPS_10BIT:
        case SMART_SC530AI_MIPI_5M_2L_30FPS_10BIT:
        case SMART_SC530AI_MIPI_5M_4L_30FPS_10BIT_WDR2TO1:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, sc530ai);
            break;
        case SMART_SC533HAI_MIPI_5M_2L_30FPS_10BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, sc533hai);
            break;
        case SMART_SC200AI_MIPI_2M_30FPS_10BIT:
        case SMART_SC200AI_MIPI_2M_30FPS_10BIT_WDR2TO1:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, sc200ai);
            break;
        case SMART_SC500AI_MIPI_5M_30FPS_10BIT:
        case SMART_SC500AI_MIPI_5M_30FPS_10BIT_WDR2TO1:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, sc500ai);
            break;
        case SMART_SC450AI_MIPI_4M_30FPS_10BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, sc450ai);
            break;
        case SMART_SC3338_MIPI_3M_30FPS_10BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, sc3338);
            break;
        case OMNIVISION_OS04C10_MIPI_4M_30FPS_10BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, os04c10);
            break;
        case OMNIVISION_OS04J10_MIPI_4M_30FPS_12BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, os04j10);
            break;
        case GALAXYCORE_GC4663_MIPI_4M_30FPS_10BIT:
        case GALAXYCORE_GC4663_MIPI_4M_25FPS_10BIT_WDR2TO1:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, gc4663);
            break;
        case GALAXYCORE_GC4653_MIPI_4M_30FPS_10BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, gc4653);
            break;
        case GALAXYCORE_OV4689_MIPI_4M_30FPS_10BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, ov4689);
            break;
        case GALAXYCORE_GC8613_MIPI_8M_20FPS_14BIT:
        case GALAXYCORE_GC8613_MIPI_8M_20FPS_10BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, gc8613);
            break;
        case OMNIVISION_OS02K10_MIPI_2M_30FPS_10BIT:
        case OMNIVISION_OS02K10_MIPI_2M_30FPS_12BIT_BUILT_IN:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, os02k10);
            break;
        case SONY_IMX415_MIPI_8M_20FPS_10BIT:
        case SONY_IMX415_MIPI_8M_20FPS_10BIT_WDR2TO1:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, imx415);
            break;
        case SONY_IMX179_MIPI_8M_20FPS_10BIT:
        case SONY_IMX179_MIPI_640_480_120FPS_10BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, imx179);
            break;
        case SONY_IMX678_MIPI_8M_20FPS_12BIT:
        case SONY_IMX678_MIPI_8M_20FPS_10BIT_WDR2TO1:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, imx678);
            break;
        case SONY_IMX307_MIPI_2M_2L_30FPS_10BIT:
        case SONY_IMX307_MIPI_2M_2L_30FPS_10BIT_WDR2TO1:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, imx307);
            break;
        case GALAXYCORE_GC5603_MIPI_5M_30FPS_10BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, gc5603);
            break;
        case GALAXYCORE_GC6603_MIPI_5M_30FPS_10BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, gc6603);
            break;
        case OMNIVISION_OX03F10_MIPI_1M_10FPS_14BIT_BUILT_IN:
        case OMNIVISION_OX03F10_MIPI_3M_60FPS_14BIT_BUILT_IN:
        case OMNIVISION_OX03F10_MIPI_3M_60FPS_10BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, ox03f10);
            break;
        case SMART_SC4336_MIPI_4M_30FPS_10BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, sc4336);
            break;
        case SMART_SC4336P_MIPI_4M_30FPS_10BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, sc4336p);
            break;
        case SMART_SC431HAI_MIPI_4M_30FPS_10BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, sc431hai);
            break;
        case SMART_SC435HAI_MIPI_4M_30FPS_10BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, sc435hai);
            break;
        case OMNIVISION_OS04A10_MIPI_4M_30FPS_12BIT:
        case OMNIVISION_OS04A10_MIPI_4M_30FPS_10BIT_WDR2TO1:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, os04a10);
            break;
        case OMNIVISION_OS04D10_MIPI_4M_30FPS_10BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, os04d10);
            break;
        case OMNIVISION_OX05B1S_MIPI_1M_30FPS_10BIT:
        case OMNIVISION_OX05B1S_MIPI_5M_60FPS_10BIT:
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, ox05b1s);
            break;
        default:
            SAMPLE_ERR("sensor %d type not supported\n", sensor_type);
            break;
    }
    return XMEDIA_SUCCESS;
}

xmedia_bool sample_comm_isp_is_wdr_mode(sample_comm_sensor_type sensor_type)
{
    if (sensor_type == SONY_IMX290_MIPI_2M_30FPS_10BIT_WDR2TO1 ||
        sensor_type == SONY_IMX307_MIPI_2M_2L_30FPS_10BIT_WDR2TO1 ||
        sensor_type == SONY_IMX678_MIPI_8M_20FPS_10BIT_WDR2TO1 ||
        sensor_type == GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT_WDR2TO1 ||
        sensor_type == GALAXYCORE_GC4663_MIPI_4M_25FPS_10BIT_WDR2TO1 ||
        sensor_type == SMART_SC8238_MIPI_2M_30FPS_10BIT_WDR2TO1 ||
        sensor_type == SMART_SC8238_MIPI_3M_30FPS_10BIT_WDR2TO1 ||
        sensor_type == SMART_SC8238_MIPI_4M_30FPS_10BIT_WDR2TO1 ||
        sensor_type == SMART_SC8238_MIPI_5M_30FPS_10BIT_WDR2TO1 ||
        sensor_type == SMART_SC8238_MIPI_8M_20FPS_10BIT_WDR2TO1 ||
        sensor_type == OMNIVISION_OS08A20_MIPI_2M_30FPS_10BIT_WDR2TO1 ||
        sensor_type == OMNIVISION_OS08A20_MIPI_4M_30FPS_10BIT_WDR2TO1 ||
        sensor_type == OMNIVISION_OS08A20_MIPI_5M_30FPS_10BIT_WDR2TO1 ||
        sensor_type == OMNIVISION_OS08A20_MIPI_8M_20FPS_10BIT_WDR2TO1 ||
        sensor_type == OMNIVISION_OS05A10_MIPI_4M_30FPS_10BIT_WDR2TO1 ||
        sensor_type == OMNIVISION_OS05A10_MIPI_5M_30FPS_10BIT_WDR2TO1 ||
        sensor_type == SMART_SC850SL_MIPI_8M_20FPS_10BIT_WDR2TO1 ||
        sensor_type == SMART_SC850SL_MIPI_5M_20FPS_10BIT_WDR2TO1 ||
        sensor_type == SMART_SC850SL_MIPI_4M_20FPS_10BIT_WDR2TO1 ||
        sensor_type == SMART_SC200AI_MIPI_2M_30FPS_10BIT_WDR2TO1 ||
        sensor_type == SMART_SC500AI_MIPI_5M_30FPS_10BIT_WDR2TO1 ||
        sensor_type == SMART_SC530AI_MIPI_5M_4L_30FPS_10BIT_WDR2TO1 ||
        sensor_type == OMNIVISION_OS04A10_MIPI_4M_30FPS_10BIT_WDR2TO1) {
        return XMEDIA_TRUE;
    }

    return XMEDIA_FALSE;
}

xmedia_void sample_comm_isp_set_multi_pipe_mode(xmedia_bool multi_pipe_mode,
    xmedia_s32 master_pipe_id, xmedia_s32 slave_pipe_id)
{
    g_multi_pipe_mode = multi_pipe_mode;
    g_multi_pipe_id[0] = master_pipe_id;
    g_multi_pipe_id[1] = slave_pipe_id;
}

xmedia_void sample_comm_isp_get_sensor_info(sample_isp_param *isp_param, sample_vi_config *vi_config)
{
    xmedia_s32 i;
    xmedia_s32 dev;
    xmedia_s32 pipe;

    for (dev = 0; dev < VI_MAX_DEV_NUM; dev++) {
        if (vi_config->dev_info[dev].dev_en == XMEDIA_FALSE) {
            continue;
        }

        if (dev == 0) {
            vi_config->dev_info[dev].i2c_id = 1;
            vi_config->dev_info[dev].sns_clk_id = 1;
        } else if (dev == 1) {
            vi_config->dev_info[dev].i2c_id = 1;
            vi_config->dev_info[dev].sns_clk_id = 1;
        } else if (dev == 2) {
            vi_config->dev_info[dev].i2c_id = 5;
            vi_config->dev_info[dev].sns_clk_id = 2;
        } else if (dev == 3) {
            vi_config->dev_info[dev].i2c_id = 6;
            vi_config->dev_info[dev].sns_clk_id = 3;
        } else {
            SAMPLE_ERR("current dev %d not support\n", dev);
        }

        for (i = 0; i < VI_DEV_BIND_PIPE_NUM; i++) {
            pipe = vi_config->dev_bind_pipe[dev].pipe[i];
            if (pipe == -1) {
                continue;
            }

            if (isp_param->isp_info[pipe].isp_pipe_en == XMEDIA_FALSE) {
                continue;
            }

            if (isp_param->isp_info[pipe].isp_sensor_en == XMEDIA_FALSE) {
                continue;
            }

            isp_param->isp_info[pipe].sns_cfg.ctrl_sig.clk_ch = vi_config->dev_info[dev].sns_clk_id;
            isp_param->isp_info[pipe].sns_cfg.ctrl_sig.rst_ch = vi_config->dev_info[dev].sns_clk_id;
            isp_param->isp_info[pipe].sns_cfg.comm_bus.type = XMEDIA_SENSOR_BUS_TYPE_I2C;
            isp_param->isp_info[pipe].sns_cfg.comm_bus.i2c_dev = vi_config->dev_info[dev].i2c_id;
            if (isp_param->isp_info[pipe].serdes_enable != XMEDIA_TRUE) {
                isp_param->isp_info[pipe].i2c_addr_en = vi_config->dev_info[dev].dev_i2c_en;
                isp_param->isp_info[pipe].i2c_addr = vi_config->dev_info[dev].dev_i2c_addr;
            }
        }
    }
}

static xmedia_void sample_comm_isp_set_dynamic_fps(xmedia_s32 pipe, xmedia_u32 iso, xmedia_u32 exp_time, xmedia_s32 sensor_type)
{
    static sampel_comm_isp_dynamic_fps dynamic_fps[ISP_MAX_PIPE_NUM] = {0};
    xmedia_u64 exp = (xmedia_u64)iso * exp_time;
    xmedia_u64 *ptr_threshold = NULL;
    xmedia_float fps;
    xmedia_isp_attr isp_attr = {0};
    xmedia_u32 exp_level = 0;
    xmedia_u32 fps_max_num = 6;

    xmedia_u64 exp_ltoh_threshold[16] = {5000000,10000000,20000000,40000000,80000000,80000000};
    xmedia_u64 exp_htol_threshold[16] = {5000000,10000000,20000000,40000000,80000000,80000000};
    xmedia_float fps_threshold[16] = {30.0, 20.0, 12.0, 10.0, 10.0,10.0};

    if (SMART_SC200AI_MIPI_2M_30FPS_10BIT == sensor_type) {
        xmedia_u64 exp_ltoh_threshold_sns[6] = {58000000, 100000000, 420000000, 900000000,3000000000,4800000000};
        xmedia_u64 exp_htol_threshold_sns[6] = {45000000,  58000000, 110000000, 420000000, 900000000,3000000000};
        xmedia_float fps_threshold_sns[6] = {30.0, 20.0, 15.0, 10.0, 7.0, 3.0};
        fps_max_num = 6;

        memcpy(exp_ltoh_threshold, exp_ltoh_threshold_sns, sizeof(xmedia_u64)*fps_max_num);
        memcpy(exp_htol_threshold, exp_htol_threshold_sns, sizeof(xmedia_u64)*fps_max_num);
        memcpy(fps_threshold, fps_threshold_sns, sizeof(xmedia_float)*fps_max_num);
    } else if (SMART_SC450AI_MIPI_4M_30FPS_10BIT == sensor_type) {
            xmedia_u64 exp_ltoh_threshold_sns[6] = {58000000, 100000000, 420000000, 900000000,3000000000,4800000000};
            xmedia_u64 exp_htol_threshold_sns[6] = {45000000,   58000000, 110000000, 420000000, 900000000,3000000000};
            xmedia_float fps_threshold_sns[6] = {30.0, 20.0, 15.0, 10.0, 7.0, 6.0};
            fps_max_num = 6;

            memcpy(exp_ltoh_threshold, exp_ltoh_threshold_sns, sizeof(xmedia_u64)*fps_max_num);
            memcpy(exp_htol_threshold, exp_htol_threshold_sns, sizeof(xmedia_u64)*fps_max_num);
            memcpy(fps_threshold, fps_threshold_sns, sizeof(xmedia_float)*fps_max_num);
    } else if (GALAXYCORE_GC8613_MIPI_8M_20FPS_14BIT == sensor_type){
            xmedia_float fps_threshold_sns[6] = {30.0, 20.0, 20.0, 20.0, 20.0, 20.0};
            fps_max_num = 6;
            memcpy(fps_threshold, fps_threshold_sns, sizeof(xmedia_float) * fps_max_num);
    } else if ((OMNIVISION_OS04A10_MIPI_4M_30FPS_12BIT == sensor_type) ||
               (SMART_SC485SL_MIPI_4M_30FPS_2LANE_12BIT == sensor_type) ||
               (SMART_SC485SL_MIPI_4M_2688_1520_30FPS_4LANE_12BIT == sensor_type) ||
               (SMART_SC465SL_MIPI_4M_4LANE_30FPS_12BIT == sensor_type)) {
        xmedia_u64 exp_ltoh_threshold_sns[6] = {32000000, 64000000, 128000000, 512000000, 1024000000, 1504000000};
        xmedia_u64 exp_htol_threshold_sns[6] = {34000000, 68000000, 132000000, 520000000, 1104000000, 1604000000};
        xmedia_float fps_threshold_sns[6] = {25.0, 25.0, 25.0, 25.0, 12.0, 12.0};
        fps_max_num = 6;

        memcpy(exp_ltoh_threshold, exp_ltoh_threshold_sns, sizeof(xmedia_u64)*fps_max_num);
        memcpy(exp_htol_threshold, exp_htol_threshold_sns, sizeof(xmedia_u64)*fps_max_num);
        memcpy(fps_threshold, fps_threshold_sns, sizeof(xmedia_float)*fps_max_num);
    } else {
        usleep(100 * 1000);
        return;
    }

    if (exp >= dynamic_fps[pipe].last_exp) {
        ptr_threshold = exp_ltoh_threshold;
    } else {
        ptr_threshold = exp_htol_threshold;
    }

    if (exp > ptr_threshold[fps_max_num - 1]) {
        exp_level = fps_max_num - 1;
    } else {
        for (exp_level = 0;(exp > ptr_threshold[exp_level]) && (exp_level < fps_max_num);exp_level++ ) {
            }
    }

    if (exp_level == dynamic_fps[pipe].last_level)
    {
        dynamic_fps[pipe].stable_count++;
    } else {
        dynamic_fps[pipe].stable_count = 0;
        dynamic_fps[pipe].last_level = exp_level;
    }

    dynamic_fps[pipe].last_exp = exp;

    if (dynamic_fps[pipe].stable_count > 15)
    {
        fps = fps_threshold[exp_level];
        if (fps != dynamic_fps[pipe].last_fps) {
            xmedia_isp_get_attr(pipe, &isp_attr);
            isp_attr.fps = fps;
            xmedia_isp_set_attr(pipe, &isp_attr);

            if (g_multi_pipe_mode == XMEDIA_TRUE) {
                xmedia_isp_get_attr(g_multi_pipe_id[1], &isp_attr);
                isp_attr.fps = fps;
                xmedia_isp_set_attr(g_multi_pipe_id[1], &isp_attr);
            }

            dynamic_fps[pipe].last_fps = fps;
        }
        dynamic_fps[pipe].stable_count = 0;
    }
}

static xmedia_void *sample_comm_isp_task(xmedia_void *args)
{
    sample_comm_isp_task_param param = *((sample_comm_isp_task_param *)args);
    xmedia_s32 pipe = param.pipe;
    xmedia_ae_exp_info exp_info;
    xmedia_s32 ret;

    prctl(PR_SET_NAME, (unsigned long)(uintptr_t)"SampleIsptask", 0, 0, 0);

    while (XMEDIA_FALSE == g_isp_task_exit[pipe])
    {
        ret = xmedia_ae_query_exposure_info(pipe, &exp_info);
        if (ret !=  XMEDIA_SUCCESS) {
            SAMPLE_ERR("xmedia_ae_query_exposure_info failed !\n");
            continue;
        }

        if ((g_multi_pipe_mode == XMEDIA_FALSE) ||
            ((g_multi_pipe_mode == XMEDIA_TRUE) && (pipe == g_multi_pipe_id[0]))) {
            sample_comm_isp_set_dynamic_fps(pipe, exp_info.iso, exp_info.exp_time, param.sensor_type);
        }

        usleep(30000);
    }
    return XMEDIA_NULL;
}

static xmedia_s32 sample_comm_isp_create_task(sample_comm_isp_task_param task_param)
{
    xmedia_s32 ret;

    g_isp_task_exit[task_param.pipe] = XMEDIA_FALSE;
    ret = pthread_create(&g_isp_task_id[task_param.pipe], NULL, sample_comm_isp_task, &task_param);
    usleep(20 * 1000);
    return ret;
}

static xmedia_void sample_comm_isp_task_exit(xmedia_s32 pipe)
{
    g_isp_task_exit[pipe] = XMEDIA_TRUE;
    if (g_isp_task_id[pipe]) {
        pthread_join(g_isp_task_id[pipe],NULL);
    }

}

xmedia_s32 sample_comm_isp_enable_fpn(xmedia_u32 isp_pipe, const xmedia_char *fpn_path)
{
    FILE *file = XMEDIA_NULL;
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_u32 file_size;
    xmedia_u32 buff_size;
    xmedia_void *fpn_buff = XMEDIA_NULL;
    xmedia_isp_attr isp_attr = { 0 };
    xmedia_isp_fpn_attr fpn_attr = { 0 };
    xmedia_char fpn_bin[64] = {0};

    if (fpn_path == XMEDIA_NULL) {
        snprintf(fpn_bin, sizeof(fpn_bin), "%s", ((isp_pipe == 0) ? "./FPN.bin" : "./FPN1.bin"));
    } else {
        snprintf(fpn_bin, strlen(fpn_path) + 1, "%s", fpn_path);
    }

    // 1. 检查bin文件是否存在
    if (access(fpn_bin, F_OK) != 0) {
        SAMPLE_ERR("FPN.bin not exist %s.\n",fpn_bin);
        return XMEDIA_ERRCODE_NOT_EXIST;
    }

    // 2. 检查大小是否合适
    file = fopen(fpn_bin, "rb");
    if (file == XMEDIA_NULL) {
        SAMPLE_ERR("open FPN.bin failed.\n");
        return XMEDIA_ERRCODE_OPEN_FAILED;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    xmedia_isp_get_attr(isp_pipe, &isp_attr);
    buff_size = (isp_attr.size.width + 15) / 16 * 16 * isp_attr.size.height;

    if (file_size != buff_size) {
        SAMPLE_ERR("file size not match. file_size = %u. buff_size = %u.\n", file_size, buff_size);
        ret = XMEDIA_FAILURE;
        goto EXIT;
    }

    // 3. 申请buffer,读取文件
    g_isp_fpn_buff[isp_pipe] = xmedia_mmz_alloc(NULL, "isp_fpn_bin", buff_size);
    if (g_isp_fpn_buff[isp_pipe] == XMEDIA_NULL) {
        SAMPLE_ERR("xmedia_mmz_alloc failed. size = %u.\n", buff_size);
        ret = XMEDIA_ERRCODE_NOT_ENOUGH_MEMORY;
        goto EXIT;
    }

    fpn_buff = xmedia_mmz_map(g_isp_fpn_buff[isp_pipe], buff_size, XMEDIA_FALSE);
    if (fpn_buff == XMEDIA_NULL) {
        SAMPLE_ERR("xmedia_mmz_map failed. size = %u.\n", buff_size);
        ret = XMEDIA_ERRCODE_NOT_ENOUGH_MEMORY;
        goto EXIT;
    }

    fread(fpn_buff, buff_size, 1, file);
    xmedia_mmz_unmap(fpn_buff);

    fpn_attr.enable = XMEDIA_TRUE;
    fpn_attr.fpn_frm_info.frame.addr.y_phy_addr = g_isp_fpn_buff[isp_pipe];
    ret = xmedia_isp_set_fpn_attr(isp_pipe, &fpn_attr);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_isp_set_fpn_attr failed.\n");
    }
    SAMPLE_PRT("xmedia_isp_set_fpn_attr success.\n");

EXIT:
    fclose(file);
    return ret;
}

xmedia_s32 sample_comm_isp_disable_fpn(xmedia_u32 isp_pipe)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_isp_fpn_attr fpn_attr = { 0 };

    fpn_attr.enable = XMEDIA_FALSE;
    ret = xmedia_isp_set_fpn_attr(isp_pipe, &fpn_attr);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_isp_set_fpn_attr failed.\n");
        return ret;
    }

    if (g_isp_fpn_buff[isp_pipe] == XMEDIA_NULL) {
        return XMEDIA_SUCCESS;
    }

    ret = xmedia_mmz_free(g_isp_fpn_buff[isp_pipe]);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_mmz_free failed.\n");
        return ret;
    }
    g_isp_fpn_buff[isp_pipe] = XMEDIA_NULL;

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_isp_init(sample_isp_param *isp_param, sample_vi_config *vi_config)
{
    xmedia_s32 ret;
    xmedia_s32 i = 0;
    sample_comm_isp_task_param task_param = { 0 };

    xmedia_isp_config *isp_cfg = XMEDIA_NULL;
    CHECK_NULL_PTR(isp_param);

    // 根据板子类型确定vi_config的i2c, 然后给sensor配置i2c的相关信息和sensor的其他属性
    sample_comm_isp_get_sensor_info(isp_param, vi_config);

    for (i = 0; i < ISP_MAX_PIPE_NUM; i++) {
        if (XMEDIA_FALSE == isp_param->isp_info[i].isp_pipe_en) {
            continue;
        }

        if (-1 == isp_param->pipe[i]) {
            continue;
        }

        if (XMEDIA_TRUE == isp_param->isp_info[i].isp_sensor_en) {
            // 注册sensor驱动，并配备sensor线序、i2c等
            ret = sample_comm_isp_sensor_init(isp_param->pipe[i], &(isp_param->isp_info[i]));
            if (XMEDIA_SUCCESS != ret) {
                SAMPLE_ERR("sensor init fail\n");
                return ret;
            }
        }

        isp_cfg = &(isp_param->isp_info[i].isp_config);

        if (isp_param->isp_info[i].isp_sensor_en == XMEDIA_TRUE) {
            // 向ISP注册AE库
            ret = xmedia_ae_register(isp_param->pipe[i]);
            CHECK_RET(ret, "ae register");

            // 向ISP注册AWB库
            ret = xmedia_awb_register(isp_param->pipe[i]);
            CHECK_RET(ret, "awb register");

            ret = sample_comm_isp_get_sensor_bayer_fmt(isp_param->pipe[i], isp_cfg->size, isp_cfg->wdr_mode,
                                                    isp_param->isp_info[i].mirror, isp_param->isp_info[i].flip,
                                                    &isp_cfg->bayer_fmt);
            CHECK_RET(ret, "get bayer fmt failed");
        }

        // 初始化isp firmware
        ret = xmedia_isp_init(isp_param->pipe[i], isp_cfg);
        CHECK_RET(ret, "isp init");

        if (isp_param->isp_info[i].isp_sensor_en == XMEDIA_TRUE) {
            if (isp_param->isp_info[i].mirror) {
                ret = xmedia_sensor_mirror(isp_param->pipe[i], isp_param->isp_info[i].mirror);
                CHECK_RET(ret, "sensor mirror");
            }

            if (isp_param->isp_info[i].flip) {
                ret = xmedia_sensor_flip(isp_param->pipe[i], isp_param->isp_info[i].flip);
                CHECK_RET(ret, "sensor flip");
            }

            task_param.sensor_type = isp_param->isp_info[i].sensor_type;
            task_param.pipe = isp_param->pipe[i];
            task_param.wdr_mode = isp_cfg->wdr_mode;

            if(isp_param->isp_info[i].disable_dynamic_fps == XMEDIA_FALSE){
                sample_comm_isp_create_task(task_param);
            }

            // When in rgbir mode, need set bayer out fmt after setting mirror or flip.
            ret = sample_comm_isp_get_sensor_bayer_fmt(isp_param->pipe[i], isp_cfg->size, isp_cfg->wdr_mode,
                                        isp_param->isp_info[i].mirror, isp_param->isp_info[i].flip,
                                        &isp_cfg->bayer_fmt);
            CHECK_RET(ret, "get bayer fmt failed");
        }
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_isp_exit(sample_isp_param *isp_param)
{
    xmedia_s32 i = 0;
    xmedia_s32 ret;

    CHECK_NULL_PTR(isp_param);

    for (i = 0; i < ISP_MAX_PIPE_NUM; i++) {
        if (XMEDIA_FALSE == isp_param->isp_info[i].isp_pipe_en) {
            continue;
        }

        if (-1 == isp_param->pipe[i]) {
            continue;
        }

        if (isp_param->isp_info[i].isp_sensor_en == XMEDIA_TRUE) {
            sample_comm_isp_task_exit(isp_param->pipe[i]);
        }

        ret = xmedia_isp_exit(isp_param->pipe[i]);
        CHECK_RET(ret, "isp exit");

        if (XMEDIA_TRUE == isp_param->isp_info[i].isp_sensor_en) {
            ret = xmedia_ae_unregister(isp_param->pipe[i]);
            CHECK_RET(ret, "ae unregister");

            ret = xmedia_awb_unregister(isp_param->pipe[i]);
            CHECK_RET(ret, "awb unregister");

            ret = sample_comm_isp_sensor_exit(isp_param->pipe[i], isp_param->isp_info[i].sensor_type);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_isp_exit fail!\n");
                return ret;
            }
        }
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_isp_start(sample_isp_param *isp_param)
{
    xmedia_s32 i = 0;
    xmedia_s32 ret;
    xmedia_vi_pipe_config vi_pipe_config = { 0 };
    xmedia_isp_bnr_attr bnr_attr = { 0 };
    xmedia_isp_stnr_attr stnr_attr = { 0 };

    CHECK_NULL_PTR(isp_param);
    for (i = ISP_MAX_PIPE_NUM - 1; i >=0; i--) {
        if (XMEDIA_FALSE == isp_param->isp_info[i].isp_pipe_en) {
            continue;
        }

        if (-1 == isp_param->pipe[i]) {
            continue;
        }

        // BNR
        ret = xmedia_vi_get_pipe_config(isp_param->pipe[i], &vi_pipe_config);
        CHECK_RET(ret, "get vi pipe attr");

        ret = xmedia_isp_get_bnr_attr(isp_param->pipe[i], &bnr_attr);
        CHECK_RET(ret, "get bnr attr");

        bnr_attr.enable = vi_pipe_config.bnr_attr.bnr_en;
        bnr_attr.tnr_enable = vi_pipe_config.bnr_attr.bnr_en;
        ret = xmedia_isp_set_bnr_attr(isp_param->pipe[i], &bnr_attr);
        CHECK_RET(ret, "set bnr attr");

        // STNR
        ret = xmedia_isp_get_stnr_attr(isp_param->pipe[i], &stnr_attr);
        CHECK_RET(ret, "get stnr attr");

        stnr_attr.stnr_enable = vi_pipe_config.stnr_attr.stnr_en;
        stnr_attr.tnr_enable = vi_pipe_config.stnr_attr.stnr_en;
        stnr_attr.cnr_enable = vi_pipe_config.stnr_attr.stnr_en;
        ret = xmedia_isp_set_stnr_attr(isp_param->pipe[i], &stnr_attr);
        CHECK_RET(ret, "set stnr attr");

        // 运行ISP firmware
        ret = xmedia_isp_start(isp_param->pipe[i]);
        CHECK_RET(ret, "isp start");

        if (XMEDIA_TRUE == isp_param->isp_info[i].isp_sensor_en) {
            ret = xmedia_sensor_start(isp_param->pipe[i]);
            CHECK_RET(ret, "sensor start");
        }
    }
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_isp_stop(sample_isp_param *isp_param)
{
    xmedia_s32 i = 0;
    xmedia_s32 ret;

    CHECK_NULL_PTR(isp_param);
    for (i = 0; i < ISP_MAX_PIPE_NUM; i++) {
        if (XMEDIA_FALSE == isp_param->isp_info[i].isp_pipe_en) {
            continue;
        }

        if (-1 == isp_param->pipe[i]) {
            continue;
        }

        ret = xmedia_isp_stop(isp_param->pipe[i]);
        CHECK_RET(ret, "isp stop");

        if (XMEDIA_TRUE == isp_param->isp_info[i].isp_sensor_en) {
            ret = xmedia_sensor_stop(isp_param->pipe[i]);
            CHECK_RET(ret, "sensor stop");
        }
    }
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_isp_get_sensor_bayer_fmt(xmedia_u32 pipe, xmedia_video_size size, xmedia_video_wdr_mode wdr_mode,
                                                xmedia_bool mirror, xmedia_bool flip,
                                                xmedia_video_bayer_format *bayer_fmt)
{
    xmedia_sensor_property property;
    xmedia_s32 ret;

    ret = xmedia_sensor_get_property(pipe, &property);
    CHECK_RET(ret, "get sensor property");

    if (mirror && flip) {
        *bayer_fmt = property.bayer_format[XMEDIA_SENSOR_MIRROR_FLIP_TYPE_MIRROR_FLIP];
    } else if (mirror) {
        *bayer_fmt = property.bayer_format[XMEDIA_SENSOR_MIRROR_FLIP_TYPE_MIRROR];
    } else if (flip) {
        *bayer_fmt = property.bayer_format[XMEDIA_SENSOR_MIRROR_FLIP_TYPE_FLIP];
    } else {
        *bayer_fmt = property.bayer_format[XMEDIA_SENSOR_MIRROR_FLIP_TYPE_NORMAL];
    }

    return ret;
}

xmedia_s32 sample_comm_isp_get_sensor_bit_rate(xmedia_u32 pipe, xmedia_u32 *bit_rate)
{
    xmedia_sensor_property property;
    xmedia_s32 ret;

    CHECK_NULL_PTR(bit_rate);

    ret = xmedia_sensor_get_property(pipe, &property);
    CHECK_RET(ret, "get sensor property");

    *bit_rate = property.bit_rate;
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_isp_get_sensor_effect_index(sample_isp_sensor_effect_list list[], xmedia_s32 list_num, sample_comm_sensor_type sensor_type)
{
    xmedia_s32 i = 0;

    for(i = 0; i < list_num; i ++) {
        if(sensor_type == list[i].sensor_type && list[i].sensor_type != SENSOR_TYPE_INVALID) {
            SAMPLE_PRT("sensor type %d with id : %d \n", sensor_type, i);
            return i;
        }
    }
    return XMEDIA_FAILURE;
}

static xmedia_s32 sample_comm_isp_get_file_size(const char *filename)
{
    xmedia_s32 ret;
    xmedia_s32 len;

    FILE *fp = fopen(filename, "r");

    if (!fp) {
        SAMPLE_PRT("cannot open %s\n", filename);
        return -1;
    }

    SAMPLE_PRT("model path: %s\n", filename);
    ret = fseek(fp, 0, SEEK_END);
    if (ret != 0) {
        fclose(fp);
        return -1;
    }

    len = ftell(fp);
    fclose(fp);

    return len;
}

static xmedia_s32 sample_comm_isp_copy_file_to_buff(const char *src_file, char *des,
                                       unsigned int offset, unsigned int size)
{
    xmedia_s32 ret;
    xmedia_u32 count;
    FILE *fp = fopen(src_file, "r");

    if (fp == NULL) {
        return -1;
    }

    ret = fseek(fp, offset, SEEK_SET);
    if (ret != 0) {
        fclose(fp);
        return -1;
    }

    count = fread(des, 1, size, fp);
    if (count != size) {
        SAMPLE_PRT("want to read size = 0x%x, in fact count = 0x%x\n", size, count);
        fclose(fp);
        return -1;
    }

    fclose(fp);

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_comm_isp_ainr_load_model(sample_isp_param *isp_param, sample_vi_config *vi_config, xmedia_u32 index)
{
    xmedia_s32 ret = 0;
    xmedia_u8  i = 0;
    xmedia_s32 model_size = 0;
    xmedia_char *model[XMEDIA_ISP_AINR_MODEL_MAX_NUM] = { XMEDIA_NULL };
    xmedia_u32 model_id = 0;

    if (isp_param->ainr_model_path[index] == XMEDIA_NULL) {
        // 此处默认双目使用的是同一款sensor，共用AINR模型
        model_id = sample_comm_isp_get_sensor_effect_index(g_ainr_model_list, AINR_LIST_NUM, isp_param->isp_info[index].sensor_type);
        if(model_id == -1) {
            SAMPLE_ERR("Can not found ainr model \n");
            return XMEDIA_FAILURE;
        }

    //读取网络模型文件到内存
        model_size = sample_comm_isp_get_file_size(g_ainr_model_list[model_id].load_file);
        if (model_size < 0) {
            SAMPLE_PRT("get model file size err.\n");
            return XMEDIA_FAILURE;
        }

        model[0] = malloc(model_size);
        if (model[0] == XMEDIA_NULL) {
            SAMPLE_PRT("malloc err\n");
            return XMEDIA_ERRCODE_NOT_ENOUGH_MEMORY;
        }

        ret = sample_comm_isp_copy_file_to_buff(g_ainr_model_list[model_id].load_file, model[0], 0, model_size);
        SAMPLE_CHECK_RET_GOTO(ret, ERR0);
    } else {
        //读取网络模型文件到内存
        model_size = sample_comm_isp_get_file_size(isp_param->ainr_model_path[index]);
        if (model_size < 0) {
            SAMPLE_PRT("get model file size err.\n");
            return XMEDIA_FAILURE;
        }

        model[0] = malloc(model_size);
        if (model[0] == XMEDIA_NULL) {
            SAMPLE_PRT("malloc err\n");
            return XMEDIA_ERRCODE_NOT_ENOUGH_MEMORY;
        }

        ret = sample_comm_isp_copy_file_to_buff(isp_param->ainr_model_path[index], model[0], 0, model_size);
        SAMPLE_CHECK_RET_GOTO(ret, ERR0);
    }

    ret = xmedia_isp_load_ainr_model(isp_param->pipe[index], (xmedia_void *)model);
    SAMPLE_CHECK_RET_GOTO(ret, ERR0);

ERR0:
    for(i = 0; i < XMEDIA_ISP_AINR_MODEL_MAX_NUM; i ++) {
        if(model[i] != XMEDIA_NULL) {
            free(model[i]);
            model[i] = XMEDIA_NULL;
        }
    }

    return ret;
}

static xmedia_s32 sample_comm_isp_ainr_unload_model(sample_isp_param *isp_param)
{
    xmedia_u8 i;
    xmedia_s32 ret;
    xmedia_isp_ainr_attr ainr_attr = {0};

    for (i = 0; i < VI_MAX_PIPE_NUM; i++) {
        if (isp_param->isp_info[i].isp_pipe_en == XMEDIA_FALSE) {
            continue;
        }

        // 卸载模型前, 关闭ainr
        ret = xmedia_isp_get_ainr_attr(isp_param->pipe[i], &ainr_attr);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("pipe %d isp get ainr attr failed\n", isp_param->pipe[i]);
            return ret;
        }

        ainr_attr.enable = XMEDIA_FALSE;
        ret = xmedia_isp_set_ainr_attr(isp_param->pipe[i], &ainr_attr);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("pipe %d isp set ainr attr failed\n", isp_param->pipe[i]);
            return ret;
        }

        xmedia_isp_unload_ainr_model(isp_param->pipe[i]);
    }

    return XMEDIA_SUCCESS;
}

// 必须在isp init后调用
xmedia_s32 sample_comm_isp_ainr_init(sample_isp_param *isp_param, sample_vi_config *vi_config)
{
    xmedia_u8  i;
    xmedia_s32 ret                 = XMEDIA_SUCCESS;
    xmedia_isp_ainr_attr ainr_attr = {0};
    xmedia_isp_bnr_attr bnr_attr   = {0};
    const xmedia_char* model_name  = NULL;
    xmedia_s32 model_id = -1;

    // 此处默认双目使用的是同一款sensor，共用AINR模型
    model_id = sample_comm_isp_get_sensor_effect_index(g_ainr_model_list, AINR_LIST_NUM, isp_param->isp_info[0].sensor_type);
    if(model_id == -1) {
        SAMPLE_ERR("Can not found ainr model \n");
        return XMEDIA_FAILURE;
    }
    model_name = g_ainr_model_list[model_id].load_file;
    SAMPLE_PRT("ainr model : %s \n", model_name);

    for (i = 0; i < ISP_MAX_PIPE_NUM; i++) {
        if (XMEDIA_FALSE == isp_param->isp_info[i].isp_pipe_en) {
            continue;
        }

        if (-1 == isp_param->pipe[i]) {
            continue;
        }

        // 0. 加载模型前, 关闭ainr
        ret = xmedia_isp_get_ainr_attr(isp_param->pipe[i], &ainr_attr);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("pipe %d isp get ainr attr failed\n", isp_param->pipe[i]);
            return ret;
        }
        ainr_attr.enable = XMEDIA_FALSE;
        ret = xmedia_isp_set_ainr_attr(isp_param->pipe[i], &ainr_attr);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("pipe %d isp set ainr attr failed\n", isp_param->pipe[i]);
            return ret;
        }

        // 1.加载模型
        ret = sample_comm_isp_ainr_load_model(isp_param, vi_config, i);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("isp load model failed\n");
            return ret;
        }

        // 2.设置bnr中的tnr开启(目前默认按照ainr post模式，必须开启tnr)
        ret = xmedia_isp_get_bnr_attr(isp_param->pipe[i], &bnr_attr);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("pipe %d isp set bnr attr failed\n", isp_param->pipe[i]);
            goto ERR0;
        }
        bnr_attr.enable = XMEDIA_TRUE;
        bnr_attr.ainr_pos_switch = XMEDIA_ISP_BNR_AINR_POS_POST;
        bnr_attr.tnr_enable = XMEDIA_TRUE;
        ret = xmedia_isp_set_bnr_attr(isp_param->pipe[i], &bnr_attr);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("pipe %d isp set bnr attr failed\n", isp_param->pipe[i]);
            goto ERR0;
        }

        // 3.设置ainr
        ret = xmedia_isp_get_ainr_attr(isp_param->pipe[i], &ainr_attr);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("pipe %d isp get ainr attr failed\n", isp_param->pipe[i]);
            goto ERR0;
        }
        ainr_attr.enable  = XMEDIA_TRUE;
        ainr_attr.op_type = XMEDIA_VIDEO_OPERATION_MODE_MANUAL;
        ainr_attr.manual_attr.pos_gain = 64;
        ainr_attr.manual_attr.gau_gain = 64;
        ret = xmedia_isp_set_ainr_attr(isp_param->pipe[i], &ainr_attr);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("pipe %d isp set ainr attr failed\n", isp_param->pipe[i]);
            goto ERR0;
        }
    }

    return XMEDIA_SUCCESS;

ERR0:
    sample_comm_isp_ainr_unload_model(isp_param);
    return ret;
}

// 必须在isp exit前调用
xmedia_void sample_comm_isp_ainr_exit(sample_isp_param *isp_param)
{
    sample_comm_isp_ainr_unload_model(isp_param);
    return;
}

// 直接配置寄存器地址方式复位取消复位sensor，用于未注册isp sensor驱动的输入，例如ahd
xmedia_s32 sample_comm_isp_sensor_reset(xmedia_s32 sns_id)
{
    xmedia_u32 value = 0;
    xmedia_u32 sns_addr = SENSOR_CRL_BASE_ADDR;
    xmedia_u32 sns_rst = 0x3; // bit[0]:sensor clk,bit[1]:sensor rst
    xmedia_bool sns_clk_close; // 复位后是否需要关闭sensor clk

    if (sample_comm_sys_mem_open() != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample comm sys mem open failed\n");
        return XMEDIA_FAILURE;
    }

    if (sample_comm_sys_get_reg(sns_addr, &value) != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample comm sys get reg 0x%x failed\n", sns_addr);
        sample_comm_sys_mem_close();
        return XMEDIA_FAILURE;
    }

    if ((value >> (sns_id * 8)) & 0x1) {
        sns_clk_close = XMEDIA_FALSE;
    } else {
        sns_clk_close = XMEDIA_TRUE;
    }

    value |= (sns_rst << (sns_id * 8));
    if (sample_comm_sys_set_reg(sns_addr, value) != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample comm sys set reg 0x%x failed\n", sns_addr);
        sample_comm_sys_mem_close();
        return XMEDIA_FAILURE;
    }

    usleep(2000);

    value &= (~(0x1 << ((sns_id * 8) + 1)));
    if (sample_comm_sys_set_reg(sns_addr, value) != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample comm sys set reg 0x%x failed\n", sns_addr);
        sample_comm_sys_mem_close();
        return XMEDIA_FAILURE;
    }

    if (sns_clk_close == XMEDIA_TRUE) {
        value &= (~(0x1 << (sns_id * 8)));
        if (sample_comm_sys_set_reg(sns_addr, value) != XMEDIA_SUCCESS) {
            SAMPLE_PRT("sample comm sys set reg 0x%x failed\n", sns_addr);
            sample_comm_sys_mem_close();
            return XMEDIA_FAILURE;
        }
    }

    sample_comm_sys_mem_close();

    return XMEDIA_SUCCESS;
}
