// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <linux/io.h>
#include "ddr_if.h"
#include "linux/lotus/i2c.h"
#include "linux/lotus/securec.h"

#define DDR_CMD_DATAEYE_STR         "dataeye"
#define DDR_CMD_GCAI_INFO         "gcai-info"
static struct ddr_training_result *g_ddrtr_result;

static int ddr_map_show(void);

static int cmd_ddr_match(const char *str, int *cmd)
{
	if (!strncmp(str, DDR_CMD_DATAEYE_STR, sizeof(DDR_CMD_DATAEYE_STR))) {
		*cmd = DDR_TRAINING_CMD_DATAEYE;
	} else if (!strncmp(str, DDR_CMD_GCAI_INFO,
		sizeof(DDR_CMD_GCAI_INFO))) {
		*cmd = DDR_CMD_SHOW_GCAI_INFO;
	} else {
		printf("Command [ddr %s] is not supported\n", str);
		return -1;
	}

	return 0;
}

static int cmd_ddr_handle(int cmd)
{
	if (cmd == DDR_TRAINING_CMD_DATAEYE)
		return	get_result(g_ddrtr_result);

	return 0;
}

static int cmd_ddr_dispatch(int cmd)
{
	int result;

	result = cmd_ddr_handle(cmd);
	if (result < 0)
		return -1;

	switch (cmd) {
	case DDR_TRAINING_CMD_DATAEYE:
		ddr_cmd_result_display(g_ddrtr_result, DDR_TRAINING_CMD_DATAEYE);
		break;
	case DDR_CMD_SHOW_GCAI_INFO:
		ddr_map_show();
		break;
	default:
		break;
	}

	return result;
}

static int do_ddr_training(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *str = NULL;
	int cmd;
	int ret = 0;

	if (argc != 2)
		return -1;

	str = argv[1];

	if (cmd_ddr_match(str, &cmd)) {
		ret = -1;
		goto out;
	}

	g_ddrtr_result = malloc(sizeof(struct ddr_training_result));
	if (g_ddrtr_result == NULL) {
		printf("Fail to alloc DDR Training result memory!\n");
		ret = -1;
		goto out;
	}

	if (cmd_ddr_dispatch(cmd)) {
		ret = -1;
		goto out;
	}

out:
	if (g_ddrtr_result)
		free(g_ddrtr_result);

	return ret;
}

static int get_ddr4_bank_group_cnt(void)
{
	u32 reg;

	reg = readl(DDRC_AXI_BASE + DDRC_DDR_INFO_REG_OFFSET);
	reg &= DDR_BG_NUM_MASK;

	return reg ? 4 : 2; /* 1:4-bg, 0: 2-bg */
}

static int get_ddr_type(void)
{
	u32 type = DDR3_TYPE;
	u32 v = readl(DDRC_TOP_FEATURE) & 0x7;

	switch (v) {
	case 1:
		type = DDR3_TYPE;
		break;
	case 2:
		type = DDR4_TYPE;
		break;
	case 3:
		type = LPDDR4_TYPE;
		break;
	default:
		type = v;
		break;
	}

	return type;
}

static int get_ddr_bank_cnt(int rank_cnt)
{
	int type, bank_cnt;

	type = get_ddr_type();

	switch (type) {
	case LPDDR4_TYPE:
		/* GCAI DDR support 2-chans */
		bank_cnt = LPDDR4_CHANNEL_NUM * rank_cnt *
			LPDDR4_BANK_NUM_OF_RANK;
		break;
	case DDR4_TYPE:
		/* Get DDR4 bank group cnt */
		int bg_cnt;

		bg_cnt = get_ddr4_bank_group_cnt();
		bank_cnt = DDR4_BANK_NUM_OF_BG * bg_cnt;
		break;
	case DDR3_TYPE:
		/* DDR3 support single channel multi ranks */
		bank_cnt = rank_cnt * LPDDR4_BANK_NUM_OF_RANK;
	default:
		bank_cnt = -1;
		break;
	}

	return bank_cnt;
}

static int show_ddr_info(char *buf, unsigned int *rank_cnt,
	unsigned int *bank_cnt, unsigned int *block_cnt)
{
	struct gcai_base_info *gcaiinfo;
	int ret;

	gcaiinfo = (struct gcai_base_info *)buf;
	*rank_cnt = (unsigned int)gcaiinfo->cs_num;

	*bank_cnt = get_ddr_bank_cnt(*rank_cnt);
	if (*bank_cnt <= 0) {
		printf("%s-%d: get_ddr_bank_cnt err\n", __func__, __LINE__);
		return -1;
	}

	*block_cnt = (*bank_cnt) * 8;

	ret = get_ddr_type();
	if (ret == LPDDR4_TYPE)
		printf("GCAI LPDDR4: channel=2, ");
	else if (ret == DDR4_TYPE)
		printf("GCAI DDR4: channel=1, ");
	else {
		printf("not support GCAI DDR type!\n");
		return -1;
	}

	printf("rank=%u, bank=%u, block=%u\n", *rank_cnt, *bank_cnt,
		*block_cnt);

	return 0;
}

static int show_gcai_bad_blk_info(char *bad_blk_info, int expect_len)
{
#ifdef CONFIG_LOTUS_I2C
	int ret = 0, i = 0, actual_len = 64;
	struct i2c_client client;
	char *read_ptr = bad_blk_info;

	hal_i2c_init(0);	/* i2c0 init */
	client.i2c_num = 0;	/* read by i2c0 */
	client.dev_addr = 0x50; /* dev addr without RD flag */
	client.reg_width = 1;	/* per reg addr width is 1-Byte */

	printf("=======begin to show GCAI DDR info=======\n");
	for (i = 0; i < expect_len; i++) {
		/* read reg addr start from [0x0, 0x63] */
		client.reg_addr = i;
		/* per reg read return 1-byte data */
		ret = hal_i2c_recv(&client, (void *)read_ptr, 1);
		if (ret) {
			printf("%s: i2c recv err, ret=%d\n", __func__, ret);
			break;
		}
		read_ptr++;
	}

	for (i = 0; i < actual_len; i++)
		printf("bad_blk_info[%d]: 0x%x\n", i, bad_blk_info[i]);

	return ret;
#else
	printf("%s: i2c is not enabled.\n", __func__);
	return 0;
#endif
}

static void show_bad_blk_config_reg(void)
{
	int i;
	u32 reg, offset;

	for (i = 0; i < BAD_BLK_MAP_REG_NUM; i++) {
		offset = i * 4;
		reg = readl(CHAN0_BAD_BLK_MAP_REG0 + offset);
		printf("ch0: reg%d(0x%x): 0x%x\n", i, CHAN0_BAD_BLK_MAP_REG0 + offset,
			reg);
	}

	for (i = 0; i < BAD_BLK_MAP_REG_NUM; i++) {
		offset = i * 4;
		reg = readl(CHAN1_BAD_BLK_MAP_REG0 + offset);
		printf("ch1: reg%d(0x%x): 0x%x\n", i, CHAN1_BAD_BLK_MAP_REG0 + offset,
			reg);
	}
	printf("=======end of show GCAI DDR info=======\n");
}

/*
 * is_gcai_ddr - Identify whether use GCAI DDR.
 * 1:GCAI DDR, 0:non-GCAI DDR.
 */
static int is_gcai_ddr(void)
{
	u32 reg;

	reg = readl(REG_BASE_SCTL + GCAI_FLAG_REG_OFFSET);

	return (reg & GCAI_FLAG_MASK) ? 1 : 0;
}

/*
 * ddr_map_show - GCAI DDR bad blk remap info.
 */
static int ddr_map_show(void)
{
	char *bad_blk_info = NULL;
	int ret, read_len = 256;
	unsigned int rank_cnt, bank_cnt, blk_cnt;

	if (!is_gcai_ddr()) {
		printf("%s: not GCAI DDR, show nothing.\n", __func__);
		ret = 0;
		goto out;
	}

	bad_blk_info = malloc(read_len * sizeof(char));
	if (bad_blk_info == NULL) {
		printf("%s: out of memory!\n", __func__);
		ret = -1;
		goto out;
	}

	memset_s(bad_blk_info, read_len, 0xEE, read_len);

	/* show bad blk info that read from i2c */
	ret = show_gcai_bad_blk_info(bad_blk_info, read_len);
	if (ret)
		goto free_mem;

	/* show ddr detail info */
	ret = show_ddr_info(bad_blk_info, &rank_cnt, &bank_cnt, &blk_cnt);
	if (ret)
		goto free_mem;

	/* show all bad blk config reg */
	show_bad_blk_config_reg();

free_mem:
	free(bad_blk_info);
out:
	return ret;
}

U_BOOT_CMD(
	ddr, CONFIG_SYS_MAXARGS, 0,	do_ddr_training,
	"ddr training cmd",
	"dataeye     - DDR dataeye training.\n"
	"gcai-info     - show GCAI DDR bad block remap info.\n"
);

