/*
 * Copyright (c) LOTUS. All rights reserved.
 */

#ifndef USB_VENDOR_H
#define USB_VENDOR_H

enum usb_init_types {
	LOTUS_DWC_U3 = 0,			/* usb3 controller is default */
	LOTUS_DWC_U2 = 1,			/* usb2 controller */
	LOTUS_USB_CTRL_UNKNOWN = 2,	/* unknown controller */
};

#ifdef CONFIG_USB_DUAL_PORT_SUPPORT
void phy_usb_init(int index, int type);
#else
void phy_usb_init(int index);
#endif
void udc_connect(void);
void udc_puts(const char *s);

#endif
