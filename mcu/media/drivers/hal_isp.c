#include "config.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "xmedia_type.h"
#include "xmedia_isp.h"
#include "xmedia_errcode.h"
#include "isp_def.h"
#include "hal_isp.h"
#include "i2c_dev.h"
#include "reserved_mem.h"

typedef union
{
    struct
    {
        unsigned int    reg_blc1_enable                     : 1; /* [0 : 0]  */
        unsigned int    reserved_0                          : 31; /* [31 : 1]  */
    } bits;

    unsigned int    u32;
} u_blc1_ch0_blc1000;

typedef union
{
    struct
    {
        unsigned int    reg_blc1_global_offset_0            : 12; /* [11 : 0]  */
        unsigned int    reserved_1                          : 4; /* [15 : 12]  */
        unsigned int    reg_blc1_global_offset_1            : 12; /* [27 : 16]  */
        unsigned int    reserved_0                          : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_blc1_ch0_blc1001;

typedef union
{
    struct
    {
        unsigned int    reg_blc1_global_offset_2            : 12; /* [11 : 0]  */
        unsigned int    reserved_1                          : 4; /* [15 : 12]  */
        unsigned int    reg_blc1_global_offset_3            : 12; /* [27 : 16]  */
        unsigned int    reserved_0                          : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_blc1_ch0_blc1002;

typedef union
{
    struct
    {
        unsigned int    reg_blc1_enable                     : 1; /* [0 : 0]  */
        unsigned int    reserved_0                          : 31; /* [31 : 1]  */
    } bits;

    unsigned int    u32;
} u_blc1_ch1_blc1000;

typedef union
{
    struct
    {
        unsigned int    reg_blc1_global_offset_0            : 12; /* [11 : 0]  */
        unsigned int    reserved_1                          : 4; /* [15 : 12]  */
        unsigned int    reg_blc1_global_offset_1            : 12; /* [27 : 16]  */
        unsigned int    reserved_0                          : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_blc1_ch1_blc1001;

typedef union
{
    struct
    {
        unsigned int    reg_blc1_global_offset_2            : 12; /* [11 : 0]  */
        unsigned int    reserved_1                          : 4; /* [15 : 12]  */
        unsigned int    reg_blc1_global_offset_3            : 12; /* [27 : 16]  */
        unsigned int    reserved_0                          : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_blc1_ch1_blc1002;

typedef union
{
    struct
    {
        unsigned int    enable                              : 1; /* [0 : 0]  */
        unsigned int    reserved_0                          : 31; /* [31 : 1]  */
    } bits;

    unsigned int    u32;
} u_dgn1_ch0_hreg000;

typedef union
{
    struct
    {
        unsigned int    digital_gain_0                      : 16; /* [15 : 0]  */
        unsigned int    digital_gain_1                      : 16; /* [31 : 16]  */
    } bits;

    unsigned int    u32;
} u_dgn1_ch0_dgn1001;

typedef union
{
    struct
    {
        unsigned int    digital_gain_2                      : 16; /* [15 : 0]  */
        unsigned int    digital_gain_3                      : 16; /* [31 : 16]  */
    } bits;

    unsigned int    u32;
} u_dgn1_ch0_dgn1002;

typedef union
{
    struct
    {
        unsigned int    blc_in_0                            : 12; /* [11 : 0]  */
        unsigned int    reserved_1                          : 4; /* [15 : 12]  */
        unsigned int    blc_in_1                            : 12; /* [27 : 16]  */
        unsigned int    reserved_0                          : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_dgn1_ch0_dgn1003;

typedef union
{
    struct
    {
        unsigned int    blc_in_2                            : 12; /* [11 : 0]  */
        unsigned int    reserved_1                          : 4; /* [15 : 12]  */
        unsigned int    blc_in_3                            : 12; /* [27 : 16]  */
        unsigned int    reserved_0                          : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_dgn1_ch0_dgn1004;

typedef union
{
    struct
    {
        unsigned int    blc_out_0                           : 12; /* [11 : 0]  */
        unsigned int    reserved_1                          : 4; /* [15 : 12]  */
        unsigned int    blc_out_1                           : 12; /* [27 : 16]  */
        unsigned int    reserved_0                          : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_dgn1_ch0_dgn1005;

typedef union
{
    struct
    {
        unsigned int    blc_out_2                           : 12; /* [11 : 0]  */
        unsigned int    reserved_1                          : 4; /* [15 : 12]  */
        unsigned int    blc_out_3                           : 12; /* [27 : 16]  */
        unsigned int    reserved_0                          : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_dgn1_ch0_dgn1006;

typedef union
{
    struct
    {
        unsigned int    enable                              : 1; /* [0 : 0]  */
        unsigned int    reserved_0                          : 31; /* [31 : 1]  */
    } bits;

    unsigned int    u32;
} u_dgn1_ch1_hreg000;

typedef union
{
    struct
    {
        unsigned int    digital_gain_0                      : 16; /* [15 : 0]  */
        unsigned int    digital_gain_1                      : 16; /* [31 : 16]  */
    } bits;

    unsigned int    u32;
} u_dgn1_ch1_dgn1001;

typedef union
{
    struct
    {
        unsigned int    digital_gain_2                      : 16; /* [15 : 0]  */
        unsigned int    digital_gain_3                      : 16; /* [31 : 16]  */
    } bits;

    unsigned int    u32;
} u_dgn1_ch1_dgn1002;

typedef union
{
    struct
    {
        unsigned int    blc_in_0                            : 12; /* [11 : 0]  */
        unsigned int    reserved_1                          : 4; /* [15 : 12]  */
        unsigned int    blc_in_1                            : 12; /* [27 : 16]  */
        unsigned int    reserved_0                          : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_dgn1_ch1_dgn1003;

typedef union
{
    struct
    {
        unsigned int    blc_in_2                            : 12; /* [11 : 0]  */
        unsigned int    reserved_1                          : 4; /* [15 : 12]  */
        unsigned int    blc_in_3                            : 12; /* [27 : 16]  */
        unsigned int    reserved_0                          : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_dgn1_ch1_dgn1004;

typedef union
{
    struct
    {
        unsigned int    blc_out_0                           : 12; /* [11 : 0]  */
        unsigned int    reserved_1                          : 4; /* [15 : 12]  */
        unsigned int    blc_out_1                           : 12; /* [27 : 16]  */
        unsigned int    reserved_0                          : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_dgn1_ch1_dgn1005;

typedef union
{
    struct
    {
        unsigned int    blc_out_2                           : 12; /* [11 : 0]  */
        unsigned int    reserved_1                          : 4; /* [15 : 12]  */
        unsigned int    blc_out_3                           : 12; /* [27 : 16]  */
        unsigned int    reserved_0                          : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_dgn1_ch1_dgn1006;

typedef union
{
    struct
    {
        unsigned int    reg_aac_initial_sram_enable         : 1; /* [0 : 0]  */
        unsigned int    reserved_3                          : 3; /* [3 : 1]  */
        unsigned int    reg_aac_ae_cfa_enable               : 1; /* [4 : 4]  */
        unsigned int    reserved_2                          : 11; /* [15 : 5]  */
        unsigned int    reg_aac_ae_cfa_tile_num_col         : 5; /* [20 : 16]  */
        unsigned int    reserved_1                          : 3; /* [23 : 21]  */
        unsigned int    reg_aac_ae_cfa_tile_num_row         : 4; /* [27 : 24]  */
        unsigned int    reserved_0                          : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_aac_be000;

typedef union
{
    struct
    {
        unsigned int    reg_aac_ae_cfa_tile_col_start       : 12; /* [11 : 00]  */
        unsigned int    reserved_1                          : 4; /* [15 : 12]  */
        unsigned int    reg_aac_ae_cfa_tile_row_start       : 12; /* [27 : 16]  */
        unsigned int    reserved_0                          : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_aac_be001;

typedef union
{
    struct
    {
        unsigned int    reg_aac_ae_cfa_tile_num_all         : 16; /* [15 : 00]  */
        unsigned int    reserved_0                          : 16; /* [31 : 16]  */
    } bits;

    unsigned int    u32;
} u_aac_be002;

typedef union
{
    struct
    {
        unsigned int    reg_aac_ae_cfa_tile_width           : 9; /* [08 : 00]  */
        unsigned int    reserved_2                          : 7; /* [15 : 9]  */
        unsigned int    reg_aac_ae_cfa_tile_height          : 9; /* [24 : 16]  */
        unsigned int    reserved_1                          : 3; /* [27 : 25]  */
        unsigned int    reg_aac_ae_stat_location            : 2; /* [29 : 28]  */
        unsigned int    reserved_0                          : 2; /* [31 : 30]  */
    } bits;

    unsigned int    u32;
} u_aac_be003;

typedef union
{
    struct
    {
        unsigned int    reg_aac_hist_enable                 : 1; /* [00 : 00]  */
        unsigned int    reg_aac_hist_area_mode              : 1; /* [01 : 01]  */
        unsigned int    reg_aac_hist_fourmode               : 2; /* [03 : 02]  */
        unsigned int    reg_aac_hist_single_color_bin       : 2; /* [05 : 04]  */
        unsigned int    reg_aac_hist_sqrtmode               : 1; /* [06 : 06]  */
        unsigned int    reg_aac_ae_cfa_tile_sqrtmode        : 1; /* [07 : 07]  */
        unsigned int    reg_aac_ae_cfa_tile_area            : 20; /* [27 : 08]  */
        unsigned int    reserved_0                          : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_aac_be004;

typedef union
{
    struct
    {
        unsigned int    reg_aac_af_enable                   : 1; /* [00 : 00]  */
        unsigned int    reserved_2                          : 15; /* [15 : 1]  */
        unsigned int    reg_aac_af_tile_num_col             : 5; /* [20 : 16]  */
        unsigned int    reserved_1                          : 3; /* [23 : 21]  */
        unsigned int    reg_aac_af_tile_num_row             : 4; /* [27 : 24]  */
        unsigned int    reserved_0                          : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_aac_be005;

typedef union
{
    struct
    {
        unsigned int    reg_aac_af_tile_col_start           : 12; /* [11 : 00]  */
        unsigned int    reserved_1                          : 4; /* [15 : 12]  */
        unsigned int    reg_aac_af_tile_row_start           : 12; /* [27 : 16]  */
        unsigned int    reserved_0                          : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_aac_be006;

typedef union
{
    struct
    {
        unsigned int    reg_aac_af_tile_width               : 9; /* [08 : 00]  */
        unsigned int    reserved_2                          : 7; /* [15 : 9]  */
        unsigned int    reg_aac_af_tile_height              : 9; /* [24 : 16]  */
        unsigned int    reserved_1                          : 3; /* [27 : 25]  */
        unsigned int    reg_aac_af_stat_location            : 2; /* [29 : 28]  */
        unsigned int    reserved_0                          : 2; /* [31 : 30]  */
    } bits;

    unsigned int    u32;
} u_aac_be007;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt1_hor_mode               : 1; /* [00 : 00]  */
        unsigned int    reg_aac_flt1_hor_stage_en_0         : 1; /* [01 : 01]  */
        unsigned int    reg_aac_flt1_hor_stage_en_1         : 1; /* [02 : 02]  */
        unsigned int    reg_aac_flt1_hor_stage_en_2         : 1; /* [03 : 03]  */
        unsigned int    reg_aac_flt2_hor_mode               : 1; /* [04 : 04]  */
        unsigned int    reg_aac_flt2_hor_stage_en_0         : 1; /* [05 : 05]  */
        unsigned int    reg_aac_flt2_hor_stage_en_1         : 1; /* [06 : 06]  */
        unsigned int    reg_aac_flt2_hor_stage_en_2         : 1; /* [07 : 07]  */
        unsigned int    reg_aac_af_hlt_thresh               : 10; /* [17 : 08]  */
        unsigned int    reserved_0                          : 14; /* [31 : 18]  */
    } bits;

    unsigned int    u32;
} u_aac_be008;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt1_hor_gain_0             : 10; /* [09 : 00]  */
        unsigned int    reserved_1                          : 6; /* [15 : 10]  */
        unsigned int    reg_aac_flt1_hor_gain_1             : 10; /* [25 : 16]  */
        unsigned int    reserved_0                          : 6; /* [31 : 26]  */
    } bits;

    unsigned int    u32;
} u_aac_be009;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt1_hor_gain_2             : 10; /* [09 : 00]  */
        unsigned int    reserved_1                          : 6; /* [15 : 10]  */
        unsigned int    reg_aac_flt1_hor_gain_3             : 10; /* [25 : 16]  */
        unsigned int    reserved_0                          : 6; /* [31 : 26]  */
    } bits;

    unsigned int    u32;
} u_aac_be010;

typedef union
{
    struct
    {
        unsigned int    reg_aac_af_pre_filter_enable        : 1; /* [00 : 00]  */
        unsigned int    reserved_7                          : 3; /* [03 : 01]  */
        unsigned int    reg_aac_flt1_hor_forward_coeff_0    : 3; /* [06 : 04]  */
        unsigned int    reserved_6                          : 1; /* [07 : 07]  */
        unsigned int    reg_aac_flt1_hor_forward_coeff_1    : 3; /* [10 : 08]  */
        unsigned int    reserved_5                          : 1; /* [11 : 11]  */
        unsigned int    reg_aac_flt1_hor_forward_coeff_2    : 3; /* [14 : 12]  */
        unsigned int    reserved_4                          : 1; /* [15 : 15]  */
        unsigned int    reg_aac_flt1_hor_forward_coeff_3    : 3; /* [18 : 16]  */
        unsigned int    reserved_3                          : 1; /* [19 : 19]  */
        unsigned int    reg_aac_flt1_hor_forward_coeff_4    : 3; /* [22 : 20]  */
        unsigned int    reserved_2                          : 1; /* [23 : 23]  */
        unsigned int    reg_aac_flt1_hor_forward_coeff_5    : 3; /* [26 : 24]  */
        unsigned int    reserved_1                          : 1; /* [27 : 27]  */
        unsigned int    reg_aac_flt1_hor_forward_coeff_6    : 3; /* [30 : 28]  */
        unsigned int    reserved_0                          : 1; /* [31 : 31]  */
    } bits;

    unsigned int    u32;
} u_aac_be011;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt1_hor_forward_coeff_7    : 3; /* [02 : 00]  */
        unsigned int    reserved_1                          : 1; /* [03 : 03]  */
        unsigned int    reg_aac_flt1_hor_forward_coeff_8    : 3; /* [06 : 04]  */
        unsigned int    reserved_0                          : 25; /* [31 : 07]  */
    } bits;

    unsigned int    u32;
} u_aac_be012;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt1_hor_backward_coeff_0   : 10; /* [09 : 00]  */
        unsigned int    reserved_1                          : 2; /* [11 : 10]  */
        unsigned int    reg_aac_flt1_hor_backward_coeff_1   : 10; /* [21 : 12]  */
        unsigned int    reserved_0                          : 10; /* [31 : 22]  */
    } bits;

    unsigned int    u32;
} u_aac_be013;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt1_hor_backward_coeff_2   : 10; /* [09 : 00]  */
        unsigned int    reserved_1                          : 2; /* [11 : 10]  */
        unsigned int    reg_aac_flt1_hor_backward_coeff_3   : 10; /* [21 : 12]  */
        unsigned int    reserved_0                          : 10; /* [31 : 22]  */
    } bits;

    unsigned int    u32;
} u_aac_be014;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt1_hor_backward_coeff_4   : 10; /* [09 : 00]  */
        unsigned int    reserved_1                          : 2; /* [11 : 10]  */
        unsigned int    reg_aac_flt1_hor_backward_coeff_5   : 10; /* [21 : 12]  */
        unsigned int    reserved_0                          : 10; /* [31 : 22]  */
    } bits;

    unsigned int    u32;
} u_aac_be015;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt1_hor_coring_en          : 1; /* [00 : 00]  */
        unsigned int    reserved_0                          : 3; /* [03 : 01]  */
        unsigned int    reg_aac_flt1_hor_coring_th          : 12; /* [15 : 04]  */
        unsigned int    reg_aac_flt1_hor_coring_slp         : 4; /* [19 : 16]  */
        unsigned int    reg_aac_flt1_hor_coring_lmt         : 12; /* [31 : 20]  */
    } bits;

    unsigned int    u32;
} u_aac_be016;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt1_hor_bias_offset        : 15; /* [14 : 00]  */
        unsigned int    reserved_1                          : 1; /* [15 : 15]  */
        unsigned int    reg_aac_flt1_hor_thresh             : 14; /* [29 : 16]  */
        unsigned int    reserved_0                          : 2; /* [31 : 30]  */
    } bits;

    unsigned int    u32;
} u_aac_be017;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt2_hor_gain_0             : 10; /* [09 : 00]  */
        unsigned int    reserved_1                          : 6; /* [15 : 10]  */
        unsigned int    reg_aac_flt2_hor_gain_1             : 10; /* [25 : 16]  */
        unsigned int    reserved_0                          : 6; /* [31 : 26]  */
    } bits;

    unsigned int    u32;
} u_aac_be018;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt2_hor_gain_2             : 10; /* [09 : 00]  */
        unsigned int    reserved_1                          : 6; /* [15 : 10]  */
        unsigned int    reg_aac_flt2_hor_gain_3             : 10; /* [25 : 16]  */
        unsigned int    reserved_0                          : 6; /* [31 : 26]  */
    } bits;

    unsigned int    u32;
} u_aac_be019;

typedef union
{
    struct
    {
        unsigned int    reserved_7                          : 4; /* [03 : 00]  */
        unsigned int    reg_aac_flt2_hor_forward_coeff_0    : 3; /* [06 : 04]  */
        unsigned int    reserved_6                          : 1; /* [07 : 07]  */
        unsigned int    reg_aac_flt2_hor_forward_coeff_1    : 3; /* [10 : 08]  */
        unsigned int    reserved_5                          : 1; /* [11 : 11]  */
        unsigned int    reg_aac_flt2_hor_forward_coeff_2    : 3; /* [14 : 12]  */
        unsigned int    reserved_4                          : 1; /* [15 : 15]  */
        unsigned int    reg_aac_flt2_hor_forward_coeff_3    : 3; /* [18 : 16]  */
        unsigned int    reserved_3                          : 1; /* [19 : 19]  */
        unsigned int    reg_aac_flt2_hor_forward_coeff_4    : 3; /* [22 : 20]  */
        unsigned int    reserved_2                          : 1; /* [23 : 23]  */
        unsigned int    reg_aac_flt2_hor_forward_coeff_5    : 3; /* [26 : 24]  */
        unsigned int    reserved_1                          : 1; /* [27 : 27]  */
        unsigned int    reg_aac_flt2_hor_forward_coeff_6    : 3; /* [30 : 28]  */
        unsigned int    reserved_0                          : 1; /* [31 : 31]  */
    } bits;

    unsigned int    u32;
} u_aac_be020;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt2_hor_forward_coeff_7    : 3; /* [02 : 00]  */
        unsigned int    reserved_1                          : 1; /* [03 : 03]  */
        unsigned int    reg_aac_flt2_hor_forward_coeff_8    : 3; /* [06 : 04]  */
        unsigned int    reserved_0                          : 25; /* [31 : 07]  */
    } bits;

    unsigned int    u32;
} u_aac_be021;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt2_hor_backward_coeff_0   : 10; /* [09 : 00]  */
        unsigned int    reserved_1                          : 2; /* [11 : 10]  */
        unsigned int    reg_aac_flt2_hor_backward_coeff_1   : 10; /* [21 : 12]  */
        unsigned int    reserved_0                          : 10; /* [31 : 22]  */
    } bits;

    unsigned int    u32;
} u_aac_be022;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt2_hor_backward_coeff_2   : 10; /* [09 : 00]  */
        unsigned int    reserved_1                          : 2; /* [11 : 10]  */
        unsigned int    reg_aac_flt2_hor_backward_coeff_3   : 10; /* [21 : 12]  */
        unsigned int    reserved_0                          : 10; /* [31 : 22]  */
    } bits;

    unsigned int    u32;
} u_aac_be023;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt2_hor_backward_coeff_4   : 10; /* [09 : 00]  */
        unsigned int    reserved_1                          : 2; /* [11 : 10]  */
        unsigned int    reg_aac_flt2_hor_backward_coeff_5   : 10; /* [21 : 12]  */
        unsigned int    reserved_0                          : 10; /* [31 : 22]  */
    } bits;

    unsigned int    u32;
} u_aac_be024;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt2_hor_coring_en          : 1; /* [00 : 00]  */
        unsigned int    reserved_0                          : 3; /* [03 : 01]  */
        unsigned int    reg_aac_flt2_hor_coring_th          : 12; /* [15 : 04]  */
        unsigned int    reg_aac_flt2_hor_coring_slp         : 4; /* [19 : 16]  */
        unsigned int    reg_aac_flt2_hor_coring_lmt         : 12; /* [31 : 20]  */
    } bits;

    unsigned int    u32;
} u_aac_be025;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt2_hor_bias_offset        : 15; /* [14 : 00]  */
        unsigned int    reserved_1                          : 1; /* [15 : 15]  */
        unsigned int    reg_aac_flt2_hor_thresh             : 14; /* [29 : 16]  */
        unsigned int    reserved_0                          : 2; /* [31 : 30]  */
    } bits;

    unsigned int    u32;
} u_aac_be026;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt1_ver_gain_0             : 6; /* [05 : 00]  */
        unsigned int    reserved_2                          : 2; /* [07 : 06]  */
        unsigned int    reg_aac_flt1_ver_gain_1             : 6; /* [13 : 08]  */
        unsigned int    reserved_1                          : 2; /* [15 : 14]  */
        unsigned int    reg_aac_flt1_ver_gain_2             : 6; /* [21 : 16]  */
        unsigned int    reserved_0                          : 10; /* [31 : 22]  */
    } bits;

    unsigned int    u32;
} u_aac_be027;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt2_ver_gain_0             : 6; /* [05 : 00]  */
        unsigned int    reserved_2                          : 2; /* [07 : 06]  */
        unsigned int    reg_aac_flt2_ver_gain_1             : 6; /* [13 : 08]  */
        unsigned int    reserved_1                          : 2; /* [15 : 14]  */
        unsigned int    reg_aac_flt2_ver_gain_2             : 6; /* [21 : 16]  */
        unsigned int    reserved_0                          : 10; /* [31 : 22]  */
    } bits;

    unsigned int    u32;
} u_aac_be028;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt1_ver_coring_en          : 1; /* [00 : 00]  */
        unsigned int    reserved_0                          : 3; /* [03 : 01]  */
        unsigned int    reg_aac_flt1_ver_coring_th          : 12; /* [15 : 04]  */
        unsigned int    reg_aac_flt1_ver_coring_slp         : 4; /* [19 : 16]  */
        unsigned int    reg_aac_flt1_ver_coring_lmt         : 12; /* [31 : 20]  */
    } bits;

    unsigned int    u32;
} u_aac_be029;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt2_ver_coring_en          : 1; /* [00 : 00]  */
        unsigned int    reserved_0                          : 3; /* [03 : 01]  */
        unsigned int    reg_aac_flt2_ver_coring_th          : 12; /* [15 : 04]  */
        unsigned int    reg_aac_flt2_ver_coring_slp         : 4; /* [19 : 16]  */
        unsigned int    reg_aac_flt2_ver_coring_lmt         : 12; /* [31 : 20]  */
    } bits;

    unsigned int    u32;
} u_aac_be030;

typedef union
{
    struct
    {
        unsigned int    reg_aac_fv1_hor_weight              : 5; /* [04 : 00]  */
        unsigned int    reserved_3                          : 3; /* [7 : 5]  */
        unsigned int    reg_aac_fv2_hor_weight              : 5; /* [12 : 08]  */
        unsigned int    reserved_2                          : 3; /* [15 : 13]  */
        unsigned int    reg_aac_fv1_ver_weight              : 5; /* [20 : 16]  */
        unsigned int    reserved_1                          : 3; /* [23 : 21]  */
        unsigned int    reg_aac_fv2_ver_weight              : 5; /* [28 : 24]  */
        unsigned int    reserved_0                          : 3; /* [31 : 29]  */
    } bits;

    unsigned int    u32;
} u_aac_be031;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt1_ver_thresh             : 14; /* [13 : 0]  */
        unsigned int    reserved_1                          : 2; /* [15 : 14]  */
        unsigned int    reg_aac_flt2_ver_thresh             : 14; /* [29 : 16]  */
        unsigned int    reserved_0                          : 2; /* [31 : 30]  */
    } bits;

    unsigned int    u32;
} u_aac_be032;

typedef union
{
    struct
    {
        unsigned int    reg_aac_awb_cfa_enable              : 1; /* [00 : 00]  */
        unsigned int    reserved_3                          : 3; /* [03 : 01]  */
        unsigned int    reg_aac_awb_stat_location           : 2; /* [05 : 04]  */
        unsigned int    reserved_2                          : 10; /* [15 : 06]  */
        unsigned int    reg_aac_awb_cfa_tile_num_col        : 5; /* [20 : 16]  */
        unsigned int    reserved_1                          : 3; /* [23 : 21]  */
        unsigned int    reg_aac_awb_cfa_tile_num_row        : 5; /* [28 : 24]  */
        unsigned int    reserved_0                          : 3; /* [31 : 29]  */
    } bits;

    unsigned int    u32;
} u_aac_be033;

typedef union
{
    struct
    {
        unsigned int    reg_aac_awb_cfa_tile_col_start      : 12; /* [11 : 00]  */
        unsigned int    reserved_1                          : 4; /* [15 : 12]  */
        unsigned int    reg_aac_awb_cfa_tile_row_start      : 12; /* [27 : 16]  */
        unsigned int    reserved_0                          : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_aac_be034;

typedef union
{
    struct
    {
        unsigned int    reg_aac_awb_cfa_tile_min_thresh     : 16; /* [15 : 00]  */
        unsigned int    reg_aac_awb_cfa_tile_max_thresh     : 16; /* [31 : 16]  */
    } bits;

    unsigned int    u32;
} u_aac_be035;

typedef union
{
    struct
    {
        unsigned int    reg_aac_awb_cfa_tile_width          : 8; /* [07 : 00]  */
        unsigned int    reg_aac_awb_cfa_tile_height         : 8; /* [15 : 08]  */
        unsigned int    reg_aac_awb_cfa_tile_num_all        : 14; /* [29 : 16]  */
        unsigned int    reserved_0                          : 2; /* [31 : 30]  */
    } bits;

    unsigned int    u32;
} u_aac_be036;

typedef union
{
    struct
    {
        unsigned int    reg_aac_awb_cfa_tile_max_r_g        : 12; /* [11 : 00]  */
        unsigned int    reserved_1                          : 4; /* [15 : 12]  */
        unsigned int    reg_aac_awb_cfa_tile_min_r_g        : 12; /* [27 : 16]  */
        unsigned int    reserved_0                          : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_aac_be037;

typedef union
{
    struct
    {
        unsigned int    reg_aac_awb_cfa_tile_max_b_g        : 12; /* [11 : 00]  */
        unsigned int    reserved_1                          : 4; /* [15 : 12]  */
        unsigned int    reg_aac_awb_cfa_tile_min_b_g        : 12; /* [27 : 16]  */
        unsigned int    reserved_0                          : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_aac_be038;

typedef union
{
    struct
    {
        unsigned int    reg_aac_awb_cfa_pseudo_y_wgt_0      : 4; /* [03 : 00]  */
        unsigned int    reserved_3                          : 4; /* [7 : 4]  */
        unsigned int    reg_aac_awb_cfa_pseudo_y_wgt_1      : 4; /* [11 : 08]  */
        unsigned int    reserved_2                          : 4; /* [15 : 12]  */
        unsigned int    reg_aac_awb_cfa_pseudo_y_wgt_2      : 4; /* [19 : 16]  */
        unsigned int    reserved_1                          : 4; /* [23 : 20]  */
        unsigned int    reg_aac_awb_cfa_pseudo_y_wgt_3      : 4; /* [27 : 24]  */
        unsigned int    reserved_0                          : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_aac_be039;

typedef union
{
    struct
    {
        unsigned int    reg_aac_ae_cfa_tile_domain_wt_0     : 4; /* [3 : 0]  */
        unsigned int    reg_aac_ae_cfa_tile_domain_wt_1     : 4; /* [7 : 4]  */
        unsigned int    reg_aac_ae_cfa_tile_domain_wt_2     : 4; /* [11 : 8]  */
        unsigned int    reg_aac_ae_cfa_tile_domain_wt_3     : 4; /* [15 : 12]  */
        unsigned int    reg_aac_ae_cfa_tile_domain_wt_4     : 4; /* [19 : 16]  */
        unsigned int    reg_aac_ae_cfa_tile_domain_wt_5     : 4; /* [23 : 20]  */
        unsigned int    reg_aac_ae_cfa_tile_domain_wt_6     : 4; /* [27 : 24]  */
        unsigned int    reg_aac_ae_cfa_tile_domain_wt_7     : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_aac_be_reg_aac_ae_cfa_tile_domain_wt;

typedef union
{
    struct
    {
        unsigned int    reg_aac_ae_cfa_tile_domain_wt_248   : 4; /* [3 : 0]  */
        unsigned int    reg_aac_ae_cfa_tile_domain_wt_249   : 4; /* [7 : 4]  */
        unsigned int    reg_aac_ae_cfa_tile_domain_wt_250   : 4; /* [11 : 8]  */
        unsigned int    reg_aac_ae_cfa_tile_domain_wt_251   : 4; /* [15 : 12]  */
        unsigned int    reg_aac_ae_cfa_tile_domain_wt_252   : 4; /* [19 : 16]  */
        unsigned int    reg_aac_ae_cfa_tile_domain_wt_253   : 4; /* [23 : 20]  */
        unsigned int    reg_aac_ae_cfa_tile_domain_wt_254   : 4; /* [27 : 24]  */
        unsigned int    reserved_0                          : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_aac_be_reg_aac_ae_cfa_tile_domain_wt_254;

typedef union
{
    struct
    {
        unsigned int    reg_aac_af_tone_curve_lut_0         : 10; /* [09 : 00]  */
        unsigned int    reserved_1                          : 6; /* [15 : 10]  */
        unsigned int    reg_aac_af_tone_curve_lut_1         : 10; /* [25 : 16]  */
        unsigned int    reserved_0                          : 6; /* [31 : 26]  */
    } bits;

    unsigned int    u32;
} u_aac_be_reg_aac_af_tone_curve_lut;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt1_hor_gain_curve_0       : 8; /* [07 : 00]  */
        unsigned int    reg_aac_flt1_hor_gain_curve_1       : 8; /* [15 : 08]  */
        unsigned int    reg_aac_flt1_hor_gain_curve_2       : 8; /* [23 : 16]  */
        unsigned int    reg_aac_flt1_hor_gain_curve_3       : 8; /* [31 : 24]  */
    } bits;

    unsigned int    u32;
} u_aac_be_reg_aac_flt1_hor_gain_curve;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt1_ver_gain_curve_0       : 8; /* [07 : 00]  */
        unsigned int    reg_aac_flt1_ver_gain_curve_1       : 8; /* [15 : 08]  */
        unsigned int    reg_aac_flt1_ver_gain_curve_2       : 8; /* [23 : 16]  */
        unsigned int    reg_aac_flt1_ver_gain_curve_3       : 8; /* [31 : 24]  */
    } bits;

    unsigned int    u32;
} u_aac_be_reg_aac_flt1_ver_gain_curve;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt2_hor_gain_curve_0       : 8; /* [07 : 00]  */
        unsigned int    reg_aac_flt2_hor_gain_curve_1       : 8; /* [15 : 08]  */
        unsigned int    reg_aac_flt2_hor_gain_curve_2       : 8; /* [23 : 16]  */
        unsigned int    reg_aac_flt2_hor_gain_curve_3       : 8; /* [31 : 24]  */
    } bits;

    unsigned int    u32;
} u_aac_be_reg_aac_flt2_hor_gain_curve;

typedef union
{
    struct
    {
        unsigned int    reg_aac_flt2_ver_gain_curve_0       : 8; /* [07 : 00]  */
        unsigned int    reg_aac_flt2_ver_gain_curve_1       : 8; /* [15 : 08]  */
        unsigned int    reg_aac_flt2_ver_gain_curve_2       : 8; /* [23 : 16]  */
        unsigned int    reg_aac_flt2_ver_gain_curve_3       : 8; /* [31 : 24]  */
    } bits;

    unsigned int    u32;
} u_aac_be_reg_aac_flt2_ver_gain_curve;

typedef union
{
    struct
    {
        unsigned int    reg_blc2_enable                     : 1; /* [0 : 0]  */
        unsigned int    reserved_0                          : 31; /* [31 : 1]  */
    } bits;

    unsigned int    u32;
} u_blc2000;

typedef union
{
    struct
    {
        unsigned int    reg_blc2_global_offset_0            : 12; /* [11 : 0]  */
        unsigned int    reserved_1                          : 4; /* [15 : 12]  */
        unsigned int    reg_blc2_global_offset_1            : 12; /* [27 : 16]  */
        unsigned int    reserved_0                          : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_blc2001;

typedef union
{
    struct
    {
        unsigned int    reg_blc2_global_offset_2            : 12; /* [11 : 0]  */
        unsigned int    reserved_1                          : 4; /* [15 : 12]  */
        unsigned int    reg_blc2_global_offset_3            : 12; /* [27 : 16]  */
        unsigned int    reserved_0                          : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_blc2002;

typedef union
{
    struct
    {
        unsigned int    reg_dgn2_enable                     : 1; /* [0 : 0]  */
        unsigned int    reserved_0                          : 15; /* [15 : 1]  */
        unsigned int    reg_dgn2_multiplier_single          : 16; /* [31 : 16]  */
    } bits;

    unsigned int    u32;
} u_dgn2_hreg000;

typedef union
{
    struct
    {
        unsigned int    reg_dgn3_enable                     : 1; /* [0 : 0]  */
        unsigned int    reserved_0                          : 31; /* [31 : 1]  */
    } bits;

    unsigned int    u32;
} u_dgn3_hreg000;

typedef union
{
    struct
    {
        unsigned int    reg_dgn3_multiplier_4ch_0           : 12; /* [11 : 0]  */
        unsigned int    reserved_1                          : 4; /* [15 : 12]  */
        unsigned int    reg_dgn3_multiplier_4ch_1           : 12; /* [27 : 16]  */
        unsigned int    reserved_0                          : 4; /* [31 : 28]  */
    } bits;

    unsigned int    u32;
} u_dgn3_reg_dgn3_multiplier_4ch;

typedef union
{
    struct
    {
        unsigned int    reg_cds422_enable                   : 1; /* [0 : 0]  */
        unsigned int    reserved_1                          : 1; /* [1 : 1]  */
        unsigned int    reg_cds422_param_0                  : 7; /* [8 : 2]  */
        unsigned int    reg_cds422_param_1                  : 7; /* [15 : 9]  */
        unsigned int    reserved_0                          : 16; /* [31 : 16]  */
    } bits;

    unsigned int    u32;
} u_cds_h000;

typedef union
{
    struct
    {
        unsigned int    reg_cds_enable                      : 1; /* [0 : 0]  */
        unsigned int    reg_cds_type                        : 1; /* [1 : 1]  */
        unsigned int    reg_cds_param_0                     : 7; /* [8 : 2]  */
        unsigned int    reg_cds_param_1                     : 7; /* [15 : 9]  */
        unsigned int    reserved_0                          : 16; /* [31 : 16]  */
    } bits;

    unsigned int    u32;
} u_cds_v000;

typedef struct {
    volatile u_blc1_ch0_blc1000                   blc1_ch0_blc1000              ;/*0x420*/
    volatile u_blc1_ch0_blc1001                   blc1_ch0_blc1001              ;/*0x424*/
    volatile u_blc1_ch0_blc1002                   blc1_ch0_blc1002              ;/*0x428*/
} u_blc1_ch0_regs;
#define ISP_BE_BLC1_CH0_NOR_REGS_NUM 3

typedef struct {
    volatile u_blc1_ch1_blc1000                   blc1_ch1_blc1000              ;/*0x440*/
    volatile u_blc1_ch1_blc1001                   blc1_ch1_blc1001              ;/*0x444*/
    volatile u_blc1_ch1_blc1002                   blc1_ch1_blc1002              ;/*0x448*/
} u_blc1_ch1_regs;
#define ISP_BE_BLC1_CH1_NOR_REGS_NUM 3

typedef struct {
    volatile u_dgn1_ch0_hreg000                   dgn1_ch0_hreg000              ;/*0x460*/
    volatile u_dgn1_ch0_dgn1001                   dgn1_ch0_digital_gain[2]      ;/*0x464*/
    volatile u_dgn1_ch0_dgn1003                   dgn1_ch0_blc_in[2]            ;/*0x46c*/
    volatile u_dgn1_ch0_dgn1005                   dgn1_ch0_blc_out[2]           ;/*0x474*/
} u_dgn1_ch0_regs;
#define ISP_BE_DGN1_CH0_NOR_REGS_NUM 7

typedef struct {
    volatile u_dgn1_ch1_hreg000                   dgn1_ch1_hreg000              ;/*0x480*/
    volatile u_dgn1_ch1_dgn1001                   dgn1_ch1_digital_gain[2]      ;/*0x484*/
    volatile u_dgn1_ch1_dgn1003                   dgn1_ch1_blc_in[2]            ;/*0x48c*/
    volatile u_dgn1_ch1_dgn1005                   dgn1_ch1_blc_out[2]           ;/*0x494*/
} u_dgn1_ch1_regs;
#define ISP_BE_DGN1_CH1_NOR_REGS_NUM 7

typedef struct {
    volatile u_aac_be000                          aac_be000                     ;/*0x4c0*/
    volatile u_aac_be001                          aac_be001                     ;/*0x4c4*/
    volatile u_aac_be002                          aac_be002                     ;/*0x4c8*/
    volatile u_aac_be003                          aac_be003                     ;/*0x4cc*/
    volatile u_aac_be004                          aac_be004                     ;/*0x4d0*/
    volatile u_aac_be005                          aac_be005                     ;/*0x4d4*/
    // volatile u_aac_be006                          aac_be006                     ;/*0x4d8*/
    // volatile u_aac_be007                          aac_be007                     ;/*0x4dc*/
    // volatile u_aac_be008                          aac_be008                     ;/*0x4e0*/
    // volatile u_aac_be009                          flt1_hor_gain[2]              ;/*0x4e4*/
    // volatile u_aac_be011                          aac_be011                     ;/*0x4ec*/
    // volatile u_aac_be012                          aac_be012                     ;/*0x4f0*/
    // volatile u_aac_be013                          flt1_hor_backward_coeff[3]    ;/*0x4f4*/
    // volatile u_aac_be016                          aac_be016                     ;/*0x500*/
    // volatile u_aac_be017                          aac_be017                     ;/*0x504*/
    // volatile u_aac_be018                          flt2_hor_gain[2]              ;/*0x508*/
    // volatile u_aac_be020                          aac_be020                     ;/*0x510*/
    // volatile u_aac_be021                          aac_be021                     ;/*0x514*/
    // volatile u_aac_be022                          flt2_hor_backward_coeff[3]    ;/*0x518*/
    // volatile u_aac_be025                          aac_be025                     ;/*0x524*/
    // volatile u_aac_be026                          aac_be026                     ;/*0x528*/
    // volatile u_aac_be027                          aac_be027                     ;/*0x52c*/
    // volatile u_aac_be028                          aac_be028                     ;/*0x530*/
    // volatile u_aac_be029                          aac_be029                     ;/*0x534*/
    // volatile u_aac_be030                          aac_be030                     ;/*0x538*/
    // volatile u_aac_be031                          aac_be031                     ;/*0x53c*/
    // volatile u_aac_be032                          aac_be032                     ;/*0x540*/
    volatile u_aac_be033                          aac_be033                     ;/*0x544*/
    volatile u_aac_be034                          aac_be034                     ;/*0x548*/
    volatile u_aac_be035                          aac_be035                     ;/*0x54c*/
    volatile u_aac_be036                          aac_be036                     ;/*0x550*/
    volatile u_aac_be037                          aac_be037                     ;/*0x554*/
    volatile u_aac_be038                          aac_be038                     ;/*0x558*/
    volatile u_aac_be039                          aac_be039                     ;/*0x55c*/
    volatile u_aac_be_reg_aac_ae_cfa_tile_domain_wt
                                                  aac_be_reg_aac_ae_cfa_tile_domain_wt[32];/*0x560*/
    // volatile u_aac_be_reg_aac_af_tone_curve_lut   aac_be_reg_aac_af_tone_curve_lut[16];/*0x5e0*/
    // volatile u_aac_be_reg_aac_flt1_hor_gain_curve aac_be_reg_aac_flt1_hor_gain_curve[4];/*0x620*/
    // volatile u_aac_be_reg_aac_flt1_ver_gain_curve aac_be_reg_aac_flt1_ver_gain_curve[4];/*0x630*/
    // volatile u_aac_be_reg_aac_flt2_hor_gain_curve aac_be_reg_aac_flt2_hor_gain_curve[4];/*0x640*/
    // volatile u_aac_be_reg_aac_flt2_ver_gain_curve aac_be_reg_aac_flt2_ver_gain_curve[4];/*0x650*/
} u_aac_be_regs;
#define ISP_BE_AAC_BE_NOR_REGS_NUM 40
#define ISP_BE_AAC_LUT_REGS_NUM ISP_BE_REGS_AAC_AE_WEIGHT_LUT_LENGTH

typedef struct {
    volatile u_blc2000                            blc2000                       ;/*0x1420*/
    volatile u_blc2001                            blc2_global_offset[2]         ;/*0x1424*/
} u_blc2_regs;
#define ISP_BE_BLC2_NOR_REGS_NUM 3

typedef struct {
    volatile u_dgn2_hreg000                       dgn2_hreg000                  ;/*0x1460*/
} u_dgn2_regs;
#define ISP_BE_DGN2_NOR_REGS_NUM 1

typedef struct {
    volatile u_dgn3_hreg000                       dgn3_hreg000                  ;/*0x1480*/
    volatile u_dgn3_reg_dgn3_multiplier_4ch       dgn3_multiplier_4ch[2]        ;/*0x1484*/
} u_dgn3_regs;
#define ISP_BE_DGN3_NOR_REGS_NUM 3

typedef struct {
    volatile u_cds_h000                           cds_h000                      ;/*0x2040*/
} u_cds_h_regs;
#define ISP_BE_CDS_H_NOR_REGS_NUM 1

typedef struct {
    volatile u_cds_v000                           cds_v000                      ;/*0x2320*/
} u_cds_v_regs;
#define ISP_BE_CDS_V_NOR_REGS_NUM 1

typedef struct {
    u_blc1_ch0_regs blc1_ch0;
    u_blc1_ch1_regs blc1_ch1;
    u_dgn1_ch0_regs dgn1_ch0;
    u_dgn1_ch1_regs dgn1_ch1;
    u_aac_be_regs   aac;
    u_blc2_regs     blc2;
    u_dgn2_regs     dgn2;
    u_dgn3_regs     dgn3;
    u_cds_h_regs    cds_h;
    u_cds_v_regs    cds_v;
} xmedia_isp_regs;

#define ISP_BE_BASE_ADDR          0x11200000
#define ISP_BE_PUBLIC_BASE_ADDR    (ISP_BE_BASE_ADDR + 0x0)
#define ISP_BE_CTRL_BASE_ADDR      (ISP_BE_BASE_ADDR + 0x120)
#define ISP_BE_COMM_BASE_ADDR      (ISP_BE_BASE_ADDR + 0x160)
#define ISP_BE_FPN_BASE_ADDR       (ISP_BE_BASE_ADDR + 0x400)
#define ISP_BE_BLC1_CH0_BASE_ADDR  (ISP_BE_BASE_ADDR + 0x420)
#define ISP_BE_BLC1_CH1_BASE_ADDR  (ISP_BE_BASE_ADDR + 0x440)
#define ISP_BE_DGN1_CH0_BASE_ADDR  (ISP_BE_BASE_ADDR + 0x460)
#define ISP_BE_DGN1_CH1_BASE_ADDR  (ISP_BE_BASE_ADDR + 0x480)
#define ISP_BE_BPC_CH0_BASE_ADDR   (ISP_BE_BASE_ADDR + 0x4a0)
#define ISP_BE_AAC_BE_BASE_ADDR    (ISP_BE_BASE_ADDR + 0x4c0)
#define ISP_BE_HDR_BASE_ADDR       (ISP_BE_BASE_ADDR + 0x660)
#define ISP_BE_EXPANDER_BASE_ADDR  (ISP_BE_BASE_ADDR + 0x6c0)
#define ISP_BE_COMP_BASE_ADDR      (ISP_BE_BASE_ADDR + 0x900)
#define ISP_BE_NLM_BASE_ADDR       (ISP_BE_BASE_ADDR + 0x920)
#define ISP_BE_DCMP_BASE_ADDR      (ISP_BE_BASE_ADDR + 0x13e0)
#define ISP_BE_BLC2_BASE_ADDR      (ISP_BE_BASE_ADDR + 0x1400)
#define ISP_BE_LSC_BASE_ADDR       (ISP_BE_BASE_ADDR + 0x1420)
#define ISP_BE_DGN2_BASE_ADDR      (ISP_BE_BASE_ADDR + 0x1440)
#define ISP_BE_DGN3_BASE_ADDR      (ISP_BE_BASE_ADDR + 0x1460)
#define ISP_BE_DRC_BASE_ADDR       (ISP_BE_BASE_ADDR + 0x1480)
#define ISP_BE_DMS_BASE_ADDR       (ISP_BE_BASE_ADDR + 0x1940)
#define ISP_BE_CLC_BASE_ADDR       (ISP_BE_BASE_ADDR + 0x1980)
#define ISP_BE_DH_BASE_ADDR        (ISP_BE_BASE_ADDR + 0x1c60)
#define ISP_BE_RYC_BASE_ADDR       (ISP_BE_BASE_ADDR + 0x1fc0)
#define ISP_BE_CUS_V_BASE_ADDR     (ISP_BE_BASE_ADDR + 0x2000)
#define ISP_BE_CDS_H_BASE_ADDR     (ISP_BE_BASE_ADDR + 0x2020)
#define ISP_BE_LCE_BASE_ADDR       (ISP_BE_BASE_ADDR + 0x2040)
#define ISP_BE_CDS_V_BASE_ADDR     (ISP_BE_BASE_ADDR + 0x22e0)
#define ISP_BE_STNR_BASE_ADDR      (ISP_BE_BASE_ADDR + 0x2300)
#define ISP_BE_CNR_BASE_ADDR       (ISP_BE_BASE_ADDR + 0x24e0)
#define ISP_BE_LDC_H_BASE_ADDR     (ISP_BE_BASE_ADDR + 0x2520)
#define ISP_BE_VIPROC_REF_ADDR     (ISP_BE_BASE_ADDR + 0x25e0)

#define ISP_CRG_SENSOR_CLK_BASE_ADDR 0x120100F0

#define ISP_RGB_CHN_NUM 3

typedef struct {
    xmedia_u32 zone_valid_count; // 有效个数点
    xmedia_u32 rgb_avg_value[ISP_RGB_CHN_NUM];
} isp_be_awb_zone;

typedef struct {
    xmedia_u32 rgb_avg_value[ISP_RGB_CHN_NUM];
} isp_be_awb_global;

#define ISP_BAYER_CHN_NUM           4
#define ISP_BE_STATISTICS_HIST_NUM  1024
typedef struct {
    xmedia_u32 rggb_avg_value[ISP_BAYER_CHN_NUM];
} isp_be_ae_zone;

typedef struct {
    xmedia_u32 rggb_avg_value[ISP_BAYER_CHN_NUM];
} isp_be_ae_global;

typedef struct {
    xmedia_u32 hist_value[ISP_BE_STATISTICS_HIST_NUM];
} isp_be_ae_hist;

typedef struct {
    xmedia_u32 isp_dgain0[XMEDIA_ISP_WDR_CHN_MAX_NUM][XMEDIA_ISP_BAYER_PATTERN_NUM];
    xmedia_u32 isp_dgain1[XMEDIA_ISP_WDR_CHN_MAX_NUM][XMEDIA_ISP_BAYER_PATTERN_NUM];
    xmedia_u32 isp_dgain2;
    xmedia_u32 exp_ratio[XMEDIA_ISP_WDR_EXP_RATIO_MAX_NUM];
    xmedia_u16 gain_mul[XMEDIA_ISP_WDR_CHN_MAX_NUM]
                       [XMEDIA_ISP_BAYER_PATTERN_NUM]; // 除此参数区分长短帧，其余的所有通道使用一套配置
} isp_ae_sync_cfg;

typedef struct {
    xmedia_bool             valid;
    xmedia_sensor_regs_info sns;
    isp_ae_sync_cfg         ae;
} isp_sync_cfg;

#define ISP_STATISTICS_BUFFER_NUM 2
#define ISP_SYNC_BUFF_MAX_NUM     6

#define ISP_BE_AAC_AE_ZONE_COL_MAX_NUM       17
#define ISP_BE_REGS_AAC_AE_WEIGHT_LUT_LENGTH 32

typedef struct {
    xmedia_u32 frame_cnt;
    xmedia_u16 blc[XMEDIA_SENSOR_BAYER_CHN_NUM];

    xmedia_u8     sync_index;
    isp_sync_cfg  sync_buff[ISP_SYNC_BUFF_MAX_NUM];
    isp_sync_cfg *sync_node[ISP_SYNC_BUFF_MAX_NUM];

    // 以下统计信息buffer都是按照实际分块数来分配的内存，并非是按照最大分块数分配的内存，需要小心使用，谨防越界
    xmedia_u8 ae_zone_row_num;
    xmedia_u8 ae_zone_col_num;
    xmedia_u8 awb_zone_row_num;
    xmedia_u8 awb_zone_col_num;
    xmedia_u8 awb_zone_width;
    xmedia_u8 awb_zone_height;

    /*
     * ping-pong buffer index 示意
     *   ----------------------------------------------------
     *   |init |fs |early |fe |fs |early |fe |fs |early |fe |
     *   ----------------------------------------------------
     * w       |---> 0 <--|   |---> 1 <--|   |---> 0 <--|
     * r                   0              1              0
     * c  0         1              0              1
     *
     * w 表示硬件正在使用的buffer索引
     * r 表示软件格式化统计信息正在使用的buffer索引
     * c 表示软件配置到寄存器的buffer索引
     */
    // 硬件输出统计信息使用的ping-pong buffer
    xmedia_u8          read_index;
    xmedia_u64         aac_statistics_buffer[ISP_STATISTICS_BUFFER_NUM]; // ping-pong
    isp_be_awb_zone   *awb_zone[ISP_STATISTICS_BUFFER_NUM];
    isp_be_awb_global *awb_global[ISP_STATISTICS_BUFFER_NUM];
    isp_be_ae_zone    *ae_zone[ISP_STATISTICS_BUFFER_NUM];
    isp_be_ae_global  *ae_global[ISP_STATISTICS_BUFFER_NUM];
    isp_be_ae_hist    *ae_hist[ISP_STATISTICS_BUFFER_NUM];
    // 软件使用的统计信息buffer,主要是给ae和awb库用
    xmedia_u32                   *awb_zone_head_addr;
    xmedia_u32                   *awb_zone_avg_r;
    xmedia_u32                   *awb_zone_avg_g;
    xmedia_u32                   *awb_zone_avg_b;
    xmedia_u32                   *awb_zone_valid_count;
    xmedia_isp_ae_stat_global_avg ae_global_avg;
    xmedia_isp_ae_stat_hist       ae_format_hist;
} xmedia_isp_context;

#define ISP_PRINT(fmt, ...) printf("[ISP][Func]:%s [Line]:%d [Info]: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ISP_DGAIN_PRECISION 256
#define ISP_DIV_0_TO_1(a)   ((a) == 0 ? 1 : (a))
#define ISP_MAX2(x, y)      ((x) > (y) ? (x) : (y))
#define ISP_ABS(x)          ((x) >= 0 ? (x) : (-(x)))

static xmedia_isp_config  g_isp_cfg[ISP_PIPE_MAX_NUM];
static xmedia_isp_regs    g_isp_reg[ISP_PIPE_MAX_NUM];
static xmedia_isp_context g_isp_ctx[ISP_PIPE_MAX_NUM];

static const xmedia_u8 g_be_ae_default_zone_weight[XMEDIA_ISP_AE_ZONE_ROW_MAX_NUM][XMEDIA_ISP_AE_ZONE_COL_MAX_NUM] = {
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
};

#define DMA_ALIGN          32
#define ISP_ALIGN_UP(x, a) ((((x) + ((a)-1)) / a) * a)

STAGE1_FUNC xmedia_void delay(xmedia_u32 us)
{
    xmedia_u32 i, j;

    for (i = us; i > 0; i--) {
        for (j = 100; j > 0; j--) {
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
        }
    }
}

STAGE1_FUNC static xmedia_u32 isp_read_reg(xmedia_u32 addr)
{
    return *((xmedia_u32 *)addr);
}

STAGE1_FUNC static xmedia_void isp_write_reg(xmedia_u32 addr, xmedia_u32 value)
{
    *((xmedia_u32 *)addr) = value;
}

STAGE1_FUNC xmedia_s32 hal_sensor_reset(xmedia_u32 ch, xmedia_bool enable)
{
    xmedia_u32 value;

    value  = isp_read_reg(ISP_CRG_SENSOR_CLK_BASE_ADDR);
    value &= ~(0x1 << 1 << (ch * 8)); // clear reset bit
    value |= (enable << 1 << (ch * 8));
    isp_write_reg(ISP_CRG_SENSOR_CLK_BASE_ADDR, value);

    return XMEDIA_SUCCESS;
}

STAGE1_FUNC xmedia_s32 hal_sensor_read_mclk(xmedia_u32 ch, xmedia_u32 *clk)
{
    xmedia_u32 value;
    xmedia_u32 clk_value;

    value  = isp_read_reg(ISP_CRG_SENSOR_CLK_BASE_ADDR);

    if (ch == 0) {
        value = (value & 0x3c) >> 2;
    } else if (ch == 1) {
        value = (value & 0x3c00) >> 10;
    } else if (ch == 2) {
        value = (value & 0x3c0000) >> 18;
    } else if (ch == 3) {
        value = (value & 0x3c000000) >> 26;
    } else {
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    if (value == 0) {
        clk_value = 74250000;
    } else if (value == 1) {
        clk_value = 72000000;
    } else if (value == 2) {
        clk_value = 54000000;
    } else if (value == 3) {
        clk_value = 50000000;
    } else if (value == 4) {
        clk_value = 37125000;
    } else if (value == 5) {
        clk_value = 36000000;
    } else if (value == 6) {
        clk_value = 27000000;
    } else if (value == 7) {
        clk_value = 25000000;
    } else if (value == 8) {
        clk_value = 24000000;
    } else if (value == 9) {
        clk_value = 20000000;
    } else if (value == 10) {
        clk_value = 18000000;
    } else if (value == 11) {
        clk_value = 12000000;
    } else {
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    *clk = clk_value;

    return XMEDIA_SUCCESS;
}

STAGE1_FUNC xmedia_s32 hal_sensor_set_mclk(xmedia_u32 ch, xmedia_u32 clk)
{
    xmedia_u32 value;
    xmedia_u32 clk_value;

    value  = isp_read_reg(ISP_CRG_SENSOR_CLK_BASE_ADDR);
    value &= ~(0x1 << (ch * 8)); // clear clk_en bit
    isp_write_reg(ISP_CRG_SENSOR_CLK_BASE_ADDR, value);

    value  = isp_read_reg(ISP_CRG_SENSOR_CLK_BASE_ADDR);
    value &= ~(0xF << 2 << (ch * 8)); // clear clk_sel bit
    if (clk == 74250000) {
        clk_value = 0;
    } else if (clk == 72000000) {
        clk_value = 1;
    } else if (clk == 54000000) {
        clk_value = 2;
    } else if (clk == 50000000) {
        clk_value = 3;
    } else if (clk == 37125000) {
        clk_value = 4;
    } else if (clk == 36000000) {
        clk_value = 5;
    } else if (clk == 27000000) {
        clk_value = 6;
    } else if (clk == 25000000) {
        clk_value = 7;
    } else if (clk == 24000000) {
        clk_value = 8;
    } else if (clk == 20000000) {
        clk_value = 9;
    } else if (clk == 18000000) {
        clk_value = 10;
    } else if (clk == 12000000) {
        clk_value = 11;
    } else {
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }
    value |= (clk_value << 2 << (ch * 8));
    isp_write_reg(ISP_CRG_SENSOR_CLK_BASE_ADDR, value);

    value  = isp_read_reg(ISP_CRG_SENSOR_CLK_BASE_ADDR);
    value &= ~(0x1 << (ch * 8)); // clear clk_en bit
    value |= (0x1 << (ch * 8));  // set clk_en bit
    isp_write_reg(ISP_CRG_SENSOR_CLK_BASE_ADDR, value);

    return XMEDIA_SUCCESS;
}

static xmedia_void isp_reg_init_aac(xmedia_u32 pipe)
{
    xmedia_s32 i, row, col;
    xmedia_u16 zone_width, zone_height, zone_col_start, zone_row_start;
    xmedia_u32 tile_num;

    // aac
    // 分块数按照分块最大宽高计算,向上取整后再计算分块宽高,分块宽高需要为偶数,起始点也要为偶数,分块区域需要居中
    zone_width     = ((g_isp_cfg[pipe].size.width / g_isp_ctx[pipe].ae_zone_col_num) >> 1 << 1);
    zone_height    = ((g_isp_cfg[pipe].size.height / g_isp_ctx[pipe].ae_zone_row_num) >> 1 << 1);
    zone_col_start = (g_isp_cfg[pipe].size.width - zone_width * g_isp_ctx[pipe].ae_zone_col_num) >> 2 << 1;
    zone_row_start = (g_isp_cfg[pipe].size.height - zone_height * g_isp_ctx[pipe].ae_zone_row_num) >> 2 << 1;

    // 1. ae只需要全局均值,不需要分块均值和直方图统计信息
    // 2. 全局均值和分块均值是由同一个寄存器控制开关,所以分块均值相关的寄存器也需要配置
    // 3. 全局均值的统计范围是由分块均值统计范围决定,所以分块均值统计范围需要尽量覆盖全图
    g_isp_reg[pipe].aac.aac_be000.bits.reg_aac_initial_sram_enable   = 1;
    g_isp_reg[pipe].aac.aac_be000.bits.reg_aac_ae_cfa_enable         = 1;
    g_isp_reg[pipe].aac.aac_be000.bits.reg_aac_ae_cfa_tile_num_col   = g_isp_ctx[pipe].ae_zone_col_num - 1;
    g_isp_reg[pipe].aac.aac_be000.bits.reg_aac_ae_cfa_tile_num_row   = g_isp_ctx[pipe].ae_zone_row_num - 1;
    g_isp_reg[pipe].aac.aac_be001.bits.reg_aac_ae_cfa_tile_col_start = zone_col_start;
    g_isp_reg[pipe].aac.aac_be001.bits.reg_aac_ae_cfa_tile_row_start = zone_row_start;
    tile_num = g_isp_ctx[pipe].ae_zone_col_num * g_isp_ctx[pipe].ae_zone_row_num;
    g_isp_reg[pipe].aac.aac_be002.bits.reg_aac_ae_cfa_tile_num_all   = (256 * 16 * 8 + (tile_num >> 1)) / tile_num;
    g_isp_reg[pipe].aac.aac_be003.bits.reg_aac_ae_cfa_tile_width     = zone_width - 1;
    g_isp_reg[pipe].aac.aac_be003.bits.reg_aac_ae_cfa_tile_height    = zone_height - 1;
    g_isp_reg[pipe].aac.aac_be003.bits.reg_aac_ae_stat_location      = XMEDIA_ISP_AE_SWITCH_AFTER_DGN2;
    g_isp_reg[pipe].aac.aac_be004.bits.reg_aac_hist_enable           = 1;
    g_isp_reg[pipe].aac.aac_be004.bits.reg_aac_hist_area_mode        = 1;
    g_isp_reg[pipe].aac.aac_be004.bits.reg_aac_hist_fourmode         = 1;
    g_isp_reg[pipe].aac.aac_be004.bits.reg_aac_hist_single_color_bin = 1;
    g_isp_reg[pipe].aac.aac_be004.bits.reg_aac_hist_sqrtmode         = 0;
    g_isp_reg[pipe].aac.aac_be004.bits.reg_aac_ae_cfa_tile_sqrtmode  = 0;
    g_isp_reg[pipe].aac.aac_be004.bits.reg_aac_ae_cfa_tile_area      = 512 * 512 * 8 / (zone_width * zone_height);
    g_isp_reg[pipe].aac.aac_be005.bits.reg_aac_af_enable             = 0;

    // 分块数按照分块最大宽高计算,向上取整后再计算分块宽高,分块宽高需要为偶数,起始点也要为偶数,分块区域需要居中
    zone_width     = (g_isp_cfg[pipe].size.width / g_isp_ctx[pipe].awb_zone_col_num) >> 1 << 1;
    zone_height    = (g_isp_cfg[pipe].size.height / g_isp_ctx[pipe].awb_zone_row_num) >> 1 << 1;
    zone_col_start = (g_isp_cfg[pipe].size.width - zone_width * g_isp_ctx[pipe].awb_zone_col_num) >> 2 << 1;
    zone_row_start = (g_isp_cfg[pipe].size.height - zone_height * g_isp_ctx[pipe].awb_zone_row_num) >> 2 << 1;

    g_isp_ctx[pipe].awb_zone_width  = zone_width;
    g_isp_ctx[pipe].awb_zone_height = zone_height;

    g_isp_reg[pipe].aac.aac_be033.bits.reg_aac_awb_cfa_enable          = 1;
    g_isp_reg[pipe].aac.aac_be033.bits.reg_aac_awb_stat_location       = XMEDIA_ISP_AWB_SWITCH_AFTER_DG2;
    g_isp_reg[pipe].aac.aac_be033.bits.reg_aac_awb_cfa_tile_num_col    = g_isp_ctx[pipe].awb_zone_col_num - 1;
    g_isp_reg[pipe].aac.aac_be033.bits.reg_aac_awb_cfa_tile_num_row    = g_isp_ctx[pipe].awb_zone_row_num - 1;
    g_isp_reg[pipe].aac.aac_be034.bits.reg_aac_awb_cfa_tile_col_start  = zone_col_start;
    g_isp_reg[pipe].aac.aac_be034.bits.reg_aac_awb_cfa_tile_row_start  = zone_row_start;
    g_isp_reg[pipe].aac.aac_be035.bits.reg_aac_awb_cfa_tile_min_thresh = 0;
    g_isp_reg[pipe].aac.aac_be035.bits.reg_aac_awb_cfa_tile_max_thresh = 65535;
    g_isp_reg[pipe].aac.aac_be036.bits.reg_aac_awb_cfa_tile_width      = zone_width - 1;
    g_isp_reg[pipe].aac.aac_be036.bits.reg_aac_awb_cfa_tile_height     = zone_height - 1;
    g_isp_reg[pipe].aac.aac_be036.bits.reg_aac_awb_cfa_tile_num_all =
        1024 * 8 / (g_isp_ctx[pipe].awb_zone_col_num * g_isp_ctx[pipe].awb_zone_row_num);
    g_isp_reg[pipe].aac.aac_be037.bits.reg_aac_awb_cfa_tile_max_r_g   = 2047;
    g_isp_reg[pipe].aac.aac_be037.bits.reg_aac_awb_cfa_tile_min_r_g   = 0;
    g_isp_reg[pipe].aac.aac_be038.bits.reg_aac_awb_cfa_tile_max_b_g   = 2047;
    g_isp_reg[pipe].aac.aac_be038.bits.reg_aac_awb_cfa_tile_min_b_g   = 0;
    g_isp_reg[pipe].aac.aac_be039.bits.reg_aac_awb_cfa_pseudo_y_wgt_0 = 4;
    g_isp_reg[pipe].aac.aac_be039.bits.reg_aac_awb_cfa_pseudo_y_wgt_1 = 5;
    g_isp_reg[pipe].aac.aac_be039.bits.reg_aac_awb_cfa_pseudo_y_wgt_2 = 2;
    g_isp_reg[pipe].aac.aac_be039.bits.reg_aac_awb_cfa_pseudo_y_wgt_3 = 5;

    for (i = 0; i < ISP_BE_REGS_AAC_AE_WEIGHT_LUT_LENGTH; i++) {
        row = i * 8 / ISP_BE_AAC_AE_ZONE_COL_MAX_NUM;
        col = i * 8 % ISP_BE_AAC_AE_ZONE_COL_MAX_NUM;
        g_isp_reg[pipe].aac.aac_be_reg_aac_ae_cfa_tile_domain_wt[i].bits.reg_aac_ae_cfa_tile_domain_wt_0 =
                                                                            g_be_ae_default_zone_weight[row][col];
        g_isp_reg[pipe].aac.aac_be_reg_aac_ae_cfa_tile_domain_wt[i].bits.reg_aac_ae_cfa_tile_domain_wt_1 =
                                                                            g_be_ae_default_zone_weight[row][col + 1];
        g_isp_reg[pipe].aac.aac_be_reg_aac_ae_cfa_tile_domain_wt[i].bits.reg_aac_ae_cfa_tile_domain_wt_2 =
                                                                            g_be_ae_default_zone_weight[row][col + 2];
        g_isp_reg[pipe].aac.aac_be_reg_aac_ae_cfa_tile_domain_wt[i].bits.reg_aac_ae_cfa_tile_domain_wt_3 =
                                                                            g_be_ae_default_zone_weight[row][col + 3];
        g_isp_reg[pipe].aac.aac_be_reg_aac_ae_cfa_tile_domain_wt[i].bits.reg_aac_ae_cfa_tile_domain_wt_4 =
                                                                            g_be_ae_default_zone_weight[row][col + 4];
        g_isp_reg[pipe].aac.aac_be_reg_aac_ae_cfa_tile_domain_wt[i].bits.reg_aac_ae_cfa_tile_domain_wt_5 =
                                                                            g_be_ae_default_zone_weight[row][col + 5];
        g_isp_reg[pipe].aac.aac_be_reg_aac_ae_cfa_tile_domain_wt[i].bits.reg_aac_ae_cfa_tile_domain_wt_6 =
                                                                            g_be_ae_default_zone_weight[row][col + 6];
        g_isp_reg[pipe].aac.aac_be_reg_aac_ae_cfa_tile_domain_wt[i].bits.reg_aac_ae_cfa_tile_domain_wt_7 =
                                                                            g_be_ae_default_zone_weight[row][col + 7];
    }
}

static xmedia_void isp_reg_init_blc(xmedia_u32 pipe)
{
    g_isp_reg[pipe].blc1_ch0.blc1_ch0_blc1000.bits.reg_blc1_enable           = 0;
    g_isp_reg[pipe].blc1_ch0.blc1_ch0_blc1001.bits.reg_blc1_global_offset_0  = g_isp_ctx[pipe].blc[0];
    g_isp_reg[pipe].blc1_ch0.blc1_ch0_blc1001.bits.reg_blc1_global_offset_1  = g_isp_ctx[pipe].blc[1];
    g_isp_reg[pipe].blc1_ch0.blc1_ch0_blc1002.bits.reg_blc1_global_offset_2  = g_isp_ctx[pipe].blc[2];
    g_isp_reg[pipe].blc1_ch0.blc1_ch0_blc1002.bits.reg_blc1_global_offset_3  = g_isp_ctx[pipe].blc[3];
    g_isp_reg[pipe].blc1_ch1.blc1_ch1_blc1000.bits.reg_blc1_enable           = 0;
    g_isp_reg[pipe].blc1_ch1.blc1_ch1_blc1001.bits.reg_blc1_global_offset_0  = g_isp_ctx[pipe].blc[0];
    g_isp_reg[pipe].blc1_ch1.blc1_ch1_blc1001.bits.reg_blc1_global_offset_1  = g_isp_ctx[pipe].blc[1];
    g_isp_reg[pipe].blc1_ch1.blc1_ch1_blc1002.bits.reg_blc1_global_offset_2  = g_isp_ctx[pipe].blc[2];
    g_isp_reg[pipe].blc1_ch1.blc1_ch1_blc1002.bits.reg_blc1_global_offset_3  = g_isp_ctx[pipe].blc[3];
    g_isp_reg[pipe].blc2.blc2000.bits.reg_blc2_enable                        = 1;
    g_isp_reg[pipe].blc2.blc2_global_offset[0].bits.reg_blc2_global_offset_0 = g_isp_ctx[pipe].blc[0];
    g_isp_reg[pipe].blc2.blc2_global_offset[0].bits.reg_blc2_global_offset_1 = g_isp_ctx[pipe].blc[1];
    g_isp_reg[pipe].blc2.blc2_global_offset[1].bits.reg_blc2_global_offset_0 = g_isp_ctx[pipe].blc[2];
    g_isp_reg[pipe].blc2.blc2_global_offset[1].bits.reg_blc2_global_offset_1 = g_isp_ctx[pipe].blc[3];
}

static xmedia_void isp_reg_init_cds(xmedia_u32 pipe)
{
    g_isp_reg[pipe].cds_h.cds_h000.bits.reg_cds422_enable  = 1;
    g_isp_reg[pipe].cds_h.cds_h000.bits.reg_cds422_param_0 = 32;
    g_isp_reg[pipe].cds_h.cds_h000.bits.reg_cds422_param_1 = 32;
    g_isp_reg[pipe].cds_v.cds_v000.bits.reg_cds_enable     = 1;
    g_isp_reg[pipe].cds_v.cds_v000.bits.reg_cds_param_0    = 32;
    g_isp_reg[pipe].cds_v.cds_v000.bits.reg_cds_param_1    = 32;
}

static xmedia_void isp_reg_init_dgn(xmedia_u32 pipe)
{
    // dgn
    g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_hreg000.bits.enable                      = 1;
    g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_digital_gain[0].bits.digital_gain_0      = ISP_DGAIN_PRECISION;
    g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_digital_gain[0].bits.digital_gain_1      = ISP_DGAIN_PRECISION;
    g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_digital_gain[1].bits.digital_gain_0      = ISP_DGAIN_PRECISION;
    g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_digital_gain[1].bits.digital_gain_1      = ISP_DGAIN_PRECISION;
    g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_blc_in[0].bits.blc_in_0                  = g_isp_ctx[pipe].blc[0];
    g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_blc_in[0].bits.blc_in_1                  = g_isp_ctx[pipe].blc[1];
    g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_blc_in[1].bits.blc_in_0                  = g_isp_ctx[pipe].blc[2];
    g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_blc_in[1].bits.blc_in_1                  = g_isp_ctx[pipe].blc[3];
    g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_blc_out[0].bits.blc_out_0                = g_isp_ctx[pipe].blc[0];
    g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_blc_out[0].bits.blc_out_1                = g_isp_ctx[pipe].blc[1];
    g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_blc_out[1].bits.blc_out_0                = g_isp_ctx[pipe].blc[2];
    g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_blc_out[1].bits.blc_out_1                = g_isp_ctx[pipe].blc[3];
    g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_hreg000.bits.enable                      = 1;
    g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_digital_gain[0].bits.digital_gain_0      = ISP_DGAIN_PRECISION;
    g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_digital_gain[0].bits.digital_gain_1      = ISP_DGAIN_PRECISION;
    g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_digital_gain[1].bits.digital_gain_0      = ISP_DGAIN_PRECISION;
    g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_digital_gain[1].bits.digital_gain_1      = ISP_DGAIN_PRECISION;
    g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_blc_in[0].bits.blc_in_0                  = g_isp_ctx[pipe].blc[0];
    g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_blc_in[0].bits.blc_in_1                  = g_isp_ctx[pipe].blc[1];
    g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_blc_in[1].bits.blc_in_0                  = g_isp_ctx[pipe].blc[2];
    g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_blc_in[1].bits.blc_in_1                  = g_isp_ctx[pipe].blc[3];
    g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_blc_out[0].bits.blc_out_0                = g_isp_ctx[pipe].blc[0];
    g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_blc_out[0].bits.blc_out_1                = g_isp_ctx[pipe].blc[1];
    g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_blc_out[1].bits.blc_out_0                = g_isp_ctx[pipe].blc[2];
    g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_blc_out[1].bits.blc_out_1                = g_isp_ctx[pipe].blc[3];
    g_isp_reg[pipe].dgn2.dgn2_hreg000.bits.reg_dgn2_enable                     = 1;
    g_isp_reg[pipe].dgn2.dgn2_hreg000.bits.reg_dgn2_multiplier_single          = ISP_DGAIN_PRECISION;
    g_isp_reg[pipe].dgn3.dgn3_hreg000.bits.reg_dgn3_enable                     = 1;
    g_isp_reg[pipe].dgn3.dgn3_multiplier_4ch[0].bits.reg_dgn3_multiplier_4ch_0 = ISP_DGAIN_PRECISION;
    g_isp_reg[pipe].dgn3.dgn3_multiplier_4ch[0].bits.reg_dgn3_multiplier_4ch_1 = ISP_DGAIN_PRECISION;
    g_isp_reg[pipe].dgn3.dgn3_multiplier_4ch[1].bits.reg_dgn3_multiplier_4ch_0 = ISP_DGAIN_PRECISION;
    g_isp_reg[pipe].dgn3.dgn3_multiplier_4ch[1].bits.reg_dgn3_multiplier_4ch_1 = ISP_DGAIN_PRECISION;
}

static xmedia_s32 isp_reg_init(xmedia_u32 pipe)
{
    xmedia_s32 i;
    xmedia_u32 value;
    xmedia_u32 bayer_pattern = 0;

    if (pipe >= ISP_PIPE_MAX_NUM) {
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    if (g_isp_cfg[pipe].bayer_fmt == XMEDIA_VIDEO_BAYER_FMT_RGGB) {
        bayer_pattern = 0;
    } else if (g_isp_cfg[pipe].bayer_fmt == XMEDIA_VIDEO_BAYER_FMT_BGGR) {
        bayer_pattern = 1;
    } else if (g_isp_cfg[pipe].bayer_fmt == XMEDIA_VIDEO_BAYER_FMT_GRBG) {
        bayer_pattern = 2;
    } else if (g_isp_cfg[pipe].bayer_fmt == XMEDIA_VIDEO_BAYER_FMT_GBRG) {
        bayer_pattern = 3;
    } else {
        puts("bayer_pattern - ");
        putdec(g_isp_cfg[pipe].bayer_fmt);
        puts(" is invalid.\n");
    }

    memset(&g_isp_reg[pipe], 0, sizeof(xmedia_isp_regs));

    isp_reg_init_aac(pipe);
    isp_reg_init_blc(pipe);
    isp_reg_init_cds(pipe);
    isp_reg_init_dgn(pipe);

    isp_write_reg(ISP_BE_CTRL_BASE_ADDR, 0x0);
    isp_write_reg(ISP_BE_CTRL_BASE_ADDR + 0x4, 0x0);
    value  = isp_read_reg(ISP_BE_COMM_BASE_ADDR);
    value  &= ~((1 << 15) | (1 << 14) | (1 << 13) | (1 << 12) | (1 << 11) | (1 << 7) | (1 << 2) | (1 << 1));
    isp_write_reg(ISP_BE_COMM_BASE_ADDR, value);
    value  = isp_read_reg(ISP_BE_COMM_BASE_ADDR + 0xC);
    value &= ~((0x3 << 0));
    value |= (bayer_pattern << 0);
    isp_write_reg(ISP_BE_COMM_BASE_ADDR + 0xC, value);
    isp_write_reg(ISP_BE_COMM_BASE_ADDR + 0x10, 64 | (((1 << 18) / 64) << 16)); // 64为一倍曝光比
    isp_write_reg(ISP_BE_COMM_BASE_ADDR + 0x10C, 0x0);
    isp_write_reg(ISP_BE_COMM_BASE_ADDR + 0x11C, 0x0);
    isp_write_reg(ISP_BE_COMM_BASE_ADDR + 0x120, g_isp_ctx[pipe].aac_statistics_buffer[0]);
    g_isp_ctx[pipe].read_index = 1;

    isp_write_reg(ISP_BE_FPN_BASE_ADDR, 0x0);
    isp_write_reg(ISP_BE_BLC1_CH0_BASE_ADDR, g_isp_reg[pipe].blc1_ch0.blc1_ch0_blc1000.u32);
    isp_write_reg(ISP_BE_BLC1_CH0_BASE_ADDR + 0x4, g_isp_reg[pipe].blc1_ch0.blc1_ch0_blc1001.u32);
    isp_write_reg(ISP_BE_BLC1_CH0_BASE_ADDR + 0x8, g_isp_reg[pipe].blc1_ch0.blc1_ch0_blc1002.u32);
    isp_write_reg(ISP_BE_BLC1_CH1_BASE_ADDR, g_isp_reg[pipe].blc1_ch1.blc1_ch1_blc1000.u32);
    isp_write_reg(ISP_BE_BLC1_CH1_BASE_ADDR + 0x4, g_isp_reg[pipe].blc1_ch1.blc1_ch1_blc1001.u32);
    isp_write_reg(ISP_BE_BLC1_CH1_BASE_ADDR + 0x8, g_isp_reg[pipe].blc1_ch1.blc1_ch1_blc1002.u32);
    isp_write_reg(ISP_BE_DGN1_CH0_BASE_ADDR, g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_hreg000.u32);
    isp_write_reg(ISP_BE_DGN1_CH0_BASE_ADDR + 0x4, g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_digital_gain[0].u32);
    isp_write_reg(ISP_BE_DGN1_CH0_BASE_ADDR + 0x8, g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_digital_gain[1].u32);
    isp_write_reg(ISP_BE_DGN1_CH0_BASE_ADDR + 0xC, g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_blc_in[0].u32);
    isp_write_reg(ISP_BE_DGN1_CH0_BASE_ADDR + 0x10, g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_blc_in[1].u32);
    isp_write_reg(ISP_BE_DGN1_CH0_BASE_ADDR + 0x14, g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_blc_out[0].u32);
    isp_write_reg(ISP_BE_DGN1_CH0_BASE_ADDR + 0x18, g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_blc_out[1].u32);
    isp_write_reg(ISP_BE_DGN1_CH1_BASE_ADDR, g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_hreg000.u32);
    isp_write_reg(ISP_BE_DGN1_CH1_BASE_ADDR + 0x4, g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_digital_gain[0].u32);
    isp_write_reg(ISP_BE_DGN1_CH1_BASE_ADDR + 0x8, g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_digital_gain[1].u32);
    isp_write_reg(ISP_BE_DGN1_CH1_BASE_ADDR + 0xC, g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_blc_in[0].u32);
    isp_write_reg(ISP_BE_DGN1_CH1_BASE_ADDR + 0x10, g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_blc_in[1].u32);
    isp_write_reg(ISP_BE_DGN1_CH1_BASE_ADDR + 0x14, g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_blc_out[0].u32);
    isp_write_reg(ISP_BE_DGN1_CH1_BASE_ADDR + 0x18, g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_blc_out[1].u32);
    isp_write_reg(ISP_BE_BPC_CH0_BASE_ADDR, 0x0);
    isp_write_reg(ISP_BE_AAC_BE_BASE_ADDR, g_isp_reg[pipe].aac.aac_be000.u32);
    isp_write_reg(ISP_BE_AAC_BE_BASE_ADDR + 0x4, g_isp_reg[pipe].aac.aac_be001.u32);
    isp_write_reg(ISP_BE_AAC_BE_BASE_ADDR + 0x8, g_isp_reg[pipe].aac.aac_be002.u32);
    isp_write_reg(ISP_BE_AAC_BE_BASE_ADDR + 0xC, g_isp_reg[pipe].aac.aac_be003.u32);
    isp_write_reg(ISP_BE_AAC_BE_BASE_ADDR + 0x10, g_isp_reg[pipe].aac.aac_be004.u32);
    isp_write_reg(ISP_BE_AAC_BE_BASE_ADDR + 0x14, g_isp_reg[pipe].aac.aac_be005.u32);
    isp_write_reg(ISP_BE_AAC_BE_BASE_ADDR + 0x84, g_isp_reg[pipe].aac.aac_be033.u32);
    isp_write_reg(ISP_BE_AAC_BE_BASE_ADDR + 0x88, g_isp_reg[pipe].aac.aac_be034.u32);
    isp_write_reg(ISP_BE_AAC_BE_BASE_ADDR + 0x8C, g_isp_reg[pipe].aac.aac_be035.u32);
    isp_write_reg(ISP_BE_AAC_BE_BASE_ADDR + 0x90, g_isp_reg[pipe].aac.aac_be036.u32);
    isp_write_reg(ISP_BE_AAC_BE_BASE_ADDR + 0x94, g_isp_reg[pipe].aac.aac_be037.u32);
    isp_write_reg(ISP_BE_AAC_BE_BASE_ADDR + 0x98, g_isp_reg[pipe].aac.aac_be038.u32);
    isp_write_reg(ISP_BE_AAC_BE_BASE_ADDR + 0x9C, g_isp_reg[pipe].aac.aac_be039.u32);
    for (i = 0; i < 32; i++) {
        isp_write_reg(ISP_BE_AAC_BE_BASE_ADDR + 0xA0 + i * 0x4, 0x11111111);
    }
    isp_write_reg(ISP_BE_HDR_BASE_ADDR, 0x0);
    isp_write_reg(ISP_BE_EXPANDER_BASE_ADDR, 0x0);
    isp_write_reg(ISP_BE_COMP_BASE_ADDR, 0x0);
    isp_write_reg(ISP_BE_NLM_BASE_ADDR, 0x0);
    isp_write_reg(ISP_BE_DCMP_BASE_ADDR, 0x0);
    isp_write_reg(ISP_BE_BLC2_BASE_ADDR, g_isp_reg[pipe].blc2.blc2000.u32);
    isp_write_reg(ISP_BE_BLC2_BASE_ADDR + 0x4, g_isp_reg[pipe].blc2.blc2_global_offset[0].u32);
    isp_write_reg(ISP_BE_BLC2_BASE_ADDR + 0x8, g_isp_reg[pipe].blc2.blc2_global_offset[1].u32);
    isp_write_reg(ISP_BE_LSC_BASE_ADDR, 0x0);
    isp_write_reg(ISP_BE_DGN2_BASE_ADDR, g_isp_reg[pipe].dgn2.dgn2_hreg000.u32);
    isp_write_reg(ISP_BE_DGN3_BASE_ADDR, g_isp_reg[pipe].dgn3.dgn3_hreg000.u32);
    isp_write_reg(ISP_BE_DGN3_BASE_ADDR + 0x4, g_isp_reg[pipe].dgn3.dgn3_multiplier_4ch[0].u32);
    isp_write_reg(ISP_BE_DGN3_BASE_ADDR + 0x8, g_isp_reg[pipe].dgn3.dgn3_multiplier_4ch[1].u32);
    isp_write_reg(ISP_BE_DRC_BASE_ADDR, 0x0);
    isp_write_reg(ISP_BE_DMS_BASE_ADDR, 0x0);
    isp_write_reg(ISP_BE_CLC_BASE_ADDR, 0x0);
    isp_write_reg(ISP_BE_DH_BASE_ADDR, 0x0);
    isp_write_reg(ISP_BE_RYC_BASE_ADDR, 0x0);
    isp_write_reg(ISP_BE_CUS_V_BASE_ADDR, 0x0);
    isp_write_reg(ISP_BE_CDS_H_BASE_ADDR, g_isp_reg[pipe].cds_h.cds_h000.u32);
    isp_write_reg(ISP_BE_LCE_BASE_ADDR, 0x0);
    isp_write_reg(ISP_BE_CDS_V_BASE_ADDR, g_isp_reg[pipe].cds_v.cds_v000.u32);
    isp_write_reg(ISP_BE_STNR_BASE_ADDR, 0x0);
    isp_write_reg(ISP_BE_CNR_BASE_ADDR, 0x0);
    isp_write_reg(ISP_BE_LDC_H_BASE_ADDR, 0x0);

    return XMEDIA_SUCCESS;
}

static xmedia_s32 isp_init_buffer(xmedia_u32 pipe)
{
    xmedia_u32 addr, size, zone_num;
    xmedia_u32 awb_zone_size, awb_global_size, ae_zone_size, ae_global_size, ae_hist_size;

    // 分块数按照分块最大宽高计算,向上取整
    g_isp_ctx[pipe].ae_zone_col_num  = (g_isp_cfg[pipe].size.width + 511) / 512;
    g_isp_ctx[pipe].ae_zone_row_num  = (g_isp_cfg[pipe].size.height + 511) / 512;
    g_isp_ctx[pipe].awb_zone_col_num = (g_isp_cfg[pipe].size.width + 255) / 256;
    g_isp_ctx[pipe].awb_zone_row_num = (g_isp_cfg[pipe].size.height + 255) / 256;

    awb_zone_size   = g_isp_ctx[pipe].awb_zone_row_num * g_isp_ctx[pipe].awb_zone_col_num * sizeof(isp_be_awb_zone);
    awb_global_size = sizeof(isp_be_awb_global);
    ae_zone_size    = g_isp_ctx[pipe].ae_zone_row_num * g_isp_ctx[pipe].ae_zone_col_num * sizeof(isp_be_ae_zone);
    ae_global_size  = sizeof(isp_be_ae_global);
    ae_hist_size    = sizeof(isp_be_ae_hist);
    size            = awb_zone_size + awb_global_size + ae_zone_size + ae_global_size + ae_hist_size;
    size            = ISP_ALIGN_UP(size, DMA_ALIGN);
    addr            = (xmedia_u32)reserved_mem_align(DMA_ALIGN, size * ISP_STATISTICS_BUFFER_NUM);
    if (addr == XMEDIA_NULL) {
        return XMEDIA_ERRCODE_NOT_ENOUGH_MEMORY;
    }
    memset((xmedia_void *)addr, 0, size * ISP_STATISTICS_BUFFER_NUM);

    g_isp_ctx[pipe].aac_statistics_buffer[0] = addr;
    g_isp_ctx[pipe].awb_zone[0]   = (isp_be_awb_zone *)(xmedia_uintptr_t)g_isp_ctx[pipe].aac_statistics_buffer[0];
    g_isp_ctx[pipe].awb_global[0] = (isp_be_awb_global *)((xmedia_u8 *)g_isp_ctx[pipe].awb_zone[0] + awb_zone_size);
    g_isp_ctx[pipe].ae_zone[0]    = (isp_be_ae_zone *)((xmedia_u8 *)g_isp_ctx[pipe].awb_global[0] + awb_global_size);
    g_isp_ctx[pipe].ae_global[0]  = (isp_be_ae_global *)((xmedia_u8 *)g_isp_ctx[pipe].ae_zone[0] + ae_zone_size);
    g_isp_ctx[pipe].ae_hist[0]    = (isp_be_ae_hist *)((xmedia_u8 *)g_isp_ctx[pipe].ae_global[0] + ae_global_size);

    g_isp_ctx[pipe].aac_statistics_buffer[1] = addr + size;
    g_isp_ctx[pipe].awb_zone[1]   = (isp_be_awb_zone *)(xmedia_uintptr_t)g_isp_ctx[pipe].aac_statistics_buffer[1];
    g_isp_ctx[pipe].awb_global[1] = (isp_be_awb_global *)((xmedia_u8 *)g_isp_ctx[pipe].awb_zone[1] + awb_zone_size);
    g_isp_ctx[pipe].ae_zone[1]    = (isp_be_ae_zone *)((xmedia_u8 *)g_isp_ctx[pipe].awb_global[1] + awb_global_size);
    g_isp_ctx[pipe].ae_global[1]  = (isp_be_ae_global *)((xmedia_u8 *)g_isp_ctx[pipe].ae_zone[1] + ae_zone_size);
    g_isp_ctx[pipe].ae_hist[1]    = (isp_be_ae_hist *)((xmedia_u8 *)g_isp_ctx[pipe].ae_global[1] + ae_global_size);

    addr = (xmedia_u32)malloc(awb_zone_size);
    if (addr == XMEDIA_NULL) {
        return XMEDIA_ERRCODE_NOT_ENOUGH_MEMORY;
    }
    memset((xmedia_void *)addr, 0, awb_zone_size);

    zone_num                             = g_isp_ctx[pipe].awb_zone_row_num * g_isp_ctx[pipe].awb_zone_col_num;
    g_isp_ctx[pipe].awb_zone_head_addr   = (xmedia_u32 *)(xmedia_uintptr_t)addr;
    g_isp_ctx[pipe].awb_zone_avg_r       = g_isp_ctx[pipe].awb_zone_head_addr;
    g_isp_ctx[pipe].awb_zone_avg_g       = g_isp_ctx[pipe].awb_zone_avg_r + zone_num;
    g_isp_ctx[pipe].awb_zone_avg_b       = g_isp_ctx[pipe].awb_zone_avg_g + zone_num;
    g_isp_ctx[pipe].awb_zone_valid_count = g_isp_ctx[pipe].awb_zone_avg_b + zone_num;

    addr = (xmedia_u32)malloc(ae_hist_size);
    if (addr == XMEDIA_NULL) {
        return XMEDIA_ERRCODE_NOT_ENOUGH_MEMORY;
    }
    memset((xmedia_void *)addr, 0, ae_hist_size);
    g_isp_ctx[pipe].ae_format_hist.be_histogram_mem_array = (xmedia_isp_be_hist *)(xmedia_uintptr_t)addr;

#if ISP_COUNT_SIZE_DEBUG
    puts("aac_stt_buff size = ");
    putdec(size * ISP_STATISTICS_BUFFER_NUM);
    puts(".\n");
    puts("awb_zone_size size = ");
    putdec(awb_zone_size);
    puts(".\n");
#endif

    return XMEDIA_SUCCESS;
}

xmedia_s32 hal_isp_init_expander(xmedia_u32 pipe, const xmedia_isp_expander_attr *attr)
{
    return XMEDIA_SUCCESS;
}

xmedia_s32 hal_isp_init(xmedia_u32 pipe, xmedia_isp_config *cfg, xmedia_sensor_black_level *blc)
{
    xmedia_u32 i;
    xmedia_s32 ret;

    if (pipe >= ISP_PIPE_MAX_NUM || cfg == XMEDIA_NULL) {
        return XMEDIA_ERRCODE_INVALID_PARAM;
    }

    memcpy(&g_isp_cfg[pipe], cfg, sizeof(xmedia_isp_config));

    ret = isp_init_buffer(pipe);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    for (i = 0; i < XMEDIA_SENSOR_BAYER_CHN_NUM; i++) {
        g_isp_ctx[pipe].blc[i] = blc->black_level[i];
    }

    isp_reg_init(pipe);

#if ISP_COUNT_SIZE_DEBUG
    puts("g_isp_cfg size = ");
    putdec(sizeof(g_isp_cfg));
    puts(".\n");
    puts("g_isp_reg size = ");
    putdec(sizeof(g_isp_reg));
    puts(".\n");
    puts("g_isp_ctx size = ");
    putdec(sizeof(g_isp_ctx));
    puts(".\n");
#endif

    return XMEDIA_SUCCESS;
}

xmedia_s32 hal_isp_stat(xmedia_u32 pipe, xmedia_isp_statistics *stat)
{
    stat->ae_stat.frame_cnt                        = g_isp_ctx[pipe].frame_cnt;
    stat->ae_stat.ae_stat_global_avg.global_avg[0] = g_isp_ctx[pipe].ae_global_avg.global_avg[0];
    stat->ae_stat.ae_stat_global_avg.global_avg[1] = g_isp_ctx[pipe].ae_global_avg.global_avg[1];
    stat->ae_stat.ae_stat_global_avg.global_avg[2] = g_isp_ctx[pipe].ae_global_avg.global_avg[2];
    stat->ae_stat.ae_stat_global_avg.global_avg[3] = g_isp_ctx[pipe].ae_global_avg.global_avg[3];

    stat->ae_stat.ae_stat_hist.be_histogram_mem_array = g_isp_ctx[pipe].ae_format_hist.be_histogram_mem_array;

    stat->awb_stat.frame_cnt            = g_isp_ctx[pipe].frame_cnt;
    stat->awb_stat.awb_zone_avg_r       = g_isp_ctx[pipe].awb_zone_avg_r;
    stat->awb_stat.awb_zone_avg_g       = g_isp_ctx[pipe].awb_zone_avg_g;
    stat->awb_stat.awb_zone_avg_b       = g_isp_ctx[pipe].awb_zone_avg_b;
    stat->awb_stat.awb_zone_valid_count = g_isp_ctx[pipe].awb_zone_valid_count;
    stat->awb_stat.awb_stat_switch      = XMEDIA_ISP_AWB_SWITCH_AFTER_DG2;
    stat->awb_stat.awb_width            = g_isp_ctx[pipe].awb_zone_width;
    stat->awb_stat.awb_height           = g_isp_ctx[pipe].awb_zone_height;
    stat->awb_stat.awb_zone_row_num     = g_isp_ctx[pipe].awb_zone_row_num;
    stat->awb_stat.awb_zone_col_num     = g_isp_ctx[pipe].awb_zone_col_num;

    return XMEDIA_SUCCESS;
}

xmedia_s32 hal_isp_update(xmedia_u32 pipe, xmedia_isp_result *result, xmedia_sensor_regs_info *reg_info)
{
    xmedia_u32 i;
    xmedia_u32 blc_gain;
    xmedia_u32 dg1_value, dg2_value;

    // sensor 和 ae gain 需要做同步后再配置到寄存器，所以先存放在同步数组里
    // update sensor config
    for (i = ISP_SYNC_BUFF_MAX_NUM - 1; i >= 1; i--) {
        g_isp_ctx[pipe].sync_node[i] = g_isp_ctx[pipe].sync_node[i - 1];
    }

    g_isp_ctx[pipe].sync_node[0] = &g_isp_ctx[pipe].sync_buff[g_isp_ctx[pipe].sync_index];
    g_isp_ctx[pipe].sync_index   = (g_isp_ctx[pipe].sync_index + 1) % ISP_SYNC_BUFF_MAX_NUM;
    memcpy(&g_isp_ctx[pipe].sync_node[0]->sns, reg_info, sizeof(xmedia_sensor_regs_info));

    // update ae config
    blc_gain = 4095 * 256 / ISP_DIV_0_TO_1(4096 - g_isp_ctx[pipe].blc[0]) + 1;
    if (result->ae_result.isp_dgain <= 0x400) {
        dg1_value = result->ae_result.isp_dgain;
        dg2_value = 0x100;
    } else if (result->ae_result.isp_dgain <= 0x1000) {
        dg1_value = 0x400;
        dg2_value = result->ae_result.isp_dgain >> 2;
    } else {
        dg1_value = result->ae_result.isp_dgain >> 2;
        dg2_value = 0x400;
    }

    for (i = 0; i < XMEDIA_ISP_BAYER_PATTERN_NUM; i++) {
        g_isp_ctx[pipe].sync_node[0]->ae.isp_dgain1[0][i] = dg1_value;
    }

    g_isp_ctx[pipe].sync_node[0]->ae.isp_dgain2 = (dg2_value * blc_gain) >> 8;

    // awb gain 不需要做同步，可以直接配置给寄存器
    // update awb config
    g_isp_reg[pipe].dgn3.dgn3_multiplier_4ch[0].bits.reg_dgn3_multiplier_4ch_0 =
        result->awb_result.white_balance_gain[0];
    g_isp_reg[pipe].dgn3.dgn3_multiplier_4ch[0].bits.reg_dgn3_multiplier_4ch_1 =
        result->awb_result.white_balance_gain[1];
    g_isp_reg[pipe].dgn3.dgn3_multiplier_4ch[1].bits.reg_dgn3_multiplier_4ch_0 =
        result->awb_result.white_balance_gain[2];
    g_isp_reg[pipe].dgn3.dgn3_multiplier_4ch[1].bits.reg_dgn3_multiplier_4ch_1 =
        result->awb_result.white_balance_gain[3];

    return XMEDIA_SUCCESS;
}

xmedia_s32 hal_isp_format_statistics(xmedia_u32 pipe)
{
    xmedia_u8        index = g_isp_ctx[pipe].read_index;
    xmedia_u32       i;
    isp_be_awb_zone *awb_zone = XMEDIA_NULL;

    g_isp_ctx[pipe].frame_cnt++;
    // ae只需要全局均值,所以只格式化了全局均值,而不格式化分块均值
    g_isp_ctx[pipe].ae_global_avg.global_avg[0] = g_isp_ctx[pipe].ae_global[index]->rggb_avg_value[0];
    g_isp_ctx[pipe].ae_global_avg.global_avg[1] = g_isp_ctx[pipe].ae_global[index]->rggb_avg_value[1];
    g_isp_ctx[pipe].ae_global_avg.global_avg[2] = g_isp_ctx[pipe].ae_global[index]->rggb_avg_value[2];
    g_isp_ctx[pipe].ae_global_avg.global_avg[3] = g_isp_ctx[pipe].ae_global[index]->rggb_avg_value[3];

    memcpy(g_isp_ctx[pipe].ae_format_hist.be_histogram_mem_array, g_isp_ctx[pipe].ae_hist[index]->hist_value,
                                                                                        sizeof(xmedia_isp_be_hist));

    // awb只需要分块均值,所以只格式化了分块均值,而不格式化全局均值
    for (i = 0; i < g_isp_ctx[pipe].awb_zone_row_num * g_isp_ctx[pipe].awb_zone_col_num; i++) {
        awb_zone                                = &g_isp_ctx[pipe].awb_zone[index][i];
        g_isp_ctx[pipe].awb_zone_avg_r[i]       = awb_zone->rgb_avg_value[0];
        g_isp_ctx[pipe].awb_zone_avg_g[i]       = awb_zone->rgb_avg_value[1];
        g_isp_ctx[pipe].awb_zone_avg_b[i]       = awb_zone->rgb_avg_value[2];
        g_isp_ctx[pipe].awb_zone_valid_count[i] = awb_zone->zone_valid_count;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 hal_isp_config(xmedia_u32 pipe)
{
    xmedia_u8  index = g_isp_ctx[pipe].read_index;

    g_isp_ctx[pipe].read_index = (index + 1) % ISP_STATISTICS_BUFFER_NUM;
    isp_write_reg(ISP_BE_COMM_BASE_ADDR + 0x120, g_isp_ctx[pipe].aac_statistics_buffer[index]);

    isp_write_reg(ISP_BE_DGN1_CH0_BASE_ADDR, g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_hreg000.u32);
    isp_write_reg(ISP_BE_DGN1_CH0_BASE_ADDR + 0x4, g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_digital_gain[0].u32);
    isp_write_reg(ISP_BE_DGN1_CH0_BASE_ADDR + 0x8, g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_digital_gain[1].u32);
    isp_write_reg(ISP_BE_DGN1_CH0_BASE_ADDR + 0xC, g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_blc_in[0].u32);
    isp_write_reg(ISP_BE_DGN1_CH0_BASE_ADDR + 0x10, g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_blc_in[1].u32);
    isp_write_reg(ISP_BE_DGN1_CH0_BASE_ADDR + 0x14, g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_blc_out[0].u32);
    isp_write_reg(ISP_BE_DGN1_CH0_BASE_ADDR + 0x18, g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_blc_out[1].u32);
    isp_write_reg(ISP_BE_DGN1_CH1_BASE_ADDR, g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_hreg000.u32);
    isp_write_reg(ISP_BE_DGN1_CH1_BASE_ADDR + 0x4, g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_digital_gain[0].u32);
    isp_write_reg(ISP_BE_DGN1_CH1_BASE_ADDR + 0x8, g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_digital_gain[1].u32);
    isp_write_reg(ISP_BE_DGN1_CH1_BASE_ADDR + 0xC, g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_blc_in[0].u32);
    isp_write_reg(ISP_BE_DGN1_CH1_BASE_ADDR + 0x10, g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_blc_in[1].u32);
    isp_write_reg(ISP_BE_DGN1_CH1_BASE_ADDR + 0x14, g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_blc_out[0].u32);
    isp_write_reg(ISP_BE_DGN1_CH1_BASE_ADDR + 0x18, g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_blc_out[1].u32);
    isp_write_reg(ISP_BE_DGN2_BASE_ADDR, g_isp_reg[pipe].dgn2.dgn2_hreg000.u32);
    isp_write_reg(ISP_BE_DGN3_BASE_ADDR, g_isp_reg[pipe].dgn3.dgn3_hreg000.u32);
    isp_write_reg(ISP_BE_DGN3_BASE_ADDR + 0x4, g_isp_reg[pipe].dgn3.dgn3_multiplier_4ch[0].u32);
    isp_write_reg(ISP_BE_DGN3_BASE_ADDR + 0x8, g_isp_reg[pipe].dgn3.dgn3_multiplier_4ch[1].u32);

    return XMEDIA_SUCCESS;
}

static isp_sync_cfg *isp_sync_get_sns_cfg(xmedia_u32 pipe, xmedia_u32 index)
{
    xmedia_u8     delay_cfg_frame_num;
    isp_sync_cfg *sync_cfg = g_isp_ctx[pipe].sync_node[0];
    isp_sync_cfg *cur_cfg  = XMEDIA_NULL;
    isp_sync_cfg *pre_cfg  = XMEDIA_NULL;

    if (sync_cfg == XMEDIA_NULL) {
        return XMEDIA_NULL;
    }

    delay_cfg_frame_num = sync_cfg->sns.cfg_to_valid_delay_max - sync_cfg->sns.i2c_data[index].delay_frame_num;
    if (delay_cfg_frame_num > ISP_SYNC_BUFF_MAX_NUM - 2) {
        return XMEDIA_NULL;
    }

    cur_cfg = g_isp_ctx[pipe].sync_node[delay_cfg_frame_num];
    pre_cfg = g_isp_ctx[pipe].sync_node[delay_cfg_frame_num + 1];
    if (cur_cfg == XMEDIA_NULL) {
        return XMEDIA_NULL;
    }

    if (pre_cfg != XMEDIA_NULL && pre_cfg == cur_cfg) {
        return XMEDIA_NULL;
    }

    return cur_cfg;
}

static xmedia_void isp_sync_set_sns_cfg(xmedia_u32 pipe, xmedia_sensor_regs_info *reg_info, xmedia_u32 index)
{
    xmedia_s32 fd;
    xmedia_u8  buf[8];
    xmedia_s32 i;
    xmedia_u32 len = 0;

    fd = i2c_open(g_isp_ctx[pipe].sync_node[0]->sns.com_bus.i2c_dev);
    if (fd < 0) {
        puts("i2c open failed.\n");
        return;
    }

    for (i = reg_info->i2c_data[index].addr_byte_num - 1; i >= 0; i--) {
        buf[len++] = ((reg_info->i2c_data[index].reg_addr >> (i * 8)) & 0xFF);
    }

    for (i = reg_info->i2c_data[index].data_byte_num - 1; i >= 0; i--) {
        buf[len++] = ((reg_info->i2c_data[index].data >> (i * 8)) & 0xFF);
    }

    i2c_write(fd, reg_info->i2c_data[index].dev_addr, buf, len);

    i2c_close(fd);
}

static xmedia_bool isp_is_online_mode()
{
    return XMEDIA_TRUE; // TODO: 待后续完善
}

static xmedia_void isp_sync_get_sync_index(xmedia_u8 cfg_to_valid_delay_max, xmedia_u8 *be_index)
{
    if (isp_is_online_mode()) {
        *be_index = cfg_to_valid_delay_max - 1;
    } else {
        *be_index = cfg_to_valid_delay_max;
    }
}

xmedia_s32 hal_isp_sync(xmedia_u32 pipe)
{
    xmedia_u32    i;
    isp_sync_cfg *sync_cfg = XMEDIA_NULL;
    xmedia_u8     be_index;

    if (g_isp_ctx[pipe].sync_node[0] == XMEDIA_NULL) {
        return XMEDIA_ERRCODE_NULL_PTR;
    }

    for (i = 0; i < g_isp_ctx[pipe].sync_node[0]->sns.reg_num; i++) {
        sync_cfg = isp_sync_get_sns_cfg(pipe, i);
        if (sync_cfg == XMEDIA_NULL) {
            continue;
        }
        if (sync_cfg->sns.i2c_data[i].update == XMEDIA_TRUE) {
            isp_sync_set_sns_cfg(pipe, &sync_cfg->sns, i);
        }
    }

    isp_sync_get_sync_index(g_isp_ctx[pipe].sync_node[0]->sns.cfg_to_valid_delay_max, &be_index);
    sync_cfg = g_isp_ctx[pipe].sync_node[be_index];
    if (sync_cfg != XMEDIA_NULL) {
        g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_digital_gain[0].bits.digital_gain_0 = sync_cfg->ae.isp_dgain1[0][0];
        g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_digital_gain[0].bits.digital_gain_1 = sync_cfg->ae.isp_dgain1[0][1];
        g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_digital_gain[1].bits.digital_gain_0 = sync_cfg->ae.isp_dgain1[0][2];
        g_isp_reg[pipe].dgn1_ch0.dgn1_ch0_digital_gain[1].bits.digital_gain_1 = sync_cfg->ae.isp_dgain1[0][3];
        g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_digital_gain[0].bits.digital_gain_0 = sync_cfg->ae.isp_dgain1[1][0];
        g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_digital_gain[0].bits.digital_gain_1 = sync_cfg->ae.isp_dgain1[1][1];
        g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_digital_gain[1].bits.digital_gain_0 = sync_cfg->ae.isp_dgain1[1][2];
        g_isp_reg[pipe].dgn1_ch1.dgn1_ch1_digital_gain[1].bits.digital_gain_1 = sync_cfg->ae.isp_dgain1[1][3];
        g_isp_reg[pipe].dgn2.dgn2_hreg000.bits.reg_dgn2_multiplier_single     = sync_cfg->ae.isp_dgain2;
    }

    return XMEDIA_SUCCESS;
}