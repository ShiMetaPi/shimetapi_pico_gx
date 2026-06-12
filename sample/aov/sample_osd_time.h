#ifndef __SAMPLE_OSD_TIME_H__
#define __SAMPLE_OSD_TIME_H__

#include "sample_comm_aov.h"
#include "sample_comm.h"
#include "drv_osd_draw_time.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SUPPORT_OSD_TIME

#define RGN_DEFAULT_WIDTH  (352)
#define RGN_DEFAULT_HEIGHT (60)

xmedia_s32 sample_osd_time_init();
xmedia_s32 sample_osd_time_attach_to_chn(xmedia_chn_info *mpp_chn, xmedia_rgn_chn_attr *chn_attr);
xmedia_s32 sample_osd_time_start();
xmedia_s32 sample_osd_time_update(xmedia_u32 time_interval, sample_aov_work_mode work_mode);
xmedia_s32 sample_osd_time_destroy();
#endif

#ifdef __cplusplus
}
#endif

#endif //__SAMPLE_OSD_TIME_H__
