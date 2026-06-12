#ifndef __DRV_VI_H_
#define __DRV_VI_H_

#include "xmedia_type.h"
#include "xmedia_vi.h"
#include "compile.h"

xmedia_s32 drv_vi_init(xmedia_void);
xmedia_s32 drv_vi_exit(xmedia_void);

xmedia_void drv_mipi_init(xmedia_void);
xmedia_s32 drv_mipi_enable(xmedia_s32 dev, xmedia_vi_dev_config *dev_config);

xmedia_s32 drv_vi_enable_dev(xmedia_s32 dev);
xmedia_s32 drv_vi_disable_dev(xmedia_s32 dev, xmedia_vi_dev_config *dev_config);

xmedia_s32 drv_vi_set_dev_bind_pipe(xmedia_s32 dev, xmedia_s32 pipe);
xmedia_s32 drv_vi_set_dev_unbind_pipe(xmedia_s32 dev, xmedia_s32 pipe);

xmedia_s32 drv_vi_start_pipe(xmedia_s32 pipe, xmedia_vi_pipe_config *pipe_config);
xmedia_s32 drv_vi_stop_pipe(xmedia_s32 pipe);

#endif
