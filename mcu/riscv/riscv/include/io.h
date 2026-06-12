#ifndef __RISCV_IO_H__
#define __RISCV_IO_H__

#include <sys/types.h>

#define __arch_getl(a)          (*(volatile unsigned int *)(a))
#define __arch_putl(v,a)        (*(volatile unsigned int *)(a) = (v))
#define __iormb()   //dmb()
#define __iowmb()   //dmb()
#define writel(v,c) ({ u32 __v = v; __iowmb(); __arch_putl(__v,c); __v; })
#define readl(c)    ({ u32 __v = __arch_getl(c); __iormb(); __v; })

#define csr_read(csr)                                           \
({                                                              \
        register unsigned long __v;                             \
        __asm__ __volatile__ ("csrr %0, "  #csr			        \
                              : "=r" (__v) :                    \
                              : "memory");                      \
        __v;                                                    \
})

#define csr_write(csr, val)                                     \
({                                                              \
        unsigned long __v = (unsigned long)(val);               \
        __asm__ __volatile__ ("csrw "  #csr ", %0"     \
                              : : "rK" (__v)                    \
                              : "memory");                      \
})

#endif /* __RISCV_IO_H__ */

