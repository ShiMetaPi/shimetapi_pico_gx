
#include <config.h>
#include <stdint.h>
#ifndef CONFIG_PRINT
#define CONFIG_PRINT
#endif
#include <stdio.h>
#include <stdlib.h>
#include <core.h>

void (*trap_c_callback)(void);

void trap_c(uint32_t *regs)
{
	int i;
	uint32_t vec = 0;

	vec = __get_MCAUSE() & 0x3FF;

	puts("CPU Exception: NO.");puthex(vec);puts("\n");

	for (i = 0; i < 31; i++) {
		puts("x");putdec(i+1);puts(": 0x");puthex(regs[i]);puts("\t");
		if ((i % 4) == 3) {
			puts("\n");
		}
	}

	puts("\n");
	puts("mepc	: 0x");puthex(regs[31]);puts("\n");
	puts("mstatus:	: 0x");puthex(regs[32]);puts("\n");

	if (trap_c_callback) {
		trap_c_callback();
	}

	while (1);
}

