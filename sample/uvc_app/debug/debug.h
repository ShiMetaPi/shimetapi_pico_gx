/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __UVC_APP_DEBUG_H__
#define __UVC_APP_DEBUG_H__

void debug_dump_frame(char* data, int size, const char* path);

void debug_dump_stream(char* data, int size, const char* path);

unsigned long long debug_time_stamp_ms(void);

#endif // __UVC_APP_DEBUG_H__
