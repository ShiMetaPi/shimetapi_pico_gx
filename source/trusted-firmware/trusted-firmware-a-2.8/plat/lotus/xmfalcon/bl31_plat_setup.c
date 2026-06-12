/*
 * Copyright (c) Lotus. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include <assert.h>
#include <bl_common.h>
#include <common_def.h>
#include <console.h>
#include <debug.h>
#include <generic_delay_timer.h>
#include <mmio.h>
#include <pl011.h>
#include <plat_private.h>
#include <platform.h>
#include <plat_lotus.h>

struct bl31_params {
       param_header_t h;
       image_info_t *bl31_image_info;
       entry_point_info_t *bl32_ep_info;
       image_info_t *bl32_image_info;
       entry_point_info_t *bl33_ep_info;
       image_info_t *bl33_image_info;
};


static entry_point_info_t bl33_image_ep_info, bl32_image_ep_info;

/*******************************************************************************
 * This variable holds the non-secure image entry address
 ******************************************************************************/
extern uint64_t ns_image_entrypoint;

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	assert(sec_state_is_valid(type));
	next_image_info = (type == NON_SECURE)
			? &bl33_image_ep_info : &bl32_image_ep_info;
	/*
	 * None of the images on the ARM development platforms can have 0x0
	 * as the entrypoint
	 */
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}

/*******************************************************************************
 * Perform any BL31 specific platform actions. Populate the BL33 and BL32 image
 * info.
 ******************************************************************************/
void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	struct bl31_params *from_bl2 = (struct bl31_params *)arg0;
	uint32_t cpu_ver;

	/*
	 * Configure the UART port to be used as the console
	 */
	lotus_console_boot_init();

	cpu_ver = cpu_get_rev_var();

	INFO("BL31 Early setup\n");

	INFO("CPU Cortex-A55 r%dp%d\n", (cpu_ver >> 4) & 0xf, cpu_ver & 0xf);
	/*
	 * Copy BL3-3, BL3-2 entry point information.
	 */
	bl33_image_ep_info = *from_bl2->bl33_ep_info;
	bl32_image_ep_info = *from_bl2->bl32_ep_info;

	SET_SECURITY_STATE(bl33_image_ep_info.h.attr,
			NON_SECURE);

	SET_SECURITY_STATE(bl32_image_ep_info.h.attr,
			SECURE);

	INFO("--%s %d\n", __func__, __LINE__);
}

/*******************************************************************************
 * Initialize the gic, configure the SCR.
 ******************************************************************************/
void bl31_platform_setup(void)
{
	INFO("BL31 Platform setup\n");
	plat_lotus_pmc_init();
	plat_lotus_gic_driver_init();
	plat_lotus_gic_init();
	plat_delay_timer_init();
	INFO("--%s %d\n", __func__, __LINE__);
}
/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this only intializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bl31_plat_arch_setup(void)
{
	INFO("BL31 Arch setup\n");
	configure_mmu_el3(BL31_START,
			       BL31_END - BL31_START,
			       BL_CODE_BASE,
			       BL_CODE_END);

	INFO("--%s %d\n", __func__, __LINE__);
}

