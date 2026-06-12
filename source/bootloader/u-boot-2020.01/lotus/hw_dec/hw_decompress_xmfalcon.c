/*
 * Copyright (c) LOTUS. All rights reserved.
 */

#include <asm/io.h>
#include <config.h>

#define PERI_CRG100 (CRG_REG_BASE + 0x190)

#define GZIP_REG_CRG PERI_CRG100
#define GZIP_CLKEN (0x1<<0)

static void disable_decompress_clock(void)
{
	unsigned int regval;

	regval = readl(GZIP_REG_CRG);
	regval &= ~GZIP_CLKEN;
	writel(regval, GZIP_REG_CRG);
}

static void enable_decompress_clock(void)
{
	unsigned int regval;

	regval = readl(GZIP_REG_CRG);
	regval |= GZIP_CLKEN;
	writel(regval, GZIP_REG_CRG);
}
