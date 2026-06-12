#ifndef __LIB_H__
#define __LIB_H__

#include <sys/types.h>
#include <config.h>

#define SECUREC_MEM_MAX_LEN (0x100000)  //1MB

void reset_cpu(void);

int memset_s(void* dest, size_t destMax, unsigned char c, size_t count);
int memcpy_s(void* dest, size_t destMax, const void* src, size_t count);
unsigned short crc16(unsigned char *data, unsigned int length);
#endif /*__LIB_H__*/

