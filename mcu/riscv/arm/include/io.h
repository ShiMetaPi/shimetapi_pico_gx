
#ifndef __ASM_ARM_IO_H
#define __ASM_ARM_IO_H

#include <sys/types.h>
#include <barriers.h>

#define __arch_getl(a)          (*(volatile unsigned int *)(a))
#define __arch_putl(v,a)        (*(volatile unsigned int *)(a) = (v))

#define mb()        dsb()
#define __iormb()   dmb()
#define __iowmb()   dmb()

#ifdef REG_IORW_DEBUG
#include <serial.h>
#define writel(v,c) ({ u32 __v = v; __iowmb(); serial_puts("REG W: 0x"); serial_put_hex(c); serial_puts(", 0x"); serial_put_hex(__v); serial_puts("\n"); __arch_putl(__v,c); __v; })
#define readl(c)    ({ u32 __v; serial_puts("REG R: 0x"); serial_put_hex(c); __v = __arch_getl(c); __iormb(); serial_puts(", 0x"); serial_put_hex(__v); serial_puts("\n"); __v; })
#else
#define writel(v,c) ({ u32 __v = v; __iowmb(); __arch_putl(__v,c); __v; })
#define readl(c)    ({ u32 __v = __arch_getl(c); __iormb(); __v; })
#endif

#endif  /* __ASM_ARM_IO_H */
