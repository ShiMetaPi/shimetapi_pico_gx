/*
 * Copyright (c) XCAM. All rights reserved.
 */

#ifndef _LIVESTREAM_RTCP_SESSION_H_
#define _LIVESTREAM_RTCP_SESSION_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct {
    xcam_s32   rtcp_send_sock;
    xcam_s32   client_rtcp_port;
    xcam_s32   server_rtcp_port;
    xcam_u32   seq_num;
    xcam_u32   data_len;
    xcam_s32   packet_len;
    xcam_char* pack_buff;
} livestream_rtcp_session;

xcam_s32 livestream_rtcp_session_start_udp(livestream_rtcp_session* rtcp_session);

xcam_s32 livestream_rtcp_session_stop_udp(livestream_rtcp_session* rtcp_session);

xcam_s32 livestream_rtcp_session_create(livestream_rtcp_session** rtcp_session, xcam_s32 packet_len);

xcam_s32 livestream_rtcp_session_destroy(livestream_rtcp_session* rtcp_session);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*_LIVESTREAM_RTCP_SESSION_H_*/
