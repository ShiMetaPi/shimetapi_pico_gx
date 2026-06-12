/*
 * Copyright (c) Lotus. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <console.h>
#include <plat_lotus.h>
#include <platform.h>

/*******************************************************************************
 * Perform any BL31 platform runtime setup prior to BL31 exit common to ARM
 * standard platforms
 * Perform BL31 platform setup
 ******************************************************************************/
void lotus_bl31_plat_runtime_setup(void)
{
	console_switch_state(CONSOLE_FLAG_RUNTIME);

	/* Initialize the runtime console */
	lotus_console_runtime_init();
}

void bl31_plat_runtime_setup(void)
{
	lotus_bl31_plat_runtime_setup();
}
