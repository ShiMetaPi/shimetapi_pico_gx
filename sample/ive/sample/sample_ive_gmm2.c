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
#include <semaphore.h>
#include <pthread.h>

#include "sample_comm_ive.h"

#define SAMPLE_IVE_GMM2_MAX(a,b) ((a) > (b) ? (a) : (b))

typedef struct sample_ive_gmm2_s
{
    xmedia_ive_src_image_s ast_src[2];
    xmedia_ive_dst_image_s st_fg;
    xmedia_ive_dst_image_s st_bg;
    xmedia_ive_src_image_s st_factor;
    xmedia_ive_dst_image_s st_match_model_info;
    xmedia_ive_src_image_s st_fg_mask;
    xmedia_ive_src_image_s st_last_img;
    xmedia_ive_dst_image_s st_diff_img;
    xmedia_ive_dst_image_s st_mag_img;
    xmedia_ive_dst_image_s st_cur_norm_mag;
    xmedia_ive_dst_image_s st_last_norm_mag;
    xmedia_ive_dst_mem_info_s st_model;
    xmedia_ive_dst_mem_info_s st_blob;
    xmedia_ive_gmm2_ctrl_s st_gmm2_ctrl;

    FILE *pfp_src;
    FILE *pfp_fg;
    FILE *pfp_bg;
} sample_ive_gmm2_s;

static sample_ive_gmm2_s     g_s_st_gmm2;
static xmedia_void sample_ive_gmm2_uninit(sample_ive_gmm2_s *pst_gmm2)
{
    xmedia_u16 i;
    for (i = 0; i < 2; i++)
    {
        IVE_MMZ_FREE(pst_gmm2->ast_src[i].au64phyaddr[0], pst_gmm2->ast_src[i].au64viraddr[0]);
    }
    IVE_MMZ_FREE(pst_gmm2->st_fg.au64phyaddr[0], pst_gmm2->st_fg.au64viraddr[0]);
    IVE_MMZ_FREE(pst_gmm2->st_bg.au64phyaddr[0], pst_gmm2->st_bg.au64viraddr[0]);
    IVE_MMZ_FREE(pst_gmm2->st_factor.au64phyaddr[0], pst_gmm2->st_factor.au64viraddr[0]);
    IVE_MMZ_FREE(pst_gmm2->st_match_model_info.au64phyaddr[0], pst_gmm2->st_match_model_info.au64viraddr[0]);
    IVE_MMZ_FREE(pst_gmm2->st_fg_mask.au64phyaddr[0], pst_gmm2->st_fg_mask.au64viraddr[0]);
    IVE_MMZ_FREE(pst_gmm2->st_diff_img.au64phyaddr[0], pst_gmm2->st_diff_img.au64viraddr[0]);
    IVE_MMZ_FREE(pst_gmm2->st_mag_img.au64phyaddr[0], pst_gmm2->st_mag_img.au64viraddr[0]);
    IVE_MMZ_FREE(pst_gmm2->st_cur_norm_mag.au64phyaddr[0], pst_gmm2->st_cur_norm_mag.au64viraddr[0]);
    IVE_MMZ_FREE(pst_gmm2->st_last_norm_mag.au64phyaddr[0], pst_gmm2->st_last_norm_mag.au64viraddr[0]);
    IVE_MMZ_FREE(pst_gmm2->st_model.u64phyaddr, pst_gmm2->st_model.u64viraddr);
    IVE_MMZ_FREE(pst_gmm2->st_blob.u64phyaddr, pst_gmm2->st_blob.u64viraddr);

    IVE_CLOSE_FILE(pst_gmm2->pfp_src);
    IVE_CLOSE_FILE(pst_gmm2->pfp_fg);
    IVE_CLOSE_FILE(pst_gmm2->pfp_bg);
}

static xmedia_s32 sample_ive_gmm2_init(sample_ive_gmm2_s *pst_gmm2,xmedia_u32 u32width,xmedia_u32 u32height, xmedia_char *pch_src_file_name,
            xmedia_char *pch_fg_file_name,xmedia_char *pch_bg_file_name)
{
    xmedia_s32 s32_ret = XMEDIA_SUCCESS;
    xmedia_u32 u32size;
    xmedia_u16 i,j;
    xmedia_u8 *pu8_tmp;
    memset(pst_gmm2, 0, sizeof(sample_ive_gmm2_s));

    for (i = 0; i < 2; i++)
    {
        s32_ret = sample_comm_ive_create_image(&(pst_gmm2->ast_src[i]), XMEDIA_IVE_IMAGE_TYPE_U8C1, u32width, u32height);
        SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, GMM2_INIT_FAIL,
            "Error(%#x),Create src[%d] image failed!\n", s32_ret,i);
    }

    s32_ret = sample_comm_ive_create_image(&(pst_gmm2->st_fg), XMEDIA_IVE_IMAGE_TYPE_U8C1, u32width, u32height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, GMM2_INIT_FAIL,
        "Error(%#x),Create fg image failed!\n", s32_ret);

    s32_ret = sample_comm_ive_create_image(&(pst_gmm2->st_bg), XMEDIA_IVE_IMAGE_TYPE_U8C1, u32width, u32height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, GMM2_INIT_FAIL,
        "Error(%#x),Create bg image failed!\n", s32_ret);

    s32_ret = sample_comm_ive_create_image(&(pst_gmm2->st_factor), XMEDIA_IVE_IMAGE_TYPE_U16C1, u32width, u32height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, GMM2_INIT_FAIL,
        "Error(%#x),Create factor image failed!\n", s32_ret);

    pu8_tmp = SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(xmedia_u8,pst_gmm2->st_factor.au64viraddr[0]);
    for (i = 0; i < pst_gmm2->st_factor.u32height; i++)
    {
        for (j = 0; j < pst_gmm2->st_factor.u32width; j++)
        {
            pu8_tmp[2 * j ]        = 8;
            pu8_tmp[ 2 * j + 1]     = 4;
        }
        pu8_tmp += pst_gmm2->st_factor.au32stride[0] * sizeof(xmedia_u16);
    }
    s32_ret = sample_comm_ive_create_image(&(pst_gmm2->st_match_model_info), XMEDIA_IVE_IMAGE_TYPE_U8C1, u32width, u32height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, GMM2_INIT_FAIL,
        "Error(%#x),Create matchModelInfo image failed!\n", s32_ret);

    s32_ret = sample_comm_ive_create_image(&(pst_gmm2->st_fg_mask), XMEDIA_IVE_IMAGE_TYPE_U8C1, u32width, u32height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, GMM2_INIT_FAIL,
        "Error(%#x),Create fgMask image failed!\n", s32_ret);

    s32_ret = sample_comm_ive_create_image(&(pst_gmm2->st_diff_img), XMEDIA_IVE_IMAGE_TYPE_U8C1, u32width, u32height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, GMM2_INIT_FAIL,
        "Error(%#x),Create diffImg image failed!\n", s32_ret);

    s32_ret = sample_comm_ive_create_image(&(pst_gmm2->st_mag_img), XMEDIA_IVE_IMAGE_TYPE_U16C1, u32width, u32height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, GMM2_INIT_FAIL,
        "Error(%#x),Create magImg image failed!\n", s32_ret);

    s32_ret = sample_comm_ive_create_image(&(pst_gmm2->st_cur_norm_mag), XMEDIA_IVE_IMAGE_TYPE_U8C1, u32width, u32height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, GMM2_INIT_FAIL,
        "Error(%#x),Create curNormImg image failed!\n", s32_ret);

    s32_ret = sample_comm_ive_create_image(&(pst_gmm2->st_last_norm_mag), XMEDIA_IVE_IMAGE_TYPE_U8C1, u32width, u32height);
    SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, GMM2_INIT_FAIL,
        "Error(%#x),Create lastNormImg image failed!\n", s32_ret);

   pst_gmm2->st_gmm2_ctrl.u16_var_rate             = 1;
   pst_gmm2->st_gmm2_ctrl.u8_model_num             = 3;
   pst_gmm2->st_gmm2_ctrl.u9q7_max_var             = (16 * 16)<<7;
   pst_gmm2->st_gmm2_ctrl.u9q7_min_var             = ( 8 *  8)<<7;
   pst_gmm2->st_gmm2_ctrl.u8_glb_sns_factor         = 8;
   pst_gmm2->st_gmm2_ctrl.en_sns_factor_mode         = XMEDIA_IVE_GMM2_SNS_FACTOR_MODE_PIX;
   pst_gmm2->st_gmm2_ctrl.u16_freq_thr             = 12000;
   pst_gmm2->st_gmm2_ctrl.u16_freq_init_val         = 20000;
   pst_gmm2->st_gmm2_ctrl.u16_freq_add_factor         = 0xEF;
   pst_gmm2->st_gmm2_ctrl.u16_freq_redu_factor      = 0xFF00;
   pst_gmm2->st_gmm2_ctrl.u16_life_thr             = 5000;
   pst_gmm2->st_gmm2_ctrl.en_life_update_factor_mode = XMEDIA_IVE_GMM2_LIFE_UPDATE_FACTOR_MODE_GLB;

   u32size = pst_gmm2->st_gmm2_ctrl.u8_model_num * 8 * pst_gmm2->ast_src[0].u32width * pst_gmm2->ast_src[0].u32height;
   s32_ret = sample_comm_ive_create_mem_info(&pst_gmm2->st_model, u32size);
   SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, GMM2_INIT_FAIL,
       "Error(%#x),Create model mem info failed!\n", s32_ret);

   memset(SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(xmedia_void,pst_gmm2->st_model.u64viraddr), 0, pst_gmm2->st_model.u32size);
   u32size = sizeof(xmedia_ive_ccblob_s);
   s32_ret = sample_comm_ive_create_mem_info(&pst_gmm2->st_blob, u32size);
   SAMPLE_CHECK_EXPR_GOTO(XMEDIA_SUCCESS != s32_ret, GMM2_INIT_FAIL,
       "Error(%#x),Create blob mem info failed!\n", s32_ret);

    s32_ret = XMEDIA_FAILURE;
    pst_gmm2->pfp_src = fopen(pch_src_file_name, "rb");
    SAMPLE_CHECK_EXPR_GOTO(NULL == pst_gmm2->pfp_src, GMM2_INIT_FAIL,
        "Error,Open file %s failed!\n", pch_src_file_name);

    pst_gmm2->pfp_fg = fopen(pch_fg_file_name, "wb");
    SAMPLE_CHECK_EXPR_GOTO(NULL == pst_gmm2->pfp_fg, GMM2_INIT_FAIL,
        "Error,Open file %s failed!\n", pch_fg_file_name);

    pst_gmm2->pfp_bg = fopen(pch_bg_file_name, "wb");
    SAMPLE_CHECK_EXPR_GOTO(NULL == pst_gmm2->pfp_bg, GMM2_INIT_FAIL,
        "Error,Open file %s failed!\n", pch_bg_file_name);
    s32_ret = XMEDIA_SUCCESS;

GMM2_INIT_FAIL:
    if (XMEDIA_SUCCESS != s32_ret)
    {
        sample_ive_gmm2_uninit(pst_gmm2);
    }
    return s32_ret;
}

static xmedia_s32 sample_ive_gen_fg_mask(xmedia_ive_src_image_s *pst_fg,xmedia_ive_dst_image_s *pst_fg_mask)
{
    xmedia_s32 s32_ret;
    xmedia_ive_handle ive_handle;
    xmedia_ive_thresh_ctrl_s st_ctrl;

    st_ctrl.en_mode    = XMEDIA_IVE_THRESH_MODE_BINARY;
    st_ctrl.u8_min_val = 0;
    st_ctrl.u8_max_val = 1;
    st_ctrl.u8_low_thr = 5;
    s32_ret = xmedia_ive_thresh(&ive_handle, pst_fg, pst_fg_mask, &st_ctrl, XMEDIA_FALSE);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_thresh failed!\n",s32_ret);

    return s32_ret;
}
/*
*Reduce the factor gradually to the default value
*/
static xmedia_void sample_ive_redu_factor(xmedia_ive_image_s *pst_factor)
{
    xmedia_u16 i,j;
    xmedia_u8 *pu8_tmp;

    pu8_tmp = SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(xmedia_u8,pst_factor->au64viraddr[0]);
    for (i = 0; i < pst_factor->u32height; i++)
    {
        for (j = 0; j < pst_factor->u32width; j++)
        {
            pu8_tmp[j<<1]        = SAMPLE_IVE_GMM2_MAX(8, pu8_tmp[i<<1] - 1);
            pu8_tmp[(i<<1) + 1]    = SAMPLE_IVE_GMM2_MAX(8, pu8_tmp[(i<<1) + 1] - 2);
        }
        pu8_tmp += pst_factor->au32stride[0] * sizeof(xmedia_u16);
    }
}
/*
*Change factor by difference frame
*/
static xmedia_bool sample_ive_chg_factor_by_diff_frm(xmedia_ive_src_image_s *pst_src,xmedia_ive_src_image_s *pst_last_img,
                                                        xmedia_ive_dst_image_s *pst_diff_img,xmedia_ive_dst_image_s *pst_factor)
{
    xmedia_s32 s32_ret;
    xmedia_ive_handle ive_handle;
    xmedia_bool b_finish;
    xmedia_bool b_block = XMEDIA_TRUE;
    xmedia_u16 i,j;
    xmedia_s32 s32_abs_th = 10;
    xmedia_u32 u32_point_sum = 0;
    xmedia_ive_sub_ctrl_s st_sub_ctrl;
    xmedia_u8 *pu8_tmp,*pu8_factor;

    st_sub_ctrl.en_mode = XMEDIA_IVE_SUB_MODE_ABS;
    s32_ret = xmedia_ive_sub(&ive_handle, pst_src, pst_last_img, pst_diff_img, &st_sub_ctrl, XMEDIA_TRUE);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,XMEDIA_FALSE,"Error(%#x),xmedia_ive_sub failed!\n",s32_ret);

    //Wait task finish
    s32_ret = xmedia_ive_query(ive_handle, &b_finish, b_block);
    while (ERR_CODE_IVE_QUERY_TIMEOUT == s32_ret)
    {
        usleep(100);
        s32_ret = xmedia_ive_query(ive_handle, &b_finish, b_block);
    }

    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,XMEDIA_FALSE,"Error(%#x),xmedia_ive_query failed!\n",s32_ret);

    pu8_tmp = SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(xmedia_u8,pst_diff_img->au64viraddr[0]);
    for (i = 0; i < pst_src->u32height; i++)
    {
        for (j = 0; j < pst_src->u32width; j++)
        {
            u32_point_sum += pu8_tmp[j] > s32_abs_th;
        }
        pu8_tmp += pst_diff_img->au32stride[0];
    }


    if ((u32_point_sum * 3) > (pst_src->u32width * pst_src->u32height* 2))
    {
        pu8_tmp   = SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(xmedia_u8,pst_diff_img->au64viraddr[0]);
        pu8_factor = SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(xmedia_u8,pst_factor->au64viraddr[0]);
        for (i = 0; i < pst_src->u32height; i++)
        {
            for (j = 0; j < pst_src->u32width; j++)
            {
                if (pu8_tmp[j] > s32_abs_th)
                {
                    pu8_factor[j<<1]          = 20;
                    pu8_factor[(j<<1) + 1]     = 200;

                }
            }
            pu8_tmp         += pst_diff_img->au32stride[0];
            pu8_factor     += pst_factor->au32stride[0] * sizeof(xmedia_u16);
        }
        return XMEDIA_TRUE;

    }

    return XMEDIA_FALSE;

}

/*
*Change factor by gradient
*/
static xmedia_s32 sample_ive_chg_factor_by_grad(xmedia_ive_src_image_s *pst_fg_mask,
                xmedia_ive_src_image_s *pst_src,xmedia_ive_src_image_s *pst_last_img,xmedia_ive_src_image_s *pst_last_norm_mag,
                xmedia_ive_dst_image_s *pst_mag_img,xmedia_ive_dst_image_s *pst_cur_norm_mag,xmedia_ive_dst_mem_info_s *pst_blob,
                xmedia_ive_dst_image_s *pst_diff_img,xmedia_ive_dst_image_s *pst_factor)
{
    xmedia_s32 s32_ret;
    xmedia_ive_handle ive_handle;
    xmedia_bool b_finish;
    xmedia_bool b_block = XMEDIA_TRUE;
    xmedia_u16 i,j,k;
    xmedia_u16 u16_top;
    xmedia_u16 u16_left;
    xmedia_u16 u16_right;
    xmedia_u16 u16_bottom;
    xmedia_u8 *pu8_fg_cur;
    xmedia_u8 *pu8_grad_diff;
    xmedia_u8 *pu8_factor;
    xmedia_s32 s32_fg_sum    = 0;
    xmedia_s32 s32_grad_sum  = 0;

    xmedia_s8 au8_mask[25] = {0, 0,0,0,0,
                        0,-1,0,1,0,
                        0,-2,0,2,0,
                        0,-1,0,1,0,
                        0, 0,0,0,0};
    xmedia_ive_ccl_ctrl_s st_ccl_ctrl;
    xmedia_ive_ccblob_s   *pst_ccl_blob;
    xmedia_ive_mag_and_ang_ctrl_s   st_mag_and_ang_ctrl;
    xmedia_ive_16bit_to_8bit_ctrl_s st16_to8_ctrl;
    xmedia_ive_sub_ctrl_s st_sub_ctrl;

    st_ccl_ctrl.en_mode   = XMEDIA_IVE_CCL_MODE_8C;
    st_ccl_ctrl.u16_step   = 2;
    st_ccl_ctrl.u16_init_area_thr = 4;
    s32_ret = xmedia_ive_ccl(&ive_handle, pst_fg_mask, pst_blob, &st_ccl_ctrl, XMEDIA_TRUE);
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_ccl failed!\n",s32_ret);

    //Wait task finish
    s32_ret = xmedia_ive_query(ive_handle, &b_finish, b_block);
    while (ERR_CODE_IVE_QUERY_TIMEOUT == s32_ret)
    {
        usleep(100);
        s32_ret = xmedia_ive_query(ive_handle, &b_finish, b_block);
    }
    SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_query failed!\n",s32_ret);

    pst_ccl_blob = SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(xmedia_ive_ccblob_s,pst_blob->u64viraddr);
    if (pst_ccl_blob->u8_region_num > 0)
    {
        //Calc the gradient difference of the current image and the last image
        memcpy(st_mag_and_ang_ctrl.as8_mask, au8_mask, 25);
        st_mag_and_ang_ctrl.u16_thr         = 0;
        st_mag_and_ang_ctrl.en_out_ctrl     = XMEDIA_IVE_MAG_AND_ANG_OUT_CTRL_MAG;
        s32_ret = xmedia_ive_mag_and_ang(&ive_handle, pst_src, pst_mag_img, XMEDIA_NULL, &st_mag_and_ang_ctrl, XMEDIA_FALSE);
        SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_mag_and_ang failed!\n",s32_ret);

        st16_to8_ctrl.s8_bias             = 0;
        st16_to8_ctrl.en_mode             = XMEDIA_IVE_16BIT_TO_8BIT_MODE_U16_TO_U8;
        st16_to8_ctrl.u8_numerator         = 255;
        st16_to8_ctrl.u16_denominator     = 255 * 4;
        s32_ret = xmedia_ive_16bit_to_8bit(&ive_handle, pst_mag_img, pst_cur_norm_mag, &st16_to8_ctrl, XMEDIA_FALSE);
        SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_16bit_to_8bit failed!\n",s32_ret);

        s32_ret = xmedia_ive_mag_and_ang(&ive_handle, pst_last_img, pst_mag_img, XMEDIA_NULL, &st_mag_and_ang_ctrl, XMEDIA_FALSE);
        SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_mag_and_ang failed!\n",s32_ret);

        s32_ret = xmedia_ive_16bit_to_8bit(&ive_handle, pst_mag_img, pst_last_norm_mag, &st16_to8_ctrl, XMEDIA_FALSE);
        SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_16bit_to_8bit failed!\n",s32_ret);

        st_sub_ctrl.en_mode = XMEDIA_IVE_SUB_MODE_ABS;
        s32_ret = xmedia_ive_sub(&ive_handle, pst_cur_norm_mag, pst_last_norm_mag, pst_diff_img, &st_sub_ctrl, XMEDIA_TRUE);
        SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_sub failed!\n",s32_ret);

        s32_ret = xmedia_ive_query(ive_handle, &b_finish, b_block);
        while (ERR_CODE_IVE_QUERY_TIMEOUT == s32_ret)
        {
            usleep(100);
            s32_ret = xmedia_ive_query(ive_handle, &b_finish, b_block);
        }

        SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_query failed!\n",s32_ret);

    }

    //for each blob, analyze the gradient change
    for(k = 0; k < XMEDIA_IVE_MAX_REGION_NUM; k++)
    {
        if(0 == pst_ccl_blob->ast_region[k].u32_area)
        {
            continue;
        }

        u16_top    =  pst_ccl_blob->ast_region[k].u16_top;
        u16_left   =  pst_ccl_blob->ast_region[k].u16_left;
        u16_right  =  pst_ccl_blob->ast_region[k].u16_right;
        u16_bottom =  pst_ccl_blob->ast_region[k].u16_bottom;
        if((u16_right - u16_left) * (u16_bottom - u16_top) < 60)
        {
            continue;
        }

        s32_fg_sum    = 0;
        s32_grad_sum  = 0;
        pu8_fg_cur    = (SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(xmedia_u8,pst_fg_mask->au64viraddr[0])) + u16_top * pst_fg_mask->au32stride[0];
        pu8_grad_diff = (SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(xmedia_u8,pst_diff_img->au64viraddr[0])) + u16_top * pst_diff_img->au32stride[0];
        for (i = u16_top; i < u16_bottom; i++)
        {
            for (j = u16_left; j < u16_right; j++)
            {
                if (pu8_fg_cur[j])
                {
                    s32_fg_sum++;
                    if (pu8_grad_diff[j])
                    {
                        s32_grad_sum++;
                    }
                }
            }

            pu8_fg_cur    += pst_fg_mask->au32stride[0];
            pu8_grad_diff += pst_diff_img->au32stride[0];
        }

        if ((s32_grad_sum < 10) || (s32_grad_sum * 100 < s32_fg_sum))
        {
            pu8_factor = (SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(xmedia_u8,pst_factor->au64viraddr[0])) + 2 * u16_top * pst_factor->au32stride[0];
            pu8_fg_cur = (SAMPLE_COMM_IVE_CONVERT_64BIT_ADDR(xmedia_u8,pst_fg_mask->au64viraddr[0])) + u16_top * pst_fg_mask->au32stride[0];
            for (i = u16_top; i < u16_bottom; i++)
            {
                for (j = u16_left; j < u16_right; j++)
                {
                    if (pu8_fg_cur[j])
                    {
                        pu8_factor[j<<1] = 16;
                        pu8_factor[(j<<1) + 1] = 200;
                    }
                }
                pu8_fg_cur     += pst_fg_mask->au32stride[0];
                pu8_factor    += pst_factor->au32stride[0] * sizeof(xmedia_u16);
            }
        }
    }

    return XMEDIA_SUCCESS;
}
/*
*Adjustment factor
*/
static xmedia_void sample_ive_adjustment_factor(xmedia_ive_src_image_s *pst_factor,
                             xmedia_ive_src_image_s *pst_fg_mask, xmedia_ive_src_image_s *pst_src,
                             xmedia_ive_src_image_s *pst_last_img, xmedia_ive_dst_image_s *pst_diff_img,
                             xmedia_ive_dst_image_s *pst_mag_img, xmedia_ive_dst_image_s *pst_cur_norm_mag,
                             xmedia_ive_dst_image_s *pst_last_norm_mag, xmedia_ive_dst_mem_info_s *pst_blob)
{

    xmedia_bool b_chg;
    //First, reduce the factor gradually to the default value------------------------------
    sample_ive_redu_factor(pst_factor);

    //Second, analyze the frame difference-------------------------------------------------
    //When the number of changed pixels is more than the threshold, there maybe a light switch.
    //And then, we should set a big factor to adapt to it quickly.
    b_chg = sample_ive_chg_factor_by_diff_frm(pst_src,pst_last_img,pst_diff_img,pst_factor);
    if (XMEDIA_TRUE == b_chg)
    {
        return;
    }

    //Third, analyze the gradient for foreground blobs------------------------------------
    //When gradient change of a foreground blob is very small, it maybe a local illumination change,
    //a ghost, or a static object.
    //Here we try to reduce the influence by a local illumination change or a ghost only.
    //分析前景斑点的梯度，当前景斑点的坡度变化很小时，可能是局部照明变化，也可能是重影或静止物体，这里我们试图减少局部照明变化或仅重影的影响
    (xmedia_void)sample_ive_chg_factor_by_grad(pst_fg_mask,pst_src,pst_last_img,pst_last_norm_mag,
                                pst_mag_img,pst_cur_norm_mag,pst_blob,
                                pst_diff_img,    pst_factor);
}

static xmedia_s32 sample_ive_gmm2_proc(sample_ive_gmm2_s *pst_gmm2)
{
    xmedia_s32 s32_ret = XMEDIA_FAILURE;
    xmedia_ive_handle ive_handle;
    xmedia_bool b_finish = XMEDIA_FALSE;
    xmedia_bool b_block = XMEDIA_TRUE;
    xmedia_bool b_instant = XMEDIA_TRUE;
    xmedia_u32 u32FrmNum;
    xmedia_s32 s32CurIdx = 0;
    xmedia_u32 u32TotalFrm = 700;

    for (u32FrmNum = 1;u32FrmNum < u32TotalFrm;u32FrmNum++)
    {

        SAMPLE_PRT("Proc Frame %d/%d\n",u32FrmNum,u32TotalFrm);
        s32_ret = sample_comm_ive_read_file(&(pst_gmm2->ast_src[s32CurIdx]), pst_gmm2->pfp_src);
        SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),Read src file failed!\n",s32_ret);

        //To building a stable background model quickly at the begin, some parameters are set specially.
        if (1 == pst_gmm2->st_gmm2_ctrl.u8_model_num)
        {
            //If the parameter u8ModelNum is set to 1, the parameter u16FreqReduFactor
            //is usually set to a small value at the first N frames. Here, N = 500.
            pst_gmm2->st_gmm2_ctrl.u16_freq_redu_factor = (u32FrmNum >= 500) ? 0xFFA0 : 0xFC00;
        }
        else
        {
            //If the parameter u8ModelNum is more than 1, the global life mode should be used at the first N frames,
            //and the parameter u16GlbLifeUpdateFactor is usually set to a big value. Here, N = 500.
            if(u32FrmNum >= 500)
            {
                pst_gmm2->st_gmm2_ctrl.en_life_update_factor_mode = XMEDIA_IVE_GMM2_LIFE_UPDATE_FACTOR_MODE_PIX;
            }
            else
            {
                pst_gmm2->st_gmm2_ctrl.u16_glb_life_update_factor = 0xFFFF / u32FrmNum;
            }
        }

        //GMM2
        s32_ret = xmedia_ive_gmm2(&ive_handle,&pst_gmm2->ast_src[s32CurIdx],&pst_gmm2->st_factor,&pst_gmm2->st_fg,&pst_gmm2->st_bg,
            &pst_gmm2->st_match_model_info,&pst_gmm2->st_model,&pst_gmm2->st_gmm2_ctrl,b_instant);
        SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_gmm2 failed!\n",s32_ret);

        //factor adjustment
        if (u32FrmNum > 1)
        {
            s32_ret = sample_ive_gen_fg_mask(&(pst_gmm2->st_fg),&(pst_gmm2->st_fg_mask));
            SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),sample_ive_gen_fg_mask failed!\n",s32_ret);

            sample_ive_adjustment_factor(&(pst_gmm2->st_factor), &(pst_gmm2->st_fg_mask),
                                    &(pst_gmm2->ast_src[s32CurIdx]),&(pst_gmm2->ast_src[1 - s32CurIdx]),
                                    &(pst_gmm2->st_diff_img), &(pst_gmm2->st_mag_img), &(pst_gmm2->st_cur_norm_mag),
                                    &(pst_gmm2->st_last_norm_mag), &(pst_gmm2->st_blob));

        }
        else
        {
            s32_ret = xmedia_ive_query(ive_handle, &b_finish, b_block);
            while (ERR_CODE_IVE_QUERY_TIMEOUT == s32_ret)
            {
                usleep(100);
                s32_ret = xmedia_ive_query(ive_handle, &b_finish, b_block);
            }
            SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),xmedia_ive_query failed!\n",s32_ret);
        }

        s32_ret = sample_comm_ive_write_file(&(pst_gmm2->st_fg), pst_gmm2->pfp_fg);
        SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),Write fg file failed!\n",s32_ret);

        s32_ret = sample_comm_ive_write_file(&(pst_gmm2->st_bg), pst_gmm2->pfp_bg);
        SAMPLE_CHECK_EXPR_RET(XMEDIA_SUCCESS != s32_ret,s32_ret,"Error(%#x),Write bg file failed!\n",s32_ret);

        //change cur image to last image
        s32CurIdx = 1 - s32CurIdx;
    }

    return s32_ret;
}

xmedia_void sample_ive_gmm2(xmedia_void)
{
    xmedia_s32                 s32_ret;
    xmedia_u32                 u32width  = 352;
    xmedia_u32                 u32height = 288;
    xmedia_char                *pch_src_file_name = "./data/input/gmm2/gmm2_352x288_sp400_frm1000.yuv";
    xmedia_char                *pch_fg_file_name = "./data/output/gmm2/fg_352x288_sp400.yuv";
    xmedia_char                *pch_bg_file_name = "./data/output/gmm2/bg_352x288_sp400.yuv";

    memset(&g_s_st_gmm2,0,sizeof(g_s_st_gmm2));

    s32_ret = sample_ive_gmm2_init(&g_s_st_gmm2, u32width,u32height,pch_src_file_name,
                    pch_fg_file_name,pch_bg_file_name);
    SAMPLE_CHECK_EXPR(XMEDIA_SUCCESS != s32_ret,"Error(%#x),sample_ive_gmm2_init failed!\n",s32_ret);

    s32_ret = sample_ive_gmm2_proc(&g_s_st_gmm2);
    if (XMEDIA_SUCCESS == s32_ret)
    {
        SAMPLE_PRT("Process success!\n");
    }

    sample_ive_gmm2_uninit(&g_s_st_gmm2);
    memset(&g_s_st_gmm2,0,sizeof(g_s_st_gmm2));
}
/******************************************************************************
* function : Gmm2 sample signal handle
******************************************************************************/
xmedia_void sample_ive_gmm2_handle_sig(xmedia_void)
{
    sample_ive_gmm2_uninit(&g_s_st_gmm2);
    memset(&g_s_st_gmm2,0,sizeof(g_s_st_gmm2));
    sample_comm_ive_ive_mpi_exit();
}


