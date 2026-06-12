/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include "drv_sys.h"
#include "sys_drv.h"
#include "sys_hal.h"

xmedia_s32 drv_sys_mod_ctrl(xmedia_chn_info *chn_info, sys_func_id func_id, xmedia_void *io_args)
{
    xmedia_s32 ret = XMEDIA_FAILURE;

    ret = sys_mod_ctrl(chn_info, func_id, io_args);

    return ret;
}
