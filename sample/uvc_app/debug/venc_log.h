/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#ifndef __VENC_LOG_H__
#define __VENC_LOG_H__

#include "log_base.h"

#undef TAG
#define TAG "VENC"

#define venc_loge LOGE
#define venc_logw LOGW
#define venc_logi LOGI
#define venc_logd LOGD

#define func_entry() venc_logd("entry\n")
#define func_success() venc_logd("success\n")
#define func_fail() venc_loge("failed\n")

#define check_null_goto(ptr, tag) do { \
    if ((ptr) == NULL) { \
        venc_loge("invalid param.\n"); \
        goto tag; \
    } \
} while (0)

#endif
