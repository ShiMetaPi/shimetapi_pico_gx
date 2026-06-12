/*
 * Copyright (c) XCAM. All rights reserved.
 */

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "livestream_comm.h"
#include "xcam_livestream_rtsp_server.h"
#include "livestream_mbuffer.h"
#include "livestream_rtp_session.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* audio specific header len is 4, see rfc2550*/
static const xcam_u32 g_audio_specific_header_len = 4;

/* increase 128 bytes when current rtp packet buf is over full */
static const xcam_u32 g_rtp_packet_buf_step_len = 128;

#define RTP_VERSION            2
#define RTP_DEFAULT_SSRC       41030
#define RTP_TRANS_TIMEVAL_SEC  (5)
#define RTP_TRANS_TIMEVAL_USEC (0)
#define RTP_TRANS_STREAM_VIDEO (0)
#define RTP_TRANS_STREAM_AUDIO (1)
#define RTP_TRANS_STREAM_DATA  (2)
#define RTP_TRANS_STREAM_MAX   (3)

static xcam_void rtp_packet_hdr(xcam_char* pack_addr, xcam_u32 timestamp, xcam_u32 marker,
                rtp_payload_type payload_type, xcam_u32 ssrc, xcam_u16 last_sn)
{
    rtp_pack_hdr* rtp_hdr = NULL;
    rtp_hdr = (rtp_pack_hdr*)pack_addr;
    RTP_HDR_SET_VERSION(rtp_hdr, RTP_VERSION);
    RTP_HDR_SET_P(rtp_hdr, 0);                  /*no padding*/
    RTP_HDR_SET_X(rtp_hdr, 0);                  /*no extension*/
    RTP_HDR_SET_CC(rtp_hdr, 0);                 /*0 CSRC*/
    RTP_HDR_SET_M(rtp_hdr, marker);
    RTP_HDR_SET_PT(rtp_hdr, payload_type);
    RTP_HDR_SET_SEQNO(rtp_hdr, htons(last_sn));
    RTP_HDR_SET_TS(rtp_hdr, htonl(timestamp));
    RTP_HDR_SET_SSRC(rtp_hdr, htonl(ssrc));
    return;
}

static xcam_s32 rtp_send_to_sockfd(xcam_s32 write_sock, const xcam_char* buffer,
                xcam_u32 data_len, const struct sockaddr_in* peer_sock_addr)
{
    const xcam_char* buffer_pos = NULL;
    struct timeval timeout_val;
    xcam_u32 rem_size = 0;
    xcam_s32 err_num = 0;
    xcam_s32 size = 0;
    xcam_s32 ret = 0;
    fd_set write_fds;

    memset(&timeout_val, 0, sizeof(struct timeval));
    rem_size = data_len;
    buffer_pos = buffer;
    while (rem_size > 0) {
        FD_ZERO(&write_fds);
        FD_SET(write_sock, &write_fds);
        timeout_val.tv_sec = RTP_TRANS_TIMEVAL_SEC;
        timeout_val.tv_usec = RTP_TRANS_TIMEVAL_USEC;
        /*judge if it can send */
        ret = select(write_sock + 1, NULL, &write_fds, NULL, &timeout_val);
        /*select found over time or error happend*/
        if (0 == ret) {
            err_num = errno;
            LOG_LIVESTREAM_ERROR("send media rtp data timeout,errno=%d,%s\n",
                errno, strerror(err_num));
            return XCAM_FAILURE;
        } else if (0 > ret) {
            if (EINTR == errno || EAGAIN == errno) {
                LOG_LIVESTREAM_ERROR("select error: %s\n", strerror(errno));
                continue;
            }
            err_num = errno;
            LOG_LIVESTREAM_ERROR("send media rtp data, select error: %s", strerror(err_num));
            return XCAM_FAILURE;
        }

        if (!FD_ISSET(write_sock, &write_fds)) {
            err_num = errno;
            LOG_LIVESTREAM_ERROR("send media rtp data error: write fd not in fdset! error:%s\n",
                strerror(err_num));
            return XCAM_FAILURE;
        }

        /*select ok and fd isset ok*/
        if (peer_sock_addr == NULL) {
            size = send(write_sock, buffer_pos, rem_size, MSG_DONTWAIT);
        } else {
            size = sendto(write_sock, buffer_pos, rem_size, 0,
                        (struct sockaddr*)peer_sock_addr, sizeof(struct sockaddr));
        }

        if (size < 0) {
            /*if it is not eagain error, means can not send*/
            if (errno != EINTR && errno != EAGAIN) {
                err_num = errno;
                LOG_LIVESTREAM_ERROR("send media data(%u Byte) error: %s\n", data_len, strerror(err_num));
                return XCAM_FAILURE;
            }

            /*it is eagain error, means can try again*/
            continue;
        }

        rem_size -= size;
        buffer_pos += size;
    }

    return XCAM_SUCCESS;
}

static xcam_void rtp_rtsp_itlv_packet(xcam_char* ptr_pack_addr, xcam_s32 channel, 
                    xcam_u32 pack_len, xcam_u32 timestamp, xcam_u32 marker,
                    rtp_payload_type payload_type, xcam_u32 ssrc, xcam_u16 last_sn)
{
    rtsp_itleaved_hdr* ptr_rtsp_itlv_hdr = XCAM_NULL;
    ptr_rtsp_itlv_hdr = (rtsp_itleaved_hdr*)ptr_pack_addr;
    ptr_rtsp_itlv_hdr->daollar = '$';
    ptr_rtsp_itlv_hdr->chn_id  = channel;
    /* rtsp payload len represent the later packet lengthpacket len = rtp head + data len */
    ptr_rtsp_itlv_hdr->payload_len = htons((xcam_u16)(pack_len - sizeof(rtsp_itleaved_hdr) + sizeof(rtp_pack_hdr)));
    /* packet rtp packet */
    rtp_packet_hdr((xcam_char*)(&(ptr_rtsp_itlv_hdr->rtp_head)), timestamp, marker, payload_type, ssrc, last_sn);

    return;
}

/*
FU-A
FU indicator
      +---------------+
      |0|1|2|3|4|5|6|7|
      +-+-+-+-+-+-+-+-+
      |F|NRI|  Type   |
      +---------------+


FU header
      +---------------+
      |0|1|2|3|4|5|6|7|
      +-+-+-+-+-+-+-+-+
      |S|E|R|  Type   |
      +---------------+

      Type   Packet      Type name
      ---------------------------------------------------------
      0      undefined                                    -
      1-23   NAL unit    Single NAL unit packet per H.264
      24     STAP-A     Single-time aggregation packet
      25     STAP-B     Single-time aggregation packet
      26     MTAP16    Multi-time aggregation packet
      27     MTAP24    Multi-time aggregation packet
      28     FU-A      Fragmentation unit
      29     FU-B      Fragmentation unit
      30-31  undefined                                    -
*/

static xcam_s32 rtp_fu_package_header(xcam_char* ptr_header, xcam_u8* ptr_message,
                    xcam_u8 start, xcam_u8 end)
{
    xcam_s32 head_len = 0;
    xcam_u8 nal_type;
    xcam_u8 nri;

    if (XCAM_NULL == ptr_header || XCAM_NULL == ptr_message) {
        LOG_LIVESTREAM_ERROR("ptr_header=%p ptr_message=%p\n", ptr_header, ptr_message);
        fflush(stdout);
        return XCAM_FAILURE;
    }

    if ((1 == start && 1 == end) || start > 1 || end > 1) {
        LOG_LIVESTREAM_ERROR("start=%d, end=%d\n", start, end);
        fflush(stdout);
        return XCAM_FAILURE;
    }

    if ((*ptr_message != 0 || *(ptr_message + 1) != 0 || *(ptr_message + 2) != 0 ||  *(ptr_message + 3) != 1)
        && (*ptr_message != 0 || *(ptr_message + 1) != 0 || *(ptr_message + 2) != 1)) {
        LOG_LIVESTREAM_ERROR("this package is not have NALU!\n");
        fflush(stdout);
        return XCAM_FAILURE;
    }

    if (*ptr_message == 0 && *(ptr_message + 1) == 0 && *(ptr_message + 2) == 0 && *(ptr_message + 3) == 1) {
        head_len = 4;
    } else if (*ptr_message == 0 && *(ptr_message + 1) == 0 && *(ptr_message + 2) == 1) {
        head_len = 3;
    } else {
        fflush(stdout);
        return XCAM_FAILURE;
    }

    nri = (ptr_message[head_len] & 0x60) >> 5;
    nal_type = ptr_message[head_len] & 0x1F;
    *ptr_header = 0;
    *ptr_header = nri << 5 | 0x1c;/*NAL   is 0x1c=28means FUA*/
    *(ptr_header + 1) = 0;
    *(ptr_header + 1) = start << 7 | end << 6 | nal_type;

    return XCAM_SUCCESS;
}

static xcam_s32 rtp_session_start_tcp_itlv(livestream_rtp_session* rtp_session)
{
    if ( RTP_SESSION_STATE_READY == rtp_session->session_state ) {
        rtp_session->session_state = RTP_SESSION_STATE_PLAY;
    } else if ( RTP_SESSION_STATE_PLAY == rtp_session->session_state) {
        /*do nothing*/
    } else {
        LOG_LIVESTREAM_ERROR("start video error: video is in not ready state.\n");
        return XCAM_FAILURE;
    }

    return XCAM_SUCCESS;
}

static xcam_s32 rtp_session_start_udp(livestream_rtp_session* rtp_session)
{
    xcam_s32 temp_sock = 0;

    if (RTP_SESSION_STATE_READY == rtp_session->session_state) {
        /*send rtp socket*/
        temp_sock = livestream_comm_open_udp_socket(rtp_session->server_rtp_port);
        if (-1 == temp_sock) {
            LOG_LIVESTREAM_ERROR("error:get socket from port%d.\n", rtp_session->server_rtp_port);
            return XCAM_FAILURE;
        }
        rtp_session->rtp_send_sock = temp_sock;
        livestream_comm_get_peer_sockaddr(rtp_session->client_ip, rtp_session->client_rtp_port, &(rtp_session->client_sockaddr));
        rtp_session->session_state = RTP_SESSION_STATE_PLAY;
    } else if ( RTP_SESSION_STATE_PLAY == rtp_session->session_state) {
        /*do nothing*/
    } else {
        LOG_LIVESTREAM_ERROR("start video error: video is in not ready state.\n");
        return XCAM_FAILURE;
    }

    return XCAM_SUCCESS;
}

xcam_s32 livestream_rtp_session_update_seq_num(livestream_rtp_session* rtp_session, xcam_u32 seq_num)
{
    /*check the appointed task exit or not*/
    if (XCAM_NULL == rtp_session) {
        LOG_LIVESTREAM_ERROR("get RTP rtp_session NULL.\n");
        return XCAM_FAILURE;
    }
    rtp_session->seq_num = seq_num;

    return XCAM_SUCCESS;
}

xcam_s32 livestream_rtp_session_get_packet_and_send_param(livestream_rtp_session* rtp_session, xcam_s32* ptr_write_sock,
            struct sockaddr_in* ptr_peer_sockaddr, xcam_u32* ptr_last_sn,
            rtp_pack_type* ptr_pack_type, xcam_u32* ptr_ssrc)
{
    xcam_s32 ret = 0;
    rtp_trans_mode trans_mode = RTP_TRANS_BUTT;
    trans_mode = rtp_session->media_trans_mode;

    /*1 get packet type*/
    /*if stream waiting packet is video and vidieo is in playing state,
      stream can be packeted*/

    /*packet the stream according to different packet type */
    *ptr_pack_type = rtp_session->pack_type;
    *ptr_ssrc = rtp_session->ssrc;

    if (RTP_TRANS_TCP_ITLV == trans_mode) {
        *ptr_write_sock = rtp_session->rtp_send_sock;
        *ptr_last_sn = rtp_session->seq_num;
    } else if (RTP_TRANS_UDP == trans_mode) { /*the branch must be udp transport*/
        *ptr_write_sock = rtp_session->rtp_send_sock;
        memcpy(ptr_peer_sockaddr, &(rtp_session->client_sockaddr), sizeof(struct sockaddr_in));
        *ptr_last_sn = rtp_session->seq_num;
    } else if (RTP_TRANS_BROADCAST == trans_mode) {
        *ptr_write_sock = rtp_session->rtp_send_sock;
        memcpy(ptr_peer_sockaddr, &(rtp_session->client_sockaddr), sizeof(struct sockaddr_in));
        *ptr_last_sn = rtp_session->seq_num;
    } else {
        LOG_LIVESTREAM_ERROR("unsupport trans type in send trans_mode:%d.\n", trans_mode);
        return ret;
    }

    return XCAM_SUCCESS;
}

xcam_s32 livestream_rtp_session_send_data_in_rtsp_itlv(livestream_rtp_session* rtp_session, const xcam_u8* data_addr,
        xcam_u32 data_len, xcam_u32 ts, rtp_payload_type rtp_type, xcam_u32* seq_num, xcam_u32 ssrc,
        xcam_s32 write_sock, struct sockaddr_in* peer_sockaddr, xcam_s32 packet_len)
{
    xcam_char* tmp_pack_buf = rtp_session->pack_buffer;
    const xcam_u8* pack_data_addr = data_addr;
    xcam_u32 pack_payload_len = packet_len;
    xcam_u32 current_seq_num = *seq_num;
    xcam_u32 rtp_send_len = 0;
    xcam_u32 package_len = 0;
    xcam_u32 send_len = 0;
    xcam_s32 channel = 0;
    xcam_u8  marker = 0;
    xcam_s32 ret = 0;

    if (pack_data_addr == XCAM_NULL) {
        LOG_LIVESTREAM_ERROR("param pack_data_addr is null\n");
        return XCAM_FAILURE;
    }

    if (rtp_type == RTP_PT_H264) {
        /*juege the slice data  head length*/
        xcam_u32 slice_head_len = 0;

        /*H264  slice data header length*/
        if ((0x00 == (xcam_u8)data_addr[0]) && (0x00 == (xcam_u8)data_addr[1])) {
            if ((0x00 == (xcam_u8)data_addr[2]) && (0x01 == (xcam_u8)data_addr[3])) {
                slice_head_len = 5;
            }

            if (0x01 == (xcam_u8)data_addr[2]) {
                slice_head_len = 4;
            }
        } else {
            LOG_LIVESTREAM_INFO("Unknow H264 Nalu Head:%x %x %x %x %x \n",
                data_addr[0], data_addr[1], data_addr[2], data_addr[3], data_addr[4]);
            slice_head_len = 5;
        }

        channel = rtp_session->itlv_client_media_chnid;

        /*when the frame data length is less than the packetsize 1500*/
        if (data_len - (slice_head_len - 1) + sizeof(rtsp_itleaved_hdr) <= pack_payload_len) {
            send_len = data_len - (slice_head_len - 1);
            rtp_send_len = send_len + sizeof( rtsp_itleaved_hdr );

            /*payload data should cut the 4 bytes head :00 00 00 01*/
            memcpy(tmp_pack_buf + sizeof(rtsp_itleaved_hdr), 
                    pack_data_addr + slice_head_len - 1, send_len);
            /*form the  rtp itlv  head */
            if (0x65 == (xcam_u8)data_addr[slice_head_len - 1] || 0x61 == (xcam_u8)data_addr[slice_head_len - 1]) {
                rtp_rtsp_itlv_packet(tmp_pack_buf, channel, rtp_send_len, ts, 1, rtp_type, ssrc, current_seq_num++);
            } else {
                rtp_rtsp_itlv_packet(tmp_pack_buf, channel, rtp_send_len, ts, 0, rtp_type, ssrc, current_seq_num++);
            }

            /*send the rtp packet */
            ret = rtp_send_to_sockfd(write_sock, tmp_pack_buf, rtp_send_len, peer_sockaddr);
            if (XCAM_SUCCESS != ret) {
                LOG_LIVESTREAM_ERROR("RTP video send error\n");
                return ret;
            }
        } else { /*when the frame data length is larger  than the packetsize 1500 */
            /*need to cut the frame into several rtp packets*/
            while (package_len < data_len) {
                if (pack_payload_len >= sizeof(rtsp_itleaved_hdr) + 2 + data_len - package_len) {
                    send_len = data_len - package_len;
                    marker = 1;
                } else {
                    send_len = pack_payload_len - sizeof(rtsp_itleaved_hdr) - 2;
                    marker = 0;
                }

                rtp_send_len = send_len + sizeof( rtsp_itleaved_hdr ) + 2;
                if (package_len == 0) { /*the first package*/
                    package_len += slice_head_len;
                    pack_data_addr += slice_head_len;
                    ret = rtp_fu_package_header(tmp_pack_buf + sizeof( rtsp_itleaved_hdr ) , (xcam_u8*)data_addr, 1, 0);

                } else if ((package_len + send_len) < data_len) { /*in the middle packages*/
                    ret = rtp_fu_package_header(tmp_pack_buf + sizeof( rtsp_itleaved_hdr ), (xcam_u8*)data_addr, 0, 0);
                    marker = 0;
                } else { /*the last package*/
                    ret = rtp_fu_package_header(tmp_pack_buf + sizeof( rtsp_itleaved_hdr ), (xcam_u8*)data_addr, 0, 1);
                    marker = 1;
                }

                if (XCAM_SUCCESS != ret) {
                    LOG_LIVESTREAM_ERROR("RTP FU Head pack error %d\n", ret);
                    return XCAM_FAILURE;
                }

                /*copy the data */
                memcpy(tmp_pack_buf + sizeof( rtsp_itleaved_hdr ) + 2,
                        pack_data_addr, send_len);
                /*form rtp itlv head*/
                rtp_rtsp_itlv_packet(tmp_pack_buf, channel, rtp_send_len, ts, marker, rtp_type, ssrc, current_seq_num++);
                ret = rtp_send_to_sockfd(write_sock, tmp_pack_buf, rtp_send_len, peer_sockaddr);
                if (XCAM_SUCCESS != ret) {
                    LOG_LIVESTREAM_ERROR("RTP video send error\n");
                    return ret;
                }

                pack_data_addr += send_len; /*update the ptr to the frame data*/
                package_len += send_len;  /*update the send data length*/
            }
        }
    } else if (RTP_PT_H265 == rtp_type) {
        /*juege the slice data  head length*/
        xcam_u32 slice_head_len = 0;

        /*H265  slice data header length*/
        if ((0x00 == (xcam_u8)data_addr[0]) && (0x00 == (xcam_u8)data_addr[1])) {
            if ((0x00 == (xcam_u8)data_addr[2]) && (0x01 == (xcam_u8)data_addr[3])) {
                slice_head_len = 5;
            }

            if (0x01 == (xcam_u8)data_addr[2]) {
                slice_head_len = 4;
            }
        } else {
            LOG_LIVESTREAM_INFO("Unknow H264 Nalu Head:%x %x %x %x %x \n",
                data_addr[0], data_addr[1], data_addr[2], data_addr[3], data_addr[4]);
            slice_head_len = 5;
        }

        int nal_type = (data_addr[slice_head_len - 1] >> 1) & 0x3F;
        channel = rtp_session->itlv_client_media_chnid;

        /*when the frame data length is less  than the packetsize 1500 */
        if (data_len - (slice_head_len - 1) + sizeof(rtsp_itleaved_hdr) <= pack_payload_len) {
            send_len = data_len - (slice_head_len - 1);
            rtp_send_len = send_len + sizeof( rtsp_itleaved_hdr );

            /*payload data should cut the 4 bytes head :00 00 00 01*/
            memcpy(tmp_pack_buf + sizeof(rtsp_itleaved_hdr),
                    pack_data_addr + slice_head_len - 1, send_len);
            if (0x26 == (xcam_u8)data_addr[slice_head_len - 1] || 0x02 == (xcam_u8)data_addr[slice_head_len - 1]) {
                rtp_rtsp_itlv_packet(tmp_pack_buf, channel, rtp_send_len, ts, 1, rtp_type, ssrc, current_seq_num++);
            } else {
                rtp_rtsp_itlv_packet(tmp_pack_buf, channel, rtp_send_len, ts, 0, rtp_type, ssrc, current_seq_num++);
            }

            ret = rtp_send_to_sockfd(write_sock, tmp_pack_buf, rtp_send_len, peer_sockaddr);
            if (XCAM_SUCCESS != ret) {
                LOG_LIVESTREAM_ERROR("RTP video send error\n");
                return ret;
            }
        } else { /*when the frame data length is larger  than the packetsize 1500 */
            tmp_pack_buf[sizeof(rtsp_itleaved_hdr)] = 49 << 1;
            tmp_pack_buf[sizeof(rtsp_itleaved_hdr) + 1] = 1;
            tmp_pack_buf[sizeof(rtsp_itleaved_hdr) + 2] = nal_type;
            /*need to cut the frame into several rtp packets*/

            while (package_len < data_len) {
                if (pack_payload_len >= sizeof(rtsp_itleaved_hdr) + 3 + data_len - package_len) {
                    send_len = data_len - package_len;
                    marker = 1;
                } else {
                    send_len = pack_payload_len - sizeof(rtsp_itleaved_hdr) - 3;
                    marker = 0;
                }

                rtp_send_len = send_len + sizeof( rtsp_itleaved_hdr ) + 3;
                if (package_len == 0) { /*the first package*/
                    package_len += slice_head_len + 1;
                    pack_data_addr += slice_head_len + 1;
                    tmp_pack_buf[sizeof(rtsp_itleaved_hdr) + 2] |= 1 << 7;
                } else if ((package_len + send_len) < data_len) { /* in the middle packages*/
                    marker = 0;
                    tmp_pack_buf[sizeof(rtsp_itleaved_hdr) + 2] &= ~(1 << 7);
                } else { /*the last package*/
                    marker = 1;
                    tmp_pack_buf[sizeof(rtsp_itleaved_hdr) + 2] &= ~(1 << 7);
                    tmp_pack_buf[sizeof(rtsp_itleaved_hdr) + 2] |= 1 << 6;
                }

                memcpy(tmp_pack_buf + sizeof( rtsp_itleaved_hdr ) + 3, pack_data_addr, send_len);
                /*form rtp itlv head*/
                rtp_rtsp_itlv_packet(tmp_pack_buf, channel, rtp_send_len, ts, marker, rtp_type, ssrc, current_seq_num++);
                ret = rtp_send_to_sockfd(write_sock, tmp_pack_buf, rtp_send_len, peer_sockaddr);
                if (XCAM_SUCCESS != ret) {
                    LOG_LIVESTREAM_ERROR("RTP video send error\n");
                    return ret;
                }

                pack_data_addr += send_len; /*update the ptr to the frame data*/
                package_len += send_len;  /*update the send data length*/
            }
        }
    } else if (RTP_PT_PCMA == rtp_type || RTP_PT_PCMU == rtp_type || RTP_PT_G726 == rtp_type) {
        /*audio do not cut into packets*/
        if (data_len >= (pack_payload_len - sizeof( rtsp_itleaved_hdr ))) {
            if (rtp_session->pack_buffer) {
                free(rtp_session->pack_buffer);
                rtp_session->pack_buffer = XCAM_NULL;
            }

            rtp_session->pack_buffer = (xcam_char*)malloc((data_len + 128));
            if (XCAM_NULL == rtp_session->pack_buffer) {
                LOG_LIVESTREAM_ERROR("dynamic alloc fail!\n");
                return XCAM_FAILURE;
            }

            tmp_pack_buf = rtp_session->pack_buffer;
        }

        channel = rtp_session->itlv_client_media_chnid;
        send_len = data_len;
        rtp_send_len = send_len + sizeof(rtsp_itleaved_hdr);
        memcpy(tmp_pack_buf + sizeof(rtsp_itleaved_hdr), pack_data_addr, send_len);
        /*form the rtp head*/
        rtp_rtsp_itlv_packet(tmp_pack_buf, channel, rtp_send_len, ts, 0, rtp_type, ssrc, current_seq_num++);
        /*send the audio data*/
        ret = rtp_send_to_sockfd(write_sock, tmp_pack_buf, rtp_send_len, peer_sockaddr);
        if (XCAM_SUCCESS != ret) {
            LOG_LIVESTREAM_ERROR("RTP audio send error\n");
            return ret;
        }
    } else if (RTP_PT_AAC == rtp_type) {
        if ((data_len - 7) >= (pack_payload_len - sizeof( rtsp_itleaved_hdr ) - 4)) {
            if (rtp_session->pack_buffer) {
                free(rtp_session->pack_buffer);
                rtp_session->pack_buffer = XCAM_NULL;
            }

            rtp_session->pack_buffer = (xcam_char*)malloc((data_len + 128));
            if (XCAM_NULL == rtp_session->pack_buffer) {
                LOG_LIVESTREAM_ERROR("dynamic alloc fail!\n");
                return XCAM_FAILURE;
            }

            tmp_pack_buf = rtp_session->pack_buffer;
        }

        channel = rtp_session->itlv_client_media_chnid;
        /*payload data should cut the first 7 bytes ADTS header */
        send_len = data_len - 7 + 4;
        rtp_send_len = send_len + sizeof( rtsp_itleaved_hdr );

        /*add 4 bytes au header*/
        *(tmp_pack_buf + sizeof( rtsp_itleaved_hdr ))   = 0x00;
        *(tmp_pack_buf + sizeof( rtsp_itleaved_hdr ) + 1) = 0x10;
        *(tmp_pack_buf + sizeof( rtsp_itleaved_hdr ) + 2) = ((data_len - 7) & 0x1fe0) >> 5;
        *(tmp_pack_buf + sizeof( rtsp_itleaved_hdr ) + 3) = ((data_len - 7) & 0x1f) << 3;
        memcpy(tmp_pack_buf + sizeof( rtsp_itleaved_hdr ) + 4,
            pack_data_addr + 7, data_len - 7);
        /*add rtp head */
        rtp_rtsp_itlv_packet(tmp_pack_buf, channel, rtp_send_len, ts, 1, rtp_type, ssrc, current_seq_num++);
        /*send the data */
        ret = rtp_send_to_sockfd(write_sock, tmp_pack_buf, rtp_send_len, peer_sockaddr);
        if (XCAM_SUCCESS != ret) {
            LOG_LIVESTREAM_ERROR("RTP video send error\n");
            return ret;
        }
    } else if (rtp_type == RTP_PT_MPA) {
        if (data_len >= (pack_payload_len - sizeof(rtsp_itleaved_hdr) - g_audio_specific_header_len)) {
            if (rtp_session->pack_buffer) {
                free(rtp_session->pack_buffer);
                rtp_session->pack_buffer = XCAM_NULL;
            }
            rtp_session->pack_buffer = (xcam_char*)malloc((data_len + g_rtp_packet_buf_step_len));
            if (rtp_session->pack_buffer == XCAM_NULL) {
                LOG_LIVESTREAM_ERROR("dynamic alloc fail!\n");
                return XCAM_FAILURE;
            }
            tmp_pack_buf = rtp_session->pack_buffer;
        }

        send_len = data_len + g_audio_specific_header_len;
        rtp_send_len = send_len + sizeof( rtsp_itleaved_hdr );
        xcam_u32 *audioSpecHeader = (xcam_u32 *)(tmp_pack_buf + sizeof(rtsp_itleaved_hdr));
        *audioSpecHeader = 0;/* MBZ is reserved, and frag offset is 0, so audio specific header is 0 */
        memcpy(tmp_pack_buf + sizeof(rtsp_itleaved_hdr) + g_audio_specific_header_len,
            pack_data_addr, data_len);
        marker = 1; /* mpa maker should be 1, see rfc2550 */
        rtp_rtsp_itlv_packet(tmp_pack_buf, channel, rtp_send_len, ts, marker, rtp_type, ssrc, current_seq_num++);
        ret = rtp_send_to_sockfd(write_sock, tmp_pack_buf, rtp_send_len, peer_sockaddr);
        if (ret != XCAM_SUCCESS) {
            LOG_LIVESTREAM_ERROR("RTP video send error\n");
            return ret;
        }
    } else {
        /*media type do not support*/
        LOG_LIVESTREAM_ERROR("unsupport stream type:%d\n", rtp_type);
        return XCAM_FAILURE;
    }

    *seq_num = current_seq_num; /*update the seq when send several packet inside*/
    return XCAM_SUCCESS;
}

xcam_s32 livestream_rtp_session_send_data_in_rtp(livestream_rtp_session* session, const xcam_u8* data_addr,
    xcam_u32 data_len, xcam_u32 ts, rtp_payload_type package_type, xcam_u32* seq_num,
    xcam_u32 ssrc, xcam_s32 write_sock, const struct sockaddr_in* peer_sockaddr, xcam_s32 packet_len)
{
    xcam_char* tmp_pack_buf = session->pack_buffer;
    const xcam_u8* pack_data_addr = data_addr;
    xcam_u32 pack_payload_len = packet_len;
    xcam_u32 current_seq_num = *seq_num;
    xcam_u32 package_len = 0;
    xcam_u32 rtp_send_len = 0;
    xcam_u32 send_len = 0;
    xcam_u8 marker = 0;
    xcam_s32 ret = 0;

    if (RTP_PT_H264 == package_type) {
        xcam_u32 slice_head_len = 0;

        /*H264  head length */
        if ((0x00 == (xcam_u8)data_addr[0]) && (0x00 == (xcam_u8)data_addr[1])) {
            if ((0x00 == (xcam_u8)data_addr[2]) && (0x01 == (xcam_u8)data_addr[3])) {
                slice_head_len = 5;
            }
            if (0x01 == (xcam_u8)data_addr[2]) {
                slice_head_len = 4;
            }
        } else {
            LOG_LIVESTREAM_INFO("Unknow H264 Nalu Head:%x %x %x %x %x \n",
                        data_addr[0], data_addr[1], data_addr[2], data_addr[3], data_addr[4]);
            slice_head_len = 5;
        }

        /*datalen less than packet length*/
        if (data_len - (slice_head_len - 1) + sizeof(rtp_pack_hdr) <= pack_payload_len) {
            send_len = data_len - (slice_head_len - 1);
            rtp_send_len = send_len + sizeof( rtp_pack_hdr );

            /*payload  data cut the first 4 bytres head ::00 00 00 01*/
            memcpy(tmp_pack_buf + sizeof( rtp_pack_hdr ), 
                pack_data_addr + slice_head_len - 1, send_len);
            /*add rtp head*/
            if (0x65 == (xcam_u8)data_addr[slice_head_len - 1] || 0x61 == (xcam_u8)data_addr[slice_head_len - 1]) {
                rtp_packet_hdr(tmp_pack_buf, ts, 1, package_type, ssrc, current_seq_num++);
            } else {
                rtp_packet_hdr(tmp_pack_buf, ts, 0, package_type, ssrc, current_seq_num++);
            }

            /*send the data */
            ret = rtp_send_to_sockfd(write_sock, tmp_pack_buf, rtp_send_len, peer_sockaddr);
            if (XCAM_SUCCESS != ret) {
                LOG_LIVESTREAM_ERROR("RTP video send error\n");
                return ret;
            }
        } else { /*when the frame data is larger than packet length*/
            while (package_len < data_len) {
                if (pack_payload_len >= sizeof(rtp_pack_hdr) + 2 + data_len - package_len) {
                    send_len = data_len - package_len;
                    marker = 1;
                } else {
                    send_len =  pack_payload_len - sizeof(rtp_pack_hdr) - 2;
                    marker = 0;
                }

                rtp_send_len = send_len + sizeof( rtp_pack_hdr ) + 2;
                if (package_len == 0) { /*the first package*/
                    package_len += slice_head_len;
                    pack_data_addr += slice_head_len;
                    ret = rtp_fu_package_header(tmp_pack_buf + sizeof( rtp_pack_hdr ) , (xcam_u8*)data_addr, 1, 0);

                } else if ((pack_payload_len == rtp_send_len) && ((package_len + send_len) < data_len)) { // in the middle packages
                    ret = rtp_fu_package_header(tmp_pack_buf + sizeof( rtp_pack_hdr ), (xcam_u8*)data_addr, 0, 0);
                    marker = 0;
                } else { /*the last package*/
                    ret = rtp_fu_package_header(tmp_pack_buf + sizeof( rtp_pack_hdr ), (xcam_u8*)data_addr, 0, 1);
                    marker = 1;
                }

                if (XCAM_SUCCESS != ret) {
                    LOG_LIVESTREAM_ERROR("RTP FU Head pack error %d\n", ret);
                    return XCAM_FAILURE;
                }

                /*cp  the data*/
                memcpy(tmp_pack_buf + sizeof( rtp_pack_hdr ) + 2, pack_data_addr, send_len);
                /*add the rtp head*/
                rtp_packet_hdr(tmp_pack_buf, ts, marker, package_type, ssrc, current_seq_num++);
                ret = rtp_send_to_sockfd(write_sock, tmp_pack_buf, rtp_send_len, peer_sockaddr);
                if (XCAM_SUCCESS != ret) {
                    LOG_LIVESTREAM_ERROR("RTP video send error\n");
                    return ret;
                }

                pack_data_addr += send_len;
                package_len += send_len;
            }
        }
    } else if (RTP_PT_H265 == package_type) {
        xcam_u32 slice_head_len = 0;

        /*H265  slice head length*/
        if ((0x00 == (xcam_u8)data_addr[0]) && (0x00 == (xcam_u8)data_addr[1])) {
            if ((0x00 == (xcam_u8)data_addr[2]) && (0x01 == (xcam_u8)data_addr[3])) {
                slice_head_len = 5;
            }

            if (0x01 == (xcam_u8)data_addr[2]) {
                slice_head_len = 4;
            }
        } else {
            LOG_LIVESTREAM_INFO("Unknow H265 Nalu Head:%x %x %x %x %x \n",
                data_addr[0], data_addr[1], data_addr[2], data_addr[3], data_addr[4]);
            slice_head_len = 5;
        }

        xcam_s32 nal_type = (data_addr[slice_head_len - 1] >> 1) & 0x3F;
        /*frame date less than the packet length*/
        if (data_len - (slice_head_len - 1) + sizeof(rtp_pack_hdr) <= pack_payload_len) {
            send_len = data_len - (slice_head_len - 1);
            rtp_send_len = send_len + sizeof( rtp_pack_hdr );

            /*payload  should cut the first 4 bytes head :00 00 00 01*/
            memcpy(tmp_pack_buf + sizeof( rtp_pack_hdr ), 
                pack_data_addr + slice_head_len - 1, send_len);
            /*rtpͷ*/
            if (0x26 == (xcam_u8)data_addr[slice_head_len - 1] || 0x02 == (xcam_u8)data_addr[slice_head_len - 1]) {
                rtp_packet_hdr(tmp_pack_buf, ts, 1, package_type, ssrc, current_seq_num++);
            } else {
                rtp_packet_hdr(tmp_pack_buf, ts, 0, package_type, ssrc, current_seq_num++);
            }

            /*send the data */
            ret = rtp_send_to_sockfd(write_sock, tmp_pack_buf, rtp_send_len, peer_sockaddr);
            if (XCAM_SUCCESS != ret) {
                LOG_LIVESTREAM_ERROR("RTP video send error\n");
                return ret;
            }
        } else { /*when the frame date is larger than packet length*/
            tmp_pack_buf[sizeof(rtp_pack_hdr)] = 49 << 1;
            tmp_pack_buf[sizeof(rtp_pack_hdr) + 1] = 1;
            tmp_pack_buf[sizeof(rtp_pack_hdr) + 2] = nal_type;

            while (package_len < data_len) {
                if (pack_payload_len >= sizeof(rtp_pack_hdr) + 3 + data_len - package_len) {
                    send_len = data_len - package_len;
                    marker = 1;
                } else {
                    send_len = pack_payload_len - sizeof(rtp_pack_hdr) - 3;
                    marker = 0;
                }

                rtp_send_len = send_len + sizeof( rtp_pack_hdr ) + 3;
                if (package_len == 0) { /*the first package*/
                    package_len += slice_head_len + 1;
                    pack_data_addr += slice_head_len + 1;
                    tmp_pack_buf[sizeof(rtp_pack_hdr) + 2] |= 1 << 7;
                } else if ((pack_payload_len == rtp_send_len) && ((package_len + send_len) < data_len)) { // in the middle packages
                    marker = 0;
                    tmp_pack_buf[sizeof(rtp_pack_hdr) + 2] &= ~(1 << 7);
                } else { /*the last package*/
                    marker = 1;
                    tmp_pack_buf[sizeof(rtp_pack_hdr) + 2] &= ~(1 << 7);
                    tmp_pack_buf[sizeof(rtp_pack_hdr) + 2] |= 1 << 6;
                }

                /*copy the data */
                memcpy(tmp_pack_buf + sizeof( rtp_pack_hdr ) + 3,
                    pack_data_addr, send_len);
                /*add the rtp head */
                rtp_packet_hdr(tmp_pack_buf, ts, marker, package_type, ssrc, current_seq_num++);
                ret = rtp_send_to_sockfd(write_sock, tmp_pack_buf, rtp_send_len, peer_sockaddr);
                if (XCAM_SUCCESS != ret) {
                    LOG_LIVESTREAM_ERROR("RTP video send error\n");
                    return ret;
                }

                pack_data_addr += send_len;
                package_len += send_len;
            }
        }
    } else if (RTP_PT_PCMA == package_type || RTP_PT_PCMU == package_type || RTP_PT_G726 == package_type) {
        /*audio not cut */
        if (data_len >= (pack_payload_len - sizeof( rtp_pack_hdr ))) {
            if (session->pack_buffer) {
                free(session->pack_buffer );
                session->pack_buffer = XCAM_NULL;
            }

            session->pack_buffer = (xcam_char*)malloc((data_len + 128));
            if (XCAM_NULL == session->pack_buffer) {
                LOG_LIVESTREAM_ERROR("dynamic alloc fail!\n");
                return XCAM_FAILURE;
            }
            tmp_pack_buf = session->pack_buffer;
        }

        send_len = data_len;
        rtp_send_len = send_len + sizeof(rtp_pack_hdr);
        memcpy(tmp_pack_buf + sizeof( rtp_pack_hdr ), pack_data_addr, send_len);
        rtp_packet_hdr(tmp_pack_buf, ts, 0, package_type, ssrc, current_seq_num++);
        ret = rtp_send_to_sockfd(write_sock, tmp_pack_buf, rtp_send_len, peer_sockaddr);
        if (XCAM_SUCCESS != ret) {
            LOG_LIVESTREAM_ERROR("RTP video send error\n");
            return ret;
        }
    } else if (RTP_PT_AAC == package_type) {
        if ((data_len - 7) >= (pack_payload_len - sizeof(rtp_pack_hdr) - 4)) {
            if (session->pack_buffer) {
                free(session->pack_buffer);
                session->pack_buffer = XCAM_NULL;
            }

            session->pack_buffer = (xcam_char*)malloc( (data_len + 128));
            if (XCAM_NULL == session->pack_buffer) {
                LOG_LIVESTREAM_ERROR("dynamic alloc fail!\n");
                return XCAM_FAILURE;
            }
            tmp_pack_buf = session->pack_buffer;
        }

        /*payload cut the first 7 BYTES ADTS head*/
        send_len = data_len - 7 + 4;
        rtp_send_len = send_len + sizeof( rtp_pack_hdr );
        if (tmp_pack_buf == XCAM_NULL) {
            return XCAM_FAILURE;
        }
        /*add 4 bytes  au header*/
        *(tmp_pack_buf + sizeof( rtp_pack_hdr ))   = 0x00;
        *(tmp_pack_buf + sizeof( rtp_pack_hdr ) + 1) = 0x10;
        *(tmp_pack_buf + sizeof( rtp_pack_hdr ) + 2) = ((data_len - 7) & 0x1fe0) >> 5;
        *(tmp_pack_buf + sizeof( rtp_pack_hdr ) + 3) = ((data_len - 7) & 0x1f) << 3;
        memcpy(tmp_pack_buf + sizeof( rtp_pack_hdr ) + 4, pack_data_addr + 7, data_len - 7);
        /*add  rtp head */
        rtp_packet_hdr(tmp_pack_buf, ts, 1, package_type, ssrc, current_seq_num++);
        /*send the data */
        ret = rtp_send_to_sockfd(write_sock, tmp_pack_buf, rtp_send_len, peer_sockaddr);
        if (XCAM_SUCCESS != ret) {
            LOG_LIVESTREAM_ERROR("RTP video send error\n");
            return ret;
        }
    } else if (package_type == RTP_PT_MPA) {
        if (data_len >= (pack_payload_len - sizeof(rtp_pack_hdr) - g_audio_specific_header_len)) {
            if (session->pack_buffer) {
                free(session->pack_buffer);
                session->pack_buffer = XCAM_NULL;
            }
            session->pack_buffer = (xcam_char*)malloc((data_len + g_rtp_packet_buf_step_len));
            if (session->pack_buffer == XCAM_NULL) {
                LOG_LIVESTREAM_ERROR("dynamic alloc fail!\n");
                return XCAM_FAILURE;
            }
            tmp_pack_buf = session->pack_buffer;
        }

        send_len = data_len + g_audio_specific_header_len;
        rtp_send_len = send_len + sizeof( rtp_pack_hdr );
        xcam_u32 *audioSpecHeader = (xcam_u32 *)(tmp_pack_buf + sizeof(rtp_pack_hdr));
        *audioSpecHeader = 0;/* MBZ is reserved, and frag offset is 0, so audio specific header is 0 */
        memcpy(tmp_pack_buf + sizeof(rtp_pack_hdr) + g_audio_specific_header_len,
            pack_data_addr, data_len);
        marker = 1; /* mpa maker should be 1, see rfc2550 */
        rtp_packet_hdr(tmp_pack_buf, ts, marker, package_type, ssrc, current_seq_num++);
        ret = rtp_send_to_sockfd(write_sock, tmp_pack_buf, rtp_send_len, peer_sockaddr);
        if (ret != XCAM_SUCCESS) {
            LOG_LIVESTREAM_ERROR("RTP video send error\n");
            return ret;
        }
    } else {
        /*not support type */
        LOG_LIVESTREAM_ERROR("unsupport stream type:%d\n", package_type);
        return XCAM_FAILURE;
    }

    *seq_num = current_seq_num; /*update the seq num */
    return XCAM_SUCCESS;
}

xcam_s32 livestream_rtp_session_start(livestream_rtp_session* rtp_session)
{
    xcam_s32 s32Ret = XCAM_SUCCESS;

    if (XCAM_NULL == rtp_session) {
        return XCAM_FAILURE;
    }

    /*if it is tcp trans type */
    if (RTP_TRANS_TCP_ITLV == rtp_session->media_trans_mode) {
        /*do not need create the socke while tcp_itlv cause the data trans sock is the same with session sock*/
        s32Ret = rtp_session_start_tcp_itlv(rtp_session);
        if (XCAM_SUCCESS != s32Ret) {
            LOG_LIVESTREAM_ERROR("start task rtp_session_start_tcp_itlv fail %d.\n", s32Ret);
            return s32Ret;
        }
    } else if (RTP_TRANS_UDP == rtp_session->media_trans_mode) {
        /*if it is udp trans type, record corresponding info*/
        s32Ret = rtp_session_start_udp(rtp_session);
        if (XCAM_SUCCESS != s32Ret) {
            LOG_LIVESTREAM_ERROR("start task rtp_session_start_udp fail %d.\n", s32Ret);
            return s32Ret;
        }
    } else if (RTP_TRANS_TCP == rtp_session->media_trans_mode) {
        /*if it is tcp trans type, record corresponding info*/
        /*to do :add the part for tcp trans*/
        LOG_LIVESTREAM_ERROR("start rtpSession error: not support tcp trans now.\n");
        return XCAM_FAILURE;
    } else {
        LOG_LIVESTREAM_ERROR("start rtpSession error: unrecognized trans now.\n");
        return XCAM_FAILURE;
    }

    return XCAM_SUCCESS;
}

xcam_s32 livestream_rtp_session_stop(livestream_rtp_session* rtp_session)
{
    if (XCAM_NULL == rtp_session) {
        LOG_LIVESTREAM_ERROR("Stop rtpSession error.\n");
        return XCAM_FAILURE;
    }

    /*set all meida state as init*/
    rtp_session->session_state = RTP_SESSION_STATE_STOP;

    /*if it is tcp trans type*/
    if (RTP_TRANS_TCP_ITLV == rtp_session->media_trans_mode) {
        /*do nothing*/
    } else if (RTP_TRANS_UDP == rtp_session->media_trans_mode) {   
        /*if it is udp trans type, record corresponding info*/
        livestream_comm_close_socket(&rtp_session->rtp_send_sock);
    } else if (RTP_TRANS_BROADCAST == rtp_session->media_trans_mode) {
        /*if it is broadcast trans type, record corresponding info*/
        livestream_comm_close_socket(&rtp_session->rtp_send_sock);
    } else {
        LOG_LIVESTREAM_ERROR(
            "top trans task error: not unrecognized trans type %d.\n", rtp_session->media_trans_mode);
        return XCAM_FAILURE;
    }

    return XCAM_SUCCESS;
}

xcam_s32 livestream_rtp_session_create(livestream_rtp_session** pptr_session, xcam_s32 packet_len)
{
    livestream_rtp_session* ptr_rtp_session = XCAM_NULL;

    if (packet_len <= 0) {
        LOG_LIVESTREAM_ERROR("ptr_rtp_session packlen less than 0\n");
        return XCAM_FAILURE;
    }

    ptr_rtp_session = (livestream_rtp_session*)malloc(sizeof(livestream_rtp_session));
    if (!ptr_rtp_session) {
        LOG_LIVESTREAM_ERROR("dynamic alloc for ptr_rtp_session failed\n");
        return XCAM_FAILURE;
    }
    memset(ptr_rtp_session, 0x00, sizeof(livestream_rtp_session));
    ptr_rtp_session->pack_buffer = (xcam_char*)malloc(packet_len);
    if (!ptr_rtp_session->pack_buffer) {
        LOG_LIVESTREAM_ERROR("dynamic alloc for pack_buffer failed\n");
        free(ptr_rtp_session);
        ptr_rtp_session = XCAM_NULL;
        return XCAM_FAILURE;
    }
    memset(ptr_rtp_session->pack_buffer, 0x00, packet_len);
    *pptr_session = ptr_rtp_session;

    return XCAM_SUCCESS;
}

xcam_s32 livestream_rtp_session_destroy(livestream_rtp_session* rtp_session)
{
    if (XCAM_NULL ==  rtp_session) {
        LOG_LIVESTREAM_ERROR("param rtp_session  is null !\n");
        return XCAM_FAILURE;
    }

    if (rtp_session->pack_buffer) {
        free(rtp_session->pack_buffer);
        rtp_session->pack_buffer = XCAM_NULL;
    }

    free(rtp_session);
    rtp_session = XCAM_NULL;

    return XCAM_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
