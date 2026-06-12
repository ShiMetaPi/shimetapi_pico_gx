
#ifndef _MEM_LIB_H_
#define _MEM_LIB_H_

#include <hlist.h>

void memlib_init(struct list *listhead, unsigned int base, unsigned int size);
void *memlib_malloc(struct list *listhead, unsigned int  bytes);
void *memlib_memalign(struct list *listhead, unsigned int alignment, unsigned int bytes);
void memlib_free(struct list *listhead, void *ptr);
void memlib_show_memnode(struct list *listhead, unsigned int cnt);
#endif /* _MEM_LIB_H_ */
