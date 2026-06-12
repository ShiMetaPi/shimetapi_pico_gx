#include <asm_macros.S>
#include <cortex_a55.h>
#include <xmfalcon_def.h>

.global lotus_cpu_suspend
.global lotus_suspend_save_param
.global a55_core_pwr_dwn
.globl  lotus_invalidate_icache

func a55_core_pwr_dwn

	/* ---------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------
	 */
	mrs	x0, CORTEX_A55_CPUPWRCTLR_EL1
	orr	x0, x0, #CORTEX_A55_CORE_PWRDN_EN_MASK
	msr	CORTEX_A55_CPUPWRCTLR_EL1, x0
	isb
	dsb sy
loop_wfi:
	dsb sy
	isb
	wfi
	b   loop_wfi

endfunc a55_core_pwr_dwn

func lotus_invalidate_icache
	ic      iallu
	ret
endfunc lotus_invalidate_icache

func lotus_suspend_save_param
	/* set scu boot entry point */
	ldr	w0,	=resume_entry_start_32
	ldr	x1,	=0x12120050
	str	w0,	[x1]

	/* set 64bit entry point */
	ldr	w0,	=bl31_warm_entrypoint
	ldr	x1,	=0x12120054
	str	w0,	[x1]

	ret
endfunc lotus_suspend_save_param

func lotus_cpu_suspend
	/* disable mmu_el3. */
	bl	disable_mmu_el3
	ldr	x2, =LOTUS_PM_CODE_START
	br      x2

	ret
endfunc lotus_cpu_suspend

