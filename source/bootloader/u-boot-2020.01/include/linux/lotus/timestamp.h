// SPDX-License-Identifier: GPL-2.0

#ifndef __LOTUS_TIMESTAMP_H__
#define __LOTUS_TIMESTAMP_H__

#include <config.h>
#include <linux/types.h>


typedef	u64 timestamp_type;

/*
 * timestamps storage arrangement
 * |-----Timestamp Item----|-----Timestamp Item----| ...
 * |stamp|func |line |type |stamp|func |line |type | ...
 */
#pragma pack(4)
typedef struct {
	timestamp_type stamp;
	char *func;
	u32 line;
	u32 type;
} timestamp_item;
#pragma pack()

#define TIME_STAMP(type) timestamp_mark(__func__, __LINE__, type)

void timestamp_mark(const char *func, u32 line, u32 type);
void timestamp_print(u32 type);
void timestamp_clear(void);

void stopwatch_trigger(void);
void stopwatch_clear(void);
void stopwatch_print(void);
#endif /* __LOTUS_TIMESTAMP_H__ */
