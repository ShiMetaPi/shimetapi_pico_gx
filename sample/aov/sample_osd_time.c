#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/times.h>
#include <time.h>
#include <sys/time.h>
#include "sample_osd_time.h"

#ifdef SUPPORT_OSD_TIME

static xmedia_s32 g_rgn_hdl = RGN_HANDLE_MAX - 1;
static xmedia_s32 g_osd_init_flag = 0;
static misc_osd_time_attr_t g_osd_time_attr;
static xmedia_s32 g_misc_fd = -1;

xmedia_u32 custom_tick_get(void)
{
    static xmedia_u64 start_time = 0;
    xmedia_u64 now_time = 0;
    xmedia_u32 time_ms;
    struct timespec st_timespec_now;

    if(start_time == 0) {
        struct timespec st_timespec_start;
        clock_gettime(CLOCK_MONOTONIC, &st_timespec_start);
        start_time = st_timespec_start.tv_sec * 1000 + st_timespec_start.tv_nsec / 1000000;
        return 0;
    }

    clock_gettime(CLOCK_MONOTONIC, &st_timespec_now);
    now_time = st_timespec_now.tv_sec * 1000 + st_timespec_now.tv_nsec / 1000000;

    time_ms = now_time - start_time;
    return time_ms;
}

xmedia_s32 sample_osd_time_init()
{
    xmedia_rgn_attr region;

    if(g_osd_init_flag) {
        SAMPLE_ERR("The OSD has already been initialized and no need reinitialization!!!\n");
        return XMEDIA_SUCCESS;
    }

    g_misc_fd = sample_comm_aov_get_misc_fd();
    if(g_misc_fd < 0) {
        SAMPLE_ERR("Should open /dev/misc first!!!\n");
        return XMEDIA_FAILURE;
    }

    memset(&g_osd_time_attr, 0, sizeof(misc_osd_time_attr_t));
    region.type = XMEDIA_RGN_TYPE_OVERLAY;
    region.attr.overlay.pixel_format = XMEDIA_VIDEO_PIXEL_FMT_ARGB_1555;
    region.attr.overlay.size.width = RGN_DEFAULT_WIDTH;
    region.attr.overlay.size.height = RGN_DEFAULT_HEIGHT;
    region.attr.overlay.bg_color = 0xffff;
    region.attr.overlay.canvas_num = 2;

    g_osd_time_attr.rgn_hdl = g_rgn_hdl;
    memcpy(&g_osd_time_attr.rgn_attr, &region, sizeof(xmedia_rgn_attr));

    g_osd_time_attr.font_size_scale = 2;
    g_osd_time_attr.text_color = ARGB1555_YELLOW;
    g_osd_time_attr.bg_color = ARGB1555_DARKBLUE;

    g_osd_init_flag = 1;

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_osd_time_attach_to_chn(xmedia_chn_info *mpp_chn, xmedia_rgn_chn_attr *chn_attr)
{
    xmedia_s32 s32Ret = XMEDIA_SUCCESS;

    if(mpp_chn == XMEDIA_NULL || chn_attr == XMEDIA_NULL) {
        SAMPLE_ERR("invalid parameter!\n");
        return XMEDIA_FAILURE;
    }

    memcpy(&g_osd_time_attr.attach_obj[g_osd_time_attr.rgn_attach_num].mpp_chn, mpp_chn, sizeof(xmedia_chn_info));
    memcpy(&g_osd_time_attr.attach_obj[g_osd_time_attr.rgn_attach_num].chn_attr, chn_attr, sizeof(xmedia_rgn_chn_attr));
    g_osd_time_attr.rgn_attach_num++;

    return s32Ret;
}

xmedia_s32 sample_osd_time_start()
{
    xmedia_s32 tz_minuteswest = -480;  // TimeZone UTC+8
    xmedia_s32 s32Ret = XMEDIA_SUCCESS;

    SAMPLE_PRT("rgn_hdl:%d, font_size_scale:%d, text_color:0x%x, bg_color:0x%x, obj size:%d\n", 
        g_osd_time_attr.rgn_hdl, g_osd_time_attr.font_size_scale, g_osd_time_attr.text_color, g_osd_time_attr.bg_color, sizeof(misc_osd_time_attr_t));

    s32Ret = ioctl(g_misc_fd, XMEDIA_MISC_OSD_TIME_CREATE_CTRL, &g_osd_time_attr);
    if(s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("misc driver osd time create failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = ioctl(g_misc_fd, XMEDIA_MISC_OSD_TIME_UPDATE_TIMEZONE_CTRL, &tz_minuteswest);
    if(s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("misc driver osd time set timezone failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

xmedia_s32 sample_osd_time_destroy()
{
    if(g_misc_fd != -1) {
        ioctl(g_misc_fd, XMEDIA_MISC_OSD_TIME_DESTROY_CTRL, &g_osd_time_attr);
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_osd_time_update(xmedia_u32 time_interval, sample_aov_work_mode work_mode) {
    static xmedia_u32 record_cnt = 0;
    static xmedia_u32 last_update_time = 0, next_update_time = 0;
    xmedia_u32 cur_time = 0;
    struct timeval tv_start;

    if(record_cnt == 0) {
        goto DO_UPDATE_OSD;
    }
    else if(work_mode == MEDIA_WORK_NORMAL && (record_cnt * time_interval *3/2 >= next_update_time)) {
        cur_time = custom_tick_get();
        if((cur_time - last_update_time)*1000 >= next_update_time) {
            goto DO_UPDATE_OSD;
        }
        else {
            record_cnt++;
        }
    }
    else {
        record_cnt++;
    }

    return XMEDIA_SUCCESS;

DO_UPDATE_OSD:
    if(g_misc_fd != -1) {
        gettimeofday(&tv_start, NULL);
        //SAMPLE_PRT("cur time:%ums, delta time:%ums, tv_sec:%lu, tv_usec:%lu\n", cur_time, cur_time - last_update_time, tv_start.tv_sec, tv_start.tv_usec);

        ioctl(g_misc_fd, XMEDIA_MISC_OSD_TIME_UPDATE_CTRL, &g_osd_time_attr);
        last_update_time = cur_time;
        next_update_time = 1000000 - tv_start.tv_usec;
        record_cnt = 1;
    }
    return XMEDIA_SUCCESS;
}

#endif  //SUPPORT_OSD_TIME

