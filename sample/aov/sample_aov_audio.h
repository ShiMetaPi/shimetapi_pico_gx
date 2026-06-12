#ifndef __SAMPLE_AOV_AUDIO_H__
#define __SAMPLE_AOV_AUDIO_H__

#include "sample_comm.h"
#include "xmedia_audio_common.h"
#include "sample_comm_audio.h"

//#define NORMAL_SUPPORT_AUDIO 1

#ifdef NORMAL_SUPPORT_AUDIO
xmedia_s32 sample_aov_audio_init();
xmedia_s32 sample_aov_audio_exit();
xmedia_s32 sample_aov_audio_start_play();
xmedia_s32 sample_aov_audio_stop_play();
xmedia_s32 sample_aov_audio_start_record();
xmedia_s32 sample_aov_audio_stop_record();
#endif

#endif //__SAMPLE_AOV_AUDIO_H__