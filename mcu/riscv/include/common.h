
#ifndef __COMMON_H_
#define __COMMON_H_
#include <config.h>
#include <sys/types.h>
#include <io.h>
#include <serial.h>
#include <stdio.h>
#include <compile.h>
#include <core.h>
#include <debug.h>
#include <assert.h>
#include <timer.h>
#include <string.h>
#include <reserved_mem.h>

extern char g_base_irqstack[];
extern char g_top_irqstack[];

extern u32 g_heap_start;
extern u32 g_heap_end;

extern char _start[];
extern char _end[];

extern char __text_start[];
extern char __text_end[];

extern char __rodata_start[];
extern char __rodata_end[];

extern char __data_start[];
extern char __data_end[];

extern char __text2_start[];
extern char __text2_end[];

extern char __rodata2_start[];
extern char __rodata2_end[];

extern char __data2_start[];
extern char __data2_end[];

extern char __bss_start[];
extern char __bss_end[];

extern char __bin_end_pad[];
extern char __bin_end[];

extern char __param_start[];
extern char __param_end[];

u32 start_shell(void);

int run_stage2_func(int (*func)(void));

u64 get_ddr_size(void);

ulong get_reserved_ddr(u32 size);

void stopwatch_trigger(void);

#endif /*__COMMON_H_*/
