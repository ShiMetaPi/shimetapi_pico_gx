.balign 4

#define ADDR_CEP64              (0x12120054)

.globl atf_32bit_resume
atf_32bit_resume:
	/* Load 64bit resume address from TEE_GEN4 which is store in aux code */
	ldr     r3, =ADDR_CEP64
	ldr     r2, [r3]
	lsr  r2, r2, #2

	/* Set 64bit resume address to PERI_CPU_RVBARADDR */
	mov	r3, #0x8000
	movt	r3, #0x1202
	str     r2, [r3, #0x10]

	mov r2, #0
	str r2, [r3, #0x14]

	/* Set all cpus booting-up in 64bit mode */
	ldr     r2, [r3, #0x0c]
	orr     r2, r2, #0xc0
	str     r2, [r3, #0x0c]

	/* Warm reset */
	isb
	dsb
	mrc     15, 0, r3, cr12, cr0, 2
	orr     r3, r3, #0x3
	mcr     15, 0, r3, cr12, cr0, 2
	isb
1:	wfi
	b       1b

