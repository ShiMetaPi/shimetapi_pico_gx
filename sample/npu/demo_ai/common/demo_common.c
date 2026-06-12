#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>

#include "xmedia_api_sys.h"
#include "xmedia_api_vb.h"
#include "xmedia_api_vi.h"
#include "xmedia_api_venc.h"
#include "xmedia_api_vpss.h"
#include "xmedia_api_region.h"
#include "xmedia_api_isp.h"
#include "xmedia_api_vgs.h"
#include "xmedia_api_ae.h"
#include "xmedia_api_awb.h"
#include "mipi.h"
#include "sns_ctrl.h"
#include "ae_comm.h"
#include "awb_comm.h"

#include "buffer.h"
#include "comm_video.h"
#include "demo_common.h"

#include "canvas.h"
#include "canvas_font.h"
typedef struct{
    xmedia_u32 width;
    xmedia_u32 height;
}rect_size;

#define SCORE_LEN   (64)
#define CONVERT_TO_32BIT_ADDR(Addr) (void*)(XMEDIA_UL)(Addr)
#define CONVERT_TO_64BIT_ADDR(Addr) (XMEDIA_U64)(XMEDIA_UL)(Addr)
static BITMAP_S g_bitmap;


static VI_DEV_ATTR_S g_vi_dev_base_attr_imx307 = {
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFF00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1280,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            720,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    XMEDIA_FALSE,
    {1920, 1080},
    {
        {
            {1920 , 1080},

        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1080
    },
    DATA_RATE_X1
};

static VI_PIPE_ATTR_S g_vi_pipe_base_attr_imx307 =
{
    VI_PIPE_BYPASS_NONE, XMEDIA_FALSE, XMEDIA_FALSE,
    1920, 1080,
    PIXEL_FORMAT_RGB_BAYER_12BPP,
    COMPRESS_MODE_NONE,
    DATA_BITWIDTH_12,
    XMEDIA_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    XMEDIA_FALSE,
    { -1, -1}
};

static VI_CHN_ATTR_S g_vi_chn_base_attr_imx307 =
{
    {1920, 1080},
    PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    DYNAMIC_RANGE_SDR8,
    VIDEO_FORMAT_LINEAR,
    COMPRESS_MODE_NONE,
    0,      0,
    0,
    { -1, -1}
};

static combo_dev_attr_t g_vi_mipi_base_attr_imx307 =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_12BIT,
            MIPI_WDR_MODE_NONE,
            {0, 2, -1, -1}
        }
    }
};

static ISP_PUB_ATTR_S g_isp_pub_attr_imx307_2m_30fps =
{
    {0, 0, 1920, 1080},
    {1920, 1080},
    30,
    BAYER_RGGB,
    WDR_MODE_NONE,
    0,
};

static VI_DEV_ATTR_S g_vi_dev_base_attr_gc4653_2l = {
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFC00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            2560,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            1440,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    XMEDIA_FALSE,
    {2560 , 1440},
    {
        {
            {2560 , 1440},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1440
    },
    DATA_RATE_X1
};

static VI_DEV_ATTR_S g_vi_dev_base_attr_gc4663_2l = {
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFC00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            2560,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            1440,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    XMEDIA_FALSE,
    {2560 , 1440},
    {
        {
            {2560 , 1440},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1440
    },
    DATA_RATE_X1
};

static VI_PIPE_ATTR_S g_vi_pipe_base_attr_gc4653_2l = {
    VI_PIPE_BYPASS_NONE, XMEDIA_FALSE, XMEDIA_FALSE,
    2560, 1440,
    PIXEL_FORMAT_RGB_BAYER_10BPP,
    COMPRESS_MODE_NONE,
    DATA_BITWIDTH_10,
    XMEDIA_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    XMEDIA_FALSE,
    { -1, -1}
};

static VI_PIPE_ATTR_S g_vi_pipe_base_attr_gc4663_2l = {
    VI_PIPE_BYPASS_NONE, XMEDIA_FALSE, XMEDIA_FALSE,
    2560, 1440,
    PIXEL_FORMAT_RGB_BAYER_10BPP,
    COMPRESS_MODE_NONE,
    DATA_BITWIDTH_10,
    XMEDIA_FALSE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    XMEDIA_FALSE,
    { -1, -1}
};

static VI_CHN_ATTR_S g_vi_chn_base_attr_gc4653_2l = {
    {2560, 1440},
    PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    DYNAMIC_RANGE_SDR8,
    VIDEO_FORMAT_LINEAR,
    COMPRESS_MODE_NONE,
    0,      0,
    0,
    { -1, -1}
};

static VI_CHN_ATTR_S g_vi_chn_base_attr_gc4663_2l = {
    {2560, 1440},
    PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    DYNAMIC_RANGE_SDR8,
    VIDEO_FORMAT_LINEAR,
    COMPRESS_MODE_NONE,
    0,      0,
    0,
    { -1, -1}
};

static combo_dev_attr_t g_vi_mipi_base_attr_gc4653_2l = {
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 2560, 1440},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            MIPI_WDR_MODE_NONE,
            {0, 2, -1, -1}
        }
    }
};

static combo_dev_attr_t g_vi_mipi_base_attr_gc4663_2l = {
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 2560, 1440},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            MIPI_WDR_MODE_NONE,
            {0, 2, -1, -1}
        }
    }
};

static ISP_PUB_ATTR_S g_isp_pub_attr_gc4653_2l_4m_30fps = {
    {0, 0, 2560, 1440},
    {2560, 1440},
    30,
    BAYER_GRBG,
    WDR_MODE_NONE,
    0,
};

static ISP_PUB_ATTR_S g_isp_pub_attr_gc4663_2l_4m_30fps = {
    {0, 0, 2560, 1440},
    {2560, 1440},
    30,
    BAYER_GRBG,
    WDR_MODE_NONE,
    0,
};

static VI_DEV_ATTR_S g_vi_dev_base_attr_gc2053 =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFC00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            1280,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            720,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    XMEDIA_FALSE,
    {1920 , 1080},
    {
        {
            {1920 , 1080},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1080
    },
    DATA_RATE_X1
};

static VI_PIPE_ATTR_S g_vi_pipe_base_attr_gc2053 =
{
    VI_PIPE_BYPASS_NONE, XMEDIA_FALSE, XMEDIA_FALSE,
    1920, 1080,
    PIXEL_FORMAT_RGB_BAYER_10BPP,
    COMPRESS_MODE_NONE,
    DATA_BITWIDTH_10,
    XMEDIA_TRUE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    XMEDIA_FALSE,
    { -1, -1}
};

static VI_CHN_ATTR_S g_vi_chn_base_attr_gc2053 =
{
    {1920, 1080},
    PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    DYNAMIC_RANGE_SDR8,
    VIDEO_FORMAT_LINEAR,
    COMPRESS_MODE_NONE,
    0,      0,
    0,
    { -1, -1}
};

static combo_dev_attr_t g_vi_mipi_base_attr_gc2053 =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 1920, 1080},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            MIPI_WDR_MODE_NONE,
            {0, 2, -1, -1}
        }
    }
};

static ISP_PUB_ATTR_S g_isp_pub_attr_gc2053_2m_30fps =
{
    {0, 0, 1920, 1080},
    {1920, 1080},
    30,
    BAYER_RGGB,
    WDR_MODE_NONE,
    0,
};

static VI_DEV_ATTR_S g_vi_dev_base_attr_sc530ai =
{
    VI_MODE_MIPI,
    VI_WORK_MODE_1Multiplex,
    {0xFFC00000,    0x0},
    VI_SCAN_PROGRESSIVE,
    { -1, -1, -1, -1},
    VI_DATA_SEQ_YUYV,

    {
        /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
        VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH, VI_VSYNC_VALID_SINGAL, VI_VSYNC_VALID_NEG_HIGH,

        /*hsync_hfb    hsync_act    hsync_hhb*/
        {
            0,            2880,        0,
            /*vsync0_vhb vsync0_act vsync0_hhb*/
            0,            1620,        0,
            /*vsync1_vhb vsync1_act vsync1_hhb*/
            0,            0,            0
        }
    },
    VI_DATA_TYPE_RGB,
    XMEDIA_FALSE,
    {2880 , 1620},
    {
        {
            {2880 , 1620},
        },
        {
            VI_REPHASE_MODE_NONE,
            VI_REPHASE_MODE_NONE
        }
    },
    {
        WDR_MODE_NONE,
        1620
    },
    DATA_RATE_X1
};

static VI_PIPE_ATTR_S g_vi_pipe_base_attr_sc530ai =
{
    VI_PIPE_BYPASS_NONE, XMEDIA_FALSE, XMEDIA_FALSE,
    2880, 1620,
    PIXEL_FORMAT_RGB_BAYER_10BPP,
    COMPRESS_MODE_NONE,
    DATA_BITWIDTH_10,
    XMEDIA_TRUE,
    {
        PIXEL_FORMAT_YVU_SEMIPLANAR_420,
        DATA_BITWIDTH_8,
        VI_NR_REF_FROM_RFR,
        COMPRESS_MODE_NONE
    },
    XMEDIA_FALSE,
    {-1, -1}
};

static VI_CHN_ATTR_S g_vi_chn_base_attr_sc530ai =
{
    {2880, 1620},
    PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    DYNAMIC_RANGE_SDR8,
    VIDEO_FORMAT_LINEAR,
    COMPRESS_MODE_NONE,
    0,      0,
    0,
    {-1, -1}
};

static combo_dev_attr_t g_vi_mipi_base_attr_sc530ai =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 2880, 1620},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            MIPI_WDR_MODE_NONE,
            {0, 1, 2, 3}
        }
    }
};

static combo_dev_attr_t g_vi_mipi_base_attr_sc530ai_2l =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 2880, 1620},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            MIPI_WDR_MODE_NONE,
            {0, 2, -1, -1}
        }
    }
};

static combo_dev_attr_t g_vi_mipi_base_attr_sc530ai_wdr =
{
    .devno = 0,
    .input_mode = INPUT_MODE_MIPI,
    .data_rate = MIPI_DATA_RATE_X1,
    .img_rect = {0, 0, 2880, 1620},

    {
        .mipi_attr =
        {
            DATA_TYPE_RAW_10BIT,
            MIPI_WDR_MODE_VC,
            {0, 1, 2, 3}
        }
    }
};

static ISP_PUB_ATTR_S g_isp_pub_attr_sc530ai_5m_30fps_wdr2to1 =
{
    {0, 0, 2880, 1620},
    {2880, 1620},
    30,
    BAYER_BGGR,
    WDR_MODE_2To1_LINE,
    0,
};

static ISP_PUB_ATTR_S g_isp_pub_attr_sc530ai_5m_30fps =
{
    {0, 0, 2880, 1620},
    {2880, 1620},
    30,
    BAYER_BGGR,
    WDR_MODE_NONE,
    0,
};

static ISP_PUB_ATTR_S g_isp_pub_attr_sc530ai_2l_5m_25fps =
{
    {0, 0, 2880, 1620},
    {2880, 1620},
    25,
    BAYER_BGGR,
    WDR_MODE_NONE,
    0,
};

XMEDIA_S32 demo_comm_get_mipi_lanemode(XMEDIA_VOID)
{
    XMEDIA_S32  ret = XMEDIA_SUCCESS;
    XMEDIA_U32  chip_id = 0;
    ret = XMEDIA_API_SYS_GetChipId(&chip_id);
    if (ret != XMEDIA_SUCCESS)  {
        DEMO_PRINT("not support this chip\n");
        return -1;
    }

    if (chip_id == CHIP_NAME_XM72050200 || chip_id == CHIP_NAME_XM72020300 || chip_id == CHIP_NAME_XM72050500
        || chip_id == CHIP_NAME_XM72020330 || chip_id == CHIP_NAME_XM72050230 || chip_id == CHIP_NAME_XM72050210
        || chip_id == CHIP_NAME_XM72010200 || chip_id == CHIP_NAME_XM72010300) {
        DEMO_PRINT("support this chip %x\n", chip_id);
        return 0;
    } else if (chip_id == CHIP_NAME_XM72050300 || chip_id == CHIP_NAME_XM72050510 ||
        chip_id == CHIP_NAME_XM76050100 || chip_id == CHIP_NAME_XM72050530) {
        DEMO_PRINT("support this chip %x\n", chip_id);
        return 1;
    } else {
        DEMO_PRINT("not support this chip\n");
        return -1;
    }
}

void demo_get_vi_dev_default_attr(demo_sensor_type sns, VI_DEV_ATTR_S *attr)
{
    switch(sns) {
        case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT:
            memcpy(attr, &g_vi_dev_base_attr_imx307, sizeof(VI_DEV_ATTR_S));
            break;
        case GALAXYCORE_GC4653_2L_MIPI_4M_30FPS_10BIT:
            memcpy(attr, &g_vi_dev_base_attr_gc4653_2l, sizeof(VI_DEV_ATTR_S));
            break;
        case GALAXYCORE_GC4663_2L_MIPI_4M_30FPS_10BIT:
            memcpy(attr, &g_vi_dev_base_attr_gc4663_2l, sizeof(VI_DEV_ATTR_S));
            break;
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT:
            memcpy(attr, &g_vi_dev_base_attr_gc2053, sizeof(VI_DEV_ATTR_S));
            attr->au32ComponentMask[0] = 0xFFC00000;
            break;
        case SMART_SC530AI_MIPI_5M_30FPS_10BIT:
        case SMART_SC530AI_MIPI_5M_30FPS_10BIT_WDR2TO1:
        case SMART_SC530AI_2L_MIPI_5M_25FPS_10BIT:
            memcpy(attr, &g_vi_dev_base_attr_sc530ai, sizeof(VI_DEV_ATTR_S));
            break;
        default:
            attr = NULL;
    }

    return;
}

void demo_get_vi_pipe_default_attr(demo_sensor_type sns, VI_PIPE_ATTR_S *attr)
{
    switch(sns) {
        case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT:
            memcpy(attr, &g_vi_pipe_base_attr_imx307, sizeof(VI_PIPE_ATTR_S));
            break;
        case GALAXYCORE_GC4653_2L_MIPI_4M_30FPS_10BIT:
            memcpy(attr, &g_vi_pipe_base_attr_gc4653_2l, sizeof(VI_PIPE_ATTR_S));
            break;
        case GALAXYCORE_GC4663_2L_MIPI_4M_30FPS_10BIT:
            memcpy(attr, &g_vi_pipe_base_attr_gc4663_2l, sizeof(VI_PIPE_ATTR_S));
            break;
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT:
            memcpy(attr, &g_vi_pipe_base_attr_gc2053, sizeof(VI_PIPE_ATTR_S));
            break;
        case SMART_SC530AI_MIPI_5M_30FPS_10BIT:
        case SMART_SC530AI_MIPI_5M_30FPS_10BIT_WDR2TO1:
        case SMART_SC530AI_2L_MIPI_5M_25FPS_10BIT:
            memcpy(attr, &g_vi_pipe_base_attr_sc530ai, sizeof(VI_PIPE_ATTR_S));
            break;
        default:
            attr = NULL;
    }

    return;
}

void demo_get_vi_chn_default_attr(demo_sensor_type sns, VI_CHN_ATTR_S *attr)
{
    switch(sns) {
        case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT:
            memcpy(attr, &g_vi_chn_base_attr_imx307, sizeof(VI_CHN_ATTR_S));
            break;
        case GALAXYCORE_GC4653_2L_MIPI_4M_30FPS_10BIT:
            memcpy(attr, &g_vi_chn_base_attr_gc4653_2l, sizeof(VI_CHN_ATTR_S));
            break;
        case GALAXYCORE_GC4663_2L_MIPI_4M_30FPS_10BIT:
            memcpy(attr, &g_vi_chn_base_attr_gc4663_2l, sizeof(VI_CHN_ATTR_S));
            break;
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT:
            memcpy(attr, &g_vi_chn_base_attr_gc2053, sizeof(VI_CHN_ATTR_S));
            break;
        case SMART_SC530AI_MIPI_5M_30FPS_10BIT:
        case SMART_SC530AI_MIPI_5M_30FPS_10BIT_WDR2TO1:
        case SMART_SC530AI_2L_MIPI_5M_25FPS_10BIT:
            memcpy(attr, &g_vi_chn_base_attr_sc530ai, sizeof(VI_CHN_ATTR_S));
            break;
        default:
            attr = NULL;
    }

    return;
}

void demo_get_mipi_default_attr(demo_sensor_type sns, combo_dev_attr_t *attr)
{
    switch(sns) {
        case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT:
            memcpy(attr, &g_vi_mipi_base_attr_imx307, sizeof(combo_dev_attr_t));
            if (demo_comm_get_mipi_lanemode() == 1) {
                attr->mipi_attr.lane_id[1] = 1;
            }
            break;
        case GALAXYCORE_GC4653_2L_MIPI_4M_30FPS_10BIT:
            memcpy(attr, &g_vi_mipi_base_attr_gc4653_2l, sizeof(combo_dev_attr_t));
            if (demo_comm_get_mipi_lanemode() == 1) {
                attr->mipi_attr.lane_id[1] = 1;
            }
            break;
        case GALAXYCORE_GC4663_2L_MIPI_4M_30FPS_10BIT:
            memcpy(attr, &g_vi_mipi_base_attr_gc4663_2l, sizeof(combo_dev_attr_t));
            if (demo_comm_get_mipi_lanemode() == 1) {
                attr->mipi_attr.lane_id[1] = 1;
            }
            break;
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT:
            memcpy(attr, &g_vi_mipi_base_attr_gc2053, sizeof(combo_dev_attr_t));
            if (demo_comm_get_mipi_lanemode() == 1) {
                attr->mipi_attr.lane_id[1] = 1;
            }
            break;
        case SMART_SC530AI_MIPI_5M_30FPS_10BIT:
            memcpy(attr, &g_vi_mipi_base_attr_sc530ai, sizeof(combo_dev_attr_t));
            break;
        case SMART_SC530AI_MIPI_5M_30FPS_10BIT_WDR2TO1:
            memcpy(attr, &g_vi_mipi_base_attr_sc530ai_wdr, sizeof(combo_dev_attr_t));
            break;
        case SMART_SC530AI_2L_MIPI_5M_25FPS_10BIT:
            memcpy(attr, &g_vi_mipi_base_attr_sc530ai_2l, sizeof(combo_dev_attr_t));
            if (demo_comm_get_mipi_lanemode() == 1) {
                attr->mipi_attr.lane_id[1] = 1;
            }
            break;
        default:
            attr = NULL;
    }

    return;
}

void demo_get_isp_pub_default_attr(demo_sensor_type sns, ISP_PUB_ATTR_S *attr)
{
    switch(sns) {
        case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT:
            memcpy(attr, &g_isp_pub_attr_imx307_2m_30fps, sizeof(ISP_PUB_ATTR_S));
            break;
        case GALAXYCORE_GC4653_2L_MIPI_4M_30FPS_10BIT:
            memcpy(attr, &g_isp_pub_attr_gc4653_2l_4m_30fps, sizeof(ISP_PUB_ATTR_S));
            break;
        case GALAXYCORE_GC4663_2L_MIPI_4M_30FPS_10BIT:
            memcpy(attr, &g_isp_pub_attr_gc4663_2l_4m_30fps, sizeof(ISP_PUB_ATTR_S));
            break;
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT:
            memcpy(attr, &g_isp_pub_attr_gc2053_2m_30fps, sizeof(ISP_PUB_ATTR_S));
            break;
        case SMART_SC530AI_MIPI_5M_30FPS_10BIT:
            memcpy(attr, &g_isp_pub_attr_sc530ai_5m_30fps, sizeof(ISP_PUB_ATTR_S));
            break;
        case SMART_SC530AI_MIPI_5M_30FPS_10BIT_WDR2TO1:
            memcpy(attr, &g_isp_pub_attr_sc530ai_5m_30fps_wdr2to1, sizeof(ISP_PUB_ATTR_S));
            break;
        case SMART_SC530AI_2L_MIPI_5M_25FPS_10BIT:
            memcpy(attr, &g_isp_pub_attr_sc530ai_2l_5m_25fps, sizeof(ISP_PUB_ATTR_S));
            break;
        default:
            attr = NULL;
    }

    return;
}

ISP_SNS_OBJ_S *demo_get_sns_obj(demo_sensor_type sns)
{
    switch(sns) {
        case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT:
            return &stSnsImx307_2l_Obj;
            break;
        case GALAXYCORE_GC4653_2L_MIPI_4M_30FPS_10BIT:
            return &stSnsgc4653_2lObj;
            break;
        case GALAXYCORE_GC4663_2L_MIPI_4M_30FPS_10BIT:
            return &stSnsGc4663Obj;
            break;
        case GALAXYCORE_GC2053_MIPI_2M_30FPS_10BIT:
            return &stSnsGc2053Obj;
            break;
        case SMART_SC530AI_MIPI_5M_30FPS_10BIT:
        case SMART_SC530AI_MIPI_5M_30FPS_10BIT_WDR2TO1:
            return &stSnsSc530aiObj;
            break;
        case SMART_SC530AI_2L_MIPI_5M_25FPS_10BIT:
            return &stSnsSc530ai_2lObj;
            break;
        default:
            return NULL;
    }

    return NULL;    
}

static rect_size sample_ivp_caculate_font_size(font_size_e font_size)
{
    xmedia_s32 asc_width = 0;
    xmedia_s32 asc_height = 0;

    rect_size font;

    switch(font_size) {
        case FONT_SIZE_16:
            asc_width = 8;
            asc_height = 16;
            break;

        case FONT_SIZE_20:
            asc_width = 10;
            asc_height = 20;
            break;

        case FONT_SIZE_24:
            asc_width = 12;
            asc_height = 24;
            break;

        case FONT_SIZE_32:
            asc_width = 16;
            asc_height = 32;
            break;

        case FONT_SIZE_40:
            asc_width = 20;
            asc_height = 40;
            break;

        case FONT_SIZE_48:
            asc_width = 24;
            asc_height = 48;
            break;

        case FONT_SIZE_56:
            asc_width = 28;
            asc_height = 56;
            break;

        default:
            printf("not support this font size: %d\n", font_size);
            asc_width = 0;
            asc_height = 0;
    }

    font.width = asc_width;
    font.height = asc_height;

    return font;
}


xmedia_s32 sample_construct_string_bitmap(xmedia_char* string_content, xmedia_s32 string_len, font_size_e font_type, BITMAP_S* bit_map)
{
    xmedia_s32 ret;
    canvas_t *canvas_context = XMEDIA_NULL;
    rect_size font_rect_size;
    xmedia_u64 vgs_osd_phy_addr = 0;
    xmedia_void *vgs_osd_vir_addr = XMEDIA_NULL;

    font_rect_size = sample_ivp_caculate_font_size(font_type);
    if ((font_rect_size.height == 0) || (font_rect_size.width == 0) || (string_len == 0)) {
        printf("Invalid font size, error! %d-%d|%d\n", font_rect_size.height,font_rect_size.width,string_len);
        return XMEDIA_FAILURE;
    }

    if (bit_map == XMEDIA_NULL) {u
        printf("bit_map can not be NULL, error \n");
        return XMEDIA_FAILURE;
    }

    canvas_context = canvas_init(font_rect_size.width*string_len, font_rect_size.height);
    if (canvas_context == XMEDIA_NULL) {
        printf("No free canvas, error! \n");
        return XMEDIA_FAILURE;
    }

    canvas_clear(canvas_context);
    //string must be end with 0;
    canvas_fill_text(canvas_context, 0, 0, string_content, font_type,
                     TEXT_STYLE_FOREGROUND_WHRITE | TEXT_STYLE_BACKGROUND_BLACK);

    bit_map->u32Width = canvas_context->width;
    bit_map->u32Height = canvas_context->height;
    bit_map->pData = canvas_context->pixels;
    bit_map->enPixelFormat = PIXEL_FORMAT_ARGB_8888;

    ret = XMEDIA_API_SYS_MmzAlloc(&vgs_osd_phy_addr, (XMEDIA_VOID**)&vgs_osd_vir_addr,"BITMAP_FILE_MEM", XMEDIA_NULL,
            bit_map->u32Height*bit_map->u32Width*4);
    if(XMEDIA_SUCCESS != ret) {
        DEMO_PRINT("XMEDIA_API_SYS_MmzAlloc %d failed with %#x!\n", bit_map->u32Height*bit_map->u32Width*4,ret);
        canvas_deinit(canvas_context);
        return XMEDIA_FAILURE;
    }

    if (vgs_osd_vir_addr != XMEDIA_NULL) {
        memcpy(vgs_osd_vir_addr, bit_map->pData,bit_map->u32Height*bit_map->u32Width*4);
        bit_map->pData = CONVERT_TO_32BIT_ADDR(vgs_osd_phy_addr);
    } else {
        canvas_deinit(canvas_context);
        return XMEDIA_FAILURE;
    }

    canvas_deinit(canvas_context);
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_disconstruct_string_bitmap(BITMAP_S  *bitmap_ptr)
{
    xmedia_u64 vgs_osd_phy_addr = 0;

    vgs_osd_phy_addr = CONVERT_TO_64BIT_ADDR(bitmap_ptr->pData);
    XMEDIA_API_SYS_MmzFree(vgs_osd_phy_addr, XMEDIA_NULL);

    memset(bitmap_ptr, 0, sizeof(BITMAP_S));

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_ivp_osd_info_t(VIDEO_FRAME_INFO_S *frame_info, xmedia_s32 s32x, xmedia_s32 s32y, xmedia_char *format_untile_line)
{
    xmedia_s32 ret;
    font_size_e font_size = FONT_SIZE_16;
    VGS_TASK_ATTR_S task;
    VGS_HANDLE handle = -1;
    VGS_ADD_OSD_S vgs_osd;
    xmedia_char str_array[512];
    rect_size font_rect_size;

    font_rect_size = sample_ivp_caculate_font_size(font_size);
    if((font_rect_size.height == 0)||(font_rect_size.width == 0)) {
        return XMEDIA_FAILURE;
    }

    ret = XMEDIA_API_VGS_BeginJob(&handle);
    if (XMEDIA_SUCCESS != ret) {
        DEMO_PRINT("sample_ivp_osd_info_t XMEDIA_API_VGS_BeginJob failed %#x!\n", ret);
        XMEDIA_API_VGS_CancelJob(handle);
        handle = -1;
        return XMEDIA_FAILURE;
    }

    memcpy(&task.stImgIn, frame_info, sizeof(VIDEO_FRAME_INFO_S));
    memcpy(&task.stImgOut, frame_info, sizeof(VIDEO_FRAME_INFO_S));

    memset(str_array, 0, sizeof(str_array));
    memcpy(str_array, format_untile_line, 512);

    ret = sample_construct_string_bitmap(str_array, strlen(str_array), font_size, &g_bitmap);

    memset(&vgs_osd, 0, sizeof(VGS_ADD_OSD_S));
    vgs_osd.stRect.u32Width = g_bitmap.u32Width;
    vgs_osd.stRect.u32Height = g_bitmap.u32Height;
    vgs_osd.u64PhyAddr = CONVERT_TO_64BIT_ADDR(g_bitmap.pData);
    vgs_osd.u32Stride = g_bitmap.u32Width*4;
    vgs_osd.enPixelFmt = g_bitmap.enPixelFormat;

    vgs_osd.stRect.s32X = s32x;
    vgs_osd.stRect.s32X = (vgs_osd.stRect.s32X+1)/2*2;
    vgs_osd.stRect.s32Y = s32y;
    vgs_osd.stRect.s32Y = (vgs_osd.stRect.s32Y+1)/2*2;
    ret = XMEDIA_API_VGS_AddOsdTask(handle, &task, &vgs_osd);
    if (ret != XMEDIA_SUCCESS) {
        DEMO_PRINT("XMEDIA_API_VGS_AddOsdTask fail,Error(%#x)\n", ret);
        XMEDIA_API_VGS_CancelJob(handle);
        handle = -1;
        return XMEDIA_FAILURE;
    }

    ret = XMEDIA_API_VGS_EndJob(handle);
    if (ret != XMEDIA_SUCCESS) {
        DEMO_PRINT("XMEDIA_API_VGS_EndJob fail,Error(%#x)\n", ret);
        XMEDIA_API_VGS_CancelJob(handle);
        handle = -1;
        return XMEDIA_FAILURE;
    }
    sample_disconstruct_string_bitmap(&g_bitmap);

    return XMEDIA_SUCCESS;
}


xmedia_s32 sample_ivp_osd_info(XMEDIA_U32 obj_num, VIDEO_FRAME_INFO_S *frame_info,
                                                xmedia_ai_result_info* ai_result)
{
    xmedia_s32 ret;
    font_size_e font_size = FONT_SIZE_16;
    VGS_TASK_ATTR_S task;
    VGS_HANDLE handle = -1;
    xmedia_u32 i;
    VGS_ADD_OSD_S vgs_osd;
    xmedia_char str_array[SCORE_LEN];
    rect_size font_rect_size;

    font_rect_size = sample_ivp_caculate_font_size(font_size);
    if((font_rect_size.height == 0)||(font_rect_size.width == 0)) {
        return XMEDIA_FAILURE;
    }

    if (0 == obj_num) {
        return XMEDIA_FAILURE;
    }

    obj_num = obj_num > NPU_TARGET_MAX ? NPU_TARGET_MAX : obj_num;

    for (i = 0; i < obj_num; i++) {
        ret = XMEDIA_API_VGS_BeginJob(&handle);
        if (XMEDIA_SUCCESS != ret) {
            DEMO_PRINT("XMEDIA_API_VGS_BeginJob failed %#x!\n", ret);
            XMEDIA_API_VGS_CancelJob(handle);
            handle = -1;
            return XMEDIA_FAILURE;
        }

        memcpy(&task.stImgIn, frame_info, sizeof(VIDEO_FRAME_INFO_S));
        memcpy(&task.stImgOut, frame_info, sizeof(VIDEO_FRAME_INFO_S));

        memset(str_array, 0, sizeof(str_array));
        // snprintf(str_array, sizeof(str_array), "%d|%4.2f|%d|%4.2f", ai_result.targets[i].tracker_id,
        //             ai_result.targets[i].score, ai_result.targets[i].tracker_age, ai_result.targets[i].tracker_iou);

        if(person_face_mask_flag == XMEDIA_TRUE) {
            if(ai_result->targets[i].class_type == XMEDIA_SVP_CLASS_MASK){
                snprintf(str_array, sizeof(str_array),"no mask");
            }else if (ai_result->targets[i].class_type == XMEDIA_SVP_CLASS_NON_MASK) {
                snprintf(str_array, sizeof(str_array),"mask");
            }

        }else if(gesture_recognition_flag == XMEDIA_TRUE) {
            // 手势
            if (ai_result->targets[i].class_type == 0) {
                snprintf(str_array, sizeof(str_array), "ok | id:%d", ai_result->targets[i].tracker_id);
            } else if (ai_result->targets[i].class_type == 1) {
                snprintf(str_array, sizeof(str_array), "thumb | id:%d", ai_result->targets[i].tracker_id);
            } else if (ai_result->targets[i].class_type == 2) {
                snprintf(str_array, sizeof(str_array), "v | id:%d", ai_result->targets[i].tracker_id);
            } else if (ai_result->targets[i].class_type == 3) {
                snprintf(str_array, sizeof(str_array), "no | id:%d", ai_result->targets[i].tracker_id);
            }
        }else if(g_emotion_flag == XMEDIA_TRUE) {
            // 幼儿表情
            if(ai_result->targets[i].class_type == 0) {
                snprintf(str_array, sizeof(str_array), "smile | id:%d", ai_result->targets[i].tracker_id);
            } else if (ai_result->targets[i].class_type == 1) {
                snprintf(str_array, sizeof(str_array), "cry | id:%d", ai_result->targets[i].tracker_id);
            } else {
                snprintf(str_array, sizeof(str_array), "normal | id:%d", ai_result->targets[i].tracker_id);
            }
         } else if(face_capture_mask_flag == XMEDIA_TRUE) {
            if(ai_result->targets[i].class_type == XMEDIA_AI_CLASS_TYPE_MASK) {
                snprintf(str_array, sizeof(str_array), "mask");
            } else {
                snprintf(str_array, sizeof(str_array), "nomask");
            }
         } else {
            if (ai_result->targets[i].class_type == XMEDIA_AI_CLASS_TYPE_BIKE) {
                snprintf(str_array, sizeof(str_array), "type : bike");
            } else if (ai_result->targets[i].class_type == XMEDIA_AI_CLASS_TYPE_BIKER) {
                snprintf(str_array, sizeof(str_array), "type : biker");
            }else if (ai_result->targets[i].class_type == XMEDIA_AI_CLASS_TYPE_MOTOR) {
                snprintf(str_array, sizeof(str_array), "type : motor");
            } else if (ai_result->targets[i].class_type == XMEDIA_AI_CLASS_TYPE_MOTORER){
                snprintf(str_array, sizeof(str_array), "type : motorer");
            } else if (ai_result->targets[i].class_type == XMEDIA_AI_CLASS_TYPE_TRICYCLER){
                snprintf(str_array, sizeof(str_array), "type : tricycler");
            }else if (ai_result->targets[i].class_type == XMEDIA_AI_CLASS_TYPE_TRICYCLE) {
                snprintf(str_array, sizeof(str_array), "type : tricycle");
            } else if (ai_result->targets[i].class_type == XMEDIA_AI_CLASS_TYPE_TRICYCLER) {
                snprintf(str_array, sizeof(str_array), "type : tricycler");
            } else {
                snprintf(str_array, sizeof(str_array), "scr[%.2f]|tp[%d]",ai_result->targets[i].score,
                         ai_result->targets[i].class_type);
            }
        }
        ret = sample_construct_string_bitmap(str_array, strlen(str_array), font_size, &g_bitmap);

        memset(&vgs_osd, 0, sizeof(VGS_ADD_OSD_S));
        vgs_osd.stRect.u32Width = g_bitmap.u32Width;
        vgs_osd.stRect.u32Height = g_bitmap.u32Height;
        vgs_osd.u64PhyAddr = CONVERT_TO_64BIT_ADDR(g_bitmap.pData);
        vgs_osd.u32Stride = g_bitmap.u32Width*4;
        vgs_osd.enPixelFmt = g_bitmap.enPixelFormat;

        vgs_osd.stRect.s32X = (xmedia_s32)ai_result->targets[i].rect.x1;
        vgs_osd.stRect.s32X = (vgs_osd.stRect.s32X+1)/2*2;
        vgs_osd.stRect.s32Y = (xmedia_s32)ai_result->targets[i].rect.y1 - 20;
        if(vgs_osd.stRect.s32Y <= 2){
            vgs_osd.stRect.s32Y = (xmedia_s32)ai_result->targets[i].rect.y1;
        }
        vgs_osd.stRect.s32Y = (vgs_osd.stRect.s32Y+1)/2*2;
        ret = XMEDIA_API_VGS_AddOsdTask(handle, &task, &vgs_osd);
        if (ret != XMEDIA_SUCCESS) {
            DEMO_PRINT("XMEDIA_API_VGS_AddOsdTask fail,Error(%#x)\n", ret);
            XMEDIA_API_VGS_CancelJob(handle);
            handle = -1;
            return XMEDIA_FAILURE;
        }

        ret = XMEDIA_API_VGS_EndJob(handle);
        if (ret != XMEDIA_SUCCESS) {
            DEMO_PRINT("XMEDIA_API_VGS_EndJob fail,Error(%#x)\n", ret);
            XMEDIA_API_VGS_CancelJob(handle);
            handle = -1;
            return XMEDIA_FAILURE;
        }
            sample_disconstruct_string_bitmap(&g_bitmap);
    }

    return XMEDIA_SUCCESS;
}


