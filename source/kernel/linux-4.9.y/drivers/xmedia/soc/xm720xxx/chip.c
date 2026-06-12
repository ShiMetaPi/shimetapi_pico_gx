#include <linux/kernel.h>
#include <mach/platform.h>
#include <asm/io.h>
#include <linux/printk.h>

unsigned int g_cpu_chipid = 0;

unsigned int get_chipid(void)
{
	void *sysid = NULL;
	unsigned int version_id = 0;

	if (g_cpu_chipid == 0) {
		sysid = ioremap(REG_VERSION_ID, PAGE_SIZE);
		if (sysid == NULL) {
			printk(KERN_ERR "soc chipid reg ioremap failed\n");
			return 0;
		}
		version_id = (unsigned int)readl(sysid);
		iounmap(sysid);
		sysid = NULL;
		switch((version_id >> 4) & 0xf) {
			case VERSIONID_XM7205V200:
				g_cpu_chipid = CHIPID_XM72050200;
				break;
			case VERSIONID_XM7202V300:
				g_cpu_chipid = CHIPID_XM72020300;
				break;
			case VERSIONID_XM7205V300:
				g_cpu_chipid = CHIPID_XM72050300;
				break;
			case VERSIONID_XM7605V100:
				g_cpu_chipid = CHIPID_XM76050100;
				break;
			case VERSIONID_XM7205V210:
				g_cpu_chipid = CHIPID_XM72050210;
				break;
			case VERSIONID_XM7205V500:
				g_cpu_chipid = CHIPID_XM72050500;
				break;
			case VERSIONID_XM7205V510:
				g_cpu_chipid = CHIPID_XM72050510;
				break;
			case VERSIONID_XM7205V530:
				g_cpu_chipid = CHIPID_XM72050530;
				break;
			case VERSIONID_XM7205V230:
				g_cpu_chipid = CHIPID_XM72050230;
				break;
			case VERSIONID_XM7202V330:
				g_cpu_chipid = CHIPID_XM72020330;
				break;
			 case VERSIONID_XMEXPENDID1:
			 case VERSIONID_XMEXPENDID2:
			 case VERSIONID_XMEXPENDID3:
				switch ((version_id >> 1) & 0x7f) {
					case VERSIONID_XM7201V200:
						g_cpu_chipid = CHIPID_XM72010200;
						break;
					case VERSIONID_XM7201V300:
						g_cpu_chipid = CHIPID_XM72010300;
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}
	}

	return g_cpu_chipid;
}
EXPORT_SYMBOL(get_chipid);
