
#include <stdio.h>
#include <stdint.h>
#include <config.h>
#include <platform.h>
#include <core.h>
#include <errno.h>

extern void default_handler(void);

void drv_irq_enable(uint32_t irq_num)
{
	rv32_vic_enable_irq(irq_num);
}

void drv_irq_disable(uint32_t irq_num)
{
	rv32_vic_disable_irq(irq_num);
}

int drv_irq_register(uint32_t irq_num, void *irq_handler)
{
	uint32_t handler = rv32_vic_get_vector(irq_num);

	if(handler != (uint32_t)&default_handler) {
		puts("IRQ ");puthex(irq_num);puts(" has been registered by 0x");
		puthex(handler);puts("!\n");
		return -EBUSY;
	}

	__disable_irq();

	rv32_vic_set_vector(irq_num, (uintptr_t)irq_handler);

	__enable_irq();

	drv_irq_enable(irq_num);

	return 0;
}

void drv_irq_unregister(uint32_t irq_num)
{
	__disable_irq();

	rv32_vic_set_vector(irq_num, (uintptr_t)default_handler);
	drv_irq_disable(irq_num);

	__enable_irq();
}
