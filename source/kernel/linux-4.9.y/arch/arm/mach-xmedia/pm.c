#include <linux/suspend.h>
#include <linux/io.h>
#include <asm/cacheflush.h>
#include <asm/fncpy.h>

void xmedia_sys_suspend(void);

extern unsigned long xmedia_ddrc_base;
extern unsigned long xmedia_peri_crg_base;
extern unsigned long xmedia_sys_suspend_sz;
extern unsigned long xmedia_debuguart_base;

static void __iomem *iram_base = NULL;
static void (*xmedia_suspend_ptr) (void) = NULL;

#define XMEDIA_IRAM_BASE 0x04010000
#define XMEDIA_XM72050200_DDRC_BASE 0x120d0000
#define XMEDIA_XM72050200_DDRC_SIZE 0xD300
#define XMEDIA_XM72050200_PERI_CRG_BASE 0x12010000
#define XMEDIA_XM72050200_PERI_CRG_SIZE  0x200
#define XMEDIA_XM72050200_DEBUG_UART 0x12040000
#define IRAM_SIZE (64 * 1024)

static int xmedia_pm_enter(suspend_state_t state)
{
	flush_cache_all();
	xmedia_suspend_ptr();

	return 0;
}

static const struct platform_suspend_ops xmedia_pm_ops = {
	.valid	= suspend_valid_only_mem,
	.enter	= xmedia_pm_enter,
};

static int __init xmedia_pm_init(void)
{
	iram_base = __arm_ioremap_exec(XMEDIA_IRAM_BASE, IRAM_SIZE, false);
	memset(iram_base, 0, IRAM_SIZE);

	xmedia_debuguart_base = ioremap(XMEDIA_XM72050200_DEBUG_UART, 0x100);
	xmedia_ddrc_base = ioremap(XMEDIA_XM72050200_DDRC_BASE, XMEDIA_XM72050200_DDRC_SIZE);
	xmedia_peri_crg_base = ioremap(XMEDIA_XM72050200_PERI_CRG_BASE, XMEDIA_XM72050200_PERI_CRG_SIZE);

	xmedia_suspend_ptr = fncpy((void *)iram_base, &xmedia_sys_suspend, xmedia_sys_suspend_sz);
	if (xmedia_suspend_ptr)
		suspend_set_ops(&xmedia_pm_ops);
	else
		pr_info("Xmedia: PM not supported, due to no SRAM allocated\n");

	return 0;
}
arch_initcall(xmedia_pm_init);
