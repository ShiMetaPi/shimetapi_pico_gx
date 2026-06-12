#ifndef __DRV_VO_H__
#define __DRV_VO_H__
#include "drv_vo_comm.h"

int init_vo(xmedia_void);
int deinit_vo(xmedia_void);
int start_vo(unsigned int dev, xmedia_s32 screen_type);
int stop_vo(unsigned int dev);
int start_videolayer(unsigned int layer, unsigned long addr, unsigned int strd, xmedia_video_rect layer_rect);
int stop_videolayer(unsigned int layer);
int start_gx(unsigned int layer, unsigned long addr, unsigned int strd, xmedia_video_rect gx_rect);
int stop_gx(unsigned int layer);
#endif