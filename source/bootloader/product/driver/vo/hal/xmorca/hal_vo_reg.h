#ifndef __HAL_VO_REG_H__
#define __HAL_VO_REG_H__
#ifdef VO_KERNEL
#include "xmedia_type.h"
#endif
#ifdef VO_BOOT
#include "drv_vo_comm.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif

/* Define the union u_vout_top_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_ck_dyn_gt_en                    : 1; /* [0]  */
        unsigned int    cfg_ck_dyn_gt_en_dma                : 1; /* [1]  */
        unsigned int    cfg_regup_debug                     : 1; /* [2]  */
        unsigned int    reserved_0                          : 25; /* [27:3]  */
        unsigned int    cfg_testpin_mode                    : 4; /* [31:28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_top_ctrl;

/* Define the union u_vout_top_upd */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_regup_en                        : 1; /* [0]  */
        unsigned int    reserved_0                          : 31; /* [31:1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_top_upd;

/* Define the union u_vout_state */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    top_regup_sta                       : 1; /* [0]  */
        unsigned int    v0_regup_sta                        : 1; /* [1]  */
        unsigned int    g0_regup_sta                        : 1; /* [2]  */
        unsigned int    intf_regup_sta                      : 1; /* [3]  */
        unsigned int    reserved_0                          : 28; /* [31:4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_state;

/* Define the union u_vout_mem_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_rfs_sram_ctrl                   : 16; /* [15:0]  */
        unsigned int    cfg_ras_sram_ctrl                   : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_mem_ctrl;

/* Define the union u_vout_mem_ctrl_1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_rft_sram_ctrl                   : 16; /* [15:0]  */
        unsigned int    cfg_rat_sram_ctrl                   : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_mem_ctrl_1;

/* Define the union u_vout_int_sta */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    intf_vt_int_0_sta                   : 1; /* [0]  */
        unsigned int    intf_vt_int_1_sta                   : 1; /* [1]  */
        unsigned int    intf_vt_int_2_sta                   : 1; /* [2]  */
        unsigned int    intf_vt_int_3_sta                   : 1; /* [3]  */
        unsigned int    reserved_3                          : 12; /* [15:4]  */
        unsigned int    intf_underflow_sta                  : 1; /* [16]  */
        unsigned int    reserved_2                          : 3; /* [19:17]  */
        unsigned int    low_delay_c_err_sta                 : 1; /* [20]  */
        unsigned int    low_delay_y_err_sta                 : 1; /* [21]  */
        unsigned int    reserved_1                          : 2; /* [23:22]  */
        unsigned int    axi_bus_rerr_sta                    : 1; /* [24]  */
        unsigned int    axi_bus_werr_sta                    : 1; /* [25]  */
        unsigned int    intf_mcu_wrerr_sta                  : 1; /* [26]  */
        unsigned int    intf_mcu_operr_sta                  : 1; /* [27]  */
        unsigned int    reserved_0                          : 4; /* [31:28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_int_sta;

/* Define the union u_vout_int_clr */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_intf_vt_int_0_clr               : 1; /* [0]  */
        unsigned int    cfg_intf_vt_int_1_clr               : 1; /* [1]  */
        unsigned int    cfg_intf_vt_int_2_clr               : 1; /* [2]  */
        unsigned int    cfg_intf_vt_int_3_clr               : 1; /* [3]  */
        unsigned int    reserved_3                          : 12; /* [15:4]  */
        unsigned int    cfg_intf_underflow_clr              : 1; /* [16]  */
        unsigned int    reserved_2                          : 3; /* [19:17]  */
        unsigned int    cfg_low_delay_c_err_clr             : 1; /* [20]  */
        unsigned int    cfg_low_delay_y_err_clr             : 1; /* [21]  */
        unsigned int    reserved_1                          : 2; /* [23:22]  */
        unsigned int    cfg_axi_bus_rerr_clr                : 1; /* [24]  */
        unsigned int    cfg_axi_bus_werr_clr                : 1; /* [25]  */
        unsigned int    cfg_intf_mcu_wrerr_clr              : 1; /* [26]  */
        unsigned int    cfg_intf_mcu_operr_clr              : 1; /* [27]  */
        unsigned int    reserved_0                          : 4; /* [31:28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_int_clr;

/* Define the union u_vout_int_msk */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_intf_vt_int_0_msk               : 1; /* [0]  */
        unsigned int    cfg_intf_vt_int_1_msk               : 1; /* [1]  */
        unsigned int    cfg_intf_vt_int_2_msk               : 1; /* [2]  */
        unsigned int    cfg_intf_vt_int_3_msk               : 1; /* [3]  */
        unsigned int    reserved_3                          : 12; /* [15:4]  */
        unsigned int    cfg_intf_underflow_msk              : 1; /* [16]  */
        unsigned int    reserved_2                          : 3; /* [19:17]  */
        unsigned int    cfg_low_delay_c_err_msk             : 1; /* [20]  */
        unsigned int    cfg_low_delay_y_err_msk             : 1; /* [21]  */
        unsigned int    reserved_1                          : 2; /* [23:22]  */
        unsigned int    cfg_axi_bus_rerr_msk                : 1; /* [24]  */
        unsigned int    cfg_axi_bus_werr_msk                : 1; /* [25]  */
        unsigned int    cfg_intf_mcu_wrerr_msk              : 1; /* [26]  */
        unsigned int    cfg_intf_mcu_operr_msk              : 1; /* [27]  */
        unsigned int    reserved_0                          : 4; /* [31:28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_int_msk;

/* Define the union u_vout_int_src_sta */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    intf_vt_int_0_src_sta               : 1; /* [0]  */
        unsigned int    intf_vt_int_1_src_sta               : 1; /* [1]  */
        unsigned int    intf_vt_int_2_src_sta               : 1; /* [2]  */
        unsigned int    intf_vt_int_3_src_sta               : 1; /* [3]  */
        unsigned int    reserved_3                          : 12; /* [15:4]  */
        unsigned int    intf_underflow_src_sta              : 1; /* [16]  */
        unsigned int    reserved_2                          : 3; /* [19:17]  */
        unsigned int    low_delay_c_err_src_sta             : 1; /* [20]  */
        unsigned int    low_delay_y_err_src_sta             : 1; /* [21]  */
        unsigned int    reserved_1                          : 2; /* [23:22]  */
        unsigned int    axi_bus_rerr_src_sta                : 1; /* [24]  */
        unsigned int    axi_bus_werr_src_sta                : 1; /* [25]  */
        unsigned int    intf_mcu_wrerr_src_sta              : 1; /* [26]  */
        unsigned int    intf_mcu_operr_src_sta              : 1; /* [27]  */
        unsigned int    reserved_0                          : 4; /* [31:28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_int_src_sta;

/* Define the union u_vout_sta_clr */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_uf_clr                          : 1; /* [0]  */
        unsigned int    reserved_0                          : 31; /* [31:1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_sta_clr;

/* Define the union u_vout_debug */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rdma_busy_v0_y                      : 1; /* [0]  */
        unsigned int    rdma_busy_v0_c                      : 1; /* [1]  */
        unsigned int    rdma1_busy_v0_y                     : 1; /* [2]  */
        unsigned int    rdma1_busy_v0_c                     : 1; /* [3]  */
        unsigned int    rdma_busy_g0                        : 1; /* [4]  */
        unsigned int    reserved_0                          : 27; /* [31:5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_debug;

/* Define the union u_vout_dma_dfx_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_axi_sta_enable                  : 1; /* [0]  */
        unsigned int    reserved_5                          : 1; /* [1]  */
        unsigned int    cfg_rd_latency_enable               : 1; /* [2]  */
        unsigned int    reserved_4                          : 1; /* [3]  */
        unsigned int    cfg_rd_bw_enable                    : 1; /* [4]  */
        unsigned int    reserved_3                          : 7; /* [11:5]  */
        unsigned int    cfg_rd_latency_ch_sel               : 4; /* [15:12]  */
        unsigned int    reserved_2                          : 4; /* [19:16]  */
        unsigned int    cfg_rd_bw_ch_sel                    : 4; /* [23:20]  */
        unsigned int    reserved_1                          : 4; /* [27:24]  */
        unsigned int    cfg_rd_bw_window_num_pow            : 3; /* [30:28]  */
        unsigned int    reserved_0                          : 1; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_dma_dfx_ctrl;

/* Define the union u_vout_dma_dfx_rd_1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dfx_axi_rd_ost_sum_h                : 8; /* [7:0]  */
        unsigned int    dfx_axi_rd_ost_max                  : 8; /* [15:8]  */
        unsigned int    reserved_0                          : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_dma_dfx_rd_1;

/* Define the union u_vout_dma_dfx_rd_3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dfx_rd_latency_max                  : 16; /* [15:0]  */
        unsigned int    dfx_rd_latency_min                  : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_dma_dfx_rd_3;

/* Define the union u_vout_dma_dfx_rd_4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dfx_rd_latency_sum_h                : 16; /* [15:0]  */
        unsigned int    reserved_0                          : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_dma_dfx_rd_4;

/* Define the union u_vout_cbm_mixer */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mixer_prio0                     : 2; /* [1:0]  */
        unsigned int    reserved_1                          : 2; /* [3:2]  */
        unsigned int    cfg_mixer_prio1                     : 2; /* [5:4]  */
        unsigned int    reserved_0                          : 26; /* [31:6]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_cbm_mixer;

/* Define the union u_vout_dma_ost_num */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_axi_rd_ost_num                  : 5; /* [4:0]  */
        unsigned int    reserved_0                          : 27; /* [31:5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_dma_ost_num;

/* Define the union u_vout_para_update */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_para_0_update                   : 1; /* [0]  */
        unsigned int    reserved_1                          : 1; /* [1]  */
        unsigned int    cfg_para_2_update                   : 1; /* [2]  */
        unsigned int    reserved_0                          : 29; /* [31:3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_para_update;

/* Define the union u_vout_para_0_addr_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_para_0_addr_h                   : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_para_0_addr_h;

/* Define the union u_vout_para_0_addr_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_para_0_addr_l                   : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_para_0_addr_l;

/* Define the union u_vout_para_2_addr_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_para_2_addr_h                   : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_para_2_addr_h;

/* Define the union u_vout_para_2_addr_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_para_2_addr_l                   : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_para_2_addr_l;

/* Define the union u_vout_v0_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_layer_en                        : 1; /* [0 : 0]  */
        unsigned int    cfg_regup_mode                      : 1; /* [1 : 1]  */
        unsigned int    cfg_regup_field                     : 1; /* [2 : 2]  */
        unsigned int    reserved_3                          : 1; /* [3 : 3]  */
        unsigned int    cfg_src_fmt                         : 3; /* [6 : 4]  */
        unsigned int    cfg_src_uv_order                    : 1; /* [7 : 7]  */
        unsigned int    reserved_2                          : 8; /* [15 : 8]  */
        unsigned int    cfg_blend_mode                      : 1; /* [16 : 16]  */
        unsigned int    reserved_1                          : 3; /* [19 : 17]  */
        unsigned int    cfg_galpha_en                       : 1; /* [20 : 20]  */
        unsigned int    reserved_0                          : 3; /* [23 : 21]  */
        unsigned int    cfg_galpha                          : 8; /* [31 : 24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_ctrl;

/* Define the union u_vout_v0_upd */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_regup_en                        : 1; /* [0 : 0]  */
        unsigned int    reserved_0                          : 31; /* [31 : 1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_upd;

/* Define the union u_vout_v0_bk */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_bk_color                        : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_bk;

/* Define the union u_vout_v0_mute_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_2                          : 4; /* [3 : 0]  */
        unsigned int    cfg_mute_mode                       : 1; /* [4 : 4]  */
        unsigned int    reserved_1                          : 3; /* [7 : 5]  */
        unsigned int    cfg_mute_pat                        : 1; /* [8 : 8]  */
        unsigned int    reserved_0                          : 7; /* [15 : 9]  */
        unsigned int    cfg_checker_h                       : 8; /* [23 : 16]  */
        unsigned int    cfg_checker_v                       : 8; /* [31 : 24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mute_ctrl;

/* Define the union u_vout_v0_oreso */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    out_h                               : 16; /* [15 : 0]  */
        unsigned int    out_v                               : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_oreso;

/* Define the union u_vout_v0_src_pos */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_src_xpos                        : 16; /* [15 : 0]  */
        unsigned int    cfg_src_ypos                        : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_src_pos;

/* Define the union u_vout_v0_src */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_src_h                           : 16; /* [15 : 0]  */
        unsigned int    cfg_src_v                           : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_src;

/* Define the union u_vout_v0_mrg_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_en                         : 1; /* [0]  */
        unsigned int    cfg_mrg1_en                         : 1; /* [1]  */
        unsigned int    reserved_2                          : 6; /* [7:2]  */
        unsigned int    cfg_mrg0_mute_en                    : 1; /* [8]  */
        unsigned int    cfg_mrg1_mute_en                    : 1; /* [9]  */
        unsigned int    reserved_1                          : 14; /* [23:10]  */
        unsigned int    cfg_mixer_prio                      : 1; /* [24]  */
        unsigned int    reserved_0                          : 7; /* [31:25]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg_ctrl;

/* Define the union u_vout_v0_mrg0_addr_y_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_src_addr_y_h               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg0_addr_y_h;

/* Define the union u_vout_v0_mrg0_addr_y_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_src_addr_y_l               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg0_addr_y_l;

/* Define the union u_vout_v0_mrg0_stride_y */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_src_stride_y               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg0_stride_y;

/* Define the union u_vout_v0_mrg0_addr_c_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_src_addr_c_h               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg0_addr_c_h;

/* Define the union u_vout_v0_mrg0_addr_c_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_src_addr_c_l               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg0_addr_c_l;

/* Define the union u_vout_v0_mrg0_stride_c */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_src_stride_c               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg0_stride_c;

/* Define the union u_vout_v0_mrg0_src_pos */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_src_xpos                   : 16; /* [15 : 0]  */
        unsigned int    cfg_mrg0_src_ypos                   : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg0_src_pos;

/* Define the union u_vout_v0_mrg0_src */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_src_h                      : 16; /* [15 : 0]  */
        unsigned int    cfg_mrg0_src_v                      : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg0_src;

/* Define the union u_vout_v0_mrg0_mute_color */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_mute_color                 : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg0_mute_color;

/* Define the union u_vout_v0_mrg1_addr_y_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_src_addr_y_h               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg1_addr_y_h;

/* Define the union u_vout_v0_mrg1_addr_y_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_src_addr_y_l               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg1_addr_y_l;

/* Define the union u_vout_v0_mrg1_stride_y */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_src_stride_y               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg1_stride_y;

/* Define the union u_vout_v0_mrg1_addr_c_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_src_addr_c_h               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg1_addr_c_h;

/* Define the union u_vout_v0_mrg1_addr_c_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_src_addr_c_l               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg1_addr_c_l;

/* Define the union u_vout_v0_mrg1_stride_c */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_src_stride_c               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg1_stride_c;

/* Define the union u_vout_v0_mrg1_src_pos */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_src_xpos                   : 16; /* [15 : 0]  */
        unsigned int    cfg_mrg1_src_ypos                   : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg1_src_pos;

/* Define the union u_vout_v0_mrg1_src */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_src_h                      : 16; /* [15 : 0]  */
        unsigned int    cfg_mrg1_src_v                      : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg1_src;

/* Define the union u_vout_v0_mrg1_mute_color */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_mute_color                 : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg1_mute_color;

/* Define the union u_vout_v0_vcus_coef */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_vcus_coef_0                     : 7; /* [6:0]  */
        unsigned int    reserved_1                          : 1; /* [7]  */
        unsigned int    cfg_vcus_coef_1                     : 7; /* [14:8]  */
        unsigned int    reserved_0                          : 17; /* [31:15]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_vcus_coef;

/* Define the union u_vout_v0_hcus_coef */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_hcus_coef_0                     : 7; /* [6:0]  */
        unsigned int    reserved_1                          : 1; /* [7]  */
        unsigned int    cfg_hcus_coef_1                     : 7; /* [14:8]  */
        unsigned int    reserved_0                          : 17; /* [31:15]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_hcus_coef;

/* Define the union u_vout_v0_mrg0_csc_offset_in */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_csc_offset_in_0            : 9; /* [8:0]  */
        unsigned int    cfg_mrg0_csc_offset_in_1            : 9; /* [17:9]  */
        unsigned int    cfg_mrg0_csc_offset_in_2            : 9; /* [26:18]  */
        unsigned int    cfg_csc_mode                        : 1; /* [27]  */
        unsigned int    reserved_0                          : 3; /* [30:28]  */
        unsigned int    cfg_csc_en                          : 1; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg0_csc_offset_in;

/* Define the union u_vout_v0_mrg0_csc_matrix_01 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_csc_matrix_0               : 16; /* [15:0]  */
        unsigned int    cfg_mrg0_csc_matrix_1               : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg0_csc_matrix_01;

/* Define the union u_vout_v0_mrg0_csc_matrix_23 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_csc_matrix_2               : 16; /* [15:0]  */
        unsigned int    cfg_mrg0_csc_matrix_3               : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg0_csc_matrix_23;

/* Define the union u_vout_v0_mrg0_csc_matrix_45 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_csc_matrix_4               : 16; /* [15:0]  */
        unsigned int    cfg_mrg0_csc_matrix_5               : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg0_csc_matrix_45;

/* Define the union u_vout_v0_mrg0_csc_matrix_67 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_csc_matrix_6               : 16; /* [15:0]  */
        unsigned int    cfg_mrg0_csc_matrix_7               : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg0_csc_matrix_67;

/* Define the union u_vout_v0_mrg0_csc_matrix_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_csc_matrix_8               : 16; /* [15:0]  */
        unsigned int    reserved_0                          : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg0_csc_matrix_8;

/* Define the union u_vout_v0_mrg0_csc_offset_out */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_csc_offset_out_0           : 9; /* [8:0]  */
        unsigned int    reserved_2                          : 1; /* [9]  */
        unsigned int    cfg_mrg0_csc_offset_out_1           : 9; /* [18:10]  */
        unsigned int    reserved_1                          : 1; /* [19]  */
        unsigned int    cfg_mrg0_csc_offset_out_2           : 9; /* [28:20]  */
        unsigned int    reserved_0                          : 3; /* [31:29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg0_csc_offset_out;

/* Define the union u_vout_v0_mrg1_csc_offset_in */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_csc_offset_in_0            : 9; /* [8:0]  */
        unsigned int    cfg_mrg1_csc_offset_in_1            : 9; /* [17:9]  */
        unsigned int    cfg_mrg1_csc_offset_in_2            : 9; /* [26:18]  */
        unsigned int    reserved_0                          : 5; /* [31:27]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg1_csc_offset_in;

/* Define the union u_vout_v0_mrg1_csc_matrix_01 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_csc_matrix_0               : 16; /* [15:0]  */
        unsigned int    cfg_mrg1_csc_matrix_1               : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg1_csc_matrix_01;

/* Define the union u_vout_v0_mrg1_csc_matrix_23 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_csc_matrix_2               : 16; /* [15:0]  */
        unsigned int    cfg_mrg1_csc_matrix_3               : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg1_csc_matrix_23;

/* Define the union u_vout_v0_mrg1_csc_matrix_45 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_csc_matrix_4               : 16; /* [15:0]  */
        unsigned int    cfg_mrg1_csc_matrix_5               : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg1_csc_matrix_45;

/* Define the union u_vout_v0_mrg1_csc_matrix_67 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_csc_matrix_6               : 16; /* [15:0]  */
        unsigned int    cfg_mrg1_csc_matrix_7               : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg1_csc_matrix_67;

/* Define the union u_vout_v0_mrg1_csc_matrix_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_csc_matrix_8               : 16; /* [15:0]  */
        unsigned int    reserved_0                          : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg1_csc_matrix_8;

/* Define the union u_vout_v0_mrg1_csc_offset_out */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_csc_offset_out_0           : 9; /* [8:0]  */
        unsigned int    reserved_2                          : 1; /* [9]  */
        unsigned int    cfg_mrg1_csc_offset_out_1           : 9; /* [18:10]  */
        unsigned int    reserved_1                          : 1; /* [19]  */
        unsigned int    cfg_mrg1_csc_offset_out_2           : 9; /* [28:20]  */
        unsigned int    reserved_0                          : 3; /* [31:29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg1_csc_offset_out;

/* Define the union u_vout_v0_lba0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_lba0_color                      : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_lba0;

/* Define the union u_vout_v0_lba1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_lba1_color                      : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_lba1;

/* Define the union u_vout_v0_mrg0_disp_pos */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_disp_xpos                  : 16; /* [15 : 0]  */
        unsigned int    cfg_mrg0_disp_ypos                  : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg0_disp_pos;

/* Define the union u_vout_v0_mrg0_disp */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_disp_h                     : 16; /* [15 : 0]  */
        unsigned int    cfg_mrg0_disp_v                     : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg0_disp;

/* Define the union u_vout_v0_mrg1_disp_pos */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_disp_xpos                  : 16; /* [15 : 0]  */
        unsigned int    cfg_mrg1_disp_ypos                  : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg1_disp_pos;

/* Define the union u_vout_v0_mrg1_disp */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_disp_h                     : 16; /* [15 : 0]  */
        unsigned int    cfg_mrg1_disp_v                     : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg1_disp;

/* Define the union u_vout_intf_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_intf_en                         : 1; /* [0 : 0]  */
        unsigned int    reserved_1                          : 3; /* [3 : 1]  */
        unsigned int    cfg_intf_sel                        : 3; /* [6 : 4]  */
        unsigned int    cfg_interlaced                      : 1; /* [7 : 7]  */
        unsigned int    cfg_clock_edge_sel                  : 1; /* [8 : 8]  */
        unsigned int    cfg_field_inv                       : 1; /* [9 : 9]  */
        unsigned int    cfg_vs_inv                          : 1; /* [10 : 10]  */
        unsigned int    cfg_hs_inv                          : 1; /* [11 : 11]  */
        unsigned int    cfg_de_inv                          : 1; /* [12 : 12]  */
        unsigned int    cfg_hs_mode                         : 2; /* [14 : 13]  */
        unsigned int    reserved_0                          : 16; /* [30 : 15]  */
        unsigned int    cfg_chk_sum_en                      : 1; /* [31 : 31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_ctrl;

/* Define the union u_vout_intf_upd */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_regup_en                        : 1; /* [0 : 0]  */
        unsigned int    reserved_0                          : 31; /* [31 : 1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_upd;

/* Define the union u_vout_intf_regup_pos */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_regup_pos                       : 16; /* [15 : 0]  */
        unsigned int    cfg_para_pos                        : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_regup_pos;

/* Define the union u_vout_intf_start_pos */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_prefetch_pos                    : 16; /* [15 : 0]  */
        unsigned int    cfg_start_pos                       : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_start_pos;

/* Define the union u_vout_intf_act */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_vact                            : 16; /* [15 : 0]  */
        unsigned int    cfg_hact                            : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_act;

/* Define the union u_vout_intf_vfb */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_vfb                             : 16; /* [15 : 0]  */
        unsigned int    cfg_vbb                             : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_vfb;

/* Define the union u_vout_intf_bvfb */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_bvfb                            : 16; /* [15 : 0]  */
        unsigned int    cfg_bvbb                            : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_bvfb;

/* Define the union u_vout_intf_hfb */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_hfb                             : 16; /* [15 : 0]  */
        unsigned int    cfg_hbb                             : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_hfb;

/* Define the union u_vout_intf_hs_w */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_hs_width                        : 16; /* [15 : 0]  */
        unsigned int    reserved_0                          : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_hs_w;

/* Define the union u_vout_intf_field */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_field_tog                       : 16; /* [15 : 0]  */
        unsigned int    cfg_bfield_tog                      : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_field;

/* Define the union u_vout_intf_vs_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_vs_st_h_field0                  : 16; /* [15 : 0]  */
        unsigned int    cfg_vs_end_h_field0                 : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_vs_h;

/* Define the union u_vout_intf_vs_v */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_vs_st_v_field0                  : 16; /* [15 : 0]  */
        unsigned int    cfg_vs_end_v_field0                 : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_vs_v;

/* Define the union u_vout_intf_bvs_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_vs_st_h_field1                  : 16; /* [15 : 0]  */
        unsigned int    cfg_vs_end_h_field1                 : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_bvs_h;

/* Define the union u_vout_intf_bvs_v */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_vs_st_v_field1                  : 16; /* [15 : 0]  */
        unsigned int    cfg_vs_end_v_field1                 : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_bvs_v;

/* Define the union u_vout_intf_sfifo_thd */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_intf_sfifo_thd                  : 16; /* [15 : 0]  */
        unsigned int    reserved_0                          : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_sfifo_thd;

/* Define the union u_vout_intf_vt_int */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_vt_int_mode_0                   : 1; /* [0 : 0]  */
        unsigned int    cfg_vt_int_mode_1                   : 1; /* [1 : 1]  */
        unsigned int    cfg_vt_int_mode_2                   : 1; /* [2 : 2]  */
        unsigned int    cfg_vt_int_mode_3                   : 1; /* [3 : 3]  */
        unsigned int    reserved_1                          : 4; /* [7 : 4]  */
        unsigned int    cfg_vt_int_cnt_sel_0                : 1; /* [8 : 8]  */
        unsigned int    cfg_vt_int_cnt_sel_1                : 1; /* [9 : 9]  */
        unsigned int    cfg_vt_int_cnt_sel_2                : 1; /* [10 : 10]  */
        unsigned int    cfg_vt_int_cnt_sel_3                : 1; /* [11 : 11]  */
        unsigned int    reserved_0                          : 20; /* [31 : 12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_vt_int;

/* Define the union u_vout_intf_vt_intf_thd0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_vt_int_thd_0                    : 16; /* [15 : 0]  */
        unsigned int    cfg_vt_int_thd_1                    : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_vt_intf_thd0;

/* Define the union u_vout_intf_vt_intf_thd2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_vt_int_thd_2                    : 16; /* [15 : 0]  */
        unsigned int    cfg_vt_int_thd_3                    : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_vt_intf_thd2;

/* Define the union u_vout_intf_sta */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vstate                              : 2; /* [1 : 0]  */
        unsigned int    reserved_1                          : 2; /* [3 : 2]  */
        unsigned int    field_flag_ppc                      : 1; /* [4 : 4]  */
        unsigned int    reserved_0                          : 11; /* [15 : 5]  */
        unsigned int    frm_cnt                             : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_sta;

/* Define the union u_vout_intf_sta2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    vcnt_st_vblk                        : 16; /* [15 : 0]  */
        unsigned int    vcnt_st_vact                        : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_sta2;

/* Define the union u_vout_intf_uf_line */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    intf_uf_line                        : 16; /* [15 : 0]  */
        unsigned int    reserved_0                          : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_uf_line;

/* Define the union u_vout_intf_y_sum */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    intf_y_sum                          : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_y_sum;

/* Define the union u_vout_intf_u_sum */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    intf_u_sum                          : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_u_sum;

/* Define the union u_vout_intf_v_sum */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    intf_v_sum                          : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_v_sum;

/* Define the union u_vout_intf_bt_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_bt_mode                         : 1; /* [0 : 0]  */
        unsigned int    reserved_3                          : 3; /* [3 : 1]  */
        unsigned int    cfg_bt_yc_order                     : 1; /* [4 : 4]  */
        unsigned int    cfg_bt_uv_order                     : 1; /* [5 : 5]  */
        unsigned int    reserved_2                          : 2; /* [7 : 6]  */
        unsigned int    cfg_vbit_mode                       : 1; /* [8 : 8]  */
        unsigned int    reserved_1                          : 3; /* [11 : 9]  */
        unsigned int    cfg_bt_bit_inv                      : 1; /* [12 : 12]  */
        unsigned int    reserved_0                          : 19; /* [31 : 13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_bt_ctrl;

/* Define the union u_vout_intf_bt_vbit */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_vbit_st                         : 16; /* [15 : 0]  */
        unsigned int    cfg_vbit_end                        : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_bt_vbit;

/* Define the union u_vout_intf_bt_bvbit */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_bvbit_st                        : 16; /* [15 : 0]  */
        unsigned int    cfg_bvbit_end                       : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_bt_bvbit;

/* Define the union u_vout_intf_lcd_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_lcd_mode                        : 3; /* [2 : 0]  */
        unsigned int    reserved_3                          : 1; /* [3 : 3]  */
        unsigned int    cfg_lcd_rgb_order_0                 : 3; /* [6 : 4]  */
        unsigned int    reserved_2                          : 1; /* [7 : 7]  */
        unsigned int    cfg_lcd_rgb_order_1                 : 3; /* [10 : 8]  */
        unsigned int    reserved_1                          : 1; /* [11 : 11]  */
        unsigned int    cfg_lcd_bit_inv                     : 1; /* [12 : 12]  */
        unsigned int    reserved_0                          : 19; /* [31 : 13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_lcd_ctrl;

/* Define the union u_vout_intf_mute_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mute_en                         : 1; /* [0 : 0]  */
        unsigned int    reserved_3                          : 3; /* [3 : 1]  */
        unsigned int    cfg_mute_mode                       : 1; /* [4 : 4]  */
        unsigned int    reserved_2                          : 3; /* [7 : 5]  */
        unsigned int    cfg_mute_pat                        : 2; /* [9 : 8]  */
        unsigned int    reserved_1                          : 2; /* [11 : 10]  */
        unsigned int    cfg_cbar_sel                        : 1; /* [12 : 12]  */
        unsigned int    reserved_0                          : 3; /* [15 : 13]  */
        unsigned int    cfg_checker_h                       : 8; /* [23 : 16]  */
        unsigned int    cfg_checker_v                       : 8; /* [31 : 24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_mute_ctrl;

/* Define the union u_vout_intf_mute_color */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mute_color                      : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_mute_color;

/* Define the union u_vout_intf_mute_bk */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_bk_color                        : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_mute_bk;

/* Define the union u_vout_intf_ink_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_ink_en                          : 1; /* [0 : 0]  */
        unsigned int    reserved_2                          : 3; /* [3 : 1]  */
        unsigned int    cfg_cross_en                        : 1; /* [4 : 4]  */
        unsigned int    reserved_1                          : 3; /* [7 : 5]  */
        unsigned int    cfg_cross_color_mode                : 1; /* [8 : 8]  */
        unsigned int    reserved_0                          : 23; /* [31 : 9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_ink_ctrl;

/* Define the union u_vout_intf_ink_pos */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_x_pos                           : 16; /* [15 : 0]  */
        unsigned int    cfg_y_pos                           : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_ink_pos;

/* Define the union u_vout_intf_ink_color */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_cross_color                     : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_ink_color;

/* Define the union u_vout_intf_ink_data */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ink_data                            : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_ink_data;

/* Define the union u_vout_intf_clip_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_clip_yuv_l                      : 30; /* [29 : 0]  */
        unsigned int    reserved_0                          : 1; /* [30 : 30]  */
        unsigned int    cfg_clip_en                         : 1; /* [31 : 31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_clip_l;

/* Define the union u_vout_intf_clip_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_clip_yuv_h                      : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_clip_h;

/* Define the union u_vout_intf_mcu_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mcu_sync_type                   : 1; /* [0]  */
        unsigned int    cfg_mcu_wrram_en                    : 1; /* [1]  */
        unsigned int    cfg_mcu_wr_fmt                      : 2; /* [3:2]  */
        unsigned int    reserved_0                          : 28; /* [31:4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_mcu_ctrl;

/* Define the union u_vout_intf_mcu_wrram_cmd */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mcu_wrram_cmd                   : 8; /* [7:0]  */
        unsigned int    reserved_0                          : 24; /* [31:8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_mcu_wrram_cmd;

/* Define the union u_vout_intf_mcu_wrramc_cmd */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mcu_wrramc_cmd                  : 8; /* [7:0]  */
        unsigned int    reserved_0                          : 24; /* [31:8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_mcu_wrramc_cmd;

/* Define the union u_vout_intf_hcds_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_hcds_en                         : 1; /* [0]  */
        unsigned int    cfg_hcds_mode                       : 1; /* [1]  */
        unsigned int    reserved_0                          : 30; /* [31:2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_hcds_ctrl;

/* Define the union u_vout_intf_hcds_coef */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_hcds_coef_0                     : 8; /* [7:0]  */
        unsigned int    cfg_hcds_coef_1                     : 8; /* [15:8]  */
        unsigned int    cfg_hcds_coef_2                     : 8; /* [23:16]  */
        unsigned int    cfg_hcds_coef_3                     : 8; /* [31:24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_hcds_coef;

/* Define the union u_vout_intf_dither_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_dither_en                       : 1; /* [0]  */
        unsigned int    cfg_gamma_en                        : 1; /* [1]  */
        unsigned int    reserved_1                          : 6; /* [7:2]  */
        unsigned int    cfg_dither_thd_l                    : 8; /* [15:8]  */
        unsigned int    cfg_dither_thd_h                    : 8; /* [23:16]  */
        unsigned int    reserved_0                          : 8; /* [31:24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_dither_ctrl;

/* Define the union u_vout_intf_gammalut_rd_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_gamma_rd_en                     : 1; /* [0 : 0]  */
        unsigned int    reserved_1                          : 7; /* [7 : 1]  */
        unsigned int    cfg_gamma_rd_addr                   : 8; /* [15 : 8]  */
        unsigned int    reserved_0                          : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_gammalut_rd_ctrl;

/* Define the union u_vout_intf_gammalut_rd_data */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    gamma_rd_data                       : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_gammalut_rd_data;

/* Define the union u_vout_mcu_screen_ctrl_0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mcu_mode                        : 8; /* [7:0]  */
        unsigned int    cfg_mcu_rst_ctrl                    : 1; /* [8]  */
        unsigned int    reserved_0                          : 23; /* [31:9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_mcu_screen_ctrl_0;

/* Define the union u_vout_mcu_screen_ctrl_1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mcu_cmd_stop                    : 1; /* [0]  */
        unsigned int    cfg_mcu_wrcmd_start                 : 1; /* [1]  */
        unsigned int    cfg_mcu_para_rd                     : 1; /* [2]  */
        unsigned int    cfg_mcu_para_raddr                  : 8; /* [10:3]  */
        unsigned int    reserved_0                          : 21; /* [31:11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_mcu_screen_ctrl_1;

/* Define the union u_vout_mcu_screen_ctrl_2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mcu_cmd_mode                    : 3; /* [2:0]  */
        unsigned int    reserved_1                          : 5; /* [7:3]  */
        unsigned int    cfg_mcu_wrcmd_size                  : 8; /* [15:8]  */
        unsigned int    reserved_0                          : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_mcu_screen_ctrl_2;

/* Define the union u_vout_mcu_screen_state */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mcu_wr_cmd_state                    : 2; /* [1:0]  */
        unsigned int    mcu_rd_cmd_state                    : 1; /* [2]  */
        unsigned int    mcu_rd_pixel_state                  : 1; /* [3]  */
        unsigned int    reserved_0                          : 28; /* [31:4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_mcu_screen_state;

/* Define the union u_vout_mcu_screen_para_rdata_0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mcu_rd_para_0                       : 8; /* [7:0]  */
        unsigned int    mcu_rd_para_1                       : 8; /* [15:8]  */
        unsigned int    mcu_rd_para_2                       : 8; /* [23:16]  */
        unsigned int    mcu_rd_para_3                       : 8; /* [31:24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_mcu_screen_para_rdata_0;

/* Define the union u_vout_mcu_screen_para_rdata_1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mcu_rd_para_4                       : 8; /* [7:0]  */
        unsigned int    mcu_rd_para_5                       : 8; /* [15:8]  */
        unsigned int    mcu_rd_para_6                       : 8; /* [23:16]  */
        unsigned int    mcu_rd_para_7                       : 8; /* [31:24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_mcu_screen_para_rdata_1;

/* Define the union u_vout_mcu_screen_para_rdata_2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mcu_rd_para_8                       : 8; /* [7:0]  */
        unsigned int    mcu_rd_para_9                       : 8; /* [15:8]  */
        unsigned int    mcu_rd_para_10                      : 8; /* [23:16]  */
        unsigned int    mcu_rd_para_11                      : 8; /* [31:24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_mcu_screen_para_rdata_2;

/* Define the union u_vout_mcu_screen_para_rdata_3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mcu_rd_para_12                      : 8; /* [7:0]  */
        unsigned int    mcu_rd_para_13                      : 8; /* [15:8]  */
        unsigned int    mcu_rd_para_14                      : 8; /* [23:16]  */
        unsigned int    mcu_rd_para_15                      : 8; /* [31:24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_mcu_screen_para_rdata_3;

//==============================================================================
/* Define the global struct */
typedef struct
{
    volatile u_vout_top_ctrl                      vout_top_ctrl                 ;/*0x0*/
    volatile u_vout_top_upd                       vout_top_upd                  ;/*0x4*/
    volatile u_vout_state                         vout_state                    ;/*0x8*/
    volatile xmedia_u32                           reserved0                     ;/*0xc*/
    volatile xmedia_u32                           vout_version_0                ;/*0x10*/
    volatile xmedia_u32                           vout_version_1                ;/*0x14*/
    volatile u_vout_mem_ctrl                      vout_mem_ctrl                 ;/*0x18*/
    volatile u_vout_mem_ctrl_1                    vout_mem_ctrl_1               ;/*0x1c*/
    volatile u_vout_int_sta                       vout_int_sta                  ;/*0x20*/
    volatile u_vout_int_clr                       vout_int_clr                  ;/*0x24*/
    volatile u_vout_int_msk                       vout_int_msk                  ;/*0x28*/
    volatile u_vout_int_src_sta                   vout_int_src_sta              ;/*0x2c*/
    volatile xmedia_u32                           reserved1[4]                  ;/*0x30*/
    volatile u_vout_sta_clr                       vout_sta_clr                  ;/*0x40*/
    volatile u_vout_debug                         vout_debug                    ;/*0x44*/
    volatile xmedia_u32                           reserved2[18]                 ;/*0x48*/
    volatile u_vout_dma_dfx_ctrl                  vout_dma_dfx_ctrl             ;/*0x90*/
    volatile xmedia_u32                           reserved3[15]                 ;/*0x94*/
    volatile xmedia_u32                           vout_dma_dfx_rd_0             ;/*0xd0*/
    volatile u_vout_dma_dfx_rd_1                  vout_dma_dfx_rd_1             ;/*0xd4*/
    volatile xmedia_u32                           vout_dma_dfx_rd_2             ;/*0xd8*/
    volatile u_vout_dma_dfx_rd_3                  vout_dma_dfx_rd_3             ;/*0xdc*/
    volatile u_vout_dma_dfx_rd_4                  vout_dma_dfx_rd_4             ;/*0xe0*/
    volatile xmedia_u32                           vout_dma_dfx_rd_5             ;/*0xe4*/
    volatile xmedia_u32                           vout_dma_dfx_rd_6             ;/*0xe8*/
    volatile xmedia_u32                           vout_dma_dfx_rd_7             ;/*0xec*/
    volatile xmedia_u32                           vout_dma_dfx_rd_8             ;/*0xf0*/
    volatile xmedia_u32                           vout_dma_dfx_rd_9             ;/*0xf4*/
    volatile xmedia_u32                           vout_dma_dfx_rd_10            ;/*0xf8*/
    volatile xmedia_u32                           reserved4                     ;/*0xfc*/
    volatile u_vout_cbm_mixer                     vout_cbm_mixer                ;/*0x100*/
    volatile xmedia_u32                           vout_cbm_bk                   ;/*0x104*/
    volatile xmedia_u32                           reserved5[32]                 ;/*0x108*/
    volatile u_vout_dma_ost_num                   vout_dma_ost_num              ;/*0x188*/
    volatile xmedia_u32                           reserved6[29]                 ;/*0x18c*/
    volatile u_vout_para_update                   vout_para_update              ;/*0x200*/
    volatile u_vout_para_0_addr_h                 vout_para_0_addr_h            ;/*0x204*/
    volatile u_vout_para_0_addr_l                 vout_para_0_addr_l            ;/*0x208*/
    volatile xmedia_u32                           reserved7[62]                 ;/*0x20c*/
    volatile u_vout_para_2_addr_h                 vout_para_2_addr_h            ;/*0x304*/
    volatile u_vout_para_2_addr_l                 vout_para_2_addr_l            ;/*0x308*/
    volatile xmedia_u32                           reserved8[829]                ;/*0x30c*/
    volatile u_vout_v0_ctrl                       vout_v0_ctrl                  ;/*0x1000*/
    volatile u_vout_v0_upd                        vout_v0_upd                   ;/*0x1004*/
    volatile xmedia_u32                           reserved9[2]                  ;/*0x1008*/
    volatile u_vout_v0_bk                         vout_v0_bk                    ;/*0x1010*/
    volatile xmedia_u32                           reserved10[3]                 ;/*0x1014*/
    volatile u_vout_v0_mute_ctrl                  vout_v0_mute_ctrl             ;/*0x1020*/
    volatile xmedia_u32                           reserved11[3]                 ;/*0x1024*/
    volatile u_vout_v0_oreso                      vout_v0_oreso                 ;/*0x1030*/
    volatile xmedia_u32                           reserved12[3]                 ;/*0x1034*/
    volatile u_vout_v0_src_pos                    vout_v0_src_pos               ;/*0x1040*/
    volatile u_vout_v0_src                        vout_v0_src                   ;/*0x1044*/
    volatile xmedia_u32                           reserved13[46]                ;/*0x1048*/
    volatile u_vout_v0_mrg_ctrl                   vout_v0_mrg_ctrl              ;/*0x1100*/
    volatile xmedia_u32                           reserved14[15]                ;/*0x1104*/
    volatile u_vout_v0_mrg0_addr_y_h              vout_v0_mrg0_addr_y_h         ;/*0x1140*/
    volatile u_vout_v0_mrg0_addr_y_l              vout_v0_mrg0_addr_y_l         ;/*0x1144*/
    volatile u_vout_v0_mrg0_stride_y              vout_v0_mrg0_stride_y         ;/*0x1148*/
    volatile xmedia_u32                           reserved15                    ;/*0x114c*/
    volatile u_vout_v0_mrg0_addr_c_h              vout_v0_mrg0_addr_c_h         ;/*0x1150*/
    volatile u_vout_v0_mrg0_addr_c_l              vout_v0_mrg0_addr_c_l         ;/*0x1154*/
    volatile u_vout_v0_mrg0_stride_c              vout_v0_mrg0_stride_c         ;/*0x1158*/
    volatile xmedia_u32                           reserved16                    ;/*0x115c*/
    volatile u_vout_v0_mrg0_src_pos               vout_v0_mrg0_src_pos          ;/*0x1160*/
    volatile u_vout_v0_mrg0_src                   vout_v0_mrg0_src              ;/*0x1164*/
    volatile u_vout_v0_mrg0_mute_color            vout_v0_mrg0_mute_color       ;/*0x1168*/
    volatile xmedia_u32                           reserved17                    ;/*0x116c*/
    volatile u_vout_v0_mrg1_addr_y_h              vout_v0_mrg1_addr_y_h         ;/*0x1170*/
    volatile u_vout_v0_mrg1_addr_y_l              vout_v0_mrg1_addr_y_l         ;/*0x1174*/
    volatile u_vout_v0_mrg1_stride_y              vout_v0_mrg1_stride_y         ;/*0x1178*/
    volatile xmedia_u32                           reserved18                    ;/*0x117c*/
    volatile u_vout_v0_mrg1_addr_c_h              vout_v0_mrg1_addr_c_h         ;/*0x1180*/
    volatile u_vout_v0_mrg1_addr_c_l              vout_v0_mrg1_addr_c_l         ;/*0x1184*/
    volatile u_vout_v0_mrg1_stride_c              vout_v0_mrg1_stride_c         ;/*0x1188*/
    volatile xmedia_u32                           reserved19                    ;/*0x118c*/
    volatile u_vout_v0_mrg1_src_pos               vout_v0_mrg1_src_pos          ;/*0x1190*/
    volatile u_vout_v0_mrg1_src                   vout_v0_mrg1_src              ;/*0x1194*/
    volatile u_vout_v0_mrg1_mute_color            vout_v0_mrg1_mute_color       ;/*0x1198*/
    volatile xmedia_u32                           reserved20[26]                ;/*0x119c*/
    volatile u_vout_v0_vcus_coef                  vout_v0_vcus_coef             ;/*0x1204*/
    volatile u_vout_v0_hcus_coef                  vout_v0_hcus_coef             ;/*0x1208*/
    volatile xmedia_u32                           reserved21[29]                ;/*0x120c*/
    volatile u_vout_v0_mrg0_csc_offset_in         vout_v0_mrg0_csc_offset_in    ;/*0x1280*/
    volatile u_vout_v0_mrg0_csc_matrix_01         vout_v0_mrg0_csc_matrix_01    ;/*0x1284*/
    volatile u_vout_v0_mrg0_csc_matrix_23         vout_v0_mrg0_csc_matrix_23    ;/*0x1288*/
    volatile u_vout_v0_mrg0_csc_matrix_45         vout_v0_mrg0_csc_matrix_45    ;/*0x128c*/
    volatile u_vout_v0_mrg0_csc_matrix_67         vout_v0_mrg0_csc_matrix_67    ;/*0x1290*/
    volatile u_vout_v0_mrg0_csc_matrix_8          vout_v0_mrg0_csc_matrix_8     ;/*0x1294*/
    volatile xmedia_u32                           reserved22                    ;/*0x1298*/
    volatile u_vout_v0_mrg0_csc_offset_out        vout_v0_mrg0_csc_offset_out   ;/*0x129c*/
    volatile u_vout_v0_mrg1_csc_offset_in         vout_v0_mrg1_csc_offset_in    ;/*0x12a0*/
    volatile u_vout_v0_mrg1_csc_matrix_01         vout_v0_mrg1_csc_matrix_01    ;/*0x12a4*/
    volatile u_vout_v0_mrg1_csc_matrix_23         vout_v0_mrg1_csc_matrix_23    ;/*0x12a8*/
    volatile u_vout_v0_mrg1_csc_matrix_45         vout_v0_mrg1_csc_matrix_45    ;/*0x12ac*/
    volatile u_vout_v0_mrg1_csc_matrix_67         vout_v0_mrg1_csc_matrix_67    ;/*0x12b0*/
    volatile u_vout_v0_mrg1_csc_matrix_8          vout_v0_mrg1_csc_matrix_8     ;/*0x12b4*/
    volatile xmedia_u32                           reserved23                    ;/*0x12b8*/
    volatile u_vout_v0_mrg1_csc_offset_out        vout_v0_mrg1_csc_offset_out   ;/*0x12bc*/
    volatile xmedia_u32                           reserved24[28]                ;/*0x12c0*/
    volatile u_vout_v0_lba0                       vout_v0_lba0                  ;/*0x1330*/
    volatile u_vout_v0_lba1                       vout_v0_lba1                  ;/*0x1334*/
    volatile u_vout_v0_mrg0_disp_pos              vout_v0_mrg0_disp_pos         ;/*0x1338*/
    volatile u_vout_v0_mrg0_disp                  vout_v0_mrg0_disp             ;/*0x133c*/
    volatile u_vout_v0_mrg1_disp_pos              vout_v0_mrg1_disp_pos         ;/*0x1340*/
    volatile u_vout_v0_mrg1_disp                  vout_v0_mrg1_disp             ;/*0x1344*/
    volatile xmedia_u32                           reserved25[814]               ;/*0x1348*/
    volatile xmedia_u32                           reserved26[1024]              ;/*0x2000*/
    volatile u_vout_intf_ctrl                     vout_intf_ctrl                ;/*0x3000*/
    volatile u_vout_intf_upd                      vout_intf_upd                 ;/*0x3004*/
    volatile u_vout_intf_regup_pos                vout_intf_regup_pos           ;/*0x3008*/
    volatile u_vout_intf_start_pos                vout_intf_start_pos           ;/*0x300c*/
    volatile u_vout_intf_act                      vout_intf_act                 ;/*0x3010*/
    volatile u_vout_intf_vfb                      vout_intf_vfb                 ;/*0x3014*/
    volatile u_vout_intf_bvfb                     vout_intf_bvfb                ;/*0x3018*/
    volatile u_vout_intf_hfb                      vout_intf_hfb                 ;/*0x301c*/
    volatile u_vout_intf_hs_w                     vout_intf_hs_w                ;/*0x3020*/
    volatile u_vout_intf_field                    vout_intf_field               ;/*0x3024*/
    volatile u_vout_intf_vs_h                     vout_intf_vs_h                ;/*0x3028*/
    volatile u_vout_intf_vs_v                     vout_intf_vs_v                ;/*0x302c*/
    volatile u_vout_intf_bvs_h                    vout_intf_bvs_h               ;/*0x3030*/
    volatile u_vout_intf_bvs_v                    vout_intf_bvs_v               ;/*0x3034*/
    volatile u_vout_intf_sfifo_thd                vout_intf_sfifo_thd           ;/*0x3038*/
    volatile xmedia_u32                           reserved35[17]                ;/*0x303c*/
    volatile u_vout_intf_vt_int                   vout_intf_vt_int              ;/*0x3080*/
    volatile u_vout_intf_vt_intf_thd0             vout_intf_vt_intf_thd0        ;/*0x3084*/
    volatile u_vout_intf_vt_intf_thd2             vout_intf_vt_intf_thd2        ;/*0x3088*/
    volatile xmedia_u32                           reserved36                    ;/*0x308c*/
    volatile u_vout_intf_sta                      vout_intf_sta                 ;/*0x3090*/
    volatile u_vout_intf_sta2                     vout_intf_sta2                ;/*0x3094*/
    volatile u_vout_intf_uf_line                  vout_intf_uf_line             ;/*0x3098*/
    volatile u_vout_intf_y_sum                    vout_intf_y_sum               ;/*0x309c*/
    volatile u_vout_intf_u_sum                    vout_intf_u_sum               ;/*0x30a0*/
    volatile u_vout_intf_v_sum                    vout_intf_v_sum               ;/*0x30a4*/
    volatile xmedia_u32                           reserved37[22]                ;/*0x30a8*/
    volatile u_vout_intf_bt_ctrl                  vout_intf_bt_ctrl             ;/*0x3100*/
    volatile u_vout_intf_bt_vbit                  vout_intf_bt_vbit             ;/*0x3104*/
    volatile u_vout_intf_bt_bvbit                 vout_intf_bt_bvbit            ;/*0x3108*/
    volatile xmedia_u32                           reserved38[5]                 ;/*0x310c*/
    volatile u_vout_intf_lcd_ctrl                 vout_intf_lcd_ctrl            ;/*0x3120*/
    volatile xmedia_u32                           reserved39[55]                ;/*0x3124*/
    volatile u_vout_intf_mute_ctrl                vout_intf_mute_ctrl           ;/*0x3200*/
    volatile u_vout_intf_mute_color               vout_intf_mute_color          ;/*0x3204*/
    volatile u_vout_intf_mute_bk                  vout_intf_mute_bk             ;/*0x3208*/
    volatile xmedia_u32                           reserved40                    ;/*0x320c*/
    volatile u_vout_intf_ink_ctrl                 vout_intf_ink_ctrl            ;/*0x3210*/
    volatile u_vout_intf_ink_pos                  vout_intf_ink_pos             ;/*0x3214*/
    volatile u_vout_intf_ink_color                vout_intf_ink_color           ;/*0x3218*/
    volatile u_vout_intf_ink_data                 vout_intf_ink_data            ;/*0x321c*/
    volatile u_vout_intf_clip_l                   vout_intf_clip_l              ;/*0x3220*/
    volatile u_vout_intf_clip_h                   vout_intf_clip_h              ;/*0x3224*/
    volatile u_vout_intf_mcu_ctrl                 vout_intf_mcu_ctrl            ;/*0x3228*/
    volatile u_vout_intf_mcu_wrram_cmd            vout_intf_mcu_wrram_cmd       ;/*0x322c*/
    volatile u_vout_intf_mcu_wrramc_cmd           vout_intf_mcu_wrramc_cmd      ;/*0x3230*/
    volatile xmedia_u32                           reserved41[67]                ;/*0x3234*/
    volatile u_vout_intf_hcds_ctrl                vout_intf_hcds_ctrl           ;/*0x3340*/
    volatile u_vout_intf_hcds_coef                vout_intf_hcds_coef           ;/*0x3344*/
    volatile xmedia_u32                           reserved42[6]                 ;/*0x3348*/
    volatile u_vout_intf_dither_ctrl              vout_intf_dither_ctrl         ;/*0x3360*/
    volatile xmedia_u32                           reserved43[3]                 ;/*0x3364*/
    volatile u_vout_intf_gammalut_rd_ctrl         vout_intf_gammalut_rd_ctrl    ;/*0x3370*/
    volatile u_vout_intf_gammalut_rd_data         vout_intf_gammalut_rd_data    ;/*0x3374*/
    volatile xmedia_u32                           reserved44[34]                ;/*0x3378*/
    volatile u_vout_mcu_screen_ctrl_0             vout_mcu_screen_ctrl_0        ;/*0x3400*/
    volatile u_vout_mcu_screen_ctrl_1             vout_mcu_screen_ctrl_1        ;/*0x3404*/
    volatile u_vout_mcu_screen_ctrl_2             vout_mcu_screen_ctrl_2        ;/*0x3408*/
    volatile xmedia_u32                           reserved45[2]                 ;/*0x340c*/
    volatile xmedia_u32                           vout_mcu_screen_ram_rdata     ;/*0x3414*/
    volatile u_vout_mcu_screen_state              vout_mcu_screen_state         ;/*0x3418*/
    volatile xmedia_u32                           reserved46[9]                 ;/*0x341c*/
    volatile u_vout_mcu_screen_para_rdata_0       vout_mcu_screen_para_rdata_0  ;/*0x3440*/
    volatile u_vout_mcu_screen_para_rdata_1       vout_mcu_screen_para_rdata_1  ;/*0x3444*/
    volatile u_vout_mcu_screen_para_rdata_2       vout_mcu_screen_para_rdata_2  ;/*0x3448*/
    volatile u_vout_mcu_screen_para_rdata_3       vout_mcu_screen_para_rdata_3  ;/*0x344c*/
    volatile xmedia_u32                           reserved47[44]                ;/*0x3450*/
    volatile xmedia_u32                           vout_mcu_screen_wr_para       ;/*0x3500*/
}s_vout_regs_type;

extern s_vout_regs_type *g_p_vout_reg;

xmedia_u32 hal_reg_vout_reg_read(volatile xmedia_uintptr_t addr);
xmedia_void hal_reg_vout_reg_write(volatile xmedia_uintptr_t addr, xmedia_u32 val);
xmedia_u8 hal_reg_vout_get_cfg_testpin_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_testpin_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_reg_vout_get_cfg_regup_debug(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_regup_debug(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_ck_dyn_gt_en_dma(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_ck_dyn_gt_en_dma(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_ck_dyn_gt_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_ck_dyn_gt_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_reg_vout_set_cfg_regup_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_intf_regup_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_g0_regup_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_v0_regup_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_top_regup_sta(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_reg_vout_get_vout_ver_0(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_reg_vout_get_vout_ver_1(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_reg_vout_get_cfg_ras_sram_ctrl(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_ras_sram_ctrl(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_rfs_sram_ctrl(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_rfs_sram_ctrl(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_rat_sram_ctrl(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_rat_sram_ctrl(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_rft_sram_ctrl(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_rft_sram_ctrl(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_bool hal_reg_vout_get_intf_mcu_operr_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_intf_mcu_wrerr_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_axi_bus_werr_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_axi_bus_rerr_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_low_delay_y_err_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_low_delay_c_err_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_intf_underflow_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_intf_vt_int_3_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_intf_vt_int_2_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_intf_vt_int_1_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_intf_vt_int_0_sta(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_intf_mcu_operr_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_reg_vout_set_cfg_intf_mcu_wrerr_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_reg_vout_set_cfg_axi_bus_werr_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_reg_vout_set_cfg_axi_bus_rerr_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_reg_vout_set_cfg_low_delay_y_err_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_reg_vout_set_cfg_low_delay_c_err_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_reg_vout_set_cfg_intf_underflow_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_reg_vout_set_cfg_intf_vt_int_3_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_reg_vout_set_cfg_intf_vt_int_2_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_reg_vout_set_cfg_intf_vt_int_1_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_reg_vout_set_cfg_intf_vt_int_0_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_intf_mcu_operr_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_intf_mcu_operr_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_intf_mcu_wrerr_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_intf_mcu_wrerr_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_axi_bus_werr_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_axi_bus_werr_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_axi_bus_rerr_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_axi_bus_rerr_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_low_delay_y_err_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_low_delay_y_err_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_low_delay_c_err_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_low_delay_c_err_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_intf_underflow_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_intf_underflow_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_intf_vt_int_3_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_intf_vt_int_3_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_intf_vt_int_2_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_intf_vt_int_2_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_intf_vt_int_1_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_intf_vt_int_1_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_intf_vt_int_0_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_intf_vt_int_0_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_intf_mcu_operr_src_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_intf_mcu_wrerr_src_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_axi_bus_werr_src_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_axi_bus_rerr_src_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_low_delay_y_err_src_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_low_delay_c_err_src_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_intf_underflow_src_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_intf_vt_int_3_src_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_intf_vt_int_2_src_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_intf_vt_int_1_src_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_intf_vt_int_0_src_sta(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_uf_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_rdma_busy_g0(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_rdma1_busy_v0_c(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_rdma1_busy_v0_y(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_rdma_busy_v0_c(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_rdma_busy_v0_y(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_reg_vout_get_cfg_rd_bw_window_num_pow(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_rd_bw_window_num_pow(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_reg_vout_get_cfg_rd_bw_ch_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_rd_bw_ch_sel(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_reg_vout_get_cfg_rd_latency_ch_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_rd_latency_ch_sel(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_reg_vout_get_cfg_rd_bw_enable(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_rd_bw_enable(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_rd_latency_enable(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_rd_latency_enable(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_axi_sta_enable(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_axi_sta_enable(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u32 hal_reg_vout_get_dfx_axi_rd_burst_num(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_reg_vout_get_dfx_axi_rd_ost_max(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_reg_vout_get_dfx_axi_rd_ost_sum_h(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_reg_vout_get_dfx_axi_rd_ost_sum_l(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_reg_vout_get_dfx_rd_latency_min(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_reg_vout_get_dfx_rd_latency_max(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_reg_vout_get_dfx_rd_latency_sum_h(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_reg_vout_get_dfx_rd_latency_sum_l(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_reg_vout_get_dfx_rd_latency_burst_num(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_reg_vout_get_dfx_rd_bw_trans_max(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_reg_vout_get_dfx_rd_bw_trans_min(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_reg_vout_get_dfx_rd_bw_trans_avg(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_reg_vout_get_dfx_rd_bw_trans_window_cycle_avg(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_reg_vout_get_cfg_mixer_prio1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mixer_prio1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_reg_vout_get_cfg_mixer_prio0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mixer_prio0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u32 hal_reg_vout_get_cfg_bk_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_bk_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u8 hal_reg_vout_get_cfg_axi_rd_ost_num(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_axi_rd_ost_num(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_void hal_reg_vout_set_cfg_para_2_update(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_reg_vout_set_cfg_para_0_update(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u32 hal_reg_vout_get_cfg_para_0_addr_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_para_0_addr_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_reg_vout_get_cfg_para_0_addr_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_para_0_addr_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_reg_vout_get_cfg_para_2_addr_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_para_2_addr_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_reg_vout_get_cfg_para_2_addr_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_para_2_addr_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u8 hal_reg_vout_get_cfg_galpha(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_galpha(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_reg_vout_get_cfg_galpha_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_galpha_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_blend_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_blend_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_src_uv_order(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_src_uv_order(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_reg_vout_get_cfg_src_fmt(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_src_fmt(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_reg_vout_get_cfg_regup_field(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_regup_field(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_regup_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_regup_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_layer_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_layer_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_reg_vout_set_v0upd_cfg_regup_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u32 hal_reg_vout_get_v0bk_cfg_bk_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_v0bk_cfg_bk_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u8 hal_reg_vout_get_cfg_checker_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_checker_v(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_reg_vout_get_cfg_checker_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_checker_h(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_reg_vout_get_cfg_mute_pat(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mute_pat(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_mute_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mute_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u16 hal_reg_vout_get_out_v(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_reg_vout_get_out_h(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_reg_vout_get_cfg_src_ypos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_src_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_src_xpos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_src_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_src_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_src_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_src_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_src_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_bool hal_reg_vout_get_cfg_mixer_prio(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mixer_prio(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_mrg1_mute_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_mute_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_mrg0_mute_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_mute_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_mrg1_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_mrg0_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u32 hal_reg_vout_get_cfg_mrg0_src_addr_y_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_src_addr_y_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_reg_vout_get_cfg_mrg0_src_addr_y_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_src_addr_y_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_reg_vout_get_cfg_mrg0_src_stride_y(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_src_stride_y(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_reg_vout_get_cfg_mrg0_src_addr_c_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_src_addr_c_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_reg_vout_get_cfg_mrg0_src_addr_c_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_src_addr_c_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_reg_vout_get_cfg_mrg0_src_stride_c(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_src_stride_c(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg0_src_ypos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_src_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg0_src_xpos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_src_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg0_src_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_src_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg0_src_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_src_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u32 hal_reg_vout_get_cfg_mrg0_mute_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_mute_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_reg_vout_get_cfg_mrg1_src_addr_y_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_src_addr_y_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_reg_vout_get_cfg_mrg1_src_addr_y_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_src_addr_y_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_reg_vout_get_cfg_mrg1_src_stride_y(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_src_stride_y(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_reg_vout_get_cfg_mrg1_src_addr_c_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_src_addr_c_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_reg_vout_get_cfg_mrg1_src_addr_c_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_src_addr_c_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_reg_vout_get_cfg_mrg1_src_stride_c(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_src_stride_c(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg1_src_ypos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_src_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg1_src_xpos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_src_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg1_src_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_src_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg1_src_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_src_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u32 hal_reg_vout_get_cfg_mrg1_mute_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_mute_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u8 hal_reg_vout_get_cfg_vcus_coef_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vcus_coef_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_reg_vout_get_cfg_vcus_coef_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vcus_coef_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_reg_vout_get_cfg_hcus_coef_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_hcus_coef_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_reg_vout_get_cfg_hcus_coef_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_hcus_coef_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_reg_vout_get_cfg_csc_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_csc_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_csc_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_csc_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_offset_in_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_csc_offset_in_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_offset_in_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_csc_offset_in_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_offset_in_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_csc_offset_in_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_matrix_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_csc_matrix_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_matrix_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_csc_matrix_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_matrix_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_csc_matrix_3(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_matrix_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_csc_matrix_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_matrix_5(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_csc_matrix_5(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_matrix_4(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_csc_matrix_4(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_matrix_7(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_csc_matrix_7(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_matrix_6(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_csc_matrix_6(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_matrix_8(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_csc_matrix_8(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_offset_out_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_csc_offset_out_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_offset_out_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_csc_offset_out_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_offset_out_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_csc_offset_out_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_offset_in_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_csc_offset_in_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_offset_in_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_csc_offset_in_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_offset_in_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_csc_offset_in_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_matrix_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_csc_matrix_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_matrix_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_csc_matrix_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_matrix_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_csc_matrix_3(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_matrix_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_csc_matrix_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_matrix_5(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_csc_matrix_5(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_matrix_4(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_csc_matrix_4(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_matrix_7(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_csc_matrix_7(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_matrix_6(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_csc_matrix_6(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_matrix_8(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_csc_matrix_8(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_offset_out_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_csc_offset_out_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_offset_out_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_csc_offset_out_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_offset_out_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_csc_offset_out_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u32 hal_reg_vout_get_cfg_lba0_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_lba0_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_reg_vout_get_cfg_lba1_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_lba1_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg0_disp_ypos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_disp_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg0_disp_xpos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_disp_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg0_disp_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_disp_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg0_disp_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg0_disp_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg1_disp_ypos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_disp_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg1_disp_xpos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_disp_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg1_disp_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_disp_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_mrg1_disp_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mrg1_disp_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_void hal_reg_vout_set_cfg_csc_offset_out_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_bool hal_reg_vout_get_cfg_chk_sum_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_chk_sum_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_reg_vout_get_cfg_hs_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_hs_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_reg_vout_get_cfg_de_inv(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_de_inv(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_hs_inv(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_hs_inv(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_vs_inv(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vs_inv(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_field_inv(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_field_inv(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_clock_edge_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_clock_edge_sel(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_interlaced(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_interlaced(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_reg_vout_get_cfg_intf_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_intf_sel(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_reg_vout_get_cfg_intf_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_intf_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_reg_vout_set_intfupd_cfg_regup_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u16 hal_reg_vout_get_cfg_para_pos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_para_pos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_regup_pos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_regup_pos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_start_pos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_start_pos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_prefetch_pos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_prefetch_pos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_hact(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_hact(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_vact(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vact(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_vbb(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vbb(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_vfb(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vfb(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_bvbb(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_bvbb(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_bvfb(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_bvfb(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_hbb(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_hbb(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_hfb(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_hfb(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_hs_width(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_hs_width(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_bfield_tog(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_bfield_tog(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_field_tog(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_field_tog(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_vs_end_h_field0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vs_end_h_field0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_vs_st_h_field0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vs_st_h_field0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_vs_end_v_field0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vs_end_v_field0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_vs_st_v_field0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vs_st_v_field0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_vs_end_h_field1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vs_end_h_field1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_vs_st_h_field1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vs_st_h_field1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_vs_end_v_field1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vs_end_v_field1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_vs_st_v_field1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vs_st_v_field1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_intf_sfifo_thd(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_intf_sfifo_thd(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_bool hal_reg_vout_get_cfg_vt_int_cnt_sel_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vt_int_cnt_sel_3(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_vt_int_cnt_sel_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vt_int_cnt_sel_2(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_vt_int_cnt_sel_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vt_int_cnt_sel_1(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_vt_int_cnt_sel_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vt_int_cnt_sel_0(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_vt_int_mode_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vt_int_mode_3(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_vt_int_mode_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vt_int_mode_2(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_vt_int_mode_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vt_int_mode_1(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_vt_int_mode_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vt_int_mode_0(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u16 hal_reg_vout_get_cfg_vt_int_thd_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vt_int_thd_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_vt_int_thd_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vt_int_thd_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_vt_int_thd_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vt_int_thd_3(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_vt_int_thd_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vt_int_thd_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_frm_cnt(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_field_flag_ppc(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_reg_vout_get_vstate(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_reg_vout_get_vcnt_st_vact(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_reg_vout_get_vcnt_st_vblk(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_reg_vout_get_intf_uf_line(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_reg_vout_get_intf_y_sum(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_reg_vout_get_intf_u_sum(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_reg_vout_get_intf_v_sum(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_cfg_bt_bit_inv(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_bt_bit_inv(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_vbit_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vbit_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_bt_uv_order(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_bt_uv_order(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_bt_yc_order(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_bt_yc_order(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_bt_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_bt_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u16 hal_reg_vout_get_cfg_vbit_end(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vbit_end(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_vbit_st(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_vbit_st(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_bvbit_end(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_bvbit_end(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_bvbit_st(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_bvbit_st(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_bool hal_reg_vout_get_cfg_lcd_bit_inv(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_lcd_bit_inv(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_reg_vout_get_cfg_lcd_rgb_order_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_lcd_rgb_order_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_reg_vout_get_cfg_lcd_rgb_order_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_lcd_rgb_order_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_reg_vout_get_cfg_lcd_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_lcd_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_reg_vout_get_voutintf_cfg_checker_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_voutintf_cfg_checker_v(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_reg_vout_get_voutintf_cfg_checker_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_voutintf_cfg_checker_h(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_reg_vout_get_cfg_cbar_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_cbar_sel(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_reg_vout_get_voutintf_cfg_mute_pat(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_voutintf_cfg_mute_pat(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_reg_vout_get_voutintf_cfg_mute_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_voutintf_cfg_mute_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_voutintf_cfg_mute_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_voutintf_cfg_mute_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u32 hal_reg_vout_get_voutintf_cfg_mute_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_voutintf_cfg_mute_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_reg_vout_get_intfmutebk_cfg_bk_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_intfmutebk_cfg_bk_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_bool hal_reg_vout_get_cfg_cross_color_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_cross_color_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_cross_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_cross_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_ink_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_ink_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u16 hal_reg_vout_get_cfg_y_pos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_y_pos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_reg_vout_get_cfg_x_pos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_x_pos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u32 hal_reg_vout_get_cfg_cross_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_cross_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_reg_vout_get_ink_data(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_cfg_clip_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_clip_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u32 hal_reg_vout_get_cfg_clip_yuv_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_clip_yuv_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_reg_vout_get_cfg_clip_yuv_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_clip_yuv_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u8 hal_reg_vout_get_cfg_mcu_wr_fmt(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mcu_wr_fmt(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_reg_vout_get_cfg_mcu_wrram_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mcu_wrram_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_mcu_sync_type(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mcu_sync_type(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_reg_vout_get_cfg_mcu_wrram_cmd(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mcu_wrram_cmd(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_reg_vout_get_cfg_mcu_wrramc_cmd(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mcu_wrramc_cmd(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_reg_vout_get_cfg_hcds_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_hcds_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_hcds_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_hcds_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_reg_vout_get_cfg_hcds_coef_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_hcds_coef_3(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_reg_vout_get_cfg_hcds_coef_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_hcds_coef_2(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_reg_vout_get_cfg_hcds_coef_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_hcds_coef_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_reg_vout_get_cfg_hcds_coef_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_hcds_coef_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_reg_vout_get_cfg_dither_thd_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_dither_thd_h(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_reg_vout_get_cfg_dither_thd_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_dither_thd_l(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_reg_vout_get_cfg_gamma_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_gamma_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_reg_vout_get_cfg_dither_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_dither_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_reg_vout_set_cfg_gamma_rd_addr(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_void hal_reg_vout_set_cfg_gamma_rd_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u32 hal_reg_vout_get_gamma_rd_data(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_cfg_mcu_rst_ctrl(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mcu_rst_ctrl(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_reg_vout_get_cfg_mcu_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mcu_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_void hal_reg_vout_set_cfg_mcu_para_raddr(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_void hal_reg_vout_set_cfg_mcu_para_rd(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_reg_vout_set_cfg_mcu_wrcmd_start(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_reg_vout_set_cfg_mcu_cmd_stop(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_reg_vout_get_cfg_mcu_wrcmd_size(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mcu_wrcmd_size(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_reg_vout_get_cfg_mcu_cmd_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_reg_vout_set_cfg_mcu_cmd_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u32 hal_reg_vout_get_mcu_para_rdata(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_mcu_rd_pixel_state(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_reg_vout_get_mcu_rd_cmd_state(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_reg_vout_get_mcu_wr_cmd_state(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_reg_vout_get_mcu_rd_para_3(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_reg_vout_get_mcu_rd_para_2(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_reg_vout_get_mcu_rd_para_1(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_reg_vout_get_mcu_rd_para_0(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_reg_vout_get_mcu_rd_para_7(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_reg_vout_get_mcu_rd_para_6(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_reg_vout_get_mcu_rd_para_5(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_reg_vout_get_mcu_rd_para_4(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_reg_vout_get_mcu_rd_para_11(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_reg_vout_get_mcu_rd_para_10(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_reg_vout_get_mcu_rd_para_9(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_reg_vout_get_mcu_rd_para_8(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_reg_vout_get_mcu_rd_para_15(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_reg_vout_get_mcu_rd_para_14(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_reg_vout_get_mcu_rd_para_13(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_reg_vout_get_mcu_rd_para_12(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_reg_vout_get_mcu_wr_para(s_vout_regs_type *p_vout_reg,xmedia_u32 i);
xmedia_void hal_reg_vout_set_mcu_wr_para(s_vout_regs_type *p_vout_reg,xmedia_u32 i,xmedia_u32 value);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_VO_REG_H__ */
