/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include "hal_vo_csc.h"

#define VO_CSC_TABLE_MAX 61

typedef struct{
    vo_csc_type type;
    const vo_csc_coef *matrix;
} vo_csc_type_matrix;

const vo_csc_coef g_csc_yuv601limit_to_yuv601limit = {
    1024, 0, 0, 0, 1024, 0, 0, 0, 1024,
    -16, -128, -128,
    16, 128, 128,
};

const vo_csc_coef g_csc_yuv601full_to_yuv601limit = {
    880, 0, 0, 0, 899, 0, 0, 0, 899,
    0, -128, -128,
    16, 128, 128,
};

const vo_csc_coef g_csc_yuv709limit_to_yuv601limit = {
    1024, 102, 196, 0, 1014, -113, 0, -74, 1007,
    -16, -128, -128,
    -16, 128, 128,
};

const vo_csc_coef g_csc_yuv709full_to_yuv601limit = {
    880, 90, 173, 0, 891, -99, 0, -65, 864,
    0, -128, -128,
    16, 128, 128,
};

const vo_csc_coef g_csc_yuv601limit_to_yuv709limit = {
    1024, -118, -213, 0, 1043, 117, 0, 77, 1050,
    -16, -128, -128,
    16, 128, 128,
};

const vo_csc_coef g_csc_yuv601full_to_yuv709limit = {
    880, -103, -187, 0, 916, 102, 0, 67, 922,
    0, -128, -128,
    16, 128, 128,
};

const vo_csc_coef g_csc_yuv709limit_to_yuv709limit = {
    1024, 0, 0, 0, 1024, 0, 0, 0, 1024,
    -16, -128, -128,
    16, 128, 128,
};

const vo_csc_coef g_csc_yuv709full_to_yuv709limit = {
    880, 0, 0, 0, 899, 0, 0, 0, 899,
    0, -128, -128,
    16, 128, 128,
};

const vo_csc_coef g_csc_yuv601limit_to_yuv601full = {
    1192, 0, 0, 0, 1165, 0, 0, 0, 1165,
    -16, -128, -128,
    0, 128, 128,
};

const vo_csc_coef g_csc_yuv601full_to_yuv601full = {
    1024, 0, 0, 0, 1024, 0, 0, 0, 1024,
    0, -128, -128,
    0, 128, 128,
};

const vo_csc_coef g_csc_yuv709limit_to_yuv601full = {
    1192, 117, 222, 0, 1154, -128, 0, -84, 1146,
    -16, -128, -128,
    0, 128, 128,
};

const vo_csc_coef g_csc_yuv709full_to_yuv601full = {
    1024, 102, 196, 0, 1014, -113, 0, -74, 1007,
    0, -128, -128,
    0, 128, 128,
};

const vo_csc_coef g_csc_yuv601limit_to_yuv709full = {
    1192, -137, -248, 0, 1188, 133, 0, 87, 1194,
    -16, -128, -128,
    0, 128, 128,
};

const vo_csc_coef g_csc_yuv601full_to_yuv709full = {
    1024, -118, -213, 0, 1043, 117, 0, 77, 1050,
    0, -128, -128,
    0, 128, 128,
};

const vo_csc_coef g_csc_yuv709limit_to_yuv709full = {
    1192, 0, 0, 0, 1165, 0, 0, 0, 1165,
    -16, -128, -128,
    0, 128, 128,
};

const vo_csc_coef g_csc_yuv709full_to_yuv709full = {
    1024, 0, 0, 0, 1024, 0, 0, 0, 1024,
    0, -128, -128,
    0, 128, 128,
};

const vo_csc_coef g_csc_yuv601limit_to_rgbfull = {
    1192, 0, 1634, 1192, -400, -833, 1192, 2066, 0,
    -16, -128, -128,
    0, 0, 0,
};

const vo_csc_coef g_csc_yuv601full_to_rgbfull = {
    1024, 0, 1040, 1024, -344, -706, 1024, 1774, 0,
    0, -128, -128,
    0, 0, 0,
};

const vo_csc_coef g_csc_yuv709limit_to_rgbfull = {
    1192, 0, 1836, 1192, -218, -547, 1192, 2166, 0,
    -16, -128, -128,
    0, 0, 0,
};

const vo_csc_coef g_csc_yuv709full_to_rgbfull = {
    1024, 0, 1577, 1024, -187, -470, 1024, 1860, 0,
    0, -128, -128,
    0, 0, 0,
};

const vo_csc_coef g_csc_yuv601limit_to_rgblimit = {
    1024, 0, 1404, 1024, -344, -716, 1024, 1775, 0,
    -16, -128, -128,
    16, 16, 16,
};

const vo_csc_coef g_csc_yuv601full_to_rgblimit = {
    880, 0, 1233, 880, -302, -629, 880, 1559, 0,
    0, -128, -128,
    16, 16, 16,
};

const vo_csc_coef g_csc_yuv709limit_to_rgblimit = {
    1024, 0, 1578, 1024, -187, -470, 1024, 1861, 0,
    -16, -128, -128,
    16, 16, 16,
};

const vo_csc_coef g_csc_yuv709full_to_rgblimit = {
    880, 0, 1385, 880, -164, -413, 880, 1634, 0,
    0, -128, -128,
    16, 16, 16,
};

const int g_sin_table[VO_CSC_TABLE_MAX] = {
    -500, -485, -469, -454, -438, -422, -407, -391, -374, -358,
    -342, -325, -309, -292, -276, -259, -242, -225, -208, -191,
    -174, -156, -139, -122, -104, -87, -70, -52, -35, -17,
    0, 17, 35, 52, 70, 87, 104, 122, 139, 156,
    174, 191, 208, 225, 242, 259, 276, 292, 309, 325,
    342, 358, 374, 391, 407, 422, 438, 454, 469, 485,
    500
};

const int g_cos_table[VO_CSC_TABLE_MAX] = {
    866, 875, 883, 891, 899, 906, 914, 921, 927, 934,
    940, 946, 951, 956, 961, 966, 970, 974, 978, 982,
    985, 988, 990, 993, 995, 996, 998, 999, 999, 1000,
    1000, 1000, 999, 999, 998, 996, 995, 993, 990, 988,
    985, 982, 978, 974, 970, 966, 961, 956, 951, 946,
    940, 934, 927, 921, 914, 906, 899, 891, 883, 875,
    866
};

//后面需要根据算法提供的参数进行更新
static vo_csc_type_matrix g_type_to_matrix[VO_CSC_MAX] = {
    { VO_CSC_YUV2RGB_601_LIMIT2FULL, &g_csc_yuv601limit_to_rgbfull },
    { VO_CSC_YUV2RGB_709_LIMIT2FULL, &g_csc_yuv709limit_to_rgbfull },
    { VO_CSC_YUV2RGB_601_FULL2LIMIT, &g_csc_yuv601full_to_rgblimit },
    { VO_CSC_YUV2RGB_709_FULL2LIMIT, &g_csc_yuv709full_to_rgblimit },
    { VO_CSC_YUV2RGB_601_FULL2FULL, &g_csc_yuv601full_to_rgbfull },
    { VO_CSC_YUV2RGB_709_FULL2FULL, &g_csc_yuv709full_to_rgbfull },
    { VO_CSC_YUV2RGB_601_LIMIT2LIMIT, &g_csc_yuv601limit_to_rgblimit },
    { VO_CSC_YUV2RGB_709_LIMIT2LIMIT, &g_csc_yuv709limit_to_rgblimit },

    { VO_CSC_YUV2YUV_601_601_LIMIT2LIMIT, &g_csc_yuv601limit_to_yuv601limit },
    { VO_CSC_YUV2YUV_601_709_LIMIT2LIMIT, &g_csc_yuv601limit_to_yuv709limit },
    { VO_CSC_YUV2YUV_709_601_LIMIT2LIMIT, &g_csc_yuv709limit_to_yuv601limit },
    { VO_CSC_YUV2YUV_709_709_LIMIT2LIMIT, &g_csc_yuv709limit_to_yuv709limit },

    { VO_CSC_YUV2YUV_601_601_FULL2FULL, &g_csc_yuv601full_to_yuv601full },
    { VO_CSC_YUV2YUV_601_709_FULL2FULL, &g_csc_yuv601full_to_yuv709full },
    { VO_CSC_YUV2YUV_709_601_FULL2FULL, &g_csc_yuv709full_to_yuv601full },
    { VO_CSC_YUV2YUV_709_709_FULL2FULL, &g_csc_yuv709full_to_yuv709full },

    { VO_CSC_YUV2YUV_601_601_LIMIT2FULL, &g_csc_yuv601limit_to_yuv601full },
    { VO_CSC_YUV2YUV_601_709_LIMIT2FULL, &g_csc_yuv601limit_to_yuv709full },
    { VO_CSC_YUV2YUV_709_601_LIMIT2FULL, &g_csc_yuv709limit_to_yuv601full },
    { VO_CSC_YUV2YUV_709_709_LIMIT2FULL, &g_csc_yuv709limit_to_yuv709full },

    { VO_CSC_YUV2YUV_601_601_FULL2LIMIT, &g_csc_yuv601full_to_yuv601limit },
    { VO_CSC_YUV2YUV_601_709_FULL2LIMIT, &g_csc_yuv601full_to_yuv709limit },
    { VO_CSC_YUV2YUV_709_601_FULL2LIMIT, &g_csc_yuv709full_to_yuv601limit },
    { VO_CSC_YUV2YUV_709_709_FULL2LIMIT, &g_csc_yuv709full_to_yuv709limit },
};

static xmedia_s32 vo_get_base_matrix_by_type(vo_csc_type csc_type, const vo_csc_coef **csc_tmp)
{
    xmedia_u32 loop;
    xmedia_u32 len;

    len = sizeof(g_type_to_matrix) / sizeof(vo_csc_type_matrix);
    for (loop = 0; loop < len; loop++) {
        if (g_type_to_matrix[loop].type == csc_type) {
            if (g_type_to_matrix[loop].matrix != XMEDIA_NULL) {
                *csc_tmp = g_type_to_matrix[loop].matrix;
            } else {
                break;
            }
            return XMEDIA_SUCCESS;
        }
    }
    return XMEDIA_FAILURE;
};

static xmedia_bool vo_check_csc_type(vo_csc_type type)
{
    if ((type >= VO_CSC_YUV2RGB_601_LIMIT2FULL) && (type <= VO_CSC_YUV2RGB_709_LIMIT2LIMIT)) {
        return XMEDIA_TRUE;
    }
    return XMEDIA_FALSE;
}

xmedia_s32 hal_vo_csc_coef_convert(vo_pic_info *src_pic_info, vo_csc_coef *pst_csc_matrix)
{
    xmedia_s32 luma     = 0;
    xmedia_s32 contrast = 0;
    xmedia_s32 hue      = 0;
    xmedia_s32 satu     = 0;
    const vo_csc_coef *base_csc_matrix = XMEDIA_NULL;

    luma = (xmedia_s32)src_pic_info->luma * 64 / 100 - 32;
    contrast = ((xmedia_s32)src_pic_info->contrast - 50) * 2 + 100;
    hue = (xmedia_s32)src_pic_info->hue * 60 / 100;
    satu = ((xmedia_s32)src_pic_info->saturation - 50) * 2 + 100;
    if (hue >= VO_CSC_TABLE_MAX) {
        VO_TRACE(MODULE_DBG_ERR,"hue %d error!\n", hue);
        return XMEDIA_FAILURE;
    }

    if (vo_get_base_matrix_by_type(src_pic_info->csc_type, &base_csc_matrix) != XMEDIA_SUCCESS) {
        VO_TRACE(MODULE_DBG_ERR,"vo_get_base_matrix_by_type error!\n");
        return XMEDIA_FAILURE;
    }

    pst_csc_matrix->csc_offset_in0  = base_csc_matrix->csc_offset_in0;
    pst_csc_matrix->csc_offset_in1  = base_csc_matrix->csc_offset_in1;
    pst_csc_matrix->csc_offset_in2  = base_csc_matrix->csc_offset_in2;
    pst_csc_matrix->csc_offset_out0 = base_csc_matrix->csc_offset_out0;
    pst_csc_matrix->csc_offset_out1 = base_csc_matrix->csc_offset_out1;
    pst_csc_matrix->csc_offset_out2 = base_csc_matrix->csc_offset_out2;

    /* C_ratio的调节范围一般是0～1.99, C_ratio=s32Contrast/100
    *  S的调节范围一般为0~1.99,S=s32Satu/100
    *  色调调节参数的范围一般为-30°~30°,通过查表法求得COS和SIN值并/1000
    */
    if (vo_check_csc_type(src_pic_info->csc_type) == XMEDIA_TRUE) {
        /* 此公式仅用于YUV->RGB转换，RGB->YUV转换不可用此公式 */
        pst_csc_matrix->csc_coef0 = (contrast * base_csc_matrix->csc_coef0) / 100;
        pst_csc_matrix->csc_coef1 = (contrast * satu * ((base_csc_matrix->csc_coef1 * g_cos_table[hue] -
                                base_csc_matrix->csc_coef2 * g_sin_table[hue]) / 1000)) / 10000;
        pst_csc_matrix->csc_coef2 = (contrast * satu * ((base_csc_matrix->csc_coef1 * g_sin_table[hue] +
                                base_csc_matrix->csc_coef2 * g_cos_table[hue]) / 1000)) / 10000;
        pst_csc_matrix->csc_coef3 = (contrast * base_csc_matrix->csc_coef3) / 100;
        pst_csc_matrix->csc_coef4 = (contrast * satu * ((base_csc_matrix->csc_coef4 * g_cos_table[hue] -
                                base_csc_matrix->csc_coef5 * g_sin_table[hue]) / 1000)) / 10000;
        pst_csc_matrix->csc_coef5 = (contrast * satu * ((base_csc_matrix->csc_coef4 * g_sin_table[hue] +
                                base_csc_matrix->csc_coef5 * g_cos_table[hue]) / 1000)) / 10000;
        pst_csc_matrix->csc_coef6 = (contrast * base_csc_matrix->csc_coef6) / 100;
        pst_csc_matrix->csc_coef7 = (contrast * satu * ((base_csc_matrix->csc_coef7 * g_cos_table[hue] -
                                base_csc_matrix->csc_coef8 * g_sin_table[hue]) / 1000)) / 10000;
        pst_csc_matrix->csc_coef8 = (contrast * satu * ((base_csc_matrix->csc_coef7 * g_sin_table[hue] +
                                base_csc_matrix->csc_coef8 * g_cos_table[hue]) / 1000)) / 10000;
        pst_csc_matrix->csc_offset_in0 += (contrast != 0) ? (luma * 100 / contrast) : luma * 100;
    } else {
        /* 此公式仅用于RGB->YUV转换，YUV->RGB转换不可用此公式，
        *  YUV->YUV仅调节图像效果可用此公式，因为常量矩阵为单位矩阵 */
        pst_csc_matrix->csc_coef0 = (contrast * base_csc_matrix->csc_coef0) / 100;
        pst_csc_matrix->csc_coef1 = (contrast * base_csc_matrix->csc_coef1) / 100;
        pst_csc_matrix->csc_coef2 = (contrast * base_csc_matrix->csc_coef2) / 100;
        pst_csc_matrix->csc_coef3 = (contrast * satu * ((base_csc_matrix->csc_coef3 * g_cos_table[hue] +
                                base_csc_matrix->csc_coef3 * g_sin_table[hue]) / 1000)) / 10000;
        pst_csc_matrix->csc_coef4 = (contrast * satu * ((base_csc_matrix->csc_coef4 * g_cos_table[hue] +
                                base_csc_matrix->csc_coef4 * g_sin_table[hue]) / 1000)) / 10000;
        pst_csc_matrix->csc_coef5 = (contrast * satu * ((base_csc_matrix->csc_coef5 * g_cos_table[hue] +
                                base_csc_matrix->csc_coef5 * g_sin_table[hue]) / 1000)) / 10000;
        pst_csc_matrix->csc_coef6 = (contrast * satu * ((base_csc_matrix->csc_coef6 * g_cos_table[hue] -
                                base_csc_matrix->csc_coef6 * g_sin_table[hue]) / 1000)) / 10000;
        pst_csc_matrix->csc_coef7 = (contrast * satu * ((base_csc_matrix->csc_coef7 * g_cos_table[hue] -
                                base_csc_matrix->csc_coef7 * g_sin_table[hue]) / 1000)) / 10000;
        pst_csc_matrix->csc_coef8 = (contrast * satu * ((base_csc_matrix->csc_coef8 * g_cos_table[hue] -
                                base_csc_matrix->csc_coef8 * g_sin_table[hue]) / 1000)) / 10000;
        pst_csc_matrix->csc_offset_out0 += luma;
    }
    return XMEDIA_SUCCESS;
}

