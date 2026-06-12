/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __XMEDIA_VERSION_H__
#define __XMEDIA_VERSION_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include <stdio.h>
typedef unsigned long XMEDIA_VERSION_T;

/*
    Version define
     31       24 23      16 15                   0
    -----------------------------------------------
    |    major  |  minor   |   build number      |
    -----------------------------------------------
*/

#define VERSION_CODE(major, minor, build) \
    ( ((major) << 24) | ((minor) << 16) | (build) )

#define VERSION_CODE_MAJOR(v) ((v) >> 24)
#define VERSION_CODE_MINOR(v) (((v) >> 16) & 0xFF)
#define VERSION_CODE_BUILD(v) ((v) & 0xFFFF )

#define MMPLIB_VER_MAJOR 0L
#define MMPLIB_VER_MINOR 6L
#define MMPLIB_VER_BUILD 0106L
#define MMPLIB_BUILD_DATE "20060415-V100ITr1"

#define MMPLIB_VER_EXTFLAG 0

#define GET_MMPLIB_VER VERSION_CODE(MMPLIB_VER_MAJOR, MMPLIB_VER_MINOR,MMPLIB_VER_BUILD )

#define VERSTF_SIZE  64
static inline const char* MMPLIB_VER_STR()
{
    static char verstr[VERSTF_SIZE];
    sprintf(verstr, "%02lu.%02lu.%04lu build:%s",
                MMPLIB_VER_MAJOR,
                MMPLIB_VER_MINOR,
                MMPLIB_VER_BUILD,
                MMPLIB_BUILD_DATE
                );
    return ( (char*)verstr );
}

static inline const char* bsp_version_str(XMEDIA_VERSION_T v)
{
    static char verstr[VERSTF_SIZE];
    sprintf(verstr, "%02lu.%02lu.%04lu ",
                VERSION_CODE_MAJOR(v),
                VERSION_CODE_MINOR(v),
                VERSION_CODE_BUILD(v)
                );
    return verstr;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __XMEDIA_VERSION_H__ */
