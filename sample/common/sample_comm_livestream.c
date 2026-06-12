#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

#include "sample_comm_livestream.h"
#include "sample_comm_venc.h"

#define APP_LISTEN_PORT  554
#define APP_MAX_CONN_NUM 4
#define APP_PATTERN_NAME "livestream"
static xmedia_char g_stream_name[4][8] = {"0", "1", "2", "3"};
static xmedia_u8 g_stream_cnt = 0;
static xmedia_char server_ip_addr[20];

static int app_liveserver_get_ip(const char *adapter_name,  char *addr)
{
    struct ifreq ifr;
    const char *ifname = adapter_name;
    int skfd;
    struct sockaddr_in *ipad;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(skfd < 0)
    {
        printf("socket create error\n");
        return -1;
    }
    memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_name, ifname);
    ifr.ifr_addr.sa_family = AF_INET;

    if (ioctl(skfd, SIOCGIFADDR, &ifr) == 0)
    {
        ipad = (struct sockaddr_in *)&ifr.ifr_addr;
        strcpy(addr,inet_ntoa(ipad->sin_addr));
    }
    else
    {
        close(skfd);
        //printf("error!!:%s,device:%s\n",strerror(errno),adapter_name);
        return -1;
    }

    close(skfd);
    return 0;

}

xmedia_s32 app_liveserver_init(xmedia_void)
{
    xcam_liveserver_init_param init_param;

    (xmedia_void)xcam_log_init();

    memset(server_ip_addr, 0, sizeof(server_ip_addr));
    if(app_liveserver_get_ip("eth0", server_ip_addr) != 0) {
        printf("rtsp get host ip fail\n");
        return XMEDIA_FAILURE;
    }

    memset(&init_param, 0, sizeof(init_param));
    init_param.listen_port = APP_LISTEN_PORT;
    init_param.max_conn_num = APP_MAX_CONN_NUM;
    strncpy(init_param.pattern_name, APP_PATTERN_NAME, 16);
    xcam_s32 ret = xcam_liveserver_init(init_param);
    if (ret != XMEDIA_SUCCESS) {
        printf("liveserver init error, ret=%#x \n", ret);
        return XMEDIA_FAILURE;
    }
    return XMEDIA_SUCCESS;
}

xmedia_void app_liveserver_deinit(xmedia_void)
{
    xcam_liveserver_deinit();
    xcam_log_deinit();
}

xmedia_s32 app_liveserver_get_video_src_info(int index, xcam_track_video_source_info *video_src_info,
                                     xmedia_video_size *chn_size,xmedia_u32 frame_rate)
{
    video_src_info->codec_type = XCAM_TRACK_VIDEO_CODEC_H265;
    video_src_info->width = chn_size[index].width;
    video_src_info->height = chn_size[index].height;
    video_src_info->speed = 1;
    video_src_info->frame_rate = frame_rate;
    video_src_info->gop = frame_rate * 2;
    video_src_info->bit_rate = 20 * 1024 * 1024;

    return XMEDIA_SUCCESS;
}

xmedia_s32 app_liveserver_video_chn_request_key_frame(xcam_handle venc_handle, xmedia_void* param)
{
    xmedia_s32 venc_chn;
    xmedia_s32 ret;

    venc_chn = (xmedia_s32)venc_handle;
    ret = xmedia_venc_request_idr(venc_chn, XMEDIA_TRUE);
    if (XMEDIA_SUCCESS != ret) {
        printf("xmedia_venc_request_idr(chn:%d) faild with%#x!\n", venc_chn, ret);
        return XMEDIA_FAILURE;
    } else {
        printf("xmedia_venc_request_idr(chn:%d) successful!\n", venc_chn);
    }

    return ret;
}

xmedia_s32 app_liveserver_start(xmedia_video_size *chn_size,xmedia_u32 chn_cnt,xmedia_u32 frame_rate)
{
    g_stream_cnt = chn_cnt;
    for (int i = 0; i < chn_cnt; i++) {
        xcam_track_video_source_info track_video;
        app_liveserver_get_video_src_info(i, &track_video,chn_size,frame_rate);
        int ret = xcam_liveserver_add_stream(i, &track_video,
                -1, 0, (const xcam_char *)g_stream_name[i], app_liveserver_video_chn_request_key_frame);
        if (ret != XMEDIA_SUCCESS) {
            printf("liveserver add stream venc_handle[%u] failed! \n\n",i);
            return -1;
        }
        printf("\nrtsp://%s:%d/%s/%d",server_ip_addr, APP_LISTEN_PORT,APP_PATTERN_NAME,i);
    }
    printf("\n");
    return XMEDIA_SUCCESS;
}

xmedia_s32 app_liveserver_stop(xmedia_void)
{
    for (int i = 0; i < g_stream_cnt; i++) {
        int ret = xcam_liveserver_remove_stream((const xcam_char *)g_stream_name[i]);
        if (ret != XMEDIA_SUCCESS) {
            printf("liveserver remove stream venc_handle[%u] failed! \n\n",i);
            return -1;
        }
    }

    return XMEDIA_SUCCESS;
}


