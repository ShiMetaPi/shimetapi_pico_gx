// SPDX-License-Identifier: GPL-2.0
/* Copyright (c) LOTUS. All rights reserved. */

#include <common.h>
#include <command.h>

#include "update.h"

static int do_xmediaapp(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
#if (CONFIG_AUTO_UPDATE == 1)
	auto_update();
#endif

	return 0;
}

U_BOOT_CMD(xmediaapp, 1, 0, do_xmediaapp,
		"xmediaapp    - Xmedia Application.\n",
		"Run auto update and others business.\n");
