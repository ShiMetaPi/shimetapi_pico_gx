/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#ifndef __LOG_BASE_H__
#define __LOG_BASE_H__

#define LOG_LEVEL_ERR 1
#define LOG_LEVEL_WARN 2
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_PRINT 3
#define LOG_LEVEL_DEBUG 5

/**
 * RED    [error]
 * YELLOW [warn]
 * GREEN  [info]
 * GREEN  [trace]
 * BLUE   [debug]
 */

#ifdef PRINT_COLOR_ON
#define RED      "\033[1;31m"
#define YELLOW   "\033[1;33m"
#define GREEN    "\033[1;32m"
#define BLUE     "\033[1;34m"
#define NONE     "\033[m"
#else
#define RED      ""
#define YELLOW   ""
#define GREEN    ""
#define BLUE     ""
#define NONE     ""
#endif

#define TAG "LOG"

extern unsigned int g_loglevel;

#define LOGE(fmt, args...) do { \
    if (g_loglevel >= LOG_LEVEL_ERR) { \
        printf(RED "[%s E] <%s : %d> " fmt NONE, \
                TAG, __func__, __LINE__, ##args); \
    } \
} while (0)

#define LOGW(fmt, args...) do { \
    if (g_loglevel >= LOG_LEVEL_WARN) { \
        printf(YELLOW "[%s W] <%s : %d> " fmt NONE, \
                TAG, __func__, __LINE__, ##args); \
    } \
} while (0)

#define LOGI(fmt, args...) do { \
    if (g_loglevel >= LOG_LEVEL_INFO) { \
        printf(GREEN "[%s I] <%s : %d> " fmt NONE, \
                TAG, __func__, __LINE__, ##args); \
    } \
} while (0)

#define LOGD(fmt, args...) do { \
    if (g_loglevel >= LOG_LEVEL_DEBUG) { \
        printf(NONE "[%s D] <%s : %d> " fmt NONE, \
                TAG, __func__, __LINE__, ##args); \
    } \
} while (0)

#define LOGT(fmt, args...) do { \
    if (g_loglevel >= LOG_LEVEL_PRINT) { \
        printf(GREEN fmt NONE, ##args); \
    } \
} while (0)

#endif
