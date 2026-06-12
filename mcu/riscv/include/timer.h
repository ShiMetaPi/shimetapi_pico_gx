
#ifndef _TIMER_H_
#define _TIMER_H_

#include <sys/types.h>
#include <io.h>
#include <lib.h>

void timer_init(void);
void timer_start(void);
ulong timer_get_val(void);

void udelay(ulong us);
void mdelay(u32 msec);

#endif /* _TIMER_H_ */
