/*
 *  Copyright (c) XMEDIA. All rights reserved.
 */
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/div64.h>
#include <linux/io.h>
#include <linux/export.h>
#include <linux/clk.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/cpu.h>
#include <linux/smp.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/sched_clock.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/signal.h>
#include <linux/poll.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/usb.h>
#include <linux/usbdevice_fs.h>
#include <linux/usb/hcd.h>
#include <linux/cdev.h>
#include <linux/notifier.h>
#include <linux/security.h>
#include <linux/user_namespace.h>
#include <linux/scatterlist.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/dma-mapping.h>
#include <asm/byteorder.h>
#include <linux/moduleparam.h>
#include <linux/hrtimer.h>
#include <linux/xmedia/timestamp.h>


static struct task_struct *s_testd1 = NULL;
static struct task_struct *s_testd2 = NULL;
static struct task_struct *s_testd3 = NULL;
static struct hrtimer s_test_hrtimer;

int timestamp_testd1(void *data)
{
	int ret = 0;
	char *func = (char *)__func__;
	unsigned int line = __LINE__;
	unsigned int type = 1;

	while (1) {
		ret = stopwatch_trigger();
		if (ret == -ENOMEM) {
			stopwatch_clear();
		}
		ret = timestamp_mark(func, line, type);
		if (ret != 0) {
			timestamp_clean();
		}
		msleep(1);
	}
}

int timestamp_testd2(void *data)
{
	int ret = 0;
	char *func = (char *)__func__;
	unsigned int line = __LINE__;
	unsigned int type = 1;

	while (1) {
		msleep(1);
		ret = stopwatch_trigger();
		if (ret == -ENOMEM) {
			stopwatch_clear();
		}
		ret = timestamp_mark(func, line, type);
		if (ret != 0) {
			timestamp_clean();
		}
	}
}

int timestamp_testd3(void *data)
{
	while (1) {
		stopwatch_print();
		timestamp_print(0);
		timestamp_print(1);
		timestamp_print(2);
		msleep(5000);
	}
}

#define TEST_HRTIMER_MS 2

enum hrtimer_restart timestamp_test_hrtimer_fn(struct hrtimer *this)
{
	int ret = 0;
	char *func = (char *)__func__;
	unsigned int line = __LINE__;
	unsigned int type = 1;

	stopwatch_trigger();
	if (ret == -ENOMEM) {
		stopwatch_clear();
	}
	ret = timestamp_mark(func, line, type);
	if (ret != 0) {
		timestamp_clean();
	}
	hrtimer_forward_now(this, ms_to_ktime(TEST_HRTIMER_MS));
	return HRTIMER_RESTART;
}

void timestamp_test(void)
{
	int i;

	printk("Clear stopwatch\n");
	stopwatch_clear();
	mdelay(1);
	stopwatch_print();

	printk("Correction test\n");
	for (i = 0; i < 10; ++i) {
		stopwatch_trigger();
		mdelay(i);
	}
	printk("Correction checking:\n");
	stopwatch_print();

	printk("Multi thread and interrupt context test\n");
	s_testd1 = kthread_run(timestamp_testd1, NULL, "timestamp testd1");
	s_testd2 = kthread_run(timestamp_testd2, NULL, "timestamp testd2");
	s_testd3 = kthread_run(timestamp_testd3, NULL, "timestamp testd3");
	hrtimer_init(&s_test_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	s_test_hrtimer.function = timestamp_test_hrtimer_fn;
	printk("hrtimer start...");
	hrtimer_start(&s_test_hrtimer, ms_to_ktime(TEST_HRTIMER_MS), HRTIMER_MODE_REL);
	printk("done\n");
}

