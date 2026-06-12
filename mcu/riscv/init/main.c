#include <platform.h>
#include <sys/types.h>
#include <common.h>
#include <version.h>
#include <timer.h>
#include <image.h>
#include <i2c.h>
#include <ut_case.h>
#include "media_sample.h"

int *__errno(void)
{
	return NULL;
}

void show_version(void)
{
#ifdef CONFIG_DEBUG_INFO
	puts("\nRISC-V MCU code - ");
	puts(MCU_VERSION"\n");
	puts("Build: ");
	puts(__DATE__" - "__TIME__"\n");
	puts("\n");
#endif
}

unsigned int main(void)
{
	__attribute__((unused)) int ret;
	show_version();
#ifdef CONFIG_MEDIA_SAMPLE_QUICKSTART
	quickstart_media_preinit();

	ret = run_stage2_func(quickstart_media_start);
	if (ret != 0) {
		puts("UT is Failed\n");
		goto sleep;
	}
#endif

#ifdef CONFIG_UT_ENABLE
	ret = run_ut_in_stage1();
	if (ret != 0) {
		puts("UT in stage1 Failed\n");
		goto sleep;
	}

	ret = run_stage2_func(run_ut_in_stage2);
	if (ret != 0) {
		puts("UT in stage2 Failed\n");
		goto sleep;
	}
#endif
#ifdef CONFIG_SIMPILE_SHELL
	start_shell();
#endif

sleep: __attribute__((unused))
	while(1) {
		__WFI();
	}
	return 0;
}
