/*
 * Copyright (c) XCAM. All rights reserved.
 */

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <netinet/tcp.h>
#include "livestream_comm.h"
#include "livestream_listener.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define KEEPALIVE_IDLE_SECONDS (3)
#define LISTEN_PORT_NUM_MIN    0
#define LISTEN_PORT_NUM_MAX    65535
#define LISTEN_TIMEOUT_SEC     (0)
#define LISTEN_TIMEOUT_USEC    (10000)
#define MAX_RECV_BUFFER        (1024)
#define DEFAULT_MAX_FD_NUM     (32)

typedef struct {
    pthread_t listen_thread;
    xcam_s32  listen_sock;
    xcam_u16  listen_port;
    /*state of the listener */
    xcam_bool is_listening;
    xcam_char first_msg_buff[MAX_RECV_BUFFER];
    xcam_s32  client_sock_list[DEFAULT_MAX_FD_NUM];
    client_connect_hook connect_callback;
    xcam_void* callback_object;
} listener_context;

static xcam_s32 listener_set_nonblock(xcam_s32 sockfd)
{
    xcam_u32 flags = 0;
    xcam_s32 ret;

    flags = (xcam_u32)fcntl(sockfd, F_GETFL, 0);
    ret = fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    if (ret < 0) {
        return ret;
    }

    return XCAM_SUCCESS;
}

static xcam_s32 listener_set_keepalive(xcam_s32 sockfd, xcam_s32 keepalive, 
                    xcam_s32 keepidle, xcam_s32 keepinterval, xcam_s32 keepcount)
{
    xcam_s32 ret;
    ret = setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive, sizeof(keepalive));
    if (ret < 0) {
        return ret;
    }
    ret = setsockopt(sockfd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepidle, sizeof(keepidle));
    if (ret < 0) {
        return ret;
    }
    ret = setsockopt(sockfd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepinterval, sizeof(keepinterval));
    if (ret < 0) {
        return ret;
    }
    ret = setsockopt(sockfd, SOL_TCP, TCP_KEEPCNT, (void *)&keepcount, sizeof(keepcount));
    if (ret < 0) {
        return ret;
    }

    return XCAM_SUCCESS;
}

static xcam_s32 listener_add_fd(listener_context* listener_ctx, xcam_s32 sockfd)
{
    xcam_s32 i = 0;

    for (i = 0; i < DEFAULT_MAX_FD_NUM; i++) {
        if (listener_ctx->client_sock_list[i] == INVALID_SOCKET) {
            listener_ctx->client_sock_list[i] = sockfd;
            return XCAM_SUCCESS;
        }
    }

    return XCAM_FAILURE;
}

static xcam_s32 listener_del_fd(listener_context* listener_ctx, xcam_s32 sockfd)
{
    xcam_s32 i = 0;

    for (i = 0; i < DEFAULT_MAX_FD_NUM; i++) {
        if (listener_ctx->client_sock_list[i] == sockfd) {
            listener_ctx->client_sock_list[i] = INVALID_SOCKET;
            return XCAM_SUCCESS;
        }
    }

    return XCAM_FAILURE;
}

static xcam_s32 listener_close_client_fd(listener_context* listener_ctx)
{
    xcam_s32 i = 0;

    for (; i < DEFAULT_MAX_FD_NUM; i++) {
        if (listener_ctx->client_sock_list[i] != INVALID_SOCKET) {
            close(listener_ctx->client_sock_list[i]);
            listener_ctx->client_sock_list[i] = INVALID_SOCKET;
        }
    }

    return XCAM_SUCCESS;
}

static xcam_s32 listener_update_max_fd(const listener_context* listener_ctx, xcam_s32* ptr_maxfd_num)
{
    xcam_s32 maxfd_num = -1;
    xcam_s32 i = 0;

    for (; i < DEFAULT_MAX_FD_NUM; i++) {
        if (listener_ctx->client_sock_list[i] != INVALID_SOCKET 
            && listener_ctx->client_sock_list[i] > maxfd_num) {
            maxfd_num = listener_ctx->client_sock_list[i];
        }
    }

    if (listener_ctx->listen_sock > maxfd_num) {
        maxfd_num = listener_ctx->listen_sock;
    }
    *ptr_maxfd_num = maxfd_num + 1;

    return XCAM_SUCCESS;
}

static xcam_bool listener_check_fd(listener_context* listener_ctx, fd_set* ptr_fdset, xcam_s32* ptr_sockfd)
{
    xcam_s32 i = 0;

    for (; i < DEFAULT_MAX_FD_NUM; i++) {
        if (listener_ctx->client_sock_list[i] != INVALID_SOCKET) {
            xcam_s32 fd = listener_ctx->client_sock_list[i];
            if (FD_ISSET(fd, ptr_fdset)) {
                *ptr_sockfd = fd;
                return XCAM_TRUE;
            }
        }
    }

    return XCAM_FALSE;
}

static xcam_s32 listener_create_socket(xcam_s32 listen_port, xcam_s32* listen_sockfd)
{
    struct timeval timeout = {KEEPALIVE_IDLE_SECONDS, 0};
    struct sockaddr_in svr_addr = {0};
    xcam_s32 sockfd = INVALID_SOCKET;  /*temp listen socket*/
    xcam_s32 opt_value = 1;
    xcam_s32 ret = 0;

    /* 1. Create socket as block mode and listen for accept new connection.*/
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        LOG_LIVESTREAM_ERROR("Create listen sockfd failed! \n");
        return XCAM_FAILURE;
    }

    /*2. set sock option*/
    if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt_value, sizeof(xcam_s32))) {
        close(sockfd);
        LOG_LIVESTREAM_ERROR("set socket option SO_REUSEADDR error.\n");
        return XCAM_FAILURE;
    }

    if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(struct timeval))) {
        close(sockfd);
        LOG_LIVESTREAM_ERROR("set socket option SO_SNDTIMEO error.\n");
        return XCAM_FAILURE;
    }

    if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(struct timeval))) {
        close(sockfd);
        LOG_LIVESTREAM_ERROR("set socket option SO_RCVTIMEO error.\n");
        return XCAM_FAILURE;
    }

    xcam_s32 keepalive = 1;
    xcam_s32 keepidle = KEEPALIVE_IDLE_SECONDS;
    xcam_s32 keepinterval = 1;
    xcam_s32 keepcount = 3;
    ret = listener_set_keepalive(sockfd, keepalive, keepidle, keepinterval, keepcount);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("SetKeepAlive error=%d\n", ret);
    }

    /*3. bind socket */
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_port = htons(listen_port);
    svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(&(svr_addr.sin_zero), '\0', sizeof(svr_addr.sin_zero));
    if (bind(sockfd, (struct sockaddr*)&svr_addr, sizeof(struct sockaddr)) != XCAM_SUCCESS) {
        LOG_LIVESTREAM_ERROR("bind fail error:%s\n", strerror(errno));
        close(sockfd);
        return XCAM_FAILURE;
    }

    /*4. listen on port*/
    ret = listen(sockfd, DEFAULT_MAX_FD_NUM);
    if (ret != XCAM_SUCCESS) {
        LOG_LIVESTREAM_ERROR("socket listen error.\n");
        close(sockfd);
        return XCAM_FAILURE;
    }

    *listen_sockfd = sockfd;
    return XCAM_SUCCESS;
}

/*main func of listen thread */
static xcam_void* listener_thread_process(void* args)
{
    listener_context* listener_ctx = (listener_context*)args;
    struct sockaddr_in accept_addr;  /*socket addr of accepted client connect*/
    struct timeval timeout_val;      /* Timeout value */
    xcam_s32 maxfd_num = 0;                    /*max socket number of read set*/
    xcam_s32 accept_sock = INVALID_SOCKET;   /*accepted client connect socket */
    xcam_s32 sockaddr_len = 0;
    xcam_s32 recv_bytes = 0;
    xcam_s32 ret = XCAM_SUCCESS;
    fd_set read_active_fds;
    fd_set read_fds;

    memset(&timeout_val, 0, sizeof(struct timeval));
    memset(&accept_addr, 0, sizeof(struct sockaddr_in));
    ret = listener_create_socket(listener_ctx->listen_port, &listener_ctx->listen_sock);
    if (ret != XCAM_SUCCESS) {
        LOG_LIVESTREAM_ERROR("create listener sockfd failed error:%d \n", ret);
        return XCAM_NULL;
    }

    prctl(PR_SET_NAME, (unsigned long)"NetlistenProc", 0, 0, 0);
    LOG_LIVESTREAM_INFO("start listener! \n");
    /*init read fds and set http lisen sock into read set*/
    FD_ZERO(&read_fds);
    FD_ZERO(&read_active_fds);
    maxfd_num = listener_ctx->listen_sock + 1;
    FD_SET(listener_ctx->listen_sock, &read_active_fds);
    while (listener_ctx->is_listening) {
        timeout_val.tv_sec = LISTEN_TIMEOUT_SEC;
        timeout_val.tv_usec = LISTEN_TIMEOUT_USEC;
        memcpy(&read_fds, &read_active_fds, sizeof(fd_set));
        /*jude if there is new connect or first message come through already connected link */
        ret = select(maxfd_num, &read_fds, NULL, NULL, &timeout_val);
        if (ret < 0) {
            if (EINTR == errno || EAGAIN == errno) {
                /*to do: interrupt or try again*/
                continue;
            }

            LOG_LIVESTREAM_ERROR("select error! ret:%x,errno:%d,err:%s\n", ret, errno, strerror(errno));
            break;
        } else if (0 == ret) {
            /*to do :select timeout over proset*/
            continue;
        }

        if (FD_ISSET(listener_ctx->listen_sock, &read_fds)) {
            /*accept new connect*/
            sockaddr_len = sizeof(accept_addr);
            accept_sock = accept(listener_ctx->listen_sock, (struct sockaddr*)&accept_addr, (socklen_t*)&sockaddr_len);
            if (accept_sock < 0) {
                LOG_LIVESTREAM_ERROR("accept conn error=%s \r\n", strerror(errno));
                continue;
            }

            // TCP socket may block in many situations, e.g. wifi disconnected
            // so we use nonblock socket and enable TCP keep alive.
            ret = listener_set_nonblock(accept_sock);
            if (XCAM_SUCCESS != ret) {
                LOG_LIVESTREAM_ERROR("SetNonblock error=%d\n", ret);
            }

            ret = listener_set_keepalive(accept_sock, 1, KEEPALIVE_IDLE_SECONDS, 1, 3);
            if (XCAM_SUCCESS != ret) {
                LOG_LIVESTREAM_ERROR("SetKeepAlive error=%d\n", ret);
            }

            LOG_LIVESTREAM_INFO("a new connect has occured fd=%d.\n", accept_sock);
            /*add new connection link into readfds*/
            if (accept_sock + 1 > maxfd_num) {
                maxfd_num = accept_sock + 1;
            }

            FD_SET(accept_sock , &read_active_fds);
            listener_add_fd(listener_ctx, accept_sock);
        } else {
            xcam_s32 clientSockfd = INVALID_SOCKET;
            if (listener_check_fd(listener_ctx, &read_fds, &clientSockfd)) {
                memset(listener_ctx->first_msg_buff, 0, MAX_RECV_BUFFER);
                recv_bytes = recv(clientSockfd, listener_ctx->first_msg_buff, MAX_RECV_BUFFER-1, 0);
                FD_CLR(clientSockfd, &read_active_fds);
                listener_del_fd(listener_ctx, clientSockfd);
                /*updat Max read Fds Num*/
                listener_update_max_fd(listener_ctx, &maxfd_num);
                if (recv_bytes <= 0) {
                    LOG_LIVESTREAM_ERROR("recv data from error=%s\n", strerror(errno));
                    livestream_comm_close_socket(&clientSockfd);
                    continue;
                }

                /*callback to user*/
                if (listener_ctx->connect_callback) {
                    listener_ctx->connect_callback(listener_ctx->callback_object, 
                                clientSockfd, listener_ctx->first_msg_buff, recv_bytes);
                }
            }
        }
    }   /*end of while*/

    /*close the client fd before connect*/
    listener_close_client_fd(listener_ctx);
    /*close the listen socket*/
    livestream_comm_close_socket(&listener_ctx->listen_sock);

    LOG_LIVESTREAM_INFO("stop listener! \n");
    return XCAM_NULL;
}

xcam_s32 livestream_listener_create(xcam_s32 listen_port, xcam_void** ptr_handle)
{
    /* malloc listen server manage struct*/
    listener_context* listener_ctx;
    xcam_s32 i = 0;

    if (listen_port <= LISTEN_PORT_NUM_MIN || listen_port > LISTEN_PORT_NUM_MAX) {
        LOG_LIVESTREAM_ERROR("invalid listen port\n");
        return XCAM_FAILURE;
    }

    listener_ctx = (listener_context*)malloc(sizeof(listener_context));
    if (!listener_ctx) {
        LOG_LIVESTREAM_ERROR("dynamic alloc for lisn server error.\r\n");
        return XCAM_FAILURE;
    }

    memset(listener_ctx, 0x00, sizeof(listener_context));
    for (; i < DEFAULT_MAX_FD_NUM; i++) {
        listener_ctx->client_sock_list[i] = INVALID_SOCKET;
    }

    listener_ctx->listen_port = listen_port;
    *ptr_handle = (xcam_void*)listener_ctx;

    return XCAM_SUCCESS;
}

/*stop the lisen svr*/
xcam_s32 livestream_listener_destroy(xcam_void* handle)
{
    listener_context* listener_ctx = (listener_context*)handle;
    xcam_s32 ret;

    ret = livestream_listener_stop(handle);
    if (XCAM_SUCCESS != ret) {
        LOG_LIVESTREAM_ERROR("livestream_listener_stop failed :%d \n", ret);
        return ret;
    }

    if (listener_ctx) {
        free(listener_ctx);
        listener_ctx = NULL;
    }

    return XCAM_SUCCESS;
}

xcam_s32 livestream_listener_start(xcam_void* handle)
{
    listener_context* listener_ctx;
    xcam_s32 ret;

    listener_ctx = (listener_context*)handle;
    if (XCAM_TRUE == listener_ctx->is_listening) {
        LOG_LIVESTREAM_ERROR("network listener already started in listening\n");
        return XCAM_FAILURE;
    }
    listener_ctx->is_listening = XCAM_TRUE;
    ret = pthread_create(&(listener_ctx->listen_thread), NULL, listener_thread_process, (xcam_void*)listener_ctx);

    return ret;
}

xcam_s32 livestream_listener_stop(xcam_void* handle)
{
    listener_context* listener_ctx = (listener_context*)handle;
    xcam_s32 ret = XCAM_SUCCESS;

    if (!listener_ctx) {
        return XCAM_FAILURE;
    }
    /*set the listen svr stop flag*/
    if (listener_ctx->is_listening) {
        listener_ctx->is_listening = XCAM_FALSE;
        ret = pthread_join(listener_ctx->listen_thread, NULL);
        if (ret != 0) {
            LOG_LIVESTREAM_ERROR("destroy network listener thread error!\n");
            return  ret;
        }
    }

    return XCAM_SUCCESS;
}

xcam_s32 livestream_listener_register_callback(xcam_void* handle,
            client_connect_hook connection_func, xcam_void* object)
{
    listener_context* listener_ctx = (listener_context*)handle;
    listener_ctx->connect_callback = connection_func;
    listener_ctx->callback_object = object;
    return XCAM_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
