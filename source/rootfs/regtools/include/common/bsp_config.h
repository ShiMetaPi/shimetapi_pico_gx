/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __XMEDIA_CONFIG_H__
#define __XMEDIA_CONFIG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#if defined(TARGET_X86)

//#define PC_EMULATOR
/*#undefine PC_EMULATOR*/

/* Whether to use the BTRACE debug library */
/*#define USE_BTRACE*/
/*#undefine USE_BTRACE*/
#endif

/* DEBUG MODE */
#define DEBUG
/*#define RELEASE*/

#define OS_LINUX

/**/

/************ -D in Makefile ******/
/* Simulate the program on ARM, Use -DARM_CPU*/
/*#define ARM_CPU*/

/*Simulate the program on ZSP, Use -DZSP_CPU*/
/*#define ZSP_CPU*/

/* Whether to perform the print function */
#define LOGQUEUE

/* Whether to do Arm and ZSP communication statistics */
#define AZ_STAT

/* multi-task logqueue*/
#define MULTI_TASK_LOGQUEUE

/* Using inerrupt mode on ARM and ZSP communication */
#define USE_AZ_INT

#define STAT

#if 0
/*Turn on both when test video */
#define AZ_POOL_LOW
#define AZ_MAGIC_LOW
#endif

//#define H264STREAM_CORRECT
/*RingBuffer count */
#define RBSTAT

#define BITSTREAM_ENC_CHECKSUM
#define BITSTREAM_DEC_CHECKSUM

#define RTSP_VOD

#define SAVE_VOICE 1

/* Support network on-demand */
#define MPLAYER_NETWORK

#define DEMO_MEDIA

#define DEMO_VOICE
#define DEMO_VIDEO_ENC
#define DEMO_VIDEO_DEC

//#define SYNC_USE_COND

#define AZPOOLS_X

#if defined(IMAGESIZE_CIF)
#define CONFIG_VIU_CAPTURE_DOWNSCALING //CIF
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __XMEDIA_CONFIG_H__ */
