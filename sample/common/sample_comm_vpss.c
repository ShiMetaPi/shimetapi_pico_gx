/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "math_fun.h"
#include "xmedia_debug.h"
#include "sample_comm.h"
#include "sample_comm_vpss.h"

xmedia_s32 sample_comm_vpss_create_pipe(xmedia_s32 pipe_id, vpss_pipe_info* pipe_info)
{
    xmedia_vpss_pipe_config pipe_cfg = {0};

    memcpy(&pipe_cfg, &pipe_info->pipe_config, sizeof(xmedia_vpss_pipe_config));
    CHECK_RET(xmedia_vpss_create_pipe(pipe_id, &pipe_cfg), "create vpss pipe");

    if (pipe_info->crop_attr.enable == XMEDIA_TRUE) {
        CHECK_RET(xmedia_vpss_set_pipe_crop(pipe_id, &pipe_info->crop_attr), "set pipe crop");
    }

    if ((pipe_info->stnr_attr.cnr_enable == XMEDIA_TRUE) ||
        (pipe_info->stnr_attr.tnr_enable == XMEDIA_TRUE) ||
        (pipe_info->stnr_attr.ynr_enable == XMEDIA_TRUE)) {
        CHECK_RET(xmedia_vpss_set_pipe_stnr_attr(pipe_id, &pipe_info->stnr_attr), "set pipe stnr");
    }

    if (pipe_info->sharpen_attr.enable == XMEDIA_TRUE) {
        CHECK_RET(xmedia_vpss_set_pipe_sharpen_attr(pipe_id, &pipe_info->sharpen_attr), "set pipe sharpen");
    }

    if (pipe_info->stitch_attr.stitch_en == XMEDIA_TRUE) {
        CHECK_RET(xmedia_vpss_set_pipe_stitch_blend_attr(pipe_id, &pipe_info->stitch_attr.stitch_blend_attr), "set pipe stitch");
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_vpss_destory_pipe(xmedia_s32 pipe_id)
{
    CHECK_RET(xmedia_vpss_destroy_pipe(pipe_id), "destroy pipe");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_vpss_set_chn_attr(xmedia_s32 pipe_id, xmedia_s32 chn_id, vpss_chn_info *chn_info)
{
    xmedia_vpss_ochn_config ochn_cfg = {0};

    memcpy(&ochn_cfg, &chn_info->chn_config, sizeof(xmedia_vpss_ochn_config));

    CHECK_RET(xmedia_vpss_set_ochn_config(pipe_id, chn_id, &ochn_cfg), "set vpss channel attr");

    if (chn_info->crop_attr.enable == XMEDIA_TRUE) {
        CHECK_RET(xmedia_vpss_set_ochn_post_crop(pipe_id, chn_id, &chn_info->crop_attr), "set vpss channel crop");
    }

    if ((chn_info->rot_attr == XMEDIA_VIDEO_ROTATION_90) ||
        (chn_info->rot_attr == XMEDIA_VIDEO_ROTATION_180) ||
        (chn_info->rot_attr == XMEDIA_VIDEO_ROTATION_270)) {
        CHECK_RET(xmedia_vpss_set_ochn_rotation(pipe_id, chn_id, chn_info->rot_attr), "set chn rotate");
    }

    if (chn_info->lowdelay_attr.enable == XMEDIA_TRUE) {
        CHECK_RET(xmedia_vpss_set_ochn_low_delay_attr(pipe_id, chn_id, &chn_info->lowdelay_attr), "set chn rotate");
    }

    if ((chn_info->wrap_attr.enable == XMEDIA_TRUE)&&(chn_id == 0)) {
        if (chn_info->wrap_attr.buf_line == 0) {
            chn_info->wrap_attr.buf_line = XM_ALIGN_DOWN(chn_info->chn_config.height/2, 8);
        }
        xmedia_vb_base_info vb_info;
        vb_info.width = chn_info->chn_config.width;
        vb_info.height = chn_info->chn_config.height;
        vb_info.align = 0; //auto
        vb_info.bit_width = XMEDIA_VIDEO_DATA_WIDTH_8;
        vb_info.cmp_mode = chn_info->chn_config.cmp_mode;
        vb_info.pixel_fmt = chn_info->chn_config.pix_fmt;
        vb_info.video_fmt = chn_info->chn_config.video_fmt;
        chn_info->wrap_attr.buf_size = xmedia_vb_get_wrap_buffer_size(&vb_info, chn_info->wrap_attr.buf_line);
        CHECK_RET(xmedia_vpss_set_ochn_wrap_attr(pipe_id, chn_id, &chn_info->wrap_attr), "set chn0 buffer wrap");
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_vpss_pipe_chn_start(sample_vpss_config *vpss_cfg)
{
    xmedia_s32 pipe_id, chn_id, ext_chn_id;
    xmedia_s32 ext_base_chn_id = VPSS_MAX_PHY_OCHN_NUM;

    for (pipe_id = 0; pipe_id < VPSS_MAX_PIPE_NUM; pipe_id++) {

        for (chn_id = 0; chn_id < VPSS_MAX_PHY_OCHN_NUM; chn_id++) {
            if (vpss_cfg->pipe_info[pipe_id].chn_info[chn_id].chn_en == XMEDIA_FALSE) {
                continue;
            }
            CHECK_RET(xmedia_vpss_enable_ochn(pipe_id, chn_id), "enable channel");
        }

        for (ext_chn_id = 0; ext_chn_id < VPSS_MAX_EXT_OCHN_NUM; ext_chn_id++) {
            if (vpss_cfg->pipe_info[pipe_id].ext_chn_info[ext_chn_id].ext_chn_en == XMEDIA_FALSE) {
                continue;
            }
            CHECK_RET(xmedia_vpss_enable_ochn(pipe_id, ext_base_chn_id + ext_chn_id), "enable channel");
        }

        if (vpss_cfg->pipe_info[pipe_id].pipe_en == XMEDIA_TRUE) {
            CHECK_RET(xmedia_vpss_start_pipe(pipe_id), "start vpss pipe");
        }
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_vpss_pipe_chn_stop(sample_vpss_config *vpss_cfg)
{
    xmedia_s32 pipe_id, chn_id, ext_chn_id;
    xmedia_s32 ext_base_chn_id = VPSS_MAX_PHY_OCHN_NUM;

    for (pipe_id = 0; pipe_id < VPSS_MAX_PIPE_NUM; pipe_id++) {
        if (vpss_cfg->pipe_info[pipe_id].pipe_en == XMEDIA_FALSE) {
            continue;
        }
        CHECK_RET(xmedia_vpss_stop_pipe(pipe_id), "stop pipe");

        for (ext_chn_id = 0; ext_chn_id < VPSS_MAX_EXT_OCHN_NUM; ext_chn_id++) {
            if (vpss_cfg->pipe_info[pipe_id].ext_chn_info[ext_chn_id].ext_chn_en == XMEDIA_FALSE) {
                continue;
            }
            CHECK_RET(xmedia_vpss_disable_ochn(pipe_id, ext_base_chn_id + ext_chn_id), "disable vpss ext channel");
        }

        for (chn_id = 0; chn_id < VPSS_MAX_PHY_OCHN_NUM; chn_id++) {
            if (vpss_cfg->pipe_info[pipe_id].chn_info[chn_id].chn_en == XMEDIA_FALSE) {
                continue;
            }
            CHECK_RET(xmedia_vpss_disable_ochn(pipe_id, chn_id), "disable vpss channel");
        }
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_vpss_get_default_pipe_cfg(xmedia_vpss_pipe_config *pipe_cfg, xmedia_video_size in_size,sample_comm_video_param *video_param)
{
    CHECK_NULL_PTR(pipe_cfg);

    pipe_cfg->max_w = in_size.width;
    pipe_cfg->max_h = in_size.height;
    pipe_cfg->pixel_fmt = video_param->pixel_fmt;
    pipe_cfg->dync_range = XMEDIA_VIDEO_DYNAMIC_RANGE_SDR;
    pipe_cfg->frm_rate_ctrl.src_frm_rate = -1;
    pipe_cfg->frm_rate_ctrl.dst_frm_rate = -1;
    pipe_cfg->nr_attr.nr_en = XMEDIA_FALSE;
    pipe_cfg->nr_attr.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_LINE;
    pipe_cfg->stitch_blend_cfg.stitch_blend_en = XMEDIA_FALSE;

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_vpss_get_default_ochn_cfg(xmedia_vpss_ochn_config *ochn_cfg, xmedia_video_size out_size, sample_comm_video_param *video_param)
{
    CHECK_NULL_PTR(ochn_cfg);

    ochn_cfg->mode = XMEDIA_VPSS_OCHN_MODE_USER;
    ochn_cfg->scale_mode = XMEDIA_VIDEO_SCALE_MODE_NORMAL;
    ochn_cfg->width = out_size.width;
    ochn_cfg->height = out_size.height;
    ochn_cfg->pix_fmt = video_param->pixel_fmt;
    ochn_cfg->video_fmt = video_param->video_fmt;
    ochn_cfg->cmp_mode = video_param->compress_mode;
    ochn_cfg->dync_range  = XMEDIA_VIDEO_DYNAMIC_RANGE_SDR;
    ochn_cfg->frm_rate_ctrl.src_frm_rate = -1;
    ochn_cfg->frm_rate_ctrl.dst_frm_rate = -1;
    ochn_cfg->mirror_en = XMEDIA_FALSE;
    ochn_cfg->flip_en = XMEDIA_FALSE;
    ochn_cfg->depth = 1;

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_vpss_set_ochn_wrap(vpss_chn_info *ochn_info)
{
    xmedia_vb_base_info vb_base_info = {0};

    vb_base_info.width = ochn_info->chn_config.width;
    vb_base_info.height = ochn_info->chn_config.height;
    vb_base_info.pixel_fmt = ochn_info->chn_config.pix_fmt;
    vb_base_info.video_fmt = ochn_info->chn_config.video_fmt;
    vb_base_info.cmp_mode = ochn_info->chn_config.cmp_mode;
    vb_base_info.bit_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    vb_base_info.align = DEFAULT_ALIGN;

    ochn_info->wrap_attr.enable = XMEDIA_TRUE;
    ochn_info->wrap_attr.buf_line = XM_ALIGN_DOWN(ochn_info->chn_config.height / 2, 8);
    ochn_info->wrap_attr.buf_size = xmedia_vb_get_wrap_buffer_size(&vb_base_info, ochn_info->wrap_attr.buf_line);

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_vpss_start(sample_vpss_config *vpss_cfg)
{
    vpss_pipe_info *pipe_info = NULL;
    vpss_chn_info *chn_info = NULL;
    vpss_ext_chn_info *ext_chn_info = NULL;
    xmedia_s32 pipe_id, chn_id, ext_chn_id;
    xmedia_s32 ext_base_chn_id = VPSS_MAX_PHY_OCHN_NUM;

    for (pipe_id = 0; pipe_id < VPSS_MAX_PIPE_NUM; pipe_id++) {
        if(vpss_cfg->pipe_info[pipe_id].pipe_en == XMEDIA_FALSE) {
            continue;
        }
        pipe_info = &vpss_cfg->pipe_info[pipe_id];
        CHECK_RET(sample_comm_vpss_create_pipe(pipe_id, pipe_info), "create vpss pipe and set attr");

        for (chn_id = 0; chn_id < VPSS_MAX_PHY_OCHN_NUM; chn_id++) {
            if(vpss_cfg->pipe_info[pipe_id].chn_info[chn_id].chn_en == XMEDIA_FALSE) {
                continue;
            }

            chn_info  = &vpss_cfg->pipe_info[pipe_id].chn_info[chn_id];
            CHECK_RET(sample_comm_vpss_set_chn_attr(pipe_id, chn_id, chn_info), "set vpss chn attr");
        }

        for (ext_chn_id = 0; ext_chn_id < VPSS_MAX_EXT_OCHN_NUM; ext_chn_id++) {
            if (vpss_cfg->pipe_info[pipe_id].ext_chn_info[ext_chn_id].ext_chn_en == XMEDIA_FALSE) {
                continue;
            }
            ext_chn_info = &vpss_cfg->pipe_info[pipe_id].ext_chn_info[ext_chn_id];
            CHECK_RET(xmedia_vpss_set_ext_ochn_config(pipe_id, ext_base_chn_id + ext_chn_id, &ext_chn_info->ext_chn), "set extchn attr");
        }
    }

    CHECK_RET(sample_comm_vpss_pipe_chn_start(vpss_cfg), "vpss module start");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_vpss_stop(sample_vpss_config *vpss_cfg)
{
    xmedia_s32 pipe_id;

    sample_comm_vpss_pipe_chn_stop(vpss_cfg);

    for (pipe_id = 0; pipe_id < VPSS_MAX_PIPE_NUM; pipe_id++) {
        if (vpss_cfg->pipe_info[pipe_id].pipe_en == XMEDIA_FALSE) {
            continue;
        }
        CHECK_RET(sample_comm_vpss_destory_pipe(pipe_id), "destroy pipe");
    }
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_vpss_init(xmedia_void)
{
    return xmedia_vpss_init();
}

xmedia_s32 sample_comm_vpss_exit(xmedia_void)
{
    return xmedia_vpss_exit();
}


