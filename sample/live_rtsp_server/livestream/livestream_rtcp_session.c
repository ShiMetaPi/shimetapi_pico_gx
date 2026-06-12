/*
 * Copyright (c) XCAM. All rights reserved.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "livestream_comm.h"
#include "xcam_livestream_rtsp_server.h"
#include "livestream_rtcp_session.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

xcam_s32 livestream_rtcp_session_start_udp(livestream_rtcp_session* rtcp_session)
{
    xcam_s32 sockfd = 0;

    /*send rtcp socket*/
    sockfd = livestream_comm_open_udp_socket(rtcp_session->server_rtcp_port);
    if (INVALID_SOCKET == sockfd) {
        LOG_LIVESTREAM_ERROR("open udp socket failed from port:%d.\n", rtcp_session->server_rtcp_port);
        return XCAM_FAILURE;
    }
    rtcp_session->rtcp_send_sock = sockfd;

    return XCAM_SUCCESS;
}

xcam_s32 livestream_rtcp_session_stop_udp(livestream_rtcp_session* rtcp_session)
{
    if (XCAM_NULL == rtcp_session) {
        LOG_LIVESTREAM_ERROR("param rtcp_session is null\n");
        return XCAM_FAILURE;
    }

    livestream_comm_close_socket(&rtcp_session->rtcp_send_sock);

    return XCAM_SUCCESS;
}

xcam_s32 livestream_rtcp_session_create(livestream_rtcp_session** pptr_session, xcam_s32 packet_len)
{
    if (packet_len <= 0) {
        LOG_LIVESTREAM_ERROR("rtcp_session packlen less than 0! \n");
        return XCAM_FAILURE;
    }

    livestream_rtcp_session* rtcp_session;
    rtcp_session = (livestream_rtcp_session*)malloc(sizeof(livestream_rtcp_session));
    if (!rtcp_session) {
        LOG_LIVESTREAM_ERROR("dynamic alloc for rtcp session failed\n");
        return XCAM_FAILURE;
    }

    memset(rtcp_session, 0x00, sizeof(livestream_rtcp_session));
    rtcp_session->pack_buff = (xcam_char*)malloc(packet_len);
    if (!rtcp_session->pack_buff) {
        LOG_LIVESTREAM_ERROR("dynamic alloc for pack_buff failed\n");
        free(rtcp_session);
        rtcp_session = XCAM_NULL;
        return XCAM_FAILURE;
    }
    memset(rtcp_session->pack_buff, 0x00, packet_len);
    *pptr_session = rtcp_session;

    return XCAM_SUCCESS;
}

xcam_s32 livestream_rtcp_session_destroy(livestream_rtcp_session* rtcp_session)
{
    if (XCAM_NULL == rtcp_session) {
        LOG_LIVESTREAM_ERROR("param rtcp_session is null !\n");
        return XCAM_FAILURE;
    }

    if (rtcp_session->pack_buff) {
        free(rtcp_session->pack_buff);
        rtcp_session->pack_buff = XCAM_NULL;
    }

    free(rtcp_session);
    rtcp_session = XCAM_NULL;

    return XCAM_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
