
#include <string.h>
#include <debug.h>
#include <common.h>

#undef __RSV_MEM_DEBUG

struct rsv_memctrl_t {
	uint32 start;
	uint32 end;
	uint32 pos;
};

static struct rsv_memctrl_t g_rsv_memctrl = {0};

void rsv_mem_init(uint32 start, uint32 len)
{
	if (g_rsv_memctrl.end == 0) {
		g_rsv_memctrl.start = start;
		g_rsv_memctrl.end = start + len;
		g_rsv_memctrl.pos = 0;
	}

#ifdef CONFIG_DEBUG_INFO
	puts("Reserved mem: 0x");puthex(start);puts(" - 0x");puthex(start + len);puts("\n");
#endif
}

void rsv_mem_reinit(uint32 start, uint32 len)
{
	g_rsv_memctrl.end = 0;
	rsv_mem_init(start, len);
}

void *__rsv_mem_alloc(uint32 bytes, const char *file, int line)
{
	uint32 new_ptr;
	void *ptr = NULL;
	if (g_rsv_memctrl.end == 0) {
		pr_error("malloc module uninitializtion.\n");
		return NULL;
	}

	new_ptr = g_rsv_memctrl.start + g_rsv_memctrl.pos + around(bytes, 4); /* 4 bytes align */
	if (new_ptr >  g_rsv_memctrl.end) {
		return NULL;
	}
	ptr = (void *)(uintptr_t)(g_rsv_memctrl.start + g_rsv_memctrl.pos);
	g_rsv_memctrl.pos += around(bytes, 4); /* 4 bytes align */
#ifdef __RSV_MEM_DEBUG
	puts(file);puts("(");putdec(line);puts("): rsv mem alloc 0x");puthex((ulong)ptr);puts("(");putdec(bytes);puts(")\n");
#endif
	return ptr;
}

void *__rsv_mem_align(uint32 alignment, uint32 bytes, const char *file, int line)
{
	uint32 new_addr;
	void *ptr = NULL;

	if (g_rsv_memctrl.end == 0) {
		puts("rsv mem NOT initialized.\n");
		return NULL;
	}

	if (alignment & (alignment - 1)) {
		puts("align must be powed of 2\n");
		return NULL;
	}

	new_addr = around(g_rsv_memctrl.start + g_rsv_memctrl.pos, alignment);
	if (new_addr + around(bytes, 4) > g_rsv_memctrl.end) { /* 4 bytes align */
		return NULL;
	}

	g_rsv_memctrl.pos = new_addr - g_rsv_memctrl.start + around(bytes, 4); /* 4 bytes align */
	ptr = (void *)(uintptr_t)new_addr;

#ifdef __RSV_MEM_DEBUG
	puts(file);puts("(");putdec(line);puts("): rsv mem align 0x");puthex((ulong)ptr);puts("(");putdec(bytes);puts(")\n");
#endif
	return ptr;
}

void dump_rsv_mem_info(void)
{
	puts("Reserved Mem Info:\n");
	puts("start:       0x");puthex(g_rsv_memctrl.start);puts("\n");
	puts("end:         0x");puthex(g_rsv_memctrl.end);puts("\n");
	puts("pos:         0x");puthex(g_rsv_memctrl.pos);puts("\n");
}
