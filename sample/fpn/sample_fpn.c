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
#include "sample_comm_isp.h"

static sample_comm_sensor_type g_sample_fpn_sensor_type[MAX_SENSOR_NUM] = { SENSOR0_TYPE, SENSOR1_TYPE, SENSOR2_TYPE,
                                                                            SENSOR3_TYPE, SENSOR4_TYPE };

static xmedia_bool g_force_exit = XMEDIA_FALSE;

xmedia_void sample_fpn_usage(xmedia_char *args)
{
    printf("\n"
           "***************************************************************\n"
           "Usage: %s [work_mode] [stream_mode]\n"
           "work_mode: \n"
           "    0: vi offline fpn calibrate\n"
           "e.g: %s 0\n"
           "***************************************************************\n"
           "\n",
           args, args);
    return;
}

xmedia_s32 sample_fpn_sys_init(sample_sys_config *sys_config)
{
    xmedia_s32 ret = 0;

    ret = sample_comm_sys_init(sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_sys_init failed!\n");
        return ret;
    }

    // in online-online mode,vi and vpss must be reset at the same time
    ret = sample_comm_vi_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vi_init failed!\n");
        return ret;
    }

    return XMEDIA_SUCCESS;
}

xmedia_void sample_fpn_sys_exit(void)
{
    sample_comm_vi_exit();
    sample_comm_sys_exit();
}

xmedia_void sample_fpn_handle_sig(xmedia_s32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo || SIGTERM == signo) {
        g_force_exit = XMEDIA_TRUE;
    }
}

xmedia_s32 sample_fpn_calibrate(xmedia_s32 pipe, xmedia_u32 width, xmedia_u32 height)
{
    xmedia_char   string[8]       = { 0 };
    xmedia_u16     calibrate_frame = 256;
    xmedia_handle vb_block        = VB_INVALID_HANDLE;
    xmedia_s8    *vb_vir_addr     = { XMEDIA_NULL };
    xmedia_u64    phy_addr;
    FILE         *file = XMEDIA_NULL;
    xmedia_u64    start, end;
    xmedia_s32    ret;
    xmedia_u32    blk_size = 0;

    xmedia_isp_fpn_calibrate_attr fpn_calibrate_attr = { 0 };
    xmedia_isp_blc_attr           blc_attr           = { 0 };

    fflush(stdin);
    SAMPLE_PRT("input number to set calibrate frame and press enter to confirm!\n");
    SAMPLE_PRT("calibrate frame: \n");
    fgets(string, sizeof string, stdin);
    calibrate_frame = (xmedia_u16)atoi(string);
    fflush(stdin);

    while (calibrate_frame > 256 || calibrate_frame < 1) {
        SAMPLE_PRT("calibrate frame supported range is [1, 256] and power of 2. please input number again!\n");
        fgets(string, sizeof string, stdin);
        calibrate_frame = (xmedia_u16)atoi(string);
        fflush(stdin);
    }

    SAMPLE_PRT("press enter to start fpn calibrate.\n");
    getchar();

    xmedia_sys_get_cur_pts(&start);

    blk_size    = (width * 8 + 63) / 64 * 8 * height;
    vb_block = xmedia_vb_get_block(VB_INVALID_POOLID, blk_size, XMEDIA_NULL);
    if (vb_block == VB_INVALID_HANDLE) {
        SAMPLE_PRT("get block failed!\n");
        goto exit;
    }

    phy_addr = xmedia_vb_handle_to_phy_addr(vb_block);
    if (phy_addr == 0) {
        SAMPLE_PRT("get phy_addr failed!\n");
        goto exit;
    }

    vb_vir_addr = (xmedia_s8 *)xmedia_mmz_map(phy_addr, blk_size, XMEDIA_FALSE);
    if (vb_vir_addr == XMEDIA_NULL) {
        SAMPLE_PRT("mmz map failed!\n");
        goto exit;
    }

    memset(vb_vir_addr, 0x63, blk_size); // 初始化为一个特殊的值，方便debug定位问题

    ret = xmedia_isp_get_black_level_attr(pipe, &blc_attr);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get black level failed!\n");
        goto exit;
    }
    fpn_calibrate_attr.offset                          = blc_attr.manual_attr.black_level[0];
    fpn_calibrate_attr.frame_num                       = calibrate_frame;
    fpn_calibrate_attr.fpn_frame.frame.addr.y_phy_addr = phy_addr;
    ret                                                = xmedia_isp_set_fpn_calibrate(pipe, &fpn_calibrate_attr);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("set fpn calibrate failed!\n");
        goto exit;
    }

    xmedia_sys_get_cur_pts(&end);
    SAMPLE_PRT("fpn calibrate finish. cost time: %llu us.\n", end - start);

    file = fopen("FPN.bin", "w");
    chmod("FPN.bin", 0666);
    if (file == XMEDIA_NULL) {
        SAMPLE_PRT("open file failed!\n");
        goto exit;
    }

    SAMPLE_PRT("start save calibrate result to FPN.bin.\n");
    xmedia_sys_get_cur_pts(&start);
    fwrite(vb_vir_addr, blk_size, 1, file);

    fclose(file);
    xmedia_sys_get_cur_pts(&end);
    SAMPLE_PRT("save FPN.bin finish. cost time: %llu us.\n", end - start);

exit:
    if (vb_block != VB_INVALID_HANDLE) {
        xmedia_vb_release_block(vb_block);
    }

    if (vb_vir_addr != XMEDIA_NULL) {
        xmedia_mmz_unmap(vb_vir_addr);
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_fpn_vi_offline_fpn_calibrate()
{
    xmedia_s32              ret           = XMEDIA_SUCCESS;
    xmedia_s32              blk_size      = 0;
    xmedia_video_size       pic_size      = { 0 };
    sample_comm_video_param video_param   = { 0 };
    sample_sys_config       sys_config    = { 0 };
    xmedia_ae_exposure_attr exposure_attr = { 0 };

    xmedia_s32       vi_dev    = 0;
    xmedia_s32       vi_pipe   = 0;
    xmedia_s32       vi_chn    = 0;
    sample_vi_config vi_config = { 0 };

    xmedia_s32        sensor_type = g_sample_fpn_sensor_type[0];
    xmedia_u32        framerate   = 0;
    vi_sensor_info    sensor_info = { 0 };
    sample_isp_param  isp_param   = { 0 };
    xmedia_isp_config isp_config  = { 0 };
    xmedia_bool       mirror      = XMEDIA_FALSE;
    xmedia_bool       flip        = XMEDIA_FALSE;

    video_param.video_fmt     = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt     = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width    = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);

    // sys init
    sys_config.sys_conf.pipe_mode[vi_pipe].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[vi_pipe].viproc_vpss_mode  = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[vi_pipe].gdc_vpss_mode     = XMEDIA_WORK_MODE_OFFLINE;

    sys_config.vb_conf.max_pool_cnt = 25;

    pic_size.width  = sensor_info.width;
    pic_size.height = sensor_info.height;
    blk_size        = sample_comm_sys_get_buffer_size(pic_size, video_param.video_fmt, sensor_info.pixel_format,
                                                      sensor_info.bit_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[0].block_size = blk_size;
    sys_config.vb_conf.common_pool[0].block_cnt  = 2;

    blk_size = sample_comm_sys_get_buffer_size(pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[1].block_size = blk_size;
    sys_config.vb_conf.common_pool[1].block_cnt  = 2;

    blk_size = sample_comm_sys_get_buffer_size(pic_size, video_param.video_fmt, sensor_info.pixel_format,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[2].block_size = blk_size;
    sys_config.vb_conf.common_pool[2].block_cnt  = 4;

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_fpn_sys_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sys init failed !\n");
        return ret;
    }

    vi_config.dev_info[vi_dev].dev_en                    = XMEDIA_TRUE;
    vi_config.dev_info[vi_dev].dev_no                    = vi_dev;
    vi_config.dev_info[vi_dev].sensor_type               = sensor_type;
    vi_config.pipe_info[vi_pipe].pipe_en                 = XMEDIA_TRUE;
    vi_config.pipe_info[vi_pipe].pipe_no                 = vi_pipe;
    vi_config.pipe_info[vi_pipe].chn_info[vi_chn].chn_en = XMEDIA_TRUE;
    vi_config.pipe_info[vi_pipe].chn_info[vi_chn].chn_no = vi_chn;
    vi_config.dev_bind_pipe[vi_dev].pipe[0]              = vi_pipe;
    vi_config.dev_bind_pipe[vi_dev].pipe[1]              = -1;

    isp_config.fps                                       = framerate;
    isp_config.mode_config.work_mode                     = XMEDIA_ISP_WORK_MODE_MASTER;
    isp_config.mode_config.master_mode.blend_stat_enable = XMEDIA_FALSE;
    isp_config.mode_config.master_mode.slave_num         = 0;
    isp_config.pixel_fmt                                 = sensor_info.pixel_format;
    isp_config.size.height                               = sensor_info.height;
    isp_config.size.width                                = sensor_info.width;
    isp_config.wdr_mode                                  = sensor_info.wdr_mode;

    // isp pipe init
    isp_param.pipe[vi_pipe]                   = vi_pipe;
    isp_param.isp_info[vi_pipe].isp_pipe_en   = XMEDIA_TRUE;
    isp_param.isp_info[vi_pipe].isp_sensor_en = XMEDIA_TRUE;
    isp_param.isp_info[vi_pipe].sensor_type   = sensor_type;
    isp_param.isp_info[vi_pipe].mirror        = mirror;
    isp_param.isp_info[vi_pipe].flip          = flip;

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

    xmedia_ae_get_exposure_attr(vi_pipe, &exposure_attr);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get_exposure failed!\n");
        goto exit2;
    }

    exposure_attr.auto_attr.dgain_range.max = 1024;
    exposure_attr.auto_attr.dgain_range.min = 1024;
    xmedia_ae_set_exposure_attr(vi_pipe, &exposure_attr);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("set_exposure failed!\n");
        goto exit2;
    }

    while (1) {
        xmedia_char c, a;

        SAMPLE_PRT("input cmd: 'q' - quit; 'f' - fpn calibrate.\n");
        c = getchar();
        while ((a = getchar()) != '\n' && a != EOF) {
        }
        if (c == 'q') {
            break;
        } else if (c == 'f') {
            sample_fpn_calibrate(vi_pipe, sensor_info.width, sensor_info.height);
        }
    }

exit2:
    sample_comm_isp_stop(&isp_param);

exit1:
    sample_comm_vi_stop(&vi_config);

exit0:
    sample_comm_isp_exit(&isp_param);
    sample_fpn_sys_exit();

    return ret;
}

int main(int argc, char **argv)
{
    xmedia_s32 ret       = XMEDIA_SUCCESS;
    xmedia_s32 work_mode = 0;

    signal(SIGINT, sample_fpn_handle_sig);
    signal(SIGTERM, sample_fpn_handle_sig);

    if (argc == 2) {
        if (!strncmp(argv[1], "-h", 2)) {
            sample_fpn_usage(argv[0]);
            return 0;
        }
    } else {
        sample_fpn_usage(argv[0]);
        return 0;
    }

    work_mode = atoi(argv[1]);

    switch (work_mode) {
        case 0:
            ret = sample_fpn_vi_offline_fpn_calibrate();
            break;

        default:
            sample_fpn_usage(argv[0]);
            return 0;
    }

    if (XMEDIA_SUCCESS == ret) {
        SAMPLE_PRT("program exit normally!\n");
    } else {
        SAMPLE_PRT("program exit abnormally!\n");
    }

    return 0;
}
