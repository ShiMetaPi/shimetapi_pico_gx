// SPDX-License-Identifier: GPL-2.0
/* Copyright (c) LOTUS. All rights reserved. */

#ifndef __STORE_MMC_H__
#define __STORE_MMC_H__

int store_mmc_init(void);

int store_mmc_deinit(void);

int mmc_get_target_dev(void);
#endif
