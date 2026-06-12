/*
 * Copyright (c) XCAM. All rights reserved.
 */

#ifndef _XCAM_LIVESTREAM_RTSP_SERVER_H_
#define _XCAM_LIVESTREAM_RTSP_SERVER_H_

#include "xcam_track_source.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct server_state_listener {
    /**
     * @brief report of server internal error.
     * @param[in] listener :listener obj
     * @param[in] errid : server error id
     * @param[in] msg : string server error message info
     */
    xcam_void (*func_server_error)(struct server_state_listener* listener, xcam_s32 errid, xcam_char* msg);

    /**
     * @brief reporting client connected.
     * @param[in] listener : listener obj
     * @param[in] ipaddr : string connected client ipaddress
     */
    xcam_void (*func_client_connect)(struct server_state_listener* listener, xcam_char* ipaddr);

    /**
     * @brief reporting client disconnected.
     * @param[in] listener : listener obj
     * @param[in] ipaddr : string disconnected client ipaddress
     */
    xcam_void (*func_client_disconnect)(const struct server_state_listener* listener, const xcam_char* ipaddr);

    /** handle for statelistener, do not change, used internal*/
    xcam_void* handle;
} xcam_server_state_listener;

#define RTSPSVR_FRAME_MAX_BLOCK (8)

/** RTSP config object type*/
typedef struct {
    xcam_s32  packet_len;  /**< length of the send packet [500,5000]recommend 1500 unit:byte*/
    xcam_s32  max_conn_num; /**< max connect client num[1,32]*/
    xcam_s32  listen_port; /**< listen port of rtspserver[1,65535] normal 554*/
    xcam_s32  max_payload; /**< max payloadtype num in mbuffer[1,254] normal 2*/
    xcam_s32  timeout; /**<set the timeout of connect   <0 for not use timeout unit: s*/
} xcam_livestream_rtsp_config;

typedef struct {
    xcam_u8* data_ptr[RTSPSVR_FRAME_MAX_BLOCK];  /**< data address */
    xcam_u32 data_len[RTSPSVR_FRAME_MAX_BLOCK];  /**< length of frame data */
    xcam_u64 pts_us;            /**< timestamp of frame unit:us*/
    xcam_u32 seq_num;   /**< frame sequence number */
    xcam_u32 block_cnt;/**< block count for one frame*/
    xcam_bool is_key_frame;   /**< key frame flag */
} xcam_livestream_rtsp_data;

typedef struct {
    xcam_track_source_handle video_src_handle;
    xcam_track_source_handle audio_src_handle;
} xcam_livestream_rtsp_source;

/**
 * @brief create server instance.
 * @param[in,out] handle xcam_void* : return rtspserver handle
 * @param[in] ptr_rtsp_config xcam_livestream_rtsp_config : rtsp config info
 * @return   0 success
 * @return  err num  failure
 */
xcam_s32 xcam_livestream_rtsp_server_create(xcam_void** handle, xcam_livestream_rtsp_config* ptr_rtsp_config);

/**
 * @brief destroy server instance.
 * @param[in] handle xcam_void* : rtspserver handle
 * @return   0 success
 * @return err num failure
 */
xcam_s32 xcam_livestream_rtsp_server_destroy(xcam_void* handle);

/**
 * @brief make server running, could handle client connecting and request.
 * @param[in] handle xcam_void* : rtspserver handle
 * @return   0 success
 * @return  err num failure
 */
xcam_s32 xcam_livestream_rtsp_server_start(xcam_void* handle);

/**
 * @brief make server stoped, deny client connecting and request.
 * @param[in] handle xcam_void* : rtspserver handle
 * @return   0 success
 * @return  err num failure
 */
xcam_s32 xcam_livestream_rtsp_server_stop(xcam_void* handle);

/**
 * @brief add media source to server, client could use specified URL \n
 * related with stream connect to this source.
 * @param[in] handle xcam_void* : rtspserver handle
 * @param[in] stream_name string : stream Name related with this mediaStream,max length 128
 * @param[in] buf_size XM_U32: size of allocate mbuffer[216K,50M] suggest frame size*5 unit:byte
 * @return   0 success
 * @return  err num failure
 */
xcam_s32 xcam_livestream_rtsp_server_add_media_stream(xcam_void* handle, xcam_livestream_rtsp_source* stream_src,
                xcam_char* stream_name, xcam_u32 buf_size);

/**
 * @brief remove media source from server, client connecting with \n
 * specified URL related with this stream will receive failing
 * @param[in] handle xcam_void* : rtspserver handle
 * @param[in] track_src xcam_track_source_handle :  Source of outputing MediaInfo and \n
  * @param[in] ptr_data xcam_livestream_rtsp_data :  Media data \n
 * VideoStream&AudioStream
 * @return 0 success
 * @return err num failure
 */
xcam_s32 xcam_livestream_rtsp_server_write_frame(xcam_void* handle,
                xcam_track_source_handle track_src, const xcam_livestream_rtsp_data* ptr_data);

/**
 * @brief remove media source from server, client connecting with \n
 * specified URL related with this stream will receive failing
 * @param[in] handle xcam_void* : rtspserver handle
 * @return 0 success
 * @return err num failure
 */
xcam_s32 xcam_livestream_rtsp_server_remove_media_stream(xcam_void* handle, const xcam_char* stream_name);

/**
 * @brief setting listener for listenning client connect&disconnect, server internal error.
 * @param[in] handle xcam_void* : rtspserver handle
 * @param[in] ptr_listener xcam_server_state_listener : input listener object
 * @return    0 success
 * @return    err num failure
 */
xcam_s32 xcam_livestream_rtsp_server_set_listener(xcam_void* handle, xcam_server_state_listener* ptr_listener);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /*_XCAM_LIVESTREAM_RTSP_SERVER_H_*/
