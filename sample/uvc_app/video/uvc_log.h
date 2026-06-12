/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#ifndef __UVC_LOG_H__
#define __UVC_LOG_H__

#include "log_base.h"

#undef TAG
#define TAG "UVC"

#define uvc_loge LOGE
#define uvc_logw LOGW
#define uvc_logi LOGI
#define uvc_logt LOGT
#define uvc_logd LOGD

#define func_entry() uvc_logd("entry\n")
#define func_success() uvc_logd("success\n")
#define func_fail() uvc_loge("failed\n")

#define check_null_goto(ptr, tag) do { \
    if ((ptr) == NULL) { \
        uvc_loge("invalid param.\n"); \
        goto tag; \
    } \
} while (0)

#endif
