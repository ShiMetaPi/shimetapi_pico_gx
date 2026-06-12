#include <platform.h>
#include <io.h>
#include <lib.h>
#include <common.h>

void reset_cpu(void)
{
}

u64 get_ddr_size(void)
{
	u64 size = readl(REG_SC_DDRT12);
	assert(size != 0);

	return size;
}

