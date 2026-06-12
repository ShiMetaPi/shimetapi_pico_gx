
#include <common.h>
#include <image.h>
#include <linux/compiler.h>
#include <bootm.h>
#include <asm/io.h>
#include <linux/kernel.h>
#include <asm/mach-types.h>
#include <linux/libfdt.h>
#include <mapmem.h>
#include <serial.h>
#include <linux/lotus/flash_read.h>
#include <linux/lotus/timestamp.h>

unsigned long ticks2usec(unsigned long  ticks)
{
        ulong tbclk = get_tbclk();

        tbclk /= 1000000;
        ticks /= tbclk;
        return ((ulong)ticks);
}


unsigned long relocate_dtb(bootm_headers_t *bootm_hdr)
{
	image_info_t os;
	void *image_buf;
	ulong image_size;

	if (!bootm_hdr) {
		pr_err("Invalid params for %s\n", __func__);
		return 0;
	}

	os = bootm_hdr->os;
	image_size = os.image_len;
	image_buf = map_sysmem(os.image_start, image_size);
	pr_info("Image addr 0x%lX, len 0x%lX\n", (ulong)image_buf, image_size);

	memmove((char *)CONFIG_DTB_BASE, (char *)(image_buf + image_size), CONFIG_DTB_MAX_SIZE);

	return CONFIG_DTB_BASE;
}

void show_boot_timestamp(void)
{
	char *show_timestamp;

	TIME_STAMP(0);
	stopwatch_trigger();
	show_timestamp = env_get("timestamp");
	if (*show_timestamp == 'y') {
		serial_enable_output(true);
		timestamp_print(0);
		stopwatch_print();
	}
}

#ifndef CONFIG_AUTOBOOT
int board_run_command(const char *cmdline)
{
	int ret = -1;
	int (*entry)(void *) = (void *)CONFIG_SBL_LOAD_ADDR;

	ret = flash_read(CONFIG_SBL_OFFSET, CONFIG_SBL_SIZE, (void *)CONFIG_SBL_LOAD_ADDR);
	if (ret != 0) {
		pr_err("Fail to read from flash\n");
		goto error;
	}

	printf ("## Starting SBL at 0x%08lX ...\n", (ulong)CONFIG_SBL_LOAD_ADDR);

	dsb();
	isb();
	ret = entry(NULL);

	printf ("## SBL terminated, ret = %d\n", ret);

error:
	return 1;
}

#endif
