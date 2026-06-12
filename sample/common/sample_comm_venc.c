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
#include <pthread.h>
#include <errno.h>
#include <stdint.h>
#include <sys/prctl.h>

#include "math_fun.h"
#include "xmedia_type.h"
#include "xmedia_debug.h"
#include "xmedia_vgs.h"
#include "sample_comm.h"
#include "sample_comm_sys.h"
#include "sample_comm_venc.h"
#include "xcam_livestream_rtsp_server.h"
#include "xcam_liveserver.h"

static sample_venc_getstream_para g_get_stream_param;
static pthread_t g_venc_pid;
static pthread_t g_venc_qpmap_pid;
static sample_venc_qpmap_sendfrm_para g_qpmap_sendfrm_para;
xmedia_bool g_user_start = XMEDIA_FALSE;

xmedia_s32 sample_comm_venc_get_file_post_fix(xmedia_payload_type payload, xmedia_char *file_post_fix)
{
    xmedia_char *post_fix = XMEDIA_NULL;

    switch(payload) {
        case PT_H264:
            post_fix = ".h264";
        break;
        case PT_H265:
            post_fix = ".h265";
        break;
        case PT_JPEG:
            post_fix = ".jpg";
        break;
        case PT_MJPEG:
            post_fix = ".mjp";
        break;
        case PT_PRORES:
            post_fix = ".prores";
        break;
        default:
            printf("Payload type err!\n");
        return XMEDIA_FAILURE;
    }

    memcpy(file_post_fix, post_fix, strlen(post_fix) + 1);

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_venc_save_stream(FILE* pFd, xmedia_venc_stream* pstStream)
{
    xmedia_s32 i;

    for (i = 0; i < pstStream->pack_count; i++) {
        fwrite(pstStream->pack[i].vir_addr + pstStream->pack[i].offset,
               pstStream->pack[i].len - pstStream->pack[i].offset, 1, pFd);
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_venc_save_jpeg_stream(xmedia_s32 chn,FILE* fd, xmedia_venc_stream* pst_stream , xmedia_u32 recv_cnt, char *save_path)
{
    xmedia_s32 i = 0;
    xmedia_u32 pack_cnt;

    if (pst_stream->pack_count == 1) {
        pack_cnt = 1;
    } else {
        pack_cnt = 2;
    }
    for (i = 0; i < pack_cnt; i++) {
        fwrite(pst_stream->pack[i].vir_addr + pst_stream->pack[i].offset,
            pst_stream->pack[i].len - pst_stream->pack[i].offset, 1, fd);
        fflush(fd);
    }

    if (pst_stream->pack_count > 2) {
        /* save mpf */
        for (i = 2; i < pst_stream->pack_count; i += 2) {
            char mpf_file[128] = {0};
            xmedia_u32 mpf_cnt = i / 2 - 1;
            snprintf(mpf_file, 128, "%s/chn%d_snap_%d_mpf%d.jpg", save_path, chn, recv_cnt, mpf_cnt);
            FILE* file_mpf = NULL;
            file_mpf = fopen(mpf_file, "wb");
            if (file_mpf == NULL) {
                SAMPLE_PRT("open file %s err\n", mpf_file);
                return XMEDIA_FAILURE;
            }
            chmod(mpf_file, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
            fwrite(pst_stream->pack[i].vir_addr + pst_stream->pack[i].offset,
                   pst_stream->pack[i].len - pst_stream->pack[i].offset, 1, file_mpf);
            fflush(file_mpf);
            fwrite(pst_stream->pack[i+1].vir_addr + pst_stream->pack[i+1].offset,
                   pst_stream->pack[i+1].len - pst_stream->pack[i+1].offset, 1, file_mpf);
            fflush(file_mpf);
            fclose(file_mpf);
            SAMPLE_PRT("save large thumb_nail:%s ok\n", mpf_file);
        }
    }

    i--;
    if ((pst_stream->pack[i].data_type.jpege_type == JPEGE_PACK_HEAD) ||
        (pst_stream->pack[i].data_type.jpege_type == JPEGE_PACK_ECS)) {
        /* When jpeg dividing slice or venc_attr.by_frame = XMEDIA_FALSE,
         * do not close cur file until the last pack of a frame is received
         */
        return XMEDIA_FAILURE;
    } else if (pst_stream->pack[i].data_type.jpege_type < JPEGE_PACK_MAX) {
        return XMEDIA_SUCCESS;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_venc_save_jpeg_thum(xmedia_char *result_path, xmedia_venc_stream* stream_pack, xmedia_u32 chn, xmedia_u32 recv_cnt)
{
    if (stream_pack->pack[0].data_num == 1) {
        char dcf_file[128] = {0};
        snprintf(dcf_file, 128, "%s/chn%d_snap_thumb_%d.jpg", result_path, chn, recv_cnt);
        FILE* file_dcf = NULL;
        file_dcf = fopen(dcf_file, "wb");
        if (file_dcf == NULL) {
            SAMPLE_PRT("open dcf file %s err\n", dcf_file);
            return XMEDIA_FAILURE;
        }
        chmod(dcf_file, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        fwrite(stream_pack->pack[0].vir_addr + stream_pack->pack[0].pack_info[0].pack_offset,
                stream_pack->pack[0].pack_info[0].pack_length, 1, file_dcf);
        fflush(file_dcf);
        SAMPLE_PRT("save thumb_nail:%s ok\n", dcf_file);
    } else {
        SAMPLE_PRT("stream pack not find dcf!\n");
        return XMEDIA_FAILURE;
    }
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_venc_start_chn(xmedia_s32 venc_chn)
{
    xmedia_s32 ret;
    xmedia_venc_recv_pic_param  recv_param;

    recv_param.recv_pic_num = -1;

    ret = xmedia_venc_start_recv_frame(venc_chn, &recv_param);
    CHECK_RET(ret,"start recv frame");

    return ret;
}

xmedia_s32 sample_comm_venc_stop_chn(xmedia_s32 venc_chn)
{
    xmedia_s32 ret;

    ret = xmedia_venc_stop_recv_frame(venc_chn);
    CHECK_RET(ret,"stop recv frame");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_venc_get_default_gop_attr(xmedia_venc_gop_mode gop_mode, xmedia_venc_gop_attr *gop_attr)
{
    switch(gop_mode) {
        case VENC_GOPMODE_NORMALP:
            gop_attr->gop_mode  = VENC_GOPMODE_NORMALP;
            gop_attr->normal_p.ip_qp_delta = 0;
            break;
        case VENC_GOPMODE_SMARTP:
        case VENC_GOPMODE_ADVSMARTP:
            gop_attr->gop_mode  = VENC_GOPMODE_SMARTP;
            gop_attr->smart_p.bg_qp_delta  = 0;
            gop_attr->smart_p.vi_qp_delta  = 0;
            gop_attr->smart_p.bg_interval =  90;
            break;
        default:
            SAMPLE_PRT("not support the gop mode !\n");
            return XMEDIA_FAILURE;
    }
    return XMEDIA_SUCCESS;
}

xmedia_void sample_comm_venc_get_default_chn_info(xmedia_video_size res,xmedia_u32 framerate,sample_venc_chn_info *chn_info)
{
    chn_info->width  = res.width;
    chn_info->height = res.height;
    chn_info->profile = (chn_info->payload_type == PT_H264) ? 2 : 0;
    chn_info->use_default_gop_attr = XMEDIA_TRUE;
    chn_info->rcn_ref_share_buf = XMEDIA_TRUE;
    chn_info->src_frame_rate = framerate;
    chn_info->dst_frame_rate = framerate;

    if (PT_JPEG == chn_info->payload_type) {
        chn_info->jpeg_attr.support_dcf = chn_info->support_dcf;
        chn_info->jpeg_attr.mpf_cfg.large_thumbnail_num = chn_info->mpf_cnt;
        chn_info->jpeg_attr.mpf_cfg.large_thumbnail_size[0].width = 1280;
        chn_info->jpeg_attr.mpf_cfg.large_thumbnail_size[0].height = 720;
        chn_info->jpeg_attr.mpf_cfg.large_thumbnail_size[1].width = 640;
        chn_info->jpeg_attr.mpf_cfg.large_thumbnail_size[1].height = 480;
        chn_info->jpeg_attr.rcv_mode = VENC_PIC_RECEIVE_SINGLE;
    }
}

xmedia_s32 sample_comm_venc_set_rc_param(xmedia_s32 venc_chn)
{
    xmedia_s32 ret;
    xmedia_venc_rc_param rc_param;
    xmedia_venc_chn_attr chn_attr;

    ret = xmedia_venc_get_chn_attr(venc_chn,&chn_attr);
    CHECK_RET(ret,"get venc chn attr");

    if (chn_attr.venc_attr.en_type != PT_JPEG) {
        ret = xmedia_venc_get_rc_param(venc_chn,&rc_param);
        CHECK_RET(ret,"get venc rc param");
    }

    if (VENC_RC_MODE_H264CBR == chn_attr.rc_attr.rc_mode) {
        rc_param.param_h264_cbr.max_re_enc_times = 0;
    } else if (VENC_RC_MODE_H264VBR == chn_attr.rc_attr.rc_mode) {
        rc_param.param_h264_vbr.max_re_enc_times = 0;
    } else if (VENC_RC_MODE_H265CBR == chn_attr.rc_attr.rc_mode) {
        rc_param.param_h265_cbr.max_re_enc_times = 0;
    } else if (VENC_RC_MODE_H265VBR == chn_attr.rc_attr.rc_mode) {
        rc_param.param_h265_vbr.max_re_enc_times = 0;
    } else {
        return XMEDIA_SUCCESS;
    }
    ret = xmedia_venc_set_rc_param(venc_chn,&rc_param);
    CHECK_RET(ret,"set venc rc param");
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_venc_create_chn(sample_venc_chn_info *chn_param)
{
    xmedia_s32 ret = 0;
    xmedia_venc_chn_attr venc_attr;
    xmedia_u32 framerate;
    xmedia_u32 stat_time;
    xmedia_u32 gop = chn_param->dst_frame_rate;
    pic_size_e pic_size;
    xmedia_u32 ext_size = 0;

    memset(&venc_attr, 0, sizeof(xmedia_venc_chn_attr));

    venc_attr.venc_attr.en_type = chn_param->payload_type;
    venc_attr.venc_attr.max_pic_width = chn_param->width;
    venc_attr.venc_attr.max_pic_height = chn_param->height;
    venc_attr.venc_attr.pic_width = chn_param->width;
    venc_attr.venc_attr.pic_height = chn_param->height;
    venc_attr.venc_attr.profile = chn_param->profile;
    venc_attr.venc_attr.by_frame = XMEDIA_TRUE;

    if (venc_attr.venc_attr.en_type == PT_JPEG) {
        if (chn_param->jpeg_attr.mpf_cfg.large_thumbnail_num > 0) {
            ext_size = ALIGN_UP(chn_param->jpeg_attr.mpf_cfg.large_thumbnail_size[0].width, 16) * \
                       ALIGN_UP(chn_param->jpeg_attr.mpf_cfg.large_thumbnail_size[0].height, 16);
        }
        if (chn_param->jpeg_attr.mpf_cfg.large_thumbnail_num == 2) {
            ext_size += ALIGN_UP(chn_param->jpeg_attr.mpf_cfg.large_thumbnail_size[1].width, 16) * \
                        ALIGN_UP(chn_param->jpeg_attr.mpf_cfg.large_thumbnail_size[1].height, 16);
        }
    }

    if (chn_param->str_buffer_size != 0) {
        venc_attr.venc_attr.stream_buf_size = chn_param->str_buffer_size;
    } else {
        if (venc_attr.venc_attr.en_type == PT_MJPEG || venc_attr.venc_attr.en_type == PT_JPEG) {
            venc_attr.venc_attr.stream_buf_size      = ALIGN_UP(venc_attr.venc_attr.pic_width, 16) * ALIGN_UP(venc_attr.venc_attr.pic_height, 16) * 2 + ext_size;
        } else {
            venc_attr.venc_attr.stream_buf_size      = ALIGN_UP(venc_attr.venc_attr.pic_width * venc_attr.venc_attr.pic_height * 3 / 8, 64);
        }
    }

    if (chn_param->use_default_gop_attr == XMEDIA_TRUE) {
        ret = sample_comm_venc_get_default_gop_attr(chn_param->venc_gop_attr.gop_mode, &chn_param->venc_gop_attr);
        if (ret != XMEDIA_SUCCESS) {

            SAMPLE_PRT("get gop attr failed !\n");
            return ret;
        }
    }

    if (VENC_GOPMODE_SMARTP == chn_param->venc_gop_attr.gop_mode) {
        stat_time = chn_param->venc_gop_attr.smart_p.bg_interval/gop;
    } else {
        stat_time = 2;
    }

    pic_size = sample_comm_sys_get_picszie_by_wxh(chn_param->width, chn_param->height);
    framerate = chn_param->dst_frame_rate;

    switch (venc_attr.venc_attr.en_type) {
        case PT_H265: {
                if (VENC_RC_MODE_H265CBR == chn_param->rc_mode) {
                    xmedia_venc_h265_cbr    h265_cbr;

                    venc_attr.rc_attr.rc_mode = VENC_RC_MODE_H265CBR;
                    h265_cbr.gop            = gop;
                    h265_cbr.stat_time       = stat_time; /* stream rate statics time(s) */
                    h265_cbr.src_frame_rate   = chn_param->src_frame_rate;
                    h265_cbr.dst_frame_rate  =  chn_param->dst_frame_rate; /* target frame rate */
                    switch (pic_size) {
                        case PIC_VGA:
                        case PIC_720P:
                            h265_cbr.bit_rate = (1024 * 1 * framerate)/30;
                            break;
                        case PIC_1080P:
                        case PIC_2304x1296: //300W
                            h265_cbr.bit_rate = (1024 * 2 * framerate)/30;
                            break;
                        case PIC_2560x1440: //360W
                        case PIC_2688x1520: //400W
                            h265_cbr.bit_rate = (1024 * 3 * framerate)/30;
                            break;
                        case PIC_2880x1620: //460W
                        case PIC_2560x1920: //500W
                        case PIC_2592x1944: //500W
                        case PIC_2688x1944: //520W
                        case PIC_3072x1728: //530W
                            h265_cbr.bit_rate = (1024 * 4 * framerate)/30;
                            break;
                        case PIC_3840x2160: //800W
                            h265_cbr.bit_rate = (1024 * 8 * framerate)/30;
                            break;
                        default : //other small resolution
                            h265_cbr.bit_rate = 1024 * 2;
                            break;
                    }
                    memcpy(&venc_attr.rc_attr.h265_cbr, &h265_cbr, sizeof(xmedia_venc_h265_cbr));
                } else if (VENC_RC_MODE_H265FIXQP == chn_param->rc_mode) {
                    xmedia_venc_h265_fixqp    h265_fix_qp;

                    venc_attr.rc_attr.rc_mode = VENC_RC_MODE_H265FIXQP;
                    h265_fix_qp.gop              = 30;
                    h265_fix_qp.src_frame_rate     = chn_param->src_frame_rate;
                    h265_fix_qp.dst_frame_rate    = chn_param->dst_frame_rate;
                    h265_fix_qp.i_qp              = 25;
                    h265_fix_qp.p_qp              = 30;
                    h265_fix_qp.b_qp              = 32;
                    memcpy(&venc_attr.rc_attr.h265_fixqp, &h265_fix_qp, sizeof(xmedia_venc_h265_fixqp));
                } else if (VENC_RC_MODE_H265VBR == chn_param->rc_mode) {
                    xmedia_venc_h265_vbr    h265_vbr;

                    venc_attr.rc_attr.rc_mode = VENC_RC_MODE_H265VBR;
                    h265_vbr.gop           = gop;
                    h265_vbr.stat_time      = stat_time;
                    h265_vbr.src_frame_rate  = chn_param->src_frame_rate;
                    h265_vbr.dst_frame_rate = chn_param->dst_frame_rate;
                    switch (pic_size) {
                        case PIC_VGA:
                        case PIC_720P:
                            h265_vbr.max_bit_rate = (1024 * 1 * framerate)/30;
                            break;
                        case PIC_1080P:
                        case PIC_2304x1296:
                            h265_vbr.max_bit_rate = (1024 * 2 * framerate)/30;
                            break;
                        case PIC_2560x1440:
                        case PIC_2688x1520:
                            h265_vbr.max_bit_rate = (1024 * 3 * framerate)/30;
                            break;
                        case PIC_2880x1620:
                        case PIC_2560x1920:
                        case PIC_2592x1944:
                        case PIC_2688x1944:
                        case PIC_3072x1728:
                            h265_vbr.max_bit_rate = (1024 * 4 * framerate)/30;
                            break;
                        case PIC_3840x2160:
                            h265_vbr.max_bit_rate = (1024 * 8 * framerate)/30;
                            break;
                        default :
                            h265_vbr.max_bit_rate = 1024 * 2;
                            break;
                    }
                    memcpy(&venc_attr.rc_attr.h265_vbr, &h265_vbr, sizeof(xmedia_venc_h265_vbr));
                } else if (VENC_RC_MODE_H265AVBR == chn_param->rc_mode) {
                    xmedia_venc_h265_avbr    h265_avbr;

                    venc_attr.rc_attr.rc_mode = VENC_RC_MODE_H265AVBR;
                    h265_avbr.gop         = gop;
                    h265_avbr.stat_time    = stat_time;
                    h265_avbr.src_frame_rate  = chn_param->src_frame_rate;
                    h265_avbr.dst_frame_rate = chn_param->dst_frame_rate;
                    switch (pic_size) {
                        case PIC_VGA:
                        case PIC_720P:
                            h265_avbr.max_bit_rate = (1024 * 1 * framerate)/30;
                            break;
                        case PIC_1080P:
                        case PIC_2304x1296:
                            h265_avbr.max_bit_rate = (1024 * 2 * framerate)/30;
                            break;
                        case PIC_2560x1440:
                        case PIC_2688x1520:
                            h265_avbr.max_bit_rate = (1024 * 3 * framerate)/30;
                            break;
                        case PIC_2880x1620:
                        case PIC_2560x1920:
                        case PIC_2592x1944:
                        case PIC_2688x1944:
                        case PIC_3072x1728:
                            h265_avbr.max_bit_rate = (1024 * 4 * framerate)/30;
                            break;
                        case PIC_3840x2160:
                            h265_avbr.max_bit_rate = (1024 * 8 * framerate)/30;
                            break;
                        default :
                            h265_avbr.max_bit_rate = 1024 * 2;
                            break;
                    }
                    memcpy(&venc_attr.rc_attr.h265_avbr, &h265_avbr, sizeof(xmedia_venc_h265_avbr));
                }else if (VENC_RC_MODE_H265QPMAP == chn_param->rc_mode) {
                    xmedia_venc_h265_qpmap    h265_qpmap;

                    venc_attr.rc_attr.rc_mode = VENC_RC_MODE_H265QPMAP;
                    h265_qpmap.gop           = gop;
                    h265_qpmap.stat_time      = stat_time;
                    h265_qpmap.src_frame_rate  = chn_param->src_frame_rate;
                    h265_qpmap.dst_frame_rate = chn_param->dst_frame_rate;
                    memcpy(&venc_attr.rc_attr.h265_qpmap, &h265_qpmap, sizeof(xmedia_venc_h265_qpmap));
                } else {
                    SAMPLE_PRT("rc mode(%d) not support\n",chn_param->rc_mode);
                    return XMEDIA_FAILURE;
                }
                venc_attr.venc_attr.h264e_attr.rcn_ref_share_buf = chn_param->rcn_ref_share_buf;
            }
            break;
        case PT_H264: {
                if (VENC_RC_MODE_H264CBR == chn_param->rc_mode) {
                    xmedia_venc_h264_cbr    h264_cbr;

                    venc_attr.rc_attr.rc_mode = VENC_RC_MODE_H264CBR;
                    h264_cbr.gop                = gop; /*the interval of IFrame*/
                    h264_cbr.stat_time           = stat_time; /* stream rate statics time(s) */
                    h264_cbr.src_frame_rate       = chn_param->src_frame_rate; /* input (vi) frame rate */
                    h264_cbr.dst_frame_rate      = chn_param->dst_frame_rate; /* target frame rate */

                    switch (pic_size) {
                        case PIC_VGA:
                        case PIC_720P:
                            h264_cbr.bit_rate = (1024 * 1 * framerate)/30;
                            break;
                        case PIC_1080P:
                        case PIC_2304x1296:
                            h264_cbr.bit_rate = (1024 * 2 * framerate)/30;
                            break;
                        case PIC_2560x1440:
                        case PIC_2688x1520:
                            h264_cbr.bit_rate = (1024 * 3 * framerate)/30;
                            break;
                        case PIC_2880x1620:
                        case PIC_2560x1920:
                        case PIC_2592x1944:
                        case PIC_2688x1944:
                        case PIC_3072x1728:
                            h264_cbr.bit_rate = (1024 * 4 * framerate)/30;
                            break;
                        case PIC_3840x2160:
                            h264_cbr.bit_rate = (1024 * 8 * framerate)/30;
                            break;
                        default :
                            h264_cbr.bit_rate = 1024 * 2;
                            break;
                    }

                    memcpy(&venc_attr.rc_attr.h264_cbr, &h264_cbr, sizeof(xmedia_venc_h264_cbr));
                } else if (VENC_RC_MODE_H264FIXQP == chn_param->rc_mode) {
                    xmedia_venc_h264_fixqp    h264_fixqp;

                    venc_attr.rc_attr.rc_mode = VENC_RC_MODE_H264FIXQP;
                    h264_fixqp.gop            = 30;
                    h264_fixqp.src_frame_rate = chn_param->src_frame_rate;
                    h264_fixqp.dst_frame_rate = chn_param->dst_frame_rate;
                    h264_fixqp.i_qp           = 25;
                    h264_fixqp.p_qp           = 30;
                    h264_fixqp.b_qp           = 32;
                    memcpy(&venc_attr.rc_attr.h264_fixqp, &h264_fixqp, sizeof(xmedia_venc_h264_fixqp));
                } else if (VENC_RC_MODE_H264VBR == chn_param->rc_mode) {
                    xmedia_venc_h264_vbr    h264_vbr;

                    venc_attr.rc_attr.rc_mode = VENC_RC_MODE_H264VBR;
                    h264_vbr.gop           = gop;
                    h264_vbr.stat_time      = stat_time;
                    h264_vbr.src_frame_rate       = chn_param->src_frame_rate;
                    h264_vbr.dst_frame_rate      = chn_param->dst_frame_rate;
                    switch (pic_size) {
                        case PIC_VGA:
                        case PIC_720P:
                            h264_vbr.max_bit_rate = (1024 * 1 * framerate)/30;
                            break;
                        case PIC_1080P:
                        case PIC_2304x1296:
                            h264_vbr.max_bit_rate = (1024 * 2 * framerate)/30;
                            break;
                        case PIC_2560x1440:
                        case PIC_2688x1520:
                            h264_vbr.max_bit_rate = (1024 * 3 * framerate)/30;
                            break;
                        case PIC_2880x1620:
                        case PIC_2560x1920:
                        case PIC_2592x1944:
                        case PIC_2688x1944:
                        case PIC_3072x1728:
                            h264_vbr.max_bit_rate = (1024 * 4 * framerate)/30;
                            break;
                        case PIC_3840x2160:
                            h264_vbr.max_bit_rate = (1024 * 8 * framerate)/30;
                            break;
                        default :
                            h264_vbr.max_bit_rate = 1024 * 2;
                            break;
                    }
                    memcpy(&venc_attr.rc_attr.h264_vbr, &h264_vbr, sizeof(xmedia_venc_h264_vbr));
                } else if (VENC_RC_MODE_H264AVBR == chn_param->rc_mode) {
                    xmedia_venc_h264_avbr    h264_avbr;

                    venc_attr.rc_attr.rc_mode = VENC_RC_MODE_H264AVBR;
                    h264_avbr.gop           = gop;
                    h264_avbr.stat_time      = stat_time;
                    h264_avbr.src_frame_rate       = chn_param->src_frame_rate;
                    h264_avbr.dst_frame_rate      = chn_param->dst_frame_rate;
                    switch (pic_size) {
                        case PIC_VGA:
                        case PIC_720P:
                            h264_avbr.max_bit_rate = (1024 * 1 * framerate)/30;
                            break;
                        case PIC_1080P:
                        case PIC_2304x1296:
                            h264_avbr.max_bit_rate = (1024 * 2 * framerate)/30;
                            break;
                        case PIC_2560x1440:
                        case PIC_2688x1520:
                            h264_avbr.max_bit_rate = (1024 * 3 * framerate)/30;
                            break;
                        case PIC_2880x1620:
                        case PIC_2560x1920:
                        case PIC_2592x1944:
                        case PIC_2688x1944:
                        case PIC_3072x1728:
                            h264_avbr.max_bit_rate = (1024 * 4 * framerate)/30;
                            break;
                        case PIC_3840x2160:
                            h264_avbr.max_bit_rate = (1024 * 8 * framerate)/30;
                            break;
                        default :
                            h264_avbr.max_bit_rate = 1024 * 2;
                            break;
                    }
                    memcpy(&venc_attr.rc_attr.h264_avbr, &h264_avbr, sizeof(xmedia_venc_h264_avbr));
                } else if (VENC_RC_MODE_H264QPMAP == chn_param->rc_mode) {
                    xmedia_venc_h264_qpmap    h264_qpmap;

                    venc_attr.rc_attr.rc_mode = VENC_RC_MODE_H264QPMAP;
                    h264_qpmap.gop           = gop;
                    h264_qpmap.stat_time      = stat_time;
                    h264_qpmap.src_frame_rate       = chn_param->src_frame_rate;
                    h264_qpmap.dst_frame_rate      = chn_param->dst_frame_rate;
                    memcpy(&venc_attr.rc_attr.h264_qpmap, &h264_qpmap, sizeof(xmedia_venc_h264_qpmap));
                } else {
                    SAMPLE_PRT("rc mode(%d) not support\n",chn_param->rc_mode);
                    return XMEDIA_FAILURE;
                }
                venc_attr.venc_attr.h264e_attr.rcn_ref_share_buf = chn_param->rcn_ref_share_buf;
            }
            break;
        case PT_MJPEG: {
            if (VENC_RC_MODE_MJPEGFIXQP == chn_param->rc_mode) {
                xmedia_venc_mjpeg_fixqp mjpege_fixqp;

                venc_attr.rc_attr.rc_mode = VENC_RC_MODE_MJPEGFIXQP;
                mjpege_fixqp.q_factor        = 95;
                mjpege_fixqp.src_frame_rate    = chn_param->src_frame_rate;
                mjpege_fixqp.dst_frame_rate   = chn_param->dst_frame_rate;

                memcpy(&venc_attr.rc_attr.mjpeg_fixqp, &mjpege_fixqp,sizeof(xmedia_venc_mjpeg_fixqp));
            } else if (VENC_RC_MODE_MJPEGCBR == chn_param->rc_mode) {
                xmedia_venc_mjpeg_cbr mjpeg_cbr;

                venc_attr.rc_attr.rc_mode = VENC_RC_MODE_MJPEGCBR;
                mjpeg_cbr.stat_time      = stat_time;
                mjpeg_cbr.src_frame_rate       = chn_param->src_frame_rate;
                mjpeg_cbr.dst_frame_rate      = chn_param->dst_frame_rate;
                switch (pic_size) {
                    case PIC_VGA:
                        mjpeg_cbr.bit_rate = (1024 * 4 * framerate)/30;
                        break;
                    case PIC_720P:
                        mjpeg_cbr.bit_rate = (1024 * 6 * framerate)/30;
                        break;
                    case PIC_1080P:
                        mjpeg_cbr.bit_rate = (1024 * 10 * framerate)/30;
                        break;
                    case PIC_2560x1440:
                    case PIC_2560x1920:
                        mjpeg_cbr.bit_rate = (1024 * 17 * framerate)/30;
                        break;
                    case PIC_2688x1520:
                    case PIC_2592x1944:
                    case PIC_2688x1944:
                        mjpeg_cbr.bit_rate = (1024 * 23 * framerate)/30;
                        break;
                    case PIC_3840x2160:
                        mjpeg_cbr.bit_rate = (1024 * 30 * framerate)/30;
                        break;
                    case PIC_4000x3000:
                        mjpeg_cbr.bit_rate = (1024 * 35 * framerate)/30;
                        break;
                    case PIC_7680x4320:
                        mjpeg_cbr.bit_rate = (1024 * 40 * framerate)/30;
                        break;
                    default :
                        mjpeg_cbr.bit_rate = (1024 * 20 * framerate)/30;
                        break;
                }

                memcpy(&venc_attr.rc_attr.mjpeg_cbr, &mjpeg_cbr,sizeof(xmedia_venc_mjpeg_cbr));
            } else if (VENC_RC_MODE_MJPEGVBR == chn_param->rc_mode) {
                xmedia_venc_mjpeg_vbr   mjpeg_vbr;

                venc_attr.rc_attr.rc_mode = VENC_RC_MODE_MJPEGVBR;
                mjpeg_vbr.stat_time      = stat_time;
                mjpeg_vbr.src_frame_rate  = chn_param->src_frame_rate;
                mjpeg_vbr.dst_frame_rate = 5;

                switch (pic_size) {
                    case PIC_VGA:
                        mjpeg_vbr.max_bit_rate = (1024 * 4 * framerate)/30;
                        break;
                    case PIC_720P:
                        mjpeg_vbr.max_bit_rate = (1024 * 6 * framerate)/30;
                        break;
                    case PIC_1080P:
                        mjpeg_vbr.max_bit_rate = (1024 * 10 * framerate)/30;
                        break;
                    case PIC_2560x1440:
                    case PIC_2560x1920:
                        mjpeg_vbr.max_bit_rate = (1024 * 17 * framerate)/30;
                        break;
                    case PIC_2688x1520:
                    case PIC_2592x1944:
                    case PIC_2688x1944:
                        mjpeg_vbr.max_bit_rate = (1024 * 23 * framerate)/30;
                        break;
                    case PIC_3840x2160:
                        mjpeg_vbr.max_bit_rate = (1024 * 30 * framerate)/30;
                        break;
                    case PIC_4000x3000:
                        mjpeg_vbr.max_bit_rate = (1024 * 35 * framerate)/30;
                        break;
                    case PIC_7680x4320:
                        mjpeg_vbr.max_bit_rate = (1024 * 40 * framerate)/30;
                        break;
                    default :
                        mjpeg_vbr.max_bit_rate = (1024 * 20 * framerate)/30;
                        break;
                }

                memcpy(&venc_attr.rc_attr.mjpeg_vbr, &mjpeg_vbr,sizeof(xmedia_venc_mjpeg_vbr));
            } else {
                SAMPLE_PRT("cann't support other mode(%d) in this version!\n",chn_param->rc_mode);
                return XMEDIA_FAILURE;
            }
        }
        break;

        case PT_JPEG:
            memcpy(&venc_attr.venc_attr.jpege_attr, &chn_param->jpeg_attr, sizeof(xmedia_venc_attr_jpeg));
            if (chn_param->jpeg_slice.slice_enable) {
                venc_attr.venc_attr.by_frame = chn_param->jpeg_slice.by_frame;
            }
            break;
        default:
            SAMPLE_PRT("not support the payload type !\n");
            return XMEDIA_FAILURE;
    }

    if (PT_MJPEG == chn_param->payload_type || PT_JPEG == chn_param->payload_type ) {
        venc_attr.gop_attr.gop_mode  = VENC_GOPMODE_NORMALP;
        venc_attr.gop_attr.normal_p.ip_qp_delta = 0;
    } else {
        memcpy(&venc_attr.gop_attr, &chn_param->venc_gop_attr,  sizeof(xmedia_venc_gop_attr));

        if ((VENC_RC_MODE_H264QPMAP == venc_attr.rc_attr.rc_mode)||(VENC_RC_MODE_H265QPMAP == venc_attr.rc_attr.rc_mode)) {
            if (VENC_GOPMODE_ADVSMARTP == chn_param->venc_gop_attr.gop_mode) {
                venc_attr.gop_attr.gop_mode = VENC_GOPMODE_SMARTP;

                SAMPLE_PRT("advsmartp not support QPMAP, so change gopmode to smartp!\n");
            }
        }
    }

    CHECK_RET(xmedia_venc_create_chn(chn_param->venc_chn, &venc_attr), "create venc chn");

    CHECK_RET(sample_comm_venc_set_rc_param(chn_param->venc_chn), "reset reencode times");

#if 0
    // For improve H.264 performance
    xmedia_venc_search_window search_window;
    search_window.ime_search = VENC_IME_SEARCH_WINDOW_W8_H8;
    search_window.pme_search = VENC_PME_SEARCH_WINDOW_128_96 ;
    CHECK_RET(xmedia_venc_set_search_window(chn_param->venc_chn, &search_window), "set search window");
#endif

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_venc_set_fps_to_stream(sample_venc_chn_info *chn_param)
{
    xmedia_venc_h264_vui h264_vui = {0};
    xmedia_venc_h265_vui h265_vui = {0};

    if ((chn_param->venc_en == XMEDIA_TRUE) && (chn_param->vui_info.vui_en == XMEDIA_TRUE)) {
        if (chn_param->payload_type == PT_H264) {
            CHECK_RET(xmedia_venc_get_h264_vui(chn_param->venc_chn, &h264_vui), "get h264 vui");
            h264_vui.vui_time_info.timing_info_present_flag = 1;
            h264_vui.vui_time_info.num_units_in_tick = 1;
            h264_vui.vui_time_info.time_scale = chn_param->vui_info.stream_display_fps * 2;
            CHECK_RET(xmedia_venc_set_h264_vui(chn_param->venc_chn, &h264_vui), "set h264 vui");
        } else if (chn_param->payload_type == PT_H265) {
            CHECK_RET(xmedia_venc_get_h265_vui(chn_param->venc_chn, &h265_vui), "get h265 vui");
            h265_vui.vui_time_info.timing_info_present_flag = 1;
            h265_vui.vui_time_info.num_units_in_tick = 1;
            h265_vui.vui_time_info.time_scale = chn_param->vui_info.stream_display_fps;
            CHECK_RET(xmedia_venc_set_h265_vui(chn_param->venc_chn, &h265_vui), "set h265 vui");
        }
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_venc_destory_chn(xmedia_s32 venc_chn)
{
    CHECK_RET(xmedia_venc_destroy_chn(venc_chn), "destroy venc chn");

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_venc_start(sample_venc_config *venc_cfg)
{
    xmedia_s32 i;
    sample_venc_chn_info *chn_param = NULL;

    for (i = 0; i < VENC_MAX_CHN_NUM; i++) {
        chn_param = &venc_cfg->chn_info[i];

        if (chn_param->venc_en == XMEDIA_FALSE) {
            continue;
        }

        CHECK_RET(sample_comm_venc_create_chn(chn_param), "create venc chn");
        if (chn_param->payload_type == PT_JPEG) {
            continue;
        }

        sample_comm_venc_set_fps_to_stream(chn_param);

        CHECK_RET(sample_comm_venc_start_chn(chn_param->venc_chn), "start venc chn");
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_venc_stop(sample_venc_config *venc_cfg)
{
    xmedia_s32 i;
    sample_venc_chn_info *chn_param = NULL;

    for (i = 0; i < VENC_MAX_CHN_NUM; i++) {
        chn_param = &venc_cfg->chn_info[i];

        if (chn_param->venc_en == XMEDIA_FALSE) {
            continue;
        }

        CHECK_RET(sample_comm_venc_stop_chn(chn_param->venc_chn), "stop venc chn");

        CHECK_RET(sample_comm_venc_destory_chn(chn_param->venc_chn), "destory venc chn");
    }

    return XMEDIA_SUCCESS;
}

xmedia_void sample_venc_save_one_frame_nv21(FILE * fp,xmedia_u8 * vir_y, xmedia_u8 * vir_c,
    xmedia_u32 width, xmedia_u32 height, xmedia_u32 stride)
{
    xmedia_u8 * dst;
    xmedia_u32 row;

    dst = vir_y;
    for(row = 0; row < height; row++) {
        fwrite( dst, width, 1, fp );
        dst += stride;
    }

    dst = vir_c;
    for(row = 0; row < height/2; row++) {
        fwrite(dst, width, 1, fp);
        dst += stride;
    }
}

/****  User send yuv + venc only start ****/
xmedia_void sample_venc_read_one_frame_nv21(FILE * fp,xmedia_u8 * vir_y, xmedia_u8 * vir_c,
    xmedia_u32 width, xmedia_u32 height, xmedia_u32 stride)
{
    xmedia_u8 * dst;
    xmedia_u32 row;

    dst = vir_y;
    for(row = 0; row < height; row++) {
        fread( dst, width, 1, fp );
        dst += stride;
    }

    dst = vir_c;
    for(row = 0; row < height/2; row++) {
        fread(dst, width, 1, fp);
        dst += stride;
    }
}

xmedia_void sample_venc_read_one_frame_sp422_semi(FILE * fp, xmedia_u8 * vir_y, xmedia_u8 * vir_c,
                                               xmedia_u32 width, xmedia_u32 height, xmedia_u32 stride)
{
    xmedia_u8 * dst;
    xmedia_u32 row;

    dst = vir_y;
    for(row = 0; row < height; row++) {
       fread( dst, width, 1, fp );
       dst += stride;
    }

    dst = vir_c;
    for(row = 0; row < height; row++) {
       fread(dst, width, 1, fp);
       dst += stride;
    }
}

xmedia_void *sample_comm_venc_user_send_thread(void *data)
{
    char a;
    xmedia_u8 i;
    xmedia_u8 j = 0;
    xmedia_s32 ret;
    xmedia_u32 cnt = 0;
    xmedia_u32 vir_addr_y, vir_addr_c;
    xmedia_u32 try_send_cnt = 0;
    xmedia_u32 max_cnt = 0xffffffff;
    sample_venc_mem_buf_info mem = {0};
    xmedia_video_frame_info video_info;
    sample_venc_yuv_thread_para* venc_user_attr;
    xmedia_u32 pool  = VB_INVALID_POOLID;
    xmedia_video_private_info vb_info = {0};
    xmedia_u32 luma_size = 0;
    // xmedia_u32 chrm_size = 0;
    xmedia_u32 pic_luma_stride;
    xmedia_u32 align_pic_height,align_pic_width;
    xmedia_u32 size = 0;
    xmedia_u32 is_save = XMEDIA_FALSE;
    FILE *input_yuv = XMEDIA_NULL;

    venc_user_attr = (sample_venc_yuv_thread_para *)data;

    if(venc_user_attr == XMEDIA_NULL) {
         return XMEDIA_NULL;
    }
    pool = venc_user_attr->pool;

    align_pic_height = ((venc_user_attr->user_height + 15) >> 4) << 4;
    align_pic_width  = ((venc_user_attr->user_width + 15) >> 4) << 4;
    pic_luma_stride     = ((venc_user_attr->user_width + 15) >> 4) << 4;


    if(venc_user_attr->user_pixel_format == XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_422 ||
       venc_user_attr->user_pixel_format == XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_422) {
        luma_size = (pic_luma_stride * venc_user_attr->user_height);
        // chrm_size = luma_size >> 1;
        size = align_pic_width * align_pic_height * 2;
    } else if (venc_user_attr->user_pixel_format == XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420 ||
               venc_user_attr->user_pixel_format == XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_420) {
        luma_size = (pic_luma_stride * venc_user_attr->user_height);
        // chrm_size = luma_size >> 2;
        size = align_pic_width * align_pic_height * 3 / 2;
    } else if (venc_user_attr->user_pixel_format == XMEDIA_VIDEO_PIXEL_FMT_YUV_400) {
        luma_size = (pic_luma_stride * venc_user_attr->user_height);
        size = align_pic_width * align_pic_height;
    }

    mem.pool = pool;

    max_cnt = venc_user_attr->max_send_yuv_cnt ? venc_user_attr->max_send_yuv_cnt : 300;
    input_yuv = venc_user_attr->input_yuv[j];
    if (venc_user_attr->user_send_yuv_delay_ms > 0) {
        printf("u32UserSendYuvDleayInMs:%u\n", venc_user_attr->user_send_yuv_delay_ms);
        usleep(venc_user_attr->user_send_yuv_delay_ms * 1000);
    }
    while((XMEDIA_TRUE == g_user_start) && (cnt < max_cnt)) {
        fread((void *)(&a), 1, 1, input_yuv);
        if (0 != feof(input_yuv)) {
            if (venc_user_attr->yuv_num > 1) {
                j++;
                if (j >= venc_user_attr->max_send_yuv_cnt) {
                     break;
                }
                input_yuv = venc_user_attr->input_yuv[j];
            } else {
                fseek(input_yuv, 0, SEEK_SET);
            }
        } else {
            fseek(input_yuv, -1, SEEK_CUR);
        }

        while ((mem.block = xmedia_vb_get_block(mem.pool, size, XMEDIA_NULL)) == VB_INVALID_HANDLE) {
            usleep(500 * 1000);
        }

        mem.phy_addr = xmedia_vb_handle_to_phy_addr(mem.block);
        mem.vir_addr = (xmedia_u8 *)xmedia_mmz_map(mem.phy_addr, size, 0);
        if (mem.vir_addr == NULL) {
            printf("mem dev may not open\n");
            xmedia_vb_release_block(mem.block);
            return XMEDIA_NULL;
        }
        ret = xmedia_vb_get_supplement_info(mem.block, &vb_info);
        if(ret != XMEDIA_SUCCESS) {
            printf("xmedia_vb_get_supplement_info ERROR\n");
            xmedia_vb_release_block(mem.block);
            return XMEDIA_NULL;
        }

        memset(&video_info.frame, 0, sizeof(xmedia_video_frame));
        video_info.frame.dynamic_range = XMEDIA_VIDEO_DYNAMIC_RANGE_SDR;
        video_info.frame.color_info.color_gamut   = XMEDIA_VIDEO_COLOR_GAMUT_BT709;
        video_info.frame.addr.y_phy_addr = mem.phy_addr;
        video_info.frame.addr.c_phy_addr = video_info.frame.addr.y_phy_addr + luma_size;
        // video_info.frame.phy_addr_cr = frm_info.frame.phy_addr[1] + chrm_size;

        vir_addr_y = (xmedia_u64)(xmedia_ulong)mem.vir_addr;
        vir_addr_c = (xmedia_u64)(xmedia_ulong)(vir_addr_y + luma_size);
        // video_info.frame.vir_addr_cr = (xmedia_u64)(xmedia_ulong)(vir_addr_c + chrm_size);

        video_info.frame.width     = venc_user_attr->user_width;
        video_info.frame.height    = venc_user_attr->user_height;

        video_info.frame.stride.y_stride = pic_luma_stride;
        video_info.frame.stride.c_stride = pic_luma_stride;
        // video_info.frame.stride_cr = pic_luma_stride;

        video_info.frame.pts   = (cnt * 40);
        video_info.frame.index = (cnt * 2);
        cnt++;
        video_info.frame.priv_info.isp_info_phy_addr = vb_info.isp_info_phy_addr;
        video_info.frame.priv_info.isp_info_vir_addr = vb_info.isp_info_vir_addr;
        video_info.frame.pixel_fmt = venc_user_attr->user_pixel_format;

        if (venc_user_attr->user_pixel_format == XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_422 ||
            venc_user_attr->user_pixel_format == XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_422) {
            sample_venc_read_one_frame_sp422_semi(input_yuv,  (xmedia_u8 *)(xmedia_ulong)vir_addr_y,
                                               (xmedia_u8 *)(xmedia_ulong)vir_addr_c,
                                               video_info.frame.width, video_info.frame.height, video_info.frame.stride.y_stride);
        } else if (venc_user_attr->user_pixel_format == XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420 ||
                   venc_user_attr->user_pixel_format == XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_420) {
            sample_venc_read_one_frame_nv21(input_yuv, (xmedia_u8 *)(xmedia_ulong)vir_addr_y,
                                         (xmedia_u8 *)(xmedia_ulong)vir_addr_c,
                                         video_info.frame.width, video_info.frame.height, video_info.frame.stride.y_stride);
        }

        mem.pool_id = xmedia_vb_handle_to_pool_id(mem.block);
        video_info.pool_id = mem.pool_id;

        video_info.frame.compress_mode = venc_user_attr->user_compress_mode;
        video_info.frame.video_fmt = venc_user_attr->user_video_format;
        if (is_save == XMEDIA_TRUE) {
            FILE *tmp = fopen("./tmp.yuv", "wb");
            sample_venc_save_one_frame_nv21(tmp, (xmedia_u8 *)(xmedia_ulong)vir_addr_y,
                                         (xmedia_u8 *)(xmedia_ulong)vir_addr_c,
                                         video_info.frame.width, video_info.frame.height, video_info.frame.stride.y_stride);
            is_save = XMEDIA_FALSE;
            fclose(tmp);
        }
        printf("%s,%d,SendPic, MaxCnt:%d, curcnt:%d\n", __FUNCTION__, __LINE__, max_cnt, cnt);
        for (i = 0; i < venc_user_attr->h265e_cnt; i++) {
            try_send_cnt = 0;
            while (xmedia_venc_send_frame(venc_user_attr->h265e_chnl[i], &video_info, 0) != 0 &&
                   (g_user_start == XMEDIA_TRUE) && try_send_cnt < 2) { // 2: timeout count
                try_send_cnt++;
                usleep(500 * 1000); //timeout
                printf("[%d] h265 xmedia_venc_send_frame ERROR === u32TrySendCnt:%d, MaxCnt:cnt:%d:%d\n",
                    __LINE__, try_send_cnt,max_cnt,cnt);
            }
        }

        for (i = 0; i < venc_user_attr->h264e_cnt; i++) {
            try_send_cnt = 0;
            while (xmedia_venc_send_frame(venc_user_attr->h264e_chnl[i], &video_info, 0) != 0 &&
                   (g_user_start == XMEDIA_TRUE) && try_send_cnt < 2) { // 2: timeout count
                try_send_cnt++;
                usleep(500 * 1000); //timeout
                printf("[%d] h264 xmedia_venc_send_frame ERROR === u32TrySendCnt:%d, MaxCnt:cnt:%d:%d\n",
                    __LINE__, try_send_cnt,max_cnt,cnt);
            }
        }

        for (i = 0; i < venc_user_attr->mjpeg_cnt; i++) {
            try_send_cnt = 0;
            while(xmedia_venc_send_frame(venc_user_attr->mjpeg_chnl[i], &video_info, 0) != 0 &&
                  (g_user_start == XMEDIA_TRUE) && try_send_cnt < 2) {
                try_send_cnt++;
                usleep(500 * 1000); //timeout
                printf("[%d] MJPEG xmedia_venc_send_frame ERROR === u32TrySendCnt:%d, MaxCnt : cnt:%d : %d\n",
                       __LINE__, try_send_cnt,max_cnt,cnt);
            }
        }

        for (i = 0; i < venc_user_attr->jpege_cnt; i++) {
            try_send_cnt = 0;
            while ((xmedia_venc_send_frame(venc_user_attr->jpege_chnl[i], &video_info, 0) != 0) &&
                  (g_user_start == XMEDIA_TRUE) && (try_send_cnt < 2)) {
                try_send_cnt++;
                usleep(500 * 1000); //timeout
                printf("[%d] JPEG xmedia_venc_send_frame ERROR === TrySendCnt:%d, MaxCnt:cnt:%d:%d\n",
                    __LINE__, try_send_cnt,max_cnt,cnt);
            }
        }

        xmedia_mmz_unmap(mem.vir_addr);
        xmedia_vb_release_block(mem.block);
    }
    printf("Send over, cnt:%d/%d\n", cnt, max_cnt);

    return XMEDIA_NULL;
}
/****  User send yuv + venc only end ****/

xmedia_s32 app_liveserver_send_video_data(xcam_handle venc_handle, xmedia_payload_type payload_type, xmedia_venc_stream *stream)
{
    xcam_livestream_rtsp_data rtsp_data;
    xcam_bool is_key_frame = XCAM_FALSE;
    xmedia_venc_data_type data_type;
    xmedia_s32 ret;
    xmedia_u64 cur_pts = 0;

    if (xcam_liveserver_is_init() == XCAM_FALSE) {
        return XCAM_LIVESERVER_ERR_NOT_INIT;
    }

    //data_type = stream->pack[stream->pack_count - 1].data_type;
    data_type = stream->pack[0].data_type;
    if (payload_type == PT_H265) {
        #if 1
        is_key_frame = ((data_type.h265e_type == H265E_NALU_VPS)
            || (data_type.h265e_type == H265E_NALU_ISLICE)) ? XMEDIA_TRUE : XMEDIA_FALSE;
        #else
        is_key_frame = ((0x00 == stream->pack[stream->pack_count - 1].vir_addr[0]) &&
                        (0x00 == stream->pack[stream->pack_count - 1].vir_addr[1]) &&
                        (0x01 == stream->pack[stream->pack_count - 1].vir_addr[2]) &&
                        (0x14 == ((stream->pack[stream->pack_count - 1].vir_addr[3] >> 0x01) & 0x3f)));
        #endif
    } else if (payload_type == PT_H264) {
        is_key_frame = ((data_type.h264e_type == H264E_NALU_IDRSLICE)
            || (data_type.h264e_type == H264E_NALU_ISLICE)
            || (data_type.h264e_type == H264E_NALU_SPS)) ? XMEDIA_TRUE : XMEDIA_FALSE;
    }

    xmedia_sys_get_cur_pts(&cur_pts);
    rtsp_data.pts_us = cur_pts;
    rtsp_data.seq_num = stream->seq;
    rtsp_data.is_key_frame = is_key_frame;
    rtsp_data.block_cnt = stream->pack_count;
    for (xcam_u32 idx = 0; idx < stream->pack_count; ++idx) {
        rtsp_data.data_ptr[idx] = stream->pack[idx].vir_addr;
        rtsp_data.data_len[idx] = stream->pack[idx].len;
    }

    ret = xcam_liveserver_send_video_data(venc_handle, &rtsp_data);

    return ret;
}

xmedia_s32 sample_comm_venc_set_roi(xmedia_s32 venc_chn[], xmedia_s32 chn_cnt)
{
    xmedia_u32 i;
    xmedia_s32 ret;
    xmedia_venc_roi_attr roi_attr;

    for (i = 0;i < chn_cnt;i++) {
        ret = xmedia_venc_get_roi_attr(venc_chn[i], 0, &roi_attr);
        if (XMEDIA_SUCCESS != ret) {
            SAMPLE_PRT("get roi attr failed for %#x!\n", ret);
            return XMEDIA_FAILURE;
        }

        roi_attr.abs_qp  = XMEDIA_TRUE;
        roi_attr.enable  = XMEDIA_TRUE;
        roi_attr.qp      = 20;
        roi_attr.index   = 0;
        roi_attr.rect.x  = 64;
        roi_attr.rect.y  = 64;
        roi_attr.rect.height = 256;
        roi_attr.rect.width  = 256;

        ret = xmedia_venc_set_roi_attr(venc_chn[i], &roi_attr);
        if (XMEDIA_SUCCESS != ret) {
            SAMPLE_PRT("set Roi attr failed for %#x!\n", ret);
            return XMEDIA_FAILURE;
        }
    }
    return XMEDIA_SUCCESS;
}

xmedia_void* sample_comm_venc_get_stream_proc(xmedia_void *pData)
{
    sample_venc_getstream_para *param = (sample_venc_getstream_para*)pData;
    xmedia_s32 chn_cnt = param->cnt;
    xmedia_s32 i;
    xmedia_s32 ret = XMEDIA_FAILURE;
    xmedia_venc_chn_attr venc_chn_attr;
    xmedia_payload_type payload_type[VENC_MAX_CHN_NUM];
    xmedia_char file_post_fix[VENC_MAX_CHN_NUM][10];

    xmedia_char file_name[VENC_MAX_CHN_NUM][512];
    xmedia_char file_path[128];
    FILE *file_ptr[VENC_MAX_CHN_NUM];

    struct timeval time_out;
    xmedia_venc_stream venc_stream;
    xmedia_venc_chn_status stat;
    xmedia_u32 picture_cnt[VENC_MAX_CHN_NUM] = {0};
    xmedia_u32 venc_mask = 0;

    if (chn_cnt >= VENC_MAX_CHN_NUM) {
        SAMPLE_PRT("input venc chn count invaild\n");
        return XMEDIA_NULL;
    }

    memset(file_path, 0, 128);
    snprintf(file_path, 128, "%s", param->save_path);
    mkdir(file_path, S_IRWXU|S_IRWXG|S_IRWXO);

    prctl(PR_SET_NAME, (unsigned long)(uintptr_t)"VencGetStream", 0, 0, 0);

    for (i = 0; i < chn_cnt; ++i) {
        if (param->venc_chn[i] < 0) {
            continue;
        }

        ret = xmedia_venc_get_chn_attr(param->venc_chn[i], &venc_chn_attr);
        if (XMEDIA_SUCCESS != ret) {
            SAMPLE_PRT("xmedia_venc_get_chn_attr chn[%d] failed with %#x!\n", param->venc_chn[i], ret);
            return XMEDIA_NULL;
        }

        payload_type[i] = venc_chn_attr.venc_attr.en_type;

        ret = sample_comm_venc_get_file_post_fix(payload_type[i], file_post_fix[i]);
        if (XMEDIA_SUCCESS != ret) {
            SAMPLE_PRT("sample_comm_venc_get_file_post_fix [%d] failed with %#x!\n", payload_type[i], ret);
            return XMEDIA_NULL;
        }

        if (param->save_stream == XMEDIA_TRUE) {
            if (PT_JPEG != payload_type[i]) {
                snprintf(file_name[i], 512, "%s/stream_chn%d%s", file_path, i, file_post_fix[i]);
                file_ptr[i] = fopen(file_name[i], "wb");
                if (!file_ptr[i]) {
                    SAMPLE_PRT("open file %s failed!\n", file_name[i]);
                    return XMEDIA_NULL;
                }
                chmod(file_name[i], S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
            } else if (param->save_jpeg) {
            }
        }
        venc_mask |= 1 << param->venc_chn[i];
    }

    while (param->thread_start) {
        time_out.tv_sec  = 12;
        time_out.tv_usec = 0;
        ret = xmedia_venc_select(venc_mask, &time_out);
        if (ret == XMEDIA_ERRCODE_INVALID_PARAM || ret == XMEDIA_FAILURE) {
            SAMPLE_PRT("select err\n");
            break;
        } else if (ret == XMEDIA_ERRCODE_TIMEOUT) {
            param->stream_timeout_cnt++;
            SAMPLE_PRT("get venc stream time out, continue. \n");
            usleep(100 * 1000);
            continue;
        }

        for (i = 0; i < chn_cnt; ++i) {
            if (param->venc_chn[i] < 0) {
                continue;
            }

            ret = xmedia_venc_query_status(param->venc_chn[i], &stat);
            if (XMEDIA_SUCCESS != ret) {
                SAMPLE_PRT("xmedia_venc_query_status chn[%d] failed with %#x!\n", i, ret);
                break;
            }

            if (0 == stat.cur_packs) {
                continue;
            } else if ((param->save_jpeg != XMEDIA_TRUE) && (payload_type[i] == PT_JPEG)) {
                continue;
            }

            memset(&venc_stream, 0, sizeof(venc_stream));
            venc_stream.pack = (xmedia_venc_pack*)malloc(sizeof(xmedia_venc_pack) * stat.cur_packs);
            if (XMEDIA_NULL == venc_stream.pack) {
                SAMPLE_PRT("malloc stream pack failed!\n");
                break;
            }

            venc_stream.pack_count = stat.cur_packs;
            ret = xmedia_venc_get_stream(param->venc_chn[i], &venc_stream, -1);
            if (XMEDIA_SUCCESS != ret) {
                free(venc_stream.pack);
                venc_stream.pack = XMEDIA_NULL;
                xmedia_venc_release_stream(param->venc_chn[i], &venc_stream);
                SAMPLE_PRT("xmedia_venc_get_stream failed with %#x!\n", ret);
                break;
            }

            if ((param->save_jpeg == XMEDIA_TRUE) && (payload_type[i] == PT_JPEG)) {
                snprintf(file_name[i], 512, "%s/snap%d_%d%s", file_path, i, picture_cnt[i], file_post_fix[i]);
                file_ptr[i] = fopen(file_name[i], "wb");
                if (!file_ptr[i]) {
                    SAMPLE_PRT("open file err!\n");
                    xmedia_venc_release_stream(param->venc_chn[i], &venc_stream);
                    return XMEDIA_NULL;
                }
                chmod(file_name[i], S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
                ret = sample_comm_venc_save_stream(file_ptr[i], &venc_stream);
                if (XMEDIA_SUCCESS != ret) {
                    SAMPLE_PRT("save stream failed!\n");
                    free(venc_stream.pack);
                    venc_stream.pack = XMEDIA_NULL;
                    xmedia_venc_release_stream(param->venc_chn[i], &venc_stream);
                    break;
                }
                fclose(file_ptr[i]);
                picture_cnt[i]++;
            }

            if (((payload_type[i] == PT_H264) || (payload_type[i] == PT_H265)) && (g_user_start == XMEDIA_FALSE)) {
                app_liveserver_send_video_data(i, payload_type[i], &venc_stream);
            }

            if ((param->save_stream == XMEDIA_TRUE) && (PT_JPEG != payload_type[i])) {
                ret = sample_comm_venc_save_stream(file_ptr[i], &venc_stream);
                if (XMEDIA_SUCCESS != ret) {
                    SAMPLE_PRT("save stream failed!\n");
                    free(venc_stream.pack);
                    venc_stream.pack = XMEDIA_NULL;
                    xmedia_venc_release_stream(param->venc_chn[i], &venc_stream);
                    break;
                }
            }

            ret = xmedia_venc_release_stream(param->venc_chn[i], &venc_stream);
            if (XMEDIA_SUCCESS != ret) {
                SAMPLE_PRT("xmedia_venc_release_stream failed!\n");
                free(venc_stream.pack);
                venc_stream.pack = XMEDIA_NULL;
                break;
            }

            free(venc_stream.pack);
            venc_stream.pack = XMEDIA_NULL;
        }
    }

    if (param->save_stream == XMEDIA_TRUE) {
        for (i = 0; i < chn_cnt; ++i) {
            if (param->venc_chn[i] < 0) {
                continue;
            }
            if (PT_JPEG != payload_type[i]) {
                fclose(file_ptr[i]);
            }
        }
    }

    return (xmedia_void*)XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_venc_set_jpeg_exif(xmedia_s32 venc_chn)
{
    xmedia_venc_exif_info exif_info;

    CHECK_RET(xmedia_venc_get_jpeg_exif_info(venc_chn, &exif_info),"get exif info");
    strncpy((char *)exif_info.image_description, "xmedia test", JPEG_DRSCRIPTION_LENGTH);
    strncpy((char *)exif_info.maker, "xmedia", JPEG_DRSCRIPTION_LENGTH);
    strncpy((char *)exif_info.model, "ipc camera", JPEG_DRSCRIPTION_LENGTH);
    strncpy((char *)exif_info.software, "Exif 2.1.0", JPEG_DRSCRIPTION_LENGTH);
    exif_info.light_source             = 1;
    exif_info.focal_length             = 6;
    exif_info.focal_length_in35mm_film = 0;
    exif_info.scene_type               = 1;
    exif_info.custom_rendered          = 1;
    exif_info.scene_capture_type       = 1;
    exif_info.gain_control             = 0;
    exif_info.contrast                 = 0;
    exif_info.saturation               = 0;
    exif_info.sharpness                = 0;
    exif_info.metering_mode            = 1;
    CHECK_RET(xmedia_venc_set_jpeg_exif_info(venc_chn, &exif_info),"set exif info");
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_venc_capture_jpeg(xmedia_s32 venc_chn, xmedia_s32 snap_cnt, xmedia_bool save_jpeg, xmedia_bool save_thumbnail, xmedia_char *save_path)
{
    xmedia_s32 i;
    xmedia_s32 ret;
    xmedia_s32 save_ret = XMEDIA_SUCCESS;
    xmedia_s32 select_ret;
    xmedia_venc_recv_pic_param recv_param;
    xmedia_venc_chn_status st_stat;
    xmedia_venc_stream st_stream;
    struct timeval timeout_val;
    FILE *file_ptr = XMEDIA_NULL;
    static xmedia_u32 recv_cnt[16] = {0};
    xmedia_char jpeg_file[128];
    xmedia_u32 venc_mask = 1 << venc_chn;

    recv_param.recv_pic_num = snap_cnt;
    ret = xmedia_venc_start_recv_frame(venc_chn, &recv_param);
    if (XMEDIA_SUCCESS != ret) {
        SAMPLE_PRT("xmedia_venc_start_recv_frame faild with%#x!\n", ret);
        return XMEDIA_NULL;
    }
    for (i = 0; i < snap_cnt; i++) {
        timeout_val.tv_sec  = 1;
        timeout_val.tv_usec = 0;

        select_ret = xmedia_venc_select(venc_mask, &timeout_val);
        if (select_ret == XMEDIA_ERRCODE_INVALID_PARAM || select_ret == XMEDIA_FAILURE) {
            SAMPLE_PRT("xmedia_venc_select failed!\n");
            break;
        } else if (select_ret == XMEDIA_ERRCODE_TIMEOUT) {
            continue;
        }
        memset(&st_stream, 0, sizeof(st_stream));
        ret = xmedia_venc_query_status(venc_chn, &st_stat);
        if (ret != XMEDIA_SUCCESS ) {
            SAMPLE_PRT("query stream failed because 0x%x \n", ret);
            continue;
        }
        if(0 == st_stat.cur_packs || 0 == st_stat.left_stream_frames) {
            SAMPLE_PRT("query stream cur_packs:%d or left_stream_frames:%d is 0 \n", \
                        st_stat.cur_packs, st_stat.left_stream_frames);
            continue;
        }
        st_stream.pack = (xmedia_venc_pack*)malloc(sizeof(xmedia_venc_pack) * st_stat.cur_packs);
        if (NULL == st_stream.pack) {
            SAMPLE_PRT("malloc memory failed!\n");
            break;
        }
        st_stream.pack_count = st_stat.cur_packs;
        ret = xmedia_venc_get_stream(venc_chn, &st_stream, -1);
        if (XMEDIA_SUCCESS != ret) {
            SAMPLE_PRT("xmedia_venc_get_stream failed with %#x!\n", ret);
            free(st_stream.pack);
            st_stream.pack = NULL;
            break;
        }
        if (save_jpeg) {
            if (file_ptr == NULL) {
                snprintf(jpeg_file, sizeof(jpeg_file), "%s/chn%d_snap_%d.jpg", save_path, venc_chn, recv_cnt[venc_chn]);
                file_ptr = fopen(jpeg_file, "wb");
                if (!file_ptr) {
                    SAMPLE_PRT("open file[%s] err!\n", jpeg_file);
                    break;
                }
                /*change mod for user*/
                chmod(jpeg_file, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
            }
            save_ret = sample_comm_venc_save_jpeg_stream(venc_chn, file_ptr, &st_stream, recv_cnt[venc_chn], save_path);
            if (save_ret == XMEDIA_SUCCESS) {
                SAMPLE_PRT("save jpeg file:%s ok\n", jpeg_file);
            }
        }
        if (save_thumbnail) {
            ret = sample_comm_venc_save_jpeg_thum(save_path, &st_stream, venc_chn, recv_cnt[venc_chn]);
            if (XMEDIA_SUCCESS != ret) {
                SAMPLE_PRT("save thm picture failed!\n");
                break;
            }
        }

        ret = xmedia_venc_release_stream(venc_chn, &st_stream);
        if (XMEDIA_SUCCESS != ret) {
            SAMPLE_PRT("xmedia_venc_release_stream failed with %#x!\n", ret);
            free(st_stream.pack);
            st_stream.pack = NULL;
            return XMEDIA_NULL;
        }
        free(st_stream.pack);
        st_stream.pack = NULL;
        if (save_ret == XMEDIA_SUCCESS) {
            recv_cnt[venc_chn]++;
            if (file_ptr != NULL) {
                fclose(file_ptr);
                file_ptr = NULL;
            }
        }
    }

    ret = xmedia_venc_stop_recv_frame(venc_chn);
    if (XMEDIA_SUCCESS != ret) {
        SAMPLE_PRT("xmedia_venc_stop_recv_frame failed with:%#x!\n", ret);
        return XMEDIA_FAILURE;
    }
    return XMEDIA_SUCCESS;
}



#define QPMAP_BUF_NUM       8
#define VENC_QPMAP_MAX_CHN  2
#define SVP_MAXINUM_LIMIT 10000

xmedia_u32 sample_venc_get_qpmap_size(xmedia_u32 width, xmedia_u32 height, xmedia_payload_type type)
{
    xmedia_u32 stride, align_height;
    xmedia_u32 qpmap_size = 0;

    if ((width > SVP_MAXINUM_LIMIT) || (height > SVP_MAXINUM_LIMIT)) {
        SAMPLE_PRT("(width:%u || height：%u) > max_limit:%d\n", width, height, SVP_MAXINUM_LIMIT);
        return 0;
    }

    if (type == PT_H265) {
        stride = DIV_UP(width, 64);
        align_height = DIV_UP(height, 64);
        qpmap_size = stride * align_height * 16;
    } else if (type == PT_H264) {
        stride = DIV_UP(width, 16);
        align_height = DIV_UP(height, 16);
        qpmap_size = stride * align_height;
    }

    return qpmap_size;
}

xmedia_u32 sample_venc_get_skip_weight_size(xmedia_payload_type type, xmedia_u32 width, xmedia_u32 height)
{
    xmedia_u32 skip_weight_width;
    xmedia_u32 skip_weight_height;
    xmedia_u32 skip_weight_size = 0;
    if (type == PT_H265) {
        skip_weight_width  = DIV_UP(width, 64);
        skip_weight_height = DIV_UP(height, 64);
        skip_weight_size   = DIV_UP(skip_weight_width * skip_weight_height, 2);
    } else if (type == PT_H264) {
        skip_weight_width  = DIV_UP(width, 16);
        skip_weight_height = DIV_UP(height, 16);
        skip_weight_size   = DIV_UP(skip_weight_width * skip_weight_height, 2);
    }

    return skip_weight_size;
}

xmedia_void* sample_comm_venc_qpmap_sendfrm_proc(xmedia_void* p)
{
    xmedia_u32 i,j,frm_id;
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_video_frame_info  *video_frame;
    xmedia_user_frame_info   usr_frame[VENC_QPMAP_MAX_CHN][QPMAP_BUF_NUM];
    sample_venc_qpmap_sendfrm_para *qpmap_para;

    xmedia_u32   qpmap_size[VENC_QPMAP_MAX_CHN];
    xmedia_u64   qpmap_phy_addr[VENC_QPMAP_MAX_CHN][QPMAP_BUF_NUM];
    xmedia_void* qpmap_vir_addr[VENC_QPMAP_MAX_CHN][QPMAP_BUF_NUM];

    xmedia_u32   skip_weight_size[VENC_QPMAP_MAX_CHN];
    xmedia_u64   skip_weight_phy_addr[VENC_QPMAP_MAX_CHN][QPMAP_BUF_NUM];
    xmedia_void* skip_weight_vir_addr[VENC_QPMAP_MAX_CHN][QPMAP_BUF_NUM];

    xmedia_u8 *tmp_vir_addr;
    xmedia_u64 tmp_phy_addr;
    xmedia_u8 *frm_vir_addr_temp;

    xmedia_vpss_ochn_attr  vpss_chn_attr;
    xmedia_venc_chn_attr  venc_chn_attr;
    xmedia_payload_type payload_type;
    skip_wight_value skip_value;

    qpmap_para = (sample_venc_qpmap_sendfrm_para*)p;

    if (qpmap_para->chn_cnt > VENC_QPMAP_MAX_CHN) {
        SAMPLE_PRT("current func'sample_comm_venc_qpmap_sendfrm_proc' not support venc channal num(%d) > %d\n",qpmap_para->chn_cnt,VENC_QPMAP_MAX_CHN);
        return NULL;
    }

    for (i = 0; i < qpmap_para->chn_cnt; i++) {
        xmedia_venc_get_chn_attr(qpmap_para->venc_chn[i],&venc_chn_attr);

        payload_type = venc_chn_attr.venc_attr.en_type;

        qpmap_size[i] = sample_venc_get_qpmap_size(qpmap_para->size[i].width, qpmap_para->size[i].height, payload_type);

        skip_weight_size[i] = sample_venc_get_skip_weight_size(venc_chn_attr.venc_attr.en_type,
                                                      qpmap_para->size[i].width, qpmap_para->size[i].height);

        /*alloc qpmap memory*/
        tmp_phy_addr = xmedia_mmz_alloc( XMEDIA_NULL, "qpmap_buf", qpmap_size[i] * QPMAP_BUF_NUM);
        if (tmp_phy_addr == 0) {
            SAMPLE_PRT("xmedia_mmz_alloc err\n");
            goto error;
        }
        tmp_vir_addr = xmedia_mmz_map(tmp_phy_addr, qpmap_size[i] * QPMAP_BUF_NUM, XMEDIA_FALSE);
        if (tmp_vir_addr == XMEDIA_NULL) {
            SAMPLE_PRT("xmedia_mmz_map err\n");
            goto error;
        }
        for (j = 0; j < QPMAP_BUF_NUM; j++) {
            qpmap_phy_addr[i][j] = tmp_phy_addr + j * qpmap_size[i];
            qpmap_vir_addr[i][j]   = tmp_vir_addr + j * qpmap_size[i];
        }

        /*alloc skipweight memory*/
        tmp_phy_addr = xmedia_mmz_alloc(XMEDIA_NULL, "skip_weight_buf", skip_weight_size[i] * QPMAP_BUF_NUM);
        if (tmp_phy_addr == 0) {
            SAMPLE_PRT("xmedia_mmz_alloc err:0x%x \n",ret);
            goto error;
        }
        tmp_vir_addr = xmedia_mmz_map(tmp_phy_addr, skip_weight_size[i] * QPMAP_BUF_NUM, XMEDIA_FALSE);
        if (tmp_vir_addr == XMEDIA_NULL) {
            SAMPLE_PRT("xmedia_mmz_map err\n");
            goto error;
        }
        for (j = 0; j < QPMAP_BUF_NUM; j++) {
            skip_weight_phy_addr[i][j] = tmp_phy_addr + j * skip_weight_size[i];
            skip_weight_vir_addr[i][j]   = tmp_vir_addr + j * skip_weight_size[i];
        }
    }

    /* set vpss buffer depth*/
    for (i = 0; i < qpmap_para->chn_cnt; i++) {
        ret = xmedia_vpss_get_ochn_attr(qpmap_para->vpss_pipe,qpmap_para->vpss_chn[i],&vpss_chn_attr);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("xmedia_vpss_get_ochn_attr err:0x%x",ret);
            goto error;
        }

        vpss_chn_attr.depth = 3;
        ret = xmedia_vpss_set_ochn_attr(qpmap_para->vpss_pipe,qpmap_para->vpss_chn[i],&vpss_chn_attr);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("xmedia_vpss_set_ochn_attr err:0x%x",ret);
            goto error;
        }
    }

    frm_id = 0;
    skip_value.bits.weight1 = 0;
    skip_value.bits.weight2 = 0;
    while (XMEDIA_TRUE == qpmap_para->thread_start) {
        for (i = 0; i < qpmap_para->chn_cnt; i++) {
            video_frame = &usr_frame[i][frm_id].user_frame;
            ret = xmedia_vpss_acquire_ochn_frame(qpmap_para->vpss_pipe, qpmap_para->vpss_chn[i], video_frame, 1000);
            if (XMEDIA_SUCCESS != ret) {
                SAMPLE_PRT("xmedia_vpss_acquire_ochn_frame err:0x%x\n",ret);
                continue;
            }

            frm_vir_addr_temp = (xmedia_u8 *)qpmap_vir_addr[i][frm_id];
            for (j = 0; j < qpmap_size[i]; j++) {
                *frm_vir_addr_temp = 0x65;    //[7]:skip flag; [6]:qp_type flag; [5:0]:qp value ==> set absolute qp = 30
                frm_vir_addr_temp++;
            }
            frm_vir_addr_temp = (xmedia_u8 *)skip_weight_vir_addr[i][frm_id];
            for (j = 0; j < skip_weight_size[i]; j++) {
                *frm_vir_addr_temp = skip_value.u8;  // not change inter block skip
                frm_vir_addr_temp++;
            }
            usr_frame[i][frm_id].user_rc_info.skip_weight_valid = 1;
            usr_frame[i][frm_id].user_rc_info.skip_weight_phy_addr = skip_weight_phy_addr[i][frm_id];
            usr_frame[i][frm_id].user_rc_info.qp_map_valid     = 1;
            usr_frame[i][frm_id].user_rc_info.qp_map_phy_addr = qpmap_phy_addr[i][frm_id];
            usr_frame[i][frm_id].user_rc_info.blk_start_qp   = 30;
            usr_frame[i][frm_id].user_rc_info.frame_type = VENC_FRAME_TYPE_NONE;

            ret = xmedia_venc_send_frame_ex(qpmap_para->venc_chn[i], &usr_frame[i][frm_id], -1);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_venc_send_frame_ex err:0x%x\n",ret);

                ret = xmedia_vpss_release_ochn_frame(qpmap_para->vpss_pipe, qpmap_para->vpss_chn[i], video_frame);
                if (ret != XMEDIA_SUCCESS) {
                    SAMPLE_PRT("xmedia_vpss_release_ochn_frame err:0x%x", ret);
                    goto error;
                }
                break;
            }

            ret = xmedia_vpss_release_ochn_frame(qpmap_para->vpss_pipe, qpmap_para->vpss_chn[i], video_frame);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_vpss_release_ochn_frame err:0x%x", ret);
                goto error;
            }

            frm_id++;
            if (frm_id >= QPMAP_BUF_NUM) {
                frm_id = 0;
            }
        }
    }

error:
    for (i = 0; i < qpmap_para->chn_cnt; i++) {
        if (qpmap_phy_addr[i][0] != 0) {
            xmedia_mmz_unmap(qpmap_vir_addr[i][0]);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_mmz_unmap err:0x%x",ret);
            }
            ret = xmedia_mmz_free(qpmap_phy_addr[i][0]);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_mmz_free err:0x%x",ret);
            }
        }
        if (skip_weight_phy_addr[i][0] != 0) {
            xmedia_mmz_unmap(skip_weight_vir_addr[i][0]);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_mmz_unmap err:0x%x",ret);
            }
            ret = xmedia_mmz_free(skip_weight_phy_addr[i][0]);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_mmz_free err:0x%x",ret);
            }
        }
    }

    return NULL;
}

xmedia_s32 sample_comm_venc_qpmap_sendfrm(xmedia_s32 pipe,xmedia_s32 vpss_chn[],xmedia_s32 venc_chn[],xmedia_s32 cnt, xmedia_video_size vpss_size[])
{
    xmedia_s32 i;

    g_qpmap_sendfrm_para.thread_start = XMEDIA_TRUE;
    g_qpmap_sendfrm_para.vpss_pipe = pipe;
    g_qpmap_sendfrm_para.chn_cnt  = cnt;

    for(i = 0; i < cnt; i++) {
        g_qpmap_sendfrm_para.venc_chn[i]   = venc_chn[i];
        g_qpmap_sendfrm_para.vpss_chn[i] = vpss_chn[i];
        g_qpmap_sendfrm_para.size[i]  = vpss_size[i];
    }

    return pthread_create(&g_venc_qpmap_pid, 0, sample_comm_venc_qpmap_sendfrm_proc, (xmedia_void*)&g_qpmap_sendfrm_para);
}

xmedia_s32 sample_comm_venc_stop_send_qpmapfrm(xmedia_void)
{
    g_qpmap_sendfrm_para.thread_start = XMEDIA_FALSE;
    pthread_join(g_venc_qpmap_pid, 0);

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_venc_save_jpeg_block(FILE *fd, xmedia_venc_stream *st_stream, xmedia_u32 cur_row, xmedia_u32 cur_col,
                                                          sample_venc_splicing_para *splicing_para)
{
    xmedia_s32 i, j;
    static xmedia_u8 rstn = 0xD0;
    xmedia_u32 invalid_data = 0;
    xmedia_u32 mcu_per_ecs;
    xmedia_u32 block_ecs_cnt = 0;
    xmedia_u32 tmp_data_offset = 0;
    xmedia_u32 tmp_data_start = 0;
    xmedia_u32 more_offset = 0;
    xmedia_u32 sos_size = 14;
    xmedia_u8 *tmp_data = XMEDIA_NULL;

    if (splicing_para->splicing_info.col_cnt == 1) {
        mcu_per_ecs = ALIGN_UP(splicing_para->splicing_info.enc_width, 16) *
                      ALIGN_UP(splicing_para->splicing_info.enc_height, 16) / 16 / 16;
    } else {
        mcu_per_ecs = ALIGN_UP(splicing_para->splicing_info.enc_width, 16) / 16;
    }

    if (cur_row == 0 && cur_col == 0) {
        for (i = st_stream->pack[0].offset;i < st_stream->pack[0].len;i++) {
            if (st_stream->pack[0].vir_addr[i] == 0xFF) {
                i++;
                if (st_stream->pack[0].vir_addr[i] == 0xC0) {
                    i += 4;
                    st_stream->pack[0].vir_addr[i++] = (xmedia_u8)((splicing_para->splicing_info.dst_height >> 8) & 0xFF);
                    st_stream->pack[0].vir_addr[i++] = (xmedia_u8)(splicing_para->splicing_info.dst_height & 0xFF);
                    st_stream->pack[0].vir_addr[i++] = (xmedia_u8)((splicing_para->splicing_info.dst_width >> 8) & 0xFF);
                    st_stream->pack[0].vir_addr[i++] = (xmedia_u8)(splicing_para->splicing_info.dst_width & 0xFF);
                }
                if (st_stream->pack[0].vir_addr[i] == 0xDD) {
                    i += 3;
                    st_stream->pack[0].vir_addr[i++] = (xmedia_u8)((mcu_per_ecs >> 8) & 0xFF);
                    st_stream->pack[0].vir_addr[i++] = (xmedia_u8)(mcu_per_ecs & 0xFF);
                }
            }
        }
        fwrite(st_stream->pack[0].vir_addr + st_stream->pack[0].offset,
            st_stream->pack[0].len - st_stream->pack[0].offset, 1, fd);
        fflush(fd);
    } else {
        more_offset = sos_size;
    }

    if (splicing_para->splicing_info.col_cnt == 1) {
        i = st_stream->pack[1].len - 1;
        do {
            if (cur_row == (splicing_para->splicing_info.row_cnt - 1) &&
                cur_col == (splicing_para->splicing_info.col_cnt - 1)) {
                rstn = 0xD0;
                break;
            }
            if (st_stream->pack[1].vir_addr[i] == 0xD9) {
                st_stream->pack[1].vir_addr[i] = rstn;
                rstn++;
                rstn = (rstn == 0XD8) ? 0xD0 : rstn;
                break;
            } else {
                invalid_data++;
            }
            i--;
        } while (i > 0);
        fwrite(st_stream->pack[1].vir_addr + st_stream->pack[1].offset + more_offset,
            st_stream->pack[1].len - st_stream->pack[1].offset - invalid_data - more_offset, 1, fd);
        fflush(fd);
    } else {
        if (cur_col != (splicing_para->splicing_info.col_cnt - 1)) {
            splicing_para->splicing_info.tmp_data[cur_col] = malloc(st_stream->pack[1].len - st_stream->pack[1].offset);
            if (splicing_para->splicing_info.tmp_data[cur_col] == XMEDIA_NULL) {
                SAMPLE_PRT("malloc tmp buf failed!\n");
                return XMEDIA_FAILURE;
            }
            memcpy(splicing_para->splicing_info.tmp_data[cur_col],
                   st_stream->pack[1].vir_addr + st_stream->pack[1].offset + more_offset,
                   st_stream->pack[1].len - st_stream->pack[1].offset - more_offset);
            splicing_para->splicing_info.tmp_data_len[cur_col] = st_stream->pack[1].len - st_stream->pack[1].offset - more_offset;

        } else {
            block_ecs_cnt = ALIGN_UP(splicing_para->splicing_info.enc_height, 16) / 16;
            if (cur_col == (splicing_para->splicing_info.col_cnt - 1)) {
                splicing_para->splicing_info.tmp_data[cur_col] = st_stream->pack[1].vir_addr + st_stream->pack[1].offset + more_offset;
                splicing_para->splicing_info.tmp_data_len[cur_col] = st_stream->pack[1].len - st_stream->pack[1].offset - more_offset;
                splicing_para->splicing_info.tmp_data_offset[cur_col] = 0;
                splicing_para->splicing_info.tmp_data_start[cur_col] = 0;
            }
            for (j = 0; j < block_ecs_cnt;j++) {
                for (i = 0;i < splicing_para->splicing_info.col_cnt;i++) {
                    tmp_data_offset = splicing_para->splicing_info.tmp_data_offset[i];
                    tmp_data_start = splicing_para->splicing_info.tmp_data_start[i];
                    tmp_data = splicing_para->splicing_info.tmp_data[i];
                    do {
                        {
                            xmedia_void *start = &tmp_data[tmp_data_offset];
                            xmedia_u32 len = splicing_para->splicing_info.tmp_data_len[i];
                            xmedia_void *pos = XMEDIA_NULL;
                            pos = memchr(start, 0xFF, len - tmp_data_offset);
                            if (pos) {
                                tmp_data_offset = pos - start + tmp_data_offset;
                            }
                        }
                        if (tmp_data[tmp_data_offset++] == 0xFF &&
                            ((tmp_data[tmp_data_offset] >= 0xD0 &&
                              tmp_data[tmp_data_offset] <= 0xD7) ||
                              tmp_data[tmp_data_offset] == 0xD9)) {
                            if (j == (block_ecs_cnt - 1) &&
                                i == (splicing_para->splicing_info.col_cnt - 1) &&
                                cur_row == (splicing_para->splicing_info.row_cnt - 1) &&
                                cur_col == (splicing_para->splicing_info.col_cnt - 1)) {
                                tmp_data_offset++;
                                rstn = 0xD0;
                                break;
                            }
                            tmp_data[tmp_data_offset] = rstn;
                            rstn++;
                            rstn = (rstn == 0xD8) ? 0xD0 : rstn;
                            tmp_data_offset++;
                            break;
                        }
                    } while (tmp_data_offset < splicing_para->splicing_info.tmp_data_len[i]);
                    fwrite(splicing_para->splicing_info.tmp_data[i] + tmp_data_start,
                        tmp_data_offset - tmp_data_start, 1, fd);
                    fflush(fd);
                    splicing_para->splicing_info.tmp_data_start[i] = tmp_data_offset;
                    splicing_para->splicing_info.tmp_data_offset[i] = tmp_data_offset;
                }
            }
            for (i = 0;i < splicing_para->splicing_info.col_cnt;i++) {
                if (splicing_para->splicing_info.tmp_data[i] != XMEDIA_NULL) {
                    if (i < splicing_para->splicing_info.col_cnt - 1) {
                        free(splicing_para->splicing_info.tmp_data[i]);
                    }
                    splicing_para->splicing_info.tmp_data[i] = XMEDIA_NULL;
                    splicing_para->splicing_info.tmp_data_offset[i] = 0;
                    splicing_para->splicing_info.tmp_data_start[i] = 0;
                }
            }
        }
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_venc_deal_jpeg_block(xmedia_u32 cur_row, xmedia_u32 cur_col, sample_venc_splicing_para *splicing_para)
{
    static xmedia_u32 snap_cnt = 0;
    static FILE *file_ptr = XMEDIA_NULL;
    static xmedia_char jpeg_file[128];
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 save_ret = XMEDIA_SUCCESS;
    xmedia_s32 select_ret;
    xmedia_venc_chn_status st_stat;
    xmedia_venc_stream st_stream;
    struct timeval timeout_val;
    xmedia_s32 venc_chn = splicing_para->venc_chn;
    xmedia_u32 venc_mask = 1 << venc_chn;
    xmedia_bool get_pack = XMEDIA_FALSE;

    while (get_pack == XMEDIA_FALSE) {
        timeout_val.tv_sec  = 2;
        timeout_val.tv_usec = 0;

        select_ret = xmedia_venc_select(venc_mask, &timeout_val);
        if (select_ret == XMEDIA_ERRCODE_INVALID_PARAM || select_ret == XMEDIA_FAILURE) {
            SAMPLE_PRT("xmedia_venc_select failed!\n");
            if (errno == EINTR) {
                SAMPLE_PRT("interrupted system call,continue\n");
                continue;
            } else {
                break;
            }
        } else if (select_ret == XMEDIA_ERRCODE_TIMEOUT) {
            continue;
        }
        memset(&st_stream, 0, sizeof(st_stream));
        ret = xmedia_venc_query_status(venc_chn, &st_stat);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("query stream failed because 0x%x \n", ret);
            continue;
        }
        if(0 == st_stat.cur_packs || 0 == st_stat.left_stream_frames) {
            SAMPLE_PRT("query stream cur_packs:%d or left_stream_frames:%d is 0 \n", \
                        st_stat.cur_packs, st_stat.left_stream_frames);
            continue;
        }
        st_stream.pack = (xmedia_venc_pack*)malloc(sizeof(xmedia_venc_pack) * st_stat.cur_packs);
        if (NULL == st_stream.pack) {
            SAMPLE_PRT("malloc memory failed!\n");
            break;
        }
        st_stream.pack_count = st_stat.cur_packs;
        ret = xmedia_venc_get_stream(venc_chn, &st_stream, -1);
        if (XMEDIA_SUCCESS != ret) {
            SAMPLE_PRT("xmedia_venc_get_stream failed with %#x!\n", ret);
            free(st_stream.pack);
            st_stream.pack = NULL;
            break;
        }
        get_pack = XMEDIA_TRUE;
        if (file_ptr == NULL) {
            snprintf(jpeg_file, sizeof(jpeg_file), "%s/chn%d_snap_%d.jpg", splicing_para->save_path, venc_chn, snap_cnt);
            file_ptr = fopen(jpeg_file, "wb");
            if (!file_ptr) {
                SAMPLE_PRT("open file[%s] err!\n", jpeg_file);
                break;
            }
            /*change mod for user*/
            chmod(jpeg_file, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        }

        save_ret = sample_comm_venc_save_jpeg_block(file_ptr, &st_stream, cur_row, cur_col, splicing_para);

        ret = xmedia_venc_release_stream(venc_chn, &st_stream);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("xmedia_venc_release_stream failed with %#x!\n", ret);
            free(st_stream.pack);
            st_stream.pack = NULL;
            return XMEDIA_FAILURE;
        }
        free(st_stream.pack);
        st_stream.pack = NULL;

        if (save_ret != XMEDIA_SUCCESS ||
            (cur_row == (splicing_para->splicing_info.row_cnt - 1) &&
             cur_col == (splicing_para->splicing_info.col_cnt - 1))) {
            fclose(file_ptr);
            file_ptr = NULL;
            snap_cnt++;
            SAMPLE_PRT("save jpeg file:%s ok\n", jpeg_file);
            memset(jpeg_file, 0, sizeof(jpeg_file));
        }
    }

    return save_ret;
}

xmedia_s32 sample_comm_venc_splicing_proc(sample_venc_splicing_para *splicing_para)
{
    xmedia_u32 i, j;
    xmedia_s32 ret = XMEDIA_SUCCESS;

    xmedia_vpss_ochn_attr  vpss_chn_attr;
    xmedia_venc_recv_pic_param recv_param = {0};
    xmedia_venc_jpeg_param jpeg_param;
    static xmedia_u32 frame_index = 0x2;
    xmedia_s32 handle;
    xmedia_vgs_frame_info task_info = {0};
    xmedia_vgs_block_info block_info = {0};
    xmedia_u32 block_width;
    xmedia_u32 block_height;
    xmedia_u32 block_align_w;
    xmedia_u32 block_remaining;

    recv_param.recv_pic_num = -1;
    xmedia_venc_start_recv_frame(splicing_para->venc_chn, &recv_param);

    /* set vpss buffer depth*/
    ret = xmedia_vpss_get_ochn_attr(splicing_para->vpss_pipe, splicing_para->vpss_chn, &vpss_chn_attr);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_vpss_get_ochn_attr err:0x%x\n",ret);
        goto error;
    }

    vpss_chn_attr.depth = 1;
    ret = xmedia_vpss_set_ochn_attr(splicing_para->vpss_pipe, splicing_para->vpss_chn, &vpss_chn_attr);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_vpss_set_ochn_attr err:0x%x\n",ret);
        goto error;
    }

    /* snap one pic and split enc after scale */
    ret = xmedia_vpss_acquire_ochn_frame(splicing_para->vpss_pipe, splicing_para->vpss_chn, &task_info.img_in, 1000);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_vpss_acquire_ochn_frame err:0x%x\n",ret);
        goto error;
    }
    if (task_info.img_in.frame.pixel_fmt > XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420) {
        SAMPLE_PRT("get pix_fmt: %d but this mode not support\n", task_info.img_in.frame.pixel_fmt);
        goto error;
    }

    if (splicing_para->splicing_info.col_cnt > 1) {
        xmedia_venc_get_jpeg_param(splicing_para->venc_chn, &jpeg_param);
        jpeg_param.mcu_per_ecs = ALIGN_UP(splicing_para->splicing_info.enc_width, 16) / 16;
        xmedia_venc_set_jpeg_param(splicing_para->venc_chn, &jpeg_param);
    }

    block_info.dst_img_size.width = splicing_para->splicing_info.dst_width;
    block_info.dst_img_size.height = splicing_para->splicing_info.dst_height;

    for (i = 0; i < splicing_para->splicing_info.row_cnt; i++) {
        for (j = 0; j < splicing_para->splicing_info.col_cnt; j++) {
            block_info.out_block_offset.x = j * splicing_para->splicing_info.enc_width;
            block_info.out_block_offset.y = i * splicing_para->splicing_info.enc_height;

            ret = xmedia_vgs_create_job(&handle);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_vgs_create_job err:0x%x\n", ret);
                goto error;
            }

            memcpy(&task_info.img_out, &task_info.img_in, sizeof(xmedia_video_frame_info));
            if ((splicing_para->splicing_info.row_cnt > 1) &&
                (i == (splicing_para->splicing_info.row_cnt - 1))) {
                block_remaining = splicing_para->splicing_info.dst_height % block_info.out_block_offset.y;
                block_height = (block_remaining > 0) ? block_remaining : splicing_para->splicing_info.enc_height;
            } else {
                block_height = splicing_para->splicing_info.enc_height;
            }
            if ((splicing_para->splicing_info.col_cnt > 1) &&
                (j == (splicing_para->splicing_info.col_cnt - 1))) {
                block_remaining = splicing_para->splicing_info.dst_width % block_info.out_block_offset.x;
                block_width = (block_remaining > 0) ? block_remaining : splicing_para->splicing_info.enc_width;
            } else {
                block_width = splicing_para->splicing_info.enc_width;
            }
            block_align_w = ALIGN_UP(splicing_para->splicing_info.enc_width, 16);
            task_info.img_out.frame.width = block_width;
            task_info.img_out.frame.height = block_height;

            task_info.img_out.frame.stride.y_stride = block_align_w;
            task_info.img_out.frame.stride.c_stride = block_align_w;
            task_info.img_out.frame.addr.y_phy_addr = splicing_para->splicing_info.phy_addr;
            task_info.img_out.frame.addr.c_phy_addr = task_info.img_out.frame.addr.y_phy_addr + \
                                                      (task_info.img_out.frame.stride.y_stride * task_info.img_out.frame.height);
            task_info.img_out.pool_id = splicing_para->splicing_info.pool_id;
            task_info.img_out.mod_id = MOD_ID_USER;
            frame_index += 2;
            task_info.img_out.frame.index = frame_index;
            task_info.img_out.frame.priv_info.isp_info_phy_addr = splicing_para->splicing_info.priv_vb_info.isp_info_phy_addr;
            task_info.img_out.frame.priv_info.isp_info_vir_addr = splicing_para->splicing_info.priv_vb_info.isp_info_vir_addr;

            ret = xmedia_vgs_add_task_block_scale(handle, &task_info, &block_info, XMEDIA_VIDEO_SCALE_MODE_NORMAL);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_vgs_add_task_block_scale err:0x%x\n", ret);
                xmedia_vgs_cancel_job(handle);
                goto error;
            }
            ret = xmedia_vgs_submit_job(handle);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_vgs_submit_job err:0x%x\n", ret);
                xmedia_vgs_cancel_job(handle);
                goto error;
            }
            ret = xmedia_vgs_wait_job(handle, 2000);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_vgs_wait_job err:0x%x\n", ret);
                xmedia_vgs_cancel_job(handle);
                goto error;
            }

            /* last scale block may less than enc width x height */
            task_info.img_out.frame.width = splicing_para->splicing_info.enc_width;
            task_info.img_out.frame.height = splicing_para->splicing_info.enc_height;

            ret = xmedia_venc_send_frame(splicing_para->venc_chn, &task_info.img_out, -1);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("xmedia_venc_send_frame err:0x%x\n", ret);

                ret = xmedia_vpss_release_ochn_frame(splicing_para->vpss_pipe, splicing_para->vpss_chn, &task_info.img_in);
                if (ret != XMEDIA_SUCCESS) {
                    SAMPLE_PRT("xmedia_vpss_release_ochn_frame err:0x%x\n", ret);
                    goto error;
                }
                goto error;
            }
            ret = sample_comm_venc_deal_jpeg_block(i, j, splicing_para);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("sample_comm_venc_deal_jpeg_block err:0x%x\n", ret);
                break;
            }
        }
    }

error:
    ret = xmedia_vpss_release_ochn_frame(splicing_para->vpss_pipe, splicing_para->vpss_chn, &task_info.img_in);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("xmedia_vpss_release_ochn_frame err:0x%x\n", ret);
    }

    xmedia_venc_stop_recv_frame(splicing_para->venc_chn);

    return ret;
}

xmedia_s32 sample_comm_venc_start_get_stream(xmedia_s32 venc_chn[], xmedia_s32 cnt, xmedia_bool save_stream, xmedia_bool save_jpeg, xmedia_char *save_path)
{
    xmedia_u32 i;

    memset(&g_get_stream_param, 0, sizeof(g_get_stream_param));
    g_get_stream_param.thread_start = XMEDIA_TRUE;
    g_get_stream_param.cnt = cnt;
    for(i = 0; i < cnt; i++) {
        g_get_stream_param.venc_chn[i] = venc_chn[i];
    }
    g_get_stream_param.save_jpeg = save_jpeg;
    g_get_stream_param.save_stream = save_stream;

    if (save_path != NULL) {
        strcpy(g_get_stream_param.save_path, save_path);
    }

    return pthread_create(&g_venc_pid, 0, sample_comm_venc_get_stream_proc, (xmedia_void*)&g_get_stream_param);
}

xmedia_s32 sample_comm_venc_stop_get_stream(void)
{
    if (XMEDIA_TRUE == g_get_stream_param.thread_start) {
        g_get_stream_param.thread_start = XMEDIA_FALSE;

        if (g_venc_pid > 0) {
            pthread_join(g_venc_pid, 0);
        }
    }
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_venc_init(xmedia_void)
{
    xmedia_venc_param_mod mod_param;

    xmedia_venc_init();

    /* VPU mini buff mode */
    mod_param.venc_mod_type = MODTYPE_H265E;
    xmedia_venc_get_mod_param(&mod_param);
    mod_param.venc_mod_type = MODTYPE_H265E;
    mod_param.h265e_mod_param.h265e_mini_buf_mode = 1;
    xmedia_venc_set_mod_param(&mod_param);

    mod_param.venc_mod_type = MODTYPE_H264E;
    xmedia_venc_get_mod_param(&mod_param);
    mod_param.venc_mod_type = MODTYPE_H264E;
    mod_param.h264e_mod_param.h264e_mini_buf_mode = 1;
    xmedia_venc_set_mod_param(&mod_param);

    mod_param.venc_mod_type = MODTYPE_JPEGE;
    xmedia_venc_get_mod_param(&mod_param);
    mod_param.venc_mod_type = MODTYPE_JPEGE;
    mod_param.jpege_mod_param.jpege_mini_buf_mode = 1;
    xmedia_venc_set_mod_param(&mod_param);

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_venc_exit(xmedia_void)
{
    if (XMEDIA_TRUE == g_qpmap_sendfrm_para.thread_start) {
        sample_comm_venc_stop_send_qpmapfrm();
    }
    sample_comm_venc_stop_get_stream();
    xmedia_venc_exit();
    return XMEDIA_SUCCESS;
}


