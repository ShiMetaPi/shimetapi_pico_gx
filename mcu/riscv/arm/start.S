#include <platform.h>

.arm
.globl _start
_start:
	push	{r4-r11, lr}
	bl	main
	pop	{r4-r11, pc}

	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	b	.                        /* bug here */

