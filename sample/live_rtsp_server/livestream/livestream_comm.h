/*
 * Copyright (c) XCAM. All rights reserved.
 */

#ifndef _LIVESTREAM_COMMON_H_
#define _LIVESTREAM_COMMON_H_

#include <netinet/in.h>
#include <sys/types.h>
#include <stdio.h>
#include <limits.h>
#include <pthread.h>
#include <time.h>
#include "xcam_type.h"
#include "xcam_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define LOG_LIVESTREAM_INFO(fmt...)  XCAM_LOG_INFO_PRINT("livestream", fmt);
#define LOG_LIVESTREAM_WARN(fmt...)  XCAM_LOG_WARN_PRINT("livestream", fmt);
#define LOG_LIVESTREAM_ERROR(fmt...) XCAM_LOG_ERR_PRINT("livestream", fmt);
#define LOG_LIVESTREAM_FATAL(fmt...) XCAM_LOG_FATAL_PRINT("livestream", fmt);

#define MAX_DATE_LEN   (200)
#define MIN_SEND_PORT  (5000)
#define MAX_SEND_PORT  (6000)
#define INVALID_SOCKET (-1)

xcam_s32 livestream_comm_format_date(xcam_char* date_buf, xcam_s32 buf_size);

xcam_void livestream_comm_random_num(xcam_u32* buf_ssrc);

xcam_void livestream_comm_random_id(xcam_char* buf_ssrc, xcam_s32 length);

xcam_s32 livestream_comm_base64_encode(const xcam_u8* input_str, xcam_s32 input_len,
                xcam_u8* output_str, xcam_s32 output_len);

xcam_s32 livestream_comm_base64_decode(const xcam_u8* input_str, xcam_s32 input_len,
                xcam_u8* output_str, xcam_s32 output_len);

xcam_s32 livestream_comm_get_peer_ip_port(xcam_s32 sockfd, xcam_char* ip, xcam_u16* port);

xcam_s32 livestream_comm_get_peer_sockaddr(xcam_char* ip, xcam_u16 port, struct sockaddr_in* sock_addr);

xcam_s32 livestream_comm_get_host_ipaddr(xcam_s32 sockfd, xcam_char* ip);

xcam_s32 livestream_comm_open_udp_socket(xcam_u16 port);

xcam_s32 livestream_comm_close_socket(xcam_s32* sockfd);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*_LIVESTREAM_COMMON_H_*/
