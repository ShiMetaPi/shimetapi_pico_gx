/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef SAMPLE_MSENSOR_H
#define SAMPLE_MSENSOR_H

#include "xmedia_type.h"

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

xmedia_s32 sample_msensor_init(xmedia_s32 dev_cnt, xmedia_s32 *dev);
xmedia_void sample_msensor_exit(xmedia_s32 dev_cnt, xmedia_s32 *dev);
xmedia_s32 sample_msensor_start_dump_data(xmedia_s32 dev);
xmedia_s32 sample_msensor_stop_dump_data(xmedia_s32 dev);
xmedia_s32 sample_msensor_parse_data(xmedia_void);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif
