#include "stdio.h"
#include "defines.h"
#include "interrupt.h"
#include "drv_vi.h"
#include "drv_isp.h"
#include "hal_vi.h"
#include <timer.h>

#define VICAP_IRQ_DEBUG  0
#define VIPROC_IRQ_DEBUG 0

static xmedia_bool vi_vicap_is_report_int(xmedia_u32 fe_id, xmedia_u32 int_status)
{
    xmedia_u32 fe_int_mask[VI_MAX_PHY_PIPE_NUM] = { 0x1 << 5, 0x1 << 6, 0x1 << 7, 0x1 << 8 };

    if (int_status & fe_int_mask[fe_id]) {
        return XMEDIA_TRUE;
    }

    return XMEDIA_FALSE;
}

static xmedia_void vi_vicap_irq_callback_proc(xmedia_u32 vicap_wch, xmedia_u32 irq_status)
{
    if (irq_status & VI_CAP_FRAME_START_INT) {
        drv_isp_notifier(vicap_wch, ISP_IRQ_TYPE_FE_FRAME_START);
    }

    if (irq_status & VI_CAP_FRAME_EARLY_INT) {
        // vicap reg newer
        hal_vi_vicap_reg_newer(vicap_wch);
    }

    if (irq_status & VI_CAP_FRAME_LOWDELAY_INT) {
    }

    if (irq_status & VI_CAP_FRAME_END_INT) {
        drv_isp_notifier(vicap_wch, ISP_IRQ_TYPE_FE_FRAME_END);
    }

    if (irq_status & VI_CAP_FRAME_TIMEOUT_INT) {
        puts("vicap timeout int!\n");
    }

    if (irq_status & VI_CAP_FRAME_ERR_INT) {
        puthex(irq_status);
        puts("vicap err int!\n");
    }
}

ATTRIBUTE_ISR xmedia_void vi_vicap_irq_route(xmedia_void)
{
    xmedia_s32 vicap_wch;
    xmedia_u32 vicap_int_status;
    xmedia_u32 vicap_int_mask;
    xmedia_u32 irq_status;

    vicap_int_status = hal_vi_vicap_get_top_int_status(0);
    vicap_int_mask = hal_vi_vicap_get_top_int_mask(0);
    vicap_int_status &= (~vicap_int_mask);

    for (vicap_wch = VI_MAX_PHY_PIPE_NUM - 1; vicap_wch >= 0; vicap_wch--) {
        if (vi_vicap_is_report_int(vicap_wch, vicap_int_status) == XMEDIA_FALSE) {
            continue;
        }

        irq_status = hal_vi_vicap_get_chn_int_status(vicap_wch);
        hal_vi_vicap_clear_int(vicap_wch, irq_status);

#if VICAP_IRQ_DEBUG
        puts("vicap irq_status 0x");
        puthex(irq_status);
        puts("\n");
#endif

        vi_vicap_irq_callback_proc(vicap_wch, irq_status);
    }
}

static xmedia_void vi_viproc_irq_callback_proc(xmedia_u32 pipe, xmedia_u32 irq_status)
{
    if (irq_status & VI_PROC_FRAME_START_INT) {
    }

    if (irq_status & VI_PROC_FRAME_EARLY_INT) {
        drv_isp_config(pipe); // 配置isp be算法寄存器
        hal_vi_viproc_reg_start(0);
    }

    if (irq_status & VI_PROC_FRAME_END_INT || irq_status & VI_PROC_FRAME_LIST_INT) {
        drv_isp_notifier(pipe, ISP_IRQ_TYPE_BE_FRAME_END);
    }

    if (irq_status & VI_PROC_LOWDELAY_INT) {
    }

    if (irq_status & VI_PROC_FRAME_ERR_INT) {
        puts("viproc err int!\n");
    }

    if (irq_status & VI_PROC_FRAME_TIMEOUT_INT) {
        puts("viproc timeout int!\n");
    }

    if (irq_status & VI_PROC_FRAME_ERR_IGNORE_INT) {
        puts("viproc err ignore int!\n");
    }
}

ATTRIBUTE_ISR xmedia_void vi_viproc_irq_route(xmedia_void)
{
    xmedia_u32 pipe = 0; // 对外暂时仅支持pipe 0
    xmedia_u32 cl_irq_status;
    xmedia_u32 irq_status;
    xmedia_u32 irq_mask;
    xmedia_u32 early_mask;

    irq_status = hal_viproc_get_reg_public_int_status(0); // viproc irq status
    irq_mask = hal_viproc_get_reg_public_int_mask(0); // irq mask
    early_mask = hal_viproc_common_get_reg_early_mask(0); // early irq mask

    cl_irq_status = irq_status;
    irq_status &= (~(irq_mask | early_mask));

    hal_viproc_clr_reg_int(0, cl_irq_status); // clear int

#if VIPROC_IRQ_DEBUG
    puts("viproc irq_status 0x");
    puthex(irq_status);
    puts("\n");
#endif

    vi_viproc_irq_callback_proc(pipe, irq_status);
}

STAGE1_FUNC xmedia_void drv_mipi_init(xmedia_void)
{
    hal_mipi_init();
}

STAGE1_FUNC xmedia_s32 drv_mipi_enable(xmedia_s32 dev, xmedia_vi_dev_config *dev_config)
{
    return hal_mipi_enable(dev, dev_config);
}

xmedia_s32 drv_vi_init(xmedia_void)
{
    xmedia_s32 ret;
    xmedia_u32 vicap_irq_num;
    xmedia_u32 viproc_irq_num;

    // 注册vicap中断、viproc中断
    vicap_irq_num = hal_vi_get_vicap_irq_num(0);
    viproc_irq_num = hal_vi_get_viproc_irq_num(0);
    ret = drv_irq_register(vicap_irq_num, vi_vicap_irq_route);
    if (ret < 0) {
        puts("Fail to register vicap irq!\n");
        return ret;
    }
    ret = drv_irq_register(viproc_irq_num, vi_viproc_irq_route);
    if (ret < 0) {
        puts("Fail to register viproc irq!\n");
        drv_irq_unregister(vicap_irq_num);
        return ret;
    }

    hal_vi_init();

    return XMEDIA_SUCCESS;
}

xmedia_s32 drv_vi_exit(xmedia_void)
{
    xmedia_u32 vicap_irq_num;
    xmedia_u32 viproc_irq_num;

    // 反注册中断
    vicap_irq_num = hal_vi_get_vicap_irq_num(0);
    viproc_irq_num = hal_vi_get_viproc_irq_num(0);
    drv_irq_unregister(vicap_irq_num);
    drv_irq_unregister(viproc_irq_num);

    hal_vi_exit();

    return XMEDIA_SUCCESS;
}

xmedia_s32 drv_vi_enable_dev(xmedia_s32 dev)
{
    return hal_vi_enable_dev(dev);
}

xmedia_s32 drv_vi_disable_dev(xmedia_s32 dev, xmedia_vi_dev_config *dev_config)
{
    hal_vi_disable_dev(dev, dev_config);

    return XMEDIA_SUCCESS;
}

xmedia_s32 drv_vi_set_dev_bind_pipe(xmedia_s32 dev, xmedia_s32 pipe)
{
    return hal_vi_set_dev_bind_pipe(dev, pipe);
}

xmedia_s32 drv_vi_set_dev_unbind_pipe(xmedia_s32 dev, xmedia_s32 pipe)
{
    hal_vi_set_dev_unbind_pipe(dev, pipe);

    return XMEDIA_SUCCESS;
}

xmedia_s32 drv_vi_start_pipe(xmedia_s32 pipe, xmedia_vi_pipe_config *pipe_config)
{
    xmedia_s32 ret;

    ret = hal_vi_start_vicap(pipe, pipe_config);
    if (ret != XMEDIA_SUCCESS) {
        puts("start vicap failed\n");
        return ret;
    }

    // 调用isp update be接口，配置第一帧算法参数
    drv_isp_config(pipe);

    ret = hal_vi_start_viproc(pipe, pipe_config);
    if (ret != XMEDIA_SUCCESS) {
        puts("start viproc failed\n");
        hal_vi_stop_vicap(pipe);
        return ret;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 drv_vi_stop_pipe(xmedia_s32 pipe)
{
    hal_vi_stop_viproc(pipe);
    hal_vi_stop_vicap(pipe);

    return XMEDIA_SUCCESS;
}
