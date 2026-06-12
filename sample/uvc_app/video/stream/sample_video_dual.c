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
#include "sample_video_dual.h"

#include "venc_log.h"
#include "frame_cache.h"

static int g_start = 0;
static pthread_t g_recv_pid;
static sample_venc_getstream_para g_st_param;

//common
struct dual_comm {
    sample_comm_sensor_type sensor_type;
    sample_comm_video_param video_param;
};

// vi config
struct dual_vi {
    xmedia_s32 vi_dev; /* 0:单板SENSOR0; 2:单板SENSOR2 */
    xmedia_s32 vi_chn;
    xmedia_s32 vi_pipe;
    sample_vi_config vi_cfg;
};

// isp config
struct dual_isp {
    sample_isp_param isp_param;
};

// vpss config
struct dual_vpss {
    xmedia_s32 vpss_pipe;
    xmedia_s32 vpss_ichn;
    xmedia_s32 vpss_ochn;
    sample_vpss_config vpss_cfg;
};

// venc config
struct dual_venc {
    xmedia_s32 venc_chn;
    sample_venc_config venc_cfg;
};

struct dual_stream {
    xmedia_s32 stream_id;

    struct dual_comm comm;
    struct dual_vi vi;
    struct dual_isp isp;
    struct dual_vpss vpss;
    struct dual_venc venc;
};

#define STREAM_NUMS 4

static struct dual_stream g_stream[STREAM_NUMS];

static void sample_dual_config_comm(struct dual_comm* comm, sample_comm_sensor_type sns_type)
{
    comm->sensor_type = sns_type;
    comm->video_param.video_fmt  = XMEDIA_VIDEO_FMT_LINEAR;
    comm->video_param.pixel_fmt  = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    comm->video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    comm->video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;
}

static void sample_dual_config_vi(struct dual_vi* vi,
        xmedia_s32 vi_chn,
        xmedia_s32 vi_dev,
        xmedia_s32 vi_pipe,
        sample_comm_sensor_type sns_type)
{
    vi->vi_chn  = vi_chn;
    vi->vi_dev  = vi_dev;
    vi->vi_pipe = vi_pipe;
    vi->vi_cfg.dev_info[vi_dev].dev_no = vi_dev;
    vi->vi_cfg.dev_info[vi_dev].dev_en = XMEDIA_TRUE;
    vi->vi_cfg.dev_info[vi_dev].sensor_type = sns_type;
    vi->vi_cfg.pipe_info[vi_pipe].pipe_no = vi_pipe;
    vi->vi_cfg.pipe_info[vi_pipe].pipe_en = XMEDIA_TRUE;
    vi->vi_cfg.pipe_info[vi_pipe].chn_info[vi_chn].chn_no = vi_chn;
    vi->vi_cfg.pipe_info[vi_pipe].chn_info[vi_chn].chn_en = XMEDIA_TRUE;
    vi->vi_cfg.dev_bind_pipe[vi_dev].pipe[0] = vi_pipe;
    vi->vi_cfg.dev_bind_pipe[vi_dev].pipe[1] = -1;
}

static void sample_dual_config_isp(struct dual_isp* isp,
        xmedia_s32 vi_pipe,
        sample_comm_sensor_type sns_type)
{
    xmedia_u32 framerate = 0;
    vi_sensor_info sensor_info = {0};

    sample_comm_vi_get_sensor_info(sns_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sns_type, &framerate);

    // isp config init
    isp->isp_param.isp_info[vi_pipe].isp_config.fps = framerate;
    isp->isp_param.isp_info[vi_pipe].isp_config.mode_config.work_mode = XMEDIA_ISP_WORK_MODE_MASTER;
    isp->isp_param.isp_info[vi_pipe].isp_config.mode_config.master_mode.blend_stat_enable = XMEDIA_FALSE;
    isp->isp_param.isp_info[vi_pipe].isp_config.mode_config.master_mode.slave_num         = 0;
    isp->isp_param.isp_info[vi_pipe].isp_config.pixel_fmt   = sensor_info.pixel_format;
    isp->isp_param.isp_info[vi_pipe].isp_config.size.height = sensor_info.height;
    isp->isp_param.isp_info[vi_pipe].isp_config.size.width  = sensor_info.width;
    isp->isp_param.isp_info[vi_pipe].isp_config.wdr_mode    = sensor_info.wdr_mode;

    // isp pipe init
    isp->isp_param.pipe[vi_pipe] = vi_pipe;
    isp->isp_param.isp_info[vi_pipe].flip   = XMEDIA_FALSE;
    isp->isp_param.isp_info[vi_pipe].mirror = XMEDIA_FALSE;
    isp->isp_param.isp_info[vi_pipe].isp_pipe_en   = XMEDIA_TRUE;
    isp->isp_param.isp_info[vi_pipe].isp_sensor_en = XMEDIA_TRUE;
    isp->isp_param.isp_info[vi_pipe].sensor_type   = sns_type;
}

static void sample_dual_config_vpss(struct dual_vpss* vpss,
        xmedia_s32 vpss_pipe,
        xmedia_s32 vpss_ichn,
        xmedia_s32 vpss_ochn)
{
    vpss->vpss_pipe = vpss_pipe;
    vpss->vpss_ichn = vpss_ichn;
    vpss->vpss_ochn = vpss_ochn;
    //vpss->vpss_cfg = ;
}

static void sample_dual_config_venc(struct dual_venc* venc,
        xmedia_s32 venc_chnl)
{
    venc->venc_chn = venc_chnl;
    venc->venc_cfg.chn_info[venc_chnl].venc_en      = XMEDIA_TRUE;
    venc->venc_cfg.chn_info[venc_chnl].venc_chn     = venc_chnl;
    venc->venc_cfg.chn_info[venc_chnl].payload_type = PT_H264;
    venc->venc_cfg.chn_info[venc_chnl].rc_mode      = VENC_RC_MODE_H264CBR;
}

static void sammple_dual_stream_config(void)
{
    xmedia_s32 id;
    xmedia_s32 vi_dev[STREAM_NUMS]    = {0, 0, 2, 2};
    xmedia_s32 vi_pipe[STREAM_NUMS]   = {0, 0, 1, 1};
    xmedia_s32 vi_chn[STREAM_NUMS]    = {0, 0, 0, 0};
    xmedia_s32 vpss_pipe[STREAM_NUMS] = {0, 0, 1, 1};
    xmedia_s32 vpss_ichn[STREAM_NUMS] = {0, 0, 0, 0};
    xmedia_s32 vpss_ochn[STREAM_NUMS] = {0, 1, 0, 1};
    xmedia_s32 venc_chnl[STREAM_NUMS] = {0, 1, 2, 3};
    sample_comm_sensor_type sns_type[STREAM_NUMS] = {SENSOR0_TYPE, SENSOR0_TYPE, SENSOR1_TYPE, SENSOR1_TYPE};

    for (id = 0; id < STREAM_NUMS; id++) {
        g_stream[id].stream_id = id;

        sample_dual_config_comm(&g_stream[id].comm, sns_type[id]);
        sample_dual_config_vi(&g_stream[id].vi, vi_chn[id], vi_dev[id], vi_pipe[id], sns_type[id]);
        sample_dual_config_isp(&g_stream[id].isp, vi_pipe[id], sns_type[id]);
        sample_dual_config_vpss(&g_stream[id].vpss, vpss_pipe[id], vpss_ichn[id], vpss_ochn[id]);
        sample_dual_config_venc(&g_stream[id].venc, venc_chnl[id]);
    }
}

static int sample_dual_sys_init(void)
{
    xmedia_s32 ret;
    xmedia_s32 id;
    xmedia_s32 blk_size, pool_id;
    xmedia_s32 vi_pipe;
    sample_comm_sensor_type sensor_type;
    vi_sensor_info sensor_info = {0};
    xmedia_video_size pic_size = {0};
    sample_sys_config sys_config = {0};
    sample_comm_video_param* video_param = NULL;

    func_entry();

    sammple_dual_stream_config();

    pool_id = 0;

    for (id = 0; id < STREAM_NUMS; id++) {
        sensor_type = g_stream[id].comm.sensor_type;

        sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);

        // sys init
        vi_pipe = g_stream[id].vi.vi_pipe;
        sys_config.sys_conf.pipe_mode[vi_pipe].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
        sys_config.sys_conf.pipe_mode[vi_pipe].viproc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
        sys_config.sys_conf.pipe_mode[vi_pipe].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;

        pic_size.width = sensor_info.width;
        pic_size.height = sensor_info.height;

        venc_logd("resolution: w(%d) x h(%d).\n", pic_size.width, pic_size.height);

        video_param = &g_stream[id].comm.video_param;
        blk_size = sample_comm_sys_get_buffer_size(pic_size,
                video_param->video_fmt,
                sensor_info.pixel_format,
                sensor_info.bit_width,
                video_param->compress_mode);
        sys_config.vb_conf.common_pool[pool_id].block_size = blk_size;
        sys_config.vb_conf.common_pool[pool_id].block_cnt = 5;
        venc_logd("allocate cache pool %d: size(%d) x cnt(%d)\n", pool_id, blk_size, 5);
        pool_id++;

        blk_size = sample_comm_sys_get_buffer_size(pic_size,
                video_param->video_fmt,
                video_param->pixel_fmt,
                video_param->data_width,
                video_param->compress_mode);
        sys_config.vb_conf.common_pool[pool_id].block_size = blk_size;
        sys_config.vb_conf.common_pool[pool_id].block_cnt = 5;
        venc_logd("allocate cache pool %d: size(%d) x cnt(%d)\n", pool_id, blk_size, 5);
        pool_id++;
    }

    sys_config.vb_conf.supplement_config  = (1 << 0);
    sys_config.vb_conf.supplement_config |= (1 << 1);

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

static int sample_dual_sys_deinit(void)
{
    sample_comm_venc_exit();

    sample_comm_vpss_exit();

    sample_comm_vi_exit();

    sample_comm_sys_exit();

    return XMEDIA_SUCCESS;
}

static FILE* fp[4] = {NULL, NULL, NULL, NULL};
char filename[4][32] = {
    {"stream_dev.h264"},
    {"stream_1.h264"},
    {"stream_2.h264"},
    {"stream_3.h264"}
};

struct dual_head {
    unsigned int id;
    unsigned int stream_offset;
    unsigned int stream_size;
    unsigned char reserve[4];
};

void save_stream(char* stream, int length)
{
    int w_actl = 0;
    int recv_size = length;
    unsigned int offset, size;
    unsigned int id = 0;
    unsigned char* tmp = (unsigned char*)stream;
    struct dual_head* stream_head = NULL;

    while (recv_size > 0) {
        stream_head = (struct dual_head*)stream;
        id = stream_head->id;
        offset = stream_head->stream_offset;
        size   = stream_head->stream_size;

        if (id >= STREAM_NUMS) {
            printf("Error: Invalid stream id.\n");
            return ;
        }

        //printf("id(%u), off(%u), size(%u)\n", id, offset, size);
        tmp += offset;
        if (fp[id] != NULL) {
            w_actl = fwrite(tmp, 1, size, fp[id]);
        }

        if (w_actl != size) {
            //printf("Warning: exp(%d) != acl(%d).", size, w_actl);
        }

        tmp += size;

        recv_size -= (offset + size);
    }
}

static int sample_dual_save_data(xmedia_venc_stream* pst_stream, int id)
{
    int i = 0;
    xmedia_venc_pack *pst_data = NULL;
    unsigned char *s = NULL;
    unsigned int data_len = 0;
    unsigned int copy_size = 0;
    struct dual_head head = {0};

    uvc_cache_t *uvc_cache = uvc_cache_get();
    frame_node_t *fnode = NULL;

    if (uvc_cache) {
        get_node_from_queue(uvc_cache->free_queue, &fnode);
    }

    if (!fnode) {
        venc_logd("drop frame.\n");
        return XMEDIA_SUCCESS;
    }

    fnode->used = sizeof(struct dual_head);

    head.id = id;
    head.stream_offset = sizeof(struct dual_head);
    head.stream_size = 0;

    for (i = 0; i < pst_stream->pack_count; ++i) {
        pst_data = &pst_stream->pack[i];
        s = pst_data->vir_addr + pst_data->offset;
        data_len = pst_data->len - pst_data->offset;
        copy_size = data_len < (fnode->length - fnode->used) ? data_len : (fnode->length - fnode->used);

        if (copy_size > 0) {
            memcpy(fnode->mem + fnode->used, s, copy_size);
            fnode->used += copy_size;
            head.stream_size += copy_size;
        }

        if (data_len > copy_size) {
            venc_logw("fnode->length = %u\n", fnode->length);
            venc_logw("WARN: missing pack. exp(%u) != act(%u)\n", data_len, copy_size);
        }
    }

    memcpy(fnode->mem, (void*)&head, sizeof(struct dual_head));

    //fwrite((char*)(fnode->mem), 1, 16, fp[0]);
    if (fp[id] != NULL) {
        //fwrite((char*)(fnode->mem + head.stream_offset), 1,  head.stream_size, fp[id]);
        //save_stream((char*)fnode->mem, fnode->used);
    }

    venc_logd("send size(%u)\n", fnode->used);
    put_node_to_queue(uvc_cache->ok_queue, fnode);

    return XMEDIA_SUCCESS;
}

static int sample_dual_save_stream(xmedia_payload_type en_type, xmedia_venc_stream *pst_stream, int id)
{
    xmedia_s32 ret = XMEDIA_FAILURE;

    if (PT_H264 == en_type) {
        ret = sample_dual_save_data(pst_stream, id);
    }

    return ret;
}

static void* sample_dual_stream_proc(void* arg)
{
    xmedia_s32 i;
    xmedia_s32 ret = XMEDIA_FAILURE;

    xmedia_venc_chn_attr venc_chn_attr;
    xmedia_payload_type payload_type[VENC_MAX_CHN_NUM];
    sample_venc_getstream_para *param = (sample_venc_getstream_para*)arg;
    xmedia_s32 chn_cnt = param->cnt;

    struct timeval time_out;
    xmedia_venc_stream venc_stream;
    xmedia_venc_chn_status stat;
    xmedia_u32 venc_mask = 0;

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
        if (fp[i] == NULL) {
            fp[i] = fopen((char*)filename[i], "w+");
        }
    }

    venc_logi("Dual start.\n");
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
                continue;
            }

            memset(&venc_stream, 0, sizeof(venc_stream));
            venc_stream.pack = (xmedia_venc_pack*)malloc(sizeof(xmedia_venc_pack) * stat.cur_packs);
            if (XMEDIA_NULL == venc_stream.pack) {
                venc_logd("malloc stream pack failed!\n");
                break;
            }

            venc_stream.pack_count = stat.cur_packs;
            ret = xmedia_venc_get_stream(param->venc_chn[i], &venc_stream, XMEDIA_TRUE);
            if (XMEDIA_SUCCESS != ret) {
                free(venc_stream.pack);
                venc_stream.pack = XMEDIA_NULL;
                xmedia_venc_release_stream(param->venc_chn[i], &venc_stream);
                venc_loge("xmedia_venc_get_stream failed with %#x!\n", ret);
                break;
            }

            sample_dual_save_stream(payload_type[i], &venc_stream, i);

            ret = xmedia_venc_release_stream(param->venc_chn[i], &venc_stream);
            if (XMEDIA_SUCCESS != ret) {
                venc_loge("xmedia_venc_release_stream failed!\n");
                free(venc_stream.pack);
                venc_stream.pack = XMEDIA_NULL;
                break;
            }

            free(venc_stream.pack);
            venc_stream.pack = XMEDIA_NULL;
        }
    }

    for (i = 0; i < 4; i++) {
        if (fp[i] != NULL) {
            fclose(fp[i]);
        }

        fp[i] = NULL;
    }

    venc_logi("Dual stream exit.\n");

    return NULL;
}

static int sample_dual_start_get_stream()
{
    int i, rc;

    g_st_param.cnt = STREAM_NUMS;
    g_st_param.thread_start = XMEDIA_TRUE;

    for (i = 0; i < STREAM_NUMS; i++) {
        g_st_param.venc_chn[i] = g_stream[i].venc.venc_chn;
    }

    rc = pthread_create(&g_recv_pid, NULL, sample_dual_stream_proc, (void*)&g_st_param);
    if (rc < 0) {
        venc_loge("Create sample_dual_stream_proc failed.\n");
        return -1;
    }

    return 0;
}

static int sample_dual_stop_get_stream(void)
{
    if (XMEDIA_TRUE == g_st_param.thread_start) {
        g_st_param.thread_start = XMEDIA_FALSE;
        pthread_join(g_recv_pid, 0);
    }

    venc_logi("stop get Dual stream ok.\n");

    return 0;
}

static int sample_dual_normalp_classic(void)
{
    xmedia_s32 id;
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_u32 framerate = 0;
    xmedia_video_size pic_size = {0};
    xmedia_video_size vpss_ochn_size = {0};
    //xmedia_isp_config isp_config = {0};
    xmedia_payload_type en_payload = PT_H264;
    vi_sensor_info sensor_info = {0};

    xmedia_s32 venc_chn_0, venc_chn_1;
    xmedia_s32 vi_pipe, vi_chn;
    xmedia_s32 vpss_pipe, vpss_ichn, vpss_ochn_0, vpss_ochn_1;
    sample_comm_sensor_type sensor_type;
    sample_isp_param* isp_param = NULL;
    sample_vi_config* vi_config = NULL;
    sample_vpss_config* vpss_config = NULL;
    sample_venc_config* venc_config = NULL;
    sample_comm_video_param* video_param = NULL;

    for (id = 0; id < STREAM_NUMS; id++) {
#if 0
        g_vi_config.dev_info[g_vi_dev].dev_en = XMEDIA_TRUE;
        g_vi_config.dev_info[g_vi_dev].dev_no = g_vi_dev;
        g_vi_config.dev_info[g_vi_dev].sensor_type = g_sensor_type;
        g_vi_config.pipe_info[g_vi_pipe].pipe_en = XMEDIA_TRUE;
        g_vi_config.pipe_info[g_vi_pipe].pipe_no = g_vi_pipe;
        g_vi_config.pipe_info[g_vi_pipe].chn_info[g_vi_chn].chn_en = XMEDIA_TRUE;
        g_vi_config.pipe_info[g_vi_pipe].chn_info[g_vi_chn].chn_no = g_vi_chn;
        g_vi_config.dev_bind_pipe[g_vi_dev].pipe[0] = g_vi_pipe;
        g_vi_config.dev_bind_pipe[g_vi_dev].pipe[1] = -1;
#endif
        sensor_type = g_stream[id].comm.sensor_type;

        sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
        sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);

        pic_size.height = sensor_info.height;
        pic_size.width = sensor_info.width;

#if 0
        isp_config.fps = framerate;
        isp_config.mode_config.work_mode = XMEDIA_ISP_WORK_MODE_MASTER;
        isp_config.mode_config.master_mode.blend_stat_enable = XMEDIA_FALSE;
        isp_config.mode_config.master_mode.slave_num = 0;
        isp_config.pixel_fmt = sensor_info.pixel_format;
        isp_config.size.height = sensor_info.height;
        isp_config.size.width = sensor_info.width;
        isp_config.wdr_mode = sensor_info.wdr_mode;

        // isp pipe init
        g_isp_param.pipe[g_vi_pipe] = g_vi_pipe;
        g_isp_param.isp_info[g_vi_pipe].sensor_type = g_sensor_type;
        g_isp_param.isp_info[g_vi_pipe].flip = XMEDIA_FALSE;
        g_isp_param.isp_info[g_vi_pipe].mirror = XMEDIA_FALSE;
        g_isp_param.isp_info[g_vi_pipe].isp_pipe_en = XMEDIA_TRUE;
        g_isp_param.isp_info[g_vi_pipe].isp_sensor_en = XMEDIA_TRUE;

        memcpy(&(g_isp_param.isp_info[g_vi_pipe].isp_config), &isp_config, sizeof(xmedia_isp_config));
#endif

        // isp init
        isp_param = &g_stream[id].isp.isp_param;
        vi_config = &g_stream[id].vi.vi_cfg;

        ret = sample_comm_isp_init(isp_param, vi_config);
        if (ret != XMEDIA_SUCCESS) {
            venc_loge("stream[%d] isp init failed!\n", id);
        }

        // vi start
        vi_config = &g_stream[id].vi.vi_cfg;
        video_param = &g_stream[id].comm.video_param;

        ret = sample_comm_vi_start(vi_config, video_param);
        if (ret != XMEDIA_SUCCESS) {
            venc_loge("stream[%d] start vi failed!\n", id);
        }

        // isp start
        isp_param = &g_stream[id].isp.isp_param;

        ret = sample_comm_isp_start(isp_param);
        if (ret != XMEDIA_SUCCESS) {
            venc_loge("stream[%d] start isp failed!\n", id);
        }

        // vpss config
        vpss_pipe   = g_stream[id].vpss.vpss_pipe;
        vpss_ichn   = g_stream[id].vpss.vpss_ichn;
        vpss_ochn_0 = g_stream[id].vpss.vpss_ochn;
        vpss_ochn_1 = g_stream[id + 1].vpss.vpss_ochn;
        vpss_config = &g_stream[id].vpss.vpss_cfg;
        video_param = &g_stream[id].comm.video_param;

        ret = sample_comm_vpss_get_default_pipe_cfg(&vpss_config->pipe_info[vpss_pipe].pipe_config,
                pic_size,
                video_param);
        if (ret != XMEDIA_SUCCESS) {
            venc_loge("stream[%d] get default pipe cfg failed !\n", id);
        }

        get_user_format(&en_payload, &pic_size, NULL);
        vpss_ochn_size.width  = pic_size.width;
        vpss_ochn_size.height = pic_size.height;

        vpss_config->pipe_info[vpss_pipe].pipe_en = XMEDIA_TRUE;
        vpss_config->pipe_info[vpss_pipe].pipe_no = vpss_pipe;
#if 0
        vpss_config->pipe_info[vpss_pipe].chn_info[vpss_ochn].chn_no = vpss_ochn;
        vpss_config->pipe_info[vpss_pipe].chn_info[vpss_ochn].chn_en = XMEDIA_TRUE;
#else
        vpss_config->pipe_info[vpss_pipe].chn_info[vpss_ochn_0].chn_no = vpss_ochn_0;
        vpss_config->pipe_info[vpss_pipe].chn_info[vpss_ochn_0].chn_en = XMEDIA_TRUE;
        vpss_config->pipe_info[vpss_pipe].chn_info[vpss_ochn_1].chn_no = vpss_ochn_1;
        vpss_config->pipe_info[vpss_pipe].chn_info[vpss_ochn_1].chn_en = XMEDIA_TRUE;
#endif
#if 0
        ret = sample_comm_vpss_get_default_ochn_cfg(
                &vpss_config->pipe_info[vpss_pipe].chn_info[vpss_ochn].chn_config,
                vpss_ochn_size,
                video_param);
        if (ret != XMEDIA_SUCCESS) {
            venc_loge("stream[%d] get default pipe cfg failed !\n", id);
        }
#else
        ret = sample_comm_vpss_get_default_ochn_cfg(
                &vpss_config->pipe_info[vpss_pipe].chn_info[vpss_ochn_0].chn_config,
                vpss_ochn_size,
                video_param);
        if (ret != XMEDIA_SUCCESS) {
            venc_loge("stream[%d] get default pipe cfg failed !\n", id);
        }

        ret = sample_comm_vpss_get_default_ochn_cfg(
                &vpss_config->pipe_info[vpss_pipe].chn_info[vpss_ochn_1].chn_config,
                vpss_ochn_size,
                video_param);
        if (ret != XMEDIA_SUCCESS) {
            venc_loge("stream[%d] get default pipe cfg failed !\n", id + 1);
        }
#endif
        // vpss start
        ret = sample_comm_vpss_start(vpss_config);
        if (ret != XMEDIA_SUCCESS) {
            venc_loge("stream[%d] vpss start failed !\n", id);
        }

        vi_chn  = g_stream[id].vi.vi_chn;
        vi_pipe = g_stream[id].vi.vi_pipe;
        vpss_ichn = g_stream[id].vpss.vpss_ichn;
        vpss_pipe = g_stream[id].vpss.vpss_pipe;
        ret = sample_comm_sys_vi_bind_vpss(vi_pipe, vi_chn, vpss_pipe, vpss_ichn);
        if (ret != XMEDIA_SUCCESS) {
            venc_loge("stream[%d] vi bind vpss failed !\n", id);
        }

        //venc init
        venc_chn_0  = g_stream[id].venc.venc_chn;
        venc_config = &g_stream[id].venc.venc_cfg;

        venc_config->chn_info[venc_chn_0].venc_chn = venc_chn_0;
        venc_config->chn_info[venc_chn_0].venc_en  = XMEDIA_TRUE;
        venc_config->chn_info[venc_chn_0].payload_type = en_payload;
        venc_config->chn_info[venc_chn_0].rc_mode = VENC_RC_MODE_H264CBR;

        sample_comm_venc_get_default_chn_info(vpss_ochn_size,
                framerate,
                &venc_config->chn_info[venc_chn_0]);

        ret = sample_comm_venc_start(venc_config);
        if (ret != XMEDIA_SUCCESS) {
            venc_loge("stream[%d] venc start failed !\n", id);
        }

        venc_chn_1  = g_stream[id + 1].venc.venc_chn;
        venc_config = &g_stream[id + 1].venc.venc_cfg;

        venc_config->chn_info[venc_chn_1].venc_chn = venc_chn_1;
        venc_config->chn_info[venc_chn_1].venc_en  = XMEDIA_TRUE;
        venc_config->chn_info[venc_chn_1].payload_type = en_payload;
        venc_config->chn_info[venc_chn_1].rc_mode = VENC_RC_MODE_H264CBR;

        sample_comm_venc_get_default_chn_info(vpss_ochn_size,
                framerate,
                &venc_config->chn_info[venc_chn_1]);

        ret = sample_comm_venc_start(venc_config);
        if (ret != XMEDIA_SUCCESS) {
            venc_loge("stream[%d] venc start failed !\n", id);
        }

        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn_0, venc_chn_0);
        if (ret != XMEDIA_SUCCESS) {
            venc_loge("stream[%d] vpss bind venc failed !\n", id);
        }

        ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn_1, venc_chn_1);
        if (ret != XMEDIA_SUCCESS) {
            venc_loge("stream[%d] vpss bind venc failed !\n", id);
        }

        id++;
    }

    return 0;
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
int sample_dual_set_idr(void)
{
    return 0;
}

int sample_dual_init(void)
{
    sample_dual_sys_init();

    return 0;
}

int sample_dual_startup(void)
{
    venc_logi("Dual stream startup.\n");
    g_start = 1;

    if (sample_dual_normalp_classic() < 0) {
        return -1;
    }

    sample_dual_start_get_stream();
    venc_logi("Dual stream startup ok.\n");

    return 0;
}

int sample_dual_shutdown(void)
{
    xmedia_s32 id;
    xmedia_s32 vi_pipe, vi_chn;
    xmedia_s32 vpss_pipe, vpss_ichn, vpss_ochn_0, vpss_ochn_1;
    xmedia_s32 venc_chn_0, venc_chn_1;
    sample_venc_config* venc_config = NULL;
    sample_vpss_config* vpss_config = NULL;
    sample_isp_param*   isp_param   = NULL;
    sample_vi_config*   vi_config   = NULL;

    if (!g_start) {
        return 0;
    }

    sample_dual_stop_get_stream();

    for (id = 0; id < STREAM_NUMS; id++) {
        venc_config = &g_stream[id].venc.venc_cfg;
        sample_comm_venc_stop(venc_config);

        venc_config = &g_stream[id + 1].venc.venc_cfg;
        sample_comm_venc_stop(venc_config);

        vpss_pipe = g_stream[id].vpss.vpss_pipe;
        vpss_ichn = g_stream[id].vpss.vpss_ichn;
        vpss_ochn_0 = g_stream[id].vpss.vpss_ochn;
        vpss_ochn_1 = g_stream[id + 1].vpss.vpss_ochn;
        venc_chn_0  = g_stream[id].venc.venc_chn;
        venc_chn_1  = g_stream[id + 1].venc.venc_chn;

        sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn_0, venc_chn_0);
        sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn_1, venc_chn_1);

        vi_pipe = g_stream[id].vi.vi_pipe;
        vi_chn  = g_stream[id].vi.vi_chn;
        sample_comm_sys_vi_unbind_vpss(vi_pipe, vi_chn, vpss_pipe, vpss_ichn);

        vpss_config = &g_stream[id].vpss.vpss_cfg;
        sample_comm_vpss_stop(vpss_config);

        isp_param = &g_stream[id].isp.isp_param;
        sample_comm_isp_stop(isp_param);

        vi_config = &g_stream[id].vi.vi_cfg;
        sample_comm_vi_stop(vi_config);

        sample_comm_isp_exit(isp_param);

        id++;
    }

    sample_dual_sys_deinit();

    g_start = 0;

    return 0;
}
