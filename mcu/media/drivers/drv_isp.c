#include "config.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "xmedia_type.h"
#include "xmedia_isp.h"
#include "xmedia_errcode.h"
#include "isp_def.h"
#include "hal_isp.h"
#include "drv_isp.h"
#include "platform.h"
#include "timer.h"

#define CCM_MINUS_PARAM_TH 32768

typedef struct {
    xmedia_bool                  ae_lib;
    xmedia_bool                  awb_lib;
    xmedia_bool                  sensor_lib;
    xmedia_isp_ae_register_info  ae_info;
    xmedia_isp_awb_register_info awb_info;
    xmedia_sensor_register_info  sensor_info;
} isp_lib_info;

typedef struct {
    xmedia_isp_config isp_config;
    xmedia_u32        isp_frame_cnt;
    xmedia_void      *firmware_info;
    isp_lib_info     *lib_info;

    xmedia_sensor_init_param *result;
} isp_context;

STAGE1_GLOBAL static isp_context *g_isp_ctx[ISP_PIPE_MAX_NUM]       = { 0 };
STAGE1_GLOBAL static isp_lib_info g_isp_lib[ISP_PIPE_MAX_NUM]       = { 0 };
STAGE1_GLOBAL static xmedia_u8    g_sensor_clk_ch[ISP_PIPE_MAX_NUM] = { 0 };
STAGE1_GLOBAL static xmedia_u8    g_sensor_rst_ch[ISP_PIPE_MAX_NUM] = { 0 };

#define ISP_GET_CTX(pipe, ctx) ctx = g_isp_ctx[pipe]
#define ISP_GET_LIB(pipe, ptr) ptr = &g_isp_lib[pipe]

#define TIMER_CLOCK_FRQ 24000000

xmedia_u32 isp_get_time_stamp(xmedia_void)
{
    return (xmedia_u32)timer_get_val() / 24; // = timer_value * 10000000 / 24000000
}

STAGE1_FUNC xmedia_s32 xmedia_isp_register_sensor(xmedia_u32 pipe, const xmedia_sensor_register_info *info)
{
    isp_context  *ctx = XMEDIA_NULL;
    isp_lib_info *lib = XMEDIA_NULL;

    if (pipe >= ISP_PIPE_MAX_NUM || info == XMEDIA_NULL) {
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    ISP_GET_CTX(pipe, ctx);
    ISP_GET_LIB(pipe, lib);

    if (lib->sensor_lib == XMEDIA_TRUE) {
        return XMEDIA_ERRCODE_EXIST;
    }

    // sensor需要在ISP初始化之前注册，ISP初始化时才会为ctx分配内存，所以用ctx来作为判断条件
    if (ctx != XMEDIA_NULL) {
        return XMEDIA_ERRCODE_NOT_PERMITTED;
    }

    memcpy(&lib->sensor_info, info, sizeof(xmedia_sensor_register_info));
    lib->sensor_lib = XMEDIA_TRUE;

    return XMEDIA_SUCCESS;
}

xmedia_s32 xmedia_isp_unregister_sensor(xmedia_u32 pipe, xmedia_u32 dev_id)
{
    isp_context  *ctx = XMEDIA_NULL;
    isp_lib_info *lib = XMEDIA_NULL;

    if (pipe >= ISP_PIPE_MAX_NUM) {
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    ISP_GET_CTX(pipe, ctx);
    ISP_GET_LIB(pipe, lib);

    if (lib->sensor_lib == XMEDIA_FALSE) {
        return XMEDIA_ERRCODE_NOT_EXIST;
    }

    // sensor需要在ISP退出之后反注册，ISP退出时才会释放ctx内存，所以用ctx来作为判断条件
    if (ctx != XMEDIA_NULL) {
        return XMEDIA_ERRCODE_NOT_PERMITTED;
    }

    if (lib->sensor_info.dev_id != dev_id) {
        return XMEDIA_ERRCODE_NOT_EXIST;
    }

    lib->sensor_lib = XMEDIA_FALSE;
    memset(&lib->sensor_info, 0, sizeof(xmedia_sensor_register_info));

    return XMEDIA_SUCCESS;
}

xmedia_s32 xmedia_isp_register_ae_lib(xmedia_u32 pipe, const xmedia_isp_ae_register_info *ae_info,
                                      xmedia_sensor_register_ae_info *sensor_func)
{
    isp_context  *ctx = XMEDIA_NULL;
    isp_lib_info *lib = XMEDIA_NULL;

    if (pipe >= ISP_PIPE_MAX_NUM || ae_info == XMEDIA_NULL || sensor_func == XMEDIA_NULL) {
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    ISP_GET_CTX(pipe, ctx);
    ISP_GET_LIB(pipe, lib);

    if (lib->ae_lib == XMEDIA_TRUE) {
        return XMEDIA_ERRCODE_EXIST;
    }

    // 3a需要在ISP初始化之前注册，ISP初始化时才会为ctx分配内存，所以用ctx来作为判断条件
    if (ctx != XMEDIA_NULL) {
        return XMEDIA_ERRCODE_NOT_PERMITTED;
    }

    if (lib->sensor_lib != XMEDIA_TRUE) {
        return XMEDIA_ERRCODE_NOT_PERMITTED;
    }

    memcpy(&sensor_func->func, &lib->sensor_info.ae_func, sizeof(xmedia_sensor_register_ae_func));
    sensor_func->dev_id = lib->sensor_info.dev_id;

    memcpy(&lib->ae_info, ae_info, sizeof(xmedia_isp_ae_register_info));
    lib->ae_lib = XMEDIA_TRUE;

    return XMEDIA_SUCCESS;
}

xmedia_s32 xmedia_isp_unregister_ae_lib(xmedia_u32 pipe, xmedia_char name[XMEDIA_ISP_ALG_NAME_MAX_SIZE], xmedia_s32 id)
{
    isp_context  *ctx = XMEDIA_NULL;
    isp_lib_info *lib = XMEDIA_NULL;

    if (pipe >= ISP_PIPE_MAX_NUM || name == XMEDIA_NULL) {
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    ISP_GET_CTX(pipe, ctx);
    ISP_GET_LIB(pipe, lib);

    if (lib->ae_lib == XMEDIA_FALSE) {
        return XMEDIA_ERRCODE_NOT_EXIST;
    }

    // 3a需要在ISP退出之后反注册，ISP退出时才会释放ctx内存，所以用ctx来作为判断条件
    if (ctx != XMEDIA_NULL) {
        return XMEDIA_ERRCODE_NOT_PERMITTED;
    }

    if (lib->ae_info.alg_id == id && strncmp(lib->ae_info.alg_name, name, XMEDIA_ISP_ALG_NAME_MAX_SIZE) == 0) {
        memset(&lib->ae_info, 0, sizeof(xmedia_isp_ae_register_info));
        lib->ae_lib = XMEDIA_FALSE;
    } else {
        return XMEDIA_ERRCODE_NOT_EXIST;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 xmedia_isp_register_awb_lib(xmedia_u32 pipe, const xmedia_isp_awb_register_info *awb_info,
                                       xmedia_sensor_register_awb_info *sensor_func)
{
    isp_context  *ctx = XMEDIA_NULL;
    isp_lib_info *lib = XMEDIA_NULL;

    if (pipe >= ISP_PIPE_MAX_NUM || awb_info == XMEDIA_NULL || sensor_func == XMEDIA_NULL) {
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    ISP_GET_CTX(pipe, ctx);
    ISP_GET_LIB(pipe, lib);

    if (lib->awb_lib == XMEDIA_TRUE) {
        return XMEDIA_ERRCODE_EXIST;
    }

    // 3a需要在ISP初始化之前注册，ISP初始化时才会为ctx分配内存，所以用ctx来作为判断条件
    if (ctx != XMEDIA_NULL) {
        return XMEDIA_ERRCODE_NOT_PERMITTED;
    }

    if (lib->sensor_lib != XMEDIA_TRUE) {
        return XMEDIA_ERRCODE_NOT_PERMITTED;
    }

    memcpy(&sensor_func->func, &lib->sensor_info.awb_func, sizeof(xmedia_sensor_register_awb_func));
    sensor_func->dev_id = lib->sensor_info.dev_id;

    memcpy(&lib->awb_info, awb_info, sizeof(xmedia_isp_awb_register_info));
    lib->awb_lib = XMEDIA_TRUE;

    return XMEDIA_SUCCESS;
}

xmedia_s32 xmedia_isp_unregister_awb_lib(xmedia_u32 pipe, xmedia_char name[XMEDIA_ISP_ALG_NAME_MAX_SIZE], xmedia_s32 id)
{
    isp_context  *ctx = XMEDIA_NULL;
    isp_lib_info *lib = XMEDIA_NULL;

    if (pipe >= ISP_PIPE_MAX_NUM || name == XMEDIA_NULL) {
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    ISP_GET_CTX(pipe, ctx);
    ISP_GET_LIB(pipe, lib);

    if (lib->awb_lib == XMEDIA_FALSE) {
        return XMEDIA_ERRCODE_NOT_EXIST;
    }

    // 3a需要在ISP退出之后反注册，ISP退出时才会释放ctx内存，所以用ctx来作为判断条件
    if (ctx != XMEDIA_NULL) {
        return XMEDIA_ERRCODE_NOT_PERMITTED;
    }

    if (lib->awb_info.alg_id == id && strncmp(lib->awb_info.alg_name, name, XMEDIA_ISP_ALG_NAME_MAX_SIZE) == 0) {
        memset(&lib->awb_info, 0, sizeof(xmedia_isp_awb_register_info));
        lib->awb_lib = XMEDIA_FALSE;
    } else {
        return XMEDIA_ERRCODE_NOT_EXIST;
    }

    return XMEDIA_SUCCESS;
}

STAGE1_FUNC xmedia_s32 drv_sensor_init(xmedia_u32 pipe, xmedia_sensor_config *cfg)
{
    xmedia_s32               ret = 0;
    isp_lib_info            *lib = XMEDIA_NULL;
    xmedia_sensor_capability capability;

    if (pipe >= ISP_PIPE_MAX_NUM || cfg == XMEDIA_NULL) {
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    ISP_GET_LIB(pipe, lib);
    if (lib->sensor_lib != XMEDIA_TRUE) {
        return XMEDIA_ERRCODE_NOT_EXIST;
    }

    if (lib->sensor_info.isp_func.pfn_sensor_init == XMEDIA_NULL ||
        lib->sensor_info.isp_func.pfn_sensor_set_bus_info == XMEDIA_NULL ||
        lib->sensor_info.isp_func.pfn_sensor_get_capability == XMEDIA_NULL) {
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    g_sensor_clk_ch[pipe] = cfg->ctrl_sig.clk_ch;
    g_sensor_rst_ch[pipe] = cfg->ctrl_sig.rst_ch;

    ret = lib->sensor_info.isp_func.pfn_sensor_set_bus_info(lib->sensor_info.dev_id, &cfg->comm_bus);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    ret = lib->sensor_info.isp_func.pfn_sensor_get_capability(&capability);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    if (capability.reset_time <= 0) {
        capability.reset_time = 100;
    }

    hal_sensor_set_mclk(g_sensor_clk_ch[pipe], capability.init_mclk);

    //sensor cold start use normal, hot start use skip
    if (cfg->init_mode != XMEDIA_SENSOR_INIT_MODE_SKIP) {
        hal_sensor_reset(g_sensor_rst_ch[pipe], XMEDIA_TRUE);
        udelay(capability.reset_time);
        hal_sensor_reset(g_sensor_rst_ch[pipe], XMEDIA_FALSE);
    }

    udelay(1000*4);

    return lib->sensor_info.isp_func.pfn_sensor_init(lib->sensor_info.dev_id, cfg->init_mode);
}

xmedia_s32 drv_sensor_exit(xmedia_u32 pipe)
{
    xmedia_s32    ret = 0;
    isp_lib_info *lib = XMEDIA_NULL;

    if (pipe >= ISP_PIPE_MAX_NUM) {
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    ISP_GET_LIB(pipe, lib);
    if (lib->sensor_lib != XMEDIA_TRUE) {
        return XMEDIA_ERRCODE_NOT_EXIST;
    }

    if (lib->sensor_info.isp_func.pfn_sensor_exit == XMEDIA_NULL) {
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    ret = lib->sensor_info.isp_func.pfn_sensor_exit(lib->sensor_info.dev_id);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    return XMEDIA_SUCCESS;
}

STAGE1_FUNC xmedia_s32 drv_sensor_resume(xmedia_u32 pipe)
{
    isp_lib_info          *lib = XMEDIA_NULL;

    if (pipe >= ISP_PIPE_MAX_NUM) {
        puts("pipe error \n");
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    ISP_GET_LIB(pipe, lib);
    if (lib->sensor_lib != XMEDIA_TRUE) {
        puts("sensor lib error\n");
        return XMEDIA_ERRCODE_NOT_EXIST;
    }

    if (lib->sensor_info.isp_func.pfn_sensor_resume == XMEDIA_NULL) {
        puts("sensor resume none\n");
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    return lib->sensor_info.isp_func.pfn_sensor_resume(lib->sensor_info.dev_id);
}

STAGE1_FUNC xmedia_s32 drv_sensor_start(xmedia_u32 pipe)
{
    xmedia_s32             ret  = 0;
    xmedia_u32             mclk = 0;
    isp_lib_info          *lib  = XMEDIA_NULL;
    xmedia_sensor_property property;

    if (pipe >= ISP_PIPE_MAX_NUM) {
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    ISP_GET_LIB(pipe, lib);
    if (lib->sensor_lib != XMEDIA_TRUE) {
        return XMEDIA_ERRCODE_NOT_EXIST;
    }

    if (lib->sensor_info.isp_func.pfn_sensor_start == XMEDIA_NULL ||
        lib->sensor_info.isp_func.pfn_sensor_get_property == XMEDIA_NULL) {
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    ret = lib->sensor_info.isp_func.pfn_sensor_get_property(lib->sensor_info.dev_id, &property);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    hal_sensor_read_mclk(g_sensor_clk_ch[pipe], &mclk);
    if (property.input_clock != mclk) {
        hal_sensor_set_mclk(g_sensor_clk_ch[pipe], property.input_clock);
    }

    return lib->sensor_info.isp_func.pfn_sensor_start(lib->sensor_info.dev_id);
}

xmedia_s32 drv_sensor_stop(xmedia_u32 pipe)
{
    isp_lib_info *lib = XMEDIA_NULL;

    if (pipe >= ISP_PIPE_MAX_NUM) {
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    ISP_GET_LIB(pipe, lib);
    if (lib->sensor_lib != XMEDIA_TRUE) {
        return XMEDIA_ERRCODE_NOT_EXIST;
    }

    if (lib->sensor_info.isp_func.pfn_sensor_stop == XMEDIA_NULL) {
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    return lib->sensor_info.isp_func.pfn_sensor_stop(lib->sensor_info.dev_id);
}

STAGE1_FUNC xmedia_s32 drv_sensor_set_mipi_lanes(xmedia_u32 pipe, const xmedia_sensor_mipi_lanes mipi_lanes)
{
    isp_lib_info *lib = XMEDIA_NULL;

    if (pipe >= ISP_PIPE_MAX_NUM) {
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    ISP_GET_LIB(pipe, lib);
    if (lib->sensor_lib != XMEDIA_TRUE) {
        return XMEDIA_ERRCODE_NOT_EXIST;
    }

    if (lib->sensor_info.isp_func.pfn_sensor_set_mipi_lanes == XMEDIA_NULL) {
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    return lib->sensor_info.isp_func.pfn_sensor_set_mipi_lanes(lib->sensor_info.dev_id, mipi_lanes);
}

STAGE1_FUNC xmedia_s32 drv_sensor_set_attr(xmedia_u32 pipe, const xmedia_sensor_attr *sns_attr)
{
    isp_lib_info *lib = XMEDIA_NULL;

    if (pipe >= ISP_PIPE_MAX_NUM) {
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    ISP_GET_LIB(pipe, lib);
    if (lib->sensor_lib != XMEDIA_TRUE) {
        return XMEDIA_ERRCODE_NOT_EXIST;
    }

    if (lib->sensor_info.isp_func.pfn_sensor_set_attr == XMEDIA_NULL) {
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    return lib->sensor_info.isp_func.pfn_sensor_set_attr(lib->sensor_info.dev_id, sns_attr);
}

xmedia_s32 drv_sensor_set_init_param(xmedia_u32 pipe, const xmedia_sensor_init_param *init_param)
{
    isp_lib_info *lib = XMEDIA_NULL;

    if (pipe >= ISP_PIPE_MAX_NUM || init_param == XMEDIA_NULL) {
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    ISP_GET_LIB(pipe, lib);
    if (lib->sensor_lib != XMEDIA_TRUE) {
        return XMEDIA_ERRCODE_NOT_EXIST;
    }

    if (lib->sensor_info.isp_func.pfn_sensor_set_init_param == XMEDIA_NULL) {
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    return lib->sensor_info.isp_func.pfn_sensor_set_init_param(lib->sensor_info.dev_id, init_param);
}

xmedia_s32 drv_isp_init(xmedia_u32 pipe, xmedia_isp_config *cfg)
{
    isp_context              *ctx = XMEDIA_NULL;
    xmedia_sensor_black_level blc;
    xmedia_s32                ret;
    xmedia_isp_ae_param       ae_param;
    xmedia_isp_awb_param      awb_param;
    xmedia_sensor_isp_default isp_default;

#if ISP_COST_TIME_DEBUG
    xmedia_u32 start, end;

    start = isp_get_time_stamp();
#endif

    if (pipe >= ISP_PIPE_MAX_NUM || cfg == XMEDIA_NULL) {
        puts("isp invalid param!\n");
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    ISP_GET_CTX(pipe, ctx);
    if (ctx != XMEDIA_NULL) {
        puts("isp ctx already exist!\n");
        return XMEDIA_ERRCODE_EXIST;
    }

    if (g_isp_lib[pipe].ae_lib == XMEDIA_FALSE) {
        puts("isp not register ae lib!\n");
        return XMEDIA_ERRCODE_NOT_EXIST;
    }

    if (g_isp_lib[pipe].awb_lib == XMEDIA_FALSE) {
        puts("isp not register awb lib!\n");
        return XMEDIA_ERRCODE_NOT_EXIST;
    }

    ctx = malloc(sizeof(isp_context));
    if (ctx == XMEDIA_NULL) {
        puts("isp malloc ctx failed!\n");
        return XMEDIA_ERRCODE_NO_BUFFER_FREE;
    }
    g_isp_ctx[pipe]           = ctx;
    g_isp_ctx[pipe]->lib_info = &g_isp_lib[pipe];

    g_isp_ctx[pipe]->lib_info->sensor_info.isp_func.pfn_sensor_get_isp_black_level(
        g_isp_ctx[pipe]->lib_info->sensor_info.dev_id, &blc);

    ae_param.balck_level[0]         = blc.black_level[0];
    ae_param.balck_level[1]         = blc.black_level[1];
    ae_param.balck_level[2]         = blc.black_level[2];
    ae_param.balck_level[3]         = blc.black_level[3];
    ae_param.sensor_id              = 0;
    ae_param.wdr_mode               = cfg->wdr_mode;
    ae_param.hdr_mode               = cfg->wdr_mode;
    ae_param.fps                    = cfg->fps;
    ae_param.bayer_format           = cfg->bayer_fmt;
    ae_param.blend_config.blend_en  = XMEDIA_FALSE;
    ae_param.blend_config.blend_num = 0;
    if (cfg->mode_config.work_mode == XMEDIA_ISP_WORK_MODE_MASTER) {
        ae_param.blend_config.blend_en  = cfg->mode_config.master_mode.blend_stat_enable;
        ae_param.blend_config.blend_num = cfg->mode_config.master_mode.slave_num + 1;
    }
    g_isp_ctx[pipe]->lib_info->ae_info.pfn_ae_init(g_isp_ctx[pipe]->lib_info->ae_info.alg_id, &ae_param);

    awb_param.wdr_mode = cfg->wdr_mode;
    awb_param.init_iso = 100;
    if (cfg->mode_config.work_mode == XMEDIA_ISP_WORK_MODE_MASTER) {
        awb_param.blend_config.blend_en  = cfg->mode_config.master_mode.blend_stat_enable;
        awb_param.blend_config.blend_num = cfg->mode_config.master_mode.slave_num + 1;
    }
    g_isp_ctx[pipe]->lib_info->awb_info.pfn_awb_init(g_isp_ctx[pipe]->lib_info->awb_info.alg_id, &awb_param);

    ctx->result = (xmedia_sensor_init_param *)CONFIG_PARAM_MEM_START;
    memset(ctx->result, 0, sizeof(xmedia_sensor_init_param));

    ret = hal_isp_init(pipe, cfg, &blc);
    if (ret != XMEDIA_SUCCESS) {
        puts("isp hal_isp_init failed.\n");
        return ret;
    }

    if (cfg->wdr_mode == XMEDIA_VIDEO_WDR_MODE_BUILT_IN ) {
        if (g_isp_ctx[pipe]->lib_info->sensor_info.isp_func.pfn_sensor_get_isp_default == XMEDIA_NULL ) {
            return XMEDIA_ERRCODE_NOT_SUPPORT;
        }

        g_isp_ctx[pipe]->lib_info->sensor_info.isp_func.pfn_sensor_get_isp_default(
                            g_isp_ctx[pipe]->lib_info->sensor_info.dev_id, &isp_default);
        ret = hal_isp_init_expander(pipe, isp_default.expander);
        if (ret != XMEDIA_SUCCESS) {
            puts("isp hal_isp_init_expander failed.\n");
            return ret;
        }
    }

#if ISP_COST_TIME_DEBUG
    end = isp_get_time_stamp();
    puts("isp init cost time: ");
    putdec(end - start);
    puts(" us\n");
#endif

#if ISP_COUNT_SIZE_DEBUG
    puts("isp_context size = ");
    putdec(sizeof(isp_context));
    puts("\n");
    puts("g_isp_ctx size = ");
    putdec(sizeof(g_isp_ctx));
    puts("\n");
    puts("g_isp_lib size = ");
    putdec(sizeof(g_isp_lib));
    puts("\n");
    puts("g_sensor_clk_ch size = ");
    putdec(sizeof(g_sensor_clk_ch));
    puts("\n");
    puts("g_sensor_rst_ch size = ");
    putdec(sizeof(g_sensor_rst_ch));
    puts("\n");
#endif

    return XMEDIA_SUCCESS;
}

xmedia_s32 drv_isp_exit(xmedia_u32 pipe)
{
    isp_context *ctx = XMEDIA_NULL;

    if (pipe >= ISP_PIPE_MAX_NUM) {
        puts("isp invalid param!\n");
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    ISP_GET_CTX(pipe, ctx);
    if (ctx == XMEDIA_NULL) {
        puts("isp ctx not exist!\n");
        return XMEDIA_ERRCODE_NOT_EXIST;
    }

    g_isp_ctx[pipe]->lib_info->ae_info.pfn_ae_exit(g_isp_ctx[pipe]->lib_info->ae_info.alg_id);
    g_isp_ctx[pipe]->lib_info->awb_info.pfn_awb_exit(g_isp_ctx[pipe]->lib_info->awb_info.alg_id);

    free(ctx);
    g_isp_ctx[pipe] = XMEDIA_NULL;

    return XMEDIA_SUCCESS;
}

xmedia_s32 drv_isp_start(xmedia_u32 pipe)
{
    isp_context *ctx = XMEDIA_NULL;

    if (pipe >= ISP_PIPE_MAX_NUM) {
        puts("isp invalid param!\n");
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    ISP_GET_CTX(pipe, ctx);
    if (ctx == XMEDIA_NULL) {
        puts("isp ctx not exist!\n");
        return XMEDIA_ERRCODE_NOT_EXIST;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 drv_isp_stop(xmedia_u32 pipe)
{
    isp_context *ctx = XMEDIA_NULL;

    if (pipe >= ISP_PIPE_MAX_NUM) {
        puts("isp invalid param!\n");
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    ISP_GET_CTX(pipe, ctx);
    if (ctx == XMEDIA_NULL) {
        puts("isp ctx not exist!\n");
        return XMEDIA_ERRCODE_NOT_EXIST;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 drv_isp_notifier(xmedia_u32 pipe, isp_irq_type irq_type)
{
    isp_context *ctx = XMEDIA_NULL;
    xmedia_s32   ret;

#if ISP_COST_TIME_DEBUG
    xmedia_u32 start, end;

    start = isp_get_time_stamp();
#endif

    if (pipe >= ISP_PIPE_MAX_NUM) {
        puts("isp invalid param!\n");
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    ISP_GET_CTX(pipe, ctx);
    if (ctx == XMEDIA_NULL) {
        puts("isp ctx not exist!\n");
        return XMEDIA_ERRCODE_NOT_EXIST;
    }

    if (irq_type == ISP_IRQ_TYPE_BE_FRAME_END) {
        ret = hal_isp_format_statistics(pipe);
#if ISP_COST_TIME_DEBUG
        end = isp_get_time_stamp();
        puts("isp ISP_IRQ_TYPE_BE_FRAME_END cost time: ");
        putdec(end - start);
        puts(" us\n");
#endif
        if (ret != XMEDIA_SUCCESS) {
            puts("isp hal_isp_format_statistics failed!\n");
            return ret;
        }
    } else if (irq_type == ISP_IRQ_TYPE_FE_FRAME_START) {
        ret = hal_isp_sync(pipe);
#if ISP_COST_TIME_DEBUG
        end = isp_get_time_stamp();
        puts("isp ISP_IRQ_TYPE_FE_FRAME_START cost time: ");
        putdec(end - start);
        puts(" us\n");
#endif
        if (ret != XMEDIA_SUCCESS) {
            puts("isp hal_isp_sync failed!\n");
            return ret;
        }
    }

    return XMEDIA_SUCCESS;
}

// TODO: 统计耗时
xmedia_s32 drv_isp_process(xmedia_u32 pipe)
{
    isp_context            *ctx = XMEDIA_NULL;
    xmedia_s32              ret;
    xmedia_isp_statistics   isp_stat       = { 0 };
    xmedia_isp_ae_info      ae_info        = { 0 };
    xmedia_isp_awb_info     awb_info       = { 0 };
    xmedia_isp_result       isp_result     = { 0 };
    xmedia_sensor_regs_info reg_info       = { 0 };
    static xmedia_u64       last_frame_cnt = -1;
    xmedia_u64              sys_gain;
    xmedia_s32              i;

#if ISP_COST_TIME_DEBUG
    xmedia_u32 start, end;

    start = isp_get_time_stamp();
#endif

    if (pipe >= ISP_PIPE_MAX_NUM) {
        puts("isp invalid param!\n");
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    ISP_GET_CTX(pipe, ctx);
    if (ctx == XMEDIA_NULL) {
        puts("isp ctx not exist!\n");
        return XMEDIA_ERRCODE_NOT_EXIST;
    }

    ret = hal_isp_stat(pipe, &isp_stat);
    if (ret != XMEDIA_SUCCESS) {
        puts("isp hal_isp_stat failed!\n");
        return ret;
    }

    if (last_frame_cnt == isp_stat.ae_stat.frame_cnt) {
        return XMEDIA_ERRCODE_NOT_READY;
    }
    last_frame_cnt = isp_stat.ae_stat.frame_cnt;

    ae_info.frame_cnt             = isp_stat.ae_stat.frame_cnt;
    ae_info.be_ae_stat_global_avg = &isp_stat.ae_stat.ae_stat_global_avg;
    ae_info.be_ae_stat_hist       = isp_stat.ae_stat.ae_stat_hist;
    ctx->lib_info->ae_info.pfn_ae_run(ctx->lib_info->ae_info.alg_id, &ae_info, &isp_result.ae_result);

    // awb分块统计信息是紧凑排布的，即行与行之间没有空白内存
    awb_info.frame_cnt              = isp_stat.awb_stat.frame_cnt;
    awb_info.awb_zone_row           = isp_stat.awb_stat.awb_zone_row_num;
    awb_info.awb_zone_col           = isp_stat.awb_stat.awb_zone_col_num;
    awb_info.awb_width              = isp_stat.awb_stat.awb_width;
    awb_info.awb_height             = isp_stat.awb_stat.awb_height;
    awb_info.awb_stat_switch        = XMEDIA_ISP_AE_SWITCH_AFTER_DGN2;
    awb_info.awb_stat_local_avg_r   = (xmedia_isp_awb_stat_local_avg_r *)isp_stat.awb_stat.awb_zone_avg_r;
    awb_info.awb_stat_local_avg_g   = (xmedia_isp_awb_stat_local_avg_g *)isp_stat.awb_stat.awb_zone_avg_g;
    awb_info.awb_stat_local_avg_b   = (xmedia_isp_awb_stat_local_avg_b *)isp_stat.awb_stat.awb_zone_avg_b;
    awb_info.stat_local_valid_count = (xmedia_isp_awb_stat_local_valid_count *)isp_stat.awb_stat.awb_zone_valid_count;
    ctx->lib_info->awb_info.pfn_awb_run(ctx->lib_info->awb_info.alg_id, &awb_info, &isp_result.awb_result);

    sys_gain = (xmedia_u64)isp_result.ae_result.again * isp_result.ae_result.dgain * isp_result.ae_result.isp_dgain /
               (1024 * 1024 * 256 / 64);
    ctx->result->exp_time = isp_result.ae_result.int_time[0];
    ctx->result->again    = isp_result.ae_result.again;
    ctx->result->dgain    = isp_result.ae_result.dgain;
    ctx->result->ispdgain = isp_result.ae_result.isp_dgain;
    ctx->result->init_iso = isp_result.ae_result.iso;
    ctx->result->exposure = (xmedia_u64)isp_result.ae_result.int_time[0] * sys_gain;
    ctx->result->wb_rgain = isp_result.awb_result.white_balance_gain[0];
    ctx->result->wb_ggain = isp_result.awb_result.white_balance_gain[1];
    ctx->result->wb_bgain = isp_result.awb_result.white_balance_gain[3];
    for (i = 0; i < XMEDIA_SENSOR_CCM_MATRIX_SIZE; i++) {
        if (isp_result.awb_result.color_matrix[i] < 0) {
            ctx->result->ccm[i] = CCM_MINUS_PARAM_TH - isp_result.awb_result.color_matrix[i];
        } else {
            ctx->result->ccm[i] = isp_result.awb_result.color_matrix[i];
        }
    }
    ctx->lib_info->sensor_info.isp_func.pfn_sensor_get_reg_info(ctx->lib_info->sensor_info.dev_id, &reg_info);

    ret = hal_isp_update(pipe, &isp_result, &reg_info);
    if (ret != XMEDIA_SUCCESS) {
        puts("isp hal_isp_update failed!\n");
        return ret;
    }

#if 0
    puts("debug dudu r");
    puthex(ctx->result->wb_rgain);
    puts(" g");
    puthex(ctx->result->wb_ggain);
    puts(" b");
    puthex(ctx->result->wb_bgain);
    puts("\n");
    puts("isp process frame count  [");
    putdec(isp_stat.ae_stat.frame_cnt);
    puts("]\n");
#endif

#if ISP_COST_TIME_DEBUG
    end = isp_get_time_stamp();
    puts("isp process cost time: ");
    putdec(end - start);
    puts(" us\n");
#endif

    return XMEDIA_SUCCESS;
}

xmedia_s32 drv_isp_config(xmedia_u32 pipe)
{
    xmedia_s32 ret;

#if ISP_COST_TIME_DEBUG
    xmedia_u32 start, end;

    start = isp_get_time_stamp();
#endif

    if (pipe >= ISP_PIPE_MAX_NUM) {
        puts("isp invalid param!\n");
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    ret = hal_isp_config(pipe);
    if (ret != XMEDIA_SUCCESS) {
        puts("isp hal_isp_config failed!\n");
        return ret;
    }

#if ISP_COST_TIME_DEBUG
    end = isp_get_time_stamp();
    puts("isp config cost time: ");
    putdec(end - start);
    puts(" us\n");
#endif

    return XMEDIA_SUCCESS;
}