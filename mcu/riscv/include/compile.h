
#ifndef _COMPILE_H_
#define _COMPILE_H_

#include <stddef.h>

#define __FILENAME__ (__builtin_strrchr(__FILE__, '/')? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#define array_size(x)  (sizeof(x) / sizeof(*(x)))

#define swap32(_x) ((uint32)( \
    ((((uint32)(_x)) & 0x000000FF) << 24) | \
    ((((uint32)(_x)) & 0x0000FF00) << 8)  | \
    ((((uint32)(_x)) & 0xFF000000) >> 24) | \
    ((((uint32)(_x)) & 0x00FF0000) >> 8)))

#define swap16(_x) ((uint16)( \
    ((((uint16)(_x)) & 0xFF00) >> 8) | \
    ((((uint16)(_x)) & 0x00FF) << 8)))

#define around(size, align)     (((size) + (align) - 1) & (~((align) - 1)))

#define roundup(x, y)           ((((x) + ((y) - 1)) / (y)) * (y))

#define STAGE1_FUNC __attribute__((section(".text.stage1")))
#define STAGE1_GLOBAL __attribute__((section(".data.stage1")))
#define STAGE1_CONST_GLOBAL __attribute__((section(".rodata.stage1"))) const
#define STAGE1_STR(str) \
    ({ \
	    static const char _str_##__COUNTER__[] \
	    __attribute__((section(".rodata.stage1"))) = str; \
	    _str_##__COUNTER__; \
    })

#define STAGE2_FUNC __attribute__((section(".text.stage2")))
#define STAGE2_GLOBAL __attribute__((section(".data.stage2")))
#define STAGE2_CONST_GLOBAL __attribute__((section(".rodata.stage2"))) const
#define STAGE2_STR(str) \
    ({ \
	    static const char _str_##__COUNTER__[] \
	    __attribute__((section(".rodata.stage2"))) = str; \
	    _str_##__COUNTER__; \
    })

#endif /* _COMPILE_H_ */
