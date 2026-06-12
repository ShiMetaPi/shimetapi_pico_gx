/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#include <stdio.h>
#include <sys/time.h>

#include "debug.h"
#include "log_base.h"

void debug_dump_frame(char* data, int size, const char* path)
{
#if DUMP_STREAM_DATA
	unsigned long long ts1, ts2;

	ts1 = debug_time_stamp_ms();

	FILE* fp = fopen(path, "w+");
	if (fp == NULL)
		return;

	fwrite(data, 1, size, fp);

	fclose(fp);
	ts2 = debug_time_stamp_ms();

	LOGD("fwrite %s bytes %d, used %lld ms\n", path, size, ts2 - ts1);
#endif
}

void debug_dump_stream(char* data, int size, const char* path)
{
#if DUMP_STREAM_DATA
	unsigned long long ts1, ts2;

	ts1 = debug_time_stamp_ms();

	FILE* fp = fopen(path, "a+");	
	if (fp == NULL)
		return;

	fwrite(data, 1, size, fp);

	fclose(fp);
	ts2 = debug_time_stamp_ms();

	LOGD("fwrite %s bytes %d, used %lld ms\n", path, size, ts2 - ts1);
#endif
}

unsigned long long debug_time_stamp_ms(void)
{
    struct timeval tv= {0};
    unsigned long long time_stamp;

    gettimeofday(&tv, NULL);
    time_stamp = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    return time_stamp;
}
