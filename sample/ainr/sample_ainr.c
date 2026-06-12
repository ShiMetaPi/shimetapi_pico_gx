/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "sample_comm.h"
#include "sample_comm_sys.h"
#include "sample_comm_vgs.h"
#include "sample_comm_vi.h"
#include "sample_comm_isp.h"
#include "sample_comm_vpss.h"
#include "sample_comm_venc.h"
#include "sample_comm_livestream.h"
#include "sample_comm_vo.h"
#include "sample_comm_pq_bin.h"

typedef struct {
    xmedia_bool is_compress;
    // sensor
    xmedia_s32 sensor_type;
    xmedia_u32 framerate;
    vi_sensor_info sensor_info;

    // isp
    sample_isp_param isp_param;
    xmedia_isp_config isp_config;

    // vi
    xmedia_s32 vi_dev[2]; // 两路
    xmedia_s32 vi_pipe[2]; // 两路
    xmedia_s32 vi_chn[2]; // 两路
    sample_vi_config vi_config;
    sample_comm_video_param video_param;

    // vpss
    xmedia_s32 vpss_pipe[2]; // 两路
    xmedia_s32 vpss_chnl;
    xmedia_s32 vpss_ichn;
    xmedia_s32 vpss_ochn[2];
    xmedia_video_size pic_size;
    sample_vpss_config vpss_config;
    xmedia_video_size vpss_ochn_size[VPSS_MAX_OCHN_NUM];
    xmedia_u32 wrap_buffer_line;

    // venc
    xmedia_s32 venc_chnl;
    xmedia_s32 venc_chn[4];
    xmedia_s32 venc_chn_cnt;
    sample_venc_config venc_config;

    xmedia_u32 ainr_route_num; // 1或2
    xmedia_sys_switch_config switch_config;
} sample_ainr_param;

static xmedia_bool g_ainr_force_exit = XMEDIA_FALSE;

static xmedia_void sample_ainr_usage(xmedia_char *args)
{
    printf("\n"
           "***************************************************************\n"
           "Usage: %s [scene_mode]\n"
           "scene_mode: \n"
           "    0:  单路线性ainr 15fps\n"
           "    1:  双路线性ainr 7fps (switch)\n"
           "    2:  双路线性ainr 7fps\n"
           "    3:  双路线性ainr 7fps (外置switch)"
           "stream_mode: VENC output \n"
           "e.g: ./sample_vio 0\n"
           "***************************************************************\n"
           "\n",
           args);
    return;
}

xmedia_void sample_ainr_handle_sig(xmedia_s32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo || SIGTERM == signo) {
        g_ainr_force_exit = XMEDIA_TRUE;
    }
}

static xmedia_s32 sample_ainr_other_mod_init(xmedia_void)
{
    xmedia_s32 ret;

    ret = sample_comm_vgs_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vgs_init failed!\n");
        return ret;
    }

    ret = sample_comm_vi_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vi_init failed!\n");
        sample_comm_vgs_exit();
        return ret;
    }

    ret = sample_comm_vpss_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vpss_init failed!\n");
        sample_comm_vgs_exit();
        sample_comm_vi_exit();
        return ret;
    }

    ret = sample_comm_vo_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vo_init failed!\n");
        sample_comm_vgs_exit();
        sample_comm_vi_exit();
        sample_comm_vpss_exit();
        return ret;
    }

    ret = sample_comm_venc_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_venc_init failed!\n");
        sample_comm_vgs_exit();
        sample_comm_vi_exit();
        sample_comm_vpss_exit();
        sample_comm_vo_exit();
        return ret;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_void sample_ainr_vb_pool_init(sample_ainr_param *ai_param, sample_sys_config *sys_config,
    xmedia_u32 ainr_route_num)
{
    xmedia_vb_base_info vb_base_info = { 0 };
    xmedia_vb_cal_cfg vb_cfg = { 0 };
    xmedia_u32 pool_index = ((ainr_route_num == 1) ? 0 : 4);
    xmedia_s32 blk_size = 0;
    xmedia_video_size pic_size = { 0 };

    // vicap 输出(npu输入)
    vb_base_info.width = ai_param->sensor_info.width;
    vb_base_info.height = ai_param->sensor_info.height;
    vb_base_info.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    vb_base_info.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_RAW;
    if (ainr_route_num == 1) {
        vb_base_info.bit_width = ai_param->sensor_info.bit_width;
    } else if (ainr_route_num == 2) {
        vb_base_info.bit_width = ai_param->sensor_info.bit_width;
    }
    SAMPLE_PRT("PIPE %d bit_width : %d \n", ainr_route_num - 1, vb_base_info.bit_width);

    vb_base_info.align = DEFAULT_ALIGN;
    vb_base_info.cmp_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;
    vb_base_info.ainr_attr.ainr_en = XMEDIA_TRUE;
    vb_base_info.ainr_attr.ainr_direction = XMEDIA_VB_AINR_DIRECTION_INPUT;
    xmedia_vb_get_buffer_config(&vb_base_info, &vb_cfg);
    sys_config->vb_conf.common_pool[pool_index].block_size = vb_cfg.vb_size;
    sys_config->vb_conf.common_pool[pool_index].block_cnt = 2;
    SAMPLE_PRT("PIPE %d : vi-npu 2VB, size : %d \n", ainr_route_num - 1, vb_cfg.vb_size);

    // npu 输出
    vb_base_info.ainr_attr.ainr_direction = XMEDIA_VB_AINR_DIRECTION_OUTPUT;
    vb_base_info.cmp_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;
    xmedia_vb_get_buffer_config(&vb_base_info, &vb_cfg);
    sys_config->vb_conf.common_pool[pool_index + 1].block_size = vb_cfg.vb_size;
    sys_config->vb_conf.common_pool[pool_index + 1].block_cnt = (ai_param->switch_config.multi_pipe_mode == XMEDIA_SYS_MULTI_PIPE_MODE_DUAL_SWITCH ? 1 : 1);
    SAMPLE_PRT("PIPE %d : npu output %dVB, size : %d \n", ainr_route_num - 1, sys_config->vb_conf.common_pool[pool_index + 1].block_cnt, vb_cfg.vb_size);

    // vpss 输出
    pic_size.width = ai_param->sensor_info.width;
    pic_size.height = ai_param->sensor_info.height;
    blk_size = sample_comm_sys_get_wrap_buffer_size(pic_size, XMEDIA_VIDEO_FMT_TILE_32x4, XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420,
                                                     XMEDIA_VIDEO_DATA_WIDTH_8, XMEDIA_VIDEO_COMPRESS_MODE_TILE_COMPACT, &ai_param->wrap_buffer_line);
    sys_config->vb_conf.common_pool[pool_index + 2].block_size = blk_size;
    sys_config->vb_conf.common_pool[pool_index + 2].block_cnt = (ai_param->switch_config.multi_pipe_mode == XMEDIA_SYS_MULTI_PIPE_MODE_DUAL_SWITCH ? 2 : 1);
    SAMPLE_PRT("PIPE %d : vpss-venc %dVB, size : %d \n", ainr_route_num - 1, sys_config->vb_conf.common_pool[pool_index + 2].block_cnt, blk_size);

    pic_size.width = 640;
    pic_size.height = 480;
    blk_size = sample_comm_sys_get_buffer_size(pic_size, XMEDIA_VIDEO_FMT_LINEAR, XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420,
                                                XMEDIA_VIDEO_DATA_WIDTH_8, XMEDIA_VIDEO_COMPRESS_MODE_NONE);
    sys_config->vb_conf.common_pool[pool_index + 3].block_size = blk_size;
    sys_config->vb_conf.common_pool[pool_index + 3].block_cnt = 2;
}

static xmedia_s32 sample_ainr_sys_prepare(sample_ainr_param *ai_param)
{
    xmedia_s32 ret;
    sample_sys_config sys_config = { 0 };
    xmedia_u32 i;
    // 系统模式，固定全离线
    for (i = 0; i < VI_MAX_PIPE_NUM; i++) {
        sys_config.sys_conf.pipe_mode[i].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
        sys_config.sys_conf.pipe_mode[i].viproc_vpss_mode = XMEDIA_WORK_MODE_ONLINE;
        sys_config.sys_conf.pipe_mode[i].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    }

    sys_config.compress_level = XMEDIA_SYS_COMPRESS_LEVEL_0;

    ret = sample_comm_sys_cmp_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("common sys failed !\n");
        return ret;
    }

    // vb 池子
    sys_config.vb_conf.max_pool_cnt = 30;
    sample_ainr_vb_pool_init(ai_param, &sys_config, 1);
    if (ai_param->ainr_route_num == 2) {
        if(ai_param->switch_config.multi_pipe_mode == XMEDIA_SYS_MULTI_PIPE_MODE_NORMAL) {
            sample_ainr_vb_pool_init(ai_param, &sys_config, 2);
        }
    }

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_comm_vb_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vb_init failed!\n");
        return ret;
    }

    if (ai_param->ainr_route_num == 2 && ai_param->switch_config.multi_pipe_mode == XMEDIA_SYS_MULTI_PIPE_MODE_DUAL_SWITCH) {
        // 双路开switch
        ret = xmedia_sys_set_switch_config(0, &ai_param->switch_config);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("xmedia_sys_set_switch_config failed!\n");
            sample_comm_sys_exit();
            return ret;
        }
    }

    ret = sample_ainr_other_mod_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_ainr_other_mod_init failed!\n");
        sample_comm_sys_exit();
        return ret;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_void sample_ainr_sys_unprepare()
{
    sample_comm_vgs_exit();
    sample_comm_vi_exit();
    sample_comm_vpss_exit();
    sample_comm_vo_exit();
    sample_comm_venc_exit();

    sample_comm_sys_exit();
}

static xmedia_s32 sample_ainr_isp_prepare(sample_ainr_param *ai_param)
{
    xmedia_s32 ret;
    sample_comm_npu_workspace_info work_space_info = {0};
    xmedia_s32 index = 0;

    index = sample_comm_isp_get_sensor_effect_index(g_ainr_model_list, AINR_LIST_NUM, ai_param->isp_param.isp_info[0].sensor_type);
    if(index == XMEDIA_FAILURE) {
        SAMPLE_PRT("get ainr model failed!\n");
        return XMEDIA_FAILURE;
    }

    ret = sample_comm_isp_init(&ai_param->isp_param, &ai_param->vi_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("isp init failed!\n");
        return ret;
    }

    work_space_info.npu_modle_num = 1;
    work_space_info.npu_model_path[0] = g_ainr_model_list[index].load_file;

    ret = sample_comm_sys_npu_acquire_workspace(&work_space_info);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("acquire workspace fail\n");
        return ret;
    }

    ret = sample_comm_isp_ainr_init(&ai_param->isp_param, &ai_param->vi_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("isp ainr init failed!\n");
        sample_comm_isp_exit(&ai_param->isp_param);
        return ret;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_void sample_ainr_isp_unprepare(sample_ainr_param *ai_param)
{
    sample_comm_isp_ainr_exit(&ai_param->isp_param);
    sample_comm_sys_npu_release_workspace();
    sample_comm_isp_exit(&ai_param->isp_param);
}

static xmedia_s32 sample_ainr_other_mod_run(sample_ainr_param *ai_param)
{
    xmedia_s32 ret;
    sample_vpss_config *vpss_config = &ai_param->vpss_config;
    xmedia_s32 vpss_pipe;
    xmedia_s32 vpss_chnl;
    xmedia_video_size pic_size = { 0 };
    xmedia_u32 i;

    pic_size.width = ai_param->sensor_info.width;
    pic_size.height = ai_param->sensor_info.height;

    // vpss config
    for (i = 0; i < ai_param->ainr_route_num; i++) {
        vpss_pipe = ai_param->vpss_pipe[i];
        ret = sample_comm_vpss_get_default_pipe_cfg(&vpss_config->pipe_info[vpss_pipe].pipe_config, pic_size,
            &ai_param->video_param);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get default pipe cfg failed !\n");
            return ret;
        }

        ai_param->vpss_ochn_size[0].width = ai_param->sensor_info.width;
        ai_param->vpss_ochn_size[0].height = ai_param->sensor_info.height;
        ai_param->vpss_ochn_size[1].width = 640;
        ai_param->vpss_ochn_size[1].height = 480;
        vpss_config->pipe_info[vpss_pipe].pipe_en = XMEDIA_TRUE;
        vpss_config->pipe_info[vpss_pipe].pipe_no = vpss_pipe;

        vpss_chnl = ai_param->vpss_ochn[0];
        vpss_config->pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
        vpss_config->pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_no = ai_param->vpss_ochn[0];
        vpss_config->pipe_info[vpss_pipe].chn_info[vpss_chnl].wrap_attr.buf_line = ai_param->wrap_buffer_line;
        vpss_config->pipe_info[vpss_pipe].chn_info[vpss_chnl].wrap_attr.enable = XMEDIA_TRUE;
        ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config->pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
                                                    ai_param->vpss_ochn_size[vpss_chnl], &ai_param->video_param);
        vpss_config->pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.cmp_mode = XMEDIA_VIDEO_COMPRESS_MODE_TILE_COMPACT;
        vpss_config->pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.video_fmt = XMEDIA_VIDEO_FMT_TILE_32x4;
        vpss_config->pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.depth = 0;
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get default pipe cfg failed !\n");
            return ret;
        }

        vpss_chnl = ai_param->vpss_ochn[1];
        vpss_config->pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
        vpss_config->pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_no = ai_param->vpss_ochn[1];
        ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config->pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
                                                    ai_param->vpss_ochn_size[vpss_chnl], &ai_param->video_param);
        vpss_config->pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.depth = 0;
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("get default pipe cfg failed !\n");
            return ret;
        }
    }

    // vpss start
    ret = sample_comm_vpss_start(vpss_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vpss start failed !\n");
        return ret;
    }

    for (i = 0; i < ai_param->ainr_route_num; i++) {
        ret = sample_comm_sys_vi_bind_vpss(ai_param->vi_pipe[i], ai_param->vi_chn[i], ai_param->vpss_pipe[i],
            ai_param->vpss_ichn);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vi bind vpss failed !\n");
            for (; i > 0; ) {
                i = i - 1;
                sample_comm_sys_vi_unbind_vpss(ai_param->vi_pipe[i], ai_param->vi_chn[i], ai_param->vpss_pipe[i],
                    ai_param->vpss_ichn);
            }
            goto FAIL0;
        }
    }

    // venc init
    app_liveserver_init();
    app_liveserver_start(ai_param->vpss_ochn_size, ai_param->venc_chn_cnt, ai_param->framerate);

    for (i = 0; i < ai_param->ainr_route_num; i++) {
        ai_param->venc_chnl = ai_param->venc_chn[i * 2];
        ai_param->venc_config.chn_info[ai_param->venc_chnl].venc_en = XMEDIA_TRUE;
        ai_param->venc_config.chn_info[ai_param->venc_chnl].venc_chn = ai_param->venc_chnl;
        ai_param->venc_config.chn_info[ai_param->venc_chnl].payload_type = PT_H265;
        ai_param->venc_config.chn_info[ai_param->venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
        sample_comm_venc_get_default_chn_info(ai_param->vpss_ochn_size[0], ai_param->framerate,
            &ai_param->venc_config.chn_info[ai_param->venc_chnl]);
        ai_param->venc_config.chn_info[ai_param->venc_chnl].rcn_ref_share_buf = XMEDIA_TRUE;

        ai_param->venc_chnl = ai_param->venc_chn[i * 2 + 1];
        ai_param->venc_config.chn_info[ai_param->venc_chnl].venc_en = XMEDIA_TRUE;
        ai_param->venc_config.chn_info[ai_param->venc_chnl].venc_chn = ai_param->venc_chnl;
        ai_param->venc_config.chn_info[ai_param->venc_chnl].payload_type = PT_H265;
        ai_param->venc_config.chn_info[ai_param->venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
        sample_comm_venc_get_default_chn_info(ai_param->vpss_ochn_size[1], ai_param->framerate,
            &ai_param->venc_config.chn_info[ai_param->venc_chnl]);
        ai_param->venc_config.chn_info[ai_param->venc_chnl].rcn_ref_share_buf = XMEDIA_TRUE;
    }

    ret = sample_comm_venc_start(&ai_param->venc_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start failed !\n");
        goto FAIL1;
    }

    for (i = 0; i < ai_param->ainr_route_num; i++) {
        xmedia_venc_chn_param chn_param;
        xmedia_venc_get_chn_param(ai_param->venc_chn[2 * i], &chn_param);
        chn_param.priority = 1;
        xmedia_venc_set_chn_param(ai_param->venc_chn[2 * i], &chn_param);

        ret = sample_comm_sys_vpss_bind_venc(ai_param->vpss_pipe[i], ai_param->vpss_ochn[0],
            ai_param->venc_chn[i * 2]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind venc failed !\n");
            for (; i > 0; ) {
                i = i - 1;
                sample_comm_sys_vpss_unbind_venc(ai_param->vpss_pipe[i], ai_param->vpss_ochn[0], ai_param->venc_chn[i * 2]);
                sample_comm_sys_vpss_unbind_venc(ai_param->vpss_pipe[i], ai_param->vpss_ochn[1], ai_param->venc_chn[i * 2 + 1]);
            }
            goto FAIL2;
        }

        ret = sample_comm_sys_vpss_bind_venc(ai_param->vpss_pipe[i], ai_param->vpss_ochn[1],
            ai_param->venc_chn[i * 2 + 1]);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vpss bind venc failed !\n");
            sample_comm_sys_vpss_unbind_venc(ai_param->vpss_pipe[i], ai_param->vpss_ochn[0], ai_param->venc_chn[i * 2]);
            for (; i > 0; i--) {
                i = i - 1;
                sample_comm_sys_vpss_unbind_venc(ai_param->vpss_pipe[i], ai_param->vpss_ochn[0], ai_param->venc_chn[i * 2]);
                sample_comm_sys_vpss_unbind_venc(ai_param->vpss_pipe[i], ai_param->vpss_ochn[1], ai_param->venc_chn[i * 2 + 1]);
            }
            goto FAIL2;
        }
    }

    ret = sample_comm_venc_start_get_stream(ai_param->venc_chn, ai_param->venc_chn_cnt, XMEDIA_FALSE,
        XMEDIA_FALSE, ".");
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start get stream failed !\n");
        goto FAIL3;
    }

    return XMEDIA_SUCCESS;
FAIL3:
    for (i = 0; i < ai_param->ainr_route_num; i++) {
        sample_comm_sys_vpss_unbind_venc(ai_param->vpss_pipe[i], ai_param->vpss_ochn[0], ai_param->venc_chn[i * 2]);
        sample_comm_sys_vpss_unbind_venc(ai_param->vpss_pipe[i], ai_param->vpss_ochn[1], ai_param->venc_chn[i * 2 + 1]);
    }
FAIL2:
    sample_comm_venc_stop(&ai_param->venc_config);
FAIL1:
    for (i = 0; i < ai_param->ainr_route_num; i++) {
        sample_comm_sys_vi_unbind_vpss(ai_param->vi_pipe[i], ai_param->vi_chn[i], ai_param->vpss_pipe[i],
            ai_param->vpss_ichn);
    }
FAIL0:
    sample_comm_vpss_stop(vpss_config);
    return ret;
}

static xmedia_s32 sample_ainr_process_run(sample_ainr_param *ai_param)
{
    xmedia_s32 ret;
    bin_module_info pq_info = {0};
    xmedia_s32 id = -1;

    pq_info.vi.enable = XMEDIA_TRUE;
    pq_info.mcf.enable = XMEDIA_FALSE;
    pq_info.vpss.enable = XMEDIA_FALSE;

    ret = sample_comm_vi_start(&ai_param->vi_config, &ai_param->video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("start vi failed!\n");
        return ret;
    }

    ret = sample_comm_isp_start(&ai_param->isp_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("start isp failed!\n");
        sample_comm_vi_stop(&ai_param->vi_config);
        return ret;
    }

    ret = sample_ainr_other_mod_run(ai_param);
    if (ret != XMEDIA_SUCCESS) {
        sample_comm_isp_stop(&ai_param->isp_param);
        sample_comm_vi_stop(&ai_param->vi_config);
        return ret;
    }

    id = sample_comm_isp_get_sensor_effect_index(g_ainr_pqbin_list, AINR_LIST_NUM, ai_param->isp_param.isp_info[0].sensor_type);
    if (id == XMEDIA_FAILURE) {
        SAMPLE_ERR("Can not found sensor0 pq.bin\n");
    } else {
        pq_info.vi.pipe = 0;
        ret = sample_comm_pq_bin_import(&pq_info, g_ainr_pqbin_list[id].load_file);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("isp load pq bin failed!\n");
        }
    }

    ret = sample_comm_isp_enable_fpn(ai_param->vi_pipe[0], XMEDIA_NULL);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("isp enable fpn failed!\n");
    }

    if(ai_param->ainr_route_num == 2) {
        id = sample_comm_isp_get_sensor_effect_index(g_ainr_pqbin_list, AINR_LIST_NUM, ai_param->isp_param.isp_info[1].sensor_type);
        if (id == XMEDIA_FAILURE) {
            SAMPLE_ERR("Can not found sensor1 pq.bin\n");
        } else {
            pq_info.vi.pipe = 1;
            ret = sample_comm_pq_bin_import(&pq_info, g_ainr_pqbin_list[id].load_file);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("isp1 load pq bin failed!\n");
            }
        }

        ret = sample_comm_isp_enable_fpn(ai_param->vi_pipe[1], XMEDIA_NULL);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("isp1 enable fpn failed!\n");
        }
    }

    return XMEDIA_SUCCESS;
}

static xmedia_void sample_ainr_process_exit(sample_ainr_param *ai_param)
{
    xmedia_u32 i;

    PAUSE(g_ainr_force_exit);
    sample_comm_venc_stop_get_stream();
    app_liveserver_stop();
    app_liveserver_deinit();

    for (i = 0; i < ai_param->ainr_route_num; i++) {
        sample_comm_sys_vpss_unbind_venc(ai_param->vpss_pipe[i], ai_param->vpss_ochn[1], ai_param->venc_chn[i * 2 + 1]);
        sample_comm_sys_vpss_unbind_venc(ai_param->vpss_pipe[i], ai_param->vpss_ochn[0], ai_param->venc_chn[i * 2]);
    }

    sample_comm_venc_stop(&ai_param->venc_config);
    sample_comm_venc_exit();

    for (i = 0; i < ai_param->ainr_route_num; i++) {
        sample_comm_sys_vi_unbind_vpss(ai_param->vi_pipe[i], ai_param->vi_chn[i], ai_param->vpss_pipe[i],
            ai_param->vpss_ichn);
    }
    sample_comm_vpss_stop(&ai_param->vpss_config);
    sample_comm_vpss_exit();
    sample_comm_isp_stop(&ai_param->isp_param);
    sample_comm_vi_stop(&ai_param->vi_config);
    sample_comm_isp_disable_fpn(ai_param->vi_pipe[0]);
    if(ai_param->ainr_route_num == 2) {
        sample_comm_isp_disable_fpn(ai_param->vi_pipe[1]);
    }

    sample_comm_isp_ainr_exit(&ai_param->isp_param);
    sample_comm_sys_npu_release_workspace();
    sample_comm_isp_exit(&ai_param->isp_param);
    sample_comm_vi_exit();
    sample_comm_vgs_exit();
    sample_comm_sys_exit();
}

static xmedia_void sample_ainr_single_linear_init(sample_ainr_param *ai_param, xmedia_u32 index)
{
    xmedia_s32 vi_dev = ai_param->vi_dev[index];
    xmedia_s32 vi_pipe = ai_param->vi_pipe[index];
    xmedia_s32 vi_chn = ai_param->vi_chn[index];

    // 从sensor获取分辨率
    sample_comm_vi_get_sensor_info(ai_param->sensor_type, &ai_param->sensor_info);
    sample_comm_vi_get_framerate_by_sensor(ai_param->sensor_type, &ai_param->framerate);

    ai_param->isp_param.pipe[vi_pipe] = vi_pipe;
    ai_param->isp_param.isp_info[vi_pipe].isp_pipe_en = XMEDIA_TRUE;
    ai_param->isp_param.isp_info[vi_pipe].isp_sensor_en = XMEDIA_TRUE;
    ai_param->isp_param.isp_info[vi_pipe].sensor_type = ai_param->sensor_type;
    ai_param->isp_param.isp_info[vi_pipe].mirror = XMEDIA_FALSE;
    ai_param->isp_param.isp_info[vi_pipe].flip = XMEDIA_FALSE;
    ai_param->isp_param.isp_info[vi_pipe].disable_dynamic_fps = XMEDIA_TRUE;

    ai_param->isp_config.mode_config.work_mode = XMEDIA_ISP_WORK_MODE_MASTER;
    ai_param->isp_config.mode_config.master_mode.blend_stat_enable = XMEDIA_FALSE;
    ai_param->isp_config.mode_config.master_mode.slave_num = 0;
    ai_param->isp_config.pixel_fmt = ai_param->sensor_info.pixel_format;
    ai_param->isp_config.size.height = ai_param->sensor_info.height;
    ai_param->isp_config.size.width = ai_param->sensor_info.width;
    ai_param->isp_config.wdr_mode = ai_param->sensor_info.wdr_mode;
    memcpy(&(ai_param->isp_param.isp_info[vi_pipe].isp_config), &ai_param->isp_config, sizeof(xmedia_isp_config));

    ai_param->vi_config.dev_info[vi_dev].dev_en = XMEDIA_TRUE;
    ai_param->vi_config.dev_info[vi_dev].dev_no = vi_dev;
    ai_param->vi_config.dev_info[vi_dev].sensor_type = ai_param->sensor_type;

    ai_param->vi_config.pipe_info[vi_pipe].pipe_en = XMEDIA_TRUE;
    ai_param->vi_config.pipe_info[vi_pipe].pipe_no = vi_pipe;
    ai_param->vi_config.pipe_info[vi_pipe].chn_info[vi_chn].chn_en = XMEDIA_TRUE;
    ai_param->vi_config.pipe_info[vi_pipe].chn_info[vi_chn].chn_no = vi_chn;

    ai_param->vi_config.dev_bind_pipe[vi_dev].pipe[0] = vi_pipe;
    ai_param->vi_config.dev_bind_pipe[vi_dev].pipe[1] = -1;
    ai_param->vi_config.pipe_info[vi_pipe].pipe_config.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    ai_param->vi_config.pipe_info[vi_pipe].ainr_en = XMEDIA_TRUE;
    ai_param->vi_config.pipe_info[vi_pipe].ainr_model_bitwidth = ai_param->sensor_info.bit_width;

    // 如果使用双联sensor模组,自行修改i2c地址并使能dev_i2c_en
    ai_param->vi_config.dev_info[vi_dev].dev_i2c_en = XMEDIA_FALSE;
    if (ai_param->sensor_type == OMNIVISION_OS08A20_MIPI_4M_30FPS_10BIT ||
        ai_param->sensor_type == OMNIVISION_OS08A20_MIPI_8M_20FPS_10BIT) {
        // demo 08a20双联模组的i2c地址。
        ai_param->vi_config.dev_info[vi_dev].dev_i2c_addr = vi_dev == 2 ? 0x6c : 0x20;
    }

    // vpss/venc/vo
    ai_param->video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    ai_param->video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    ai_param->video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    ai_param->video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    ai_param->vpss_chnl = 0;
    ai_param->vpss_ichn = 0;
    ai_param->vpss_ochn[0] = 0;
    ai_param->vpss_ochn[1] = 1;

    ai_param->venc_chnl = 0;
    if (index == 0) {
        ai_param->venc_chn[0] = 0;
        ai_param->venc_chn[1] = 1;
        ai_param->venc_chn_cnt = 2;
    } else {
        ai_param->venc_chn[2] = 2;
        ai_param->venc_chn[3] = 3;
        ai_param->venc_chn_cnt = 4;
    }

    return;
}

static xmedia_void sample_ainr_double_linear_init(sample_ainr_param *ai_param)
{
    sample_ainr_single_linear_init(ai_param, 0);
    sample_ainr_single_linear_init(ai_param, 1);

    return;
}

static xmedia_s32 sample_ainr_single_linear(xmedia_void)
{
    xmedia_s32 ret;
    sample_ainr_param ai_param = {0};

    ai_param.sensor_type = SENSOR0_TYPE;
    ai_param.is_compress = XMEDIA_FALSE;
    ai_param.ainr_route_num = 1;
    ai_param.vi_dev[0] = 0;
    ai_param.vi_pipe[0] = 0;
    ai_param.vi_chn[0] = 0;
    ai_param.vpss_pipe[0] = 0;
    ai_param.isp_config.fps = 15;
    sample_ainr_single_linear_init(&ai_param, 0);

    ret = sample_ainr_sys_prepare(&ai_param);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    ret = sample_ainr_isp_prepare(&ai_param);
    if (ret != XMEDIA_SUCCESS) {
        sample_ainr_sys_unprepare();
        return ret;
    }

    ret = sample_ainr_process_run(&ai_param);
    if (ret != XMEDIA_SUCCESS) {
        sample_ainr_isp_unprepare(&ai_param);
        sample_ainr_sys_unprepare();
        return ret;
    }

    sample_ainr_process_exit(&ai_param);

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_ainr_double_linear_switch(xmedia_void)
{
    xmedia_s32 ret;
    sample_ainr_param ai_param = {0};

    ai_param.sensor_type = SENSOR0_TYPE;
    ai_param.is_compress = XMEDIA_FALSE;
    ai_param.ainr_route_num = 2;
    ai_param.vi_dev[0] = 0;
    ai_param.vi_pipe[0] = 0;
    ai_param.vi_chn[0] = 0;
    ai_param.vpss_pipe[0] = 0;
    ai_param.vi_dev[1] = 1;
    ai_param.vi_pipe[1] = 1;
    ai_param.vi_chn[1] = 0;
    ai_param.vpss_pipe[1] = 1;
    ai_param.isp_config.fps = 7;
    sample_ainr_double_linear_init(&ai_param);

    // switch init
    ai_param.switch_config.ext_switch_en = XMEDIA_FALSE;
    ai_param.switch_config.master_pipe_id = 0;
    ai_param.switch_config.multi_pipe_mode = XMEDIA_SYS_MULTI_PIPE_MODE_DUAL_SWITCH;
    ai_param.switch_config.pipe_id[0] = 0;
    ai_param.switch_config.pipe_id[1] = 1;
    ai_param.switch_config.pipe_num = 2;
    ai_param.switch_config.pipe_sel[0] = 0;
    ai_param.switch_config.pipe_sel[1] = 1;
    ai_param.switch_config.sync_mode = XMEDIA_SYS_SWITCH_SYNC_MODE_SOC_MASTER_SLAVE;
    ai_param.isp_param.isp_info[0].sns_work_mode = XMEDIA_SENSOR_WORK_MODE_NORMAL;
    ai_param.isp_param.isp_info[1].sns_work_mode = XMEDIA_SENSOR_WORK_MODE_SLAVE;
    ai_param.isp_param.isp_info[0].disable_dynamic_fps = XMEDIA_TRUE;
    ai_param.isp_param.isp_info[1].disable_dynamic_fps = XMEDIA_TRUE;

    ret = sample_ainr_sys_prepare(&ai_param);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    ret = sample_ainr_isp_prepare(&ai_param);
    if (ret != XMEDIA_SUCCESS) {
        sample_ainr_sys_unprepare();
        return ret;
    }

    ret = sample_ainr_process_run(&ai_param);
    if (ret != XMEDIA_SUCCESS) {
        sample_ainr_isp_unprepare(&ai_param);
        sample_ainr_sys_unprepare();
        return ret;
    }

    sample_ainr_process_exit(&ai_param);

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_ainr_double_linear(xmedia_void)
{
    xmedia_s32 ret;
    sample_ainr_param ai_param = {0};

    ai_param.sensor_type = SENSOR0_TYPE;
    ai_param.is_compress = XMEDIA_FALSE;
    ai_param.ainr_route_num = 2;
    ai_param.vi_dev[0] = 0;
    ai_param.vi_pipe[0] = 0;
    ai_param.vi_chn[0] = 0;
    ai_param.vpss_pipe[0] = 0;
    ai_param.vi_dev[1] = 1;
    ai_param.vi_pipe[1] = 1;
    ai_param.vi_chn[1] = 0;
    ai_param.vpss_pipe[1] = 1;
    ai_param.isp_config.fps = 7;
    sample_ainr_double_linear_init(&ai_param);

    ai_param.isp_param.isp_info[0].disable_dynamic_fps = XMEDIA_TRUE;
    ai_param.isp_param.isp_info[1].disable_dynamic_fps = XMEDIA_TRUE;

    ret = sample_ainr_sys_prepare(&ai_param);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    ret = sample_ainr_isp_prepare(&ai_param);
    if (ret != XMEDIA_SUCCESS) {
        sample_ainr_sys_unprepare();
        return ret;
    }

    ret = sample_ainr_process_run(&ai_param);
    if (ret != XMEDIA_SUCCESS) {
        sample_ainr_isp_unprepare(&ai_param);
        sample_ainr_sys_unprepare();
        return ret;
    }

    sample_ainr_process_exit(&ai_param);

    return XMEDIA_SUCCESS;
}

static xmedia_s32 sample_ainr_double_linear_ext_switch(xmedia_void)
{
    xmedia_s32 ret;
    sample_ainr_param ai_param = {0};

    ai_param.sensor_type = SENSOR0_TYPE;
    ai_param.is_compress = XMEDIA_FALSE;
    ai_param.ainr_route_num = 2;
    ai_param.vi_dev[0] = 0;
    ai_param.vi_pipe[0] = 0;
    ai_param.vi_chn[0] = 0;
    ai_param.vpss_pipe[0] = 0;
    ai_param.vi_dev[1] = 0;
    ai_param.vi_pipe[1] = 1;
    ai_param.vi_chn[1] = 0;
    ai_param.vpss_pipe[1] = 1;
    ai_param.isp_config.fps = 7;
    sample_ainr_double_linear_init(&ai_param);

    // 外置switch,绑定关系特殊处理
    ai_param.vi_config.dev_bind_pipe[0].pipe[0] = 0;
    ai_param.vi_config.dev_bind_pipe[0].pipe[1] = 1;

    // ext switch init
    ai_param.switch_config.ext_switch_en = XMEDIA_TRUE;
    ai_param.switch_config.master_pipe_id = 0;
    ai_param.switch_config.multi_pipe_mode = XMEDIA_SYS_MULTI_PIPE_MODE_DUAL_SWITCH;
    ai_param.switch_config.pipe_id[0] = 0;
    ai_param.switch_config.pipe_id[1] = 1;
    ai_param.switch_config.pipe_num = 2;
    ai_param.switch_config.pipe_sel[0] = 0;
    ai_param.switch_config.pipe_sel[1] = 1;
    ai_param.switch_config.sync_mode = XMEDIA_SYS_SWITCH_SYNC_MODE_SOC_MASTER_SLAVE;
    ai_param.isp_param.isp_info[0].sns_work_mode = XMEDIA_SENSOR_WORK_MODE_NORMAL;
    ai_param.isp_param.isp_info[1].sns_work_mode = XMEDIA_SENSOR_WORK_MODE_SLAVE;
    ai_param.isp_param.isp_info[0].disable_dynamic_fps = XMEDIA_TRUE;
    ai_param.isp_param.isp_info[1].disable_dynamic_fps = XMEDIA_TRUE;

    ai_param.isp_param.isp_info[1].serdes_enable = XMEDIA_TRUE;
    ai_param.isp_param.isp_info[1].i2c_addr_en = XMEDIA_TRUE;
    ai_param.isp_param.isp_info[1].i2c_addr = 0X62;

    ret = sample_ainr_sys_prepare(&ai_param);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    ret = sample_ainr_isp_prepare(&ai_param);
    if (ret != XMEDIA_SUCCESS) {
        sample_ainr_sys_unprepare();
        return ret;
    }

    ret = sample_ainr_process_run(&ai_param);
    if (ret != XMEDIA_SUCCESS) {
        sample_ainr_isp_unprepare(&ai_param);
        sample_ainr_sys_unprepare();
        return ret;
    }

    sample_ainr_process_exit(&ai_param);

    return XMEDIA_SUCCESS;

}

int main(int argc, char **argv)
{
    xmedia_s32 scene_mode = 0;
    xmedia_s32 ret;

    signal(SIGINT, sample_ainr_handle_sig);
    signal(SIGTERM, sample_ainr_handle_sig);

    if (argc == 2) {
        if (!strncmp(argv[1], "-h", 2)) {
            sample_ainr_usage(argv[0]);
            return 0;
        }
    } else {
        sample_ainr_usage(argv[0]);
        return 0;
    }

    scene_mode = atoi(argv[1]);

    switch (scene_mode) {
        case 0:
            ret = sample_ainr_single_linear();
            break;
        case 1:
            ret = sample_ainr_double_linear_switch();
            break;
        case 2:
            ret = sample_ainr_double_linear();
            break;
        case 3:
            ret = sample_ainr_double_linear_ext_switch();
            break;
        default:
            sample_ainr_usage(argv[0]);
            return 0;
    }

    if (ret == XMEDIA_SUCCESS) {
        SAMPLE_PRT("program exit normally!\n");
    } else {
        SAMPLE_PRT("program exit abnormally!\n");
    }

    return 0;
}

