
#ifndef _DIV64_H_
#define _DIV64_H_

#include <sys/types.h>

extern uint32_t __div64_32(uint64_t *dividend, uint32_t divisor);

#define do_div(n, base) ({                           \
	uint32_t __base = (base);                      \
	uint32_t __rem;                                \
	if (((n) >> 32) == 0) {                      \
		__rem = (uint32_t)(n) % __base;        \
		(n) = (uint32_t)(n) / __base;          \
	} else                                       \
		__rem = __div64_32(&(n), __base);    \
	__rem;                                       \
})

#endif /* _DIV64_H_ */
