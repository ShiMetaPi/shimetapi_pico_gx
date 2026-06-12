/*
 * Copyright (c) XCAM. All rights reserved.
 */

#ifndef __XCAM_COMMON_H__
#define __XCAM_COMMON_H__

#include <stdlib.h>

#include "xcam_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

typedef enum {
    /* common */
    XCAM_MOD_COMMON     = 0x0,
    XCAM_MOD_LOG        = 0x1,

    /* middleware */
    XCAM_MOD_EVTHUB     = 0x100,
    XCAM_MOD_MBUF       = 0x101,
    XCAM_MOD_STORAGE    = 0x102,
    XCAM_MOD_TIMER      = 0x103,
    XCAM_MOD_MSGHANDLE  = 0x104,
    XCAM_MOD_BARCODE    = 0x105,
    XCAM_MOD_DCF        = 0x106,
    XCAM_MOD_DTCF       = 0x107,
    XCAM_MOD_TS         = 0x108,
    XCAM_MOD_MP4        = 0x109,
    XCAM_MOD_EXIF       = 0x10A,
    XCAM_MOD_FSTOOL     = 0x10B,
    XCAM_MOD_RECORD     = 0x10C,
    XCAM_MOD_LIVESTREAM = 0x10D,
    XCAM_MOD_STATEMACHINE = 0x10E,
    XCAM_MOD_SYSLINK    = 0x110,
    XCAM_MOD_MAPI       = 0x111,
    XCAM_MOD_RTSP       = 0x112,

    /* product wrapper layer */
    XCAM_MOD_FLASH      = 0x200,
    XCAM_MOD_CONFACCESS = 0x201,
    XCAM_MOD_DEVMNG     = 0x202,
    XCAM_MOD_FILEMNG    = 0x203,
    XCAM_MOD_FILETRANS  = 0x204,
    XCAM_MOD_DEVHAL     = 0x205,
    XCAM_MOD_LIVESERVER = 0x206,
    XCAM_MOD_MEDIA      = 0x207,
    XCAM_MOD_NETSERV    = 0x208,
    XCAM_MOD_PHOTOMNG   = 0x209,
    XCAM_MOD_PLAYBACK   = 0x20A,
    XCAM_MOD_PM         = 0x20B,
    XCAM_MOD_QUEUE      = 0x20C,
    XCAM_MOD_RAWCAP     = 0x20D,
    XCAM_MOD_RECORDMNG  = 0x20E,
    XCAM_MOD_SCENCEAUTO = 0x210,
    XCAM_MOD_STORAGEMNG = 0x211,
    XCAM_MOD_SYSCTL  = 0x212,
    XCAM_MOD_TIMEDTASK  = 0x213,
    XCAM_MOD_TIMESTAMP  = 0x214,
    XCAM_MOD_UPGRADE    = 0x215,
    XCAM_MOD_USBMNG     = 0x216,
    XCAM_MOD_VIDANALISIS = 0x217,
    XCAM_MOD_VIDPROCESS = 0x218,
    XCAM_MOD_UTILS      = 0X219,

    /* application */
    XCAM_MOD_APPLICATION = 0x300,

    XCAM_MOD_MAX

} xcam_mod_id;

#define XCAM_BUILD_ERRNO(module, errno) ((xcam_u32)((0x80000000L) | ((module) << 16) | (errno)))

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

