#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include "sample_comm.h"
#include "sample_comm_sys.h"
#include "sample_comm_vi.h"
#include "sample_comm_vpss.h"
#include "sample_comm_venc.h"
#include "sample_comm_isp.h"
#include "web_server.h"

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

static volatile int g_motion_count = 0;
static volatile int g_motion_threshold = 20;
static volatile unsigned long g_last_frame_size = 0;
static volatile unsigned long g_last_frame_checksum = 0;
static volatile time_t g_last_frame_time = 0;
static volatile time_t g_last_motion_time = 0;
static int g_have_motion_baseline = 0;
static unsigned long g_motion_baseline_size = 0;
static unsigned int g_motion_warmup_frames = 0;
static unsigned int g_motion_consecutive_frames = 0;
static unsigned char *g_snapshot_buf = NULL;
static size_t g_snapshot_size = 0;
static time_t g_snapshot_mtime = 0;
static pthread_mutex_t g_snapshot_lock = PTHREAD_MUTEX_INITIALIZER;

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

static unsigned long frame_checksum(const xmedia_venc_stream *stream)
{
    unsigned long hash = 5381;
    xmedia_u32 i;

    for (i = 0; i < stream->pack_count; ++i) {
        unsigned char *p = (unsigned char *)stream->pack[i].vir_addr + stream->pack[i].offset;
        size_t len = stream->pack[i].len - stream->pack[i].offset;
        size_t step = (len / 256) + 1;
        size_t pos;

        for (pos = 0; pos < len; pos += step) {
            hash = ((hash << 5) + hash) ^ p[pos];
        }
    }

    return hash;
}

static unsigned long abs_size_delta(unsigned long a, unsigned long b)
{
    return (a > b) ? (a - b) : (b - a);
}

static void reset_motion_baseline(void)
{
    g_have_motion_baseline = 0;
    g_motion_baseline_size = 0;
    g_motion_warmup_frames = 0;
    g_motion_consecutive_frames = 0;
}

static void update_snapshot_cache(const xmedia_venc_stream *stream, size_t frame_len)
{
    unsigned char *buf = NULL;
    xmedia_u32 i;
    size_t offset = 0;

    buf = (unsigned char *)malloc(frame_len);
    if (buf == NULL) {
        return;
    }

    for (i = 0; i < stream->pack_count; ++i) {
        size_t len = stream->pack[i].len - stream->pack[i].offset;
        memcpy(buf + offset, stream->pack[i].vir_addr + stream->pack[i].offset, len);
        offset += len;
    }

    pthread_mutex_lock(&g_snapshot_lock);
    free(g_snapshot_buf);
    g_snapshot_buf = buf;
    g_snapshot_size = frame_len;
    g_snapshot_mtime = time(NULL);
    pthread_mutex_unlock(&g_snapshot_lock);
}

static void update_motion_stats(const xmedia_venc_stream *stream, size_t frame_len)
{
    unsigned long checksum = frame_checksum(stream);
    unsigned long current_size = (unsigned long)frame_len;
    unsigned long prev_size = g_last_frame_size;
    unsigned long jitter_delta;
    unsigned long baseline_delta;
    unsigned long trigger_delta;
    time_t now = time(NULL);

    g_last_frame_size = current_size;
    g_last_frame_checksum = checksum;
    g_last_frame_time = now;

    if (!g_have_motion_baseline) {
        g_motion_baseline_size = current_size;
        g_have_motion_baseline = 1;
        return;
    }

    if (g_motion_warmup_frames < 30) {
        g_motion_baseline_size = (g_motion_baseline_size * 7 + current_size) / 8;
        ++g_motion_warmup_frames;
        return;
    }

    /*
     * MJPEG/JPEG bytes are not stable frame-to-frame even when the scene is still:
     * encoder rate control, sensor noise and ISP auto exposure can change both the
     * compressed size and sampled bytes.  The old checksum based score therefore
     * reported motion on a static image.  Use frame size as a coarse signal, ignore
     * normal encoder jitter, and require several consecutive abnormal frames before
     * counting one motion event.
     */
    jitter_delta = abs_size_delta(current_size, prev_size);
    baseline_delta = abs_size_delta(current_size, g_motion_baseline_size);
    trigger_delta = (unsigned long)g_motion_threshold * 128UL;
    if (trigger_delta < 4096UL) {
        trigger_delta = 4096UL;
    }

    if (baseline_delta > trigger_delta && jitter_delta > (trigger_delta / 4UL)) {
        ++g_motion_consecutive_frames;
    } else {
        g_motion_consecutive_frames = 0;
        g_motion_baseline_size = (g_motion_baseline_size * 31 + current_size) / 32;
    }

    if (g_motion_consecutive_frames >= 3) {
        ++g_motion_count;
        g_last_motion_time = now;
        g_motion_consecutive_frames = 0;
        g_motion_baseline_size = current_size;
    }
}

void detector_get_status(int *motion_count, int *motion_threshold, unsigned long *last_frame_size,
    time_t *last_frame_time, time_t *last_motion_time)
{
    if (motion_count != NULL) *motion_count = g_motion_count;
    if (motion_threshold != NULL) *motion_threshold = g_motion_threshold;
    if (last_frame_size != NULL) *last_frame_size = g_last_frame_size;
    if (last_frame_time != NULL) *last_frame_time = g_last_frame_time;
    if (last_motion_time != NULL) *last_motion_time = g_last_motion_time;
}

void detector_set_motion_threshold(int threshold)
{
    if (threshold < 1) threshold = 1;
    if (threshold > 80) threshold = 80;
    g_motion_threshold = threshold;
    reset_motion_baseline();
}

void detector_record_browser_motion(void)
{
    ++g_motion_count;
    g_last_motion_time = time(NULL);
}

void detector_reset_motion_count(void)
{
    g_motion_count = 0;
    g_last_motion_time = 0;
}

static xmedia_s32 rpi_detector_mjpeg_sys_init(sample_sys_config *sys_config)
{
    xmedia_s32 ret;

    ret = sample_comm_sys_init(sys_config);
    if (ret != XMEDIA_SUCCESS) return ret;

    ret = sample_comm_vi_init();
    if (ret != XMEDIA_SUCCESS) { sample_comm_sys_exit(); return ret; }

    ret = sample_comm_vpss_init();
    if (ret != XMEDIA_SUCCESS) { sample_comm_vi_exit(); sample_comm_sys_exit(); return ret; }

    ret = sample_comm_venc_init();
    if (ret != XMEDIA_SUCCESS) { sample_comm_vpss_exit(); sample_comm_vi_exit(); sample_comm_sys_exit(); return ret; }

    return XMEDIA_SUCCESS;
}

static void rpi_detector_mjpeg_sys_exit(void)
{
    sample_comm_venc_exit();
    sample_comm_vpss_exit();
    sample_comm_vi_exit();
    sample_comm_sys_exit();
}

static void rpi_detector_mjpeg_deinit_pipeline(void)
{
    if (!g_inited) return;

    sample_comm_sys_vpss_unbind_venc(g_vpss_pipe, g_vpss_ochn[1], g_venc_bind_chn[1]);
    sample_comm_sys_vpss_unbind_venc(g_vpss_pipe, g_vpss_ochn[0], g_venc_bind_chn[0]);
    sample_comm_venc_stop(&g_venc_cfg);
    sample_comm_sys_vi_unbind_vpss(g_vi_pipe, g_vi_chn, g_vpss_pipe, g_vpss_ichn);
    sample_comm_vpss_stop(&g_vpss_cfg);
    sample_comm_isp_stop(&g_isp_param);
    sample_comm_vi_stop(&g_vi_cfg);
    sample_comm_isp_exit(&g_isp_param);
    rpi_detector_mjpeg_sys_exit();

    memset(&g_venc_cfg, 0, sizeof(g_venc_cfg));
    memset(&g_vi_cfg, 0, sizeof(g_vi_cfg));
    memset(&g_vpss_cfg, 0, sizeof(g_vpss_cfg));
    memset(&g_isp_param, 0, sizeof(g_isp_param));
    g_inited = 0;
}

static xmedia_s32 rpi_detector_mjpeg_init_pipeline(void)
{
    xmedia_s32 ret;
    xmedia_s32 blk_size;
    xmedia_video_size pic_size = {0};
    xmedia_video_size sub_pic_size = {0};
    sample_comm_video_param video_param = {0};
    sample_sys_config sys_config = {0};
    sample_vi_config *vi_config = &g_vi_cfg;
    xmedia_s32 sensor_type = g_sensor_type[0];
    xmedia_u32 framerate = 0;
    vi_sensor_info sensor_info = {0};
    sample_isp_param *isp_param = &g_isp_param;
    xmedia_isp_config isp_config = {0};
    sample_vpss_config *vpss_config = &g_vpss_cfg;
    xmedia_video_size vpss_ochn_size[VPSS_MAX_OCHN_NUM] = {0};
    xmedia_s32 venc_chn[2] = {0, 1};

    if (g_inited) return XMEDIA_SUCCESS;

    memset(&g_vi_cfg, 0, sizeof(g_vi_cfg));
    memset(&g_vpss_cfg, 0, sizeof(g_vpss_cfg));
    memset(&g_isp_param, 0, sizeof(g_isp_param));

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);

    sys_config.sys_conf.pipe_mode[0].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[0].viproc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[0].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.vb_conf.max_pool_cnt = 25;

    pic_size.width = sensor_info.width;
    pic_size.height = sensor_info.height;
    blk_size = sample_comm_sys_get_buffer_size(pic_size, video_param.video_fmt, sensor_info.pixel_format,
        sensor_info.bit_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[0].block_size = blk_size;
    sys_config.vb_conf.common_pool[0].block_cnt = 2;

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
    sys_config.vb_conf.supplement_config = 1;

    ret = rpi_detector_mjpeg_sys_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) return ret;

    vi_config->dev_info[0].dev_en = XMEDIA_TRUE;
    vi_config->dev_info[0].dev_no = 0;
    vi_config->dev_info[0].sensor_type = sensor_type;
    vi_config->pipe_info[0].pipe_en = XMEDIA_TRUE;
    vi_config->pipe_info[0].pipe_no = 0;
    vi_config->pipe_info[0].chn_info[0].chn_en = XMEDIA_TRUE;
    vi_config->pipe_info[0].chn_info[0].chn_no = 0;
    vi_config->dev_bind_pipe[0].pipe[0] = 0;
    vi_config->dev_bind_pipe[0].pipe[1] = -1;

    isp_config.fps = framerate;
    isp_config.mode_config.work_mode = XMEDIA_ISP_WORK_MODE_MASTER;
    isp_config.mode_config.master_mode.blend_stat_enable = XMEDIA_FALSE;
    isp_config.mode_config.master_mode.slave_num = 0;
    isp_config.pixel_fmt = sensor_info.pixel_format;
    isp_config.size.height = sensor_info.height;
    isp_config.size.width = sensor_info.width;
    isp_config.wdr_mode = sensor_info.wdr_mode;

    isp_param->pipe[0] = 0;
    isp_param->isp_info[0].isp_pipe_en = XMEDIA_TRUE;
    isp_param->isp_info[0].isp_sensor_en = XMEDIA_TRUE;
    isp_param->isp_info[0].sensor_type = sensor_type;
    memcpy(&(isp_param->isp_info[0].isp_config), &isp_config, sizeof(xmedia_isp_config));

    ret = sample_comm_isp_init(isp_param, vi_config); if (ret != XMEDIA_SUCCESS) goto exit0;
    ret = sample_comm_vi_start(vi_config, &video_param); if (ret != XMEDIA_SUCCESS) goto exit1;
    ret = sample_comm_isp_start(isp_param); if (ret != XMEDIA_SUCCESS) goto exit2;
    ret = sample_comm_vpss_get_default_pipe_cfg(&vpss_config->pipe_info[0].pipe_config, pic_size, &video_param); if (ret != XMEDIA_SUCCESS) goto exit2;

    vpss_ochn_size[0].width = sensor_info.width;
    vpss_ochn_size[0].height = sensor_info.height;
    vpss_ochn_size[1].width = 640;
    vpss_ochn_size[1].height = 480;
    vpss_config->pipe_info[0].pipe_en = XMEDIA_TRUE;
    vpss_config->pipe_info[0].pipe_no = 0;
    for (int ch = 0; ch < 2; ++ch) {
        vpss_config->pipe_info[0].chn_info[ch].chn_en = XMEDIA_TRUE;
        vpss_config->pipe_info[0].chn_info[ch].chn_no = ch;
        ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config->pipe_info[0].chn_info[ch].chn_config,
            vpss_ochn_size[ch], &video_param);
        if (ret != XMEDIA_SUCCESS) goto exit2;
        vpss_config->pipe_info[0].chn_info[ch].chn_config.depth = 0;
    }

    ret = sample_comm_vpss_start(vpss_config); if (ret != XMEDIA_SUCCESS) goto exit2;
    ret = sample_comm_sys_vi_bind_vpss(0, 0, 0, 0); if (ret != XMEDIA_SUCCESS) goto exit3;

    memset(&g_venc_cfg, 0, sizeof(g_venc_cfg));
    g_venc_cfg.chn_info[0].venc_en = XMEDIA_TRUE;
    g_venc_cfg.chn_info[0].venc_chn = 0;
    g_venc_cfg.chn_info[0].payload_type = PT_JPEG;
    g_venc_cfg.chn_info[0].support_dcf = XMEDIA_TRUE;
    g_venc_cfg.chn_info[0].mpf_cnt = 2;
    sample_comm_venc_get_default_chn_info(vpss_ochn_size[0], framerate, &g_venc_cfg.chn_info[0]);

    g_venc_cfg.chn_info[1].venc_en = XMEDIA_TRUE;
    g_venc_cfg.chn_info[1].venc_chn = 1;
    g_venc_cfg.chn_info[1].payload_type = PT_MJPEG;
    g_venc_cfg.chn_info[1].rc_mode = VENC_RC_MODE_MJPEGCBR;
    sample_comm_venc_get_default_chn_info(vpss_ochn_size[1], framerate, &g_venc_cfg.chn_info[1]);

    ret = sample_comm_venc_start(&g_venc_cfg); if (ret != XMEDIA_SUCCESS) goto exit4;
    ret = sample_comm_sys_vpss_bind_venc(0, 0, venc_chn[0]); if (ret != XMEDIA_SUCCESS) goto exit5;
    ret = sample_comm_sys_vpss_bind_venc(0, 1, venc_chn[1]); if (ret != XMEDIA_SUCCESS) goto exit6;
    ret = sample_comm_venc_set_jpeg_exif(venc_chn[0]); if (ret != XMEDIA_SUCCESS) goto exit7;

    g_venc_chn = 1;
    g_stream_stop_requested = 0;
    reset_motion_baseline();
    g_inited = 1;
    return XMEDIA_SUCCESS;

exit7:
    sample_comm_sys_vpss_unbind_venc(0, 1, venc_chn[1]);
exit6:
    sample_comm_sys_vpss_unbind_venc(0, 0, venc_chn[0]);
exit5:
    sample_comm_venc_stop(&g_venc_cfg);
exit4:
    sample_comm_sys_vi_unbind_vpss(0, 0, 0, 0);
exit3:
    sample_comm_vpss_stop(vpss_config);
exit2:
    sample_comm_isp_stop(isp_param);
exit1:
    sample_comm_vi_stop(vi_config);
exit0:
    sample_comm_isp_exit(isp_param);
    rpi_detector_mjpeg_sys_exit();
    return ret;
}

void rpi_detector_mjpeg_request_stop(void)
{
    pthread_mutex_lock(&g_mjpeg_lock);
    g_stream_stop_requested = 1;
    pthread_cond_broadcast(&g_mjpeg_cond);
    pthread_mutex_unlock(&g_mjpeg_lock);
}

void rpi_detector_mjpeg_shutdown(void)
{
    pthread_mutex_lock(&g_mjpeg_lock);
    g_stream_stop_requested = 1;
    while (g_stream_ref_count > 0) {
        pthread_cond_wait(&g_mjpeg_cond, &g_mjpeg_lock);
    }
    rpi_detector_mjpeg_deinit_pipeline();
    g_stream_stop_requested = 0;
    pthread_mutex_unlock(&g_mjpeg_lock);
}

static void send_snapshot_response(int client_fd)
{
    unsigned char *buf = NULL;
    size_t size = 0;
    time_t mtime = 0;

    pthread_mutex_lock(&g_snapshot_lock);
    if (g_snapshot_buf != NULL && g_snapshot_size > 0) {
        buf = (unsigned char *)malloc(g_snapshot_size);
        if (buf != NULL) {
            memcpy(buf, g_snapshot_buf, g_snapshot_size);
            size = g_snapshot_size;
            mtime = g_snapshot_mtime;
        }
    }
    pthread_mutex_unlock(&g_snapshot_lock);

    if (buf == NULL || size == 0) {
        web_send_response(client_fd, "404 Not Found", "text/plain; charset=utf-8", "no snapshot available\n");
        free(buf);
        return;
    }

    web_send_header(client_fd, "200 OK", "image/jpeg", size);
    web_send_all(client_fd, (const char *)buf, size);
    free(buf);
    (void)mtime;
}

void rpi_detector_mjpeg_send_stream(int client_fd)
{
    char header[256];
    xmedia_s32 ret;
    xmedia_u32 venc_mask;
    struct timeval timeout_val;

    pthread_mutex_lock(&g_mjpeg_lock);
    ret = rpi_detector_mjpeg_init_pipeline();
    if (ret == XMEDIA_SUCCESS) {
        g_stream_stop_requested = 0;
        ++g_stream_ref_count;
    }
    pthread_mutex_unlock(&g_mjpeg_lock);

    if (ret != XMEDIA_SUCCESS) {
        snprintf(header, sizeof(header),
            "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain; charset=utf-8\r\nConnection: close\r\n\r\nmjpeg pipeline init failed: %#x\n", ret);
        send_all_fd(client_fd, header, strlen(header));
        return;
    }

    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\nContent-Type: multipart/x-mixed-replace; boundary=frame\r\nCache-Control: no-store\r\nPragma: no-cache\r\nConnection: close\r\n\r\n");
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
        if (ret == XMEDIA_ERRCODE_TIMEOUT) continue;
        if (ret != XMEDIA_SUCCESS) break;

        memset(&stat, 0, sizeof(stat));
        ret = xmedia_venc_query_status(g_venc_chn, &stat);
        if (ret != XMEDIA_SUCCESS || stat.cur_packs == 0) continue;

        memset(&stream, 0, sizeof(stream));
        stream.pack = (xmedia_venc_pack *)malloc(sizeof(xmedia_venc_pack) * stat.cur_packs);
        if (stream.pack == NULL) break;
        stream.pack_count = stat.cur_packs;

        ret = xmedia_venc_get_stream(g_venc_chn, &stream, -1);
        if (ret != XMEDIA_SUCCESS) { free(stream.pack); break; }

        for (i = 0; i < stream.pack_count; ++i) {
            frame_len += stream.pack[i].len - stream.pack[i].offset;
        }
        update_motion_stats(&stream, frame_len);
        update_snapshot_cache(&stream, frame_len);

        snprintf(header, sizeof(header), "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %zu\r\n\r\n", frame_len);
        send_all_fd(client_fd, header, strlen(header));
        for (i = 0; i < stream.pack_count; ++i) {
            send_all_fd(client_fd, stream.pack[i].vir_addr + stream.pack[i].offset,
                stream.pack[i].len - stream.pack[i].offset);
        }
        send_all_fd(client_fd, "\r\n", 2);

        ret = xmedia_venc_release_stream(g_venc_chn, &stream);
        free(stream.pack);
        if (ret != XMEDIA_SUCCESS) break;
    }

    pthread_mutex_lock(&g_mjpeg_lock);
    if (g_stream_ref_count > 0) --g_stream_ref_count;
    if (g_stream_ref_count == 0) {
        rpi_detector_mjpeg_deinit_pipeline();
        pthread_cond_broadcast(&g_mjpeg_cond);
    }
    pthread_mutex_unlock(&g_mjpeg_lock);
}

void rpi_detector_mjpeg_send_snapshot(int client_fd)
{
    send_snapshot_response(client_fd);
}
