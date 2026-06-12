/*
 * Copyright (c) LOTUS. All rights reserved.
 */

#ifndef PHY_LOTUS_USB
#define PHY_LOTUS_USB

#define U_LEVEL1	10
#define U_LEVEL2	20
#define U_LEVEL3	30
#define U_LEVEL4	50
#define U_LEVEL5	100
#define U_LEVEL6	200
#define U_LEVEL7	300
#define U_LEVEL8	500
#define U_LEVEL9	1000
#define U_LEVEL10	2000

enum usb_phy_width_mode {
	PHY_WIDTH_8BIT = 0,
	PHY_WIDTH_16BIT,
};

enum usb_scenes {
	/* U3 CTRL connect U3/U2 PHY */
	LOTUS_USB_SCENE2 = 0,
	/* U3 CTRL connect U3 PHY, U2 CTRL connect U2 PHY */
	LOTUS_USB_SCENE1 = 1,
};

#endif // PHY_LOTUS_USB
