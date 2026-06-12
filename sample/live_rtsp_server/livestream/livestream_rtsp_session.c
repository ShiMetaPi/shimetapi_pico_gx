/*
 * Copyright (c) XCAM. All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "livestream_errno.h"
#include "livestream_comm.h"
#include "livestream_mbuffer.h"
#include "xcam_livestream_rtsp_server.h"
#include "livestream_rtsp_session.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define RTSP_MAX_SDP_LEN    (1024)
#define RTSP_CLOCK_RATE_90K (90000)
#define RTSP_CLOCK_RATE_8K  (8000)
#define RTSP_TIME_SCALE     (1000000)
#define H265_NAL_MASK       (0x7f)
#define H264_NAL_MASK       (0x1f)
#define RTSP_TRANS_TIMEVAL_SEC     (3)
#define RTSP_TRANS_TIMEVAL_USEC    (0)

static xcam_void cut_message(xcam_char* msg_buf, xcam_u32* out_len)
{
    xcam_char temp[RTSP_MAX_PROTOCOL_BUFFER] = {0};
    xcam_char* idx_temp = XCAM_NULL;
    xcam_u32 msg_len = 0;

    //strncpy(temp, msg_buf, RTSP_MAX_PROTOCOL_BUFFER - 1);
    //temp[RTSP_MAX_PROTOCOL_BUFFER - 1] = '\0';
    snprintf(temp, sizeof(temp), "%s", msg_buf);
    idx_temp = strstr(temp, "\r\n\r\n");
    if (XCAM_NULL == idx_temp) {
        return;
    }

    memset(msg_buf, 0x00, *out_len);
    msg_len = (xcam_u32)(idx_temp - temp) + 4;
    idx_temp += 4;
    *out_len = *out_len - msg_len;
    if (*out_len > 0) {
        strncpy(msg_buf, idx_temp, RTSP_MAX_PROTOCOL_BUFFER - 1);
        temp[RTSP_MAX_PROTOCOL_BUFFER - 1] = '\0';
    }

    return;
}

/**
 * @brief cut header response message.
 * @param[in] request : message from client, out_len: length of message
 * @return   0 success
 * @return  -1 failure
 */
static xcam_s32 cut_invaild_message(xcam_char* request, xcam_u32* out_len)
{
    xcam_char temp[RTSP_MAX_PROTOCOL_BUFFER] = {0};
    xcam_u32 msg_len = 0;

    if ((XCAM_NULL == request) || (XCAM_NULL == out_len)) {
        LOG_LIVESTREAM_ERROR("The input msg is NULL!\n");
        return XCAM_FAILURE;
    }

    if ((*(request + 0) == RTSP_RESPONSE_PACKRT_FIRST_BYTE)) {
        LOG_LIVESTREAM_INFO("It is contains response packet!\n");
        msg_len = RTSP_RESPONSE_PACKET_LENGTH;
        memset(temp, 0, RTSP_MAX_PROTOCOL_BUFFER);
        memcpy(temp, request, RTSP_MAX_PROTOCOL_BUFFER);
        temp[RTSP_MAX_PROTOCOL_BUFFER - 1] = '\0';
        memset(request, 0x00, RTSP_MAX_PROTOCOL_BUFFER);
        memcpy(request, temp + msg_len, RTSP_MAX_PROTOCOL_BUFFER - msg_len);
        if (*out_len == msg_len) {
            LOG_LIVESTREAM_INFO("It is just a single response packet!\n");
            *out_len = 0;
            return XCAM_SUCCESS;
        }

        *out_len = *out_len - msg_len;
    }

    return XCAM_SUCCESS;
}

/**
 * @brief cut header rtcp msg.
 * @param[in] request : msg from client, out_len: length of Msg, half_flag: the flag of half packet
 * @return   0 success
 * @return  -1 failure
 */
static xcam_s32 cut_rtcp_message(xcam_char* request, xcam_u32* out_len, xcam_bool* half_flag)
{
    xcam_char temp[RTSP_MAX_PROTOCOL_BUFFER] = {0};
    xcam_u32 msg_len = 0;
    xcam_u32 pt = 0;

    if ((XCAM_NULL == request) || (XCAM_NULL == out_len)) {
        LOG_LIVESTREAM_ERROR("The input msg is NULL!\n");
        return XCAM_FAILURE;
    }

    pt = *(request + 1) & 0xFF;
    while ((RTSP_RTCP_PACKRT_TYPE_SR == pt)  || (RTSP_RTCP_PACKRT_TYPE_RR == pt)
        || (RTSP_RTCP_PACKRT_TYPE_SDES == pt) || (RTSP_RTCP_PACKRT_TYPE_BYE == pt)
        || (RTSP_RTCP_PACKRT_TYPE_APP == pt)) {
        LOG_LIVESTREAM_INFO("It contains just rtcp packet!\n");
        memset(temp, 0, RTSP_MAX_PROTOCOL_BUFFER);
        memcpy(temp, request, RTSP_MAX_PROTOCOL_BUFFER);
        temp[RTSP_MAX_PROTOCOL_BUFFER - 1] = '\0';
        if ((xcam_u32)(((temp[2] & 0xFF) << 8) + temp[3]) < UINT_MAX / RTSP_RTCP_BYTES_LENGTH) {
            msg_len = (((temp[2] & 0xFF) << 8) + temp[3] ) * RTSP_RTCP_BYTES_LENGTH  + RTSP_RTCP_HEADER_LENGTH;
        }

        if (*out_len >= msg_len) {
            memset(request, 0x00, RTSP_MAX_PROTOCOL_BUFFER);
            memcpy(request, temp + msg_len, RTSP_MAX_PROTOCOL_BUFFER - msg_len);
            if (*out_len == msg_len) {
                LOG_LIVESTREAM_INFO("It is just a single rtcp packet!\n");
                *out_len = 0;
                *half_flag = XCAM_FALSE;
                return XCAM_SUCCESS;
            }

            LOG_LIVESTREAM_INFO("The packet contains a rtcp and other packet(s)!\n");
            *out_len = *out_len - msg_len;
            pt = *(request + 1) & 0xFF;
            if ((RTSP_RTCP_PACKRT_TYPE_SR == pt) || (RTSP_RTCP_PACKRT_TYPE_RR == pt)
              || (RTSP_RTCP_PACKRT_TYPE_SDES == pt) || (RTSP_RTCP_PACKRT_TYPE_BYE == pt)
              || (RTSP_RTCP_PACKRT_TYPE_APP == pt)) {
                LOG_LIVESTREAM_INFO("The packet still contains a rtcp and other packet(s)!\n");
                continue;
            }
            *half_flag = XCAM_FALSE;
            return XCAM_SUCCESS;
        }
        *half_flag = XCAM_TRUE;
        return XCAM_SUCCESS;
    }
    *half_flag = XCAM_FALSE;
    return XCAM_SUCCESS;
}

/**
 * @brief refresh the select time out.
 * @param[in] rtsp_session : session info, recv_buff : msg from client, buff_len: length of msg
 * @return   0 success
 * @return  -1 failure
 */
static xcam_void refresh_select_time(livestream_rtsp_stream_session* rtsp_session,
            xcam_char* recv_buff, const xcam_u32* buff_len, struct timeval* timeout_val)
{
    if ((XCAM_NULL == rtsp_session) || (XCAM_NULL == recv_buff)) {
        LOG_LIVESTREAM_ERROR("Input argument is null!\n");
        return;
    }

    /*media data transmiting period, do not block media sending*/
    if (LIVESTREAM_RTSP_SESSION_STATE_PLAY == rtsp_session->session_state) {
        timeout_val->tv_sec = 0;
        timeout_val->tv_usec = 0;
    } else {
        /*if request not deal completely, not block the thread*/
        if (0 != *buff_len) {
            /*if there is only a half,not need to change the waiting time*/
            if (XCAM_NULL != strstr(recv_buff, "\r\n\r\n")) {
                /*have request msg to be handled*/
                timeout_val->tv_sec = 0;
                timeout_val->tv_usec = 0;
            } else {
                timeout_val->tv_sec = RTSP_TRANS_TIMEVAL_SEC;
                timeout_val->tv_usec = 0;
            }
        } else {
            /*single packet condition*/
            timeout_val->tv_sec = RTSP_TRANS_TIMEVAL_SEC;
            timeout_val->tv_usec = 0;
        }
    }
}

/**
 * @brief handle request when select time out.
 * @param[in] rtsp_session: session info,  pcRecvBuff: msg from client, buff_len: length of msg
 * @return   0 success
 * @return  -1 failure
 */
static xcam_s32 recv_message_timeout(livestream_rtsp_stream_session* rtsp_session, xcam_char* recv_buff, xcam_u32* buff_len)
{
    if ((XCAM_NULL == rtsp_session) || (XCAM_NULL == recv_buff)) {
        LOG_LIVESTREAM_ERROR("Input argument is null!!!\n");
        return XCAM_FAILURE;
    }

    if (LIVESTREAM_RTSP_SESSION_STATE_PLAY != rtsp_session->session_state) {
        if (0 != *buff_len) {
            if (XCAM_NULL != strstr(recv_buff, "\r\n\r\n")) {
                /*have complete request packet to be handled*/
                return XCAM_SUCCESS;
            }
        }

        return XCAM_FAILURE;
    }

    return XCAM_SUCCESS;
}

static xcam_void handle_bad_request(livestream_rtsp_stream_session* rtsp_session)
{
    xcam_char buffer[MAX_DATE_LEN] = {0};
    xcam_char status_str[MAX_DATE_LEN] = {0};

    memset(status_str, '\0', sizeof(status_str));
    livestream_rtsp_message_status_code_to_str(RTSP_STATUS_CODE_BAD_REQUEST, status_str, sizeof(status_str)-1),

    memset(buffer, '\0', sizeof(buffer));
    livestream_comm_format_date(buffer, sizeof(buffer)-1);
    int ret = snprintf(rtsp_session->response_buf, sizeof(rtsp_session->response_buf) - 1,
                    "%s %d %s\r\n"
                    "%sAllow: %s\r\n\r\n",
                    RTSP_VER_STR,
                    RTSP_STATUS_CODE_BAD_REQUEST,
                    status_str,
                    buffer,
                    RTSP_SUPPORTED_CMD_LIST);
    if (ret < 0) {
        LOG_LIVESTREAM_ERROR("String print response buf error\n");
        return;
    }
    return;
}

static xcam_void handle_server_error(livestream_rtsp_stream_session* rtsp_session)
{
    livestream_rtsp_message_get_response(RTSP_STATUS_CODE_INTERNAL_SERVER_ERROR,
        rtsp_session->cur_request_seq, rtsp_session->response_buf, sizeof(rtsp_session->response_buf));
}

static xcam_void handle_not_supported(livestream_rtsp_stream_session* rtsp_session)
{
    xcam_char status_str[MAX_DATE_LEN] = {0};
    xcam_char buffer[MAX_DATE_LEN] = {0};
    int ret;

    memset(status_str, '\0', sizeof(status_str));
    livestream_rtsp_message_status_code_to_str(RTSP_STATUS_CODE_METHOD_NOT_ALLOWED,
        status_str, sizeof(status_str)-1);
    memset(buffer, '\0', sizeof(buffer));
    livestream_comm_format_date(buffer, sizeof(buffer)-1);
    ret = snprintf(rtsp_session->response_buf, sizeof(rtsp_session->response_buf) - 1,
                "%s %d %s\r\n"
                "CSeq: %d \r\n"
                "%sAllow: %s\r\n\r\n",
                RTSP_VER_STR,
                RTSP_STATUS_CODE_METHOD_NOT_ALLOWED,
                status_str,
                rtsp_session->cur_request_seq,
                buffer,
                RTSP_SUPPORTED_CMD_LIST);
    if (ret < 0) {
        LOG_LIVESTREAM_ERROR("string print response buf error\n");
        return;
    }
    return;
}

static xcam_void handle_stream_not_found(livestream_rtsp_stream_session* rtsp_session)
{
    livestream_rtsp_message_get_response(RTSP_STATUS_CODE_STREAM_NOT_FOUND,
        rtsp_session->cur_request_seq, rtsp_session->response_buf, sizeof(rtsp_session->response_buf));
}

static xcam_void handle_option_not_support(livestream_rtsp_stream_session* rtsp_session)
{
    livestream_rtsp_message_get_response(RTSP_STATUS_CODE_OPTION_UNSUPPORT,
        rtsp_session->cur_request_seq, rtsp_session->response_buf, sizeof(rtsp_session->response_buf));
}

static xcam_void handle_unsupported_transport(livestream_rtsp_stream_session* rtsp_session)
{
    livestream_rtsp_message_get_response(RTSP_STATUS_CODE_UNSUPPORT_TRANSPORT,
        rtsp_session->cur_request_seq, rtsp_session->response_buf, sizeof(rtsp_session->response_buf));
}

static xcam_void update_state(livestream_rtsp_stream_session* rtsp_session, livestream_rtsp_request_method method)
{
    (xcam_void)pthread_mutex_lock(&rtsp_session->state_lock);
    if (LIVESTREAM_RTSP_REQ_METHOD_SETUP == method
        && LIVESTREAM_RTSP_SESSION_STATE_INIT == rtsp_session->session_state) {
        rtsp_session->session_state = LIVESTREAM_RTSP_SESSION_STATE_READY;
    }
    if (LIVESTREAM_RTSP_REQ_METHOD_PLAY == method
        && LIVESTREAM_RTSP_SESSION_STATE_READY == rtsp_session->session_state) {
        usleep(10 * 1000);
        rtsp_session->session_state = LIVESTREAM_RTSP_SESSION_STATE_PLAY;
    }
    (xcam_void)pthread_mutex_unlock(&rtsp_session->state_lock);

    return;
}

static xcam_s32 get_rtp_payload_type(livestream_rtsp_stream_session* rtsp_session,
    livestream_mbuffer_data_type data_type, rtp_payload_type* rtp_playload_type)
{
    xcam_track_audio_codec audio_codec = XCAM_TRACK_AUDIO_CODEC_BUTT;   /*audio format*/
    xcam_track_video_codec video_codec = XCAM_TRACK_VIDEO_CODEC_BUTT;   /*video format*/

    RTSPSVR_CHECK_NULL_ERROR(rtsp_session);
    if (data_type == MBUFFER_DATA_VIDEO) {
        /* get currnet video format*/
        video_codec = rtsp_session->media_info.video_source_info.codec_type;
        if (video_codec == XCAM_TRACK_VIDEO_CODEC_H265) {
            *rtp_playload_type = RTP_PT_H265;
        } else if (video_codec == XCAM_TRACK_VIDEO_CODEC_H264) {
            *rtp_playload_type = RTP_PT_H264;
        } else {
            LOG_LIVESTREAM_ERROR("Unsupport video codec[%d]!\n", video_codec);
            return LIVESTREAM_ERRNO_SESS_UNSUPPORT_MEDIATYPE;
        }
    } else if (data_type == MBUFFER_DATA_AUDIO) {
        /* get currnet audio format*/
        audio_codec = rtsp_session->media_info.audio_source_info.codec_type;
        if (audio_codec == XCAM_TRACK_AUDIO_CODEC_G711A) {
            *rtp_playload_type = RTP_PT_PCMA;
        } else if (audio_codec == XCAM_TRACK_AUDIO_CODEC_G711U) {
            *rtp_playload_type = RTP_PT_PCMU;
        } else if (audio_codec == XCAM_TRACK_AUDIO_CODEC_G726) {
            *rtp_playload_type = RTP_PT_G726;
        } else if (audio_codec == XCAM_TRACK_AUDIO_CODEC_AMR) {
            *rtp_playload_type = RTP_PT_AMR;
        } else if (audio_codec == XCAM_TRACK_AUDIO_CODEC_ADPCM) {
            *rtp_playload_type = RTP_PT_ADPCM;
        } else if (audio_codec == XCAM_TRACK_AUDIO_CODEC_AAC) {
            *rtp_playload_type = RTP_PT_AAC;
        } else if (audio_codec == XCAM_TRACK_AUDIO_CODEC_MP3) {
            *rtp_playload_type = RTP_PT_MPA;
        } else {
            LOG_LIVESTREAM_ERROR("Unsupport audio codec[%d]!\n", audio_codec);
            return LIVESTREAM_ERRNO_SESS_UNSUPPORT_MEDIATYPE;
        }
    } else {
        return LIVESTREAM_ERRNO_SESS_UNSUPPORT_MEDIATYPE;
    }

    return XCAM_SUCCESS;
}

static xcam_void get_media_send_port(livestream_rtsp_media_session* media_session)
{
    static xcam_u32 curr_port = 0;
    xcam_u32 max_port = 0;
    xcam_u32 min_port = 0;

    min_port = MIN_SEND_PORT;
    max_port = MAX_SEND_PORT;
    (xcam_void)pthread_mutex_lock(&(media_session->mutex_get_port));
    /*allocate the port num between the min and max user set*/
    if (curr_port < max_port - 2 && curr_port >= min_port) {
        curr_port += 2;
    } else {
        curr_port = min_port;
    }

    media_session->rtp_session->server_rtp_port = curr_port;
    media_session->rtcp_session->server_rtcp_port = curr_port + 1;
    (xcam_void)pthread_mutex_unlock(&(media_session->mutex_get_port));

    return;
}

static xcam_s32 parse_transports(livestream_rtsp_stream_session* rtsp_session,
                    const xcam_char* request, livestream_rtsp_media_session* media_session)
{
    xcam_bool last_transtype_flag = XCAM_FALSE;
    xcam_char trash[RTSP_TRASH_MAX_LEN] = {0};
    xcam_char line[RTSP_LINE_MAX_LEN] = {0};
    xcam_char line1[RTSP_LINE_MAX_LEN] = {0};
    xcam_char line2[RTSP_LINE_MAX_LEN] = {0};
    xcam_char ch_id1[RTSP_CHAR_MAX_LEN] = {0};
    xcam_char ch_id2[RTSP_CHAR_MAX_LEN] = {0};
    xcam_char* ptr_line1 = XCAM_NULL;
    xcam_char* ptr_line2 = XCAM_NULL;
    xcam_char* temp = XCAM_NULL;

    RTSPSVR_CHECK_NULL_ERROR(rtsp_session);
    RTSPSVR_CHECK_NULL_ERROR(media_session);
    /* Get the URL */
    if (!sscanf(request, " %*s %255s ", media_session->rtsp_url)) {
        handle_bad_request(rtsp_session);
        return XCAM_FAILURE;
    }
    media_session->rtsp_url[RTSP_URL_MAX_LEN - 1] = '\0';

    temp = strstr(request, RTSP_HEADER_TRANSPORT);
    if (XCAM_NULL == temp) {
        /*must has "Transport:", otherwise not accetpable*/
        LOG_LIVESTREAM_ERROR("There no Transport field in setup url! \n");
        handle_bad_request(rtsp_session);
        return XCAM_FAILURE;
    }

    /*
    Transport: RTP/AVP/TCP;unicast;interleaved=0-1;source=ip;ssrc=00003654
    trash-- Transport:
    line: RTP/AVP/TCP;unicast;interleaved=0-1;source=ip;ssrc=00003654
    */
    if (sscanf(temp, "%10s %255s", trash, line) != RTSP_SCANF_RET_TWO) {
        LOG_LIVESTREAM_ERROR("Transport formed unstandard!\n");
        handle_bad_request(rtsp_session);
        return XCAM_FAILURE;
    } else {
        ptr_line1 = line;
    }
    trash[RTSP_TRASH_MAX_LEN - 1] = '\0';
    line[RTSP_LINE_MAX_LEN - 1] = '\0';

    /*assert how mang trans type contained by Transport*/
    do {
        memset(line1, 0, RTSP_LINE_MAX_LEN);
        memset(line2, 0, RTSP_LINE_MAX_LEN);
        memset(ch_id1, 0, RTSP_CHAR_MAX_LEN);
        memset(ch_id2, 0, RTSP_CHAR_MAX_LEN);
        temp = XCAM_NULL;

        /*if the string has no "," it means has reached the last trans type info */
        /*line: RTP/AVP/TCP;unicast;interleaved=0-1,RTP/AVP;unicast;clientport=a-b
          line1: RTP/AVP/TCP;unicast;interleaved=0-1
          line2: RTP/AVP;unicast;clientport=c-d*/
        if (sscanf(ptr_line1, "%255s,%255s", line1, line2) != RTSP_SCANF_RET_TWO) {
            last_transtype_flag = XCAM_TRUE;
        }

        ptr_line2 = line2;
        if (strstr(line1, "RTP/AVP/TCP") != XCAM_NULL) {
            media_session->rtp_session->media_trans_mode = RTP_TRANS_TCP_ITLV;
            media_session->rtp_session->pack_type = PACK_TYPE_RTSP_ITLV;
            media_session->rtp_session->rtp_send_sock = rtsp_session->session_sockfd;
            /*get client's tcp receive channel: port1-- media data packet channel(rtp),port2-- net adapt packet channel(rtcp)*/
            temp = strstr(line1, "interleaved");
            if (XCAM_NULL == temp ) {
                media_session->rtp_session->itlv_client_media_chnid = rtsp_session->channel_id;
                rtsp_session->channel_id++;
                media_session->rtp_session->itlv_client_adapt_chnid = rtsp_session->channel_id;
                rtsp_session->channel_id++;
            } else {
                if (RTSP_SCANF_RET_TWO != sscanf(temp, "%*[^=]=%31[^-]-%31[^,]", ch_id1, ch_id2)) {
                    media_session->rtp_session->itlv_client_media_chnid = rtsp_session->channel_id;
                    rtsp_session->channel_id++;
                    media_session->rtp_session->itlv_client_adapt_chnid = rtsp_session->channel_id;
                    rtsp_session->channel_id++;
                } else {
                    ch_id1[RTSP_CHAR_MAX_LEN - 1] = '\0';
                    ch_id2[RTSP_CHAR_MAX_LEN - 1] = '\0';
                    media_session->rtp_session->itlv_client_media_chnid = (xcam_u32)atoi(ch_id1);
                    media_session->rtp_session->itlv_client_adapt_chnid = (xcam_u32)atoi(ch_id2);
                }
            }
        } else if ( (strstr(line1, "RTP/AVP") != XCAM_NULL && strstr(line1, "interleaved") == XCAM_NULL)
                || strstr(line1, "RTP/AVP/UDP") != XCAM_NULL ) {
            media_session->rtp_session->media_trans_mode = RTP_TRANS_UDP;
            media_session->rtp_session->pack_type = PACK_TYPE_RTP_FUA;

            /*get client's udp receive port: port1--recv media data packet(rtp),port2--recv net adapt packet(rtcp) */
            temp = strstr(line1, "client_port");
            if (XCAM_NULL == temp) {
                LOG_LIVESTREAM_ERROR("client_port not found in setup msg! \n");
                handle_bad_request(rtsp_session);
                return XCAM_FAILURE;
            } else {
                if (RTSP_SCANF_RET_TWO != sscanf(temp, "%*[^=]=%31[^-]-%31[^,]", ch_id1, ch_id2)) {
                    LOG_LIVESTREAM_ERROR("udp port not found in setup msg! \n");
                    handle_bad_request(rtsp_session);
                    return XCAM_FAILURE;
                } else {
                    ch_id1[RTSP_CHAR_MAX_LEN - 1] = '\0';
                    ch_id2[RTSP_CHAR_MAX_LEN - 1] = '\0';
                    media_session->rtp_session->client_rtp_port = (xcam_s32)atoi(ch_id1);
                    media_session->rtcp_session->client_rtcp_port = (xcam_s32)atoi(ch_id2);
                }
            }
        } else {
            /*not support tans type or not understand characters*/
            LOG_LIVESTREAM_ERROR("not support tans type in setup msg! \n");
            handle_bad_request(rtsp_session);
            return XCAM_FAILURE;
        }

        if ( (RTP_TRANS_UDP == media_session->rtp_session->media_trans_mode
             || RTP_TRANS_TCP_ITLV == media_session->rtp_session->media_trans_mode
             || RTP_TRANS_BROADCAST == media_session->rtp_session->media_trans_mode) ) {
            /*if parse out one of the support transtype  then stop else go on */
            LOG_LIVESTREAM_INFO("parse out the support media trans mode[%d]!\n",
                    media_session->rtp_session->media_trans_mode);
            break;
        } else if (XCAM_TRUE == last_transtype_flag) {
            /*if cant parse out one of the support transtype at the last time return failure */
            LOG_LIVESTREAM_ERROR("last time dont get the support trans type!\n");
            handle_bad_request(rtsp_session);
            return XCAM_FAILURE;
        } else {
            ptr_line1 = ptr_line2; /*search trans info in line2  at next loop*/
        }
    } while (last_transtype_flag != XCAM_TRUE);

    return XCAM_SUCCESS;
}

static xcam_u32 get_hevc_nal_unit(xcam_u8* nalu_type, const xcam_u8* buffer, xcam_u32 size)
{
    xcam_u8 nal_head = 0;
    xcam_u32 code = 0;
    xcam_u32 tmp = 0;
    xcam_u32 pos = 0;

    for (code = 0xffffffff, pos = 0; pos < 4; pos++) {
        tmp = buffer[pos];
        code = (code << 8) | tmp;
    }

    if (code != 0x00000001) {
        return 0;
    }

    nal_head = buffer[pos++];
    nal_head = nal_head >> 1;

    *nalu_type = nal_head & H265_NAL_MASK;
    for (code = 0xffffffff; pos < size; pos++) {
        tmp = buffer[pos];
        code = (code << 8) | tmp;
        if (code == 0x00000001) {
            break;                //next start code is found
        }
    }

    if (pos == size ) {           // next start code is not found, this must be the last nalu
        return size;
    } else {
        return pos - 4 + 1;
    }
}

static xcam_u32 get_avc_nal_unit(xcam_u8* nalu_type, const xcam_u8* buffer, xcam_u32 size)
{
    xcam_u8 nal_head = 0;
    xcam_u32 code = 0;
    xcam_u32 tmp = 0;
    xcam_u32 pos = 0;

    for (code = 0xffffffff, pos = 0; pos < 4; pos++) {
        tmp = buffer[pos];
        code = (code << 8) | tmp;
    }
    if (code != 0x00000001) {
        return 0;
    }
    nal_head = buffer[pos++];
    *nalu_type = nal_head & H264_NAL_MASK;
    for (code = 0xffffffff; pos < size; pos++) {
        tmp = buffer[pos];
        if ((code = (code << 8) | tmp) == 0x00000001) {
            break;  // next start code is found
        }
    }

    if (pos == size) { // next start code is not found, this must be the last nalu
        return size;
    } else {
        return pos - 4 + 1;
    }
}

static xcam_s32 parse_avc_key_frame(livestream_rtsp_stream_session* rtsp_session,
                    const xcam_u8* frame_buf, xcam_u32 frame_len, xcam_bool* is_idr_found)
{
    xcam_u32 cursor = 0;

    // get first I frame and parse sps pps
    do {
        xcam_u8 nal_type = 0;
        xcam_u32 nal_len;

        nal_len = get_avc_nal_unit(&nal_type, frame_buf + cursor, frame_len - cursor);
        if (nal_len <= 4 || cursor + nal_len > frame_len) {
            LOG_LIVESTREAM_ERROR("nal_len illegal:%d!\n", nal_len);
            return XCAM_FAILURE;
        }

        if (AVC_SPS == nal_type) {
            LOG_LIVESTREAM_INFO("Find nal type sps.\n");
            if (nal_len - 4 > MAX_NAL_PARAM_LEN) {
                LOG_LIVESTREAM_ERROR("nal_len reach max: %d!\n", nal_len);
                return XCAM_FAILURE;
            }
            if (!rtsp_session->is_get_key_frame) {
                rtsp_session->media_info.sps_buf.data_len = nal_len - 4;
                memcpy(rtsp_session->media_info.sps_buf.data_buf, frame_buf + cursor + 4, nal_len - 4);
            }
        } else if (AVC_PPS == nal_type) {
            if (nal_len - 4 > MAX_NAL_PARAM_LEN) {
                LOG_LIVESTREAM_ERROR("nal_len reach max: %d!\n", nal_len);
                return XCAM_FAILURE;
            }
            if (!rtsp_session->is_get_key_frame) {
                rtsp_session->media_info.pps_buf.data_len = nal_len - 4;
                memcpy(rtsp_session->media_info.pps_buf.data_buf, frame_buf + cursor + 4, nal_len - 4);
            }
        }

        if ( AVC_IDR == nal_type || AVC_SPS == nal_type) {
            *is_idr_found = XCAM_TRUE;
        }
        cursor += nal_len;
    } while (cursor + 4 < frame_len);

    return XCAM_SUCCESS;
}

static xcam_s32  parse_hevc_key_frame(livestream_rtsp_stream_session* rtsp_session,
                        xcam_u8* frame_buf, xcam_u32 frame_len, xcam_bool* is_idr_found)
{
    xcam_u32 cursor = 0;

    do {
        xcam_u8 nal_type = 0;
        xcam_u32 nal_len = get_hevc_nal_unit(&nal_type, frame_buf + cursor, frame_len - cursor);
        if (nal_len <= 4 || cursor + nal_len  > frame_len) {
            LOG_LIVESTREAM_ERROR("nal_len illegal: %d \n", nal_len);
            return XCAM_FAILURE;
        }

        if (HEVC_SPS == nal_type) {
            if (nal_len - 4 > MAX_NAL_PARAM_LEN) {
                LOG_LIVESTREAM_ERROR("nal_len reach max: %d!\n", nal_len);
                return XCAM_FAILURE;
            }
            if (!rtsp_session->is_get_key_frame) {
                rtsp_session->media_info.sps_buf.data_len = nal_len - 4;
                memcpy(rtsp_session->media_info.sps_buf.data_buf, frame_buf + cursor + 4, nal_len - 4);
            }
        } else if (HEVC_PPS == nal_type) {
            if (nal_len - 4 > MAX_NAL_PARAM_LEN) {
                LOG_LIVESTREAM_ERROR("nal_len reach max: %d!\n", nal_len);
                return XCAM_FAILURE;
            }
            if (!rtsp_session->is_get_key_frame) {
                rtsp_session->media_info.pps_buf.data_len = nal_len - 4;
                memcpy(rtsp_session->media_info.pps_buf.data_buf, frame_buf + cursor + 4, nal_len - 4);
            }
        } else if (HEVC_VPS == nal_type) {
            if (nal_len - 4 > MAX_NAL_PARAM_LEN) {
                LOG_LIVESTREAM_ERROR("nal_len reach max: %d!\n",nal_len);
                return XCAM_FAILURE;
            }
            if (!rtsp_session->is_get_key_frame) {
                rtsp_session->media_info.vps_buf.data_len = nal_len - 4;
                memcpy(rtsp_session->media_info.vps_buf.data_buf, frame_buf + cursor + 4, nal_len - 4);
            }
        }
        if (HEVC_IDR == nal_type || HEVC_VPS == nal_type) {
            *is_idr_found = XCAM_TRUE;
        }

        cursor += nal_len;
    } while (cursor + 4 < frame_len);

    return XCAM_SUCCESS;
}

static xcam_s32 check_is_get_key_frame(const livestream_rtsp_stream_session* rtsp_session)
{
    xcam_u32 wait_count = 0;

    while (!rtsp_session->is_get_key_frame) {
        //wait for the i frame come and get sps pps
        usleep(CHECK_WAIT_TIMEOUT);
        wait_count++;
        if (wait_count > CHECK_WAIT_I_FRAME_COUNT) {
            return XCAM_FAILURE;
        }
    }

    return XCAM_SUCCESS;
}

static xcam_s32 generate_sdp_lines(const livestream_rtsp_stream_session* rtsp_session,
            xcam_char* sdpbuf, xcam_u32 sdpbuf_len)
{
    xcam_char base_sps_outInfo[RTSP_MAX_NALBASE_LEN] = {0};
    xcam_char base_pps_outInfo[RTSP_MAX_NALBASE_LEN] = {0};
    xcam_char base_vps_outInfo[RTSP_MAX_NALBASE_LEN] = {0};
    xcam_char interop_cons[RTSP_MAX_NALBASE_LEN] = {0};
    xcam_u32  profile_space = 0;
    xcam_u32  profile_id = 0;
    xcam_u32  tier_flag = 0;
    xcam_u32  level_id = 0;
    xcam_s32  tmp_len = 0;
    xcam_u32  msg_len = 0;
    xcam_s32  ret = 0;

    RTSPSVR_CHECK_NULL_ERROR(rtsp_session);
    ret = snprintf(sdpbuf, sdpbuf_len - 1,
                "v=0\r\n"
                "o=StreamingServer 3331435948 1116907222000 IN IP4 %s\r\n"
                "s=%s\r\n"
                "c=IN IP4 0.0.0.0\r\n"
                "t=0 0\r\n"
                "a=control:*\r\n"
                "a=range:npt=0-\r\n",
                rtsp_session->host_ip,
                rtsp_session->stream_name);
    if (ret < 0) {
        LOG_LIVESTREAM_ERROR("string printf sdp buffer error!\n");
        return XCAM_FAILURE;
    }

    msg_len += ret;
    if ( msg_len >= sdpbuf_len) {
        LOG_LIVESTREAM_ERROR("sdp buffer len is too short!\n");
        return XCAM_FAILURE;
    }

    if (rtsp_session->media_info.video_enable) {
        xcam_track_video_codec video_codec = rtsp_session->media_info.video_source_info.codec_type;
        if (XCAM_TRACK_VIDEO_CODEC_H264 == video_codec) {
            if (XCAM_SUCCESS != check_is_get_key_frame(rtsp_session)) {
                LOG_LIVESTREAM_ERROR("H264 get sps pps failed, i frame not come yet!\n");
                return XCAM_FAILURE;
            }
            xcam_u32 temp_id1 = rtsp_session->media_info.sps_buf.data_buf[1] << 16;
            xcam_u32 temp_id2 = rtsp_session->media_info.sps_buf.data_buf[2] << 8;
            xcam_u32 temp_id3 = rtsp_session->media_info.sps_buf.data_buf[3];
            level_id = temp_id1 | temp_id2 | temp_id3;
            livestream_comm_base64_encode((const xcam_u8*)rtsp_session->media_info.sps_buf.data_buf,
                rtsp_session->media_info.sps_buf.data_len, (xcam_u8*)base_sps_outInfo, tmp_len);
            livestream_comm_base64_encode((const xcam_u8*)rtsp_session->media_info.pps_buf.data_buf,
                rtsp_session->media_info.pps_buf.data_len, (xcam_u8*)base_pps_outInfo, tmp_len);
            ret = snprintf(sdpbuf + msg_len, sdpbuf_len - msg_len - 1,
                                "m=video 0 RTP/AVP %d\r\n"
                                "a=framerate:%.1f\r\n"
                                "a=transform:1,0,0;0,1,0;0,0,1\r\n"
                                "a=control:trackID=%d\r\n"
                                "a=rtpmap:%d H264/%d\r\n"
                                "a=fmtp:%d packetization-mode=1;profile-level-id=%06X;sprop-parameter-sets=%s,%s\r\n"
                                "a=x-dimensions:%d,%d\r\n",
                                RTP_PT_H264,
                                rtsp_session->media_info.video_source_info.frame_rate,
                                RTSP_TRACKID_VIDEO,
                                RTP_PT_H264,
                                RTSP_CLOCK_RATE_90K,
                                RTP_PT_H264,
                                level_id,
                                base_sps_outInfo,
                                base_pps_outInfo,
                                rtsp_session->media_info.video_source_info.width,
                                rtsp_session->media_info.video_source_info.height);
            if (ret < 0) {
                LOG_LIVESTREAM_ERROR("string printf sdp buffer error!\n");
                return XCAM_FAILURE;
            }
            msg_len += ret;
        } else if (XCAM_TRACK_VIDEO_CODEC_H265 == video_codec) {
            if (XCAM_SUCCESS != check_is_get_key_frame(rtsp_session)) {
                LOG_LIVESTREAM_ERROR("H265 get sps pps failed, i frame not come yet!\n");
                return XCAM_FAILURE;
            }

            profile_space = rtsp_session->media_info.vps_buf.data_buf[6] >> 6;
            profile_id = rtsp_session->media_info.vps_buf.data_buf[6] & 0x1F;
            tier_flag = (rtsp_session->media_info.vps_buf.data_buf[6] >> 5) & 0x1;
            level_id = rtsp_session->media_info.vps_buf.data_buf[17];
            ret = snprintf(interop_cons, RTSP_MAX_NALBASE_LEN - 1, "%02X%02X%02X%02X%02X%02X",
                                 rtsp_session->media_info.vps_buf.data_buf[11], rtsp_session->media_info.vps_buf.data_buf[12],
                                 rtsp_session->media_info.vps_buf.data_buf[13], rtsp_session->media_info.vps_buf.data_buf[14],
                                 rtsp_session->media_info.vps_buf.data_buf[15], rtsp_session->media_info.vps_buf.data_buf[16]);
            if (ret < 0) {
                LOG_LIVESTREAM_ERROR("string printf interop_cons error\n");
                return XCAM_FAILURE;
            }
            livestream_comm_base64_encode((const xcam_u8*)rtsp_session->media_info.sps_buf.data_buf,
                rtsp_session->media_info.sps_buf.data_len, ( xcam_u8*)base_sps_outInfo, tmp_len);
            livestream_comm_base64_encode((const xcam_u8*)rtsp_session->media_info.pps_buf.data_buf,
                rtsp_session->media_info.pps_buf.data_len, ( xcam_u8*)base_pps_outInfo, tmp_len);
            livestream_comm_base64_encode((const xcam_u8*)rtsp_session->media_info.vps_buf.data_buf,
                rtsp_session->media_info.vps_buf.data_len, ( xcam_u8*)base_vps_outInfo, tmp_len);
            ret = snprintf(sdpbuf + msg_len, sdpbuf_len - msg_len - 1,
                                "m=video 0 RTP/AVP %d\r\n"
                                "a=framerate:%.1f\r\n"
                                "a=transform:1,0,0;0,1,0;0,0,1\r\n"
                                "a=control:trackID=%d\r\n"
                                "a=rtpmap:%d H265/%d\r\n"
                                "a=fmtp:%d profile-space=%u;profile-id=%u;tier-flag=%u;level-id=%u;interop-constraints=%s;"
                                "sprop-vps=%s;sprop-sps=%s;sprop-pps=%s\r\n"
                                "a=x-dimensions:%d,%d\r\n",
                                RTP_PT_H265,
                                rtsp_session->media_info.video_source_info.frame_rate,
                                RTSP_TRACKID_VIDEO,
                                RTP_PT_H265,
                                RTSP_CLOCK_RATE_90K,
                                RTP_PT_H265,
                                profile_space,
                                profile_id,
                                tier_flag,
                                level_id,
                                interop_cons,
                                base_vps_outInfo,
                                base_sps_outInfo,
                                base_pps_outInfo,
                                rtsp_session->media_info.video_source_info.width,
                                rtsp_session->media_info.video_source_info.height);
            if (ret < 0) {
                LOG_LIVESTREAM_ERROR("string printf sdpbuf error\n");
                return XCAM_FAILURE;
            }
            msg_len += ret;
        } else {
            LOG_LIVESTREAM_ERROR("video codec[%d] not supported\n", video_codec);
            return XCAM_FAILURE;
        }

        if (msg_len >= sdpbuf_len) {
            LOG_LIVESTREAM_ERROR("sdp buffer len is too short\n");
            return XCAM_FAILURE;
        }
    }

    if (rtsp_session->media_info.audio_enable) {
        xcam_track_audio_codec audio_codec = rtsp_session->media_info.audio_source_info.codec_type;
        if (XCAM_TRACK_AUDIO_CODEC_G726 == audio_codec) {
            ret = snprintf(sdpbuf + msg_len, sdpbuf_len - msg_len - 1,
                                "m=audio 0 RTP/AVP %d\r\n"
                                "a=control:trackID=%d\r\n"
                                "a=rtpmap:%d G726-%d/%d/%d\r\n",
                                RTP_PT_G726, RTSP_TRACKID_AUDIO, RTP_PT_G726,
                                rtsp_session->media_info.audio_source_info.avg_bytes_per_sec,
                                RTSP_CLOCK_RATE_8K,
                                rtsp_session->media_info.audio_source_info.chn_cnt);
            if (ret < 0) {
                LOG_LIVESTREAM_ERROR("string printf sdpbuf error\n");
                return XCAM_FAILURE;
            }
            msg_len += ret;
        } else if (XCAM_TRACK_AUDIO_CODEC_G711A == audio_codec) {
            ret =  snprintf(sdpbuf + msg_len, sdpbuf_len - msg_len - 1,
                                 "m=audio 0 RTP/AVP %d\r\n"
                                 "a=control:trackID=%d\r\n"
                                 "a=rtpmap:%d PCMA/%d/%d\r\n",
                                 RTP_PT_PCMA, RTSP_TRACKID_AUDIO, RTP_PT_PCMA,
                                 RTSP_CLOCK_RATE_8K,
                                 rtsp_session->media_info.audio_source_info.chn_cnt);
            if (ret < 0) {
                LOG_LIVESTREAM_ERROR("string printf sdpbuf error\n");
                return XCAM_FAILURE;
            }
            msg_len += ret;
        } else if (XCAM_TRACK_AUDIO_CODEC_G711U == audio_codec) {
            ret =  snprintf(sdpbuf + msg_len, sdpbuf_len - msg_len - 1,
                                 "m=audio 0 RTP/AVP %d\r\n"
                                 "a=control:trackID=%d\r\n"
                                 "a=rtpmap:%d PCMU/%d/%d\r\n",
                                 RTP_PT_PCMU, RTSP_TRACKID_AUDIO, RTP_PT_PCMU,
                                 RTSP_CLOCK_RATE_8K,
                                 rtsp_session->media_info.audio_source_info.chn_cnt);
            if (ret < 0) {
                LOG_LIVESTREAM_ERROR("string printf sdpbuf error\n");
                return XCAM_FAILURE;
            }
            msg_len += ret;
        } else if (XCAM_TRACK_AUDIO_CODEC_AAC == audio_codec) {
            xcam_u32 config_id = livestream_rtsp_message_get_aac_config(
                            rtsp_session->media_info.audio_source_info.sample_rate,
                            rtsp_session->media_info.audio_source_info.chn_cnt);

            LOG_LIVESTREAM_INFO("config_id: %u\n", config_id);
            ret = snprintf(sdpbuf + msg_len, sdpbuf_len - msg_len - 1,
                                "m=audio 0 RTP/AVP %d\r\n"
                                "a=control:trackID=%d\r\n"
                                "a=rtpmap:%d MPEG4-GENERIC/%d/%d\r\n"
                                "a=fmtp:%d streamtype=5;profile-level-id=1;mode=AAC-hbr;"
                                "sizelength=13;indexlength=3;indexdeltalength=3;config=%d\r\n",
                                RTP_PT_AAC, RTSP_TRACKID_AUDIO, RTP_PT_AAC,
                                RTSP_CLOCK_RATE_90K,
                                rtsp_session->media_info.audio_source_info.chn_cnt,
                                RTP_PT_AAC,
                                config_id);
            if (ret < 0) {
                LOG_LIVESTREAM_ERROR("string printf sdpbuf error\n");
                return XCAM_FAILURE;
            }
            msg_len += ret;
        } else if (audio_codec == XCAM_TRACK_AUDIO_CODEC_MP3) {
            ret = snprintf(sdpbuf + msg_len, sdpbuf_len - msg_len - 1,
                "m=audio 0 RTP/AVP %d\r\n"
                "a=control:trackID=%d\r\n"
                "a=rtpmap:%d MPA/%d/%d\r\n",
                RTP_PT_MPA, RTSP_TRACKID_AUDIO, RTP_PT_MPA,
                RTSP_CLOCK_RATE_90K,
                rtsp_session->media_info.audio_source_info.chn_cnt);
            if (ret < 0) {
                LOG_LIVESTREAM_ERROR("string printf sdpbuf error!\n");
                return XCAM_FAILURE;
            }
            msg_len += ret;
        } else {
            LOG_LIVESTREAM_ERROR("audio codec[%d] not supported!\n", audio_codec);
            return XCAM_FAILURE;
        }
    }

    return XCAM_SUCCESS;
}

static xcam_s32 generate_setup_response(livestream_rtsp_stream_session* rtsp_session,
            livestream_rtsp_media_session* media_session, xcam_char* resp_buffer, xcam_u32 buf_len)
{
    xcam_u32 msg_len = 0;
    xcam_s32 ret = 0;

    RTSPSVR_CHECK_NULL_ERROR(rtsp_session);
    RTSPSVR_CHECK_NULL_ERROR(media_session);
    RTSPSVR_CHECK_NULL_ERROR(media_session->rtcp_session);
    if (RTP_TRANS_UDP == media_session->rtp_session->media_trans_mode) {
        ret = snprintf(resp_buffer + msg_len, buf_len - msg_len - 1,
                "Transport: RTP/AVP;unicast;client_port=%d-%d;server_port=%d-%d;ssrc=%08x;mode=\"PLAY\"\r\n",
                media_session->rtp_session->client_rtp_port,
                media_session->rtcp_session->client_rtcp_port,
                media_session->rtp_session->server_rtp_port,
                media_session->rtcp_session->server_rtcp_port,
                media_session->rtp_session->ssrc);
        if (ret < 0) {
            LOG_LIVESTREAM_ERROR("string printf resp_buffer error!\n");
            return XCAM_FAILURE;
        }
        msg_len += ret;
    } else if (RTP_TRANS_TCP_ITLV == media_session->rtp_session->media_trans_mode) {
        ret =  snprintf(resp_buffer + msg_len, buf_len - msg_len - 1,
                "Transport: RTP/AVP/TCP;unicast;interleaved=%d-%d;ssrc=%08x;mode=\"PLAY\"\r\n",
                media_session->rtp_session->itlv_client_media_chnid,
                media_session->rtp_session->itlv_client_adapt_chnid,
                media_session->rtp_session->ssrc);
        if (ret < 0) {
            LOG_LIVESTREAM_ERROR("string printf resp_buffer error!\n");
            return XCAM_FAILURE;
        }
        msg_len += ret;
    } else {
        handle_bad_request(rtsp_session);
        return XCAM_FAILURE;
    }

    return XCAM_SUCCESS;
}

static xcam_s32 generate_play_response(livestream_rtsp_stream_session* rtsp_session,
        xcam_char* resp_buffer, xcam_u32 buf_len)
{
    xcam_u32 msg_len = 0;
    xcam_s32 ret = 0;

    RTSPSVR_CHECK_NULL_ERROR(rtsp_session);
    ret = snprintf(resp_buffer + msg_len, buf_len - msg_len - 1, "Range: %s\r\n", "npt=0.000-");
    if (ret < 0) {
        LOG_LIVESTREAM_ERROR("string printf resp_buffer error!\n");
        return XCAM_FAILURE;
    }

    msg_len += ret;
    ret = snprintf(resp_buffer + msg_len, buf_len  - msg_len - 1, "RTP-Info: ");
    if (ret < 0) {
        LOG_LIVESTREAM_ERROR("string printf resp_buffer error!\n");
        return XCAM_FAILURE;
    }

    msg_len += ret;
    if (rtsp_session->video_session.track_id != RTSP_INVALID_TRACK_ID) {
        rtsp_session->video_session.rtp_session->seq_num = 0;
        ret = snprintf(resp_buffer + msg_len, buf_len - msg_len - 1, "url=%s;seq=%u;rtptime=%u,",
                            rtsp_session->video_session.rtsp_url,
                            rtsp_session->video_session.rtp_session->seq_num, 0);
        if (ret < 0) {
            LOG_LIVESTREAM_ERROR("string printf resp_buffer error!\n");
            return XCAM_FAILURE;
        }
        msg_len += ret;
    }

    if (rtsp_session->audio_session.track_id != RTSP_INVALID_TRACK_ID) {
        RTSPSVR_CHECK_NULL_ERROR(rtsp_session->audio_session.rtp_session);
        rtsp_session->audio_session.rtp_session->seq_num = 0;
        ret = snprintf(resp_buffer + msg_len, buf_len - msg_len - 1, "url=%s;seq=%u;rtptime=%u",
                            rtsp_session->audio_session.rtsp_url,
                            rtsp_session->audio_session.rtp_session->seq_num, 0);
        if (ret < 0) {
            LOG_LIVESTREAM_ERROR("string printf resp_buffer error!\n");
            return XCAM_FAILURE;
        }

        msg_len += ret;
    }

    ret = snprintf(resp_buffer + msg_len, buf_len - msg_len - 1, "\r\n");
    if (ret < 0) {
        LOG_LIVESTREAM_ERROR("string printf resp_buffer error!\n");
        return XCAM_FAILURE;
    }
    msg_len += ret;

    return XCAM_SUCCESS;
}

static xcam_s32 reduce_count(livestream_rtsp_stream_session* rtsp_session,
                    xcam_track_source_handle track_src, xcam_u32* out_count)
{
    livestream_media_stream_node* stream_node = XCAM_NULL;
    livestream_rtsp_server_ctx* server_ctx = XCAM_NULL;
    xcam_bool is_found_stream = XCAM_FALSE;
    struct list_head* pos_node = XCAM_NULL;
    struct list_head* tmp_node = XCAM_NULL;

    //get stream node, and cal start num
    server_ctx = (livestream_rtsp_server_ctx*)rtsp_session->rtspsvr_handle;
    pthread_mutex_lock(&server_ctx->stream_list_lock);
    list_for_each_safe(pos_node, tmp_node, &server_ctx->stream_list) {
        stream_node = list_entry(pos_node, livestream_media_stream_node, list_ptr);
        if (XCAM_TRACK_SOURCE_TYPE_VIDEO == track_src->track_type) {
            if (stream_node->video_stream == track_src
                    && !strcasecmp(stream_node->name, rtsp_session->stream_name)) {
                is_found_stream = XCAM_TRUE;
                break;
            }
        } else if (XCAM_TRACK_SOURCE_TYPE_AUDIO == track_src->track_type) {
            if (stream_node->audio_stream == track_src
                    && !strcasecmp(stream_node->name, rtsp_session->stream_name)) {
                is_found_stream = XCAM_TRUE;
                break;
            }
        }
    }

    if (is_found_stream) {
        if (XCAM_TRACK_SOURCE_TYPE_VIDEO == track_src->track_type) {
            if (0 == stream_node->video_start_cnt) {
                LOG_LIVESTREAM_ERROR("Video already stop, do not stop again!\n");
                pthread_mutex_unlock(&server_ctx->stream_list_lock);
                return XCAM_FAILURE;
            }
            stream_node->video_start_cnt--;
            *out_count = stream_node->video_start_cnt;
        } else if (XCAM_TRACK_SOURCE_TYPE_AUDIO == track_src->track_type) {
            if (0 == stream_node->audio_start_cnt) {
                LOG_LIVESTREAM_ERROR("Audio already stop, do not stop again!\n");
                pthread_mutex_unlock(&server_ctx->stream_list_lock);
                return XCAM_FAILURE;
            }
            stream_node->audio_start_cnt--;
            *out_count = stream_node->audio_start_cnt;
        }
    } else {
        LOG_LIVESTREAM_ERROR("Session video stream not found!\n");
        pthread_mutex_unlock(&server_ctx->stream_list_lock);
        return XCAM_FAILURE;
    }
    pthread_mutex_unlock(&server_ctx->stream_list_lock);

    return XCAM_SUCCESS;
}

static xcam_s32 add_count(livestream_rtsp_stream_session* rtsp_session,
                    xcam_track_source_handle track_src, xcam_u32* count)
{
    livestream_media_stream_node* stream_node = XCAM_NULL;
    livestream_rtsp_server_ctx* server_ctx = XCAM_NULL;
    struct list_head* tmp_node = XCAM_NULL;
    struct list_head* pos_node = XCAM_NULL;
    xcam_bool is_found_stream = XCAM_FALSE;

    //get stream node, and cal start num
    server_ctx = (livestream_rtsp_server_ctx*)rtsp_session->rtspsvr_handle;
    pthread_mutex_lock(&server_ctx->stream_list_lock);
    list_for_each_safe(pos_node, tmp_node, &server_ctx->stream_list) {
        stream_node = list_entry(pos_node, livestream_media_stream_node, list_ptr);
        if (XCAM_TRACK_SOURCE_TYPE_VIDEO == track_src->track_type) {
            if (stream_node->video_stream == track_src
                && !strcasecmp(stream_node->name, rtsp_session->stream_name)) {
                is_found_stream = XCAM_TRUE;
                break;
            }
        } else if (XCAM_TRACK_SOURCE_TYPE_AUDIO == track_src->track_type) {
            if (stream_node->audio_stream == track_src
                && !strcasecmp(stream_node->name, rtsp_session->stream_name)) {
                is_found_stream = XCAM_TRUE;
                break;
            }
        }
    }
    if (is_found_stream) {
        if (XCAM_TRACK_SOURCE_TYPE_VIDEO == track_src->track_type) {
            stream_node->video_start_cnt++;
            *count = stream_node->video_start_cnt;
        } else if (XCAM_TRACK_SOURCE_TYPE_AUDIO == track_src->track_type) {
            stream_node->audio_start_cnt++;
            *count = stream_node->audio_start_cnt;
        }
    } else {
        LOG_LIVESTREAM_ERROR("Session video stream not found\n");
        pthread_mutex_unlock(&server_ctx->stream_list_lock);
        return XCAM_FAILURE;
    }

    pthread_mutex_unlock(&server_ctx->stream_list_lock);
    return XCAM_SUCCESS;
}

static xcam_s32 remove_video_source(livestream_rtsp_stream_session* rtsp_session,
        xcam_track_source_handle track_src)
{
    xcam_s32 ret = XCAM_SUCCESS;
    xcam_u32 count = 0;

    RTSPSVR_CHECK_NULL_ERROR(rtsp_session);
    RTSPSVR_CHECK_NULL_ERROR(track_src);
    //get stream node, and cal start num
    ret = reduce_count(rtsp_session, track_src, &count);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("Session video reduce count failed ret:0x%x\n", ret);
        return ret;
    }

    return ret;
}

static xcam_s32 add_video_source(livestream_rtsp_stream_session* rtsp_session,
                        xcam_track_source_handle track_src)
{
    xcam_s32 ret = XCAM_SUCCESS;
    xcam_u32 count = 0;

    RTSPSVR_CHECK_NULL_ERROR(rtsp_session);
    RTSPSVR_CHECK_NULL_ERROR(track_src);

    ret = add_count(rtsp_session, track_src, &count);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("Session video add count failed ret:0x%x!\n", ret);
        return ret;
    }

    ret = track_src->func_request_key_frame(track_src->private_handle, rtsp_session->rtspsvr_handle);
    if (ret != XCAM_SUCCESS) {
        LOG_LIVESTREAM_ERROR("Session request KeyFrame failed ret:0x%x!\n", ret);
        return XCAM_FAILURE;
    }

    return XCAM_SUCCESS;
}

static xcam_s32 remove_audio_source(livestream_rtsp_stream_session* rtsp_session,
                    xcam_track_source_handle track_src)
{
    xcam_s32 ret = XCAM_SUCCESS;
    xcam_u32 count = 0;

    RTSPSVR_CHECK_NULL_ERROR(rtsp_session);
    RTSPSVR_CHECK_NULL_ERROR(track_src);
    //get stream node, and cal start num
    ret = reduce_count(rtsp_session, track_src, &count);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("Session audio reduce count failed ret: %d!\n", ret);
        return ret;
    }

    return ret;
}

static xcam_s32 add_audio_source(livestream_rtsp_stream_session* rtsp_session,
                    xcam_track_source_handle track_src)
{
    xcam_s32 ret = XCAM_SUCCESS;
    xcam_u32 count = 0;

    RTSPSVR_CHECK_NULL_ERROR(rtsp_session);
    RTSPSVR_CHECK_NULL_ERROR(track_src);
    //get stream node, and cal start num
    ret = add_count(rtsp_session, track_src, &count);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("Session audio add count failed ret:0x%x\n", ret);
        return ret;
    }

    return XCAM_SUCCESS;
}

static xcam_void handle_options(livestream_rtsp_stream_session* rtsp_session)
{
    xcam_char status_str[MAX_DATE_LEN] = {0};
    xcam_char buffer[MAX_DATE_LEN] = {0};
    int ret;

    memset(status_str, '\0', sizeof(status_str));
    livestream_rtsp_message_status_code_to_str(RTSP_STATUS_CODE_OK,
        status_str, sizeof(status_str)-1);
    memset(buffer, '\0', sizeof(buffer));
    livestream_comm_format_date(buffer, sizeof(buffer)-1);
    ret = snprintf(rtsp_session->response_buf, sizeof(rtsp_session->response_buf) - 1,
                "%s %d %s\r\n"
                "CSeq: %d\r\n"
                "Cache-Control: no-cache\r\n"
                "Server: %s\r\n"
                "%sPublic: %s\r\n\r\n",
                RTSP_VER_STR,
                RTSP_STATUS_CODE_OK,
                status_str,
                rtsp_session->cur_request_seq,
                RTSP_SERVER_DESCRIPTION,
                buffer,
                RTSP_SUPPORTED_CMD_LIST);
    if (ret < 0) {
        LOG_LIVESTREAM_ERROR("string printf pRespBuffer error!\n");
        return;
    }

    return;
}

static xcam_s32 get_stream(livestream_rtsp_stream_session* rtsp_session, const xcam_char* request)
{
    livestream_media_stream_node* stream_node = XCAM_NULL;
    livestream_rtsp_server_ctx* server_ctx = XCAM_NULL;
    struct list_head* pos_node = XCAM_NULL;
    xcam_bool is_found = XCAM_FALSE;
    xcam_s32 ret = 0;
    xcam_char stream_name[RTSP_MAX_STREAMNAME_LEN] = {0};

    ret = livestream_rtsp_message_get_stream_name(request, stream_name, RTSP_MAX_STREAMNAME_LEN);
    if (ret != XCAM_SUCCESS) {
        LOG_LIVESTREAM_ERROR("invalid rtsp url, could not find stream name!\n");
        return XCAM_FAILURE;
    }

    //get stream node
    server_ctx = (livestream_rtsp_server_ctx*)rtsp_session->rtspsvr_handle;
    pthread_mutex_lock(&server_ctx->stream_list_lock);
    list_for_each(pos_node, &server_ctx->stream_list) {
        stream_node = list_entry(pos_node, livestream_media_stream_node, list_ptr);
        if (!strcasecmp(stream_node->name, stream_name)) {
            is_found = XCAM_TRUE;
            break;
        }
    }

    if (!is_found || XCAM_NULL == stream_node) {
        LOG_LIVESTREAM_ERROR("invalid rtsp request, could not find correspond stream\n");
        pthread_mutex_unlock(&server_ctx->stream_list_lock);
        return XCAM_FAILURE;
    }
    if (stream_node->video_stream == XCAM_NULL && stream_node->audio_stream == XCAM_NULL) {
        return XCAM_FAILURE;
    }
    //set stream
    rtsp_session->buf_size = stream_node->buf_size;
    livestream_rtsp_session_set_media_source(rtsp_session, stream_node->video_stream,
            stream_node->audio_stream, stream_name);
    pthread_mutex_unlock(&server_ctx->stream_list_lock);

    return XCAM_SUCCESS;
}

static xcam_s32 handle_describe(livestream_rtsp_stream_session* rtsp_session, const xcam_char* request)
{
    xcam_track_source_handle video_stream = XCAM_NULL;
    xcam_track_source_handle audio_stream = XCAM_NULL;
    xcam_char sdp_msg[RTSP_MAX_SDP_LEN] = {0};
    xcam_s32 resp_len = 0;
    xcam_s32 ret = 0;

    // if not get streamname yet
    if (XCAM_NULL == rtsp_session->video_stream && XCAM_NULL == rtsp_session->audio_stream) {
        ret = get_stream(rtsp_session, request);
        if (ret != XCAM_SUCCESS) {
            LOG_LIVESTREAM_ERROR("Get rtsp video or audio stream failed!\n");
            handle_bad_request(rtsp_session);
            return XCAM_FAILURE;
        }
    }

    video_stream = rtsp_session->video_stream;
    audio_stream = rtsp_session->audio_stream;

    if (strcasestr(request, RTSP_HEADER_ACCEPT) != XCAM_NULL
        && strcasestr(request, RTSP_SDP_CONTENT_TYPE) == XCAM_NULL) {
        LOG_LIVESTREAM_ERROR("describe handle Only accept %s.\n", RTSP_SDP_CONTENT_TYPE);
        handle_option_not_support(rtsp_session);
        return XCAM_FAILURE;
    }

    if (!video_stream && !audio_stream) {
        LOG_LIVESTREAM_ERROR("session vid &aud  source null, not exist \n");
        handle_stream_not_found(rtsp_session);
        return XCAM_FAILURE;
    }

    if (XCAM_NULL == rtsp_session->media_info.mbuf_handle) {
        ret = livestream_mbuffer_create(&(rtsp_session->media_info.mbuf_info), &rtsp_session->media_info.mbuf_handle,
                    rtsp_session->buf_size, rtsp_session->max_payload);
        if (ret != XCAM_SUCCESS) {
            handle_server_error(rtsp_session);
            return ret;
        }
    }

    if (video_stream && !rtsp_session->media_info.video_enable) {
        memcpy(&rtsp_session->media_info.video_source_info,
                &video_stream->track_source_attr.video_info,
                sizeof(xcam_track_video_source_info));
        ret = livestream_mbuffer_register(rtsp_session->media_info.mbuf_handle,
                        rtsp_session->media_info.video_source_info.codec_type);
        if (ret != XCAM_SUCCESS) {
            handle_server_error(rtsp_session);
            LOG_LIVESTREAM_ERROR("livestream mbuffer register video error!\n");
            goto MBUF_DESTROY;
        }

        LOG_LIVESTREAM_INFO("livestream mbuffer register video type:%d success.\n",
                rtsp_session->media_info.video_source_info.codec_type);
        ret = add_video_source(rtsp_session, video_stream);
        if (ret != XCAM_SUCCESS) {
            LOG_LIVESTREAM_ERROR("add video source failed!\n");
            handle_server_error(rtsp_session);
            goto VIDEO_UNREG;
        }
        rtsp_session->media_info.video_enable = XCAM_TRUE;
    }

    if (audio_stream && !rtsp_session->media_info.audio_enable) {
        memcpy(&rtsp_session->media_info.audio_source_info,
                &audio_stream->track_source_attr.audio_info,
                sizeof(xcam_track_audio_source_info));
        ret = livestream_mbuffer_register(rtsp_session->media_info.mbuf_handle,
                    rtsp_session->media_info.audio_source_info.codec_type);
        if (ret != XCAM_SUCCESS) {
            handle_server_error(rtsp_session);
            LOG_LIVESTREAM_ERROR("livestream mbuffer register audio error\n");
            goto VIDEO_REMOVE;
        }
        LOG_LIVESTREAM_INFO("livestream mbuffer register audio type:%d success.\n",
                rtsp_session->media_info.audio_source_info.codec_type);
        ret = add_audio_source(rtsp_session, audio_stream);
        if (ret != XCAM_SUCCESS) {
            handle_server_error(rtsp_session);
            goto AUDIO_UNREG;
        }
        rtsp_session->media_info.audio_enable = XCAM_TRUE;
    }

    ret = generate_sdp_lines(rtsp_session, sdp_msg, RTSP_MAX_SDP_LEN);
    resp_len = strlen(sdp_msg);
    if (ret != XCAM_SUCCESS || resp_len <= 0) {
        handle_server_error(rtsp_session);
        goto AUDIO_REMOVE;
    }

    xcam_char status_str[MAX_DATE_LEN] = {0};
    xcam_char buffer[MAX_DATE_LEN] = {0};
    memset(status_str, '\0', sizeof(status_str));
    livestream_rtsp_message_status_code_to_str(RTSP_STATUS_CODE_OK,
        status_str, sizeof(status_str)-1);
    memset(buffer, '\0', sizeof(buffer));
    livestream_comm_format_date(buffer, sizeof(buffer)-1);
    int retval = snprintf(rtsp_session->response_buf, sizeof(rtsp_session->response_buf) - 1,
                        "%s %d %s\r\n"
                        "CSeq: %d\r\n"
                        "Content-Type: application/sdp\r\n"
                        "Cache-Control: no-cache\r\n"
                        "Server: %s\r\n"
                        "Content-Length: %d\r\n"
                        "%s\r\n"
                        "%s",
                        RTSP_VER_STR,
                        RTSP_STATUS_CODE_OK,
                        status_str,
                        rtsp_session->cur_request_seq,
                        RTSP_SERVER_DESCRIPTION,
                        resp_len,
                        buffer,
                        sdp_msg
                        );
    if (retval < 0) {
        LOG_LIVESTREAM_ERROR("string printf response buf error\n");
        ret = XCAM_FAILURE;
        goto AUDIO_REMOVE;
    }
    return XCAM_SUCCESS;

AUDIO_REMOVE:
    if (rtsp_session->media_info.audio_enable && audio_stream) {
        if (XCAM_SUCCESS != remove_audio_source(rtsp_session, audio_stream)) {
            LOG_LIVESTREAM_ERROR("remove audio source fail\n");
        }
    }
AUDIO_UNREG:
    if (rtsp_session->media_info.audio_enable && audio_stream) {
        livestream_mbuffer_unregister(rtsp_session->media_info.mbuf_handle,
            rtsp_session->media_info.audio_source_info.codec_type);
        rtsp_session->media_info.audio_enable = XCAM_FALSE;
    }
VIDEO_REMOVE:
    if (rtsp_session->media_info.video_enable && video_stream) {
        if (XCAM_SUCCESS != remove_video_source(rtsp_session, video_stream)) {
            LOG_LIVESTREAM_ERROR("remove video source fail \n");
        }
    }
VIDEO_UNREG:
    if (rtsp_session->media_info.video_enable && video_stream) {
        livestream_mbuffer_unregister(rtsp_session->media_info.mbuf_handle,
            rtsp_session->media_info.video_source_info.codec_type);
            rtsp_session->media_info.video_enable = XCAM_FALSE;
    }
MBUF_DESTROY:
    if (XCAM_NULL != rtsp_session->media_info.mbuf_handle) {
        if (XCAM_SUCCESS != livestream_mbuffer_destroy(&(rtsp_session->media_info.mbuf_info),
                                rtsp_session->media_info.mbuf_handle)) {
            LOG_LIVESTREAM_ERROR("livestream mbuffer destroy fail \n");
        }
        rtsp_session->media_info.mbuf_handle = XCAM_NULL;
    }

    return ret;
}

static xcam_s32 handle_setup(livestream_rtsp_stream_session* rtsp_session, const xcam_char* request)
{
    livestream_rtsp_media_session* rtsp_media_session = XCAM_NULL;
    livestream_rtp_session* rtp_session = XCAM_NULL;
    xcam_s32 track_id = RTSP_INVALID_TRACK_ID;
    xcam_char resp_msg[RTSP_MAX_SDP_LEN] = {0};
    xcam_char url[RTSP_URL_MAX_LEN] = {0};
    xcam_char* tmp_ptr = XCAM_NULL;
    xcam_s32 ret = XCAM_SUCCESS;
    xcam_s32 url_len = 0;
    xcam_s32 count = 0;

    /*get the url*/
    count = sscanf(request, "SETUP %255s", url);
    if (count != RTSP_SCANF_RET_ONE) {
        LOG_LIVESTREAM_ERROR("rtsp req format error\n");
        handle_bad_request(rtsp_session);
        return XCAM_FAILURE;
    }
    url[RTSP_URL_MAX_LEN - 1] = '\0';
    url_len = strlen(url);
    if (url_len > 0 && url_len < RTSP_URL_MAX_LEN) {
        strncpy(rtsp_session->url, url, RTSP_URL_MAX_LEN - 1);
        rtsp_session->url[RTSP_URL_MAX_LEN - 1] = '\0';
    }

    tmp_ptr = strcasestr(url, RTSP_TRACK_ID);
    if (!tmp_ptr) {
        LOG_LIVESTREAM_ERROR("there no trackID field in setup url\n");
        handle_bad_request(rtsp_session);
        return XCAM_FAILURE;
    }

    /*get the track id 0-video 1-audio*/
    tmp_ptr += strlen(RTSP_TRACK_ID);
    track_id = atoi(tmp_ptr);
    /*malloc of rtpsession and rtcpsession when request video or audio */
    if (RTSP_TRACKID_VIDEO == track_id) {
        /*do not setup again*/
        if (rtsp_session->video_session.track_id != RTSP_INVALID_TRACK_ID) {
            LOG_LIVESTREAM_ERROR("setup video request repeat \n");
            handle_bad_request(rtsp_session);
            return XCAM_FAILURE;
        }

        /*judge whether need to register mbuff and start video venc*/
        if (!rtsp_session->media_info.video_enable) {
            ret = livestream_mbuffer_register(rtsp_session->media_info.mbuf_handle,
                        rtsp_session->media_info.video_source_info.codec_type);
            if (ret != XCAM_SUCCESS) {
                handle_server_error(rtsp_session);
                LOG_LIVESTREAM_ERROR("livestream mbuffer register video error\n");
                return XCAM_FAILURE;
            }

            ret = add_video_source(rtsp_session, rtsp_session->video_stream);
            if (ret != XCAM_SUCCESS) {
                LOG_LIVESTREAM_ERROR("add audio stream fail \n");
                handle_bad_request(rtsp_session);
                return XCAM_FAILURE;
            }

            rtsp_session->media_info.video_enable = XCAM_TRUE;
        }

        rtsp_session->video_session.track_id = RTSP_TRACKID_VIDEO;
        /*init the video rtcp session and rtp session when connect */
        ret = livestream_rtp_session_create(&rtsp_session->video_session.rtp_session, rtsp_session->packet_len);
        if (ret != XCAM_SUCCESS) {
            LOG_LIVESTREAM_ERROR("session video create rtpsession error\n");
            handle_server_error(rtsp_session);
            return XCAM_FAILURE;
        }
        ret = livestream_rtcp_session_create(&rtsp_session->video_session.rtcp_session, rtsp_session->packet_len);
        if (ret != XCAM_SUCCESS) {
            LOG_LIVESTREAM_ERROR("session video create rtcpsession error\n");
            handle_server_error(rtsp_session);
            goto RELEASE_VID_RTP;
        }
        rtp_session = rtsp_session->video_session.rtp_session;
        rtsp_media_session = &rtsp_session->video_session;
    } else if (RTSP_TRACKID_AUDIO == track_id) {
        /*do not setup again */
        if (rtsp_session->audio_session.track_id != RTSP_INVALID_TRACK_ID) {
            LOG_LIVESTREAM_ERROR("setup audio request repeat \n");
            handle_bad_request(rtsp_session);
            goto RELEASE_VID_RTCP;
        }

        /*judge whether need to register mbuff and start audio venc*/
        if (!rtsp_session->media_info.audio_enable) {
            ret = livestream_mbuffer_register(rtsp_session->media_info.mbuf_handle,
                        rtsp_session->media_info.audio_source_info.codec_type);
            if (ret != XCAM_SUCCESS) {
                handle_server_error(rtsp_session);
                LOG_LIVESTREAM_ERROR("livestream mbuffer register audio error\n");
                goto RELEASE_VID_RTCP;
            }

            ret = add_audio_source(rtsp_session, rtsp_session->audio_stream);
            if (ret != XCAM_SUCCESS) {
                handle_server_error(rtsp_session);
                goto RELEASE_VID_RTCP;
            }
            rtsp_session->media_info.audio_enable = XCAM_TRUE;
        }

        rtsp_session->audio_session.track_id = RTSP_TRACKID_AUDIO;
        /*init the audio rtcp session and rtp session when connect */
        ret = livestream_rtp_session_create(&rtsp_session->audio_session.rtp_session, rtsp_session->packet_len);
        if (ret != XCAM_SUCCESS) {
            LOG_LIVESTREAM_ERROR("session audio create rtcpsession error\n");
            handle_server_error(rtsp_session);
            goto RELEASE_VID_RTCP;
        }
        ret = livestream_rtcp_session_create(&rtsp_session->audio_session.rtcp_session, rtsp_session->packet_len);
        if (ret != XCAM_SUCCESS) {
            LOG_LIVESTREAM_ERROR("session audio create rtcpsession error\n");
            handle_server_error(rtsp_session);
            goto RELEASE_AUD_RTP;
        }
        rtp_session = rtsp_session->audio_session.rtp_session;
        rtsp_media_session = &rtsp_session->audio_session;
    } else {
        LOG_LIVESTREAM_ERROR("setup video request unsupport trackid: %d\n", track_id);
        handle_stream_not_found(rtsp_session);
        return XCAM_FAILURE;
    }

    /*parse the transformat of rtp packet */
    ret = parse_transports(rtsp_session, request, rtsp_media_session);
    if (ret != XCAM_SUCCESS) {
        LOG_LIVESTREAM_ERROR("rtsp req format error\n");
        handle_unsupported_transport(rtsp_session);
        goto RELEASE_SOURCE;
    }

    /*get the send port of server*/
    get_media_send_port(rtsp_media_session);

    /*get the random ssrc*/
    livestream_comm_random_num(&rtp_session->ssrc);

    ret = snprintf(rtp_session->client_ip, RTP_TRANS_IP_MAX_LEN - 1, "%s", rtsp_session->client_ip);
    if (ret < 0) {
        LOG_LIVESTREAM_ERROR("string printf client_ip error\n");
        handle_bad_request(rtsp_session);
        goto RELEASE_SOURCE;
    }

    ret = generate_setup_response(rtsp_session, rtsp_media_session, resp_msg, RTSP_MAX_PROTOCOL_BUFFER);
    if (ret != XCAM_SUCCESS) {
        LOG_LIVESTREAM_ERROR("rtsp GenerateSetupResponse error\n");
        handle_bad_request(rtsp_session);
        goto RELEASE_SOURCE;
    }

    if (rtsp_session->timeout > 0) {
        xcam_char buffer[MAX_DATE_LEN] = {0};
        xcam_char status_str[MAX_DATE_LEN] = {0};
        memset(status_str, '\0', sizeof(status_str));
        livestream_rtsp_message_status_code_to_str(RTSP_STATUS_CODE_OK,
            status_str, sizeof(status_str)-1);
        memset(buffer, '\0', sizeof(buffer));
        livestream_comm_format_date(buffer, sizeof(buffer)-1);
        ret = snprintf(rtsp_session->response_buf, sizeof(rtsp_session->response_buf) - 1,
                            "%s %d %s\r\n"
                            "CSeq: %d\r\n"
                            "Session: %s;timeout=%d\r\n"
                            "Server: %s\r\n"
                            "%s"
                            "%s\r\n",
                            RTSP_VER_STR,
                            RTSP_STATUS_CODE_OK,
                            status_str,
                            rtsp_session->cur_request_seq,
                            rtsp_session->session_id,
                            rtsp_session->timeout,
                            RTSP_SERVER_DESCRIPTION,
                            buffer,
                            resp_msg);
    } else {
        xcam_char buffer[MAX_DATE_LEN] = {0};
        xcam_char status_str[MAX_DATE_LEN] = {0};
        memset(status_str, '\0', sizeof(status_str));
        livestream_rtsp_message_status_code_to_str(RTSP_STATUS_CODE_OK,
            status_str, sizeof(status_str)-1);
        memset(buffer, '\0', sizeof(buffer));
        livestream_comm_format_date(buffer, sizeof(buffer)-1);
        ret = snprintf(rtsp_session->response_buf, sizeof(rtsp_session->response_buf) - 1,
                            "%s %d %s\r\n"
                            "CSeq: %d\r\n"
                            "Session: %s\r\n"
                            "Server: %s\r\n"
                            "%s"
                            "%s\r\n",
                            RTSP_VER_STR,
                            RTSP_STATUS_CODE_OK,
                            status_str,
                            rtsp_session->cur_request_seq,
                            rtsp_session->session_id,
                            RTSP_SERVER_DESCRIPTION,
                            buffer,
                            resp_msg);
    }

    if (ret < 0) {
        LOG_LIVESTREAM_ERROR("string printf client_ip error\n");
        handle_bad_request(rtsp_session);
        goto RELEASE_SOURCE;
    }

    if ((LIVESTREAM_RTSP_SESSION_STATE_INIT != rtsp_session->session_state)
       && (LIVESTREAM_RTSP_SESSION_STATE_READY != rtsp_session->session_state)
       && (LIVESTREAM_RTSP_SESSION_STATE_PLAY != rtsp_session->session_state)) {
        LOG_LIVESTREAM_ERROR("rtsp server session state error\n");
        handle_bad_request(rtsp_session);
        ret = XCAM_FAILURE;
        goto RELEASE_SOURCE;
    }

    rtp_session->session_state = RTP_SESSION_STATE_READY;//rtp state

    return XCAM_SUCCESS;

RELEASE_SOURCE:
    if (RTSP_INVALID_TRACK_ID != rtsp_session->audio_session.track_id) {
        livestream_rtcp_session_destroy(rtsp_session->audio_session.rtcp_session);
        rtsp_session->audio_session.rtcp_session = XCAM_NULL;
    }
RELEASE_AUD_RTP:
    if (RTSP_INVALID_TRACK_ID != rtsp_session->audio_session.track_id) {
        rtsp_session->audio_session.track_id = RTSP_INVALID_TRACK_ID;
        livestream_rtp_session_destroy(rtsp_session->audio_session.rtp_session);
        rtsp_session->audio_session.rtp_session = XCAM_NULL;
    }
RELEASE_VID_RTCP:
    if (RTSP_INVALID_TRACK_ID !=  rtsp_session->video_session.track_id) {
        livestream_rtcp_session_destroy(rtsp_session->video_session.rtcp_session);
        rtsp_session->video_session.rtcp_session = XCAM_NULL;
    }
RELEASE_VID_RTP:
    if (RTSP_INVALID_TRACK_ID !=  rtsp_session->video_session.track_id) {
        rtsp_session->video_session.track_id = RTSP_INVALID_TRACK_ID;
        livestream_rtp_session_destroy(rtsp_session->video_session.rtp_session);
        rtsp_session->video_session.rtp_session = XCAM_NULL;
    }

    return ret;
}

/*
PLAY rtsp://ip/sample_h264_100kbit.mp4 RTSP/1.0

CSeq: 7

Range: npt=0.000000-70.000000

Session: x

======================================================

RTSP/1.0 200 OK

Server: Streaming Media Server/1.0.0(Jul 30 2015)

Cseq: 7

Session: x

Range: npt=0.00000-70.00000

RTP-Info: url=rtsp://ip:554/12?/trackID=0;seq=379;rtptime=2384477337,
*/
static xcam_s32 handle_play(livestream_rtsp_stream_session* rtsp_session, const  xcam_char* request)
{
    xcam_char resp_msg[RTSP_MAX_SDP_LEN] = {0};
    xcam_s32 ret = XCAM_SUCCESS;

    RTSPSVR_CHECK_NULL_ERROR(rtsp_session);
    RTSPSVR_CHECK_NULL_ERROR(request);
    RTSPSVR_CHECK_NULL_ERROR(rtsp_session->video_session.rtcp_session);

    if ((RTSP_INVALID_TRACK_ID == rtsp_session->video_session.track_id)
        && (RTSP_INVALID_TRACK_ID == rtsp_session->audio_session.track_id)) {
        LOG_LIVESTREAM_ERROR("nothing to play \n");
        handle_stream_not_found(rtsp_session);
        return XCAM_FAILURE;
    }

    if (((  RTSP_INVALID_TRACK_ID != rtsp_session->video_session.track_id) &&
         (RTP_SESSION_STATE_READY != rtsp_session->video_session.rtp_session->session_state))
        || ((  RTSP_INVALID_TRACK_ID != rtsp_session->audio_session.track_id) &&
            (RTP_SESSION_STATE_READY != rtsp_session->audio_session.rtp_session->session_state))
       ) {
        if (((  RTSP_INVALID_TRACK_ID != rtsp_session->video_session.track_id) &&
             (RTP_SESSION_STATE_PLAY == rtsp_session->video_session.rtp_session->session_state))
            || ((  RTSP_INVALID_TRACK_ID != rtsp_session->audio_session.track_id) &&
                (RTP_SESSION_STATE_PLAY == rtsp_session->audio_session.rtp_session->session_state))
           ) {
            LOG_LIVESTREAM_ERROR("One track already is in playing state!!\r\n");
        } else {
            LOG_LIVESTREAM_ERROR("the media is not in ready state\n");
            return XCAM_FAILURE;
        }
    }

    /*start trans task for media*/
    if (RTSP_INVALID_TRACK_ID != rtsp_session->video_session.track_id) {
        if (RTP_SESSION_STATE_PLAY != rtsp_session->video_session.rtp_session->session_state) {
            /* start video trans task*/
            ret = livestream_rtp_session_start(rtsp_session->video_session.rtp_session);
            if (ret != XCAM_SUCCESS) {
                LOG_LIVESTREAM_ERROR("rtsp livestream_rtp_session_start video error\n");
                return XCAM_FAILURE;
            }

            if (RTP_TRANS_UDP == rtsp_session->video_session.rtp_session->media_trans_mode) {
                ret = livestream_rtcp_session_start_udp(rtsp_session->video_session.rtcp_session);
                if (ret != XCAM_SUCCESS) {
                    LOG_LIVESTREAM_ERROR("rtcp session start udp fail\n");
                    goto VIDEO_RTP_STOP;
                }
            }

            livestream_mbuffer_pts(rtsp_session->media_info.mbuf_handle,
                    rtsp_session->media_info.video_source_info.codec_type,
                    &rtsp_session->media_info.video_start_pts);
        }
    }

    if (RTSP_INVALID_TRACK_ID != rtsp_session->audio_session.track_id) {
        if (RTP_SESSION_STATE_PLAY != rtsp_session->audio_session.rtp_session->session_state) {
            /* start audio trans task*/
            ret = livestream_rtp_session_start(rtsp_session->audio_session.rtp_session);
            if (ret != XCAM_SUCCESS) {
                LOG_LIVESTREAM_ERROR("rtsp livestream_rtp_session_start audio error\n");
                goto VIDEO_RTCP_STOP;
            }

            if (RTP_TRANS_UDP == rtsp_session->audio_session.rtp_session->media_trans_mode) {
                ret = livestream_rtcp_session_start_udp(rtsp_session->audio_session.rtcp_session);
                if (ret != XCAM_SUCCESS) {
                    LOG_LIVESTREAM_ERROR("rtcp session start udp fail!\n");
                    goto AUDIO_RTP_STOP;
                }
            }

            livestream_mbuffer_pts(rtsp_session->media_info.mbuf_handle,
                    rtsp_session->media_info.audio_source_info.codec_type,
                    &rtsp_session->media_info.audio_start_pts);
        }
    }

    ret = generate_play_response(rtsp_session, resp_msg, RTSP_MAX_PROTOCOL_BUFFER);
    if (ret != XCAM_SUCCESS) {
        LOG_LIVESTREAM_ERROR("rtsp GeneratePlayResponse error\n");
        handle_bad_request(rtsp_session);
        goto AUDIO_RTCP_STOP;
    }

    xcam_char buffer[MAX_DATE_LEN] = {0};
    xcam_char status_str[MAX_DATE_LEN] = {0};
    memset(status_str, '\0', sizeof(status_str));
    livestream_rtsp_message_status_code_to_str(RTSP_STATUS_CODE_OK,
        status_str, sizeof(status_str)-1);
    memset(buffer, '\0', sizeof(buffer));
    livestream_comm_format_date(buffer, sizeof(buffer)-1);
    ret = snprintf(rtsp_session->response_buf, sizeof(rtsp_session->response_buf) - 1,
                        "%s %d %s\r\n"
                        "Server: %s\r\n"
                        "CSeq: %d\r\n"
                        "Session: %s\r\n"
                        "Date: %s"
                        "%s\r\n",
                        RTSP_VER_STR,
                        RTSP_STATUS_CODE_OK,
                        status_str,
                        RTSP_SERVER_DESCRIPTION,
                        rtsp_session->cur_request_seq,
                        rtsp_session->session_id,
                        buffer,
                        resp_msg);
    if (ret < 0) {
        LOG_LIVESTREAM_ERROR("string print response buf error\n");
        handle_bad_request(rtsp_session);
        goto AUDIO_RTCP_STOP;
    }

    if ((LIVESTREAM_RTSP_SESSION_STATE_READY != rtsp_session->session_state)
        && (LIVESTREAM_RTSP_SESSION_STATE_PLAY != rtsp_session->session_state)) {
        LOG_LIVESTREAM_ERROR("rtsp server state  error\n");
        handle_server_error(rtsp_session);
        ret = XCAM_FAILURE;
        goto AUDIO_RTCP_STOP;
    }

    return XCAM_SUCCESS;

AUDIO_RTCP_STOP:
    if ( RTSP_INVALID_TRACK_ID != rtsp_session->audio_session.track_id) {
        if (RTP_TRANS_UDP == rtsp_session->audio_session.rtp_session->media_trans_mode) {
            livestream_rtcp_session_stop_udp(rtsp_session->audio_session.rtcp_session);
        }
    }
AUDIO_RTP_STOP:
    if ( RTSP_INVALID_TRACK_ID != rtsp_session->audio_session.track_id) {
        livestream_rtp_session_stop(rtsp_session->audio_session.rtp_session);
    }

VIDEO_RTCP_STOP:
    if ( RTSP_INVALID_TRACK_ID != rtsp_session->video_session.track_id) {
        if (RTP_TRANS_UDP == rtsp_session->video_session.rtp_session->media_trans_mode) {
            livestream_rtcp_session_stop_udp(rtsp_session->video_session.rtcp_session);
        }
    }
VIDEO_RTP_STOP:
    if ( RTSP_INVALID_TRACK_ID != rtsp_session->video_session.track_id) {
        livestream_rtp_session_stop(rtsp_session->video_session.rtp_session);
    }

    return ret;
}

/*
TEARDOWN rtsp://ip/sample_100kbit.mp4/ RTSP/1.0

Session: x

User-Agent: RealMedia Player/mc.30.26.01 (s60; epoc_av30_armv5)

CSeq: 7

===============================================================

RTSP/1.0 200 OK

Server: DSS/5.5.5 (Build/489.16; Platform/Win32; Release/Darwin; state/beta; )

Cseq: 7

Session: x

Connection: Close
*/
/**
 * @brief teardown video track.
 * @param[in] session : session info,
 * @return   0 success
 * @return  -1 failure
 */
static xcam_s32 video_track_teardown(livestream_rtsp_stream_session* rtsp_session)
{
    xcam_s32 ret = XCAM_SUCCESS;

    if (XCAM_NULL == rtsp_session) {
        LOG_LIVESTREAM_ERROR("Input argument is null!\n");
        return XCAM_FAILURE;
    }

    if (RTSP_INVALID_TRACK_ID == rtsp_session->video_session.track_id) {
        LOG_LIVESTREAM_ERROR("Video track is XCAM_NULL, not need to teardown again\n");
        return XCAM_SUCCESS;
    }

    if (rtsp_session->media_info.video_enable) {
        rtsp_session->media_info.video_enable = XCAM_FALSE;
        ret = remove_video_source(rtsp_session, rtsp_session->video_stream);
        if (XCAM_SUCCESS != ret) {
            LOG_LIVESTREAM_ERROR("remove video source fail ret:0x%x\n", ret);
        }

        livestream_mbuffer_unregister(rtsp_session->media_info.mbuf_handle,
                rtsp_session->media_info.video_source_info.codec_type);

        if (rtsp_session->video_session.track_id != RTSP_INVALID_TRACK_ID) {
            livestream_rtp_session_stop(rtsp_session->video_session.rtp_session);

            if (RTP_TRANS_UDP == rtsp_session->video_session.rtp_session->media_trans_mode) {
                livestream_rtcp_session_stop_udp(rtsp_session->video_session.rtcp_session);
            }

            rtsp_session->video_session.track_id = RTSP_INVALID_TRACK_ID;
            livestream_rtp_session_destroy(rtsp_session->video_session.rtp_session);
            rtsp_session->video_session.rtp_session = XCAM_NULL;

            livestream_rtcp_session_destroy(rtsp_session->video_session.rtcp_session);
            rtsp_session->video_session.rtcp_session = XCAM_NULL;
        }
    }

    if (!rtsp_session->media_info.audio_enable) {
        (xcam_void)pthread_mutex_lock(&rtsp_session->state_lock);
        rtsp_session->session_state = LIVESTREAM_RTSP_SESSION_STATE_STOP;
        (xcam_void)pthread_mutex_unlock(&rtsp_session->state_lock);
    }

    return XCAM_SUCCESS;
}

/**
 * @brief teardown audio track.
 * @param[in] session : session info,
 * @return   0 success
 * @return  -1 failure
 */
static xcam_s32 audio_track_teardown(livestream_rtsp_stream_session* rtsp_session)
{
    xcam_s32 ret = XCAM_SUCCESS;

    if (XCAM_NULL == rtsp_session) {
        LOG_LIVESTREAM_ERROR("Input argument is null!!!\n");
        return XCAM_FAILURE;
    }

    if (RTSP_INVALID_TRACK_ID == rtsp_session->audio_session.track_id) {
        LOG_LIVESTREAM_ERROR("Audio track is XCAM_NULL, not need to teardown again\n");
        return XCAM_SUCCESS;
    }

    if (rtsp_session->media_info.audio_enable) {
        rtsp_session->media_info.audio_enable = XCAM_FALSE;
        ret = remove_audio_source(rtsp_session, rtsp_session->audio_stream);
        if (XCAM_SUCCESS != ret) {
            LOG_LIVESTREAM_ERROR("remove audio source fail, ret:0x%X \n", ret);
        }

        livestream_mbuffer_unregister(rtsp_session->media_info.mbuf_handle,
            rtsp_session->media_info.audio_source_info.codec_type);

        if (rtsp_session->audio_session.track_id != RTSP_INVALID_TRACK_ID) {
            livestream_rtp_session_stop(rtsp_session->audio_session.rtp_session);

            if (RTP_TRANS_UDP == rtsp_session->audio_session.rtp_session->media_trans_mode) {
                livestream_rtcp_session_stop_udp(rtsp_session->audio_session.rtcp_session);
            }

            rtsp_session->audio_session.track_id = RTSP_INVALID_TRACK_ID;
            livestream_rtp_session_destroy(rtsp_session->audio_session.rtp_session);
            rtsp_session->audio_session.rtp_session = XCAM_NULL;

            livestream_rtcp_session_destroy(rtsp_session->audio_session.rtcp_session);
            rtsp_session->audio_session.rtcp_session = XCAM_NULL;
        }
    }

    if (!rtsp_session->media_info.video_enable) {
        (xcam_void)pthread_mutex_lock(&rtsp_session->state_lock);
        rtsp_session->session_state = LIVESTREAM_RTSP_SESSION_STATE_STOP;
        (xcam_void)pthread_mutex_unlock(&rtsp_session->state_lock);
    }

    return XCAM_SUCCESS;
}

/**
 * @brief handle teardown request.
 * @param[in] session : session info, pcRequest : msg from client, u32ReqLen: length of msg
 * @return   0 success
 * @return  -1 failure
 */
static xcam_s32 handle_teardown(livestream_rtsp_stream_session* rtsp_session, const xcam_char* request)
{
    xcam_char sess_id[RTSP_SESSID_MAX_LEN] = {0};
    xcam_char url[RTSP_URL_MAX_LEN] = {0};
    xcam_s32 track_id = RTSP_INVALID_TRACK_ID;
    xcam_char* tmp_ptr = XCAM_NULL;
    xcam_s32 ret = XCAM_SUCCESS;
    xcam_s32 url_len = 0;
    xcam_s32 count = 0;

    RTSPSVR_CHECK_NULL_ERROR(rtsp_session);
    memset(rtsp_session->response_buf, 0, RTSP_MAX_PROTOCOL_BUFFER);
    ret = livestream_rtsp_message_get_session_id(request, sess_id, RTSP_SESSID_MAX_LEN);
    if (XCAM_SUCCESS != ret) {
        /*reply the wrong message and close the socket*/
        strncpy(sess_id, rtsp_session->session_id, RTSP_SESSID_MAX_LEN - 1);
        sess_id[RTSP_SESSID_MAX_LEN - 1] = '\0';
        handle_bad_request(rtsp_session);
        LOG_LIVESTREAM_ERROR("get teardown session id failed :%d \n", ret);
        return ret;
    }

    /*Get the trackid, judge which track will be tear down*/
    count = sscanf(request, "TEARDOWN %255s", url);
    if (count != RTSP_SCANF_RET_ONE) {
        LOG_LIVESTREAM_ERROR("rtsp req format error\n");
        handle_bad_request(rtsp_session);
        return XCAM_FAILURE;
    }

    url[RTSP_URL_MAX_LEN - 1] = '\0';
    url_len = strlen(url);
    if ((url_len > 0) && (url_len < RTSP_URL_MAX_LEN)) {
        strncpy(rtsp_session->url, url, RTSP_MAX_STREAMNAME_LEN - 1);
        rtsp_session->url[RTSP_URL_MAX_LEN - 1] = '\0';
    }

    tmp_ptr = strcasestr(url, RTSP_TRACK_ID);
    if (!tmp_ptr) {
        ret = livestream_rtsp_session_stop(rtsp_session);
        if (XCAM_SUCCESS != ret) {
            handle_bad_request(rtsp_session);
            LOG_LIVESTREAM_ERROR("teardown session stop failed :%d \n", ret);
            return ret;
        }

        return XCAM_SUCCESS;
    } else {
        /*get the track id 0-video 1-audio*/
        tmp_ptr += strlen(RTSP_TRACK_ID);
        track_id = atoi(tmp_ptr);

        if (RTSP_TRACKID_VIDEO == track_id) {
            ret = video_track_teardown(rtsp_session);
            if (XCAM_SUCCESS != ret) {
                handle_bad_request(rtsp_session);
                LOG_LIVESTREAM_ERROR("teardown video track failed :%d \n", ret);
                return ret;
            }
        } else if (RTSP_TRACKID_AUDIO == track_id) {
            ret = audio_track_teardown(rtsp_session);
            if (XCAM_SUCCESS != ret) {
                handle_bad_request(rtsp_session);
                LOG_LIVESTREAM_ERROR("teardown audio track failed :%d \n", ret);
                return ret;
            }
        } else {
            /*no need to return error!!*/
            LOG_LIVESTREAM_ERROR("teardown request unsupport trackid: %d\n", track_id);
            handle_bad_request(rtsp_session);
            return XCAM_SUCCESS;
        }
    }

    return XCAM_SUCCESS;
}

static xcam_void handle_get_param(livestream_rtsp_stream_session* rtsp_session)
{
    xcam_char status_str[MAX_DATE_LEN] = {0};

    memset(status_str, '\0', sizeof(status_str));
    livestream_rtsp_message_status_code_to_str(RTSP_STATUS_CODE_OK,
        status_str, sizeof(status_str)-1);
    int ret = snprintf(rtsp_session->response_buf, sizeof(rtsp_session->response_buf) - 1,
                    "%s %d %s\r\n"
                    "CSeq: %d\r\n"
                    "Session: %s;timeout=%d\r\n"
                    "Cache-Control: no-cache\r\n"
                    "Content-Length: 0\r\n"
                    "Server: %s\r\n\r\n",
                    RTSP_VER_STR,
                    RTSP_STATUS_CODE_OK,
                    status_str,
                    rtsp_session->cur_request_seq,
                    rtsp_session->session_id,
                    rtsp_session->timeout,
                    RTSP_SERVER_DESCRIPTION);
    if (ret < 0) {
        LOG_LIVESTREAM_ERROR("string printf pRespBuffer error\n");
        return;
    }
    return;
}

static xcam_s32 session_send_data(livestream_rtp_session* rtp_session, rtp_pack_type pack_type,
                    const xcam_u8* frame, xcam_u32 frame_len, xcam_u32 timestamp,
                    rtp_payload_type package_type, xcam_u32* out_seq_num, xcam_u32 ssrc,
                    xcam_s32 write_sock, struct sockaddr_in* peer_sockaddr, xcam_s32 packet_len)
{
    xcam_u32 seq_num = *out_seq_num;
    xcam_s32 ret = XCAM_SUCCESS;

    /*rtp on udp  pack in fua send in udp */
    if (PACK_TYPE_RTP_FUA == pack_type) { /*RTP/UDP*/
        ret = livestream_rtp_session_send_data_in_rtp(rtp_session, frame, frame_len, timestamp,
                    package_type, &seq_num, ssrc, write_sock, peer_sockaddr, packet_len);
    }
    /*rtp on tcp pack in PACK_TYPE_RTSP_ITLV send in tcp*/
    else if (PACK_TYPE_RTSP_ITLV == pack_type || PACK_TYPE_RTSP_O_HTTP == pack_type) { /*RTP/TCP*/
        ret = livestream_rtp_session_send_data_in_rtsp_itlv(rtp_session, frame, frame_len, timestamp,
                 package_type, &seq_num, ssrc, write_sock, XCAM_NULL, packet_len);
    } else {
        /*rtsp only support tcl_itlv and udp now ;broadcast is not supported*/
        LOG_LIVESTREAM_ERROR("packtype not support %d.\n", pack_type);
        return XCAM_FAILURE;
    }

    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("send_avc_key_frame fail\n");
        return ret;
    }

    *out_seq_num = seq_num;

    return XCAM_SUCCESS;
}

static xcam_s32 send_avc_key_frame(livestream_rtp_session* rtp_session, rtp_pack_type pack_type,
                    const xcam_u8* frame, xcam_u32 frame_len, xcam_u32 timestamp,
                    rtp_payload_type package_type, xcam_u32* out_seq_num, xcam_u32 ssrc,
                    xcam_s32 write_sock, struct sockaddr_in* peer_sockaddr, xcam_s32 packet_len)
{
    const xcam_u8* tmp_buffer = XCAM_NULL;
    xcam_s32 ret = XCAM_SUCCESS;
    xcam_u32 cursor = 0;
    xcam_u32 buf_len = 0;
    xcam_u32 seq_num = *out_seq_num;

    // get first I frame and parse sps pps
    do {
        xcam_u8 nal_type = 0;
        xcam_u32 nal_len = get_avc_nal_unit(&nal_type, frame + cursor, frame_len - cursor);
        if (nal_len <= 4 || cursor + nal_len  > frame_len) {
            LOG_LIVESTREAM_ERROR("nal_len illegal: %d \n", nal_len);
            return XCAM_FAILURE;
        }

        tmp_buffer = frame + cursor; //  + 4,;
        buf_len = nal_len; //- 4;
        ret = session_send_data(rtp_session, pack_type, tmp_buffer, buf_len, timestamp,
                    package_type, &seq_num, ssrc, write_sock, peer_sockaddr, packet_len);
        if (XCAM_SUCCESS != ret) {
            LOG_LIVESTREAM_ERROR("send_avc_key_frame fail\n");
            return ret;
        }

        cursor += nal_len;
    }
    while (cursor + 4 < frame_len);

    *out_seq_num  = seq_num;

    return XCAM_SUCCESS;
}

static xcam_s32  send_hevc_key_frame(livestream_rtp_session* rtp_session, rtp_pack_type pack_type,
                    xcam_u8* frame, xcam_u32 frame_len, xcam_u32 timestamp,
                    rtp_payload_type package_type, xcam_u32* out_seq_num, xcam_u32 ssrc,
                    xcam_s32 write_sock, struct sockaddr_in* peer_sockaddr, xcam_s32 packet_len)
{
    xcam_u8* tmp_buffer = XCAM_NULL;
    xcam_s32 ret = XCAM_SUCCESS;
    xcam_u32 buf_len = 0;
    xcam_u32 cursor = 0;
    xcam_u32 seq_num = *out_seq_num;

    do {
        xcam_u8 nal_type = 0;
        xcam_u32 nal_len = get_hevc_nal_unit(&nal_type, frame + cursor, frame_len - cursor);
        if (nal_len <= 4 || cursor + nal_len  > frame_len) {
            LOG_LIVESTREAM_ERROR("nal_len illegal: %d \n", nal_len);
            return XCAM_FAILURE;
        }

        tmp_buffer = frame + cursor; //  + 4,;
        buf_len = nal_len; //- 4;

        ret = session_send_data(rtp_session, pack_type, tmp_buffer, buf_len, timestamp,
                    package_type, &seq_num, ssrc, write_sock, peer_sockaddr, packet_len);
        if (XCAM_SUCCESS != ret) {
            LOG_LIVESTREAM_ERROR("send_avc_key_frame fail\n");
            return ret;
        }

        cursor += nal_len;
    }
    while (cursor + 4 < frame_len);

    *out_seq_num = seq_num;

    return XCAM_SUCCESS;
}

static xcam_s32 media_sending_process(livestream_rtsp_stream_session* rtsp_session)
{
    xcam_rtp_session_state video_state = RTP_SESSION_STATE_BUTT;
    xcam_rtp_session_state audio_state = RTP_SESSION_STATE_BUTT;
    livestream_mbuffer_data_type data_type = MBUFFER_DATA_BUTT;
    rtp_trans_mode trans_mode = RTP_TRANS_BUTT;
    rtp_pack_type pack_type = PACK_TYPE_BUTT;
    rtp_payload_type rtp_payload_type = RTP_PT_INVALID;
    xcam_s32 write_sock = INVALID_SOCKET;
    xcam_bool is_discard_flag = XCAM_FALSE;
    struct sockaddr_in peer_sockaddr;
    xcam_u8* buff_addr = XCAM_NULL;/*pointer to addr of stream ready to send */
    livestream_rtp_session* rtp_session = XCAM_NULL;
    livestream_rtp_session* video_rtp_session = XCAM_NULL;
    livestream_rtp_session* audio_rtp_session = XCAM_NULL;
    xcam_void* buff_handle = XCAM_NULL;
    xcam_bool is_key_flag = XCAM_FALSE;
    xcam_u32 data_len = 0;  /*len of stream ready to send*/
    xcam_u32 calced_pts = 0;  /*calculated pts,used in FU-A*/
    xcam_u32 last_sn = 0;
    xcam_u32 ssrc = 0;
    xcam_u64 pts = 0;  /*stream pts*/
    xcam_s32 ret = 0;

    if (XCAM_NULL == rtsp_session) {
        LOG_LIVESTREAM_ERROR("media_sending_process null param ! \n");
        return XCAM_FAILURE;
    }

    if (rtsp_session->video_session.track_id != RTSP_INVALID_TRACK_ID) {
        video_rtp_session = rtsp_session->video_session.rtp_session;
        trans_mode = video_rtp_session->media_trans_mode;
        video_state = video_rtp_session->session_state;
    }

    if (rtsp_session->audio_session.track_id != RTSP_INVALID_TRACK_ID) {
        audio_rtp_session = rtsp_session->audio_session.rtp_session;
        trans_mode = audio_rtp_session->media_trans_mode;
        audio_state = audio_rtp_session->session_state;
    }

    /* trans mode must be tcp interleaved or udp otherwise, media data is not send by session thread*/
    if (RTP_TRANS_TCP_ITLV != trans_mode && RTP_TRANS_UDP != trans_mode
            && RTP_TRANS_BROADCAST != trans_mode) {
        LOG_LIVESTREAM_ERROR("trans mode must be tcp or udp or broadcast.\n");
        return XCAM_SUCCESS;
    }

    buff_handle = rtsp_session->media_info.mbuf_handle;
    if ((RTP_SESSION_STATE_PLAY != video_state) && (RTP_SESSION_STATE_PLAY != audio_state)) {
        /*video,audio and data are all not playing,so sleep 1 s*/
        usleep(1000 * 1000);
        return XCAM_SUCCESS;
    } else {
        /*request data from mediabuff */
        ret = livestream_mbuffer_read(buff_handle, (xcam_void**)(&buff_addr), &data_len, &pts, &data_type, &is_key_flag);
        /*there is no data ready */
        if (XCAM_SUCCESS != ret || 0 == data_len) {
            usleep(10 * 1000);
            return LIVESTREAM_ERRNO_SESS_NO_DATA;
        } else { /*judge the readed data is whether in playing state, if is not, just free this slice and not sending*/
            switch (data_type) {
                case MBUFFER_DATA_VIDEO:
                    (RTP_SESSION_STATE_PLAY != video_state) ?
                    (is_discard_flag = XCAM_TRUE) : (is_discard_flag = XCAM_FALSE);
                    break;

                case MBUFFER_DATA_AUDIO:
                    (RTP_SESSION_STATE_PLAY != audio_state) ?
                    (is_discard_flag = XCAM_TRUE) : (is_discard_flag = XCAM_FALSE);
                    break;

                default:
                    is_discard_flag = XCAM_TRUE;
                    break;
            }

            /*if this media type is not in playing state, discard the slice*/
            if (XCAM_TRUE == is_discard_flag) {
                livestream_mbuffer_set(buff_handle, 1);
                return XCAM_SUCCESS;
            }
        }
    }

    /*calculate the pts of the audio and video,64 bit in venc&aenc ,32 bit in rtp*/
    if (MBUFFER_DATA_VIDEO == data_type) {
        pts = pts - rtsp_session->media_info.video_start_pts; /*setup agian after teardown may cause two kinds of time*/
        rtp_session = rtsp_session->video_session.rtp_session;
        pts = (pts * RTSP_CLOCK_RATE_90K / RTSP_TIME_SCALE);
        calced_pts = (xcam_u32)pts;

        rtp_session->data_len += data_len;
    } else if (MBUFFER_DATA_AUDIO == data_type) {
        pts = pts - rtsp_session->media_info.audio_start_pts;  /*setup agian after teardown may cause two kinds of time*/
        rtp_session = rtsp_session->audio_session.rtp_session;

        xcam_track_audio_codec audio_codec = rtsp_session->media_info.audio_source_info.codec_type;
        if (audio_codec == XCAM_TRACK_AUDIO_CODEC_G711A || audio_codec == XCAM_TRACK_AUDIO_CODEC_G711U) {
            pts = (pts * RTSP_CLOCK_RATE_8K / RTSP_TIME_SCALE);
        } else {
            pts = (pts * RTSP_CLOCK_RATE_90K / RTSP_TIME_SCALE);
        }

        calced_pts = (xcam_u32)pts;
        rtp_session->data_len += data_len;
    }

    /*get the send data length*/
    livestream_rtp_session_get_packet_and_send_param(rtp_session, &write_sock,
                                         &peer_sockaddr, &last_sn, &pack_type, &ssrc);
    /*get rtp payloadtype according to stream's type (mbuff modle naming the stream type)*/
    ret = get_rtp_payload_type(rtsp_session, data_type, &rtp_payload_type);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("GetRtpPayloadType fail %d.\n", ret);
        return ret;
    }

    // if video i frame,parse I frame into sps pps idr frame and send rtp packet
    if (is_key_flag && MBUFFER_DATA_VIDEO == data_type) {
        if (RTP_PT_H264 == rtp_payload_type) {
            ret = send_avc_key_frame(rtp_session, pack_type, buff_addr, data_len,
                        calced_pts, rtp_payload_type, &last_sn, ssrc, write_sock,
                        &peer_sockaddr, rtsp_session->packet_len);
        } else if (RTP_PT_H265 == rtp_payload_type) {
            ret = send_hevc_key_frame(rtp_session, pack_type, buff_addr, data_len,
                        calced_pts, rtp_payload_type, &last_sn, ssrc, write_sock,
                        &peer_sockaddr, rtsp_session->packet_len);
        } else {
            //not support type
            ret = XCAM_FAILURE;
        }
    } else {
        ret = session_send_data(rtp_session, pack_type, buff_addr, data_len, calced_pts,
                    rtp_payload_type, &last_sn, ssrc, write_sock, &peer_sockaddr, rtsp_session->packet_len);
        if (XCAM_SUCCESS != ret) {
            LOG_LIVESTREAM_ERROR("send_avc_key_frame fail\n");
            return ret;
        }
    }

    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("rtspserver Send Stream Data Fail.\n");
        livestream_mbuffer_set(buff_handle, 1);
        return ret;
    } else {
        livestream_mbuffer_set(buff_handle, 1);
        livestream_rtp_session_update_seq_num(rtp_session, last_sn);
    }

    return XCAM_SUCCESS;
}

/**
 * @brief send the data.
 * @param[in] rtsp_session : session info
 * @return   0 success
 * @return  -1 failure
 */
static xcam_s32 send_media(livestream_rtsp_stream_session* rtsp_session)
{
    xcam_s32 ret = XCAM_SUCCESS;

    if (XCAM_NULL == rtsp_session) {
        LOG_LIVESTREAM_ERROR("Input argument is null!\n");
        return XCAM_FAILURE;
    }

    if (LIVESTREAM_RTSP_SESSION_STATE_PLAY == rtsp_session->session_state) {
        ret = media_sending_process(rtsp_session);
        if (ret != XCAM_SUCCESS && ret != LIVESTREAM_ERRNO_SESS_NO_DATA) {
            return XCAM_FAILURE;
        }
    }

    return XCAM_SUCCESS;
}

/*
rfc2326bis06-Page42 SETUP can be used in all three states; INIT,and READY
InitState:
    Describe: --> init_state
    Setup   : --> *ready_state
    Teardown: --> init_state
    Options : --> init_state
    Play    : --> init_state
    Pause   : --> init_state

ReadyState:
    Play    : --> *play_state
    Setup   : --> ready
    Teardown: --> ??
    Options : -->
    pause   : -->
    describe: -->

*/
static xcam_s32 handle_stream_session_request(livestream_rtsp_stream_session* rtsp_session, const xcam_char* request)
{
    livestream_rtsp_request_method method = LIVESTREAM_RTSP_REQ_METHOD_BUTT;
    xcam_s32 ret = XCAM_SUCCESS;
    xcam_s32 resp_len = 0;
    xcam_s32 cseq = 0;

    RTSPSVR_CHECK_NULL_ERROR(rtsp_session);
    RTSPSVR_CHECK_NULL_ERROR(request);
    /*response msg or rtcp msg do not handle and return success*/
    if (!livestream_rtsp_message_check_request(request)) {
        LOG_LIVESTREAM_INFO("response msg or rtcp in tcp_itlv invalid rtsp \n");
        return XCAM_SUCCESS;
    }

    ret = livestream_rtsp_message_parse_method(request, &method);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("invalid rtsp request, invalid method \n");
        return ret;
    }

    ret = livestream_rtsp_message_get_cseq(request, &cseq);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("invalid rtsp request, have no cseq \n");
        return ret;
    }

    rtsp_session->cur_request_seq = cseq;
    switch (method) {
        case LIVESTREAM_RTSP_REQ_METHOD_OPTIONS:
            handle_options(rtsp_session);
            break;
        case LIVESTREAM_RTSP_REQ_METHOD_DISCRIBLE:
            ret = handle_describe(rtsp_session, request);
            break;
        case LIVESTREAM_RTSP_REQ_METHOD_SETUP:
            ret = handle_setup(rtsp_session, request);
            break;
        case LIVESTREAM_RTSP_REQ_METHOD_PLAY:
            ret = handle_play(rtsp_session, request);
            break;
        case LIVESTREAM_RTSP_REQ_METHOD_TEARDOWN:
            ret = handle_teardown(rtsp_session, request);
            break;
        case LIVESTREAM_RTSP_REQ_METHOD_GET_PARAM:
            handle_get_param(rtsp_session);
            break;
        default:
            handle_not_supported(rtsp_session);
            break;
    }

    resp_len = strlen(rtsp_session->response_buf);
    if (resp_len > 0) {
        if (XCAM_SUCCESS != livestream_rtsp_session_send(rtsp_session->session_sockfd, rtsp_session->response_buf, resp_len)) {
            LOG_LIVESTREAM_ERROR("send RTSP response to client fail\n");
            return XCAM_FAILURE;
        }
    }

    memset(rtsp_session->response_buf, 0x00, RTSP_MAX_PROTOCOL_BUFFER);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("rtspserver handle request failed\n");
        return ret;
    }

    update_state(rtsp_session, method);

    return XCAM_SUCCESS;
}

/**
 * @brief handle request.
 * @param[in] rtsp_session : session info, pcRecvBuff : msg from client, buff_len: length of msg
 * @return   0 success
 * @return  -1 failure
 */
static xcam_s32 handle_message(livestream_rtsp_stream_session* rtsp_session, xcam_char* recv_buff, xcam_u32* buff_len)
{
    xcam_bool is_half_flag = XCAM_FALSE;
    xcam_s32 ret = XCAM_SUCCESS;

    if ((XCAM_NULL == rtsp_session) || (XCAM_NULL == recv_buff)) {
        LOG_LIVESTREAM_ERROR("Input argument is null!!!\n");
        return XCAM_FAILURE;
    }

    if (0 != *buff_len) {
        (xcam_void)cut_invaild_message(recv_buff, buff_len);
        ret = cut_rtcp_message(recv_buff, buff_len , &is_half_flag);
        if (XCAM_SUCCESS != ret) {
            /*it is only a rtcp packet*/
            LOG_LIVESTREAM_INFO("it is only a rtcp packet!!!\n");
            return XCAM_SUCCESS;
        }

        if (is_half_flag != XCAM_TRUE) {
            if (XCAM_NULL != strstr(recv_buff, "\r\n\r\n")) {
                /*normal rtsp packet*/
                ret = handle_stream_session_request(rtsp_session, recv_buff);
                if (XCAM_SUCCESS != ret) {
                    LOG_LIVESTREAM_ERROR("Handle Request msg failed ret: %d\n", ret);
                    return XCAM_FAILURE;
                }
                cut_message(recv_buff, buff_len);
            } else {
                /*other invaild packets, not rtcp not response*/
                memset(recv_buff, 0x00, RTSP_MAX_PROTOCOL_BUFFER);
                *buff_len = 0;
            }
        } else {
            if (XCAM_NULL == strstr(recv_buff, "\r\n\r\n")) {
                /*half rtcp packet!*/
                memset(recv_buff, 0x00, RTSP_MAX_PROTOCOL_BUFFER);
                *buff_len = 0;
            }
        }
    }

    if (LIVESTREAM_RTSP_SESSION_STATE_STOP == rtsp_session->session_state) {
        LOG_LIVESTREAM_INFO("RTSP Session state change into stop !!\n");
        return XCAM_SUCCESS;
    }

    return XCAM_SUCCESS;
}

static xcam_void* stream_session_process(xcam_void* arg)
{
    livestream_rtsp_stream_session* rtsp_session = (livestream_rtsp_stream_session*)arg;
    xcam_char recv_buff[RTSP_MAX_PROTOCOL_BUFFER] = {0};
    struct timeval timeout_val;
    xcam_s32 recv_bytes = 0;
    xcam_u32  buff_len = 0;
    xcam_s32 ret = 0;
    xcam_s32 len = 0;
    fd_set read_fds;

    memset(&timeout_val, 0, sizeof(struct timeval));
    timeout_val.tv_sec = RTSP_TRANS_TIMEVAL_SEC;
    timeout_val.tv_usec = 0;
    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, (unsigned long)(uintptr_t)"RtspSessProc", 0, 0, 0);

    /* block Signal Handler SIGPIPE */
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    sigprocmask(SIG_BLOCK, &set, XCAM_NULL);

    len = strlen(rtsp_session->recv_buf);
    if (len <= 0) {
        LOG_LIVESTREAM_ERROR("RECV buffer len not supported\n");
        return XCAM_NULL;
    } else { /*already have first request in buffer to be handle*/
        rtsp_session->recv_buf[RTSP_MAX_PROTOCOL_BUFFER - 1] = '\0';
        strncpy(recv_buff, rtsp_session->recv_buf, RTSP_MAX_PROTOCOL_BUFFER - 1);
        recv_buff[RTSP_MAX_PROTOCOL_BUFFER - 1] = '\0';
        buff_len = buff_len + len;
    }

    time_t LastTimeTick = time(XCAM_NULL);
    time_t CurTimeTick = time(XCAM_NULL);
    /*process RTSP live session*/
    while (LIVESTREAM_RTSP_SESSION_STATE_STOP != rtsp_session->session_state) {
        ret = handle_message(rtsp_session, recv_buff, &buff_len);
        if (XCAM_SUCCESS != ret) {
            LOG_LIVESTREAM_ERROR("Need to break the msg proc looper!!\n");
            break;
        }

        time(&CurTimeTick);
        /*if set the timeout value then check if timeout  and destroy  the session */
        if (rtsp_session->timeout > 0 && CurTimeTick - LastTimeTick > rtsp_session->timeout) {
            LOG_LIVESTREAM_ERROR("client disconnected :[RTSP session timeout %llds]\n",
                (xcam_s64)(CurTimeTick - LastTimeTick));
            break;
        }

        FD_ZERO(&read_fds);
        FD_SET(rtsp_session->session_sockfd, &read_fds);
        refresh_select_time(rtsp_session, recv_buff, &buff_len, &timeout_val);
        ret = select(rtsp_session->session_sockfd + 1, &read_fds, XCAM_NULL, XCAM_NULL, &timeout_val);
        if (ret < 0) {
            if (EINTR == errno || EAGAIN == errno) {
                /*to do: interrupt or try again*/
                continue;
            }

            LOG_LIVESTREAM_ERROR("client disconnected! ret:%x,errno:%d,err:%s\n", ret, errno, strerror(errno));
            break;
        } else if (0 == ret) {
            /*connection establish period, timeout will close session*/
            ret = recv_message_timeout(rtsp_session, recv_buff, &buff_len);
            if (XCAM_SUCCESS != ret) {
                LOG_LIVESTREAM_ERROR("client  disconnected :" \
                                     "[not complete RTSP session,select: overtime %llu s.%llu us)]\n",
                                     (xcam_u64)timeout_val.tv_sec, (xcam_u64)timeout_val.tv_usec);
                break;
            }
        } else if (ret > 0) {
            if (FD_ISSET(rtsp_session->session_sockfd, &read_fds)) {
                memset(rtsp_session->recv_buf, 0, RTSP_MAX_PROTOCOL_BUFFER);
                recv_bytes = recv(rtsp_session->session_sockfd, rtsp_session->recv_buf, RTSP_MAX_PROTOCOL_BUFFER - 1, 0);
                if (recv_bytes <= 0) {
                    if (recv_bytes < 0) {
                        LOG_LIVESTREAM_ERROR("client disconnected: [RTSP msg read from socket: %s]!\n", strerror(errno));
                    }
                    break;
                } else {
                    rtsp_session->recv_buf[RTSP_MAX_PROTOCOL_BUFFER - 1] = '\0';
                    rtsp_session->recv_len = recv_bytes;
                    if (RTSP_MAX_PROTOCOL_BUFFER - buff_len > rtsp_session->recv_len) {
                        memcpy(recv_buff + buff_len, rtsp_session->recv_buf, rtsp_session->recv_len);
                        buff_len = buff_len + rtsp_session->recv_len;
                    } else {
                        LOG_LIVESTREAM_ERROR("the ache buff is over!\n");
                        break;
                    }

                    /*refresh session timeout*/
                    time(&LastTimeTick);
                }
            } else {
                LOG_LIVESTREAM_ERROR("Select readset ok, but socket not in read set, error happen!\n");
                break;
            }
        }

        if (XCAM_SUCCESS != send_media(rtsp_session)) {
            LOG_LIVESTREAM_ERROR("something wrong in send data ret: %d!\n", ret);
            break;
        }
    }   /*end of while()*/

    ret = livestream_rtsp_session_stop(rtsp_session);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("process session stop failed: %d!\n", ret);
    }

    if (rtsp_session->session_listener.session_destroy_func) {
        rtsp_session->session_listener.session_destroy_func(rtsp_session->listener_object, rtsp_session);
    }

    return XCAM_NULL;
}

static xcam_s32 write_video_stream(livestream_rtsp_stream_session* rtsp_session,
                    xcam_track_source_handle track_src, const xcam_livestream_rtsp_data* stream_data)
{
    xcam_mbuf_pack_info pack_info = {0};
    xcam_bool is_idr_found = XCAM_FALSE;
    xcam_s32 ret = XCAM_SUCCESS;
    xcam_u32 frame_len = 0;
    xcam_u32 i = 0;

    if (!stream_data->is_key_frame && !rtsp_session->is_get_key_frame) {
        return ret;
    }

    if (LIVESTREAM_RTSP_SESSION_STATE_INIT == rtsp_session->session_state
      && XCAM_NULL == rtsp_session->media_info.mbuf_handle) {
        LOG_LIVESTREAM_INFO("rtsp session state is INIT! \n");
        return ret;
    }

    for (i = 0; i < stream_data->block_cnt; i++) {
        frame_len += stream_data->data_len[i];
        pack_info.pack_addr[i] = stream_data->data_ptr[i];
        pack_info.pack_size[i] = stream_data->data_len[i];
    }

    pack_info.pack_count = stream_data->block_cnt;
    pack_info.pts = stream_data->pts_us;
    pack_info.seq = stream_data->seq_num;
    pack_info.key_frame = stream_data->is_key_frame;
    pack_info.payload_type = track_src->track_source_attr.video_info.codec_type;
    ret = livestream_mbuffer_write_frame(rtsp_session->media_info.mbuf_handle, &pack_info);
    if (XCAM_SUCCESS == ret) {
        if (!rtsp_session->start_pts && !rtsp_session->is_get_key_frame) {
            rtsp_session->start_pts = pack_info.pts;
        }
    }

    if (XCAM_SUCCESS == ret && stream_data->is_key_frame && !rtsp_session->is_get_key_frame) {
        // read first I frame and parse sps pps for first I frame
        xcam_u8* buffer = XCAM_NULL;
        xcam_u32 i_frame_len = frame_len;
        xcam_u64 pts = 0;
        livestream_mbuffer_data_type data_type = MBUFFER_DATA_BUTT;
        xcam_bool is_key_flag = XCAM_FALSE;

        ret = livestream_mbuffer_read(rtsp_session->media_info.mbuf_handle, (xcam_void**)(&buffer),
                    &i_frame_len, &pts, &data_type, &is_key_flag);

        if (XCAM_SUCCESS != ret) {
            LOG_LIVESTREAM_ERROR("write video stream get first iframe fail\n");
            return ret;
        }

        if (!is_key_flag) {
            LOG_LIVESTREAM_ERROR("Not key frame\n");
            livestream_mbuffer_set(rtsp_session->media_info.mbuf_handle,0);
            return ret;
        }

        // first I frame
        if (XCAM_TRACK_VIDEO_CODEC_H264 == pack_info.payload_type) {
            ret = parse_avc_key_frame(rtsp_session, buffer, i_frame_len, &is_idr_found);
        } else if (XCAM_TRACK_VIDEO_CODEC_H265 == pack_info.payload_type) {
            ret = parse_hevc_key_frame(rtsp_session, buffer, i_frame_len, &is_idr_found);
        }

        livestream_mbuffer_set(rtsp_session->media_info.mbuf_handle, 0);

        if (XCAM_SUCCESS != ret) {
            return ret;
        }

        if (is_idr_found) {
            rtsp_session->is_get_key_frame = XCAM_TRUE;
        }
    }

    return ret;
}

static xcam_s32 write_audio_stream(livestream_rtsp_stream_session* rtsp_session,
                    xcam_track_source_handle track_src, const xcam_livestream_rtsp_data* stream_data)
{
    xcam_mbuf_pack_info pack_info = {0};
    xcam_s32 ret = XCAM_SUCCESS;
    xcam_u32 frame_len = 0;
    xcam_u32 i = 0;

    for (i = 0; i < stream_data->block_cnt; i++) {
        frame_len += stream_data->data_len[i];
        pack_info.pack_addr[i] = stream_data->data_ptr[i];
        pack_info.pack_size[i] = stream_data->data_len[i];
    }
    pack_info.pack_count = stream_data->block_cnt;
    pack_info.pts = stream_data->pts_us;
    pack_info.seq = stream_data->seq_num;
    pack_info.key_frame = XCAM_TRUE;
    pack_info.payload_type = track_src->track_source_attr.audio_info.codec_type;
    if (rtsp_session == XCAM_NULL) {
        return XCAM_FAILURE;
    }
    // if i frame come and audio pts later than video,put in
    if (XCAM_TRUE == rtsp_session->is_get_key_frame && pack_info.pts >= rtsp_session->start_pts) {
        ret = livestream_mbuffer_write_frame(rtsp_session->media_info.mbuf_handle, &pack_info);
    }

    return ret;
}

xcam_s32 livestream_rtsp_session_write_frame(livestream_rtsp_stream_session* rtsp_session,
            xcam_track_source_handle track_src, const xcam_livestream_rtsp_data* stream_data)
{
    xcam_s32 ret = XCAM_SUCCESS;

    // if session stop ,do not need to write,just return
    if (LIVESTREAM_RTSP_SESSION_STATE_STOP == rtsp_session->session_state) {
        return XCAM_SUCCESS;
    }

    if (XCAM_TRACK_SOURCE_TYPE_VIDEO == track_src->track_type) {
        ret = write_video_stream(rtsp_session, track_src, stream_data);
    } else if (XCAM_TRACK_SOURCE_TYPE_AUDIO == track_src->track_type) {
        ret = write_audio_stream(rtsp_session, track_src, stream_data);
    }

    if (XCAM_ERR_MBUF_BUFFER_FULL == ret) {
        ret = LIVESTREAM_ERRNO_MBUF_FULL;
    } else if (XCAM_SUCCESS != ret) {
        ret = LIVESTREAM_ERRNO_WRITE_FRAME;
    }

    return ret;
}

xcam_s32 livestream_rtsp_session_send(xcam_s32 writ_sock, xcam_char* ptr_buff, xcam_u32 data_len)
{
    xcam_char* buffer_pos = XCAM_NULL;
    struct timeval timeout_val;  /* Timeout value */
    xcam_s32 errno_num = 0;
    xcam_u32 rem_size = 0;
    xcam_s32 size = 0;
    xcam_s32 ret = 0;
    fd_set write_fds;

    memset(&timeout_val, 0, sizeof(struct timeval));
    rem_size = data_len;
    buffer_pos = ptr_buff;
    while (rem_size > 0) {
        FD_ZERO(&write_fds);
        FD_SET(writ_sock, &write_fds);
        timeout_val.tv_sec = RTSP_TRANS_TIMEVAL_SEC;
        timeout_val.tv_usec = RTSP_TRANS_TIMEVAL_USEC;
        /*judge if it can send */
        ret = select(writ_sock + 1, XCAM_NULL, &write_fds, XCAM_NULL, &timeout_val);
        if (ret > 0) {
            if (FD_ISSET(writ_sock, &write_fds)) {
                size = send(writ_sock, buffer_pos, rem_size, 0);
                if (size < 0) {
                    /*if it is not eagain error, means can not send*/
                    if (errno != EINTR && errno != EAGAIN) {
                        errno_num = errno;
                        LOG_LIVESTREAM_ERROR("RTSP response Send error:%s\n", strerror(errno_num));
                        return LIVESTREAM_ERRNO_SESS_SEND_FAIL;
                    }

                    /*it is eagain error, means can try again*/
                    continue;
                }

                rem_size -= size;
                buffer_pos += size;
            } else {
                errno_num = errno;
                LOG_LIVESTREAM_ERROR("RTSP response Send error:fd not in fd_set error:%s\n", strerror(errno_num));
                return LIVESTREAM_ERRNO_SESS_SEND_FAIL;
            }
        } else if (ret == 0) { /*select found over time or error happend*/
            errno_num = errno;
            LOG_LIVESTREAM_ERROR("RTSP response Send error:%s  select overtime %d.%ds\n", strerror(errno_num),
                    RTSP_TRANS_TIMEVAL_SEC, RTSP_TRANS_TIMEVAL_USEC);
            return LIVESTREAM_ERRNO_SESS_SEND_FAIL;
        } else if (ret < 0) {
            if (EINTR == errno || EAGAIN == errno) {
                LOG_LIVESTREAM_ERROR("  [select err: %s]\n",  strerror(errno));
                continue;
            }
            errno_num = errno;
            LOG_LIVESTREAM_ERROR("RTSP response Send error:%s\n", strerror(errno_num));
            return LIVESTREAM_ERRNO_SESS_SEND_FAIL;
        }
    }

    return XCAM_SUCCESS;
}

xcam_s32 livestream_rtsp_session_client_connect(livestream_rtsp_stream_session* rtsp_session,
        xcam_char* client_request, xcam_u32 request_len)
{
    xcam_s32 ret = XCAM_SUCCESS;
    xcam_char* ptr_temp = XCAM_NULL;
    struct sockaddr_in sock_addr;
    xcam_s32 addr_len = 0;

    memset(&sock_addr, 0, sizeof(struct sockaddr_in));
    RTSPSVR_CHECK_NULL_ERROR(client_request);
    RTSPSVR_CHECK_NULL_ERROR(rtsp_session);

    /*cache first request into recvbuffer to be handled in threadprocess*/
    memset(rtsp_session->recv_buf, 0, sizeof(rtsp_session->recv_buf));
    strncpy(rtsp_session->recv_buf, client_request, RTSP_MAX_PROTOCOL_BUFFER - 1);
    rtsp_session->recv_buf[RTSP_MAX_PROTOCOL_BUFFER - 1] = '\0';
    rtsp_session->recv_len = request_len;

    addr_len = sizeof(sock_addr);
    if (0 != getpeername(rtsp_session->session_sockfd, (struct sockaddr*)(&sock_addr), (socklen_t*)&addr_len)) {
        LOG_LIVESTREAM_ERROR("getpeername  error.\n");
        return XCAM_FAILURE;
    }

    ptr_temp = inet_ntoa(sock_addr.sin_addr);
    strncpy(rtsp_session->client_ip, ptr_temp, RTSP_IP_MAX_LEN - 1);
    rtsp_session->client_ip[RTSP_IP_MAX_LEN - 1] = '\0';
    ret = pthread_create((&(rtsp_session->session_thdid)), XCAM_NULL, stream_session_process, (xcam_void*)rtsp_session);
    if (ret != XCAM_SUCCESS) {
        LOG_LIVESTREAM_ERROR("Start RTSP session process thread error.\n");
        rtsp_session->session_thdid = RTSP_INVALID_THREAD_ID;
        return  XCAM_FAILURE;
    }

    return XCAM_SUCCESS;
}

xcam_void livestream_rtsp_session_get_client_ipaddr(livestream_rtsp_stream_session* rtsp_session,
        xcam_char* buffer, xcam_s32 buf_len)
{
    if (XCAM_NULL == rtsp_session) {
        LOG_LIVESTREAM_ERROR("param session is null.\n");
        return;
    }

    strncpy(buffer, rtsp_session->client_ip, buf_len - 1);
    buffer[RTSP_IP_MAX_LEN - 1] = '\0';
}

xcam_s32 livestream_rtsp_session_set_listener(livestream_rtsp_stream_session* rtsp_session,
        const livestream_rtsp_session_listener* listener, xcam_void* object)
{
    RTSPSVR_CHECK_NULL_ERROR(listener);
    RTSPSVR_CHECK_NULL_ERROR(listener->session_destroy_func);
    RTSPSVR_CHECK_NULL_ERROR(rtsp_session);
    rtsp_session->session_listener.session_destroy_func = listener->session_destroy_func;
    rtsp_session->listener_object = object;
    return XCAM_SUCCESS;
}

xcam_void livestream_rtsp_session_set_media_source(livestream_rtsp_stream_session* rtsp_session,
        xcam_track_source_handle vid_stream, xcam_track_source_handle aud_stream,
        const xcam_char* stream_name)
{
    rtsp_session->audio_stream = aud_stream;
    rtsp_session->video_stream = vid_stream;
    strncpy(rtsp_session->stream_name, stream_name, RTSP_MAX_STREAMNAME_LEN - 1);
    rtsp_session->stream_name[RTSP_MAX_STREAMNAME_LEN - 1] = '\0';
    return;
}

xcam_s32 livestream_rtsp_session_stop(livestream_rtsp_stream_session* rtsp_session)
{
    xcam_s32 ret = XCAM_SUCCESS;

    if (XCAM_NULL == rtsp_session) {
        LOG_LIVESTREAM_ERROR("param session is null.\n");
        return XCAM_FAILURE;
    }

    if (LIVESTREAM_RTSP_SESSION_STATE_STOP  == rtsp_session->session_state) {
        LOG_LIVESTREAM_INFO("session already stop.\n");
        return XCAM_SUCCESS;
    }

    if (rtsp_session->media_info.video_enable) {
        rtsp_session->media_info.video_enable = XCAM_FALSE;
        ret = remove_video_source(rtsp_session, rtsp_session->video_stream);
        if (XCAM_SUCCESS != ret) {
            LOG_LIVESTREAM_ERROR("remove video source fail ret:0x%X\n", ret);
        }
        livestream_mbuffer_unregister(rtsp_session->media_info.mbuf_handle,
            rtsp_session->media_info.video_source_info.codec_type);
    }

    if (rtsp_session->media_info.audio_enable) {
        rtsp_session->media_info.audio_enable = XCAM_FALSE;
        ret = remove_audio_source(rtsp_session, rtsp_session->audio_stream);
        if (XCAM_SUCCESS != ret) {
            LOG_LIVESTREAM_ERROR("remove audio source fail, ret:0x%X\n", ret);
        }
        livestream_mbuffer_unregister(rtsp_session->media_info.mbuf_handle,
            rtsp_session->media_info.audio_source_info.codec_type);
    }

    (xcam_void)pthread_mutex_lock(&rtsp_session->state_lock);
    rtsp_session->session_state = LIVESTREAM_RTSP_SESSION_STATE_STOP;
    (xcam_void)pthread_mutex_unlock(&rtsp_session->state_lock);

    return XCAM_SUCCESS;
}

xcam_s32 livestream_rtsp_session_destroy(livestream_rtsp_stream_session* rtsp_session)
{
    xcam_s32 ret = XCAM_SUCCESS;

    if (XCAM_NULL == rtsp_session) {
        LOG_LIVESTREAM_ERROR("param session is null.\n");
        return XCAM_FAILURE;
    }

    if (rtsp_session->video_session.track_id != RTSP_INVALID_TRACK_ID) {
        livestream_rtp_session_stop(rtsp_session->video_session.rtp_session);
        if (RTP_TRANS_UDP == rtsp_session->video_session.rtp_session->media_trans_mode) {
            livestream_rtcp_session_stop_udp(rtsp_session->video_session.rtcp_session);
        }
        rtsp_session->video_session.track_id = RTSP_INVALID_TRACK_ID;
        livestream_rtp_session_destroy(rtsp_session->video_session.rtp_session);
        rtsp_session->video_session.rtp_session = XCAM_NULL;
        livestream_rtcp_session_destroy(rtsp_session->video_session.rtcp_session);
        rtsp_session->video_session.rtcp_session = XCAM_NULL;
    }

    if (rtsp_session->audio_session.track_id != RTSP_INVALID_TRACK_ID) {
        livestream_rtp_session_stop(rtsp_session->audio_session.rtp_session);

        if (RTP_TRANS_UDP == rtsp_session->audio_session.rtp_session->media_trans_mode) {
            livestream_rtcp_session_stop_udp(rtsp_session->audio_session.rtcp_session);
        }
        rtsp_session->audio_session.track_id = RTSP_INVALID_TRACK_ID;
        livestream_rtp_session_destroy(rtsp_session->audio_session.rtp_session);
        rtsp_session->audio_session.rtp_session = XCAM_NULL;
        livestream_rtcp_session_destroy(rtsp_session->audio_session.rtcp_session);
        rtsp_session->audio_session.rtcp_session = XCAM_NULL;
    }

    if (XCAM_NULL != rtsp_session->media_info.mbuf_handle) {
        ret = livestream_mbuffer_destroy(&(rtsp_session->media_info.mbuf_info),
                        rtsp_session->media_info.mbuf_handle);
        if (XCAM_SUCCESS != ret) {
            LOG_LIVESTREAM_ERROR("livestream mbuffer destroy fail %X \n", ret);
        }
        rtsp_session->media_info.mbuf_handle = XCAM_NULL;
    }

    pthread_mutex_destroy(&rtsp_session->video_session.mutex_get_port);
    pthread_mutex_destroy(&rtsp_session->audio_session.mutex_get_port);
    pthread_mutex_destroy(&rtsp_session->state_lock);
    livestream_comm_close_socket(&rtsp_session->session_sockfd);
    free(rtsp_session);
    rtsp_session = XCAM_NULL;

    return XCAM_SUCCESS;
}

xcam_s32 livestream_rtsp_session_create(livestream_rtsp_stream_session** pptr_session, xcam_s32 sockfd)
{
    livestream_rtsp_stream_session* rtsp_session = XCAM_NULL;

    rtsp_session = (livestream_rtsp_stream_session*)malloc(sizeof(livestream_rtsp_stream_session));
    if (!rtsp_session) {
        LOG_LIVESTREAM_ERROR("dynamic alloc for XMRTSPStreamSession failed\n");
        return XCAM_FAILURE;
    }

    memset(rtsp_session, 0x00, sizeof(livestream_rtsp_stream_session));
    rtsp_session->session_state = LIVESTREAM_RTSP_SESSION_STATE_INIT;
    rtsp_session->session_thdid = RTSP_INVALID_THREAD_ID;
    rtsp_session->video_session.track_id = RTSP_INVALID_TRACK_ID;
    rtsp_session->audio_session.track_id = RTSP_INVALID_TRACK_ID;
    rtsp_session->is_get_key_frame = XCAM_FALSE;
    rtsp_session->media_info.pps_buf.data_len = MAX_NAL_PARAM_LEN;
    rtsp_session->media_info.sps_buf.data_len = MAX_NAL_PARAM_LEN;
    rtsp_session->media_info.vps_buf.data_len = MAX_NAL_PARAM_LEN;
    rtsp_session->media_info.mbuf_handle = XCAM_NULL;
    rtsp_session->session_sockfd = sockfd;
    /*get session id*/
    memset(rtsp_session->session_id, 0, sizeof(xcam_char) * RTSP_SESSID_MAX_LEN);
    livestream_comm_random_id(rtsp_session->session_id, RTSP_SESSID_MAX_LEN - 1);
    /*get peer host info */
    livestream_comm_get_peer_ip_port(sockfd, rtsp_session->client_ip, &(rtsp_session->client_rtsp_port));
    /*get local host info*/
    livestream_comm_get_host_ipaddr(sockfd, rtsp_session->host_ip);

    (xcam_void)pthread_mutex_init(&rtsp_session->state_lock, XCAM_NULL);
    (xcam_void)pthread_mutex_init(&rtsp_session->video_session.mutex_get_port,  XCAM_NULL);
    (xcam_void)pthread_mutex_init(&rtsp_session->audio_session.mutex_get_port,  XCAM_NULL);
    *pptr_session = rtsp_session;

    return XCAM_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
