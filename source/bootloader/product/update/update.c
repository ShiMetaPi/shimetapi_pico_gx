#include <common.h>

#if (CONFIG_AUTO_UPDATE == 1)
#include <linux/io.h>
#include "update_comm.h"
#include "auto_update_adapter.h"

#if defined(CONFIG_TARGET_RATELVXX)
static int check_update_button(void)
{
    /* to add some judgement if neccessary */
    unsigned int sd_update_flag;

    printf("board check_update_button bootflag:0x%x REG_ROM_RUN_TIME:%d\n",
            readl(REG_START_FLAG), readl(REG_ROM_RUN_TIME));

    /* sd update   */
    sd_update_flag = readl(REG_START_FLAG);
    if (SD_UPDATE_MODE(sd_update_flag))
        return UP_TRUE;

    /* usb device update  */
    if (SD_UDISK_UPDATE_CHECK_NEED(sd_update_flag))
        return UP_TRUE;    /* update enable */
    else
        return UP_FALSE;
}

static void clear_update_flag(void)
{
    /* REG_UPDATE_MODE_CLEAR: write only */
    writel(0, REG_START_FLAG);
}

#else /* CONFIG_TARGET_RATELVXX */

#define REG_SYS_BOOT6   0x0148
#define SD_UPDAT_MAGIC  0x444f574e

#define BIT_UPDATE_FLAG       2
#define BIT_UPDATE_FLAG_CLEAR 0
#define REG_SYS_STAT        0x008C
#define REG_MISC_CTRL51     0x8200

#ifndef REG_SC_DDRT14
#define REG_SC_DDRT14       0x00C8
#endif

#define REG_UPDATE_MODE       (SYS_CTRL_REG_BASE + REG_SYS_STAT)
#define REG_UPDATE_MODE_CLEAR (SYS_CTRL_REG_BASE + REG_MISC_CTRL51)
#define REG_BOOTSTRAP_FLAG    (SYS_CTRL_REG_BASE + REG_SC_DDRT14)

static int check_update_button(void)
{
	unsigned int sd_update_flag;
	unsigned int update_mode = 0;

	/* sd update   */
	sd_update_flag = readl(SYS_CTRL_REG_BASE + REG_SYS_BOOT6);
	if (sd_update_flag == SD_UPDAT_MAGIC) {
		printf("SD card upgrade\n");
		return UP_TRUE;
	}

	/* usb device update  */
	update_mode = readl(REG_UPDATE_MODE);
	update_mode = ((update_mode >> BIT_UPDATE_FLAG) & 0x01);
	if (update_mode) {
		printf("U disk upgrade\n");
		return UP_TRUE;
	} else {
		printf("Upgrade flag disabled\n");
		return UP_FALSE;
	}
}

static void clear_update_flag(void)
{
	unsigned int clear_value = 0x1;

	clear_value = (clear_value << BIT_UPDATE_FLAG_CLEAR);
	/* update mode clear */
	writel(clear_value, REG_UPDATE_MODE_CLEAR);
	/* set bootstrap flag */
	/* 0:主flash， 主uboot分区启动 */
	writel(0x00, REG_BOOTSTRAP_FLAG);
}
#endif /* CONFIG_TARGET_RATELVXX */

void auto_update(void)
{
	int update_flag = -1;

	printf("%s entry.\n", __func__);

	if (check_update_button()) {
		update_flag = do_auto_update();
		clear_update_flag();
		if (update_flag == 0) {
			printf("%s ok, reset.\n", __func__);
			do_reset(NULL, 0, 0, NULL);
		}
	}

	printf("%s exit.\n", __func__);
}

#endif // CONFIG_AUTO_UPDATE == 1
