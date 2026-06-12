/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "sample_comm.h"
#include "sample_comm_vpss.h"
#include "sample_comm_vi.h"
#include "sample_comm_sys.h"
#include "sample_comm_isp.h"

#include "xmedia_sys.h"
#include "xmedia_mmz.h"
#include "xmedia_vb.h"
#include "xmedia_vpss.h"
#include "xmedia_vgs.h"

#include "video_stream.h"
#include "sample_video.h"
#include "sample_video_yuv.h"

#include "venc_log.h"
#include "frame_cache.h"
#include "debug.h"

#define v4l2_fourcc(a, b, c, d) \
    ((uint32_t)(a) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

#define V4L2_PIX_FMT_YUYV v4l2_fourcc('Y', 'U', 'Y', 'V')   /* 16  YUV 4:2:2     */
#define V4L2_PIX_FMT_NV21 v4l2_fourcc('N', 'V', '2', '1')   /* 12  Y/CrCb 4:2:0  */
#define V4L2_PIX_FMT_NV12 v4l2_fourcc('N', 'V', '1', '2')   /* 12  Y/CbCr 4:2:0  */
#define V4L2_PIX_FMT_YUV420 v4l2_fourcc('Y', 'U', '1', '2') /* 12  YUV 4:2:0     */
#define V4L2_PIX_FMT_YVU420 v4l2_fourcc('Y', 'V', '1', '2') /* 12  YUV 4:2:0     */

static int g_tag_fmt = V4L2_PIX_FMT_YUYV;

static pthread_t g_recv_pid;
static int g_start = 0;
static int g_recv = 0;
static xmedia_u32 g_vpss_depth_flag = 0;
//static xmedia_u32 g_vpss_ori_depth = 0;

//common
static sample_comm_sensor_type g_sensor_type = SENSOR0_TYPE;
static sample_comm_video_param g_video_param = {0};

// vi config
static xmedia_s32 g_vi_dev  = 0; /* 0:单板SENSOR0; 2:单板SENSOR2 */
static xmedia_s32 g_vi_chn  = 0;
static xmedia_s32 g_vi_pipe = 0;
static xmedia_u32 g_vgs_pool_id   = 0;
static sample_vi_config g_vi_config = {0};

// isp config
static sample_isp_param g_isp_param = {0};

// vpss config
static xmedia_s32 g_vpss_pipe = 0;
static xmedia_s32 g_vpss_ichn = 0;
static xmedia_s32 g_vpss_ochn = 0;
static sample_vpss_config g_vpss_config = {0};
static xmedia_video_frame_info g_vpss_frame_info = {0};

static int sample_yuv_create_yuyv_pool(void)
{
    xmedia_vb_pool_config pool_config;
    vi_sensor_info sensor_info = {0};

    sample_comm_vi_get_sensor_info(g_sensor_type, &sensor_info);

    pool_config.block_cnt = 1;
    pool_config.block_size = sensor_info.width * sensor_info.height * 2;
    pool_config.map_mode = XMEDIA_VB_MAP_MODE_NONE;
    g_vgs_pool_id = xmedia_vb_create_pool(&pool_config);
    if (g_vgs_pool_id == -1) {
        venc_logd("resolution: w(%d) x h(%d).\n", sensor_info.width, sensor_info.height);
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

static int vgs_nv12_to_yuyv(xmedia_video_frame_info* vpss_frame, xmedia_video_frame_info* scale_frame)
{
    xmedia_s32 ret;
    xmedia_vgs_frame_info task_info;
    xmedia_s32 job_handle;
    xmedia_handle vb_handle;
    vi_sensor_info sensor_info = {0};
    xmedia_vb_base_info base_info = {0};
    xmedia_vb_cal_cfg vb_cal_cfg = {0};

    sample_comm_vi_get_sensor_info(g_sensor_type, &sensor_info);

    base_info.width = vpss_frame->frame.width;
    base_info.height = vpss_frame->frame.height;
    base_info.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YUYV_PACKAGE_422; //XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_422;
    base_info.bit_width = vpss_frame->frame.bit_width;
    base_info.cmp_mode = vpss_frame->frame.compress_mode;
    base_info.align = 8;
    xmedia_vb_get_buffer_config(&base_info, &vb_cal_cfg);

    vb_handle = xmedia_vb_get_block(g_vgs_pool_id, sensor_info.width * sensor_info.height * 2, XMEDIA_NULL);
    if (vb_handle == VB_INVALID_HANDLE) {
        SAMPLE_PRT("xmedia_vb_get_block(size:%d) failed!\n", sensor_info.width * sensor_info.height * 2);
        return XMEDIA_FAILURE;
    }

    ret = xmedia_vgs_init();
    if (ret != XMEDIA_SUCCESS) {
        venc_loge("vgs init failed\n");
        return ret;
    }

    ret = xmedia_vgs_create_job(&job_handle);
    if (ret != XMEDIA_SUCCESS) {
        venc_loge("create vgs job failed\n");
        xmedia_vgs_exit();
        goto EXIT1;
    }

    memset(scale_frame, 0, sizeof(xmedia_video_frame_info));
    scale_frame->pool_id = g_vgs_pool_id;
    scale_frame->frame.width = vpss_frame->frame.width;
    scale_frame->frame.height = vpss_frame->frame.height;
    scale_frame->frame.dynamic_range = XMEDIA_VIDEO_DYNAMIC_RANGE_SDR;
    scale_frame->frame.color_info.color_gamut = XMEDIA_VIDEO_COLOR_GAMUT_BT709;
    scale_frame->frame.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YUYV_PACKAGE_422; //XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_422; XMEDIA_VIDEO_PIXEL_FMT_YUYV_PACKAGE_422
    scale_frame->frame.pts  = vpss_frame->frame.pts;
    scale_frame->frame.compress_mode = vpss_frame->frame.compress_mode;
    scale_frame->frame.video_fmt  = vpss_frame->frame.video_fmt;
    scale_frame->frame.bit_width = vpss_frame->frame.bit_width;
    scale_frame->mod_id = MOD_ID_USER;
    scale_frame->frame.addr.y_head_phy_addr = xmedia_vb_handle_to_phy_addr(vb_handle);
    scale_frame->frame.addr.c_head_phy_addr = scale_frame->frame.addr.y_head_phy_addr + vb_cal_cfg.head_y_size;
    scale_frame->frame.stride.y_head_stride  = vb_cal_cfg.head_y_stride;
    scale_frame->frame.stride.c_head_stride  = vb_cal_cfg.head_c_stride;
    scale_frame->frame.addr.y_phy_addr  = scale_frame->frame.addr.y_head_phy_addr + vb_cal_cfg.head_size;
    scale_frame->frame.addr.c_phy_addr  = scale_frame->frame.addr.y_head_phy_addr + vb_cal_cfg.main_y_size;
    scale_frame->frame.stride.y_stride = vb_cal_cfg.main_stride;
    scale_frame->frame.stride.c_stride = vb_cal_cfg.main_stride;

    memcpy(&task_info.img_in, vpss_frame, sizeof(xmedia_video_frame_info));
    memcpy(&task_info.img_out, scale_frame, sizeof(xmedia_video_frame_info));
    ret = xmedia_vgs_add_task_scale(job_handle, &task_info, XMEDIA_VIDEO_SCALE_MODE_NORMAL);
    if(ret != XMEDIA_SUCCESS) {
        xmedia_vgs_cancel_job(job_handle);
        SAMPLE_PRT("xmedia_vgs_add_task_scale failed!\n");
        goto EXIT2;
    }

    ret = xmedia_vgs_submit_job(job_handle);
    if (ret != XMEDIA_SUCCESS) {
        venc_loge("submit vgs job failed\n");
        goto EXIT2;
    }

    ret = xmedia_vgs_wait_job(job_handle, 2000);
    if (ret != XMEDIA_SUCCESS) {
        venc_loge("wait vgs job failed\n");
        goto EXIT1;
    }

    xmedia_vgs_exit();

    return XMEDIA_SUCCESS;

EXIT2:
    xmedia_vgs_cancel_job(job_handle);
EXIT1:
    xmedia_vgs_exit();

    return XMEDIA_FAILURE;
}

static int sample_yuv_sys_init(int sensor_type)
{
    xmedia_s32 ret, idx;
    xmedia_s32 blk_size = 0;
    vi_sensor_info sensor_info = {0};
    xmedia_video_size pic_size = {0};
    sample_sys_config sys_config = {0};

    func_entry();

    g_video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    g_video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420; /* NV21 */
    g_video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    g_video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);

    // sys init
    sys_config.sys_conf.pipe_mode[g_vi_pipe].vicap_viproc_mode = XMEDIA_WORK_MODE_ONLINE;
    sys_config.sys_conf.pipe_mode[g_vi_pipe].viproc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[g_vi_pipe].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;

    sys_config.vb_conf.max_pool_cnt = 25;

    pic_size.width = sensor_info.width;
    pic_size.height = sensor_info.height;

    venc_logd("sensor resolution: w(%d) x h(%d).\n", pic_size.width, pic_size.height);

    idx = 0;

    if (sys_config.sys_conf.pipe_mode[g_vi_pipe].vicap_viproc_mode == XMEDIA_WORK_MODE_OFFLINE) {
        blk_size = sample_comm_sys_get_buffer_size(pic_size, g_video_param.video_fmt, sensor_info.pixel_format,
                sensor_info.bit_width, g_video_param.compress_mode);
        sys_config.vb_conf.common_pool[idx].block_size = blk_size;
        sys_config.vb_conf.common_pool[idx].block_cnt = 2;

        idx++;
    }

    if (sys_config.sys_conf.pipe_mode[g_vi_pipe].viproc_vpss_mode == XMEDIA_WORK_MODE_OFFLINE) {
        blk_size = sample_comm_sys_get_buffer_size(pic_size, g_video_param.video_fmt, g_video_param.pixel_fmt,
                g_video_param.data_width, g_video_param.compress_mode);
        sys_config.vb_conf.common_pool[idx].block_size = blk_size;
        sys_config.vb_conf.common_pool[idx].block_cnt = 3;

        idx++;
    }

    get_user_format(NULL, &pic_size, NULL);
    venc_logi("user resolution: %d(w) x %d(h)\n", pic_size.width, pic_size.height);
    blk_size = sample_comm_sys_get_buffer_size(pic_size, g_video_param.video_fmt, g_video_param.pixel_fmt,
            g_video_param.data_width, g_video_param.compress_mode);
    sys_config.vb_conf.common_pool[idx].block_size = blk_size;
    sys_config.vb_conf.common_pool[idx].block_cnt = 3;

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_comm_sys_init(&sys_config);
    if(ret != XMEDIA_SUCCESS) {
        venc_loge("sample_comm_sys_init failed!\n");
        return ret;
    }

    //in online-online mode,vi and vpss must be reset at the same time
    ret = sample_comm_vi_init();
    if(ret != XMEDIA_SUCCESS) {
        venc_loge("sample_comm_vi_init failed!\n");
        return ret;
    }

    ret = sample_comm_vpss_init();
    if(ret != XMEDIA_SUCCESS) {
        venc_loge("sample_comm_vpss_init failed!\n");
        return ret;
    }

    func_success();

    return XMEDIA_SUCCESS;
}

static int sample_yuv_sys_deinit(void)
{
    sample_comm_vpss_exit();

    sample_comm_vi_exit();

    sample_comm_sys_exit();

    return XMEDIA_SUCCESS;
}

static xmedia_s32 vpss_ochn_dump_transform_format(xmedia_video_frame_info *frame_in,
        xmedia_video_frame_info *tmp_frame, xmedia_video_frame_info *frame_out)
{
    xmedia_s32 ret;
    xmedia_vgs_frame_info task_info;
    xmedia_s32 job_handle;

    ret = xmedia_vgs_init();
    if (ret != XMEDIA_SUCCESS) {
        venc_loge("vgs init failed\n");
        return ret;
    }

    ret = xmedia_vgs_create_job(&job_handle);
    if (ret != XMEDIA_SUCCESS) {
        venc_loge("create vgs job failed\n");
        xmedia_vgs_exit();
        goto EXIT1;
    }

    if (frame_in->frame.video_fmt == XMEDIA_VIDEO_FMT_TILE_16x4) {
        memcpy(&task_info.img_in, frame_in, sizeof(xmedia_video_frame_info));
        memcpy(&task_info.img_out, tmp_frame, sizeof(xmedia_video_frame_info));
        ret = xmedia_vgs_add_task_rotation(job_handle, &task_info, XMEDIA_VIDEO_ROTATION_0);
        if (ret != XMEDIA_SUCCESS) {
            venc_loge("add vgs task failed\n");
            goto EXIT2;
        }
    }

    memcpy(&task_info.img_in, tmp_frame, sizeof(xmedia_video_frame_info));
    memcpy(&task_info.img_out, frame_out, sizeof(xmedia_video_frame_info));
    ret = xmedia_vgs_add_task_rotation(job_handle, &task_info, XMEDIA_VIDEO_ROTATION_0);
    if (ret != XMEDIA_SUCCESS) {
        venc_loge("add vgs task failed\n");
        goto EXIT2;
    }

    ret = xmedia_vgs_submit_job(job_handle);
    if (ret != XMEDIA_SUCCESS) {
        venc_loge("submit vgs job failed\n");
        goto EXIT2;
    }

    ret = xmedia_vgs_wait_job(job_handle, 2000);
    if (ret != XMEDIA_SUCCESS) {
        venc_loge("wait vgs job failed\n");
        goto EXIT1;
    }

    xmedia_vgs_exit();

    return XMEDIA_SUCCESS;

EXIT2:
    xmedia_vgs_cancel_job(job_handle);
EXIT1:
    xmedia_vgs_exit();

    return XMEDIA_FAILURE;
}

static int nv21_to_nv21(unsigned char* src_nv21, unsigned int width, unsigned int height,
        unsigned char* nv21, unsigned int nv21_len)
{
    int copy_size;

    copy_size = width * height * 3 / 2;
    if (copy_size > nv21_len) {
        return 0;
    }

    memcpy(nv21, src_nv21, copy_size);

    return copy_size;
}

static int nv21_to_yuyv(unsigned char* nv21, unsigned int width, unsigned int height,
        unsigned char* yuyv, unsigned int yuyv_len)
{
    int copy_size;

    copy_size = width * height * 2;
    if (copy_size > yuyv_len) {
        return 0;
    }

    memcpy(yuyv, nv21, copy_size);

    return copy_size;
}

static void sample_yuv_save_data(unsigned char* yuv, unsigned int src_fmt,
        unsigned int width, unsigned int height)
{
    unsigned int copy_size = 0;
    uvc_cache_t *uvc_cache = uvc_cache_get();
    frame_node_t *fnode = NULL;

    if (uvc_cache) {
        get_node_from_queue(uvc_cache->free_queue, &fnode);
    }

    if (!fnode) {
        venc_logd("drop frame.\n");
        return;
    }

    switch (g_tag_fmt) {
        case V4L2_PIX_FMT_YUYV:
            copy_size = nv21_to_yuyv(yuv, width, height, fnode->mem, fnode->length);
            break;

        case V4L2_PIX_FMT_NV21:
            copy_size = nv21_to_nv21(yuv, width, height, fnode->mem, fnode->length);
            break;

        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_YVU420:
        default:
            venc_logw("send default format NV21.\n");
            copy_size = nv21_to_nv21(yuv, width, height, fnode->mem, fnode->length);
            break;
    }


    if (copy_size == 0) {
        venc_loge("cache size is to small.\n");
    }

    fnode->used = copy_size;

    venc_logd("send size(%u)\n", fnode->used);

    put_node_to_queue(uvc_cache->ok_queue, fnode);
}

//static int flag = 0;
static void sample_yuv_save_frame(xmedia_video_frame_info vpss_frame)
{
    char* virt_addr_y;
    xmedia_u64 phy_addr;
    xmedia_u32 size;
    xmedia_char* page_addr[2];
    xmedia_video_frame *video_frame = &vpss_frame.frame;
    xmedia_video_pixel_format pixel_fmt = video_frame->pixel_fmt;
    xmedia_video_frame_info scale_frame;

    if (g_tag_fmt == V4L2_PIX_FMT_YUYV) {
        vgs_nv12_to_yuyv(&vpss_frame, &scale_frame);
    }

    if (video_frame->compress_mode != XMEDIA_VIDEO_COMPRESS_MODE_NONE) {
        video_frame->stride.y_stride = video_frame->width;
        video_frame->stride.c_stride = video_frame->width;
    }

    if ((pixel_fmt == XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420) ||
        (pixel_fmt == XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_420)) {
        size = video_frame->stride.y_stride * video_frame->height * 3 / 2;
    } else if (pixel_fmt == XMEDIA_VIDEO_PIXEL_FMT_YUV_400) {
        size = video_frame->stride.y_stride * video_frame->height;
    } else {
        size = video_frame->stride.y_stride * video_frame->height * 3 / 2;
    }

    if (g_tag_fmt == V4L2_PIX_FMT_YUYV) {
        phy_addr = scale_frame.frame.addr.y_phy_addr;
        size = video_frame->stride.y_stride * video_frame->height * 2;
        pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YUYV_PACKAGE_422; //XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_422;
    } else {
        phy_addr = video_frame->addr.y_phy_addr;
    }

    page_addr[0] = (xmedia_char*)xmedia_mmz_map(phy_addr, size, XMEDIA_FALSE);
    if (XMEDIA_NULL == page_addr[0]) {
        venc_loge("xmedia_mmz_map failed.\n");
        return;
    }

    virt_addr_y = page_addr[0];

    debug_dump_frame((char*)virt_addr_y, size, "frame_vpss.yuv");
    sample_yuv_save_data((unsigned char*)virt_addr_y, pixel_fmt,
            video_frame->stride.y_stride, video_frame->height);

    xmedia_mmz_unmap(page_addr[0]);
    page_addr[0] = XMEDIA_NULL;
    if (g_tag_fmt == V4L2_PIX_FMT_YUYV) {
        xmedia_vb_release_block(xmedia_vb_phy_addr_to_handle(scale_frame.frame.addr.y_head_phy_addr));
    }
}

void* sample_yuv_stream_proc(void* arg)
{
    int ret;
    xmedia_vb_base_info base_info;
    xmedia_vb_cal_cfg cal_cfg;
    xmedia_s32 millisec = -1;
    xmedia_handle vb_handle = VB_INVALID_HANDLE;
    xmedia_handle tmp_handle = VB_INVALID_HANDLE;
    xmedia_video_frame_info tmp_frame = {0};
    xmedia_video_frame_info frame_out = {0};

    g_vpss_depth_flag = 1;

    while (g_recv) {
        memset(&g_vpss_frame_info, 0, sizeof(xmedia_video_frame_info));
        g_vpss_frame_info.pool_id = VB_INVALID_POOLID;

        ret = xmedia_vpss_acquire_ochn_frame(g_vpss_pipe, g_vpss_ochn, &g_vpss_frame_info, millisec);
        if (ret != XMEDIA_SUCCESS) {
            venc_logi("piipe(%d) ochn(%d) get frame failed!\n", g_vpss_pipe, g_vpss_ochn);
            usleep(1000);
            continue;
        }

        venc_logd("pipe(%d) ochn(%d) video_fmt(%d) save frame!\n", g_vpss_pipe, g_vpss_ochn,
                g_vpss_frame_info.frame.video_fmt);

        if (g_vpss_frame_info.frame.video_fmt != XMEDIA_VIDEO_FMT_LINEAR) {
            memset(&base_info, 0, sizeof(xmedia_vb_base_info));
            base_info.align = 0;
            base_info.bit_width = g_vpss_frame_info.frame.bit_width;
            base_info.cmp_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;
            base_info.width = g_vpss_frame_info.frame.width;
            base_info.height = g_vpss_frame_info.frame.height;
            base_info.pixel_fmt = g_vpss_frame_info.frame.pixel_fmt;

            if (g_vpss_frame_info.frame.video_fmt == XMEDIA_VIDEO_FMT_TILE_16x4) {
                base_info.video_fmt = XMEDIA_VIDEO_FMT_TILE_64x4;
                ret = xmedia_vb_get_buffer_config(&base_info, &cal_cfg);
                if (ret != XMEDIA_SUCCESS) {
                    venc_logw("get buffer config failed with 0x%x\n", ret);
                    continue;
                }

                tmp_handle = xmedia_vb_get_block(-1, cal_cfg.vb_size, NULL);
                if (tmp_handle == VB_INVALID_HANDLE) {
                    venc_logw("get block failed\n");
                    continue;
                }

                memcpy(&tmp_frame, &g_vpss_frame_info, sizeof(xmedia_video_frame_info));
                tmp_frame.pool_id = xmedia_vb_handle_to_pool_id(tmp_handle);
                tmp_frame.frame.addr.y_phy_addr = xmedia_vb_handle_to_phy_addr(tmp_handle);
                tmp_frame.frame.addr.y_head_phy_addr = tmp_frame.frame.addr.y_phy_addr;
                tmp_frame.frame.addr.c_phy_addr = tmp_frame.frame.addr.y_phy_addr + cal_cfg.main_y_size;
                tmp_frame.frame.stride.y_stride = cal_cfg.main_stride;
                tmp_frame.frame.stride.c_stride = cal_cfg.main_stride;
                tmp_frame.frame.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;
                tmp_frame.frame.video_fmt = XMEDIA_VIDEO_FMT_TILE_64x4;
            } else {
                memcpy(&tmp_frame, &g_vpss_frame_info, sizeof(xmedia_video_frame_info));
            }

            base_info.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
            ret = xmedia_vb_get_buffer_config(&base_info, &cal_cfg);
            if (ret != XMEDIA_SUCCESS) {
                if (tmp_handle != VB_INVALID_HANDLE) {
                    xmedia_vb_release_block(tmp_handle);
                    tmp_handle = VB_INVALID_HANDLE;
                }
                venc_logw("get buffer config failed with 0x%x\n", ret);
                continue;
            }

            vb_handle = xmedia_vb_get_block(-1, cal_cfg.vb_size, NULL);
            if (vb_handle == VB_INVALID_HANDLE) {
                if (tmp_handle != VB_INVALID_HANDLE) {
                    xmedia_vb_release_block(tmp_handle);
                    tmp_handle = VB_INVALID_HANDLE;
                }
                venc_logw("get block failed\n");
                continue;
            }

            memcpy(&frame_out, &tmp_frame, sizeof(xmedia_video_frame_info));
            frame_out.pool_id = xmedia_vb_handle_to_pool_id(vb_handle);
            frame_out.frame.addr.y_phy_addr = xmedia_vb_handle_to_phy_addr(vb_handle);
            frame_out.frame.addr.y_head_phy_addr = frame_out.frame.addr.y_phy_addr;
            frame_out.frame.addr.c_phy_addr = frame_out.frame.addr.y_phy_addr + cal_cfg.main_y_size;
            frame_out.frame.stride.y_stride = cal_cfg.main_stride;
            frame_out.frame.stride.c_stride = cal_cfg.main_stride;
            frame_out.frame.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;
            frame_out.frame.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;

            ret = vpss_ochn_dump_transform_format(&g_vpss_frame_info, &tmp_frame, &frame_out);
            if (ret != XMEDIA_SUCCESS) {
                if (tmp_handle != VB_INVALID_HANDLE) {
                    xmedia_vb_release_block(tmp_handle);
                    tmp_handle = VB_INVALID_HANDLE;
                }

                if (vb_handle != VB_INVALID_HANDLE) {
                    xmedia_vb_release_block(vb_handle);
                    vb_handle = VB_INVALID_HANDLE;
                }
                venc_logw("pipe %d ochn %d vpss_ochn_dump_transform_format failed\n", g_vpss_pipe, g_vpss_ochn);
                continue;
            }
        } else {
            memcpy(&frame_out, &g_vpss_frame_info, sizeof(xmedia_video_frame_info));
        }

        sample_yuv_save_frame(frame_out);

        if (tmp_handle != VB_INVALID_HANDLE) {
            xmedia_vb_release_block(tmp_handle);
            tmp_handle = VB_INVALID_HANDLE;
        }

        if (vb_handle != VB_INVALID_HANDLE) {
            xmedia_vb_release_block(vb_handle);
            vb_handle = VB_INVALID_HANDLE;
        }

        ret = xmedia_vpss_release_ochn_frame(g_vpss_pipe, g_vpss_ochn, &g_vpss_frame_info);
        if (ret != XMEDIA_SUCCESS) {
            printf("pipe(%d) ochn(%d) release frame failed!\n", g_vpss_pipe, g_vpss_ochn);
        }
    }

    g_vpss_frame_info.pool_id = VB_INVALID_POOLID;

    if (tmp_handle != VB_INVALID_HANDLE) {
        xmedia_vb_release_block(tmp_handle);
        tmp_handle = VB_INVALID_HANDLE;
    }

    if (vb_handle != VB_INVALID_HANDLE) {
        xmedia_vb_release_block(vb_handle);
        vb_handle = VB_INVALID_HANDLE;
    }

    return NULL;
}

static int sample_yuv_start_get_stream(int venc_chn, int cnt)
{
    int rc;

    g_recv = 1;

    rc = pthread_create(&g_recv_pid, NULL, sample_yuv_stream_proc, NULL);
    if (rc < 0) {
        venc_loge("Create sample_yuv_stream_proc failed.\n");
        return -1;
    }

    return 0;
}

static int sample_yuv_stop_get_stream(void)
{

    if (g_recv == 1) {
        g_recv = 0;
        pthread_join(g_recv_pid, 0);
    }

    return 0;
}

static int sample_yuv_normalp_classic(void)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;

    xmedia_u32 framerate = 0, fps_user = 0;
    xmedia_video_size pic_size = {0};
    xmedia_video_size vpss_ochn_size = {0};
    xmedia_isp_config isp_config = {0};
    xmedia_payload_type en_payload = PT_MJPEG;
    vi_sensor_info sensor_info = {0};

    g_vi_config.dev_info[g_vi_dev].dev_en = XMEDIA_TRUE;
    g_vi_config.dev_info[g_vi_dev].dev_no = g_vi_dev;
    g_vi_config.dev_info[g_vi_dev].sensor_type = g_sensor_type;
    g_vi_config.pipe_info[g_vi_pipe].pipe_en = XMEDIA_TRUE;
    g_vi_config.pipe_info[g_vi_pipe].pipe_no = g_vi_pipe;
    g_vi_config.pipe_info[g_vi_pipe].chn_info[g_vi_chn].chn_en = XMEDIA_TRUE;
    g_vi_config.pipe_info[g_vi_pipe].chn_info[g_vi_chn].chn_no = g_vi_chn;
    g_vi_config.dev_bind_pipe[g_vi_dev].pipe[0] = g_vi_pipe;
    g_vi_config.dev_bind_pipe[g_vi_dev].pipe[1] = -1;

    sample_comm_vi_get_sensor_info(g_sensor_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(g_sensor_type, &framerate);

    get_user_format(NULL, NULL, &fps_user);
    if (fps_user > framerate) {
        venc_logw("User framerate(%d) > sensor framerate(%d), used sensor framerate\n", fps_user, framerate);
    } else {
        framerate = fps_user;
    }

    venc_logi("isp framerate(%d)\n", framerate);
    isp_config.fps = framerate;
    isp_config.mode_config.work_mode = XMEDIA_ISP_WORK_MODE_MASTER;
    isp_config.mode_config.master_mode.blend_stat_enable = XMEDIA_FALSE;
    isp_config.mode_config.master_mode.slave_num = 0;
    isp_config.pixel_fmt = sensor_info.pixel_format;
    isp_config.size.height = sensor_info.height;
    isp_config.size.width = sensor_info.width;
    pic_size.height = sensor_info.height;
    pic_size.width = sensor_info.width;
    isp_config.wdr_mode = sensor_info.wdr_mode;

    // isp pipe init
    g_isp_param.pipe[g_vi_pipe] = g_vi_pipe;
    g_isp_param.isp_info[g_vi_pipe].sensor_type = g_sensor_type;
    g_isp_param.isp_info[g_vi_pipe].flip = XMEDIA_FALSE;
    g_isp_param.isp_info[g_vi_pipe].mirror = XMEDIA_FALSE;
    g_isp_param.isp_info[g_vi_pipe].isp_pipe_en = XMEDIA_TRUE;
    g_isp_param.isp_info[g_vi_pipe].isp_sensor_en = XMEDIA_TRUE;

    memcpy(&(g_isp_param.isp_info[g_vi_pipe].isp_config), &isp_config, sizeof(xmedia_isp_config));

    // isp init
    ret = sample_comm_isp_init(&g_isp_param, &g_vi_config);
    if (ret != XMEDIA_SUCCESS) {
        venc_loge("isp init failed!\n");
        goto exit0;
    }

    // vi start
    ret = sample_comm_vi_start(&g_vi_config, &g_video_param);
    if (ret != XMEDIA_SUCCESS) {
        venc_loge("start vi failed!\n");
        goto exit1;
    }

    // isp start
    ret = sample_comm_isp_start(&g_isp_param);
    if (ret != XMEDIA_SUCCESS) {
        venc_loge("start isp failed!\n");
        goto exit2;
    }

    // vpss config
    ret = sample_comm_vpss_get_default_pipe_cfg(&g_vpss_config.pipe_info[g_vpss_pipe].pipe_config, pic_size, &g_video_param);
    if (ret != XMEDIA_SUCCESS) {
        venc_loge("get default pipe cfg failed !\n");
        goto exit2;
    }

    get_user_format(&en_payload, &pic_size, NULL);
#if 0
    vpss_ochn_size.width = sensor_info.width;
    vpss_ochn_size.height = sensor_info.height;
#else
    vpss_ochn_size.width  = pic_size.width;
    vpss_ochn_size.height = pic_size.height;
#endif

    g_vpss_config.pipe_info[g_vpss_pipe].pipe_en = XMEDIA_TRUE;
    g_vpss_config.pipe_info[g_vpss_pipe].pipe_no = g_vpss_pipe;

    g_vpss_config.pipe_info[g_vpss_pipe].chn_info[g_vpss_ochn].chn_en = XMEDIA_TRUE;
    g_vpss_config.pipe_info[g_vpss_pipe].chn_info[g_vpss_ochn].chn_no = g_vpss_ochn;
    ret = sample_comm_vpss_get_default_ochn_cfg(&g_vpss_config.pipe_info[g_vpss_pipe].chn_info[g_vpss_ochn].chn_config,
            vpss_ochn_size, &g_video_param);
    if (ret != XMEDIA_SUCCESS) {
        venc_loge("get default pipe cfg failed !\n");
        goto exit2;
    }
#if 0
    ret = sample_comm_vpss_set_ochn_wrap(&g_vpss_config.pipe_info[g_vpss_pipe].chn_info[g_vpss_ochn]);
    if (ret != XMEDIA_SUCCESS) {
        venc_loge("get default pipe cfg failed !\n");
        goto exit2;
    }
#endif

    // vpss start
    ret = sample_comm_vpss_start(&g_vpss_config);
    if (ret != XMEDIA_SUCCESS) {
        venc_loge("vpss start failed !\n");
        goto exit2;
    }

    ret = sample_comm_sys_vi_bind_vpss(g_vi_pipe, g_vi_chn, g_vpss_pipe, g_vpss_ichn);
    if (ret != XMEDIA_SUCCESS) {
        venc_loge("vi bind vpss failed !\n");
        goto exit3;
    }

    return 0;

exit3:
    sample_comm_vpss_stop(&g_vpss_config);
exit2:
    sample_comm_isp_stop(&g_isp_param);
exit1:
    sample_comm_vi_stop(&g_vi_config);
exit0:
    sample_comm_isp_exit(&g_isp_param);

    sample_yuv_sys_deinit();

    return -1;
}

/*******************************************************
 * Processing Unit Operation Functions
 *******************************************************/

/* Processing Unit Operation Functions End */

/*******************************************************
 * Input Terminal Operation Functions
 *******************************************************/

/* Input Terminal Operation Functions End */

/*******************************************************
 * Stream Control Operation Functions
 *******************************************************/
int sample_yuv_set_idr(void)
{
    return 0;
}

int sample_yuv_init(void)
{
    g_sensor_type = SENSOR0_TYPE;
    sample_yuv_sys_init(g_sensor_type);

    return 0;
}

int sample_yuv_startup(int fmt)
{
    g_start = 1;

    if (sample_yuv_normalp_classic() < 0) {
        return -1;
    }

    g_tag_fmt = fmt;
    if (g_tag_fmt == V4L2_PIX_FMT_YUYV) {
        sample_yuv_create_yuyv_pool();
    }
    sample_yuv_start_get_stream(g_vpss_ochn, 1);
    return 0;
}

int sample_yuv_shutdown(void)
{
    if (!g_start) {
        return 0;
    }

    sample_yuv_stop_get_stream();

    sample_comm_sys_vi_unbind_vpss(g_vi_pipe, g_vi_chn, g_vpss_pipe, g_vpss_ichn);

    sample_comm_vpss_stop(&g_vpss_config);

    sample_comm_isp_stop(&g_isp_param);

    sample_comm_vi_stop(&g_vi_config);

    sample_comm_isp_exit(&g_isp_param);

    sample_yuv_sys_deinit();

    g_start = 0;

    return 0;
}
