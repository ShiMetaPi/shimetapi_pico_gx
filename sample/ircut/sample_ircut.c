/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

#include "sample_comm.h"
#include "sample_comm_sys.h"
#include "sample_comm_isp.h"
#include "sample_comm_ircut.h"

#define GAIN_MAX_COEF               (280)
#define GAIN_MIN_COEF               (190)

#define ISO_MAX_ARR_NUM             16

#define DIV_0_TO_1(a)               ( ((a) == 0) ? 1 : (a) )

typedef enum{
    SAMPLE_IRCUT_MANUAL_MODE = 0,
    SAMPLE_IRCUT_CALIBRATION_MODE = 1,
    SAMPLE_IRCUT_AUTO_MODE = 2,

    SAMPLE_IRCUT_MODE_BUTT,
}sample_ircut_mode;

typedef enum {
    SAMPLE_IRCUT_STATUS_NORMAL = 0,
    SAMPLE_IRCUT_STATUS_IR     = 1,

    SAMPLE_IRCUT_STATUS_BUTT,
} sample_ircut_status;

typedef enum  {
    SAMPLE_IRCUT_SWITCH_NONE      = 0,
    SAMPLE_IRCUT_SWITCH_TO_NORMAL = 1,
    SAMPLE_IRCUT_SWITCH_TO_IR     = 2,

    SAMPLE_IRCUT_SWITCH_BUTT,
} sample_ircut_switch_status;

typedef struct {
    xmedia_u32  normal_to_ir_iso_thr;
    xmedia_u32  ir_to_normal_iso_thr;
    xmedia_u32  rgmax;
    xmedia_u32  rgmin;
    xmedia_u32  bgmax;
    xmedia_u32  bgmin;

    sample_ircut_status status;
    sample_ircut_switch_status switch_status;
}sample_ircut_attr;

typedef struct {
    xmedia_u8 cnt;
    xmedia_u32 iso[ISO_MAX_ARR_NUM];
}sample_ircut_ctx;

typedef struct {
    xmedia_s32 vi_dev;
    xmedia_s32 board_type;
}sample_ircut_thread_param;

static xmedia_bool g_force_exit = XMEDIA_FALSE;
static sample_ircut_attr g_ircut_attr[VI_MAX_DEV_NUM] = {0};
static sample_ircut_ctx g_ircut_ctx[VI_MAX_DEV_NUM] = {0};

xmedia_void sample_ircut_usage(xmedia_char* args)
{
    printf("Usage : %s mode board_type videv status normal2ir_iso_thr ir2normal_iso_thr rgmax rgmin bgmax bgmin\n", args);
    printf("mode:\n");
    printf("\t  0: manul.\n");
    printf("\t  1: calibration.\n");
    printf("\t  2: auto.\n");
    printf("board_type:\n");
    printf("\t  0: xm7206v10rb.\n");
    printf("\t  1: xm7206v10brba.\n");
    printf("\t  2: xm7206v10bevb.\n");
    printf("\t  3: xm7206v11arba.\n");
    printf("\t  4: xm7206v11aevb.\n");
    printf("videv:\n");
    printf("\t  0: vi 0.\n");
    printf("\t  1: vi 1.\n");
    printf("status:\n");
    printf("\t  0: ir -->  normal (manual).\n");
    printf("\t  1) normal --> ir (manual).\n");
    printf("\t  0: current ir status: normal (auto).\n");
    printf("\t  1) current ir status: ir (auto).\n");
    printf("normal2ir_iso_thr:\n");
    printf("\t  iso threshold of switching from normal to ir mode.\n");
    printf("ir2normal_iso_thr:\n");
    printf("\t  iso threshold of switching from ir to normal mode.\n");
    printf("rgmax/rgmin/bgmax/bgmin:\n");
    printf("\t  maximum(minimum) value of r/g(b/g) in ir scene.\n");
    printf("eg: manula:\n");
    printf("\t  %s 0 0 0 0\n", args);
    printf("eg: calibration:\n");
    printf("\t  %s 1 0 0\n", args);
    printf("eg: auto:\n");
    printf("\t  %s 2 0 0 0 16000 400 280 190 280 190\n", args);

    return;
}

static xmedia_s32 sample_ircut_switch_to_normal(sample_comm_board_type board_type, xmedia_s32 vi_dev) 
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_awb_attr awb_attr = {0};
    xmedia_awb_saturation_attr awb_saturation_attr = {0};
    xmedia_awb_ccm_attr awb_ccm_attr = {0};

    sample_comm_ircut_set_status_normal(board_type, vi_dev);

    ret = xmedia_awb_get_attr(vi_dev, &awb_attr);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vi dev:%d sample ircut get awb attr failed .\n", vi_dev);
        return ret;
    }
    awb_attr.op_type = XMEDIA_VIDEO_OPERATION_MODE_AUTO;
    ret = xmedia_awb_set_attr(vi_dev, &awb_attr);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vi dev:%d sample ircut set awb attr failed .\n", vi_dev);
        return ret;
    }

    ret = xmedia_awb_get_ccm_attr(vi_dev, &awb_ccm_attr);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vi dev:%d sample ircut get awb ccm attr failed .\n", vi_dev);
        return ret;
    }
    awb_ccm_attr.op_type = XMEDIA_VIDEO_OPERATION_MODE_AUTO;
    ret = xmedia_awb_set_ccm_attr(vi_dev, &awb_ccm_attr);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vi dev:%d sample ircut set awb ccm attr failed .\n", vi_dev);
        return ret;
    }

    usleep(1000000);

    ret = xmedia_awb_get_saturation_attr(vi_dev, &awb_saturation_attr);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vi dev:%d sample ircut get awb saturation attr failed .\n", vi_dev);
        return ret;
    }
    awb_saturation_attr.op_type = XMEDIA_VIDEO_OPERATION_MODE_AUTO;
    ret = xmedia_awb_set_saturation_attr(vi_dev, &awb_saturation_attr);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vi dev:%d sample ircut set awb saturation attr failed .\n", vi_dev);
        return ret;
    }

    return ret;
}

static xmedia_s32 sample_ircut_switch_to_ir(sample_comm_board_type board_type, xmedia_s32 vi_dev) 
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_awb_attr awb_attr = {0};
    xmedia_awb_saturation_attr awb_saturation_attr = {0};
    xmedia_awb_ccm_attr awb_ccm_attr = {0};

    ret = xmedia_awb_get_saturation_attr(vi_dev, &awb_saturation_attr);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vi dev:%d sample ircut get awb saturation attr failed .\n", vi_dev);
        return ret;
    }
    awb_saturation_attr.op_type = XMEDIA_VIDEO_OPERATION_MODE_MANUAL;
    awb_saturation_attr.manual_attr.saturation = 0;
    ret = xmedia_awb_set_saturation_attr(vi_dev, &awb_saturation_attr);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vi dev:%d sample ircut set awb saturation attr failed .\n", vi_dev);
        return ret;
    }

    usleep(1000000);

    ret = xmedia_awb_get_attr(vi_dev, &awb_attr);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vi dev:%d sample ircut get awb attr failed .\n", vi_dev);
        return ret;
    }
    awb_attr.op_type = XMEDIA_VIDEO_OPERATION_MODE_MANUAL;
    awb_attr.manual_attr.rgain = 0x100;
    awb_attr.manual_attr.bgain = 0x100;
    awb_attr.manual_attr.grgain = 0x100;
    awb_attr.manual_attr.gbgain = 0x100;
    ret = xmedia_awb_set_attr(vi_dev, &awb_attr);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vi dev:%d sample ircut set awb attr failed .\n", vi_dev);
        return ret;
    }

    ret = xmedia_awb_get_ccm_attr(vi_dev, &awb_ccm_attr);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vi dev:%d sample ircut get awb ccm attr failed .\n", vi_dev);
        return ret;
    }
    awb_ccm_attr.op_type = XMEDIA_VIDEO_OPERATION_MODE_MANUAL;
    awb_ccm_attr.manual_attr.ccm[0] = 0x100;
    awb_ccm_attr.manual_attr.ccm[1] = 0;
    awb_ccm_attr.manual_attr.ccm[2] = 0;
    awb_ccm_attr.manual_attr.ccm[3] = 0;
    awb_ccm_attr.manual_attr.ccm[4] = 0x100;
    awb_ccm_attr.manual_attr.ccm[5] = 0;
    awb_ccm_attr.manual_attr.ccm[6] = 0;
    awb_ccm_attr.manual_attr.ccm[7] = 0;
    awb_ccm_attr.manual_attr.ccm[8] = 0x100;
    ret = xmedia_awb_set_ccm_attr(vi_dev, &awb_ccm_attr);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vi dev:%d sample ircut set awb ccm attr failed .\n", vi_dev);
        return ret;
    }

    sample_comm_ircut_set_status_ir(board_type, vi_dev);

    return ret;
}

static xmedia_s32 sample_ircut_manual_mode(sample_comm_board_type board_type, xmedia_s32 vi_dev, xmedia_s32 status)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;

    if (status == SAMPLE_IRCUT_STATUS_NORMAL) {
        ret = sample_ircut_switch_to_normal(board_type, vi_dev);
        if (ret == XMEDIA_SUCCESS) {
            SAMPLE_PRT("vi dev:%d sample ircut switch to normal success .\n", vi_dev);
        } else {
            SAMPLE_PRT("vi dev:%d sample ircut switch to normal failed .\n", vi_dev);
        }
    } else {
        ret = sample_ircut_switch_to_ir(board_type, vi_dev);
        if (ret == XMEDIA_SUCCESS) {
            SAMPLE_PRT("vi dev:%d sample ircut switch to ir success .\n", vi_dev);
        } else {
            SAMPLE_PRT("vi dev:%d sample ircut switch to ir failed .\n", vi_dev);
        }
    }

    return ret;
}

static xmedia_s32 sample_ircut_calibration_mode(sample_comm_board_type board_type, xmedia_s32 vi_dev)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_u32 rg = 0;
    xmedia_u32 bg = 0;
    xmedia_isp_awb_statistics awb_stat = {0};
    
    ret = sample_ircut_switch_to_ir(board_type, vi_dev);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vi dev:%d sample ircut switch to ir failed .\n", vi_dev);
        return ret;
    }

    //waiting for ae to stabilize */
    usleep(1000000);

    ret = xmedia_isp_get_awb_statistics(vi_dev, &awb_stat);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vi dev:%d sample ircut get awb stat failed .\n", vi_dev);
        return ret;
    }

    rg = ((xmedia_u32)awb_stat.global_r << 8) / DIV_0_TO_1(awb_stat.global_g);
    bg = ((xmedia_u32)awb_stat.global_b << 8) / DIV_0_TO_1(awb_stat.global_g);
    SAMPLE_PRT("rgmax:%d, rgmin:%d, bgmax:%d, bgmin:%d\n",
        rg * GAIN_MAX_COEF >> 8, rg * GAIN_MIN_COEF >> 8,
        bg * GAIN_MAX_COEF >> 8, bg * GAIN_MIN_COEF >> 8);

    sample_ircut_switch_to_normal(board_type, vi_dev);

    return ret;
}

static xmedia_bool sample_ircut_check_ae_stable(xmedia_u32 now_iso, sample_ircut_ctx *ircut_ctx) 
{
    xmedia_s32 i = 0;
    xmedia_u32 iso_mean = 0;
    xmedia_u64 iso_sum = 0;
    xmedia_u64 iso_diff_sum = 0;
    CHECK_NULL_PTR(ircut_ctx);

    for (i = 0; i < ISO_MAX_ARR_NUM - 1; i++) {
        ircut_ctx->iso[i] = ircut_ctx->iso[i + 1];
    }
    ircut_ctx->iso[ISO_MAX_ARR_NUM - 1] = now_iso;

    for (i = 0; i < ISO_MAX_ARR_NUM; i++) {
        iso_sum += ircut_ctx->iso[i];
    }
    iso_mean = iso_sum / ISO_MAX_ARR_NUM;

    for (i = 0; i < ISO_MAX_ARR_NUM; i++) {
        iso_diff_sum += (abs(ircut_ctx->iso[i] - iso_mean) * (i + 1)); 
    }

    for (i = 0; i < ISO_MAX_ARR_NUM; i++) {
        if (!ircut_ctx->iso[i]) {
            break;
        }

        if (i == (ISO_MAX_ARR_NUM - 1)) {
            return (iso_diff_sum <= (iso_mean << 3));
        }
    }

    return XMEDIA_FALSE;
}

static xmedia_s32 sample_ircut_auto_run_once(xmedia_s32 vi_dev, sample_ircut_attr *ircut_attr)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_bool ae_stable = XMEDIA_FALSE;
    xmedia_u32 now_iso = 0;
    xmedia_u32 now_rg = 0;
    xmedia_u32 now_bg = 0;
    xmedia_isp_awb_statistics awb_stat = {0};
    xmedia_ae_exp_info exp_info = {0};
    sample_ircut_ctx *ctx = &g_ircut_ctx[vi_dev];

    CHECK_NULL_PTR(ircut_attr);

    ret = xmedia_isp_get_awb_statistics(vi_dev, &awb_stat);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vi dev:%d sample ircut get awb stat failed .\n", vi_dev);
        return ret;
    }
    now_rg = ((xmedia_u32)awb_stat.global_r << 8) / DIV_0_TO_1(awb_stat.global_g);
    now_bg = ((xmedia_u32)awb_stat.global_b << 8) / DIV_0_TO_1(awb_stat.global_g);

    ret = xmedia_ae_query_exposure_info(vi_dev, &exp_info);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vi dev:%d sample ircut get ae info failed .\n", vi_dev);
        return ret;
    }
    now_iso = exp_info.iso;

    if (ctx->cnt > 15) {
        ctx->cnt = 16;
        ae_stable = sample_ircut_check_ae_stable(now_iso, ctx);
        if (ae_stable) {
            if (ircut_attr->status == SAMPLE_IRCUT_STATUS_IR) {
                if (now_iso < ircut_attr->ir_to_normal_iso_thr || now_rg > ircut_attr->rgmax || \
                    now_rg < ircut_attr->rgmin || now_bg > ircut_attr->bgmax || now_bg < ircut_attr->bgmin) {
                    ircut_attr->switch_status = SAMPLE_IRCUT_SWITCH_TO_NORMAL;
                    ctx->cnt = 0;
                    return XMEDIA_SUCCESS;
                }
            }
            else if (now_iso > ircut_attr->normal_to_ir_iso_thr) {
                ircut_attr->switch_status = SAMPLE_IRCUT_SWITCH_TO_IR;
                ctx->cnt = 0;
                return XMEDIA_SUCCESS;
            }
        }
    }
    else {
        ctx->cnt++;
    }

    ircut_attr->switch_status = SAMPLE_IRCUT_SWITCH_NONE;
    return XMEDIA_SUCCESS;
}

static xmedia_void *sample_ircut_auto_thread(void *param) 
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    sample_comm_board_type board_type = 0;
    xmedia_s32 vi_dev = 0;
    sample_ircut_thread_param *thread_param = (sample_ircut_thread_param *)param;
    sample_ircut_attr ircut_attr = {0};

    if (!thread_param) {
        SAMPLE_PRT("sample ircut thread_param is null .\n");
        return XMEDIA_NULL;
    }

    vi_dev = thread_param->vi_dev;
    board_type = thread_param->board_type;
    memcpy(&ircut_attr, &g_ircut_attr[vi_dev], sizeof(sample_ircut_attr));

    while (!g_force_exit) {
        usleep(40 * 1000 * 4);

        ret = sample_ircut_auto_run_once(vi_dev, &ircut_attr);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("vi dev:%d sample ircut atou run once failed .\n", vi_dev);
            return XMEDIA_NULL;
        }

        if (ircut_attr.switch_status == SAMPLE_IRCUT_SWITCH_TO_NORMAL) {
            SAMPLE_PRT("\nir -> normal \n");
            ret = sample_ircut_switch_to_normal(board_type, vi_dev);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("vi dev:%d sample ircut switch to normal failed .\n", vi_dev);
            }
            ircut_attr.status = SAMPLE_IRCUT_STATUS_NORMAL;
        }
        else if (ircut_attr.switch_status == SAMPLE_IRCUT_SWITCH_TO_IR) {
            SAMPLE_PRT("\nnormal -> ir \n");
            ret = sample_ircut_switch_to_ir(board_type, vi_dev);
            if (ret != XMEDIA_SUCCESS) {
                SAMPLE_PRT("vi dev:%d sample ircut switch to ir failed .\n", vi_dev);
            }
            ircut_attr.status = SAMPLE_IRCUT_STATUS_IR;
        }

    }

    return XMEDIA_NULL;
}

static xmedia_s32 sample_ircut_auto_mode(sample_comm_board_type board_type, xmedia_s32 vi_dev)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    pthread_t thread_tid;
    sample_ircut_thread_param thread_param = {0};

    g_force_exit = XMEDIA_FALSE;
    thread_param.board_type = board_type;
    thread_param.vi_dev = vi_dev;
    ret = pthread_create(&thread_tid, 0, sample_ircut_auto_thread, &thread_param);
    if (ret) {
        SAMPLE_PRT("sample ircut create pthread failed .\n");
        return XMEDIA_FAILURE;
    }

    PAUSE(g_force_exit);
    g_force_exit = XMEDIA_TRUE;
    pthread_join(thread_tid, XMEDIA_NULL);

    return ret;
}

xmedia_void sample_ircut_handle_sig(xmedia_s32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo || SIGTERM == signo) {
        g_force_exit = XMEDIA_TRUE;
    }
}

int main(int argc,char **argv)
{
    xmedia_s32 mode = 0;
    sample_comm_board_type board_type = 0;
    xmedia_s32 vi_dev = 0;
    xmedia_s32 status = 0;

    signal(SIGINT, sample_ircut_handle_sig);
    signal(SIGTERM, sample_ircut_handle_sig);

    if (argc < 4) {
        sample_ircut_usage(argv[0]);
        return 0;
    }

    mode = atoi(argv[1]);
    board_type = (sample_comm_board_type)atoi(argv[2]);
    vi_dev = atoi(argv[3]);

    if (vi_dev > VI_MAX_DEV_NUM) {
        SAMPLE_PRT("input vi dev num(%d) > vi max dev num(%d) .\n", vi_dev, VI_MAX_DEV_NUM);
        return 0;
    }

    if (argc == 5 && mode == SAMPLE_IRCUT_MANUAL_MODE) {
        status = !!atoi(argv[4]);
        sample_ircut_manual_mode(board_type, vi_dev, status);
    }
    else if (argc == 4 && mode == SAMPLE_IRCUT_CALIBRATION_MODE) {
        sample_ircut_calibration_mode(board_type, vi_dev);
    }
    else if (argc == 11 && mode == SAMPLE_IRCUT_AUTO_MODE) {
        g_ircut_attr[vi_dev].status = !!atoi(argv[4]);
        g_ircut_attr[vi_dev].normal_to_ir_iso_thr = atoi(argv[5]);
        g_ircut_attr[vi_dev].ir_to_normal_iso_thr = atoi(argv[6]);
        g_ircut_attr[vi_dev].rgmax = atoi(argv[7]);
        g_ircut_attr[vi_dev].rgmin = atoi(argv[8]);
        g_ircut_attr[vi_dev].bgmax = atoi(argv[9]);
        g_ircut_attr[vi_dev].bgmin = atoi(argv[10]);
        sample_ircut_auto_mode(board_type, vi_dev);
    }
    else {
        sample_ircut_usage(argv[0]);
    }

    return 0;
}