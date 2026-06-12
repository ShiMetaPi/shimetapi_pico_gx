#include <asm_macros.S>
#include <cortex_a55.h>

.globl lotus_sys_suspend;
.extern lotus_pm_ddr_saving;

func lotus_sys_suspend
	/*****set sp address:0x121203c0(the end of lpds) ******/
	mov x0, #0x1212
	lsl x0, x0, #16
	add x0, x0, #0x03c0
	mov sp, x0
	/***** DDR CODE ******/
	bl lotus_ddr_saving
	/***** power down ****/

endfunc lotus_sys_suspend

.global lotus_ddr_saving
.balign 4
lotus_ddr_saving:
.incbin "./str_bit/str_bit.bin"

.balign 4
.globl lotus_sys_suspend_size;
lotus_sys_suspend_size:
	.word . - lotus_sys_suspend
