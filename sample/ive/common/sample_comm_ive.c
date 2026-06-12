/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
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

#include "common.h"
#include "xmedia_vb.h"
#include "xmedia_sys.h"
#include "xmedia_vi.h"
#include "xmedia_vo.h"
#include "xmedia_vgs.h"

#include "sample_comm_ive.h"

static xmedia_bool g_b_mpi_init = XMEDIA_FALSE;

xmedia_s32 sample_ive_sys_mmz_alloc(xmedia_u64 *pu64_phy_addr,
                                          xmedia_void **pp_vir_addr,
                                          const xmedia_char *str_mmb,
                                          const xmedia_char *str_zone,
                                          xmedia_u32 u32_len)
{
    *pu64_phy_addr = xmedia_mmz_alloc(str_zone, str_mmb, u32_len);
    if (*pu64_phy_addr == 0) {
        return XMEDIA_FAILURE;
    }

    *pp_vir_addr = xmedia_mmz_map(*pu64_phy_addr, u32_len, 0);
    if (*pp_vir_addr == NULL) {
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_ive_sys_mmz_alloc_cached(xmedia_u64 *pu64_phy_addr,
                                          xmedia_void **pp_vir_addr,
                                          const xmedia_char *str_mmb,
                                          const xmedia_char *str_zone,
                                          xmedia_u32 u32_len)
{
    *pu64_phy_addr = xmedia_mmz_alloc(str_zone, str_mmb, u32_len);
    if (*pu64_phy_addr == 0) {
        return XMEDIA_FAILURE;
    }

    *pp_vir_addr = xmedia_mmz_map(*pu64_phy_addr, u32_len, 1);
    if (*pp_vir_addr == NULL) {
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}


xmedia_s32 sample_ive_sys_mmz_free(xmedia_u64 u64_phy_addr, xmedia_void *p_vir_addr)
{
    xmedia_s32 ret;

    ret = xmedia_mmz_unmap(p_vir_addr);
    if (ret != XMEDIA_SUCCESS) {
        return -1;
    }

    return xmedia_mmz_free(u64_phy_addr);
}


xmedia_s32 sample_ive_sys_mmz_flush_cache(xmedia_u64 u64_phy_addr,
                                        xmedia_void *p_vir_addr,
                                        xmedia_u32 u32_size)
{
    return xmedia_mmz_flush_cache(u64_phy_addr, p_vir_addr, u32_size);
}


xmedia_u16 sample_comm_ive_calc_stride(xmedia_u32 u32width, xmedia_u8 u8align)
{
    return (u32width + (u8align - u32width % u8align) % u8align);
}

static xmedia_s32 sample_ive_init(xmedia_void)
{
    xmedia_s32 s32_ret;

    xmedia_sys_exit();

    sample_sys_config sys_conifg = { 0 };
    s32_ret = xmedia_sys_init(&sys_conifg.sys_conf);
    if (XMEDIA_SUCCESS != s32_ret)
    {
        SAMPLE_PRT("xmedia_sys_init fail,Error(%#x)\n", s32_ret);
        return s32_ret;
    }

    return XMEDIA_SUCCESS;
}


xmedia_void sample_comm_ive_check_ive_mpi_init(xmedia_void)
{
    if (XMEDIA_FALSE == g_b_mpi_init)
    {
        if (sample_ive_init())
        {
            SAMPLE_PRT("Ive mpi init failed!\n");
            exit(-1);
        }
        g_b_mpi_init = XMEDIA_TRUE;
    }
}
xmedia_s32 sample_comm_ive_ive_mpi_exit(xmedia_void)
{
    g_b_mpi_init = XMEDIA_FALSE;
    if (xmedia_sys_exit())
    {
        SAMPLE_PRT("Sys exit failed!\n");
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_vgs_fill_rect(xmedia_video_frame_info* pst_frm_info, sample_rect_array_s* pst_rect, xmedia_u32 u32_color)
{
    xmedia_s32 vgs_handle = -1;
    xmedia_s32 s32_ret = XMEDIA_SUCCESS;
    xmedia_u16 i;
    xmedia_vgs_frame_info st_vgs_task;
    xmedia_vgs_cover_attr st_vgs_add_cover;

    if (0 == pst_rect->u16_num)
    {
        return s32_ret;
    }
    s32_ret = xmedia_vgs_create_job(&vgs_handle);
    if (s32_ret != XMEDIA_SUCCESS)
    {
        SAMPLE_PRT("xmedia_vgs_create_job fail,Error(%#x)\n", s32_ret);
        return s32_ret;
    }

    memcpy(&st_vgs_task.img_in, pst_frm_info, sizeof(xmedia_video_frame_info));
    memcpy(&st_vgs_task.img_out, pst_frm_info, sizeof(xmedia_video_frame_info));

    st_vgs_add_cover.cover_type = XMEDIA_VGS_COVER_TYPE_QUAD_RANGLE;
    st_vgs_add_cover.color = u32_color;
    for (i = 0; i < pst_rect->u16_num; i++)
    {
        st_vgs_add_cover.quadrangle.is_solid = XMEDIA_FALSE;
        st_vgs_add_cover.quadrangle.thick = 2;
        memcpy(st_vgs_add_cover.quadrangle.points, pst_rect->ast_rect[i].ast_point, sizeof(pst_rect->ast_rect[i].ast_point));
        s32_ret = xmedia_vgs_add_task_cover(vgs_handle, &st_vgs_task, &st_vgs_add_cover,1);
        if (s32_ret != XMEDIA_SUCCESS)
        {
            SAMPLE_PRT("xmedia_vgs_add_task_cover fail,Error(%#x)\n", s32_ret);
            xmedia_vgs_cancel_job(vgs_handle);
            return s32_ret;
        }
    }

    s32_ret = xmedia_vgs_submit_job(vgs_handle);
    if (s32_ret != XMEDIA_SUCCESS)
    {
        SAMPLE_PRT("XMEDIA_API_VGS_EndJob fail,Error(%#x)\n", s32_ret);
        xmedia_vgs_cancel_job(vgs_handle);
        return s32_ret;
    }

    return s32_ret;

}

xmedia_s32 sample_comm_ive_read_file(xmedia_ive_image_s* pst_img, FILE* pfp)
{
    xmedia_u16 y;
    xmedia_u8* p_u8;
    xmedia_u16 height;
    xmedia_u16 width;
    xmedia_u16 loop;
    xmedia_s32 s32_ret;

    (xmedia_void)fgetc(pfp);
    if (feof(pfp))
    {
        SAMPLE_PRT("end of file!\n");
        s32_ret = fseek(pfp, 0 , SEEK_SET );
        if (0 != s32_ret)
        {
            SAMPLE_PRT("fseek failed!\n");
            return s32_ret;
        }

    }
    else
    {
        s32_ret = fseek(pfp, -1 , SEEK_CUR );
        if (0 != s32_ret)
        {
            SAMPLE_PRT("fseek failed!\n");
            return s32_ret;
        }
    }

    height = pst_img->u32height;
    width = pst_img->u32width;

    switch (pst_img->en_type)
    {
        case  XMEDIA_IVE_IMAGE_TYPE_U8C1:
        {
            p_u8 = (xmedia_u8 *)(xmedia_ulong)pst_img->au64viraddr[0];
            for (y = 0; y < height; y++)
            {
                if ( 1 != fread(p_u8, width, 1, pfp))
                {
                    SAMPLE_PRT("Read file fail\n");
                    return XMEDIA_FAILURE;
                }

                p_u8 += pst_img->au32stride[0];
            }
        }
        break;
        case  XMEDIA_IVE_IMAGE_TYPE_YUV420SP:
        {
            p_u8 = (xmedia_u8 *)(xmedia_ulong)pst_img->au64viraddr[0];
            for (y = 0; y < height; y++)
            {
                if ( 1 != fread(p_u8, width, 1, pfp))
                {
                    SAMPLE_PRT("Read file fail\n");
                    return XMEDIA_FAILURE;
                }

                p_u8 += pst_img->au32stride[0];
            }

            p_u8 = (xmedia_u8 *)(xmedia_ulong)pst_img->au64viraddr[1];
            for (y = 0; y < height / 2; y++)
            {
                if ( 1 != fread(p_u8, width, 1, pfp))
                {
                    SAMPLE_PRT("Read file fail\n");
                    return XMEDIA_FAILURE;
                }

                p_u8 += pst_img->au32stride[1];
            }
        }
        break;
        case XMEDIA_IVE_IMAGE_TYPE_YUV422SP:
        {
            p_u8 = (xmedia_u8 *)(xmedia_ulong)pst_img->au64viraddr[0];
            for (y = 0; y < height; y++)
            {
                if ( 1 != fread(p_u8, width, 1, pfp))
                {
                    SAMPLE_PRT("Read file fail\n");
                    return XMEDIA_FAILURE;
                }

                p_u8 += pst_img->au32stride[0];
            }

            p_u8 = (xmedia_u8 *)(xmedia_ulong)pst_img->au64viraddr[1];
            for (y = 0; y < height; y++)
            {
                if ( 1 != fread(p_u8, width, 1, pfp))
                {
                    SAMPLE_PRT("Read file fail\n");
                    return XMEDIA_FAILURE;
                }

                p_u8 += pst_img->au32stride[1];
            }
        }
        break;
        case XMEDIA_IVE_IMAGE_TYPE_U8C3_PACKAGE:
        {
            p_u8 = (xmedia_u8 *)(xmedia_ulong)pst_img->au64viraddr[0];
            for (y = 0; y < height; y++)
            {
                if ( 1 != fread(p_u8, width * 3, 1, pfp))
                {
                    SAMPLE_PRT("Read file fail\n");
                    return XMEDIA_FAILURE;
                }

                p_u8 += pst_img->au32stride[0] * 3;
            }

        }
        break;
        case XMEDIA_IVE_IMAGE_TYPE_U8C3_PLANAR:
        {
            for (loop = 0; loop < 3; loop++)
            {
                p_u8 = (xmedia_u8 *)(xmedia_ulong)pst_img->au64viraddr[loop];
                for (y = 0; y < height; y++)
                {
                    if ( 1 != fread(p_u8, width, 1, pfp))
                    {
                        SAMPLE_PRT("Read file fail\n");
                        return XMEDIA_FAILURE;
                    }

                    p_u8 += pst_img->au32stride[loop];
                }
            }

        }
        break;
        case XMEDIA_IVE_IMAGE_TYPE_S16C1:
        case XMEDIA_IVE_IMAGE_TYPE_U16C1:
        {
            p_u8 = (xmedia_u8 *)(xmedia_ulong)pst_img->au64viraddr[0];
            for ( y = 0; y < height; y++ )
            {
                if ( sizeof(xmedia_u16) != fread(p_u8, width, sizeof(xmedia_u16), pfp) )
                {
                    SAMPLE_PRT("Read file fail\n");
                    return XMEDIA_FAILURE;
                }

                p_u8 += pst_img->au32stride[0] * 2;
            }
        }
        break;
        default:
            break;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_ive_write_file(xmedia_ive_image_s* pst_img, FILE* pfp)
{
    xmedia_u16 y;
    xmedia_u8* p_u8;
    xmedia_u16 height;
    xmedia_u16 width;

    height = pst_img->u32height;
    width = pst_img->u32width;

    switch (pst_img->en_type)
    {
        case  XMEDIA_IVE_IMAGE_TYPE_U8C1:
        case  XMEDIA_IVE_IMAGE_TYPE_S8C1:
        {
            p_u8 = (xmedia_u8 *)(xmedia_ulong)pst_img->au64viraddr[0];
            for (y = 0; y < height; y++)
            {
                if ( 1 != fwrite(p_u8, width, 1, pfp))
                {
                    SAMPLE_PRT("Write file fail\n");
                    return XMEDIA_FAILURE;
                }

                p_u8 += pst_img->au32stride[0];
            }
        }
        break;
        case  XMEDIA_IVE_IMAGE_TYPE_YUV420SP:
        {
            p_u8 = (xmedia_u8 *)(xmedia_ulong)pst_img->au64viraddr[0];
            for (y = 0; y < height; y++)
            {
                if ( width != fwrite(p_u8, 1, width, pfp))
                {
                    SAMPLE_PRT("Write file fail\n");
                    return XMEDIA_FAILURE;
                }

                p_u8 += pst_img->au32stride[0];
            }

            p_u8 = (xmedia_u8 *)(xmedia_ulong)pst_img->au64viraddr[1];
            for (y = 0; y < height / 2; y++)
            {
                if ( width != fwrite(p_u8, 1, width, pfp))
                {
                    SAMPLE_PRT("Write file fail\n");
                    return XMEDIA_FAILURE;
                }

                p_u8 += pst_img->au32stride[1];
            }
        }
        break;
        case XMEDIA_IVE_IMAGE_TYPE_YUV422SP:
        {
            p_u8 = (xmedia_u8 *)(xmedia_ulong)pst_img->au64viraddr[0];
            for (y = 0; y < height; y++)
            {
                if ( width != fwrite(p_u8, 1, width, pfp))
                {
                    SAMPLE_PRT("Write file fail\n");
                    return XMEDIA_FAILURE;
                }

                p_u8 += pst_img->au32stride[0];
            }

            p_u8 = (xmedia_u8 *)(xmedia_ulong)pst_img->au64viraddr[1];
            for (y = 0; y < height; y++)
            {
                if ( width != fwrite(p_u8, 1, width, pfp))
                {
                    SAMPLE_PRT("Write file fail\n");
                    return XMEDIA_FAILURE;
                }

                p_u8 += pst_img->au32stride[1];
            }
        }
        break;
        case XMEDIA_IVE_IMAGE_TYPE_S16C1:
        case  XMEDIA_IVE_IMAGE_TYPE_U16C1:
        {
            p_u8 = (xmedia_u8 *)(xmedia_ulong)pst_img->au64viraddr[0];
            for ( y = 0; y < height; y++ )
            {
                if ( sizeof(xmedia_u16) != fwrite(p_u8, width, sizeof(xmedia_u16), pfp) )
                {
                    SAMPLE_PRT("Write file fail\n");
                    return XMEDIA_FAILURE;
                }

                p_u8 += pst_img->au32stride[0] * 2;
            }
        }
        break;
        case XMEDIA_IVE_IMAGE_TYPE_U32C1:
        {

            p_u8 = (xmedia_u8 *)(xmedia_ulong)pst_img->au64viraddr[0];
            for ( y = 0; y < height; y++ )
            {
                if ( width != fwrite(p_u8, sizeof(xmedia_u32), width, pfp) )
                {
                    SAMPLE_PRT("Write file fail\n");
                    return XMEDIA_FAILURE;
                }

                p_u8 += pst_img->au32stride[0] * 4;
            }
            break;
        }

        default:
            break;
    }

    return XMEDIA_SUCCESS;
}

xmedia_void sample_comm_ive_blob_to_rect(xmedia_ive_ccblob_s *pst_blob, sample_rect_array_s *pst_rect,
                                            xmedia_u16 u16_rect_max_num,xmedia_u16 u16_area_thr_step,
                                            xmedia_u32 u32_src_width, xmedia_u32 u32_src_height,
                                            xmedia_u32 u32_dst_width,xmedia_u32 u32_dst_height)
{
    xmedia_u16 u16_num;
    xmedia_u16 i,j,k;
    xmedia_u16 u16_thr= 0;
    xmedia_bool b_valid;

    if(pst_blob->u8_region_num > u16_rect_max_num)
    {
        u16_thr = pst_blob->u16_cur_area_thr;
        do
        {
            u16_num = 0;
            u16_thr += u16_area_thr_step;
            for(i = 0;i < 254;i++)
            {
                if(pst_blob->ast_region[i].u32_area > u16_thr)
                {
                    u16_num++;
                }
            }
        }while(u16_num > u16_rect_max_num);
    }

   u16_num = 0;

   for(i = 0;i < 254;i++)
    {
        if(pst_blob->ast_region[i].u32_area > u16_thr)
        {
            pst_rect->ast_rect[u16_num].ast_point[0].x = (xmedia_u32)((xmedia_float)pst_blob->ast_region[i].u16_left / (xmedia_float)u32_src_width * (xmedia_float)u32_dst_width) & (~1) ;
            pst_rect->ast_rect[u16_num].ast_point[0].y = (xmedia_u32)((xmedia_float)pst_blob->ast_region[i].u16_top / (xmedia_float)u32_src_height * (xmedia_float)u32_dst_height) & (~1);

            pst_rect->ast_rect[u16_num].ast_point[1].x = (xmedia_u32)((xmedia_float)pst_blob->ast_region[i].u16_right/ (xmedia_float)u32_src_width * (xmedia_float)u32_dst_width) & (~1);
            pst_rect->ast_rect[u16_num].ast_point[1].y = (xmedia_u32)((xmedia_float)pst_blob->ast_region[i].u16_top / (xmedia_float)u32_src_height * (xmedia_float)u32_dst_height) & (~1);

            pst_rect->ast_rect[u16_num].ast_point[2].x = (xmedia_u32)((xmedia_float)pst_blob->ast_region[i].u16_right / (xmedia_float)u32_src_width * (xmedia_float)u32_dst_width) & (~1);
            pst_rect->ast_rect[u16_num].ast_point[2].y = (xmedia_u32)((xmedia_float)pst_blob->ast_region[i].u16_bottom / (xmedia_float)u32_src_height * (xmedia_float)u32_dst_height) & (~1);

            pst_rect->ast_rect[u16_num].ast_point[3].x = (xmedia_u32)((xmedia_float)pst_blob->ast_region[i].u16_left / (xmedia_float)u32_src_width * (xmedia_float)u32_dst_width) & (~1);
            pst_rect->ast_rect[u16_num].ast_point[3].y = (xmedia_u32)((xmedia_float)pst_blob->ast_region[i].u16_bottom / (xmedia_float)u32_src_height * (xmedia_float)u32_dst_height) & (~1);

            b_valid = XMEDIA_TRUE;
            for(j = 0; j < 3;j++)
            {
                for (k = j + 1; k < 4;k++)
                {
                    if ((pst_rect->ast_rect[u16_num].ast_point[j].x == pst_rect->ast_rect[u16_num].ast_point[k].x)
                         &&(pst_rect->ast_rect[u16_num].ast_point[j].y == pst_rect->ast_rect[u16_num].ast_point[k].y))
                    {
                    b_valid = XMEDIA_FALSE;
                    break;
                    }
                }
            }
            if (XMEDIA_TRUE == b_valid)
            {
                u16_num++;
            }
        }
    }

    pst_rect->u16_num = u16_num;
}

/******************************************************************************
* function : Create ive image
******************************************************************************/
xmedia_s32 sample_comm_ive_create_image(xmedia_ive_image_s* pst_img, xmedia_ive_image_type_e en_type, xmedia_u32 u32width, xmedia_u32 u32height)
{
    xmedia_u32 u32size = 0;
    xmedia_s32 s32_ret;
    if (NULL == pst_img)
    {
        SAMPLE_PRT("pst_img is null\n");
        return XMEDIA_FAILURE;
    }

    pst_img->en_type = en_type;
    pst_img->u32width = u32width;
    pst_img->u32height = u32height;
    pst_img->au32stride[0] = sample_comm_ive_calc_stride(pst_img->u32width, IVE_ALIGN);

    switch (en_type)
    {
        case XMEDIA_IVE_IMAGE_TYPE_U8C1:
        case XMEDIA_IVE_IMAGE_TYPE_S8C1:
        {
            u32size = pst_img->au32stride[0] * pst_img->u32height;
            s32_ret = sample_ive_sys_mmz_alloc(&pst_img->au64phyaddr[0], (xmedia_void**)&pst_img->au64viraddr[0], NULL, XMEDIA_NULL, u32size);
            if (s32_ret != XMEDIA_SUCCESS)
            {
                SAMPLE_PRT("Mmz Alloc fail,Error(%#x)\n", s32_ret);
                return s32_ret;
            }
        }
        break;
        case XMEDIA_IVE_IMAGE_TYPE_YUV420SP:
        {
            u32size = pst_img->au32stride[0] * pst_img->u32height * 3 / 2;
            s32_ret = sample_ive_sys_mmz_alloc(&pst_img->au64phyaddr[0], (xmedia_void**)&pst_img->au64viraddr[0], NULL, XMEDIA_NULL, u32size);
            if (s32_ret != XMEDIA_SUCCESS)
            {
                SAMPLE_PRT("Mmz Alloc fail,Error(%#x)\n", s32_ret);
                return s32_ret;
            }
            pst_img->au32stride[1] = pst_img->au32stride[0];
            pst_img->au64phyaddr[1] = pst_img->au64phyaddr[0] + pst_img->au32stride[0] * pst_img->u32height;
            pst_img->au64viraddr[1] = pst_img->au64viraddr[0] + pst_img->au32stride[0] * pst_img->u32height;

        }
        break;
        case XMEDIA_IVE_IMAGE_TYPE_YUV422SP:
        {
            u32size = pst_img->au32stride[0] * pst_img->u32height * 2;
            s32_ret = sample_ive_sys_mmz_alloc(&pst_img->au64phyaddr[0], (xmedia_void**)&pst_img->au64viraddr[0], NULL, XMEDIA_NULL, u32size);
            if (s32_ret != XMEDIA_SUCCESS)
            {
                SAMPLE_PRT("Mmz Alloc fail,Error(%#x)\n", s32_ret);
                return s32_ret;
            }
            pst_img->au32stride[1] = pst_img->au32stride[0];
            pst_img->au64phyaddr[1] = pst_img->au64phyaddr[0] + pst_img->au32stride[0] * pst_img->u32height;
            pst_img->au64viraddr[1] = pst_img->au64viraddr[0] + pst_img->au32stride[0] * pst_img->u32height;

        }
        break;
        case XMEDIA_IVE_IMAGE_TYPE_YUV420P:
            break;
        case XMEDIA_IVE_IMAGE_TYPE_YUV422P:
            break;
        case XMEDIA_IVE_IMAGE_TYPE_S8C2_PACKAGE:
            break;
        case XMEDIA_IVE_IMAGE_TYPE_S8C2_PLANAR:
            break;
        case XMEDIA_IVE_IMAGE_TYPE_S16C1:
        case XMEDIA_IVE_IMAGE_TYPE_U16C1:
        {

            u32size = pst_img->au32stride[0] * pst_img->u32height * sizeof(xmedia_u16);
            s32_ret = sample_ive_sys_mmz_alloc(&pst_img->au64phyaddr[0], (xmedia_void**)&pst_img->au64viraddr[0], NULL, XMEDIA_NULL, u32size);
            if (s32_ret != XMEDIA_SUCCESS)
            {
                SAMPLE_PRT("Mmz Alloc fail,Error(%#x)\n", s32_ret);
                return s32_ret;
            }
        }
        break;
        case XMEDIA_IVE_IMAGE_TYPE_U8C3_PACKAGE:
        {
            u32size = pst_img->au32stride[0] * pst_img->u32height * 3;
            s32_ret = sample_ive_sys_mmz_alloc(&pst_img->au64phyaddr[0], (xmedia_void**)&pst_img->au64viraddr[0], NULL, XMEDIA_NULL, u32size);
            if (s32_ret != XMEDIA_SUCCESS)
            {
                SAMPLE_PRT("Mmz Alloc fail,Error(%#x)\n", s32_ret);
                return s32_ret;
            }
            pst_img->au64viraddr[1] = pst_img->au64viraddr[0] + 1;
            pst_img->au64viraddr[2] = pst_img->au64viraddr[1] + 1;
            pst_img->au64phyaddr[1] = pst_img->au64phyaddr[0] + 1;
            pst_img->au64phyaddr[2] = pst_img->au64phyaddr[1] + 1;
            pst_img->au32stride[1] = pst_img->au32stride[0];
            pst_img->au32stride[2] = pst_img->au32stride[0];
        }
        break;
        case XMEDIA_IVE_IMAGE_TYPE_U8C3_PLANAR:
            break;
        case XMEDIA_IVE_IMAGE_TYPE_S32C1:
        case XMEDIA_IVE_IMAGE_TYPE_U32C1:
        {
            u32size = pst_img->au32stride[0] * pst_img->u32height * sizeof(xmedia_u32);
            s32_ret = sample_ive_sys_mmz_alloc(&pst_img->au64phyaddr[0], (xmedia_void**)&pst_img->au64viraddr[0], NULL, XMEDIA_NULL, u32size);
            if (s32_ret != XMEDIA_SUCCESS)
            {
                SAMPLE_PRT("Mmz Alloc fail,Error(%#x)\n", s32_ret);
                return s32_ret;
            }
        }
        break;
        case XMEDIA_IVE_IMAGE_TYPE_S64C1:
        case XMEDIA_IVE_IMAGE_TYPE_U64C1:
        {

            u32size = pst_img->au32stride[0] * pst_img->u32height * sizeof(xmedia_u64);
            s32_ret = sample_ive_sys_mmz_alloc(&pst_img->au64phyaddr[0], (xmedia_void**)&pst_img->au64viraddr[0], NULL, XMEDIA_NULL, u32size);
            if (s32_ret != XMEDIA_SUCCESS)
            {
                SAMPLE_PRT("Mmz Alloc fail,Error(%#x)\n", s32_ret);
                return s32_ret;
            }
        }
        break;
        default:
            break;

    }

    return XMEDIA_SUCCESS;
}
/******************************************************************************
* function : Create memory info
******************************************************************************/
xmedia_s32 sample_comm_ive_create_mem_info(xmedia_ive_mem_info_s* pst_mem_info, xmedia_u32 u32size)
{
    xmedia_s32 s32_ret;

    if (NULL == pst_mem_info)
    {
        SAMPLE_PRT("pst_mem_info is null\n");
        return XMEDIA_FAILURE;
    }
    pst_mem_info->u32size = u32size;
    s32_ret = sample_ive_sys_mmz_alloc(&pst_mem_info->u64phyaddr, (xmedia_void**)&pst_mem_info->u64viraddr, NULL, XMEDIA_NULL, u32size);
    if (s32_ret != XMEDIA_SUCCESS)
    {
        SAMPLE_PRT("Mmz Alloc fail,Error(%#x)\n", s32_ret);
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}
/******************************************************************************
* function : Create ive image by cached
******************************************************************************/
xmedia_s32 sample_comm_ive_create_image_by_cached(xmedia_ive_image_s* pst_img,
        xmedia_ive_image_type_e en_type, xmedia_u32 u32width, xmedia_u32 u32height)
{
    xmedia_u32 u32size = 0;
    xmedia_s32 s32_ret;
    if (NULL == pst_img)
    {
        SAMPLE_PRT("pst_img is null\n");
        return XMEDIA_FAILURE;
    }

    pst_img->en_type = en_type;
    pst_img->u32width = u32width;
    pst_img->u32height = u32height;
    pst_img->au32stride[0] = sample_comm_ive_calc_stride(pst_img->u32width, IVE_ALIGN);

    switch (en_type)
    {
        case XMEDIA_IVE_IMAGE_TYPE_U8C1:
        case XMEDIA_IVE_IMAGE_TYPE_S8C1:
        {
            u32size = pst_img->au32stride[0] * pst_img->u32height;
            s32_ret = sample_ive_sys_mmz_alloc_cached(&pst_img->au64phyaddr[0], (xmedia_void**)&pst_img->au64viraddr[0], NULL, XMEDIA_NULL, u32size);
            if (s32_ret != XMEDIA_SUCCESS)
            {
                SAMPLE_PRT("Mmz Alloc fail,Error(%#x)\n", s32_ret);
                return s32_ret;
            }
        }
        break;
        case XMEDIA_IVE_IMAGE_TYPE_YUV420SP:
            break;
        case XMEDIA_IVE_IMAGE_TYPE_YUV422SP:
            break;
        case XMEDIA_IVE_IMAGE_TYPE_YUV420P:
            break;
        case XMEDIA_IVE_IMAGE_TYPE_YUV422P:
            break;
        case XMEDIA_IVE_IMAGE_TYPE_S8C2_PACKAGE:
            break;
        case XMEDIA_IVE_IMAGE_TYPE_S8C2_PLANAR:
            break;
        case XMEDIA_IVE_IMAGE_TYPE_S16C1:
        case XMEDIA_IVE_IMAGE_TYPE_U16C1:
        {

            u32size = pst_img->au32stride[0] * pst_img->u32height * sizeof(xmedia_u16);
            s32_ret = sample_ive_sys_mmz_alloc_cached(&pst_img->au64phyaddr[0], (xmedia_void**)&pst_img->au64viraddr[0], NULL, XMEDIA_NULL, u32size);
            if (s32_ret != XMEDIA_SUCCESS)
            {
                SAMPLE_PRT("Mmz Alloc fail,Error(%#x)\n", s32_ret);
                return s32_ret;
            }
        }
        break;
        case XMEDIA_IVE_IMAGE_TYPE_U8C3_PACKAGE:
            break;
        case XMEDIA_IVE_IMAGE_TYPE_U8C3_PLANAR:
            break;
        case XMEDIA_IVE_IMAGE_TYPE_S32C1:
        case XMEDIA_IVE_IMAGE_TYPE_U32C1:
        {
            u32size = pst_img->au32stride[0] * pst_img->u32height * sizeof(xmedia_u32);
            s32_ret = sample_ive_sys_mmz_alloc_cached(&pst_img->au64phyaddr[0], (xmedia_void**)&pst_img->au64viraddr[0], NULL, XMEDIA_NULL, u32size);
            if (s32_ret != XMEDIA_SUCCESS)
            {
                SAMPLE_PRT("Mmz Alloc fail,Error(%#x)\n", s32_ret);
                return s32_ret;
            }
        }
        break;
        case XMEDIA_IVE_IMAGE_TYPE_S64C1:
        case XMEDIA_IVE_IMAGE_TYPE_U64C1:
        {

            u32size = pst_img->au32stride[0] * pst_img->u32height * sizeof(xmedia_u64);
            s32_ret = sample_ive_sys_mmz_alloc_cached(&pst_img->au64phyaddr[0], (xmedia_void**)&pst_img->au64viraddr[0], NULL, XMEDIA_NULL, u32size);
            if (s32_ret != XMEDIA_SUCCESS)
            {
                SAMPLE_PRT("Mmz Alloc fail,Error(%#x)\n", s32_ret);
                return s32_ret;
            }
        }
        break;
        default:
            break;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_ive_create_data(xmedia_ive_data_s* pst_data,xmedia_u32 u32width, xmedia_u32 u32height)
{
    xmedia_s32 s32_ret;
    xmedia_u32 u32size;

    if (NULL == pst_data)
    {
        SAMPLE_PRT("pst_data is null\n");
        return XMEDIA_FAILURE;
    }
    pst_data->u32width = u32width;
    pst_data->u32height = u32height;
    pst_data->u32stride = sample_comm_ive_calc_stride(pst_data->u32width, IVE_ALIGN);
    u32size = pst_data->u32stride * pst_data->u32height;
    s32_ret = sample_ive_sys_mmz_alloc(&pst_data->u64phyaddr, (xmedia_void**)&pst_data->u64viraddr, NULL, XMEDIA_NULL, u32size);
    if (s32_ret != XMEDIA_SUCCESS)
    {
        SAMPLE_PRT("Mmz Alloc fail,Error(%#x)\n", s32_ret);
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}



xmedia_s32 sample_ive_comm_sys_get_pic_size(pic_size_e enPicSize, xmedia_video_size* pst_size)
{
    switch (enPicSize)
    {
        case PIC_CIF:   /* 352 * 288 */
            pst_size->width  = 352;
            pst_size->height = 288;
            break;

        case PIC_360P:   /* 640 * 360 */
            pst_size->width  = 640;
            pst_size->height = 360;
            break;

        case PIC_VGA:   /* 640 * 480 */
            pst_size->width  = 640;
            pst_size->height = 480;
            break;
        case PIC_640x360:   /* 640 * 360 */
            pst_size->width  = 640;
            pst_size->height = 360;
            break;

        case PIC_D1_PAL:   /* 720 * 576 */
            pst_size->width  = 720;
            pst_size->height = 576;
            break;

        case PIC_D1_NTSC:   /* 720 * 480 */
            pst_size->width  = 720;
            pst_size->height = 480;
            break;

        case PIC_720P:   /* 1280 * 720 */
            pst_size->width  = 1280;
            pst_size->height = 720;
            break;

        case PIC_1080P:  /* 1920 * 1080 */
            pst_size->width  = 1920;
            pst_size->height = 1080;
            break;

        case PIC_2304x1296:  /* 2304 * 1296 */
            pst_size->width  = 2304;
            pst_size->height = 1296;
            break;

        case PIC_2560x1440:  /* 2560 * 1440 */
            pst_size->width  = 2560;
            pst_size->height = 1440;
            break;

        case PIC_2592x1520:
            pst_size->width  = 2592;
            pst_size->height = 1520;
            break;

        case PIC_2592x1944:
            pst_size->width  = 2592;
            pst_size->height = 1944;
            break;

        case PIC_2592x1536:
            pst_size->width  = 2592;
            pst_size->height = 1536;
            break;

        case PIC_2688x1520:
            pst_size->width  = 2688;
            pst_size->height = 1520;
            break;

        case PIC_2716x1524:
            pst_size->width  = 2716;
            pst_size->height = 1524;
            break;

        case PIC_3840x2160:
            pst_size->width  = 3840;
            pst_size->height = 2160;
            break;

        case PIC_3000x3000:
            pst_size->width  = 3000;
            pst_size->height = 3000;
            break;

        case PIC_4000x3000:
            pst_size->width  = 4000;
            pst_size->height = 3000;
            break;

        case PIC_4096x2160:
            pst_size->width  = 4096;
            pst_size->height = 2160;
            break;

        case PIC_7680x4320:
            pst_size->width  = 7680;
            pst_size->height = 4320;
            break;
        case PIC_3840x8640:
            pst_size->width = 3840;
            pst_size->height = 8640;
            break;
        case PIC_2688x1536:
            pst_size->width  = 2688;
            pst_size->height = 1536;
            break;
        case PIC_2688x1944:
            pst_size->width  = 2688;
            pst_size->height = 1944;
            break;
        case PIC_2880x1620:
            pst_size->width  = 2880;
            pst_size->height = 1620;
            break;
        case PIC_3072x1728:
            pst_size->width  = 3072;
            pst_size->height = 1728;
            break;
        default:
            return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

/******************************************************************************
* function : Dma frame info to  ive image
******************************************************************************/
xmedia_s32 sample_comm_ive_dma_image(xmedia_video_frame_info *pst_frame_info,xmedia_ive_dst_image_s *pst_dst,xmedia_bool b_instant)
{
    xmedia_s32 s32_ret;
    xmedia_ive_handle h_ive_handle;
    xmedia_ive_src_data_s st_src_data;
    xmedia_ive_dst_data_s st_dst_data;
    xmedia_ive_dma_ctrl_s st_ctrl = {XMEDIA_IVE_DMA_MODE_DIRECT_COPY,0};
    xmedia_bool b_finish = XMEDIA_FALSE;
    xmedia_bool b_block = XMEDIA_TRUE;

    //fill src
    //st_src_data.u64viraddr = pst_frame_info->stVFrame.u64viraddr[0];
    //need test
    st_src_data.u64phyaddr = pst_frame_info->frame.addr.y_phy_addr;
    st_src_data.u32width   = pst_frame_info->frame.width;
    st_src_data.u32height  = pst_frame_info->frame.height;
    st_src_data.u32stride  = pst_frame_info->frame.stride.y_stride;

    //fill dst
    //st_dst_data.u64viraddr = pst_dst->au64viraddr[0];
    st_dst_data.u64phyaddr = pst_dst->au64phyaddr[0];
    st_dst_data.u32width   = pst_dst->u32width;
    st_dst_data.u32height  = pst_dst->u32height;
    st_dst_data.u32stride  = pst_dst->au32stride[0];

    s32_ret = xmedia_ive_dma(&h_ive_handle,&st_src_data,&st_dst_data,&st_ctrl,b_instant);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_dma failed!\n",s32_ret);

    if (XMEDIA_TRUE == b_instant)
    {
        s32_ret = xmedia_ive_query(h_ive_handle,&b_finish,b_block);
        while(ERR_CODE_IVE_QUERY_TIMEOUT == s32_ret)
        {
            usleep(100);
            s32_ret = xmedia_ive_query(h_ive_handle,&b_finish,b_block);
        }
        SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_query failed!\n",s32_ret);
    }

    return XMEDIA_SUCCESS;
}

/*
 * function : Create memory info with cache
 */
xmedia_s32 sample_common_malloc_mem_cache(xmedia_u64 *phys_addr, void **virt_addr, xmedia_u32 size)
{
    xmedia_s32 ret;

    ret = sample_ive_sys_mmz_alloc_cached(phys_addr, virt_addr, (xmedia_char*)NULL, (xmedia_char*)XMEDIA_NULL, size);
    SAMPLE_CHECK_EXPR_RET(ret != XMEDIA_SUCCESS, ret, "Error(%#x), MmzAlloc with cache failed!\n", ret);
    return ret;
}

/*
 * function : free memory
 */
xmedia_s32 sample_common_free_mem(xmedia_u64 phys_addr, void *virt_addr)
{
    xmedia_s32 ret;

    ret = sample_ive_sys_mmz_free(phys_addr, virt_addr);
    SAMPLE_CHECK_EXPR_RET(ret != XMEDIA_SUCCESS, ret, "Error(%#x), sample_ive_sys_mmz_free failed!\n", ret);
    return ret;
}




