
#define pr_fmt(fmt) "%s: " fmt, "LOAD MCU S2"

#include <common.h>
#include <malloc.h>
#include <linux/io.h>
#include <linux/lotus/flash_read.h>
#include <linux/lotus/image.h>

static int do_load_mcu_stage2(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret = -1;
	u32 res, aligned_offset, aligned_size;
	u32 total_size;
	u32 stage1_size;
	u32 mcu_offset = CONFIG_MCU_FW_AREA_POS;
	struct mcu_fw_head *fw_head = (struct mcu_fw_head *)MCU_SRAM_BASE;
	u8 *buf = (u8 *)CONFIG_MCU_FW_BUF_ADDR;
	u32 v;

	if (CONFIG_LOAD_MCU_FW_FLAG == 0 || CONFIG_MCU_FW_AREA_LEN == 0)
		goto error;

	if (fw_head->fw_magic != _MCU_FW_MAGIC) {
		pr_info("No MCU FW\n");
		goto error;
	}

	total_size = fw_head->fw_total_size;
	stage1_size = fw_head->fw_stage1_size;
	if (total_size < stage1_size
		|| (((total_size + 1024 - 1) >> 10) & (~((1 << 10) - 1))) != 0) {
		pr_err("Invalid MCU FW size!Total: %u, Stage1: %u\n", total_size, stage1_size);
		goto error;
	}

	res = mcu_offset & (FLASH_ALIGNED_SIZE - 1);
	aligned_offset = rounddown(mcu_offset, FLASH_ALIGNED_SIZE);
	aligned_size = roundup(total_size + res, FLASH_ALIGNED_SIZE);
	if (aligned_size > CONFIG_MCU_FW_BUF_SIZE) {
		pr_err("No buffer for MCU FW!\n");
		goto error;
	}

	ret = flash_read(aligned_offset, aligned_size, buf);
	if (ret != 0) {
		pr_err("Fail to read MCU area!\n");
		goto error;
	}

	memcpy((void *)MCU_SRAM_BASE + stage1_size, buf + res + stage1_size, total_size - stage1_size);
	memmove(buf, buf + res, total_size);

	printf("MCU FW location in DDR: 0x%x, 0x%x Bytes\n", (u32)buf, total_size);

	v = readl(REG_MCU_FW_DDR_SIZE);
	v &= ~((1 << 10) - 1);
	v |= (total_size + 1024 - 1) >> 10;
	writel(v, REG_MCU_FW_DDR_SIZE);

	dsb();
	isb();

	/* Release MCU to stage2 */
	writel(0xbeef0001, SYS_CTRL_REG_BASE + REG_SC_SYSBOOT7);

	dsb();
	isb();

	return 1;
error:
	return CMD_RET_FAILURE;
}

U_BOOT_CMD(
	loadmcustage2,  1,  0,  do_load_mcu_stage2,
	"Load MCU Stage2",
	"loadmcustage2\n"
	""
);
