/*
 * Copyright (c) XCAM. All rights reserved.
 */

#ifndef _LIVESTREAM_MBUFFER_H_
#define _LIVESTREAM_MBUFFER_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include "xcam_mbuf.h"
#include "pthread.h"


typedef enum {
    MBUFFER_DATA_VIDEO = 1,
    MBUFFER_DATA_AUDIO,
    MBUFFER_DATA_DATA,
    MBUFFER_DATA_BUTT
} livestream_mbuffer_data_type;

typedef struct {
    pthread_mutex_t mutex; /* lock for mbuffer write pos */
} livestream_mbuffer_info;

enum {
    BUF_PAYLOAD_G711U  = 0,   /**< G.711 Mu           */
    BUF_PAYLOAD_G711A  = 8,   /**< G.711 A            */
    BUF_PAYLOAD_G726   = 97,  /**< G.726              */
    BUF_PAYLOAD_H264   = 96,  /**< H264               */
    BUF_PAYLOAD_H265   = 98,  /**< H265               */
    BUF_PAYLOAD_ADPCM  = 104, /**< ADPCM              */
    BUF_PAYLOAD_AAC    = 105, /**< AAC encoder        */
    BUF_PAYLOAD_MP3    = 109, /**< MP3 encoder        */
    BUF_PAYLOAD_BUTT          /**< invalid            */
};

xcam_s32 livestream_mbuffer_write_frame(xcam_void* handle, const xcam_mbuf_pack_info* pack_info);

xcam_s32 livestream_mbuffer_create(const xcam_void* argv, xcam_void** buf_handle, 
                xcam_u32 buf_size, xcam_s32 max_payload);

xcam_s32 livestream_mbuffer_destroy(xcam_void* argv, xcam_void* handle);

xcam_s32 livestream_mbuffer_register(xcam_void* handle, xcam_u8 media_type);

xcam_s32 livestream_mbuffer_unregister(xcam_void* handle, xcam_u8 media_type);

xcam_s32 livestream_mbuffer_read(xcam_void* handle, xcam_void** paddr, xcam_u32* plen,
                xcam_u64* ppts, livestream_mbuffer_data_type* type, xcam_bool* ptr_bkey_flag);

xcam_s32 livestream_mbuffer_set(xcam_void* handle, xcam_u32 step);

xcam_s32 livestream_mbuffer_pts(xcam_void* handle, xcam_u8 payload_type, xcam_u64* ppts);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* _LIVESTREAM_MBUFFER_H_ */
