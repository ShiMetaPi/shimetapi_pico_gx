/*
 * Copyright (c) XCAM. All rights reserved.
 */

#ifndef __XCAM_MBUF_H__
#define __XCAM_MBUF_H__

#include "xcam_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define XCAM_MBUF_MAX_PACK                 (8)
#define XCAM_MBUF_PAYLOAD_TYPE_ALL         (0xFFFE)
#define XCAM_MBUF_PAYLOAD_TYPE_INVALID     (0xFFFF)

/* error number */
#define XCAM_ERR_MBUF_BASE                XCAM_FAILURE
#define XCAM_ERR_MBUF_MUTEX_INIT_FAILED  (XCAM_ERR_MBUF_BASE - 1)
#define XCAM_ERR_MBUF_PARAM_INVALID      (XCAM_ERR_MBUF_BASE - 2)
#define XCAM_ERR_MBUF_ALLOC_FAILED       (XCAM_ERR_MBUF_BASE - 3)
#define XCAM_ERR_MBUF_ADDR_NULL          (XCAM_ERR_MBUF_BASE - 4)
#define XCAM_ERR_MBUF_NO_MEMORY          (XCAM_ERR_MBUF_BASE - 5)
#define XCAM_ERR_MBUF_BUFFER_FULL        (XCAM_ERR_MBUF_BASE - 6)
#define XCAM_ERR_MBUF_BUFFER_EMPTY       (XCAM_ERR_MBUF_BASE - 7)
#define XCAM_ERR_MBUF_FREE_ERR           (XCAM_ERR_MBUF_BASE - 8)
#define XCAM_ERR_MBUF_GET_BUF_FAILED     (XCAM_ERR_MBUF_BASE - 9)
#define XCAM_ERR_MBUF_PUT_BUF_FAILED     (XCAM_ERR_MBUF_BASE - 10)
#define XCAM_ERR_MBUF_READ_FAILED        (XCAM_ERR_MBUF_BASE - 11)
#define XCAM_ERR_MBUF_BUSY               (XCAM_ERR_MBUF_BASE - 12)
#define XCAM_ERR_MBUF_PAYLOAD_CHECK_ERR  (XCAM_ERR_MBUF_BASE - 13)
#define XCAM_ERR_MBUF_NOT_REGISTERED     (XCAM_ERR_MBUF_BASE - 14)
#define XCAM_ERR_MBUF_RELEASE_FAILED     (XCAM_ERR_MBUF_BASE - 15)



/* mbuffer config */
typedef struct cam_mbuf_cfg_s {
    xcam_u32 buf_size;                 /* memory size of mbuffer */
    xcam_s32 payload_count;            /* max patload type num in mbuffer */
} xcam_mbuf_cfg;

/* mbuffer pack info */
typedef struct cam_mbuf_pack_info_s {
    xcam_u64  pts;                                /* timestamp if the frame */
    xcam_u32  seq;                                /* sequence num of the frame */
    xcam_u16  payload_type;                       /* mbuffer data payload type 0~0xFFF0,0xFFFE:all,0xFFFF:invalid */
    xcam_u8   key_frame;                          /* whether keyframe or not */
    xcam_u8   pack_count;                         /* pack_count < XCAM_MBUF_MAX_PACK*/
    xcam_u8  *pack_addr[XCAM_MBUF_MAX_PACK];      /* start address of the data block */
    xcam_u32  pack_size[XCAM_MBUF_MAX_PACK];      /* len of the block data agree with pAdder */
} xcam_mbuf_pack_info;

/*
 * Create the instant of mbuf, allocate buf.
 * param[in]  xcam_mbuf_cfg *mbuf_cfg: property of buffer
 * param[out] xcam_void **handle: mbuf handle
 * return  XCAM_SUCCESS success
 *         XCAM_ERR_MBUF_PARAM_INVALID  illegal params
 *         XCAM_ERR_MBUF_ALLOC_FAILED   buffer malloc failed
 *         XCAM_ERR_MBUF_MUTEX_INIT_FAILED
 */
xcam_s32 xcam_mbuf_get_buffer(xcam_handle **handle, xcam_mbuf_cfg *mbuf_cfg);

/*
 * Realse mbuffer.
 * param[in]  xcam_void *handle: mbuf handle
 * return  XCAM_SUCCESS success
 *         XCAM_ERR_MBUF_PARAM_INVALID  illegal params
 */
xcam_s32 xcam_mbuf_release_buffer(xcam_handle *handle);

/*
 * register MBUF payload type, for example 0 for video 1 for audio.
 * param[in] xcam_void * handle: mbuf handle
 * param[in]  xcam_u16   payload_type: payload type
 * return  XCAM_SUCCESS success
 *         XCAM_ERR_MBUF_PARAM_INVALID  illegal params
 *         XCAM_FAILURE   regiter space is full
 */
xcam_s32 xcam_mbuf_register_payload(xcam_handle *handle, xcam_u16 payload_type);

/*
 * unregister MBUF payload type.
 * param[in] xcam_void * handle: mbuf handle
 * param[in]  xcam_u16   payload_type: payload type
 * return  XCAM_SUCCESS success
 *         XCAM_ERR_MBUF_PARAM_INVALID  illegal params
 *         XCAM_FAILURE
 */
xcam_s32 xcam_mbuf_unregister_payload(xcam_handle *handle, xcam_u16 payload_type);

/*
 * Set read/write status of mbuf, according to the payload type.
 * param[in] xcam_void * handle: muf handle
 * param[in]  xcam_u16   payload_type: payload type
 * param[in]  xcam_bool enable_read : enable read ptr or not
 * param[in]  xcam_bool enable_write: enable write ptr or not
 * return XCAM_SUCCESS success
 *        XCAM_ERR_MBUF_PARAM_INVALID  illegal params
 *        XCAM_FAILURE  payload not found
 */
xcam_s32 xcam_mbuf_set_rw_enable(xcam_handle *handle, xcam_u16 payload_type, xcam_bool enable_read, xcam_bool enable_write);

/*
 * Get the  read/write status of mbuffer, according to the payload type.
 * param[in] xcam_void *handle: the handle of mbuf
 * param[in] xcam_u16   payload_type:  payload type
 * param[in] xcam_bool *enable_read :  enable read ptr or not
 * param[in] xcam_bool *enable_write:  enable write ptr or not
 * return XCAM_SUCCESS success
 *        XCAM_ERR_MBUF_PARAM_INVALID  illegal params
 *        XCAM_ERR_MBUF_NOT_REGISTERED
 */
xcam_s32 xcam_mbuf_get_rw_enable(xcam_handle *handle, xcam_u16 payload_type, xcam_bool *enable_read, xcam_bool *enable_write);

/*
 * Write package into mbuf
 * param[in] xcam_void * handle: mbuf handle
 * param[in]   xcam_mbuf_pack_info *pack_info     pack info data
 * return  XCAM_SUCCESS success
 *         XCAM_ERR_MBUF_PARAM_INVALID,XCAM_ERR_MBUF_GET_BUF_FAILED,XCAM_ERR_MBUF_PUT_BUF_FAILED
 *         XCAM_ERR_MBUF_FULL,XCAM_ERR_MBUF_BUSY,
 *         XCAM_ERR_MBUF_BUSY
 */
xcam_s32 xcam_mbuf_write_pack(xcam_handle *handle, const xcam_mbuf_pack_info *pack_info);

/*
 * Read data from mbuf
 * param[in]  xcam_void * handle: mbuf handle
 * param[in]  xcam_u16   payload_type    payload type
 * param[out] xcam_mbuf_pack_info *pack_info  read out pack info
 * return  XCAM_SUCCESS success
 *         XCAM_ERR_MBUF_PARAM_INVALID  illegal params
 *         XCAM_ERR_MBUF_BUFFER_EMPTY
 *         XCAM_ERR_MBUF_PAYLOAD_CHECK_ERR  no such data of the giving payload type
 */
xcam_s32 xcam_mbuf_read_pack(xcam_handle *handle, xcam_u16 payload_type, xcam_mbuf_pack_info *pack_info);

/*
 * Forward the read ptr for the payload type data
 * param[in]  xcam_void * handle: mbuf handle
 * param[in]  xcam_u16   payload_type:    payload type
 * param[out] xcam_u32   step:  moving steps for the read ptr,1 for move one package after
 * return  XCAM_SUCCESS success
 *         XCAM_ERR_MBUF_PARAM_INVALID  illegal params
 *         XCAM_ERR_MBUF_RELEASE_FAILED
 */
xcam_s32 xcam_mbuf_forward(xcam_handle *handle, xcam_u16 payload_type, xcam_u32 step);

/*
 * Print the debug info
 */
xcam_s32 xcam_mbuf_debug(xcam_handle *handle, xcam_u16 payload_type);

/*
 * Get package data offset
 */
xcam_s32 xcam_mbuf_get_package_data_offset(xcam_handle *handle, const xcam_mbuf_pack_info *pack_info, xcam_u32 *offset);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __XCAM_MBUF_H__ */
