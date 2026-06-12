/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __SAMPLE_COMM_IRCUT_H__
#define __SAMPLE_COMM_IRCUT_H__

#include <pthread.h>

#include "common.h"
#include "defines.h"
#include "xmedia_sys.h"
#include "xmedia_type.h"
#include "xmedia_mmz.h"
#include "sample_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


xmedia_s32 sample_comm_ircut_set_status_normal(sample_comm_board_type board_type, xmedia_s32 vi_dev);

xmedia_s32 sample_comm_ircut_set_status_ir(sample_comm_board_type board_type, xmedia_s32 vi_dev);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


#endif



