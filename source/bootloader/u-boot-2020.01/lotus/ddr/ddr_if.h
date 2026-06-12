/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _DDR_IF_H
#define _DDR_IF_H

#define DDR_PHY_BYTE_MAX		4
#define DDR_PHY_BIT_NUM			8

/* Max bit 32 */
#define DDR_PHY_BIT_MAX			(DDR_PHY_BYTE_MAX * DDR_PHY_BIT_NUM)
/* Max phy number */
#define DDR_SUPPORT_PHY_MAX		2
/* Max rank number */
#define DDR_SUPPORT_RANK_MAX		2

#define DDR_DATAEYE_RESULT_MASK		0xffff
#define DDR_DATAEYE_RESULT_BIT		16

#define DDR_TRAINING_CMD_DATAEYE	(1 << 5)
#define DDR_CMD_SHOW_GCAI_INFO		(1 << 0)

/* phy number */
#define DDR_PHY_NUM			1

#define DDRC_BASE			0x120D0000
#define DDRC_AXI_BASE			(DDRC_BASE + 0x1000)

#define DDRC_TOP_FEATURE		(DDRC_BASE + 0)
#define DDR3_TYPE			1
#define DDR4_TYPE			2
#define LPDDR4_TYPE			3

#define DDRC_DDR_INFO_REG_OFFSET	0x98
#define DDR_BG_NUM_MASK			(0x1 << 28)

#define LPDDR4_CHANNEL_NUM		0x2
#define LPDDR4_BANK_NUM_OF_RANK		0x8
#define DDR4_BANK_NUM_OF_BG		0x4

#define GCAI_FLAG_REG_OFFSET		0x134
#define GCAI_FLAG_MASK			(0x1 << 4)

#define BAD_BLK_MAP_REG_NUM		0x40
#define CHAN0_BAD_BLK_MAP_REG_OFFSET	0x578
#define CHAN1_BAD_BLK_MAP_REG_OFFSET	0x678
#define CHAN0_BAD_BLK_MAP_REG0	(DDRC_AXI_BASE + CHAN0_BAD_BLK_MAP_REG_OFFSET)
#define CHAN1_BAD_BLK_MAP_REG0	(DDRC_AXI_BASE + CHAN1_BAD_BLK_MAP_REG_OFFSET)

struct training_data {
	unsigned int ddr_bit_result[DDR_PHY_BIT_MAX];
	unsigned int ddr_bit_best[DDR_PHY_BIT_MAX];
	unsigned int ddr_win_sum;
};

struct ddr_training_data {
	unsigned int byte_num;
	struct training_data read;
	struct training_data write;
};

struct rank_data {
	unsigned int rank_idx;
	struct ddr_training_data ddrtr_data;
};

struct phy_data {
	unsigned int rank_num;
	struct rank_data rank[DDR_SUPPORT_RANK_MAX];
};

struct ddr_training_result {
	unsigned int phy_num;
	struct phy_data phy[DDR_SUPPORT_PHY_MAX];
};

struct gcai_base_info {
	u8 unused0;
	u8 type;
	u8 cs_num;
	u8 cs0_cap;
	u8 cs1_cap;
	u8 unused1;
	u8 unused2;
	u8 unused3;
	u8 unused4;
};

void ddr_cmd_result_display(const struct ddr_training_result *ddrtr_result, unsigned int cmd);
int get_result(struct ddr_training_result *result);
void show_win_reg(void);
#endif /* _DDR_IF_H */

