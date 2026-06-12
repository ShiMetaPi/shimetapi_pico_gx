/* #define REG_IORW_DEBUG */
#include <io.h>
#include <platform.h>

unsigned long get_gpio_base(int group)
{
	return GPIO0_BASE + (group << 12);
}


void gpio_set_dir(int group, int bit, int dir)
{
	unsigned long base = get_gpio_base(group);
	unsigned int v = readl(base + GPIO_DIR);
	v &= ~(1 << bit);
	v |= dir << bit;
	writel(v, base + GPIO_DIR);
}

void gpio_set_data(int group, int bit, int data)
{
	unsigned long base = get_gpio_base(group);
	unsigned long data_addr = (base + (1 << (bit + 2)));

	writel(data << bit, data_addr);
}
