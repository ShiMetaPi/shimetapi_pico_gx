// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include "ddr_if.h"

#ifndef PHY_DQ_BDL_LEVEL
#define PHY_DQ_BDL_LEVEL	256
#endif

static void print_dataeye_win(unsigned int dq_num, unsigned int range,
	unsigned int dq, unsigned int win)
{
	unsigned int k;

	printf("%-4u", dq_num);
	for (k = 0; k < PHY_DQ_BDL_LEVEL; k += 2) {
		if (k >= (range >> DDR_DATAEYE_RESULT_BIT) &&
			k <= (range & DDR_DATAEYE_RESULT_MASK))
			printf("%-1s", "-");
		else
			printf("%-1s", "X");
	}
	printf(" %-4u  %-4u  %-4u %-4u\n", range >> 16, range & 0xffff, dq, win);
}

static void print_dataeye_title(void)
{
	unsigned int k;

	printf("%-4s", "DQ");
	for (k = 0; k < PHY_DQ_BDL_LEVEL; k++) {
		if (k % 8 == 0) /* Print out the CA number which is a multiple of 4 */
			printf("%-4u", k);
	}
	printf(" %-4s  %-4s  %-4s %-4s\n", "MIN", "MAX", "AVG", "WIN");
}

static void ddr_cmd_result_print_write_dataeye(const struct ddr_training_data *ddrtr_data,
	unsigned int win_min, unsigned int win_max, unsigned int win_sum)
{
	unsigned int i, j;
	unsigned int dq_num, dq, win;

	printf("Write window:\n");
	printf("--------------------------------------------------------\n");
	print_dataeye_title();
	if (ddrtr_data->byte_num > DDR_PHY_BYTE_MAX) {
		printf("byte num error, byte_num = %x", ddrtr_data->byte_num);
		return;
	}
	for (j = 0; j < ddrtr_data->byte_num; j++) {
		for (i = 0; i < DDR_PHY_BIT_NUM; i++) {
			dq_num = (j << 3) + i; /* shift left 3: 8 bit */
			if (dq_num >= DDR_PHY_BIT_MAX)
				return;

			win = ddrtr_data->write.ddr_bit_best[dq_num] >> DDR_DATAEYE_RESULT_BIT;
			if (win < win_min)
				win_min = win;
			if (win > win_max)
				win_max = win;
			win_sum += win;
			dq =  ddrtr_data->write.ddr_bit_best[dq_num] & DDR_DATAEYE_RESULT_MASK;
			print_dataeye_win(dq_num,
				ddrtr_data->write.ddr_bit_result[dq_num], dq, win);
		}
	}
	printf("--------------------------------------------------------\n");
	printf("Sum WIN: %u. Avg WIN: %u\n", win_sum,
		win_sum / (ddrtr_data->byte_num * DDR_PHY_BIT_NUM));
	printf("Min WIN: %u. DQ Index: ", win_min);
	for (i = 0; i < DDR_PHY_BIT_MAX; i++) {
		win = ddrtr_data->write.ddr_bit_best[i] >> DDR_DATAEYE_RESULT_BIT;
		if (win == win_min)
			printf("%u ", i);
	}
	printf("\nMax WIN: %u. DQ Index: ", win_max);
	for (i = 0; i < DDR_PHY_BIT_MAX; i++) {
		win = ddrtr_data->write.ddr_bit_best[i] >> DDR_DATAEYE_RESULT_BIT;
		if (win == win_max)
			printf("%u ", i);
	}
	printf("\n\n");
}

static void ddr_cmd_result_print_read_dataeye(const struct ddr_training_data *ddrtr_data,
	unsigned int win_min, unsigned int win_max, unsigned int win_sum)
{
	unsigned int i, j;
	unsigned int dq_num, dq, win;

	if (ddrtr_data->byte_num > DDR_PHY_BYTE_MAX) {
		printf("Invalid  byte_num = %d", ddrtr_data->byte_num);
		return;
	}
	printf("Read window:\n");
	printf("--------------------------------------------------------\n");
	print_dataeye_title();
	for (j = 0; j < ddrtr_data->byte_num; j++) {
		for (i = 0; i < DDR_PHY_BIT_NUM; i++) {
			dq_num = (j << 3) + i; /* shift left 3: 8 bit */
			if (dq_num >= DDR_PHY_BIT_MAX)
				return;

			win = ddrtr_data->read.ddr_bit_best[dq_num] >> DDR_DATAEYE_RESULT_BIT;
			if (win < win_min)
				win_min = win;
			if (win > win_max)
				win_max = win;
			win_sum += win;
			dq = ddrtr_data->read.ddr_bit_best[dq_num] & DDR_DATAEYE_RESULT_MASK;
			print_dataeye_win(dq_num,
				ddrtr_data->read.ddr_bit_result[dq_num], dq, win);
		}
	}
	printf("--------------------------------------------------------\n");
	printf("Sum WIN: %u. Avg WIN: %u\n", win_sum,
		win_sum / (ddrtr_data->byte_num * DDR_PHY_BIT_NUM));
	printf("Min WIN: %u. DQ Index: ", win_min);
	for (i = 0; i < DDR_PHY_BIT_MAX; i++) {
		win = ddrtr_data->read.ddr_bit_best[i] >> DDR_DATAEYE_RESULT_BIT;
		if (win == win_min)
			printf("%u ", i);
	}
	printf("\nMax WIN: %u. DQ Index: ", win_max);
	for (i = 0; i < DDR_PHY_BIT_MAX; i++) {
		win = ddrtr_data->read.ddr_bit_best[i] >> DDR_DATAEYE_RESULT_BIT;
		if (win == win_max)
			printf("%u ", i);
	}
	printf("\n\n");
}

static void ddr_cmd_result_print_dataeye(const struct ddr_training_data *ddrtr_data)
{
	/* Write window */
	ddr_cmd_result_print_write_dataeye(ddrtr_data, PHY_DQ_BDL_LEVEL, 0, 0);

	/* Read window */
	ddr_cmd_result_print_read_dataeye(ddrtr_data, PHY_DQ_BDL_LEVEL, 0, 0);
}


static void ddr_cmd_result_print_by_rank(const struct ddr_training_result *ddrtr_result,
	unsigned int cmd, unsigned int phy_index, unsigned int rank_index)
{
	const struct rank_data *rank = &ddrtr_result->phy[phy_index].rank[rank_index];

	printf("\r\n[PHY%u][RANK%u]:\r\n", phy_index, rank_index);
	if (DDR_TRAINING_CMD_DATAEYE & cmd)
		ddr_cmd_result_print_dataeye(&rank->ddrtr_data);

}

static void ddr_cmd_result_print_by_phy(const struct ddr_training_result *ddrtr_result,
	unsigned int cmd, unsigned int phy_index)
{
	int i;

	if (phy_index >= DDR_PHY_NUM) {
		printf("Array index phy_idx out of range");
		return;
	}
	if (ddrtr_result->phy[phy_index].rank_num > DDR_SUPPORT_RANK_MAX) {
		printf("loop upper limit rank number out of range, rank_num = %x",
			ddrtr_result->phy[phy_index].rank_num);
		return;
	}

	for (i = 0; i < ddrtr_result->phy[phy_index].rank_num; i++)
		ddr_cmd_result_print_by_rank(ddrtr_result, cmd, phy_index, i);
}

void ddr_cmd_result_display(const struct ddr_training_result *ddrtr_result, unsigned int cmd)
{
	int i;

	if (ddrtr_result == NULL) {
		printf("Pointer parameter ddrtr_result is NULL!");
		return;
	}
	if (ddrtr_result->phy_num > DDR_PHY_NUM)
		return;

	for (i = 0; i < ddrtr_result->phy_num; i++)
		ddr_cmd_result_print_by_phy(ddrtr_result, cmd, i);

	show_win_reg();
}


