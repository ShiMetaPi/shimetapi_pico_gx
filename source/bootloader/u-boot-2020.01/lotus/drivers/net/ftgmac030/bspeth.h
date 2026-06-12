/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __BSPETH_H__
#define __BSPETH_H__

#include <config.h>
#include <common.h>
#include <command.h>

#include <linux/list.h>
#include <linux/io.h>

#include <malloc.h> /* malloc, free, realloc */

#include <net.h>
#include <miiphy.h>
#include <asm/arch/platform.h>

#ifndef bit
#define bit(nr) (1UL << (nr))
#endif
#define mdelay(n) udelay((n)*1000)

/* ***********************************************************
 *
 * Global varibles
 *
 * ***********************************************************
 */
#define BSPETH_PHY_CRG_REG (CRG_REG_BASE + 0x16c)
#define BSPETH_MAC_CRG_REG (CRG_REG_BASE + 0x170)
#define BSPETH_CFG_CRG_REG (SYS_CTRL_REG_BASE + 0x134)
#define BSPETH_DLY_CRG_REG (SYS_CTRL_REG_BASE + 0x8940)
#define BSPETH_DLY_STAT_REG (CRG_REG_BASE + 0x1a8)

#define ETH_DLY_SEL bit(0)
#define ETH_DLY_STAT bit(0)

#define ETH_SOFT_RESET bit(1)

#define ETH_EXTERNAL_PHY_RESET bit(1)

#define ETH_PHY_INTF (bit(0) | bit(3))
#define ETH_PHY_RST_N bit(1)

/* configuerable values */

/* ***********************************************************
 *
 * Only for internal used!
 *
 * ***********************************************************
 */

/* read/write IO */

#define _readl(c)     ({ u32 __v = le32_to_cpu(__raw_readl(c)); __v; })
#define _writel(v, c) __raw_writel(cpu_to_le32(v), c)

#define mk_bits(shift, nbits) ((((shift) & 0x1F) << 16) | ((nbits) & 0x1F))

#endif
