
#include <stddef.h>
#include <hlist.h>
#include "memlib.h"

static struct list g_malloc_list = {0};

void malloc_init(unsigned int base, unsigned int size)
{
    if ((g_malloc_list.base.next != NULL) && (g_malloc_list.base.prev != NULL)) {
        return;
    }
    memlib_init(&g_malloc_list, base, size);
}

void *malloc(uint32 bytes)
{
    if ((g_malloc_list.base.next == NULL) || (g_malloc_list.base.prev == NULL)) {
        return NULL;
    }
    return memlib_malloc(&g_malloc_list, bytes);
}

void *memalign(uint32 alignment, uint32 bytes)
{
    if ((g_malloc_list.base.next == NULL) || (g_malloc_list.base.prev == NULL)) {
        return NULL;
    }
    return memlib_memalign(&g_malloc_list, alignment, bytes);
}

void free(void *ptr)
{
    if ((g_malloc_list.base.next == NULL) || (g_malloc_list.base.prev == NULL)) {
        return;
    }
    memlib_free(&g_malloc_list, ptr);
}

void show_memnode(unsigned int cnt)
{
    if ((g_malloc_list.base.next == NULL) || (g_malloc_list.base.prev == NULL)) {
        return;
    }
    memlib_show_memnode(&g_malloc_list, cnt);
}
