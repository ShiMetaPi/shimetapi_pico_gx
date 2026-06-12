// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <command.h>
#include <linux/io.h>
#include "ddr_if.h"

#define REG_DQ_W_BDL_BASE	0x12104110
#define REG_DQ_R_BDL_BASE	0x12104130
#define REG_DQ_RES_SW		0x12101020

int get_result(struct ddr_training_result *result)
{
	int i, j;
	unsigned int *result_w, *result_r, *best_w, *best_r;

	memset(result, 0, sizeof(struct ddr_training_result));

	result->phy_num = DDR_PHY_NUM;

	result->phy[0].rank_num = 1;

	result->phy[0].rank[0].rank_idx = 0;

	result->phy[0].rank[0].ddrtr_data.byte_num = 4;

	result_w = result->phy[0].rank[0].ddrtr_data.write.ddr_bit_result;
	result_r = result->phy[0].rank[0].ddrtr_data.read.ddr_bit_result;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			unsigned int v = readl(REG_DQ_W_BDL_BASE + 0x1000 * i + j * 4);

			result_w[i * 8 + j * 2] |= v & 0xff;
			result_w[i * 8 + j * 2 + 1] |= (v >> 16) & 0xff;

		}
	}

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			unsigned int v = readl(REG_DQ_R_BDL_BASE + 0x1000 * i + j * 4);

			result_r[i * 8 + j * 2] |= v & 0xff;
			result_r[i * 8 + j * 2 + 1] |= (v >> 16) & 0xff;

		}
	}

	/* Switch to getting min bdl */
	writel(1, REG_DQ_RES_SW);

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			unsigned int v = readl(REG_DQ_W_BDL_BASE + 0x1000 * i + j * 4);

			result_w[i * 8 + j * 2] |= (v & 0xff) << DDR_DATAEYE_RESULT_BIT;
			result_w[i * 8 + j * 2 + 1] |= (v >> 16 & 0xff) << DDR_DATAEYE_RESULT_BIT;
		}
	}

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			unsigned int v = readl(REG_DQ_R_BDL_BASE + 0x1000 * i + j * 4);

			result_r[i * 8 + j * 2] |= (v & 0xff) << DDR_DATAEYE_RESULT_BIT;
			result_r[i * 8 + j * 2 + 1] |= (v >> 16 & 0xff) << DDR_DATAEYE_RESULT_BIT;
		}
	}

	/* Restore function mode */
	writel(0, REG_DQ_RES_SW);

	best_w = result->phy[0].rank[0].ddrtr_data.write.ddr_bit_best;
	best_r = result->phy[0].rank[0].ddrtr_data.read.ddr_bit_best;
	for (i = 0; i < DDR_PHY_BIT_MAX; ++i) {
		unsigned int res;
		unsigned short max, min;

		res = result_w[i];
		min = res >> DDR_DATAEYE_RESULT_BIT;
		max = (res & DDR_DATAEYE_RESULT_MASK) *  2 - min;
		result_w[i] = (min << DDR_DATAEYE_RESULT_BIT) | max;
		best_w[i] = (res & DDR_DATAEYE_RESULT_MASK) | ((max - min) << DDR_DATAEYE_RESULT_BIT);

		res = result_r[i];
		min = res >> DDR_DATAEYE_RESULT_BIT;
		max = (res & DDR_DATAEYE_RESULT_MASK) *  2 - min;
		result_r[i] = (min << DDR_DATAEYE_RESULT_BIT) | max;
		best_r[i] = (res & DDR_DATAEYE_RESULT_MASK) | ((max - min) << DDR_DATAEYE_RESULT_BIT);

	}

	return 0;
}

void show_win_reg(void)
{
	int i;

	printf("DDR DQ Win REGs\n");
	printf("------------------------------------------------------\n");
	printf("Avg BDL of Writing:\n");
	writel(0x0, REG_DQ_RES_SW);
	for (i = 0; i < 4; ++i) {
		unsigned long addr = REG_DQ_W_BDL_BASE + 0x1000 * i;

		print_buffer(addr, (void *)addr, 4, 4, 4);
	}

	printf("Min BDL of Writing:\n");
	writel(0x1, REG_DQ_RES_SW);
	for (i = 0; i < 4; ++i) {
		unsigned long addr = REG_DQ_W_BDL_BASE + 0x1000 * i;

		print_buffer(addr, (void *)addr, 4, 4, 4);
	}

	printf("Avg BDL of Reading:\n");
	writel(0x0, REG_DQ_RES_SW);
	for (i = 0; i < 4; ++i) {
		unsigned long addr = REG_DQ_R_BDL_BASE + 0x1000 * i;

		print_buffer(addr, (void *)addr, 4, 4, 4);
	}

	printf("Min BDL of Reading:\n");
	writel(0x1, REG_DQ_RES_SW);
	for (i = 0; i < 4; ++i) {
		unsigned long addr = REG_DQ_R_BDL_BASE + 0x1000 * i;

		print_buffer(addr, (void *)addr, 4, 4, 4);
	}

	writel(0x0, REG_DQ_RES_SW);
}

