#include <common.h>
#include <asm/io.h>
#include <asm/arch/platform.h>

unsigned int g_cpu_chipid = 0;


unsigned int get_chipid(void)
{
	if (g_cpu_chipid == 0)
		g_cpu_chipid = readl(REG_SC_CHIPID);

	return g_cpu_chipid;
}

