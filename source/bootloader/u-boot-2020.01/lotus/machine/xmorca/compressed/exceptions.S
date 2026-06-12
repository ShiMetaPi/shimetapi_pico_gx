/* SPDX-License-Identifier: GPL-2.0 */

/* exception handlers */
#include <config.h>

.align	2
_irq_stack_start:
	.word CONFIG_SYS_INIT_SP_ADDR

.align	2
.global	irq_undefined_instruction
.type	irq_undefined_instruction, %function
irq_undefined_instruction:
	ldr	sp, _irq_stack_start
	mov	r10, lr
	add	r0, pc, #20
	bl	uart_early_puts
	mov	r0, r10
	bl	uart_early_put_hex
	mov	r0, #2000
	bl	udelay
	bl	reset_cpu
	.ascii "\r\n*** irq: undefined instruction\r\nLR: \0"

.align	2
.global	irq_software_interrupt
.type	irq_software_interrupt, %function
irq_software_interrupt:
	ldr	sp, _irq_stack_start
	mov	r10, lr
	add	r0, pc, #20
	bl	uart_early_puts
	mov	r0, r10
	bl	uart_early_put_hex
	mov	r0, #2000
	bl	udelay
	bl	reset_cpu
	.ascii "\r\n*** irq: software interrupt\r\nLR: \0"

.align	2
.global	irq_prefetch_abort
.type	irq_prefetch_abort, %function
irq_prefetch_abort:
	ldr	sp, _irq_stack_start
	mov	r10, lr
	add	r0, pc, #20
	bl	uart_early_puts
	mov	r0, r10
	bl	uart_early_put_hex
	mov	r0, #2000
	bl	udelay
	bl	reset_cpu
	.ascii "\r\n*** irq: prefetch abort\r\nLR: \0"

.align	2
.global	irq_data_abort
.type	irq_data_abort, %function
irq_data_abort:
	ldr	sp, _irq_stack_start
	mov	r10, lr
	add	r0, pc, #20
	bl	uart_early_puts
	mov	r0, r10
	bl	uart_early_put_hex
	mov	r0, #2000
	bl	udelay
	bl	reset_cpu
	.ascii "\r\n*** irq: data abort\r\nLR: \0"

.align	2
.global	irq_not_used
.type	irq_not_used, %function
irq_not_used:
	ldr	sp, _irq_stack_start
	mov	r10, lr
	add	r0, pc, #20
	bl	uart_early_puts
	mov	r0, r10
	bl	uart_early_put_hex
	mov	r0, #2000
	bl	udelay
	bl	reset_cpu
	.ascii "\r\n*** irq: not used\r\nLR: \0"

.align	2
.global	irq_irq
.type	irq_irq, %function
irq_irq:
	ldr	sp, _irq_stack_start
	mov	r10, lr
	add	r0, pc, #20
	bl	uart_early_puts
	mov	r0, r10
	bl	uart_early_put_hex
	mov	r0, #2000
	bl	udelay
	bl	reset_cpu
	.ascii "\r\n*** irq: interrupt\r\nLR: \0"

.align	2
.global	irq_firq
.type	irq_firq, %function
irq_firq:
	ldr	sp, _irq_stack_start
	mov	r10, lr
	add	r0, pc, #20
	bl	uart_early_puts
	mov	r0, r10
	bl	uart_early_put_hex
	mov	r0, #2000
	bl	udelay
	bl	reset_cpu
	.ascii "\r\n*** irq: fast interrupt\r\nLR: \0"
