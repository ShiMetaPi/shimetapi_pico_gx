/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <math.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include "sample_comm.h"
#include "sample_comm_vo.h"
#include "xmedia_vo.h"
#include "xmedia_mmz.h"
#include "xmedia_tde.h"
#include "gfbg.h"

typedef struct{
    xmedia_u8 *mmap_vir;
    xmedia_u32 size;
}fb_mmap_info;

#define MIN(x,y) ((x) > (y) ? (y) : (x))
#define MAX(x,y) ((x) > (y) ? (x) : (y))

static const xmedia_char *g_image_names[] =
{
    "res/apple.bits",
    "res/applets.bits",
    "res/calendar.bits",
    "res/foot.bits",
    "res/gmush.bits",
    "res/gimp.bits",
    "res/gsame.bits",
    "res/keys.bits"
};
#define BACKGROUND_NAME  "res/background.bits"
#define N_IMAGES (xmedia_s32)((sizeof (g_image_names) / sizeof (g_image_names[0])))
#define SCREEN_WIDTH    720
#define SCREEN_HEIGHT   480
#define CYCLE_LEN       60

xmedia_s32 g_fb_fd = -1;
xmedia_s32 g_vo_dev = 0;
xmedia_s32 g_vo_layer = 0;
xmedia_s32 g_vo_chn[1] = {0};
xmedia_s32 g_vo_chn_num = 1;
xmedia_tde_surface_info *g_fb_background_surface = XMEDIA_NULL;
xmedia_tde_surface_info *g_fb_images[N_IMAGES] = {0};
fb_mmap_info g_fb_mmap_info = {0};
xmedia_bool g_tde_exit = XMEDIA_FALSE;

static xmedia_void *process_keyboard(xmedia_void *arg)
{
    xmedia_char ch;
    xmedia_bool *status = (xmedia_bool *) arg;

    printf("press q to quit\n");
    printf("press s to suspend and resume\n");
    while (1) {
        ch = getchar();
        if (ch == 's') {
            system("echo mem > /sys/power/state"); // 进入休眠状态 之后自动唤醒
        } else if (ch == 'q') {
            *status = XMEDIA_TRUE;
            break;
        } else {
            printf("press q to quit\n");
            printf("press s to suspend and resume\n");
        }
    }

    return XMEDIA_NULL;
}

static xmedia_void tde_destroy_surface(xmedia_tde_surface_info *sur_info)
{
    if (sur_info->surface.phys_addr[0] == 0) {
        return;
    }

    xmedia_mmz_free(sur_info->surface.phys_addr[0]);
    return;
}

void sample_tde_handle_sig(xmedia_s32 signo)
{
    if (signo == SIGINT || signo == SIGTERM) {

        // 处理资源泄露
        for (xmedia_s32 i = 0; i < N_IMAGES; i++) {
            if (g_fb_images[i] != XMEDIA_NULL) {
                tde_destroy_surface(g_fb_images[i]);
            }
        }

        if (g_fb_background_surface != XMEDIA_NULL) {
            tde_destroy_surface(g_fb_background_surface);
        }

        if(g_fb_mmap_info.mmap_vir != XMEDIA_NULL){
            munmap(g_fb_mmap_info.mmap_vir, g_fb_mmap_info.size);
            g_fb_mmap_info.mmap_vir = XMEDIA_NULL;
        }


        if (g_fb_fd != -1) {
            close(g_fb_fd);
        }

        xmedia_tde_exit();
        sample_comm_vo_stop(g_vo_dev, g_vo_layer, g_vo_chn, g_vo_chn_num);
        xmedia_vo_exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

static xmedia_void circumrotate (xmedia_tde_surface_info *bg, xmedia_tde_surface_info *image,
    xmedia_tde_surface_info *dst, xmedia_u32 cur_on_show)
{
    xmedia_s32 ret;
    xmedia_s32 handle;
    xmedia_tde_surface_info *dst_sur_tmp = (cur_on_show == 0) ? dst + 1 : dst;
    xmedia_tde_surface_info dst_sur = {0};
    static xmedia_s32 frame_num = 0;
    xmedia_float f;
    xmedia_float x_mid, y_mid;
    xmedia_float radius;
    xmedia_s32 i;
    xmedia_tde_operation_attr opt = {0};

    memcpy(&dst_sur, dst_sur_tmp, sizeof(xmedia_tde_surface_info));

    opt.blend_en = XMEDIA_TRUE;
    opt.blend_attr.pixel_alpha_en = XMEDIA_TRUE;
    opt.blend_attr.blend_mode = XMEDIA_TDE_BLEND_MODE_SRCOVER;

    ret = xmedia_tde_create_job(&handle);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_tde_create_job failed !\n");
        return;
    }

    bg->rect.x = 0;
    bg->rect.y = 0;
    bg->rect.width = SCREEN_WIDTH;
    bg->rect.height = SCREEN_HEIGHT;

    f = (float) (frame_num % CYCLE_LEN) / CYCLE_LEN;
    x_mid = bg->rect.width / 2.16f;
    y_mid = bg->rect.height / 2.304f;
    radius = MIN (x_mid, y_mid) / 2.0f;

    ret = xmedia_tde_add_task_single_blit(handle, bg, &dst_sur, 0, XMEDIA_NULL);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_tde_add_task_single_blit failed !\n");
        xmedia_tde_cancel_job(handle);
        return;
    }

    for(i = 0; i < N_IMAGES; i++) {
        xmedia_float ang;
        xmedia_float r;

        ang = 2.0f * (xmedia_float) M_PI * (xmedia_float) i / N_IMAGES - f * 2.0f * (xmedia_float) M_PI;
        r = radius + (radius / 3.0f) * sinf (f * 2.0 * M_PI);

        dst_sur.rect.x = x_mid + r * cosf (ang) - image[i].surface.width / 2.0f;
        dst_sur.rect.x = x_mid + r * cosf (ang) - image[i].surface.width / 2.0f;;
        dst_sur.rect.y = y_mid + r * sinf (ang) - image[i].surface.height / 2.0f;
        dst_sur.rect.width = image[i].surface.width;
        dst_sur.rect.height = image[i].surface.height;

        /* 4. bitblt image to screen */
        ret = xmedia_tde_add_task_double_blit(handle, &dst_sur, &image[i], &dst_sur, &opt);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("xmedia_tde_add_task_double_blit failed !\n");
            xmedia_tde_cancel_job(handle);
            return;
        }

    }

    ret = xmedia_tde_submit_job(handle);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_tde_submit_job failed !\n");
        xmedia_tde_cancel_job(handle);
        return;
    }

    frame_num++;
    return;
}

static xmedia_s32 tde_create_surface_by_file(const xmedia_char *file_name,
    xmedia_tde_surface_info *sur_info)
{
    FILE *fp;
    xmedia_u32 buffer_length = 0;
    xmedia_u8 *vir_addr;

    if (file_name == XMEDIA_NULL || sur_info == XMEDIA_NULL) {
        SAMPLE_PRT("NULL ptr!\n");
        return -1;
    }

    fp = fopen(file_name, "rb");
    if (fp == XMEDIA_NULL) {
        SAMPLE_PRT("fopen FILE:%s!\n", file_name);
        return -1;
    }

    fread(&sur_info->surface.pixel_format, 1, 4, fp);
    fread(&sur_info->surface.width, 1, 4, fp);
    fread(&sur_info->surface.height, 1, 4, fp);
    fread(&sur_info->surface.stride[0], 1, 4, fp);

    buffer_length = sur_info->surface.stride[0] * sur_info->surface.height;
    if (buffer_length > 0x7FFFFFFF) {
        SAMPLE_PRT("fopen FILE:%s!\n", file_name);
        fclose(fp);
        return -1;
    }

    // 分配mmz内存
    sur_info->surface.phys_addr[0] = xmedia_mmz_alloc(XMEDIA_NULL, XMEDIA_NULL, buffer_length);
    if (sur_info->surface.phys_addr[0] == 0) {
        SAMPLE_PRT("xmedia_mmz_alloc failed!\n");
        fclose(fp);
        return -1;
    }
    vir_addr = xmedia_mmz_map(sur_info->surface.phys_addr[0], buffer_length, XMEDIA_FALSE);
    if (vir_addr == 0) {
        SAMPLE_PRT("xmedia_mmz_map failed!\n");
        xmedia_mmz_free(sur_info->surface.phys_addr[0]);
        fclose(fp);
        return -1;
    }

    fread(vir_addr, 1, buffer_length, fp); // 图片是标准大小，自然对齐

    sur_info->surface.alpha0 = 0x0;
    sur_info->surface.alpha1 = 0xff;
    sur_info->surface.is_alpha_ext_1555 = XMEDIA_TRUE;

    sur_info->rect.x = 0;
    sur_info->rect.y = 0;
    sur_info->rect.width = sur_info->surface.width;
    sur_info->rect.height = sur_info->surface.height;

    xmedia_mmz_unmap((void *)vir_addr);
    fclose(fp);
    return XMEDIA_SUCCESS;
}

static xmedia_void tde_get_suface_by_fix_info(struct fb_fix_screeninfo *fix_info,
    xmedia_tde_surface_info *sur_info, xmedia_u32 index)
{
    sur_info->surface.alpha0 = 0x0;
    sur_info->surface.alpha1 = 0xff;
    sur_info->surface.clut_phys_addr = 0;
    sur_info->surface.clut_reload = XMEDIA_FALSE;
    sur_info->surface.is_alpha_ext_1555 = XMEDIA_TRUE;
    sur_info->surface.pixel_format = XMEDIA_VIDEO_PIXEL_FMT_ARGB_1555;
    sur_info->surface.width = SCREEN_WIDTH;
    sur_info->surface.height = SCREEN_HEIGHT;
    sur_info->surface.stride[0] = SCREEN_WIDTH * 2;
    sur_info->surface.phys_addr[0] = (index == 0) ? fix_info->smem_start : fix_info->smem_start + SCREEN_WIDTH * 2 *
        SCREEN_HEIGHT;
    sur_info->rect.x = 0;
    sur_info->rect.y = 0;
    sur_info->rect.width = sur_info->surface.width;
    sur_info->rect.height = sur_info->surface.height;

    return;
}

xmedia_s32 tde_draw_graphic_sample(xmedia_void)
{
    xmedia_s32 ret = 0;
    xmedia_s32 i, j;
    xmedia_u32 times = 0;
    xmedia_s32 fb_fd;
    gfbg_drv_alpha fb_alpha = {0};
    struct fb_var_screeninfo var_info = {0};
    struct fb_fix_screeninfo fix_info = {0};
    struct fb_bitfield red32 = {10, 5, 0};
    struct fb_bitfield green32 = {5, 5, 0};
    struct fb_bitfield blue32 = {0, 5, 0};
    struct fb_bitfield alpha32 = {15, 1, 0};
    xmedia_u8 *fb_mmap_vir = XMEDIA_NULL;
    xmedia_bool state = XMEDIA_TRUE;

    xmedia_tde_surface_info bg = {0};
    xmedia_tde_surface_info image[8] = {0};
    xmedia_tde_surface_info dst[2] = {0};

    fb_fd = open("/dev/fb0", O_RDWR);
    if (fb_fd < 0) {
        SAMPLE_PRT("open fb0 failed !\n");
        goto GRAPHIC_FAIL0;
    }
    g_fb_fd = fb_fd;

    ret = ioctl(fb_fd, GFBG_PUT_ALPHA, &fb_alpha);
    if (ret < 0) {
        SAMPLE_PRT("Put alpha info failed !\n");
        goto GRAPHIC_FAIL1;
    }

    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &var_info) < 0) {
        SAMPLE_PRT("Get variable screen info failed!\n");
        goto GRAPHIC_FAIL1;
    }

    var_info.xres_virtual = SCREEN_WIDTH;
    var_info.yres_virtual = SCREEN_HEIGHT * 2;
    var_info.xres = SCREEN_WIDTH;
    var_info.yres = SCREEN_HEIGHT;
    var_info.activate = FB_ACTIVATE_NOW;
    var_info.bits_per_pixel	= 16;
    var_info.xoffset = 0;
    var_info.yoffset = 0;
    var_info.red = red32;
    var_info.green = green32;
    var_info.blue = blue32;
    var_info.transp = alpha32;

    if (ioctl(fb_fd, FBIOPUT_VSCREENINFO, &var_info) < 0) {
        SAMPLE_PRT("FBIOPUT_VSCREENINFO error\n");
        goto GRAPHIC_FAIL1;
    }

    if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &fix_info) < 0) {
        SAMPLE_PRT("FBIOGET_FSCREENINFO error\n");
        goto GRAPHIC_FAIL1;
    }

    fb_mmap_vir = mmap(NULL, fix_info.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if (fb_mmap_vir == XMEDIA_NULL) {
        SAMPLE_PRT("mmap fb0 failed!\n");
        goto GRAPHIC_FAIL1;
    }
    memset(fb_mmap_vir, 0x00, fix_info.smem_len);
    g_fb_mmap_info.mmap_vir = fb_mmap_vir;
    g_fb_mmap_info.size = fix_info.smem_len;

    ret = tde_create_surface_by_file(BACKGROUND_NAME, &bg);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("create bg surface failed!\n");
        goto GRAPHIC_FAIL2;
    }
    g_fb_background_surface = &bg;

    for (i = 0; i < N_IMAGES; i++) {
        ret = tde_create_surface_by_file(g_image_names[i], &image[i]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("create image[%d] surface failed!\n", i);
            for (j = i; j > 0; j--) {
                tde_destroy_surface(&image[j - 1]);
            }
            goto GRAPHIC_FAIL3;
        }
        g_fb_images[i]= &image[i];
    }

    tde_get_suface_by_fix_info(&fix_info, &dst[0], 0);
    tde_get_suface_by_fix_info(&fix_info, &dst[1], 1);

    if (ioctl(fb_fd, GFBG_PUT_SHOW, &state) < 0) {
        SAMPLE_PRT("FBIOPUT_SHOW failed!\n");
        goto GRAPHIC_FAIL4;
    }

    do {
        circumrotate(&bg, image, dst, times % 2);
        var_info.yoffset = (times % 2) ? 0 : 480;

        if (ioctl(fb_fd, FBIOPAN_DISPLAY, &var_info) < 0) {
            SAMPLE_PRT("FBIOPAN_DISPLAY error\n");
            goto GRAPHIC_FAIL4;
        }

        times++;
        sleep(1);
    } while (g_tde_exit == XMEDIA_FALSE && times < 20);

    SAMPLE_PRT("exit\n");
    ret = XMEDIA_SUCCESS;
GRAPHIC_FAIL4:
    for (j = N_IMAGES; j > 0; j--) {
        tde_destroy_surface(&image[j - 1]);
    }
GRAPHIC_FAIL3:
    tde_destroy_surface(&bg);
GRAPHIC_FAIL2:
    munmap(fb_mmap_vir, fix_info.smem_len);
    fb_mmap_vir = XMEDIA_NULL;
GRAPHIC_FAIL1:
    close(fb_fd);
    fb_fd = -1;
GRAPHIC_FAIL0:
    return ret;
}

int main()
{
    xmedia_s32 ret;
    sample_vo_config_input vo_input = {0};
    sample_vo_config vo_config = {0};
    pthread_t t1;

    signal(SIGINT, sample_tde_handle_sig);
    signal(SIGTERM, sample_tde_handle_sig);

    ret = xmedia_vo_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vo init failed !\n");
        goto FAIL0;
    }

    vo_input.intf_sync = XMEDIA_VO_INTF_SYNC_1080P30;
    vo_input.intf_type = XMEDIA_INTF_TYPE_BT1120;
    vo_input.square_sort = 1;
    ret = sample_comm_vo_get_default_config(vo_input, &vo_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get vo default config failed !\n");
        goto FAIL1;
    }

    ret = sample_comm_vo_start(g_vo_dev, g_vo_layer, g_vo_chn, g_vo_chn_num, XMEDIA_FALSE, &vo_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vo start failed !\n");
        goto FAIL1;
    }

    ret = xmedia_tde_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("tde init failed !\n");
        goto FAIL2;
    }

    pthread_create(&t1, XMEDIA_NULL, process_keyboard, &g_tde_exit);
    pthread_detach(t1);

    ret = tde_draw_graphic_sample();
    if(ret != XMEDIA_SUCCESS) {
        goto FAIL3;
    }

    ret = XMEDIA_SUCCESS;
FAIL3:
    xmedia_tde_exit();
FAIL2:
    sample_comm_vo_stop(g_vo_dev, g_vo_layer, g_vo_chn, g_vo_chn_num);
FAIL1:
    xmedia_vo_exit();
FAIL0:
    return ret;
}
