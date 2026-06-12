
#ifndef _RESERVED_MEM_H_
#define _RESERVED_MEM_H_

#include <sys/types.h>
#include <compile.h>

void rsv_mem_init(uint32 start, uint32 len);
void *__rsv_mem_alloc(uint32 bytes, const char *file, int line);
#define reserved_mem_alloc(bytes) \
	__rsv_mem_alloc(bytes, __FILENAME__, __LINE__)
void *__rsv_mem_align(uint32 alignment, uint32 bytes, const char *file, int line);
#define reserved_mem_align(alignment, bytes) \
	__rsv_mem_align(alignment, bytes, __FILENAME__, __LINE__)

void dump_rsv_mem_info(void);

#endif /* ifndef _RESERVED_MEM_H_ */
