#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <errno.h>

#include "xmedia_npu.h"

#include "quickstart_comm.h"
#include "media_comm.h"


#define SVP_CHECK_FRAME_COUNT    1
#define LAYER_NUM_FOUR 4

static xmedia_s32 g_svp_handle = -1;
static pthread_t g_venc_thread_pid = 0;
static quickstart_comm_venc_get_stream_param g_stream_param = {0};

static xmedia_s32 quickstart_comm_venc_save_stream(FILE* pFd, xmedia_venc_stream* pstStream)
{
    xmedia_s32 i;

    for (i = 0; i < pstStream->pack_count; i++) {
        fwrite(pstStream->pack[i].vir_addr + pstStream->pack[i].offset,
               pstStream->pack[i].len - pstStream->pack[i].offset, 1, pFd);
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 quickstart_comm_svp_proc(xmedia_void)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_video_frame_info video_frame = {0};
    xmedia_s32 vpss_pipe = 0;
    xmedia_s32 vpss_chn = 2;
    xmedia_svp_task_input task_input = {0};
    xmedia_svp_aov_detect_output svp_result = {0};
    xmedia_s32 milli_sec = 20000;
    xmedia_s32 i = 0;

    for(i = 0; i < SVP_CHECK_FRAME_COUNT; i++) {
        s32Ret = xmedia_vpss_acquire_ochn_frame(vpss_pipe, vpss_chn, &video_frame, milli_sec);
        if (XMEDIA_SUCCESS != s32Ret) {
            MEDIA_ERR("Warning, acquire vpss ochn frame fail, continue, ret = 0x%x\n", s32Ret);
            continue;
        }

        task_input.frame_num = 1;
        task_input.frame = &video_frame;
        s32Ret = xmedia_svp_task_process(g_svp_handle, &task_input, &svp_result);
        if (s32Ret != XMEDIA_SUCCESS) {
            MEDIA_ERR("xmedia_svp_process failed with %#x!\n", s32Ret);
            return XMEDIA_FAILURE;
        }

        if (svp_result.detect_output.target_num > 0) {
            MEDIA_ERR("check person success \n");
        }

        s32Ret = xmedia_vpss_release_ochn_frame(vpss_pipe, vpss_chn, &video_frame);
        if (s32Ret != XMEDIA_SUCCESS) {
            MEDIA_ERR("XMEDIA_API_VPSS_ReleaseChnFrame failed with %#x!\n", s32Ret);
        }
    }

    return XMEDIA_SUCCESS;
}

static xmedia_void* quickstart_comm_svp_thread(xmedia_void* args)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_svp_modules model[1] = {0};
    xmedia_svp_task_cfg task_cfg = {0};
    xmedia_svp_aov_attr yolov5_attr = {0};
    xmedia_s32 npu_quickstart_flag = 1;

    s32Ret = xmedia_npu_set_quick_start_flag(npu_quickstart_flag);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("set npu cfg fail");
        return XMEDIA_NULL;
    }

    s32Ret = xmedia_tde_init();
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("tde init fail");
        return XMEDIA_NULL;
    }

    s32Ret = xmedia_svp_init();
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("svp init fail");
        return XMEDIA_NULL;
    }

    model[0].pathname = "/usr/bin/gnn_person_detect_640x360_rgb888hwc_v0102_20240909.bin";
    model[0].alg_type = XMEDIA_SVP_ALG_TYPE_PERSON;
    model[0].load_mode = XMEDIA_SVP_MODEL_FILE;
    model[0].format = XMEDIA_SVP_INPUTDATA_FORMAT_RGB888;

    task_cfg.module_num = 1;
    task_cfg.modules = model;
    task_cfg.task_type = XMEDIA_SVP_TASK_DETECT;

    s32Ret = xmedia_svp_task_create(&g_svp_handle, task_cfg);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("svp create fail.\n");
        xmedia_svp_uninit();
        return XMEDIA_NULL;
    }

    s32Ret = xmedia_svp_task_get_attr(g_svp_handle, &yolov5_attr);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("svp get task attr fail.\n");
        xmedia_svp_task_destroy(g_svp_handle);
        xmedia_svp_uninit();
        return XMEDIA_NULL;
    }

    yolov5_attr.detect_threshold = 0.5f;
    yolov5_attr.classifier_threshold = 0.01f;
    yolov5_attr.iou_threshold = 0.45f;
    yolov5_attr.max_target_num = 10;
    yolov5_attr.bytetrack_enable = XMEDIA_TRUE;
    yolov5_attr.motionless_filter_enable = XMEDIA_FALSE;
    yolov5_attr.aov_only_target = XMEDIA_FALSE;

    s32Ret = xmedia_svp_task_set_attr(g_svp_handle, &yolov5_attr);
    if(s32Ret != XMEDIA_SUCCESS) {
        MEDIA_ERR("svp set task attr fail.\n");
        xmedia_svp_task_destroy(g_svp_handle);
        xmedia_svp_uninit();
        return XMEDIA_NULL;
    }

    quickstart_comm_svp_proc();

    s32Ret = xmedia_svp_task_destroy(g_svp_handle);
    if (XMEDIA_SUCCESS != s32Ret) {
        MEDIA_ERR("svp destory error\n");
    }

    g_svp_handle = -1;
    xmedia_svp_uninit();
    xmedia_tde_exit();

    return XMEDIA_NULL;
}

static xmedia_void* quickstart_comm_venc_get_venc_stream_proc(xmedia_void* p)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_s32 i = 0;
    xmedia_s32 venc_chn_num = 0;
    xmedia_u32 venc_mask = 0;
    FILE *venc_fd[VENC_MAX_CHN_NUM];
    xmedia_char file_path[VENC_MAX_CHN_NUM][256] = {0};
    struct timeval time_out;
    xmedia_venc_chn_attr venc_chn_attr = {0};
    xmedia_venc_chn_status stat = {0};
    xmedia_venc_stream venc_stream = {0}; 
    quickstart_comm_venc_get_stream_param* pst_param = NULL;

    pst_param = (quickstart_comm_venc_get_stream_param*)p;

    venc_chn_num = pst_param->s32Cnt;
    if (venc_chn_num > VENC_MAX_CHN_NUM) {
        MEDIA_ERR("input venc chn num is invail \n");
        return NULL;
    }

    for (i = 0; i < venc_chn_num; i++) {
        if (pst_param->venc_chn[i] < 0) {
            MEDIA_ERR("input venc chn is invail \n");
            continue;
        }

        s32Ret = xmedia_venc_get_chn_attr(pst_param->venc_chn[i], &venc_chn_attr);
        if (XMEDIA_SUCCESS != s32Ret) {
            MEDIA_ERR("xmedia_venc_get_chn_attr chn[%d] failed with %#x!\n", pst_param->venc_chn[i], s32Ret);
            return XMEDIA_NULL;
        }

        switch (venc_chn_attr.venc_attr.en_type)
        {
            case PT_H264:
                snprintf(file_path[i], 255, "/mnt/stream_chn%d%s", i, ".h264");
                break;
            case PT_H265:
                snprintf(file_path[i], 255, "/mnt/stream_chn%d%s", i, ".h265");
                break;
            case PT_JPEG:
                snprintf(file_path[i], 255, "/mnt/stream_chn%d%s", i, ".jpg");
                break;
            case PT_MJPEG:
                snprintf(file_path[i], 255, "/mnt/stream_chn%d%s", i, ".mjpeg");
                break;
            default:
                MEDIA_ERR("venc type is invail exit stream thread \n");
                return NULL;
        }

        venc_fd[i] = fopen(file_path[i],"wb");
        if (venc_fd[i] == NULL) {
            MEDIA_ERR("open venc stream save file fail\n");
            return NULL;
        }

        venc_mask |= 1 << pst_param->venc_chn[i];
    }

    while (g_stream_param.thread_start)
    {
        time_out.tv_sec  = 2;
        time_out.tv_usec = 0;
        s32Ret = xmedia_venc_select(venc_mask, &time_out);
        if (s32Ret == XMEDIA_ERRCODE_INVALID_PARAM || s32Ret == XMEDIA_FAILURE) {
            MEDIA_ERR("select err\n");
            break;
        } else if (s32Ret == XMEDIA_ERRCODE_TIMEOUT) {
            MEDIA_PRT("get venc stream time out, continue. \n");
            continue;
        }

        for (i = 0; i < venc_chn_num; i++) {
            if (pst_param->venc_chn[i] < 0) {
                MEDIA_ERR("input venc chn is invail \n");
                continue;
            }

            s32Ret = xmedia_venc_query_status(pst_param->venc_chn[i], &stat);
            if (XMEDIA_SUCCESS != s32Ret) {
                MEDIA_ERR("xmedia_venc_query_status chn[%d] failed with %#x!\n", pst_param->venc_chn[i], s32Ret);
                break;
            }

            if (0 == stat.cur_packs) {
                continue;
            }

            memset(&venc_stream, 0, sizeof(venc_stream));
            venc_stream.pack = (xmedia_venc_pack*)malloc(sizeof(xmedia_venc_pack) * stat.cur_packs);
            if (XMEDIA_NULL == venc_stream.pack) {
                MEDIA_ERR("malloc stream pack failed!\n");
                break;
            }

            venc_stream.pack_count = stat.cur_packs;
            s32Ret = xmedia_venc_get_stream(pst_param->venc_chn[i], &venc_stream, XMEDIA_TRUE);
            if (XMEDIA_SUCCESS != s32Ret) {
                free(venc_stream.pack);
                venc_stream.pack = XMEDIA_NULL;
                xmedia_venc_release_stream(pst_param->venc_chn[i], &venc_stream);
                MEDIA_ERR("xmedia_venc_get_stream failed with %#x!\n", s32Ret);
                break;
            }

            quickstart_comm_venc_save_stream(venc_fd[i], &venc_stream);

            s32Ret = xmedia_venc_release_stream(pst_param->venc_chn[i], &venc_stream);
            if (XMEDIA_SUCCESS != s32Ret) {
                MEDIA_ERR("xmedia_venc_release_stream failed!\n");
                free(venc_stream.pack);
                venc_stream.pack = XMEDIA_NULL;
                break;
            }

            free(venc_stream.pack);
            venc_stream.pack = XMEDIA_NULL;
        }
    }

    for (i = 0; i < venc_chn_num; i++) {
        if (venc_fd[i] != NULL) {
            fclose(venc_fd[i]);
            venc_fd[i] = NULL;
        }
    }

    return NULL;
}

xmedia_s32 quickstart_comm_venc_start_get_stream(xmedia_s32 venc_chn[], xmedia_s32 venc_chn_num)
{
    xmedia_s32 i = 0;
    g_stream_param.thread_start = XMEDIA_TRUE;
    g_stream_param.s32Cnt = venc_chn_num;

    for (i = 0; i < venc_chn_num; i++) {
        g_stream_param.venc_chn[i] = venc_chn[i];
    }

    return pthread_create(&g_venc_thread_pid, 0, quickstart_comm_venc_get_venc_stream_proc, (xmedia_void*)&g_stream_param);
}

xmedia_s32 quickstart_comm_venc_stop_get_stream(void)
{
    if (XMEDIA_TRUE == g_stream_param.thread_start) {
        g_stream_param.thread_start = XMEDIA_FALSE;

        if (g_venc_thread_pid > 0) {
            pthread_join(g_venc_thread_pid, 0);
        }
    }
    return XMEDIA_SUCCESS;
}

xmedia_s32 quickstart_comm_svp_thread_create(xmedia_void)
{
    xmedia_s32 s32Ret = -1;
    pthread_attr_t svp_create;
    pthread_t svp_thread;
    pthread_attr_init(&svp_create);
    pthread_attr_setdetachstate(&svp_create, PTHREAD_CREATE_DETACHED);

    s32Ret = pthread_create(&svp_thread, &svp_create, quickstart_comm_svp_thread, NULL);
    if(s32Ret != 0)
    {
        MEDIA_ERR("create svp fail \n");
    }
    return s32Ret;
}
