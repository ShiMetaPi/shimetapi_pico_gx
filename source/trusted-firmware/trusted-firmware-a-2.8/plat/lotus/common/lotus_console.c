/*
 * Copyright (c) Lotus. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>
#include <console.h>
#include <debug.h>
#include <pl011.h>
#include <plat_lotus.h>
#include <platform_def.h>

/*******************************************************************************
 * Functions that set up the console
 ******************************************************************************/
static console_t lotus_boot_console;
static console_t lotus_runtime_console;

/* Initialize the console to provide early debug support */
void lotus_console_boot_init(void)
{
	int rc = console_pl011_register(PLAT_LOTUS_UART_BASE,
					PLAT_LOTUS_UART_CLOCK,
					PLAT_LOTUS_UART_BAUDRATE,
					&lotus_boot_console);
	if (rc == 0) {
		/*
		 * The crash console doesn't use the multi console API, it uses
		 * the core console functions directly. It is safe to call panic
		 * and let it print debug information.
		 */
		panic();
	}

	console_set_scope(&lotus_boot_console, CONSOLE_FLAG_BOOT);
}

void lotus_console_boot_end(void)
{
	(void)console_flush();

	(void)console_unregister(&lotus_boot_console);
}

/* Initialize the runtime console */
void lotus_console_runtime_init(void)
{
	int rc = console_pl011_register(PLAT_LOTUS_UART_BASE,
					PLAT_LOTUS_UART_CLOCK,
					PLAT_LOTUS_UART_BAUDRATE,
					&lotus_runtime_console);
	if (rc == 0)
		panic();

	console_set_scope(&lotus_runtime_console, CONSOLE_FLAG_RUNTIME);
}

void lotus_console_runtime_end(void)
{
	(void)console_flush();

	(void)console_unregister(&lotus_runtime_console);
}
