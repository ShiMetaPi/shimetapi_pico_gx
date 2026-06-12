/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __UVC_APP_LOG_H__
#define __UVC_APP_LOG_H__

#include "log_base.h"

#undef TAG
#define TAG "UVC_APP"

#define app_loge LOGE
#define app_logw LOGW
#define app_logi LOGI
#define app_logt LOGT
#define app_logd LOGD

#define check_null_goto(ptr, tag) do { \
    if ((ptr) == NULL) { \
        app_loge("invalid param.\n"); \
        goto tag; \
    } \
} while (0)

#endif
