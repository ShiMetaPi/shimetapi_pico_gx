/* SPDX-License-Identifier: GPL-2.0-or-later */


#ifndef _DRIVERS_LOTUS_SDHCI_H
#define _DRIVERS_LOTUS_SDHCI_H

#include <asm/arch/platform.h>

#define EMMC_LAYER_MAX        2
#define EMMC_SPEED_MAX        5

#define SDIO_LAYER_MAX        2
#define SDIO_SPEED_MAX        5

#define BSP_CFG_CRG_REG (SYS_CTRL_REG_BASE + 0x134)

struct sdhci_pad_cell {
	unsigned char clk_pad;
	unsigned char dq_cmd_pad;
};

#endif /* _DRIVERS_LOTUS_SDHCI_H */
