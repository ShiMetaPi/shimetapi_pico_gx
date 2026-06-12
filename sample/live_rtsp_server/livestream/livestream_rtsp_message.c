/*
 * Copyright (c) XCAM. All rights reserved.
 */

#include <string.h>

#include "livestream_comm.h"
#include "xcam_livestream_rtsp_server.h"
#include "livestream_rtsp_message.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define RTSP_INVALID_STATUS_STR  "Invalid Status"
#define RTSP_INVALID_STATUS_CODE  (-1)

typedef struct {
    xcam_char* status_str;
    xcam_s32 status_code;
} rtsp_msg_status;

static rtsp_msg_status g_rtsp_status[] = {
    {"Continue", 100},
    {"OK", 200},
    {"Created", 201},
    {"Accepted", 202},
    {"Non-Authoritative Information", 203},
    {"No Content", 204},
    {"Reset Content", 205},
    {"Partial Content", 206},
    {"Multiple Choices", 300},
    {"Moved Permanently", 301},
    {"Moved Temporarily", 302},
    {"Bad Request", 400},
    {"Unauthorized", 401},
    {"Payment Required", 402},
    {"Forbidden", 403},
    {"Stream Not Found", 404},
    {"Method Not Allowed", 405},
    {"Not Acceptable", 406},
    {"Proxy Authentication Required", 407},
    {"Request Time-out", 408},
    {"Conflict", 409},
    {"Gone", 410},
    {"Length Required", 411},
    {"Precondition Failed", 412},
    {"Request Entity Too Large", 413},
    {"Request-URI Too Large", 414},
    {"Unsupported Media Type", 415},
    {"Over Supported connection ", 416},
    {"Bad Extension", 420},
    {"Invalid Parameter", 450},
    {"Parameter Not Understood", 451},
    {"Conference Not Found", 452},
    {"Not Enough Bandwidth", 453},
    {"Session Not Found", 454},
    {"Method Not Valid In This State", 455},
    {"Header Field Not Valid for Resource", 456},
    {"Invalid Range", 457},
    {"Parameter Is Read-Only", 458},
    {"Unsupported Transport", 461},
    {"Internal Server Error", 500},
    {"Not Implemented", 501},
    {"Bad Gateway", 502},
    {"Service Unavailable", 503},
    {"Gateway Time-out", 504},
    {"RTSP Version Not Supported", 505},
    {"Option not support", 551},
    {"Extended Error:", 911},
    {0, RTSP_INVALID_STATUS_CODE}
};

/*response header example
RTSP/1.0 200 OK
Server: Streaming Media Server/1.0.0(Jul 30 2015)
Cseq: 1
Public: OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN
*/
static xcam_bool msg_parser_is_response(const xcam_char* request)
{
    xcam_char version[RTSP_VER_MAX_LEN] = {0};
    xcam_u32 stat = 0;
    xcam_s32 cnt = 0;

    memset(version, '\0', sizeof(version));
    cnt = sscanf(request, " %31s %d", version, &stat);
    version[RTSP_VER_MAX_LEN - 1] = '\0';
    if (strncasecmp(version, "RTSP/", strlen("RTSP/"))
        || cnt < RTSP_SCANF_RET_TWO || 0 == stat) {
        return XCAM_FALSE; /* not a response message */
    }

    return XCAM_TRUE;
}

xcam_bool livestream_rtsp_message_check_request(const xcam_char* request)
{
    livestream_rtsp_request_method method;
    xcam_s32 ret = XCAM_SUCCESS;

    /* check for request message. */
    if (msg_parser_is_response(request)) {
        LOG_LIVESTREAM_INFO("msg formate error, not request, just rtsp response\n");
        return XCAM_FALSE;
    }

    /* not a response message, check for method request. */
    ret = livestream_rtsp_message_parse_method(request, &method);
    if (ret != XCAM_SUCCESS) {
        LOG_LIVESTREAM_INFO("method=%s requested was invalid. Message discarded.\n", request);
        return XCAM_FALSE;
    }

    return XCAM_TRUE;
}

xcam_void livestream_rtsp_message_get_response(xcam_s32 status_code, xcam_s32 cseq,
        xcam_char* reply_buf, xcam_u32 buf_len)
{
    xcam_char status_str[MAX_DATE_LEN] = {0};
    xcam_char buffer[MAX_DATE_LEN] = {0};
    int ret;

    memset(status_str, '\0', sizeof(status_str));
    livestream_rtsp_message_status_code_to_str(status_code, status_str, sizeof(status_str)-1);
    memset(buffer, '\0', sizeof(buffer));
    livestream_comm_format_date(buffer, sizeof(buffer)-1);
    ret = snprintf(reply_buf, buf_len - 1,
                    "%s %d %s\r\n"
                    "CSeq: %d\r\n"
                    "Cache-Control: no-cache\r\n"
                    "Server: %s\r\n"
                    "%s\r\n",
                    RTSP_VER_STR,
                    status_code,
                    status_str,
                    cseq,
                    RTSP_SERVER_DESCRIPTION,
                    buffer);
    if (ret < 0) {
        LOG_LIVESTREAM_ERROR("string print reply_buf error\n");
        return;
    }

    return;
}

/*OPTIONS rtsp://ip:554/12 RTSP/1.0
CSeq: 1
Authorization: Basic Og==
User-Agent: Streaming Media Client/1.0.0(May 23 2016)*/
xcam_s32 livestream_rtsp_message_get_stream_name(const xcam_char* request, xcam_char* stream_name, xcam_u32 buf_len)
{
    xcam_char method[RTSP_METHOD_MAX_LEN] = {0};
    xcam_char url[RTSP_URL_MAX_LEN] = {0};
    xcam_char* tmp_ptr = XCAM_NULL;
    xcam_s32 cnt = 0;

    cnt = sscanf(request, " %15s %255s", method, url);
    if (cnt != RTSP_SCANF_RET_TWO) {
        LOG_LIVESTREAM_ERROR("rtsp req format error\n");
        return XCAM_FAILURE;
    }
    method[RTSP_METHOD_MAX_LEN - 1] = '\0';
    url[RTSP_URL_MAX_LEN - 1] = '\0';

    tmp_ptr = strcasestr(url, "rtsp://");
    if (XCAM_NULL == tmp_ptr) {
        LOG_LIVESTREAM_ERROR("rtsp req format error, url do not have rtsp://\n");
        return XCAM_FAILURE;
    }

    tmp_ptr += strlen("rtsp://");
    tmp_ptr = strchr(tmp_ptr, '/');
    if (!tmp_ptr) {
        LOG_LIVESTREAM_ERROR("rtsp req  url do not have stream name\n");
        return XCAM_FAILURE;
    }

    strncpy(stream_name, tmp_ptr + 1, buf_len - 1);
    stream_name[buf_len - 1] = '\0';
    return XCAM_SUCCESS;
}

xcam_s32 livestream_rtsp_message_get_cseq( const xcam_char* request, xcam_s32* ptr_cseq)
{
    xcam_char* ptr_temp = XCAM_NULL;

    ptr_temp = strcasestr(request, RTSP_HEADER_CSEQ);
    if (XCAM_NULL == ptr_temp) {
        LOG_LIVESTREAM_ERROR("there no Cseq in req str\n");
        return XCAM_FAILURE;
    } else {
        if (sscanf(ptr_temp, "%*s %d", ptr_cseq) != RTSP_SCANF_RET_ONE) {
            return XCAM_FAILURE;
        }
    }

    return XCAM_SUCCESS;
}

xcam_s32 livestream_rtsp_message_get_session_id(const xcam_char* ptr_msg_str, xcam_char* ptr_sess_id,
            xcam_u32 sess_id_size)
{
    xcam_char* ptr_temp = XCAM_NULL;

    if (sess_id_size == 0) {
        LOG_LIVESTREAM_ERROR("sessid buf size is 0\n");
        return XCAM_FAILURE;
    }

    /*get sessionid*/
    ptr_temp = strcasestr(ptr_msg_str, RTSP_HEADER_SESSION);
    if ( XCAM_NULL == ptr_temp) {
        LOG_LIVESTREAM_ERROR("there no sessid in req str\n");
        return XCAM_FAILURE;
    } else {
        if (sscanf(ptr_temp, "%*s %15s", ptr_sess_id) != RTSP_SCANF_RET_ONE) {
            return XCAM_FAILURE;
        }
    }
    ptr_sess_id[sess_id_size - 1] = '\0';

    return XCAM_SUCCESS;
}

xcam_s32 livestream_rtsp_message_parse_method(const xcam_char* request, 
            livestream_rtsp_request_method* ptr_method)
{
    xcam_char method[RTSP_METHOD_MAX_LEN] = {0};

    if (RTSP_SCANF_RET_ONE != sscanf(request, "%15[^ ]", method)) {
        return XCAM_FAILURE;
    }
    method[RTSP_METHOD_MAX_LEN - 1] = '\0';

    if (0 == strncmp(method, RTSP_METHOD_DESCRIBE, strlen(RTSP_METHOD_DESCRIBE))) {
        *ptr_method = LIVESTREAM_RTSP_REQ_METHOD_DISCRIBLE;
    } else if (0 == strncmp(method, RTSP_METHOD_SETUP, strlen(RTSP_METHOD_SETUP))) {
        *ptr_method = LIVESTREAM_RTSP_REQ_METHOD_SETUP;
    } else if (0 == strncmp(method, RTSP_METHOD_PLAY, strlen(RTSP_METHOD_PLAY))) {
        *ptr_method = LIVESTREAM_RTSP_REQ_METHOD_PLAY;
    } else if (0 == strncmp(method, RTSP_METHOD_TEARDOWN, strlen(RTSP_METHOD_TEARDOWN))) {
        *ptr_method = LIVESTREAM_RTSP_REQ_METHOD_TEARDOWN;
    } else if (0 == strncmp(method, RTSP_METHOD_OPTIONS, strlen(RTSP_METHOD_OPTIONS))) {
        *ptr_method = LIVESTREAM_RTSP_REQ_METHOD_OPTIONS;
    } else if (0 == strncmp(method, RTSP_METHOD_PAUSE, strlen(RTSP_METHOD_PAUSE))) {
        *ptr_method = LIVESTREAM_RTSP_REQ_METHOD_PAUSE;
    } else if (0 == strncmp(method, RTSP_METHOD_GET_PARAMETER, strlen(RTSP_METHOD_GET_PARAMETER))) {
        *ptr_method = LIVESTREAM_RTSP_REQ_METHOD_GET_PARAM;
    } else {
        LOG_LIVESTREAM_INFO("not supported rtsp request method maybe rtcp in tcp_itlv\n");
        return XCAM_FAILURE;
    }

    return XCAM_SUCCESS;
}

xcam_s32 livestream_rtsp_message_status_code_to_str(xcam_s32 code, xcam_char *out_buf_status,
                xcam_s32 buf_len)
{
    if (out_buf_status == XCAM_NULL || buf_len <= 0) {
        LOG_LIVESTREAM_ERROR("Invalid param %d.\n", code);
        return XCAM_FAILURE;
    }

    rtsp_msg_status* ptr;
    for (ptr = g_rtsp_status; ptr->status_code != RTSP_INVALID_STATUS_CODE; ptr++) {
        if (ptr->status_code == code) {
            strncpy(out_buf_status, ptr->status_str, buf_len);
            return XCAM_SUCCESS;
        }
    }

    LOG_LIVESTREAM_ERROR("Invalid status code %d.\n", code);
    strncpy(out_buf_status, RTSP_INVALID_STATUS_STR, buf_len);
    return XCAM_FAILURE;
}

xcam_u32 livestream_rtsp_message_get_aac_config(xcam_u32 sample_rate, xcam_u32 chan_num)
{
    xcam_u32 config_num = LIVESTREAM_AUDIO_DOUBLECHN_CONFIGNUM_16000;

    if (chan_num == RTSP_AUDIO_SINGLE_CHN) {
        switch (sample_rate) {
            case LIVESTREAM_AUDIO_SAMPLE_RATE_8000:
                config_num = LIVESTREAM_AUDIO_SINGLECHN_CONFIGNUM_8000;
                break;
            case LIVESTREAM_AUDIO_SAMPLE_RATE_16000:
                config_num = LIVESTREAM_AUDIO_SINGLECHN_CONFIGNUM_16000;
                break;
            case LIVESTREAM_AUDIO_SAMPLE_RATE_32000:
                config_num = LIVESTREAM_AUDIO_SINGLECHN_CONFIGNUM_32000;
                break;
            case LIVESTREAM_AUDIO_SAMPLE_RATE_48000:
                config_num = LIVESTREAM_AUDIO_SINGLECHN_CONFIGNUM_48000;
                break;
            default:
                break;
        }
    } else if (chan_num == RTSP_AUDIO_DOUBLE_CHN) {
        switch (sample_rate) {
            case LIVESTREAM_AUDIO_SAMPLE_RATE_8000:
                config_num = LIVESTREAM_AUDIO_DOUBLECHN_CONFIGNUM_8000;
                break;
            case LIVESTREAM_AUDIO_SAMPLE_RATE_16000:
                config_num = LIVESTREAM_AUDIO_DOUBLECHN_CONFIGNUM_16000;
                break;
            case LIVESTREAM_AUDIO_SAMPLE_RATE_32000:
                config_num = LIVESTREAM_AUDIO_DOUBLECHN_CONFIGNUM_32000;
                break;
            case LIVESTREAM_AUDIO_SAMPLE_RATE_48000:
                config_num = LIVESTREAM_AUDIO_DOUBLECHN_CONFIGNUM_48000;
                break;
            default:
                break;
        }
    } else {
        LOG_LIVESTREAM_ERROR("Invalid audio channel %d\n", chan_num);
    }

    return config_num;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
