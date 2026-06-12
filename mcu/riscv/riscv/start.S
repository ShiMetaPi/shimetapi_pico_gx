
#include <config.h>

.text
	.align  2
	.globl _start
	.type _start, %function
_start:
	.option push
	.option norvc
	.option norelax
	/* struct mcu_fw_head */
	j	_reset
	.word	CONFIG_MCU_FW_MAGIC
	.word	CONFIG_MCU_FW_VERSION
	.word	__bin_end - _start
	.word	(CONFIG_STAGE2_START - CONFIG_TEXT_BASE)
	.option	pop
.=CONFIG_MCU_HEAD_SIZE
	.align	2
_reset:
	.option push
	.option norelax
	la	gp, __global_pointer$
	.option pop
	la	a0, default_handler
	ori	a0, a0, 3
	csrw	mtvec, a0

	la	a0, __vectors
	csrw	mtvt, a0

	la	sp, g_top_irqstack
	csrw	mscratch, sp

	/* Clear bss section */
	la	a0, __bss_start
	la	a1, __bss_end
	bgeu	a0, a1, 2f
1:
	sw	zero, (a0)
	addi	a0, a0, 4
	bltu	a0, a1, 1b
2:

	jal	system_init
	jal	main

	.size   _start, . - _start

__exit:
	j      __exit

.section .stack, "aw", @nobits
    .align  2
    .global g_base_irqstack
    .global g_top_irqstack
g_base_irqstack:
    .space CONFIG_STACK_SIZE
g_top_irqstack:

.section .vectors, "aw", @progbits
	.align  6
	.globl  __vectors
	.type   __vectors, @object
__vectors:
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   tspend_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   core_timer_irq_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler

	/* External interrupts */
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler
	.long   default_handler

	.size   __vectors, . - __vectors

