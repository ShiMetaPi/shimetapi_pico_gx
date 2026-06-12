#ifndef __MEDIA_COMM_H__
#define __MEDIA_COMM_H__

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

#define MEDIA_PRINTF 1
#define MEDIA_ERROR  1

#define VI_MAX_BIND_PIPE   2
#define MAGIC_DATA_LEN 16


#define MEDIA_PRT(fmt...)   \
    do {\
       if(MEDIA_PRINTF == 1)\
       {\
            printf("[%s]-%d: ", __FUNCTION__, __LINE__);\
            printf(fmt);\
       }\
    }while(0)

#define MEDIA_ERR(fmt...)   \
    do {\
       if(MEDIA_ERROR == 1)\
       {\
           printf("[%s]-%d: ", __FUNCTION__, __LINE__);\
           printf(fmt);\
       }\
    }while(0)

#define SCENE_AUTO_BIN_PATH      "/usr/param/pq.bin"
#define FPN_BIN_PATH             "/usr/param/fpn.bin"
#define ISP_AINR_MODEL_PATH      "/usr/param/neuron_network.xmm"

typedef enum media_comm_sensor_num {
    MEDIA_SENSOR0 = 0,
    MEDIA_SENSOR1,
} media_comm_sensor_num;

typedef struct media_comm_sys_config {
    xmedia_sys_config sys_conf;
    xmedia_vb_config vb_conf;
} media_comm_sys_config;

typedef struct media_comm_isp_cfg {
    xmedia_s32 vi_dev;
    xmedia_s32 isp_pipe;
    media_comm_sensor_num sns_num;
    xmedia_bool en_quickstart;
} media_comm_isp_cfg;

typedef struct media_comm_vi_pipe_info {
     xmedia_vi_pipe_config vi_pipe_cfg;
     xmedia_vi_chn_config  vi_chn_cfg;
     xmedia_bool en_vi_cap_lowdelay;
     xmedia_vi_lowdelay_attr vi_cap_lowdelay;
     xmedia_bool en_vi_proc_lowdelay;
     xmedia_vi_lowdelay_attr vi_proc_lowdelay;
} media_comm_vi_pipe_info;

typedef struct media_comm_vi_pipe_param {
    xmedia_s32 vi_pipe_work_num;
    xmedia_s32 vi_pipe[VI_MAX_BIND_PIPE];
    media_comm_vi_pipe_info vi_pipe_info[VI_MAX_BIND_PIPE];
} media_comm_vi_pipe_param;

typedef struct media_comm_vi_grp_param {
    xmedia_s32 vi_grp_work_num;
    xmedia_s32 vi_grp[VI_MAX_GRP_NUM];
    xmedia_vi_grp_config vi_grp_cfg[VI_MAX_GRP_NUM];
} media_comm_vi_grp_param;

typedef struct media_comm_vi_cfg {
    xmedia_s32 vi_dev_work_num;
    xmedia_s32 vi_dev[VI_MAX_DEV_NUM];
    xmedia_bool en_quickstart[VI_MAX_DEV_NUM];
    xmedia_vi_dev_mipi_phy_config mipi_cfg[VI_MAX_DEV_NUM];
    xmedia_vi_dev_config vi_dev_cfg[VI_MAX_DEV_NUM];
    media_comm_vi_pipe_param vi_pipe_param[VI_MAX_DEV_NUM];
    media_comm_vi_grp_param vi_grp_param;
} media_comm_vi_cfg;

typedef struct media_comm_vpss_ochn_info {
    xmedia_s32 vpss_ochn_work_num;
    xmedia_s32 vpss_ochn[VPSS_MAX_PHY_OCHN_NUM];
    xmedia_vpss_ochn_config vpss_ochn_cfg[VPSS_MAX_PHY_OCHN_NUM];
    xmedia_vpss_low_delay_attr vpss_lowdelay_cfg[VPSS_MAX_PHY_OCHN_NUM];
    xmedia_video_wrap_attr vpss_wrap_cfg[VPSS_MAX_PHY_OCHN_NUM];
} media_comm_vpss_ochn_info;

typedef struct media_comm_vpss_cfg {
    xmedia_s32 vpss_pipe_work_num;
    xmedia_s32 vpss_pipe[VPSS_MAX_PIPE_NUM];
    xmedia_vpss_pipe_config pipe_cfg[VPSS_MAX_PIPE_NUM];
    media_comm_vpss_ochn_info vpss_ochn_info[VPSS_MAX_PIPE_NUM];
} media_comm_vpss_cfg;

typedef struct media_comm_venc_cfg {
    xmedia_u32 max_pic_width;
    xmedia_u32 max_pic_height;
    xmedia_u32 profile;
    xmedia_payload_type en_type;
    xmedia_venc_rc_mode rc_mode;
    xmedia_bool rcn_ref_share_buf;
    xmedia_u32 src_frame_rate;
    xmedia_u32 dst_frame_rate;
    xmedia_venc_gop_attr venc_gop_attr;
} media_comm_venc_cfg;

typedef struct media_comm_isp_ainr_cfg {
    xmedia_cl_context    cl_ctx;
    xmedia_u32           cl_device_num;
    xmedia_cl_device_id *cl_device;
    xmedia_u64           work_space_phyaddr;
    xmedia_void         *work_space_viraddr;
} media_comm_isp_ainr_cfg;

typedef struct media_comm_isp_sensor_info {
    xmedia_video_size sns_max_size;
    xmedia_float      frame_rate;
} media_comm_isp_sensor_info;

typedef struct vi_info {
    xmedia_s32 enable;
    xmedia_s32 pipe;
} bin_vi_info;

typedef struct module_info {
    bin_vi_info vi;
} bin_module_info;

static const xmedia_u8 magic_isp[MAGIC_DATA_LEN] = "ISP_BIN";

xmedia_s32 media_comm_sys_init(media_comm_sys_config* sys_cfg);
xmedia_s32 media_comm_sys_exit();
xmedia_s32 media_comm_media_init();
xmedia_s32 media_comm_media_exit();
xmedia_s32 media_comm_mem_open(xmedia_void);
xmedia_void media_comm_mem_close(xmedia_void);
xmedia_void media_comm_set_reg(xmedia_u32 addr,xmedia_u32 value);
xmedia_s32 media_comm_get_reg(xmedia_u32 u32Addr);
xmedia_u64 media_comm_get_cur_time();
xmedia_s32 media_comm_vi_bind_vpss(xmedia_s32 vi_pipe, xmedia_s32 vi_chn, xmedia_s32 vpss_pipe, xmedia_s32 vpss_ichn);
xmedia_s32 media_comm_vi_unbind_vpss(xmedia_s32 vi_pipe, xmedia_s32 vi_chn, xmedia_s32 vpss_pipe, xmedia_s32 vpss_ichn);
xmedia_s32 media_comm_sys_vpss_bind_venc(xmedia_s32 vpss_pipe, xmedia_s32 vpss_ochn, xmedia_s32 venc_chn);
xmedia_s32 media_comm_sys_vpss_unbind_venc(xmedia_s32 vpss_pipe, xmedia_s32 vpss_ochn, xmedia_s32 venc_chn);
xmedia_s32 media_comm_isp_get_sensor_info(media_comm_sensor_num sns_num, media_comm_isp_sensor_info *sns_info);
xmedia_s32 media_comm_isp_set_sensor_info(media_comm_sensor_num sns_num, media_comm_isp_sensor_info *sns_info);
xmedia_s32 media_comm_isp_init(media_comm_isp_cfg media_isp_cfg);
xmedia_s32 media_comm_isp_exit(xmedia_s32 pipe, media_comm_sensor_num sns_num);
xmedia_s32 media_comm_isp_start(xmedia_s32 pipe, xmedia_bool en_start);
xmedia_s32 media_comm_isp_stop(xmedia_s32 pipe, xmedia_bool en_start);
xmedia_s32 media_comm_isp_get_mipi_rate(xmedia_s32 isp_pipe);
xmedia_s32 media_comm_vi_start(media_comm_vi_cfg* vi_cfg);
xmedia_s32 media_comm_vi_stop(media_comm_vi_cfg* vi_cfg);
xmedia_s32 media_comm_vpss_start(media_comm_vpss_cfg* vpss_cfg);
xmedia_s32 media_comm_vpss_stop(media_comm_vpss_cfg* vpss_cfg);
xmedia_s32 media_comm_venc_start(xmedia_s32 venc_chn, media_comm_venc_cfg* venc_cfg);
xmedia_s32 media_comm_venc_stop(xmedia_s32 venc_chn);
xmedia_s32 media_comm_venc_get_max_chn_size(xmedia_s32 venc_chn);

xmedia_s32 media_comm_isp_ainr_init(const xmedia_char* ainr_model_name);
xmedia_s32 media_comm_isp_ainr_exit();
xmedia_s32 media_comm_isp_ainr_load_model(xmedia_s32 isp_pipe, xmedia_s32 model_num, xmedia_char** ainr_model_name);
xmedia_s32 media_comm_isp_ainr_unload_model(xmedia_s32 isp_pipe);
xmedia_s32 media_comm_isp_ainr_enable(xmedia_s32 isp_pipe);
xmedia_s32 media_comm_scene_auto_enable(bin_module_info *module);
xmedia_s32 media_comm_fpn_enable(xmedia_s32 isp_pipe, xmedia_s32 width, xmedia_s32 height);


#endif /*__MEDIA_COMM_H__*/
