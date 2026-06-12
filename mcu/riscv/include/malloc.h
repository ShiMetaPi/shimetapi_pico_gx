
#ifndef _MALLOC_H_
#define _MALLOC_H_

#include <config.h>
#include <stddef.h>

void malloc_init(uint32 start, uint32 len);
#ifdef CONFIG_MALLOC
void *malloc(uint32 bytes); /* not thread safe */
void *memalign(uint32 alignment, uint32 bytes);
void free(void *ptr);
void show_memnode(unsigned int);
#else
void *__malloc(uint32 bytes, const char *file, int line);
void *__memalign(uint32 alignment, uint32 bytes, const char *file, int line);
void __free(const void *mem, const char *file, int line);
#define malloc(bytes) __malloc(bytes, __FILE__, __LINE__)
#define memalign(alignment, bytes) __memalign(alignment, bytes, __FILE__, __LINE__)
#define free(mem) __free(mem, __FILE__, __LINE__)
#endif
#endif /* _MALLOC_H_ */
