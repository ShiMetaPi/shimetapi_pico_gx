/*
 * Copyright (c) XCAM. All rights reserved.
 */

#ifndef _LIVESTREAM_RTP_SESSION_H_
#define _LIVESTREAM_RTP_SESSION_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/*RTP Payload type define rfc3551*/
typedef enum {
    RTP_PT_PCMU             = 0,        /* mu-law G.711Mu*/
    RTP_PT_GSM              = 3,        /* GSM */
    RTP_PT_G723             = 4,        /* G.723 */
    RTP_PT_PCMA             = 8,        /* a-law G.711A*/
    RTP_PT_G722             = 9,        /* G.722 */
    RTP_PT_L16_STEREO       = 10,       /* linear 16, 44.1khz, 2 channel */
    RTP_PT_L16_MONO         = 11,       /* linear 16, 44.1khz, 1 channel */
    RTP_PT_MPA              = 14,       /* mpeg audio */
    RTP_PT_JPEG             = 26,       /* jpeg */
    RTP_PT_H261             = 31,       /* h.261 */
    RTP_PT_MPV              = 32,       /* mpeg video */
    RTP_PT_MP2T             = 33,       /* mpeg2 TS stream */
    RTP_PT_H263             = 34,       /* old H263 encapsulation */

    /*96-127 dynamic*/
    RTP_PT_H264             = 96,       /* xmedia define as h.264 */
    RTP_PT_G726             = 97,       /* xmedia define as G.726 */
    RTP_PT_H265             = 98,       /* xmedia define as h.265 */
    RTP_PT_DATA             = 100,      /* xmedia define as md alarm data*/
    RTP_PT_AMR              = 101,      /* xmedia define as AMR*/
    RTP_PT_MJPEG            = 102,      /* xmedia define as MJPEG */
    RTP_PT_YUV              = 103,      /* xmedia define as YUV*/
    RTP_PT_ADPCM            = 104,      /* xmedia define as ADPCM */
    RTP_PT_AAC              = 105,      /* xmedia define as AAC */
    RTP_PT_SEC              = 106,      /* xmedia define as SEC*/
    RTP_PT_RED              = 107,      /* xmedia define as RED */
    RTP_PT_INVALID          = 128
} rtp_payload_type;

typedef enum {
    PACK_TYPE_RAW = 0,
    PACK_TYPE_RTP,
    PACK_TYPE_RTP_STAP,
    PACK_TYPE_RTP_FUA,
    PACK_TYPE_RTSP_ITLV,
    PACK_TYPE_XM_ITLV,
    PACK_TYPE_RTSP_O_HTTP,
    PACK_TYPE_BUTT
} rtp_pack_type;

typedef enum {
    RTP_TRANS_UDP = 0,
    RTP_TRANS_UDP_ITLV,
    RTP_TRANS_TCP,
    RTP_TRANS_TCP_ITLV,
    RTP_TRANS_BROADCAST,
    RTP_TRANS_BUTT
} rtp_trans_mode;

#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif

/* total 12Bytes */
typedef struct {
#if (BYTE_ORDER == LITTLE_ENDIAN)
    /* byte 0 */
    xcam_u16 cc      : 4;  /* CSRC count */
    xcam_u16 x       : 1;  /* header extension flag */
    xcam_u16 p       : 1;  /* padding flag */
    xcam_u16 version : 2;  /* protocol version */
    /* byte 1 */
    xcam_u16 pt      : 7;  /* payload type */
    xcam_u16 marker  : 1;  /* marker bit */
#elif (BYTE_ORDER == BIG_ENDIAN)
    /* byte 0 */
    xcam_u16 version : 2;  /* protocol version */
    xcam_u16 p       : 1;  /* padding flag */
    xcam_u16 x       : 1;  /* header extension flag */
    xcam_u16 cc      : 4;  /* CSRC count */
    /*byte 1*/
    xcam_u16 marker  : 1;  /* marker bit */
    xcam_u16 pt      : 7;  /* payload type */
#else
#error YOU MUST DEFINE BYTE_ORDER == LITTLE_ENDIAN OR BIG_ENDIAN !
#endif
    /* bytes 2, 3 */
    xcam_u16 seqno  : 16;  /* sequence number */
    /* bytes 4-7 */
    xcam_u32 ts;            /* timestamp in ms */
    /* bytes 8-11 */
    xcam_u32 ssrc;          /* synchronization source */
} rtp_pack_hdr;

/*rtp field packet*/
#define RTP_HDR_SET_VERSION(HDR, val)  ((HDR)->version = val)
#define RTP_HDR_SET_P(HDR, val)        ((HDR)->p       = val)
#define RTP_HDR_SET_X(HDR, val)        ((HDR)->x       = val)
#define RTP_HDR_SET_CC(HDR, val)       ((HDR)->cc      = val)

#define RTP_HDR_SET_M(HDR, val)        ((HDR)->marker  = val)
#define RTP_HDR_SET_PT(HDR, val)       ((HDR)->pt      = val)

#define RTP_HDR_SET_SEQNO(HDR, _sn)    ((HDR)->seqno  = (_sn))
#define RTP_HDR_SET_TS(HDR, _ts)       ((HDR)->ts     = (_ts))

#define RTP_HDR_SET_SSRC(HDR, _ssrc)    ((HDR)->ssrc  = _ssrc)

#define RTP_TRANS_IP_MAX_LEN   (32)

/*rtsp interleaved packet*/
typedef struct {
    xcam_u8  daollar;      /*8, $:dollar sign(24 decimal)*/
    xcam_u8  chn_id;    /*8, channel id*/
    xcam_u16 payload_len;   /*16, payload length*/
    rtp_pack_hdr rtp_head;   /*rtp head*/
} rtsp_itleaved_hdr;

typedef enum {
    RTP_SESSION_STATE_INIT  = 0,
    RTP_SESSION_STATE_READY = 1,
    RTP_SESSION_STATE_PLAY  = 2,
    RTP_SESSION_STATE_STOP  = 3,
    RTP_SESSION_STATE_BUTT
} xcam_rtp_session_state;

typedef struct {
    rtp_pack_type pack_type;
    rtp_trans_mode media_trans_mode;
    xcam_s32   rtp_send_sock;
    xcam_s32   client_rtp_port;
    xcam_s32   server_rtp_port;
    xcam_u32   seq_num;
    xcam_u32   ssrc;
    xcam_u32   data_len;
    xcam_char* pack_buffer;
    xcam_u32   itlv_client_media_chnid;
    xcam_u32   itlv_client_adapt_chnid;
    xcam_rtp_session_state session_state;
    xcam_char client_ip[RTP_TRANS_IP_MAX_LEN];
    struct sockaddr_in client_sockaddr;
} livestream_rtp_session;

xcam_s32 livestream_rtp_session_update_seq_num(livestream_rtp_session* rtp_session, xcam_u32 seq_num);

xcam_s32 livestream_rtp_session_get_packet_and_send_param(livestream_rtp_session* rtp_session, xcam_s32* ptr_write_sock,
            struct sockaddr_in* ptr_peer_sockaddr, xcam_u32* ptr_last_sn,
            rtp_pack_type* ptr_pack_type, xcam_u32* ptr_ssrc);

xcam_s32 livestream_rtp_session_send_data_in_rtsp_itlv(livestream_rtp_session* rtp_session, const xcam_u8* data_addr,
        xcam_u32 data_len, xcam_u32 ts, rtp_payload_type rtp_type, xcam_u32* seq_num, xcam_u32 ssrc,
        xcam_s32 write_sock, struct sockaddr_in* peer_sockaddr, xcam_s32 packet_len);

xcam_s32 livestream_rtp_session_send_data_in_rtp(livestream_rtp_session* session, const xcam_u8* data_addr,
    xcam_u32 data_len, xcam_u32 ts, rtp_payload_type package_type, xcam_u32* seq_num,
    xcam_u32 ssrc, xcam_s32 write_sock, const struct sockaddr_in* peer_sockaddr, xcam_s32 packet_len);

xcam_s32 livestream_rtp_session_start(livestream_rtp_session* rtp_session);

xcam_s32 livestream_rtp_session_stop(livestream_rtp_session* rtp_session);

xcam_s32 livestream_rtp_session_create(livestream_rtp_session** rtp_session, xcam_s32 packet_len);

xcam_s32 livestream_rtp_session_destroy(livestream_rtp_session* rtp_session);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*_LIVESTREAM_RTP_SESSION_H_*/
