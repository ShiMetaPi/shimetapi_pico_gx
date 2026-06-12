/*
 * Copyright (c) Hunan xmedia,Chengdu xmedia,Shandong xmedia. 2021. All rights reserved.
 */

#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "math_fun.h"

#include "sample_comm.h"
#include "xmedia_type.h"
#include "xmedia_sys.h"
#include "xmedia_vb.h"
#include "xmedia_vo.h"
#include "sample_comm_sys.h"
#include "xmedia_scene.h"

static sample_comm_npu_workspace_cfg g_npu_workspace_cfg = {0};
static xmedia_s32 g_mem_dev;

xmedia_s32 sample_comm_sys_init(sample_sys_config *sys_conifg)
{
    CHECK_RET(xmedia_vb_exit(), "vb exit");

    CHECK_RET(xmedia_sys_exit(), "sys exit");

    CHECK_RET(xmedia_sys_init(&sys_conifg->sys_conf), "sys init");

    CHECK_RET(xmedia_vb_init(&sys_conifg->vb_conf), "vb init");

#if 0
    // setting scene capability if necessary

    CHECK_RET(xmedia_scene_init(), "scene init");

    xmedia_media_npu_load npu_load = XMEDIA_NPU_LOAD_LIGHT;
    xmedia_scene_capability scene_capability = { 0 };

    scene_capability.vpu_chn[0].vpu_chn_id = 0;
    scene_capability.vpu_chn[0].capability.width = 3840;
    scene_capability.vpu_chn[0].capability.height = 2160;
    scene_capability.vpu_chn[0].capability.framerate = 30;
    scene_capability.vpu_chn[1].vpu_chn_id = 1;
    scene_capability.vpu_chn[1].capability.width = 1280;
    scene_capability.vpu_chn[1].capability.height = 720;
    scene_capability.vpu_chn[1].capability.framerate = 30;
    CHECK_RET(xmedia_scene_config(npu_load, &scene_capability), "scene config");
#endif

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_sys_cmp_init(sample_sys_config *sys_conifg)
{
    CHECK_RET(xmedia_vb_exit(), "vb exit");

    CHECK_RET(xmedia_sys_exit(), "sys exit");

    CHECK_RET(xmedia_sys_init(&sys_conifg->sys_conf), "sys init");

    CHECK_RET(xmedia_sys_set_compress_level(sys_conifg->compress_level), "set compress level");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_vb_init(sample_sys_config *sys_conifg)
{
    CHECK_RET(xmedia_vb_init(&sys_conifg->vb_conf), "vb init");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_sys_exit(xmedia_void)
{
    CHECK_RET(xmedia_vb_exit(), "vb exit");

    CHECK_RET(xmedia_sys_exit(), "sys exit");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_sys_vi_bind_vo(xmedia_s32 vi_pipe, xmedia_s32 vi_chn, xmedia_vo_layer vo_layer,
                                      xmedia_s32 vo_chn)
{
    xmedia_chn_info src_chn;
    xmedia_chn_info dst_chn;

    src_chn.mod_id = MOD_ID_VI;
    src_chn.dev_id = vi_pipe;
    src_chn.chn_id = vi_chn;

    dst_chn.mod_id = MOD_ID_VO;
    dst_chn.dev_id = vo_layer;
    dst_chn.chn_id = vo_chn;

    CHECK_RET(xmedia_sys_bind(&src_chn, &dst_chn), "xmedia_sys_bind(VI-VO)");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_sys_vi_unbind_vo(xmedia_s32 vi_pipe, xmedia_s32 vi_chn, xmedia_vo_layer vo_layer,
                                        xmedia_s32 vo_chn)
{
    xmedia_chn_info src_chn;
    xmedia_chn_info dst_chn;

    src_chn.mod_id = MOD_ID_VI;
    src_chn.dev_id = vi_pipe;
    src_chn.chn_id = vi_chn;

    dst_chn.mod_id = MOD_ID_VO;
    dst_chn.dev_id = vo_layer;
    dst_chn.chn_id = vo_chn;

    CHECK_RET(xmedia_sys_unbind(&src_chn, &dst_chn), "xmedia_sys_unbind(VI-VO)");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_sys_vi_bind_vpss(xmedia_s32 vi_pipe, xmedia_s32 vi_chn, xmedia_s32 vpss_pipe,
                                        xmedia_s32 vpss_ichn)
{
    xmedia_chn_info src_chn;
    xmedia_chn_info dst_chn;

    src_chn.mod_id = MOD_ID_VI;
    src_chn.dev_id = vi_pipe;
    src_chn.chn_id = vi_chn;

    dst_chn.mod_id = MOD_ID_VPSS;
    dst_chn.dev_id = vpss_pipe;
    dst_chn.chn_id = vpss_ichn;

    CHECK_RET(xmedia_sys_bind(&src_chn, &dst_chn), "xmedia_sys_bind(VI-VPSS)");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_sys_vi_unbind_vpss(xmedia_s32 vi_pipe, xmedia_s32 vi_chn, xmedia_s32 vpss_pipe,
                                          xmedia_s32 vpss_ichn)
{
    xmedia_chn_info src_chn;
    xmedia_chn_info dst_chn;

    src_chn.mod_id = MOD_ID_VI;
    src_chn.dev_id = vi_pipe;
    src_chn.chn_id = vi_chn;

    dst_chn.mod_id = MOD_ID_VPSS;
    dst_chn.dev_id = vpss_pipe;
    dst_chn.chn_id = vpss_ichn;

    CHECK_RET(xmedia_sys_unbind(&src_chn, &dst_chn), "xmedia_sys_unbind(VI-VPSS)");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_sys_vi_bind_mcf(xmedia_s32 vi_pipe, xmedia_s32 vi_chn, xmedia_s32 mcf_pipe,
                                        xmedia_s32 mcf_ichn)
{
    xmedia_chn_info src_chn;
    xmedia_chn_info dst_chn;

    src_chn.mod_id = MOD_ID_VI;
    src_chn.dev_id = vi_pipe;
    src_chn.chn_id = vi_chn;

    dst_chn.mod_id = MOD_ID_MCF;
    dst_chn.dev_id = mcf_pipe;
    dst_chn.chn_id = mcf_ichn;

    CHECK_RET(xmedia_sys_bind(&src_chn, &dst_chn), "xmedia_sys_bind(VI-MCF)");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_sys_vi_unbind_mcf(xmedia_s32 vi_pipe, xmedia_s32 vi_chn, xmedia_s32 mcf_pipe,
                                          xmedia_s32 mcf_ichn)
{
    xmedia_chn_info src_chn;
    xmedia_chn_info dst_chn;

    src_chn.mod_id = MOD_ID_VI;
    src_chn.dev_id = vi_pipe;
    src_chn.chn_id = vi_chn;

    dst_chn.mod_id = MOD_ID_MCF;
    dst_chn.dev_id = mcf_pipe;
    dst_chn.chn_id = mcf_ichn;

    CHECK_RET(xmedia_sys_unbind(&src_chn, &dst_chn), "xmedia_sys_unbind(VI-MCF)");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_sys_vi_bind_venc(xmedia_s32 vi_pipe, xmedia_s32 vi_chn, xmedia_s32 venc_chn)
{
    xmedia_chn_info src_chn;
    xmedia_chn_info dst_chn;

    src_chn.mod_id = MOD_ID_VI;
    src_chn.dev_id = vi_pipe;
    src_chn.chn_id = vi_chn;

    dst_chn.mod_id = MOD_ID_VENC;
    dst_chn.dev_id = 0;
    dst_chn.chn_id = venc_chn;

    CHECK_RET(xmedia_sys_bind(&src_chn, &dst_chn), "xmedia_sys_bind(VI-VENC)");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_sys_vi_unbind_venc(xmedia_s32 vi_pipe, xmedia_s32 vi_chn, xmedia_s32 venc_chn)
{
    xmedia_chn_info src_chn;
    xmedia_chn_info dst_chn;

    src_chn.mod_id = MOD_ID_VI;
    src_chn.dev_id = vi_pipe;
    src_chn.chn_id = vi_chn;

    dst_chn.mod_id = MOD_ID_VENC;
    dst_chn.dev_id = 0;
    dst_chn.chn_id = venc_chn;

    CHECK_RET(xmedia_sys_unbind(&src_chn, &dst_chn), "xmedia_sys_unbind(VI-VENC)");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_sys_mcf_bind_vpss(xmedia_s32 mcf_pipe, xmedia_s32 mcf_chn, xmedia_s32 vpss_pipe,
                                        xmedia_s32 vpss_ichn)
{
    xmedia_chn_info src_chn;
    xmedia_chn_info dst_chn;

    src_chn.mod_id = MOD_ID_MCF;
    src_chn.dev_id = mcf_pipe;
    src_chn.chn_id = mcf_chn;

    dst_chn.mod_id = MOD_ID_VPSS;
    dst_chn.dev_id = vpss_pipe;
    dst_chn.chn_id = vpss_ichn;

    CHECK_RET(xmedia_sys_bind(&src_chn, &dst_chn), "xmedia_sys_bind(MCF-VPSS)");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_sys_mcf_unbind_vpss(xmedia_s32 mcf_pipe, xmedia_s32 mcf_chn, xmedia_s32 vpss_pipe,
                                          xmedia_s32 vpss_ichn)
{
    xmedia_chn_info src_chn;
    xmedia_chn_info dst_chn;

    src_chn.mod_id = MOD_ID_MCF;
    src_chn.dev_id = mcf_pipe;
    src_chn.chn_id = mcf_chn;

    dst_chn.mod_id = MOD_ID_VPSS;
    dst_chn.dev_id = vpss_pipe;
    dst_chn.chn_id = vpss_ichn;

    CHECK_RET(xmedia_sys_unbind(&src_chn, &dst_chn), "xmedia_sys_unbind(MCF-VPSS)");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_sys_vpss_bind_vo(xmedia_s32 vpss_pipe, xmedia_s32 vpss_ochn, xmedia_vo_layer vo_layer,
                                        xmedia_s32 vo_chn)
{
    xmedia_chn_info src_chn;
    xmedia_chn_info dst_chn;

    src_chn.mod_id = MOD_ID_VPSS;
    src_chn.dev_id = vpss_pipe;
    src_chn.chn_id = vpss_ochn;

    dst_chn.mod_id = MOD_ID_VO;
    dst_chn.dev_id = vo_layer;
    dst_chn.chn_id = vo_chn;

    CHECK_RET(xmedia_sys_bind(&src_chn, &dst_chn), "xmedia_sys_bind(VPSS-VO)");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_sys_vpss_unbind_vo(xmedia_s32 vpss_pipe, xmedia_s32 vpss_ochn, xmedia_vo_layer vo_layer,
                                          xmedia_s32 vo_chn)
{
    xmedia_chn_info src_chn;
    xmedia_chn_info dst_chn;

    src_chn.mod_id = MOD_ID_VPSS;
    src_chn.dev_id = vpss_pipe;
    src_chn.chn_id = vpss_ochn;

    dst_chn.mod_id = MOD_ID_VO;
    dst_chn.dev_id = vo_layer;
    dst_chn.chn_id = vo_chn;

    CHECK_RET(xmedia_sys_unbind(&src_chn, &dst_chn), "xmedia_sys_unbind(VPSS-VO)");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_sys_vpss_bind_venc(xmedia_s32 vpss_pipe, xmedia_s32 vpss_ochn, xmedia_s32 venc_chn)
{
    xmedia_chn_info src_chn;
    xmedia_chn_info dst_chn;

    src_chn.mod_id = MOD_ID_VPSS;
    src_chn.dev_id = vpss_pipe;
    src_chn.chn_id = vpss_ochn;

    dst_chn.mod_id = MOD_ID_VENC;
    dst_chn.dev_id = 0;
    dst_chn.chn_id = venc_chn;

    CHECK_RET(xmedia_sys_bind(&src_chn, &dst_chn), "xmedia_sys_bind(VPSS-VENC)");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_sys_vpss_unbind_venc(xmedia_s32 vpss_pipe, xmedia_s32 vpss_ochn, xmedia_s32 venc_chn)
{
    xmedia_chn_info src_chn;
    xmedia_chn_info dst_chn;

    src_chn.mod_id = MOD_ID_VPSS;
    src_chn.dev_id = vpss_pipe;
    src_chn.chn_id = vpss_ochn;

    dst_chn.mod_id = MOD_ID_VENC;
    dst_chn.dev_id = 0;
    dst_chn.chn_id = venc_chn;

    CHECK_RET(xmedia_sys_unbind(&src_chn, &dst_chn), "xmedia_sys_unbind(VPSS-VENC)");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_sys_vdec_bind_vpss(xmedia_s32 vdec_chn, xmedia_s32 vpss_pipe, xmedia_s32 vpss_ichn)
{
    xmedia_chn_info src_chn;
    xmedia_chn_info dst_chn;

    src_chn.mod_id = MOD_ID_VDEC;
    src_chn.dev_id = 0;
    src_chn.chn_id = vdec_chn;

    dst_chn.mod_id = MOD_ID_VPSS;
    dst_chn.dev_id = vpss_pipe;
    dst_chn.chn_id = vpss_ichn;

    CHECK_RET(xmedia_sys_bind(&src_chn, &dst_chn), "xmedia_sys_bind(VDEC-VPSS)");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_sys_vdec_unbind_vpss(xmedia_s32 vdec_chn, xmedia_s32 vpss_pipe, xmedia_s32 vpss_ichn)
{
    xmedia_chn_info src_chn;
    xmedia_chn_info dst_chn;

    src_chn.mod_id = MOD_ID_VDEC;
    src_chn.dev_id = 0;
    src_chn.chn_id = vdec_chn;

    dst_chn.mod_id = MOD_ID_VPSS;
    dst_chn.dev_id = vpss_pipe;
    dst_chn.chn_id = vpss_ichn;

    CHECK_RET(xmedia_sys_unbind(&src_chn, &dst_chn), "xmedia_sys_unbind(VDEC-VPSS)");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_sys_vdec_bind_vo(xmedia_vo_dev vo_dev, xmedia_vo_chn vo_chn, xmedia_s32 vdec_chn)
{
    xmedia_chn_info src_chn;
    xmedia_chn_info dst_chn;

    src_chn.mod_id = MOD_ID_VDEC;
    src_chn.dev_id = 0;
    src_chn.chn_id = vdec_chn;

    dst_chn.mod_id = MOD_ID_VO;
    dst_chn.dev_id = vo_dev;
    dst_chn.chn_id = vo_chn;

    CHECK_RET(xmedia_sys_bind(&src_chn, &dst_chn), "xmedia_sys_bind(VDEC-VO)");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_sys_vdec_unbind_vo(xmedia_vo_dev vo_dev, xmedia_vo_chn vo_chn, xmedia_s32 vdec_chn)
{
    xmedia_chn_info src_chn;
    xmedia_chn_info dst_chn;

    src_chn.mod_id = MOD_ID_VDEC;
    src_chn.dev_id = 0;
    src_chn.chn_id = vdec_chn;

    dst_chn.mod_id = MOD_ID_VO;
    dst_chn.dev_id = vo_dev;
    dst_chn.chn_id = vo_chn;

    CHECK_RET(xmedia_sys_unbind(&src_chn, &dst_chn), "xmedia_sys_unbind(VDEC-VO)");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_sys_vo_bind_venc(xmedia_vo_dev vo_dev, xmedia_s32 venc_chn)
{
    xmedia_chn_info src_chn;
    xmedia_chn_info dst_chn;

    src_chn.mod_id = MOD_ID_VO;
    src_chn.dev_id = vo_dev;
    src_chn.chn_id = 0;

    dst_chn.mod_id = MOD_ID_VENC;
    dst_chn.dev_id = 0;
    dst_chn.chn_id = venc_chn;

    CHECK_RET(xmedia_sys_bind(&src_chn, &dst_chn), "xmedia_sys_bind(VO-VENC)");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_sys_vo_unbind_venc(xmedia_vo_dev vo_dev, xmedia_s32 venc_chn)
{
    xmedia_chn_info src_chn;
    xmedia_chn_info dst_chn;

    src_chn.mod_id = MOD_ID_VO;
    src_chn.dev_id = vo_dev;
    src_chn.chn_id = 0;

    dst_chn.mod_id = MOD_ID_VENC;
    dst_chn.dev_id = 0;
    dst_chn.chn_id = venc_chn;


    CHECK_RET(xmedia_sys_unbind(&src_chn, &dst_chn), "xmedia_sys_unbind(VO-VENC)");

    return XMEDIA_SUCCESS;
}

pic_size_e sample_comm_sys_get_picszie_by_wxh(xmedia_u32 width, xmedia_u32 height)
{
    pic_size_e pic_size;

    if ((width == 352) && (height == 288)) {
        pic_size = PIC_CIF;
    } else if ((width == 640) && (height == 360)) {
        pic_size = PIC_360P;
    } else if ((width == 640) && (height == 480)) {
        pic_size = PIC_VGA;
    } else if ((width == 640) && (height == 360)) {
        pic_size = PIC_640x360;
    } else if ((width == 720) && (height == 576)) {
        pic_size = PIC_D1_PAL;
    } else if ((width == 720) && (height == 480)) {
        pic_size = PIC_D1_NTSC;
    } else if ((width == 1280) && (height == 720)) {
        pic_size = PIC_720P;
    } else if ((width == 1920) && (height == 1080)) {
        pic_size = PIC_1080P;
    } else if ((width == 2304) && (height == 1296)) {
        pic_size = PIC_2304x1296;
    } else if ((width == 2560) && (height == 1440)) {
        pic_size = PIC_2560x1440;
    } else if ((width == 2560) && (height == 1920)) {
        pic_size = PIC_2560x1920;
    } else if ((width == 2592) && (height == 1520)) {
        pic_size = PIC_2592x1520;
    } else if ((width == 2592) && (height == 1944)) {
        pic_size = PIC_2592x1944;
    } else if ((width == 2592) && (height == 1536)) {
        pic_size = PIC_2592x1536;
    } else if ((width == 2688) && (height == 1520)) {
        pic_size = PIC_2688x1520;
    } else if ((width == 2716) && (height == 1524)) {
        pic_size = PIC_2716x1524;
    } else if ((width == 3840) && (height == 2160)) {
        pic_size = PIC_3840x2160;
    } else if ((width == 3000) && (height == 3000)) {
        pic_size = PIC_3000x3000;
    } else if ((width == 4000) && (height == 3000)) {
        pic_size = PIC_4000x3000;
    } else if ((width == 4096) && (height == 2160)) {
        pic_size = PIC_4096x2160;
    } else if ((width == 7680) && (height == 4320)) {
        pic_size = PIC_7680x4320;
    } else if ((width == 3840) && (height == 8640)) {
        pic_size = PIC_3840x8640;
    } else if ((width == 2688) && (height == 1536)) {
        pic_size = PIC_2688x1536;
    } else if ((width == 2688) && (height == 1944)) {
        pic_size = PIC_2688x1944;
    } else if ((width == 2880) && (height == 1620)) {
        pic_size = PIC_2880x1620;
    } else if ((width == 3072) && (height == 1728)) {
        pic_size = PIC_3072x1728;
    } else {
        pic_size = PIC_BUTT;
    }

    return pic_size;
}

xmedia_s32 sample_comm_sys_get_buffer_size(xmedia_video_size pic_size, xmedia_video_format video_fmt,
                                           xmedia_video_pixel_format pixel_fmt, xmedia_video_data_width data_width,
                                           xmedia_video_compress_mode compress_mode)
{
    xmedia_vb_base_info vb_base_info = { 0 };
    xmedia_vb_cal_cfg vb_cfg = { 0 };

    vb_base_info.width = pic_size.width;
    vb_base_info.height = pic_size.height;
    vb_base_info.video_fmt = video_fmt;
    vb_base_info.pixel_fmt = pixel_fmt;
    vb_base_info.bit_width = data_width;
    vb_base_info.align = DEFAULT_ALIGN;
    vb_base_info.cmp_mode = compress_mode;

    xmedia_vb_get_buffer_config(&vb_base_info, &vb_cfg);

    return vb_cfg.vb_size;
}

xmedia_s32 sample_comm_sys_get_wrap_buffer_size(xmedia_video_size pic_size, xmedia_video_format video_fmt,
                                                xmedia_video_pixel_format pixel_fmt, xmedia_video_data_width data_width,
                                                xmedia_video_compress_mode compress_mode, xmedia_u32 *wrap_buffer_line)
{
    xmedia_vb_base_info vb_base_info = { 0 };

    vb_base_info.width = pic_size.width;
    vb_base_info.height = pic_size.height;
    vb_base_info.video_fmt = XMEDIA_VIDEO_FMT_TILE_32x4;
    vb_base_info.pixel_fmt = pixel_fmt;
    vb_base_info.bit_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    vb_base_info.align = DEFAULT_ALIGN;
    vb_base_info.cmp_mode = XMEDIA_VIDEO_COMPRESS_MODE_TILE_COMPACT;

    *wrap_buffer_line = XM_ALIGN_DOWN(pic_size.height * 5 / 10, 8);
    SAMPLE_PRT("wrap buffer line is  %d \n", *wrap_buffer_line);

    return xmedia_vb_get_wrap_buffer_size(&vb_base_info, *wrap_buffer_line);
}

xmedia_s32 sample_comm_sys_getchar_timeout(xmedia_u32 timeout_ms)
{
    fd_set read_fds;
    struct timeval timeout;
    xmedia_s32 retval;

    FD_ZERO(&read_fds);
    FD_SET(STDIN_FILENO, &read_fds);

    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;

    retval = select(STDIN_FILENO + 1, &read_fds, XMEDIA_NULL, XMEDIA_NULL, &timeout);
    if (retval <= 0) {
        return XMEDIA_FAILURE;
    } else {
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            retval = getchar();
            return retval;
        }
    }

    return XMEDIA_FAILURE;
}

xmedia_s32 sample_comm_sys_npu_acquire_workspace(sample_comm_npu_workspace_info *npu_info)
{
    xmedia_s32 s32_ret = XMEDIA_FAILURE;
    xmedia_s32 cl_err_code = 0;
    xmedia_u32 cl_work_size = 0;
    xmedia_u32 cl_weight_size = 0;
    xmedia_u32 max_cl_work_size = 0;

    CHECK_NULL_PTR(npu_info);

    if (npu_info->npu_modle_num == 0) {
        SAMPLE_ERR("acquire workspace fail model num 0 .\n");
        return XMEDIA_FAILURE;
    }

    s32_ret = xmedia_cl_init();
    if (s32_ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_cl_init fail \n");
        return s32_ret;
    }

    s32_ret = xmedia_cl_get_device_ids(XMEDIA_CL_DEVICE_ALL, XMEDIA_NULL, &g_npu_workspace_cfg.cl_device_num);
    if (s32_ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_cl_get_device_ids fail \n");
        goto CL_ERR0;
    }

    g_npu_workspace_cfg.cl_device = (xmedia_cl_device_id *)calloc(g_npu_workspace_cfg.cl_device_num, sizeof(xmedia_cl_device_id));
    if (g_npu_workspace_cfg.cl_device == XMEDIA_NULL) {
        SAMPLE_ERR("cl device calloc fail\n");
        goto CL_ERR0;
    }

    s32_ret = xmedia_cl_get_device_ids(XMEDIA_CL_DEVICE_ALL, g_npu_workspace_cfg.cl_device, &g_npu_workspace_cfg.cl_device_num);
    if (s32_ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_cl_get_device_ids sec fail \n");
        goto CL_ERR1;
    }

    g_npu_workspace_cfg.cl_ctx = xmedia_cl_create_context(g_npu_workspace_cfg.cl_device_num, g_npu_workspace_cfg.cl_device, &cl_err_code);
    if (cl_err_code != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_cl_create_context fail\n");
        goto CL_ERR2;
    }

    for (xmedia_s32 i = 0; i < npu_info->npu_modle_num; i++) {
        s32_ret = xmedia_cl_graph_querysize_from_file(npu_info->npu_model_path[i], &cl_work_size, &cl_weight_size);
        if (cl_err_code != XMEDIA_SUCCESS) {
            SAMPLE_ERR("xmedia_cl_graph_querysize_from_file fail\n");
            goto CL_ERR3;
        }

        max_cl_work_size = (max_cl_work_size > cl_work_size) ? max_cl_work_size : cl_work_size;
    }

    g_npu_workspace_cfg.work_space_phyaddr = xmedia_mmz_alloc(XMEDIA_NULL, "AIworkspace", max_cl_work_size);
    if (g_npu_workspace_cfg.work_space_phyaddr == XMEDIA_NULL) {
        SAMPLE_ERR("xmedia_mmz_alloc fail\n");
        goto CL_ERR3;
    }

    g_npu_workspace_cfg.work_space_viraddr = xmedia_mmz_map(g_npu_workspace_cfg.work_space_phyaddr, max_cl_work_size, 1);
    if (g_npu_workspace_cfg.work_space_viraddr == XMEDIA_NULL) {
        SAMPLE_ERR("xmedia_mmz_map fail\n");
        goto CL_ERR4;
    }

    s32_ret = xmedia_cl_set_workspace_addr(g_npu_workspace_cfg.cl_ctx, g_npu_workspace_cfg.work_space_viraddr, max_cl_work_size);
    if (s32_ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_mmz_map fail\n");
        goto CL_ERR5;
    }

    return XMEDIA_SUCCESS;

CL_ERR5:
    if (g_npu_workspace_cfg.work_space_viraddr != XMEDIA_NULL) {
        xmedia_mmz_unmap(g_npu_workspace_cfg.work_space_viraddr);
    }
CL_ERR4:
    xmedia_mmz_free(g_npu_workspace_cfg.work_space_phyaddr);
CL_ERR3:
    s32_ret = xmedia_cl_release_context(g_npu_workspace_cfg.cl_ctx);
    if (s32_ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_cl_release_context id fail \n");
    }
CL_ERR2:
    s32_ret = xmedia_cl_release_device_ids(g_npu_workspace_cfg.cl_device, &g_npu_workspace_cfg.cl_device_num);
    if (s32_ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_cl_release_device_ids id fail \n");
    }
CL_ERR1:
    free(g_npu_workspace_cfg.cl_device);
    g_npu_workspace_cfg.cl_device = XMEDIA_NULL;
CL_ERR0:
    s32_ret = xmedia_cl_uninit();
    if (s32_ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_cl_uninit fail \n");
        return s32_ret;
    }

    return XMEDIA_FAILURE;
}

xmedia_s32 sample_comm_sys_npu_release_workspace()
{
    xmedia_s32 s32_ret = XMEDIA_FAILURE;

    if (g_npu_workspace_cfg.work_space_viraddr != XMEDIA_NULL) {
        xmedia_mmz_unmap(g_npu_workspace_cfg.work_space_viraddr);
    }

    xmedia_mmz_free(g_npu_workspace_cfg.work_space_phyaddr);

    s32_ret = xmedia_cl_release_context(g_npu_workspace_cfg.cl_ctx);
    if (s32_ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_cl_release_context id fail \n");
    }

    s32_ret = xmedia_cl_release_device_ids(g_npu_workspace_cfg.cl_device, &g_npu_workspace_cfg.cl_device_num);
    if (s32_ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_cl_release_device_ids id fail \n");
    }

    free(g_npu_workspace_cfg.cl_device);
    g_npu_workspace_cfg.cl_device = XMEDIA_NULL;

    s32_ret = xmedia_cl_uninit();
    if (s32_ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_cl_uninit fail \n");
        return s32_ret;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_sys_mem_open(xmedia_void)
{
    if (g_mem_dev <= 0) {
        g_mem_dev = open ("/dev/mem", O_CREAT | O_RDWR | O_SYNC);
        if (g_mem_dev <= 0) {
            return XMEDIA_FAILURE;
        }
    }
    return XMEDIA_SUCCESS;
}

xmedia_void sample_comm_sys_mem_close(xmedia_void)
{
    if (g_mem_dev > 0) {
        close(g_mem_dev);
        g_mem_dev = 0;
    }
}

xmedia_void* sample_comm_sys_mem_map(xmedia_u32 phy_addr, xmedia_u32 size)
{
    xmedia_u32 diff;
    xmedia_u32 page_phy;
    xmedia_u32 page_size;
    xmedia_u8* page_addr;

    page_phy = phy_addr & 0xfffff000;
    diff = phy_addr - page_phy;

    page_size = ((size + diff - 1) & 0xfffff000) + 0x1000;
    page_addr = mmap((void *)0, page_size, PROT_READ|PROT_WRITE, MAP_SHARED, g_mem_dev, page_phy);
    if (MAP_FAILED == page_addr) {
        perror("mmap error\n");
        return XMEDIA_NULL;
    }

    return (xmedia_void *) (page_addr + diff);
}

xmedia_void sample_comm_sys_mem_unmap(xmedia_void* vir_addr, xmedia_u32 size)
{
    xmedia_u32 page_addr;
    xmedia_u32 page_size;
    xmedia_u32 diff;

    page_addr = (((xmedia_u32)vir_addr) & 0xfffff000);
    diff      = (xmedia_u32)vir_addr - page_addr;
    page_size = ((size + diff - 1) & 0xfffff000) + 0x1000;

    munmap((xmedia_void*)page_addr, page_size);
}

xmedia_s32 sample_comm_sys_set_reg(xmedia_u32 addr, xmedia_u32 value)
{
    xmedia_u32 *vir_addr = XMEDIA_NULL;

    vir_addr  = sample_comm_sys_mem_map(addr, sizeof(value));
    if (vir_addr == XMEDIA_NULL) {
        SAMPLE_PRT("sample sys mem map 0x%x failed\n", addr);
        return XMEDIA_FAILURE;
    }

    *vir_addr = value;
    sample_comm_sys_mem_unmap(vir_addr, sizeof(value));

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_sys_get_reg(xmedia_u32 addr, xmedia_u32 *value)
{
    xmedia_u32 *vir_addr;

    vir_addr  = sample_comm_sys_mem_map(addr, sizeof(value));
    if (vir_addr == XMEDIA_NULL) {
        SAMPLE_PRT("sample sys mem map 0x%x failed\n", addr);
        return XMEDIA_FAILURE;
    }

    *value = *vir_addr;
    sample_comm_sys_mem_unmap(vir_addr, sizeof(value));

    return XMEDIA_SUCCESS;
}
