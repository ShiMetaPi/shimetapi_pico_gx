
#define pr_fmt(fmt) "%s: " fmt, "HW GUNZIP"

#include <common.h>
#include <memalign.h>
#include <linux/lotus/hw_decompress.h>
#include <mmc.h>
#include <linux/lotus/flash_read.h>
#include <asm/cache.h>
#include <usb.h>
#include <linux/lotus/timestamp.h>
#include <irq_func.h>
#include <dm/device.h>
#include <serial.h>
#include <linux/lotus/tags.h>
#include <linux/ctype.h>

#define HW_GUNZIP_STEP_MIN	0x20000
#define DEFAULT_STEP		(256 * 1024)

static bool decomp_done = false;

int decompress_by_step(void *dst, uint dst_len, void *src, uint *z_len,
		ulong offset, uint step)
{
	uint cur_read_size = step;
	uchar *buffer = src;
	hw_decompress_op_type op_type;
	uint cur_offset;
	int ret = -1;
	int head_size = sizeof(image_header_t) + HW_GZIP_HEAD_SIZE;
	uint size;

	if (!dst || !buffer)
		return -EINVAL;

	if (!step)
		step = DEFAULT_STEP;

	cur_read_size = step;

	ret = flash_read(offset, cur_read_size, buffer);
	if (ret < 0) {
		pr_err("Fail to read from flash\n");
		goto out;
	}

	size = image_get_size((image_header_t *)buffer) + sizeof(image_header_t);
	if (size < step) {
		pr_err("Invalid image size, %u(size) less than %u(step)\n", size, step);
		return -EINVAL;
	}

	dcache_disable();
	hw_dec_init();

	ret = hw_dec_decompress_ex(HW_DECOMPRESS_OP_START,
		dst, &dst_len, buffer + head_size, cur_read_size - head_size);
	if (ret != 0) {
		pr_err("hw decompress start failed(%d)\n", ret);
		goto out;
	}

	cur_offset = cur_read_size;
	while (cur_offset < size) {
		if ((cur_offset + step) < size) {
			cur_read_size = step;
			op_type = HW_DECOMPRESS_OP_CONTINUE;
		} else {
			cur_read_size = size - cur_offset;
			op_type = HW_DECOMPRESS_OP_END;
		}

		ret = flash_read(offset + cur_offset, cur_read_size, buffer + cur_offset);
		if (ret < 0) {
			pr_err("Read flash failed(%d).\n", ret);
			goto out;
		}

		pr_debug("Decompress 0x%p to 0x%p, size 0x%x\n", buffer + cur_offset, dst, cur_read_size);

		ret = hw_dec_decompress_ex(op_type,
			dst, &dst_len, buffer + cur_offset, cur_read_size);
		if (ret != 0) {
			pr_err("hw decompress failed.(%d)\n", ret);
			goto out;
		}

		cur_offset += cur_read_size;
	}

	if (z_len)
		*z_len = size;
	ret = 0;
out:
	hw_dec_uinit();
	dcache_enable();

	return ret;
}

int hw_gunzip(void *dst, uint dst_len, void *src, uint *src_len)
{
	int ret = 0;

	if (decomp_done)
		return 0;

	dcache_disable();

	hw_dec_init();

	ret = hw_dec_decompress_ex(HW_DECOMPRESS_OP_ONCE, (uchar *)dst,
		&dst_len, (uchar *)src, *src_len);
	if (ret != 0) {
		pr_err("hw decompress fail!\n");
		goto out;
	}

out:
	hw_dec_uinit();

	dcache_enable();

	return ret;
}

static int do_hw_gunzip(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	int ret;

	if (argc != 3)
		return CMD_RET_USAGE;

	uintptr_t src = simple_strtoul(argv[1], NULL, 16); /* 1: src argc, 16: base */
	uintptr_t dst = simple_strtoul(argv[2], NULL, 16); /* 2: dst argc, 16: base */

	if (src & 0xf || dst & 0xf) {
		pr_err("src[0x%lx] or dst[0x%lx] NOT 16Byte-aligned!\n", src, dst);
		return CMD_RET_USAGE;
	}

	uint magic_num0 = *(uint *)(src + HEAD_MAGIC_NUM0_OFFSET);
	uint magic_num1 = *(uint *)(src + HEAD_MAGIC_NUM1_OFFSET);
	if ((magic_num0 != HEAD_MAGIC_NUM0) || (magic_num1 != HEAD_MAGIC_NUM1)) {
		pr_err("Invalid image!\n");
		return CMD_RET_FAILURE;
	}
	uint comp_len = *(uint *)(src + COMPRESSED_SIZE_OFFSET);
	uint uncomp_len = *(uint *)(src + UNCOMPRESSED_SIZE_OFFSET);

	ret = hw_gunzip((void *)dst, uncomp_len, (uchar *)(src + HW_GZIP_HEAD_SIZE), &comp_len);
	if (ret != 0) {
		pr_err("hw gunzip fail!(%d)\n", ret);
		return CMD_RET_FAILURE;
	}

	return 1;
}


static int read_dtb(ulong offset, uint image_size,
	ulong src_addr, uint src_size)
{
	int ret;

	if (src_size & (FLASH_ALIGNED_SIZE - 1))
		src_size = src_size & ~(FLASH_ALIGNED_SIZE - 1);

	ret = flash_read(offset + src_size, image_size - src_size,
		(uchar *)(src_addr + src_size));
	if (ret < 0) {
		pr_err("read DTB file failed(0x%x).\n", ret);
		return -1;
	}

	return 0;
}

static int do_boothz(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	uchar *src_addr = NULL;
	uchar *dst_addr = NULL;
	ulong offset;
	uint image_size;
	uint step;
	int ret;
	char *sub_cmd = NULL;
	char bootm_str[30];
	uint z_len = 0;

	switch (argc) {
	case 5:
		step = DEFAULT_STEP;
		break;
	case 6:
		if (isdigit(*(argv[5]))) {
			step = \
			       (uint)(uintptr_t)simple_strtoul(argv[5], NULL, 16);
		} else {
			step = DEFAULT_STEP;
			sub_cmd = argv[5];
		}
		break;
	case 7:
		if (isdigit(*(argv[5])) || !isdigit(*(argv[6]))) {
			return CMD_RET_USAGE;
		}
		sub_cmd = argv[5];
		step = \
		       (uint)(uintptr_t)simple_strtoul(argv[6], NULL, 16);
	default:
		return CMD_RET_USAGE;
	}

	if (step < HW_GUNZIP_STEP_MIN) {
		pr_err("step less than 0x%x!\n", HW_GUNZIP_STEP_MIN);
		ret = CMD_RET_USAGE;
		goto out;
	}

	if ((step % HW_GUNZIP_STEP_MIN) != 0) {
		pr_err("step should be aligned with 0x%x!\n", HW_GUNZIP_STEP_MIN);
		ret = CMD_RET_USAGE;
		goto out;
	}

	src_addr = (uchar *)(uintptr_t)simple_strtoul(argv[1], NULL, 16);
	dst_addr = (uchar *)(uintptr_t)simple_strtoul(argv[2], NULL, 16);
	offset = (ulong)simple_strtoul(argv[3], NULL, 16);
	image_size = (ulong)simple_strtoul(argv[4], NULL, 16);

	ret = decompress_by_step(dst_addr, CONFIG_SYS_BOOTM_LEN, src_addr, &z_len, offset, step);
	if (ret != 0) {
		pr_err("Fail to get image from flash(%d)\n", ret);
		goto out;
	}

	decomp_done = true;

	ret = read_dtb(offset, image_size, (ulong)src_addr, z_len);
	if (ret != 0) {
		goto out;
	}

	if (sub_cmd)
		run_command(sub_cmd, 0);

	memset(bootm_str, 0, sizeof(bootm_str));
	snprintf(bootm_str, sizeof(bootm_str), "bootm 0x%p", src_addr);
	pr_info("%s\n", bootm_str);
	run_command(bootm_str, 0);
out:
	return ret;

}

U_BOOT_CMD(
	hwgunzip,  3,  0,  do_hw_gunzip,
	"uncompress file with hardware IP, eg:[ugzip compress_addr uncompress_addr]",
	"hwgunzip <src> <dst>\n"
	"src and dst must be 16Byte-aligned"
);

U_BOOT_CMD(
	boothz,  7,  0,  do_boothz,
	"Read and decompress image by hardware: boothz <src> <dst> <flash offset> <size> [sub command | chunk size]",
	"<src> <dst> <flash offset> <size> [sub command | chunk size]\n"
	"       <src> <dst> <flash offset> <size> <sub command> <chunk size>\n"
 	"    - Read image from <flash offset> to <src>, and decompress to <dst> by hardware decompressed module.\n"
	"      eg:\n"
	"        boothz 0x41000000 0x40008000 0x100000 0x400000\n"
	"        boothz 0x41000000 0x40008000 0x100000 0x400000 xmediaapp\n"
	"        boothz 0x41000000 0x40008000 0x100000 0x400000 0x40000\n"
	"        boothz 0x41000000 0x40008000 0x100000 0x400000 xmediaapp 0x40000\n"
	"      <src> and <dst> is DDR location, and must be 16Byte-aligned;\n"
	"      <flash offset> and <size> must be flash-block-size-aligned;\n"
	"      <chunk size> is one step size for reading and decompressing one time;\n"
	"      <sub command> is a command that will be run after decompressing image and before going to kernel."
);
