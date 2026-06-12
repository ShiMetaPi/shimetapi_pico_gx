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

#include "sample_comm.h"
#include "sample_comm_sys.h"
#include "sample_comm_vi.h"
#include "sample_comm_vpss.h"
#include "sample_comm_vo.h"
#include "sample_comm_isp.h"
#include "sample_comm_vgs.h"
#include "sample_comm_venc.h"
#include "sample_comm_ircut.h"
#include "sample_comm_livestream.h"
#include "sample_svp_main.h"

//#define SAMPLE_TIME_DEBUG 1
#define SVP_SMALL_STREAM_W 640
#define SVP_SMALL_STREAM_H 360
#define SVP_MID_STREAM_W 1280
#define SVP_MID_STREAM_H 720
#define SVP_BIG_STREAM_W 1920
#define SVP_BIG_STREAM_H 1080
#define SAMPLE_MAX_STRING_NUM 64
#define SVP_SCALE_RATIO 10000.0f

#ifdef SAMPLE_TIME_DEBUG

static struct timeval g_stTimeCostStart;
static struct timeval g_stTimeCostEnd;

#define TIME_COST_START()   do {    \
    gettimeofday(&g_stTimeCostStart,NULL); \
} while(0)

#define TIME_COST_END()   do {    \
        gettimeofday(&g_stTimeCostEnd,NULL); \
} while(0)

#define TIME_COST_PRINT(info)   do { \
    xmedia_u64 u64TimeUseMs = (g_stTimeCostEnd.tv_sec - g_stTimeCostStart.tv_sec) * 1000000 \
                      + (g_stTimeCostEnd.tv_usec - g_stTimeCostStart.tv_usec);   \
    printf("%s cost time:%llu us\n",info,u64TimeUseMs);  \
} while(0)
#endif

static sample_comm_sensor_type sample_svp_sensor_type[MAX_SENSOR_NUM] = { SENSOR0_TYPE, SENSOR1_TYPE, SENSOR2_TYPE,
                                                                          SENSOR3_TYPE, SENSOR4_TYPE };
static sample_comm_screen_type sample_vo_screen_type[MAX_SCREEN_NUM] = { SCREEN0_TYPE, SCREEN0_TYPE};
static xmedia_bool g_force_exit = XMEDIA_FALSE;
static xmedia_bool g_venc_use = XMEDIA_TRUE;
xmedia_bool g_svp_start_flag = XMEDIA_FALSE;
static pthread_t g_svp_thread;
static xmedia_svp_cfg g_svp_cfg = {0};

#ifdef USE_OSD

static BITMAP_S g_bitmap;

rect_size sample_ivp_caculate_font_size(font_size_e font_size)
{
    xmedia_s32 asc_width = 0;
    xmedia_s32 asc_height = 0;

    rect_size font;

    switch(font_size) {
        case FONT_SIZE_16:
            asc_width = 8;
            asc_height = 16;
            break;

        case FONT_SIZE_20:
            asc_width = 10;
            asc_height = 20;
            break;

        case FONT_SIZE_24:
            asc_width = 12;
            asc_height = 24;
            break;

        case FONT_SIZE_32:
            asc_width = 16;
            asc_height = 32;
            break;

        case FONT_SIZE_40:
            asc_width = 20;
            asc_height = 40;
            break;

        case FONT_SIZE_48:
            asc_width = 24;
            asc_height = 48;
            break;

        case FONT_SIZE_56:
            asc_width = 28;
            asc_height = 56;
            break;

        default:
            SAMPLE_PRT("not support this font size: %d\n", font_size);
            asc_width = 0;
            asc_height = 0;
    }

    font.width = asc_width;
    font.height = asc_height;

    return font;
}

xmedia_s32 sample_construct_string_bitmap(xmedia_char* string_content, xmedia_s32 string_len,
    font_size_e font_type, BITMAP_S* bit_map)
{
    canvas_t *canvas_context = XMEDIA_NULL;
    rect_size font_rect_size;
    xmedia_u64 vgs_osd_phy_addr = 0;

    font_rect_size = sample_ivp_caculate_font_size(font_type);
    if ((font_rect_size.height == 0) || (font_rect_size.width == 0) || (string_len == 0)) {
        SAMPLE_PRT("Invalid font size, error! %d-%d|%d\n", font_rect_size.height,font_rect_size.width,string_len);
        return XMEDIA_FAILURE;
    }

    if (bit_map == XMEDIA_NULL) {
        SAMPLE_PRT("bit_map can not be NULL, error \n");
        return XMEDIA_FAILURE;
    }

    canvas_context = canvas_init(font_rect_size.width * string_len, font_rect_size.height);
    if (canvas_context == XMEDIA_NULL) {
        SAMPLE_PRT("No free canvas, error! \n");
        return XMEDIA_FAILURE;
    }

    canvas_clear(canvas_context);
    // string must be end with 0;
    canvas_fill_text(canvas_context, 0, 0, string_content, font_type,
                     TEXT_STYLE_FOREGROUND_WHRITE | TEXT_STYLE_BACKGROUND_BLACK);

    bit_map->u32Width = canvas_context->width;
    bit_map->u32Height = canvas_context->height;
    bit_map->pData = canvas_context->pixels;
    bit_map->enPixelFormat = XMEDIA_VIDEO_PIXEL_FMT_ARGB_8888;

    vgs_osd_phy_addr = xmedia_mmz_alloc(XMEDIA_NULL, "osd_buf", canvas_context->width * canvas_context->height * 4);
    if (0 == vgs_osd_phy_addr) {
        SAMPLE_PRT("xmedia_mmz_alloc %d failed with!\n", bit_map->u32Height*bit_map->u32Width*4);
        canvas_deinit(canvas_context);
        return XMEDIA_FAILURE;
    }

    xmedia_void *vgs_osd_vir_addr =
        xmedia_mmz_map(vgs_osd_phy_addr, canvas_context->width * canvas_context->height * 4, XMEDIA_FALSE);
    if (vgs_osd_vir_addr == 0) {
        SAMPLE_PRT("xmedia_mmz_map failed\n");
        canvas_deinit(canvas_context);
        return XMEDIA_FAILURE;
    }

    memcpy(vgs_osd_vir_addr, bit_map->pData, canvas_context->width * canvas_context->height * 4);
    xmedia_mmz_unmap(vgs_osd_vir_addr);

    bit_map->pData = CONVERT_TO_32BIT_ADDR(vgs_osd_phy_addr);

    canvas_deinit(canvas_context);

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_disconstruct_string_bitmap(BITMAP_S *bitmap_ptr)
{
    xmedia_u64 vgs_osd_phy_addr = 0;

    vgs_osd_phy_addr = CONVERT_TO_64BIT_ADDR(bitmap_ptr->pData);
    xmedia_mmz_free(vgs_osd_phy_addr);

    memset(bitmap_ptr, 0, sizeof(BITMAP_S));

    return XMEDIA_SUCCESS;
}

xmedia_s32 utf8_to_gbk(xmedia_char *utf8_string, xmedia_char *gbk_string)
{
    if (strlen(utf8_string) < 3) {
      return XMEDIA_FAILURE;
    }

    if (strncmp(utf8_string, "粤", 3) == 0) {
      memcpy(gbk_string, "\xD4\xC1",2);
    } else if (strncmp(utf8_string, "桂", 3) == 0) {
      memcpy(gbk_string, "\xB9\xF0",2);
    } else if (strncmp(utf8_string, "贵", 3) == 0) {
      memcpy(gbk_string, "\xB9\xF3",2);
    } else if (strncmp(utf8_string, "冀", 3) == 0) {
      memcpy(gbk_string, "\xBC\xBD",2);
    } else if (strncmp(utf8_string, "吉", 3) == 0) {
      memcpy(gbk_string, "\xBC\xAA",2);
    } else if (strncmp(utf8_string, "京", 3) == 0) {
      memcpy(gbk_string, "\xBE\xA9",2);
    } else if (strncmp(utf8_string, "吉", 3) == 0) {
      memcpy(gbk_string, "\xC7\xED",2);
    } else if (strncmp(utf8_string, "陕", 3) == 0) {
      memcpy(gbk_string, "\xC9\xC2",2);
    } else if (strncmp(utf8_string, "苏", 3) == 0) {
      memcpy(gbk_string, "\xCB\xD5",2);
    } else if (strncmp(utf8_string, "湘", 3) == 0) {
      memcpy(gbk_string, "\xCF\xE6",2);
    } else if (strncmp(utf8_string, "渝", 3) == 0) {
      memcpy(gbk_string, "\xD3\xE5",2);
    } else if (strncmp(utf8_string, "豫", 3) == 0) {
      memcpy(gbk_string, "\xD4\xA5",2);
    } else if (strncmp(utf8_string, "藏", 3) == 0) {
      memcpy(gbk_string, "\xB2\xD4",2);
    } else if (strncmp(utf8_string, "川", 3) == 0) {
      memcpy(gbk_string, "\xB4\xA8",2);
    } else if (strncmp(utf8_string, "鄂", 3) == 0) {
      memcpy(gbk_string, "\xB6\xF5",2);
    } else if (strncmp(utf8_string, "甘", 3) == 0) {
      memcpy(gbk_string, "\xB8\xCA",2);
    } else if (strncmp(utf8_string, "赣", 3) == 0) {
      memcpy(gbk_string, "\xB8\xD3",2);
    } else if (strncmp(utf8_string, "黑", 3) == 0) {
      memcpy(gbk_string, "\xBA\xDA",2);
    } else if (strncmp(utf8_string, "沪", 3) == 0) {
      memcpy(gbk_string, "\xBB\xA6",2);
    } else if (strncmp(utf8_string, "津", 3) == 0) {
      memcpy(gbk_string, "\xBD\xF2",2);
    } else if (strncmp(utf8_string, "晋", 3) == 0) {
      memcpy(gbk_string, "\xBD\xFA",2);
    } else if (strncmp(utf8_string, "鲁", 3) == 0) {
      memcpy(gbk_string, "\xC2\xB3",2);
    } else if (strncmp(utf8_string, "蒙", 3) == 0) {
      memcpy(gbk_string, "\xC3\xC9",2);
    } else if (strncmp(utf8_string, "闽", 3) == 0) {
      memcpy(gbk_string, "\xC3\xF6",2);
    } else if (strncmp(utf8_string, "宁", 3) == 0) {
      memcpy(gbk_string, "\xC4\xFE",2);
    } else if (strncmp(utf8_string, "青", 3) == 0) {
      memcpy(gbk_string, "\xC7\xE0",2);
    } else if (strncmp(utf8_string, "挂", 3) == 0) {
      memcpy(gbk_string, "\xB9\xD2",2);
    } else if (strncmp(utf8_string, "皖", 3) == 0) {
      memcpy(gbk_string, "\xCD\xEE",2);
    } else if (strncmp(utf8_string, "新", 3) == 0) {
      memcpy(gbk_string, "\xD0\xC2",2);
    } else if (strncmp(utf8_string, "云", 3) == 0) {
      memcpy(gbk_string, "\xD4\xC6",2);
    } else if (strncmp(utf8_string, "浙", 3) == 0) {
      memcpy(gbk_string, "\xD5\xE3",2);
    } else if (strncmp(utf8_string, "辽", 3) == 0) {
      memcpy(gbk_string, "\xC1\xC9",2);
    } else if (strncmp(utf8_string, "学", 3) == 0) {
      memcpy(gbk_string, "\xD1\xA7",2);
    } else if (strncmp(utf8_string, "警", 3) == 0) {
      memcpy(gbk_string, "\xBE\xAF",2);
    } else if (strncmp(utf8_string, "领", 3) == 0) {
      memcpy(gbk_string, "\xC1\xEC",2);
    } else if (strncmp(utf8_string, "使", 3) == 0) {
      memcpy(gbk_string, "\xCA\xB9",2);
    } else if (strncmp(utf8_string, "港", 3) == 0) {
      memcpy(gbk_string, "\xB8\xD8",2);
    } else if (strncmp(utf8_string, "澳", 3) == 0) {
      memcpy(gbk_string, "\xB0\xC4",2);
    } else {
      memcpy(gbk_string, "\xCE\xDE",2); // 无
    }

    return XMEDIA_SUCCESS;
}

// 打印跟踪信息
xmedia_s32 sample_target_osd(xmedia_video_frame_info *frame_info, xmedia_s32 s32x, xmedia_s32 s32y,
                                    xmedia_char *format_untile_line)
{
    xmedia_s32 ret;
    font_size_e font_size = FONT_SIZE_16;
    VGS_HANDLE handle = -1;
    xmedia_char str_array[512];
    rect_size font_rect_size;
    xmedia_vgs_osd_attr osd_attr = {0};
    xmedia_vgs_frame_info task_info = {0};

    ret = xmedia_vgs_init();
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_vgs_init failed !\n");
        return ret;
    }

    memcpy(&task_info.img_in, frame_info, sizeof(xmedia_video_frame_info));
    memcpy(&task_info.img_out, frame_info, sizeof(xmedia_video_frame_info));

    ret = xmedia_vgs_create_job(&handle);
    if (XMEDIA_SUCCESS != ret) {
        SAMPLE_PRT("sample_ivp_osd_info_t xmedia_vgs_create_job failed %#x!\n", ret);
        xmedia_vgs_cancel_job(handle);
        handle = -1;
        return XMEDIA_FAILURE;
    }

    font_rect_size = sample_ivp_caculate_font_size(font_size);
    if((font_rect_size.height == 0)||(font_rect_size.width == 0)) {
        return XMEDIA_FAILURE;
    }

    memset(str_array, 0, sizeof(str_array));
    memcpy(str_array, format_untile_line, 512);
    ret = sample_construct_string_bitmap(str_array, strlen(str_array), font_size, &g_bitmap);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("string_bitmap failed \n");
        xmedia_vgs_cancel_job(handle);
        handle = -1;
        return XMEDIA_FAILURE;
    }

    osd_attr.osd_rect.y = ((s32y + g_bitmap.u32Height) > 360 ? s32y - g_bitmap.u32Height : s32y);
    osd_attr.osd_rect.y = (osd_attr.osd_rect.y + 1) / 2 * 2;
    osd_attr.osd_rect.x = ((s32x + g_bitmap.u32Width) > 640 ? s32x - g_bitmap.u32Width : s32x);
    osd_attr.osd_rect.x = (osd_attr.osd_rect.x + 1) / 2 * 2;


    osd_attr.osd_rect.width = g_bitmap.u32Width;
    osd_attr.osd_rect.height = g_bitmap.u32Height;

    // printf("x[%d]y[%d]w[%d]h[%d]\n", osd_attr.osd_rect.x,
    //     osd_attr.osd_rect.y,osd_attr.osd_rect.width,osd_attr.osd_rect.height);
    osd_attr.pixel_fmt = g_bitmap.enPixelFormat;
    osd_attr.alpha0 = 0;
    osd_attr.alpha1 = 0xff;
    osd_attr.is_osd_revert = XMEDIA_FALSE;
    osd_attr.stride = g_bitmap.u32Width*4;
    osd_attr.phys_addr = CONVERT_TO_64BIT_ADDR(g_bitmap.pData);

    ret = xmedia_vgs_add_task_osd(handle, &task_info, &osd_attr, 1);
    if(ret != XMEDIA_SUCCESS) {
        xmedia_vgs_cancel_job(handle);
        SAMPLE_PRT("xmedia_vgs_wait_job failed!\n");
    }

     ret = xmedia_vgs_submit_job(handle);
    if(ret != XMEDIA_SUCCESS) {
        xmedia_vgs_cancel_job(handle);
        SAMPLE_PRT("xmedia_vgs_submit_job failed!\n");
    }

    ret = xmedia_vgs_wait_job(handle, 2000);
    if(ret != XMEDIA_SUCCESS) {
        xmedia_vgs_cancel_job(handle);
        SAMPLE_PRT("xmedia_vgs_wait_job failed!\n");
    }

    sample_disconstruct_string_bitmap(&g_bitmap);

    return XMEDIA_SUCCESS;
}
#endif

xmedia_s32 sample_vgs_resize(const xmedia_video_frame_info *input_frame, xmedia_video_frame_info *output_frame)
{
    xmedia_s32 ret;
    xmedia_s32 vgs_handle;
    xmedia_vgs_frame_info vgs_task;

    memset(&vgs_task, 0, sizeof(vgs_task));
    ret = xmedia_vgs_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_vgs_init failed !\n");
        return ret;
    }

    ret = xmedia_vgs_create_job(&vgs_handle);
    if (XMEDIA_SUCCESS != ret) {
        SAMPLE_PRT("XMEDIA_API_VGS_BeginJob failed %#x!\n", ret);
        return XMEDIA_FAILURE;
    }

    output_frame->mod_id = MOD_ID_USER;
    output_frame->frame.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;
    output_frame->frame.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    output_frame->frame.bit_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    memcpy(&vgs_task.img_in, input_frame, sizeof(xmedia_video_frame_info));
    memcpy(&vgs_task.img_out, output_frame, sizeof(xmedia_video_frame_info));

    ret = xmedia_vgs_add_task_scale(vgs_handle, &vgs_task, XMEDIA_VIDEO_SCALE_MODE_NORMAL);
    if (XMEDIA_SUCCESS != ret) {
        (xmedia_void) xmedia_vgs_cancel_job(vgs_handle);
        SAMPLE_PRT("XMEDIA_API_VGS_AddScaleTask failed %#x!\n", ret);
        return XMEDIA_FAILURE;
    }

    ret = xmedia_vgs_submit_job(vgs_handle);
    if (XMEDIA_SUCCESS != ret) {
        (xmedia_void) xmedia_vgs_cancel_job(vgs_handle);
        SAMPLE_PRT("vgs submit failed %#x!\n", ret);
        return XMEDIA_FAILURE;
    }

    ret = xmedia_vgs_wait_job(vgs_handle, 2000); // 2000ms超时时间
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vgs wait job failed %#x!\n", ret);
        return ret;
    }
    return XMEDIA_SUCCESS;
}

// dms画图
static xmedia_s32 sample_svp_draw_dms(xmedia_svp_dms_output *dms_result,xmedia_video_frame_info *frame)
{
    xmedia_s32 ret;
    xmedia_vgs_frame_info task = {0};
    xmedia_vgs_cover_attr cover[XMEDIA_SVP_MAX_TARGET_NUM];
    xmedia_video_rect target_rect[XMEDIA_SVP_MAX_TARGET_NUM];
    xmedia_s32 handle = -1;
    xmedia_s32 i,j;
    xmedia_u32 w, h;
    xmedia_s32 x1, y1, x2, y2;
    xmedia_u8 object_num = 0;
    xmedia_u8 tmp_num = dms_result->target_num;

    if (0 == tmp_num)
    {
        return XMEDIA_FAILURE;
    }

    for (i = 0; i < tmp_num; i++) {
        x1 = (xmedia_s32)roundf(dms_result->targets[i].rect.x1 / 2) * 2;
        y1 = (xmedia_s32)roundf(dms_result->targets[i].rect.y1 / 2) * 2;
        x2 = (xmedia_s32)roundf(dms_result->targets[i].rect.x2 / 2) * 2;
        y2 = (xmedia_s32)roundf(dms_result->targets[i].rect.y2 / 2) * 2;
        w = ABS(x2 - x1);
        h = ABS(y2 - y1);
        target_rect[object_num].x = x1;
        target_rect[object_num].y = y1;
        target_rect[object_num].width = w;
        target_rect[object_num].height = h;
        object_num++;
        for (j = 0; j < dms_result->targets[i].dms_report.phone_num; j++) {
            x1 = (xmedia_s32)roundf(dms_result->targets[i].dms_report.phone[j].rect.x1 / 2) * 2;
            y1 = (xmedia_s32)roundf(dms_result->targets[i].dms_report.phone[j].rect.y1 / 2) * 2;
            x2 = (xmedia_s32)roundf(dms_result->targets[i].dms_report.phone[j].rect.x2 / 2) * 2;
            y2 = (xmedia_s32)roundf(dms_result->targets[i].dms_report.phone[j].rect.y2 / 2) * 2;
            w = ABS(x2 - x1);
            h = ABS(y2 - y1);
            target_rect[object_num].x = x1;
            target_rect[object_num].y = y1;
            target_rect[object_num].width = w;
            target_rect[object_num].height = h;
            object_num++;
        }
        for (j = 0; j < dms_result->targets[i].dms_report.cigar_num; j++) {
            x1 = (xmedia_s32)roundf(dms_result->targets[i].dms_report.cigar[j].rect.x1 / 2) * 2;
            y1 = (xmedia_s32)roundf(dms_result->targets[i].dms_report.cigar[j].rect.y1 / 2) * 2;
            x2 = (xmedia_s32)roundf(dms_result->targets[i].dms_report.cigar[j].rect.x2 / 2) * 2;
            y2 = (xmedia_s32)roundf(dms_result->targets[i].dms_report.cigar[j].rect.y2 / 2) * 2;
            w = ABS(x2 - x1);
            h = ABS(y2 - y1);
            target_rect[object_num].x = x1;
            target_rect[object_num].y = y1;
            target_rect[object_num].width = w;
            target_rect[object_num].height = h;
            object_num++;
        }
    }

    object_num = object_num > XMEDIA_SVP_MAX_TARGET_NUM ? XMEDIA_SVP_MAX_TARGET_NUM : object_num;
    memcpy(&task.img_in,frame,sizeof(xmedia_video_frame_info));
    memcpy(&task.img_out,frame,sizeof(xmedia_video_frame_info));
    ret = xmedia_vgs_init();
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_vgs_init failed !\n");
        return ret;
    }
    ret = xmedia_vgs_create_job(&handle);
    if (XMEDIA_SUCCESS != ret)
    {
        xmedia_vgs_exit();
        SAMPLE_PRT("xmedia_vgs_cancel_job failed !\n");
        handle = -1;
        return XMEDIA_FAILURE;
    }

    for (i = 0; i < object_num; i++) {
        if (dms_result->targets[i].special_target == XMEDIA_TRUE) {
            cover[i].color = 0x0000FF; // blue
        } else {
            cover[i].color = 0xFF0000; // red
        }
        cover[i].cover_type = XMEDIA_VGS_COVER_TYPE_QUAD_RANGLE;
        cover[i].quadrangle.is_solid = XMEDIA_FALSE;
        cover[i].quadrangle.thick = 2;
        cover[i].quadrangle.points[0].x = target_rect[i].x;
        cover[i].quadrangle.points[0].y = target_rect[i].y;
        cover[i].quadrangle.points[1].x = target_rect[i].x + target_rect[i].width;
        cover[i].quadrangle.points[1].y = target_rect[i].y;
        cover[i].quadrangle.points[2].x = target_rect[i].x + target_rect[i].width;
        cover[i].quadrangle.points[2].y = target_rect[i].y + target_rect[i].height;
        cover[i].quadrangle.points[3].x = target_rect[i].x;
        cover[i].quadrangle.points[3].y = target_rect[i].y + target_rect[i].height;
    }
    ret = xmedia_vgs_add_task_cover(handle,&task,&cover[0], object_num);
    if (XMEDIA_SUCCESS != ret)
    {
        xmedia_vgs_cancel_job(handle);
        xmedia_vgs_exit();
        SAMPLE_PRT("xmedia_vgs_add_task_cover failed ! ret [%d] \n", ret);
        return XMEDIA_FAILURE;
    }
    ret = xmedia_vgs_submit_job(handle);
    if (XMEDIA_SUCCESS != ret)
    {
        xmedia_vgs_cancel_job(handle);
        xmedia_vgs_exit();
        SAMPLE_PRT("xmedia_vgs_submit_job failed ! ret [%d] \n", ret);
        return XMEDIA_FAILURE;
    }
    ret = xmedia_vgs_wait_job(handle, 2000); // 2000ms超时时间
    if (ret != XMEDIA_SUCCESS) {
        xmedia_vgs_exit();
        SAMPLE_PRT("xmedia_vgs_wait_job failed ! ret [%d] \n", ret);
        return ret;
    }
    return XMEDIA_SUCCESS;
}

// yolov8画图
static xmedia_s32 sample_svp_draw_v8(xmedia_video_rect target_rect[], xmedia_video_frame_info *frame,
                                                                     xmedia_svp_yolov8_output result)
{
    xmedia_s32 ret;
    xmedia_vgs_frame_info task = {0};
    xmedia_vgs_cover_attr cover[XMEDIA_SVP_MAX_TARGET_NUM];
    xmedia_s32 handle = -1;
    xmedia_s32 i;
    xmedia_u8 object_num = result.target_num;
    if (0 == object_num)
    {
        return XMEDIA_FAILURE;
    }
    //object_num = 1;
    object_num = object_num > XMEDIA_SVP_MAX_TARGET_NUM ? XMEDIA_SVP_MAX_TARGET_NUM : object_num;
    memcpy(&task.img_in,frame,sizeof(xmedia_video_frame_info));
    memcpy(&task.img_out,frame,sizeof(xmedia_video_frame_info));
    ret = xmedia_vgs_init();
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_vgs_init failed !\n");
        return ret;
    }
    ret = xmedia_vgs_create_job(&handle);
    if (XMEDIA_SUCCESS != ret)
    {
        xmedia_vgs_exit();
        SAMPLE_PRT("xmedia_vgs_cancel_job failed !\n");
        handle = -1;
        return XMEDIA_FAILURE;
    }
    for (i = 0;i < object_num;i++)
    {
        if (result.targets[i].class_type == XMEDIA_SVP_CLASS_TYPE_SMOKE) {
            cover[i].color = 0x0000FF; // blue
        } else {
            cover[i].color = 0xFF0000; // red
        }
        cover[i].cover_type = XMEDIA_VGS_COVER_TYPE_QUAD_RANGLE;
        cover[i].quadrangle.is_solid = XMEDIA_FALSE;
        cover[i].quadrangle.thick = 2;
        cover[i].quadrangle.points[0].x = target_rect[i].x;
        cover[i].quadrangle.points[0].y = target_rect[i].y;
        cover[i].quadrangle.points[1].x = target_rect[i].x + target_rect[i].width;
        cover[i].quadrangle.points[1].y = target_rect[i].y;
        cover[i].quadrangle.points[2].x = target_rect[i].x + target_rect[i].width;
        cover[i].quadrangle.points[2].y = target_rect[i].y + target_rect[i].height;
        cover[i].quadrangle.points[3].x = target_rect[i].x;
        cover[i].quadrangle.points[3].y = target_rect[i].y + target_rect[i].height;
    }
    ret = xmedia_vgs_add_task_cover(handle,&task,&cover[0], object_num);
    if (XMEDIA_SUCCESS != ret)
    {
        xmedia_vgs_cancel_job(handle);
        xmedia_vgs_exit();
        SAMPLE_PRT("xmedia_vgs_add_task_cover failed ! ret [%d] \n", ret);
        return XMEDIA_FAILURE;
    }
    ret = xmedia_vgs_submit_job(handle);
    if (XMEDIA_SUCCESS != ret)
    {
        xmedia_vgs_cancel_job(handle);
        xmedia_vgs_exit();
        SAMPLE_PRT("xmedia_vgs_submit_job failed ! ret [%d] \n", ret);
        return XMEDIA_FAILURE;
    }
    ret = xmedia_vgs_wait_job(handle, 2000); // 2000ms超时时间
    if (ret != XMEDIA_SUCCESS) {
        xmedia_vgs_exit();
        SAMPLE_PRT("xmedia_vgs_wait_job failed ! ret [%d] \n", ret);
        return ret;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_svp_draw(xmedia_video_rect target_rect[],
                                  xmedia_video_frame_info *frame,
                                  xmedia_svp_yolov5_output result)
{
    xmedia_s32 ret;
    xmedia_vgs_frame_info task = {0};
    xmedia_vgs_cover_attr cover[XMEDIA_SVP_MAX_TARGET_NUM];
    xmedia_s32 handle = -1;
    xmedia_s32 i;
    xmedia_u8 object_num = result.target_num;
    if (0 == object_num)
    {
        return XMEDIA_FAILURE;
    }
    //object_num = 1;
    object_num = object_num > XMEDIA_SVP_MAX_TARGET_NUM ? XMEDIA_SVP_MAX_TARGET_NUM : object_num;
    memcpy(&task.img_in,frame,sizeof(xmedia_video_frame_info));
    memcpy(&task.img_out,frame,sizeof(xmedia_video_frame_info));
    ret = xmedia_vgs_init();
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_vgs_init failed !\n");
        return ret;
    }
    ret = xmedia_vgs_create_job(&handle);
    if (XMEDIA_SUCCESS != ret)
    {
        xmedia_vgs_exit();
        SAMPLE_PRT("xmedia_vgs_cancel_job failed !\n");
        handle = -1;
        return XMEDIA_FAILURE;
    }
    for (i = 0; i < object_num; i++) {
        if (result.targets[i].class_type == XMEDIA_SVP_CLASS_TYPE_FIREWORKS_SMOKE ||
            result.targets[i].class_type == XMEDIA_SVP_CLASS_TYPE_TRICYCLER) {
            cover[i].color = 0x0000FF; // blue
        } else if (result.targets[i].class_type == XMEDIA_SVP_CLASS_TYPE_BIKER) {
            cover[i].color = 0x000FFF;
        } else if (result.targets[i].class_type == XMEDIA_SVP_CLASS_TYPE_MOTOR) {
            cover[i].color = 0x0F000F;
        } else if (result.targets[i].class_type == XMEDIA_SVP_CLASS_TYPE_MOTORER) {
            cover[i].color = 0xFF00FF;
        } else if (result.targets[i].class_type == XMEDIA_SVP_CLASS_TYPE_TRICYCLE) {
            cover[i].color = 0x00FF00;
        } else {
            cover[i].color = 0xFF0000; // red
        }
        cover[i].cover_type = XMEDIA_VGS_COVER_TYPE_QUAD_RANGLE;
        cover[i].quadrangle.is_solid = XMEDIA_FALSE;
        cover[i].quadrangle.thick = 2;
        cover[i].quadrangle.points[0].x = target_rect[i].x;
        cover[i].quadrangle.points[0].y = target_rect[i].y;
        cover[i].quadrangle.points[1].x = target_rect[i].x + target_rect[i].width;
        cover[i].quadrangle.points[1].y = target_rect[i].y;
        cover[i].quadrangle.points[2].x = target_rect[i].x + target_rect[i].width;
        cover[i].quadrangle.points[2].y = target_rect[i].y + target_rect[i].height;
        cover[i].quadrangle.points[3].x = target_rect[i].x;
        cover[i].quadrangle.points[3].y = target_rect[i].y + target_rect[i].height;
    }
    ret = xmedia_vgs_add_task_cover(handle, &task, &cover[0], object_num);
    if (XMEDIA_SUCCESS != ret) {
        xmedia_vgs_cancel_job(handle);
        xmedia_vgs_exit();
        SAMPLE_PRT("xmedia_vgs_add_task_cover failed ! ret [%d] \n", ret);
        return XMEDIA_FAILURE;
    }
    ret = xmedia_vgs_submit_job(handle);
    if (XMEDIA_SUCCESS != ret) {
        xmedia_vgs_cancel_job(handle);
        xmedia_vgs_exit();
        SAMPLE_PRT("xmedia_vgs_submit_job failed ! ret [%d] \n", ret);
        return XMEDIA_FAILURE;
    }
    ret = xmedia_vgs_wait_job(handle, 2000);
    if (ret != XMEDIA_SUCCESS) {
        xmedia_vgs_exit();
        SAMPLE_PRT("xmedia_vgs_wait_job failed ! ret [%d] \n", ret);
        return ret;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_svp_draw_keypoint(xmedia_svp_keypoint target_rect[], xmedia_u32 kpt_num,
                                           xmedia_video_frame_info* frame)
{
    xmedia_s32 ret;
    xmedia_vgs_frame_info task = { 0 };
    xmedia_vgs_cover_attr cover[kpt_num];
    xmedia_s32 handle = -1;
    xmedia_s32 i;
    xmedia_s32 x, y;
    xmedia_u8 point_num = 0;
    xmedia_bool horizontal_line = XMEDIA_FALSE, vertical_line = XMEDIA_FALSE;
    ret = xmedia_vgs_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_vgs_init failed !\n");
        return ret;
    }

    ret = xmedia_vgs_create_job(&handle);
    if (XMEDIA_SUCCESS != ret) {
        xmedia_vgs_exit();
        SAMPLE_PRT("xmedia_vgs_cancel_job failed !\n");
        handle = -1;
        return XMEDIA_FAILURE;
    }

    for (i = 0; i < kpt_num; i++) {
        x = (xmedia_s32)roundf(target_rect[i].x / 2) * 2;
        y = (xmedia_s32)roundf(target_rect[i].y / 2) * 2;

        x = (x < 0) ? 0 : ((x > 638) ? 638 : x);
        y = (y < 0) ? 0 : ((y > 358) ? 358 : y);

        cover[point_num].color = 0x0000CD;
        cover[point_num].cover_type = XMEDIA_VGS_COVER_TYPE_QUAD_RANGLE;
        cover[point_num].quadrangle.is_solid = XMEDIA_FALSE;
        cover[point_num].quadrangle.thick = 2;
        // 画2x2的小矩形标记关键点
        cover[point_num].quadrangle.points[0].x = x;
        cover[point_num].quadrangle.points[0].y = y;
        cover[point_num].quadrangle.points[1].x = (x + 2 > 640) ? 640 : x + 2; // 不超过640
        cover[point_num].quadrangle.points[1].y = y;
        cover[point_num].quadrangle.points[2].x = (x + 2 > 640) ? 640 : x + 2; // 不超过640
        cover[point_num].quadrangle.points[2].y = (y + 2 > 360) ? 360 : y + 2; // 不超过360
        cover[point_num].quadrangle.points[3].x = x;
        cover[point_num].quadrangle.points[3].y = (y + 2 > 360) ? 360 : y + 2; // 不超过360

        // 检查是否构成水平线或垂直线
        horizontal_line = (cover[point_num].quadrangle.points[0].y == cover[point_num].quadrangle.points[1].y &&
                           cover[point_num].quadrangle.points[1].y == cover[point_num].quadrangle.points[2].y &&
                           cover[point_num].quadrangle.points[2].y == cover[point_num].quadrangle.points[3].y);

        vertical_line = (cover[point_num].quadrangle.points[0].x == cover[point_num].quadrangle.points[1].x &&
                         cover[point_num].quadrangle.points[1].x == cover[point_num].quadrangle.points[2].x &&
                         cover[point_num].quadrangle.points[2].x == cover[point_num].quadrangle.points[3].x);

        if (horizontal_line || vertical_line) {
            continue;
        }
        point_num++;
    }

    memcpy(&task.img_in, frame, sizeof(xmedia_video_frame_info));
    memcpy(&task.img_out, frame, sizeof(xmedia_video_frame_info));

    ret = xmedia_vgs_add_task_cover(handle, &task, &cover[0], point_num);
    if (XMEDIA_SUCCESS != ret) {
        xmedia_vgs_cancel_job(handle);
        xmedia_vgs_exit();
        SAMPLE_PRT("xmedia_vgs_add_task_cover failed ! ret [%d] \n", ret);
        return XMEDIA_FAILURE;
    }

    ret = xmedia_vgs_submit_job(handle);
    if (XMEDIA_SUCCESS != ret) {
        xmedia_vgs_cancel_job(handle);
        xmedia_vgs_exit();
        SAMPLE_PRT("xmedia_vgs_submit_job failed ! ret [%d] \n", ret);
        return XMEDIA_FAILURE;
    }

    ret = xmedia_vgs_wait_job(handle, 2000);
    if (ret != XMEDIA_SUCCESS) {
        xmedia_vgs_exit();
        SAMPLE_PRT("xmedia_vgs_wait_job failed ! ret [%d] \n", ret);
        return ret;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_draw_line(xmedia_svp_keypoint* target_rect, xmedia_video_frame_info *frame)
{
    xmedia_s32 ret;
    xmedia_vgs_frame_info task_attr;
    xmedia_u32 line_num = 17;
    xmedia_u32 vgs_line_num = 0;
    xmedia_u32 line_num_process = 0;
    xmedia_vgs_line_attr vgs_line[17];
    xmedia_s32 handle = -1;
    xmedia_s32 i = 0;
    xmedia_u32 index1,index2;
    xmedia_u32 line_start[17][2];
    xmedia_u32 line_end[17][2];

    xmedia_s32 match_line[17][2] = {{0,1},{0,2},{1,3},{2,4},{3,5},{4,6},
                             {6,5},{5,7},{7,9},{6,8},{8,10},{6,12},
                             {12,14},{14,16},{5,11},{11,13},{13,15}};

    ret = xmedia_vgs_init();
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_vgs_init failed !\n");
        return ret;
    }

    memcpy(&task_attr.img_in,frame,sizeof(xmedia_video_frame_info));
    memcpy(&task_attr.img_out,frame,sizeof(xmedia_video_frame_info));

    for(i = 0;i < line_num; i++) {
        index1 = match_line[i][0];
        index2 = match_line[i][1];
        if (target_rect[index1].score >= 0.2 && target_rect[index2].score >= 0.2) {
            line_start[line_num_process][0] = (round(target_rect[index1].x) / 2);
            line_start[line_num_process][0] = line_start[line_num_process][0] * 2;
            line_start[line_num_process][1] = (round(target_rect[index1].y) / 2);
            line_start[line_num_process][1] = line_start[line_num_process][1] * 2;

            line_end[line_num_process][0] = (round(target_rect[index2].x) / 2);
            line_end[line_num_process][0] = line_end[line_num_process][0] * 2;
            line_end[line_num_process][1] = (round(target_rect[index2].y) / 2);
            line_end[line_num_process][1] = line_end[line_num_process][1] * 2;
            line_num_process++;
        }
    }

    if (line_num_process == 0) {
      SAMPLE_PRT("line_num_process is nullptr  don't draw\n");
      return XMEDIA_SUCCESS;
    }

    for(i = 0;i < line_num_process; i++) {
        if (line_start[i][0] == line_end[i][0] && line_start[i][1] == line_end[i][1])
            continue;
        vgs_line[vgs_line_num].start_point.x = line_start[i][0];
        vgs_line[vgs_line_num].start_point.y = line_start[i][1];
        vgs_line[vgs_line_num].end_point.x   = line_end[i][0];
        vgs_line[vgs_line_num].end_point.y   = line_end[i][1];
        vgs_line[vgs_line_num].thick          = 2;
        vgs_line[vgs_line_num].color          = 0xFF0000;
        vgs_line_num ++;
    }

    ret = xmedia_vgs_create_job(&handle);
    if (XMEDIA_SUCCESS != ret)
    {
        SAMPLE_PRT("xmedia_vgs_craete_job failed !\n");
        handle = -1;
        return XMEDIA_FAILURE;
    }

    ret = xmedia_vgs_add_task_line(handle, &task_attr, &vgs_line[0], vgs_line_num);
    if (XMEDIA_SUCCESS != ret)
    {
        SAMPLE_PRT("xmedia_vgs_add_task_line failed ! ret : %x\n", ret);
    }

    ret = xmedia_vgs_submit_job(handle);
    if (XMEDIA_SUCCESS != ret)
    {
        SAMPLE_PRT("xmedia_vgs_submit_job failed !\n");
        return XMEDIA_FAILURE;
    }

    ret = xmedia_vgs_wait_job(handle, 2000);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_vgs_wait_job failed ! ret [%d] \n", ret);
        return ret;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_void sample_svp_draw_thick_pixel(xmedia_void* img_addr, xmedia_s32 width, xmedia_s32 height,
    xmedia_s32 x, xmedia_s32 y, xmedia_s32 thickness)
{
    xmedia_u8 *addr = (xmedia_u8 *)img_addr;
    for(xmedia_s32 dy = -thickness/2; dy <= thickness/2; dy++) {
        for(xmedia_s32 dx = -thickness/2; dx <= thickness/2; dx++) {
            xmedia_s32 nx = x + dx;
            xmedia_s32 ny = y + dy;
            if(nx >=0 && nx < width && ny >=0 && ny < height) {
                  /*三组数字构成颜色为yuv的紫色*/
                  addr[ny * SVP_BIG_STREAM_W + nx] = 150;
                  addr[SVP_BIG_STREAM_W * SVP_BIG_STREAM_H + (ny / 2) * SVP_BIG_STREAM_W + nx / 2 * 2] = 44;
                  addr[SVP_BIG_STREAM_W * SVP_BIG_STREAM_H + (ny / 2) * SVP_BIG_STREAM_W + nx / 2 * 2 + 1] = 21;
            }
        }
    }
}

#ifdef READ_YUV
// 文件名比较函数（升序）
static xmedia_s32 compare_names(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

xmedia_s32 sample_get_input_yuv_data(xmedia_void *p, xmedia_s32 width, xmedia_s32 height, const xmedia_char *filename)
{

    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        SAMPLE_PRT("open input data file failed:%s \n", filename);
        return XMEDIA_FAILURE;
    }

    xmedia_s32 filesSize = height * width * 3 / 2;

    xmedia_s32 count = fread(p, sizeof(uint8_t), filesSize, fp);

    fclose(fp);

    if (count != filesSize) {
        SAMPLE_PRT("read %s error, want to read %d , in fact read %d\n", filename, filesSize, count);
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}
#endif

xmedia_s32 append_to_file(const xmedia_char* filename, const xmedia_char* content) {
    FILE* file = fopen(filename, "a");  // 追加模式
    if (file == XMEDIA_NULL) {
        SAMPLE_PRT("can not open file: %s\n", filename);
        return XMEDIA_FAILURE;
    }

    fputs(content, file);
    fputs("\n", file);
    fclose(file);
    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_draw_zone(xmedia_svp_zone* zone, xmedia_video_frame_info *frame)
{
    xmedia_s32 ret;
    xmedia_vgs_frame_info task_attr;
    xmedia_s32 handle = -1;
    xmedia_s32 i = 0, j = 0, line_idx = 0;
    xmedia_vgs_line_attr *vgs_line;
    xmedia_u32 line_num = 0;

    ret = xmedia_vgs_init();
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_vgs_init failed !\n");
        return ret;
    }

    memcpy(&task_attr.img_in,frame,sizeof(xmedia_video_frame_info));
    memcpy(&task_attr.img_out,frame,sizeof(xmedia_video_frame_info));

    for (i = 0; i < zone->zone_num; i++) {
        if (zone->zones[i].points_count >= 3) { // 小于3个点 无法构成封闭图形
            line_num += zone->zones[i].points_count;
        }
    }

    vgs_line = (xmedia_vgs_line_attr *)calloc(1, line_num * sizeof(xmedia_vgs_line_attr));
    if (vgs_line == XMEDIA_NULL) {
        SAMPLE_PRT("vgs_line calloc failed !\n");
        return XMEDIA_FAILURE;
    }

    for (i = 0; i < zone->zone_num; i++) {
        for (j = 0; j < zone->zones[i].points_count; j++) {
            if (zone->zones[i].points_count < 3) { // 小于3个点 无法构成封闭图形
                continue;
            }

            if (j == zone->zones[i].points_count - 1) {
                vgs_line[line_idx].start_point.x =
                    (xmedia_s32)(zone->zones[i].points_array[j].x * SVP_SMALL_STREAM_W / SVP_SCALE_RATIO) / 2 * 2;
                vgs_line[line_idx].start_point.y =
                    (xmedia_s32)(zone->zones[i].points_array[j].y * SVP_SMALL_STREAM_H / SVP_SCALE_RATIO) / 2 * 2;
                vgs_line[line_idx].end_point.x   =
                    (xmedia_s32)(zone->zones[i].points_array[0].x * SVP_SMALL_STREAM_W / SVP_SCALE_RATIO) / 2 * 2;
                vgs_line[line_idx].end_point.y   =
                    (xmedia_s32)(zone->zones[i].points_array[0].y * SVP_SMALL_STREAM_H / SVP_SCALE_RATIO) / 2 * 2;
            } else {
                vgs_line[line_idx].start_point.x =
                    (xmedia_s32)(zone->zones[i].points_array[j].x * SVP_SMALL_STREAM_W / SVP_SCALE_RATIO) / 2 * 2;
                vgs_line[line_idx].start_point.y =
                    (xmedia_s32)(zone->zones[i].points_array[j].y * SVP_SMALL_STREAM_H / SVP_SCALE_RATIO) / 2 * 2;
                vgs_line[line_idx].end_point.x   =
                    (xmedia_s32)(zone->zones[i].points_array[j + 1].x * SVP_SMALL_STREAM_W / SVP_SCALE_RATIO) / 2 * 2;
                vgs_line[line_idx].end_point.y   =
                    (xmedia_s32)(zone->zones[i].points_array[j + 1].y * SVP_SMALL_STREAM_H / SVP_SCALE_RATIO) / 2 * 2;
            }

            vgs_line[line_idx].color = 0x00FF00;
            vgs_line[line_idx].thick = 2;
            line_idx++;
        }
    }

    if (line_idx == 0) {
        SAMPLE_PRT("line_idx is zero  don't draw\n");
        free(vgs_line);
        return XMEDIA_SUCCESS;
    }

    ret = xmedia_vgs_create_job(&handle);
    if (XMEDIA_SUCCESS != ret) {
        SAMPLE_PRT("xmedia_vgs_craete_job failed !\n");
        handle = -1;
        free(vgs_line);
        return XMEDIA_FAILURE;
    }

    ret = xmedia_vgs_add_task_line(handle, &task_attr, vgs_line, line_idx);
    if (XMEDIA_SUCCESS != ret) {
        SAMPLE_PRT("xmedia_vgs_add_task_line failed ! ret : %x\n", ret);
    }

    ret = xmedia_vgs_submit_job(handle);
    if (XMEDIA_SUCCESS != ret) {
        SAMPLE_PRT("xmedia_vgs_submit_job failed !\n");
        free(vgs_line);
        return XMEDIA_FAILURE;
    }

    ret = xmedia_vgs_wait_job(handle, 2000);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_vgs_wait_job failed ! ret [%d] \n", ret);
        free(vgs_line);
        return ret;
    }

    free(vgs_line);

    return XMEDIA_SUCCESS;
}

xmedia_void* sample_svp_proc(xmedia_void* p)
{
    xmedia_s32 ret;
    sample_svp_info *svp_info = (sample_svp_info *)p;
    xmedia_video_frame_info video_frame;
    xmedia_video_frame_info video_frame_big;
    xmedia_s32 milli_sec = 20000;
    xmedia_svp_task_input task_input;
    xmedia_video_frame_info frame_info[2];

#ifdef READ_YUV
    xmedia_s32 SVP_YUV_W = SVP_SMALL_STREAM_W;
    xmedia_s32 SVP_YUV_H = SVP_SMALL_STREAM_H;
    // 720P分辨率输入
    if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_PERSON_RCNN_720P ||
        svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_CAR_RCNN_720P ||
        svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_PET_RCNN_720P) {
        SVP_YUV_W = SVP_MID_STREAM_W;
        SVP_YUV_H = SVP_MID_STREAM_H;
    } else if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_ADAS ||
        svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_GESTURE_RECOGNITION ||
        svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_FACE_EMOTION ||
        svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_FACE_RECOGNITION ||
        svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_PLATE_RECOGNITION ||
        svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_VEHICLE_RECOGNITION ||
        svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_PLATE_VEHICLE_RECO) {
        SVP_YUV_W = SVP_BIG_STREAM_W;
        SVP_YUV_H = SVP_BIG_STREAM_H;
    }

    DIR *dir;
    struct dirent *entry;
    xmedia_char *file_names[MAX_FILES_NUM];
    xmedia_s32 file_count = 0;
    xmedia_bool input_file_flag = XMEDIA_FALSE;


    // 打开目录并读取文件名
    xmedia_char *input_path = "./yuv_dir";
    dir = opendir(input_path);
    if (dir == NULL) {
        SAMPLE_PRT("opendir %s failed\n", input_path);
        return NULL;
    }

    while ((entry = readdir(dir)) != NULL && file_count < MAX_FILES_NUM) {
        if (entry->d_type == DT_REG || entry->d_type == DT_UNKNOWN) { // 只处理普通文件
            xmedia_char *ext = strrchr(entry->d_name, '.');
            if (ext && (strcmp(ext, ".yuv") == 0 || strcmp(ext, ".YUV") == 0)) {
                file_names[file_count] = malloc(strlen(entry->d_name) + 1);
                strcpy(file_names[file_count], entry->d_name);
                file_count++;
            }
        }
    }
    closedir(dir);
    // 按文件名排序
    qsort(file_names, file_count, sizeof(char *), compare_names);
    xmedia_s32 file_index = 0;
    input_file_flag = XMEDIA_TRUE;
#endif

#ifdef DMS_OPENCV
    sample_dms_state dms_state = { 0 };
    dms_state.distract = XMEDIA_FALSE;
    dms_state.tracker_id = -1;
    dms_state.warn_frame = 10;
#endif

    while (g_svp_start_flag == XMEDIA_TRUE) {
        ret = xmedia_vpss_acquire_ochn_frame(svp_info->vpss_pipe, svp_info->vpss_ochn[1], &video_frame, milli_sec);
        if (XMEDIA_SUCCESS != ret) {
            SAMPLE_PRT(" get vpss small frame failed !\n");
            continue;
        }

        task_input.frame_num = 1;
        if (svp_info->big_stream == XMEDIA_TRUE) {
            task_input.frame_num = 2;
            ret = xmedia_vpss_acquire_ochn_frame(svp_info->vpss_pipe, svp_info->vpss_ochn[2], &video_frame_big, milli_sec);
            if (XMEDIA_SUCCESS != ret) {
                SAMPLE_PRT(" get vpss big frame failed !\n");
                continue;
            }
        }

#ifdef READ_YUV
        if (input_file_flag) {
            xmedia_char full_path[MAX_STR_NAME_LEN];
            snprintf(full_path, sizeof(full_path), "%s/%s", input_path, file_names[file_index]);
            if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_ADAS ||
                svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_GESTURE_RECOGNITION ||
                svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_FACE_EMOTION ||
                svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_FACE_RECOGNITION ||
                svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_PLATE_RECOGNITION ||
                svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_VEHICLE_RECOGNITION ||
                svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_PLATE_VEHICLE_RECO) {
                xmedia_void* yuv_data = (xmedia_void*)xmedia_mmz_map(video_frame_big.frame.addr.y_phy_addr,
                    SVP_YUV_W * SVP_YUV_H * 3 / 2, XMEDIA_FALSE);
                sample_get_input_yuv_data(yuv_data, SVP_YUV_W, SVP_YUV_H, full_path);
                if (svp_info->detect_type != SAMPLE_SVP_ALG_TYPE_PLATE_RECOGNITION &&
                    svp_info->detect_type != SAMPLE_SVP_ALG_TYPE_VEHICLE_RECOGNITION &&
                    svp_info->detect_type != SAMPLE_SVP_ALG_TYPE_PLATE_VEHICLE_RECO) {
                    ret = sample_vgs_resize(&video_frame_big, &video_frame);
                }
                xmedia_mmz_unmap(yuv_data);
            } else {
                xmedia_void* yuv_data = (xmedia_void*)xmedia_mmz_map(video_frame.frame.addr.y_head_phy_addr,
                    SVP_YUV_W * SVP_YUV_H * 3 / 2, XMEDIA_FALSE);
                sample_get_input_yuv_data(yuv_data, SVP_YUV_W, SVP_YUV_H, full_path);
                xmedia_mmz_unmap(yuv_data);
            }
            SAMPLE_PRT("Processing file: %s\n", file_names[file_index]);
            free(file_names[file_index]);
            file_index++;
        }
#endif

        frame_info[0] = video_frame;
        if (svp_info->big_stream == XMEDIA_TRUE) {
            frame_info[1] = video_frame_big;
        }
        task_input.frame = frame_info;

        if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_AOV_PERSON) {
            xmedia_svp_aov_detect_output aov_result = {0};
#ifdef SAMPLE_TIME_DEBUG
        TIME_COST_START();
#endif
            ret = xmedia_svp_task_process(svp_info->svp_handle, &task_input, &aov_result);
#ifdef SAMPLE_TIME_DEBUG
        TIME_COST_END();
        TIME_COST_PRINT("svp process all");
#endif
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_venc_send_frame failed with %#x!\n", ret);
            }
            SAMPLE_PRT(" is target [%d] \n", aov_result.is_exist);
            if (aov_result.detect_output.target_num > 0) {
                xmedia_u32 w,h;
                xmedia_s32 x1, y1, x2, y2;
                xmedia_video_rect target_rect[XMEDIA_SVP_MAX_TARGET_NUM];

                for (xmedia_s32 i = 0; i < aov_result.detect_output.target_num; i++) {
                    x1 = (xmedia_s32)roundf(aov_result.detect_output.targets[i].rect.x1 / 2) * 2;
                    y1 = (xmedia_s32)roundf(aov_result.detect_output.targets[i].rect.y1 / 2) * 2;
                    x2 = (xmedia_s32)roundf(aov_result.detect_output.targets[i].rect.x2 / 2) * 2;
                    y2 = (xmedia_s32)roundf(aov_result.detect_output.targets[i].rect.y2 / 2) * 2;
                    w = ABS(x2 - x1);
                    h = ABS(y2 - y1);
                    target_rect[i].x = x1;
                    target_rect[i].y = y1;
                    target_rect[i].width = w;
                    target_rect[i].height = h;
                }
                sample_svp_draw(&target_rect[0], &video_frame, aov_result.detect_output);
            }
        } else if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_PERSON_KEYPOINT) {
            xmedia_svp_yolov8_output yolov8_result = {0};
            ret = xmedia_svp_task_process(svp_info->svp_handle, &task_input, &yolov8_result);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_venc_send_frame failed with %#x!\n", ret);
            }
            for (xmedia_s32 i = 0; i < yolov8_result.target_num; i++) {
                sample_draw_line(&yolov8_result.targets[i].kpt[0], &video_frame);
            }
        } else if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_ADAS) {
            xmedia_svp_adas_result adas_result = { 0 };
            ret = xmedia_svp_task_process(svp_info->svp_handle, &task_input, &adas_result);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_venc_send_frame failed with %#x!\n", ret);
            }
            xmedia_u32 w, h;
            xmedia_s32 x1, y1, x2, y2;
            xmedia_video_rect target_rect[XMEDIA_SVP_MAX_TARGET_NUM];
            xmedia_s32 x, y;
            xmedia_float y_min, y_max;
            xmedia_s32 step = 600;
            xmedia_float h_s;
            xmedia_float tmp_x, tmp_y;
            xmedia_void* p_yuv_data = (xmedia_void*)xmedia_mmz_map(video_frame_big.frame.addr.y_phy_addr,
                SVP_BIG_STREAM_W * SVP_BIG_STREAM_H * 3 / 2, XMEDIA_FALSE);

            for (xmedia_s32 i = 0; i < XMEDIA_SVP_LANE_MAX_NUM; i++) {
                if (adas_result.lane_fitted_point_num[i] > 0) {
                    switch (i) {
                    case 0:
                        y_min = adas_result.col_fitted_result[0][0].y;
                        y_max = adas_result.col_fitted_result[0][adas_result.lane_fitted_point_num[i] - 1].y;
                        for (int k = 0; k < adas_result.lane_fitted_point_num[i]; k++) {
                            y_min = fmin(y_min, adas_result.col_fitted_result[0][k].y);
                            y_max = fmax(y_max, adas_result.col_fitted_result[0][k].y);
                        }
                        h_s = y_max - y_min;
                        for (xmedia_s32 k = 0; k < step; k++) {
                            tmp_y = ((xmedia_float)k / step * h_s + y_min);
                            tmp_x = (xmedia_float)(adas_result.coeffs[i][2] * tmp_y * tmp_y + adas_result.coeffs[i][1]
                                * tmp_y + adas_result.coeffs[i][0]);
                            tmp_x = fmin(SVP_BIG_STREAM_W, fmax(0, tmp_x * 3));
                            tmp_y = fmin(SVP_BIG_STREAM_H, fmax(0, tmp_y * 3));
                            x = (xmedia_s32)round(tmp_x);
                            y = (xmedia_s32)round(tmp_y);
                            sample_svp_draw_thick_pixel(p_yuv_data, SVP_BIG_STREAM_W, SVP_BIG_STREAM_H, x, y, 6);
                        }
                        break;
                    case 1:
                        y_min = adas_result.row_fitted_result[0][0].y;
                        y_max = adas_result.row_fitted_result[0][adas_result.lane_fitted_point_num[i] - 1].y;
                        for (xmedia_s32 k = 0; k < adas_result.lane_fitted_point_num[i]; k++) {
                            y_min = fmin(y_min, adas_result.row_fitted_result[0][k].y);
                            y_max = fmax(y_max, adas_result.row_fitted_result[0][k].y);
                        }
                        h_s = y_max - y_min;
                        for (xmedia_s32 k = 0; k < step; k++) {
                            tmp_y = ((xmedia_float)k / step * h_s + y_min);
                            tmp_x = (xmedia_float)(adas_result.coeffs[i][2] * tmp_y * tmp_y + adas_result.coeffs[i][1]
                                * tmp_y + adas_result.coeffs[i][0]);
                            tmp_x = fmin(SVP_BIG_STREAM_W, fmax(0, tmp_x * 3));
                            tmp_y = fmin(SVP_BIG_STREAM_H, fmax(0, tmp_y * 3));
                            x = (xmedia_s32)round(tmp_x);
                            y = (xmedia_s32)round(tmp_y);
                            sample_svp_draw_thick_pixel(p_yuv_data, SVP_BIG_STREAM_W, SVP_BIG_STREAM_H, x, y, 6);
                        }
                        break;
                    case 2:
                        y_min = adas_result.row_fitted_result[1][0].y;
                        y_max = adas_result.row_fitted_result[1][adas_result.lane_fitted_point_num[i] - 1].y;
                        for (xmedia_s32 k = 0; k < adas_result.lane_fitted_point_num[i]; k++) {
                            y_min = fmin(y_min, adas_result.row_fitted_result[1][k].y);
                            y_max = fmax(y_max, adas_result.row_fitted_result[1][k].y);
                        }
                        h_s = y_max - y_min;
                        for (xmedia_s32 k = 0; k < step; k++) {
                            tmp_y = ((xmedia_float)k / step * h_s + y_min);
                            tmp_x = (xmedia_float)(adas_result.coeffs[i][2] * tmp_y * tmp_y + adas_result.coeffs[i][1]
                                * tmp_y + adas_result.coeffs[i][0]);
                            tmp_x = fmin(SVP_BIG_STREAM_W, fmax(0, tmp_x * 3));
                            tmp_y = fmin(SVP_BIG_STREAM_H, fmax(0, tmp_y * 3));
                            x = (xmedia_s32)round(tmp_x);
                            y = (xmedia_s32)round(tmp_y);
                            sample_svp_draw_thick_pixel(p_yuv_data, SVP_BIG_STREAM_W, SVP_BIG_STREAM_H, x, y, 6);
                        }
                        break;
                    case 3:
                        y_min = adas_result.col_fitted_result[1][0].y;
                        y_max = adas_result.col_fitted_result[1][adas_result.lane_fitted_point_num[i] - 1].y;
                        for (xmedia_s32 k = 0; k < adas_result.lane_fitted_point_num[i]; k++) {
                            y_min = fmin(y_min, adas_result.col_fitted_result[1][k].y);
                            y_max = fmax(y_max, adas_result.col_fitted_result[1][k].y);
                        }
                        h_s = y_max - y_min;
                        for (xmedia_s32 k = 0; k < step; k++) {
                            tmp_y = ((xmedia_float)k / step * h_s + y_min);
                            tmp_x = (xmedia_float)(adas_result.coeffs[i][2] * tmp_y * tmp_y + adas_result.coeffs[i][1]
                                * tmp_y + adas_result.coeffs[i][0]);
                            tmp_x = fmin(SVP_BIG_STREAM_W, fmax(0, tmp_x * 3));
                            tmp_y = fmin(SVP_BIG_STREAM_H, fmax(0, tmp_y * 3));
                            x = (xmedia_s32)round(tmp_x);
                            y = (xmedia_s32)round(tmp_y);
                            sample_svp_draw_thick_pixel(p_yuv_data, SVP_BIG_STREAM_W, SVP_BIG_STREAM_H, x, y, 6);
                        }
                        break;
                    }
                }
            }
            xmedia_mmz_unmap(p_yuv_data);

            if (adas_result.car_result.target_num > 0) {
                for (xmedia_s32 i = 0; i < adas_result.car_result.target_num; i++) {
                    x1 = (xmedia_s32)roundf(adas_result.car_result.targets[i].rect.x1 / 2) * 2 * 3;
                    y1 = (xmedia_s32)roundf(adas_result.car_result.targets[i].rect.y1 / 2) * 2 * 3;
                    x2 = (xmedia_s32)roundf(adas_result.car_result.targets[i].rect.x2 / 2) * 2 * 3;
                    y2 = (xmedia_s32)roundf(adas_result.car_result.targets[i].rect.y2 / 2) * 2 * 3;
                    w = ABS(x2 - x1);
                    h = ABS(y2 - y1);
                    target_rect[i].x = x1;
                    target_rect[i].y = y1;
                    target_rect[i].width = w;
                    target_rect[i].height = h;
                    adas_result.car_result.targets[i].class_type = XMEDIA_SVP_CLASS_TYPE_CAR;
#ifdef USE_OSD
                    if (adas_result.car_result.targets[i].special_target == XMEDIA_TRUE) {
                        xmedia_char format_untile_line[SAMPLE_MAX_STRING_NUM];
                        snprintf(format_untile_line, sizeof(format_untile_line), "d:%.1fcm",
                                 adas_result.car_result.targets[i].distance);
                        sample_target_osd(&video_frame_big, x1, y1, format_untile_line);
                    }
#endif
                }
                sample_svp_draw(&target_rect[0], &video_frame_big, adas_result.car_result);
            }

            if (adas_result.plate_result.target_num > 0) {
                for (xmedia_s32 i = 0; i < adas_result.plate_result.target_num; i++) {
                    x1 = (xmedia_s32)roundf(adas_result.plate_result.targets[i].rect.x1 / 2) * 2 * 3;
                    y1 = (xmedia_s32)roundf(adas_result.plate_result.targets[i].rect.y1 / 2) * 2 * 3;
                    x2 = (xmedia_s32)roundf(adas_result.plate_result.targets[i].rect.x2 / 2) * 2 * 3;
                    y2 = (xmedia_s32)roundf(adas_result.plate_result.targets[i].rect.y2 / 2) * 2 * 3;
                    w = ABS(x2 - x1);
                    h = ABS(y2 - y1);
                    target_rect[i].x = x1;
                    target_rect[i].y = y1;
                    target_rect[i].width = w;
                    target_rect[i].height = h;
                }
                sample_svp_draw(&target_rect[0], &video_frame_big, adas_result.plate_result);
            }

            if (adas_result.person_result.target_num > 0) {
                for (xmedia_s32 i = 0; i < adas_result.person_result.target_num; i++) {
                    x1 = (xmedia_s32)roundf(adas_result.person_result.targets[i].rect.x1 / 2) * 2 * 3;
                    y1 = (xmedia_s32)roundf(adas_result.person_result.targets[i].rect.y1 / 2) * 2 * 3;
                    x2 = (xmedia_s32)roundf(adas_result.person_result.targets[i].rect.x2 / 2) * 2 * 3;
                    y2 = (xmedia_s32)roundf(adas_result.person_result.targets[i].rect.y2 / 2) * 2 * 3;
                    w = ABS(x2 - x1);
                    h = ABS(y2 - y1);
                    target_rect[i].x = x1;
                    target_rect[i].y = y1;
                    target_rect[i].width = w;
                    target_rect[i].height = h;
                }
                sample_svp_draw(&target_rect[0], &video_frame_big, adas_result.person_result);
            }

            if (adas_result.nmv_result.target_num > 0) {
                for (xmedia_s32 i = 0; i < adas_result.nmv_result.target_num; i++) {
                    x1 = (xmedia_s32)roundf(adas_result.nmv_result.targets[i].rect.x1 / 2) * 2 * 3;
                    y1 = (xmedia_s32)roundf(adas_result.nmv_result.targets[i].rect.y1 / 2) * 2 * 3;
                    x2 = (xmedia_s32)roundf(adas_result.nmv_result.targets[i].rect.x2 / 2) * 2 * 3;
                    y2 = (xmedia_s32)roundf(adas_result.nmv_result.targets[i].rect.y2 / 2) * 2 * 3;
                    w = ABS(x2 - x1);
                    h = ABS(y2 - y1);
                    target_rect[i].x = x1;
                    target_rect[i].y = y1;
                    target_rect[i].width = w;
                    target_rect[i].height = h;
                }
                sample_svp_draw(&target_rect[0], &video_frame_big, adas_result.nmv_result);
            }
        } else if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_FIRESMOKE) {
            xmedia_svp_yolov8_output fire_result = { 0 };
            ret = xmedia_svp_task_process(svp_info->svp_handle, &task_input, &fire_result);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_venc_send_frame failed with %#x!\n", ret);
            }
            xmedia_u32 w, h;
            xmedia_s32 x1, y1, x2, y2;
            xmedia_video_rect target_rect[XMEDIA_SVP_MAX_TARGET_NUM];
            if (fire_result.target_num > 0) {
                for (xmedia_s32 i = 0; i < fire_result.target_num; i++) {
                    // if (fire_result->targets[i].class_type == XMEDIA_SVP_CLASS_TYPE_FIRE) {
                    //     // 模拟红外 默认IR_THRES 0.6f
                    //     xmedia_float ir_thres = 0.6;
                    //     if (ir_thres < IR_THRES) {
                    //         continue;
                    //     }
                    // }
                    x1 = (xmedia_s32)roundf(fire_result.targets[i].rect.x1 / 2) * 2;
                    y1 = (xmedia_s32)roundf(fire_result.targets[i].rect.y1 / 2) * 2;
                    x2 = (xmedia_s32)roundf(fire_result.targets[i].rect.x2 / 2) * 2;
                    y2 = (xmedia_s32)roundf(fire_result.targets[i].rect.y2 / 2) * 2;
                    w = ABS(x2 - x1);
                    h = ABS(y2 - y1);
                    target_rect[i].x = x1;
                    target_rect[i].y = y1;
                    target_rect[i].width = w;
                    target_rect[i].height = h;
                }
                sample_svp_draw_v8(&target_rect[0], &video_frame, fire_result);
            }
        } else if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_GESTURE_RECOGNITION) {
            xmedia_svp_gesture_result gesture_result = { 0 };
            ret = xmedia_svp_task_process(svp_info->svp_handle, &task_input, &gesture_result);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_venc_send_frame failed with %#x!\n", ret);
            }
            xmedia_u32 w, h;
            xmedia_s32 x1, y1, x2, y2;
            xmedia_video_rect target_rect[XMEDIA_SVP_MAX_TARGET_NUM];
            if (gesture_result.result.target_num > 0) {
                for (xmedia_s32 i = 0; i < gesture_result.result.target_num; i++) {
                    x1 = (xmedia_s32)roundf(gesture_result.result.targets[i].rect.x1 / 2) * 2;
                    y1 = (xmedia_s32)roundf(gesture_result.result.targets[i].rect.y1 / 2) * 2;
                    x2 = (xmedia_s32)roundf(gesture_result.result.targets[i].rect.x2 / 2) * 2;
                    y2 = (xmedia_s32)roundf(gesture_result.result.targets[i].rect.y2 / 2) * 2;
                    w = ABS(x2 - x1);
                    h = ABS(y2 - y1);
                    target_rect[i].x = x1;
                    target_rect[i].y = y1;
                    target_rect[i].width = w;
                    target_rect[i].height = h;
#ifdef USE_OSD
                    if (gesture_result.result.targets[i].class_type == XMEDIA_SVP_CLASS_TYPE_GESTURE_OK) {
                        xmedia_char format_untile_line[SAMPLE_MAX_STRING_NUM];
                        snprintf(format_untile_line, sizeof(format_untile_line), "Gesture is OK!!!");
                        sample_target_osd(&video_frame, x1, y1, format_untile_line);
                    } else if (gesture_result.result.targets[i].class_type == XMEDIA_SVP_CLASS_TYPE_GESTURE_THUMB) {
                        xmedia_char format_untile_line[SAMPLE_MAX_STRING_NUM];
                        snprintf(format_untile_line, sizeof(format_untile_line), "Gesture is Thumb!!!");
                        sample_target_osd(&video_frame, x1, y1, format_untile_line);
                    } else if (gesture_result.result.targets[i].class_type == XMEDIA_SVP_CLASS_TYPE_GESTURE_V) {
                        xmedia_char format_untile_line[SAMPLE_MAX_STRING_NUM];
                        snprintf(format_untile_line, sizeof(format_untile_line), "Gesture is V!!!");
                        sample_target_osd(&video_frame, x1, y1, format_untile_line);
                    } else if (gesture_result.result.targets[i].class_type == XMEDIA_SVP_CLASS_TYPE_GESTURE_PALM) {
                        xmedia_char format_untile_line[SAMPLE_MAX_STRING_NUM];
                        snprintf(format_untile_line, sizeof(format_untile_line), "Gesture is Palm!!!");
                        sample_target_osd(&video_frame, x1, y1, format_untile_line);
                    }
#endif
                    ret = sample_svp_draw_keypoint(&gesture_result.result.targets[i].kpt[0],
                                                   GESTURE_KEYPOINT_LEN,
                                                   &video_frame);
                    if (ret != XMEDIA_SUCCESS) {
                        SAMPLE_PRT("sample_svp_draw_keypoint failed with %#x!\n", ret);
                    }
                }
                sample_svp_draw_v8(&target_rect[0], &video_frame, gesture_result.result);
            }
        } else if(svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_FACE_EMOTION) {
            xmedia_svp_face_attribute_output attribute_output = {0};
            ret = xmedia_svp_task_process(svp_info->svp_handle, &task_input, &attribute_output);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_svp_task_process failed with %#x!\n", ret);
            }
            if (attribute_output.face_num > 0) {
                xmedia_u32 w,h;
                xmedia_s32 x1, y1, x2, y2;
                xmedia_video_rect target_rect[XMEDIA_SVP_MAX_TARGET_NUM];

                for (xmedia_s32 i = 0; i < attribute_output.face_num; i++) {
                    x1 = (xmedia_s32)roundf(attribute_output.attribute_result[i].rect.x1 / 2) * 2;
                    y1 = (xmedia_s32)roundf(attribute_output.attribute_result[i].rect.y1 / 2) * 2;
                    x2 = (xmedia_s32)roundf(attribute_output.attribute_result[i].rect.x2 / 2) * 2;
                    y2 = (xmedia_s32)roundf(attribute_output.attribute_result[i].rect.y2 / 2) * 2;
                    w = ABS(x2 - x1);
                    h = ABS(y2 - y1);
                    target_rect[i].x = x1;
                    target_rect[i].y = y1;
                    target_rect[i].width = w;
                    target_rect[i].height = h;
#ifdef USE_OSD
                    xmedia_char str_array[SAMPLE_MAX_STRING_NUM];
                    if (attribute_output.attribute_result[i].emotion_class == XMEDIA_SVP_CLASS_EMOTION_SMILE) {
                        snprintf(str_array, sizeof(str_array), "id[%d] | %s",
                            attribute_output.attribute_result[i].face_tracker_id, "smile");
                    } else if (attribute_output.attribute_result[i].emotion_class == XMEDIA_SVP_CLASS_EMOTION_NORMAL) {
                        snprintf(str_array, sizeof(str_array), "id[%d] | %s",
                            attribute_output.attribute_result[i].face_tracker_id, "normal");
                    } else {
                        snprintf(str_array, sizeof(str_array), "id[%d] | %s",
                            attribute_output.attribute_result[i].face_tracker_id, "unknown");
                    }
                    sample_target_osd(&video_frame, x1, y1, str_array);
#endif
                }
                xmedia_svp_yolov5_output result = {0};
                result.target_num = attribute_output.face_num;
                sample_svp_draw(&target_rect[0], &video_frame, result);
            }
        } else if(svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_FACE_RECOGNITION) {
            xmedia_svp_fr_output fr_output = {0};
            db_match_result match_result = {0};
            ret = xmedia_svp_task_process(svp_info->svp_handle, &task_input, &fr_output);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_svp_task_process failed with %#x!\n", ret);
            }
            if (svp_info->add_flag == XMEDIA_FALSE && svp_info->delete_flag == XMEDIA_FALSE) {
                xmedia_u32 w,h;
                xmedia_s32 x1, y1, x2, y2;
                xmedia_video_rect target_rect[XMEDIA_SVP_MAX_TARGET_NUM];

                for (xmedia_s32 i = 0; i < fr_output.face_num; i++) {
                    x1 = (xmedia_s32)roundf(fr_output.fr_result[i].rect.x1 / 2) * 2;
                    y1 = (xmedia_s32)roundf(fr_output.fr_result[i].rect.y1 / 2) * 2;
                    x2 = (xmedia_s32)roundf(fr_output.fr_result[i].rect.x2 / 2) * 2;
                    y2 = (xmedia_s32)roundf(fr_output.fr_result[i].rect.y2 / 2) * 2;
                    w = ABS(x2 - x1);
                    h = ABS(y2 - y1);
                    target_rect[i].x = x1;
                    target_rect[i].y = y1;
                    target_rect[i].width = w;
                    target_rect[i].height = h;
                }
                xmedia_svp_yolov5_output result = {0};
                result.target_num = fr_output.face_num;
                sample_svp_draw(&target_rect[0], &video_frame, result);

                ret = svp_get_match_result(svp_info->g_db_handle, &fr_output, &match_result);
                if (ret != XMEDIA_SUCCESS) {
                    SAMPLE_PRT("svp_get_match_result failed %#x!\n", ret);
                }

                xmedia_s32 draw_index = 0;
                if (match_result.match_num != 0) {
                    for (xmedia_s32 match_index = 0; match_index < match_result.face_num; match_index++) {
                        if (strcmp(match_result.match_name[match_index], "") != 0) { // this index got name
#ifndef USE_OSD
                            SAMPLE_PRT("----------------- finish fr_process, got name :[%s] -----------------\n",
                                match_result.match_name[match_index]);
#endif
                            // draw the box that match name
                            x1 = (xmedia_s32)roundf(fr_output.fr_result[match_index].rect.x1 / 2) * 2;
                            y1 = (xmedia_s32)roundf(fr_output.fr_result[match_index].rect.y1 / 2) * 2;
                            x2 = (xmedia_s32)roundf(fr_output.fr_result[match_index].rect.x2 / 2) * 2;
                            y2 = (xmedia_s32)roundf(fr_output.fr_result[match_index].rect.y2 / 2) * 2;
                            w = abs(x2 - x1);
                            h = abs(y2 - y1);
                            target_rect[draw_index].x = x1;
                            target_rect[draw_index].y = y1;
                            target_rect[draw_index].width = w;
                            target_rect[draw_index].height = h;
                            draw_index++;
#ifdef USE_OSD
                            sample_target_osd(&video_frame, x1, y1, match_result.match_name[match_index]);
#endif
                        }
                    }
                }else {
                    // SAMPLE_PRT("this frame have not face\n");
                }
            }

            // add someone, one frame only have a face!
            if (svp_info->add_flag && fr_output.face_num != 0) {
                ret = svp_get_match_result(svp_info->g_db_handle, &fr_output, &match_result);
                SAMPLE_PRT("this frame face_num: %d, match_num:%d\n", match_result.face_num, match_result.match_num);
                if (match_result.match_num != 0) {
                    for (xmedia_s32 match_index = 0; match_index < match_result.face_num; match_index++) {
                        if (strcmp(match_result.match_name[match_index], "") != 0) {
                            SAMPLE_PRT("人脸已存在，无法录入 :%s\n", match_result.match_name[match_index]);
                        }
                    }
                }
                if (match_result.match_num == 0) {
                    add_person_info person_info;
                    person_info.add_person_name = svp_info->add_name;

                    for (xmedia_s32 add_fea_index = 0; add_fea_index < XMEDIA_SVP_FEATURE_COL; add_fea_index++) {
                        person_info.add_person_feature[add_fea_index] = fr_output.fr_result[0].vector[add_fea_index];
                    }

                    ret = svp_add_someone_to_database(svp_info->g_db_handle, &person_info);
                    if (ret == XMEDIA_SUCCESS) {
                        SAMPLE_PRT(" add someone to database success %#x!\n", ret);
                    } else {
                        SAMPLE_PRT(" add someone to database failed %#x!\n", ret);
                    }
                }
                SAMPLE_PRT("please input q to quit!\n");
                g_svp_start_flag = XMEDIA_FALSE;
            }

            // delete someone
            if (svp_info->delete_flag) {
                ret = svp_delete_someone_from_database(svp_info->g_db_handle, svp_info->delete_name);
                if (ret != XMEDIA_SUCCESS) {
                    SAMPLE_PRT("  delete someone from databasae failed %#x!\n", ret);
                }

                SAMPLE_PRT("please input q to quit!\n");
                g_svp_start_flag = XMEDIA_FALSE;
            }
        } else if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_COMPANION_ROBOT) {
            xmedia_svp_companion_robot_output robot_output = {0};
            ret = xmedia_svp_task_process(svp_info->svp_handle, &task_input, &robot_output);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_venc_send_frame failed with %#x!\n", ret);
            }

            if (robot_output.target_num > 0) {
                xmedia_u32 w,h;
                xmedia_s32 x1, y1, x2, y2;
                xmedia_video_rect target_rect[XMEDIA_SVP_MAX_TARGET_NUM];

                for (xmedia_s32 i = 0; i < robot_output.target_num; i++) {
                    x1 = (xmedia_s32)roundf(robot_output.targets[i].rect.x1 / 2) * 2;
                    y1 = (xmedia_s32)roundf(robot_output.targets[i].rect.y1 / 2) * 2;
                    x2 = (xmedia_s32)roundf(robot_output.targets[i].rect.x2 / 2) * 2;
                    y2 = (xmedia_s32)roundf(robot_output.targets[i].rect.y2 / 2) * 2;
                    w = ABS(x2 - x1);
                    h = ABS(y2 - y1);
                    target_rect[i].x = x1;
                    target_rect[i].y = y1;
                    target_rect[i].width = w;
                    target_rect[i].height = h;
#ifdef USE_OSD
                    xmedia_char format_untile_line[SAMPLE_MAX_STRING_NUM];
                    snprintf(format_untile_line, sizeof(format_untile_line), "id[%d]s[%.2f]c[%.2f]d[%.2f]",
                    robot_output.targets[i].tracker_id, robot_output.targets[i].detect_score,
                    robot_output.targets[i].classfier_score, robot_output.targets[i].distance);
                    sample_target_osd(&video_frame, x1, y1, format_untile_line);
#endif
                }
                xmedia_svp_yolov5_output yolov5_result = {0};
                yolov5_result.target_num = robot_output.target_num;
                sample_svp_draw(&target_rect[0], &video_frame, yolov5_result);
            }
        } else if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_DMS) {
            xmedia_svp_dms_output dms_result = { 0 };
            ret = xmedia_svp_task_process(svp_info->svp_handle, &task_input, &dms_result);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_venc_send_frame failed with %#x!\n", ret);
            }
            if (dms_result.target_num > 0) {
                xmedia_s32 i = 0;
#ifdef USE_OSD
                xmedia_s32 x1, y1, j = 0;
#endif
                for (i = 0; i < dms_result.target_num; i++) {
                    if (dms_result.targets[i].special_target == XMEDIA_TRUE) {
#ifdef DMS_OPENCV
                        if (dms_result.targets[i].tracker_id != dms_state.tracker_id) {
                            dms_state.distract_frame = 0;
                            dms_state.distract = XMEDIA_FALSE;
                            dms_state.tracker_id = dms_result.targets[i].tracker_id;
                        }
                        // 姿态计算
                        svp_euler_angles angles = { 0 };
                        ret = svp_opencv_face_orientation(dms_result.targets[i].kpt, &angles);
                        dms_state.pitch = (xmedia_float)angles.pitch;
                        dms_state.yaw = (xmedia_float)angles.yaw;
                        dms_state.roll = (xmedia_float)angles.roll;
                        // 分心判断逻辑
                        if (dms_state.pitch >= 20 ||
                            dms_state.pitch <= -20 ||
                            dms_state.roll >= 20 ||
                            dms_state.roll <= -20 ||
                            dms_state.yaw >= 20 ||
                            dms_state.yaw <= -20) {
                            if (dms_state.distract == XMEDIA_FALSE &&
                                dms_state.distract_frame >= dms_state.warn_frame) {
                                dms_state.distract = XMEDIA_TRUE;
                            }
                            if (dms_state.distract_frame < dms_state.warn_frame) {
                                dms_state.distract_frame++;
                            }
                        } else {
                            if (dms_state.distract_frame <= 0) {
                                dms_state.distract = XMEDIA_FALSE;
                            }
                            if (dms_state.distract_frame > 0) {
                                dms_state.distract_frame--;
                            }
                        }
#endif
#ifdef USE_OSD
                        x1 = (xmedia_s32)roundf(dms_result.targets[i].rect.x1 / 2) * 2;
                        y1 = (xmedia_s32)roundf(dms_result.targets[i].rect.y1 / 2) * 2;
                        xmedia_u8 offset = 0;
                        xmedia_char format_untile_line[SAMPLE_MAX_STRING_NUM];
                        snprintf(format_untile_line, sizeof(format_untile_line), "ID[%d]SCR[%.2f]",
                            dms_result.targets[i].tracker_id,
                            dms_result.targets[i].detect_score);
                        sample_target_osd(&video_frame, x1, y1, format_untile_line);
                        offset += 16;
                        if (dms_result.targets[i].dms_report.fatigue_flag == XMEDIA_TRUE) {
                            xmedia_char format_untile_line[SAMPLE_MAX_STRING_NUM];
                            snprintf(format_untile_line, sizeof(format_untile_line), "fatigue");
                            sample_target_osd(&video_frame, x1, y1 + offset, format_untile_line);
                            offset += 16;
                        }
                        if (dms_state.distract == XMEDIA_TRUE) {
                            xmedia_char format_untile_line[SAMPLE_MAX_STRING_NUM];
                            snprintf(format_untile_line, sizeof(format_untile_line), "distract");
                            sample_target_osd(&video_frame, x1, y1 + offset, format_untile_line);
                            offset += 16;
                        }
                        for (j = 0; j < dms_result.targets[i].dms_report.phone_num; j++) {
                            if (dms_result.targets[i].dms_report.phone[j].flag == XMEDIA_TRUE) {
                                xmedia_char format_untile_line[SAMPLE_MAX_STRING_NUM];
                                snprintf(format_untile_line, sizeof(format_untile_line), "phone");
                                sample_target_osd(&video_frame, x1, y1 + offset, format_untile_line);
                                offset += 16;
                            }
                        }
                        for (j = 0; j < dms_result.targets[i].dms_report.cigar_num; j++) {
                            if (dms_result.targets[i].dms_report.cigar[j].flag == XMEDIA_TRUE) {
                                xmedia_char format_untile_line[SAMPLE_MAX_STRING_NUM];
                                snprintf(format_untile_line, sizeof(format_untile_line), "smoke");
                                sample_target_osd(&video_frame, x1, y1 + offset, format_untile_line);
                            }
                        }
#endif
                    }
                    ret = sample_svp_draw_keypoint(&dms_result.targets[i].kpt[0],
                        MAX_DETECT_KEYPOINT_NUM,
                        &video_frame);
                    if (ret != XMEDIA_SUCCESS) {
                        SAMPLE_PRT("sample_svp_draw_keypoint failed with %#x!\n", ret);
                    }
                }
                sample_svp_draw_dms(&dms_result, &video_frame);
            }
        } else if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_PLATE_RECOGNITION) {
            xmedia_svp_plate_reco_output plate_output = {0};
            ret = xmedia_svp_task_process(svp_info->svp_handle, &task_input, &plate_output);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_svp_task_process failed with %#x!\n", ret);
            }

            xmedia_u32 w,h;
            xmedia_s32 x1, y1, x2, y2;
            xmedia_video_rect target_rect[XMEDIA_SVP_MAX_TARGET_NUM];
            xmedia_svp_yolov5_output draw_output;
            xmedia_float scale_x = (xmedia_float)SVP_SMALL_STREAM_W / video_frame_big.frame.width;
            xmedia_float scale_y = (xmedia_float)SVP_SMALL_STREAM_H / video_frame_big.frame.height;
            xmedia_svp_plate_reco_attr plate_attr;
            ret = xmedia_svp_task_get_attr(svp_info->svp_handle, &plate_attr);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_svp_task_get_attr failed with %#x!\n", ret);
            }

            sample_draw_zone(&plate_attr.detect_zone, &video_frame);
            for (xmedia_s32 i = 0; i < plate_output.target_num; i++) {
                append_to_file("./plate_info.log", plate_output.targets[i].plate_char);
                SAMPLE_PRT("char: %s score:%.2f color: %d! color_score: %.2f direction: %d [0: forward 1:backward]\n",
                    plate_output.targets[i].plate_char,
                    plate_output.targets[i].char_score,
                    plate_output.targets[i].color,
                    plate_output.targets[i].color_score,
                    plate_output.targets[i].lpr_direction);
                x1 = (xmedia_s32)roundf(plate_output.targets[i].rect.x1 * scale_x / 2) * 2;
                y1 = (xmedia_s32)roundf(plate_output.targets[i].rect.y1 * scale_y / 2) * 2;
                x2 = (xmedia_s32)roundf(plate_output.targets[i].rect.x2 * scale_x / 2) * 2;
                y2 = (xmedia_s32)roundf(plate_output.targets[i].rect.y2 * scale_y / 2) * 2;
                w = ABS(x2 - x1);
                h = ABS(y2 - y1);
                target_rect[i].x = x1;
                target_rect[i].y = y1;
                target_rect[i].width = w;
                target_rect[i].height = h;
                draw_output.targets[i].rect = plate_output.targets[i].rect;
#ifdef USE_OSD
                xmedia_char format_untile_line[SAMPLE_MAX_STRING_NUM];
                snprintf(format_untile_line, sizeof(format_untile_line), "id[%d]scr[%.2f]cls[%.2f]mv[%d]",
                    plate_output.targets[i].tracker_id,
                    plate_output.targets[i].detect_score,
                    plate_output.targets[i].classfier_score,
                    plate_output.targets[i].motion_state);
                sample_target_osd(&video_frame, x1, y1, format_untile_line);
#endif
            }
            draw_output.target_num = plate_output.target_num;
            sample_svp_draw(&target_rect[0], &video_frame, draw_output);

        }  else if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_VEHICLE_RECOGNITION) {
            xmedia_svp_vehicle_reco_output veh_output = {0};
            ret = xmedia_svp_task_process(svp_info->svp_handle, &task_input, &veh_output);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_svp_task_process failed with %#x!\n", ret);
            }

        {
            xmedia_u32 w,h;
            xmedia_s32 x1, y1, x2, y2;
            xmedia_video_rect target_rect[XMEDIA_SVP_MAX_TARGET_NUM];
            xmedia_svp_yolov5_output draw_output;
            xmedia_float scale_x = (xmedia_float)SVP_SMALL_STREAM_W / video_frame_big.frame.width;
            xmedia_float scale_y = (xmedia_float)SVP_SMALL_STREAM_H / video_frame_big.frame.height;
            for (xmedia_s32 i = 0; i < veh_output.target_num; i++) {
//                SAMPLE_PRT("id:[%d] x:[%.2f] y[%.2f]\n", result.targets[i].tracker_id,
//                     result.targets[i].rect.x1,result.targets[i].rect.y1);
                x1 = (xmedia_s32)roundf(veh_output.targets[i].rect.x1 * scale_x / 2) * 2;
                y1 = (xmedia_s32)roundf(veh_output.targets[i].rect.y1 * scale_y / 2) * 2;
                x2 = (xmedia_s32)roundf(veh_output.targets[i].rect.x2 * scale_x / 2) * 2;
                y2 = (xmedia_s32)roundf(veh_output.targets[i].rect.y2 * scale_y / 2) * 2;
                w = ABS(x2 - x1);
                h = ABS(y2 - y1);
                target_rect[i].x = x1;
                target_rect[i].y = y1;
                target_rect[i].width = w;
                target_rect[i].height = h;
                draw_output.targets[i].rect = veh_output.targets[i].rect;
                 SAMPLE_PRT("tracker_id[%d]x1.y1:[%d,%d] x2,y2:[%d,%d] w[%d] h[%d]\n",
                    veh_output.targets[i].tracker_id, x1, y1, x2, y2, w, h);
#ifdef USE_OSD
                xmedia_char format_untile_line[SAMPLE_MAX_STRING_NUM];
                snprintf(format_untile_line, sizeof(format_untile_line), "id[%d]scr[%.2f]cls[%.2f]mv[%d]",
                    veh_output.targets[i].tracker_id,
                    veh_output.targets[i].detect_score,
                    veh_output.targets[i].classfier_score,
                    veh_output.targets[i].motion_state);
                sample_target_osd(&video_frame, x1, y1, format_untile_line);
#endif

                if (veh_output.targets[i].color == XMEDIA_SVP_VEHICLE_COLOR_WHITE) {
                    SAMPLE_PRT("WHITE score:%.2f\n", veh_output.targets[i].color_scr);
                } else if (veh_output.targets[i].color == XMEDIA_SVP_VEHICLE_COLOR_RED) {
                    SAMPLE_PRT("RED score:%.2f\n", veh_output.targets[i].color_scr);
                } else if (veh_output.targets[i].color == XMEDIA_SVP_VEHICLE_COLOR_YELLOW) {
                    SAMPLE_PRT("YELLOW score:%.2f\n", veh_output.targets[i].color_scr);
                } else if (veh_output.targets[i].color == XMEDIA_SVP_VEHICLE_COLOR_BLACK) {
                    SAMPLE_PRT("BLACK score:%.2f\n", veh_output.targets[i].color_scr);
                } else if (veh_output.targets[i].color == XMEDIA_SVP_VEHICLE_COLOR_BLUE) {
                    SAMPLE_PRT("BLUE score:%.2f\n", veh_output.targets[i].color_scr);
                } else if (veh_output.targets[i].color == XMEDIA_SVP_VEHICLE_COLOR_GREEN) {
                    SAMPLE_PRT("GREEN score:%.2f\n", veh_output.targets[i].color_scr);
                } else {
                    SAMPLE_PRT("unkown score:%.2f\n", veh_output.targets[i].color_scr);
                }

                if (veh_output.targets[i].type == XMEDIA_SVP_VEHICLE_TYPE_SEDAN) {
                    SAMPLE_PRT("小车 score:%.2f\n", veh_output.targets[i].type_scr);
                } else if (veh_output.targets[i].type == XMEDIA_SVP_VEHICLE_TYPE_MINIVAN) {
                    SAMPLE_PRT("小货车 score:%.2f\n", veh_output.targets[i].type_scr);
                } else if (veh_output.targets[i].type == XMEDIA_SVP_VEHICLE_TYPE_TRUCK) {
                    SAMPLE_PRT("卡车 score:%.2f\n", veh_output.targets[i].type_scr);
                } else if (veh_output.targets[i].type == XMEDIA_SVP_VEHICLE_TYPE_TANKER) {
                    SAMPLE_PRT("罐车 score:%.2f\n", veh_output.targets[i].type_scr);
                } else if (veh_output.targets[i].type == XMEDIA_SVP_VEHICLE_TYPE_MINIBUS) {
                    SAMPLE_PRT("巴士 score:%.2f\n", veh_output.targets[i].type_scr);
                } else {
                    SAMPLE_PRT("未知 score:%.2f\n", veh_output.targets[i].type_scr);
                }
                SAMPLE_PRT("vehicle_direction[%d] {0:forward 1:backward}\n", veh_output.targets[i].vehicle_direction);
            }
            draw_output.target_num = veh_output.target_num;
            sample_svp_draw(&target_rect[0], &video_frame, draw_output);
        }

        } else if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_PLATE_VEHICLE_RECO) {
            xmedia_svp_plate_vehicle_output pv_output = {0};
            ret = xmedia_svp_task_process(svp_info->svp_handle, &task_input, &pv_output);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_svp_task_process failed with %#x!\n", ret);
            }

            xmedia_u32 w,h;
            xmedia_s32 x1, y1, x2, y2;
            xmedia_video_rect target_rect[XMEDIA_SVP_MAX_TARGET_NUM];
            xmedia_svp_yolov5_output draw_output;
            xmedia_float scale_x = (xmedia_float)SVP_SMALL_STREAM_W / video_frame_big.frame.width;
            xmedia_float scale_y = (xmedia_float)SVP_SMALL_STREAM_H / video_frame_big.frame.height;
            for (xmedia_s32 i = 0; i < pv_output.target_num; i++) {
                append_to_file("./plate_vehicle_info.log", pv_output.targets[i].plate_char);
                SAMPLE_PRT("char: %s score:%.2f color: %d! color_type: %.2f direction: %d [0: forward 1:backward]\n",
                    pv_output.targets[i].plate_char,
                    pv_output.targets[i].plate_score,
                    pv_output.targets[i].plate_color,
                    pv_output.targets[i].plate_color_scr,
                    pv_output.targets[i].plate_direction);
                SAMPLE_PRT("[car_type: %d score:%.2f] [color: %d score: %.2f] direction: %d [0: forward 1:backward]\n",
                    pv_output.targets[i].veh_type,
                    pv_output.targets[i].veh_type_scr,
                    pv_output.targets[i].veh_color,
                    pv_output.targets[i].veh_color_scr,
                    pv_output.targets[i].veh_direction);
                x1 = (xmedia_s32)roundf(pv_output.targets[i].plate_rect.x1 * scale_x / 2) * 2;
                y1 = (xmedia_s32)roundf(pv_output.targets[i].plate_rect.y1 * scale_y / 2) * 2;
                x2 = (xmedia_s32)roundf(pv_output.targets[i].plate_rect.x2 * scale_x / 2) * 2;
                y2 = (xmedia_s32)roundf(pv_output.targets[i].plate_rect.y2 * scale_y / 2) * 2;
                w = ABS(x2 - x1);
                h = ABS(y2 - y1);
                target_rect[i].x = x1;
                target_rect[i].y = y1;
                target_rect[i].width = w;
                target_rect[i].height = h;
                draw_output.targets[i].rect = pv_output.targets[i].plate_rect;
                // SAMPLE_PRT("sample_num[%d]x1.y1:[%d,%d] x2,y2:[%d,%d] w[%d] h[%d]\n", i, x1, y1, x2, y2, w, h);
#ifdef USE_OSD
                xmedia_char format_untile_line[SAMPLE_MAX_STRING_NUM];
                snprintf(format_untile_line, sizeof(format_untile_line), "id[%d]char[%s]",
                    pv_output.targets[i].tracker_id,
                    pv_output.targets[i].plate_char);
                sample_target_osd(&video_frame, x1, y1, format_untile_line);
#endif
            }
            draw_output.target_num = pv_output.target_num;
            sample_svp_draw(&target_rect[0], &video_frame, draw_output);
        } else {
            xmedia_svp_yolov5_output result = { 0 };
#ifdef SAMPLE_TIME_DEBUG
            TIME_COST_START();
#endif
            ret = xmedia_svp_task_process(svp_info->svp_handle, &task_input, &result);
#ifdef SAMPLE_TIME_DEBUG
            TIME_COST_END();
            TIME_COST_PRINT("svp process all");
#endif
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_venc_send_frame failed with %#x!\n", ret);
            }
#if 0
    SAMPLE_PRT(" target num :[%d] \n", result.target_num);
    if (result.target_num > 0) {
       for (xmedia_s32 i = 0; i < result.target_num; i++) {
          int x1 = (xmedia_s32)roundf(result.targets[i].rect.x1 / 2) * 2;
          int y1 = (xmedia_s32)roundf(result.targets[i].rect.y1 / 2) * 2;
          int x2 = (xmedia_s32)roundf(result.targets[i].rect.x2 / 2) * 2;
          int y2 = (xmedia_s32)roundf(result.targets[i].rect.y2 / 2) * 2;
          SAMPLE_PRT("rect x1[%d]y1[%d]x2[%d]y2[%d]s[%.2f]\n",x1,y1,x2,y2,result.targets[i].detect_score);
       }
    }
#endif
            if (result.target_num > 0) {
                xmedia_u32 w,h;
                xmedia_s32 x1, y1, x2, y2;
                xmedia_video_rect target_rect[XMEDIA_SVP_MAX_TARGET_NUM];

                for (xmedia_s32 i = 0; i < result.target_num; i++) {
                //    printf("id:[%d] x:[%.2f] y[%.2f]\n", result.targets[i].tracker_id,
                //         result.targets[i].rect.x1,result.targets[i].rect.y1);
                    x1 = (xmedia_s32)roundf(result.targets[i].rect.x1 / 2) * 2;
                    y1 = (xmedia_s32)roundf(result.targets[i].rect.y1 / 2) * 2;
                    x2 = (xmedia_s32)roundf(result.targets[i].rect.x2 / 2) * 2;
                    y2 = (xmedia_s32)roundf(result.targets[i].rect.y2 / 2) * 2;
                    w = ABS(x2 - x1);
                    h = ABS(y2 - y1);
                    target_rect[i].x = x1;
                    target_rect[i].y = y1;
                    target_rect[i].width = w;
                    target_rect[i].height = h;
                    // printf("sample_num[%d]x1.y1:[%d,%d] x2,y2:[%d,%d] w[%d] h[%d]\n", i, x1, y1, x2, y2, w, h);
#ifdef USE_OSD
                    xmedia_char format_untile_line[SAMPLE_MAX_STRING_NUM];
                    snprintf(format_untile_line, sizeof(format_untile_line), "id[%d]scr[%.2f]cls[%.2f]mv[%d]",
                        result.targets[i].tracker_id,
                        result.targets[i].detect_score,
                        result.targets[i].classfier_score,
                        result.targets[i].motion_state);
                    sample_target_osd(&video_frame, x1, y1, format_untile_line);
#endif
                }
                sample_svp_draw(&target_rect[0], &video_frame, result);
            }
        }
        ret = xmedia_venc_send_frame(svp_info->venc_chn[1], &video_frame, milli_sec);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("xmedia_venc_send_frame failed with %#x!\n", ret);
        }

        if (svp_info->big_stream == XMEDIA_TRUE) {
            ret = xmedia_venc_send_frame(svp_info->venc_chn[2], &video_frame_big, milli_sec);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_venc_send_frame failed with %#x!\n", ret);
            }
        }

        ret = xmedia_vpss_release_ochn_frame(svp_info->vpss_pipe, svp_info->vpss_ochn[1], &video_frame);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("xmedia_vpss_release_ochn_frame failed with %#x!\n", ret);
        }
        if (svp_info->big_stream == XMEDIA_TRUE) {
            ret = xmedia_vpss_release_ochn_frame(svp_info->vpss_pipe, svp_info->vpss_ochn[2], &video_frame_big);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_vpss_release_ochn_frame failed with %#x!\n", ret);
            }
        }
        memset(&video_frame, 0, sizeof(xmedia_video_frame_info));
        memset(&video_frame_big, 0, sizeof(xmedia_video_frame_info));
#ifdef READ_YUV
        if (input_file_flag && file_index >= file_count) {
            break;
        }
#endif
    }

    SAMPLE_PRT("exit svp thread...\n");
    return NULL;
}

static xmedia_s32 sample_mmz_alloc_and_map(const xmedia_char *mmz_name, xmedia_char *buf_name,
                                           xmedia_u64 *phy_addr, xmedia_void **virt_addr, xmedia_u32 size)
{
   *phy_addr = xmedia_mmz_alloc(mmz_name, buf_name, size);
   if (*phy_addr != XMEDIA_NULL) {
       *virt_addr = xmedia_mmz_map(*phy_addr, size, XMEDIA_FALSE);
       if (*virt_addr == XMEDIA_NULL) {
           SAMPLE_PRT("xmedia_mmz_map filed, size %d \n", size);
               xmedia_mmz_free(*phy_addr);
               return XMEDIA_FAILURE;
       }
   } else {
       SAMPLE_PRT("xmedia_mmz_alloc filed, size %d \n", size);
       return XMEDIA_FAILURE;
   }

   return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_mmz_alloc_and_map_cache(const xmedia_char *mmz_name, xmedia_char *buf_name,
                                                 xmedia_u64 *phy_addr, xmedia_void **virt_addr, xmedia_u32 size)
{
    *phy_addr = xmedia_mmz_alloc(mmz_name, buf_name, size);
    if (*phy_addr != XMEDIA_NULL) {
        *virt_addr = xmedia_mmz_map(*phy_addr, size, XMEDIA_TRUE);
        if (*virt_addr == XMEDIA_NULL) {
            SAMPLE_PRT("xmedia_mmz_map filed, size %d \n", size);
                xmedia_mmz_free(*phy_addr);
                return XMEDIA_FAILURE;
        }
    } else {
        SAMPLE_PRT("xmedia_mmz_alloc filed, size %d \n", size);
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_void sample_mmz_unmap_and_free(xmedia_u64 phy_addr, xmedia_void *virt_addr)
{

   if (virt_addr != XMEDIA_NULL) {
       xmedia_mmz_unmap(virt_addr);
   }
   xmedia_mmz_free(phy_addr);
}

xmedia_s32 sample_svp_set_config(xmedia_svp_task_cfg task_cfg)
{
    xmedia_s32 ret;
    xmedia_svp_modules *temp_module;
    xmedia_cl_mem_info model_mem_info[task_cfg.module_num];

    ret = xmedia_svp_get_config(&g_svp_cfg);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_svp_get_config error ret : %d\n", ret);
        return XMEDIA_FAILURE;
    }

    g_svp_cfg.reuse_type = XMEDIA_SVP_MEM_TYPE_BLOCK;

    for (xmedia_s32 i = 0; i < task_cfg.module_num; i++) {
        temp_module = task_cfg.modules + i;
        ret = xmedia_cl_graph_query_model_info_from_file(temp_module->pathname, &model_mem_info[i], XMEDIA_CL_MEM_INFO);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("xmedia_cl_graph_query_model_info_from_file error ret : %d\n", ret);
            return XMEDIA_FAILURE;
        }
        if (g_svp_cfg.workbuf_reuse_mem.size < model_mem_info[i].worksize) {
            g_svp_cfg.workbuf_reuse_mem.size = model_mem_info[i].worksize;
        }
        if (g_svp_cfg.input_reuse_mem.size < model_mem_info[i].inputsize) {
            g_svp_cfg.input_reuse_mem.size = model_mem_info[i].inputsize;
        }
        if (g_svp_cfg.output_reuse_mem.size < model_mem_info[i].outputsize) {
            g_svp_cfg.output_reuse_mem.size = model_mem_info[i].outputsize;
        }
    }

    ret = sample_mmz_alloc_and_map_cache(XMEDIA_NULL, "npu_work_mem", &(g_svp_cfg.workbuf_reuse_mem.phyaddr),
                    (xmedia_void **)(&(g_svp_cfg.workbuf_reuse_mem.viraddr)), g_svp_cfg.workbuf_reuse_mem.size);
    if (XMEDIA_SUCCESS != ret) {
        SAMPLE_PRT("sample_mmz_alloc_and_map npu_work_mem errno: %d\n", ret);
        return XMEDIA_FAILURE;
    }

    ret = sample_mmz_alloc_and_map(XMEDIA_NULL, "npu_input_mem", &(g_svp_cfg.input_reuse_mem.phyaddr),
                        (xmedia_void **)(&(g_svp_cfg.input_reuse_mem.viraddr)), g_svp_cfg.input_reuse_mem.size);
    if (XMEDIA_SUCCESS != ret) {
        SAMPLE_PRT("sample_mmz_alloc_and_map npu_input_mem errno: %d\n", ret);
        goto EXIT0;
    }

    ret = sample_mmz_alloc_and_map_cache(XMEDIA_NULL, "npu_output_mem", &(g_svp_cfg.output_reuse_mem.phyaddr),
                        (xmedia_void **)(&(g_svp_cfg.output_reuse_mem.viraddr)), g_svp_cfg.output_reuse_mem.size);
    if (XMEDIA_SUCCESS != ret) {
        SAMPLE_PRT("sample_mmz_alloc_and_map npu_output_mem errno: %d\n", ret);
        goto EXIT1;
    }

    ret = xmedia_svp_set_config(&g_svp_cfg);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_svp_set_config error ret : %d\n", ret);
        goto EXIT2;
    }

    return ret;

EXIT2:
    sample_mmz_unmap_and_free(g_svp_cfg.output_reuse_mem.phyaddr, g_svp_cfg.output_reuse_mem.viraddr);
EXIT1:
    sample_mmz_unmap_and_free(g_svp_cfg.input_reuse_mem.phyaddr, g_svp_cfg.input_reuse_mem.viraddr);
EXIT0:
    sample_mmz_unmap_and_free(g_svp_cfg.workbuf_reuse_mem.phyaddr, g_svp_cfg.workbuf_reuse_mem.viraddr);

    return XMEDIA_FAILURE;
}

xmedia_s32 sample_svp_stop(sample_svp_info *svp_info)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    g_svp_start_flag = XMEDIA_FALSE;
    usleep(200000);
    if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_FACE_RECOGNITION) {
        svp_deinit_database(svp_info->g_db_handle);
    }
    ret = xmedia_svp_task_destroy(svp_info->svp_handle);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_svp_task_destroy failed with %d!!! \n", ret);
    }
    svp_info->svp_handle = -1;

    if (g_svp_cfg.reuse_type == XMEDIA_SVP_MEM_TYPE_BLOCK) {
        if (g_svp_cfg.output_reuse_mem.viraddr != NULL) {
            sample_mmz_unmap_and_free(g_svp_cfg.output_reuse_mem.phyaddr, g_svp_cfg.output_reuse_mem.viraddr);
        }
        if (g_svp_cfg.input_reuse_mem.viraddr != NULL) {
            sample_mmz_unmap_and_free(g_svp_cfg.input_reuse_mem.phyaddr, g_svp_cfg.input_reuse_mem.viraddr);
        }
        if (g_svp_cfg.workbuf_reuse_mem.viraddr != NULL) {
            sample_mmz_unmap_and_free(g_svp_cfg.workbuf_reuse_mem.phyaddr, g_svp_cfg.workbuf_reuse_mem.viraddr);
        }
    }

    xmedia_svp_uninit();
    SAMPLE_PRT("svp_stop successfully!!! \n");

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_svp_start(sample_svp_info *svp_info)
{
    xmedia_s32 ret;
    xmedia_svp_task_cfg task_cfg;
    xmedia_svp_modules modules[8];

    ret = xmedia_svp_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_svp_init error.\n");
        return XMEDIA_FAILURE;
    }

    if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_PERSON ||
        svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_FACE ||
        svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_PERSON_RCNN) {
        ret = xmedia_venc_enable_svc(0, XMEDIA_TRUE);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("xmedia_venc_enable_svc fail.\n");
        }
    }

    switch (svp_info->detect_type) {
        case SAMPLE_SVP_ALG_TYPE_PERSON:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_PERSON;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_person_detect_640x360_rgb888hwc_v0103_20251203.bin";
            task_cfg.module_num = 1;
            task_cfg.task_type = XMEDIA_SVP_TASK_DETECT;
            task_cfg.modules = modules;
#if CHECK_SVP_SVC
            xmedia_venc_svc_param svc_param;
            ret = xmedia_venc_get_svc_param(0, &svc_param);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_venc_get_svc_param fail.\n");
            }
            svc_param.bg_region.qpmap_value_p = 0x80;
            svc_param.bg_region.qpmap_value_i = 0x80;
            ret = xmedia_venc_set_svc_param(0, &svc_param);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_venc_set_svc_param fail.\n");
            }
#endif
            break;
        case SAMPLE_SVP_ALG_TYPE_PERSON_KEYPOINT:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_PERSON_KEYPOINT;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_person_keypoint_640x360_rgb888hwc_v0101_20250523.bin";
            task_cfg.module_num = 1;
            task_cfg.task_type = XMEDIA_SVP_TASK_DETECT_AND_KEYPOINT;
            task_cfg.modules = modules;
            break;
        case SAMPLE_SVP_ALG_TYPE_FACE:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_FACE;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_face_detect_640x360_rgb888hwc_v0103_20251209.bin";
            task_cfg.module_num = 1;
            task_cfg.task_type = XMEDIA_SVP_TASK_DETECT;
            task_cfg.modules = modules;
            break;
        case SAMPLE_SVP_ALG_TYPE_PET:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_PET;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_pet_detect_640x360_rgb888hwc_v0104_20251209.bin";
            task_cfg.module_num = 1;
            task_cfg.task_type = XMEDIA_SVP_TASK_DETECT;
            task_cfg.modules = modules;
            break;
        case SAMPLE_SVP_ALG_TYPE_CAR:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_CAR;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_car_detect_640x360_rgb888hwc_v0103_20251203.bin";
            task_cfg.module_num = 1;
            task_cfg.task_type = XMEDIA_SVP_TASK_DETECT;
            task_cfg.modules = modules;
            break;
        case SAMPLE_SVP_ALG_TYPE_NON_MOTORIZED_VEHICLE:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_NON_MOTORIZED_VEHICLE;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_nocar_detect_640x360_rgb888hwc_v0102_20240417.bin";
            task_cfg.module_num = 1;
            task_cfg.task_type = XMEDIA_SVP_TASK_DETECT;
            task_cfg.modules = modules;
            break;
        case SAMPLE_SVP_ALG_TYPE_HEAD:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_HEAD;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_head_detect_640x360_rgb888hwc_v0102_20240318.bin";
            task_cfg.module_num = 1;
            task_cfg.task_type = XMEDIA_SVP_TASK_DETECT;
            task_cfg.modules = modules;
            break;
        case SAMPLE_SVP_ALG_TYPE_FIREWORKS:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_FIREWORKS;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_fireworks_detect_640x360_rgb888hwc_v0101_20250523.bin";
            task_cfg.module_num = 1;
            task_cfg.task_type = XMEDIA_SVP_TASK_DETECT;
            task_cfg.modules = modules;
            break;
        case SAMPLE_SVP_ALG_TYPE_PACKAGE:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_PACKAGE;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_package_detect_640x360_rgb888hwc_v0101_20250523.bin";
            task_cfg.module_num = 1;
            task_cfg.task_type = XMEDIA_SVP_TASK_DETECT;
            task_cfg.modules = modules;
            break;
        case SAMPLE_SVP_ALG_TYPE_AOV_PERSON:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_PERSON;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_aov_person_detect_640x360_rgb888hwc_v0101_20240909.bin";
            task_cfg.module_num = 1;
            task_cfg.task_type = XMEDIA_SVP_TASK_AOV;
            task_cfg.modules = modules;
            break;
        case SAMPLE_SVP_ALG_TYPE_ADAS:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_LICENSE_DETECT;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_license_plate_detection_rgb888hwc_v0101_20241205.bin";
            modules[1].alg_type = XMEDIA_SVP_ALG_TYPE_NON_MOTORIZED_VEHICLE;
            modules[1].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[1].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[1].pathname = "./model/gnn_nocar_detect_640x360_rgb888hwc_v0102_20240417.bin";
            modules[2].alg_type = XMEDIA_SVP_ALG_TYPE_CAR;
            modules[2].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[2].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[2].pathname = "./model/gnn_car_detect_640x360_rgb888hwc_v0103_20251203.bin";
            modules[3].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[3].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[3].pathname = "./model/gnn_laneline_detect_640x360_rgb888hwc_v0101_20250616.bin";
            modules[4].alg_type = XMEDIA_SVP_ALG_TYPE_PERSON;
            modules[4].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[4].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[4].pathname = "./model/gnn_person_detect_640x360_rgb888hwc_v0103_20251203.bin";
            task_cfg.module_num = 5;
            task_cfg.task_type = XMEDIA_SVP_TASK_ADAS;
            task_cfg.modules = modules;
            break;
        case SAMPLE_SVP_ALG_TYPE_FIRESMOKE:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_FIRESMOKE;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_fire_detect_640x360_rgb888hwc_v0101_20250313.bin";
            task_cfg.module_num = 1;
            task_cfg.task_type = XMEDIA_SVP_TASK_FIRE;
            task_cfg.modules = modules;
            break;
        case SAMPLE_SVP_ALG_TYPE_GESTURE_RECOGNITION:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_GESTURE;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_gesture_detection_640x360_rgb888hwc_v0101_20241011.bin";
            modules[1].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[1].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[1].pathname = "./model/gnn_gesture_classification_96x96_rgb888hwc_v0101_20241203.bin";
            task_cfg.module_num = 2;
            task_cfg.task_type = XMEDIA_SVP_TASK_GESTURE;
            task_cfg.modules = modules;
            break;
        case SAMPLE_SVP_ALG_TYPE_FACE_EMOTION:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_FACE;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_face_detect_640x360_rgb888hwc_v0103_20251209.bin";
            modules[1].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[1].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[1].pathname = "./model/gnn_face_keypoint_48x48_rgb888hwc_v0101_20250818.bin";
            modules[2].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[2].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[2].pathname = "./model/gnn_face_emotion_112x112_rgb888hwc_v0101_20250904.bin";
            task_cfg.module_num = 3;
            task_cfg.task_type = XMEDIA_SVP_TASK_EMOTION_CLASSIFITION;
            task_cfg.modules = modules;
            break;
        case SAMPLE_SVP_ALG_TYPE_FACE_RECOGNITION:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_FACE;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_face_detect_640x360_rgb888hwc_v0103_20251209.bin";
            modules[1].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[1].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[1].pathname = "./model/gnn_face_keypoint_48x48_rgb888hwc_v0101_20250818.bin";
            modules[2].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[2].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[2].pathname = "./model/gnn_face_recognition_112x112_rgb888hwc_v0101_20250818.bin";
            task_cfg.module_num = 3;
            task_cfg.task_type = XMEDIA_SVP_TASK_FACE_RECOGNITON;
            task_cfg.modules = modules;
            break;
        case SAMPLE_SVP_ALG_TYPE_PERSON_RCNN:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_PERSON;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_person_detect_640x360_rgb888hwc_v0103_20251203.bin";
            modules[1].alg_type = XMEDIA_SVP_ALG_TYPE_SECOND_STAGE;
            modules[1].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[1].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[1].pathname = "./model/gnn_second_stage_rgb888hwc_v0102_20251106.bin";
            task_cfg.module_num = 2;
            task_cfg.task_type = XMEDIA_SVP_TASK_RCNN;
            task_cfg.modules = modules;
#if CHECK_SVP_SMART_AE
            xmedia_ae_smart_exposure_attr smart_exp_attr;
            xmedia_ae_get_smart_exposure_attr(0, &smart_exp_attr);
            smart_exp_attr.enable = XMEDIA_TRUE;
            smart_exp_attr.smart_exp_type = XMEDIA_VIDEO_OPERATION_MODE_AUTO;
            smart_exp_attr.iso_num = 1;
            smart_exp_attr.luma_target[0] = 20;
            smart_exp_attr.exp_coef_min = 128;
            smart_exp_attr.exp_coef_max = 256 * 200;
            smart_exp_attr.ir_mode = XMEDIA_FALSE;
            smart_exp_attr.smart_speed = 64;
            smart_exp_attr.smart_delay_num = 64;
            xmedia_ae_set_smart_exposure_attr(0, &smart_exp_attr);
#endif
            break;
        case SAMPLE_SVP_ALG_TYPE_CAR_RCNN:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_CAR;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_car_detect_640x360_rgb888hwc_v0103_20251203.bin";
            modules[1].alg_type = XMEDIA_SVP_ALG_TYPE_SECOND_STAGE;
            modules[1].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[1].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[1].pathname = "./model/gnn_second_stage_rgb888hwc_v0102_20251106.bin";
            task_cfg.module_num = 2;
            task_cfg.task_type = XMEDIA_SVP_TASK_RCNN;
            task_cfg.modules = modules;
            break;
        case SAMPLE_SVP_ALG_TYPE_PET_RCNN:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_PET;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_pet_detect_640x360_rgb888hwc_v0104_20251209.bin";
            modules[1].alg_type = XMEDIA_SVP_ALG_TYPE_SECOND_STAGE;
            modules[1].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[1].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[1].pathname = "./model/gnn_second_stage_rgb888hwc_v0102_20251106.bin";
            task_cfg.module_num = 2;
            task_cfg.task_type = XMEDIA_SVP_TASK_RCNN;
            task_cfg.modules = modules;
            break;
        case SAMPLE_SVP_ALG_TYPE_COMPANION_ROBOT:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_PET;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_pet_detect_640x360_rgb888hwc_v0103_20250520.bin";
            modules[1].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[1].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[1].pathname = "./model/gnn_pet_classify_128x128_rgb888hwc_v0101_20250926.bin";
            task_cfg.module_num = 2;
            task_cfg.task_type = XMEDIA_SVP_TASK_COMPANION_ROBOT;
            task_cfg.modules = modules;
            break;
        case SAMPLE_SVP_ALG_TYPE_PERSON_RCNN_720P:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_PERSON;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_person_nocar_detect_1280x720_rgb888hwc_v0101_202507025.bin";
            modules[1].alg_type = XMEDIA_SVP_ALG_TYPE_SECOND_STAGE;
            modules[1].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[1].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[1].pathname = "./model/gnn_second_stage_rgb888hwc_v0102_20251106.bin";
            task_cfg.module_num = 2;
            task_cfg.task_type = XMEDIA_SVP_TASK_RCNN_720P;
            task_cfg.modules = modules;
            break;
        case SAMPLE_SVP_ALG_TYPE_CAR_RCNN_720P:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_CAR;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_car_detect_1280x720_rgb888hwc_v0101_20251103.bin";
            modules[1].alg_type = XMEDIA_SVP_ALG_TYPE_SECOND_STAGE;
            modules[1].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[1].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[1].pathname = "./model/gnn_second_stage_rgb888hwc_v0102_20251106.bin";
            task_cfg.module_num = 2;
            task_cfg.task_type = XMEDIA_SVP_TASK_RCNN_720P;
            task_cfg.modules = modules;
            break;
        case SAMPLE_SVP_ALG_TYPE_PET_RCNN_720P:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_PET;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_pet_detect_1280x720_rgb888hwc_v0101_20251103.bin";
            modules[1].alg_type = XMEDIA_SVP_ALG_TYPE_SECOND_STAGE;
            modules[1].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[1].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[1].pathname = "./model/gnn_second_stage_rgb888hwc_v0102_20251106.bin";
            task_cfg.module_num = 2;
            task_cfg.task_type = XMEDIA_SVP_TASK_RCNN_720P;
            task_cfg.modules = modules;
            break;
        case SAMPLE_SVP_ALG_TYPE_MULTI_DETECT:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_CAR;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_car_detect_640x360_rgb888hwc_v0103_20251203.bin";
            modules[1].alg_type = XMEDIA_SVP_ALG_TYPE_PERSON;
            modules[1].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[1].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[1].pathname = "./model/gnn_person_detect_640x360_rgb888hwc_v0103_20251203.bin";
            task_cfg.module_num = 2;
            task_cfg.task_type = XMEDIA_SVP_TASK_MULTI_DETECT;
            task_cfg.modules = modules;
            break;
        case SAMPLE_SVP_ALG_TYPE_PERSON_CAR:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_PERSON_CAR;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_person_car_detect_640x360_rgb888hwc_v0101_20251203.bin";
            task_cfg.module_num = 1;
            task_cfg.task_type = XMEDIA_SVP_TASK_DETECT;
            task_cfg.modules = modules;
            break;
        case SAMPLE_SVP_ALG_TYPE_DMS:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_DMS;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_dms_640x360_rgb888hwc_v0101_20240906.bin";
            task_cfg.module_num = 1;
            task_cfg.task_type = XMEDIA_SVP_TASK_DMS;
            task_cfg.modules = modules;
            break;
        case SAMPLE_SVP_ALG_TYPE_PLATE_RECOGNITION:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_LICENSE_DETECT;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_plate_detect_512x288_rgb888hwc_v0101_20241205.bin";

            modules[1].alg_type = XMEDIA_SVP_ALG_TYPE_LICENSE_RECO;
            modules[1].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[1].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[1].pathname = "./model/gnn_plate_recognition_192x48_rgb888hwc_v0101_20251125.bin";

            modules[2].alg_type = XMEDIA_SVP_ALG_TYPE_LICENSE_COLOR;
            modules[2].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[2].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[2].pathname = "./model/gnn_plate_color_192x48_rgb888hwc_v0101_20251123.bin";
#define PLATE_GATE
#ifdef PLATE_GATE
            modules[3].alg_type = XMEDIA_SVP_ALG_TYPE_VEHICLE_DETECT;
            modules[3].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[3].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[3].pathname = "./model/gnn_vehicle_detect_384x224_rgb888hwc_v0101_20240930.bin";
            task_cfg.module_num = 4;
#else
            task_cfg.module_num = 3;
#endif
            task_cfg.task_type = XMEDIA_SVP_TASK_PLATE;
            task_cfg.modules = modules;
            break;
        case SAMPLE_SVP_ALG_TYPE_VEHICLE_RECOGNITION:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_VEHICLE_DETECT;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_vehicle_detect_384x224_rgb888hwc_v0101_20240930.bin";

            modules[1].alg_type = XMEDIA_SVP_ALG_TYPE_VEHICLE_RECO;
            modules[1].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[1].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[1].pathname = "./model/gnn_vehicle_recognition_224x224_rgb888hwc_v0101_20240930.bin";

            task_cfg.module_num = 2;
            task_cfg.task_type = XMEDIA_SVP_TASK_VEHICLE;
            task_cfg.modules = modules;
            break;
        case SAMPLE_SVP_ALG_TYPE_PLATE_VEHICLE_RECO:
            modules[0].alg_type = XMEDIA_SVP_ALG_TYPE_LICENSE_DETECT;
            modules[0].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[0].pathname = "./model/gnn_plate_detect_512x288_rgb888hwc_v0101_20241205.bin";

            modules[1].alg_type = XMEDIA_SVP_ALG_TYPE_LICENSE_RECO;
            modules[1].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[1].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[1].pathname = "./model/gnn_plate_recognition_192x48_rgb888hwc_v0101_20251125.bin";

            modules[2].alg_type = XMEDIA_SVP_ALG_TYPE_LICENSE_COLOR;
            modules[2].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[2].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[2].pathname = "./model/gnn_plate_color_192x48_rgb888hwc_v0101_20251123.bin";

            modules[3].alg_type = XMEDIA_SVP_ALG_TYPE_VEHICLE_DETECT;
            modules[3].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[3].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[3].pathname = "./model/gnn_vehicle_detect_384x224_rgb888hwc_v0101_20240930.bin";

            modules[4].alg_type = XMEDIA_SVP_ALG_TYPE_VEHICLE_RECO;
            modules[4].load_mode = XMEDIA_SVP_MODEL_FILE;
            modules[4].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;
            modules[4].pathname = "./model/gnn_vehicle_recognition_224x224_rgb888hwc_v0101_20240930.bin";

            task_cfg.module_num = 5;
            task_cfg.task_type = XMEDIA_SVP_TASK_PLATE_VEHICLE;
            task_cfg.modules = modules;
            break;
        default:
            SAMPLE_PRT("detect type not supported! \n");
            xmedia_svp_uninit();
            return XMEDIA_FAILURE;
        }

    ret = sample_svp_set_config(task_cfg);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_svp_set_config error ret : %d\n", ret);
        xmedia_svp_uninit();
        return XMEDIA_FAILURE;
    }

    ret = xmedia_svp_task_create(&svp_info->svp_handle, task_cfg);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_svp_task_create error ret : %d\n", ret);
        xmedia_svp_uninit();
        return XMEDIA_FAILURE;
    }
    if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_AOV_PERSON) {
        xmedia_svp_aov_attr yolov5_attr;
        xmedia_svp_task_get_attr(svp_info->svp_handle, &yolov5_attr);
        yolov5_attr.detect_threshold = 0.8f;
        yolov5_attr.classifier_threshold = 0.01f;
        yolov5_attr.iou_threshold = 0.45f;
        yolov5_attr.max_target_num = 10;
        yolov5_attr.bytetrack_enable = XMEDIA_FALSE;
        yolov5_attr.motionless_filter_enable = XMEDIA_FALSE;
        yolov5_attr.aov_only_target = XMEDIA_TRUE;
        ret = xmedia_svp_task_set_attr(svp_info->svp_handle, &yolov5_attr);
    } else if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_PERSON_KEYPOINT) {
        xmedia_svp_yolov8_attr yolov8_attr;
        xmedia_svp_task_get_attr(svp_info->svp_handle, &yolov8_attr);
        yolov8_attr.detect_threshold = 0.5f;
        yolov8_attr.classifier_threshold = 0.01f;
        yolov8_attr.iou_threshold = 0.45f;
        yolov8_attr.max_target_num = 10;
        yolov8_attr.bytetrack_enable = XMEDIA_FALSE;
        yolov8_attr.motionless_filter_enable = XMEDIA_FALSE;
        ret = xmedia_svp_task_set_attr(svp_info->svp_handle, &yolov8_attr);
    } else if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_ADAS) {
        xmedia_svp_adas_attr adas_attr;
        xmedia_svp_task_get_attr(svp_info->svp_handle, &adas_attr);
        adas_attr.car_attr.detect_threshold = 0.6f;
        adas_attr.car_attr.iou_threshold = 0.4f;
        adas_attr.car_attr.max_target_num = 5;
        adas_attr.car_attr.classifier_threshold = 0.01;

        adas_attr.plate_attr.detect_threshold = 0.5f;
        adas_attr.plate_attr.iou_threshold = 0.4f;
        adas_attr.plate_attr.max_target_num = 5;
        adas_attr.plate_attr.classifier_threshold = 0.01;

        adas_attr.person_attr.detect_threshold = 0.5f;
        adas_attr.person_attr.iou_threshold = 0.4f;
        adas_attr.person_attr.max_target_num = 5;
        adas_attr.person_attr.classifier_threshold = 0.01;

        adas_attr.nmv_attr.detect_threshold = 0.5f;
        adas_attr.nmv_attr.iou_threshold = 0.4f;
        adas_attr.nmv_attr.max_target_num = 5;
        adas_attr.nmv_attr.classifier_threshold = 0.01;

        adas_attr.distance_attr.focal_length_px = 500;
        adas_attr.distance_attr.car_limit_angle = 10;
        adas_attr.distance_attr.camera_height = 100;
        adas_attr.distance_attr.plate_width = 14;
        ret = xmedia_svp_task_set_attr(svp_info->svp_handle, &adas_attr);
    } else if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_FIRESMOKE) {
        xmedia_svp_fire_attr fire_attr;
        xmedia_svp_task_get_attr(svp_info->svp_handle, &fire_attr);
        fire_attr.detect_attr.detect_threshold = 0.4f;
        fire_attr.detect_attr.iou_threshold = 0.5f;
        fire_attr.detect_attr.max_target_num = 10;
        fire_attr.detect_attr.classifier_threshold = 0.01;
        fire_attr.fire_change_ratio = 0.1f;
        fire_attr.fire_loss_count = 10;
        fire_attr.fire_ratio_sum = 0.5f;
        fire_attr.smoke_apear_count = 3;
        fire_attr.smoke_change_ratio = 0.1f;
        fire_attr.smoke_loss_count = 5;
        ret = xmedia_svp_task_set_attr(svp_info->svp_handle, &fire_attr);
    } else if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_GESTURE_RECOGNITION) {
        xmedia_svp_yolov8_attr gesture_attr;
        xmedia_svp_task_get_attr(svp_info->svp_handle, &gesture_attr);
        gesture_attr.bytetrack_enable = XMEDIA_FALSE;
        gesture_attr.detect_threshold = 0.5f;
        gesture_attr.iou_threshold = 0.5f;
        gesture_attr.max_target_num = 3;
        gesture_attr.classifier_threshold = 0.01;
        ret = xmedia_svp_task_set_attr(svp_info->svp_handle, &gesture_attr);
    } else if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_FACE_RECOGNITION) {
        xmedia_svp_fr_attr fr_attr;
        xmedia_svp_task_get_attr(svp_info->svp_handle, &fr_attr);
        fr_attr.detect_attr.detect_threshold = 0.5f;
        fr_attr.detect_attr.classifier_threshold = 0.01f;
        fr_attr.detect_attr.iou_threshold = 0.45f;
        fr_attr.detect_attr.max_target_num = 10;
        fr_attr.detect_attr.bytetrack_enable = XMEDIA_TRUE;
        fr_attr.detect_attr.motionless_filter_enable = XMEDIA_FALSE;
        fr_attr.luma_score = 60;
        fr_attr.ive_sobel_score = 25;
        fr_attr.keypoint_score = 0.8f;
        fr_attr.fr_pose_attr.clockwise_thres = 35.0f;
        fr_attr.fr_pose_attr.anticlockwise_thres = 35.0f;
        fr_attr.fr_pose_attr.left_thres = 35.0f;
        fr_attr.fr_pose_attr.right_thres = 35.0f;
        fr_attr.fr_pose_attr.upward_thres = 35.0f;
        fr_attr.fr_pose_attr.downward_thres = 40.0f;
        ret = xmedia_svp_task_set_attr(svp_info->svp_handle, &fr_attr);

        // 数据库初始化
        config_info db_cfg;
        db_cfg.file_name = "./res/face_feature_data.txt";
        db_cfg.match_thres = 0.25;
        ret = svp_init_database(&svp_info->g_db_handle, &db_cfg);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("database_init failed ! \n");
        }

        printf("已进入人脸识别模式,请输入具体的功能: 1:识别  2: 录入  3: 删除\n");
        xmedia_s32 input_number;
        scanf("%d", &input_number);
        if (input_number == 1) {
            printf("开始识别\n");
        } else if (input_number == 2) {
            printf("请输入要添加的人名:\n");
            scanf("%s", svp_info->add_name);
            printf("确认输入的名字：[%s] ,面对镜头 \n", svp_info->add_name);
            svp_info->add_flag = XMEDIA_TRUE;
        } else if (input_number == 3) {
            printf("请输入要删除的人名:\n");
            scanf("%s", svp_info->delete_name);
            printf("确认输入的名字：%s\n", svp_info->delete_name);
            svp_info->delete_flag = XMEDIA_TRUE;
        } else {
            printf("开始识别\n");
        }
    } else if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_FACE_EMOTION) {
        xmedia_svp_fr_attr fr_attr;
        xmedia_svp_task_get_attr(svp_info->svp_handle, &fr_attr);
        fr_attr.detect_attr.detect_threshold = 0.5f;
        fr_attr.detect_attr.classifier_threshold = 0.01f;
        fr_attr.detect_attr.iou_threshold = 0.45f;
        fr_attr.detect_attr.max_target_num = 10;
        fr_attr.detect_attr.bytetrack_enable = XMEDIA_TRUE;
        fr_attr.detect_attr.motionless_filter_enable = XMEDIA_FALSE;
        fr_attr.luma_score = 60;
        fr_attr.ive_sobel_score = 10;
        fr_attr.keypoint_score = 0.6f;
        fr_attr.fr_pose_attr.clockwise_thres = 35.0f;
        fr_attr.fr_pose_attr.anticlockwise_thres = 35.0f;
        fr_attr.fr_pose_attr.left_thres = 35.0f;
        fr_attr.fr_pose_attr.right_thres = 35.0f;
        fr_attr.fr_pose_attr.upward_thres = 35.0f;
        fr_attr.fr_pose_attr.downward_thres = 40.0f;
        ret = xmedia_svp_task_set_attr(svp_info->svp_handle, &fr_attr);
    } else if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_PERSON_RCNN ||
               svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_CAR_RCNN ||
               svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_PET_RCNN) {
        xmedia_svp_yolov5_attr yolov5_attr;
        xmedia_svp_task_get_attr(svp_info->svp_handle, &yolov5_attr);
        yolov5_attr.detect_threshold = 0.3f;
        yolov5_attr.classifier_threshold = 0.45f;
        yolov5_attr.iou_threshold = 0.45f;
        yolov5_attr.max_target_num = 10;
        yolov5_attr.bytetrack_enable = XMEDIA_TRUE;
        yolov5_attr.motionless_filter_enable = XMEDIA_FALSE;
        yolov5_attr.stillness_thres = 0.95f;
        yolov5_attr.movement_fps_thres = 7;
        if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_PERSON_RCNN) {
            yolov5_attr.smart_ae_enable = XMEDIA_TRUE;
            yolov5_attr.smart_venc_enable = XMEDIA_TRUE;
            yolov5_attr.smart_ae_array[0] = XMEDIA_TRUE;
            yolov5_attr.smart_venc_array[0] = XMEDIA_TRUE;
        }
        ret = xmedia_svp_task_set_attr(svp_info->svp_handle, &yolov5_attr);
    } else if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_COMPANION_ROBOT) {
        xmedia_svp_yolov5_attr yolov5_attr;
        xmedia_svp_task_get_attr(svp_info->svp_handle, &yolov5_attr);
        yolov5_attr.detect_threshold = 0.1f;
        yolov5_attr.classifier_threshold = 0.5f;
        yolov5_attr.iou_threshold = 0.45f;
        yolov5_attr.max_target_num = 10;
        yolov5_attr.bytetrack_enable = XMEDIA_TRUE;
        yolov5_attr.motionless_filter_enable = XMEDIA_FALSE;
        yolov5_attr.stillness_thres = 0.95f;
        yolov5_attr.movement_fps_thres = 7;
        ret = xmedia_svp_task_set_attr(svp_info->svp_handle, &yolov5_attr);
    } else if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_PERSON_RCNN_720P ||
               svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_CAR_RCNN_720P ||
               svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_PET_RCNN_720P) {
        xmedia_svp_yolov5_attr yolov5_attr;
        xmedia_svp_task_get_attr(svp_info->svp_handle, &yolov5_attr);
        yolov5_attr.detect_threshold = 0.1f;
        yolov5_attr.classifier_threshold = 0.5f;
        yolov5_attr.iou_threshold = 0.45f;
        yolov5_attr.max_target_num = 10;
        yolov5_attr.bytetrack_enable = XMEDIA_FALSE;
        yolov5_attr.motionless_filter_enable = XMEDIA_FALSE;
        yolov5_attr.stillness_thres = 0.95f;
        yolov5_attr.movement_fps_thres = 7;
        ret = xmedia_svp_task_set_attr(svp_info->svp_handle, &yolov5_attr);
    } else if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_MULTI_DETECT) {
        xmedia_svp_multi_detect_attr multi_attr;
        xmedia_svp_task_get_attr(svp_info->svp_handle, &multi_attr);

        multi_attr.car_attr.detect_threshold = 0.65f;
        multi_attr.car_attr.iou_threshold = 0.45f;
        multi_attr.car_attr.max_target_num = 10;
        multi_attr.car_attr.classifier_threshold = 0.01f;

        multi_attr.person_attr.detect_threshold = 0.65f;
        multi_attr.person_attr.iou_threshold = 0.45f;
        multi_attr.person_attr.max_target_num = 10;
        multi_attr.person_attr.classifier_threshold = 0.01f;

        multi_attr.bytetrack_enable = XMEDIA_TRUE;
        multi_attr.motionless_filter_enable = XMEDIA_FALSE;
        multi_attr.stillness_thres = 0.95f;
        multi_attr.movement_fps_thres = 7;
        ret = xmedia_svp_task_set_attr(svp_info->svp_handle, &multi_attr);
    } else if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_DMS) {
        xmedia_svp_dms_attr dms_attr;
        xmedia_svp_task_get_attr(svp_info->svp_handle, &dms_attr);
        dms_attr.detect_attr.detect_threshold = 0.5f;
        dms_attr.detect_attr.iou_threshold = 0.5f;
        dms_attr.detect_attr.max_target_num = 5;
        dms_attr.detect_attr.classifier_threshold = 0.01f;
        dms_attr.warn_frame = 10;
        dms_attr.iou_thres = 0.1f;
        dms_attr.detect_attr.bytetrack_enable = XMEDIA_TRUE;
        ret = xmedia_svp_task_set_attr(svp_info->svp_handle, &dms_attr);
   } else if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_PLATE_RECOGNITION) {
        xmedia_svp_plate_reco_attr plate_attr;
        xmedia_svp_task_get_attr(svp_info->svp_handle, &plate_attr);
        plate_attr.plate_detect_attr.detect_threshold = 0.65f;
        plate_attr.plate_detect_attr.iou_threshold = 0.5f;
        plate_attr.plate_detect_attr.max_target_num = 5;
        plate_attr.plate_detect_attr.classifier_threshold = 0.01;
        plate_attr.plate_detect_attr.bytetrack_enable = XMEDIA_TRUE;
        plate_attr.plate_min_width = 85;
        plate_attr.report_count = 15;
        plate_attr.report_thres = 0.90;
        plate_attr.detect_zone.zone_num = 1;
        plate_attr.detect_zone.zones[0].points_count = 4;
        plate_attr.detect_zone.zones[0].points_array[0].x = 0;
        plate_attr.detect_zone.zones[0].points_array[0].y = 0;
        plate_attr.detect_zone.zones[0].points_array[1].x = 1920 * SVP_SCALE_RATIO / SVP_BIG_STREAM_W;
        plate_attr.detect_zone.zones[0].points_array[1].y = 0 * SVP_SCALE_RATIO / SVP_BIG_STREAM_H;
        plate_attr.detect_zone.zones[0].points_array[2].x = 1920 * SVP_SCALE_RATIO / SVP_BIG_STREAM_W;
        plate_attr.detect_zone.zones[0].points_array[2].y = 1080 * SVP_SCALE_RATIO / SVP_BIG_STREAM_H;
        plate_attr.detect_zone.zones[0].points_array[3].x = 0 * SVP_SCALE_RATIO / SVP_BIG_STREAM_W;
        plate_attr.detect_zone.zones[0].points_array[3].y = 1080 * SVP_SCALE_RATIO / SVP_BIG_STREAM_H;
        xmedia_svp_task_set_attr(svp_info->svp_handle, &plate_attr);
    } else if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_VEHICLE_RECOGNITION) {
        xmedia_svp_vehicle_reco_attr veh_attr;
        xmedia_svp_task_get_attr(svp_info->svp_handle, &veh_attr);
        veh_attr.detect_attr.detect_threshold = 0.65f;
        veh_attr.detect_attr.iou_threshold = 0.45f;
        veh_attr.detect_attr.max_target_num = 5;
        veh_attr.detect_attr.classifier_threshold = 0.01;
        veh_attr.detect_attr.bytetrack_enable = XMEDIA_TRUE;
        veh_attr.report_count = 8;
        veh_attr.report_thres = 0.60;
        xmedia_svp_task_set_attr(svp_info->svp_handle, &veh_attr);
    } else if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_PLATE_VEHICLE_RECO) {
        xmedia_svp_plate_vehicle_attr pv_attr;
        xmedia_svp_task_get_attr(svp_info->svp_handle, &pv_attr);
        pv_attr.plate_attr.detect_threshold = 0.65f;
        pv_attr.plate_attr.iou_threshold = 0.5f;
        pv_attr.plate_attr.max_target_num = 5;
        pv_attr.plate_attr.classifier_threshold = 0.01;
        pv_attr.plate_attr.bytetrack_enable = XMEDIA_TRUE;
        pv_attr.veh_attr.detect_threshold = 0.65f;
        pv_attr.veh_attr.iou_threshold = 0.45f;
        pv_attr.veh_attr.max_target_num = 5;
        pv_attr.veh_attr.classifier_threshold = 0.01;
        pv_attr.veh_attr.bytetrack_enable = XMEDIA_TRUE;
        pv_attr.plate_min_width = 85;
        pv_attr.report_count = 8;
        pv_attr.report_thres = 0.60;
        xmedia_svp_task_set_attr(svp_info->svp_handle, &pv_attr);
   } else {
        xmedia_svp_yolov5_attr yolov5_attr;
        xmedia_svp_task_get_attr(svp_info->svp_handle, &yolov5_attr);
        yolov5_attr.detect_threshold = 0.65f;
        yolov5_attr.classifier_threshold = 0.01f;
        yolov5_attr.iou_threshold = 0.45f;
        yolov5_attr.max_target_num = 10;
        yolov5_attr.bytetrack_enable = XMEDIA_TRUE;
        yolov5_attr.motionless_filter_enable = XMEDIA_FALSE;
        yolov5_attr.stillness_thres = 0.95f;
        // 当前帧与历史帧相交大于95%算当前帧静止，静止计数+1，阈值越低越容易被判断为静止
        yolov5_attr.movement_fps_thres = 7;
        // 静止帧计数连续7帧大于stillness_thres，则目标被判断为静止，判断帧数越低越容易被判断为静止
        if (svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_PERSON ||
            svp_info->detect_type == SAMPLE_SVP_ALG_TYPE_FACE) {
            yolov5_attr.smart_ae_enable = XMEDIA_TRUE;
            yolov5_attr.smart_venc_enable = XMEDIA_TRUE;
            yolov5_attr.smart_ae_array[0] = XMEDIA_TRUE;
            yolov5_attr.smart_venc_array[0] = XMEDIA_TRUE;
        }
        ret = xmedia_svp_task_set_attr(svp_info->svp_handle, &yolov5_attr);
    }
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_svp_task_create error ret : %d\n", ret);
        xmedia_svp_task_destroy(svp_info->svp_handle);
        xmedia_svp_uninit();
        return XMEDIA_FAILURE;
    }

    g_svp_start_flag = XMEDIA_TRUE;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setstacksize(&attr, 1024 * 1024 * 2); // 设置2M栈空间大小
    pthread_create(&g_svp_thread, &attr, sample_svp_proc, svp_info);
    pthread_setname_np(g_svp_thread, "svp_pthread_t");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_svp_sys_init(sample_sys_config *sys_config)
{
    xmedia_s32 ret = 0;

    ret = sample_comm_sys_init(sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_sys_init failed!\n");
        return ret;
    }

    ret = sample_comm_vgs_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vgs_init failed!\n");
        return ret;
    }

    // in online-online mode,vi and vpss must be reset at the same time
    ret = sample_comm_vi_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vi_init failed!\n");
        return ret;
    }

    ret = sample_comm_vpss_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vpss_init failed!\n");
        return ret;
    }

    ret = sample_comm_vo_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vo_init failed!\n");
        return ret;
    }

    ret = sample_comm_venc_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_venc_init failed!\n");
        return ret;
    }

    return XMEDIA_SUCCESS;
}

xmedia_void sample_svp_sys_exit(void)
{
    sample_comm_venc_stop_get_stream();
    sample_comm_venc_exit();
    sample_comm_vo_exit();
    sample_comm_vpss_exit();
    sample_comm_vi_exit();
    sample_comm_vgs_exit();
    sample_comm_sys_exit();
}

xmedia_void sample_svp_handle_sig(xmedia_s32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo || SIGTERM == signo) {
        g_force_exit = XMEDIA_TRUE;
    }
}

xmedia_s32 sample_svp_vi_online_vpss_online(sample_svp_alg_type detect_type)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 blk_size = 0;
    xmedia_video_size pic_size = { 0 };
    xmedia_video_size sub_pic_size = { 0 };
    xmedia_video_size alg_big_pic_size = { 0 };
    sample_comm_video_param video_param = { 0 };
    sample_sys_config sys_config = { 0 };
    sample_svp_info svp_info;

    xmedia_s32 vi_dev = 0;
    xmedia_s32 vi_pipe = 0;
    xmedia_s32 vi_chn = 0;
    sample_vi_config vi_config = { 0 };

    xmedia_s32 sensor_type = sample_svp_sensor_type[0];
    xmedia_u32 framerate = 0;
    vi_sensor_info sensor_info = { 0 };
    sample_isp_param isp_param = { 0 };
    xmedia_isp_config isp_config = { 0 };
    xmedia_bool mirror = XMEDIA_FALSE;
    xmedia_bool flip = XMEDIA_FALSE;

    xmedia_s32 vpss_chnl = 0;
    xmedia_s32 vpss_pipe = 0;
    xmedia_s32 vpss_ochn[3] = { 0, 1, 2};
    sample_vpss_config vpss_config = { 0 };
    xmedia_video_size vpss_ochn_size[VPSS_MAX_OCHN_NUM] = { 0 };

    xmedia_s32 venc_chnl = 0;
    xmedia_s32 venc_chn[3] = { 0, 1, 2};
    xmedia_s32 venc_chn_cnt;
    sample_venc_config venc_config = { 0 };

    xmedia_s32 vo_dev = 0;
    xmedia_s32 vo_layer = vo_dev;
    xmedia_s32 vo_chn[1] = { 0 };
    xmedia_s32 vo_chn_num = 1;
    sample_vo_config_input vo_input = { 0 };
    sample_vo_config vo_config = { 0 };
    vo_input.square_sort = 1;
    vo_input.screen_type = sample_vo_screen_type[0];

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);

    // 这些类别需要1080p码流
    if (detect_type == SAMPLE_SVP_ALG_TYPE_ADAS ||
        detect_type == SAMPLE_SVP_ALG_TYPE_GESTURE_RECOGNITION ||
        detect_type == SAMPLE_SVP_ALG_TYPE_FACE_EMOTION ||
        detect_type == SAMPLE_SVP_ALG_TYPE_FACE_RECOGNITION ||
        detect_type == SAMPLE_SVP_ALG_TYPE_PLATE_RECOGNITION ||
        detect_type == SAMPLE_SVP_ALG_TYPE_VEHICLE_RECOGNITION ||
        detect_type == SAMPLE_SVP_ALG_TYPE_PLATE_VEHICLE_RECO) {
        venc_chn_cnt = 3;
        svp_info.big_stream = XMEDIA_TRUE;
    } else {
        venc_chn_cnt = 2;
        svp_info.big_stream = XMEDIA_FALSE;
    }

    // sys init
    sys_config.sys_conf.pipe_mode[0].vicap_viproc_mode = XMEDIA_WORK_MODE_ONLINE;
    sys_config.sys_conf.pipe_mode[0].viproc_vpss_mode = XMEDIA_WORK_MODE_ONLINE;
    sys_config.sys_conf.pipe_mode[0].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;

    sys_config.vb_conf.max_pool_cnt = 20;

    pic_size.width = sensor_info.width;
    pic_size.height = sensor_info.height;

    blk_size = sample_comm_sys_get_buffer_size(pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[1].block_size = blk_size;
    sys_config.vb_conf.common_pool[1].block_cnt = 4;

    // 这些类别是720P模型，不需要360P码流
    if (detect_type == SAMPLE_SVP_ALG_TYPE_PERSON_RCNN_720P ||
        detect_type == SAMPLE_SVP_ALG_TYPE_CAR_RCNN_720P ||
        detect_type == SAMPLE_SVP_ALG_TYPE_PET_RCNN_720P) {
        sub_pic_size.width = SVP_MID_STREAM_W;
        sub_pic_size.height = SVP_MID_STREAM_H;
    } else {
        sub_pic_size.width = SVP_SMALL_STREAM_W;
        sub_pic_size.height = SVP_SMALL_STREAM_H;
    }

    blk_size = sample_comm_sys_get_buffer_size(sub_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[2].block_size = blk_size;
    sys_config.vb_conf.common_pool[2].block_cnt = 3;

    if (svp_info.big_stream == XMEDIA_TRUE) {
        alg_big_pic_size.width = SVP_BIG_STREAM_W;
        alg_big_pic_size.height = SVP_BIG_STREAM_H;
        blk_size = sample_comm_sys_get_buffer_size(alg_big_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                                   video_param.data_width, video_param.compress_mode);
        sys_config.vb_conf.common_pool[3].block_size = blk_size;
        sys_config.vb_conf.common_pool[3].block_cnt = 3;
    }

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_svp_sys_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sys init failed !\n");
        return ret;
    }

    vi_config.dev_info[vi_dev].dev_en = XMEDIA_TRUE;
    vi_config.dev_info[vi_dev].dev_no = vi_dev;
    vi_config.dev_info[vi_dev].sensor_type = sensor_type;
    vi_config.pipe_info[vi_pipe].pipe_en = XMEDIA_TRUE;
    vi_config.pipe_info[vi_pipe].pipe_no = vi_pipe;
    vi_config.pipe_info[vi_pipe].chn_info[vi_chn].chn_en = XMEDIA_TRUE;
    vi_config.pipe_info[vi_pipe].chn_info[vi_chn].chn_no = vi_chn;
    vi_config.dev_bind_pipe[vi_dev].pipe[0] = vi_pipe;
    vi_config.dev_bind_pipe[vi_dev].pipe[1] = -1;

    isp_config.fps = framerate;
    isp_config.mode_config.work_mode = XMEDIA_ISP_WORK_MODE_MASTER;
    isp_config.mode_config.master_mode.blend_stat_enable = XMEDIA_FALSE;
    isp_config.mode_config.master_mode.slave_num = 0;
    isp_config.pixel_fmt = sensor_info.pixel_format;
    isp_config.size.height = sensor_info.height;
    isp_config.size.width = sensor_info.width;
    isp_config.wdr_mode = sensor_info.wdr_mode;

    // isp pipe init
    isp_param.pipe[vi_pipe] = vi_pipe;
    isp_param.isp_info[vi_pipe].isp_pipe_en = XMEDIA_TRUE;
    isp_param.isp_info[vi_pipe].isp_sensor_en = XMEDIA_TRUE;
    isp_param.isp_info[vi_pipe].sensor_type = sensor_type;
    isp_param.isp_info[vi_pipe].mirror = mirror;
    isp_param.isp_info[vi_pipe].flip = flip;
    memcpy(&(isp_param.isp_info[vi_pipe].isp_config), &isp_config, sizeof(xmedia_isp_config));

    // isp init
    ret = sample_comm_isp_init(&isp_param, &vi_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("isp init failed!\n");
        goto exit0;
    }

    // vi start
    ret = sample_comm_vi_start(&vi_config, &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("start vi failed!\n");
        goto exit1;
    }

    // isp start
    ret = sample_comm_isp_start(&isp_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("start isp failed!\n");
        goto exit2;
    }

    // vpss config
    ret = sample_comm_vpss_get_default_pipe_cfg(&vpss_config.pipe_info[vpss_pipe].pipe_config, pic_size, &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get default pipe cfg failed !\n");
        goto exit2;
    }

    vpss_ochn_size[0].width = sensor_info.width;
    vpss_ochn_size[0].height = sensor_info.height;
    vpss_ochn_size[1].width = sub_pic_size.width;
    vpss_ochn_size[1].height = sub_pic_size.height;
    if (svp_info.big_stream == XMEDIA_TRUE) {
        vpss_ochn_size[2].width = SVP_BIG_STREAM_W;
        vpss_ochn_size[2].height = SVP_BIG_STREAM_H;
    }

    vpss_config.pipe_info[vpss_pipe].pipe_en = XMEDIA_TRUE;
    vpss_config.pipe_info[vpss_pipe].pipe_no = vpss_pipe;

    vpss_chnl = vpss_ochn[0];
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_no = vpss_ochn[0];
    ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
                                                vpss_ochn_size[vpss_chnl], &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get default pipe cfg failed !\n");
        goto exit2;
    }

    vpss_chnl = vpss_ochn[1];
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_no = vpss_ochn[1];
    ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
                                                vpss_ochn_size[vpss_chnl], &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get default pipe cfg failed !\n");
        goto exit2;
    }

    if (svp_info.big_stream == XMEDIA_TRUE) {
        vpss_chnl = vpss_ochn[2];
        vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
        vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_no = vpss_ochn[2];
        ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
                                                    vpss_ochn_size[vpss_chnl], &video_param);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get default pipe cfg failed !\n");
            goto exit2;
        }
    }

    // vpss start
    ret = sample_comm_vpss_start(&vpss_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vpss start failed !\n");
        goto exit2;
    }

    if (g_venc_use == XMEDIA_TRUE) {
        // venc init
        app_liveserver_init();
        app_liveserver_start(vpss_ochn_size, venc_chn_cnt, framerate);

        venc_chnl = venc_chn[0];
        venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
        venc_config.chn_info[venc_chnl].payload_type = PT_H265;
        venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
        sample_comm_venc_get_default_chn_info(vpss_ochn_size[0], framerate, &venc_config.chn_info[venc_chnl]);
        venc_config.chn_info[venc_chnl].rcn_ref_share_buf = XMEDIA_TRUE;

        venc_chnl = venc_chn[1];
        venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
        venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
        venc_config.chn_info[venc_chnl].payload_type = PT_H265;
        venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
        sample_comm_venc_get_default_chn_info(vpss_ochn_size[1], framerate, &venc_config.chn_info[venc_chnl]);
        venc_config.chn_info[venc_chnl].rcn_ref_share_buf = XMEDIA_TRUE;

        if (svp_info.big_stream == XMEDIA_TRUE) {
            venc_chnl = venc_chn[2];
            venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
            venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
            venc_config.chn_info[venc_chnl].payload_type = PT_H265;
            venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
            sample_comm_venc_get_default_chn_info(vpss_ochn_size[2], framerate, &venc_config.chn_info[venc_chnl]);
            venc_config.chn_info[venc_chnl].rcn_ref_share_buf = XMEDIA_TRUE;
        }

        ret = sample_comm_venc_start(&venc_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("venc start failed !\n");
            goto exit3;
        }

        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind venc failed !\n");
            goto exit4;
        }

        ret = sample_comm_venc_start_get_stream(venc_chn, venc_chn_cnt, XMEDIA_FALSE, XMEDIA_FALSE, "./output");
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("venc start get stream failed !\n");
            goto exit5;
        }
    } else {
        // vo init
        ret = sample_comm_vo_get_default_config(vo_input, &vo_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get vo default config failed !\n");
            goto exit3;
        }

        ret = sample_comm_vo_start(vo_dev, vo_layer, vo_chn, vo_chn_num, XMEDIA_TRUE, &vo_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vo start failed !\n");
            goto exit3;
        }

        ret = sample_comm_sys_vpss_bind_vo(vpss_pipe, vpss_ochn[0], vo_layer, vo_chn[0]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind vo failed !\n");
            goto exit4;
        }
    }

    svp_info.vpss_ochn[0] = vpss_ochn[0];
    svp_info.vpss_ochn[1] = vpss_ochn[1];
    svp_info.vpss_ochn[2] = vpss_ochn[2];
    svp_info.venc_chn[0] = venc_chn[0];
    svp_info.venc_chn[1] = venc_chn[1];
    svp_info.venc_chn[2] = venc_chn[2];
    svp_info.vpss_pipe = vpss_pipe;
    //svp_info.venc_chnl = 1;
    svp_info.detect_type = detect_type;
    svp_info.svp_handle = -1;

    ret = sample_svp_start(&svp_info);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("svp start failed !\n");
        goto exit6;
    }

    PAUSE(g_force_exit);

    sample_svp_stop(&svp_info);


exit6:
    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_venc_stop_get_stream();
        app_liveserver_stop();
        app_liveserver_deinit();
    }

exit5:
    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
    } else {
        sample_comm_sys_vpss_unbind_vo(vpss_pipe, vpss_ochn[0], vo_layer, vo_chn[0]);
    }

exit4:
    if (g_venc_use == XMEDIA_TRUE) {
        sample_comm_venc_stop(&venc_config);
    } else {
        sample_comm_vo_stop(vo_dev, vo_layer, vo_chn, vo_chn_num);
    }

exit3:
    sample_comm_vpss_stop(&vpss_config);

exit2:
    sample_comm_isp_stop(&isp_param);

exit1:
    sample_comm_vi_stop(&vi_config);

exit0:
    sample_comm_isp_exit(&isp_param);
    sample_svp_sys_exit();

    return ret;
}

void sample_svp_usage(char* prg_name)
{
    printf("Usage : %s <index> \n", prg_name);
    printf("index:\n");
    printf("\t  0) person detect \n");
    printf("\t  1) person keypoint detect \n");
    printf("\t  2) face detect \n");
    printf("\t  3) pet detect \n");
    printf("\t  4) car detect \n");
    printf("\t  5) non motorized vehicle detect \n");
    printf("\t  6) head detect \n");
    printf("\t  7) fireworks detect \n");
    printf("\t  8) package detect \n");
    printf("\t  9) aov person \n");
    printf("\t  10) adas detect \n");
    printf("\t  11) fire detect \n");
    printf("\t  12) gesture recognition \n");
    printf("\t  13) face emotion classification \n");
    printf("\t  14) face recognition \n");
    printf("\t  15) person rcnn detect \n");
    printf("\t  16) car rcnn detect \n");
    printf("\t  17) pet rcnn detect \n");
    printf("\t  18) companion robot \n");
    printf("\t  19) person rcnn 720p detect \n");
    printf("\t  20) car rcnn 720p detect \n");
    printf("\t  21) pet rcnn 720p detect \n");
    printf("\t  22) multi detect \n");
    printf("\t  23) person and car detect \n");
    printf("\t  24) dms detect \n");
    printf("\t  25) plate reco detect \n");
    printf("\t  26) vehicle reco detect \n");
    printf("\t  27) plate vehicle reco detect \n");
    return;
}

int main(int argc, char **argv)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    sample_svp_alg_type type = 0;

#ifdef USE_OSD
    // font init
    ret = canvas_font_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("svp canvas_font_init failed !\n");
        return 0;
    }
#endif

    signal(SIGINT, sample_svp_handle_sig);
    signal(SIGTERM, sample_svp_handle_sig);

    if (argc != 2) {
        sample_svp_usage(argv[0]);
        return 0;
    }

    type = atoi(argv[1]);
    ret = sample_svp_vi_online_vpss_online(type);
    if (XMEDIA_SUCCESS == ret) {
        SAMPLE_PRT("program exit normally!\n");
    } else {
        SAMPLE_PRT("program exit abnormally!\n");
    }

    return 0;
}
