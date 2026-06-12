/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __STEP_H_
#define __STEP_H_

#include <config.h>

#ifdef CONFIG_LOTUS_EDA
#ifdef __ASSEMBLY__

.macro step, num
	push	{r11, r12}

	ldr	r11, = SYS_CTRL_REG_BASE
	mov	r12, #\num
	str	r12, [r11, #REG_SC_SYSBOOT2]

	pop	{r11, r12}
.endm

.macro step_no_stack, num, tmp_reg1, tmp_reg2
	ldr	\tmp_reg1, = SYS_CTRL_REG_BASE
	mov	\tmp_reg2, #\num
	str	\tmp_reg2, [\tmp_reg1, #REG_SC_SYSBOOT2]
.endm

#else
#define step(x)  ((*(volatile u32 *) (SYS_CTRL_REG_BASE + REG_SC_SYSBOOT2)) = (x))
#endif
#else
#ifdef __ASSEMBLY__
.macro step, num
.endm
.macro step_no_stack, num, tmp_reg1, tmp_reg2
.endm
#else
#define step(x)
#endif
#endif

#endif /*__STEP_H_*/
