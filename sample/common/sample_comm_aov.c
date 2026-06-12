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
#include <semaphore.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "sample_comm.h"
#include "sample_comm_aov.h"
#include "sample_comm_periph.h"
#include "sample_comm_thrd_pool.h"

#define MAX_STREAM_SIZE         (1024 * 1024 * 1024)
#define VENC_MAX_PACK_NUM        5
#define SDCARD_MOUNT_PATH       "/tmp/sd"

#define DEVICES_BIND_DONE_BASE "bind@/devices/platform/soc"
#define DEVICES_UNBIND_DONE_BASE "unbind@/devices/platform/soc"

#define SDCARD_DEVICE0 "10010000.sdhci"
#define SDCARD_DEVICE1 "10040000.sdhci"
#define SDCARD_DEVICE2 "10020000.sdhci"
#define SDCARD_DRIVER "/sys/bus/platform/drivers/sdhci-lotus"

static xmedia_bool g_transfer_drv_already_load = XMEDIA_FALSE;
static xmedia_bool g_audio_already_open = XMEDIA_FALSE;

#ifdef AOV_APP_TIMESTAMP
typedef struct time_stamp {
    xmedia_u64 drv_resume_start;
    xmedia_u64 app_in;
    xmedia_u64 vpss_in;
    xmedia_u64 svp_in;
    xmedia_u64 svp_out;
    xmedia_u64 app_out;
    xmedia_u64 drv_suspend_end;
} time_stamp;

static time_stamp timestamp[15] = {0};
static xmedia_s32 test_num = -1;
#endif

static sample_aov_work_mode  g_media_work_mode = MEDIA_WORK_NORMAL;
static sample_aov_init_param g_aov_init_param = {0};
static xmedia_bool           g_venc_save_all = XMEDIA_FALSE;

static xmedia_bool             g_venc_thread = XMEDIA_FALSE;
static xmedia_bool             g_work_aov_thread = XMEDIA_FALSE;

static pthread_mutex_t g_sync_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_npu_detect_mutex = PTHREAD_MUTEX_INITIALIZER;
static sem_t           g_venc_steram_save_start_sem;
static sem_t           g_venc_steram_save_stop_sem;
thread_pool_t *g_aov_thrd_pool_handle = NULL;

static xmedia_s32 g_misc_fd = 0;

sample_aov_detect_t g_aov_detect;

#ifdef SUPPORT_SENSOR_PWDN_MODE
static xmedia_bool support_sensor_pwdn_standby[VI_MAX_DEV_NUM] = {0}; 
#endif

xmedia_s32 sample_aov_system(xmedia_char *command)
{
    FILE* fp = popen(command, "w");
    if (fp == NULL)
    {
        SAMPLE_ERR("aov system command open fail \n");
        return XMEDIA_FAILURE;
    }

    pclose(fp);
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_aov_drv_audio_load()
{
    g_audio_already_open = XMEDIA_TRUE;
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_aov_drv_audio_unload()
{
    g_audio_already_open = XMEDIA_FALSE;
    return XMEDIA_SUCCESS;
}

#ifdef AOV_APP_TIMESTAMP
// precision : 10ms.
xmedia_u64 sample_aov_get_debug_timestamp()
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;

    xmedia_pm_rtc_info rtc_info = {0};
    s32Ret = xmedia_pm_get_rtc_info(&rtc_info);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_pm_get_rtc_info fail\n");
        return 0;
    }

    return (rtc_info.rtc_current_time);
}
#endif

static xmedia_s32 sample_aov_drv_sdcard_load()
{
    xmedia_s32 s32Ret = XMEDIA_SUCCESS;
    xmedia_char buf[256] = {0};

    pthread_mutex_lock(&g_sync_mutex);

    memset(buf, 0 , sizeof(buf));
    snprintf(buf, 256, "%s/%s", DEVICES_BIND_DONE_BASE,SDCARD_DEVICE0);
    sample_comm_periph_drv_bind(SDCARD_DEVICE0, SDCARD_DRIVER, buf);

    memset(buf, 0 , sizeof(buf));
    snprintf(buf, 256, "%s/%s", DEVICES_BIND_DONE_BASE,SDCARD_DEVICE1);
    sample_comm_periph_drv_bind(SDCARD_DEVICE1, SDCARD_DRIVER, buf);

    memset(buf, 0 , sizeof(buf));
    snprintf(buf, 256, "%s/%s", DEVICES_BIND_DONE_BASE,SDCARD_DEVICE2);
    sample_comm_periph_drv_bind(SDCARD_DEVICE2, SDCARD_DRIVER, buf);

    s32Ret = sample_comm_periph_mount_sdcard(SDCARD_MOUNT_PATH, 2000);
    if(s32Ret != XMEDIA_SUCCESS)
    {
        SAMPLE_ERR("mount SD failed!\n");
        g_transfer_drv_already_load = XMEDIA_FALSE;
    } else {
        g_transfer_drv_already_load = XMEDIA_TRUE;
    }
    pthread_mutex_unlock(&g_sync_mutex);
    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_aov_drv_sdcard_unload()
{
    xmedia_char buf[256] = {0};

    pthread_mutex_lock(&g_sync_mutex);

    sample_comm_periph_umount_sdcard(SDCARD_MOUNT_PATH);

    memset(buf, 0 , sizeof(buf));
    snprintf(buf, 256, "%s/%s", DEVICES_UNBIND_DONE_BASE,SDCARD_DEVICE2);
    sample_comm_periph_drv_unbind(SDCARD_DEVICE2, SDCARD_DRIVER, buf);

    memset(buf, 0 , sizeof(buf));
    snprintf(buf, 256, "%s/%s", DEVICES_UNBIND_DONE_BASE,SDCARD_DEVICE1);
    sample_comm_periph_drv_unbind(SDCARD_DEVICE1, SDCARD_DRIVER, buf);

    memset(buf, 0 , sizeof(buf));
    snprintf(buf, 256, "%s/%s", DEVICES_UNBIND_DONE_BASE,SDCARD_DEVICE0);
    sample_comm_periph_drv_unbind(SDCARD_DEVICE0, SDCARD_DRIVER, buf);
    g_transfer_drv_already_load = XMEDIA_FALSE;
    pthread_mutex_unlock(&g_sync_mutex);
    return XMEDIA_SUCCESS;
}

#if 0
static xmedia_s32 sample_aov_isp_set_max_exptime(xmedia_u32 isp_pipe)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_ae_exposure_attr exp_attr = {0};
    media_comm_isp_sensor_info sns_info = {0};

    media_comm_isp_get_sensor_info(isp_pipe, &sns_info);

    s32Ret = xmedia_ae_get_exposure_attr(isp_pipe, &exp_attr);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_ae_get_exposure_attr fail\n");
        return s32Ret;
    }

    exp_attr.auto_attr.exp_time_range.max = (1000 / sns_info.frame_rate) * 1000;//us
    s32Ret = xmedia_ae_set_exposure_attr(isp_pipe, &exp_attr);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_ae_get_exposure_attr fail\n");
        return s32Ret;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_aov_isp_set_mode(xmedia_u32 isp_pipe, aov_media_state media_state)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_isp_attr isp_attr = {0};
    media_comm_isp_sensor_info sns_info = {0};

    media_comm_isp_get_sensor_info(isp_pipe, &sns_info);

    s32Ret = xmedia_isp_get_attr(isp_pipe, &isp_attr);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_ae_get_exposure_attr fail\n");
        return s32Ret;
    }

    if (media_state == MEDIA_STATE_AOV ||  media_state == MEDIA_STATE_AOV_SAVE) {
        isp_attr.fps = (isp_attr.fps >= 30)?(sns_info.frame_rate - 5):(sns_info.frame_rate);
    } else {
        isp_attr.fps = sns_info.frame_rate;
    }
    s32Ret = xmedia_isp_set_attr(isp_pipe, &isp_attr);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_ae_get_exposure_attr fail\n");
        return s32Ret;
    }

    return XMEDIA_SUCCESS;
}
#endif

static xmedia_s32 sample_aov_sensor_resume(xmedia_u32 isp_pipe)
{
    xmedia_s32 s32Ret = XMEDIA_SUCCESS;

#ifdef SUPPORT_SENSOR_PWDN_MODE
    if(g_misc_fd != -1 && support_sensor_pwdn_standby[isp_pipe] == XMEDIA_TRUE) {
        ioctl(g_misc_fd, XMEDIA_MISC_SENSOR_PWDN_RESUME, &isp_pipe);
    }
#endif
    
    xmedia_sensor_low_power(isp_pipe, XMEDIA_FALSE);
    s32Ret = xmedia_sensor_resume(isp_pipe);
    return s32Ret;
}

#ifdef APP_STANDBY_SENSOR
static xmedia_s32 sample_aov_sensor_standby(xmedia_u32 isp_pipe)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;

LOOP:
    s32Ret = xmedia_isp_wait_state(isp_pipe, XMEDIA_ISP_WAIT_TYPE_FE_END, 2000);
    if (s32Ret == XMEDIA_SUCCESS) {
        s32Ret = xmedia_sensor_standby(isp_pipe);
        xmedia_sensor_low_power(isp_pipe, XMEDIA_TRUE);
        if (s32Ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("sensor standby set fail , try again\n");
        }
        else {
            for (xmedia_s32 i = 0; i < g_aov_init_param.isp_param.isp_pipe_num; i++) {
                if(g_aov_init_param.isp_param.isp_pipe[i] == isp_pipe) {
                    g_aov_init_param.sensor_status[i] = SENSOR_STATUS_STANDBY;
                }
            }
        }
    } else {
        SAMPLE_ERR("wait isp fe state start fail %#x\n",s32Ret);
        goto LOOP;
    }

    return XMEDIA_SUCCESS;
}

// 线程执行函数
xmedia_void sensor_standby_job_func(xmedia_void *arg)
{
    xmedia_u32 isp_pipe = *(xmedia_u32 *)arg;
    sample_aov_sensor_standby(isp_pipe);
    return ;
}

xmedia_s32 start_sensor_standby_nonblocking(xmedia_s32* isp_pipe)
{
    xmedia_s32 s32Ret = 0;
    s32Ret = sample_comm_thrd_job_submit(g_aov_thrd_pool_handle, &sensor_standby_job_func, isp_pipe);
    return s32Ret;
}
#else
static xmedia_s32 sample_aov_set_sensor_mode(sample_aov_work_mode work_mode)
{
    misc_aov_state aov_state = {0};

    aov_state.en_aov_mode = (work_mode == MEDIA_WORK_AOV ? XMEDIA_TRUE : XMEDIA_FALSE);
    aov_state.frame_num = 1;

    return ioctl(g_misc_fd, XMEDIA_MISC_AOV_SET_SENSOR_MODE, (xmedia_u32)&aov_state, NULL);
}
#endif

xmedia_s32 sample_aov_ae_modify_path(xmedia_u32 isp_pipe)
{
    xmedia_s32 s32Ret = 0;
    xmedia_s32 node_cnt = 0;
    xmedia_isp_ae_route ae_route_attr = {0};

    s32Ret = xmedia_ae_get_route_attr(0, &ae_route_attr);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("get ae route fail\n");
        return s32Ret;
    }

    ae_route_attr.route_node[node_cnt].int_time = 33;
    ae_route_attr.route_node[node_cnt].sys_gain = 1024;
    node_cnt++;

    ae_route_attr.route_node[node_cnt].int_time = 33;
    ae_route_attr.route_node[node_cnt].sys_gain = 1024*2;
    node_cnt++;

    ae_route_attr.route_node[node_cnt].int_time = 5000;
    ae_route_attr.route_node[node_cnt].sys_gain = 1024*2;
    node_cnt++;

    ae_route_attr.route_node[node_cnt].int_time = 5000;
    ae_route_attr.route_node[node_cnt].sys_gain = 1024*4;
    node_cnt++;

    ae_route_attr.route_node[node_cnt].int_time = 10000;
    ae_route_attr.route_node[node_cnt].sys_gain = 1024*4;
    node_cnt++;

    ae_route_attr.route_node[node_cnt].int_time = 10000;
    ae_route_attr.route_node[node_cnt].sys_gain = 109993;
    node_cnt++;

    ae_route_attr.route_node[node_cnt].int_time = 1000 * 1000;
    ae_route_attr.route_node[node_cnt].sys_gain = 109993;
    node_cnt++;

    ae_route_attr.route_node[node_cnt].int_time = 1000 * 1000;
    ae_route_attr.route_node[node_cnt].sys_gain = 3519776;
    node_cnt++;

    ae_route_attr.route_node[node_cnt].int_time = 1000 * 1000;
    ae_route_attr.route_node[node_cnt].sys_gain = 35197760;

    ae_route_attr.total_num = node_cnt;
    s32Ret = xmedia_ae_set_route_attr(isp_pipe, &ae_route_attr);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("set ae route fail\n");
    }

    return s32Ret;
}

/****************************************************************************
**aov start processs:
*** |  delta0        |    app              |  delta1      | ****************
**   por---auxcode----drv_resume----app-----drv_suspend----set rtc_timeout---soc power off ----*
**delta0: use rtc resume set rtc_timeout; = (rtc_delta_time - rtc_last_suspend_time - rtc_timeout).*
**delta_time1: drv_suspend; = rtc_last_suspend_time - last app_end_timestamp;     *
**keep work time + suspend = rtc_timeout,should get delta0 & delta1.                                   *
****/

xmedia_s32 sample_aov_set_suspend_time()
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_pm_attr pm_attr = {0};
    xmedia_pm_rtc_info rtc_info = {0};
    xmedia_u32 app_time = 0;
    static xmedia_s32 delta_time0 = 0;
    static xmedia_s32 delta_time1 = 0;
    static xmedia_u64 app_end_timestamp = 0;
    static xmedia_bool rtc_time_amend_end = XMEDIA_FALSE;
    static xmedia_bool update_rtc_time_end = XMEDIA_FALSE;

    s32Ret = xmedia_pm_get_rtc_info(&rtc_info);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_pm_get_rtc_info fail\n");
        return XMEDIA_FAILURE;
    }

    if (rtc_time_amend_end == XMEDIA_TRUE) {
        if (delta_time0 == 0 && delta_time1 == 0) {
            delta_time0 = rtc_info.rtc_delta_time - rtc_info.rtc_last_suspend_time - g_aov_init_param.suspend_time;
            delta_time1 = rtc_info.rtc_last_suspend_time - app_end_timestamp;
            g_aov_init_param.suspend_time -= delta_time0;
        }

        app_time = rtc_info.rtc_current_time - rtc_info.rtc_delta_time;

        if ((app_time + delta_time1 + RTC_SAFE_THRESOHLD_MS) > g_aov_init_param.suspend_time || update_rtc_time_end == XMEDIA_FALSE) {
            pm_attr.rtc_attr.rtc_enable = XMEDIA_TRUE;
            pm_attr.rtc_attr.rtc_timeout = (g_aov_init_param.suspend_time / 10);//10ms 精度: /10;

            s32Ret = xmedia_pm_set_attr(&pm_attr);
            if (s32Ret != XMEDIA_SUCCESS) {
                SAMPLE_ERR("pm set fail \n");
            }

            SAMPLE_PRT("dudu debug delta0 %d delta1 %d rtc_timeout %d \n",delta_time0, delta_time1, g_aov_init_param.suspend_time);

            update_rtc_time_end = XMEDIA_TRUE;
        }
    } else {
        pm_attr.rtc_attr.rtc_enable = XMEDIA_TRUE;
        pm_attr.rtc_attr.rtc_timeout = (g_aov_init_param.suspend_time / 10);//10ms 精度: /10;

        s32Ret = xmedia_pm_set_attr(&pm_attr);
        if (s32Ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("pm set fail \n");
        }

        app_end_timestamp = rtc_info.rtc_current_time;
        rtc_time_amend_end = XMEDIA_TRUE;
    }

    return XMEDIA_SUCCESS;
}

xmedia_void sample_aov_suspend()
{
    sample_aov_set_suspend_time();

#ifdef AOV_APP_TIMESTAMP
    if(test_num != -1) {
        timestamp[test_num].app_out = sample_aov_get_debug_timestamp();
    }

    if (test_num == 11) {
        for (test_num = 0; test_num < 10; test_num ++) {
            SAMPLE_PRT("debug aov %d test : drv_resume_in[%lld] app_in[%lld] vpss_in[%lld] svp_in[%lld] svp_out[%lld] app_out[%lld] drv_suspend_end[%lld]\n",
            test_num, timestamp[test_num].drv_resume_start, timestamp[test_num].app_in, timestamp[test_num].vpss_in,timestamp[test_num].svp_in,
            timestamp[test_num].svp_out,timestamp[test_num].app_out,timestamp[test_num].drv_suspend_end);
        }
        sleep(20);

        test_num = -1;
        memset(timestamp, 0, sizeof (timestamp));
    }

    test_num ++;
#endif
    xmedia_pm_suspend();

#ifdef AOV_APP_TIMESTAMP
    timestamp[test_num].app_in = sample_aov_get_debug_timestamp();

    xmedia_pm_rtc_info rtc_info = {0};
    xmedia_pm_get_rtc_info(&rtc_info);

    timestamp[test_num].drv_resume_start = rtc_info.rtc_delta_time;

    if (test_num >= 1) {
        timestamp[test_num - 1].drv_suspend_end = rtc_info.rtc_last_suspend_time;
    }

#endif

#ifdef APP_STANDBY_SENSOR
    //isp driver has do sensor resume
    for (xmedia_s32 i = 0; i < g_aov_init_param.isp_param.isp_pipe_num; i++) {
        g_aov_init_param.sensor_status[i] = SENSOR_STATUS_RESUME;
    }
#endif

    return;
}

static xmedia_s32 sample_aov_npu_set_work_mode(sample_aov_work_mode work_mode)
{
    xmedia_s32 s32Ret = XMEDIA_SUCCESS;
    xmedia_s32 i = 0;
    xmedia_svp_aov_attr yolov5_attr = {0};
    xmedia_bool cur_npu_mode = XMEDIA_TRUE;
    sample_aov_vpss_npu_param *npu_param = XMEDIA_NULL;

    npu_param = &(g_aov_init_param.npu_param);

    for (i = 0; i < npu_param->npu_detect_num; i++) {
        if (npu_param->en_switch_mode[i] == XMEDIA_FALSE) {
            continue;
        }
        s32Ret = xmedia_svp_task_get_attr(npu_param->npu_svp_handle[i], &yolov5_attr);
        if (XMEDIA_SUCCESS != s32Ret) {
            SAMPLE_ERR("svp get task attr fail.\n");
            return s32Ret;
        }

        cur_npu_mode = (MEDIA_WORK_AOV == work_mode) ? XMEDIA_TRUE : XMEDIA_FALSE;
        if (cur_npu_mode == yolov5_attr.aov_only_target) {
            return XMEDIA_SUCCESS;
        }

        yolov5_attr.aov_only_target = cur_npu_mode;

        s32Ret = xmedia_svp_task_set_attr(npu_param->npu_svp_handle[i], &yolov5_attr);
        if(s32Ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("svp set task attr fail.\n");
            return XMEDIA_FAILURE;
        }
    }

    return XMEDIA_SUCCESS;
}

static xmedia_void sample_aov_venc_set_stream_save_start()
{
    sem_post(&g_venc_steram_save_start_sem);
}

static xmedia_void sample_aov_venc_get_stream_save_stop()
{
    sem_wait(&g_venc_steram_save_stop_sem);
}

static xmedia_void sample_aov_venc_set_save_all(xmedia_bool en_save)
{
    pthread_mutex_lock(&g_sync_mutex);
    g_venc_save_all = en_save;
    pthread_mutex_unlock(&g_sync_mutex);
}

static xmedia_void sample_aov_venc_get_save_all(xmedia_bool *en_save)
{
    pthread_mutex_lock(&g_sync_mutex);
    *en_save = g_venc_save_all;
    pthread_mutex_unlock(&g_sync_mutex);
}

static xmedia_s32 sample_aov_venc_stream_write(FILE* pFd, xmedia_venc_stream* venc_stream)
{
    xmedia_s32 i;
    for (i = 0; i < venc_stream->pack_count; i++) {
        fwrite(venc_stream->pack[i].vir_addr + venc_stream->pack[i].offset,
               venc_stream->pack[i].len - venc_stream->pack[i].offset, 1, pFd);
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_aov_venc_get_transfer_path(xmedia_s32 venc_chn, xmedia_char *file_path)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_venc_chn_attr venc_chn_attr = {0};
    xmedia_char buf[256] = {0};
    struct stat file_stat = {0};
    static xmedia_s32 path_count = 0;

    s32Ret = xmedia_venc_get_chn_attr(venc_chn, &venc_chn_attr);
    if (XMEDIA_SUCCESS != s32Ret) 
    {
        SAMPLE_ERR("xmedia_venc_get_chn_attr chn[%d] failed with %#x!\n", venc_chn, s32Ret);
        return s32Ret;
    }

    switch (venc_chn_attr.venc_attr.en_type)
    {
        case PT_H264:
            snprintf(buf, 255, ""SDCARD_MOUNT_PATH"/stream_chn%d_%d%s", venc_chn, path_count, ".h264");
                break;
        case PT_H265:
            snprintf(buf, 255, ""SDCARD_MOUNT_PATH"/stream_chn%d_%d%s", venc_chn, path_count, ".h265");
                break;
        case PT_JPEG:
            snprintf(buf, 255, ""SDCARD_MOUNT_PATH"/stream_chn%d_%d%s", venc_chn, path_count, ".jpg");
            break;
        case PT_MJPEG:
            snprintf(buf, 255, ""SDCARD_MOUNT_PATH"/stream_chn%d_%d%s", venc_chn, path_count, ".mjpeg");
            break;
        default:
            SAMPLE_ERR("venc type is invail exit stream thread \n");
            return XMEDIA_FAILURE;
    }
    strcpy(file_path, buf);

    s32Ret = stat(file_path, &file_stat);
    if (s32Ret < 0) {
        SAMPLE_PRT("get stream file fail, create file.\n");
        return XMEDIA_SUCCESS;
    }

    if (file_stat.st_size > MAX_STREAM_SIZE) {
        path_count ++;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_aov_venc_save_stream(sample_aov_work_mode work_mode)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_s32 i = 0;
    xmedia_s32 milli_sec = 1;
    xmedia_venc_chn_status chn_stat = {0};
    xmedia_venc_stream venc_stream = {0};
    xmedia_char file_path[256] = {0};
    FILE * venc_fd[VENC_MAX_CHN_NUM] = {XMEDIA_NULL};
    sample_aov_venc_param *venc_param = XMEDIA_NULL;
    xmedia_s32 cur_frame_num = 0;
    xmedia_bool save_all_stream = XMEDIA_FALSE;

    venc_param = &(g_aov_init_param.venc_param);

    for (i = 0; i < venc_param->venc_chn_num; i++) {
        s32Ret = sample_aov_venc_get_transfer_path(venc_param->venc_chn[i], file_path);
        if (s32Ret != XMEDIA_SUCCESS)
        {
            SAMPLE_ERR("get venc type fail \n");
            return XMEDIA_FAILURE;
        }

        venc_fd[i] = fopen(file_path,"ab");
        if (venc_fd[i] == XMEDIA_NULL)
        {
            SAMPLE_ERR("open aov chn fd fail,file_path:%s,errno:%s\n", file_path, strerror(errno));
            return XMEDIA_FAILURE;
        }
    }

    memset(&venc_stream, 0, sizeof(venc_stream));
    venc_stream.pack = (xmedia_venc_pack*)malloc(sizeof(xmedia_venc_pack) * VENC_MAX_PACK_NUM);
NORMAL_SAVE:
#if 0 //暂不考虑normal buf 满再写的方案
    sample_aov_venc_get_save_all(&save_all_stream);
    if (MEDIA_WORK_AOV == work_mode && save_all_stream == XMEDIA_FALSE) {
        for (i = 0; i < venc_param->venc_chn_num; i++) {
            s32Ret = xmedia_venc_query_status(venc_param->venc_chn[i], &chn_stat);
            if (XMEDIA_SUCCESS != s32Ret)
            {
                SAMPLE_ERR("xmedia_venc_query_status chn[%d] failed with %#x!\n", venc_param->venc_chn[i], s32Ret);
            } else {
                if ((venc_param->chn_max_size[i] - chn_stat.left_stream_bytes) > venc_param->chn_threshold_size[i]) {
                    chn_stat.left_stream_frames = 0;//未达到阈值，暂时不写
                }
            }
            cur_frame_num |= chn_stat.left_stream_frames;
        }

        if (cur_frame_num == 0) {
            goto NORMAL_SAVE;
        }
    }
#endif
//获取完成当前帧
STREAM_SAVE_CONTINUE:
    for (i = 0; i < venc_param->venc_chn_num; i++) {
        s32Ret = xmedia_venc_query_status(venc_param->venc_chn[i], &chn_stat);
        if (XMEDIA_SUCCESS != s32Ret)
        {
            SAMPLE_ERR("xmedia_venc_query_status chn[%d] failed with %#x!\n", venc_param->venc_chn[i], s32Ret);
        }

        if (chn_stat.cur_packs == 0) {
            continue;
        }

        venc_stream.pack_count = chn_stat.cur_packs;

        s32Ret = xmedia_venc_get_stream(venc_param->venc_chn[i], &venc_stream, milli_sec);
        if (XMEDIA_SUCCESS != s32Ret) {
            SAMPLE_ERR("xmedia_venc_get_stream chn %d failed with %#x! \n",venc_param->venc_chn[i], s32Ret);
        }

        sample_aov_venc_stream_write(venc_fd[i], &venc_stream);

        s32Ret = xmedia_venc_release_stream(venc_param->venc_chn[i], &venc_stream);
        if (XMEDIA_SUCCESS != s32Ret) {
            SAMPLE_ERR("xmedia_venc_release_stream failed!\n");
        }

        if (chn_stat.left_stream_frames > 0) {
            chn_stat.left_stream_frames -= 1;
        }
        cur_frame_num |= chn_stat.left_stream_frames;
    }

    if (cur_frame_num != 0) {
        cur_frame_num = 0;
        goto STREAM_SAVE_CONTINUE;
    }

    sample_aov_venc_get_save_all(&save_all_stream);
    if (MEDIA_WORK_NORMAL == work_mode && XMEDIA_FALSE == save_all_stream) {
        goto NORMAL_SAVE;//normal达到阈值再写卡
    } else {
        for (i = 0; i < venc_param->venc_chn_num; i++) {
            s32Ret = xmedia_venc_query_status(venc_param->venc_chn[i], &chn_stat);
            if (XMEDIA_SUCCESS != s32Ret)
            {
                SAMPLE_ERR("xmedia_venc_query_status chn[%d] failed with %#x!\n", venc_param->venc_chn[i], s32Ret);
            }

            cur_frame_num |= chn_stat.left_stream_frames;
        }

        if (cur_frame_num != 0) {
            cur_frame_num = 0;
            goto STREAM_SAVE_CONTINUE;
        }
    }

    free(venc_stream.pack);
    venc_stream.pack = XMEDIA_NULL;
    for (i = 0; i < venc_param->venc_chn_num; i++) {
        if (venc_fd[i] != XMEDIA_NULL) {
            fflush(venc_fd[i]);
            fsync(fileno(venc_fd[i]));
            fclose(venc_fd[i]);
            venc_fd[i] = XMEDIA_NULL;
        }
    }
    if (work_mode == MEDIA_WORK_NORMAL) {
        sample_aov_venc_set_save_all(XMEDIA_FALSE);//reset
    }

    return XMEDIA_SUCCESS;
}

static xmedia_bool sample_aov_venc_get_aov_buf_full(xmedia_void)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_bool buf_full = XMEDIA_FALSE;
    xmedia_s32 i = 0;
    xmedia_venc_chn_status stat = {0};
    sample_aov_venc_param *venc_param = XMEDIA_NULL;
#if 1
    static xmedia_s32 last_frame_size[VENC_MAX_CHN_NUM] = {0};
    static xmedia_s32 cur_frame_size[VENC_MAX_CHN_NUM] = {0};
    static xmedia_s32 max_frame_size[VENC_MAX_CHN_NUM] = {0};
#endif

    venc_param = &(g_aov_init_param.venc_param);

    for (i = 0; i < venc_param->venc_chn_num; i++) {
        if (venc_param->en_aov[i] != XMEDIA_TRUE) {
            continue;
        }

        s32Ret = xmedia_venc_query_status(venc_param->venc_chn[i], &stat);
        if (s32Ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("get buf full state venc query fail chn[%d] \n",venc_param->venc_chn[i]);
        }

        if (venc_param->chn_max_size[i] - stat.left_stream_bytes <= venc_param->chn_threshold_size[i]) {
            buf_full |= XMEDIA_TRUE;
            break;
        } else {
            buf_full |= XMEDIA_FALSE;
        }

        if (stat.left_stream_frames >= VENC_STR_BUF_DEFAULT_MAX_FRAME_COUNT) {
            buf_full |= XMEDIA_TRUE;
            break;
        } else {
            buf_full |= XMEDIA_FALSE;
        }

// update aov mode chn_threshold , use 2 * (last max size frame).
#if 1
        cur_frame_size[i] = stat.left_stream_bytes;
        last_frame_size[i] = cur_frame_size[i] - last_frame_size[i];
        max_frame_size[i] = (max_frame_size[i] > last_frame_size[i]) ? max_frame_size[i] : last_frame_size[i];
        venc_param->chn_threshold_size[i] = ((venc_param->chn_threshold_size[i] / 2) > max_frame_size[i]) ? venc_param->chn_threshold_size[i] : max_frame_size[i] * 2;
#if 0
        SAMPLE_PRT("DEBUG VENC CHN%d\n",venc_param->venc_chn[i]);
        SAMPLE_PRT("cur frame: %d\n",stat.left_stream_frames);
        SAMPLE_PRT("max_size:  %d\n", venc_param->chn_max_size[i]);
        SAMPLE_PRT("used:      %d\n",stat.left_stream_bytes);
        SAMPLE_PRT("remaining: %d\n",venc_param->chn_max_size[i]- stat.left_stream_bytes);
        SAMPLE_PRT("threshold: %d\n",venc_param->chn_threshold_size[i]);
        SAMPLE_PRT("last_frame:%d\n",last_frame_size[i]);
        SAMPLE_PRT("max_frame: %d\n",max_frame_size[i]);
#endif
        last_frame_size[i] = stat.left_stream_bytes;
#endif
    }

    return buf_full;
}

static xmedia_s32 sample_aov_venc_clear_buf(xmedia_s32 venc_chn)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_venc_recv_pic_param  recv_param = {0};
    recv_param.recv_pic_num = -1;

    s32Ret = xmedia_venc_stop_recv_frame(venc_chn);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("aov clear buf stop recv chn[%d] frame fail\n",venc_chn);
        return XMEDIA_FAILURE;
    }

    s32Ret = xmedia_venc_reset_chn(venc_chn);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("aov clear buf reset chn[%d] frame fail\n",venc_chn);
        return XMEDIA_FAILURE;
    }

    s32Ret = xmedia_venc_start_recv_frame(venc_chn, &recv_param);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("aov clear buf start recv chn[%d] frame fail\n",venc_chn);
        return XMEDIA_FAILURE;
    }
    return XMEDIA_SUCCESS;
}


static xmedia_void sample_aov_work_aov_save_resume()
{
    sample_aov_drv_sdcard_load();
    sample_aov_venc_set_stream_save_start();
}

static xmedia_void sample_aov_work_aov_save_suspend()
{
    sample_aov_venc_get_stream_save_stop();
    sample_aov_drv_sdcard_unload();
}

static xmedia_s32 sample_aov_work_normal_pipe_resume()
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_venc_recv_pic_param  recv_param = {0};
    recv_param.recv_pic_num = -1;
    xmedia_s32 i = 0;
    xmedia_s32 j = 0;

    sample_aov_isp_param *isp_param = XMEDIA_NULL;
    sample_aov_vpss_normal_only_param *vpss_param = XMEDIA_NULL;
    sample_aov_venc_param *venc_param = XMEDIA_NULL;

    isp_param  = &(g_aov_init_param.isp_param);
    vpss_param = &(g_aov_init_param.vpss_param);
    venc_param = &(g_aov_init_param.venc_param);

    //npu not only target
    sample_aov_npu_set_work_mode(MEDIA_WORK_NORMAL);

    sample_aov_drv_sdcard_load();

    // restart aov no used vpss ochn
    for (i = 0; i < vpss_param->vpss_pipe_num; i++) {
        for (j = 0; j < vpss_param->vpss_ochn_num[i]; j++) {
            s32Ret = xmedia_vpss_enable_ochn(vpss_param->vpss_pipe[i], vpss_param->vpss_ochn[i][j]);
            if (s32Ret != XMEDIA_SUCCESS) {
                SAMPLE_ERR("normal ressume enable vpss pipe %d ochn[%d] fail\n", vpss_param->vpss_pipe[i], vpss_param->vpss_ochn[i][j]);
            }
        }
    }

    // restart aov no used venc chn
    for (i = 0; i < venc_param->venc_chn_num; i++) {
        if (venc_param->en_aov[i] == XMEDIA_TRUE) {
            continue;//only restart normal chn
        }
        s32Ret = xmedia_venc_start_recv_frame(venc_param->venc_chn[i], &recv_param);
        if (s32Ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("normal resume start recv chn[%d] fail\n",venc_param->venc_chn[i]);
        }
    }

    sample_aov_venc_set_stream_save_start();

#ifndef APP_STANDBY_SENSOR
    sample_aov_set_sensor_mode(MEDIA_WORK_NORMAL);
#endif

    for (i = 0; i < isp_param->isp_pipe_num; i++) {
        sample_aov_sensor_resume(isp_param->isp_pipe[i]);
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_aov_work_normal_pipe_suspend()
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_s32 i = 0, j = 0;
#ifdef APP_STANDBY_SENSOR
    xmedia_s32 sensor_standby_flag = 0, need_delay = 0;
#endif

    sample_aov_isp_param *isp_param = XMEDIA_NULL;
    sample_aov_vpss_normal_only_param *vpss_param = XMEDIA_NULL;
    sample_aov_venc_param *venc_param = XMEDIA_NULL;

    isp_param  = &(g_aov_init_param.isp_param);
    vpss_param = &(g_aov_init_param.vpss_param);
    venc_param = &(g_aov_init_param.venc_param);

#ifdef APP_STANDBY_SENSOR
    for (i = 0; i < isp_param->isp_pipe_num; i++) {
        start_sensor_standby_nonblocking(&isp_param->isp_pipe[i]);
        sensor_standby_flag |= 1 << i;
    }
#else
    sample_aov_set_sensor_mode(MEDIA_WORK_AOV);
#endif

#ifdef IVE_DETECT_SUPPORT
    for (i = 0; i < isp_param->isp_pipe_num; i++) {
        g_aov_detect.ive_param[i].is_first_frm = XMEDIA_TRUE;
    }
#endif

    sample_aov_venc_set_save_all(XMEDIA_TRUE);

    sample_aov_venc_get_stream_save_stop();

    // stop aov no used venc chn, clear aov chn
    for (i = 0; i < venc_param->venc_chn_num; i++) {
        if (venc_param->en_aov[i] != XMEDIA_TRUE) {
            s32Ret = xmedia_venc_stop_recv_frame(venc_param->venc_chn[i]);
            if (s32Ret != XMEDIA_SUCCESS) {
                SAMPLE_ERR("normal suspend stop recv chn[%d] fail\n",venc_param->venc_chn[i]);
            }
        } else {
            sample_aov_venc_clear_buf(venc_param->venc_chn[i]);
        }
    }

    // stop aov no used vpss ochn
    for (i = 0; i < vpss_param->vpss_pipe_num; i++) {
        for (j = 0; j < vpss_param->vpss_ochn_num[i]; j++) {
            s32Ret = xmedia_vpss_disable_ochn(vpss_param->vpss_pipe[i], vpss_param->vpss_ochn[i][j]);
            if (s32Ret != XMEDIA_SUCCESS) {
                SAMPLE_ERR("normal suspend disable vpss pipe %d ochn[%d] fail\n", vpss_param->vpss_pipe[i], vpss_param->vpss_ochn[i][j]);
            }
        }
    }

    // npu only target
    sample_aov_npu_set_work_mode(MEDIA_WORK_AOV);

    sample_aov_drv_sdcard_unload();

#ifdef APP_STANDBY_SENSOR
    j = 0;
    while(sensor_standby_flag != 0) {
        need_delay = 1;
        for (i = 0; i < isp_param->isp_pipe_num; i++) {
            if(g_aov_init_param.sensor_status[i] == SENSOR_STATUS_STANDBY) {
                sensor_standby_flag &= ~(1 << i);
                need_delay = 0;
            }
        }
        
        //print alert 
        if(++j % 10000 == 0) {
            SAMPLE_ERR("Maybe something error in sensor standby!!!\n");
        }

        if(need_delay)
            usleep(500);
    }
#endif

    return XMEDIA_SUCCESS;
}


#ifdef IVE_DETECT_SUPPORT
static xmedia_s32 sample_aov_ive_mmz_alloc(xmedia_u64 *pu64_phy_addr, xmedia_void **pp_vir_addr,
                                          const xmedia_char *str_mmb, const xmedia_char *str_zone,
                                          xmedia_u32 u32_len)
{
    *pu64_phy_addr = xmedia_mmz_alloc(str_zone, str_mmb, u32_len);
    if (*pu64_phy_addr == 0) {
        return XMEDIA_FAILURE;
    }

    *pp_vir_addr = xmedia_mmz_map(*pu64_phy_addr, u32_len, 0);
    if (*pp_vir_addr == XMEDIA_NULL) {
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_aov_ive_mmz_free(xmedia_u64 u64_phy_addr, xmedia_void *p_vir_addr)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;

    if (u64_phy_addr == 0 || p_vir_addr == XMEDIA_NULL) {
        SAMPLE_ERR("sys_mmz_free fail.\n");
        return XMEDIA_FAILURE;
    }

    s32Ret = xmedia_mmz_unmap(p_vir_addr);
    if (s32Ret != XMEDIA_SUCCESS) {
        return XMEDIA_FAILURE;
    }

    return xmedia_mmz_free(u64_phy_addr);
}

static xmedia_s32 sample_aov_ive_create_image(xmedia_ive_image_s* pst_img, xmedia_u32 u32width, xmedia_u32 u32height)
{
    xmedia_u32 u32size = 0;
    xmedia_s32 s32Ret = XMEDIA_FAILURE;

    if (XMEDIA_NULL == pst_img) {
        SAMPLE_ERR("create_image is NULL.\n");
        return XMEDIA_FAILURE;
    }

    pst_img->u32width = u32width;
    pst_img->u32height = u32height;
    pst_img->au32stride[0] = (pst_img->u32width + (8 - pst_img->u32width % 8) % 8);

    u32size = pst_img->au32stride[0] * pst_img->u32height;
    s32Ret = sample_aov_ive_mmz_alloc(&pst_img->au64phyaddr[0], (xmedia_void**)&pst_img->au64viraddr[0], "ive_img", XMEDIA_NULL, u32size);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("Mmz Alloc fail,Error(%#x)\n", s32Ret);
        return s32Ret;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_aov_ive_create_mem_info(xmedia_ive_mem_info_s* pst_mem_info, xmedia_u32 u32size)
{
    xmedia_s32 s32Ret;

    if (NULL == pst_mem_info)
    {
        SAMPLE_ERR("pst_mem_info is null\n");
        return XMEDIA_FAILURE;
    }

    pst_mem_info->u32size = u32size;
    s32Ret = sample_aov_ive_mmz_alloc(&pst_mem_info->u64phyaddr, (xmedia_void**)&pst_mem_info->u64viraddr, "ive_mem", XMEDIA_NULL, u32size);
    if (s32Ret != XMEDIA_SUCCESS)
    {
        SAMPLE_ERR("Mmz Alloc fail,Error(%#x)\n", s32Ret);
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}


static xmedia_s32 sample_aov_ive_dma_image(xmedia_video_frame_info *pst_frame_info,xmedia_ive_dst_image_s *pst_dst,xmedia_bool b_instant)
{
    xmedia_s32 s32Ret;
    xmedia_ive_handle h_ive_handle;
    xmedia_ive_src_data_s st_src_data;
    xmedia_ive_dst_data_s st_dst_data;
    xmedia_ive_dma_ctrl_s st_ctrl = {XMEDIA_IVE_DMA_MODE_DIRECT_COPY,0};
    xmedia_bool b_finish = XMEDIA_FALSE;
    xmedia_bool b_block = XMEDIA_TRUE;

    //fill src
    st_src_data.u64phyaddr = pst_frame_info->frame.addr.y_phy_addr;
    st_src_data.u32width   = pst_frame_info->frame.width;
    st_src_data.u32height  = pst_frame_info->frame.height;
    st_src_data.u32stride  = pst_frame_info->frame.stride.y_stride;

    //fill dst
    st_dst_data.u64phyaddr = pst_dst->au64phyaddr[0];
    st_dst_data.u32width   = pst_dst->u32width;
    st_dst_data.u32height  = pst_dst->u32height;
    st_dst_data.u32stride  = pst_dst->au32stride[0];

    s32Ret = xmedia_ive_dma(&h_ive_handle,&st_src_data,&st_dst_data,&st_ctrl,b_instant);
    if (XMEDIA_SUCCESS != s32Ret) {
        SAMPLE_ERR("xmedia_ive_dma failed! \n");
        return s32Ret;
    }

    if (XMEDIA_TRUE == b_instant)
    {
        s32Ret = xmedia_ive_query(h_ive_handle,&b_finish,b_block);
        while(ERR_CODE_IVE_QUERY_TIMEOUT == s32Ret)
        {
            usleep(100);
            s32Ret = xmedia_ive_query(h_ive_handle,&b_finish,b_block);
        }
        if (XMEDIA_SUCCESS != s32Ret) {
            SAMPLE_ERR("xmedia_ive_query failed! \n");
            return s32Ret;
        }
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_aov_ive_init(xmedia_void)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_u8 u8WndSz = 0;
    xmedia_s32 i = 0, j = 0;
    
    memset(&g_aov_detect, 0, sizeof(sample_aov_detect_t));

    s32Ret = xmedia_md_init();
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("ive md init fail.\n");
        return XMEDIA_FAILURE;
    }

    for (i = 0; i < g_aov_init_param.isp_param.isp_pipe_num; i++) {
        g_aov_detect.ive_param[i].stMdAttr.en_alg_mode = XMEDIA_MD_ALG_MODE_REF;
        g_aov_detect.ive_param[i].stMdAttr.en_sad_mode = XMEDIA_IVE_SAD_MODE_MB_8X8;
        g_aov_detect.ive_param[i].stMdAttr.en_sad_out_ctrl = XMEDIA_IVE_SAD_OUT_CTRL_THRESH;
        g_aov_detect.ive_param[i].stMdAttr.u16_sad_thr = 1600;
        g_aov_detect.ive_param[i].stMdAttr.u32_width = NPU_CHN_WIDTH;
        g_aov_detect.ive_param[i].stMdAttr.u32_height = NPU_CHN_HEIGHT;
        g_aov_detect.ive_param[i].stMdAttr.st_add_ctrl.u0q16x = 32768;
        g_aov_detect.ive_param[i].stMdAttr.st_add_ctrl.u0q16y = 32768;
        g_aov_detect.ive_param[i].stMdAttr.st_ccl_ctrl.en_mode = XMEDIA_IVE_CCL_MODE_4C;
        u8WndSz = ( 1 << (2 + g_aov_detect.ive_param[i].stMdAttr.en_sad_mode));
        g_aov_detect.ive_param[i].stMdAttr.st_ccl_ctrl.u16_init_area_thr = 180;
        g_aov_detect.ive_param[i].stMdAttr.st_ccl_ctrl.u16_step = u8WndSz;
        g_aov_detect.ive_param[i].is_first_frm = XMEDIA_TRUE;
        g_aov_detect.ive_param[i].md_chn = i;

        for (j = 0; j < AOV_IVE_MD_IMAGE_NUM; j++) {
            s32Ret = sample_aov_ive_create_image(&g_aov_detect.ive_param[i].astImg[j], NPU_CHN_WIDTH, NPU_CHN_HEIGHT);
            if (s32Ret != XMEDIA_SUCCESS) {
                SAMPLE_ERR("ive create img fail.\n");
                goto ive_err0;
            }
        }
    
        s32Ret = sample_aov_ive_create_mem_info(&g_aov_detect.ive_param[i].stBlob, sizeof(xmedia_ive_ccblob_s));
        if (s32Ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("ive create mem fail.\n");
            goto ive_err1;
        }

        s32Ret = xmedia_md_create_chn(g_aov_detect.ive_param[i].md_chn, &(g_aov_detect.ive_param[i].stMdAttr));
        if (XMEDIA_SUCCESS != s32Ret) {
            goto ive_err2;
        }
    }

    return XMEDIA_SUCCESS;

ive_err2:
    xmedia_md_exit();
ive_err1:
    for (i = 0; i < g_aov_init_param.isp_param.isp_pipe_num; i++) {
        if(g_aov_detect.ive_param[i].stBlob.u64phyaddr != 0 &&
                g_aov_detect.ive_param[i].stBlob.u64viraddr != XMEDIA_NULL)
            sample_aov_ive_mmz_free(g_aov_detect.ive_param[i].stBlob.u64phyaddr, (xmedia_void *)(xmedia_ulong)g_aov_detect.ive_param[i].stBlob.u64viraddr);
    }
ive_err0:
    for (i = 0; i < g_aov_init_param.isp_param.isp_pipe_num; i++) {
        for (j = 0; j < AOV_IVE_MD_IMAGE_NUM; j++) {
            if(g_aov_detect.ive_param[i].astImg[j].au64phyaddr[0] != 0 &&
                    g_aov_detect.ive_param[i].astImg[j].au64viraddr[0] != XMEDIA_NULL)
            sample_aov_ive_mmz_free(g_aov_detect.ive_param[i].astImg[j].au64phyaddr[0], (xmedia_void *)(xmedia_ulong)g_aov_detect.ive_param[i].astImg[j].au64viraddr[0]);
        }
    }

    return XMEDIA_FAILURE;
}

static xmedia_s32 sample_aov_ive_exit(xmedia_void)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_s32 i = 0, j = 0;

    for (i = 0; i < g_aov_init_param.isp_param.isp_pipe_num; i++) {
        s32Ret = xmedia_md_destroy_chn(g_aov_detect.ive_param[i].md_chn);
        if (XMEDIA_SUCCESS != s32Ret) {
            SAMPLE_ERR("xmedia_md_destroy_chn fail.\n");
        }

        sample_aov_ive_mmz_free(g_aov_detect.ive_param[i].stBlob.u64phyaddr, (xmedia_void *)(xmedia_ulong)g_aov_detect.ive_param[i].stBlob.u64viraddr);

        for (j = 0; j < AOV_IVE_MD_IMAGE_NUM; j++) {
            sample_aov_ive_mmz_free(g_aov_detect.ive_param[i].astImg[j].au64phyaddr[0], (xmedia_void *)(xmedia_ulong)g_aov_detect.ive_param[i].astImg[j].au64viraddr[0]);
        }
    }

    xmedia_md_exit();

    return XMEDIA_SUCCESS;
}

static xmedia_bool sample_aov_svp_ive_detect(xmedia_video_frame_info *video_frame, aov_ive_param *ive_param)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_ive_ccblob_s *pst_blob = XMEDIA_NULL;
    
    if(ive_param == XMEDIA_NULL)
    {
        SAMPLE_ERR("input ive_param is NULL!\n");
        return XMEDIA_FALSE;
    }

    s32Ret = sample_aov_ive_dma_image(video_frame, &ive_param->astImg[ive_param->cur_idx], XMEDIA_TRUE);
    if (XMEDIA_SUCCESS != s32Ret) {
        SAMPLE_ERR("aov_ive_dma_image, ret = 0x%x\n", s32Ret);
        return XMEDIA_FALSE;
    }

    if (XMEDIA_TRUE == ive_param->is_first_frm) {
        ive_param->is_first_frm = XMEDIA_FALSE;
        goto CHANGE_IDX;
    }

    s32Ret = xmedia_md_process(ive_param->md_chn, &ive_param->astImg[ive_param->cur_idx], &ive_param->astImg[1 - ive_param->cur_idx],
                                XMEDIA_NULL, &ive_param->stBlob);
    if (XMEDIA_SUCCESS != s32Ret) {
        SAMPLE_ERR("xmedia_md_process, ret = 0x%x\n", s32Ret);
        return XMEDIA_FALSE;
    }

    pst_blob = (xmedia_ive_ccblob_s*)(xmedia_ulong)ive_param->stBlob.u64viraddr;
    if (pst_blob->u8_region_num > 0) {
        goto CHANGE_IDX;
    } else {
        return XMEDIA_FALSE;
    }

CHANGE_IDX: //Change reference and current frame index
    ive_param->cur_idx = 1 - ive_param->cur_idx;

    return XMEDIA_TRUE;
}
#endif

xmedia_void normal_job_func(xmedia_void *arg)
{
    xmedia_s32 milli_sec = 2000;
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_video_frame_info video_frame = {0};
    xmedia_svp_task_input task_input = {0};
    xmedia_s32 i = 0, index;
    sample_aov_vpss_npu_param *npu_param = XMEDIA_NULL;
    sample_aov_normal_npu_notice npu_notice = XMEDIA_NULL;
    xmedia_svp_aov_detect_output svp_result;
    sample_aov_isp_param *isp_param = XMEDIA_NULL;

    isp_param = &(g_aov_init_param.isp_param);
    
    index = *(xmedia_s32*)arg;
    if(index >= isp_param->isp_pipe_num)
    {
        SAMPLE_ERR("invalid index(%d)!!!\n", index);
        return;
    }

    npu_param = &(g_aov_init_param.npu_param);
    npu_notice = g_aov_init_param.npu_notice_callback;

    s32Ret = xmedia_vpss_acquire_ochn_frame(npu_param->vpss_pipe[index], npu_param->vpss_ochn[index], &video_frame, milli_sec);
    if (XMEDIA_SUCCESS != s32Ret) {
        SAMPLE_ERR("Warning, acquire pipe[%d] vpss ochn[%d] frame fail ret = 0x%x\n", npu_param->vpss_pipe[index], npu_param->vpss_ochn[index], s32Ret);
        goto END;
    }
    
    task_input.frame_num = 1;
    task_input.frame = &video_frame;

    s32Ret = xmedia_svp_task_process(npu_param->npu_svp_handle[index], &task_input, &svp_result);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_svp_process failed with %#x!\n", s32Ret);
        svp_result.is_exist = XMEDIA_FALSE;
    }
    
    s32Ret = xmedia_vpss_release_ochn_frame(npu_param->vpss_pipe[index], npu_param->vpss_ochn[index], &video_frame);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_vpss_release_ochn_frame failed with %#x!\n", s32Ret);
    }

    if(svp_result.is_exist == XMEDIA_FALSE)
        g_aov_detect.svp_detect_none_cnt[index]++;
    else
        g_aov_detect.svp_detect_none_cnt[index] = 0;

    if(g_aov_detect.svp_detect_none_cnt[index] >= NPU_NORMAL_STATE_CONTINUE_COUNT) {
        if (npu_notice != NULL) {
            npu_notice(MEDIA_WORK_AOV);
            for (i = 0; i < isp_param->isp_pipe_num; i++) {
                g_aov_detect.svp_detect_none_cnt[i] = 0;
            }
        } else {
            SAMPLE_ERR("npu notice callback not register \n");
        }
    }
    
END:
    g_aov_detect.detect_complete_flag[index] = 1;
}

static xmedia_void sample_aov_npu_normal_proc()
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_s32 i = 0, detect_flag = 0, need_delay = 0;
    static xmedia_s32 index[AOV_SUPPORT_MAX_SENSOR_NUM];
    sample_aov_isp_param *isp_param = XMEDIA_NULL;

    isp_param = &(g_aov_init_param.isp_param);

    for (i = 0; i < isp_param->isp_pipe_num; i++) {
        index[i] = i;
        g_aov_detect.detect_complete_flag[i] = 0;
        s32Ret = sample_comm_thrd_job_submit(g_aov_thrd_pool_handle, &normal_job_func, &index[i]);
        if (s32Ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("submit normal job[%d] failed!!!\n", i);
            continue;
        }
        detect_flag |= 1 << i;
    }
    
    while(detect_flag != 0) {
        need_delay = 1;
        for (i = 0; i < isp_param->isp_pipe_num; i++) {
            if(g_aov_detect.detect_complete_flag[i] == 1) {
                detect_flag &= ~(1 << i);
                g_aov_detect.detect_complete_flag[i] = 0;
                need_delay = 0;
            }
        }
        if(need_delay)
            usleep(500);
    }
}

xmedia_void aov_job_func(xmedia_void *arg)
{
    xmedia_s32 milli_sec = 2000;
    sample_aov_isp_param *isp_param = XMEDIA_NULL;
    sample_aov_vpss_npu_param *npu_param = XMEDIA_NULL;
    xmedia_s32 index = 0, s32Ret = XMEDIA_FAILURE;
    xmedia_svp_task_input task_input = {0};
    xmedia_video_frame_info video_frame = {0};
#ifdef IVE_DETECT_SUPPORT
    xmedia_bool ive_detect = XMEDIA_TRUE;
#endif

    npu_param = &(g_aov_init_param.npu_param);
    isp_param = &(g_aov_init_param.isp_param);

    index = *(xmedia_s32*)arg;
    if(index >= isp_param->isp_pipe_num)
    {
        SAMPLE_ERR("invalid index(%d)!!!\n", index);
        return;
    }

#ifdef APP_STANDBY_SENSOR
    sample_aov_sensor_standby(isp_param->isp_pipe[index]);
#endif

#ifdef AOV_APP_TIMESTAMP
    timestamp[test_num].vpss_in = sample_aov_get_debug_timestamp();
#endif

    s32Ret = xmedia_vpss_acquire_ochn_frame(npu_param->vpss_pipe[index], npu_param->vpss_ochn[index], &video_frame, milli_sec);
    if (XMEDIA_SUCCESS != s32Ret) {
        SAMPLE_ERR("Warning, acquire pipe[%d] vpss ochn[%d] frame fail ret = 0x%x\n", npu_param->vpss_pipe[index], npu_param->vpss_ochn[index], s32Ret);
        g_aov_detect.svp_result[index].is_exist = XMEDIA_FALSE;
        goto END;
    }

#ifdef AOV_APP_TIMESTAMP
    timestamp[test_num].svp_in = sample_aov_get_debug_timestamp();
#endif

    task_input.frame_num = 1;
    task_input.frame = &video_frame;

#ifdef IVE_DETECT_SUPPORT
    ive_detect = sample_aov_svp_ive_detect(&video_frame, &g_aov_detect.ive_param[index]);
    if (g_aov_detect.ive_param[index].is_first_frm == XMEDIA_TRUE) {
        g_aov_detect.ive_param[index].is_first_frm = XMEDIA_FALSE;
    }

    if (ive_detect == XMEDIA_FALSE) {
        g_aov_detect.svp_result[index].is_exist = XMEDIA_FALSE;
    }
    else
#endif
    {
        s32Ret = xmedia_svp_task_process(npu_param->npu_svp_handle[index], &task_input, &g_aov_detect.svp_result[index]);
        if (s32Ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("xmedia_svp_process failed with %#x!\n", s32Ret);
            g_aov_detect.svp_result[index].is_exist = XMEDIA_FALSE;
        }
    }

    s32Ret = xmedia_vpss_release_ochn_frame(npu_param->vpss_pipe[index], npu_param->vpss_ochn[index], &video_frame);
    if (s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_vpss_release_ochn_frame failed with %#x!\n", s32Ret);
    }

#ifdef IVE_DETECT_SUPPORT
    if(g_aov_detect.svp_result[index].is_exist == XMEDIA_TRUE)
        g_aov_detect.ive_param[index].is_first_frm = XMEDIA_TRUE;
#endif

END:
    g_aov_detect.detect_complete_flag[index] = 1;
}

static xmedia_void sample_aov_npu_aov_proc(xmedia_bool *is_exist)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_s32 i = 0, detect_flag = 0, need_delay = 0;
    static xmedia_s32 index[AOV_SUPPORT_MAX_SENSOR_NUM];
    sample_aov_isp_param *isp_param = XMEDIA_NULL;
   
    *is_exist = XMEDIA_FALSE;

    isp_param = &(g_aov_init_param.isp_param);

    for (i = 0; i < isp_param->isp_pipe_num; i++) {
        index[i] = i;
        g_aov_detect.detect_complete_flag[i] = 0;
        s32Ret = sample_comm_thrd_job_submit(g_aov_thrd_pool_handle, &aov_job_func, &index[i]);
        if (s32Ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("submit aov job[%d] failed!!!\n", i);
            continue;
        }
        detect_flag |= 1 << i;
    }

    while(detect_flag != 0)
    {
        need_delay = 1;
        for (i = 0; i < isp_param->isp_pipe_num; i++) {
            if(g_aov_detect.detect_complete_flag[i] == 1)
            {
                *is_exist |= g_aov_detect.svp_result[i].is_exist;
                detect_flag &= ~(1 << i);
                need_delay = 0;
            }
        }
        if(need_delay)
            usleep(500);
    }

#ifdef AOV_APP_TIMESTAMP
    timestamp[test_num].svp_out = sample_aov_get_debug_timestamp();
#endif

    return ;
}

static xmedia_void* sample_aov_venc_stream_thread(xmedia_void* p)
{
    sample_aov_work_mode work_mode = MEDIA_WORK_NORMAL;

VENC_THREAD_IDEL:
    sem_wait(&g_venc_steram_save_start_sem);
    sample_comm_aov_get_work_mode(&work_mode);

    if (g_venc_thread == XMEDIA_FALSE) {
        return XMEDIA_NULL;
    }

    sample_aov_venc_save_stream(work_mode);

    sem_post(&g_venc_steram_save_stop_sem);

    goto VENC_THREAD_IDEL;

    return XMEDIA_NULL;
}

static xmedia_void* sample_aov_work_thread(xmedia_void* p)
{
    xmedia_bool goal_is_exist = XMEDIA_FALSE;
    xmedia_bool venc_buf_full = XMEDIA_FALSE;
    xmedia_bool first_aov_state = XMEDIA_TRUE;
    sample_aov_work_mode work_mode = MEDIA_WORK_NORMAL;

    sample_aov_normal_npu_notice npu_notice = XMEDIA_NULL;
    sample_aov_normal_resume normal_resume = XMEDIA_NULL;
    sample_aov_normal_suspend normal_suspend = XMEDIA_NULL;

    npu_notice = g_aov_init_param.npu_notice_callback;
    normal_resume = g_aov_init_param.normal_resume_callback;
    normal_suspend = g_aov_init_param.normal_suspend_callback;


    while(g_work_aov_thread) {
        sample_comm_aov_get_work_mode(&work_mode);
        if (work_mode == MEDIA_WORK_NORMAL) {
            //normal npu set results only.
            sample_aov_npu_normal_proc();
        } else {
            if (first_aov_state != XMEDIA_TRUE) {
                //aov get npu goal state.
                sample_aov_npu_aov_proc(&goal_is_exist);
                if (goal_is_exist == XMEDIA_TRUE) {
                    //goal is exist switch normal. resume normal pipe only.
                    sample_comm_aov_set_work_mode(MEDIA_WORK_NORMAL);
                    sample_aov_work_normal_pipe_resume();

                    if (npu_notice != XMEDIA_NULL) {
                        npu_notice(MEDIA_WORK_NORMAL);
                    } else {
                        SAMPLE_ERR("npu notice callback not register \n");
                    }

                    if (normal_resume != XMEDIA_NULL)
                    {
                        normal_resume();
                    }
                    first_aov_state = XMEDIA_TRUE;
                    continue;
                } else {
                    //goal is nit exist, get venc buf state .
                    venc_buf_full = sample_aov_venc_get_aov_buf_full();
                    if (venc_buf_full == XMEDIA_TRUE) {
                        sample_aov_work_aov_save_resume();
                        sample_aov_work_aov_save_suspend();
                    } else {
                    }
                }
            }else {
                //normal switch aov first. suspend normal pipe.
                if (normal_suspend != XMEDIA_NULL)
                {
                    normal_suspend();
                }
                sample_aov_work_normal_pipe_suspend();
                first_aov_state = XMEDIA_FALSE;
            }
            sample_aov_suspend();
        }
    }

    return XMEDIA_NULL;
}

xmedia_void sample_comm_aov_init(sample_aov_init_param *aov_init_param)
{
    memcpy(&g_aov_init_param, aov_init_param, sizeof(sample_aov_init_param));
    pthread_mutex_init(&g_sync_mutex, NULL);
    pthread_mutex_init(&g_npu_detect_mutex, NULL);
    sem_init(&g_venc_steram_save_start_sem, 0, 1);
    sem_init(&g_venc_steram_save_stop_sem, 0, 0);
    for (xmedia_s32 i = 0; i < g_aov_init_param.isp_param.isp_pipe_num; i++) {
        sample_aov_ae_modify_path(g_aov_init_param.isp_param.isp_pipe[i]);
#ifdef APP_STANDBY_SENSOR
        g_aov_init_param.sensor_status[i] = SENSOR_STATUS_START;
#endif
    }

    g_aov_thrd_pool_handle = sample_comm_thrd_pool_init(g_aov_init_param.isp_param.isp_pipe_num, g_aov_init_param.isp_param.isp_pipe_num * 2);

    sample_aov_drv_sdcard_load();

#ifdef IVE_DETECT_SUPPORT
    sample_aov_ive_init();
#endif

}

xmedia_void sample_comm_aov_exit(xmedia_void)
{
#ifdef IVE_DETECT_SUPPORT
    sample_aov_ive_exit();
#endif

    if(g_aov_thrd_pool_handle)
    {
        sample_comm_thrd_pool_destroy(g_aov_thrd_pool_handle);
        g_aov_thrd_pool_handle = NULL;
    }

    sem_destroy(&g_venc_steram_save_start_sem);
    sem_destroy(&g_venc_steram_save_stop_sem);
    pthread_mutex_lock(&g_sync_mutex);
    pthread_mutex_unlock(&g_sync_mutex);
    pthread_mutex_destroy(&g_sync_mutex);
    pthread_mutex_lock(&g_npu_detect_mutex);
    pthread_mutex_unlock(&g_npu_detect_mutex);
    pthread_mutex_destroy(&g_npu_detect_mutex);
}

xmedia_void sample_comm_aov_set_work_mode(sample_aov_work_mode work_mode)
{
    pthread_mutex_lock(&g_sync_mutex);
    g_media_work_mode = work_mode;
    if (work_mode == MEDIA_WORK_AOV) {
        SAMPLE_ERR("--------->debug switch to aov<------\n");

    } else {
        SAMPLE_ERR("--------->debug switch to normal<--------\n");
    }
    pthread_mutex_unlock(&g_sync_mutex);
}

xmedia_void sample_comm_aov_get_work_mode(sample_aov_work_mode *work_mode)
{
    pthread_mutex_lock(&g_sync_mutex);
    *work_mode = g_media_work_mode;
    pthread_mutex_unlock(&g_sync_mutex);
}

xmedia_s32 sample_comm_aov_npu_init(xmedia_void)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;

    s32Ret = xmedia_tde_init();
    if (XMEDIA_SUCCESS != s32Ret) {
        SAMPLE_ERR("tde init fail");
        return s32Ret;
    }

    s32Ret = xmedia_svp_init();
    if (XMEDIA_SUCCESS != s32Ret) {
        SAMPLE_ERR("svp init fail");
        return s32Ret;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_aov_npu_create(xmedia_s32 *handle, xmedia_svp_task_cfg *task_cfg)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_svp_aov_attr yolov5_attr = {0};

    s32Ret = xmedia_svp_task_create(handle, *task_cfg);
    if (XMEDIA_SUCCESS != s32Ret) {
        SAMPLE_ERR("------------- svp create fail--please check your npu model name & path.\n");
        return s32Ret;
    }

    s32Ret = xmedia_svp_task_get_attr(*handle, &yolov5_attr);
    if (XMEDIA_SUCCESS != s32Ret) {
        SAMPLE_ERR("svp get task attr fail.\n");
        xmedia_svp_task_destroy(*handle);
        xmedia_svp_uninit();
        return s32Ret;
    }

    yolov5_attr.aov_only_target = XMEDIA_FALSE;

    s32Ret = xmedia_svp_task_set_attr(*handle, &yolov5_attr);
    if(s32Ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("svp set task attr fail.\n");
        xmedia_svp_task_destroy(*handle);
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_aov_npu_destroy(xmedia_s32 handle)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;

    s32Ret = xmedia_svp_task_destroy(handle);
    if (XMEDIA_SUCCESS != s32Ret) {
        SAMPLE_ERR("svp destroy error\n");
    }

    return s32Ret;
}

xmedia_void sample_comm_aov_npu_exit(xmedia_void)
{
    xmedia_svp_uninit();
    xmedia_tde_exit();
}

xmedia_s32 sample_comm_aov_venc_thread_create(xmedia_void)
{
    pthread_attr_t venc_thread_creat;
    pthread_t venc_thread;

    if (g_venc_thread == XMEDIA_TRUE) {
        SAMPLE_ERR("venc get stream thread already create \n");
        return XMEDIA_SUCCESS;
    }

    g_venc_thread = XMEDIA_TRUE;

    pthread_attr_init(&venc_thread_creat);
    pthread_attr_setdetachstate(&venc_thread_creat, PTHREAD_CREATE_DETACHED);
    return pthread_create(&venc_thread, &venc_thread_creat, sample_aov_venc_stream_thread, XMEDIA_NULL);
}

xmedia_s32 sample_comm_aov_venc_thread_destroy(xmedia_void)
{
    if (XMEDIA_TRUE == g_venc_thread) {
        g_venc_thread = XMEDIA_FALSE;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_aov_work_thread_create(xmedia_void)
{
    pthread_attr_t work_aov_creat;
    pthread_t work_aov_thraed;

    if (g_work_aov_thread == XMEDIA_TRUE) {
        SAMPLE_ERR("venc get stream thread already create \n");
        return XMEDIA_SUCCESS;
    }

    g_work_aov_thread = XMEDIA_TRUE;

    pthread_attr_init(&work_aov_creat);
    pthread_attr_setdetachstate(&work_aov_creat, PTHREAD_CREATE_DETACHED);
    return pthread_create(&work_aov_thraed, &work_aov_creat, sample_aov_work_thread, XMEDIA_NULL);
}

xmedia_s32 sample_comm_aov_work_thread_destroy(xmedia_void)
{
    if (XMEDIA_TRUE == g_work_aov_thread) {
        g_work_aov_thread = XMEDIA_FALSE;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_aov_misc_init(misc_aov_isp_info *isp_info)
{
    if(isp_info == XMEDIA_NULL) {
        SAMPLE_ERR("invalid input parameter!\n");
        return XMEDIA_FAILURE;
    }

    if(isp_info->isp_pipe_num == 0) {
        SAMPLE_ERR("invalid isp_pipe_num!\n");
        return XMEDIA_FAILURE;
    }

#ifdef SUPPORT_SENSOR_PWDN_MODE
    for(xmedia_s32 i = 0; i < isp_info->isp_pipe_num; i++) {
        support_sensor_pwdn_standby[i] = isp_info->support_sensor_pwdn_standby[i];
    }
#endif

    g_misc_fd = open("/dev/misc", O_RDONLY);
    if(g_misc_fd == -1) {
        SAMPLE_ERR("open /dev/misc failed, errno:%d %s\n", errno, strerror(errno));
        return XMEDIA_FAILURE;
    }
    return ioctl(g_misc_fd, XMEDIA_MISC_AOV_INIT, (xmedia_u32)isp_info, NULL);
}

xmedia_s32 sample_comm_aov_get_misc_fd(xmedia_void)
{
    return g_misc_fd;
}
