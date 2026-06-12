
#include <stdio.h>
#include <malloc.h>
#include <config.h>
#include <core.h>
#include <interrupt.h>
#include <common.h>

void show_memlayout(void)
{
	puts("Mem Layout Info:\n");
	puts(".text     : 0x");puthex((ulong)&__text_start);	puts(" - 0x");puthex((ulong)&__text_end);puts("\n");
	puts(".rodata   : 0x");puthex((ulong)&__rodata_start);	puts(" - 0x");puthex((ulong)&__rodata_end);puts("\n");
	puts(".data     : 0x");puthex((ulong)&__data_start);	puts(" - 0x");puthex((ulong)&__data_end);puts("\n");
	puts(".text2    : 0x");puthex((ulong)&__text2_start);	puts(" - 0x");puthex((ulong)&__text2_end);puts("\n");
	puts(".rodata2  : 0x");puthex((ulong)&__rodata2_start);	puts(" - 0x");puthex((ulong)&__rodata2_end);puts("\n");
	puts(".data2    : 0x");puthex((ulong)&__data2_start);	puts(" - 0x");puthex((ulong)&__data2_end);puts("\n");
	puts(".bss      : 0x");puthex((ulong)&__bss_start);	puts(" - 0x");puthex((ulong)&__bss_end);puts("\n");
	puts(".stack    : 0x");puthex((ulong)&g_base_irqstack);	puts(" - 0x");puthex((ulong)&g_top_irqstack);puts("(top)\n");
	puts("Heap      : 0x");puthex(g_heap_start);		puts(" - 0x");puthex(g_heap_end);puts("\n");
	puts("Param     : 0x");puthex((ulong)&__param_start);   puts(" - 0x");puthex((ulong)&__param_start + CONFIG_PARAM_MEM_SIZE);puts("\n");
	puts("\n");
}

void clic_config(void)
{
    int i, num_int;

    /* get interrupt level from info */
    CLIC->CLICCFG = (((CLIC->CLICINFO & CLIC_INFO_CLICINTCTLBITS_Msk) >> CLIC_INFO_CLICINTCTLBITS_Pos) << CLIC_CLICCFG_NLBIT_Pos);
    /* get interrutp num */
    num_int = CLIC->CLICINFO & 0x1fff ? CLIC->CLICINFO & 0x1fff : 256;

    for (i = 0; i < num_int; i++) {
	CLIC->CLICINT[i].IP = 0;
	/* use vector interrupt */
	CLIC->CLICINT[i].ATTR = 1;
	/* set all interrupts with lowest prio */
	rv32_vic_set_prio(i, 0);
    }
}

void cpu_config(void)
{
#ifdef CONFIG_XTHEAD_ISA_EXT
	uint32_t mxstatus = __get_MXSTATUS();
#endif
	uint32_t mstatus = __get_MSTATUS();
	uint32_t mhcr = __get_MHCR();

	/* enable mstatus FS and XS */
	mstatus |= (1 << 13);
	mstatus |= (3 << 15);
	__set_MSTATUS(mstatus);

#ifdef CONFIG_XTHEAD_ISA_EXT
	Enable T-HEAD Ext instruction set
	mxstatus |= (1 << 22);
	__set_MXSTATUS(mxstatus);
#endif

	mhcr &= ~(CACHE_MHCR_WA_Msk | CACHE_MHCR_WB_Msk);
	mhcr |= CACHE_MHCR_RS_Msk | CACHE_MHCR_BPE_Msk | CACHE_MHCR_L0BTB_Msk;
	__set_MHCR(mhcr);
}

void cache_config(void)
{
	/* rv32_dcache_enable(); */ //FIXME
	rv32_icache_enable();
}

void system_init(void)
{
	timer_init();

	serial_init();

	puts_always("MCU Start\n\n");

	cpu_config();

	clic_config();

	irq_init();

	cache_config();

	__enable_excp_irq();

	/* core_timer_config(drv_get_sys_freq() / CONFIG_SYSTICK_HZ, CORET_IRQn);    //10ms */

	malloc_init((u32)_end, _start + CONFIG_MCU_SRAM_SIZE - _end - CONFIG_PARAM_MEM_SIZE);

	rsv_mem_init(CONFIG_DDR_BASE + (ulong)get_ddr_size() - CONFIG_RESERVED_MEM_SIZE, CONFIG_RESERVED_MEM_SIZE);

#ifdef CONFIG_DEBUG_INFO
	puts("\nRISC-V status:\n");
	puts("MSTATUS: 0x");puthex(__get_MSTATUS());puts("\n");
	puts("MXSTATUS: 0x");puthex(__get_MXSTATUS());puts("\n");
	puts("MHCR: 0x");puthex(__get_MHCR());puts("\n\n");

	show_memlayout();
#endif
}
