// SPDX-License-Identifier: GPL-2.0
/* Copyright (c) LOTUS. All rights reserved. */

#include <common.h>
#include <sparse_format.h>

#if (CONFIG_AUTO_UPDATE == 1)
#if (CONFIG_AUTO_USB_UPDATE == 1 || CONFIG_AUTO_SD_UPDATE == 1)
#include <command.h>
#include <malloc.h>
#include <fat.h>
#include <console.h>
#include <linux/io.h>

#include "update_comm.h"
#include "au_nand_flash.h"
#include "au_spinor_flash.h"
#include "au_emmc_flash.h"

#if (UBOOTVERSION == 201611)
#include <environment.h>
#endif

//#define CONFIG_XMEDIA_UPGRADE_BY_SEGMENT 1

#if CONFIG_XMEDIA_UPGRADE_BY_SEGMENT
#define SECTION_SIZE 0x1000000 // 16M
#endif

/* possible names of files on the medium. */
#define AU_CONFIG   "config"
#define CONFIG_MAX_SIZE 2048
#define ENV_MAX_LEN    (CONFIG_MAX_SIZE / 2)

/* where to load files into memory */
#ifdef CONFIG_BOOT_STORE_ADDR
#define LOAD_ADDR CONFIG_BOOT_STORE_ADDR
#endif

#define MIN(x, y) ((x) > (y) ? (y) : (x))

#include "chip_adapter.h"

typedef enum {
	FS_RAW,
	FS_JFFS2,
	FS_YAFFS2,
	FS_EXT4,
	FS_UBI,
	FS_BUTT
} FS_TYPE;

struct au_store {
	loff_t exp_len;
	loff_t act_len;
	loff_t file_pos;
	loff_t file_size;
};

struct au_flash {
	unsigned long part_start;
	unsigned long part_offset;
	unsigned long part_size;
	unsigned long write_len;
};

struct au_image {
	void *addr;
	unsigned long addr_size;
	struct au_store store;
	struct au_flash flash;
};

static int au_flash_init(int boot_type)
{
	int ret = UP_FAILURE;

	switch (boot_type) {
		case AD_BOOTFROM_SPINOR:
			printf("spinor init...\n");
			ret = au_spinor_flash_init();
			break;

		case AD_BOOTFROM_NAND:
			printf("nand init...\n");
			ret = au_nand_flash_init();
			break;

		case AD_BOOTFROM_EMMC:
			printf("emmc init...\n");
			ret = au_emmc_flash_init();
			break;

		default:
			printf("%s: Unkonw flash type.\n", __func__);
	}

	return ret;
}

static int au_flash_erase(int boot_type, unsigned long offset, unsigned long len)
{
	int ret = UP_FAILURE;

	switch (boot_type) {
		case AD_BOOTFROM_SPINOR:
			printf("spinor erase...\n");
			ret = au_spinor_flash_erase(offset, len);
			break;

		case AD_BOOTFROM_NAND:
			printf("nand erase...\n");
			ret = au_nand_flash_erase(offset, len);
			break;

		case AD_BOOTFROM_EMMC:
			printf("emmc erase...\n");
			ret = au_emmc_flash_erase(offset, len);
			break;

		default:
			printf("%s: Unkonw flash type.\n", __func__);
	}

	return ret;
}

static int au_flash_naked_write(int boot_type, unsigned long offset,
		unsigned long len, unsigned long lim, unsigned char* buf)
{
	int ret = UP_FAILURE;

	switch (boot_type) {
		case AD_BOOTFROM_SPINOR:
			printf("spinor write...\n");
			ret = au_spinor_flash_write(offset, len, lim, buf);
			break;

		case AD_BOOTFROM_NAND:
			printf("nand write...\n");
			ret = au_nand_flash_write(offset, len, lim, buf);
			break;

		case AD_BOOTFROM_EMMC:
			printf("emmc write...\n");
			ret = au_emmc_flash_write(offset, len, lim, buf);
			break;

		default:
			printf("%s: Unkonw flash type.\n", __func__);
	}

	return ret;
}

static int au_spinor_fs_write(int fs_type, unsigned long offset,
		unsigned long len, unsigned long lim, unsigned char* buf)
{
	int ret = UP_FAILURE;

	switch (fs_type) {
		case FS_JFFS2:
			printf("spinor jffs2 write...\n");
			ret = au_spinor_flash_jffs_write(offset, len, lim, buf);
			break;

		case FS_UBI:
			printf("spinor ubifs write...\n");
			ret = au_spinor_flash_ubifs_write(offset, len, lim, buf);
			break;

		default:
			printf("%s: Unsupport fs type.\n", __func__);
	}

	return ret;
}

static int au_nand_fs_write(int fs_type, unsigned long offset,
		unsigned long len, unsigned long lim, unsigned char* buf)
{
	int ret = UP_FAILURE;

	switch (fs_type) {
		case FS_YAFFS2:
			printf("nand yaffs2 write...\n");
			ret = au_nand_flash_yaffs_write(offset, len, lim, buf);
			break;

		case FS_UBI:
			printf("nand ubifs write...\n");
			ret = au_nand_flash_ubifs_write(offset, len, lim, buf);
			break;

		default:
			printf("%s: Unsupport fs type.\n", __func__);
	}

	return ret;
}

static int au_emmc_fs_write(int fs_type, unsigned long offset,
		unsigned long len, unsigned long lim, unsigned char* buf)
{
	int ret = UP_FAILURE;

	switch (fs_type) {
		case FS_EXT4:
			printf("emmc ext4 write...\n");
			ret = au_emmc_flash_ext4_write(offset, len, lim, buf);
			break;

		default:
			printf("%s: Unsupport fs type.\n", __func__);
	}

	return ret;
}

static int au_flash_fs_write(int boot_type, int fs_type, unsigned long offset,
		unsigned long len, unsigned long lim, unsigned char* buf)
{
	int ret = UP_FAILURE;

	switch (boot_type) {
		case AD_BOOTFROM_SPINOR:
			ret = au_spinor_fs_write(fs_type, offset, len, lim, buf);
			break;

		case AD_BOOTFROM_NAND:
			ret = au_nand_fs_write(fs_type, offset, len, lim, buf);
			break;

		case AD_BOOTFROM_EMMC:
			ret = au_emmc_fs_write(fs_type, offset, len, lim, buf);
			break;

		default:
			printf("%s: Unkonw flash type.\n", __func__);
	}

	return ret;
}

static unsigned long au_flash_get_page_size(int boot_type, int with_oob)
{
	unsigned long page_size = 1;

	if (boot_type == AD_BOOTFROM_NAND) {
		page_size = (unsigned long)au_nand_flash_get_page_size(with_oob);
	} else if (boot_type == AD_BOOTFROM_EMMC){
		page_size = (unsigned long)au_emmc_flash_get_block_size();
	}

	return page_size;
}

static unsigned long au_flash_get_erase_size(int boot_type)
{
	int erase_size = 1;

	if (boot_type == AD_BOOTFROM_NAND) {
		erase_size = au_nand_flash_get_erase_size();
	}

	return erase_size;
}

static int au_flash_get_bad_blkcnt(int boot_type, unsigned long start, unsigned long len)
{
	int count = 0;

	if (boot_type == AD_BOOTFROM_NAND) {
		count = au_nand_flash_get_bad_blkcnt(start, len);
	}

	return count;
}

/*
 * pick up env form config file and set env
 * fail:return -1;
 * ok:  return 0;
 */
static int env_pick_up(const char *envname, const char *buffer)
{
	char *str, *str_s, *str_e;
	char env[ENV_MAX_LEN];

	str = strstr(buffer, envname);
	if (!str) {
		printf("ERROR:%s not found!\n", envname);
		return UP_FAILURE;
	}

	str_s = strchr(str, '\'');
	if (!str_s) {
		printf("ERROR:%s Invalid configuration.\n", envname);
		return UP_FAILURE;
	}

	str_e = strchr(++str_s, '\'');
	if (!str_e){
		printf("ERROR:%s Invalid configuration.\n", envname);
		return UP_FAILURE;
	}

	if ((unsigned long)(str_e - str_s) > ENV_MAX_LEN) {
		printf("ERROR:%s too long!\n", envname);
		return UP_FAILURE;
	}

	strncpy(env, str_s, (size_t)(str_e - str_s));
	env[(size_t)(str_e - str_s)] = '\0';
#if (UBOOTVERSION == 202001)
	env_set((char *)envname, env);
#elif (UBOOTVERSION == 201611)
	setenv((char *)envname, env);
#else
	printf("%s:invalid uboot version!\n", __func__);
	return UP_FAILURE;
#endif

	return UP_SUCCESS;
}

/*
 * pick up bootargs and bootcmd from config file and save env
 * fail:return -1;
 * ok:  return 0;
 */
static int get_env_from_config(void)
{
	char *aufile = AU_CONFIG;
	int ret;
	long sz = file_fat_read(aufile, (void *)LOAD_ADDR, CONFIG_MAX_SIZE);

	if (sz <= 0) {
		printf("ERROR:%s not found!\n", aufile);
		return UP_FAILURE;
	} else {
		debug_print("Load config to %08x, size=%ld.\n", LOAD_ADDR, sz);
	}

	ret = env_pick_up("bootargs", (char *)LOAD_ADDR);
	if (ret < 0) {
		return UP_FAILURE;
	}
#if 0
	ret = env_pick_up("bootcmd", (char *)LOAD_ADDR);
	if (ret < 0) {
		return UP_FAILURE;
	}
#endif

	return UP_SUCCESS;
}

/* max. number of files which could interest us */
#define AU_MAXFILES 32

#define NAME_MAX_LEN 0x20
#define DECI_VALUE		10
#define STR_STEPS		2
#define SIZE_M			(1024 * 1024)
#define SIZE_K			1024

struct flash_layout {
	long start;
	long end;
} flash_layout_t;

struct mtd_part_s {
	char *str;
	int unit;
	struct mtd_part_s *next;
};

/* sizes of flash areas for each file */
long ausize[AU_MAXFILES] = {0};

/* array of flash areas start and end addresses */
struct flash_layout aufl_layout[AU_MAXFILES];

/* pointers to file names */
char *aufile[AU_MAXFILES] = {0};

unsigned long aufile_size[AU_MAXFILES] = {0};
char aufile_table[AU_MAXFILES][NAME_MAX_LEN];
/*
 * insert node to list
 */
static struct mtd_part_s *list_insert(struct mtd_part_s *head, struct mtd_part_s *node)
{
	struct mtd_part_s *p = head;

	if (!head)
		head = node;

	while (p) {
		if (p->next == NULL) {
			p->next = node;
			break;
		}
		p = p->next;
	}

	return head;
}

/*
 * sort list by str
 */
struct mtd_part_s *mtd_list_sort(struct mtd_part_s *head)
{
	int flag;
	struct mtd_part_s *p, *pt1, *pt2;

	if (!head)
		return NULL;

	if (head->next == NULL)
		return head;

	do {
		flag = 0;

		if ((uintptr_t)(head->str) >= (uintptr_t)
				(head->next->str)) {
			pt1 = head->next->next;
			pt2 = head->next;
			pt2->next = head;
			head->next = pt1;
			head = pt2;
		}

		for (p = head; p->next->next != NULL; p = p->next) {
			if ((uintptr_t)(p->next->str) >= (uintptr_t)
					(p->next->next->str)) {
				pt1 = p->next->next->next;
				pt2 = p->next->next;
				pt2->next = p->next;
				p->next->next = pt1;
				p->next = pt2;
				flag = 1;
			}
		}
	} while (flag);

	return head;
}

/*
 * get mtd parttion info list from env string
 */
static struct mtd_part_s *get_mtd_parts(char *env)
{
	char *str = NULL;
	struct mtd_part_s *part_p = NULL;
	struct mtd_part_s *head = NULL;

	if (env == NULL) {
		printf("env is null!\n");
		return NULL;
	}

	str = env;
	while ((str = strstr(str, "M("))) {
		part_p = malloc(sizeof(struct mtd_part_s));
		if (part_p == NULL)
			return NULL;

		part_p->str = str;
		part_p->unit = SIZE_M;
		part_p->next = NULL;
		head = list_insert(head, part_p);
		str++;
	}

	str = env;
	while ((str = strstr(str, "K("))) {
		part_p = malloc(sizeof(struct mtd_part_s));
		if (part_p == NULL)
			return NULL;

		part_p->str = str;
		part_p->unit = SIZE_K;
		part_p->next = NULL;
		head = list_insert(head, part_p);
		str++;
	}

	head = mtd_list_sort(head);
	return head;
}

static unsigned long get_mtd_part_size(const char *str_num)
{
	unsigned long size = 0;
	int k;
	int j = 0;
	int num;

	while (*str_num >= '0' && *str_num <= '9') {
		k = j;
		num = *str_num - '0';
		while (k) {
			num *= DECI_VALUE;
			k--;
		}
		size += num;
		j++;
		str_num--;
	}

	return size;
}

static int bootargs_analyze(void)
{
	int i;
	long start = 0;
	char *str = NULL;
	char *str_0 = NULL;
	char *envp = NULL;
	char env[ENV_MAX_LEN] = {0};
	struct mtd_part_s *part_p = NULL;

#if (UBOOTVERSION == 202001)
	envp = env_get("bootargs");
#elif (UBOOTVERSION == 201611)
	envp = getenv("bootargs");
#else
	printf("%s:ERROR:invalid uboot version!\n", __func__);
	return UP_FAILURE;
#endif

	if (envp == NULL) {
		printf("ERROR:bootargs is NULL!\n");
		return UP_FAILURE;
	}

	if (strlen(envp) > ENV_MAX_LEN - 1) {
		printf("ERROR:bootargs is too long!\n");
		return UP_FAILURE;
	}

	memset(ausize, 0, sizeof(ausize));
	memset(aufl_layout, 0, sizeof(aufl_layout));
	strcpy(env, envp);

	str = env;
	str_0 = env;
	i = 0;
	part_p = get_mtd_parts(env);

	while (part_p) {
		if (i >= AU_MAXFILES) {
			printf("ERROR:Num of partition is more than %0d!\n",
					AU_MAXFILES);
			break;
		}
		str = part_p->str;
		ausize[i] = get_mtd_part_size(str - 1) * part_p->unit;
		aufl_layout[i].start = start;
		aufl_layout[i].end = start + ausize[i] - 1;
		start += ausize[i];

		str += STR_STEPS;
		str_0 = strstr(str, ")");
		if ((unsigned long)(str_0 - str) > NAME_MAX_LEN) {
			printf("file name len is too long\n");
			return UP_FAILURE;
		}
		strncpy(aufile_table[i], str, (unsigned long)(str_0 - str));
		aufile_table[i][str_0 - str] = '\0';
		aufile[i] = &(aufile_table[i][0]);
		printf("\n[%0d]=%-16s start=0x%08lx end=0x%08lx size=0x%08lx",
				i, aufile_table[i],
				(unsigned long)(aufl_layout[i].start),
				(unsigned long)(aufl_layout[i].end),
				(unsigned long)ausize[i]);
		i++;
		part_p = part_p->next;
	}

	if (i == 0) {
		printf("ERROR:Can't find valid partition info!\n");
		return UP_FAILURE;
	}

	return UP_SUCCESS;
}

/*
 * offset [param in] : offset within the partition
 * success : return 0
 * failure : return 1
 */
static int au_do_update(char* file_name, struct au_image* param)
{
	int rc = 0;
	char *buf = NULL;
	void *pbuf = NULL;
	int boot_type = get_boot_info();
	unsigned long start    = param->flash.part_start;
	unsigned long part_len = param->flash.part_size;
	unsigned long offset   = param->flash.part_offset;
	/* offset on the entire flash */
	unsigned long write_offset = start + offset;
	unsigned long write_len    = param->flash.write_len;

	/* when writing for the first time, erase the entire partition  */
	if (!offset) {
		/* erase the address range. */
		if (au_flash_erase(boot_type, start, part_len) == UP_FAILURE) {
			printf("sector erase failed\n");
			return UP_FAILURE;
		}
	}

	buf = map_physmem((unsigned long)LOAD_ADDR, write_len, MAP_WRBACK);
	if (!buf) {
		printf("Failed to map physical memory\n");
		return UP_FAILURE;
	}

	/* write the whole file to flash;uboot and rootfs image have head
	 * kernel has head,it's head also be writed to flash
	 */
	pbuf = buf;

	/* copy the data from RAM to FLASH */

	if (strstr(file_name, "yaffs2")) {
		rc = au_flash_fs_write(boot_type, FS_YAFFS2, write_offset, write_len, part_len, pbuf);
	} else if (strstr(file_name, "ext4")) {
		rc = au_flash_fs_write(boot_type, FS_EXT4, write_offset, write_len, part_len, pbuf);
	} else if (strstr(file_name, "ubifs")) {
		rc = au_flash_fs_write(boot_type, FS_UBI, write_offset, write_len, part_len, pbuf);
	} else if (strstr(file_name, "jffs2")) {
		rc = au_flash_fs_write(boot_type, FS_JFFS2, write_offset, write_len, part_len, pbuf);
	} else {
		printf("write flash at 0x%lx, size 0x%lx\n", write_offset, write_len);
		rc = au_flash_naked_write(boot_type, write_offset, write_len, part_len, pbuf);
	}

	if (rc != UP_SUCCESS) {
		printf("write flash failed\n");
		return UP_FAILURE;
	}

	unmap_physmem(buf, part_len);

	return UP_SUCCESS;
}

static int init_param(unsigned int idx, loff_t file_size, struct au_image* param)
{
	unsigned long long addr_size = get_ddr_size();
	unsigned long part_size = aufl_layout[idx].end - aufl_layout[idx].start + 1;
	unsigned long mem_size;

#ifdef CONFIG_XMEDIA_UPGRADE_BY_SEGMENT
	mem_size = SECTION_SIZE;
#else
	mem_size = part_size;
#endif

	addr_size = addr_size - (CONFIG_BOOT_STORE_ADDR - CONFIG_SYS_SDRAM_BASE) - 0x100000;
	mem_size = MIN(mem_size, addr_size);

	if (part_size < file_size) {
		printf("error: the written file exceeds the partition size\n");
		return -1;
	}

	param->addr = (void*)LOAD_ADDR;
	param->addr_size = mem_size;

	param->store.file_pos  = 0;
	param->store.act_len   = 0;
	param->store.file_size = file_size;
	param->store.exp_len   = MIN(mem_size, file_size);

	param->flash.part_start  = aufl_layout[idx].start;
	param->flash.part_size   = part_size;
	param->flash.part_offset = 0;
	param->flash.write_len   = 0;

	return 0;
}

static void set_expect_len(char* file_name, struct au_image* param)
{
	unsigned long page_size = 1;
	int boot_type = get_boot_info();

	if (strstr(file_name, "yaffs2")) {
		page_size = au_flash_get_page_size(boot_type, 1);
	} else {
		page_size = au_flash_get_page_size(boot_type, 0);
	}

	if (param->addr_size < param->store.file_size) {
		param->store.exp_len = (param->addr_size / page_size) * page_size;
	} else {
		param->store.exp_len = param->store.file_size;
	}
}

static void set_write_len(char* file_name, struct au_image* param)
{
	if (param->addr_size < param->store.file_size) {
			param->flash.write_len = param->store.act_len;
	} else {
		if (strstr(file_name, "ext4")) {
			param->flash.write_len = param->flash.part_size;
		} else {
			param->flash.write_len = param->store.act_len;
		}
	}

	//printf("write_len: %lu\n", param->flash.write_len);
}

static void set_flash_offset(char* file_name, struct au_image* param)
{
	unsigned long sec_yaffs;
	unsigned long page_size;
	unsigned long erase_size;
	unsigned long offset = 0;
	unsigned long write_start = param->flash.part_start + param->flash.part_offset;
	int bad_blkcnt = 0;
	int boot_type = get_boot_info();

	sec_yaffs  = au_flash_get_page_size(boot_type, 1);
	page_size  = au_flash_get_page_size(boot_type, 0);
	bad_blkcnt = au_flash_get_bad_blkcnt(boot_type, write_start, param->flash.write_len);
	erase_size = au_flash_get_erase_size(boot_type);

	if (strstr(file_name, "yaffs2")) {
		if (param->flash.write_len % sec_yaffs) {
			offset = (param->flash.write_len / sec_yaffs + 1) * page_size;
		} else {
			offset = (param->flash.write_len / sec_yaffs) * page_size;
		}
	} else {
		if (param->flash.write_len % page_size) {
			offset = (param->flash.write_len / page_size + 1) * page_size;
		} else {
			offset = param->flash.write_len / page_size * page_size;
		}
	}

	if (bad_blkcnt) {
		printf("Found %d bad blocks\n", bad_blkcnt);
	}

	offset += (bad_blkcnt * erase_size);

	param->flash.part_offset += offset;

	//printf("offset: %lu\n", param->flash.part_offset);
}

static int check_mem(unsigned long mem_start, unsigned long size)
{
	unsigned long long soc_addr_start = CONFIG_SYS_SDRAM_BASE;
	unsigned long long soc_addr_size  = get_ddr_size;
	unsigned long long soc_addr_end   = soc_addr_start + soc_addr_size;

	if (mem_start < soc_addr_start) {
		printf("Invliad memory addr\n");
		return -1;
	}

	if ((mem_start + size) > soc_addr_end) {
		printf("Memory out of bounds.\n");
		return -1;
	}

	return 0;
}

static int get_image(char* file_name, struct au_image* param)
{
	set_expect_len(file_name, param);

	if (check_mem((unsigned long)param->addr, param->store.exp_len) < 0) {
		return -1;
	}

	memset(param->addr, 0xff, param->store.exp_len);

	file_fat_read_at(file_name,
			param->store.file_pos,
			param->addr,
			param->store.exp_len,
			&(param->store.act_len));
	printf("read %s, exp_len:%lld, act_len:%lld\n",
			file_name, param->store.exp_len, param->store.act_len);

	set_write_len(file_name, param);

	if (param->store.act_len <= 0) {
		printf("read %s failed!\n", file_name);
		return -1;;
	}

	param->store.file_pos += param->store.act_len;

	return 0;
}

static int check_flash_part(int idx, char* file_name, unsigned long file_size)
{
	int bad_block_count;
	unsigned int pages_len;
	unsigned long part_start = aufl_layout[idx].start;
	unsigned long part_end   = aufl_layout[idx].end;
	unsigned long part_size  = part_end - part_start + 1;
	int boot_type = get_boot_info();
	unsigned int page_size = au_flash_get_page_size(boot_type, 1);
	unsigned int write_size = au_flash_get_page_size(boot_type, 0);
	unsigned int erase_size = au_flash_get_erase_size(boot_type);

	if (part_size < file_size) {
		printf("error: the written file exceeds the partition size\n");
		return -1;
	}

	if (strstr(file_name, "yaffs2")) {
		if (file_size % page_size) {
			pages_len = (file_size / page_size + 1) * write_size;
		} else {
			pages_len = (file_size / page_size) * write_size;
		}
	} else {
		if (file_size % page_size) {
			pages_len = (file_size / write_size + 1) * write_size;
		} else {
			pages_len = (file_size / write_size) * write_size;
		}
	}

	bad_block_count = au_flash_get_bad_blkcnt(boot_type, part_start, pages_len);
	if (part_end < (part_start + file_size + (bad_block_count * erase_size))) {
		printf("The actual length of the partition is greater\
				than the specified value due to the address\
				offset caused by bad blocks.\n");
		return -1;
	}

	return 0;
}

static int do_update_ordinary_img(char* img_name, struct au_image* param)
{
	int res = -1;

	do {
		if (get_image(img_name, param) < 0) {
			break;
		}

		/* this is really not a good idea, but it's what the */
		/* customer wants. */
		do {
			res = au_do_update(img_name, param);
			/* let the user break out of the loop */
			if (ctrlc() || had_ctrlc()) {
				clear_ctrlc();

				break;
			}

			set_flash_offset(img_name, param);
		} while (res < 0);

	} while (param->store.file_size > param->store.file_pos);

	return res;
}

static int do_update_special_img(char* img_name, struct au_image* param)
{
	int i;
	loff_t act_size;
	unsigned int tmp_addr_size;
	unsigned int exp_size, raw_size;
	unsigned int total_chunks;
	//unsigned int total_blks;
	char *tmp_addr = NULL;
	unsigned int tmp_addr_pos = 0;
	chunk_header_t chunk_header = {0};
	sparse_header_t sparse_header = {0};
	int boot_type = get_boot_info();
	unsigned long block_size = au_flash_get_page_size(boot_type, 0);

	file_fat_read_at(img_name, 0, &sparse_header, sizeof(sparse_header_t), &act_size);
	if (act_size != sparse_header.file_hdr_sz) {
		printf("Error: file_fat_read sparse_header fail.\n");
		return -1;
	}

	if (sparse_header.magic != SPARSE_HEADER_MAGIC) {
		/* For unsparse ext4 image. */
		return do_update_ordinary_img(img_name, param);
	} else {
		/* For sparse ext4 image. */

		param->store.file_pos += sparse_header.file_hdr_sz;
		//total_blks   = sparse_header.total_blks;
		total_chunks = sparse_header.total_chunks;

		memset(param->addr, 0xFF, param->addr_size);
		tmp_addr_pos = 0;
		tmp_addr_size = (param->addr_size / block_size) * block_size;

		for (i = 0; i < total_chunks; i++) {
			file_fat_read_at(img_name, param->store.file_pos, &chunk_header, sizeof(chunk_header_t), &act_size);
			if (act_size != sparse_header.chunk_hdr_sz) {
				printf("Error: file_fat_read chunk_header fail.\n");
				return -1;
			}

			param->store.file_pos += sparse_header.chunk_hdr_sz;

			switch (chunk_header.chunk_type) {
				case CHUNK_TYPE_RAW:
					//printf("R->chunk:%d, offset:%lu, number: %u\n", i, param->flash.part_offset, chunk_header.chunk_sz);
					raw_size = chunk_header.total_sz - sparse_header.chunk_hdr_sz;
					if (raw_size > (tmp_addr_size - tmp_addr_pos)) {
						do {
							exp_size = tmp_addr_size - tmp_addr_pos;
							exp_size = (exp_size > raw_size ? raw_size : exp_size);
							tmp_addr = (char*)param->addr + tmp_addr_pos;
							file_fat_read_at(img_name, param->store.file_pos, tmp_addr, exp_size, &act_size);
							if (exp_size != act_size) {
								printf("Error: file_fat_read raw fail\n");
								return -1;
							}

							tmp_addr_pos += act_size;
							param->store.file_pos += act_size;

							raw_size -= act_size;

							if (tmp_addr_pos) {
								param->flash.write_len = tmp_addr_pos;
								if (au_do_update("raw", param) != UP_SUCCESS) {
									return -1;
								} else {
									param->flash.part_offset += tmp_addr_pos;
									tmp_addr_pos = 0;
								}
							}
						} while (raw_size);
					} else {
						// exp_size = chunk_header.chunk_sz * sparse_header.blk_sz;
						exp_size = raw_size;
						tmp_addr = (char*)param->addr + tmp_addr_pos;
						file_fat_read_at(img_name, param->store.file_pos, tmp_addr, exp_size, &act_size);
						if (exp_size != act_size) {
							printf("Error: file_fat_read raw fail\n");
							return -1;
						}

						tmp_addr_pos += act_size;
						param->store.file_pos += act_size;
					}
					break;

				case CHUNK_TYPE_DONT_CARE:
					//printf("D->chunk:%d, offset:%lu, number: %u\n", i, param->flash.part_offset, chunk_header.chunk_sz);
					if (tmp_addr_pos) {
						param->flash.write_len = tmp_addr_pos;
						if (au_do_update("raw", param) != UP_SUCCESS) {
							return -1;
						} else {
							param->flash.part_offset += tmp_addr_pos;
							tmp_addr_pos = 0;
						}
					}
					raw_size = chunk_header.chunk_sz * sparse_header.blk_sz;
					param->flash.part_offset += raw_size;
					break;

				case CHUNK_TYPE_FILL:
					printf("Error: Unsupport chunk type.\n");
					break;

				default:
					printf("Error: Unkonw chunk type.\n");
					break;
			}

			if (tmp_addr_pos == tmp_addr_size) {
				param->flash.write_len = tmp_addr_pos;
				if (au_do_update("raw", param) != UP_SUCCESS) {
					return -1;
				} else {
					param->flash.part_offset += tmp_addr_pos;
					tmp_addr_pos = 0;
				}
			}
		}

		if (tmp_addr_pos) {
			param->flash.write_len = tmp_addr_pos;
			if (au_do_update("raw", param) != UP_SUCCESS) {
				return -1;
			} else {
				param->flash.part_offset += tmp_addr_pos;
				tmp_addr_pos = 0;
			}
		}
	}

	return 0;
}

/*
 * If none of the update file(u-boot, bootargs, kernel or rootfs) was found
 * in the medium, return -1;
 * Others, return 0;
 */
static int update_to_flash(void)
{
	int i;
	int res = -1;
	int updatefile_found = 0;
	struct au_image param = {0};
	loff_t file_size = 0;

	/* just loop thru all the possible files */
	for (i = 0; i < AU_MAXFILES && aufile[i] != NULL; i++) {
		printf("\n");

		if (!fat_exists(aufile[i])) {
			printf("%s not found!\n", aufile[i]);
			continue;
		}

		/* get file's real size */
		if (!fat_size(aufile[i], &file_size)) {
			//file_size = ALIGN((unsigned long)file_size, CONFIG_SYS_CACHELINE_SIZE);
		} else {
			printf("get file size of %s failed!\n", aufile[i]);
			continue;
		}
		printf("%s size=0x%08llx\n", aufile[i], file_size);

		if (check_flash_part(i, aufile[i], file_size) < 0) {
			continue;
		}

		if (init_param(i, file_size, &param) < 0) {
			continue;
		}

		if ((strstr(aufile[i], "ext4")) && (param.addr_size < param.store.file_size)) {
			res = do_update_special_img(aufile[i], &param);
		} else {
			res = do_update_ordinary_img(aufile[i], &param);
		}

		if (res == 0)
			updatefile_found = 1;
	}

	if (updatefile_found == 1)
		return UP_SUCCESS;
	else
		return UP_FAILURE;
}

#endif // CONFIG_AUTO_SD_UPDATE || CONFIG_AUTO_USB_UPDATE

#if (CONFIG_AUTO_SD_UPDATE == 1)

#ifndef CONFIG_MMC
#error "should have defined CONFIG_MMC"
#endif
#include "store_mmc.h"

static int is_mmc(void)
{
	const char dev_name[8] = "mmc";
	int part_no = 0;
	int dev_no = 0;
	struct blk_desc *stor_dev;

	if (store_mmc_init() != UP_SUCCESS) {
		return UP_FALSE;
	}

	dev_no = mmc_get_target_dev();
	if (dev_no < 0) {
		return UP_FALSE;
	}

	stor_dev = blk_get_dev(dev_name, dev_no);
	if (stor_dev == NULL) {
		printf("Unknown device type!\n");
		return UP_FALSE;
	}

retry:
	debug_print("device name %s, device [%d], part[%d].\n", dev_name, dev_no, part_no);
	if (fat_register_device(stor_dev, part_no) != 0) {
		printf("Unable to use %s %d:%d for fatls\n", dev_name, dev_no, part_no);
		if (++part_no <= 4) {
			goto retry;
		}

		return UP_FALSE;
	}
#if 0
	if (file_fat_detectfs() != 0) {
		printf("file_fat_detectfs failed\n");
		return UP_FALSE;
	}

	return UP_TRUE;
#else
	if (fat_exists("u-boot.bin") || fat_exists("config")) {
		return UP_TRUE;
	} else if (++part_no <= 4) {
		goto retry;
	} else {
		printf("File not found\n");
	}

	return UP_FALSE;
#endif
}

static int update_by_mmc(void)
{
	int boot_type = get_boot_info();
	int state = UP_FAILURE;
	int old_ctrlc;

	if (is_mmc() != UP_TRUE) {
		store_mmc_deinit();
		return UP_FAILURE;
	}

	if (au_flash_init(boot_type) == UP_FAILURE) {
		store_mmc_deinit();
		return UP_FAILURE;
	}

	if (get_env_from_config() == UP_FAILURE) {
		printf("Try to use old env!\n");
	}

	if (bootargs_analyze() == UP_FAILURE) {
		printf("ERROR:bootargs analyze fail!\n");
		store_mmc_deinit();
		return UP_FAILURE;
	}

	/*
	 * make sure that we see CTRL-C
	 * and save the old state
	 */
	old_ctrlc = disable_ctrlc(0);

	state = update_to_flash();

	/* restore the old state */
	disable_ctrlc(old_ctrlc);

	store_mmc_deinit();

	return state;
}

#endif // CONFIG_AUTO_SD_UPDATE

#if (CONFIG_AUTO_USB_UPDATE == 1)

#ifndef CONFIG_XMEDIA_MC
#if !defined CONFIG_USB_OHCI && !defined CONFIG_USB_XHCI_HCD
#error "should have defined CONFIG_USB_OHCI or CONFIG_USB_XHCI"
#endif
#endif
#ifndef CONFIG_USB_STORAGE
#error "should have defined CONFIG_USB_STORAGE"
#endif
#include "store_usb.h"

static int is_usb(void)
{
	const char dev_name[8] = "usb";
	int part_no = 0;
	const int dev_no = 0;
	struct blk_desc *stor_dev = NULL;

	if (store_usb_init() != UP_SUCCESS) {
		return UP_FALSE;
	}

	debug_print("device name %s, device[%d] part[%d].\n", dev_name, dev_no, part_no);
	stor_dev = blk_get_dev(dev_name, dev_no);
	if (stor_dev == NULL) {
		printf("Unknown device type!\n");
		return UP_FALSE;
	}

retry:
	if (fat_register_device(stor_dev, part_no) != 0) {
		printf("Unable to use %s %d:%d for fatls\n", dev_name, dev_no, part_no);
		if (++part_no < 4) {
			goto retry;
		}
		return UP_FALSE;
	}

#if 0
	if (file_fat_detectfs() != 0) {
		printf("file_fat_detectfs failed\n");
		return UP_FALSE;
	}

	return UP_TRUE;
#else
	if (fat_exists("u-boot.bin") || fat_exists("config")) {
		return UP_TRUE;
	} else if (++part_no <= 4) {
		goto retry;
	} else {
		printf("File not found\n");
	}

	return UP_FALSE;
#endif
}

static int update_by_usb(void)
{
	int boot_type = get_boot_info();
	int state = UP_FAILURE;
	int old_ctrlc;

	if (is_usb() != UP_TRUE) {
		store_usb_deinit();
		return UP_FAILURE;
	}

	if (au_flash_init(boot_type) == UP_FAILURE) {
		store_usb_deinit();
		return UP_FAILURE;
	}

	if (get_env_from_config() == UP_FAILURE) {
		printf("Try to use old env!\n");
	}

	if (bootargs_analyze() == UP_FAILURE) {
		printf("ERROR:bootargs analyze fail!\n");
		store_usb_deinit();
		return UP_FAILURE;
	}

	/*
	 * make sure that we see CTRL-C
	 * and save the old state
	 */
	old_ctrlc = disable_ctrlc(0);

	state = update_to_flash();

	/* restore the old state */
	disable_ctrlc(old_ctrlc);

	store_usb_deinit();

	return state;
}
#endif // CONFIG_AUTO_USB_UPDATE

int do_auto_update(void)
{
#if (CONFIG_AUTO_SD_UPDATE == 1)
	if (update_by_mmc() == UP_SUCCESS) {
		return 0;
	}
#endif

#if (CONFIG_AUTO_USB_UPDATE == 1)
	if (update_by_usb() == UP_SUCCESS){
		return 0;
	}
#endif

	return -1;
}

#endif // CONFIG_AUTO_UPDATE
