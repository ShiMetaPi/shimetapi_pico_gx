/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __EARLY_TIME_H__
#define __EARLY_TIME_H__


int timer_init(void);
void udelay(unsigned long usec);
unsigned long long get_ticks(void);

#endif /* ifndef __EARLY_TIME_H__ */
