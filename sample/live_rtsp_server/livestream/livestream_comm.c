/*
 * Copyright (c) XCAM. All rights reserved.
 */

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdlib.h>
#include "livestream_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifndef UNUSED
#define UNUSED(x) (void)x
#endif

#define IP_MAX_LEN    (16)

static xcam_u8 g_basis64_string[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static xcam_u8 g_basis64_index[128] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,   62, 0xff, 0xff, 0xff,   63,
    52,   53,   54,   55,   56,   57,   58,   59,   60,   61, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff,    0,    1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,
    15,   16,   17,   18,   19,   20,   21,   22,   23,   24,   25, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff,   26,   27,   28,   29,   30,   31,   32,   33,   34,   35,   36,   37,   38,   39,   40,
    41,   42,   43,   44,   45,   46,   47,   48,   49,   50,   51, 0xff, 0xff, 0xff, 0xff, 0xff
};

#define CHAR64(c)  (((c) > 127) ? 0xff : g_basis64_index[(c)])  /* define CHAR64(c) */

xcam_s32 livestream_comm_format_date(xcam_char* date_buf, xcam_s32 buf_size)
{
    if (XCAM_NULL == date_buf) {
        LOG_LIVESTREAM_ERROR("Null pointer [date_buf]!\n");
        return XCAM_FAILURE;
    }
    if (buf_size <= 0) {
        LOG_LIVESTREAM_ERROR("Param buf_size(%d) is invalid!\n", buf_size);
        return XCAM_FAILURE;
    }

    xcam_char buffer[MAX_DATE_LEN] = {0};
    struct tm* ptime = XCAM_NULL;
    time_t tt;

    tt = time(XCAM_NULL);
    ptime = gmtime(&tt);
    if (XCAM_NULL == ptime) {
        LOG_LIVESTREAM_ERROR("gmtime fail!\n");
        return XCAM_FAILURE;
    }

    memset(buffer, '\0', sizeof(buffer));
    if (strftime(buffer, sizeof(buffer), "Date: %a, %b %d %Y %H:%M:%S GMT\r\n", ptime) > 0) {
        strncpy(date_buf, buffer, buf_size);
        return XCAM_SUCCESS;
    }

    return XCAM_FAILURE;
}

xcam_void livestream_comm_random_num(xcam_u32* buf_ssrc)
{
    FILE* fp_file = fopen("/dev/urandom", "r");
    if (XCAM_NULL == fp_file) {
        LOG_LIVESTREAM_ERROR("Open /dev/urandom failed and errno is %d!\n", errno);
        return;
    }

    if (1 != fread(buf_ssrc, sizeof(xcam_u32), 1, fp_file)) {
        LOG_LIVESTREAM_ERROR("Read /dev/urandom failed and errno is %d!\n", errno);
    }

    fclose(fp_file);
    return;
}

xcam_void livestream_comm_random_id(xcam_char* buf_ssrc, xcam_s32 length)
{
    xcam_u32* tmpid;
    xcam_s32 i;
    FILE* file;

    file = fopen("/dev/urandom", "r");
    if (XCAM_NULL == file) {
        LOG_LIVESTREAM_ERROR("Open /dev/urandom failed and errno is %d!\n", errno);
        return;
    }

    if (buf_ssrc == XCAM_NULL) {
        fclose(file);
        LOG_LIVESTREAM_ERROR("Null pointer [buf_ssrc]!\n");
        return;
    }

    tmpid = (xcam_u32 *)malloc(sizeof(xcam_u32));
    if (tmpid == XCAM_NULL) {
        fclose(file);
        LOG_LIVESTREAM_ERROR("Dynamic alloc for random id fail!\n");
        return;
    }

    memset(tmpid, 0x00, sizeof(xcam_u32));
    for (i = 0; i < length; ++i) {
        if (fread(tmpid, sizeof(xcam_u32), 1, file) != 1) {
            LOG_LIVESTREAM_ERROR("Read /dev/urandom failed and errno is %d!\n", errno);
            fclose(file);
            free(tmpid);
            tmpid = XCAM_NULL;
            return;
        } else {
            buf_ssrc[i] = (*tmpid % 10) + '0';
        }
    }

    buf_ssrc[length] = 0;
    fclose(file);
    free(tmpid);
    tmpid = XCAM_NULL;
}

xcam_s32 livestream_comm_base64_encode(const xcam_u8* input_str, xcam_s32 input_len, 
            xcam_u8* output_str, xcam_s32 output_len)
{
    xcam_s32 i = 0;
    xcam_s32 j = 0;
    xcam_s32 pad;

    UNUSED(output_len);
    if (input_str == XCAM_NULL) {
        LOG_LIVESTREAM_ERROR("Null pointer [input_str]!\n");
        return XCAM_FAILURE;
    }
    if (output_str == XCAM_NULL) {
        LOG_LIVESTREAM_ERROR("Null pointer [output_str]!\n");
        return XCAM_FAILURE;
    }

    while (i < input_len) {
        pad = 3 - (input_len - i);
        if (pad == 2) {
            output_str[j  ] = g_basis64_string[input_str[i] >> 2];
            output_str[j + 1] = g_basis64_string[(input_str[i] & 0x03) << 4];
            output_str[j + 2] = '=';
            output_str[j + 3] = '=';
        } else if (pad == 1) {
            output_str[j  ] = g_basis64_string[input_str[i] >> 2];
            output_str[j + 1] = g_basis64_string[((input_str[i] & 0x03) << 4) | ((input_str[i + 1] & 0xf0) >> 4)];
            output_str[j + 2] = g_basis64_string[(input_str[i + 1] & 0x0f) << 2];
            output_str[j + 3] = '=';
        } else {
            output_str[j  ] = g_basis64_string[input_str[i] >> 2];
            output_str[j + 1] = g_basis64_string[((input_str[i] & 0x03) << 4) | ((input_str[i + 1] & 0xf0) >> 4)];
            output_str[j + 2] = g_basis64_string[((input_str[i + 1] & 0x0f) << 2) | ((input_str[i + 2] & 0xc0) >> 6)];
            output_str[j + 3] = g_basis64_string[input_str[i + 2] & 0x3f];
        }

        i += 3;
        j += 4;
    }

    return j;
}

xcam_s32 livestream_comm_base64_decode(const xcam_u8* input_str, xcam_s32 input_len,
                xcam_u8* output_str, xcam_s32 output_len)
{
    xcam_s32 i = 0;
    xcam_s32 j = 0;
    xcam_s32 pad;
    xcam_u8 temp[4];

    UNUSED(output_len);
    if (output_str == XCAM_NULL) {
        LOG_LIVESTREAM_ERROR("Null pointer [output_str]!\n");
        return XCAM_FAILURE;
    }

    while ((i + 3) < input_len) {
        pad = 0;
        temp[0] = CHAR64(input_str[i  ]);
        if (temp[0] == 0xff) {
            pad ++;
        }
        temp[1] = CHAR64(input_str[i + 1]);
        if (temp[1] == 0xff) {
            pad ++;
        }
        temp[2] = CHAR64(input_str[i + 2]);
        if (temp[2] == 0xff) {
            pad ++;
        }
        temp[3] = CHAR64(input_str[i + 3]);
        if (temp[3] == 0xff) {
            pad ++;
        }

        if (pad == 2) {
            output_str[j++] = (temp[0] << 2) | ((temp[1] & 0x30) >> 4);
            output_str[j] = (temp[1] & 0x0f) << 4;
        } else if (pad == 1) {
            output_str[j++] = (temp[0] << 2) | ((temp[1] & 0x30) >> 4);
            output_str[j++] = ((temp[1] & 0x0f) << 4) | ((temp[2] & 0x3c) >> 2);
            output_str[j]   = (temp[2] & 0x03) << 6;
        } else {
            output_str[j++] = (temp[0] << 2) | ((temp[1] & 0x30) >> 4);
            output_str[j++] = ((temp[1] & 0x0f) << 4) | ((temp[2] & 0x3c) >> 2);
            output_str[j++] = ((temp[2] & 0x03) << 6) | (temp[3] & 0x3f);
        }

        i += 4;
    }

    return j;
}

xcam_s32 livestream_comm_close_socket(xcam_s32* sockfd)
{
    if (sockfd == XCAM_NULL) {
        return XCAM_FAILURE;
    }
    if (*sockfd >= 0) {
        close(*sockfd);
        *sockfd = INVALID_SOCKET;
        return XCAM_SUCCESS;
    }

    return XCAM_FAILURE;
}

xcam_s32 livestream_comm_get_peer_ip_port(xcam_s32 sockfd, xcam_char* ip, xcam_u16* port)
{
    xcam_char* ptr_temp = XCAM_NULL;
    socklen_t name_len = 0;
    struct sockaddr_in addr;

    name_len = sizeof(struct sockaddr_in);
    if (0 != getpeername(sockfd, (struct sockaddr*)&addr, &name_len)) {
        return XCAM_FAILURE;
    }

    *port = (xcam_u16)ntohs(addr.sin_port);
    ptr_temp = inet_ntoa(addr.sin_addr);
    if (XCAM_NULL == ptr_temp) {
        LOG_LIVESTREAM_ERROR("ip address inet_ntoa error!\n");
        return XCAM_FAILURE;
    }

    if (snprintf(ip, IP_MAX_LEN - 1, "%s", ptr_temp) < 0) {
        LOG_LIVESTREAM_ERROR("string print ip fail!\n");
        return  XCAM_FAILURE;
    }

    return XCAM_SUCCESS;
}

xcam_s32 livestream_comm_get_peer_sockaddr(xcam_char* ip, xcam_u16 port, struct sockaddr_in* sock_addr)
{
    if (XCAM_NULL == sock_addr) {
        LOG_LIVESTREAM_ERROR("param sock_addr is null!\n");
        return XCAM_FAILURE;
    }

    sock_addr->sin_family = AF_INET;
    sock_addr->sin_port = htons(port);
    memset(&(sock_addr->sin_zero), '\0', sizeof(sock_addr->sin_zero));
    if (0 == inet_aton(ip, (struct in_addr*) & (sock_addr->sin_addr.s_addr))) {
        LOG_LIVESTREAM_ERROR("inet_aton fail!\n");
        return XCAM_FAILURE;
    }

    return XCAM_SUCCESS;
}

xcam_s32 livestream_comm_get_host_ipaddr(xcam_s32 sockfd, xcam_char* ip)
{
    xcam_char* ptr_temp = XCAM_NULL;
    xcam_s32 name_len = 0;
    struct sockaddr_in addr;

    name_len = sizeof(struct sockaddr);
    if (0 != getsockname(sockfd, (struct sockaddr*)&addr, (socklen_t*)&name_len)) {
        return XCAM_FAILURE;
    }

    ptr_temp = inet_ntoa(addr.sin_addr);
    if (XCAM_NULL == ptr_temp) {
        LOG_LIVESTREAM_ERROR("inet_ntoa fail!\n");
        return XCAM_FAILURE;
    }

    if (snprintf(ip, IP_MAX_LEN - 1, "%s", ptr_temp) < 0) {
        LOG_LIVESTREAM_ERROR("string print ip address  error\n");
        return XCAM_FAILURE;
    }

    return XCAM_SUCCESS;
}

xcam_s32 livestream_comm_open_udp_socket(xcam_u16 port)
{
    struct sockaddr_in addr = {0};
    xcam_s32 fd;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        LOG_LIVESTREAM_ERROR("socket udp fail!\n");
        return XCAM_FAILURE;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    memset(&(addr.sin_zero), '\0', sizeof(addr.sin_zero));
    if (bind(fd, (struct sockaddr*)&addr, sizeof (addr))) {
        LOG_LIVESTREAM_ERROR("bind udp socket fail %s!\n", strerror(errno));
        close(fd);
        return XCAM_FAILURE;
    }

    return fd;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
