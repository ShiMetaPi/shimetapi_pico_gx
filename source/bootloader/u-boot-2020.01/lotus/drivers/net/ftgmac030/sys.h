/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __BSPETH_SYS_H__
#define __BSPETH_SYS_H__

void bspeth_sys_init(void);
void bspeth_sys_exit(void);

//void bspeth_sys_startup(void);
//void bspeth_sys_allstop(void);

u32 bspeth_get_phy_intf(void);
void bspeth_set_tx_delay(void);

#endif
