
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "sample_comm_sys.h"
#include "sample_comm_vgs.h"
#include "sample_comm_vo.h"
#include "sample_comm_vpss.h"
#include "sample_comm_venc.h"
#include "sample_comm_livestream.h"
#include "sample_comm.h"
#include "sample_comm_vi.h"
#include "sample_comm_isp.h"

xmedia_bool pause_flag = 0;

#define SAMPLE_VO_YUV_PATH_SRC "./res/"

#define SAMPLE_VO_VDEC_FILE_1920X1080 SAMPLE_VO_YUV_PATH_SRC"1920x1080_normalP_103_school_10Mbps.h265"
#define SAMPLE_VO_VDEC_FILE_256X256 SAMPLE_VO_YUV_PATH_SRC"256x256_402_Main@l2_30Mbps.h265"
#define SAMPLE_VO_VDEC_FILE_256X256_171 SAMPLE_VO_YUV_PATH_SRC"256x256_171_slice_Main@l2.h265"
#define SAMPLE_VO_VDEC_FILE_3840X2160 SAMPLE_VO_YUV_PATH_SRC"venc_mst_fun_road_tree_3840x2160_60_chn0.h265"

#define SAMPLE_VO_YUV_1920X1080_420SP SAMPLE_VO_YUV_PATH_SRC"1920x1080_Y_UV20.yuv"
#define SAMPLE_VO_YUV_800X1280_420SP SAMPLE_VO_YUV_PATH_SRC"800x1280_Y_UV20.yuv"
#define SAMPLE_VO_YUV_800X480_420SP SAMPLE_VO_YUV_PATH_SRC"800x480_Y_UV20.yuv"
#define SAMPLE_VO_YUV_400X240_420SP SAMPLE_VO_YUV_PATH_SRC"400x240_Y_UV20.yuv"
#define SAMPLE_VO_YUV_400X640_420SP SAMPLE_VO_YUV_PATH_SRC"400x640_Y_UV20.yuv"
#define SAMPLE_VO_YUV_960X540_420SP SAMPLE_VO_YUV_PATH_SRC"960x540_Y_UV20.yuv"
#define SAMPLE_VO_YUV_320X240_420SP SAMPLE_VO_YUV_PATH_SRC"320x240_Y_UV20.yuv"
#define SAMPLE_VO_YUV_1280X720_420SP SAMPLE_VO_YUV_PATH_SRC"1280x720_Y_UV20.yuv"
#define SAMPLE_VO_YUV_1280X800_420SP SAMPLE_VO_YUV_PATH_SRC"1280x800_Y_UV20.yuv"
#define SAMPLE_VO_YUV_1920X720_420SP SAMPLE_VO_YUV_PATH_SRC"1920x720_Y_UV20.yuv"
#define SAMPLE_VO_YUV_240X320_420SP SAMPLE_VO_YUV_PATH_SRC"240x320_Y_UV20.yuv"
#define SAMPLE_VO_YUV_3840X2160_420SP SAMPLE_VO_YUV_PATH_SRC"3840x2160_Y_UV20.yuv"
#define SAMPLE_VO_YUV_1920X384_420SP SAMPLE_VO_YUV_PATH_SRC"1920x384_Y_UV20.yuv"
#define SAMPLE_VO_YUV_320X480_420SP SAMPLE_VO_YUV_PATH_SRC"320x480_Y_UV20.yuv"
#define SAMPLE_VO_YUV_960X1080_420SP SAMPLE_VO_YUV_PATH_SRC"960x1080_Y_UV20.yuv"
#define SAMPLE_VO_YUV_640X720_420SP SAMPLE_VO_YUV_PATH_SRC"640x720_Y_UV20.yuv"
#define SAMPLE_VO_YUV_400X480_420SP SAMPLE_VO_YUV_PATH_SRC"400x480_Y_UV20.yuv"
#define SAMPLE_VO_YUV_120X320_420SP SAMPLE_VO_YUV_PATH_SRC"120x320_Y_UV20.yuv"
#define SAMPLE_VO_YUV_160X240_420SP SAMPLE_VO_YUV_PATH_SRC"160x240_Y_UV20.yuv"
#define SAMPLE_VO_YUV_640X360_420SP SAMPLE_VO_YUV_PATH_SRC"640x360_Y_UV20.yuv"

typedef enum {
    SAMPLE_VO_VDEC_ES_TYPE_1920X1080_H265_FRAME = 0,
    SAMPLE_VO_VDEC_ES_TYPE_256X256_H265_FRAME,
    SAMPLE_VO_VDEC_ES_TYPE_3840X2160_H265_FRAME,
    SAMPLE_VO_VDEC_ES_TYPE_1920X1080_H265_STREAM,
    SAMPLE_VO_VDEC_ES_TYPE_256X256_H265_STREAM,
    SAMPLE_VO_VDEC_ES_TYPE_3840X2160_H265_STREAM,
    SAMPLE_VO_VDEC_ES_TYPE_MAX,
} sample_vo_vdec_es_type;

typedef struct {
    xmedia_video_size vpss_ichn_size[VPSS_MAX_PIPE_NUM];
    xmedia_s32 vpss_pipe_num;
    xmedia_s32 vpss_pipe_ochn_num[VPSS_MAX_PIPE_NUM];
    xmedia_video_size vpss_ochn_size[VPSS_MAX_PIPE_NUM][VPSS_MAX_OCHN_NUM];
    xmedia_vpss_ochn_mode vpss_mode;
    sample_vpss_config vpss_config;

    sample_vo_config vo_config[VO_MAX_DEV_NUM];
    xmedia_s32 vo_dev_num;
    xmedia_s32 vo_chn_num[VO_MAX_DEV_NUM];
    xmedia_s32 vo_chn[VO_MAX_DEV_NUM][VO_MAX_CHN_NUM];

    sample_comm_video_param video_param;

    //venc
    xmedia_s32 venc_chn_num;
    xmedia_s32 venc_chn[1];
    xmedia_video_size venc_out_size[1];
    sample_venc_config venc_config;
} sample_vo_all_mod_cfg;

#define SAMPLE_VO_FILE_NAME_MAX_LEN 256

typedef struct{
    xmedia_u32 width;
    xmedia_u32 height;
    xmedia_char *file_name;
} sample_vo_file_info;

typedef enum{
    SAMPLE_VO_SRC_FILE_1920X1080_YUV420SP = 0,
    SAMPLE_VO_SRC_FILE_800X1280_YUV420SP,
    SAMPLE_VO_SRC_FILE_800X480_YUV420SP,
    SAMPLE_VO_SRC_FILE_400X640_YUV420SP,
    SAMPLE_VO_SRC_FILE_400X240_YUV420SP,
    SAMPLE_VO_SRC_FILE_960X540_YUV420SP,
    SAMPLE_VO_SRC_FILE_320X240_YUV420SP,
    SAMPLE_VO_SRC_FILE_1280X720_YUV420SP,
    SAMPLE_VO_SRC_FILE_1280X800_YUV420SP,
    SAMPLE_VO_SRC_FILE_1920X720_YUV420SP,
    SAMPLE_VO_SRC_FILE_240X320_YUV420SP,
    SAMPLE_VO_SRC_FILE_3840X2160_YUV420SP,
    SAMPLE_VO_SRC_FILE_1920X384_YUV420SP,
    SAMPLE_VO_SRC_FILE_320X480_420SP,
    SAMPLE_VO_SRC_FILE_960X1080_420SP,
    SAMPLE_VO_SRC_FILE_640X720_420SP,
    SAMPLE_VO_SRC_FILE_400X480_420SP,
    SAMPLE_VO_SRC_FILE_120X320_420SP,
    SAMPLE_VO_SRC_FILE_160X240_420SP,
    SAMPLE_VO_SRC_FILE_640X360_420SP,
    SAMPLE_VO_SRC_FILE_MAX,
}sample_vo_src_file;

sample_vo_file_info g_sample_vo_src_file[SAMPLE_VO_SRC_FILE_MAX] = {
    {1920, 1080, SAMPLE_VO_YUV_1920X1080_420SP},
    {800, 1280, SAMPLE_VO_YUV_800X1280_420SP},
    {800, 480, SAMPLE_VO_YUV_800X480_420SP},
    {400, 640, SAMPLE_VO_YUV_400X640_420SP},
    {400, 240, SAMPLE_VO_YUV_400X240_420SP},
    {960, 540, SAMPLE_VO_YUV_960X540_420SP},
    {320, 240, SAMPLE_VO_YUV_320X240_420SP},
    {1280, 720, SAMPLE_VO_YUV_1280X720_420SP},
    {1280, 800, SAMPLE_VO_YUV_1280X800_420SP},
    {1920, 720, SAMPLE_VO_YUV_1920X720_420SP},
    {240, 320, SAMPLE_VO_YUV_240X320_420SP},
    {3840, 2160, SAMPLE_VO_YUV_3840X2160_420SP},
    {1920, 384, SAMPLE_VO_YUV_1920X384_420SP},
    {320, 480, SAMPLE_VO_YUV_320X480_420SP},
    {960,1080,SAMPLE_VO_YUV_960X1080_420SP},
    {640,720,SAMPLE_VO_YUV_640X720_420SP},
    {400,480,SAMPLE_VO_YUV_400X480_420SP},
    {120,320,SAMPLE_VO_YUV_120X320_420SP},
    {160,240,SAMPLE_VO_YUV_160X240_420SP},
    {640,360,SAMPLE_VO_YUV_640X360_420SP},
};

typedef struct{
    char filename[SAMPLE_VO_FILE_NAME_MAX_LEN];
    pthread_t tid;
    xmedia_u32 width;
    xmedia_u32 height;
    xmedia_video_pixel_format pix_format;
    xmedia_video_dynamic_range dynmic_range;
    xmedia_video_color_descript csc_info;
    xmedia_vo_dev dev;
    xmedia_u32 chn_num;
    xmedia_s32 chn_id;
    xmedia_bool run_flag;
    xmedia_u32 time_out_ms;
    xmedia_video_format video_fmt;
    xmedia_s32 send_frame_delay;
} sample_vo_send_thread_info;

static xmedia_s32 sample_vo_sys_init(sample_vo_all_mod_cfg *all_mod_cfg)
{
    xmedia_video_size pic_size = {0};
    sample_sys_config sys_config = {0};
    xmedia_u32 buf_size = 0;
    // vpss vb pool config
    pic_size.width = all_mod_cfg->vo_config[0].layer_config.img_size.width;
    pic_size.height = all_mod_cfg->vo_config[0].layer_config.img_size.height;
    buf_size = sample_comm_sys_get_buffer_size(pic_size, all_mod_cfg->video_param.video_fmt, all_mod_cfg->video_param.pixel_fmt,
                                               all_mod_cfg->video_param.data_width, all_mod_cfg->video_param.compress_mode);
    sys_config.vb_conf.common_pool[1].block_size = buf_size;
    sys_config.vb_conf.common_pool[1].block_cnt = 10;
    sys_config.vb_conf.common_pool[1].map_mode = XMEDIA_VB_MAP_MODE_NONE;
    // vo vb pool config
    pic_size.width = all_mod_cfg->vo_config[0].layer_config.img_size.width;
    pic_size.height = all_mod_cfg->vo_config[0].layer_config.img_size.height;
    buf_size = sample_comm_sys_get_buffer_size(pic_size, all_mod_cfg->video_param.video_fmt, all_mod_cfg->video_param.pixel_fmt,
                all_mod_cfg->video_param.data_width, all_mod_cfg->video_param.compress_mode);
    sys_config.vb_conf.common_pool[0].block_size = buf_size;
    sys_config.vb_conf.common_pool[0].block_cnt = 12;
    sys_config.vb_conf.common_pool[0].map_mode = XMEDIA_VB_MAP_MODE_NONE;

    if(all_mod_cfg->vo_dev_num == 2) {
        pic_size.width = all_mod_cfg->vo_config[1].layer_config.img_size.width;
        pic_size.height = all_mod_cfg->vo_config[1].layer_config.img_size.height;
        buf_size = sample_comm_sys_get_buffer_size(pic_size, all_mod_cfg->video_param.video_fmt, all_mod_cfg->video_param.pixel_fmt,
            all_mod_cfg->video_param.data_width, all_mod_cfg->video_param.compress_mode);
        sys_config.vb_conf.common_pool[2].block_size = buf_size;
        sys_config.vb_conf.common_pool[2].block_cnt = 6;
        sys_config.vb_conf.common_pool[2].map_mode = XMEDIA_VB_MAP_MODE_NONE;
    }

    if (all_mod_cfg->venc_chn_num == 1) {
        pic_size.width = all_mod_cfg->vo_config[0].layer_config.img_size.width;
        pic_size.height = all_mod_cfg->vo_config[0].layer_config.img_size.height;
        buf_size = sample_comm_sys_get_buffer_size(pic_size, all_mod_cfg->video_param.video_fmt, all_mod_cfg->video_param.pixel_fmt,
            all_mod_cfg->video_param.data_width, all_mod_cfg->video_param.compress_mode);
        sys_config.vb_conf.common_pool[3].block_size = buf_size;
        sys_config.vb_conf.common_pool[3].block_cnt = 6;
        sys_config.vb_conf.common_pool[3].map_mode = XMEDIA_VB_MAP_MODE_NONE;
    }

    sys_config.vb_conf.max_pool_cnt = 20;
    sys_config.vb_conf.supplement_config = XMEDIA_VB_SUPPLEMENT_VDEC_INFO_ENABLE;

    return sample_comm_sys_init(&sys_config);
}

static xmedia_s32 sample_vivo_sys_init(sample_sys_config *sys_config)
{
    xmedia_s32 ret = 0;

    ret = sample_comm_sys_init(sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_sys_init failed!\n");
        return ret;
    }

    ret = sample_comm_vgs_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vgs_init failed!\n");
        return ret;
    }

    // in online-online mode,vi and vpss must be reset at the same time
    ret = sample_comm_vi_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vi_init failed!\n");
        return ret;
    }

    ret = sample_comm_vpss_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vpss_init failed!\n");
        return ret;
    }

    ret = sample_comm_vo_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vo_init failed!\n");
        return ret;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_void sample_vivo_sys_exit(void)
{
    sample_comm_venc_stop_get_stream();
    sample_comm_vo_exit();
    sample_comm_vpss_exit();
    sample_comm_vi_exit();
    sample_comm_vgs_exit();
    sample_comm_sys_exit();
}

xmedia_u32 g_sample_vo_pool_id[XMEDIA_VO_DEV_MAX][VO_MAX_CHN_NUM];
typedef struct {
    xmedia_u64 y_head_vir_addr;
    xmedia_u64 c_head_vir_addr;
    xmedia_u64 y_vir_addr;
    xmedia_u64 c_vir_addr;
}sample_vo_vb_vir;
typedef struct {
    xmedia_video_frame_info frame_info;
    sample_vo_vb_vir vb_vir;
}sample_vo_vb;

static xmedia_void sample_vo_fill_video_frame(sample_vo_vb *vo_vb, xmedia_vb_base_info *vb_base,
    sample_vo_send_thread_info* pst_thd_info,xmedia_vb_cal_cfg *vb_cal_cfg, xmedia_handle vb_handle)
{
    static xmedia_u32 frame_index = 0;
    struct timeval sys_time;
    gettimeofday(&sys_time, XMEDIA_NULL);
    vo_vb->frame_info.pool_id = xmedia_vb_handle_to_pool_id(vb_handle);
    vo_vb->frame_info.frame.width = vb_base->width;
    vo_vb->frame_info.frame.height = vb_base->height;
    vo_vb->frame_info.frame.dynamic_range = XMEDIA_VIDEO_DYNAMIC_RANGE_SDR;
    vo_vb->frame_info.frame.color_info.color_gamut = pst_thd_info->csc_info.color_gamut;
    vo_vb->frame_info.frame.color_info.color_space = pst_thd_info->csc_info.color_space;
    vo_vb->frame_info.frame.color_info.quantify_range = pst_thd_info->csc_info.quantify_range;
    vo_vb->frame_info.frame.pixel_fmt = vb_base->pixel_fmt;
    vo_vb->frame_info.frame.pts  = 0;
    vo_vb->frame_info.frame.compress_mode = vb_base->cmp_mode;
    vo_vb->frame_info.frame.video_fmt = vb_base->video_fmt;
    vo_vb->frame_info.frame.bit_width = vb_base->bit_width;
    vo_vb->frame_info.mod_id = MOD_ID_VO;
    vo_vb->frame_info.frame.stride.y_head_stride = vb_cal_cfg->head_y_stride;
    vo_vb->frame_info.frame.stride.y_stride = vb_cal_cfg->main_stride;

    vo_vb->frame_info.frame.addr.y_head_phy_addr = xmedia_vb_handle_to_phy_addr(vb_handle);
    vo_vb->frame_info.frame.addr.c_head_phy_addr = vo_vb->frame_info.frame.addr.y_head_phy_addr + vb_cal_cfg->head_y_size;
    vo_vb->frame_info.frame.addr.y_phy_addr = vo_vb->frame_info.frame.addr.y_head_phy_addr + vb_cal_cfg->head_size;
    vo_vb->frame_info.frame.addr.c_phy_addr = vo_vb->frame_info.frame.addr.y_phy_addr + vb_cal_cfg->main_y_size;

    vo_vb->frame_info.frame.stride.y_head_stride = vb_cal_cfg->head_y_stride;
    vo_vb->frame_info.frame.stride.c_head_stride = vb_cal_cfg->head_c_stride;
    vo_vb->frame_info.frame.stride.y_stride =  vb_cal_cfg->main_stride;
    vo_vb->frame_info.frame.stride.c_stride =  vb_cal_cfg->main_stride;
    vo_vb->frame_info.frame.index = frame_index;
    frame_index += 2;
    vo_vb->frame_info.frame.pts = sys_time.tv_sec * 1000000 + sys_time.tv_usec;

    vo_vb->vb_vir.y_head_vir_addr = (xmedia_u64)(xmedia_ulong)xmedia_mmz_map(vo_vb->frame_info.frame.addr.y_head_phy_addr,
        vb_cal_cfg->vb_size, XMEDIA_FALSE);
    vo_vb->vb_vir.c_head_vir_addr = vo_vb->vb_vir.y_head_vir_addr + vb_cal_cfg->head_y_size;
    vo_vb->vb_vir.y_vir_addr = vo_vb->vb_vir.y_head_vir_addr + vb_cal_cfg->head_size;
    vo_vb->vb_vir.c_vir_addr = vo_vb->vb_vir.y_vir_addr + vb_cal_cfg->main_y_size;
    memset((xmedia_u8*)(xmedia_ulong)vo_vb->vb_vir.y_head_vir_addr, 0, vb_cal_cfg->vb_size);
}

static xmedia_void sample_vo_free_video_frame(sample_vo_vb *vo_vb)
{
    if (xmedia_mmz_unmap((xmedia_void *)(xmedia_ulong)vo_vb->vb_vir.y_head_vir_addr) != XMEDIA_SUCCESS) {
        printf("%s %d Unmap vb failed\n", __func__, __LINE__);
    }
}

static xmedia_s32 sample_vo_buffer_read_from_file_ex(sample_vo_vb *vo_vb, FILE *fp)
{
    xmedia_u8 *y_dst = XMEDIA_NULL;
    xmedia_u8 *uv_dst = XMEDIA_NULL;
    xmedia_u32 row;
    xmedia_u32 y_width = vo_vb->frame_info.frame.width;
    xmedia_u32 y_height = vo_vb->frame_info.frame.height;
    xmedia_u32 y_stride;
    xmedia_u32 c_width;
    xmedia_u32 c_height;
    xmedia_u32 c_stride;

    y_stride = vo_vb->frame_info.frame.stride.y_stride;
    y_dst = (xmedia_u8 *)(xmedia_ulong)vo_vb->vb_vir.y_vir_addr;
    uv_dst = (xmedia_u8 *)(xmedia_ulong)vo_vb->vb_vir.c_vir_addr;

    /* read y data */
    for (row = 0; row < y_height; row++) {
        fread(y_dst, y_width, 1, fp);
        y_dst += y_stride;
    }

    if (vo_vb->frame_info.frame.pixel_fmt == XMEDIA_VIDEO_PIXEL_FMT_YUV_400) {
        return XMEDIA_SUCCESS;
    }

    switch (vo_vb->frame_info.frame.pixel_fmt) {
        case XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420:
        case XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_420:
            c_width = y_width / 2;
            c_height = y_height / 2;
            c_stride = y_stride;
            break;
        case XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_422: /* 默认水平下采样，暂不考虑垂直下采样 */
        case XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_422:
            c_width = y_width / 2;
            c_height = y_height;
            c_stride = y_stride;
            break;
        case XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_444:
        case XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_444:
            c_width = y_width;
            c_height = y_height;
            c_stride = y_stride * 2;
            break;
        default:
            printf("[%s:%d]:pixel_fmt invalid, actual:%d!\n", __FUNCTION__, __LINE__, vo_vb->frame_info.frame.pixel_fmt);
            return XMEDIA_FAILURE;
    }

    /* read uv data */
    for (row = 0; row < c_height; row++) {
        fread(uv_dst, c_width * 2, 1, fp);
        uv_dst += c_stride;
    }
    return XMEDIA_SUCCESS;
}


xmedia_void* sample_vo_send_frame(xmedia_void* args)
{
    sample_vo_send_thread_info* pst_thd_info;
    sample_vo_vb vo_vb = {0};
    FILE* fpStrm = XMEDIA_NULL;
    xmedia_vo_layer layer;
    xmedia_s32 chn = 0;
    xmedia_u32 pool_id;
    xmedia_u32 time_out = 0;
    xmedia_char temp[1];
    xmedia_s32 ret = 0;
    xmedia_vb_base_info base_info = {0};
    xmedia_vb_cal_cfg vb_cal_cfg = {0};
    xmedia_vb_pool_config pool_cfg = { 0 };
    xmedia_handle vb_handle;

    pst_thd_info = (sample_vo_send_thread_info*)args;

    layer = pst_thd_info->dev;
    chn = pst_thd_info->chn_id;

    time_out = pst_thd_info->time_out_ms;

    fpStrm = fopen(pst_thd_info->filename, "rb");
    if (fpStrm == XMEDIA_NULL){
        printf("can't open file %s in send stream thread\n", pst_thd_info->filename);
        return XMEDIA_NULL;
    }
    fflush(stdout);

    base_info.width = pst_thd_info->width;
    base_info.height = pst_thd_info->height;
    base_info.pixel_fmt = pst_thd_info->pix_format;
    base_info.bit_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    base_info.cmp_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;
    base_info.align = DEFAULT_ALIGN;
    base_info.video_fmt = pst_thd_info->video_fmt;
    xmedia_vb_get_buffer_config(&base_info, &vb_cal_cfg);
    memset(&pool_cfg, 0, sizeof(xmedia_vb_pool_config));
    pool_cfg.block_cnt = 3;
    pool_cfg.block_size = vb_cal_cfg.vb_size;
    pool_cfg.map_mode = XMEDIA_VB_MAP_MODE_NONE;
    pool_id = xmedia_vb_create_pool(&pool_cfg);
    if (pool_id == VB_INVALID_POOLID) {
        printf("xmedia_vb_create_pool failed!\n");
        return XMEDIA_NULL;
    }

    if(pool_id == VB_INVALID_POOLID){
         printf("create vb pool failed\n");
         fclose(fpStrm);
         return XMEDIA_NULL;
    }

    g_sample_vo_pool_id[pst_thd_info->dev][pst_thd_info->chn_id] = pool_id;

    pst_thd_info->run_flag = XMEDIA_TRUE;
    do
    {
        if (feof(fpStrm) != 0) { //到文件末尾则从文件头开始读
            fseek(fpStrm, 0, SEEK_SET);
        }
        vb_handle = xmedia_vb_get_block(pool_id, vb_cal_cfg.vb_size, XMEDIA_NULL);
        if (vb_handle == VB_INVALID_HANDLE) {
            usleep(10*1000);
            continue;
        }
        sample_vo_fill_video_frame(&vo_vb, &base_info, pst_thd_info, &vb_cal_cfg, vb_handle);
        if(sample_vo_buffer_read_from_file_ex(&vo_vb, fpStrm) != XMEDIA_SUCCESS) {
            printf("[%s:%d]:sample_vo_buffer_read_from_file_ex err!\n", __FUNCTION__, __LINE__);
        }

        //每读完一帧，fp做矫正
        fread(temp, 1, 1, fpStrm);
        if (feof(fpStrm) == 0) {
            fseek(fpStrm, ftell(fpStrm) - 1, SEEK_SET);
        }

        ret = xmedia_vo_send_chn_frame(layer, chn, &vo_vb.frame_info, time_out);
        if(ret != XMEDIA_SUCCESS) {
            printf("[%s:%d]:xmedia_vo_send_chn_frame err!\n", __FUNCTION__, __LINE__);
        }

        //添加送帧延时，以免送帧过快
        if(pst_thd_info->send_frame_delay != -1){
            usleep(pst_thd_info->send_frame_delay * 1000);
        }

        sample_vo_free_video_frame(&vo_vb);
        ret = xmedia_vb_release_block(vb_handle);
        if (ret != XMEDIA_SUCCESS) {
            printf("Func:%s,%d Info:xmedia_vb_release_block failed\n", __FUNCTION__, __LINE__);
            return XMEDIA_NULL;
        }
    } while(pst_thd_info->run_flag == XMEDIA_TRUE);

    fclose(fpStrm);

    printf("exit file to vo dev:%d,,wxh:%dx%d!!!\n", pst_thd_info->dev, pst_thd_info->width, pst_thd_info->height);

    return XMEDIA_NULL;
}

static xmedia_s32 sample_vo_start_send_thread(sample_vo_send_thread_info *thd_info)
{
    if (thd_info == XMEDIA_NULL) {
        return XMEDIA_FAILURE;
    }

    pthread_create(&(thd_info->tid), 0, sample_vo_send_frame, (xmedia_void *)thd_info);
    return XMEDIA_SUCCESS;
}

sample_vo_file_info *sample_vo_match_src_file(xmedia_s32 width, xmedia_s32 height)
{
    xmedia_s32 i;
    for(i = 0;i < SAMPLE_VO_SRC_FILE_MAX;i++) {
        if((width == g_sample_vo_src_file[i].width) && (height == g_sample_vo_src_file[i].height)) {
            return &g_sample_vo_src_file[i];
        }
    }
    return XMEDIA_NULL;
}

xmedia_s32 sample_vo_get_thread_default_config(sample_vo_send_thread_info *thd_info)
{
    if (thd_info == XMEDIA_NULL) {
        return XMEDIA_FAILURE;
    }
    thd_info->run_flag = XMEDIA_FALSE;
    thd_info->pix_format = XMEDIA_VIDEO_PIXEL_FMT_YUV_SEMIPLANAR_420;
    thd_info->width = 1920;
    thd_info->height = 1080;
    thd_info->dev = XMEDIA_VO_DEV_0;
    thd_info->dynmic_range = XMEDIA_VIDEO_DYNAMIC_RANGE_SDR;
    thd_info->chn_num = 1;
    thd_info->chn_id = 0;
    thd_info->tid = 0;
    thd_info->time_out_ms = 30;
    thd_info->csc_info.color_gamut = XMEDIA_VIDEO_COLOR_GAMUT_BT709;
    thd_info->csc_info.color_space = XMEDIA_VIDEO_COLOR_SPACE_YUV;
    thd_info->csc_info.quantify_range = XMEDIA_VIDEO_COLOR_FULL_RANGE;
    thd_info->video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    thd_info->send_frame_delay = -1;
    return 0;
}

xmedia_s32 sample_vo_stop_send_thread(sample_vo_send_thread_info *thd_info)
{
    if (thd_info == XMEDIA_NULL) {
        return XMEDIA_FAILURE;
    }

    thd_info->run_flag = XMEDIA_FALSE;
    pthread_join(thd_info->tid, XMEDIA_NULL);
    return XMEDIA_SUCCESS;
}

sample_vo_send_thread_info g_thread_info[VO_MAX_DEV_NUM][VO_MAX_CHN_NUM];

xmedia_void sample_vo_handle_sig(xmedia_s32 signo)
{
    xmedia_u32 chn = 0;
    xmedia_u32 dev = 0;
    if (SIGINT == signo || SIGTSTP == signo || SIGTERM == signo) {
        for (dev = 0; dev < VO_MAX_DEV_NUM; dev++) {
            for (chn = 0; chn < VO_MAX_CHN_NUM; chn++) {
                if(g_thread_info[dev][chn].run_flag == XMEDIA_TRUE) {
                    sample_vo_stop_send_thread(&g_thread_info[dev][chn]);
                }
            }
        }
        xmedia_vb_exit();
        xmedia_sys_exit();
        SAMPLE_PRT("\033[0;31mprogram exit abnormally!\033[0;39m\n");
    }
    exit(0);
}

static xmedia_s32 sample_vo_test(xmedia_s32 vo_chn_num, sample_comm_screen_type screen0,
    xmedia_vo_partition_mode vo_part_mode)
{
    xmedia_s32 ret;
    char c = 0;
    xmedia_u32 i = 0;
    xmedia_u32 chn = 0;
    xmedia_u32 dev = 0;

    sample_vo_config_input vo_input = { 0 };
    sample_vo_all_mod_cfg all_mod_cfg;
    sample_vo_file_info *file_info;
    memset(&all_mod_cfg, 0, sizeof(sample_vo_all_mod_cfg));

    for (dev = 0; dev < VO_MAX_DEV_NUM; dev++) {
        for (chn = 0; chn < VO_MAX_CHN_NUM; chn++) {
            memset(&g_thread_info[dev][chn], 0 , sizeof(sample_vo_send_thread_info));
        }
    }
    all_mod_cfg.vo_dev_num = 1;
    all_mod_cfg.vo_chn_num[0] = vo_chn_num;

    for (i = 0; i < VO_MAX_DEV_NUM; i++) {
        all_mod_cfg.vo_chn[0][i] = i;
    }

    vo_input.square_sort = all_mod_cfg.vo_chn_num[0];
    vo_input.screen_type = screen0;
    ret = sample_comm_vo_get_default_config(vo_input, &all_mod_cfg.vo_config[0]);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vo_get_default_config failed !\n");
        return ret;
    }

    ret = sample_vo_sys_init(&all_mod_cfg);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_vo_sys_init failed !\n");
        return ret;
    }

    ret = sample_comm_vo_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vo init fail!\n");
        goto OUT1;
    }

    all_mod_cfg.vo_config[0].layer_config.part_mode = vo_part_mode;
    ret = sample_comm_vo_start(0, 0, all_mod_cfg.vo_chn[0], all_mod_cfg.vo_chn_num[0], XMEDIA_TRUE, &all_mod_cfg.vo_config[0]);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vo start failed !\n");
        goto OUT2;
    }

    file_info = sample_vo_match_src_file( all_mod_cfg.vo_config[0].chn_attr[0].out_rect.width,
        all_mod_cfg.vo_config[0].chn_attr[0].out_rect.height);
    if(file_info == XMEDIA_NULL) {
        printf("file_info : XMEDIA_NULL\n");
        goto OUT3;
    }
    for (i = 0; i < vo_chn_num; i++) {
        sample_vo_get_thread_default_config(&g_thread_info[0][i]);
        g_thread_info[0][i].dev = 0;
        g_thread_info[0][i].chn_id = all_mod_cfg.vo_chn[0][i];
        memcpy(g_thread_info[0][i].filename, file_info->file_name, strlen(file_info->file_name) + 1);
        g_thread_info[0][i].pix_format = all_mod_cfg.vo_config[0].layer_config.pix_format;
        g_thread_info[0][i].width = file_info->width;
        g_thread_info[0][i].height = file_info->height;
        g_thread_info[0][i].tid = i;
        sample_vo_start_send_thread(&g_thread_info[0][i]);
    }

    while(1) {
        SAMPLE_PRT("\nSAMPLE_TEST:press 'e' to exit; !\n");
        c = getchar();
        if (c == 'e') {
            break;
        }
    }
    for (i = 0; i < vo_chn_num; i++) {
        sample_vo_stop_send_thread(&g_thread_info[0][i]);
    }

OUT3:
    sample_comm_vo_stop(0, 0, all_mod_cfg.vo_chn[0], all_mod_cfg.vo_chn_num[0]);
OUT2:
    sample_comm_vo_exit();
OUT1:
    sample_comm_sys_exit();
    return ret;
}

static xmedia_s32 sample_vo_virt_dev_test(xmedia_vo_dev vo_virt_dev, xmedia_s32 vo_chn_num,
    sample_comm_screen_type screen0_type)
{
    xmedia_s32 ret;
    char c = 0;
    xmedia_u32 i = 0;
    xmedia_u32 chn = 0;
    xmedia_u32 dev;
    xmedia_s32 venc_chn;

    sample_vo_config_input vo_input = { 0 };
    sample_vo_all_mod_cfg all_mod_cfg;
    sample_vo_file_info *file_info;
    memset(&all_mod_cfg, 0, sizeof(sample_vo_all_mod_cfg));

    for (dev = 0; dev < VO_MAX_DEV_NUM; dev++) {
        for (chn = 0; chn < VO_MAX_CHN_NUM; chn++) {
            memset(&g_thread_info[dev][chn], 0 , sizeof(sample_vo_send_thread_info));
        }
    }
    all_mod_cfg.vo_dev_num = 1;
    all_mod_cfg.vo_chn_num[vo_virt_dev] = vo_chn_num;
    for (i = 0; i < VO_MAX_DEV_NUM; i++) {
        all_mod_cfg.vo_chn[vo_virt_dev][i] = i;
    }

    //venc
    all_mod_cfg.venc_chn_num = 1;
    all_mod_cfg.venc_chn[0] = 0;

    vo_input.square_sort = all_mod_cfg.vo_chn_num[vo_virt_dev];
    vo_input.screen_type = screen0_type;
    ret = sample_comm_vo_get_default_config(vo_input, &all_mod_cfg.vo_config[vo_virt_dev]);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vo_get_default_config failed !\n");
        return ret;
    }

    ret = sample_vo_sys_init(&all_mod_cfg);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_vo_sys_init failed !\n");
        return ret;
    }

    ret = sample_comm_vo_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vo init fail!\n");
        goto OUT1;
    }
    ret = sample_comm_venc_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc init fail!\n");
        goto OUT2;
    }

    all_mod_cfg.vo_config[vo_virt_dev].layer_config.part_mode = XMEDIA_VO_PARTITION_MODE_SINGLE;
    SAMPLE_PRT("all_mod_cfg.vo_chn_num[vo_virt_dev] %d !\n", all_mod_cfg.vo_chn_num[vo_virt_dev]);
    ret = sample_comm_vo_start(vo_virt_dev, vo_virt_dev, all_mod_cfg.vo_chn[vo_virt_dev],
        all_mod_cfg.vo_chn_num[vo_virt_dev],  XMEDIA_TRUE, &all_mod_cfg.vo_config[vo_virt_dev]);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vo start failed !\n");
        goto OUT3;
    }

    // venc config
    venc_chn = all_mod_cfg.venc_chn[0];
    all_mod_cfg.venc_out_size[0].width = all_mod_cfg.vo_config[vo_virt_dev].layer_config.img_size.width;
    all_mod_cfg.venc_out_size[0].height = all_mod_cfg.vo_config[vo_virt_dev].layer_config.img_size.height;
    app_liveserver_init();
    app_liveserver_start(all_mod_cfg.venc_out_size, all_mod_cfg.venc_chn_num,
        all_mod_cfg.vo_config[vo_virt_dev].dev_config.user_sync_config.frame_rate);
    memset(&all_mod_cfg.venc_config, venc_chn , sizeof(sample_venc_config));
    all_mod_cfg.venc_config.chn_info[venc_chn].venc_en = XMEDIA_TRUE;
    all_mod_cfg.venc_config.chn_info[venc_chn].venc_chn = venc_chn;
    all_mod_cfg.venc_config.chn_info[venc_chn].payload_type = PT_H265;
    all_mod_cfg.venc_config.chn_info[venc_chn].rc_mode = VENC_RC_MODE_H265CBR;
    sample_comm_venc_get_default_chn_info(all_mod_cfg.venc_out_size[0],
        all_mod_cfg.vo_config[vo_virt_dev].dev_config.user_sync_config.frame_rate,
        &all_mod_cfg.venc_config.chn_info[venc_chn]);
    ret = sample_comm_venc_start(&all_mod_cfg.venc_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start failed !\n");
        goto OUT4;
    }

    ret = sample_comm_sys_vo_bind_venc(vo_virt_dev, venc_chn);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_sys_vo_bind_venc failed !\n");
        goto OUT5;
    }

    file_info = sample_vo_match_src_file( all_mod_cfg.vo_config[vo_virt_dev].chn_attr[0].out_rect.width,
        all_mod_cfg.vo_config[vo_virt_dev].chn_attr[0].out_rect.height);
    if(file_info == XMEDIA_NULL) {
        printf("file_info : XMEDIA_NULL\n");
        goto OUT6;
    }
    #if 1
    ret = sample_comm_venc_start_get_stream(all_mod_cfg.venc_chn, all_mod_cfg.venc_chn_num, XMEDIA_TRUE, XMEDIA_FALSE, ".");
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start get stream failed !\n");
        goto OUT6;
    }
#endif
    for (i = 0; i < vo_chn_num; i++) {
        sample_vo_get_thread_default_config(&g_thread_info[vo_virt_dev][i]);
        g_thread_info[vo_virt_dev][i].dev = vo_virt_dev;
        g_thread_info[vo_virt_dev][i].chn_id = all_mod_cfg.vo_chn[vo_virt_dev][i];
        memcpy(g_thread_info[vo_virt_dev][i].filename, file_info->file_name, strlen(file_info->file_name) + 1);
        g_thread_info[vo_virt_dev][i].pix_format = all_mod_cfg.vo_config[vo_virt_dev].layer_config.pix_format;
        g_thread_info[vo_virt_dev][i].width = file_info->width;
        g_thread_info[vo_virt_dev][i].height = file_info->height;
        g_thread_info[vo_virt_dev][i].tid = i;
        sample_vo_start_send_thread(&g_thread_info[vo_virt_dev][i]);
    }

    while(1) {
        SAMPLE_PRT("\nSAMPLE_TEST:press 'e' to exit; !\n");
        c = getchar();
        if (c == 'e') {
            break;
        }
    }
    for (i = 0; i < vo_chn_num; i++) {
        sample_vo_stop_send_thread(&g_thread_info[vo_virt_dev][i]);
    }
    sample_comm_venc_stop_get_stream();
OUT6:
    sample_comm_sys_vo_unbind_venc(vo_virt_dev, venc_chn);
OUT5:
    sample_comm_venc_stop(&all_mod_cfg.venc_config);
OUT4:
    app_liveserver_stop();
    app_liveserver_deinit();
    sample_comm_vo_stop(vo_virt_dev, vo_virt_dev, all_mod_cfg.vo_chn[vo_virt_dev], all_mod_cfg.vo_chn_num[vo_virt_dev]);
OUT3:
    sample_comm_venc_exit();
OUT2:
    sample_comm_vo_exit();
OUT1:
    sample_comm_sys_exit();
    return ret;
}

static xmedia_s32 sample_vi_vpss_vo_test(xmedia_s32 vo_chn_num, sample_comm_screen_type screen0_type,
    sample_comm_sensor_type vi_sensor_type)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 blk_size = 0;
    xmedia_video_size pic_size = { 0 };
    xmedia_video_size sub_pic_size = { 0 };
    sample_comm_video_param video_param = { 0 };
    sample_sys_config sys_config = { 0 };

    xmedia_s32 vi_dev = 0;
    xmedia_s32 vi_pipe = 0;
    xmedia_s32 vi_chn = 0;
    sample_vi_config vi_config = { 0 };

    xmedia_s32 sensor_type = vi_sensor_type;
    xmedia_u32 framerate = 0;
    vi_sensor_info sensor_info = { 0 };
    sample_isp_param isp_param = { 0 };
    xmedia_isp_config isp_config = { 0 };
    xmedia_bool mirror = XMEDIA_FALSE;
    xmedia_bool flip = XMEDIA_FALSE;

    xmedia_s32 vpss_chnl = 0;
    xmedia_s32 vpss_pipe = 0;
    xmedia_s32 vpss_ichn = 0;
    xmedia_s32 vpss_ochn[2] = { 0, 1 };
    sample_vpss_config vpss_config = { 0 };
    xmedia_video_size vpss_ochn_size[VPSS_MAX_OCHN_NUM] = { 0 };

    xmedia_s32 vo_dev = 0;
    xmedia_s32 vo_layer = vo_dev;
    xmedia_s32 vo_chn[2] = { 0, 1 };
    sample_vo_config_input vo_input = { 0 };
    sample_vo_config vo_config = { 0 };

    vo_input.square_sort = vo_chn_num;
    vo_input.screen_type = screen0_type;

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);

    // sys init
    sys_config.sys_conf.pipe_mode[vi_pipe].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[vi_pipe].viproc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[vi_pipe].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;

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

    ret = sample_vivo_sys_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_vivo_sys_init failed !\n");
        return ret;
    }

    vi_config.dev_info[vi_dev].dev_en = XMEDIA_TRUE;
    vi_config.dev_info[vi_dev].dev_no = vi_dev;
    vi_config.dev_info[vi_dev].sensor_type = sensor_type;
    vi_config.pipe_info[vi_pipe].pipe_en = XMEDIA_TRUE;
    vi_config.pipe_info[vi_pipe].pipe_no = vi_pipe;
    vi_config.pipe_info[vi_pipe].chn_info[vi_chn].chn_en = XMEDIA_TRUE;
    vi_config.pipe_info[vi_pipe].chn_info[vi_chn].chn_no = vi_chn;
    vi_config.dev_bind_pipe[vi_dev].pipe[0] = vi_pipe;
    vi_config.dev_bind_pipe[vi_dev].pipe[1] = -1;

    isp_config.fps = framerate;
    isp_config.mode_config.work_mode = XMEDIA_ISP_WORK_MODE_MASTER;
    isp_config.mode_config.master_mode.blend_stat_enable = XMEDIA_FALSE;
    isp_config.mode_config.master_mode.slave_num = 0;
    isp_config.pixel_fmt = sensor_info.pixel_format;
    isp_config.size.height = sensor_info.height;
    isp_config.size.width = sensor_info.width;
    isp_config.wdr_mode = sensor_info.wdr_mode;

    // isp pipe init
    isp_param.pipe[vi_pipe] = vi_pipe;
    isp_param.isp_info[vi_pipe].isp_pipe_en = XMEDIA_TRUE;
    isp_param.isp_info[vi_pipe].isp_sensor_en = XMEDIA_TRUE;
    isp_param.isp_info[vi_pipe].sensor_type = sensor_type;
    isp_param.isp_info[vi_pipe].mirror = mirror;
    isp_param.isp_info[vi_pipe].flip = flip;

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

    // vpss config
    ret = sample_comm_vpss_get_default_pipe_cfg(&vpss_config.pipe_info[vpss_pipe].pipe_config, pic_size, &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get default pipe cfg failed !\n");
        goto exit2;
    }

    vpss_ochn_size[0].width = sensor_info.width;
    vpss_ochn_size[0].height = sensor_info.height;
    vpss_ochn_size[1].width = 640;
    vpss_ochn_size[1].height = 480;

    vpss_config.pipe_info[vpss_pipe].pipe_en = XMEDIA_TRUE;
    vpss_config.pipe_info[vpss_pipe].pipe_no = vpss_pipe;

    vpss_chnl = vpss_ochn[0];
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_no = vpss_ochn[0];
    ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
                                                vpss_ochn_size[vpss_chnl], &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get default pipe cfg failed !\n");
        goto exit2;
    }

    vpss_chnl = vpss_ochn[1];
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_no = vpss_ochn[1];
    ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
                                                vpss_ochn_size[vpss_chnl], &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get default pipe cfg failed !\n");
        goto exit2;
    }

    // vpss start
    ret = sample_comm_vpss_start(&vpss_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vpss start failed !\n");
        goto exit3;
    }

    ret = sample_comm_sys_vi_bind_vpss(vi_pipe, vi_chn, vpss_pipe, vpss_ichn);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vi bind vpss failed !\n");
        goto exit4;
    }

    // vo init
    ret = sample_comm_vo_get_default_config(vo_input, &vo_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get vo default config failed !\n");
        goto exit4;
    }

    ret = sample_comm_vo_start(vo_dev, vo_layer, vo_chn, vo_chn_num, XMEDIA_TRUE, &vo_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vo start failed !\n");
        goto exit5;
    }

    ret = sample_comm_sys_vpss_bind_vo(vpss_pipe, vpss_ochn[0], vo_layer, vo_chn[0]);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vpss bind vo failed !\n");
        goto exit6;
    }

    PAUSE(XMEDIA_FALSE);

exit6:
    sample_comm_sys_vpss_unbind_vo(vpss_pipe, vpss_ochn[0], vo_layer, vo_chn[0]);

exit5:
    sample_comm_vo_stop(vo_dev, vo_layer, vo_chn, vo_chn_num);

exit4:
    sample_comm_sys_vi_unbind_vpss(vi_pipe, vi_chn, vpss_pipe, vpss_ichn);

exit3:
    sample_comm_vpss_stop(&vpss_config);

exit2:
    sample_comm_isp_stop(&isp_param);

exit1:
    sample_comm_vi_stop(&vi_config);

exit0:
    sample_comm_isp_exit(&isp_param);
    sample_vivo_sys_exit();

    return ret;
}

typedef enum {
    SAMPLE_VO_SMART_SC485SL_MIPI_4M_30FPS_2LANE_12BIT,
    SAMPLE_VO_SENSOR_TYPE_MAX,
} sample_vo_sensor_type;

static sample_comm_sensor_type sample_vo_convert_sensor_type(sample_vo_sensor_type vo_sensor_type)
{
    switch (vo_sensor_type) {
        case SAMPLE_VO_SMART_SC485SL_MIPI_4M_30FPS_2LANE_12BIT:
            return SMART_SC485SL_MIPI_4M_30FPS_2LANE_12BIT;
        default:
            return SENSOR_TYPE_INVALID;
    }
}

int main(int argc, char *argv[])
{
    xmedia_s32 ret = XMEDIA_FAILURE;
    xmedia_s32 test_mode = 0;

    xmedia_s32 vo_chn_num = 0;
    sample_comm_screen_type screen0_type;
    xmedia_vo_partition_mode vo_part_mode;
    xmedia_vo_dev vo_virt_dev;
    sample_comm_sensor_type sensor_type;
    signal(SIGINT, sample_vo_handle_sig);
    signal(SIGTERM, sample_vo_handle_sig);

    if (argc >= 2) {
        test_mode = atoi(argv[1]);
    } else {
        test_mode = -1;
    }

    switch (test_mode) {
        case 0:
            if (argc != 5){
                SAMPLE_PRT("./sample_vo [test_mode] [vo_chn_num] [screen0_TYPE] [vo_part_mode]\n");
                SAMPLE_PRT("example VO SINGLE MODE: ./sample_vo 0 1 16 0\n");
                SAMPLE_PRT("example VO SINGLE MODE: ./sample_vo 0 4 16 0\n");
                SAMPLE_PRT("example VO MULTI MODE:  ./sample_vo 0 2 16 1\n");
                ret = XMEDIA_FAILURE;
                break;
            }
            vo_chn_num = atoi(argv[2]);
            screen0_type = atoi(argv[3]);
            vo_part_mode = atoi(argv[4]);
            ret = sample_vo_test(vo_chn_num,screen0_type,vo_part_mode);
            break;
        case 4:
            if (argc != 5){
                SAMPLE_PRT("./sample_vo [test_mode] [vo_virt_dev] [vo_chn_num] [screen0_TYPE]\n");
                SAMPLE_PRT("example VO SINGLE MODE: ./sample_vo 4 2 2 16\n");
                ret = XMEDIA_FAILURE;
                break;
            }
            vo_virt_dev = atoi(argv[2]);
            vo_chn_num = atoi(argv[3]);
            screen0_type = atoi(argv[4]);
            ret = sample_vo_virt_dev_test(vo_virt_dev, vo_chn_num, screen0_type);
            break;
        case 5:
            if (argc != 4){
                SAMPLE_PRT("./sample_vo [test_mode] [screen0_TYPE] [sensor_type]\n");
                SAMPLE_PRT("example VO SINGLE MODE: ./sample_vo 5 16 0\n");
                ret = XMEDIA_FAILURE;
                break;
            }
            vo_chn_num = 1;
            screen0_type = atoi(argv[2]);
            sensor_type = sample_vo_convert_sensor_type(atoi(argv[3]));
            if (sensor_type == SENSOR_TYPE_INVALID) {
                SAMPLE_PRT("sensor type not support!\n");
                ret = XMEDIA_FAILURE;
                break;
            }
            ret = sample_vi_vpss_vo_test(vo_chn_num,screen0_type, sensor_type);
            break;
        default:
            SAMPLE_PRT("test mode:         0:normal test    4:virtual_dev test  5:vi-vo test\n");
            SAMPLE_PRT("vo_chn_num:        1 or 2 or 4 (only single mode support 4 chn)\n");
            SAMPLE_PRT("screen0_TYPE:      0:PLCD800x480      8:SLCD_240X320_18BIT    12:KD026_320X240P10_4LINE_RGB565\n");
            SAMPLE_PRT("screen0_TYPE:      14:KD026_320X240P60_18BIT_18BPP_RGB666_8080I    16:SIL9024_BT1120_1280X720P60 \n");
            SAMPLE_PRT("screen0_TYPE:      17:SIL9024_BT656_1280X720P30_UYVY422\n");
            SAMPLE_PRT("vo_part_mode:      0:single_mode     1:multi_mode\n");
            SAMPLE_PRT("vo_virt_dev:       2\n");
            SAMPLE_PRT("sensor_type:       0: SMART_SC485SL_MIPI_4M_30FPS_2LANE_12BIT\n");

            SAMPLE_PRT("./sample_vo [test_mode] [vo_chn_num] [screen0_TYPE] [vo_part_mode]\n");
            SAMPLE_PRT("example VO SINGLE MODE:     ./sample_vo 0 1 16 0\n");
            SAMPLE_PRT("example VO SINGLE MODE:     ./sample_vo 0 4 16 0\n");
            SAMPLE_PRT("example VO MULTI MODE:      ./sample_vo 0 2 16 1\n\n");

            SAMPLE_PRT("./sample_vo [test_mode] [vo_virt_dev] [vo_chn_num] [screen0_TYPE]\n");
            SAMPLE_PRT("example:                    ./sample_vo 4 2 2 16\n");

            SAMPLE_PRT("./sample_vo [test_mode] [screen0_TYPE] [sensor_type]\n");
            SAMPLE_PRT("example:                    ./sample_vo 5 16 0\n");

            return 0;
    }

    if (XMEDIA_SUCCESS == ret) {
        SAMPLE_PRT("program exit normally!\n");
    } else {
        SAMPLE_PRT("program exit abnormally!\n");
    }

    return ret;
}
