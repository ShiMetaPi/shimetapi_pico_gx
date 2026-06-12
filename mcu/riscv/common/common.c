
#include <stddef.h>
#include <common.h>
#include <time.h>

void wait_stage2_ready(void)
{
	int c = 0;

	while (readl(REG_SC_SYSBOOT7) != 0xbeef0001) {
		udelay(1);
		if (c > 0 && (c % (1000 * 1000 * 2)) == 0) {
			puts("Waiting firmware ready!\n");
		}
		c++;
	}
}

int run_stage2_func(int (*func)(void))
{
	int ret;

	__DSB();
	__ISB();

	puts("Check firmware...\n");

	wait_stage2_ready();

	rv32_dcache_disable();
	rv32_icache_disable();


	if (readl((ulong *)__bin_end_pad) != CONFIG_BIN_END_PAD) {
		puts("Firmware ERROR!\n");
		return -1;
	}

	rv32_icache_enable();
	rv32_dcache_enable();

	puts("Run 0x");puthex((u32)func);puts("\n");
	ret = func();

	return ret;
}
