/*
 * Copyright (c) LOTUS. All rights reserved.
 */

#define pr_fmt(fmt) "%s: " fmt, "BOOT BL31"

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
#include <linux/lotus/atf.h>

#define OPTEE_MAGIC                     0x4554504f

extern int cleanup_before_linux(void);

struct optee_header {
	uint32_t magic;
	uint8_t version;
	uint8_t arch;
	uint16_t flags;
	uint32_t init_size;
	uint32_t init_load_addr_hi;
	uint32_t init_load_addr_lo;
	uint32_t init_mem_usage;
	uint32_t paged_size;
};

int do_load_optee_os(ulong addr, uint32_t *rtos_load_addr, uint32_t *aarch_mode)
{
	struct optee_header *imgheader = (struct optee_header *)addr;
	char *src = (char *)(addr + sizeof(struct optee_header)), *dst = (char *)imgheader->init_load_addr_lo;
	uint32_t loadaddr, img_size;

	printf("## OpenTEE OS Image:\n");
	printf("   version: 0x%X\n", imgheader->version);
	printf("   arch: %s\n", ((imgheader->version) ? "arm32" : "arm64"));
	printf("   init_size: 0x%X\n", imgheader->init_size);
	printf("   load_addr_hi: 0x%x (%u KB)\n", imgheader->init_load_addr_hi, imgheader->init_load_addr_hi >> 10);
	printf("   load_addr_lo: 0x%x (%u KB)\n", imgheader->init_load_addr_lo, imgheader->init_load_addr_lo >> 10);
	printf("   init_mem_usage: %d\n", imgheader->init_mem_usage);
	printf("   Tpaged_size: 0x%X (%u KB)\n", imgheader->paged_size, imgheader->paged_size >> 10);

	if (aarch_mode)
		*aarch_mode = (imgheader->version == 1 ? MODE_RW_32 : MODE_RW_64);

	loadaddr = imgheader->init_load_addr_lo;
	img_size = imgheader->init_size;

	memmove(dst, src, img_size);

	if (rtos_load_addr)
		*rtos_load_addr = (uint32_t)imgheader->init_load_addr_lo;

	printf("   succeed to load Optee-OS to :0x%x\n", loadaddr);

	return 0;
}

int is_optee_img(char *buf)
{
	struct optee_header *hdr = (struct optee_header *)buf;

	if (hdr->magic != OPTEE_MAGIC)
		return 0;

	return 1;
}

void run_bl31(bootm_headers_t *bootm_hdr, uint32_t machid)
{
	uint32_t arch;
	image_header_t *hdr = NULL;
	image_info_t os;
	entry_point_info_t bl32_ep;
	entry_point_info_t bl33_ep;
	bl31_params_t bl31_p;
	ulong bl31_pc = 0;
	u32 val = 0;
	void *image_buf;
	ulong image_size;
	char *show_timestamp;

	if (!bootm_hdr) {
		pr_err("Invalid params for %s\n", __func__);
		return;
	}

	hdr = &(bootm_hdr->legacy_hdr_os_copy);
	os = bootm_hdr->os;
	image_size = os.image_len;
	image_buf = map_sysmem(os.image_start, image_size);

	pr_info("Image addr 0x%lX, len 0x%lX\n", (ulong)image_buf, image_size);

	memset(&bl31_p, 0, sizeof(bl31_params_t));
	memset(&bl32_ep, 0, sizeof(entry_point_info_t));
	memset(&bl33_ep, 0, sizeof(entry_point_info_t));
	bl31_p.bl32_ep_info = (uint64_t)((uint32_t)&bl32_ep);
	bl31_p.bl33_ep_info = (uint64_t)((uint32_t)&bl33_ep);

	if (image_get_arch(hdr) == IH_ARCH_ARM) {
		arch = BL33_IMG_ARM32;
		pr_info("Start bl31 with aarch32 bl33...\n");
	} else if (image_check_arch(hdr, IH_ARCH_ARM64)) {
		arch = BL33_IMG_ARM64;
		pr_info("Start bl31 with aarch64 bl33...\n");
	} else {
		pr_err("Invalid ARCH Type!\n");
		return;
	}

	if(arch == BL33_IMG_ARM64) {
		bl33_ep.pc = (uint64_t)(image_get_ep(hdr));
		memmove((char *)CONFIG_DTB_BASE, (char *)(image_buf + image_size), CONFIG_DTB_MAX_SIZE);
		bl33_ep.args.arg0 = (uint64_t)(CONFIG_DTB_BASE);
		bl33_ep.args.arg1 = 0;
		bl33_ep.args.arg2 = 0;
		bl33_ep.args.arg3 = 0;
		bl33_ep.spsr = spsr_64(MODE_EL1,MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	} else if (arch == BL33_IMG_ARM32) {
		bl33_ep.pc = (uint64_t)(image_get_ep(hdr));
		memmove((char *)CONFIG_DTB_BASE, (char *)(image_buf + image_size), CONFIG_DTB_MAX_SIZE);
		bl33_ep.args.arg0 = 0;
		bl33_ep.args.arg1 = (uint64_t)(machid);
		bl33_ep.args.arg2 = (uint64_t)(CONFIG_DTB_BASE);
		bl33_ep.spsr = spsr_mode32(MODE32_SVC, 0x0 , EP_EE_LITTLE, DISABLE_ALL_EXCEPTIONS);
	}

	if (is_optee_img((char *)CONFIG_BL32_BASE)) {
		uint32_t rtos_load_addr;
		uint32_t aarch_mode;

		do_load_optee_os(CONFIG_BL32_BASE, &rtos_load_addr, &aarch_mode);

		bl32_ep.pc = rtos_load_addr;
		bl32_ep.args.arg0 = aarch_mode;
		bl32_ep.args.arg1 = 0;
		bl32_ep.spsr = 0;
	}

	pr_info("BL33 SPSR: 0x%X\n", bl33_ep.spsr);
	pr_info("DTB: 0x%llX\n", bl33_ep.args.arg0);

	TIME_STAMP(0);
	stopwatch_trigger();
	show_timestamp = env_get("timestamp");
	if (*show_timestamp == 'y') {
		serial_enable_output(true);
		timestamp_print(0);
		stopwatch_print();
	}

	bl31_pc = CONFIG_BL31_BASE;

	writel((u32)(bl31_pc >> 2 & 0xFFFFFFFF), REG_SYS_RVBAR_ADDR0);
	writel(0, REG_SYS_RVBAR_ADDR1);
        writel(readl(REG_SYS_CPU_AARCH_MODE) | (0x3 << 14), REG_SYS_CPU_AARCH_MODE);

        (*(volatile uint64_t *)0) = (uint64_t)((uintptr_t)&bl31_p);
        (*(volatile uint64_t *)8) = (uint64_t)((uint32_t)NULL);

	/* __asm__ __volatile__("b ."); */

        pr_info("Start Warm Reseting\n");

        cleanup_before_linux();

        /* warm reseting */
        __asm__ __volatile__("isb\n\r"
                "dsb\n\r"
                "mrc p15, 0, %0, c12, c0, 2\n\r"
                "orr %0, %0, #0x3\n\r"
                "mcr p15, 0, %0, c12, c0, 2\n\r"
                "isb\n\r"
                "wfi":"=r"(val)::"cc");
        pr_err("Fail to warm resetting...\n");
        hang();

}

