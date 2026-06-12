#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "media_comm.h"
#include "media_param.h"

#define MEM_DEV_NODE        "/dev/mem"

#define VI_PIPE_CHN  0

static xmedia_u64 g_point_time = 0;
static xmedia_s32 g_s32MemDev = 0;
static xmedia_s32 g_venc_chn_max_size[VENC_MAX_CHN_NUM] = {0};
static media_comm_isp_ainr_cfg g_isp_ainr_cfg = {0};
static xmedia_char *g_isp_ainr_model[XMEDIA_ISP_PIPE_MAX_NUM][XMEDIA_ISP_AINR_MODEL_MAX_NUM] = {XMEDIA_NULL};

static xmedia_s32 media_comm_isp_get_default_cfg(media_comm_sensor_num sns_num, xmedia_isp_config* isp_cfg)
{
    switch (sns_num)
    {
        case MEDIA_SENSOR0:
        {
            memcpy(isp_cfg, &MEDIA_COMM_ISP_CFG_SENSOR0, sizeof(xmedia_isp_config));
        }
            break;
        default:
            MEDIA_ERR("not support more than 1 sensor type\n");
            return XMEDIA_FAILURE;
    }
    return XMEDIA_SUCCESS;
}

static xmedia_s32 media_comm_vi_get_default_dev_cfg(media_comm_sensor_num sns_num, xmedia_vi_dev_config* vi_dev_cfg)
{
    switch (sns_num)
    {
        case MEDIA_SENSOR0:
        {
            memcpy(vi_dev_cfg, &MEDIA_COMM_VI_DEV_CFG_SENSOR0, sizeof(xmedia_vi_dev_config));
        }
            break;
        default:
            MEDIA_ERR("not support more than 1 sensor type\n");
            return XMEDIA_FAILURE;
    }
    return XMEDIA_SUCCESS;
}

static xmedia_s32 media_comm_vi_get_default_pipe_cfg(media_comm_sensor_num sns_num, xmedia_vi_pipe_config* vi_pipe_cfg)
{
    switch (sns_num)
    {
        case MEDIA_SENSOR0:
        {
            memcpy(vi_pipe_cfg, &MEDIA_COMM_VI_PIPE_CFG_SENSOR0, sizeof(xmedia_vi_pipe_config));
        }
            break;
        default:
            MEDIA_ERR("not support more than 1 sensor type\n");
            return XMEDIA_FAILURE;
    }
    return XMEDIA_SUCCESS;
}

static xmedia_s32 media_comm_vi_get_default_chn_cfg(media_comm_sensor_num sns_num, xmedia_vi_chn_config* vi_chn_cfg)
{
        switch (sns_num)
    {
        case MEDIA_SENSOR0:
        {
            memcpy(vi_chn_cfg, &MEDIA_COMM_VI_CHN_CFG_SENSOR0, sizeof(xmedia_vi_chn_config));
        }
            break;
        default:
            MEDIA_ERR("not support more than 1 sensor type\n");
            return XMEDIA_FAILURE;
    }
    return XMEDIA_SUCCESS;
}

static xmedia_void * memmap(xmedia_u32 phy_addr, xmedia_u32 size)
{
    xmedia_u32 diff;
    xmedia_u32 page_phy;
    xmedia_u32 page_size;
    xmedia_u8 * page_addr;

    page_phy = phy_addr & 0xfffff000;
    diff    = phy_addr - page_phy;

    page_size = ((size + diff - 1) & 0xfffff000) + 0x1000;
    page_addr   = mmap ((xmedia_void *)0, page_size, PROT_READ|PROT_WRITE,
                                    MAP_SHARED, g_s32MemDev, page_phy);
    if (MAP_FAILED == page_addr )
    {
        perror("mmap error\n");
        return NULL;
    }
    return (void *) (page_addr + diff);
}

static xmedia_s32 memunmap(xmedia_void* vir_addr, xmedia_u32 size)
{
    xmedia_u32 page_addr;
    xmedia_u32 page_size;
    xmedia_u32 diff;

    page_addr = (((xmedia_u32)vir_addr) & 0xfffff000);
    diff     = (xmedia_u32)vir_addr - page_addr;
    page_size = ((size + diff - 1) & 0xfffff000) + 0x1000;

    return munmap((xmedia_void*)page_addr, page_size);
}

static xmedia_s32 media_comm_sensor_register(xmedia_s32 pipe, media_comm_sensor_num sns_num)
{
    switch (sns_num)
    {
        case MEDIA_SENSOR0:
        {
#ifdef SMART_SC485SL_MIPI_4M_30FPS_2LANE_12BIT
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc485sl);
#elif SMART_SC485SL_MIPI_4M_30FPS_4LANE_12BIT
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc485sl);
#elif SMART_SC485SL_MIPI_4M_60FPS_4LANE_12BIT
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc485sl);
#elif  SMART_SC235HAI_MIPI_2M_2LANE_30FPS_10BIT
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, sc235hai);
#elif  OMNIVISION_OS04D10_MIPI_4M_30FPS_10BIT
            XMEDIA_SENSOR_REGISTER_DRIVER(pipe, os04d10);
#else
            #error "should define sensor type first"
#endif
        }
            break;
        default:
            MEDIA_ERR("not support more than 1 sensor type\n");
            return XMEDIA_FAILURE;
    }
    return XMEDIA_SUCCESS;
}

static xmedia_s32 media_comm_sensor_unregister(xmedia_s32 pipe, media_comm_sensor_num sns_num)
{
    switch (sns_num)
    {
        case MEDIA_SENSOR0:
        {
#ifdef SMART_SC485SL_MIPI_4M_30FPS_2LANE_12BIT
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, sc485sl);
#elif SMART_SC485SL_MIPI_4M_30FPS_4LANE_12BIT
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, sc485sl);
#elif SMART_SC485SL_MIPI_4M_60FPS_4LANE_12BIT
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, sc485sl);
#elif SMART_SC235HAI_MIPI_2M_2LANE_30FPS_10BIT
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, sc235hai);
#elif OMNIVISION_OS04D10_MIPI_4M_30FPS_10BIT
            XMEDIA_SENSOR_UNREGISTER_DRIVER(pipe, os04d10);

#else
            #error "should define sensor type first"
#endif
        }
            break;
        default:
            MEDIA_ERR("not support more than 1 sensor type\n");
            return XMEDIA_FAILURE;
        }
    return XMEDIA_SUCCESS;
}

xmedia_void media_comm_sensor_get_mipi_lanes(xmedia_sensor_mipi_lanes* mipi_lanes)
{
#ifdef SMART_SC485SL_MIPI_4M_30FPS_2LANE_12BIT
    *mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
#elif SMART_SC485SL_MIPI_4M_30FPS_4LANE_12BIT
    *mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_4L;
#elif SMART_SC485SL_MIPI_4M_60FPS_4LANE_12BIT
    *mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_4L;
#elif  SMART_SC235HAI_MIPI_2M_2LANE_30FPS_10BIT
    *mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
#elif  OMNIVISION_OS04D10_MIPI_4M_30FPS_10BIT
    *mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;

#else
    #error "should define sensor type first"
#endif
}

static xmedia_s32 media_comm_sensor_get_cfg(xmedia_s32 vi_dev, xmedia_sensor_config* sensor_cfg)
{
    switch (vi_dev)
    {
        case 0:
        {
            sensor_cfg->comm_bus.type = XMEDIA_SENSOR_BUS_TYPE_I2C;
            sensor_cfg->comm_bus.i2c_dev = 0;
            sensor_cfg->ctrl_sig.clk_ch = 0;
            sensor_cfg->ctrl_sig.rst_ch = 0;
        }
            break;
        case 1:
        {
            sensor_cfg->comm_bus.type = XMEDIA_SENSOR_BUS_TYPE_I2C;
            sensor_cfg->comm_bus.i2c_dev = 1;
            sensor_cfg->ctrl_sig.clk_ch = 1;
            sensor_cfg->ctrl_sig.rst_ch = 1;
        }
            break;
        case 2:
        {
            sensor_cfg->comm_bus.type = XMEDIA_SENSOR_BUS_TYPE_I2C;
            sensor_cfg->comm_bus.i2c_dev = 5;
            sensor_cfg->ctrl_sig.clk_ch = 2;
            sensor_cfg->ctrl_sig.rst_ch = 2;

        }
            break;
        case 3:
        {
            sensor_cfg->comm_bus.type = XMEDIA_SENSOR_BUS_TYPE_I2C;
            sensor_cfg->comm_bus.i2c_dev = 6;
            sensor_cfg->ctrl_sig.clk_ch = 3;
            sensor_cfg->ctrl_sig.rst_ch = 3;
        }
            break;
        default:
            MEDIA_ERR("curr board not support this dev %d\n", vi_dev);
            return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 media_comm_sensor_get_attr(media_comm_sensor_num sns_num, xmedia_sensor_attr* sns_attr)
{
    switch (sns_num)
    {
        case MEDIA_SENSOR0:
        {
            sns_attr->wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
            sns_attr->width = MEDIA_COMM_ISP_CFG_SENSOR0.size.width;
            sns_attr->height= MEDIA_COMM_ISP_CFG_SENSOR0.size.height;
        }
            break;
        default:
            MEDIA_ERR("not support num [%d] sensor \n",sns_num);
            return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 media_comm_sensor_init(xmedia_s32 vi_dev, xmedia_s32 pipe, media_comm_sensor_num sns_num, xmedia_bool en_quickstart)
{
    xmedia_s32 s32Ret = XMEDIA_SUCCESS;
    xmedia_sensor_config sensor_cfg = {0};
    xmedia_sensor_mipi_lanes mipi_lanes;
    xmedia_sensor_attr sns_attr = {0};

    s32Ret = media_comm_sensor_register(pipe, sns_num);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("media_comm_sensor_register fail\n");
        return s32Ret;
    }

    s32Ret = media_comm_sensor_get_cfg(vi_dev, &sensor_cfg);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("media_comm_sensor_get_cfg fail\n");
        return s32Ret;
    }

    /*sensor quickstart: the mcu already init sensor, cpu not need init again*/
    if (en_quickstart == XMEDIA_TRUE) {
        sensor_cfg.init_mode = XMEDIA_SENSOR_INIT_MODE_SKIP;
    } else {
        sensor_cfg.init_mode = XMEDIA_SENSOR_INIT_MODE_NORMAL;
    }

    s32Ret = xmedia_sensor_init(pipe, &sensor_cfg);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_sensor_init fail");
        return s32Ret;
    }

    s32Ret = media_comm_sensor_get_attr(sns_num, &sns_attr);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("media_comm_sensor_get_attr fail");
        return s32Ret;
    }

    media_comm_sensor_get_mipi_lanes(&mipi_lanes);

    s32Ret = xmedia_sensor_set_mipi_lanes(pipe, mipi_lanes);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_sensor_set_mipi_lanes fail");
        return s32Ret;
    }

    s32Ret = xmedia_sensor_set_attr(pipe, &sns_attr);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_sensor_set_attr fail");
        return s32Ret;
    }

    s32Ret = xmedia_sensor_set_work_mode(pipe, XMEDIA_SENSOR_WORK_MODE_NORMAL);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_sensor_set_work_mode fail");
        return s32Ret;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 media_comm_sensor_exit(xmedia_s32 pipe, media_comm_sensor_num sns_num)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;

    s32Ret = xmedia_sensor_exit(pipe);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_sensor_exit fail");
        return s32Ret;
    }

    media_comm_sensor_unregister(pipe, sns_num);

    return XMEDIA_SUCCESS;
}

/*get isp init param by mcu & set init param*/
xmedia_s32 media_comm_isp_set_init_param(xmedia_s32 pipe)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_sensor_init_param init_param = {0};

    s32Ret = xmedia_isp_get_sensor_init_param(pipe, &init_param);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("get sensor init param by mcu fail\n");
        return s32Ret;
    }

    s32Ret = xmedia_sensor_set_init_param(pipe, &init_param);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("get sensor init param by mcu fail\n");
        return s32Ret;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 media_comm_venc_set_rc_param(xmedia_s32 venc_chn)
{
    xmedia_s32 s32Ret;
    xmedia_venc_rc_param rc_param = {0};
    xmedia_venc_chn_attr chn_attr = {0};

    s32Ret = xmedia_venc_get_chn_attr(venc_chn,&chn_attr);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("xmedia_venc_get_chn_attr fail\n");
        return s32Ret;
    }

    if (chn_attr.venc_attr.en_type != PT_JPEG) {
        s32Ret = xmedia_venc_get_rc_param(venc_chn,&rc_param);
        if (XMEDIA_SUCCESS != s32Ret) {
            MEDIA_ERR("xmedia_venc_get_rc_param fail\n");
            return s32Ret;
        }
    }

    if (VENC_RC_MODE_H264CBR == chn_attr.rc_attr.rc_mode) {
        rc_param.param_h264_cbr.max_re_enc_times = 0;
    } else if (VENC_RC_MODE_H264VBR == chn_attr.rc_attr.rc_mode) {
        rc_param.param_h264_vbr.max_re_enc_times = 0;
    } else if (VENC_RC_MODE_H265CBR == chn_attr.rc_attr.rc_mode) {
        rc_param.param_h265_cbr.max_re_enc_times = 0;
    } else if (VENC_RC_MODE_H265VBR == chn_attr.rc_attr.rc_mode) {
        rc_param.param_h265_vbr.max_re_enc_times = 0;
    } else {
        return XMEDIA_SUCCESS;
    }
    s32Ret = xmedia_venc_set_rc_param(venc_chn,&rc_param);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("xmedia_venc_set_rc_param fail\n");
        return s32Ret;
    }
    return XMEDIA_SUCCESS;
}

static xmedia_s32 media_comm_venc_get_venc_attr(media_comm_venc_cfg* venc_cfg, xmedia_venc_chn_attr* venc_chn_attr)
{
    xmedia_u32 gop = 60;
    xmedia_u32 frame_rate = 0;
    xmedia_u32 stat_time = 0;

    //venc attr cfg
    venc_chn_attr->venc_attr.max_pic_width = venc_cfg->max_pic_width;
    venc_chn_attr->venc_attr.max_pic_height = venc_cfg->max_pic_height;
    venc_chn_attr->venc_attr.pic_width = venc_cfg->max_pic_width;
    venc_chn_attr->venc_attr.pic_height = venc_cfg->max_pic_height;
    venc_chn_attr->venc_attr.profile = venc_cfg->profile;
    venc_chn_attr->venc_attr.en_type = venc_cfg->en_type;
    venc_chn_attr->venc_attr.by_frame = XMEDIA_TRUE;

    if (venc_chn_attr->venc_attr.en_type == PT_MJPEG || venc_chn_attr->venc_attr.en_type == PT_JPEG) {
        venc_chn_attr->venc_attr.stream_buf_size      = ALIGN_UP(venc_chn_attr->venc_attr.pic_width, 16) * ALIGN_UP(venc_chn_attr->venc_attr.pic_height, 16) * 2;
    } else {
        venc_chn_attr->venc_attr.stream_buf_size      = ALIGN_UP(venc_chn_attr->venc_attr.pic_width * venc_chn_attr->venc_attr.pic_height * 4, 64);
    }

    frame_rate = venc_cfg->dst_frame_rate;
    if (VENC_GOPMODE_SMARTP == venc_cfg->venc_gop_attr.gop_mode) {
        stat_time = venc_cfg->venc_gop_attr.smart_p.bg_interval / gop;
    } else {
        stat_time = 2;
    }

    //venc rc attr cfg
    switch (venc_chn_attr->venc_attr.en_type)
    {
        case PT_H265:
        {
            if (VENC_RC_MODE_H265CBR == venc_cfg->rc_mode) {
                xmedia_venc_h265_cbr h265_cbr = {0};
                venc_chn_attr->rc_attr.rc_mode = VENC_RC_MODE_H265CBR;
                h265_cbr.gop                = gop;
                h265_cbr.stat_time          = stat_time; /* stream rate statics time(s) */
                h265_cbr.src_frame_rate     = venc_cfg->src_frame_rate;
                h265_cbr.dst_frame_rate     = venc_cfg->dst_frame_rate; /* target frame rate */
                h265_cbr.bit_rate = (1024 * 2) + venc_chn_attr->venc_attr.pic_width * venc_chn_attr->venc_attr.pic_height * frame_rate / 1024 / 30;
                memcpy(&venc_chn_attr->rc_attr.h265_cbr, &h265_cbr, sizeof(xmedia_venc_h265_cbr));
            } else if (VENC_RC_MODE_H265FIXQP == venc_cfg->rc_mode) {
                xmedia_venc_h265_fixqp h265_fixqp = {0};
                venc_chn_attr->rc_attr.rc_mode = VENC_RC_MODE_H265FIXQP;
                h265_fixqp.gop               = 30;
                h265_fixqp.src_frame_rate    = venc_cfg->src_frame_rate;
                h265_fixqp.dst_frame_rate    = venc_cfg->dst_frame_rate;
                h265_fixqp.i_qp              = 25;
                h265_fixqp.p_qp              = 30;
                h265_fixqp.b_qp              = 32;
                memcpy(&venc_chn_attr->rc_attr.h265_fixqp, &h265_fixqp, sizeof(xmedia_venc_h265_fixqp));
            } else if (VENC_RC_MODE_H265VBR == venc_cfg->rc_mode) {
                xmedia_venc_h265_vbr h265_vbr = {0};
                venc_chn_attr->rc_attr.rc_mode = VENC_RC_MODE_H265VBR;
                h265_vbr.gop             = gop;
                h265_vbr.stat_time       = stat_time;
                h265_vbr.src_frame_rate  = venc_cfg->src_frame_rate;
                h265_vbr.dst_frame_rate  = venc_cfg->dst_frame_rate;
                h265_vbr.max_bit_rate = (1024 * 2) + venc_chn_attr->venc_attr.pic_width * venc_chn_attr->venc_attr.pic_height * frame_rate / 1024 / 30; 
                memcpy(&venc_chn_attr->rc_attr.h265_vbr, &h265_vbr, sizeof(xmedia_venc_h265_vbr));
            } else if (VENC_RC_MODE_H265AVBR == venc_cfg->rc_mode) {
                xmedia_venc_h265_avbr h265_avbr = {0};
                venc_chn_attr->rc_attr.rc_mode = VENC_RC_MODE_H265AVBR;
                h265_avbr.gop            = gop;
                h265_avbr.stat_time      = stat_time;
                h265_avbr.src_frame_rate = venc_cfg->src_frame_rate;
                h265_avbr.dst_frame_rate = venc_cfg->dst_frame_rate;
                h265_avbr.max_bit_rate = (1024 * 2) + venc_chn_attr->venc_attr.pic_width * venc_chn_attr->venc_attr.pic_height * frame_rate / 1024 / 30; 
                memcpy(&venc_chn_attr->rc_attr.h265_avbr, &h265_avbr, sizeof(xmedia_venc_h265_avbr));
            } else if (VENC_RC_MODE_H265QPMAP == venc_cfg->rc_mode) {
                xmedia_venc_h265_qpmap h265_qpmap = {0};
                venc_chn_attr->rc_attr.rc_mode = VENC_RC_MODE_H265QPMAP;
                h265_qpmap.gop             = gop;
                h265_qpmap.stat_time       = stat_time;
                h265_qpmap.src_frame_rate  = venc_cfg->src_frame_rate;
                h265_qpmap.dst_frame_rate  = venc_cfg->dst_frame_rate;
                memcpy(&venc_chn_attr->rc_attr.h265_qpmap, &h265_qpmap, sizeof(xmedia_venc_h265_qpmap));
            } else {
                MEDIA_ERR("rc mode(%d) not support\n",venc_cfg->rc_mode);
                return XMEDIA_FAILURE;
            }
            venc_chn_attr->venc_attr.h265e_attr.rcn_ref_share_buf = venc_cfg->rcn_ref_share_buf;
        }
            break;
        case PT_H264:
        {
            if (VENC_RC_MODE_H264CBR == venc_cfg->rc_mode) {
                xmedia_venc_h264_cbr h264_cbr = {0};
                venc_chn_attr->rc_attr.rc_mode = VENC_RC_MODE_H264CBR;
                h264_cbr.gop                 = gop; /*the interval of IFrame*/
                h264_cbr.stat_time           = stat_time; /* stream rate statics time(s) */
                h264_cbr.src_frame_rate      = venc_cfg->src_frame_rate; /* input (vi) frame rate */
                h264_cbr.dst_frame_rate      = venc_cfg->dst_frame_rate; /* target frame rate */
                h264_cbr.bit_rate = venc_chn_attr->venc_attr.pic_width * venc_chn_attr->venc_attr.pic_height * frame_rate / 1024 / 30; 
                memcpy(&venc_chn_attr->rc_attr.h264_cbr, &h264_cbr, sizeof(xmedia_venc_h264_cbr));
            } else if (VENC_RC_MODE_H264FIXQP == venc_cfg->rc_mode) {
                xmedia_venc_h264_fixqp h264_fixqp = {0};
                venc_chn_attr->rc_attr.rc_mode = VENC_RC_MODE_H264FIXQP;
                h264_fixqp.gop               = 30;
                h264_fixqp.src_frame_rate    = venc_cfg->src_frame_rate;
                h264_fixqp.dst_frame_rate    = venc_cfg->dst_frame_rate;
                h264_fixqp.i_qp              = 25;
                h264_fixqp.p_qp              = 30;
                h264_fixqp.b_qp              = 32;
                memcpy(&venc_chn_attr->rc_attr.h264_fixqp, &h264_fixqp, sizeof(xmedia_venc_h264_fixqp));
            } else if (VENC_RC_MODE_H264VBR == venc_cfg->rc_mode) {
                xmedia_venc_h264_vbr h264_vbr = {0};
                venc_chn_attr->rc_attr.rc_mode = VENC_RC_MODE_H264VBR;
                h264_vbr.gop             = gop;
                h264_vbr.stat_time       = stat_time;
                h264_vbr.src_frame_rate  = venc_cfg->src_frame_rate;
                h264_vbr.dst_frame_rate  = venc_cfg->dst_frame_rate;
                h264_vbr.max_bit_rate = (1024 * 2) + venc_chn_attr->venc_attr.pic_width * venc_chn_attr->venc_attr.pic_height * frame_rate / 1024 / 30; 
                memcpy(&venc_chn_attr->rc_attr.h264_vbr, &h264_vbr, sizeof(xmedia_venc_h264_vbr));
            } else if (VENC_RC_MODE_H264AVBR == venc_cfg->rc_mode) {
                xmedia_venc_h264_avbr h264_avbr = {0};
                venc_chn_attr->rc_attr.rc_mode = VENC_RC_MODE_H264AVBR;
                h264_avbr.gop            = gop;
                h264_avbr.stat_time      = stat_time;
                h264_avbr.src_frame_rate = venc_cfg->src_frame_rate;
                h264_avbr.dst_frame_rate = venc_cfg->dst_frame_rate;
                h264_avbr.max_bit_rate = (1024 * 2) + venc_chn_attr->venc_attr.pic_width * venc_chn_attr->venc_attr.pic_height * frame_rate / 1024 / 30; 
                memcpy(&venc_chn_attr->rc_attr.h264_avbr, &h264_avbr, sizeof(xmedia_venc_h264_avbr));
            } else if (VENC_RC_MODE_H264QPMAP == venc_cfg->rc_mode) {
                xmedia_venc_h264_qpmap h264_qpmap = {0};
                venc_chn_attr->rc_attr.rc_mode = VENC_RC_MODE_H264QPMAP;
                h264_qpmap.gop             = gop;
                h264_qpmap.stat_time       = stat_time;
                h264_qpmap.src_frame_rate  = venc_cfg->src_frame_rate;
                h264_qpmap.dst_frame_rate  = venc_cfg->dst_frame_rate;
                memcpy(&venc_chn_attr->rc_attr.h264_qpmap, &h264_qpmap, sizeof(xmedia_venc_h264_qpmap));
            } else {
                MEDIA_ERR("rc mode(%d) not support\n",venc_cfg->rc_mode);
                return XMEDIA_FAILURE;
            }
            venc_chn_attr->venc_attr.h265e_attr.rcn_ref_share_buf = venc_cfg->rcn_ref_share_buf;
        }
            break;
        case PT_MJPEG:
        {
            if (VENC_RC_MODE_MJPEGFIXQP == venc_cfg->rc_mode) {
                xmedia_venc_mjpeg_fixqp mjpege_fixqp = {0};
                venc_chn_attr->rc_attr.rc_mode = VENC_RC_MODE_MJPEGFIXQP;
                mjpege_fixqp.q_factor         = 95;
                mjpege_fixqp.src_frame_rate   = venc_cfg->src_frame_rate;
                mjpege_fixqp.dst_frame_rate   = venc_cfg->dst_frame_rate;
                memcpy(&venc_chn_attr->rc_attr.mjpeg_fixqp, &mjpege_fixqp,sizeof(xmedia_venc_mjpeg_fixqp));
            } else if (VENC_RC_MODE_MJPEGCBR == venc_cfg->rc_mode) {
                xmedia_venc_mjpeg_cbr mjpeg_cbr = {0};
                venc_chn_attr->rc_attr.rc_mode = VENC_RC_MODE_MJPEGCBR;
                mjpeg_cbr.stat_time           = stat_time;
                mjpeg_cbr.src_frame_rate      = venc_cfg->src_frame_rate;
                mjpeg_cbr.dst_frame_rate      = venc_cfg->dst_frame_rate;
                mjpeg_cbr.bit_rate = venc_chn_attr->venc_attr.pic_width * venc_chn_attr->venc_attr.pic_height * frame_rate / 1024 / 30 * 5; 
                memcpy(&venc_chn_attr->rc_attr.mjpeg_cbr, &mjpeg_cbr,sizeof(xmedia_venc_mjpeg_cbr));
            } else if (VENC_RC_MODE_MJPEGVBR == venc_cfg->rc_mode) {
                xmedia_venc_mjpeg_vbr mjpeg_vbr = {0};
                venc_chn_attr->rc_attr.rc_mode = VENC_RC_MODE_MJPEGVBR;
                mjpeg_vbr.stat_time       = stat_time;
                mjpeg_vbr.src_frame_rate  = venc_cfg->src_frame_rate;
                mjpeg_vbr.dst_frame_rate  = 5;
                mjpeg_vbr.max_bit_rate = venc_chn_attr->venc_attr.pic_width * venc_chn_attr->venc_attr.pic_height * frame_rate / 1024 / 30 * 5; 
                memcpy(&venc_chn_attr->rc_attr.mjpeg_vbr, &mjpeg_vbr,sizeof(xmedia_venc_mjpeg_vbr));
            } else {
                MEDIA_ERR("rc mode(%d) not support\n",venc_cfg->rc_mode);
                return XMEDIA_FAILURE;
            }
        }
            break;
        default:
            MEDIA_ERR("not support the payload type !\n");
            return XMEDIA_FAILURE;
    }

    memcpy(&venc_chn_attr->gop_attr, &venc_cfg->venc_gop_attr,sizeof(xmedia_venc_gop_attr));
    return XMEDIA_SUCCESS;
}

xmedia_s32 media_comm_sys_init(media_comm_sys_config* sys_cfg)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;

    s32Ret = xmedia_sys_exit();
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_sys_exit fail");
        return s32Ret;
    }

    s32Ret = xmedia_vb_exit();
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_vb_exit fail");
        return s32Ret;
    }

    s32Ret = xmedia_vb_init(&sys_cfg->vb_conf);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_vb_init fail");
        return s32Ret;
    }

    s32Ret = xmedia_sys_init(&sys_cfg->sys_conf);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_sys_init fail");
        xmedia_vb_exit();
        return s32Ret;
    }

    xmedia_sys_get_cur_pts(&g_point_time);
    MEDIA_PRT("sys init success point time = %lld\n",g_point_time);

    return XMEDIA_SUCCESS;
}

xmedia_s32 media_comm_sys_exit()
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;

    s32Ret = xmedia_sys_exit();
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_sys_exit fail");
        return s32Ret;
    }

    s32Ret = xmedia_vb_exit();
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_vb_exit fail");
        return s32Ret;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 media_comm_media_init()
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;

    s32Ret = xmedia_vi_init();
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_vi_init fail");
        return s32Ret;
    }

    s32Ret = xmedia_vpss_init();
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_vpss_init fail");
        return s32Ret;
    }

    s32Ret = xmedia_venc_init();
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_venc_init fail");
        return s32Ret;
    }

    s32Ret = xmedia_vgs_init();
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_vgs_init fail");
        return s32Ret;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 media_comm_media_exit()
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;

    s32Ret = xmedia_vi_exit();
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_vi_exit fail");
        return s32Ret;
    }

    s32Ret = xmedia_vpss_exit();
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_vpss_exit fail");
        return s32Ret;
    }

    s32Ret = xmedia_venc_exit();
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_venc_exit fail");
        return s32Ret;
    }

    xmedia_vgs_exit();

    return XMEDIA_SUCCESS;
}

xmedia_s32 media_comm_mem_open(xmedia_void)
{
    if (g_s32MemDev <= 0)
    {
        g_s32MemDev = open (MEM_DEV_NODE, O_CREAT|O_RDWR|O_SYNC);
        if (g_s32MemDev <= 0)
        {
            return -1;
        }
    }
    return 0;
}

xmedia_void media_comm_mem_close(xmedia_void)
{
    close(g_s32MemDev);
}

xmedia_void media_comm_set_reg(xmedia_u32 addr,xmedia_u32 value)
{
    xmedia_u32 *port_vir_addr;
    port_vir_addr  = memmap(addr,sizeof(value));
    *port_vir_addr = value;
    memunmap(port_vir_addr,sizeof(value));
}

xmedia_s32 media_comm_get_reg(xmedia_u32 u32Addr)
{
    xmedia_s32 s32Value = 0;
    xmedia_u32 *pPortVirAddr;
    pPortVirAddr =   memmap(u32Addr,sizeof(u32Addr) );
    s32Value = *pPortVirAddr;
    memunmap(pPortVirAddr,sizeof(u32Addr));
    return s32Value;
}


xmedia_u64 media_comm_get_cur_time()
{
    xmedia_u64 cur_time = 0;
    xmedia_sys_get_cur_pts(&cur_time);
    return (cur_time);
}

xmedia_s32 media_comm_vi_bind_vpss(xmedia_s32 vi_pipe, xmedia_s32 vi_chn, xmedia_s32 vpss_pipe, xmedia_s32 vpss_ichn)
{
    xmedia_chn_info src_chn;
    xmedia_chn_info dst_chn;

    src_chn.mod_id   = MOD_ID_VI;
    src_chn.dev_id  = vi_pipe;
    src_chn.chn_id  = vi_chn;

    dst_chn.mod_id  = MOD_ID_VPSS;
    dst_chn.dev_id = vpss_pipe;
    dst_chn.chn_id = vpss_ichn;

    return xmedia_sys_bind(&src_chn, &dst_chn);
}

xmedia_s32 media_comm_vi_unbind_vpss(xmedia_s32 vi_pipe, xmedia_s32 vi_chn, xmedia_s32 vpss_pipe, xmedia_s32 vpss_ichn)
{
    xmedia_chn_info src_chn;
    xmedia_chn_info dst_chn;

    src_chn.mod_id   = MOD_ID_VI;
    src_chn.dev_id  = vi_pipe;
    src_chn.chn_id  = vi_chn;

    dst_chn.mod_id  = MOD_ID_VPSS;
    dst_chn.dev_id = vpss_pipe;
    dst_chn.chn_id = vpss_ichn;

    return xmedia_sys_unbind(&src_chn, &dst_chn);
}

xmedia_s32 media_comm_sys_vpss_bind_venc(xmedia_s32 vpss_pipe, xmedia_s32 vpss_ochn, xmedia_s32 venc_chn)
{
    xmedia_chn_info src_chn;
    xmedia_chn_info dst_chn;

    src_chn.mod_id = MOD_ID_VPSS;
    src_chn.dev_id = vpss_pipe;
    src_chn.chn_id = vpss_ochn;

    dst_chn.mod_id = MOD_ID_VENC;
    dst_chn.dev_id = 0;
    dst_chn.chn_id = venc_chn;

    return xmedia_sys_bind(&src_chn, &dst_chn);
}

xmedia_s32 media_comm_sys_vpss_unbind_venc(xmedia_s32 vpss_pipe, xmedia_s32 vpss_ochn, xmedia_s32 venc_chn)
{
    xmedia_chn_info src_chn;
    xmedia_chn_info dst_chn;

    src_chn.mod_id = MOD_ID_VPSS;
    src_chn.dev_id = vpss_pipe;
    src_chn.chn_id = vpss_ochn;

    dst_chn.mod_id = MOD_ID_VENC;
    dst_chn.dev_id = 0;
    dst_chn.chn_id = venc_chn;

    return xmedia_sys_unbind(&src_chn, &dst_chn);
}

xmedia_s32 media_comm_isp_get_sensor_info(media_comm_sensor_num sns_num, media_comm_isp_sensor_info *sns_info)
{
    if (sns_num == MEDIA_SENSOR0) {
        sns_info->sns_max_size.width = MEDIA_COMM_ISP_CFG_SENSOR0.size.width;
        sns_info->sns_max_size.height = MEDIA_COMM_ISP_CFG_SENSOR0.size.height;
        sns_info->frame_rate = MEDIA_COMM_ISP_CFG_SENSOR0.fps;
    } else {
        MEDIA_ERR("not support double sns\n");
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 media_comm_isp_set_sensor_info(media_comm_sensor_num sns_num, media_comm_isp_sensor_info *sns_info)
{
    if (sns_num == MEDIA_SENSOR0) {
        MEDIA_COMM_ISP_CFG_SENSOR0.size.width = sns_info->sns_max_size.width;
        MEDIA_COMM_ISP_CFG_SENSOR0.size.height = sns_info->sns_max_size.height;
        MEDIA_COMM_ISP_CFG_SENSOR0.fps = sns_info->frame_rate;
    } else {
        MEDIA_ERR("not support double sns\n");
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 media_comm_isp_init(media_comm_isp_cfg media_isp_cfg)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_isp_config isp_cfg = {0};
    xmedia_isp_bnr_attr bnr_attr = {0};
    xmedia_isp_stnr_attr stnr_attr = {0};
    xmedia_vi_pipe_config vi_pipe_cfg = {0};

    s32Ret = media_comm_sensor_init(media_isp_cfg.vi_dev, media_isp_cfg.isp_pipe, media_isp_cfg.sns_num, media_isp_cfg.en_quickstart);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("media_comm_sensor_init fail");
        return s32Ret;
    }

    s32Ret = xmedia_ae_register(media_isp_cfg.isp_pipe);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("xmedia_ae_register fail\n");
        return s32Ret;
    }

    s32Ret = xmedia_awb_register(media_isp_cfg.isp_pipe);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("xmedia_awb_register fail\n");
           return s32Ret;
     }

    s32Ret = media_comm_isp_get_default_cfg(media_isp_cfg.sns_num, &isp_cfg);
    if (XMEDIA_SUCCESS != s32Ret) {
         MEDIA_ERR("media_comm_get_isp_default_cfg fail\n");
         return XMEDIA_FAILURE;
      }

    if (media_isp_cfg.en_quickstart == XMEDIA_TRUE) {
        s32Ret = media_comm_isp_set_init_param(media_isp_cfg.isp_pipe);
            if (XMEDIA_SUCCESS != s32Ret) {
                MEDIA_ERR("media_comm_isp_set_init_paramfail\n");
                return s32Ret;
        }
    }

    s32Ret = xmedia_isp_init(media_isp_cfg.isp_pipe, &isp_cfg);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("xmedia_isp_init fail\n");
        return s32Ret;
    }

    media_comm_vi_get_default_pipe_cfg(media_isp_cfg.isp_pipe, &vi_pipe_cfg);

    s32Ret = xmedia_isp_get_bnr_attr(media_isp_cfg.isp_pipe, &bnr_attr);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("xmedia_isp_get_bnr_attr fail\n");
        return s32Ret;
    }

    bnr_attr.enable = XMEDIA_TRUE;
    bnr_attr.ainr_pos_switch = XMEDIA_ISP_BNR_AINR_POS_OFF;
    bnr_attr.tnr_enable = vi_pipe_cfg.bnr_attr.bnr_en;
    s32Ret = xmedia_isp_set_bnr_attr(media_isp_cfg.isp_pipe, &bnr_attr);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("xmedia_isp_set_bnr_attr fail\n");
        return s32Ret;
    }

    s32Ret = xmedia_isp_get_stnr_attr(media_isp_cfg.isp_pipe, &stnr_attr);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("xmedia_isp_get_stnr_attr fail\n");
        return s32Ret;
    }

    stnr_attr.stnr_enable = XMEDIA_TRUE;
    stnr_attr.ynr_enable = XMEDIA_TRUE;
    stnr_attr.tnr_enable = vi_pipe_cfg.stnr_attr.stnr_en;
    stnr_attr.cnr_enable = XMEDIA_TRUE;
    s32Ret = xmedia_isp_set_stnr_attr(media_isp_cfg.isp_pipe, &stnr_attr);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("xmedia_isp_set_stnr_attr fail\n");
        return s32Ret;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 media_comm_isp_exit(xmedia_s32 pipe, media_comm_sensor_num sns_num)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;

    s32Ret = xmedia_isp_exit(pipe);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("xmedia_isp_exit fail\n");
        return s32Ret;
    }

    s32Ret = xmedia_awb_unregister(pipe);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("xmedia_awb_unregister fail\n");
        return s32Ret;
    }

    s32Ret = xmedia_ae_unregister(pipe);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("xmedia_ae_unregister fail\n");
        return s32Ret;
    }

    s32Ret = media_comm_sensor_exit(pipe, sns_num);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("media_comm_sensor_exit fail\n");
        return s32Ret;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 media_comm_isp_start(xmedia_s32 pipe, xmedia_bool en_start)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;

    s32Ret = xmedia_isp_start(pipe);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("xmedia_isp_start fail\n");
        return s32Ret;
    }

    if (en_start == XMEDIA_FALSE) {
        s32Ret = xmedia_sensor_start(pipe);
        if (XMEDIA_SUCCESS != s32Ret) {
            MEDIA_ERR("xmedia_isp_start fail\n");
            return s32Ret;
        }
    } else {
        s32Ret = xmedia_sensor_resume(pipe);
        if (XMEDIA_SUCCESS != s32Ret) {
            MEDIA_ERR("xmedia_isp_start fail\n");
            return s32Ret;
        }
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 media_comm_isp_stop(xmedia_s32 pipe, xmedia_bool en_start)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;

    s32Ret = xmedia_isp_stop(pipe);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("xmedia_isp_stop fail\n");
        return s32Ret;
    }

    if (en_start == XMEDIA_FALSE) {
        s32Ret = xmedia_sensor_stop(pipe);
        if (XMEDIA_SUCCESS != s32Ret) {
            MEDIA_ERR("xmedia_sensor_stop fail\n");
            return s32Ret;
        }
    } else {
        s32Ret = xmedia_sensor_standby(pipe);
        if (XMEDIA_SUCCESS != s32Ret) {
            MEDIA_ERR("xmedia_sensor_standby fail\n");
            return s32Ret;
        }
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 media_comm_isp_get_mipi_rate(xmedia_s32 isp_pipe)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_sensor_property property = {0};

    s32Ret = xmedia_sensor_get_property(isp_pipe, &property);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("get mipi rate fail \n");
        return 0;
    }

    return property.bit_rate;
}

xmedia_s32 media_comm_vi_start(media_comm_vi_cfg* vi_cfg)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_s32 i = 0;
    xmedia_s32 j = 0;

    /*set vi default cfg*/
    for (i = 0; i < vi_cfg->vi_dev_work_num; i++) {
        s32Ret = media_comm_vi_get_default_dev_cfg(i, &vi_cfg->vi_dev_cfg[i]);
        if (XMEDIA_SUCCESS != s32Ret) {
            MEDIA_PRT("media_comm_vi_get_default_dev_cfg\n");
            return XMEDIA_FAILURE;
        }

        for (j = 0;j < vi_cfg->vi_pipe_param[i].vi_pipe_work_num; j++) {
            s32Ret = media_comm_vi_get_default_pipe_cfg(i, &vi_cfg->vi_pipe_param[i].vi_pipe_info[j].vi_pipe_cfg);
            s32Ret |= media_comm_vi_get_default_chn_cfg(i, &vi_cfg->vi_pipe_param[i].vi_pipe_info[j].vi_chn_cfg);
            if (XMEDIA_SUCCESS != s32Ret) {
                MEDIA_PRT("media_comm_vi_get_default_pipe_cfg\n");
                return XMEDIA_FAILURE;
            }
        }
    }

    //dev init & dev bind pipe
    for (i = 0; i < vi_cfg->vi_dev_work_num; i++) {
        s32Ret = xmedia_vi_set_dev_config(vi_cfg->vi_dev[i], &vi_cfg->vi_dev_cfg[i]);
        if (XMEDIA_SUCCESS != s32Ret) {
            MEDIA_ERR("xmedia_vi_set_dev_config fail\n");
            return s32Ret;
        }

        if (vi_cfg->mipi_cfg[i].mipi_rate != 0) {
            s32Ret = xmedia_vi_set_dev_mipi_phy_config(vi_cfg->vi_dev[i], &vi_cfg->mipi_cfg[i]);
            if (XMEDIA_SUCCESS != s32Ret) {
                MEDIA_ERR("xmedia_vi_set_dev_mipi_phy_config fail\n");
                return s32Ret;
            }
        }

       /*vi quickstart: mcu already init mipi, cpu not need init again*/
       if(vi_cfg->en_quickstart[vi_cfg->vi_dev[i]] == XMEDIA_TRUE) {
           s32Ret = xmedia_vi_enable_dev_quick_start(vi_cfg->vi_dev[i]);
           if (XMEDIA_SUCCESS != s32Ret) {
               MEDIA_ERR("set vi quickstart fail\n");
               return s32Ret;
           }
       }

        s32Ret = xmedia_vi_enable_dev(vi_cfg->vi_dev[i]);
        if (XMEDIA_SUCCESS != s32Ret) {
            MEDIA_ERR("xmedia_vi_enable_dev fail\n");
            return s32Ret;
        }

        for (j = 0; j < vi_cfg->vi_pipe_param[i].vi_pipe_work_num; j++) {
            s32Ret = xmedia_vi_set_dev_bind_pipe(vi_cfg->vi_dev[i], vi_cfg->vi_pipe_param[i].vi_pipe[j]);
            if (XMEDIA_SUCCESS != s32Ret) {
                MEDIA_ERR("xmedia_vi_set_dev_bind_pipe fail\n");
                goto error0;
            }
        }
    }

    //pipe init grp mode
    for (i = 0; i < vi_cfg->vi_grp_param.vi_grp_work_num; i++) {
        s32Ret = xmedia_vi_set_grp_config(vi_cfg->vi_grp_param.vi_grp[i], &vi_cfg->vi_grp_param.vi_grp_cfg[i]);
        if (XMEDIA_SUCCESS != s32Ret) {
            MEDIA_ERR("xmedia_vi_set_grp_config fail\n");
            return s32Ret;
        }
    }

   //set pipe cfg
   for (i = 0; i < vi_cfg->vi_dev_work_num; i++) {
       for (j = 0; j < vi_cfg->vi_pipe_param[i].vi_pipe_work_num; j++) {
           s32Ret = xmedia_vi_create_pipe(vi_cfg->vi_pipe_param[i].vi_pipe[j], &vi_cfg->vi_pipe_param[i].vi_pipe_info[j].vi_pipe_cfg);
           if (XMEDIA_SUCCESS != s32Ret) {
               MEDIA_ERR("xmedia_vi_create_pipe fail\n");
               goto error0;
           }

           if (vi_cfg->vi_pipe_param[i].vi_pipe_info[j].en_vi_cap_lowdelay == XMEDIA_TRUE) {
               s32Ret = xmedia_vi_set_pipe_low_delay_attr(vi_cfg->vi_pipe_param[i].vi_pipe[j], &vi_cfg->vi_pipe_param[i].vi_pipe_info[j].vi_cap_lowdelay);
               if (XMEDIA_SUCCESS != s32Ret) {
                   MEDIA_ERR("xmedia_vi_set_pipe_low_delay_attr fail\n");
                   goto error0;
               }
           }

            s32Ret = xmedia_vi_start_pipe(vi_cfg->vi_pipe_param[i].vi_pipe[j]);
            if (XMEDIA_SUCCESS != s32Ret) {
                MEDIA_ERR("xmedia_vi_start_pipe fail\n");
                goto error1;
            }

           s32Ret = xmedia_vi_set_chn_config(vi_cfg->vi_pipe_param[i].vi_pipe[j], VI_PIPE_CHN, &vi_cfg->vi_pipe_param[i].vi_pipe_info[j].vi_chn_cfg);
           if (XMEDIA_SUCCESS != s32Ret) {
               MEDIA_ERR("xmedia_vi_set_chn_config fail\n");
               goto error1;
           }

           if (vi_cfg->vi_pipe_param[i].vi_pipe_info[j].en_vi_proc_lowdelay == XMEDIA_TRUE) {
               s32Ret = xmedia_vi_set_chn_low_delay_attr(vi_cfg->vi_pipe_param[i].vi_pipe[j], VI_PIPE_CHN, &vi_cfg->vi_pipe_param[i].vi_pipe_info[j].vi_proc_lowdelay);
               if (XMEDIA_SUCCESS != s32Ret) {
                   MEDIA_ERR("xmedia_vi_set_chn_low_delay_attr fail\n");
                   goto error1;
               }
           }

            s32Ret = xmedia_vi_enable_chn(vi_cfg->vi_pipe_param[i].vi_pipe[j], VI_PIPE_CHN);
            if (XMEDIA_SUCCESS != s32Ret) {
                 MEDIA_ERR("xmedia_vi_enable_chn fail\n");
                goto error2;
            }
        }
    }

    return XMEDIA_SUCCESS;

error2:
    for (i = 0; i < vi_cfg->vi_dev_work_num; i++) {
        for (j = 0; j < vi_cfg->vi_pipe_param[i].vi_pipe_work_num; j++) {
            s32Ret = xmedia_vi_disable_chn(vi_cfg->vi_pipe_param[i].vi_pipe[j], VI_PIPE_CHN);
            if (XMEDIA_SUCCESS != s32Ret) {
                MEDIA_ERR("xmedia_vi_disable_chn fail\n");
                return s32Ret;
            }
        }
    }

error1:
    for (i = 0; i < vi_cfg->vi_dev_work_num; i++) {
        for (j = 0; j < vi_cfg->vi_pipe_param[i].vi_pipe_work_num; j++) {
            s32Ret = xmedia_vi_stop_pipe(vi_cfg->vi_pipe_param[i].vi_pipe[j]);
            if (XMEDIA_SUCCESS != s32Ret) {
                MEDIA_ERR("xmedia_vi_stop_pipe fail\n");
                return s32Ret;
            }

            s32Ret = xmedia_vi_destroy_pipe(vi_cfg->vi_pipe_param[i].vi_pipe[j]);
            if (XMEDIA_SUCCESS != s32Ret) {
                MEDIA_ERR("xmedia_vi_stop_pipe fail\n");
                return s32Ret;
            }
        }
    }

error0:
    for (i = 0; i < vi_cfg->vi_dev_work_num; i++) {
        s32Ret = xmedia_vi_disable_dev(vi_cfg->vi_dev[i]);
        if (XMEDIA_SUCCESS != s32Ret) {
            MEDIA_ERR("xmedia_vi_disable_dev fail\n");
            return s32Ret;
        }
    }

    return XMEDIA_FAILURE;
}

xmedia_s32 media_comm_vi_stop(media_comm_vi_cfg* vi_cfg)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_s32 i = 0;
    xmedia_s32 j = 0;

    for (i = 0; i < vi_cfg->vi_dev_work_num; i++) {
        for (j = 0; j < vi_cfg->vi_pipe_param[i].vi_pipe_work_num; j++) {
            s32Ret = xmedia_vi_disable_chn(vi_cfg->vi_pipe_param[i].vi_pipe[j], VI_PIPE_CHN);
            if (XMEDIA_SUCCESS != s32Ret) {
                MEDIA_ERR("xmedia_vi_disable_chn fail\n");
                return s32Ret;
            }

            s32Ret = xmedia_vi_stop_pipe(vi_cfg->vi_pipe_param[i].vi_pipe[j]);
            if (XMEDIA_SUCCESS != s32Ret) {
                MEDIA_ERR("xmedia_vi_stop_pipe fail\n");
                return s32Ret;
            }

            s32Ret = xmedia_vi_destroy_pipe(vi_cfg->vi_pipe_param[i].vi_pipe[j]);
            if (XMEDIA_SUCCESS != s32Ret) {
                MEDIA_ERR("xmedia_vi_stop_pipe fail\n");
                return s32Ret;
            }
        }
    }

    for (i = 0; i < vi_cfg->vi_dev_work_num; i++) {
        s32Ret = xmedia_vi_disable_dev(vi_cfg->vi_dev[i]);
        if (XMEDIA_SUCCESS != s32Ret) {
            MEDIA_ERR("xmedia_vi_disable_dev fail\n");
            return s32Ret;
        }
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 media_comm_vpss_start(media_comm_vpss_cfg* vpss_cfg)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;

    xmedia_s32 i = 0;
    xmedia_s32 j = 0;

    for (i = 0; i < vpss_cfg->vpss_pipe_work_num; i++) {
        s32Ret = xmedia_vpss_create_pipe(vpss_cfg->vpss_pipe[i], &vpss_cfg->pipe_cfg[i]);
        if (XMEDIA_SUCCESS != s32Ret) {
            MEDIA_ERR("xmedia_vpss_create_pipe fail\n");
            goto error0;
        }

        for (j = 0; j < vpss_cfg->vpss_ochn_info[i].vpss_ochn_work_num; j++) {
            s32Ret = xmedia_vpss_set_ochn_config(vpss_cfg->vpss_pipe[i], vpss_cfg->vpss_ochn_info[i].vpss_ochn[j], &vpss_cfg->vpss_ochn_info[i].vpss_ochn_cfg[j]);
            if (XMEDIA_SUCCESS != s32Ret) {
                MEDIA_ERR("xmedia_vpss_set_ochn_config fail\n");
                goto error0;
            }

            if (vpss_cfg->vpss_ochn_info[i].vpss_lowdelay_cfg[j].enable == XMEDIA_TRUE)
            {
                s32Ret = xmedia_vpss_set_ochn_low_delay_attr(vpss_cfg->vpss_pipe[i], vpss_cfg->vpss_ochn_info[i].vpss_ochn[j], &vpss_cfg->vpss_ochn_info[i].vpss_lowdelay_cfg[j]);
                if (XMEDIA_SUCCESS != s32Ret) {
                    MEDIA_ERR("xmedia_vpss_set_ochn_low_delay_attr fail\n");
                    goto error0;
                }
            }

            if (vpss_cfg->vpss_ochn_info[i].vpss_wrap_cfg[j].enable == XMEDIA_TRUE)
            {
                s32Ret = xmedia_vpss_set_ochn_wrap_attr(vpss_cfg->vpss_pipe[i], vpss_cfg->vpss_ochn_info[i].vpss_ochn[j], &vpss_cfg->vpss_ochn_info[i].vpss_wrap_cfg[j]);
                if (XMEDIA_SUCCESS != s32Ret) {
                    MEDIA_ERR("xmedia_vpss_set_ochn_wrap_attr fail\n");
                    goto error0;
                }
            }
        }
    }

    for (i = 0; i < vpss_cfg->vpss_pipe_work_num; i++) {
        for (j = 0; j < vpss_cfg->vpss_ochn_info[i].vpss_ochn_work_num; j++) {
            s32Ret = xmedia_vpss_enable_ochn(vpss_cfg->vpss_pipe[i], vpss_cfg->vpss_ochn_info[i].vpss_ochn[j]);
            if (XMEDIA_SUCCESS != s32Ret) {
                MEDIA_ERR("xmedia_vpss_enable_ochn fail\n");
                goto error1;
            }
        }

        s32Ret = xmedia_vpss_start_pipe(vpss_cfg->vpss_pipe[i]);
        if (XMEDIA_SUCCESS != s32Ret) {
            MEDIA_ERR("xmedia_vpss_start_pipe fail\n");
            goto error2;
        }
    }

    return XMEDIA_SUCCESS;
error2:
    for (i = 0; i < vpss_cfg->vpss_pipe_work_num; i++) {
       s32Ret = xmedia_vpss_stop_pipe(vpss_cfg->vpss_pipe[i]);
        if (XMEDIA_SUCCESS != s32Ret) {
            MEDIA_ERR("xmedia_vpss_stop_pipe fail\n");
            return s32Ret;
        }
    }
error1:
    for (i = 0; i < vpss_cfg->vpss_pipe_work_num; i++) {
        for (j = 0; j < vpss_cfg->vpss_ochn_info[i].vpss_ochn_work_num; j++) {
            s32Ret = xmedia_vpss_disable_ochn(vpss_cfg->vpss_pipe[i], vpss_cfg->vpss_ochn_info[i].vpss_ochn[j]);
            if (XMEDIA_SUCCESS != s32Ret) {
                MEDIA_ERR("xmedia_vpss_disable_ochn fail\n");
                return s32Ret;
            }
        }
    }
error0:
    for (i = 0; i < vpss_cfg->vpss_pipe_work_num; i++) {
        s32Ret = xmedia_vpss_destroy_pipe(vpss_cfg->vpss_pipe[i]);
        if (XMEDIA_SUCCESS != s32Ret) {
            MEDIA_ERR("xmedia_vpss_stop_pipe fail\n");
            return s32Ret;
        }
    }

    return XMEDIA_FAILURE;
}

xmedia_s32 media_comm_vpss_stop(media_comm_vpss_cfg* vpss_cfg)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_s32 i = 0;
    xmedia_s32 j = 0;

    for (i = 0; i < vpss_cfg->vpss_pipe_work_num; i++) {
        s32Ret = xmedia_vpss_stop_pipe(vpss_cfg->vpss_pipe[i]);
        if (XMEDIA_SUCCESS != s32Ret) {
            MEDIA_ERR("xmedia_vpss_stop_pipe fail\n");
            return s32Ret;
        }

        for (j = 0; j < vpss_cfg->vpss_ochn_info[i].vpss_ochn_work_num; j++) {
            s32Ret = xmedia_vpss_disable_ochn(vpss_cfg->vpss_pipe[i], vpss_cfg->vpss_ochn_info[i].vpss_ochn[j]);
            if (XMEDIA_SUCCESS != s32Ret) {
                MEDIA_ERR("xmedia_vpss_disable_ochn fail\n");
                return s32Ret;
            }
        }

        s32Ret = xmedia_vpss_destroy_pipe(vpss_cfg->vpss_pipe[i]);
        if (XMEDIA_SUCCESS != s32Ret) {
            MEDIA_ERR("xmedia_vpss_stop_pipe fail\n");
            return s32Ret;
        }
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 media_comm_venc_start(xmedia_s32 venc_chn, media_comm_venc_cfg* venc_cfg)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;

    xmedia_venc_chn_attr venc_chn_attr = {0};
    xmedia_venc_recv_pic_param recv_param = {0};
    xmedia_venc_h264_vui stH264Vui = {0};
    xmedia_venc_h265_vui stH265Vui = {0};

    recv_param.recv_pic_num = -1;

    s32Ret = media_comm_venc_get_venc_attr(venc_cfg, &venc_chn_attr);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("venc create param error \n");
        return s32Ret;
    }

    g_venc_chn_max_size[venc_chn] = venc_chn_attr.venc_attr.stream_buf_size;
    MEDIA_ERR("set venc chn str buf size %d\n",g_venc_chn_max_size[venc_chn]);

    s32Ret = xmedia_venc_create_chn(venc_chn, &venc_chn_attr);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("venc create chn fail \n");
        return s32Ret;
    }

    s32Ret = media_comm_venc_set_rc_param(venc_chn);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("venc set rc fail \n");
        return s32Ret;
    }

    //  set player fps
    if (venc_cfg->en_type == PT_H264) {
        s32Ret = xmedia_venc_get_h264_vui(venc_chn, &stH264Vui);
        if (XMEDIA_SUCCESS != s32Ret) {
            MEDIA_ERR("venc xmedia_venc_get_h264_vui error \n");
        }
        stH264Vui.vui_time_info.timing_info_present_flag = 1;
        stH264Vui.vui_time_info.num_units_in_tick = 1;
        stH264Vui.vui_time_info.time_scale = venc_cfg->dst_frame_rate * 2;
        s32Ret = xmedia_venc_set_h264_vui(venc_chn, &stH264Vui);
        if (XMEDIA_SUCCESS != s32Ret) {
            MEDIA_ERR("venc xmedia_venc_set_h264_vui error \n");
        }
    } else if (venc_cfg->en_type == PT_H265) {
        s32Ret = xmedia_venc_get_h265_vui(venc_chn, &stH265Vui);
        if (XMEDIA_SUCCESS != s32Ret) {
            MEDIA_ERR("venc xmedia_venc_get_h265_vui error \n");
        }
        stH265Vui.vui_time_info.timing_info_present_flag = 1;
        stH265Vui.vui_time_info.num_units_in_tick = 1;
        stH265Vui.vui_time_info.time_scale = venc_cfg->dst_frame_rate;
        s32Ret = xmedia_venc_set_h265_vui(venc_chn, &stH265Vui);
        if (XMEDIA_SUCCESS != s32Ret) {
            MEDIA_ERR("venc xmedia_venc_set_h265_vui error \n");
        }
    }

    s32Ret = xmedia_venc_start_recv_frame(venc_chn, &recv_param);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("venc xmedia_venc_start_recv_frame error \n");
        return s32Ret;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 media_comm_venc_stop(xmedia_s32 venc_chn)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;

    s32Ret = xmedia_venc_stop_recv_frame(venc_chn);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("venc xmedia_venc_stop_recv_frame error \n");
        return s32Ret;
    }

    s32Ret = xmedia_venc_destroy_chn(venc_chn);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("venc xmedia_venc_destroy_chn error \n");
        return s32Ret;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 media_comm_venc_get_max_chn_size(xmedia_s32 venc_chn)
{
    MEDIA_ERR("get venc chn str buf size %d\n",g_venc_chn_max_size[venc_chn]);
    return g_venc_chn_max_size[venc_chn];
}

static xmedia_s32 media_comm_isp_ainr_acquire_model(xmedia_s32 isp_pipe, xmedia_s32 model_num,const xmedia_char* ainr_model_name)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_s32 model_len = 0;
    xmedia_s32 read_len = 0;

    FILE *fp = fopen(ainr_model_name, "r");
    if (!fp) {
        MEDIA_ERR("open model fail\n");
    }

    s32Ret = fseek(fp, 0L, SEEK_END);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("get model size fail\n");
        goto LOAD_ERR0;
    }

    model_len = ftell(fp);

    g_isp_ainr_model[isp_pipe][model_num] = malloc(model_len);
    if (g_isp_ainr_model[isp_pipe][model_num] == XMEDIA_NULL) {
        MEDIA_ERR("isp model malloc fail");
        goto LOAD_ERR1;
    }

    s32Ret = fseek(fp, 0L, SEEK_SET);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("reset file start fail\n");
        goto LOAD_ERR2;
    }

    read_len = fread(g_isp_ainr_model[isp_pipe][model_num], 1 ,model_len, fp);
    if (read_len != model_len) {
        MEDIA_ERR("read model fail filesize %d readsize %d\n", model_len, read_len);
        goto LOAD_ERR2;
    }

    fclose(fp);
    return XMEDIA_SUCCESS;

LOAD_ERR2:
    free(g_isp_ainr_model[isp_pipe][model_num]);
    g_isp_ainr_model[isp_pipe][model_num] = XMEDIA_NULL;
LOAD_ERR1:
    g_isp_ainr_model[isp_pipe][model_num] = XMEDIA_NULL;
LOAD_ERR0:
    fclose(fp);
    return XMEDIA_FAILURE;
}

static xmedia_s32 media_comm_isp_ainr_release_model()
{
    for(xmedia_s32 i = 0; i < VI_MAX_PIPE_NUM; i++) {
        for(xmedia_s32 j = 0; j < XMEDIA_ISP_AINR_MODEL_MAX_NUM; j++) {
            if (g_isp_ainr_model[i][j] != NULL) {
                free(g_isp_ainr_model[i][j]);
                g_isp_ainr_model[i][j] = XMEDIA_NULL;
            }
        }
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 media_comm_isp_ainr_init(const xmedia_char* ainr_model_name)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_s32 cl_err_code = 0;
    xmedia_u32 cl_work_size = 0;
    xmedia_u32 cl_weight_size = 0;

    s32Ret = xmedia_cl_init();
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_cl_init fail \n");
        return s32Ret;
    }

    s32Ret = xmedia_cl_get_device_ids(XMEDIA_CL_DEVICE_ALL, XMEDIA_NULL, &g_isp_ainr_cfg.cl_device_num);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_cl_get_device_ids fail \n");
        goto CL_ERR0;
    }

    g_isp_ainr_cfg.cl_device = (xmedia_cl_device_id *)calloc(g_isp_ainr_cfg.cl_device_num, sizeof(xmedia_cl_device_id));
    if (g_isp_ainr_cfg.cl_device == XMEDIA_NULL) {
        MEDIA_ERR("cl device calloc fail\n");
        goto CL_ERR0;
    }

    s32Ret = xmedia_cl_get_device_ids(XMEDIA_CL_DEVICE_ALL, g_isp_ainr_cfg.cl_device, &g_isp_ainr_cfg.cl_device_num);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_cl_get_device_ids sec fail \n");
        goto CL_ERR1;
    }

    g_isp_ainr_cfg.cl_ctx = xmedia_cl_create_context(g_isp_ainr_cfg.cl_device_num, g_isp_ainr_cfg.cl_device, &cl_err_code);
    if (cl_err_code != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_cl_create_context fail\n");
        goto CL_ERR2;
    }

    s32Ret = xmedia_cl_graph_querysize_from_file(ainr_model_name, &cl_work_size, &cl_weight_size);
    if (cl_err_code != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_cl_graph_querysize_from_file fail\n");
        goto CL_ERR3;
    }

    g_isp_ainr_cfg.work_space_phyaddr = xmedia_mmz_alloc(XMEDIA_NULL, "AIworkspace", cl_work_size);
    if (g_isp_ainr_cfg.work_space_phyaddr == XMEDIA_NULL) {
        MEDIA_ERR("xmedia_mmz_alloc fail\n");
        goto CL_ERR3;
    }

    g_isp_ainr_cfg.work_space_viraddr = xmedia_mmz_map(g_isp_ainr_cfg.work_space_phyaddr, cl_work_size, 1);
    if (g_isp_ainr_cfg.work_space_viraddr == XMEDIA_NULL) {
        MEDIA_ERR("xmedia_mmz_map fail\n");
        goto CL_ERR4;
    }

    s32Ret = xmedia_cl_set_workspace_addr(g_isp_ainr_cfg.cl_ctx, g_isp_ainr_cfg.work_space_viraddr, cl_work_size);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_mmz_map fail\n");
        goto CL_ERR5;
    }

    return XMEDIA_SUCCESS;

CL_ERR5:
    if (g_isp_ainr_cfg.work_space_viraddr != XMEDIA_NULL) {
        xmedia_mmz_unmap(g_isp_ainr_cfg.work_space_viraddr);
    }
CL_ERR4:
    xmedia_mmz_free(g_isp_ainr_cfg.work_space_phyaddr);
CL_ERR3:
    s32Ret = xmedia_cl_release_context(g_isp_ainr_cfg.cl_ctx);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_cl_release_context id fail \n");
    }
CL_ERR2:
    s32Ret = xmedia_cl_release_device_ids(g_isp_ainr_cfg.cl_device, &g_isp_ainr_cfg.cl_device_num);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_cl_release_device_ids id fail \n");
    }
CL_ERR1:
    free(g_isp_ainr_cfg.cl_device);
CL_ERR0:
    s32Ret = xmedia_cl_uninit();
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_cl_uninit fail \n");
        return s32Ret;
    }

    return XMEDIA_FAILURE;
}

xmedia_s32 media_comm_isp_ainr_exit()
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;

    if (g_isp_ainr_cfg.work_space_viraddr != XMEDIA_NULL) {
        xmedia_mmz_unmap(g_isp_ainr_cfg.work_space_viraddr);
    }

    xmedia_mmz_free(g_isp_ainr_cfg.work_space_phyaddr);

    s32Ret = xmedia_cl_release_context(g_isp_ainr_cfg.cl_ctx);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_cl_release_context id fail \n");
    }

    s32Ret = xmedia_cl_release_device_ids(g_isp_ainr_cfg.cl_device, &g_isp_ainr_cfg.cl_device_num);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_cl_release_device_ids id fail \n");
    }

    free(g_isp_ainr_cfg.cl_device);

    s32Ret = xmedia_cl_uninit();
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_cl_uninit fail \n");
        return s32Ret;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 media_comm_isp_ainr_load_model(xmedia_s32 isp_pipe, xmedia_s32 model_num, xmedia_char** ainr_model_name)
{
    xmedia_s32 s32Ret = XMEDIA_SUCCESS;

    for(xmedia_s32 i = 0; i < model_num; i++) {
        s32Ret |= media_comm_isp_ainr_acquire_model(isp_pipe, i, ainr_model_name[i]);
    }

    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("isp acquire model fail\n");
        media_comm_isp_ainr_release_model();
        return XMEDIA_FAILURE;
    }

    s32Ret = xmedia_isp_load_ainr_model(isp_pipe, (xmedia_void*) g_isp_ainr_model[isp_pipe]);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("isp load ainr fail\n");
        media_comm_isp_ainr_release_model();
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 media_comm_isp_ainr_unload_model(xmedia_s32 isp_pipe)
{
    xmedia_isp_unload_ainr_model(isp_pipe);
    media_comm_isp_ainr_release_model();

    return XMEDIA_SUCCESS;
}

xmedia_s32 media_comm_isp_ainr_enable(xmedia_s32 isp_pipe)
{
    xmedia_s32 s32Ret = XMEDIA_SUCCESS;
    xmedia_isp_ainr_attr ainr_attr = {0};
    xmedia_isp_bnr_attr bnr_attr   = {0};

    s32Ret = xmedia_isp_get_bnr_attr(isp_pipe, &bnr_attr);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_isp_get_bnr_attr fail \n");
        return XMEDIA_FAILURE;
    }

    bnr_attr.enable = XMEDIA_TRUE;
    bnr_attr.ainr_pos_switch = XMEDIA_ISP_BNR_AINR_POS_POST;
    bnr_attr.tnr_enable = XMEDIA_TRUE;
    s32Ret = xmedia_isp_set_bnr_attr(isp_pipe, &bnr_attr);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_isp_set_bnr_attr fail \n");
        return XMEDIA_FAILURE;
    }

    s32Ret = xmedia_isp_get_ainr_attr(isp_pipe, &ainr_attr);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_isp_get_ainr_attr fail \n");
        return XMEDIA_FAILURE;
    }

    ainr_attr.enable  = XMEDIA_TRUE;
    ainr_attr.op_type = XMEDIA_VIDEO_OPERATION_MODE_MANUAL;
    ainr_attr.manual_attr.pos_gain = 64;
    ainr_attr.manual_attr.gau_gain = 64;
    s32Ret = xmedia_isp_set_ainr_attr(isp_pipe, &ainr_attr);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_isp_set_ainr_attr fail \n");
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 scene_auto_get_isp_data_len(xmedia_u8 *buf, xmedia_s32 *data_len)
{
    xmedia_s32 i = 0;
    xmedia_u8 *pos = buf;
    xmedia_isp_module_info isp_mod_info[XMEDIA_ISP_MODULE_MAX_NUM] = { 0 };

    pos += MAGIC_DATA_LEN;
    pos += sizeof(xmedia_s32);

    memcpy(&isp_mod_info, pos, sizeof(isp_mod_info));
    pos += sizeof(isp_mod_info);

    for (i = 0; i < XMEDIA_ISP_MODULE_MAX_NUM; i++) {
        pos += isp_mod_info[i].length;
    }

    *data_len = pos - buf;

    return XMEDIA_SUCCESS;
}

xmedia_s32 scene_auto_import_isp_data(bin_module_info *module, xmedia_u8* buffer)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 i = 0;
    xmedia_u8 *pos = buffer;
    xmedia_isp_module_info isp_mod_info[XMEDIA_ISP_MODULE_MAX_NUM] = { 0 };

    pos += MAGIC_DATA_LEN;
    pos += sizeof(xmedia_s32);

    memcpy(&isp_mod_info, pos, sizeof(isp_mod_info));
    pos += sizeof(isp_mod_info);

    for (i = 0; i < XMEDIA_ISP_MODULE_MAX_NUM; i++) {
        isp_mod_info[i].addr = pos;
        pos += isp_mod_info[i].length;
    }

    ret = xmedia_isp_import(module->vi.pipe, isp_mod_info);
    if (ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_isp_import failed.\n");
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 scene_auto_import(bin_module_info *module, xmedia_u8 *buffer, xmedia_u32 total_len)
{
    xmedia_s32 ret = XMEDIA_FAILURE;
    xmedia_s32 len = 0;
    xmedia_s32 result = -1;

    if (module == XMEDIA_NULL || buffer == XMEDIA_NULL) {
        MEDIA_ERR("scene auto input param is a null point \n");
        return XMEDIA_FAILURE;
    }

    xmedia_sys_get_tuning_connect(&result);
    if (result == 0 || result == -1) {
        MEDIA_ERR("xmedia_sys_get_tuning_connect failed\n");
        return XMEDIA_FAILURE;
    }

    //import isp data
    if (total_len > sizeof(magic_isp) && memcmp(&magic_isp, buffer, sizeof(magic_isp)) == 0) {
        scene_auto_get_isp_data_len(buffer, &len);

        total_len -= len;
        if (total_len < 0) {
            MEDIA_ERR("pq_bin data is incomplete!\n");
            return XMEDIA_FAILURE;
        }

        if (module->vi.enable == XMEDIA_TRUE) {
            ret = scene_auto_import_isp_data(module, buffer);
            if (ret != XMEDIA_SUCCESS) {
                MEDIA_ERR("pq_bin import isp data failed\n");
                return ret;
            }
        }

        buffer += len;
    } else {
        //check scene auto bin & soft version
       if (module->vi.enable == XMEDIA_TRUE) {
            MEDIA_ERR("scene auto bin not match to soft version\n");
            return XMEDIA_FAILURE;
       }
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 media_comm_scene_auto_enable(bin_module_info *module)
{
    xmedia_s32 ret = XMEDIA_FAILURE;
    xmedia_s32 file_size = 0;
    xmedia_s32 file_len = 0;
    xmedia_u8 *buf = XMEDIA_NULL;

    FILE *file_fd = XMEDIA_NULL;

    file_fd = fopen(SCENE_AUTO_BIN_PATH,"r");
    if (XMEDIA_NULL == file_fd) {
        MEDIA_ERR("open scene auto file fail \n");
        return XMEDIA_FAILURE;
    }

    fseek(file_fd, 0, SEEK_END);
    file_size= ftell(file_fd);
    fseek(file_fd, 0, SEEK_SET);

    buf = (xmedia_u8 *)malloc(file_size);
    if (buf == XMEDIA_NULL) {
        MEDIA_ERR("scene auto file malloc fail\n");
        fclose(file_fd);
        return XMEDIA_FAILURE;
    }

    file_len = fread(buf, sizeof(xmedia_u8), file_size, file_fd);
    if (file_len == 0) {
        MEDIA_ERR("scene auto file read fail \n");
        fclose(file_fd);
        free(buf);
        buf = XMEDIA_NULL;
        return XMEDIA_FAILURE;
    }

    ret = scene_auto_import(module, buf, file_size);
    if (ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("scene auto file import fail\n");
        return XMEDIA_FAILURE;
    }

    fclose(file_fd);
    free(buf);
    buf = XMEDIA_NULL;
    return XMEDIA_SUCCESS;
}

static xmedia_u64 g_fpn_phy_addr[VI_MAX_PIPE_NUM] = {0};

xmedia_s32 media_comm_fpn_enable(xmedia_s32 isp_pipe, xmedia_s32 width, xmedia_s32 height)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_void *fpn_buf = XMEDIA_NULL;
    xmedia_u32 buf_size = 0;
    xmedia_s32 fpn_size = 0;
    FILE* fp = XMEDIA_NULL;
    xmedia_isp_fpn_attr fpn_attr = {0};

    fp = fopen(FPN_BIN_PATH, "rb");
    if (fp == XMEDIA_NULL) {
        MEDIA_ERR("open fpn file fail\n");
        goto fpn_err0;
    }

    fseek(fp, 0, SEEK_END);
    fpn_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    buf_size = (width + 15) / 16 * 16 * height;

    if (buf_size != fpn_size) {
        MEDIA_ERR("fpn size no match fpn bin buf_size %d fpn %d\n",buf_size, fpn_size);
        goto fpn_err0;
    }

    g_fpn_phy_addr[isp_pipe] = xmedia_mmz_alloc(XMEDIA_NULL, "fpn_bin", buf_size);
    if (g_fpn_phy_addr[isp_pipe] == XMEDIA_NULL) {
        MEDIA_ERR("fpn alloc fail\n");
        goto fpn_err0;
    }

    fpn_buf = xmedia_mmz_map(g_fpn_phy_addr[isp_pipe], buf_size, XMEDIA_FALSE);
    if (fpn_buf == XMEDIA_NULL) {
        MEDIA_ERR("fpn mmz map fail\n");
        goto fpn_err1;
    }

    fread(fpn_buf, buf_size, 1, fp);

    xmedia_mmz_unmap(fpn_buf);

    fpn_attr.enable = XMEDIA_TRUE;
    fpn_attr.fpn_frm_info.frame.addr.y_phy_addr = g_fpn_phy_addr[isp_pipe];
    s32Ret = xmedia_isp_set_fpn_attr(isp_pipe, &fpn_attr);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("xmedia_isp_set_fpn_attr failed.\n");
        goto fpn_err1;
    }

    fclose(fp);

    return XMEDIA_SUCCESS;

fpn_err1:
    xmedia_mmz_free(g_fpn_phy_addr[isp_pipe]);
fpn_err0:
    fclose(fp);
    return XMEDIA_FAILURE;
}
