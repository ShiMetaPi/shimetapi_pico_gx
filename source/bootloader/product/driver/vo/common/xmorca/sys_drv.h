/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef SYS_DRV_H
#define SYS_DRV_H

#include "drv_sys.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */

/*
 * End : Segment compress parameter data definition.
 */
xmedia_s32 sys_mod_init(xmedia_void);
xmedia_s32 sys_mod_exit(xmedia_void);

xmedia_s32 sys_mod_ctrl(xmedia_chn_info *chn_info, sys_func_id func_id, xmedia_void *io_args);


#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef SYS_DRV_H */

