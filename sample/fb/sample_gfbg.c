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

#include "sample_comm.h"
#include "sample_comm_vo.h"
#include "xmedia_vo.h"
#include "xmedia_mmz.h"
#include "xmedia_tde.h"
#include "gfbg.h"

#define SAMPLE_GFBG_PATH_SRC "./res/"

static const xmedia_char *sample_gfbg_tde_image_names[] =
{
    SAMPLE_GFBG_PATH_SRC"apple.bits",
    SAMPLE_GFBG_PATH_SRC"applets.bits",
    SAMPLE_GFBG_PATH_SRC"calendar.bits",
    SAMPLE_GFBG_PATH_SRC"foot.bits",
    SAMPLE_GFBG_PATH_SRC"gmush.bits",
    SAMPLE_GFBG_PATH_SRC"gimp.bits",
    SAMPLE_GFBG_PATH_SRC"gsame.bits",
    SAMPLE_GFBG_PATH_SRC"keys.bits"
};
#define BACKGROUND_NAME  "res/background.bits"
#define SAMPLE_GFBG_SRC_IMAGES (xmedia_s32)((sizeof (sample_gfbg_tde_image_names) / sizeof (sample_gfbg_tde_image_names[0])))

typedef struct {
    struct fb_bitfield red32;
    struct fb_bitfield green32;
    struct fb_bitfield blue32;
    struct fb_bitfield alpha32;
}sample_gfbg_bitfield;

typedef struct {
    gfbg_layer_id layer_id;
    xmedia_u32 width;
    xmedia_u32 height;
    sample_gfbg_bitfield bitfield;
    xmedia_video_pixel_format pixel_fmt;
    xmedia_u32 bits_per_pixel;
    gfbg_drv_alpha gfbg_alpha;
    struct fb_var_screeninfo var_info;
    struct fb_fix_screeninfo fix_info;
    xmedia_u8 *gfbg_mmap_vir;
    xmedia_bool stop_flag;
    xmedia_s32 thread_sleep_time;
    pthread_t tid;
}sample_gfbg_config;

typedef struct {
    sample_vo_config vo_config[VO_MAX_DEV_NUM];
    xmedia_s32 vo_dev_num;
    xmedia_s32 vo_chn_num[VO_MAX_DEV_NUM];
    xmedia_s32 vo_chn[VO_MAX_DEV_NUM][VO_MAX_CHN_NUM];

    xmedia_s32 gfbg_layer_num;
    sample_gfbg_config gfbg_config[GFBG_LAYER_ID_MAX];
    sample_comm_screen_type screen[VO_MAX_DEV_NUM];
} sample_gfbg_all_mod_cfg;

#define SAMPLE_GFBG_ARGB8888TOARGB1555(color) (((color & 0x80000000) >> 16) | ((color & 0xf80000) >> 9) | ((color & 0xf800) >> 6) | ((color & 0xf8) >> 3))

static xmedia_void sample_gfbg_circum_rotate (xmedia_tde_surface_info *ui_buffer_sf, xmedia_tde_surface_info *image,
    xmedia_tde_surface_info *dst, xmedia_u32 cur_on_show)
{
    xmedia_s32 ret;
    xmedia_s32 handle;
    xmedia_tde_surface_info *dst_sur_tmp = (cur_on_show == 0) ? dst + 1 : dst;
    xmedia_tde_surface_info dst_sur = {0};
    static xmedia_s32 frame_num = 0;
    xmedia_s32 i;
    xmedia_tde_operation_attr opt = {0};
    xmedia_tde_line_attr line[2];
    xmedia_video_rect ui_rect_back;

    memcpy(&dst_sur, dst_sur_tmp, sizeof(xmedia_tde_surface_info));

    opt.blend_en = XMEDIA_TRUE;
    opt.blend_attr.pixel_alpha_en = XMEDIA_TRUE;
    opt.blend_attr.blend_mode = XMEDIA_TDE_BLEND_MODE_SRCOVER;

    ret = xmedia_tde_create_job(&handle);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_tde_create_job failed !\n");
        return;
    }

    //清fb 缓存 任务
    ret = xmedia_tde_add_task_quick_fill(handle, &dst_sur, 0);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_tde_add_task_quick_fill failed !\n");
        xmedia_tde_cancel_job(handle);
        return;
    }

    ui_rect_back.x = ui_buffer_sf->rect.x;
    ui_rect_back.y = ui_buffer_sf->rect.y;
    ui_rect_back.width = ui_buffer_sf->rect.width;
    ui_rect_back.height = ui_buffer_sf->rect.height;

    //清ui 缓存 任务
    ret = xmedia_tde_add_task_quick_fill(handle, ui_buffer_sf, 0);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_tde_add_task_quick_fill failed !\n");
        xmedia_tde_cancel_job(handle);
        return;
    }

    //绘制不同的ui buffer 任务
    for(i = 0; i < (SAMPLE_GFBG_SRC_IMAGES); i++) {
        ui_buffer_sf->rect.x = ((i + frame_num) % (SAMPLE_GFBG_SRC_IMAGES)) * image[i].surface.width;
        ui_buffer_sf->rect.y = ((i + frame_num) % (SAMPLE_GFBG_SRC_IMAGES)) * image[i].surface.height;
        ui_buffer_sf->rect.width = image[i].surface.width;
        ui_buffer_sf->rect.height = image[i].surface.height;

        //绘制内容超出屏幕范围则跳过
        if (((ui_buffer_sf->rect.x + ui_buffer_sf->rect.width) > ui_buffer_sf->surface.width) ||
            ((ui_buffer_sf->rect.y + ui_buffer_sf->rect.height) > ui_buffer_sf->surface.height)) {
            continue;
        }
        /* 4. bitblt image to screen */
        ret = xmedia_tde_add_task_double_blit(handle, ui_buffer_sf, &image[i], ui_buffer_sf, &opt);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("xmedia_tde_add_task_double_blit failed !\n");
            xmedia_tde_cancel_job(handle);
            return;
        }
    }

    //将ui_sf 叠加到fb dst_sur上 任务
    ui_buffer_sf->rect.x = ui_rect_back.x;
    ui_buffer_sf->rect.y = ui_rect_back.y;
    ui_buffer_sf->rect.width = ui_rect_back.width;
    ui_buffer_sf->rect.height = ui_rect_back.height;

    ret = xmedia_tde_add_task_double_blit(handle, &dst_sur, ui_buffer_sf, &dst_sur, &opt);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_tde_add_task_double_blit failed !\n");
        xmedia_tde_cancel_job(handle);
        return;
    }

    //提交tde工作
    ret = xmedia_tde_submit_job(handle);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_tde_submit_job failed !\n");
        xmedia_tde_cancel_job(handle);
        return;
    }

    //等待tde完成 再去送显，否则可能出现闪屏或撕裂
    ret = xmedia_tde_wait_job(handle, 2000);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_tde_wait_job failed !\n");
    }

    //画线任务需要单独放在一个工作里，不能和其他类型任务在工作里共存
    ret = xmedia_tde_create_job(&handle);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_tde_create_job failed !\n");
        return;
    }

    line[0].start_point.x = dst_sur.rect.width / 2;
    line[0].start_point.y = 0;
    line[0].end_point.x = dst_sur.rect.width / 2;
    line[0].end_point.y = dst_sur.rect.height;
    line[0].color = 0xffff0000;
    line[0].thick = 4;
    line[1].start_point.x = 0;
    line[1].start_point.y = dst_sur.rect.height / 2;
    line[1].end_point.x = dst_sur.rect.width;
    line[1].end_point.y = dst_sur.rect.height / 2;
    line[1].color = 0xff00ff00;
    line[1].thick = 4;
    ret = xmedia_tde_add_task_draw_line(handle, &dst_sur, line, 2);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_tde_add_task_draw_line failed !\n");
        xmedia_tde_cancel_job(handle);
        return;
    }

    //提交tde工作
    ret = xmedia_tde_submit_job(handle);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_tde_submit_job failed !\n");
        xmedia_tde_cancel_job(handle);
        return;
    }

    //等待tde完成 再去送显，否则可能出现闪屏或撕裂
    ret = xmedia_tde_wait_job(handle, 2000);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_tde_wait_job failed !\n");
    }
    frame_num++;
    return;
}

static xmedia_s32 sample_gfbg_create_tde_surface(xmedia_u32 w, xmedia_u32 h, xmedia_u32 stride,
    xmedia_video_pixel_format pix_fmt, xmedia_tde_surface_info *ui_buffer_sf)
{
    xmedia_u32 buffer_length = 0;
    xmedia_u8 *vir_addr;

    buffer_length = stride * h;
    ui_buffer_sf->surface.phys_addr[0] = xmedia_mmz_alloc(XMEDIA_NULL, XMEDIA_NULL, buffer_length);
    if (ui_buffer_sf->surface.phys_addr[0] == 0) {
        SAMPLE_PRT("xmedia_mmz_alloc failed!\n");
        return XMEDIA_FAILURE;
    }
    vir_addr = xmedia_mmz_map(ui_buffer_sf->surface.phys_addr[0], buffer_length, XMEDIA_FALSE);
    if (vir_addr == 0) {
        SAMPLE_PRT("xmedia_mmz_map failed!\n");
        xmedia_mmz_free(ui_buffer_sf->surface.phys_addr[0]);
        return XMEDIA_FAILURE;
    }
    ui_buffer_sf->surface.alpha0 = 0x0;
    ui_buffer_sf->surface.alpha1 = 0xff;
    ui_buffer_sf->surface.is_alpha_ext_1555 = XMEDIA_TRUE;
    ui_buffer_sf->surface.width = w;
    ui_buffer_sf->surface.height = h;
    ui_buffer_sf->surface.stride[0] = stride;

    ui_buffer_sf->rect.x = 0;
    ui_buffer_sf->rect.y = 0;
    ui_buffer_sf->rect.width = w;
    ui_buffer_sf->rect.height = h;
    ui_buffer_sf->surface.pixel_format = pix_fmt;

    memset(vir_addr, 0, buffer_length);
    xmedia_mmz_unmap((void *)vir_addr);
    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_gfbg_create_tde_surface_by_file(const xmedia_char *file_name,
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

static xmedia_void sample_gfbg_destroy_tde_surface(xmedia_tde_surface_info *sur_info)
{
    xmedia_mmz_free(sur_info->surface.phys_addr[0]);
    return;
}

static xmedia_void sample_gfbg_get_tde_suface_by_gfbg_info(sample_gfbg_config *gfbg_config,
    xmedia_tde_surface_info *sur_info, xmedia_u32 index)
{
    sur_info->surface.alpha0 = 0x0;
    sur_info->surface.alpha1 = 0xff;
    sur_info->surface.clut_phys_addr = 0;
    sur_info->surface.clut_reload = XMEDIA_FALSE;
    sur_info->surface.is_alpha_ext_1555 = XMEDIA_TRUE;
    sur_info->surface.pixel_format = XMEDIA_VIDEO_PIXEL_FMT_ARGB_1555;
    sur_info->surface.width = gfbg_config->var_info.xres;
    sur_info->surface.height = gfbg_config->var_info.yres;
    sur_info->surface.stride[0] = gfbg_config->var_info.xres * 2;
    sur_info->surface.phys_addr[0] = (index == 0) ? gfbg_config->fix_info.smem_start : gfbg_config->fix_info.smem_start + gfbg_config->var_info.xres * 2 *
        gfbg_config->var_info.yres;
    sur_info->rect.x = 0;
    sur_info->rect.y = 0;
    sur_info->rect.width = sur_info->surface.width;
    sur_info->rect.height = sur_info->surface.height;

    return;
}

xmedia_void* sample_gfbg_use_tde_draw_graphic(xmedia_void* args)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 i, j;
    xmedia_u64 times;
    xmedia_s32 gfbg_fd;
    xmedia_bool state = XMEDIA_TRUE;
    xmedia_tde_surface_info ui_buffer_sf = {0};
    xmedia_tde_surface_info image[8] = {0};
    xmedia_tde_surface_info dst[2] = {0};
    sample_gfbg_config *gfbg_config;

    gfbg_config = (sample_gfbg_config*)args;
    if(gfbg_config == XMEDIA_NULL) {
        SAMPLE_PRT("args is null !\n");
        return XMEDIA_NULL;
    }

    if(gfbg_config->layer_id == 0){
        gfbg_fd = open("/dev/fb0", O_RDWR);
    } else {
        gfbg_fd = open("/dev/fb1", O_RDWR);
    }
    if (gfbg_fd < 0) {
        SAMPLE_PRT("open fb0 failed !\n");
        goto OUT0;
    } else {
        SAMPLE_PRT("open fb0 success !\n");
    }

    ret = ioctl(gfbg_fd, GFBG_PUT_ALPHA, &gfbg_config->gfbg_alpha);
    if (ret < 0) {
        SAMPLE_PRT("GFBG_PUT_ALPHA failed !\n");
        goto OUT1;
    }
    if (ioctl(gfbg_fd, FBIOGET_VSCREENINFO, &gfbg_config->var_info) < 0) {
        SAMPLE_PRT("Get variable screen info failed!\n");
        goto OUT1;
    }
    gfbg_config->var_info.xres_virtual = gfbg_config->width;
    gfbg_config->var_info.yres_virtual = gfbg_config->height * 2;
    gfbg_config->var_info.xres = gfbg_config->width;
    gfbg_config->var_info.yres = gfbg_config->height;
    gfbg_config->var_info.activate = FB_ACTIVATE_NOW;
    gfbg_config->var_info.bits_per_pixel = gfbg_config->bits_per_pixel;
    gfbg_config->var_info.xoffset = 0;
    gfbg_config->var_info.yoffset = 0;
    gfbg_config->var_info.red = gfbg_config->bitfield.red32;
    gfbg_config->var_info.green = gfbg_config->bitfield.green32;
    gfbg_config->var_info.blue = gfbg_config->bitfield.blue32;
    gfbg_config->var_info.transp = gfbg_config->bitfield.alpha32;

    if (ioctl(gfbg_fd, FBIOPUT_VSCREENINFO, &gfbg_config->var_info) < 0) {
        SAMPLE_PRT("FBIOPUT_VSCREENINFO error\n");
        goto OUT1;
    }
    if (ioctl(gfbg_fd, FBIOGET_FSCREENINFO, &gfbg_config->fix_info) < 0) {
        SAMPLE_PRT("FBIOGET_FSCREENINFO error\n");
        goto OUT1;
    }
    gfbg_config->gfbg_mmap_vir = mmap(NULL, gfbg_config->fix_info.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, gfbg_fd, 0);
    if (gfbg_config->gfbg_mmap_vir == XMEDIA_NULL) {
        SAMPLE_PRT("mmap fb0 failed!\n");
        goto OUT1;
    }
    memset(gfbg_config->gfbg_mmap_vir, 0x00, gfbg_config->fix_info.smem_len);

    //创建ui画布 得到ui_buffer_sf
    ret = sample_gfbg_create_tde_surface(gfbg_config->width, gfbg_config->height,
        gfbg_config->width * gfbg_config->bits_per_pixel,
        gfbg_config->pixel_fmt, &ui_buffer_sf);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_gfbg_create_tde_surface failed!\n");
        goto OUT2;
    }

    //将源文件读到tde_surface_info，得到image[]
    for (i = 0; i < SAMPLE_GFBG_SRC_IMAGES; i++) {
        ret = sample_gfbg_create_tde_surface_by_file(sample_gfbg_tde_image_names[i], &image[i]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("create image[%d] surface failed!\n", i);
            for (j = i; j > 0; j--) {
                sample_gfbg_destroy_tde_surface(&image[j - 1]);
            }
            goto OUT3;
        }
    }

    //将gfbg内存分成两块 填充到tde_surface_info 得到dst[]
    sample_gfbg_get_tde_suface_by_gfbg_info(gfbg_config, &dst[0], 0);
    sample_gfbg_get_tde_suface_by_gfbg_info(gfbg_config, &dst[1], 1);

    if (ioctl(gfbg_fd, GFBG_PUT_SHOW, &state) < 0) {
        SAMPLE_PRT("GFBG_PUT_SHOW failed!\n");
        goto OUT4;
    }

    //循环添加tde任务，将image[]叠加到ui_buffer_sf，得到新的ui_buffer_sf，再和dst[]做叠加送显
    for (times = 0;; times++) {
        //叠加不同到ui画面到buffer0 或者 buffer1上,image种有一张图片alpha值为0，所以显示为透明
        sample_gfbg_circum_rotate(&ui_buffer_sf, image, dst, times % 2);

        //送显buffer0 或者 buffer1
        gfbg_config->var_info.yoffset = (times % 2)? 0 : gfbg_config->height;

        if (ioctl(gfbg_fd, FBIOPAN_DISPLAY, &gfbg_config->var_info) < 0) {
            SAMPLE_PRT("FBIOPAN_DISPLAY error\n");
            goto OUT4;
        }
        //等待送显后vsync 再进行另一块buffer绘制
        if (ioctl(gfbg_fd, GFBG_GET_VBLANK, XMEDIA_NULL) < 0) {
            SAMPLE_PRT("GFBG_GET_VBLANK_GFBG error\n");
            goto OUT4;
        }
        if(gfbg_config->thread_sleep_time !=0) {
            usleep(gfbg_config->thread_sleep_time*1000);
        } else {
            usleep(100*1000);
        }
        if(gfbg_config->stop_flag == XMEDIA_TRUE) {
            break;
        }
    }

OUT4:
    for (j = SAMPLE_GFBG_SRC_IMAGES; j > 0; j--) {
        sample_gfbg_destroy_tde_surface(&image[j - 1]);
    }
OUT3:
    sample_gfbg_destroy_tde_surface(&ui_buffer_sf);
OUT2:
    munmap(gfbg_config->gfbg_mmap_vir, gfbg_config->fix_info.smem_len);
    gfbg_config->gfbg_mmap_vir = XMEDIA_NULL;
OUT1:
    close(gfbg_fd);
    gfbg_fd = -1;
OUT0:
    return XMEDIA_NULL;
}

xmedia_void sample_gfbg_handle_sig(xmedia_s32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo || SIGTERM == signo) {
        xmedia_vb_exit();
        xmedia_sys_exit();
        SAMPLE_PRT("\033[0;31mprogram exit abnormally!\033[0;39m\n");
    }
    exit(0);
}

static xmedia_s32 sample_gfbg_get_default_config(sample_gfbg_all_mod_cfg *all_mod_cfg,xmedia_u32 screen_num)
{
    xmedia_u32 layer = 0;
    xmedia_u32 freq, width, height;

    for(layer = 0 ;layer < screen_num;layer++){
        all_mod_cfg->gfbg_config[layer].bitfield.red32.offset = 10;
        all_mod_cfg->gfbg_config[layer].bitfield.red32.length = 5;
        all_mod_cfg->gfbg_config[layer].bitfield.red32.msb_right = 0;

        all_mod_cfg->gfbg_config[layer].bitfield.green32.offset = 5;
        all_mod_cfg->gfbg_config[layer].bitfield.green32.length = 5;
        all_mod_cfg->gfbg_config[layer].bitfield.green32.msb_right = 0;

        all_mod_cfg->gfbg_config[layer].bitfield.blue32.offset = 0;
        all_mod_cfg->gfbg_config[layer].bitfield.blue32.length = 5;
        all_mod_cfg->gfbg_config[layer].bitfield.blue32.msb_right = 0;

        all_mod_cfg->gfbg_config[layer].bitfield.alpha32.offset = 15;
        all_mod_cfg->gfbg_config[layer].bitfield.alpha32.length = 1;
        all_mod_cfg->gfbg_config[layer].bitfield.alpha32.msb_right = 0;

        all_mod_cfg->gfbg_config[layer].pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_ARGB_1555;
        all_mod_cfg->gfbg_config[layer].bits_per_pixel = 16;
        all_mod_cfg->gfbg_config[layer].gfbg_alpha.alpha0 = 0x0;
        all_mod_cfg->gfbg_config[layer].gfbg_alpha.alpha1 = 0xff;
        all_mod_cfg->gfbg_config[layer].gfbg_alpha.global_alpha = XMEDIA_TRUE;
        all_mod_cfg->gfbg_config[layer].gfbg_alpha.global_alpha_value = 0xff;
        all_mod_cfg->gfbg_config[layer].gfbg_alpha.pixel_alpha = XMEDIA_TRUE;
        all_mod_cfg->gfbg_config[layer].layer_id = layer;

        if (sample_comm_vo_get_dev_config(all_mod_cfg->screen[layer], &all_mod_cfg->vo_config[0], &width, &height, &freq) != XMEDIA_SUCCESS) {
            SAMPLE_PRT("%s all_mod_cfg->screen[%d]: %d, can not get dev config information!\n", __func__, layer, all_mod_cfg->screen[layer]);
            return XMEDIA_FAILURE;
        }
        all_mod_cfg->gfbg_config[layer].width = width;
        all_mod_cfg->gfbg_config[layer].height = height;
    }
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_gfbg_stop_send_thread(sample_gfbg_config *gfbg_cfg)
{
    if (gfbg_cfg == XMEDIA_NULL) {
        return XMEDIA_FAILURE;
    }

    gfbg_cfg->stop_flag = XMEDIA_TRUE;
    pthread_join(gfbg_cfg->tid, XMEDIA_NULL);
    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_gfbg_tde_test(sample_comm_screen_type screen0, xmedia_s32 sleep_ms)
{
    xmedia_s32 ret;
    sample_vo_config_input vo_input = { 0 };
    sample_gfbg_all_mod_cfg all_mod_cfg;
    xmedia_char c;
    memset(&all_mod_cfg, 0, sizeof(sample_gfbg_all_mod_cfg));
    all_mod_cfg.vo_dev_num = 1;
    all_mod_cfg.vo_chn_num[0] = 1;
    all_mod_cfg.gfbg_layer_num = all_mod_cfg.vo_dev_num;
    all_mod_cfg.gfbg_config[0].tid = 0;
    all_mod_cfg.gfbg_config[0].thread_sleep_time = sleep_ms;
    all_mod_cfg.screen[0] = screen0;

    vo_input.square_sort = all_mod_cfg.vo_chn_num[0];
    vo_input.screen_type = all_mod_cfg.screen[0];
    ret = sample_comm_vo_get_default_config(vo_input, &all_mod_cfg.vo_config[0]);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vo_get_default_config failed !\n");
        return ret;
    }
    ret = sample_comm_vo_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vo init fail!\n");
        goto OUT1;
    }
    ret = sample_comm_vo_start(0, 0, all_mod_cfg.vo_chn[0], all_mod_cfg.vo_chn_num[0], XMEDIA_FALSE, &all_mod_cfg.vo_config[0]);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vo start failed !\n");
        goto OUT2;
    }
    ret = xmedia_tde_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("tde init failed !\n");
        goto OUT3;
    }

    ret = sample_gfbg_get_default_config(&all_mod_cfg, all_mod_cfg.vo_dev_num);
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_gfbg_get_default_config failed !\n");
        goto OUT4;
    }

    pthread_create(&(all_mod_cfg.gfbg_config[0].tid), 0, sample_gfbg_use_tde_draw_graphic, (xmedia_void *)&all_mod_cfg.gfbg_config[0]);

    while(1) {
        SAMPLE_PRT("\nSAMPLE_TEST:press 'e' to exit; !\n");
        c = getchar();
        if (c == 'e') {
            break;
        }
    }
    sample_gfbg_stop_send_thread((xmedia_void *)&all_mod_cfg.gfbg_config[0]);

OUT4:
    xmedia_tde_exit();
OUT3:
    sample_comm_vo_stop(0, 0, all_mod_cfg.vo_chn[0], all_mod_cfg.vo_chn_num[0]);
OUT2:
    sample_comm_vo_exit();
OUT1:
    return ret;
}

static void sample_gfbg_draw_box(xmedia_video_rect *pstRect, xmedia_u32 stride,
             xmedia_u8* pmapped_mem, xmedia_u32 color, xmedia_u32 u32Bpp)
{
    xmedia_u8 *pMem;
    xmedia_s32 column,row;
    xmedia_u32 u32Color = color;

    if (u32Bpp != 4 && u32Bpp != 2)
    {
        SAMPLE_PRT("sample_gfbg_draw_box just support pixelformat ARGB1555&ARGB8888");
        return;
    }

    if (2 == u32Bpp)
    {
        u32Color = SAMPLE_GFBG_ARGB8888TOARGB1555(color);
    }

    for (column = pstRect->y;column < (pstRect->y+pstRect->height);column++)
    {
        pMem =     pmapped_mem + column*stride;
        for (row = pstRect->x;row < (pstRect->x+pstRect->width);row++)
        {
            if (2 == u32Bpp)
            {
                *(xmedia_u16*)(pMem + row*u32Bpp) = (xmedia_u16)u32Color;
                //usleep(1000*50);
                //printf("x = %d y = %d \n",row,column);
            }
            else if (4 == u32Bpp)
            {
                *(xmedia_u32*)(pMem + row*u32Bpp) = u32Color;
                //usleep(1000*50);
                //printf("x = %d y = %d \n",row,column);
            }
        }
    }
}

 xmedia_void sample_gfbg_draw_box_offset(sample_gfbg_config *gfbg_config,xmedia_video_rect rect,
    xmedia_s32 color,xmedia_s32 bufferx)
 {
     xmedia_u8* mem_addr = (xmedia_u8*)gfbg_config->gfbg_mmap_vir +
        bufferx * gfbg_config->fix_info.line_length * gfbg_config->var_info.yres;

     sample_gfbg_draw_box(&rect, gfbg_config->fix_info.line_length, mem_addr, color, gfbg_config->bits_per_pixel / 8);
     return;
 }

xmedia_void* sample_gfbg_test_thread(xmedia_void* args)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_u64 times;
    xmedia_s32 gfbg_fd;
    xmedia_bool state = XMEDIA_TRUE;
    sample_gfbg_config *gfbg_config;
    xmedia_video_rect draw_rect[4];

    gfbg_config = (sample_gfbg_config*)args;
    if(gfbg_config == XMEDIA_NULL) {
        SAMPLE_PRT("args is null !\n");
        return XMEDIA_NULL;
    }

    if(gfbg_config->layer_id == 0){
        gfbg_fd = open("/dev/fb0", O_RDWR);
    } else {
        gfbg_fd = open("/dev/fb1", O_RDWR);
    }
    if (gfbg_fd < 0) {
        SAMPLE_PRT("open fb0 failed !\n");
        goto OUT0;
    } else {
        SAMPLE_PRT("open fb0 success !\n");
    }

    ret = ioctl(gfbg_fd, GFBG_PUT_ALPHA, &gfbg_config->gfbg_alpha);
    if (ret < 0) {
        SAMPLE_PRT("GFBG_PUT_ALPHA failed !\n");
        goto OUT1;
    }
    if (ioctl(gfbg_fd, FBIOGET_VSCREENINFO, &gfbg_config->var_info) < 0) {
        SAMPLE_PRT("Get variable screen info failed!\n");
        goto OUT1;
    }
    gfbg_config->var_info.xres_virtual = gfbg_config->width;
    gfbg_config->var_info.yres_virtual = gfbg_config->height * 2;
    gfbg_config->var_info.xres = gfbg_config->width;
    gfbg_config->var_info.yres = gfbg_config->height;
    gfbg_config->var_info.activate = FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE;
    gfbg_config->var_info.bits_per_pixel = gfbg_config->bits_per_pixel;
    gfbg_config->var_info.xoffset = 0;
    gfbg_config->var_info.yoffset = 0;
    gfbg_config->var_info.red = gfbg_config->bitfield.red32;
    gfbg_config->var_info.green = gfbg_config->bitfield.green32;
    gfbg_config->var_info.blue = gfbg_config->bitfield.blue32;
    gfbg_config->var_info.transp = gfbg_config->bitfield.alpha32;

    if (ioctl(gfbg_fd, FBIOPUT_VSCREENINFO, &gfbg_config->var_info) < 0) {
        SAMPLE_PRT("FBIOPUT_VSCREENINFO error\n");
        goto OUT1;
    }
    if (ioctl(gfbg_fd, FBIOGET_FSCREENINFO, &gfbg_config->fix_info) < 0) {
        SAMPLE_PRT("FBIOGET_FSCREENINFO error\n");
        goto OUT1;
    }
    gfbg_config->gfbg_mmap_vir = mmap(NULL, gfbg_config->fix_info.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, gfbg_fd, 0);
    if (gfbg_config->gfbg_mmap_vir == XMEDIA_NULL) {
        SAMPLE_PRT("mmap fb0 failed!\n");
        goto OUT1;
    }
    memset(gfbg_config->gfbg_mmap_vir, 0x00, gfbg_config->fix_info.smem_len);

    if (ioctl(gfbg_fd, GFBG_PUT_SHOW, &state) < 0) {
        SAMPLE_PRT("GFBG_PUT_SHOW failed!\n");
        goto OUT2;
    }

    draw_rect[0].x = 0;
    draw_rect[0].y = 0;
    draw_rect[0].width = gfbg_config->width / 10;
    draw_rect[0].height = gfbg_config->height / 10;

    draw_rect[1].x = gfbg_config->width - gfbg_config->width / 10;
    draw_rect[1].y = 0;
    draw_rect[1].width = gfbg_config->width / 10;
    draw_rect[1].height = gfbg_config->height / 10;

    draw_rect[2].x = 0;
    draw_rect[2].y = gfbg_config->height - gfbg_config->height / 10;
    draw_rect[2].width = gfbg_config->width / 10;
    draw_rect[2].height = gfbg_config->height / 10;

    draw_rect[3].x = gfbg_config->width - gfbg_config->width / 10;
    draw_rect[3].y = gfbg_config->height - gfbg_config->height / 10;
    draw_rect[3].width = gfbg_config->width / 10;
    draw_rect[3].height = gfbg_config->height / 10;

    for (times = 0;; times++) {
        sample_gfbg_draw_box_offset(gfbg_config, draw_rect[0], 0xffff0000, times % 2);
        sample_gfbg_draw_box_offset(gfbg_config, draw_rect[1], 0xff00ff00, times % 2);
        sample_gfbg_draw_box_offset(gfbg_config, draw_rect[2], 0xff0000ff, times % 2);
        sample_gfbg_draw_box_offset(gfbg_config, draw_rect[3], 0xffff00ff, times % 2);

        //送显buffer0 或者 buffer1
        gfbg_config->var_info.yoffset = (times % 2)? 0 : gfbg_config->height;

        if (ioctl(gfbg_fd, FBIOPAN_DISPLAY, &gfbg_config->var_info) < 0) {
            SAMPLE_PRT("FBIOPAN_DISPLAY error\n");
            goto OUT2;
        }
        //等待送显后vsync 再进行另一块buffer绘制
        if (ioctl(gfbg_fd, GFBG_GET_VBLANK, XMEDIA_NULL) < 0) {
            SAMPLE_PRT("GFBG_GET_VBLANK_GFBG error\n");
            goto OUT2;
        }
        if(gfbg_config->thread_sleep_time !=0) {
            usleep(gfbg_config->thread_sleep_time*1000);
        } else {
            usleep(100*1000);
        }
        if(gfbg_config->stop_flag == XMEDIA_TRUE) {
            break;
        }
    }

OUT2:
    munmap(gfbg_config->gfbg_mmap_vir, gfbg_config->fix_info.smem_len);
    gfbg_config->gfbg_mmap_vir = XMEDIA_NULL;
OUT1:
    close(gfbg_fd);
    gfbg_fd = -1;
OUT0:
    return XMEDIA_NULL;
}

static xmedia_s32 sample_gfbg_test(sample_comm_screen_type screen0, xmedia_s32 sleep_ms)
{
    xmedia_s32 ret;
    sample_vo_config_input vo_input = { 0 };
    sample_gfbg_all_mod_cfg all_mod_cfg;
    xmedia_char c;
    memset(&all_mod_cfg, 0, sizeof(sample_gfbg_all_mod_cfg));
    all_mod_cfg.vo_dev_num = 1;
    all_mod_cfg.vo_chn_num[0] = 1;
    all_mod_cfg.gfbg_layer_num = all_mod_cfg.vo_dev_num;
    all_mod_cfg.gfbg_config[0].tid = 0;
    all_mod_cfg.gfbg_config[0].thread_sleep_time = sleep_ms;
    all_mod_cfg.screen[0] = screen0;

    vo_input.square_sort = all_mod_cfg.vo_chn_num[0];
    vo_input.screen_type = all_mod_cfg.screen[0];
    ret = sample_comm_vo_get_default_config(vo_input, &all_mod_cfg.vo_config[0]);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vo_get_default_config failed !\n");
        return ret;
    }
    ret = sample_comm_vo_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vo init fail!\n");
        goto OUT1;
    }
    ret = sample_comm_vo_start(0, 0, all_mod_cfg.vo_chn[0], all_mod_cfg.vo_chn_num[0], XMEDIA_FALSE, &all_mod_cfg.vo_config[0]);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vo start failed !\n");
        goto OUT2;
    }

    ret = sample_gfbg_get_default_config(&all_mod_cfg, all_mod_cfg.vo_dev_num);
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_gfbg_get_default_config failed !\n");
        goto OUT3;
    }

    pthread_create(&(all_mod_cfg.gfbg_config[0].tid), 0, sample_gfbg_test_thread, (xmedia_void *)&all_mod_cfg.gfbg_config[0]);

    while(1) {
        SAMPLE_PRT("\nSAMPLE_TEST:press 'e' to exit; !\n");
        c = getchar();
        if (c == 'e') {
            break;
        }
    }
    sample_gfbg_stop_send_thread((xmedia_void *)&all_mod_cfg.gfbg_config[0]);

OUT3:
    sample_comm_vo_stop(0, 0, all_mod_cfg.vo_chn[0], all_mod_cfg.vo_chn_num[0]);
OUT2:
    sample_comm_vo_exit();
OUT1:
    return ret;
}

xmedia_void* sample_gfbg_one_buffer_test_thread(xmedia_void* args)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_u64 times;
    xmedia_s32 gfbg_fd;
    xmedia_bool state = XMEDIA_TRUE;
    sample_gfbg_config *gfbg_config;
    xmedia_video_rect draw_rect[4];

    gfbg_config = (sample_gfbg_config*)args;
    if(gfbg_config == XMEDIA_NULL) {
        SAMPLE_PRT("args is null !\n");
        return XMEDIA_NULL;
    }

    if(gfbg_config->layer_id == 0){
        gfbg_fd = open("/dev/fb0", O_RDWR);
    } else {
        gfbg_fd = open("/dev/fb1", O_RDWR);
    }
    if (gfbg_fd < 0) {
        SAMPLE_PRT("open fb0 failed !\n");
        goto OUT0;
    } else {
        SAMPLE_PRT("open fb0 success !\n");
    }

    ret = ioctl(gfbg_fd, GFBG_PUT_ALPHA, &gfbg_config->gfbg_alpha);
    if (ret < 0) {
        SAMPLE_PRT("GFBG_PUT_ALPHA failed !\n");
        goto OUT1;
    }
    if (ioctl(gfbg_fd, FBIOGET_VSCREENINFO, &gfbg_config->var_info) < 0) {
        SAMPLE_PRT("Get variable screen info failed!\n");
        goto OUT1;
    }
    gfbg_config->var_info.xres_virtual = gfbg_config->width;
    gfbg_config->var_info.yres_virtual = gfbg_config->height;
    gfbg_config->var_info.xres = gfbg_config->width;
    gfbg_config->var_info.yres = gfbg_config->height;
    gfbg_config->var_info.activate = FB_ACTIVATE_NOW;
    gfbg_config->var_info.bits_per_pixel = gfbg_config->bits_per_pixel;
    gfbg_config->var_info.xoffset = 0;
    gfbg_config->var_info.yoffset = 0;
    gfbg_config->var_info.red = gfbg_config->bitfield.red32;
    gfbg_config->var_info.green = gfbg_config->bitfield.green32;
    gfbg_config->var_info.blue = gfbg_config->bitfield.blue32;
    gfbg_config->var_info.transp = gfbg_config->bitfield.alpha32;

    if (ioctl(gfbg_fd, FBIOPUT_VSCREENINFO, &gfbg_config->var_info) < 0) {
        SAMPLE_PRT("FBIOPUT_VSCREENINFO error\n");
        goto OUT1;
    }
    if (ioctl(gfbg_fd, FBIOGET_FSCREENINFO, &gfbg_config->fix_info) < 0) {
        SAMPLE_PRT("FBIOGET_FSCREENINFO error\n");
        goto OUT1;
    }
    gfbg_config->gfbg_mmap_vir = mmap(NULL, gfbg_config->fix_info.smem_len, PROT_READ|PROT_WRITE, MAP_SHARED, gfbg_fd, 0);
    if (gfbg_config->gfbg_mmap_vir == XMEDIA_NULL) {
        SAMPLE_PRT("mmap fb0 failed!\n");
        goto OUT1;
    }
    memset(gfbg_config->gfbg_mmap_vir, 0x00, gfbg_config->fix_info.smem_len);

    if (ioctl(gfbg_fd, GFBG_PUT_SHOW, &state) < 0) {
        SAMPLE_PRT("GFBG_PUT_SHOW failed!\n");
        goto OUT2;
    }

    draw_rect[0].x = 0;
    draw_rect[0].y = 0;
    draw_rect[0].width = gfbg_config->width / 10;
    draw_rect[0].height = gfbg_config->height / 10;

    draw_rect[1].x = gfbg_config->width - gfbg_config->width / 10;
    draw_rect[1].y = 0;
    draw_rect[1].width = gfbg_config->width / 10;
    draw_rect[1].height = gfbg_config->height / 10;

    draw_rect[2].x = 0;
    draw_rect[2].y = gfbg_config->height - gfbg_config->height / 10;
    draw_rect[2].width = gfbg_config->width / 10;
    draw_rect[2].height = gfbg_config->height / 10;

    draw_rect[3].x = gfbg_config->width - gfbg_config->width / 10;
    draw_rect[3].y = gfbg_config->height - gfbg_config->height / 10;
    draw_rect[3].width = gfbg_config->width / 10;
    draw_rect[3].height = gfbg_config->height / 10;

    if (ioctl(gfbg_fd, FBIOPAN_DISPLAY, &gfbg_config->var_info) < 0) {
        SAMPLE_PRT("FBIOPAN_DISPLAY error\n");
        goto OUT2;
    }

    for (times = 0;; times++) {
        if(times % 2) {
            sample_gfbg_draw_box_offset(gfbg_config, draw_rect[0], 0xffff0000, 0);
            sample_gfbg_draw_box_offset(gfbg_config, draw_rect[1], 0xff00ff00, 0);
            sample_gfbg_draw_box_offset(gfbg_config, draw_rect[2], 0xff0000ff, 0);
            sample_gfbg_draw_box_offset(gfbg_config, draw_rect[3], 0xffff00ff, 0);
        } else {
            sample_gfbg_draw_box_offset(gfbg_config, draw_rect[0], 0xff00ff00, 0);
            sample_gfbg_draw_box_offset(gfbg_config, draw_rect[1], 0xff0000ff, 0);
            sample_gfbg_draw_box_offset(gfbg_config, draw_rect[2], 0xff0000ff, 0);
            sample_gfbg_draw_box_offset(gfbg_config, draw_rect[3], 0xffff0000, 0);
        }

        if(gfbg_config->thread_sleep_time !=0) {
            usleep(gfbg_config->thread_sleep_time*1000);
        } else {
            usleep(1000*1000);
        }
        if(gfbg_config->stop_flag == XMEDIA_TRUE) {
            break;
        }
    }

OUT2:
    munmap(gfbg_config->gfbg_mmap_vir, gfbg_config->fix_info.smem_len);
    gfbg_config->gfbg_mmap_vir = XMEDIA_NULL;
OUT1:
    close(gfbg_fd);
    gfbg_fd = -1;
OUT0:
    return XMEDIA_NULL;
}

static xmedia_s32 sample_gfbg_one_buffer_test(sample_comm_screen_type screen0, xmedia_s32 sleep_ms)
{
    xmedia_s32 ret;
    sample_vo_config_input vo_input = { 0 };
    sample_gfbg_all_mod_cfg all_mod_cfg;
    xmedia_char c;
    memset(&all_mod_cfg, 0, sizeof(sample_gfbg_all_mod_cfg));
    all_mod_cfg.vo_dev_num = 1;
    all_mod_cfg.vo_chn_num[0] = 1;
    all_mod_cfg.gfbg_layer_num = all_mod_cfg.vo_dev_num;
    all_mod_cfg.gfbg_config[0].tid = 0;
    all_mod_cfg.gfbg_config[0].thread_sleep_time = sleep_ms;
    all_mod_cfg.screen[0] = screen0;

    vo_input.square_sort = all_mod_cfg.vo_chn_num[0];
    vo_input.screen_type = all_mod_cfg.screen[0];
    ret = sample_comm_vo_get_default_config(vo_input, &all_mod_cfg.vo_config[0]);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vo_get_default_config failed !\n");
        return ret;
    }
    ret = sample_comm_vo_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vo init fail!\n");
        goto OUT1;
    }
    ret = sample_comm_vo_start(0, 0, all_mod_cfg.vo_chn[0], all_mod_cfg.vo_chn_num[0], XMEDIA_FALSE, &all_mod_cfg.vo_config[0]);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vo start failed !\n");
        goto OUT2;
    }

    ret = sample_gfbg_get_default_config(&all_mod_cfg, all_mod_cfg.vo_dev_num);
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_gfbg_get_default_config failed !\n");
        goto OUT3;
    }

    pthread_create(&(all_mod_cfg.gfbg_config[0].tid), 0, sample_gfbg_one_buffer_test_thread, (xmedia_void *)&all_mod_cfg.gfbg_config[0]);

    while(1) {
        SAMPLE_PRT("\nSAMPLE_TEST:press 'e' to exit; !\n");
        c = getchar();
        if (c == 'e') {
            break;
        }
    }
    sample_gfbg_stop_send_thread((xmedia_void *)&all_mod_cfg.gfbg_config[0]);

OUT3:
    sample_comm_vo_stop(0, 0, all_mod_cfg.vo_chn[0], all_mod_cfg.vo_chn_num[0]);
OUT2:
    sample_comm_vo_exit();
OUT1:
    return ret;
}


int main(int argc, char *argv[])
{
    xmedia_s32 ret = XMEDIA_FAILURE;
    xmedia_s32 test_mode = 0;
    xmedia_s32 sleep_ms = 0;
    sample_comm_screen_type screen0_type = SIL9024_BT1120_1280X720P60;

    signal(SIGINT, sample_gfbg_handle_sig);
    signal(SIGTERM, sample_gfbg_handle_sig);

    if (argc >= 2) {
        test_mode = atoi(argv[1]);
    } else {
        test_mode = -1;
    }

    switch (test_mode) {
        case 0:
            if (argc < 3){
                SAMPLE_PRT("gfbg_test:./sample_gfbg [test_mode] [screen0_TYPE] [sleep_ms]\n");
                SAMPLE_PRT("example : ./sample_vo 0 2 20\n");
                SAMPLE_PRT("example : ./sample_vo 0 1 20\n");
                ret = XMEDIA_FAILURE;
                break;
            }
            screen0_type = atoi(argv[2]);
            if(argc == 4) {
                sleep_ms = atoi(argv[3]);
            }
            sample_gfbg_test(screen0_type, sleep_ms);
            break;
        case 2:
            if (argc < 3){
                SAMPLE_PRT("gfbg_tde_test:  ./sample_gfbg [test_mode] [screen0_TYPE] [sleep_ms]\n");
                SAMPLE_PRT("example :       ./sample_gfbg 2 2 20\n");
                SAMPLE_PRT("example :       ./sample_gfbg 2 1 20\n");
                ret = XMEDIA_FAILURE;
                break;
            }
            screen0_type = atoi(argv[2]);
            if(argc == 4) {
                sleep_ms = atoi(argv[3]);
            }
            ret = sample_gfbg_tde_test(screen0_type, sleep_ms);
            break;
        case 4:
            if (argc < 3){
                SAMPLE_PRT("gfbg_one_buffer_test: ./sample_gfbg [test_mode] [screen0_TYPE] [sleep_ms]\n");
                SAMPLE_PRT("example :             ./sample_gfbg 4 2 20\n");
                SAMPLE_PRT("example :             ./sample_gfbg 4 1 20\n");
                ret = XMEDIA_FAILURE;
                break;
            }
            screen0_type = atoi(argv[2]);
            if(argc == 4) {
                sleep_ms = atoi(argv[3]);
            }
            sample_gfbg_one_buffer_test(screen0_type, sleep_ms);
            break;

        default:
            SAMPLE_PRT("test mode:         0:normal test    2:tde-fb test   4:one buffer test\n");
            SAMPLE_PRT("screen0_TYPE:      0:PLCD800x480   5:BT656_1280X720_UYVY422     8:SLCD_240X320_18BIT    12:KD026_320X240P10_4LINE_RGB565\n");
            SAMPLE_PRT("screen0_TYPE:      14:KD026_320X240P60_18BIT_18BPP_RGB666_8080I    16:SIL9024_BT1120_1280X720P60 \n");
            SAMPLE_PRT("sleep_ms:          20:send frame gap 20ms\n");

            SAMPLE_PRT("gfbg_test :                ./sample_gfbg [test_mode] [screen0_TYPE] [sleep_ms]\n");
            SAMPLE_PRT("example :                  ./sample_gfbg 0 16 20\n");

            SAMPLE_PRT("gfbg_tde_test              ./sample_gfbg [test_mode] [screen0_TYPE] [sleep_ms]\n");
            SAMPLE_PRT("example :                  ./sample_gfbg 2 16 20\n");

            SAMPLE_PRT("gfbg_one_buffer_test       ./sample_gfbg [test_mode] [screen0_TYPE] [sleep_ms]\n");
            SAMPLE_PRT("example :                  ./sample_gfbg 4 16 20\n");
            return XMEDIA_FAILURE;
    }

    if (XMEDIA_SUCCESS == ret) {
        SAMPLE_PRT("program exit normally!\n");
    } else {
        SAMPLE_PRT("program exit abnormally!\n");
    }

    return ret;
}
