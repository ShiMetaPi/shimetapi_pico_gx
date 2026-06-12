#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "xmedia_pm.h"
#include "xmedia_sys.h"

#define AOV_PERIOD 1000 //ms
#define AOV_CALIBRATION_LOOPS 10

xmedia_pm_attr pm_wakeup_src = {
	.wakeup_attr = {
		.wakeup_enable = 0,//1,
		.wakeup_polarity = PMC_PWR_FALLING_EDGE,
	},
	.rtc_attr = {
		.rtc_enable = 1,
		.rtc_timeout = AOV_PERIOD / 10,
	},
	.button_enable = 1,
	.button_attr = {
		.button_adj_enable = 1,
		.button_adj_time = 1,	//default: 1, resolution: 100ms
	},
};

char *wakeup_source_str[] = {
	"gpio wakeup",
	"ldo wakeup",
	"button wakeup",
	"rtc wakeup",
	"unknown wakeup",
};

xmedia_s32 pm_popen(xmedia_char *command)
{
	FILE *fp = popen(command, "w");
	if (fp == NULL) {
		printf("popen ""%s""fail \n", command);
		return XMEDIA_FAILURE;
	}

	pclose(fp);
	return XMEDIA_SUCCESS;
}

void pm_trigger_stopwatch(void)
{
	pm_popen("echo 1 >/proc/stopwatch");
}

void pm_enable_stopwatch_info(void)
{
/* #define PM_SHOW_STOPWATCH */
#ifdef PM_SHOW_STOPWATCH
	pm_popen("xmmm 0x120200bc 0xa");
#endif
}

#define DEFAULT_TEST_COUNT			1000000
xmedia_s32 main(xmedia_s32 argc, char* argv[])
{
	xmedia_pm_attr wakeup_src;
	unsigned int count = 0;
	int status;
	xmedia_pm_wakeup_source wake_source;
	unsigned int test_count = DEFAULT_TEST_COUNT;
	xmedia_pm_init_param param;
	xmedia_pm_rtc_info rtc_info;
	xmedia_u64 last_rtc_resume_time = 0, delta = 0;
	xmedia_s64 delta_sum = 0, delta_cnt = 0;
	xmedia_s32 ret;

	if (argc == 2) {
		test_count = (unsigned int)atoi(argv[1]);
	}

	ret = xmedia_sys_init(XMEDIA_NULL);
	if (ret != XMEDIA_SUCCESS) {
		printf("xmedia_sys_init err, errno %d\n", ret);
		return ret;
	}

	param.type = PM_INIT_TYPE_STR;
	if (xmedia_pm_init_ext(&param) != XMEDIA_SUCCESS) {
		printf("pm init error\n");
		goto exit;
	}

	if (xmedia_pm_set_attr(&pm_wakeup_src) != XMEDIA_SUCCESS) {
		printf("pm set wakeup source error\n");
		goto exit;
	}

	while (count < test_count) {
		if (xmedia_pm_get_attr(&wakeup_src) != XMEDIA_SUCCESS) {
			printf("pm get attr error\n");
			break;
		} else {
			printf("gpio %s attr polarity:%d\n", (wakeup_src.wakeup_attr.wakeup_enable ? "enable" : "disable"), wakeup_src.wakeup_attr.wakeup_polarity);
			printf("button %s\n", wakeup_src.button_enable ? "enable" : "disable");
			printf("rtc %s\n", (wakeup_src.rtc_attr.rtc_enable ? "enable" : "disable"));
		}

		pm_trigger_stopwatch();

		pm_enable_stopwatch_info();

		status = xmedia_pm_suspend();
		if (status != XMEDIA_SUCCESS) {
			printf("str suspend resume %d counts, now error\n", count);
			break;
		}

		pm_trigger_stopwatch();

		if (xmedia_pm_get_wakeup_source(&wake_source) != XMEDIA_SUCCESS) {
			printf("get wakeup source error\n");
			break;
		}

		printf("wakeup souce:%s\n", wakeup_source_str[wake_source]);

		count++;

		/* AOV Period Calibration */
		if (xmedia_pm_get_rtc_info(&rtc_info) != XMEDIA_SUCCESS) {
			printf("get rtc info error\n");
			break;
		}

		delta = rtc_info.rtc_delta_time - last_rtc_resume_time;

		printf("rtc resume time: %llu ms, delta: %llu ms\n", rtc_info.rtc_delta_time, delta);
		printf("rtc current time: %llu ms\n", rtc_info.rtc_current_time);
		printf("rtc last suspend time: %llu ms\n", rtc_info.rtc_last_suspend_time);

		last_rtc_resume_time = rtc_info.rtc_delta_time;

		if (delta_cnt < AOV_CALIBRATION_LOOPS && count != 1) {
			delta_sum += delta;
			delta_cnt++;
		}

		if (delta_cnt == AOV_CALIBRATION_LOOPS) {
			sleep(2);
			printf("delta_sum=%lld, delta_cnt=%lld\n", delta_sum, delta_cnt);
			pm_wakeup_src.rtc_attr.rtc_timeout = ((xmedia_s64)AOV_PERIOD - (delta_sum / delta_cnt - (xmedia_s64)AOV_PERIOD)) / 10;
			printf("set pm attr again: rtc_timeout = %d\n", pm_wakeup_src.rtc_attr.rtc_timeout);
			if (xmedia_pm_set_attr(&pm_wakeup_src) != XMEDIA_SUCCESS) {
				printf("pm set attr error\n");
				break;
			}
			delta_cnt++;
		}


		printf("str suspend resume %d counts\n", count);
	}

exit:
	xmedia_pm_deinit();
	xmedia_sys_exit();
}
