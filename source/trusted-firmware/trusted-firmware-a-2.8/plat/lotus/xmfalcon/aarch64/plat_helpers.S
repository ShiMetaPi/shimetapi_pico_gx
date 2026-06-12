/*
 * Copyright (c) Lotus. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <cpu_macros.S>
#include <cortex_a55.h>
#include <platform_def.h>

	/* Global functions */
	.globl	plat_is_my_cpu_primary
	.globl	plat_my_core_pos
	.globl	plat_get_my_entrypoint
	.globl	plat_secondary_cold_boot_setup
	.globl	platform_mem_init
	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl  plat_crash_console_flush
	.globl	plat_reset_handler

	/* Global variables */
	.globl	lotus_sec_entry_point
	.globl	ns_image_entrypoint
	.globl	lotus_bl31_phys_base

	/* ---------------------
	 * Common CPU init code
	 * ---------------------
	 */
.macro	cpu_init_common

	/* --------------------------------
	 * Enable the cycle count register
	 * --------------------------------
	 */
	mrs	x0, pmcr_el0
	ubfx	x0, x0, #11, #5		// read PMCR.N field
	mov	x1, #1
	lsl	x0, x1, x0
	sub	x0, x0, #1		// mask of event counters
	orr	x0, x0, #0x80000000	// disable overflow intrs
	msr	pmintenclr_el1, x0
	msr	pmuserenr_el0, x1	// enable user mode access

	/* ----------------------------------------------------------------
	 * Allow non-privileged access to CNTVCT: Set CNTKCTL (Kernel Count
	 * register), bit 1 (EL0VCTEN) to enable access to CNTVCT/CNTFRQ
	 * registers from EL0.
	 * ----------------------------------------------------------------
	 */
	mrs	x0, cntkctl_el1
	orr	x0, x0, #EL0VCTEN_BIT
	msr	cntkctl_el1, x0
.endm

	/* -----------------------------------------------------
	 * unsigned int plat_is_my_cpu_primary(void);
	 *
	 * This function checks if this is the Primary CPU
	 * -----------------------------------------------------
	 */
func plat_is_my_cpu_primary
	mrs	x0, mpidr_el1
	and	x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
	cmp	x0, #PLAT_LOTUS_PRIMARY_CPU
	cset	x0, eq
	ret
endfunc plat_is_my_cpu_primary

	/* -----------------------------------------------------
	 * unsigned int plat_my_core_pos(void);
	 *
	 * result: CorePos = CoreId + (ClusterId << 2)
	 * -----------------------------------------------------
	 */
func plat_my_core_pos
	mrs	x0, mpidr_el1
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #8
	ret
endfunc plat_my_core_pos

	/* -----------------------------------------------------
	 * unsigned long plat_get_my_entrypoint (void);
	 *
	 * Main job of this routine is to distinguish between
	 * a cold and warm boot. If the tegra_sec_entry_point for
	 * this CPU is present, then it's a warm boot.
	 *
	 * -----------------------------------------------------
	 */
func plat_get_my_entrypoint
	adr	x1, lotus_sec_entry_point
	ldr	x0, [x1]
	ret
endfunc plat_get_my_entrypoint

	/* -----------------------------------------------------
	 * void plat_secondary_cold_boot_setup (void);
	 *
	 * This function performs any platform specific actions
	 * needed for a secondary cpu after a cold reset. Right
	 * now this is a stub function.
	 * -----------------------------------------------------
	 */
func plat_secondary_cold_boot_setup
	mov	x0, #0
	ret
endfunc plat_secondary_cold_boot_setup

	/* ---------------------------------------------
	 * int plat_crash_console_init(void)
	 * Function to initialize the crash console
	 * without a C Runtime to print crash report.
	 * Clobber list : x0 - x4
	 * ---------------------------------------------
	 */
func plat_crash_console_init
	mov_imm	x0, PLAT_LOTUS_UART_BASE
	mov_imm	x1, PLAT_LOTUS_UART_CLOCK_IN_HZ
	mov_imm	x2, PLAT_LOTUS_UART_BAUDRATE
	b	console_pl011_core_init
endfunc plat_crash_console_init

	/* ---------------------------------------------
	 * int plat_crash_console_putc(int c)
	 * Function to print a character on the crash
	 * console without a C Runtime.
	 * Clobber list : x1, x2
	 * ---------------------------------------------
	 */
func plat_crash_console_putc
	mov_imm	x1, PLAT_LOTUS_UART_BASE
	b	console_pl011_core_putc
endfunc plat_crash_console_putc

	/* ---------------------------------------------
	 * void plat_crash_console_flush()
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * Out : void.
	 * Clobber list : r0
	 * ---------------------------------------------
	 */
func plat_crash_console_flush
	mov_imm	x0, PLAT_LOTUS_UART_BASE
	b	console_pl011_core_flush
endfunc plat_crash_console_flush

	/* --------------------------------------------------------
	 * void platform_mem_init (void);
	 *
	 * Any memory init, relocation to be done before the
	 * platform boots. Called very early in the boot process.
	 * --------------------------------------------------------
	 */
func platform_mem_init
	mov	x0, #0
	ret
endfunc platform_mem_init

	/* ---------------------------------------------------
	 * Function to handle a platform reset and store
	 * input parameters passed by BL2.
	 * ---------------------------------------------------
	 */
func plat_reset_handler

	/* -----------------------------------
	 * derive and save the phys_base addr
	 * -----------------------------------
	 */
#if 0
	adr	x17, lotus_bl31_phys_base
	ldr	x18, [x17]
	cbnz	x18, 1f
	adr	x18, bl31_entrypoint
	str	x18, [x17]

1:	cpu_init_common
#endif
	ret
endfunc plat_reset_handler

	.data
	.align 3

	/* --------------------------------------------------
	 * CPU Secure entry point - resume from suspend
	 * --------------------------------------------------
	 */
lotus_sec_entry_point:
	.quad	0

	/* --------------------------------------------------
	 * NS world's cold boot entry point
	 * --------------------------------------------------
	 */
ns_image_entrypoint:
	.quad	0

	/* --------------------------------------------------
	 * BL31's physical base address
	 * --------------------------------------------------
	 */
lotus_bl31_phys_base:
	.quad	0

