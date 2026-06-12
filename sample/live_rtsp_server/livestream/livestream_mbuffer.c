/*
 * Copyright (c) XCAM. All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "livestream_comm.h"
#include "livestream_mbuffer.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define UNUSED(x)         (void)x
#define MAX_WAIT_COUNT    (100)
#define DEFAULT_MBUFFER_SIZE (1280 * 720)

static xcam_s32 mbuffer_enable(xcam_void* handle, xcam_u8 payload_type)
{
    xcam_s32 ret;

    ret = xcam_mbuf_register_payload(handle, payload_type);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("mbuf register payload fail! handle:%p payload_type:%d\n",
                handle, payload_type);
        return ret;
    }

    ret = xcam_mbuf_set_rw_enable(handle, payload_type, XCAM_TRUE, XCAM_TRUE);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("mbuf set r/w enable fail, ret=%d!\n", ret);
        if (XCAM_SUCCESS != xcam_mbuf_unregister_payload(handle, payload_type)) {
            LOG_LIVESTREAM_ERROR("mbuf unregister payload_type:%d fail!\n", payload_type);
        }
        return ret;
    }

    return XCAM_SUCCESS;
}

xcam_s32 livestream_mbuffer_write_frame(xcam_void* handle, const xcam_mbuf_pack_info* pack_info)
{
    if (pack_info != XCAM_NULL) {
        return xcam_mbuf_write_pack(handle, pack_info);
    }

    return XCAM_FAILURE;
}

xcam_s32 livestream_mbuffer_create(const xcam_void* argv, xcam_void** buf_handle,
                    xcam_u32 buf_size, xcam_s32 max_payload)
{
    xcam_s32 ret = XCAM_SUCCESS;
    xcam_mbuf_cfg mbuf_cfg;
    xcam_handle *handle;

    mbuf_cfg.payload_count = max_payload;
    mbuf_cfg.buf_size = buf_size;
    if (mbuf_cfg.buf_size == 0) {
        mbuf_cfg.buf_size = DEFAULT_MBUFFER_SIZE;
    }

    ret = xcam_mbuf_get_buffer(&handle, &mbuf_cfg);
    if (ret != XCAM_SUCCESS) {
        LOG_LIVESTREAM_ERROR("mbuf get buffer failed ret: %d\n", ret);
        return XCAM_FAILURE;
    }

    *buf_handle = handle;
    LOG_LIVESTREAM_INFO("mbuffer create success, size:%d payload_count:%d.\n",
        mbuf_cfg.buf_size, mbuf_cfg.payload_count);
    return XCAM_SUCCESS;
}

xcam_s32 livestream_mbuffer_destroy(xcam_void* argv, xcam_void* handle)
{
    xcam_s32 ret;

    ret = xcam_mbuf_release_buffer(handle);
    if (ret != XCAM_SUCCESS) {
        LOG_LIVESTREAM_ERROR("Release mbuf failed ret:0x%x!\n", ret);
        return XCAM_FAILURE;
    }

    return XCAM_SUCCESS;
}

xcam_s32 livestream_mbuffer_register(xcam_void* handle, xcam_u8 media_type)
{
    xcam_s32 ret;

    ret = mbuffer_enable(handle, media_type);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("mbuf enable failed ret:0x%x!\n", ret);
        return ret;
    }

    return XCAM_SUCCESS;
}

xcam_s32 livestream_mbuffer_unregister(xcam_void* handle, xcam_u8 media_type)
{
    xcam_s32 ret = XCAM_SUCCESS;

    ret = xcam_mbuf_unregister_payload(handle, media_type);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("mbuf unregister fail, media type:%d ret:%d!\n", media_type, ret);
        return ret;
    }

    return XCAM_SUCCESS;
}

xcam_s32 livestream_mbuffer_read(xcam_void* handle, xcam_void** paddr, xcam_u32* plen,
                xcam_u64* ppts, livestream_mbuffer_data_type* out_type, xcam_bool* bkey_flag)
{
    xcam_mbuf_pack_info pack_info;
    xcam_s32 ret = XCAM_FAILURE;
    xcam_u32 i = 0;

    pack_info.key_frame = XCAM_FALSE;
    pack_info.seq = 0;
    pack_info.pts = 0;
    pack_info.payload_type = 0;
    pack_info.pack_count = 0;
    if (paddr == XCAM_NULL || plen == XCAM_NULL || ppts == XCAM_NULL
        || out_type == XCAM_NULL || bkey_flag == XCAM_NULL) {
        return ret;
    }
    for (i = 0; i < XCAM_MBUF_MAX_PACK; i++) {
        pack_info.pack_addr[i] = XCAM_NULL;
        pack_info.pack_size[i] = 0;
    }

    *plen = 0;
    ret = xcam_mbuf_read_pack(handle, XCAM_MBUF_PAYLOAD_TYPE_ALL, &pack_info);
    if (XCAM_SUCCESS == ret) {
        *paddr = pack_info.pack_addr[0];
        for (i = 0; i < pack_info.pack_count; i++) {
            *plen += pack_info.pack_size[i];
        }
        *ppts = pack_info.pts;
        *bkey_flag = pack_info.key_frame;

        switch (pack_info.payload_type) {
            case BUF_PAYLOAD_H264:
            case BUF_PAYLOAD_H265:
                *out_type = MBUFFER_DATA_VIDEO;
                break;
            case BUF_PAYLOAD_G711U:
            case BUF_PAYLOAD_G711A:
            case BUF_PAYLOAD_G726:
            case BUF_PAYLOAD_AAC:
            case BUF_PAYLOAD_MP3:
                *out_type = MBUFFER_DATA_AUDIO;
                break;
            default:
                *out_type = MBUFFER_DATA_DATA;
                break;
        }
    }

    return ret;
}

xcam_s32 livestream_mbuffer_set(xcam_void* handle, xcam_u32 step)
{
    xcam_s32 ret;

    ret = xcam_mbuf_forward(handle, XCAM_MBUF_PAYLOAD_TYPE_ALL, step);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("mbuf forward fail, ret=%d!\n", ret);
        return ret;
    }

    return XCAM_SUCCESS;
}

xcam_s32 livestream_mbuffer_pts(xcam_void* handle, xcam_u8 payload_type, xcam_u64* ppts)
{
    xcam_s32 ret = XCAM_FAILURE;
    xcam_s32 count = 0;
    xcam_u32 i = 0;
    xcam_mbuf_pack_info pack_info;

    pack_info.key_frame = XCAM_FALSE;
    pack_info.seq = 0;
    pack_info.pts = 0;
    pack_info.payload_type = 0;
    pack_info.pack_count = 0;
    for (i = 0; i < XCAM_MBUF_MAX_PACK; i++) {
        pack_info.pack_addr[i] = XCAM_NULL;
        pack_info.pack_size[i] = 0;
    }

    while (XCAM_SUCCESS != ret && count < MAX_WAIT_COUNT) {
        ret = xcam_mbuf_read_pack(handle, payload_type, &pack_info);
        if (XCAM_SUCCESS == ret) {
            *ppts = pack_info.pts;
             xcam_mbuf_forward(handle, XCAM_MBUF_PAYLOAD_TYPE_ALL, 0); 
            break;
        }
        usleep(10 * 1000);
        count++;
    }

    return ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
