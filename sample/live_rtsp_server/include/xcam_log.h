/*
 * Copyright (c) XCAM. All rights reserved.
 */

#ifndef __XCAM_LOG_H__
#define __XCAM_LOG_H__

#include "xcam_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

typedef enum {
    XCAM_LOG_LEVEL_FATAL  = 0,
    XCAM_LOG_LEVEL_ERR    = 1,
    XCAM_LOG_LEVEL_WARN   = 2,
    XCAM_LOG_LEVEL_INFO   = 3,
    XCAM_LOG_LEVEL_DBG    = 4
} xcam_log_level;

typedef enum {
    XCAM_LOG_MODE_PRINTF,
    XCAM_LOG_MODE_FILE
} xcam_log_mode;

typedef struct {
    xcam_log_level level;
    xcam_log_mode mode;
    xcam_u32 log_buffer_size;
} xcam_log_attr;

#define XCAM_LOG_TRACE(module, level, format...) \
    do { \
        xcam_log_output(module, level, __FUNCTION__, __LINE__, format);  \
    } while (0)

// use this config macro to optimize code size
#ifndef CONFIG_XCAM_LOG_LEVEL
#define CONFIG_XCAM_LOG_LEVEL 4
#endif

#if (CONFIG_XCAM_LOG_LEVEL == XCAM_LOG_LEVEL_FATAL)
#define XCAM_LOG_FATAL_PRINT(module, format...) XCAM_LOG_TRACE(module, XCAM_LOG_LEVEL_FATAL, format)
#define XCAM_LOG_ERR_PRINT(module, format...)
#define XCAM_LOG_WARN_PRINT(module, format...)
#define XCAM_LOG_INFO_PRINT(module, format...)
#define XCAM_LOG_DBG_PRINT(module, format...)
#elif (CONFIG_XCAM_LOG_LEVEL == XCAM_LOG_LEVEL_ERR)
#define XCAM_LOG_FATAL_PRINT(module, format...) XCAM_LOG_TRACE(module, XCAM_LOG_LEVEL_FATAL, format)
#define XCAM_LOG_ERR_PRINT(module, format...)   XCAM_LOG_TRACE(module, XCAM_LOG_LEVEL_ERR, format)
#define XCAM_LOG_WARN_PRINT(module, format...)
#define XCAM_LOG_INFO_PRINT(module, format...)
#define XCAM_LOG_DBG_PRINT(module, format...)
#elif (CONFIG_XCAM_LOG_LEVEL == XCAM_LOG_LEVEL_WARN)
#define XCAM_LOG_FATAL_PRINT(module, format...) XCAM_LOG_TRACE(module, XCAM_LOG_LEVEL_FATAL, format)
#define XCAM_LOG_ERR_PRINT(module, format...)   XCAM_LOG_TRACE(module, XCAM_LOG_LEVEL_ERR, format)
#define XCAM_LOG_WARN_PRINT(module, format...)  XCAM_LOG_TRACE(module, XCAM_LOG_LEVEL_WARN, format)
#define XCAM_LOG_INFO_PRINT(module, format...)
#define XCAM_LOG_DBG_PRINT(module, format...)
#elif (CONFIG_XCAM_LOG_LEVEL == XCAM_LOG_LEVEL_INFO)
#define XCAM_LOG_FATAL_PRINT(module, format...) XCAM_LOG_TRACE(module, XCAM_LOG_LEVEL_FATAL, format)
#define XCAM_LOG_ERR_PRINT(module, format...)   XCAM_LOG_TRACE(module, XCAM_LOG_LEVEL_ERR, format)
#define XCAM_LOG_WARN_PRINT(module, format...)  XCAM_LOG_TRACE(module, XCAM_LOG_LEVEL_WARN, format)
#define XCAM_LOG_INFO_PRINT(module, format...)  XCAM_LOG_TRACE(module, XCAM_LOG_LEVEL_INFO, format)
#define XCAM_LOG_DBG_PRINT(module, format...)
#else
#define XCAM_LOG_FATAL_PRINT(module, format...) XCAM_LOG_TRACE(module, XCAM_LOG_LEVEL_FATAL, format)
#define XCAM_LOG_ERR_PRINT(module, format...)   XCAM_LOG_TRACE(module, XCAM_LOG_LEVEL_ERR, format)
#define XCAM_LOG_WARN_PRINT(module, format...)  XCAM_LOG_TRACE(module, XCAM_LOG_LEVEL_WARN, format)
#define XCAM_LOG_INFO_PRINT(module, format...)  XCAM_LOG_TRACE(module, XCAM_LOG_LEVEL_INFO, format)
#define XCAM_LOG_DBG_PRINT(module, format...)   XCAM_LOG_TRACE(module, XCAM_LOG_LEVEL_DBG, format)
#endif

xcam_s32 xcam_log_init(xcam_void);

xcam_void xcam_log_deinit(xcam_void);

xcam_s32 xcam_log_set_attr(xcam_log_attr *attr);

xcam_s32 xcam_log_get_attr(xcam_log_attr *attr);

xcam_void xcam_log_output(const xcam_char *module, xcam_log_level level, const xcam_char *func, xcam_u32 line, const xcam_char *format, ...);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
