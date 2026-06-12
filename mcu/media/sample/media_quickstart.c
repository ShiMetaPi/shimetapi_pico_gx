#include <malloc.h>
#include <timer.h>
#include <pwm.h>
#include <common.h>

#include "media_sample.h"

#define MCU_CPU_SYNC_REG      0x1202014c
#define MCU_END_VALUE         0xbeef0002
#define MCU_EXIT_VALUE        0xbeef0003
//#define INFRARED_SUPPORT  1

#ifdef INFRARED_SUPPORT
#define PWM_R_LED_NUM   8
#define PWM_R_LED_REG   0x11980014
#define PWM_R_LED_VALUE 0x401
#define PWM_ENABLE_REG  0x11980018
#endif

#ifdef SENSOR_SC485SL_2LANE_SUPPORT
    #define MCU_SENSOR_WIDTH   2560
    #define MCU_SENSOR_HEIGHT  1440
#elif defined (SENSOR_SC485SL_4LANE_SUPPORT)
    #define MCU_SENSOR_WIDTH   2688
    #define MCU_SENSOR_HEIGHT  1520
#elif defined (SENSOR_SC485SL_4LANE_60FPS_SUPPORT)
    #define MCU_SENSOR_WIDTH   2560
    #define MCU_SENSOR_HEIGHT  1440
#elif defined (SENSOR_SC235HAI_2LANE_60FPS_SUPPORT)
    #define MCU_SENSOR_WIDTH   1920
    #define MCU_SENSOR_HEIGHT  1080
#else
    #define MCU_SENSOR_WIDTH   2688
    #define MCU_SENSOR_HEIGHT  1520
#endif

STAGE1_FUNC xmedia_void media_comm_write_reg(xmedia_u32 addr, xmedia_u32 value)
{
    *((xmedia_u32 *)addr) = value;
}

STAGE1_FUNC xmedia_u32 media_comm_read_reg(xmedia_u32 addr)
{
    return *((xmedia_u32 *)addr);
}

xmedia_bool media_comm_get_mcu_end_signal()
{
    if (MCU_END_VALUE == media_comm_read_reg(MCU_CPU_SYNC_REG))
    {
        return XMEDIA_TRUE;
    }

    return XMEDIA_FALSE;
}

xmedia_void media_comm_set_mcu_exit_signal()
{
    media_comm_write_reg(MCU_CPU_SYNC_REG, MCU_EXIT_VALUE);
}

#ifdef INFRARED_SUPPORT
xmedia_s32 media_comm_infrared_init()
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    //set pinmux
    media_comm_write_reg(PWM_R_LED_REG, PWM_R_LED_VALUE);

    //duty_cycle_us > 0, is smaller the light brighter
    s32Ret = pwm_config(PWM_R_LED_NUM, 1, 100);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("pwm config fail \n");
        return s32Ret;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 media_comm_set_infrared(LIGHT_STATUS state)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;

    if (state == LIGHT_STATUS_NIGHT) {
        media_comm_write_reg(PWM_ENABLE_REG, 1 << 8);
        return pwm_enable(PWM_R_LED_NUM);
    } else if (state == LIGHT_STATUS_DAY) {
        return pwm_disable(PWM_R_LED_NUM);
    }

    return s32Ret;
}

xmedia_s32 media_comm_set_ir_cut_state(xmedia_s32 vi_dev, LIGHT_STATUS state)
{
    if (vi_dev == 0 || vi_dev == 1) {
        media_comm_write_reg(0x112C00BC,0x1400);
        media_comm_write_reg(0x112C00C0,0x1400);

        if (state == LIGHT_STATUS_DAY) {
            media_comm_write_reg(0x120B2400,0x03);
            media_comm_write_reg(0x120B200C,0x02);
        } else if (state == LIGHT_STATUS_NIGHT){
            media_comm_write_reg(0x120B2400,0x03);
            media_comm_write_reg(0x120B200C,0x01);
        } else {
            return XMEDIA_FAILURE;
        }
    } else if (vi_dev == 2 || vi_dev == 3) {
        media_comm_write_reg(0x112C00C8,0x1400);
        media_comm_write_reg(0x112C00CC,0x1400);

        if (state == LIGHT_STATUS_DAY) {
            media_comm_write_reg(0x120B5400,0x03);
            media_comm_write_reg(0x120B500C,0x02);
        } else if (state == LIGHT_STATUS_NIGHT){
            media_comm_write_reg(0x120B5400,0x03);
            media_comm_write_reg(0x120B500C,0x01);
        } else {
            return XMEDIA_FAILURE;
        }
    }

    if (vi_dev == 0 || vi_dev == 1) {
        media_comm_write_reg(0x120B2400,0x00);
        media_comm_write_reg(0x120B200C,0x00);
    } else if (vi_dev == 2 || vi_dev == 3) {
        media_comm_write_reg(0x120B5400,0x00);
        media_comm_write_reg(0x120B500C,0x00);
    }

    return XMEDIA_SUCCESS;
}
#endif

STAGE1_FUNC static xmedia_void quickstart_sensor_register(xmedia_s32 isp_pipe)
{
#if defined(SENSOR_SC485SL_2LANE_SUPPORT) || defined(SENSOR_SC485SL_4LANE_SUPPORT) || defined(SENSOR_SC485SL_4LANE_60FPS_SUPPORT)
        XMEDIA_SENSOR_REGISTER_DRIVER(isp_pipe, sc485sl);
#elif defined(SENSOR_SC235HAI_2LANE_60FPS_SUPPORT)
        XMEDIA_SENSOR_REGISTER_DRIVER(isp_pipe, sc235hai);
#else
    #error "should define sensor type first"
#endif
}

STAGE1_FUNC static xmedia_void quickstart_sensor_unregister(xmedia_s32 isp_pipe)
{
#if defined(SENSOR_SC485SL_2LANE_SUPPORT) || defined(SENSOR_SC485SL_4LANE_SUPPORT)
    XMEDIA_SENSOR_UNREGISTER_DRIVER(isp_pipe, sc485sl);
#elif defined(SENSOR_SC485SL_4LANE_60FPS_SUPPORT)
    XMEDIA_SENSOR_UNREGISTER_DRIVER(isp_pipe, sc485sl);
#elif defined(SENSOR_SC235HAI_2LANE_60FPS_SUPPORT)
    XMEDIA_SENSOR_UNREGISTER_DRIVER(isp_pipe, sc235hai);
#else
    #error "should define sensor type first"
#endif
}

STAGE1_FUNC static xmedia_void quickstart_vi_get_devcfg(xmedia_vi_dev_config *vi_dev_cfg)
{
#if defined(SENSOR_SC485SL_2LANE_SUPPORT)
    vi_dev_cfg->mipi_lane_num = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    vi_dev_cfg->data_type = XMEDIA_INTF_MIPI_CSI_DATA_TYPE_RAW_12BIT;
    vi_dev_cfg->mipi_rate = 1080;
#elif defined(SENSOR_SC485SL_4LANE_SUPPORT)
    vi_dev_cfg->mipi_lane_num = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE;
    vi_dev_cfg->data_type = XMEDIA_INTF_MIPI_CSI_DATA_TYPE_RAW_12BIT;
    vi_dev_cfg->mipi_rate = 1126;
#elif defined(SENSOR_SC485SL_4LANE_60FPS_SUPPORT)
    vi_dev_cfg->mipi_lane_num = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE;
    vi_dev_cfg->data_type = XMEDIA_INTF_MIPI_CSI_DATA_TYPE_RAW_12BIT;
    vi_dev_cfg->mipi_rate = 864;
#elif defined(SENSOR_SC235HAI_2LANE_60FPS_SUPPORT)
    vi_dev_cfg->mipi_lane_num = XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE;
    vi_dev_cfg->data_type = XMEDIA_INTF_MIPI_CSI_DATA_TYPE_RAW_10BIT;
    vi_dev_cfg->mipi_rate = 756;
#else
    #error "should define sensor type first"
#endif
}

STAGE1_FUNC static xmedia_sensor_mipi_lanes quickstart_from_vi_get_mipi_lanes(xmedia_vi_dev_config *vi_dev_cfg)
{
    xmedia_sensor_mipi_lanes mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
    if (vi_dev_cfg->mipi_lane_num == XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_1_LANE) {
        mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_1L;
    } else if (vi_dev_cfg->mipi_lane_num == XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE) {
        mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;
    } else if (vi_dev_cfg->mipi_lane_num == XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE) {
        mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_4L;
    }

    return mipi_lanes;
}

STAGE1_FUNC xmedia_s32 quickstart_media_preinit(void)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_s32 isp_pipe = 0;
    xmedia_s32 vi_dev = 0;
    xmedia_vi_dev_config vi_dev_cfg = {0};
    xmedia_sensor_config sensor_cfg = {0};
    xmedia_sensor_attr sensor_attr = {0};
    xmedia_sensor_mipi_lanes mipi_lanes = XMEDIA_SENSOR_MIPI_LANES_2L;

    sensor_cfg.comm_bus.type = XMEDIA_SENSOR_BUS_TYPE_I2C;
    sensor_cfg.init_mode = XMEDIA_SENSOR_INIT_MODE_NORMAL;
    sensor_cfg.comm_bus.i2c_dev = 0;
    sensor_cfg.ctrl_sig.clk_ch = 0;
    sensor_cfg.ctrl_sig.rst_ch = 0;

    s32Ret = i2c_init(sensor_cfg.comm_bus.i2c_dev);
    if (s32Ret != 0) {
        MEDIA_STAGE1_ERR("i2c init error\n");
        return s32Ret;
    }

    drv_mipi_init();

    quickstart_vi_get_devcfg(&vi_dev_cfg);

    s32Ret = drv_mipi_enable(vi_dev, &vi_dev_cfg);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_STAGE1_ERR("mipi enable fail \n");
        return s32Ret;
    }

    quickstart_sensor_register(isp_pipe);

    s32Ret = drv_sensor_init(isp_pipe, &sensor_cfg);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_STAGE1_ERR("sensor init fail\n");
        return s32Ret;
    }

    mipi_lanes = quickstart_from_vi_get_mipi_lanes(&vi_dev_cfg);
    drv_sensor_set_mipi_lanes(isp_pipe, mipi_lanes);
    if (s32Ret != XMEDIA_SUCCESS) {
          MEDIA_STAGE1_ERR("sensor set mipi lanes fail\n");
          return s32Ret;
    }

    sensor_attr.width = MCU_SENSOR_WIDTH;
    sensor_attr.height = MCU_SENSOR_HEIGHT;
    sensor_attr.wdr_mode = 0;
    s32Ret = drv_sensor_set_attr(isp_pipe, &sensor_attr);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_STAGE1_ERR("sensor set attr fail\n");
        return s32Ret;
    }

    s32Ret = drv_sensor_start(isp_pipe);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("sensor start fail\n");
        return s32Ret;
    }
#ifdef CONFIG_STOPWATCH_SUPPORT
    stopwatch_trigger();
#endif
    return XMEDIA_SUCCESS;
}

xmedia_s32 quickstart_media_start(void)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_s32 isp_pipe = 0;
    xmedia_s32 vi_dev = 0;
    xmedia_s32 vi_pipe = 0;
    xmedia_vi_dev_config vi_dev_cfg = {0};
    xmedia_vi_pipe_config vi_pipe_cfg = {0};
    xmedia_isp_config isp_cfg = {0};

    quickstart_vi_get_devcfg(&vi_dev_cfg);

    s32Ret = drv_vi_init();
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("vi init fail \n");
        return s32Ret;
    }

    s32Ret = drv_vi_enable_dev(vi_dev);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("vi enable dev fail\n");
        return s32Ret;
    }

    s32Ret = drv_vi_set_dev_bind_pipe(vi_dev, vi_pipe);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("vi dev bind pipe fail\n");
        return s32Ret;
    }

    quickstart_vi_get_devcfg(&vi_dev_cfg);

    vi_pipe_cfg.bit_width = vi_dev_cfg.data_type;
    vi_pipe_cfg.width = MCU_SENSOR_WIDTH;
    vi_pipe_cfg.height = MCU_SENSOR_HEIGHT;
    vi_pipe_cfg.vi_work_mode = 0;
    vi_pipe_cfg.wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;

    s32Ret = xmedia_awb_register(isp_pipe);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("awb init fail\n");
        puthex(s32Ret);
        return s32Ret;
    }

    s32Ret = xmedia_ae_register(isp_pipe);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("ae init fail\n");
        puthex(s32Ret);
        return s32Ret;
    }

    isp_cfg.size.width = MCU_SENSOR_WIDTH;
    isp_cfg.size.height = MCU_SENSOR_HEIGHT;
#if defined(SENSOR_SC485SL_4LANE_60FPS_SUPPORT) ||defined(SENSOR_SC235HAI_2LANE_60FPS_SUPPORT)
    isp_cfg.fps = 60;
#else
    isp_cfg.fps = 30;
#endif
    isp_cfg.wdr_mode = XMEDIA_VIDEO_WDR_MODE_NONE;
    isp_cfg.bayer_fmt = XMEDIA_VIDEO_BAYER_FMT_BGGR;
    isp_cfg.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_RAW;
    isp_cfg.mode_config.work_mode = XMEDIA_ISP_WORK_MODE_MASTER;
    isp_cfg.mode_config.master_mode.blend_stat_enable = XMEDIA_FALSE;
    isp_cfg.mode_config.master_mode.slave_num = 0;

    s32Ret = drv_isp_init(isp_pipe, &isp_cfg);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("isp init fail\n");
        return s32Ret;
    }

    s32Ret = drv_vi_start_pipe(vi_pipe, &vi_pipe_cfg);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("vi start pipe fail\n");
        return s32Ret;
    }

    s32Ret = drv_isp_start(isp_pipe);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("isp start fail\n");
        return s32Ret;
    }

    while(1)
    {
        s32Ret = drv_isp_process(isp_pipe);
        if (s32Ret == XMEDIA_SUCCESS ) {
            if (XMEDIA_TRUE == media_comm_get_mcu_end_signal()){
                break;
            }
        }
    }
#ifdef CONFIG_STOPWATCH_SUPPORT
        stopwatch_trigger();
#endif
    s32Ret = drv_isp_stop(isp_pipe);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("isp stop fail\n");
        return s32Ret;
    }

    s32Ret = drv_vi_stop_pipe(vi_pipe);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("isp stop fail\n");
        return s32Ret;
    }

    s32Ret = drv_isp_exit(isp_pipe);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("isp exit fail\n");
        return s32Ret;
    }

    s32Ret = xmedia_ae_unregister(isp_pipe);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("ae exit fail\n");
        return s32Ret;
    }

    s32Ret = xmedia_awb_unregister(isp_pipe);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("awb exit fail\n");
        return s32Ret;
    }

    quickstart_sensor_unregister(isp_pipe);

    s32Ret = drv_vi_set_dev_unbind_pipe(vi_dev, vi_pipe);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("vi dev unbind pipe fail\n");
        return s32Ret;
    }

    s32Ret = drv_vi_disable_dev(vi_dev, &vi_dev_cfg);
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("vi disable dev fail\n");
        return s32Ret;
    }

    s32Ret = drv_vi_exit();
    if (s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("vi exit fail \n");
        return s32Ret;
    }

    media_comm_set_mcu_exit_signal();

    return XMEDIA_SUCCESS;
}
