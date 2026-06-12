
#ifndef __BARRIERS_H__
#define __BARRIERS_H__

#ifndef __ASSEMBLY__

#define ISB	asm volatile ("isb sy" : : : "memory")
#define DSB	asm volatile ("dsb sy" : : : "memory")
#define DMB	asm volatile ("dmb sy" : : : "memory")

#define isb()	ISB
#define dsb()	DSB
#define dmb()	DMB
#endif	/* __ASSEMBLY__ */
#endif	/* __BARRIERS_H__ */
