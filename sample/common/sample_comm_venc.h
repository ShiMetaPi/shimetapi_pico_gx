/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __SAMPLE_COMM_VENC_H__
#define __SAMPLE_COMM_VENC_H__

#include "xmedia_type.h"
#include "xmedia_sys.h"
#include "xmedia_vb.h"
#include "xmedia_venc.h"
#include "xmedia_vpss.h"
#include "xmedia_mmz.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VENC_FILE_PATH_SIZE        (128)
#define VENC_JPEG_SPLICING_MAX_VER_CNT (8)

typedef union
{
    /* Define the struct bits */
    struct
    {
        xmedia_s32    weight1               : 4; /* [0:3]  */
        xmedia_s32    weight2               : 4; /* [4:7]  */
    } bits;

    /* Define an unsigned member */
    xmedia_u8    u8;

} skip_wight_value;

typedef struct
{
    xmedia_bool  thread_start;
    xmedia_s32 vpss_pipe;
    xmedia_s32 vpss_chn[VPSS_MAX_PHY_OCHN_NUM];
    xmedia_s32 venc_chn[VENC_MAX_CHN_NUM];
    xmedia_s32   chn_cnt;
    xmedia_video_size   size[VENC_MAX_CHN_NUM];
} sample_venc_qpmap_sendfrm_para;

typedef struct sample_venc_getstream_para
{
    xmedia_bool        thread_start;
    xmedia_s32         venc_chn[VENC_MAX_CHN_NUM];
    xmedia_s32         cnt;
    xmedia_s32         save_stream;
    xmedia_bool        save_jpeg;
    xmedia_char        save_path[VENC_FILE_PATH_SIZE];
    xmedia_u32         stream_timeout_cnt;
} sample_venc_getstream_para;

typedef struct sample_venc_jpeg_slice
{
    xmedia_bool         slice_enable;
    xmedia_bool         by_frame;
} sample_venc_jpeg_slice;

typedef struct sample_venc_vui_info
{
    xmedia_bool vui_en;
    xmedia_u32  stream_display_fps;
} sample_venc_vui_info;

typedef struct sample_venc_chn_info
{
    xmedia_bool             venc_en;
    xmedia_s32              venc_chn;
    xmedia_payload_type     payload_type;
    xmedia_u32              width;
    xmedia_u32              height;
    xmedia_venc_rc_mode     rc_mode;
    xmedia_u32              profile;
    xmedia_venc_gop_attr    venc_gop_attr;
    xmedia_bool             use_default_gop_attr;
    xmedia_bool             rcn_ref_share_buf;
    xmedia_venc_attr_jpeg   jpeg_attr;
    xmedia_u32              src_frame_rate;
    xmedia_u32              dst_frame_rate;
    xmedia_bool             support_dcf;
    xmedia_u32              mpf_cnt;
    sample_venc_jpeg_slice  jpeg_slice;
    sample_venc_vui_info    vui_info;
    xmedia_u32              str_buffer_size;
}sample_venc_chn_info;

typedef enum {
    SAMPLE_RC_CBR = 0,
    SAMPLE_RC_VBR,
    SAMPLE_RC_AVBR,
    SAMPLE_RC_QPMAP,
    SAMPLE_RC_FIXQP,
} sample_rc;

typedef struct {
    sample_venc_chn_info chn_info[VENC_MAX_CHN_NUM];
}sample_venc_config;

typedef struct {
    xmedia_u32 src_width;
    xmedia_u32 src_height;
    xmedia_u32 dst_width;
    xmedia_u32 dst_height;
    xmedia_u32 enc_width;
    xmedia_u32 enc_height;
    xmedia_u32 row_cnt;
    xmedia_u32 col_cnt;

    xmedia_u32 vb_block;
    xmedia_u32 vb_pool;
    xmedia_u32 pool_id;
    xmedia_u64 phy_addr;
    xmedia_u8 *vir_addr;
    xmedia_u32 vb_size;
    xmedia_u32 tmp_data_start[VENC_JPEG_SPLICING_MAX_VER_CNT];
    xmedia_u32 tmp_data_offset[VENC_JPEG_SPLICING_MAX_VER_CNT];
    xmedia_u32 tmp_data_len[VENC_JPEG_SPLICING_MAX_VER_CNT];
    xmedia_u8 *tmp_data[VENC_JPEG_SPLICING_MAX_VER_CNT];
    xmedia_video_private_info priv_vb_info;
} sample_venc_splicing_info;

typedef struct
{
    xmedia_s32 vpss_pipe;
    xmedia_s32 vpss_chn;
    xmedia_s32 venc_chn;
    xmedia_char save_path[64];
    sample_venc_splicing_info splicing_info;
} sample_venc_splicing_para;

typedef struct {
    xmedia_u32                  pool;
    FILE                        *input_yuv[10];
    pthread_t                   user_send_yuv_thread;
    xmedia_video_pixel_format   user_pixel_format;
    xmedia_video_format         user_video_format;
    xmedia_video_compress_mode  user_compress_mode;

    xmedia_u32                  max_send_yuv_cnt;
    xmedia_u32                  user_width;
    xmedia_u32                  user_height;

    xmedia_u32                  yuv_num; // YUV source file number
    xmedia_char                 ch_yuv_path[128];
    xmedia_char                 ch_yuv_name[10][128];

    xmedia_u32                  h264e_cnt;
    xmedia_u32                  h264e_chnl[VENC_MAX_CHN_NUM];
    xmedia_u32                  h265e_cnt;
    xmedia_u32                  h265e_chnl[VENC_MAX_CHN_NUM];
    xmedia_u32                  mjpeg_cnt;
    xmedia_u32                  mjpeg_chnl[VENC_MAX_CHN_NUM];
    xmedia_u32                  jpege_cnt;
    xmedia_u32                  jpege_chnl[VENC_MAX_CHN_NUM];
    xmedia_u32                  user_send_yuv_delay_ms;
}sample_venc_yuv_thread_para;

typedef struct {
    xmedia_u32  block;
    xmedia_u32  pool;
    xmedia_u32  pool_id;
    xmedia_u64  phy_addr;
    xmedia_u8   *vir_addr;
    xmedia_u32  size;
    xmedia_s32  dev;
} sample_venc_mem_buf_info;

xmedia_s32 sample_comm_venc_get_file_post_fix(xmedia_payload_type payload, xmedia_char *file_post_fix);

xmedia_s32 sample_comm_venc_start_chn(xmedia_s32 venc_chn);

xmedia_s32 sample_comm_venc_stop_chn(xmedia_s32 venc_chn);

xmedia_s32 sample_comm_venc_get_default_gop_attr(xmedia_venc_gop_mode gop_mode, xmedia_venc_gop_attr *gop_attr);

xmedia_void sample_comm_venc_get_default_chn_info(xmedia_video_size res,xmedia_u32 framerate,sample_venc_chn_info *chn_info);

xmedia_s32 sample_comm_venc_set_rc_param(xmedia_s32 venc_chn);

xmedia_s32 sample_comm_venc_create_chn(sample_venc_chn_info *chn_param);

xmedia_s32 sample_comm_venc_destory_chn(xmedia_s32 venc_chn);

xmedia_s32 sample_comm_venc_start(sample_venc_config *venc_cfg);

xmedia_s32 sample_comm_venc_stop(sample_venc_config *venc_cfg);

xmedia_s32 sample_comm_venc_set_jpeg_exif(xmedia_s32 venc_chn);

xmedia_s32 sample_comm_venc_save_jpeg_stream(xmedia_s32 chn,FILE* fd, xmedia_venc_stream* pst_stream , xmedia_u32 recv_cnt, char *save_path);

xmedia_s32 sample_comm_venc_save_jpeg_thum(xmedia_char *result_path, xmedia_venc_stream* stream_pack, xmedia_u32 chn, xmedia_u32 recv_cnt);

xmedia_s32 sample_comm_venc_capture_jpeg(xmedia_s32 venc_chn, xmedia_s32 snap_cnt, xmedia_bool save_jpeg, xmedia_bool save_thumbnail, xmedia_char *save_path);

xmedia_s32 sample_comm_venc_start_get_stream(xmedia_s32 venc_chn[], xmedia_s32 cnt, xmedia_bool stream_no_save, xmedia_bool jpeg_no_save, xmedia_char *save_path);

xmedia_s32 sample_comm_venc_stop_get_stream(void);

xmedia_void *sample_comm_venc_user_send_thread(void *data);

xmedia_s32 sample_comm_venc_qpmap_sendfrm(xmedia_s32 pipe,xmedia_s32 vpss_chn[],xmedia_s32 venc_chn[],xmedia_s32 cnt, xmedia_video_size vpss_size[]);

xmedia_s32 sample_comm_venc_stop_send_qpmapfrm(xmedia_void);

xmedia_s32 sample_comm_venc_splicing_proc(sample_venc_splicing_para *splicing_para);

xmedia_s32 sample_comm_venc_set_roi(xmedia_s32 venc_chn[], xmedia_s32 chn_cnt);

xmedia_s32 sample_comm_venc_init(xmedia_void);

xmedia_s32 sample_comm_venc_exit(xmedia_void);

xmedia_s32 sample_comm_venc_set_fps_to_stream(sample_venc_chn_info *chn_param);

#ifdef __cplusplus
}
#endif

#endif

