#ifndef __MEDIA_SAMPLE_H__
#define __MEDIA_SAMPLE_H__

#include <stdio.h>

#include "xmedia_type.h"
#include "drv_vi.h"
#include "drv_isp.h"
#include "xmedia_ae.h"
#include "xmedia_awb.h"
#include "i2c_dev.h"
#include "i2c.h"
#include "compile.h"

#define MEDIA_ERROR 1
#define MEDIA_PRINTF 1

#define MEDIA_ERR(fmt...)   \
    do {\
       if(MEDIA_ERROR == 1)\
       {\
           puts(fmt);\
       }\
    }while(0)

#define MEDIA_PRT(fmt...)   \
    do {\
        if(MEDIA_PRINTF == 1)\
        {\
            puts(fmt);\
        }\
    }while(0)

#define MEDIA_STAGE1_ERR(fmt...)   \
    do {\
       if(MEDIA_ERROR == 1)\
       {\
           puts(STAGE1_STR(fmt));\
       }\
    }while(0)


//#define SENSOR_SC485SL_2LANE_SUPPORT 1
//#define SENSOR_SC485SL_4LANE_SUPPORT
#define SENSOR_SC485SL_4LANE_60FPS_SUPPORT
//#define SENSOR_SC235HAI_2LANE_60FPS_SUPPORT

#ifdef CONFIG_MEDIA_SAMPLE_QUICKSTART
xmedia_s32 quickstart_media_preinit(void);
xmedia_s32 quickstart_media_start(void);
#endif

#endif /*__MEDIA_SAMPLE_H__*/
