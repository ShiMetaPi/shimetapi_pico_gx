#ifndef __SAMPLE_COMM_AOV_H__
#define __SAMPLE_COMM_AOV_H__
#include <stdio.h>

#include "xmedia_type.h"
#include "xmedia_video_common.h"
#include "xmedia_sys.h"
#include "xmedia_vb.h"
#include "xmedia_vi.h"
#include "xmedia_isp.h"
#include "xmedia_ae.h"
#include "xmedia_awb.h"
#include "xmedia_venc.h"
#include "xmedia_vpss.h"
#include "xmedia_vgs.h"
#include "xmedia_svp.h"
#include "xmedia_tde.h"
#include "xmedia_pm.h"
#include "xmedia_cl.h"
#include "xmedia_mmz.h"
#include "xmedia_ive_common.h"
#include "xmedia_ive.h"
#include "xmedia_md.h"
#include "drv_misc.h"

#define NPU_CHN_WIDTH       640
#define NPU_CHN_HEIGHT      360
#define NPU_NORMAL_STATE_CONTINUE_COUNT  300
#define VENC_STR_BUF_DEFAULT_MAX_FRAME_COUNT     600
#define RTC_SAFE_THRESOHLD_MS   50
#define AOV_IVE_MD_IMAGE_NUM    2
#define IVE_DETECT_SUPPORT      1
//#define APP_STANDBY_SENSOR 1
//#define AOV_APP_TIMESTAMP 1

#define AOV_SUPPORT_MAX_SENSOR_NUM      2

typedef struct aov_ive_param {
    xmedia_s32 md_chn;
    xmedia_ive_src_image_s astImg[AOV_IVE_MD_IMAGE_NUM];
    xmedia_s32 cur_idx;
    xmedia_ive_dst_mem_info_s stBlob;
    xmedia_md_attr_s stMdAttr;
    xmedia_bool is_first_frm;
} aov_ive_param;

typedef enum sample_aov_work_mode {
    MEDIA_WORK_AOV = 0,
    MEDIA_WORK_NORMAL,
} sample_aov_work_mode;

#ifdef APP_STANDBY_SENSOR
typedef enum sensor_status_ {
    SENSOR_STATUS_NONE = 0,
    SENSOR_STATUS_START,
    SENSOR_STATUS_STANDBY,
    SENSOR_STATUS_RESUME,
} sensor_status_t;
#endif

typedef struct sample_aov_venc_chn_frame {
    xmedia_s32 venc_chn_num;
    xmedia_s32 venc_chn[VENC_MAX_CHN_NUM];
    xmedia_u32 frame_num[VENC_MAX_CHN_NUM];
} sample_aov_venc_chn_frame;

typedef struct sample_aov_isp_param {
    xmedia_s32 isp_pipe_num;
    xmedia_s32 isp_pipe[VI_MAX_DEV_NUM];
} sample_aov_isp_param;

typedef struct sample_aov_venc_param {
    xmedia_s32  venc_chn_num;
    xmedia_bool en_aov[VENC_MAX_CHN_NUM];
    xmedia_s32  venc_chn[VENC_MAX_CHN_NUM];
    xmedia_s32  chn_max_size[VENC_MAX_CHN_NUM];
    xmedia_s32  chn_threshold_size[VENC_MAX_CHN_NUM];
} sample_aov_venc_param;

typedef struct sample_aov_vpss_normal_only_param {
    xmedia_s32 vpss_pipe_num;
    xmedia_s32 vpss_pipe[VPSS_MAX_PIPE_NUM];
    xmedia_s32 vpss_ochn_num[VPSS_MAX_PIPE_NUM];
    xmedia_s32 vpss_ochn[VPSS_MAX_PIPE_NUM][VPSS_MAX_OCHN_NUM];
} sample_aov_vpss_normal_only_param;

typedef struct sample_aov_vpss_npu_param {
    xmedia_s32 npu_detect_num;
    xmedia_s32 npu_svp_handle[VPSS_MAX_PIPE_NUM];
    xmedia_bool en_switch_mode[VPSS_MAX_PIPE_NUM];
    xmedia_s32 vpss_pipe[VPSS_MAX_PIPE_NUM];
    xmedia_s32 vpss_ochn[VPSS_MAX_PIPE_NUM];
} sample_aov_vpss_npu_param;

typedef xmedia_void (*sample_aov_normal_npu_notice)(sample_aov_work_mode);
typedef xmedia_void (*sample_aov_normal_resume)(xmedia_void);
typedef xmedia_void (*sample_aov_normal_suspend)(xmedia_void);

typedef struct sample_aov_init_param {
    xmedia_s32 suspend_time;
    sample_aov_normal_npu_notice npu_notice_callback;
    sample_aov_normal_resume normal_resume_callback;
    sample_aov_normal_suspend normal_suspend_callback;
    sample_aov_isp_param isp_param;//sensor standby or resume used.
    sample_aov_vpss_normal_only_param vpss_param;//aov mode:vpss ochn should disable.
    sample_aov_vpss_npu_param npu_param;//npu detect. svp handle vpss pipe & ochn.
    sample_aov_venc_param venc_param;// venc chn param.
#ifdef APP_STANDBY_SENSOR
    sensor_status_t sensor_status[AOV_SUPPORT_MAX_SENSOR_NUM];
#endif
} sample_aov_init_param;

typedef struct sample_aov_detect_ {
#ifdef IVE_DETECT_SUPPORT
    aov_ive_param ive_param[AOV_SUPPORT_MAX_SENSOR_NUM];
#endif
    xmedia_svp_aov_detect_output svp_result[AOV_SUPPORT_MAX_SENSOR_NUM];
    xmedia_s32 svp_detect_none_cnt[AOV_SUPPORT_MAX_SENSOR_NUM];
    xmedia_s32 detect_complete_flag[AOV_SUPPORT_MAX_SENSOR_NUM];
}sample_aov_detect_t;

xmedia_void sample_comm_aov_init(sample_aov_init_param *aov_init_param);
xmedia_void sample_comm_aov_exit(xmedia_void);
xmedia_void sample_comm_aov_set_work_mode(sample_aov_work_mode work_mode);
xmedia_void sample_comm_aov_get_work_mode(sample_aov_work_mode *work_mode);
xmedia_s32 sample_comm_aov_npu_init(xmedia_void);
xmedia_s32 sample_comm_aov_npu_create(xmedia_s32 *handle, xmedia_svp_task_cfg *task_cfg);
xmedia_s32 sample_comm_aov_npu_destroy(xmedia_s32 handle);
xmedia_void sample_comm_aov_npu_exit(xmedia_void);
xmedia_s32  sample_comm_aov_venc_thread_create(xmedia_void);
xmedia_s32  sample_comm_aov_venc_thread_destroy(xmedia_void);
xmedia_s32  sample_comm_aov_work_thread_create(xmedia_void);
xmedia_s32  sample_comm_aov_work_thread_destroy(xmedia_void);

xmedia_s32 sample_comm_aov_misc_init(misc_aov_isp_info *isp_info);
xmedia_s32 sample_comm_aov_get_misc_fd(xmedia_void);


#endif   //__SAMPLE_COMM_AOV_H__
