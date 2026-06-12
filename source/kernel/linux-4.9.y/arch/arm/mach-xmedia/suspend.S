#include <linux/linkage.h>

#define XMEDIA_PM_DDR_DOWN_TO_24MHZ    0x1
#define XMEDIA_PM_DDR_OECONFIG         0x1
#define XMEDIA_PM_DDR_WFI_SLEEP        0x1
#define XMEDIA_UART_DEBUG              0x1

/*
md 0x120d8000;md 0x120d8200;md 0x120DC000; md 0x120DD000;md 0x120DD200
mm 0x120d8000 0x1
mm 0x120DC078 0x3104;mm 0x120DD014 0x2880;mm 0x120DD024  0x3000;mm 0x120DD218 0x3000; mm 0x120DD0C8 0xffff

1. 0x120DC078 set bit[7] 0
md 0x120DC000; 0x00003184 --> bit7 0 -->   mm 0x120DC078 0x3104

2. 0x120DD014 set bit[12] bit[8] 0
md 0x120DD000;0x00003980 -->bit8 bit12 0 -->         mm 0x120DD014 0x2880

3. 0x120DD024 set 0x3000
md 0x120DD000;               mm 0x120DD024 = 0x3000

4. 0x120DD218 set 0x3000
md 0x120DD200;             mm 0x120DD218 = 0x3000

5. 0x120DD0C8 set 0xffff
md 0x120DD000;            mm 0x120DD0C8 = 0xffff
*/

.data
/*
 * this code will be copied from
 * ddr to sram for system resumeing.
 * so it is ".data section".
 */
.arm
/* xmedia_sys_suspend must be 8-byte aligned per the requirements of fncpy() */
.align 3

ENTRY(xmedia_sys_suspend)
	// save back ddr address in ram jump_back_ddr_address (push to sp)
	str r0, r0_ram_addr
	str r1, r1_ram_addr
	str r2, r2_ram_addr
	str r3, r3_ram_addr
	str r4, r4_ram_addr
	str r5, r5_ram_addr
	str r6, r6_ram_addr
	str r7, r7_ram_addr
	str r8, r8_ram_addr
	str r9, r9_ram_addr
	str r10, r10_ram_addr

	mov r10, lr
	str r10, jump_back_ddr_address

	// r7 -- oeflage  r8 -- cpufreq
	ldr r7, xmedia_ddr_oeflage
	ldr r8, ddr_cpufreq

	// xmedia_peri_crg_base vir base in r9
	ldr r9, xmedia_peri_crg_base

	// xmedia_ddrc_base vir base in r10
	ldr r10, xmedia_ddrc_base

	/* cpu to target freq */
	ldr r0, peri_crg32_addr
	add r0, r0, r9
	ldr r1, cpu_freq_900mhz

	@ 001b -- 900Mhz
	@ 011b -- 600Mhz
	@ 000b -- 24Mhz
	cmp r8, #900
	bne get_freq_600mhz
	ldr r1, cpu_freq_900mhz

get_freq_600mhz:
	cmp r8, #600
	bne get_freq_24mhz
	ldr r1, cpu_freq_600mhz

get_freq_24mhz:
	cmp r8, #24
	bne mod_cpu_freq
	ldr r1, cpu_freq_24mhz

mod_cpu_freq:
	// load crg 0x80, clear bit0-1, orr cpu freq
	ldr r2, [r0]
	bic r2, r2, #0x0000003 @ clear bits 0,1 (--V-)
	orr r1, r1, r2
	str r1, [r0]

	/* wait cpu 300/8 = 38us for stable */
	ldr r0, xmedia_ddrc_statble_loop
	mov r0, r0, lsr #3
cpu_down_stable_loop:
	sub r0, r0, #1
	cmp r0, #0
	bne cpu_down_stable_loop
	/* end cpu to freq */

#ifdef XMEDIA_UART_DEBUG
	// footprint 0
	ldr r0, xmedia_debuguart_base
	ldr r1, debugchar_zero
	str r1, [r0]
	ldr r1, xmedia_ddr_oeflage
	add r1, r1, #0x21
	str r1, [r0]
#endif

	// ddr enter self refresh
	// 0x120d8000 = 1 ddr -> self refresh
	ldr	r0, ddrc_ctrl_sref
	add r0, r0, r10
	mov	r1, #1
	str r1, [r0]

	ldr r0, ddrc_curr_func
	add r0, r0, r10
wait_refresh_status:
	// ddr self refresh status
	ldr r1, [r0]
	and r1, r1, #0x1
	cmp r1, #0x1
	bne wait_refresh_status

#ifdef XMEDIA_UART_DEBUG
	// footprint 1
	ldr r0, xmedia_debuguart_base
	ldr r1, debugchar_one
	str r1, [r0]
#endif

	// not set oe config jump to no_oeconfig
	cmp r7, #1
	bne no_oeconfig

	/* close ddr io */
	// io oe
	/* 0x120DC078 mask bit7 = 0 */
	ldr r0, ddr_dmc_phyctrl0
	add r0, r0, r10
	ldr r1, [r0]
	mov r2, r1	@ save ddr_dmc_phyctrl0 value in r2
	str r2, save_ddr_dmc_phyctrl0 @ save ddr_dmc_phyctrl0 value in  save_ddr_dmc_phyctrl0 addr
	bic r1, r1, #0x00000080 @ clear bits 7 (--V-)
	str r1, [r0]
wait_dmc_phyctrl0_set_ready:
	ldr r1, [r0]
	and r1, #0x00000080  @bit7 must be zero
	cmp r1, #0x0
	bne wait_dmc_phyctrl0_set_ready
	/* end 0x120DC078 */

#ifdef XMEDIA_UART_DEBUG
	// footprint 2
	ldr r0, xmedia_debuguart_base
	ldr r1, debugchar_two
	str r1, [r0]
#endif

	// cs/ck oe config
	/* 0x120DD014 mask bit8 & bit12 = 0 */
	ldr r0, ddr_acstatic_phyctrl0
	add r0, r0, r10
	ldr r1, [r0]
	mov r3, r1  @ save ddr_acstatic_phyctrl0 value in save_ddr_acstatic_phyctrl0
	str r3, save_ddr_acstatic_phyctrl0
	bic r1, r1, #0x00001100 @ clear bits 8 & 12 (--V-)
	str r1, [r0]
wait_acstatic_phyctrl0_set_ready:
	ldr r1, [r0]
	tst r1, #0x00001100   @bit8 & 12 must zero
	bne wait_acstatic_phyctrl0_set_ready
	/* end 0x120DD014 */

#ifdef XMEDIA_UART_DEBUG
	// footprint 3
	ldr r0, xmedia_debuguart_base
	ldr r1, debugchar_three
	str r1, [r0]
#endif

	// verf
	/* 0x120DD024 = 0x3000  */
	ldr r0, ddr_acstatic_ioctrl1
	add r0, r0, r10
	ldr r1, [r0]  @ save ddr_acstatic_ioctrl1 value in r4
	str r1, save_ddr_acstatic_ioctrl1
	ldr r1, =0x3000
	str r1, [r0]
	/* end  0x120DD024 */

	/* 0x120DD218 = 0x3000 */
	ldr r0, ddr_dxstatic_ioctrl1
	add r0, r0, r10
	ldr r5, [r0] @ save ddr_dxstatic_ioctrl1 value in r5
	str r5, save_ddr_dxstatic_ioctrl1
	ldr r1, =0x3000
	str r1, [r0]
	/* end  0x120DD218 */

#ifdef XMEDIA_UART_DEBUG
	// footprint 4
	ldr r0, xmedia_debuguart_base
	ldr r1, debugchar_four
	str r1, [r0]
#endif

	/* 0x120DD0C8 = 0xffff */
	ldr r0, ddr_dxstatic_ioctrl5
	add r0, r0, r10
	ldr r6, [r0]  @ save ddr_dxstatic_ioctrl5 value in r6
	str r6, save_ddr_dxstatic_ioctrl5
	mov r1, #0xffff
	str r1, [r0]
	/* end 0x120DD0C8 */

#ifdef XMEDIA_UART_DEBUG
	// footprint 5
	ldr r0, xmedia_debuguart_base
	ldr r1, debugchar_five
	str r1, [r0]
#endif

#ifdef XMEDIA_PM_DDR_DOWN_TO_24MHZ
	/* set DDR to 24Mhz */
	ldr r0, peri_crg32_addr
	add r0, r0, r9
	ldr r1, [r0]
	bic r1, r1, #0x00018  @mask bit 3-5 to zero
	str r1, [r0]

	ldr r0, peri_crg31_addr
	add r0, r0, r9
	ldr r1, [r0]
	bic r1, r1, #0x0004  @ mask bit2 to zero
	str r1, [r0]
	/* end to DDR pll source */
#endif

	/* ddr io close done */
no_oeconfig:

	/* ddr wait 300us for stable */
	ldr r0, xmedia_ddrc_statble_loop
ddrc_oeclose_stable_loop:
	sub r0, r0, #1
	cmp r0, #0
	bne ddrc_oeclose_stable_loop

#ifdef XMEDIA_UART_DEBUG
	// footprint 6
	ldr r0, xmedia_debuguart_base
	ldr r1, debugchar_six
	ldr r2, debugchar_lf
	ldr r3, debugchar_cr
	str r1, [r0]
	str r2, [r0]
	str r3, [r0]
#endif

#if XMEDIA_PM_DDR_WFI_SLEEP != 1
	ldr r0, wait_loop_count
deadloop:
	sub r0, r0, #1
	cmp r0, #0
	bne deadloop
	@bl deadloop
#else
	// cpu enter wfi
	wfi
#endif
	nop
	nop
	nop
	isb
	dsb

#ifdef XMEDIA_UART_DEBUG
	// footprint 6
	ldr r0, xmedia_debuguart_base
	ldr r1, debugchar_six
	str r1, [r0]
#endif

	cmp r7, #1
	bne no_oerestore

#ifdef XMEDIA_PM_DDR_DOWN_TO_24MHZ
	/* set DDR back to 300Mhz */
	ldr r0, peri_crg32_addr
	add r0, r0, r9
	ldr r1, [r0]
	orr r1, r1, #0x00018  @mask bit 3-5 to 1
	str r1, [r0]

	ldr r0, peri_crg31_addr
	add r0, r0, r9
	ldr r1, [r0]
	orr r1, r1, #0x0004  @ mask bit2 to 1
	str r1, [r0]
	/* end to DDR source */
#endif

	/* cpu wait 300us for stable */
	ldr r0, xmedia_ddrc_statble_loop
ddrc_up_stable_loop:
	sub r0, r0, #1
	cmp r0, #0
	bne ddrc_up_stable_loop

#ifdef XMEDIA_UART_DEBUG
	// footprint 5
	ldr r0, xmedia_debuguart_base
	ldr r1, debugchar_five
	str r1, [r0]
#endif

	// verf
	/* restore ddr_dxstatic_ioctrl5 from r6 */
	/* 0x120DD0C8 */
	ldr r0, ddr_dxstatic_ioctrl5
	add r0, r0, r10
	ldr r1, save_ddr_dxstatic_ioctrl5
	str r1, [r0]

	/* restore ddr_dxstatic_ioctrl1 from r5 */
	/* 0x120DD218 */
	ldr r0, ddr_dxstatic_ioctrl1
	add r0, r0, r10
	ldr r1, save_ddr_dxstatic_ioctrl1
	str r1, [r0]
	/* end 0x120DD218 */

	/* restore ddr_acstatic_ioctrl1 from r4 */
	/* 0x120DD024 */
	ldr r0, ddr_acstatic_ioctrl1
	add r0, r0, r10
	ldr r1, save_ddr_acstatic_ioctrl1
	str r1, [r0]
	/* end 0x120DD024 */

#ifdef XMEDIA_UART_DEBUG
	// footprint 4
	ldr r0, xmedia_debuguart_base
	ldr r1, debugchar_four
	str r1, [r0]
#endif

	// io oe
	/* restore ddr_dmc_phyctrl0 from r2 bit 7 = 1 */
	/* 0x120DC078 */
	ldr r0, ddr_dmc_phyctrl0
	add r0, r0, r10
	ldr r1, save_ddr_dmc_phyctrl0 @load restore val from save_ddr_dmc_phyctrl0 to r1
	str r1, [r0]
wait_dmc_phyctrl0_ready:
	ldr r2, [r0]
	cmp r2, r1
	bne wait_dmc_phyctrl0_ready
	/* end 0x120DC078 */

#ifdef XMEDIA_UART_DEBUG
	// footprint 3
	ldr r0, xmedia_debuguart_base
	ldr r1, debugchar_three
	str r1, [r0]
#endif

	// cs/ck oe config
	/* restore ddr_acstatic_phyctrl0 from save_ddr_acstatic_phyctrl0  bit 8&12 = 1 */
	/* 0x120DD014 */
	ldr r0, ddr_acstatic_phyctrl0
	add r0, r0, r10
	ldr r1, save_ddr_acstatic_phyctrl0
	str r1, [r0]
wait_acstatic_phyctrl0_ready:
	ldr r2, [r0]
	cmp r2, r1
	bne wait_acstatic_phyctrl0_ready
	/* end 0x120DD014 */

#ifdef XMEDIA_UART_DEBUG
	// footprint 2
	ldr r0, xmedia_debuguart_base
	ldr r1, debugchar_three
	str r1, [r0]
#endif

/* no oe config flage exit place*/
no_oerestore:

	/* cpu wait 300us for stable */
	ldr r0, xmedia_ddrc_statble_loop
ddrc_oeopen_stable_loop:
	sub r0, r0, #1
	cmp r0, #0
	bne ddrc_oeopen_stable_loop

	/* exit self refresh */
	ldr r0, ddrc_ctrl_sref
	add r0, r0, r10
	mov r1, #2
	str r1, [r0]

	ldr r0, ddrc_curr_func
	add r0, r0, r10
wait_refresh_exit:
	// ddr self refresh status
	ldr r1, [r0]
	and r1, r1, #0x1
	cmp r1, #0x0
	bne wait_refresh_exit

#ifdef XMEDIA_UART_DEBUG
	// footprint 1
	ldr r0, xmedia_debuguart_base
	ldr r1, debugchar_one
	str r1, [r0]
#endif

	// set cpu freq up to 900Mhz
	ldr r0, peri_crg32_addr
	add r0, r0, r9
	ldr r2, [r0]
	bic r2, r2, #0x0000003 @ clear bits 0-1 (--V-)
	ldr r1, cpu_freq_900mhz
	orr r1, r1, r2
	str r1, [r0]

	/* cpu wait 300us for stable */
	ldr r0, xmedia_ddrc_statble_loop
	mov r0, r0, lsr #1
cpu_up_stable_loop:
	sub r0, r0, #1
	cmp r0, #0
	bne cpu_up_stable_loop

#ifdef XMEDIA_UART_DEBUG
	// footprint 0
	ldr r0, xmedia_debuguart_base
	ldr r1, debugchar_zero
	str r1, [r0]
#endif

	// restore ddr address from ram jump_back_ddr_address (push from sp)
	ldr r0, r0_ram_addr
	ldr r1, r1_ram_addr
	ldr r2, r2_ram_addr
	ldr r3, r3_ram_addr
	ldr r4, r4_ram_addr
	ldr r5, r5_ram_addr
	ldr r6, r6_ram_addr
	ldr r7, r7_ram_addr
	ldr r8, r8_ram_addr
	ldr r9, r9_ram_addr
	ldr r10, r10_ram_addr

	// get back jump addr
	ldr lr, jump_back_ddr_address
	mov pc, lr
ENDPROC(xmedia_sys_suspend)

ENTRY(xmedia_sys_resume)
	nop
	nop
	nop
	dsb
	isb
	@wfi
	mov pc, lr
ENDPROC(xmedia_sys_resume)

/* Parameters filled in by the kernel */

/* ddr enter self fresh */
	@.globl ddrc_ctrl_sref
ddrc_ctrl_sref:
	.long 0x8000

/* ddr enter self refresh status */
	@.globl ddrc_curr_func
ddrc_curr_func:
	.long 0x8294

/* dmc phyctrl0 */
	@.globl ddr_dmc_phyctrl0
ddr_dmc_phyctrl0:
	.long 0xC078

/* save dmc phyctrl0 */
	@.globl save_ddr_dmc_phyctrl0
save_ddr_dmc_phyctrl0:
	.long 0xC078

/* ac static phyctrl0 */
	@.globl ddr_acstatic_phyctrl0
ddr_acstatic_phyctrl0:
	.long 0xD014

/* save ac static phyctrl0 */
	@.globl save_ddr_acstatic_phyctrl0
save_ddr_acstatic_phyctrl0:
	.long 0xD014

/* ac static IOCTRL1 */
	@.globl ddr_acstatic_ioctrl1
ddr_acstatic_ioctrl1:
	.long 0xD024

/* save ac static IOCTRL1 */
	@.globl save_ddr_acstatic_ioctrl1
save_ddr_acstatic_ioctrl1:
	.long 0xD024

/* dx static ioctrl1 */
	@.globl ddr_dxstatic_ioctrl1
ddr_dxstatic_ioctrl1:
	.long 0xD218

/* save dx static ioctrl1 */
	@.globl save_ddr_dxstatic_ioctrl1
save_ddr_dxstatic_ioctrl1:
	.long 0xD218

/* ac static ioctrl5 */
	@.globl ddr_dxstatic_ioctrl5
ddr_dxstatic_ioctrl5:
	.long 0xD0C8

/* save ac static ioctrl5 */
	@.globl save_ddr_dxstatic_ioctrl5
save_ddr_dxstatic_ioctrl5:
	.long 0xD0C8

/* original addr */
	@.globl exit_back_addr
exit_back_addr:
	.long 0

	@.globl peri_crg31_addr
peri_crg31_addr:
	.long 0x007C

/* cpu freq addr  */
	@.globl peri_crg32_addr
peri_crg32_addr:
	.long 0x0080

/* loop 1s */
	@.globl wait_loop_count
wait_loop_count:
	@.long 90000
	.long 9000000

/* ddr_oeflage  1s */
	.globl xmedia_ddr_oeflage
xmedia_ddr_oeflage:
	.long XMEDIA_PM_DDR_OECONFIG

/* ddr_cpufreq 24 */
	@.globl ddr_cpufreq
ddr_cpufreq:
	.long 24

/* ddr refresh loop 1 */
	@.globl ddr_refresh_loop
ddr_refresh_loop:
	.long 1

/* cpu freq regval 900Mhz = bit1-0: 01b */
	@.globl cpu_freq_900mhz
cpu_freq_900mhz:
	.long 0x01

/* cpu freq regval 600Mhz = bit1-0: 11b */
	@.globl cpu_freq_600mhz
cpu_freq_600mhz:
	.long 0x03

/* cpu freq regval 24Mhz = bit1-0: 00b */
	@.globl cpu_freq_24mhz
cpu_freq_24mhz:
	.long 0x00

/* ddrc oe need more than 100us stable
   900Mhz 300us = loop 900*300 = 270000
   24Mhz 300us = loop 24*300 = 7200
 */
xmedia_ddrc_statble_loop:
	.long 12000

/* back address */
	@.globl jump_back_ddr_address
jump_back_ddr_address:
	.long 0x0

/*save register from r0 - r10 */
	@.globl r0_ram_addr
r0_ram_addr:
	.long 0xbeef0
	@.globl r1_ram_addr
r1_ram_addr:
	.long 0xbeef1
	@.globl r2_ram_addr
r2_ram_addr:
	.long 0xbeef2
	@.globl r3_ram_addr
r3_ram_addr:
	.long 0xbeef3
	@.globl r4_ram_addr
r4_ram_addr:
	.long 0xbeef4
	@.globl r5_ram_addr
r5_ram_addr:
	.long 0xbeef5
	@.globl r6_ram_addr
r6_ram_addr:
	.long 0xbeef6
	@.globl r7_ram_addr
r7_ram_addr:
	.long 0xbeef7
	@.globl r8_ram_addr
r8_ram_addr:
	.long 0xbeef8
	@.globl r9_ram_addr
r9_ram_addr:
	.long 0xbeef9
	@.globl r10_ram_addr
r10_ram_addr:
	.long 0xbeefa

	.globl xmedia_ddrc_base
xmedia_ddrc_base:
	.long 0x120d0000

	.globl xmedia_peri_crg_base
xmedia_peri_crg_base:
	.long 0x12010000

	.globl xmedia_debuguart_base
xmedia_debuguart_base:
	.long 0x12040000
#ifdef XMEDIA_UART_DEBUG

debugchar_zero:
	.long 0x30
debugchar_one:
	.long 0x31
debugchar_two:
	.long 0x32
debugchar_three:
	.long 0x33
debugchar_four:
	.long 0x34
debugchar_five:
	.long 0x35
debugchar_six:
	.long 0x36
debugchar_lf:
	.long 0x0a
debugchar_cr:
	.long 0x0d

	@"
debugchar_kh:
	.long 0x22
#endif

/* mark flage */
	@.globl mark_flag
mark_flag:
	.long 0xdeadbeef

ENTRY(xmedia_sys_suspend_sz)
	.word   . - xmedia_sys_suspend
