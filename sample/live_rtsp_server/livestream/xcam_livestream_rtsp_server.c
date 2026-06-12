/*
 * Copyright (c) XCAM. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "livestream_errno.h"
#include "livestream_comm.h"
#include "livestream_listener.h"
#include "livestream_mbuffer.h"
#include "livestream_rtsp_session.h"
#include "livestream_rtsp_message.h"
#include "xcam_livestream_rtsp_server.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define RTSP_MAX_CONN_NUM       (32)
#define RTSP_MAX_LISTEN_PORT    (65535)
#define RTSP_MIN_BUFFER_SIZE    (216*1024)
#define RTSP_MAX_BUFFER_SIZE    (50*1024*1024)
#define RTSP_MAX_PAYLOAD_TYPE   (254)
#define RTSP_MAX_PACKET_LEN     (5000)
#define RTSP_MIN_PACKET_LEN     (500)
#define RTSP_MAX_STREAM_NUM     (8)
#define RTSP_KEEPALIVE_SECONDS_MIN (6)

static livestream_rtsp_server_ctx* g_server_ctx = XCAM_NULL;

static xcam_bool is_audio_codec_support(xcam_track_audio_codec codec)
{
    if (codec == XCAM_TRACK_AUDIO_CODEC_AAC || codec == XCAM_TRACK_AUDIO_CODEC_G711A ||
        codec == XCAM_TRACK_AUDIO_CODEC_G711U || codec == XCAM_TRACK_AUDIO_CODEC_G726 ||
        codec == XCAM_TRACK_AUDIO_CODEC_ADPCM || codec == XCAM_TRACK_AUDIO_CODEC_MP3) {
        return XCAM_TRUE;
    }

    LOG_LIVESTREAM_ERROR("Unsupport audio type: %d\n", codec);
    return XCAM_FALSE;
}

static xcam_s32 check_frame_data(const xcam_livestream_rtsp_data* ptr_frame_data)
{
    xcam_s32 ret = XCAM_SUCCESS;
    xcam_u32 blk_idx = 0;

    RTSPSVR_CHECK_NULL_ERROR(ptr_frame_data);
    if (0 == ptr_frame_data->block_cnt 
        || ptr_frame_data->block_cnt > RTSPSVR_FRAME_MAX_BLOCK) {
        LOG_LIVESTREAM_ERROR("Illegal block count: %u!\n", ptr_frame_data->block_cnt);
        ret = LIVESTREAM_ERRNO_ILLEGAL_PARAM;
    }

    for (blk_idx = 0; blk_idx < ptr_frame_data->block_cnt; blk_idx++) {
        RTSPSVR_CHECK_NULL_ERROR(ptr_frame_data->data_ptr[blk_idx]);
        if (0 == ptr_frame_data->data_len[blk_idx]) {
            LOG_LIVESTREAM_ERROR("Illegal frame data len: 0!\n");
            ret = LIVESTREAM_ERRNO_ILLEGAL_PARAM;
        }
    }

    return ret;
}

static xcam_s32 check_stream_data(xcam_track_source_handle ptr_track_src, const xcam_livestream_rtsp_data* ptr_data)
{
    xcam_s32 ret = XCAM_SUCCESS;

    if (XCAM_TRACK_SOURCE_TYPE_VIDEO == ptr_track_src->track_type) {
        if (XCAM_TRACK_VIDEO_CODEC_H264 != ptr_track_src->track_source_attr.video_info.codec_type 
          && XCAM_TRACK_VIDEO_CODEC_H265 != ptr_track_src->track_source_attr.video_info.codec_type) {
            LOG_LIVESTREAM_ERROR("not support video type :%d \n", ptr_track_src->track_source_attr.video_info.codec_type);
            return LIVESTREAM_ERRNO_ILLEGAL_PARAM;
        }
    } else if (XCAM_TRACK_SOURCE_TYPE_AUDIO == ptr_track_src->track_type) {
        if (is_audio_codec_support(ptr_track_src->track_source_attr.audio_info.codec_type) == XCAM_FALSE) {
            LOG_LIVESTREAM_ERROR("stream data illegal\n");
            return LIVESTREAM_ERRNO_ILLEGAL_PARAM;
        }
    } else {
        LOG_LIVESTREAM_ERROR("source track type illegal \n");
        return LIVESTREAM_ERRNO_ILLEGAL_PARAM;
    }

    ret = check_frame_data(ptr_data);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("Check frame data fail!\n");
        return ret;
    }

    return XCAM_SUCCESS;
}

static xcam_s32 set_rtsp_config(livestream_rtsp_stream_session* stream_session,
                    livestream_rtsp_server_ctx* server_ctx)
{
    RTSPSVR_CHECK_NULL_ERROR(stream_session);
    RTSPSVR_CHECK_NULL_ERROR(server_ctx);
    stream_session->max_payload = server_ctx->max_payload;
    stream_session->timeout = server_ctx->timeout;
    stream_session->packet_len = server_ctx->packet_len;
    stream_session->rtspsvr_handle = (xcam_void*)server_ctx;
    return XCAM_SUCCESS;
}

static xcam_s32 check_rtsp_config(xcam_livestream_rtsp_config* rtsp_config)
{
    if (rtsp_config->packet_len < RTSP_MIN_PACKET_LEN 
       || rtsp_config->packet_len > RTSP_MAX_PACKET_LEN) {
        LOG_LIVESTREAM_ERROR("param packet_len not in the range  min:%d- max:%d!\n", 
                RTSP_MIN_PACKET_LEN, RTSP_MAX_PACKET_LEN);
        return LIVESTREAM_ERRNO_ILLEGAL_PARAM;
    }

    if (rtsp_config->max_conn_num <= 0 
        || rtsp_config->max_conn_num > RTSP_MAX_CONN_NUM) {
        LOG_LIVESTREAM_ERROR("param max_conn_num not in the range  max:%d!\n", RTSP_MAX_CONN_NUM);
        return LIVESTREAM_ERRNO_ILLEGAL_PARAM;
    }

    if (rtsp_config->listen_port <= 0
      || rtsp_config->listen_port > RTSP_MAX_LISTEN_PORT) {
        LOG_LIVESTREAM_ERROR("param listen port not in the range  max:%d!\n", RTSP_MAX_LISTEN_PORT);
        return LIVESTREAM_ERRNO_ILLEGAL_PARAM;
    }

    if (rtsp_config->max_payload <= 0
        || rtsp_config->max_payload > RTSP_MAX_PAYLOAD_TYPE) {
        LOG_LIVESTREAM_ERROR("param max_payload not in the range  max:%d!\n", RTSP_MAX_PAYLOAD_TYPE);
        return LIVESTREAM_ERRNO_ILLEGAL_PARAM;
    }

    if (rtsp_config->timeout >= 0
        && rtsp_config->timeout < RTSP_KEEPALIVE_SECONDS_MIN) {
            LOG_LIVESTREAM_ERROR("timeout out of range, min:%d!\n", RTSP_KEEPALIVE_SECONDS_MIN);
        return LIVESTREAM_ERRNO_ILLEGAL_PARAM;
    }

    return XCAM_SUCCESS;
}

/*destroy a session when  connect  teardown */
static xcam_s32 on_session_destroy(xcam_void* object, livestream_rtsp_stream_session* rtsp_session)
{
    livestream_rtsp_server_ctx* ptr_server_ctx = (livestream_rtsp_server_ctx*)object;
    livestream_rtsp_stream_session* session_in_list = XCAM_NULL;
    xcam_char client_ip[RTSP_IP_MAX_LEN] = {0};
    struct list_head* ptr_pos_node = XCAM_NULL;
    struct list_head* ptr_tmp_node = XCAM_NULL;
    xcam_bool is_found = XCAM_FALSE;
    xcam_s32 ret = XCAM_SUCCESS;

    RTSPSVR_CHECK_NULL_ERROR(ptr_server_ctx);
    RTSPSVR_CHECK_NULL_ERROR(rtsp_session);
    pthread_mutex_lock(&ptr_server_ctx->session_list_lock);
    list_for_each_safe(ptr_pos_node, ptr_tmp_node, &ptr_server_ctx->session_list) {
        session_in_list = list_entry(ptr_pos_node, livestream_rtsp_stream_session, list_ptr);
        if (0 == strncmp(rtsp_session->session_id, session_in_list->session_id, RTSP_SESSID_MAX_LEN)) {
            is_found = XCAM_TRUE;
            break;
        }
    }

    if (!is_found || XCAM_NULL == session_in_list) {
        LOG_LIVESTREAM_ERROR("could not find correspond media session\n");
        pthread_mutex_unlock(&ptr_server_ctx->session_list_lock);
        return LIVESTREAM_ERRNO_SESS_NOT_EXISTED;
    }

    /* when teardown destroy the session and remove it from the sessionlist*/
    list_del(&(session_in_list->list_ptr));
    ptr_server_ctx->user_num -= 1;

    livestream_rtsp_session_get_client_ipaddr(rtsp_session, client_ip, RTSP_IP_MAX_LEN);
    ret = livestream_rtsp_session_destroy(rtsp_session);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("stream session destroy failed:0x%x\n", ret);
        pthread_mutex_unlock(&ptr_server_ctx->session_list_lock);
        return LIVESTREAM_ERRNO_SESS_DESTROY_FAIL;
    }

    if (ptr_server_ctx->state_listener.func_client_disconnect && strlen(client_ip) > 0) {
        ptr_server_ctx->state_listener.func_client_disconnect(&ptr_server_ctx->state_listener, client_ip);
    }

    pthread_mutex_unlock(&ptr_server_ctx->session_list_lock);
    return XCAM_SUCCESS;
}

/*add a connect session for rtspserver related to a stream*/
static xcam_s32 add_stream_session(livestream_rtsp_server_ctx* ptr_server_ctx, 
            xcam_char* client_req, xcam_u32 req_len, xcam_s32 socket_fd, 
            livestream_media_stream_node* ptr_stream_node)
{
    livestream_rtsp_session_listener listener;
    livestream_rtsp_stream_session* stream_session = XCAM_NULL;
    xcam_s32 ret = XCAM_SUCCESS;
    struct list_head* pos_node = XCAM_NULL;
    livestream_rtsp_stream_session* session_node = XCAM_NULL;
    xcam_char client_ip[RTSP_IP_MAX_LEN] = {0};

    if (XCAM_NULL == ptr_server_ctx || XCAM_NULL == client_req) {
        LOG_LIVESTREAM_ERROR("Invalid param!\n");
        return LIVESTREAM_ERRNO_ILLEGAL_PARAM;
    }

    if (ptr_server_ctx->user_num >= ptr_server_ctx->max_conn_num) {
        LOG_LIVESTREAM_ERROR("reach the max connect num:%d \n", 
            ptr_server_ctx->max_conn_num);
        return LIVESTREAM_ERRNO_REACH_MAX_CONNECT;
    }

    ret = livestream_rtsp_session_create(&stream_session, socket_fd);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("livestream rtsp session create failed ret %d \n", ret);
        return LIVESTREAM_ERRNO_SESS_CREATE_FAIL;
    }

    ret = set_rtsp_config(stream_session, ptr_server_ctx);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("set session rtsp config failed \n");
        livestream_rtsp_session_destroy(stream_session);
        return LIVESTREAM_ERRNO_SESS_SET_FAIL;
    }

    if (XCAM_NULL != ptr_stream_node) {
        stream_session->buf_size = ptr_stream_node->buf_size;
        livestream_rtsp_session_set_media_source(stream_session, 
            ptr_stream_node->video_stream, ptr_stream_node->audio_stream, ptr_stream_node->name);
    }

    listener.session_destroy_func = on_session_destroy;
    ret = livestream_rtsp_session_set_listener(stream_session, &listener, (xcam_void*)ptr_server_ctx);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("set session listerner  failed  \n");
        livestream_rtsp_session_destroy(stream_session);
        return LIVESTREAM_ERRNO_SESS_SET_FAIL;
    }

    ret = livestream_rtsp_session_client_connect(stream_session, client_req, req_len);
    if (ret != XCAM_SUCCESS) {
        LOG_LIVESTREAM_ERROR("on client connect failed  ret :%d  \n", ret);
        livestream_rtsp_session_destroy(stream_session);
        return LIVESTREAM_ERRNO_SESS_CONNECT_FAIL;
    }

    pthread_mutex_lock(&ptr_server_ctx->session_list_lock);

    list_for_each(pos_node, &ptr_server_ctx->session_list) {
        session_node = list_entry(pos_node, livestream_rtsp_stream_session, list_ptr);
        if (session_node == stream_session) {
            LOG_LIVESTREAM_ERROR("rtsp stream session already exist\n");
            pthread_mutex_unlock(&ptr_server_ctx->session_list_lock);
            ret = LIVESTREAM_ERRNO_SESS_EXISTED;
            goto DES_SESSION;
        }
    }
    list_add(&(stream_session->list_ptr), &(ptr_server_ctx->session_list));
    ptr_server_ctx->user_num += 1;

    pthread_mutex_unlock(&ptr_server_ctx->session_list_lock);

    livestream_rtsp_session_get_client_ipaddr(stream_session, client_ip, RTSP_IP_MAX_LEN);
    if (ptr_server_ctx->state_listener.func_client_connect && strlen(client_ip) > 0) {
        ptr_server_ctx->state_listener.func_client_connect(&ptr_server_ctx->state_listener, client_ip);
    }

    return XCAM_SUCCESS;

DES_SESSION:

    livestream_rtsp_session_destroy(stream_session);
    return ret;
}

static xcam_s32 check_list_null(livestream_rtsp_server_ctx* ptr_server_ctx, const xcam_char* stream_name)
{
    livestream_rtsp_stream_session* session_in_list = XCAM_NULL;
    struct list_head* pos_node = XCAM_NULL;
    struct list_head* tmp_node = XCAM_NULL;
    xcam_bool need_wait = XCAM_FALSE;
    xcam_u32  wait_cnt = 0;

    while (wait_cnt <= CHECK_WAIT_MAX_NUM) {
        pthread_mutex_lock(&ptr_server_ctx->session_list_lock);
        list_for_each_safe(pos_node, tmp_node, &ptr_server_ctx->session_list) {
            session_in_list = list_entry(pos_node, livestream_rtsp_stream_session, list_ptr);
            if (0 == strncmp(stream_name, session_in_list->stream_name, RTSP_MAX_STREAMNAME_LEN)) {
                need_wait = XCAM_TRUE;
                break;
            }
        }
        pthread_mutex_unlock(&ptr_server_ctx->session_list_lock);

        if (need_wait) {
            usleep(CHECK_WAIT_TIMEOUT);
            wait_cnt++;
            need_wait = XCAM_FALSE;
            continue;
        } else {
            return XCAM_SUCCESS;
        }
    }

    return XCAM_FAILURE;
}

static xcam_bool find_session_valid(livestream_rtsp_server_ctx* ptr_server_ctx,
    const xcam_char* stream_name, livestream_rtsp_stream_session** pptr_session)
{
    livestream_rtsp_stream_session* session_in_list = XCAM_NULL;
    struct list_head* pos_node = XCAM_NULL;
    struct list_head* tmp_node = XCAM_NULL;
    xcam_bool is_found = XCAM_FALSE;

    pthread_mutex_lock(&ptr_server_ctx->session_list_lock);
    list_for_each_safe(pos_node, tmp_node, &ptr_server_ctx->session_list) {
        session_in_list = list_entry(pos_node, livestream_rtsp_stream_session, list_ptr);
        if (0 == strncmp(stream_name, session_in_list->stream_name, RTSP_MAX_STREAMNAME_LEN)) {
            if (session_in_list->session_state != LIVESTREAM_RTSP_SESSION_STATE_STOP) {
                is_found = XCAM_TRUE;
                *pptr_session = session_in_list;
                break;
            }
        }
    }
    pthread_mutex_unlock(&ptr_server_ctx->session_list_lock);

    return is_found;
}

/*remove all the session related to the media stream if a stream is removed*/
static xcam_s32 remove_stream_session(livestream_rtsp_server_ctx* ptr_server_ctx,
    const xcam_char* stream_name)
{
    livestream_rtsp_stream_session* session_in_list = XCAM_NULL;
    xcam_bool is_found = XCAM_TRUE;
    xcam_s32 ret = XCAM_SUCCESS;

    if (XCAM_NULL == ptr_server_ctx || XCAM_NULL == stream_name) {
        LOG_LIVESTREAM_ERROR("Invalid param ptr_server_ctx=%p, stream_name=%p\n",
            ptr_server_ctx, stream_name);
        return LIVESTREAM_ERRNO_ILLEGAL_PARAM;
    }

    while (is_found) {
        is_found = find_session_valid(ptr_server_ctx, stream_name, &session_in_list);
        if (!is_found) {
            break;
        }

        /*when remove a streamsession set stop state and wait for the destroy*/
        ret = livestream_rtsp_session_stop(session_in_list);
        if ( XCAM_SUCCESS != ret) {
            LOG_LIVESTREAM_ERROR("stream session destroy failed ret:0x%x\n", ret);
            return LIVESTREAM_ERRNO_SESS_DESTROY_FAIL;
        }
    }

    ret = check_list_null(ptr_server_ctx, stream_name);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("remove session timeout, and ret:%d\n", ret);
        return LIVESTREAM_ERRNO_SESS_DESTROY_FAIL;
    }

    return XCAM_SUCCESS;
}

static xcam_s32 on_client_connection(xcam_void* object, xcam_s32 sockfd, 
                        xcam_char* ptr_client_req, xcam_u32 req_len)
{
    livestream_rtsp_server_ctx* ptr_server_ctx = (livestream_rtsp_server_ctx*)object;
    livestream_media_stream_node* stream_node = XCAM_NULL;
    xcam_char stream_str[RTSP_MAX_STREAMNAME_LEN] = {0};
    xcam_char reply_str[RTSP_MAX_PROTOCOL_BUFFER] = {0};
    struct list_head* pos_node = XCAM_NULL;
    xcam_s32 stat_code = RTSP_STATUS_CODE_OK;
    xcam_bool is_get_stream_name = XCAM_TRUE;
    xcam_bool is_found = XCAM_FALSE;
    xcam_s32 ret = XCAM_SUCCESS;
    xcam_s32 resp_len = 0;
    xcam_s32 cseq = 0;

    RTSPSVR_CHECK_NULL_ERROR(ptr_server_ctx);
    RTSPSVR_CHECK_NULL_ERROR(ptr_client_req);
    if (!livestream_rtsp_message_check_request(ptr_client_req)) {
        LOG_LIVESTREAM_ERROR("invalid rtsp request, just ignore this connection\n");
        stat_code = RTSP_STATUS_CODE_BAD_REQUEST;
        ret = LIVESTREAM_ERRNO_SESS_BAD_REQUEST;
        goto Failed;
    }

    ret = livestream_rtsp_message_get_stream_name(ptr_client_req, stream_str, RTSP_MAX_STREAMNAME_LEN);
    if (ret  != XCAM_SUCCESS) {
        LOG_LIVESTREAM_ERROR("could not find stream name!\n");
        is_get_stream_name = XCAM_FALSE;
    }

    ret = livestream_rtsp_message_get_cseq(ptr_client_req, &cseq);
    if (ret  != XCAM_SUCCESS) {
        LOG_LIVESTREAM_ERROR("invalid rtsp url, could not find  CSeq\n");
        stat_code = RTSP_STATUS_CODE_BAD_REQUEST;
        ret = LIVESTREAM_ERRNO_SESS_BAD_REQUEST;
        goto Failed;
    }

    if (!is_get_stream_name) {
        if (add_stream_session(ptr_server_ctx, ptr_client_req, req_len, sockfd, XCAM_NULL) != XCAM_SUCCESS) {
            LOG_LIVESTREAM_ERROR("Add stream session failed\n");
            stat_code = RTSP_STATUS_CODE_INTERNAL_SERVER_ERROR;
            ret = LIVESTREAM_ERRNO_STREAM_ADD_SESS_FAIL;
            goto Failed;
        }
        return XCAM_SUCCESS;
    }

    //if find stream name ,start session with streamnode
    pthread_mutex_lock(&ptr_server_ctx->stream_list_lock);
    list_for_each(pos_node, &ptr_server_ctx->stream_list) {
        stream_node = list_entry(pos_node, livestream_media_stream_node, list_ptr);
        if (!strcasecmp(stream_node->name, stream_str)) {
            is_found = XCAM_TRUE;
            break;
        }
    }

    if (!is_found || XCAM_NULL == stream_node) {
        LOG_LIVESTREAM_ERROR("invalid rtsp request, could not find correspond stream\n");
        stat_code = RTSP_STATUS_CODE_BAD_REQUEST;
        ret = LIVESTREAM_ERRNO_SESS_STREAM_NOT_FOUND;
        pthread_mutex_unlock(&ptr_server_ctx->stream_list_lock);
        goto Failed;
    }

    if (add_stream_session(ptr_server_ctx, ptr_client_req, req_len, sockfd, stream_node) != XCAM_SUCCESS) {
        LOG_LIVESTREAM_ERROR("Add stream session failed!\n");
        stat_code = RTSP_STATUS_CODE_INTERNAL_SERVER_ERROR;
        ret = LIVESTREAM_ERRNO_STREAM_ADD_SESS_FAIL;
        pthread_mutex_unlock(&ptr_server_ctx->stream_list_lock);
        goto Failed;
    }

    pthread_mutex_unlock(&ptr_server_ctx->stream_list_lock);

    return XCAM_SUCCESS;

Failed:
    livestream_rtsp_message_get_response(stat_code, cseq, reply_str, RTSP_MAX_PROTOCOL_BUFFER);
    resp_len = strlen(reply_str);
    if (resp_len > 0) {
        livestream_rtsp_session_send(sockfd, reply_str, resp_len);
    }

    livestream_comm_close_socket(&sockfd);
    return ret;
}

static xcam_s32 check_stream_config(xcam_livestream_rtsp_source* stream_src, xcam_char* stream_name, xcam_u32 buf_size)
{
    if (RTSP_MAX_STREAMNAME_LEN < strlen(stream_name) || 0 == strlen(stream_name)) {
        LOG_LIVESTREAM_ERROR("stream name too long max len:%d\n", RTSP_MAX_STREAMNAME_LEN);
        return LIVESTREAM_ERRNO_ILLEGAL_PARAM;
    }

    if (buf_size < RTSP_MIN_BUFFER_SIZE || buf_size > RTSP_MAX_BUFFER_SIZE) {
        LOG_LIVESTREAM_ERROR("param buf_size not in the range min:%d  max:%d!\n", 
                RTSP_MIN_BUFFER_SIZE, RTSP_MAX_BUFFER_SIZE);
        return LIVESTREAM_ERRNO_ILLEGAL_PARAM;
    }

    if (XCAM_NULL == stream_src->video_src_handle && XCAM_NULL == stream_src->audio_src_handle) {
        LOG_LIVESTREAM_ERROR("can not add a stream with no audio and no video\n");
        return LIVESTREAM_ERRNO_ILLEGAL_PARAM;
    }

    if (XCAM_NULL != stream_src->audio_src_handle 
        && XCAM_TRACK_SOURCE_TYPE_AUDIO != stream_src->audio_src_handle->track_type) {
        LOG_LIVESTREAM_ERROR("can not add a audio src with video type\n");
        return LIVESTREAM_ERRNO_ILLEGAL_PARAM;
    }

    if (XCAM_NULL != stream_src->video_src_handle) {
        if (XCAM_TRACK_SOURCE_TYPE_VIDEO != stream_src->video_src_handle->track_type) {
            LOG_LIVESTREAM_ERROR("can not add a video src with audio type\n");
            return LIVESTREAM_ERRNO_ILLEGAL_PARAM;
        }
        if (XCAM_TRACK_VIDEO_CODEC_H264 != stream_src->video_src_handle->track_source_attr.video_info.codec_type 
           && XCAM_TRACK_VIDEO_CODEC_H265 != stream_src->video_src_handle->track_source_attr.video_info.codec_type) {
            LOG_LIVESTREAM_ERROR("not support video type :%d \n",
                stream_src->video_src_handle->track_source_attr.video_info.codec_type);
            return LIVESTREAM_ERRNO_ILLEGAL_PARAM;
        }
        if (XCAM_NULL == stream_src->video_src_handle->func_request_key_frame) {
            LOG_LIVESTREAM_ERROR("video source call back null error \n");
            return LIVESTREAM_ERRNO_ILLEGAL_PARAM;
        }
    }

    if (XCAM_NULL != stream_src->audio_src_handle) {
        if (XCAM_TRACK_SOURCE_TYPE_AUDIO != stream_src->audio_src_handle->track_type) {
            LOG_LIVESTREAM_ERROR("can not add a audio src with video type\n");
            return LIVESTREAM_ERRNO_ILLEGAL_PARAM;
        }
        if (is_audio_codec_support(stream_src->audio_src_handle->track_source_attr.audio_info.codec_type) == XCAM_FALSE) {
            LOG_LIVESTREAM_ERROR("stream data illegal\n");
            return LIVESTREAM_ERRNO_ILLEGAL_PARAM;
        }
    }

    return XCAM_SUCCESS;
}

static xcam_void get_one_stream(livestream_rtsp_server_ctx* server_ctx, xcam_char* name, xcam_u32 size)
{
    livestream_media_stream_node* stream_node = XCAM_NULL;
    struct list_head* tmp_node = XCAM_NULL;
    struct list_head* pos_node = XCAM_NULL;

    pthread_mutex_lock(&server_ctx->stream_list_lock);
    if (size == 0) {
        LOG_LIVESTREAM_ERROR("stream name buf size[%u] is illegal\n", size);
        return;
    }
    /* delete all the  session related to the mediastream*/
    list_for_each_safe(pos_node, tmp_node, &server_ctx->stream_list) {
        stream_node = list_entry(pos_node, livestream_media_stream_node, list_ptr);
        if (snprintf(name, size - 1, "%s", stream_node->name) < 0) {
            LOG_LIVESTREAM_ERROR("string print name error\n");
        }
        break;
    }
    pthread_mutex_unlock(&server_ctx->stream_list_lock);

    return;
}

static xcam_bool find_one_stream_and_delete(livestream_rtsp_server_ctx* server_ctx,
                    const xcam_char* stream_name)
{
    livestream_media_stream_node* stream_node = XCAM_NULL;
    struct list_head* pos_node = XCAM_NULL;
    struct list_head* tmp_node = XCAM_NULL;
    xcam_bool is_found = XCAM_FALSE;

    pthread_mutex_lock(&server_ctx->stream_list_lock);
    list_for_each_safe(pos_node, tmp_node, &server_ctx->stream_list) {
        stream_node = list_entry(pos_node, livestream_media_stream_node, list_ptr);
        if (!strcasecmp(stream_node->name, stream_name))
        {
            is_found = XCAM_TRUE;
            list_del(&(stream_node->list_ptr));
            free(stream_node);
            stream_node = XCAM_NULL;
            server_ctx->stream_num--;
            break;
        }
    }
    pthread_mutex_unlock(&server_ctx->stream_list_lock);

    return is_found;
}

xcam_s32 xcam_livestream_rtsp_server_write_frame(xcam_void* handle, xcam_track_source_handle track_src,
                const xcam_livestream_rtsp_data* ptr_data)
{
    livestream_rtsp_stream_session* session_in_list = XCAM_NULL;
    livestream_media_stream_node* stream_node = XCAM_NULL;
    livestream_rtsp_server_ctx* ptr_server_ctx = XCAM_NULL;
    struct list_head* tmp_node = XCAM_NULL;
    struct list_head* pos_node = XCAM_NULL;
    xcam_bool is_found_stream = XCAM_FALSE;
    xcam_s32 ret = XCAM_SUCCESS;

    RTSPSVR_CHECK_NULL_ERROR(handle);
    if (XCAM_NULL == g_server_ctx) {
        LOG_LIVESTREAM_ERROR("rtspserver not created fail!\n");
        return LIVESTREAM_ERRNO_NOT_CREATE;
    }

    if (handle != (xcam_void*)g_server_ctx) {
        LOG_LIVESTREAM_ERROR("rtspserver handle invalid \n");
        return LIVESTREAM_ERRNO_HANDLE_INVALID;
    }

    ptr_server_ctx = (livestream_rtsp_server_ctx*)handle;
    RTSPSVR_CHECK_NULL_ERROR(ptr_server_ctx);
    RTSPSVR_CHECK_NULL_ERROR(track_src);
    RTSPSVR_CHECK_NULL_ERROR(ptr_data);
    ret = check_stream_data(track_src, ptr_data);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("rtsperver write frame data illegal\n");
        return ret;
    }

    //find the stream node in the list
    pthread_mutex_lock(&ptr_server_ctx->stream_list_lock);
    list_for_each_safe(pos_node, tmp_node, &ptr_server_ctx->stream_list) {
        stream_node = list_entry(pos_node, livestream_media_stream_node, list_ptr);
        if (stream_node->video_stream == track_src ||  stream_node->audio_stream == track_src) {
            is_found_stream = XCAM_TRUE;
            break;
        }
    }
    pthread_mutex_unlock(&ptr_server_ctx->stream_list_lock);

    if (XCAM_TRUE != is_found_stream) {
        LOG_LIVESTREAM_WARN("stream not find, write frame fail\n");
        return LIVESTREAM_ERRNO_STREAM_NOT_EXIST;
    }

    pthread_mutex_lock(&ptr_server_ctx->session_list_lock);
    list_for_each_safe(pos_node, tmp_node, &ptr_server_ctx->session_list) {
        session_in_list = list_entry(pos_node, livestream_rtsp_stream_session, list_ptr);
        if (session_in_list->video_stream == track_src || session_in_list->audio_stream == track_src) {
            /*write frame for the session*/
            ret = livestream_rtsp_session_write_frame(session_in_list, track_src, ptr_data);
            if (XCAM_SUCCESS != ret) {
                pthread_mutex_unlock(&ptr_server_ctx->session_list_lock);
                LOG_LIVESTREAM_ERROR("Write frame failed ret:0x%x mediatype:0x%x!\n",
                    ret, track_src->track_type);
                return ret;
            }
        }
    }
    pthread_mutex_unlock(&ptr_server_ctx->session_list_lock);

    return XCAM_SUCCESS;
}

xcam_s32 xcam_livestream_rtsp_server_add_media_stream(xcam_void* handle, xcam_livestream_rtsp_source* stream_src,
                xcam_char* stream_name, xcam_u32 buf_size)
{
    RTSPSVR_CHECK_NULL_ERROR(handle);

    livestream_media_stream_node* stream_node = XCAM_NULL;
    livestream_rtsp_server_ctx* ptr_server_ctx = XCAM_NULL;
    struct list_head* pos_node = XCAM_NULL;
    xcam_s32 ret = XCAM_SUCCESS;

    if (XCAM_NULL == g_server_ctx) {
        LOG_LIVESTREAM_ERROR("rtsperver not created!\n");
        return LIVESTREAM_ERRNO_NOT_CREATE;
    }

    if (handle != (xcam_void*)g_server_ctx) {
        LOG_LIVESTREAM_ERROR("rtspserver handle invalid \n");
        return LIVESTREAM_ERRNO_HANDLE_INVALID;
    }

    ptr_server_ctx = (livestream_rtsp_server_ctx*)handle;
    RTSPSVR_CHECK_NULL_ERROR(ptr_server_ctx);
    RTSPSVR_CHECK_NULL_ERROR(stream_name);
    RTSPSVR_CHECK_NULL_ERROR(stream_src);

    if (ptr_server_ctx->stream_num >= RTSP_MAX_STREAM_NUM) {
        LOG_LIVESTREAM_ERROR("rtsperver stream num reach max:%d\n", RTSP_MAX_STREAM_NUM);
        return LIVESTREAM_ERRNO_STREAM_ADD_FAIL;
    }

    ret = check_stream_config(stream_src, stream_name, buf_size);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("rtsperver check param illegal!\n");
        return ret;
    }

    /*find the stream node in the list */
    pthread_mutex_lock(&ptr_server_ctx->stream_list_lock);
    list_for_each(pos_node, &ptr_server_ctx->stream_list) {
        stream_node = list_entry(pos_node, livestream_media_stream_node, list_ptr);
        if (!strcasecmp(stream_node->name, stream_name)) {
            LOG_LIVESTREAM_ERROR("media source streamname alreay exist\n");
            pthread_mutex_unlock(&ptr_server_ctx->stream_list_lock);
            return LIVESTREAM_ERRNO_STREAM_EXISTED;
        }
    }

    /* stream node not exist then create and add to the list*/
    stream_node = (livestream_media_stream_node*)malloc(sizeof(livestream_media_stream_node));
    if (!stream_node) {
        LOG_LIVESTREAM_ERROR("dynamic alloc rtsp LiveMediaStreamNode failed\n");
        pthread_mutex_unlock(&ptr_server_ctx->stream_list_lock);
        return LIVESTREAM_ERRNO_MALLOC_FAIL;
    }

    memset(stream_node, 0x00, sizeof(livestream_media_stream_node));
    strncpy(stream_node->name, stream_name, RTSP_MAX_STREAMNAME_LEN - 1);
    stream_node->name[RTSP_MAX_STREAMNAME_LEN - 1] = '\0';
    stream_node->audio_stream = stream_src->audio_src_handle;
    stream_node->video_stream = stream_src->video_src_handle;
    stream_node->buf_size = buf_size;
    stream_node->audio_start_cnt = 0;
    stream_node->video_start_cnt = 0;

    ptr_server_ctx->stream_num++;
    list_add(&(stream_node->list_ptr), &(ptr_server_ctx->stream_list));
    pthread_mutex_unlock(&ptr_server_ctx->stream_list_lock);

    return XCAM_SUCCESS;
}

xcam_s32 xcam_livestream_rtsp_server_remove_media_stream(xcam_void* handle, const xcam_char* stream_name)
{
    RTSPSVR_CHECK_NULL_ERROR(handle);

    livestream_rtsp_server_ctx* ptr_server_ctx = XCAM_NULL;
    xcam_bool is_found = XCAM_FALSE;
    xcam_s32 ret = XCAM_SUCCESS;

    if ( XCAM_NULL == g_server_ctx) {
        LOG_LIVESTREAM_ERROR("rtsperver not created!\n");
        return LIVESTREAM_ERRNO_NOT_CREATE;
    }

    if (handle != (xcam_void*)g_server_ctx) {
        LOG_LIVESTREAM_ERROR("rtspserver handle invalid \n");
        return LIVESTREAM_ERRNO_HANDLE_INVALID;
    }

    ptr_server_ctx = (livestream_rtsp_server_ctx*)handle;
    RTSPSVR_CHECK_NULL_ERROR(ptr_server_ctx);
    RTSPSVR_CHECK_NULL_ERROR(stream_name);

    if (RTSP_MAX_STREAMNAME_LEN < strlen(stream_name) || 0 == strlen(stream_name)) {
        LOG_LIVESTREAM_ERROR("rtsperver  streamname error len \n");
        return LIVESTREAM_ERRNO_ILLEGAL_PARAM;
    }

    /*when remove a media stream destroy the related connections if existed*/
    ret = remove_stream_session(ptr_server_ctx, stream_name);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("could not find remove stream session for stream: %s ret:%d\n",
            stream_name, ret);
        return LIVESTREAM_ERRNO_STREAM_REMOVE_FAIL;
    }

    is_found = find_one_stream_and_delete(ptr_server_ctx, stream_name);
    if (!is_found) {
        LOG_LIVESTREAM_ERROR("could not find correspond media stream\n");
        return LIVESTREAM_ERRNO_STREAM_NOT_EXIST;
    }

    return XCAM_SUCCESS;
}

xcam_s32 xcam_livestream_rtsp_server_start(xcam_void* handle)
{
    RTSPSVR_CHECK_NULL_ERROR(handle);

    livestream_rtsp_server_ctx* ptr_server_ctx = XCAM_NULL;
    xcam_s32 ret = XCAM_SUCCESS;

    if (XCAM_NULL == g_server_ctx) {
        LOG_LIVESTREAM_ERROR("rtsperver not created!\n");
        return LIVESTREAM_ERRNO_NOT_CREATE;
    }

    if (handle != (xcam_void*)g_server_ctx) {
        LOG_LIVESTREAM_ERROR("rtspserver handle invalid \n");
        return LIVESTREAM_ERRNO_HANDLE_INVALID;
    }

    ptr_server_ctx = (livestream_rtsp_server_ctx*)handle;
    RTSPSVR_CHECK_NULL_ERROR(ptr_server_ctx);
    RTSPSVR_CHECK_NULL_ERROR(ptr_server_ctx->net_listener);
    if (ptr_server_ctx->is_started) {
        LOG_LIVESTREAM_ERROR("Rtspserver already started  \n");
        return LIVESTREAM_ERRNO_START_LISTENER_AGAIN;
    }

    ret = livestream_listener_start(ptr_server_ctx->net_listener);
    if (ret != XCAM_SUCCESS) {
        LOG_LIVESTREAM_ERROR("Rtsp start listener failed ret:%d \n", ret);
        return LIVESTREAM_ERRNO_START_LISTENER_FAIL;
    }

    livestream_listener_register_callback(ptr_server_ctx->net_listener,
            on_client_connection, (xcam_void*)ptr_server_ctx);

    ptr_server_ctx->is_started = XCAM_TRUE;
    return XCAM_SUCCESS;
}

xcam_s32 xcam_livestream_rtsp_server_stop(xcam_void* handle)
{
    RTSPSVR_CHECK_NULL_ERROR(handle);

    livestream_rtsp_server_ctx* ptr_server_ctx;
    xcam_char tmp_name[RTSP_MAX_STREAMNAME_LEN];
    xcam_s32 ret = XCAM_SUCCESS;

    if (XCAM_NULL == g_server_ctx) {
        LOG_LIVESTREAM_ERROR("rtsperver not created!\n");
        return LIVESTREAM_ERRNO_NOT_CREATE;
    }

    if (handle != (xcam_void*)g_server_ctx) {
        LOG_LIVESTREAM_ERROR("rtspserver handle invalid \n");
        return LIVESTREAM_ERRNO_HANDLE_INVALID;
    }

    ptr_server_ctx = (livestream_rtsp_server_ctx*)handle;
    RTSPSVR_CHECK_NULL_ERROR(ptr_server_ctx);

    if (!ptr_server_ctx->is_started) {
        LOG_LIVESTREAM_ERROR("Rtspserver have not been started\n");
        return LIVESTREAM_ERRNO_STOP_LISTENER_NOT_STARTED;
    }
    if (ptr_server_ctx->net_listener == XCAM_NULL) {
        LOG_LIVESTREAM_ERROR("net_listener is XCAM_NULL Rtsp start listener failed \n");
        return LIVESTREAM_ERRNO_STOP_LISTENER_FAIL;
    }

    ret = livestream_listener_stop(ptr_server_ctx->net_listener);
    if (ret != XCAM_SUCCESS) {
        LOG_LIVESTREAM_ERROR("Rtsp start listener failed ret:%d \n", ret);
        return LIVESTREAM_ERRNO_STOP_LISTENER_FAIL;
    }

    while (0 != ptr_server_ctx->stream_num) {
        get_one_stream(ptr_server_ctx, tmp_name, RTSP_MAX_STREAMNAME_LEN);
        ret = remove_stream_session(ptr_server_ctx, tmp_name);
        if (XCAM_SUCCESS != ret) {
            LOG_LIVESTREAM_ERROR("could not find remove stream session for stream: %s\n", tmp_name);
            return LIVESTREAM_ERRNO_STREAM_REMOVE_SESS_FAIL;
        }

        find_one_stream_and_delete(ptr_server_ctx, tmp_name);
    }

    ptr_server_ctx->is_started = XCAM_FALSE;

    return XCAM_SUCCESS;
}

xcam_s32 xcam_livestream_rtsp_server_set_listener(xcam_void* handle, xcam_server_state_listener* state_listener)
{
    RTSPSVR_CHECK_NULL_ERROR(handle);
    livestream_rtsp_server_ctx* ptr_server_ctx;

    if (XCAM_NULL == g_server_ctx) {
        LOG_LIVESTREAM_ERROR("rtsperver not created!\n");
        return LIVESTREAM_ERRNO_NOT_CREATE;
    }

    if (handle != (xcam_void*)g_server_ctx) {
        LOG_LIVESTREAM_ERROR("rtspserver handle invalid \n");
        return LIVESTREAM_ERRNO_HANDLE_INVALID;
    }

    ptr_server_ctx = (livestream_rtsp_server_ctx*)handle;
    RTSPSVR_CHECK_NULL_ERROR(state_listener);
    RTSPSVR_CHECK_NULL_ERROR(state_listener->func_client_connect);
    RTSPSVR_CHECK_NULL_ERROR(state_listener->func_client_disconnect);
    RTSPSVR_CHECK_NULL_ERROR(state_listener->func_server_error);
    ptr_server_ctx->state_listener.func_client_connect = state_listener->func_client_connect;
    ptr_server_ctx->state_listener.func_client_disconnect = state_listener->func_client_disconnect;
    ptr_server_ctx->state_listener.func_server_error = state_listener->func_server_error;

    return XCAM_SUCCESS;
}

xcam_s32 xcam_livestream_rtsp_server_create(xcam_void** handle, xcam_livestream_rtsp_config* ptr_rtsp_config)
{
    livestream_rtsp_server_ctx* ptr_server_ctx = XCAM_NULL;
    xcam_s32 ret = XCAM_SUCCESS;

    RTSPSVR_CHECK_NULL_ERROR(handle);
    RTSPSVR_CHECK_NULL_ERROR(ptr_rtsp_config);
    ret = check_rtsp_config(ptr_rtsp_config);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("config param null !\n");
        return ret;
    }

    if (XCAM_NULL != g_server_ctx) {
        LOG_LIVESTREAM_ERROR("RTSPSERVER already created do not create again!\n");
        return LIVESTREAM_ERRNO_CREATE_AGAIN;
    }

    ptr_server_ctx = (livestream_rtsp_server_ctx*)malloc(sizeof(livestream_rtsp_server_ctx));
    if (!ptr_server_ctx) {
        LOG_LIVESTREAM_ERROR("dynamic alloc livestream_rtsp_server_ctx failed\n");
        return LIVESTREAM_ERRNO_MALLOC_FAIL;
    }

    memset(ptr_server_ctx, 0x00, sizeof(livestream_rtsp_server_ctx));
    pthread_mutex_init(&ptr_server_ctx->session_list_lock, XCAM_NULL);
    pthread_mutex_init(&ptr_server_ctx->stream_list_lock, XCAM_NULL);
    ptr_server_ctx->max_conn_num = ptr_rtsp_config->max_conn_num;
    ptr_server_ctx->listen_port = ptr_rtsp_config->listen_port;
    ptr_server_ctx->max_payload = ptr_rtsp_config->max_payload;
    ptr_server_ctx->packet_len = ptr_rtsp_config->packet_len;
    ptr_server_ctx->timeout = ptr_rtsp_config->timeout;
    ptr_server_ctx->user_num = 0;
    ptr_server_ctx->stream_num = 0;

    INIT_LIST_HEAD(&ptr_server_ctx->stream_list);
    INIT_LIST_HEAD(&ptr_server_ctx->session_list);
    ret = livestream_listener_create(ptr_server_ctx->listen_port, &ptr_server_ctx->net_listener);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("Rtsp create listener failed ret:%d\n", ret);
        pthread_mutex_destroy(&ptr_server_ctx->stream_list_lock);
        pthread_mutex_destroy(&ptr_server_ctx->session_list_lock);
        free(ptr_server_ctx);
        ptr_server_ctx = XCAM_NULL;
        return LIVESTREAM_ERRNO_CREATE_LISTENER_FAIL;
    }

    *handle = (xcam_void*)ptr_server_ctx;
    g_server_ctx = ptr_server_ctx;
    return XCAM_SUCCESS;
}

xcam_s32 xcam_livestream_rtsp_server_destroy(xcam_void* handle)
{
    xcam_char tmp_name[RTSP_MAX_STREAMNAME_LEN];
    xcam_s32 ret = XCAM_SUCCESS;

    RTSPSVR_CHECK_NULL_ERROR(handle);
    if (XCAM_NULL == g_server_ctx) {
        LOG_LIVESTREAM_ERROR("rtsperver not created!\n");
        return LIVESTREAM_ERRNO_NOT_CREATE;
    }

    if (handle != (xcam_void*)g_server_ctx) {
        LOG_LIVESTREAM_ERROR("rtspserver handle invalid \n");
        return LIVESTREAM_ERRNO_HANDLE_INVALID;
    }

    livestream_rtsp_server_ctx* ptr_server_ctx = (livestream_rtsp_server_ctx*)handle;
    RTSPSVR_CHECK_NULL_ERROR(ptr_server_ctx);

    while (0 != ptr_server_ctx->stream_num) {
        get_one_stream(ptr_server_ctx, tmp_name, RTSP_MAX_STREAMNAME_LEN);
        ret = remove_stream_session(ptr_server_ctx, tmp_name);
        if (XCAM_SUCCESS != ret) {
            LOG_LIVESTREAM_ERROR("could not find remove stream session for stream: %s\n", tmp_name);
            return LIVESTREAM_ERRNO_STREAM_REMOVE_SESS_FAIL;
        }

        find_one_stream_and_delete(ptr_server_ctx, tmp_name);
    }

    pthread_mutex_destroy(&ptr_server_ctx->stream_list_lock);
    pthread_mutex_destroy(&ptr_server_ctx->session_list_lock);

    ret = livestream_listener_destroy(ptr_server_ctx->net_listener);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("Rtsp destroy listener failed ret:%d \n", ret);
        return LIVESTREAM_ERRNO_DESTROY_LISTENER_FAIL;
    }

    free(ptr_server_ctx);
    ptr_server_ctx = XCAM_NULL;
    g_server_ctx = XCAM_NULL;

    return XCAM_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
