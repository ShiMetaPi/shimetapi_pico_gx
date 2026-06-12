
#include <malloc.h>
#include <string.h>
#include <debug.h>
#include <common.h>

struct memctrl_t {
    uint32 start;
    uint32 end;
    uint32 pos;
};

uint32 g_heap_start = 0;
uint32 g_heap_end = 0;

static struct memctrl_t g_memctrl = {0};
void malloc_init(uint32 start, uint32 len)
{
    if (g_memctrl.end == 0) {
        g_heap_start = g_memctrl.start = start;
        g_heap_end = g_memctrl.end = start + len;
        g_memctrl.pos = 0;
    }
}

void *__malloc(uint32 bytes, const char *file, int line)
{
    uint32 new_ptr;
    void *ptr = NULL;
    if (g_memctrl.end == 0) {
        pr_error("malloc module uninitializtion.\n");
        return NULL;
    }

    pr_debug("%s(%d): malloc:0x%x\n" _NONE, file, line, bytes);
    new_ptr = g_memctrl.start + g_memctrl.pos + around(bytes, 4); /* 4 bytes align */
    if (new_ptr >  g_memctrl.end) {
        return NULL;
    }
    ptr = (void *)(uintptr_t)(g_memctrl.start + g_memctrl.pos);
    g_memctrl.pos += around(bytes, 4); /* 4 bytes align */
    return ptr;
}

void __free(const void *mem, const char *file, int line)
{
    if (g_memctrl.end == 0) {
        pr_error("malloc module uninitializtion.\n");
        return;
    }

    pr_debug("%s(%d): free\n" _NONE, file, line);
    return;
}

void *__memalign(uint32 alignment, uint32 bytes, const char *file, int line)
{
    uint32 new_addr;
    void *ptr = NULL;
    if (g_memctrl.end == 0) {
        pr_error("malloc module uninitializtion.\n");
        return NULL;
    }
    if (alignment & (alignment - 1)) {
        pr_error("align must be powed of 2\n");
        return NULL;
    }
    new_addr = around(g_memctrl.start + g_memctrl.pos, alignment);
    if (new_addr + around(bytes, 4) > g_memctrl.end) { /* 4 bytes align */
        return NULL;
    }
    g_memctrl.pos = new_addr - g_memctrl.start + around(bytes, 4); /* 4 bytes align */
    ptr = (void *)(uintptr_t)new_addr;
    pr_debug("%s(%d): memalign:0x%x\n" _NONE, file, line, bytes);

    return ptr;
}

void dump_malloc(void)
{
    printf("start:       0x%08x\n", g_memctrl.start);
    printf("end:         0x%08x\n", g_memctrl.end);
    printf("pos:         0x%08x\n", g_memctrl.pos);
}
