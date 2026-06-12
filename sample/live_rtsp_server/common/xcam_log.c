/*
 * Copyright (c) XCAM. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdarg.h>

#include "xcam_log.h"

#define COLOR_NONE      "\033[m"
#define COLOR_RED       "\033[0;32;31m"
#define COLOR_YELLOW    "\033[1;33m"

#define XCAM_LOG_MAX_PREFIX 64
#define XCAM_LOG_MAX_PER_CONTENT 256

typedef struct {
    xcam_log_attr attr;
    xcam_u8 *log_buffer;
} xcam_log_global;

static pthread_mutex_t g_log_mutex = PTHREAD_MUTEX_INITIALIZER;
static xcam_log_global *g_xcam_log = XCAM_NULL;

#define XCAM_LOG_MUTEX_LOCK()                  \
    do {                                         \
        (void)pthread_mutex_lock(&g_log_mutex);  \
    } while (0)

#define XCAM_LOG_MUTEX_UNLOCK()                 \
    do {                                          \
        (void)pthread_mutex_unlock(&g_log_mutex); \
    } while (0)

static xcam_void do_logout(xcam_char *prefix, xcam_char *content)
{
    printf("%s%s", prefix, content);
    printf("%s", COLOR_NONE);

    return;
}

xcam_s32 xcam_log_init(xcam_void)
{
    xcam_s32 ret = XCAM_SUCCESS;
    XCAM_LOG_MUTEX_LOCK();

    // already inited
    if (g_xcam_log != XCAM_NULL) {
        ret = XCAM_SUCCESS;
        goto __err_unlock__;
    }

    g_xcam_log = (xcam_log_global *)malloc(sizeof(xcam_log_global));
    if (g_xcam_log == XCAM_NULL) {
        ret = XCAM_FAILURE;
        goto __err_unlock__;
    }

    memset(g_xcam_log, 0x0, sizeof(xcam_log_global));

    g_xcam_log->attr.mode = XCAM_LOG_MODE_PRINTF;
    g_xcam_log->attr.level = XCAM_LOG_LEVEL_ERR;

    XCAM_LOG_MUTEX_UNLOCK();
    return XCAM_SUCCESS;

__err_unlock__:
    XCAM_LOG_MUTEX_UNLOCK();
    return ret;
}

xcam_void xcam_log_deinit(xcam_void)
{
    XCAM_LOG_MUTEX_LOCK();

    // already deinited
    if (g_xcam_log == XCAM_NULL) {
        XCAM_LOG_MUTEX_UNLOCK();
        return;
    }

    free(g_xcam_log);
    g_xcam_log = XCAM_NULL;

    XCAM_LOG_MUTEX_UNLOCK();
    return;
}

xcam_s32 xcam_log_set_attr(xcam_log_attr *attr)
{
    XCAM_LOG_MUTEX_LOCK();

    if (g_xcam_log == XCAM_NULL) {
        XCAM_LOG_MUTEX_UNLOCK();
        return XCAM_FAILURE;
    }

    memcpy(&g_xcam_log->attr, attr, sizeof(xcam_log_attr));

    XCAM_LOG_MUTEX_UNLOCK();

    return XCAM_SUCCESS;
}

xcam_s32 xcam_log_get_attr(xcam_log_attr *attr)
{
    XCAM_LOG_MUTEX_LOCK();

    if (g_xcam_log == XCAM_NULL) {
        XCAM_LOG_MUTEX_UNLOCK();
        return XCAM_FAILURE;
    }

    memcpy(attr, &g_xcam_log->attr, sizeof(xcam_log_attr));

    XCAM_LOG_MUTEX_UNLOCK();

    return XCAM_SUCCESS;
}

xcam_void xcam_log_output(const xcam_char *module, xcam_log_level level, const xcam_char *func, xcam_u32 line, const xcam_char *format, ...)
{
    xcam_char prefix[XCAM_LOG_MAX_PREFIX] = {0};
    xcam_char content[XCAM_LOG_MAX_PER_CONTENT] = {0};
    va_list args;

    if (!g_xcam_log || level > g_xcam_log->attr.level) {
        return;
    }

    switch (level) {
        case XCAM_LOG_LEVEL_DBG:
            snprintf(prefix, 64, "%s[%s-DEBUG][%s-%u]: ", COLOR_NONE, module, func, line);
            break;
        case XCAM_LOG_LEVEL_INFO:
            snprintf(prefix, 64, "%s[%s-INFO][%s-%u]: ", COLOR_NONE, module, func, line);
            break;
        case XCAM_LOG_LEVEL_WARN:
            snprintf(prefix, 64, "%s[%s-WARN][%s-%u]: ", COLOR_YELLOW, module, func, line);
            break;
        case XCAM_LOG_LEVEL_ERR:
            snprintf(prefix, 64, "%s[%s-ERROR][%s-%u]: ", COLOR_RED, module, func, line);
            break;
        case XCAM_LOG_LEVEL_FATAL:
            snprintf(prefix, 64, "%s[%s-FATAL][%s-%u]: ", COLOR_RED, module, func, line);
            break;
        default:
            return;
    }

    va_start(args, format);

    vsnprintf(content, XCAM_LOG_MAX_PER_CONTENT, format, args);

    // if the content is too long, ensure it has an end.
    if (content[XCAM_LOG_MAX_PER_CONTENT - 1] != '\0') {
        content[XCAM_LOG_MAX_PER_CONTENT - 5] = '.';
        content[XCAM_LOG_MAX_PER_CONTENT - 4] = '.';
        content[XCAM_LOG_MAX_PER_CONTENT - 3] = '.';
        content[XCAM_LOG_MAX_PER_CONTENT - 2] = '\n';
        content[XCAM_LOG_MAX_PER_CONTENT - 1] = '\0';
    }

    va_end(args);

    do_logout(prefix, content);

    return;
}
