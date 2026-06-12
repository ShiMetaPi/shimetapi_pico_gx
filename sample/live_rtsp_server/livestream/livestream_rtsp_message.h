/*
 * Copyright (c) XCAM. All rights reserved.
 */

#ifndef _LIVESTREAM_RTSP_MESSAGE_H_
#define _LIVESTREAM_RTSP_MESSAGE_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define RTSP_VER_MAX_LEN    (32)
#define RTSP_METHOD_MAX_LEN (16)
#define RTSP_URL_MAX_LEN    (256)

#define RTSP_MAX_ONLINE_USER     (16)
#define RTSP_IPADDE_LEN          (16)
#define RTSP_TIME_LEN            (32)
#define RTSP_SPS_MAX_LEN         (128)
#define RTSP_PPS_MAX_LEN         (128)
#define RTSP_VPS_MAX_LEN         (128)

#define RTSP_SESSID_MAX_LEN      (16)
#define RTSP_PASSWORD_MAX_LEN    (256)
#define RTSP_USERNAME_MAX_LEN    (256)
#define RTSP_AGENTBUF_MAX_LEN    (128)
#define RTSP_LINE_MAX_LEN        (256)
#define RTSP_TRASH_MAX_LEN       (256)
#define RTSP_RANGE_MAX_LEN       (64)
#define RTSP_OBJ_MAX_LEN         (256)
#define RTSP_UNCARE_MAX_LEN      (256)
#define RTSP_IP_MAX_LEN          (16)
#define RTSP_COOK_MAX_LEN        (64)
#define RTSP_CHAR_MAX_LEN        (32)
#define RTSP_MAX_PROTOCOL_BUFFER (1024)
#define RTSP_SCANF_RET_ONE       (1)
#define RTSP_SCANF_RET_TWO       (2)
#define RTSP_SCANF_RET_THREE     (3)
#define RTSP_AUDIO_SINGLE_CHN    (1)
#define RTSP_AUDIO_DOUBLE_CHN    (2)
#define RTSP_MAX_STREAMNAME_LEN  (128)

#define XMRTSP_VER_STR  "RTSP/1.0"
#define RTSP_LR   "\r"
#define RTSP_LF   "\n"
#define RTSP_LRLF "\r\n"
#define RTSP_SERVER_DESCRIPTION "XCAM RTSP Streaming Media Server/1.0.0"
#define RTSP_METHOD_OPTIONS     "OPTIONS"
#define RTSP_METHOD_DESCRIBE    "DESCRIBE"
#define RTSP_METHOD_SETUP       "SETUP"
#define RTSP_METHOD_PLAY        "PLAY"
#define RTSP_METHOD_PAUSE       "PAUSE"
#define RTSP_METHOD_TEARDOWN     "TEARDOWN"
#define RTSP_METHOD_GET_PARAMETER "GET_PARAMETER"

/* message header keywords */
#define RTSP_HEADER_CONTENTLENGTH "Content-Length"
#define RTSP_HEADER_ACCEPT "Accept"
#define RTSP_HEADER_CONTENTTYPE "Content-Type"
#define RTSP_HEADER_DATE "Date"
#define RTSP_HEADER_CSEQ "CSeq"
#define RTSP_HEADER_SESSION "Session"
#define RTSP_HEADER_TRANSPORT "Transport"
#define RTSP_VER_STR "RTSP/1.0"
#define RTSP_TRACK_ID "trackID="
#define RTSP_SDP_CONTENT_TYPE "application/sdp"
#define RTSP_SUPPORTED_CMD_LIST   "OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN, GET_PARAMETER"

#define RTSP_STATUS_CODE_CONTINUE             (100)
#define RTSP_STATUS_CODE_OK                   (200)
#define RTSP_STATUS_CODE_ACCEPTED             (202)
#define RTSP_STATUS_CODE_BAD_REQUEST          (400)
#define RTSP_STATUS_CODE_UNAUTHORIZED         (401)
#define RTSP_STATUS_CODE_STREAM_NOT_FOUND     (404)
#define RTSP_STATUS_CODE_METHOD_NOT_ALLOWED   (405)
#define RTSP_STATUS_CODE_OVER_SUPPORTED_CONNECTION (416)
#define RTSP_STATUS_CODE_SESSION_NOT_FOUND     (454)
#define RTSP_STATUS_CODE_UNSUPPORT_TRANSPORT   (461)
#define RTSP_STATUS_CODE_INTERNAL_SERVER_ERROR (500)
#define RTSP_STATUS_CODE_SERVICE_UNAVAILIABLE  (503)
#define RTSP_STATUS_CODE_OPTION_UNSUPPORT      (551)

typedef enum {
    /* method codes */
    LIVESTREAM_RTSP_REQ_METHOD_OPTIONS   = 0,
    LIVESTREAM_RTSP_REQ_METHOD_DISCRIBLE = 1,
    LIVESTREAM_RTSP_REQ_METHOD_SETUP     = 2,
    LIVESTREAM_RTSP_REQ_METHOD_PLAY      = 3,
    LIVESTREAM_RTSP_REQ_METHOD_PAUSE     = 4,
    LIVESTREAM_RTSP_REQ_METHOD_TEARDOWN  = 5,
    LIVESTREAM_RTSP_REQ_METHOD_GET_PARAM = 6,
    LIVESTREAM_RTSP_REQ_METHOD_SET_PARAM = 7,
    LIVESTREAM_RTSP_REQ_METHOD_BUTT
} livestream_rtsp_request_method;

/** audio sample rate*/
typedef enum {
    LIVESTREAM_AUDIO_SAMPLE_RATE_8000  = 8000,  /* 8K Sample rate     */
    LIVESTREAM_AUDIO_SAMPLE_RATE_11025 = 11025, /* 11.025K Sample rate*/
    LIVESTREAM_AUDIO_SAMPLE_RATE_16000 = 16000, /* 16K Sample rate    */
    LIVESTREAM_AUDIO_SAMPLE_RATE_22050 = 22050, /* 22.050K Sample rate*/
    LIVESTREAM_AUDIO_SAMPLE_RATE_24000 = 24000, /* 24K Sample rate    */
    LIVESTREAM_AUDIO_SAMPLE_RATE_32000 = 32000, /* 32K Sample rate    */
    LIVESTREAM_AUDIO_SAMPLE_RATE_44100 = 44100, /* 44.1K Sample rate  */
    LIVESTREAM_AUDIO_SAMPLE_RATE_48000 = 48000, /* 48K Sample rate    */
    LIVESTREAM_AUDIO_SAMPLE_RATE_BUTT
} livestream_audio_sample_rate;
/**
aac low profile
8k     single chn 1588  double chn 1590
16k     single chn 1408  double chn  1410
22.05K  single chn 1388  double chn  1390
24K     single chn 1308  double chn  1310
32k     single chn 1288  double chn  1290
44.1k   single chn 1208  double chn  1210
48k   single chn 1188  double chn  1190
*/
typedef enum {
    LIVESTREAM_AUDIO_SINGLECHN_CONFIGNUM_8000  = 1588,   /**< 8K Sample rate config num    */
    LIVESTREAM_AUDIO_SINGLECHN_CONFIGNUM_16000 = 1408,   /* *<16K Sample rate config num      */
    LIVESTREAM_AUDIO_SINGLECHN_CONFIGNUM_22050 = 1388,   /**<22.050K Sample rate config num  */
    LIVESTREAM_AUDIO_SINGLECHN_CONFIGNUM_24000 = 1308,   /* *<24K Sample rate config num      */
    LIVESTREAM_AUDIO_SINGLECHN_CONFIGNUM_32000 = 1288,   /**< 32K Sample rate config num      */
    LIVESTREAM_AUDIO_SINGLECHN_CONFIGNUM_44100 = 1208,   /**< 44.1K Sample rate config num    */
    LIVESTREAM_AUDIO_SINGLECHN_CONFIGNUM_48000 = 1188,   /* *<48K Sample rate config num      */
    LIVESTREAM_AUDIO_SINGLECHN_CONFIGNUM_BUTT
} livestream_audio_singlechn_confignum;

typedef enum {
    LIVESTREAM_AUDIO_DOUBLECHN_CONFIGNUM_8000  = 1590,   /**< 8K Sample rate  config num      */
    LIVESTREAM_AUDIO_DOUBLECHN_CONFIGNUM_16000 = 1410,   /**<16K Sample rate config num     */
    LIVESTREAM_AUDIO_DOUBLECHN_CONFIGNUM_22050 = 1390,   /**< 22.050K Sample rate config num  */
    LIVESTREAM_AUDIO_DOUBLECHN_CONFIGNUM_24000 = 1310,   /**<24K Sample rate  config num     */
    LIVESTREAM_AUDIO_DOUBLECHN_CONFIGNUM_32000 = 1290,   /* *<32K Sample rate   config num    */
    LIVESTREAM_AUDIO_DOUBLECHN_CONFIGNUM_44100 = 1210,   /* *<44.1K Sample rate  config num   */
    LIVESTREAM_AUDIO_DOUBLECHN_CONFIGNUM_48000 = 1190,   /* *<48K Sample rate config num      */
    LIVESTREAM_AUDIO_DOUBLECHN_CONFIGNUM_BUTT
} livestream_audio_doublechn_confignum;

xcam_bool livestream_rtsp_message_check_request(const xcam_char* request);

xcam_void livestream_rtsp_message_get_response(xcam_s32 stat_code, xcam_s32 cseq,
        xcam_char* reply_buf, xcam_u32 buf_len);

xcam_s32 livestream_rtsp_message_get_stream_name(const xcam_char* request,
                xcam_char* stream_name, xcam_u32 buf_len);

xcam_s32 livestream_rtsp_message_get_cseq( const xcam_char* request, xcam_s32* ptr_cseq);

xcam_s32 livestream_rtsp_message_get_session_id(const xcam_char* ptr_msg_str, xcam_char* ptr_sess_id,
    xcam_u32 sess_id_size);

xcam_s32 livestream_rtsp_message_parse_method(const xcam_char* request, livestream_rtsp_request_method* ptr_method);

xcam_s32 livestream_rtsp_message_status_code_to_str(xcam_s32 code, xcam_char *out_buf_status,
                xcam_s32 buf_len);

xcam_u32 livestream_rtsp_message_get_aac_config(xcam_u32 sample_rate, xcam_u32 chan_num);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*_LIVESTREAM_RTSP_MESSAGE_H_*/
