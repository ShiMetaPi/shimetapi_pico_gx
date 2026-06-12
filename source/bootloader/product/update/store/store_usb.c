// SPDX-License-Identifier: GPL-2.0
/* Copyright (c) LOTUS. All rights reserved. */

#include <common.h>

#if (CONFIG_AUTO_USB_UPDATE == 1)
#include <usb.h>
#include "update_comm.h"

int store_usb_init(void)
{
	int ret;
#ifndef CONFIG_XMEDIA_MC
try_again:
	if (usb_stop() < 0) {
		debug("usb_stop failed\n");
		return UP_FAILURE;
	}
	/* delay for 1000 ms */
	mdelay(1000);
	ret = usb_init();
	if (ret == -ESRCH) {
		goto try_again;
	}

	if (ret < 0) {
		debug("usb_init failed!\n");
		return UP_FAILURE;
	}

	/*
	 * check whether a storage device is attached (assume that it's
	 * a USB memory stick, since nothing else should be attached).
	 */
	ret = usb_stor_scan(0);
	if (ret == -1) {
		debug("No USB device found. Not initialized!\n");
		return UP_FAILURE;
	} else {
		return UP_SUCCESS;
	}
#endif
	return UP_FAILURE;
}

void store_usb_deinit(void)
{
#ifndef CONFIG_XMEDIA_MC
	usb_stop();
#endif
}

#endif // CONFIG_AUTO_USB_UPDATE
