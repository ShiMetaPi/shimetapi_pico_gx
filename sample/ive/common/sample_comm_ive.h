/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#ifndef __SAMPLE_COMM_IVE_H__
#define __SAMPLE_COMM_IVE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include "sample_comm.h"
#include "sample_comm_sys.h"
#include "sample_comm_vi.h"
#include "sample_comm_vpss.h"
#include "sample_comm_venc.h"
#include "sample_comm_isp.h"

#include "xmedia_debug.h"
#include "xmedia_ive_common.h"
#include "xmedia_ive.h"
#include "sample_comm.h"

#define VIDEO_WIDTH 352
#define VIDEO_HEIGHT 288
#define IVE_ALIGN 8
#define IVE_CHAR_CALW 8
#define IVE_CHAR_CALH 8
#define IVE_CHAR_NUM (IVE_CHAR_CALW *IVE_CHAR_CALH)
#define IVE_FILE_NAME_LEN 256
#define IVE_RECT_NUM   20
#define VPSS_CHN_NUM 2

#define SAMPLE_ALIGN_BACK(x, a)     ((a) * (((x) / (a))))

//malloc
xmedia_s32 sample_ive_sys_mmz_alloc(xmedia_u64 *pu64_phy_addr,
                                          xmedia_void **pp_vir_addr,
                                          const xmedia_char *str_mmb,
                                          const xmedia_char *str_zone,
                                          xmedia_u32 u32_len);

xmedia_s32 sample_ive_sys_mmz_alloc_cached(xmedia_u64 *pu64_phy_addr,
                                          xmedia_void **pp_vir_addr,
                                          const xmedia_char *str_mmb,
                                          const xmedia_char *str_zone,
                                          xmedia_u32 u32_len);
                                    
xmedia_s32 sample_ive_sys_mmz_free(xmedia_u64 u64_phy_addr, xmedia_void *p_vir_addr);

xmedia_s32 sample_ive_sys_mmz_flush_cache(xmedia_u64 u64_phy_addr,
                                        xmedia_void *p_vir_addr,
                                        xmedia_u32 u32_size);


typedef struct sample_ive_switch_s
{
   xmedia_bool b_venc;
   xmedia_bool b_vo;
}sample_ive_switch_s;

typedef struct sample_ive_rect_s
{
    xmedia_video_point ast_point[4];
} sample_ive_rect_s;

typedef struct sample_rect_array_s
{
    xmedia_u16 u16_num;
    sample_ive_rect_s ast_rect[IVE_RECT_NUM];
} sample_rect_array_s;

typedef struct ive_linear_data_s
{
    xmedia_s32 s32_linear_num;
    xmedia_s32 s32_thresh_num;
    xmedia_video_point* pst_linear_point;
} ive_linear_data_s;

typedef struct sample_ive_draw_rect_msg_s
{
    xmedia_video_frame_info st_frame_info;
    sample_rect_array_s st_region;
} sample_ive_draw_rect_msg_s;

//free mmz
#define IVE_MMZ_FREE(phy,vir)\
    do{\
        if ((0 != (phy)) && (0 != (vir)))\
        {\
            sample_ive_sys_mmz_free((phy),(xmedia_void *)(xmedia_ulong)(vir));\
            (phy) = 0;\
            (vir) = 0;\
        }\
    }while(0)

#define IVE_CLOSE_FILE(fp)\
    do{\
        if (NULL != (fp))\
        {\
            fclose((fp));\
            (fp) = NULL;\
        }\
    }while(0)

#define SAMPLE_PAUSE()\
    do {\
        printf("---------------press Enter key to exit!---------------\n");\
        (void)getchar();\
    } while (0)
#define SAMPLE_CHECK_EXPR(expr, fmt...)\
do\
{\
    if(expr)\
    {\
        SAMPLE_PRT(fmt);\
    }\
}while(0)
#define SAMPLE_CHECK_EXPR_RET(expr, ret, fmt...)\
do\
{\
    if(expr)\
    {\
        SAMPLE_PRT(fmt);\
        return (ret);\
    }\
}while(0)
#define SAMPLE_CHECK_EXPR_GOTO(expr, label, fmt...)\
do\
{\
    if(expr)\
    {\
        SAMPLE_PRT(fmt);\
        goto label;\
    }\
}while(0)

#define SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(Type,Addr) (Type*)(xmedia_ulong)(Addr)

/******************************************************************************
* function : Mpi init
******************************************************************************/
xmedia_void sample_comm_ive_check_ive_mpi_init(xmedia_void);
/******************************************************************************
* function : Mpi exit
******************************************************************************/
xmedia_s32 sample_comm_ive_ive_mpi_exit(xmedia_void);
/******************************************************************************
* function :Read file
******************************************************************************/
xmedia_s32 sample_comm_ive_read_file(xmedia_ive_image_s* pst_img, FILE* pfp);
/******************************************************************************
* function :Write file
******************************************************************************/
xmedia_s32 sample_comm_ive_write_file(xmedia_ive_image_s* pst_img, FILE* pfp);
/******************************************************************************
* function :Calc stride
******************************************************************************/
xmedia_u16 sample_comm_ive_calc_stride(xmedia_u32 u32width, xmedia_u8 u8align);
/******************************************************************************
* function : Copy blob to rect
******************************************************************************/
xmedia_void sample_comm_ive_blob_to_rect(xmedia_ive_ccblob_s *pst_blob, sample_rect_array_s *pst_rect,
                                            xmedia_u16 u16_rect_max_num,xmedia_u16 u16_area_thr_step,
                                            xmedia_u32 u32_src_width, xmedia_u32 u32_src_height,
                                            xmedia_u32 u32_dst_width,xmedia_u32 u32_dst_height);
/******************************************************************************
* function : Create ive image
******************************************************************************/
xmedia_s32 sample_comm_ive_create_image(xmedia_ive_image_s* pst_img, xmedia_ive_image_type_e en_type,
                                   xmedia_u32 u32width, xmedia_u32 u32height);
/******************************************************************************
* function : Create memory info
******************************************************************************/
xmedia_s32 sample_comm_ive_create_mem_info(xmedia_ive_mem_info_s* pst_mem_info, xmedia_u32 u32size);
/******************************************************************************
* function : Create ive image by cached
******************************************************************************/
xmedia_s32 sample_comm_ive_create_image_by_cached(xmedia_ive_image_s* pst_img,
        xmedia_ive_image_type_e en_type, xmedia_u32 u32width, xmedia_u32 u32height);
/******************************************************************************
* function : Create xmedia_ive_data_s
******************************************************************************/
xmedia_s32 sample_comm_ive_create_data(xmedia_ive_data_s* pst_data,xmedia_u32 u32width, xmedia_u32 u32height);
/******************************************************************************
* function : Init Vb
******************************************************************************/
xmedia_s32 sample_comm_ive_vb_init(pic_size_e *paenSize,xmedia_video_size *pastSize,xmedia_u32 u32VpssChnNum);
/******************************************************************************
* function : Dma frame info to  ive image
******************************************************************************/
xmedia_s32 sample_comm_ive_dma_image(xmedia_video_frame_info *pst_frame_info,xmedia_ive_dst_image_s *pst_dst,xmedia_bool b_instant);

/******************************************************************************
* function : Call vgs to fill rect
******************************************************************************/
xmedia_s32 sample_comm_vgs_fill_rect(xmedia_video_frame_info* pst_frm_info, sample_rect_array_s* pst_rect, xmedia_u32 u32_color);

/******************************************************************************
* function : Start Vpss
******************************************************************************/
xmedia_s32 sample_comm_ive_start_vpss(xmedia_video_size *pastSize,xmedia_u32 u32VpssChnNum);
/******************************************************************************
* function : Stop Vpss
******************************************************************************/
xmedia_void sample_comm_ive_stop_vpss(xmedia_u32 u32VpssChnNum);
/******************************************************************************
* function : Start Vo
******************************************************************************/
xmedia_s32 sample_comm_ive_start_vo(pic_size_e  enPicSize);
/******************************************************************************
* function : Stop Vo
******************************************************************************/
xmedia_void sample_comm_ive_stop_vo(pic_size_e  enPicSize);
/******************************************************************************
* function : Start Vi/Vpss/Venc/Vo
******************************************************************************/
xmedia_s32 sample_comm_ive_start_vi_vpss_venc_vo(sample_vi_config *pstViConfig,sample_ive_switch_s *pstSwitch,pic_size_e *penExtPicSize);
/******************************************************************************
* function : Stop Vi/Vpss/Venc/Vo
******************************************************************************/
xmedia_void sample_comm_ive_stop_vi_vpss_venc_vo(sample_vi_config *pstViConfig,sample_ive_switch_s *pstSwitch);

/*
 * function : Create memory info with cache
 */
xmedia_s32 sample_common_malloc_mem_cache(xmedia_u64 *phys_addr, void **virt_addr, xmedia_u32 size);

/*
 * function : free memory
 */
xmedia_s32 sample_common_free_mem(xmedia_u64 phys_addr, void *virt_addr);

#endif


