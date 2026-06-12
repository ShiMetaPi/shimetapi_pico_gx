/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "sample_comm.h"
#include "sample_comm_venc.h"
#include "sample_comm_vpss.h"
#include "sample_comm_vi.h"
#include "sample_comm_sys.h"
#include "sample_comm_isp.h"

#include "video_stream.h"
#include "sample_video.h"
#include "sample_video_mjpeg.h"

#include "venc_log.h"
#include "frame_cache.h"

#include "debug.h"

extern unsigned int pack_h264_to_jpeg(unsigned char* frame_jpeg, unsigned int size_jpeg, unsigned int len_jpeg,
        unsigned char* frame_h264, unsigned int size_h264, unsigned int fps);

static int g_start = 0;
static pthread_t g_recv_pid;
static sample_venc_getstream_para g_st_param;

//common
static sample_comm_sensor_type g_sensor_type = SENSOR0_TYPE;
static sample_comm_video_param g_video_param = {0};

// vi config
static xmedia_s32 g_vi_dev  = 0; /* 0:单板SENSOR0; 2:单板SENSOR2 */
static xmedia_s32 g_vi_chn  = 0;
static xmedia_s32 g_vi_pipe = 0;
static sample_vi_config g_vi_config = {0};

// isp config
static sample_isp_param g_isp_param = {0};

// vpss config
static xmedia_s32 g_vpss_pipe = 0;
static xmedia_s32 g_vpss_ichn = 0;
static xmedia_s32 g_vpss_ochn_0 = 0;
static xmedia_s32 g_vpss_ochn_1 = 1;
static sample_vpss_config g_vpss_config = {0};

// venc config
static xmedia_s32 g_venc_chn_0 = 0;
static xmedia_s32 g_venc_chn_1 = 1;
static sample_venc_config g_venc_config = {0};

static int sample_mjpeg_h264_sys_init(int sensor_type)
{
    xmedia_s32 ret, idx;
    xmedia_s32 blk_size = 0;
    vi_sensor_info sensor_info = {0};
    xmedia_video_size pic_size = {0};
    sample_sys_config sys_config = {0};

    func_entry();

    g_video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    g_video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    g_video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    g_video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);

    // sys init
    sys_config.sys_conf.pipe_mode[g_vi_pipe].vicap_viproc_mode = XMEDIA_WORK_MODE_ONLINE;
    sys_config.sys_conf.pipe_mode[g_vi_pipe].viproc_vpss_mode = XMEDIA_WORK_MODE_ONLINE;
    sys_config.sys_conf.pipe_mode[g_vi_pipe].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;

    idx = 0;

    if (sys_config.sys_conf.pipe_mode[g_vi_pipe].vicap_viproc_mode == XMEDIA_WORK_MODE_OFFLINE) {
        pic_size.width = sensor_info.width;
        pic_size.height = sensor_info.height;

        blk_size = sample_comm_sys_get_buffer_size(pic_size, g_video_param.video_fmt, sensor_info.pixel_format,
                sensor_info.bit_width, g_video_param.compress_mode);
        sys_config.vb_conf.common_pool[idx].block_size = blk_size;
        sys_config.vb_conf.common_pool[idx].block_cnt = 2;

        idx++;
    }

    if (sys_config.sys_conf.pipe_mode[g_vi_pipe].viproc_vpss_mode == XMEDIA_WORK_MODE_OFFLINE) {
        pic_size.width = sensor_info.width;
        pic_size.height = sensor_info.height;

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
    sys_config.vb_conf.common_pool[idx].block_cnt = 6;

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

    ret = sample_comm_venc_init();
    if(ret != XMEDIA_SUCCESS) {
        venc_loge("sample_comm_venc_init failed!\n");
        return ret;
    }

    func_success();

    return XMEDIA_SUCCESS;
}

static int sample_mjpeg_h264_sys_deinit(void)
{
    sample_comm_venc_exit();

    sample_comm_vpss_exit();

    sample_comm_vi_exit();

    sample_comm_sys_exit();

    return XMEDIA_SUCCESS;
}

extern void memcpy_jpeg_user(unsigned char* src_buffer, unsigned int src_len,
        unsigned char* dest_buffer, unsigned int dest_length, unsigned int* dest_offset);

static int sample_copy_frame(xmedia_venc_stream* pst_stream, char* dst_buf, int buf_len, int buf_used, int type)
{
    int i = 0;
    xmedia_venc_pack *pst_data = NULL;
    unsigned char *s = NULL;
    unsigned int data_len = 0;
    unsigned int copy_size = 0;

    for (i = 0; i < pst_stream->pack_count; ++i) {
        pst_data = &pst_stream->pack[i];
        s = pst_data->vir_addr + pst_data->offset;
        data_len = pst_data->len - pst_data->offset;
        copy_size = data_len < (buf_len - buf_used) ? data_len : (buf_len - buf_used);

        if (copy_size > 0) {
            if (type == PT_MJPEG) {
                memcpy_jpeg_user(s, data_len, (unsigned char*)dst_buf, buf_len, (unsigned int *)&buf_used);
            } else {
                memcpy(dst_buf + buf_used, s, copy_size);
                buf_used += copy_size;
            }
        }

        if (data_len > copy_size) {
            venc_logw("WARN: missing pack.\n");
        }
    }

    return buf_used;
}

static char* g_h264 = NULL;
#define H264_BUFF_LEN 0x80000

extern unsigned int pack_h264_to_jpeg(unsigned char* frame_jpeg, unsigned int size_jpeg, unsigned int len_jpeg,
        unsigned char* frame_h264, unsigned int size_h264, unsigned int fps);

static void sample_save_h264(xmedia_venc_stream* pst_stream, frame_node_t* fnode)
{
    int ret;

    if (fnode->used == 0) {
        venc_loge("Not found mjpeg frame, drop current frame\n");
        return;
    }

    memset(g_h264, 0, H264_BUFF_LEN);
    ret = sample_copy_frame(pst_stream, g_h264, H264_BUFF_LEN, 0, PT_H264);

    venc_logd("h264_size = %d, fnode->used = %u, fnode->length = %d\n", ret, fnode->used, fnode->length);

    debug_dump_frame((char*)g_h264, ret, "frame.h264");
    debug_dump_frame((char*)fnode->mem, fnode->used, "frame.mjpeg");
    ret = pack_h264_to_jpeg((unsigned char*)fnode->mem, fnode->used, fnode->length,
            (unsigned char*)g_h264, ret, 30);

    debug_dump_frame((char*)fnode->mem, ret, "frame_mjpeg_h264h.bin");
    fnode->used = ret;
}

static void sample_save_mjpeg(xmedia_venc_stream* pst_stream, frame_node_t* fnode)
{
    int ret;

    ret = sample_copy_frame(pst_stream, (char*)fnode->mem, fnode->length, fnode->used, PT_MJPEG);

    fnode->used = ret;
}

static void sample_mjpeg_h264_save_stream(xmedia_payload_type en_type, xmedia_venc_stream *pst_stream, frame_node_t* fnode)
{
    if (PT_MJPEG == en_type) {
        sample_save_mjpeg(pst_stream, fnode);
    } else if (PT_H264 == en_type){
        sample_save_h264(pst_stream, fnode);
    }
}

void* sample_mjpeg_h264_stream_proc(void* arg)
{
    sample_venc_getstream_para *param = (sample_venc_getstream_para*)arg;
    xmedia_s32 chn_cnt = param->cnt;
    xmedia_s32 i;
    xmedia_s32 ret = XMEDIA_FAILURE;
    xmedia_venc_chn_attr venc_chn_attr;
    xmedia_payload_type payload_type[VENC_MAX_CHN_NUM];

    struct timeval time_out;
    void *venc_pack = NULL;
    xmedia_venc_stream venc_stream;
    xmedia_venc_chn_status stat;
    xmedia_u32 venc_mask = 0;

    uvc_cache_t *uvc_cache = uvc_cache_get();
    frame_node_t *fnode = NULL;

    if (chn_cnt >= VENC_MAX_CHN_NUM) {
        venc_loge("input venc chn count invaild\n");
        return XMEDIA_NULL;
    }

    for (i = 0; i < chn_cnt; ++i) {
        if (param->venc_chn[i] < 0) {
            continue;
        }

        ret = xmedia_venc_get_chn_attr(param->venc_chn[i], &venc_chn_attr);
        if (XMEDIA_SUCCESS != ret) {
            venc_loge("xmedia_venc_get_chn_attr chn[%d] failed with %#x!\n", param->venc_chn[i], ret);
            return XMEDIA_NULL;
        }

        payload_type[i] = venc_chn_attr.venc_attr.en_type;
        venc_mask |= 1 << param->venc_chn[i];
    }

    venc_pack = malloc(sizeof(xmedia_venc_pack) * 5);
    if (venc_pack == NULL) {
        venc_loge("Failed to malloc venc pack\n");
        return NULL;
    }

    g_h264 = (char*)malloc(H264_BUFF_LEN);
    if (g_h264 == NULL) {
        venc_loge("Failed to malloc\n");
        free(venc_pack);
        return NULL;
    }

    venc_logi("mjpeg and h264 start.\n");
    while (param->thread_start) {
        time_out.tv_sec  = 2;
        time_out.tv_usec = 0;
        ret = xmedia_venc_select(venc_mask, &time_out);
        if (ret == XMEDIA_ERRCODE_INVALID_PARAM || ret == XMEDIA_FAILURE) {
            venc_loge("select err\n");
            break;
        } else if (ret == XMEDIA_ERRCODE_TIMEOUT) {
            param->stream_timeout_cnt++;
            venc_logd("get venc stream time out, continue. \n");
            continue;
        }

        if (uvc_cache) {
            get_node_from_queue(uvc_cache->free_queue, &fnode);
        }

        if (!fnode) {
            //venc_logw("drop frame.\n");
        } else {
            fnode->used = 0;
        }

        for (i = 0; i < chn_cnt; ++i) {
            if (param->venc_chn[i] < 0) {
                continue;
            }

            ret = xmedia_venc_query_status(param->venc_chn[i], &stat);
            if (XMEDIA_SUCCESS != ret) {
                venc_logd("xmedia_venc_query_status chn[%d] failed with %#x!\n", i, ret);
                break;
            }

            if (0 == stat.cur_packs) {
                break;
            }

            memset(&venc_stream, 0, sizeof(venc_stream));
            venc_stream.pack = (xmedia_venc_pack*)venc_pack;
            if (XMEDIA_NULL == venc_stream.pack) {
                venc_logd("malloc stream pack failed!\n");
                break;
            }

            venc_stream.pack_count = stat.cur_packs;
            ret = xmedia_venc_get_stream(param->venc_chn[i], &venc_stream, XMEDIA_TRUE);
            if (XMEDIA_SUCCESS != ret) {
                venc_stream.pack = XMEDIA_NULL;
                xmedia_venc_release_stream(param->venc_chn[i], &venc_stream);
                venc_loge("xmedia_venc_get_stream failed with %#x!\n", ret);
                break;
            }

            if (fnode != NULL) {
                sample_mjpeg_h264_save_stream(payload_type[i], &venc_stream, fnode);
            }

            ret = xmedia_venc_release_stream(param->venc_chn[i], &venc_stream);
            if (XMEDIA_SUCCESS != ret) {
                venc_loge("xmedia_venc_release_stream failed!\n");
                venc_stream.pack = XMEDIA_NULL;
                break;
            }

            venc_stream.pack = XMEDIA_NULL;
        }

        if (fnode != NULL) {
            venc_logd("send size(%u)\n", fnode->used);
            if (fnode->used) {
                put_node_to_queue(uvc_cache->ok_queue, fnode);
            } else {
                put_node_to_queue(uvc_cache->free_queue, fnode);
            }
        }
    }

    free(venc_pack);
    free(g_h264);
    venc_logi("mjpeg and stream exit.\n");

    return NULL;
}

static int sample_mjpeg_h264_start_get_stream(void)
{
    int i, rc, cnt;

    cnt = 2;
    g_st_param.cnt = cnt;
    g_st_param.thread_start = XMEDIA_TRUE;

    for (i = 0; i < cnt; i++) {
        g_st_param.venc_chn[i] = i; // g_venc_chn_0, g_venc_chn_1
    }

    rc = pthread_create(&g_recv_pid, NULL, sample_mjpeg_h264_stream_proc, (void*)&g_st_param);
    if (rc < 0) {
        venc_loge("Create sample_mjpeg_h264_stream_proc failed.\n");
        return -1;
    }

    return 0;
}

static int sample_mjpeg_h264_stop_get_stream(void)
{
    if (XMEDIA_TRUE == g_st_param.thread_start) {
        g_st_param.thread_start = XMEDIA_FALSE;
        pthread_join(g_recv_pid, 0);
    }

    venc_logi("stop get mjpeg stream ok.\n");

    return 0;
}

static int sample_mjpeg_h264_normalp_classic(void)
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
    vpss_ochn_size.width  = pic_size.width;
    vpss_ochn_size.height = pic_size.height;

    g_vpss_config.pipe_info[g_vpss_pipe].pipe_en = XMEDIA_TRUE;
    g_vpss_config.pipe_info[g_vpss_pipe].pipe_no = g_vpss_pipe;

    // vpss out channel 0
    g_vpss_config.pipe_info[g_vpss_pipe].chn_info[g_vpss_ochn_0].chn_en = XMEDIA_TRUE;
    g_vpss_config.pipe_info[g_vpss_pipe].chn_info[g_vpss_ochn_0].chn_no = g_vpss_ochn_0;
    ret = sample_comm_vpss_get_default_ochn_cfg(&g_vpss_config.pipe_info[g_vpss_pipe].chn_info[g_vpss_ochn_0].chn_config,
            vpss_ochn_size, &g_video_param);
    if (ret != XMEDIA_SUCCESS) {
        venc_loge("get default pipe cfg failed !\n");
        goto exit2;
    }

    // vpss out channel 1
    g_vpss_config.pipe_info[g_vpss_pipe].chn_info[g_vpss_ochn_1].chn_en = XMEDIA_TRUE;
    g_vpss_config.pipe_info[g_vpss_pipe].chn_info[g_vpss_ochn_1].chn_no = g_vpss_ochn_1;
    ret = sample_comm_vpss_get_default_ochn_cfg(&g_vpss_config.pipe_info[g_vpss_pipe].chn_info[g_vpss_ochn_1].chn_config,
            vpss_ochn_size, &g_video_param);
    if (ret != XMEDIA_SUCCESS) {
        venc_loge("get default pipe cfg failed !\n");
        goto exit2;
    }

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

    //venc channel 0 init
    g_venc_config.chn_info[g_venc_chn_0].venc_en = XMEDIA_TRUE;
    g_venc_config.chn_info[g_venc_chn_0].venc_chn = g_venc_chn_0;
    g_venc_config.chn_info[g_venc_chn_0].payload_type = PT_MJPEG;
    //g_venc_config.chn_info[g_venc_chn_0].rc_mode = VENC_RC_MODE_MJPEGFIXQP;
    g_venc_config.chn_info[g_venc_chn_0].rc_mode = VENC_RC_MODE_MJPEGCBR;
    sample_comm_venc_get_default_chn_info(vpss_ochn_size, framerate, &g_venc_config.chn_info[g_venc_chn_0]);

    //venc channel 1 init
    g_venc_config.chn_info[g_venc_chn_1].venc_en = XMEDIA_TRUE;
    g_venc_config.chn_info[g_venc_chn_1].venc_chn = g_venc_chn_1;
    g_venc_config.chn_info[g_venc_chn_1].payload_type = PT_H264;
    g_venc_config.chn_info[g_venc_chn_1].rc_mode = VENC_RC_MODE_H264CBR;
    sample_comm_venc_get_default_chn_info(vpss_ochn_size, framerate, &g_venc_config.chn_info[g_venc_chn_1]);

    ret = sample_comm_venc_start(&g_venc_config);
    if (ret != XMEDIA_SUCCESS) {
        venc_loge("venc start failed !\n");
        goto exit4;
    }

    ret = sample_comm_sys_vpss_bind_venc(g_vpss_pipe, g_vpss_ochn_0, g_venc_chn_0);
    if (ret != XMEDIA_SUCCESS) {
        venc_loge("vpss bind venc failed !\n");
        goto exit5;
    }

    ret = sample_comm_sys_vpss_bind_venc(g_vpss_pipe, g_vpss_ochn_1, g_venc_chn_1);
    if (ret != XMEDIA_SUCCESS) {
        venc_loge("vpss bind venc failed !\n");
        goto exit5;
    }

    return 0;

    sample_comm_sys_vpss_unbind_venc(g_vpss_pipe, g_vpss_ochn_0, g_venc_chn_0);
    sample_comm_sys_vpss_unbind_venc(g_vpss_pipe, g_vpss_ochn_1, g_venc_chn_1);
exit5:
    sample_comm_venc_stop(&g_venc_config);
exit4:
    sample_comm_sys_vi_unbind_vpss(g_vi_pipe, g_vi_chn, g_vpss_pipe, g_vpss_ichn);
exit3:
    sample_comm_vpss_stop(&g_vpss_config);
exit2:
    sample_comm_isp_stop(&g_isp_param);
exit1:
    sample_comm_vi_stop(&g_vi_config);
exit0:
    sample_comm_isp_exit(&g_isp_param);

    sample_mjpeg_h264_sys_deinit();

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
int sample_mjpeg_h264_set_idr(void)
{
    return 0;
}

int sample_mjpeg_h264_init(void)
{
    g_sensor_type = SENSOR0_TYPE;
    sample_mjpeg_h264_sys_init(g_sensor_type);

    return 0;
}

int sample_mjpeg_h264_startup(void)
{
    venc_logi("Mjpeg stream startup.\n");
    g_start = 1;

    if (sample_mjpeg_h264_normalp_classic() < 0) {
        return -1;
    }

    sample_mjpeg_h264_start_get_stream();
    venc_logi("Mjpeg stream startup ok.\n");

    return 0;
}

int sample_mjpeg_h264_shutdown(void)
{
    if (!g_start) {
        return 0;
    }

    sample_mjpeg_h264_stop_get_stream();

    sample_comm_sys_vpss_unbind_venc(g_vpss_pipe, g_vpss_ochn_0, g_venc_chn_0);

    sample_comm_sys_vpss_unbind_venc(g_vpss_pipe, g_vpss_ochn_1, g_venc_chn_1);

    sample_comm_venc_stop(&g_venc_config);

    sample_comm_sys_vi_unbind_vpss(g_vi_pipe, g_vi_chn, g_vpss_pipe, g_vpss_ichn);

    sample_comm_vpss_stop(&g_vpss_config);

    sample_comm_isp_stop(&g_isp_param);

    sample_comm_vi_stop(&g_vi_config);

    sample_comm_isp_exit(&g_isp_param);

    sample_mjpeg_h264_sys_deinit();

    g_start = 0;

    return 0;
}
