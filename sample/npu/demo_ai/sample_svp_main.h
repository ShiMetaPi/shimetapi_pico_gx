#ifndef __DEMO_NPU_H
#define __DEMO_NPU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <math.h>
#include <dirent.h>

#include "sample_comm.h"
#include "sample_comm_sys.h"
#include "sample_comm_vi.h"
#include "sample_comm_vpss.h"
#include "sample_comm_venc.h"
#include "sample_comm_isp.h"
#include "sample_comm_livestream.h"

#include "xmedia_svp.h"
#include "xmedia_mmz.h"
#include "xmedia_vgs.h"
#include "xmedia_tde.h"

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/mem.h"
#include "libswscale/swscale.h"
#include "libavutil/file.h"
#include "libavutil/imgutils.h"
#include "libavutil/pixfmt.h"
#include "xmedia_ive_common.h"
#include "xmedia_ive.h"


#include "common.h"
#include "data_base.h"
#include "common/cJSON.h"

#include "canvas.h"
#include "canvas_font.h"

#ifdef DMS_OPENCV
#include "svp_opencv.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define SCORE_LEN   (128)
#define CONVERT_TO_32BIT_ADDR(Addr) (void*)(xmedia_ulong)(Addr)
#define CONVERT_TO_64BIT_ADDR(Addr) (xmedia_u64)(xmedia_ulong)(Addr)
#define MAX_FILES_NUM 8192
#define MAX_STR_NAME_LEN 256
#define GESTURE_KEYPOINT_LEN 2
typedef xmedia_s32 VGS_HANDLE;

typedef struct {
    xmedia_bool distract;       // 分心状态
    xmedia_u8 distract_frame;   // 分心帧数
    xmedia_s32 tracker_id;      // 追踪id
    xmedia_u8 warn_frame;       // 警告帧数
    xmedia_float pitch;         // 俯仰角
    xmedia_float yaw;           // 偏航角
    xmedia_float roll;          // 翻滚角
} sample_dms_state;

typedef struct{
    xmedia_u32 width;
    xmedia_u32 height;
} rect_size;

typedef enum PIXEL_FORMAT_E
{
    PIXEL_FORMAT_RGB_444 = 0,
    PIXEL_FORMAT_RGB_555,
    PIXEL_FORMAT_RGB_565,
    PIXEL_FORMAT_RGB_888,

    PIXEL_FORMAT_BGR_444,
    PIXEL_FORMAT_BGR_555,
    PIXEL_FORMAT_BGR_565,
    PIXEL_FORMAT_BGR_888,

    PIXEL_FORMAT_ARGB_1555,
    PIXEL_FORMAT_ARGB_4444,
    PIXEL_FORMAT_ARGB_8565,
    PIXEL_FORMAT_ARGB_8888,
    PIXEL_FORMAT_ARGB_2BPP,

    PIXEL_FORMAT_ABGR_1555,
    PIXEL_FORMAT_ABGR_4444,
    PIXEL_FORMAT_ABGR_8565,
    PIXEL_FORMAT_ABGR_8888,

    PIXEL_FORMAT_RGB_BAYER_8BPP,
    PIXEL_FORMAT_RGB_BAYER_10BPP,
    PIXEL_FORMAT_RGB_BAYER_12BPP,
    PIXEL_FORMAT_RGB_BAYER_14BPP,
    PIXEL_FORMAT_RGB_BAYER_16BPP,


    PIXEL_FORMAT_YVU_PLANAR_422,
    PIXEL_FORMAT_YVU_PLANAR_420,
    PIXEL_FORMAT_YVU_PLANAR_444,

    PIXEL_FORMAT_YVU_SEMIPLANAR_422,
    PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    PIXEL_FORMAT_YVU_SEMIPLANAR_444,

    PIXEL_FORMAT_YUV_SEMIPLANAR_422,
    PIXEL_FORMAT_YUV_SEMIPLANAR_420,
    PIXEL_FORMAT_YUV_SEMIPLANAR_444,

    PIXEL_FORMAT_YUYV_PACKAGE_422,
    PIXEL_FORMAT_YVYU_PACKAGE_422,
    PIXEL_FORMAT_UYVY_PACKAGE_422,
    PIXEL_FORMAT_VYUY_PACKAGE_422,
    PIXEL_FORMAT_YYUV_PACKAGE_422,
    PIXEL_FORMAT_YYVU_PACKAGE_422,
    PIXEL_FORMAT_UVYY_PACKAGE_422,
    PIXEL_FORMAT_VUYY_PACKAGE_422,
    PIXEL_FORMAT_VY1UY0_PACKAGE_422,

    PIXEL_FORMAT_YUV_400,
    PIXEL_FORMAT_UV_420,


    PIXEL_FORMAT_BGR_888_PLANAR,
    PIXEL_FORMAT_HSV_888_PACKAGE,
    PIXEL_FORMAT_HSV_888_PLANAR,
    PIXEL_FORMAT_LAB_888_PACKAGE,
    PIXEL_FORMAT_LAB_888_PLANAR,
    PIXEL_FORMAT_S8C1,
    PIXEL_FORMAT_S8C2_PACKAGE,
    PIXEL_FORMAT_S8C2_PLANAR,
    PIXEL_FORMAT_S8C3_PLANAR,
    PIXEL_FORMAT_S16C1,
    PIXEL_FORMAT_U8C1,
    PIXEL_FORMAT_U16C1,
    PIXEL_FORMAT_S32C1,
    PIXEL_FORMAT_U32C1,
    PIXEL_FORMAT_U64C1,
    PIXEL_FORMAT_S64C1,

    PIXEL_FORMAT_BUTT
} PIXEL_FORMAT_E;

typedef struct BITMAP_S
{
    PIXEL_FORMAT_E enPixelFormat;
    xmedia_u32 u32Width;
    xmedia_u32 u32Height;
    xmedia_void* ATTRIBUTE pData;
} BITMAP_S;



typedef enum {
    SAMPLE_SVP_ALG_TYPE_PERSON = 0,                   // 人形检测
    SAMPLE_SVP_ALG_TYPE_PERSON_KEYPOINT,              // 人体关键点检测
    SAMPLE_SVP_ALG_TYPE_FACE,                         // 人脸检测
    SAMPLE_SVP_ALG_TYPE_PET,                          // 宠物检测
    SAMPLE_SVP_ALG_TYPE_CAR,                          // 车形检测
    SAMPLE_SVP_ALG_TYPE_NON_MOTORIZED_VEHICLE,        // 非机动车检测
    SAMPLE_SVP_ALG_TYPE_HEAD,                         // 人头检测
    SAMPLE_SVP_ALG_TYPE_FIREWORKS,                    // 烟火检测
    SAMPLE_SVP_ALG_TYPE_PACKAGE,                      // 包裹检测
    SAMPLE_SVP_ALG_TYPE_AOV_PERSON,                   // AOV人形检测
    SAMPLE_SVP_ALG_TYPE_ADAS,                         // 高级驾驶辅助系统
    SAMPLE_SVP_ALG_TYPE_FIRESMOKE,                    // 烟火检测（YOLOV8）
    SAMPLE_SVP_ALG_TYPE_GESTURE_RECOGNITION,          // 手势识别
    SAMPLE_SVP_ALG_TYPE_FACE_EMOTION,                 // 表情分类
    SAMPLE_SVP_ALG_TYPE_FACE_RECOGNITION,             // 人脸识别
    SAMPLE_SVP_ALG_TYPE_PERSON_RCNN,                  // 人形检测二阶段
    SAMPLE_SVP_ALG_TYPE_CAR_RCNN,                     // 车形检测二阶段
    SAMPLE_SVP_ALG_TYPE_PET_RCNN,                     // 宠物检测二阶段
    SAMPLE_SVP_ALG_TYPE_COMPANION_ROBOT,              // 陪伴机器人
    SAMPLE_SVP_ALG_TYPE_PERSON_RCNN_720P,             // 人形检测720P
    SAMPLE_SVP_ALG_TYPE_CAR_RCNN_720P,                // 车形检测720P
    SAMPLE_SVP_ALG_TYPE_PET_RCNN_720P,                // 宠物检测720P
    SAMPLE_SVP_ALG_TYPE_MULTI_DETECT,                 // 多类型检测
    SAMPLE_SVP_ALG_TYPE_PERSON_CAR,                   // 人车检测
    SAMPLE_SVP_ALG_TYPE_DMS,                          // DMS检测
    SAMPLE_SVP_ALG_TYPE_PLATE_RECOGNITION,            // 车牌识别
    SAMPLE_SVP_ALG_TYPE_VEHICLE_RECOGNITION,          // 车辆识别
    SAMPLE_SVP_ALG_TYPE_PLATE_VEHICLE_RECO,           // 车牌车辆识别
    // SAMPLE_SVP_ALG_TYPE_FACE_AGE,                     // 年龄分类
    // SAMPLE_SVP_ALG_TYPE_FACE_MASK,                    // 口罩分类
    // SAMPLE_SVP_ALG_TYPE_FACE_GLASSES,                 // 眼镜分类
    // SAMPLE_SVP_ALG_TYPE_FACE_GENDER,                  // 性别分类
    // SAMPLE_SVP_ALG_TYPE_FACE_ATTRIBUTE,               // 人脸属性
    // SAMPLE_SVP_ALG_TYPE_PASSENGER_FLOW_STATISTICS,    // 客流量统计
    // SAMPLE_SVP_ALG_TYPE_OFF_DUTY,                     // 离岗检测
    // SAMPLE_SVP_ALG_TYPE_BOUNDARY,                     // 周界检测
    SAMPLE_SVP_ALG_TYPE_MAX,
} sample_svp_alg_type;


typedef struct {
    sample_svp_alg_type detect_type;
    xmedia_s32 vpss_ochn[3];
    xmedia_s32 venc_chn[3];
    xmedia_s32 vpss_pipe;
    xmedia_s32 venc_chnl;
    xmedia_s32 svp_handle;
    xmedia_bool big_stream;

    // 数据库
    db_attr *g_db_handle;
    xmedia_bool add_flag;
    xmedia_char delete_name[VENC_MAX_NAME_LEN];
    xmedia_char add_name[VENC_MAX_NAME_LEN];
    xmedia_bool delete_flag;
} sample_svp_info;



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // __DEMO_NPU_H
