/*
 * Copyright (c) XCAM. All rights reserved.
 */

#ifndef _LIVESTREAM_LISTENER_H_
#define _LIVESTREAM_LISTENER_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef xcam_s32 (*client_connect_hook)(xcam_void* object, xcam_s32 link_fd,
                xcam_char* msg_buff, xcam_u32 msg_len);

xcam_s32 livestream_listener_create(xcam_s32 listen_port,  xcam_void** ptr_handle);

xcam_s32 livestream_listener_destroy(xcam_void* handle);

xcam_s32 livestream_listener_start(xcam_void* handle);

xcam_s32 livestream_listener_stop(xcam_void* handle);

xcam_s32 livestream_listener_register_callback(xcam_void* handle, 
                client_connect_hook connection_func, xcam_void* object);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* _LIVESTREAM_LISTENER_H_ */
