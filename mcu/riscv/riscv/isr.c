
#include <stdio.h>
#include <config.h>
#include <io.h>
#include <interrupt.h>
#include <core.h>

void systick_handler(void)
{
}



ATTRIBUTE_ISR void core_timer_irq_handler(void)
{
    systick_handler();
}

ATTRIBUTE_ISR void nmi_handler(void)
{
	puts("NMI Exception!\n");
	puts("mepc        : 0x");puthex(__get_MEPC());puts("\n");
	puts("mnmipc      : 0x");puthex(__get_MNMIPC());puts("\n");
	puts("mcause      : 0x");puthex(__get_MCAUSE());puts("\n");
	puts("mnmicause   : 0x");puthex(__get_MNMICAUSE());puts("\n");
	puts("mstatus     : 0x");puthex(__get_MSTATUS());puts("\n");
	puts("mtval       : 0x");puthex(__get_MTVAL());puts("\n");
	puts("mxstatus    : 0x");puthex(__get_MXSTATUS());puts("\n");
	puts("mexstatus   : 0x");puthex(__get_MEXSTATUS());puts("\n");
	puts("mintstatus  : 0x");puthex(__get_MINTSTATUS());puts("\n");
	puts("mie         : 0x");puthex(__get_MIE());puts("\n");
	puts("mhcr        : 0x");puthex(__get_MHCR());puts("\n");
}

ATTRIBUTE_ISR void mbox_handler(void)
{
	puts("Mailbox interrupt!\n");
	writel(0, 0x100AF030);
}

int irq_init(void)
{
	int ret;

	ret = drv_irq_register(IRQ_NUM_MBOX, &mbox_handler);
	if (ret <0) {
		puts("Fail to register Mailbox irq!\n");
	}

	return 0;
}


