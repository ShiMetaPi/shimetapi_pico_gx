/*
 * Copyright (c) XCAM. All rights reserved.
 */

#ifndef _LIVESTREAM_RTSP_SESSION_H_
#define _LIVESTREAM_RTSP_SESSION_H_

#include <pthread.h>
#include "list.h"
#include "xcam_livestream_rtsp_server.h"
#include "xcam_track_source.h"
#include "livestream_rtsp_message.h"
#include "livestream_rtp_session.h"
#include "livestream_rtcp_session.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define RTSP_TRACKID_VIDEO       (0)
#define RTSP_TRACKID_AUDIO       (1)
#define RTSP_INVALID_TRACK_ID    (-1)
#define RTSP_INVALID_THREAD_ID   (pthread_t)(-1)
#define RTSP_MAX_NALBASE_LEN     (256)
#define CHECK_WAIT_MAX_NUM       (600)
#define MAX_NAL_PARAM_LEN        (128)    /** max len for nal param*/
#define CHECK_WAIT_TIMEOUT       (10 * 1000)
#define CHECK_WAIT_I_FRAME_COUNT (100)

#define RTSPSVR_CHECK_NULL_ERROR(condition) \
    do { \
        if(condition == XCAM_NULL) { \
            LOG_LIVESTREAM_ERROR("pointer is null\n"); \
            return LIVESTREAM_ERRNO_NULL_PTR; \
        }\
    } while(0)

typedef enum {
    LIVESTREAM_RTSP_SESSION_STATE_INIT = 0,
    LIVESTREAM_RTSP_SESSION_STATE_READY,
    LIVESTREAM_RTSP_SESSION_STATE_PLAY,
    LIVESTREAM_RTSP_SESSION_STATE_STOP,
    LIVESTREAM_RTSP_SESSION_STATE_BUTT
} livestream_rtsp_session_state;

typedef enum {
    AVC_SEI = 0x06,
    AVC_IDR = 0x05,
    AVC_SPS = 0x07,
    AVC_PPS = 0x08,
} livestream_rtsp_avc_nalu;

typedef enum {
    HEVC_PSLICE = 0x01,
    HEVC_IDR = 0x13,
    HEVC_SPS = 0x21,
    HEVC_PPS = 0x22,
    HEVC_VPS = 0x20,
    HEVC_SEI = 0x27
} livestream_rtsp_hevc_nalu;

typedef struct {
    livestream_rtp_session* rtp_session;
    livestream_rtcp_session* rtcp_session;
    pthread_mutex_t  mutex_get_port;
    xcam_s32 track_id;
    xcam_char rtsp_url[RTSP_URL_MAX_LEN];
} livestream_rtsp_media_session;

typedef struct {
    /*used to maintain link list*/
    struct list_head list_ptr;
    xcam_char name[RTSP_MAX_STREAMNAME_LEN];
    xcam_track_source_handle video_stream;
    xcam_track_source_handle audio_stream;
    xcam_u32 buf_size;
    xcam_u32 video_start_cnt;
    xcam_u32 audio_start_cnt;
} livestream_media_stream_node;

typedef struct {
    struct list_head stream_list;
    pthread_mutex_t stream_list_lock;
    struct list_head session_list;
    pthread_mutex_t session_list_lock;
    xcam_server_state_listener state_listener;
    xcam_s32  max_conn_num;
    xcam_s32  user_num;
    xcam_s32  stream_num;
    xcam_s32  listen_port;
    xcam_s32  packet_len;
    xcam_u32  buf_size;
    xcam_s32  max_payload;
    xcam_s32  timeout;
    xcam_void* net_listener;
    xcam_bool is_started;
} livestream_rtsp_server_ctx;

/** nal param buffer struct*/
typedef struct {
    xcam_u8  data_buf[MAX_NAL_PARAM_LEN];/**< nal data buf  */
    xcam_u32 data_len;/**< nal data len  */
} nal_param_buffer;

typedef struct {
    xcam_bool  video_enable;
    xcam_bool  audio_enable;
    xcam_void* mbuf_handle;
    livestream_mbuffer_info mbuf_info;
    xcam_track_video_source_info video_source_info;
    xcam_track_audio_source_info audio_source_info;
    nal_param_buffer sps_buf;
    nal_param_buffer pps_buf;
    nal_param_buffer vps_buf;
    xcam_u64 video_start_pts; /**< start timestamp of frame */
    xcam_u64 audio_start_pts; /**< start timestamp of frame */
} livestream_rtsp_media_info;

struct rtsp_stream_session;

typedef struct {
    xcam_s32 (*session_destroy_func)(xcam_void* object, struct rtsp_stream_session* rtsp_session);
} livestream_rtsp_session_listener;

typedef struct rtsp_stream_session {
    /*for the management of the list */
    struct list_head list_ptr;
    xcam_track_source_handle video_stream;
    xcam_track_source_handle audio_stream;
    livestream_rtsp_session_listener session_listener;
    xcam_void* listener_object;
    xcam_void* rtspsvr_handle;  // rtspserver handle
    pthread_t  session_thdid;
    xcam_s32   session_sockfd;
    livestream_rtsp_session_state session_state;
    pthread_mutex_t state_lock;
    xcam_s32  cur_request_seq;
    xcam_char recv_buf[RTSP_MAX_PROTOCOL_BUFFER];
    xcam_char response_buf[RTSP_MAX_PROTOCOL_BUFFER];
    xcam_char stream_name[RTSP_MAX_STREAMNAME_LEN];
    xcam_char url[RTSP_URL_MAX_LEN];
    xcam_char client_ip[RTSP_IP_MAX_LEN];
    xcam_char host_ip[RTSP_IP_MAX_LEN];
    xcam_u32  recv_len;
    xcam_char session_id[RTSP_SESSID_MAX_LEN];
    xcam_s32  packet_len;
    xcam_u32  buf_size;
    xcam_s32  max_payload;
    xcam_s32  timeout;
    xcam_u64  start_pts;
    xcam_u64  last_pts;
    livestream_rtsp_media_info media_info;
    livestream_rtsp_media_session video_session;
    livestream_rtsp_media_session audio_session;
    xcam_bool is_get_key_frame;
    xcam_u16  client_rtsp_port;
    xcam_u32  channel_id;
} livestream_rtsp_stream_session;

/*newly add for supporting teardown one track and multi packets*/
#define RTSP_RTCP_HEADER_LENGTH            (4)
#define RTSP_RTCP_BYTES_LENGTH             (4)

#define RTSP_RTCP_PACKRT_TYPE_SR           (200)
#define RTSP_RTCP_PACKRT_TYPE_RR           (201)
#define RTSP_RTCP_PACKRT_TYPE_SDES         (202)
#define RTSP_RTCP_PACKRT_TYPE_BYE          (203)
#define RTSP_RTCP_PACKRT_TYPE_APP          (204)

#define RTSP_RESPONSE_PACKET_LENGTH        (4)
#define RTSP_RESPONSE_PACKRT_FIRST_BYTE    (36)
#define RTSP_RESPONSE_PACKRT_THIRD_BYTE    (0)

xcam_s32  livestream_rtsp_session_create(livestream_rtsp_stream_session** pptr_session, xcam_s32 sockfd);

xcam_s32  livestream_rtsp_session_destroy(livestream_rtsp_stream_session* rtsp_session);

xcam_void livestream_rtsp_session_set_media_source(livestream_rtsp_stream_session* rtsp_session,
        xcam_track_source_handle vid_stream, xcam_track_source_handle aud_stream,
        const xcam_char* stream_name);

xcam_s32  livestream_rtsp_session_set_listener(livestream_rtsp_stream_session* rtsp_session,
        const livestream_rtsp_session_listener* listener, xcam_void* object);

xcam_void livestream_rtsp_session_get_client_ipaddr(livestream_rtsp_stream_session* rtsp_session,
        xcam_char* buffer, xcam_s32 buf_len);

xcam_s32  livestream_rtsp_session_client_connect(livestream_rtsp_stream_session* rtsp_session,
        xcam_char* client_request, xcam_u32 request_len);

xcam_s32  livestream_rtsp_session_send(xcam_s32 writ_sock, xcam_char* ptr_buff, xcam_u32 data_len);

xcam_s32  livestream_rtsp_session_stop(livestream_rtsp_stream_session* rtsp_session);

xcam_s32  livestream_rtsp_session_write_frame(livestream_rtsp_stream_session* rtsp_session, 
            xcam_track_source_handle track_src, const xcam_livestream_rtsp_data* stream_data);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*_LIVESTREAM_RTSP_SESSION_H_*/
