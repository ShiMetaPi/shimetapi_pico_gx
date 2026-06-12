
#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include <stdint.h>

#define  ATTRIBUTE_ISR __attribute__ ((interrupt ("machine")))

void drv_irq_enable(uint32_t irq_num);
void drv_irq_disable(uint32_t irq_num);
int  drv_irq_register(uint32_t irq_num, void *irq_handler);
void drv_irq_unregister(uint32_t irq_num);
int irq_init(void);

#endif /* ifndef _INTERRUPT_H_ */
