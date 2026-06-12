/*
 * Copyright (c) LOTUS. All rights reserved.
 */

#include <asm-offsets.h>
#include <config.h>
#include <asm/system.h>
#include <linux/linkage.h>
#include <asm/armv7.h>
#include <linux/lotus/step.h>

.globl	_start
_start:
/***** Key Area ****/
.=CONFIG_ROOT_RSA_PUB_KEY_N_POS
	.fill CONFIG_ROOT_RSA_PUB_KEY_N_LEN, 1, 0
.=CONFIG_ROOT_RSA_PUB_KEY_E_POS
	.fill CONFIG_ROOT_RSA_PUB_KEY_E_ZERO_LEN, 1, 0
.=(CONFIG_ROOT_RSA_PUB_KEY_E_POS + CONFIG_ROOT_RSA_PUB_KEY_E_ZERO_LEN)
	.word	CONFIG_ROOT_RSA_PUB_KEY_E_V

/***** Params Area ****/
.=CONFIG_AUXCODE_AREA_LEN_POS
	.word CONFIG_AUXAREA_LEN
.=CONFIG_BOOT_CODE_AREA_LEN_POS
	.word _checked_area_end - _checked_area_start
.=CONFIG_TOTAL_BOOT_AREA_LEN_POS
	.word (_checked_area_end - _checked_area_start + 0x200)
.=CONFIG_PARAMS_FORM_LEN_POS
	.word CONFIG_PARAMS_FORM_LEN
.=CONFIG_BOOT_ENC_FLAG_POS
	.word CONFIG_BOOT_ENC_FLAG
.=CONFIG_BOOT_ENTRY_PIONT_POS
	.word CONFIG_BOOT_ENTRY_PIONT
.=CONFIG_AUXCODE_AES_IV_POS
	.fill 16, 1, 0
.=CONFIG_BOOT_AES_IV_POS
	.fill 16, 1, 0
.=CONFIG_PARAMS_FORM_POS
_default_boot_reg_pos:
	.incbin CONFIG_BOOTREG_DEFAULT    /* reg   */
_default_boot_reg_end:
.=CONFIG_LOAD_MCU_FW_FLAG_POS
	.word CONFIG_LOAD_MCU_FW_FLAG
.=CONFIG_MCU_FW_AREA_LEN_POS
	.word CONFIG_MCU_FW_AREA_LEN
.=CONFIG_LBIST_TEST_PATTERN_OFF_POS
	.word (_checked_area_end - _checked_area_start + 0x200 + CONFIG_AUXAREA_LEN + CONFIG_KEY_AREA_LEN + CONFIG_MCU_FW_AREA_LEN)
.=COFNIG_LBIST_TEST_PATTERN_LEN_POS
	.word CONFIG_LBIST_TEST_PATTERN_IMAGE_SIZE
.=COFNIG_LBIST_TEST_PATTERN_HASH_POS
	.incbin CONFIG_LBIST_TEST_PATTERN_HASH_IMAGE
.=COFNIG_AUXCODE_ALG_FLAG_POS
	.word CONFIG_AUXCODE_2048_FLAG
.=CONFIG_RESERVED0_POS
	.fill CONFIG_RESERVED0_LEN, 1, 0
.=CONFIG_SIG_PARAMS_POS
	.fill CONFIG_SIG_PARAMS_LEN, 1, 0

/**** Uncheck Area ****/
.=CONFIG_SCS_SIMULATE_FLAG_POS
	.word CONFIG_SCS_SIMULATE_FLAG
.=CONFIG_BOOT_STORE_ADDR_POS
	.word CONFIG_BOOT_STORE_ADDR
.=CONFIG_FLASH_EMPYT_FLAG_POS
	.word CONFIG_FLASH_EMPYT_FLAG
.=CONFIG_RESERVED1_POS
	.fill CONFIG_RESERVED1_LEN, 1, 0

/**** Auxcode Area *****/
.=CONFIG_IMAGE_INFO_POS
	.incbin CONFIG_AUXIMAGE

/* mcu fw */
#if (CONFIG_MCU_FW_AREA_LEN > 0)
.=CONFIG_MCU_FW_AREA_POS
	.incbin CONFIG_MCU_FW_IMAGE
#endif

/**** Boot Code Area *****/
.=CONFIG_BOOT_CODE_AREA_POS
.align 4
_checked_area_start:
	b	reset
        ldr     pc, _undefined_instruction
        ldr     pc, _software_interrupt
        ldr     pc, _prefetch_abort
        ldr     pc, _data_abort
        ldr     pc, _not_used
        ldr     pc, _irq
        ldr     pc, _fiq

	.balignl 64,0xdeadbeef

_undefined_instruction: .word irq_undefined_instruction
_software_interrupt:    .word irq_software_interrupt
_prefetch_abort:        .word irq_prefetch_abort
_data_abort:            .word irq_data_abort
_not_used:              .word irq_not_used
_irq:                   .word irq_irq
_fiq:                   .word irq_firq

	.globl	reset
	.globl	save_boot_params_ret
	/* fix text_base,real code position   */
	.globl	_TEXT_BASE
_TEXT_BASE:
	.word TEXT_BASE + CONFIG_KEY_AREA_LEN + CONFIG_AUXAREA_LEN

_start_armboot:
	.word start_armboot

_bss_start:
        .word   __bss_start
_bss_end:
        .word   __bss_end

	.globl __checked_area_start
__checked_area_start:
        .word _checked_area_start

reset:
	step_no_stack	30, r11, r12
	/* Allow the board to save important registers */
	b	save_boot_params
save_boot_params_ret:
	/*
	 * disable interrupts (FIQ and IRQ), also set the cpu to SVC32 mode,
	 * except if in HYP mode already
	 */
	mrs	r0, cpsr
	and	r1, r0, #0x1f		@ mask mode bits
	teq	r1, #0x1a		@ test for HYP mode
	bicne	r0, r0, #0x1f		@ clear all mode bits
	orrne	r0, r0, #0x13		@ set SVC mode
	orr	r0, r0, #0xc0		@ disable FIQ and IRQ
	msr	cpsr,r0

	/* Set V=0 in CP15 SCTLR register - for VBAR to point to vector */
	mrc	p15, 0, r0, c1, c0, 0	@ Read CP15 SCTLR Register
	bic	r0, #CR_V		@ V = 0
	mcr	p15, 0, r0, c1, c0, 0	@ Write CP15 SCTLR Register

	/* Set vector address in CP15 VBAR register */
	adrl	r0, _checked_area_start
	mcr	p15, 0, r0, c12, c0, 0	@Set VBAR

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	bl	cpu_init_cp15
#endif

normal_start_flow:
	/* Early init */
	ldr     sp, =CONFIG_SYS_INIT_SP_ADDR
	step	31
	bl	timer_init
	bl	timestamp_init
	bl	uart_early_init
	bl	msg_main_cpu_startup

#ifndef CONFIG_SYS_ICACHE_OFF
	@enable I-Cache now
	mrc p15, 0, r0, c1, c0, 0
	orr r0, r0, #0x00001000 /* set bit 12 (I) I-Cache */
	mcr p15, 0, r0, c1, c0, 0
#endif

	step	32

#ifndef CONFIG_LOTUS_GZIP_BOOT
	/* move uboot to final text base */
	ldr	r1, =CONFIG_SYS_TEXT_BASE

	ldr	r0, =input_data
	ldr	r3, =input_data_end
	sub	r2, r3, r0
	/* memcpy(r1, r0, r2)  */
	bl	memcpy
#endif

	step	33

clear_bss:
        /* clean .bss section */
        ldr     r0, _bss_start
        ldr     r1, _bss_end
        mov     r2, #0x0
next_bss:
        str     r2, [r0]           @ clear BSS location
        cmp     r0, r1             @ are we at the end yet
        add     r0, r0, #4         @ increment clear index pointer
        bne     next_bss           @ keep clearing till at end

	step	34

relocate:
	ldr	r0, =_start_armboot
	ldr	pc, [r0]

bug:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	b       .                       /* bug here */
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@
@       void memcpy(r1, r0, r2);
@

.align  2
memcpy:
	add	r2, r0, r2
memcpy_loop:
	ldmia	r0!, {r3 - r10}
	stmia	r1!, {r3 - r10}
	cmp	r0, r2
	ble	memcpy_loop
	mov	pc, lr

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

.align  2
msg_main_cpu_startup:
	mov	r5, lr
	add	r0, pc, #4
	bl	uart_early_puts
	mov	pc, r5
	L10:
	.ascii "\r\n\r\nSystem startup\r\n\0"

/*------------------------------------------------------------------------------*/

ENTRY(c_runtime_cpu_setup)
/*
 * If I-cache is enabled invalidate it
 */
#ifndef CONFIG_SYS_ICACHE_OFF
	mcr	p15, 0, r0, c7, c5, 0	@ invalidate icache
	mcr	p15, 0, r0, c7, c10, 4	@ DSB
	mcr	p15, 0, r0, c7, c5, 4	@ ISB
#endif

	bx	lr

ENDPROC(c_runtime_cpu_setup)

/*************************************************************************
 *
 * void save_boot_params(u32 r0, u32 r1, u32 r2, u32 r3)
 *	__attribute__((weak));
 *
 * Stack pointer is not yet initialized at this moment
 * Don't save anything to stack even if compiled with -O0
 *
 *************************************************************************/
ENTRY(save_boot_params)
	b	save_boot_params_ret		@ back to my caller
ENDPROC(save_boot_params)
	.weak	save_boot_params

/*************************************************************************
 *
 * cpu_init_cp15
 *
 * Setup CP15 registers (cache, MMU, TLBs). The I-cache is turned on unless
 * CONFIG_SYS_ICACHE_OFF is defined.
 *
 *************************************************************************/
ENTRY(cpu_init_cp15)
	/*
	 * Invalidate L1 I/D
	 */
	mov	r0, #0			@ set up for MCR
	mcr	p15, 0, r0, c8, c7, 0	@ invalidate TLBs
	mcr	p15, 0, r0, c7, c5, 0	@ invalidate icache
	mcr	p15, 0, r0, c7, c5, 6	@ invalidate BP array
	mcr	p15, 0, r0, c7, c10, 4	@ DSB
	mcr	p15, 0, r0, c7, c5, 4	@ ISB

	/*
	 * disable MMU stuff and caches
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002000	@ clear bits 13 (--V-)
	bic	r0, r0, #0x00000007	@ clear bits 2:0 (-CAM)
	orr	r0, r0, #0x00000002	@ set bit 1 (--A-) Align
	orr	r0, r0, #0x00000800	@ set bit 11 (Z---) BTB
#ifdef CONFIG_SYS_ICACHE_OFF
	bic	r0, r0, #0x00001000	@ clear bit 12 (I) I-cache
#else
	orr	r0, r0, #0x00001000	@ set bit 12 (I) I-cache
#endif
	mcr	p15, 0, r0, c1, c0, 0

#ifdef CONFIG_ARM_ERRATA_716044
	mrc	p15, 0, r0, c1, c0, 0	@ read system control register
	orr	r0, r0, #1 << 11	@ set bit #11
	mcr	p15, 0, r0, c1, c0, 0	@ write system control register
#endif

#if (defined(CONFIG_ARM_ERRATA_742230) || defined(CONFIG_ARM_ERRATA_794072))
	mrc	p15, 0, r0, c15, c0, 1	@ read diagnostic register
	orr	r0, r0, #1 << 4		@ set bit #4
	mcr	p15, 0, r0, c15, c0, 1	@ write diagnostic register
#endif

#ifdef CONFIG_ARM_ERRATA_743622
	mrc	p15, 0, r0, c15, c0, 1	@ read diagnostic register
	orr	r0, r0, #1 << 6		@ set bit #6
	mcr	p15, 0, r0, c15, c0, 1	@ write diagnostic register
#endif

#ifdef CONFIG_ARM_ERRATA_751472
	mrc	p15, 0, r0, c15, c0, 1	@ read diagnostic register
	orr	r0, r0, #1 << 11	@ set bit #11
	mcr	p15, 0, r0, c15, c0, 1	@ write diagnostic register
#endif
#ifdef CONFIG_ARM_ERRATA_761320
	mrc	p15, 0, r0, c15, c0, 1	@ read diagnostic register
	orr	r0, r0, #1 << 21	@ set bit #21
	mcr	p15, 0, r0, c15, c0, 1	@ write diagnostic register
#endif

	mov	r5, lr			@ Store my Caller
	mrc	p15, 0, r1, c0, c0, 0	@ r1 has Read Main ID Register (MIDR)
	mov	r3, r1, lsr #20		@ get variant field
	and	r3, r3, #0xf		@ r3 has CPU variant
	and	r4, r1, #0xf		@ r4 has CPU revision
	mov	r2, r3, lsl #4		@ shift variant field for combined value
	orr	r2, r4, r2		@ r2 has combined CPU variant + revision

#ifdef CONFIG_ARM_ERRATA_798870
	cmp	r2, #0x30		@ Applies to lower than R3p0
	bge	skip_errata_798870      @ skip if not affected rev
	cmp	r2, #0x20		@ Applies to including and above R2p0
	blt	skip_errata_798870      @ skip if not affected rev

	mrc	p15, 1, r0, c15, c0, 0  @ read l2 aux ctrl reg
	orr	r0, r0, #1 << 7         @ Enable hazard-detect timeout
	push	{r1-r5}			@ Save the cpu info registers
	bl	v7_arch_cp15_set_l2aux_ctrl
	isb				@ Recommended ISB after l2actlr update
	pop	{r1-r5}			@ Restore the cpu info - fall through
skip_errata_798870:
#endif

#ifdef CONFIG_ARM_ERRATA_801819
	cmp	r2, #0x24		@ Applies to lt including R2p4
	bgt	skip_errata_801819      @ skip if not affected rev
	cmp	r2, #0x20		@ Applies to including and above R2p0
	blt	skip_errata_801819      @ skip if not affected rev
	mrc	p15, 0, r0, c0, c0, 6	@ pick up REVIDR reg
	and	r0, r0, #1 << 3		@ check REVIDR[3]
	cmp	r0, #1 << 3
	beq	skip_errata_801819	@ skip erratum if REVIDR[3] is set

	mrc	p15, 0, r0, c1, c0, 1	@ read auxilary control register
	orr	r0, r0, #3 << 27	@ Disables streaming. All write-allocate
					@ lines allocate in the L1 or L2 cache.
	orr	r0, r0, #3 << 25	@ Disables streaming. All write-allocate
					@ lines allocate in the L1 cache.
	push	{r1-r5}			@ Save the cpu info registers
	bl	v7_arch_cp15_set_acr
	pop	{r1-r5}			@ Restore the cpu info - fall through
skip_errata_801819:
#endif

#ifdef CONFIG_ARM_ERRATA_454179
	cmp	r2, #0x21		@ Only on < r2p1
	bge	skip_errata_454179

	mrc	p15, 0, r0, c1, c0, 1	@ Read ACR
	orr	r0, r0, #(0x3 << 6)	@ Set DBSM(BIT7) and IBE(BIT6) bits
	push	{r1-r5}			@ Save the cpu info registers
	bl	v7_arch_cp15_set_acr
	pop	{r1-r5}			@ Restore the cpu info - fall through

skip_errata_454179:
#endif

#ifdef CONFIG_ARM_ERRATA_430973
	cmp	r2, #0x21		@ Only on < r2p1
	bge	skip_errata_430973

	mrc	p15, 0, r0, c1, c0, 1	@ Read ACR
	orr	r0, r0, #(0x1 << 6)	@ Set IBE bit
	push	{r1-r5}			@ Save the cpu info registers
	bl	v7_arch_cp15_set_acr
	pop	{r1-r5}			@ Restore the cpu info - fall through

skip_errata_430973:
#endif

#ifdef CONFIG_ARM_ERRATA_621766
	cmp	r2, #0x21		@ Only on < r2p1
	bge	skip_errata_621766

	mrc	p15, 0, r0, c1, c0, 1	@ Read ACR
	orr	r0, r0, #(0x1 << 5)	@ Set L1NEON bit
	push	{r1-r5}			@ Save the cpu info registers
	bl	v7_arch_cp15_set_acr
	pop	{r1-r5}			@ Restore the cpu info - fall through

skip_errata_621766:
#endif

	mov	pc, r5			@ back to my caller
ENDPROC(cpu_init_cp15)


