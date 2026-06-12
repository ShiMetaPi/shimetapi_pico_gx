#ifndef __QUICKSTART_COMM_H__
#define __QUICKSTART_COMM_H__

#include <stdio.h>
#include "media_comm.h"

typedef struct quickstart_comm_venc_get_stream_param {
    xmedia_bool thread_start;
    xmedia_u32  venc_chn[VENC_MAX_CHN_NUM];
    xmedia_s32  s32Cnt;
} quickstart_comm_venc_get_stream_param;

xmedia_s32 quickstart_comm_venc_start_get_stream(xmedia_s32 venc_chn[], xmedia_s32 venc_chn_num);
xmedia_s32 quickstart_comm_venc_stop_get_stream(void);
xmedia_s32 quickstart_comm_svp_thread_create(xmedia_void);
xmedia_s32 quickstart_comm_test();
xmedia_bool quickstart_comm_get_ai_detect();
xmedia_s32 quickstart_comm_set_ai_detect(xmedia_bool ai_detect);



#endif //__QUICKSTART_COMM_H__
