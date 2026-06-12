#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#include "sample_comm.h"
#include "sample_comm_sys.h"
#include "sample_comm_vi.h"
#include "sample_comm_vpss.h"
#include "sample_comm_venc.h"
#include "sample_comm_isp.h"

static sample_comm_sensor_type g_sensor_type[MAX_SENSOR_NUM] = {
    SENSOR0_TYPE, SENSOR1_TYPE, SENSOR2_TYPE, SENSOR3_TYPE, SENSOR4_TYPE
};

static int g_inited = 0;
static xmedia_s32 g_venc_chn = 1;
static sample_venc_config g_venc_cfg;
static sample_vi_config g_vi_cfg;
static sample_vpss_config g_vpss_cfg;
static sample_isp_param g_isp_param;
static xmedia_s32 g_vi_pipe = 0;
static xmedia_s32 g_vi_chn = 0;
static xmedia_s32 g_vpss_pipe = 0;
static xmedia_s32 g_vpss_ichn = 0;
static xmedia_s32 g_vpss_ochn[2] = {0, 1};
static xmedia_s32 g_venc_bind_chn[2] = {0, 1};
static pthread_mutex_t g_mjpeg_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_mjpeg_cond = PTHREAD_COND_INITIALIZER;
static volatile int g_stream_ref_count = 0;
static volatile int g_stream_stop_requested = 0;

static void send_all_fd(int fd, const void *data, size_t len)
{
    const char *p = (const char *)data;
    size_t sent = 0;

    while (sent < len) {
        ssize_t ret = send(fd, p + sent, len - sent, 0);
        if (ret <= 0) {
            return;
        }
        sent += (size_t)ret;
    }
}

static xmedia_s32 factory_mjpeg_sys_init(sample_sys_config *sys_config)
{
    xmedia_s32 ret;

    ret = sample_comm_sys_init(sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_sys_init failed\n");
        return ret;
    }

    ret = sample_comm_vi_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vi_init failed\n");
        sample_comm_sys_exit();
        return ret;
    }

    ret = sample_comm_vpss_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vpss_init failed\n");
        sample_comm_vi_exit();
        sample_comm_sys_exit();
        return ret;
    }

    ret = sample_comm_venc_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_venc_init failed\n");
        sample_comm_vpss_exit();
        sample_comm_vi_exit();
        sample_comm_sys_exit();
        return ret;
    }

    return XMEDIA_SUCCESS;
}

static void factory_mjpeg_sys_exit(void)
{
    sample_comm_venc_exit();
    sample_comm_vpss_exit();
    sample_comm_vi_exit();
    sample_comm_sys_exit();
}

static void factory_mjpeg_deinit_pipeline(void)
{
    if (!g_inited) {
        return;
    }

    sample_comm_sys_vpss_unbind_venc(g_vpss_pipe, g_vpss_ochn[1], g_venc_bind_chn[1]);
    sample_comm_sys_vpss_unbind_venc(g_vpss_pipe, g_vpss_ochn[0], g_venc_bind_chn[0]);
    sample_comm_venc_stop(&g_venc_cfg);
    sample_comm_sys_vi_unbind_vpss(g_vi_pipe, g_vi_chn, g_vpss_pipe, g_vpss_ichn);
    sample_comm_vpss_stop(&g_vpss_cfg);
    sample_comm_isp_stop(&g_isp_param);
    sample_comm_vi_stop(&g_vi_cfg);
    sample_comm_isp_exit(&g_isp_param);
    factory_mjpeg_sys_exit();

    memset(&g_venc_cfg, 0, sizeof(g_venc_cfg));
    memset(&g_vi_cfg, 0, sizeof(g_vi_cfg));
    memset(&g_vpss_cfg, 0, sizeof(g_vpss_cfg));
    memset(&g_isp_param, 0, sizeof(g_isp_param));
    g_vi_pipe = 0;
    g_vi_chn = 0;
    g_vpss_pipe = 0;
    g_vpss_ichn = 0;
    g_vpss_ochn[0] = 0;
    g_vpss_ochn[1] = 1;
    g_venc_bind_chn[0] = 0;
    g_venc_bind_chn[1] = 1;
    g_venc_chn = 1;
    g_inited = 0;
}


static xmedia_s32 factory_mjpeg_init_pipeline(void)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;// 修改返回值为XMEDIA_SUCCESS
    xmedia_s32 blk_size;// 定义块大小
    xmedia_video_size pic_size = {0};// 定义主图像大小-VPSS第1路输出
    xmedia_video_size sub_pic_size = {0};// 定义子图像大小-VPSS第2路输出
    xmedia_video_size dcf_pic_size = {0};// 定义DCF图像大小-VPSS第3路输出
    sample_comm_video_param video_param = {0};// 定义视频参数
    sample_sys_config sys_config = {0};// 系统配置

    xmedia_s32 vi_dev = 0;// 定义VI设备号
    xmedia_s32 vi_pipe = 0;// 定义VI管道号
    xmedia_s32 vi_chn = 0;// 定义VI通道号
    sample_vi_config *vi_config = &g_vi_cfg;// 定义VI配置
    xmedia_s32 sensor_type = g_sensor_type[0];// 定义传感器类型
    xmedia_u32 framerate = 0;// 定义帧率
    vi_sensor_info sensor_info = {0};// 定义摄像头信息
    sample_isp_param *isp_param = &g_isp_param;// 定义ISP参数
    xmedia_isp_config isp_config = {0};// 定义ISP配置
    xmedia_bool mirror = XMEDIA_FALSE;// 定义镜像
    xmedia_bool flip = XMEDIA_FALSE;// 定义翻转
    xmedia_s32 vpss_chnl;// 定义VPSS通道号
    xmedia_s32 vpss_pipe = 0;// 定义VPSS管道号
    xmedia_s32 vpss_ichn = 0;// 定义VPSS输入通道号
    xmedia_s32 vpss_ochn[2] = {0, 1};// 定义VPSS输出通道号
    sample_vpss_config *vpss_config = &g_vpss_cfg;// 定义VPSS配置
    xmedia_video_size vpss_ochn_size[VPSS_MAX_OCHN_NUM] = {0};// 定义VPSS输出通道大小
    xmedia_s32 chnl;// 定义VENC通道号
    xmedia_s32 venc_chn[2] = {0, 1};// 定义VENC通道号
    xmedia_bool support_dcf = XMEDIA_TRUE;// 定义支持DCF

    if (g_inited) {// 如果已经初始化，则返回成功
        return XMEDIA_SUCCESS;
    }

    memset(&g_vi_cfg, 0, sizeof(g_vi_cfg));
    memset(&g_vpss_cfg, 0, sizeof(g_vpss_cfg));
    memset(&g_isp_param, 0, sizeof(g_isp_param));

    // 视频参数初始化
    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;// 设置视频存储格式为线性
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;// 设置像素格式为YVU半平面420
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;// 设置数据宽度为8
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;// 设置压缩模式为无压缩

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);// 获取摄像头信息
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);// 获取摄像头帧率

    sys_config.sys_conf.pipe_mode[vi_pipe].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;// 设置VI-VIPROC管道模式为离线
    sys_config.sys_conf.pipe_mode[vi_pipe].viproc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE; // 设置VIPROC-VPSS管道模式为离线
    sys_config.sys_conf.pipe_mode[vi_pipe].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;    // 设置GDC-VPSS管道模式为离线
    sys_config.vb_conf.max_pool_cnt = 25;

    pic_size.width = sensor_info.width;// 设置主图像宽度
    pic_size.height = sensor_info.height;// 设置主图像高度
    blk_size = sample_comm_sys_get_buffer_size(pic_size, video_param.video_fmt, sensor_info.pixel_format,
        sensor_info.bit_width, video_param.compress_mode);// 获取块大小
    sys_config.vb_conf.common_pool[0].block_size = blk_size;
    sys_config.vb_conf.common_pool[0].block_cnt = 2;// 设置块数量为2                    

    blk_size = sample_comm_sys_get_buffer_size(pic_size, video_param.video_fmt, video_param.pixel_fmt, 
        video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[1].block_size = blk_size;
    sys_config.vb_conf.common_pool[1].block_cnt = 5;

    sub_pic_size.width = 640;
    sub_pic_size.height = 480;
    blk_size = sample_comm_sys_get_buffer_size(sub_pic_size, video_param.video_fmt, video_param.pixel_fmt,
        video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[2].block_size = blk_size;
    sys_config.vb_conf.common_pool[2].block_cnt = 3;

    dcf_pic_size.width = 160;
    dcf_pic_size.height = 120;
    blk_size = sample_comm_sys_get_buffer_size(dcf_pic_size, video_param.video_fmt, video_param.pixel_fmt,
        video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[3].block_size = blk_size;
    sys_config.vb_conf.common_pool[3].block_cnt = 1;
    sys_config.vb_conf.supplement_config = 1;

    ret = factory_mjpeg_sys_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    vi_config->dev_info[vi_dev].dev_en = XMEDIA_TRUE;
    vi_config->dev_info[vi_dev].dev_no = vi_dev;
    vi_config->dev_info[vi_dev].sensor_type = sensor_type;
    vi_config->pipe_info[vi_pipe].pipe_en = XMEDIA_TRUE;
    vi_config->pipe_info[vi_pipe].pipe_no = vi_pipe;
    vi_config->pipe_info[vi_pipe].chn_info[vi_chn].chn_en = XMEDIA_TRUE;
    vi_config->pipe_info[vi_pipe].chn_info[vi_chn].chn_no = vi_chn;
    vi_config->dev_bind_pipe[vi_dev].pipe[0] = vi_pipe;
    vi_config->dev_bind_pipe[vi_dev].pipe[1] = -1;

    isp_config.fps = framerate;
    isp_config.mode_config.work_mode = XMEDIA_ISP_WORK_MODE_MASTER;
    isp_config.mode_config.master_mode.blend_stat_enable = XMEDIA_FALSE;
    isp_config.mode_config.master_mode.slave_num = 0;
    isp_config.pixel_fmt = sensor_info.pixel_format;
    isp_config.size.height = sensor_info.height;
    isp_config.size.width = sensor_info.width;
    isp_config.wdr_mode = sensor_info.wdr_mode;

    isp_param->pipe[vi_pipe] = vi_pipe;
    isp_param->isp_info[vi_pipe].isp_pipe_en = XMEDIA_TRUE;
    isp_param->isp_info[vi_pipe].isp_sensor_en = XMEDIA_TRUE;
    isp_param->isp_info[vi_pipe].sensor_type = sensor_type;
    isp_param->isp_info[vi_pipe].mirror = mirror;
    isp_param->isp_info[vi_pipe].flip = flip;
    memcpy(&(isp_param->isp_info[vi_pipe].isp_config), &isp_config, sizeof(xmedia_isp_config));

    ret = sample_comm_isp_init(isp_param, vi_config);
    if (ret != XMEDIA_SUCCESS) goto exit0;
    ret = sample_comm_vi_start(vi_config, &video_param);
    if (ret != XMEDIA_SUCCESS) goto exit1;
    ret = sample_comm_isp_start(isp_param);
    if (ret != XMEDIA_SUCCESS) goto exit2;

    ret = sample_comm_vpss_get_default_pipe_cfg(&vpss_config->pipe_info[vpss_pipe].pipe_config, pic_size, &video_param);
    if (ret != XMEDIA_SUCCESS) goto exit2;

    vpss_ochn_size[0].width = sensor_info.width;
    vpss_ochn_size[0].height = sensor_info.height;
    vpss_ochn_size[1].width = 640;
    vpss_ochn_size[1].height = 480;
    vpss_config->pipe_info[vpss_pipe].pipe_en = XMEDIA_TRUE;
    vpss_config->pipe_info[vpss_pipe].pipe_no = vpss_pipe;

    vpss_chnl = vpss_ochn[0];
    vpss_config->pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
    vpss_config->pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_no = vpss_ochn[0];
    ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config->pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
        vpss_ochn_size[vpss_chnl], &video_param);
    if (ret != XMEDIA_SUCCESS) goto exit2;
    vpss_config->pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.depth = 0;

    vpss_chnl = vpss_ochn[1];
    vpss_config->pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
    vpss_config->pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_no = vpss_ochn[1];
    ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config->pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
        vpss_ochn_size[vpss_chnl], &video_param);
    if (ret != XMEDIA_SUCCESS) goto exit2;
    vpss_config->pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.depth = 0;

    ret = sample_comm_vpss_start(vpss_config);
    if (ret != XMEDIA_SUCCESS) goto exit2;
    ret = sample_comm_sys_vi_bind_vpss(vi_pipe, vi_chn, vpss_pipe, vpss_ichn);
    if (ret != XMEDIA_SUCCESS) goto exit3;

    memset(&g_venc_cfg, 0, sizeof(g_venc_cfg));
    chnl = venc_chn[0];
    g_venc_cfg.chn_info[chnl].venc_en = XMEDIA_TRUE;
    g_venc_cfg.chn_info[chnl].venc_chn = chnl;
    g_venc_cfg.chn_info[chnl].payload_type = PT_JPEG;
    g_venc_cfg.chn_info[chnl].support_dcf = support_dcf;
    g_venc_cfg.chn_info[chnl].mpf_cnt = 2;
    sample_comm_venc_get_default_chn_info(vpss_ochn_size[0], framerate, &g_venc_cfg.chn_info[chnl]);

    chnl = venc_chn[1];
    g_venc_cfg.chn_info[chnl].venc_en = XMEDIA_TRUE;
    g_venc_cfg.chn_info[chnl].venc_chn = chnl;
    g_venc_cfg.chn_info[chnl].payload_type = PT_MJPEG;
    g_venc_cfg.chn_info[chnl].rc_mode = VENC_RC_MODE_MJPEGCBR;
    sample_comm_venc_get_default_chn_info(vpss_ochn_size[1], framerate, &g_venc_cfg.chn_info[chnl]);

    ret = sample_comm_venc_start(&g_venc_cfg);
    if (ret != XMEDIA_SUCCESS) goto exit4;
    ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
    if (ret != XMEDIA_SUCCESS) goto exit5;
    ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
    if (ret != XMEDIA_SUCCESS) goto exit6;
    ret = sample_comm_venc_set_jpeg_exif(venc_chn[0]);
    if (ret != XMEDIA_SUCCESS) goto exit7;

    g_vi_pipe = vi_pipe;
    g_vi_chn = vi_chn;
    g_vpss_pipe = vpss_pipe;
    g_vpss_ichn = vpss_ichn;
    g_vpss_ochn[0] = vpss_ochn[0];
    g_vpss_ochn[1] = vpss_ochn[1];
    g_venc_bind_chn[0] = venc_chn[0];
    g_venc_bind_chn[1] = venc_chn[1];
    g_venc_chn = venc_chn[1];
    g_stream_stop_requested = 0;
    g_inited = 1;
    return XMEDIA_SUCCESS;

exit7:
    sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[1], venc_chn[1]);
exit6:
    sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
exit5:
    sample_comm_venc_stop(&g_venc_cfg);
exit4:
    sample_comm_sys_vi_unbind_vpss(vi_pipe, vi_chn, vpss_pipe, vpss_ichn);
exit3:
    sample_comm_vpss_stop(vpss_config);
exit2:
    sample_comm_isp_stop(isp_param);
exit1:
    sample_comm_vi_stop(vi_config);
exit0:
    sample_comm_isp_exit(isp_param);
    factory_mjpeg_sys_exit();
    memset(&g_vi_cfg, 0, sizeof(g_vi_cfg));
    memset(&g_vpss_cfg, 0, sizeof(g_vpss_cfg));
    memset(&g_isp_param, 0, sizeof(g_isp_param));
    memset(&g_venc_cfg, 0, sizeof(g_venc_cfg));
    return ret;
}

void factory_mjpeg_request_stop(void)
{
    pthread_mutex_lock(&g_mjpeg_lock);
    g_stream_stop_requested = 1;
    pthread_cond_broadcast(&g_mjpeg_cond);
    pthread_mutex_unlock(&g_mjpeg_lock);
}

void factory_mjpeg_shutdown(void)
{
    pthread_mutex_lock(&g_mjpeg_lock);
    g_stream_stop_requested = 1;
    while (g_stream_ref_count > 0) {
        pthread_cond_wait(&g_mjpeg_cond, &g_mjpeg_lock);
    }
    factory_mjpeg_deinit_pipeline();
    g_stream_stop_requested = 0;
    pthread_mutex_unlock(&g_mjpeg_lock);
}

void factory_mjpeg_send_stream(int client_fd)
{
    char header[256];
    xmedia_s32 ret;
    xmedia_u32 venc_mask;
    struct timeval timeout_val;

    pthread_mutex_lock(&g_mjpeg_lock);
    ret = factory_mjpeg_init_pipeline();
    if (ret == XMEDIA_SUCCESS) {
        g_stream_stop_requested = 0;
        ++g_stream_ref_count;
    }
    pthread_mutex_unlock(&g_mjpeg_lock);

    if (ret != XMEDIA_SUCCESS) {
        snprintf(header, sizeof(header),
            "HTTP/1.1 500 Internal Server Error\r\n"
            "Content-Type: text/plain; charset=utf-8\r\n"
            "Connection: close\r\n\r\n"
            "mjpeg pipeline init failed: %#x\n", ret);
        send_all_fd(client_fd, header, strlen(header));
        return;
    }

    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n"
        "Cache-Control: no-store\r\n"
        "Pragma: no-cache\r\n"
        "Connection: close\r\n\r\n");
    send_all_fd(client_fd, header, strlen(header));

    venc_mask = 1U << g_venc_chn;
    while (!g_stream_stop_requested) {
        xmedia_venc_chn_status stat;
        xmedia_venc_stream stream;
        size_t frame_len = 0;
        xmedia_u32 i;

        timeout_val.tv_sec = 2;
        timeout_val.tv_usec = 0;
        ret = xmedia_venc_select(venc_mask, &timeout_val);
        if (ret == XMEDIA_ERRCODE_TIMEOUT) {
            continue;
        }
        if (ret != XMEDIA_SUCCESS) {
            break;
        }

        memset(&stat, 0, sizeof(stat));
        ret = xmedia_venc_query_status(g_venc_chn, &stat);
        if (ret != XMEDIA_SUCCESS || stat.cur_packs == 0) {
            continue;
        }

        memset(&stream, 0, sizeof(stream));
        stream.pack = (xmedia_venc_pack *)malloc(sizeof(xmedia_venc_pack) * stat.cur_packs);
        if (stream.pack == NULL) {
            break;
        }
        stream.pack_count = stat.cur_packs;

        ret = xmedia_venc_get_stream(g_venc_chn, &stream, -1);
        if (ret != XMEDIA_SUCCESS) {
            free(stream.pack);
            break;
        }

        for (i = 0; i < stream.pack_count; ++i) {
            frame_len += stream.pack[i].len - stream.pack[i].offset;
        }

        snprintf(header, sizeof(header),
            "--frame\r\n"
            "Content-Type: image/jpeg\r\n"
            "Content-Length: %zu\r\n\r\n", frame_len);
        send_all_fd(client_fd, header, strlen(header));

        for (i = 0; i < stream.pack_count; ++i) {
            send_all_fd(client_fd,
                stream.pack[i].vir_addr + stream.pack[i].offset,
                stream.pack[i].len - stream.pack[i].offset);
        }
        send_all_fd(client_fd, "\r\n", 2);

        ret = xmedia_venc_release_stream(g_venc_chn, &stream);
        free(stream.pack);
        if (ret != XMEDIA_SUCCESS) {
            break;
        }
    }

    pthread_mutex_lock(&g_mjpeg_lock);
    if (g_stream_ref_count > 0) {
        --g_stream_ref_count;
    }
    if (g_stream_ref_count == 0) {
        factory_mjpeg_deinit_pipeline();
        pthread_cond_broadcast(&g_mjpeg_cond);
    }
    pthread_mutex_unlock(&g_mjpeg_lock);
}
