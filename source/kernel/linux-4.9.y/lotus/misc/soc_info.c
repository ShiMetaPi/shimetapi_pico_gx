
// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <mach/platform.h>
#include <linux/lotus/soc_chip.h>
#include <linux/version.h>

#define _SOC_INFO_PROC_NAME "soc_info"

static int proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "ChipID      : 0x%8X\n", get_chipid());
	seq_printf(m, "CpuFreq     : %d\n", get_cpu_freq());

	return 0;
}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
static int __init lotus_soc_info_init(void)
{
	pr_info("ChipID: 0x%8X\n", get_chipid());
	pr_info("CpuFreq: %d\n", get_cpu_freq());

	if (!proc_create_single(_SOC_INFO_PROC_NAME, 0, NULL, proc_show)) {
		pr_err("Fail to create proc "_SOC_INFO_PROC_NAME"!\n");
		return -ENOMEM;
	}

	return 0;
}
#else

static int proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_show, NULL);
}

static const struct file_operations proc_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int __init lotus_soc_info_init(void)
{
	pr_info("ChipID: 0x%8X\n", get_chipid());
	pr_info("CpuFreq: %d\n", get_cpu_freq());

	if (!proc_create(_SOC_INFO_PROC_NAME, 0, NULL, &proc_fops)) {
		pr_err("Fail to create proc "_SOC_INFO_PROC_NAME"!\n");
		return -ENOMEM;
	}

	return 0;
}

static void __exit lotus_soc_info_exit(void)
{
    remove_proc_entry(_SOC_INFO_PROC_NAME, NULL);
}
module_exit(lotus_soc_info_exit);

#endif

early_initcall(lotus_soc_info_init);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Lotus");
