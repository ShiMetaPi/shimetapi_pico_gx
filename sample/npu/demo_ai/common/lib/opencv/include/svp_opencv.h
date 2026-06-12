#ifndef __SVP_OPENCV__
#define __SVP_OPENCV__

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "common.h"
#include "xmedia_svp.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct  {
    xmedia_double roll;
    xmedia_double pitch;
    xmedia_double yaw;
} svp_euler_angles;

xmedia_s32 svp_opencv_face_orientation(const xmedia_svp_keypoint *landmarks, svp_euler_angles *angles);

#ifdef __cplusplus
}
#endif

#endif
