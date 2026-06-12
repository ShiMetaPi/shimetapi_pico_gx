/*
 * Copyright (c) XCAM. All rights reserved.
 */

#ifndef __XCAM_TYPE_H__
#define __XCAM_TYPE_H__

#ifdef __KERNEL__

#include <linux/types.h>
#else

#include <stdint.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

typedef enum {
    XCAM_FALSE = 0,
    XCAM_TRUE  = 1,
} xcam_bool;

#ifndef NULL
    #define NULL    0L
#endif

#ifndef FALSE
    #define FALSE   0
#endif

#ifndef TRUE
    #define TRUE    1
#endif

#define XCAM_NULL                 0L
#define XCAM_SUCCESS              0
#define XCAM_FAILURE              (-1)

typedef unsigned char           xcam_uchar;
typedef unsigned char           xcam_u8;
typedef unsigned short          xcam_u16;
typedef unsigned int            xcam_u32;
typedef unsigned long long      xcam_u64;
typedef unsigned long           xcam_ulong;

typedef char                    xcam_char;
typedef signed char             xcam_s8;
typedef short                   xcam_s16;
typedef int                     xcam_s32;
typedef long long               xcam_s64;
typedef long                    xcam_slong;

typedef float                   xcam_float;
typedef double                  xcam_double;

typedef void                    xcam_void;

typedef unsigned long           xcam_size_t;
typedef unsigned long           xcam_length_t;

typedef xcam_u32              xcam_handle;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif