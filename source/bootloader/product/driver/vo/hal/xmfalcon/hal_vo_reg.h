#ifndef __HAL_VO_REG_H__
#define __HAL_VO_REG_H__

#include "drv_vo_comm.h"

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
        unsigned int    v1_regup_sta                        : 1; /* [4]  */
        unsigned int    g1_regup_sta                        : 1; /* [5]  */
        unsigned int    intf1_regup_sta                     : 1; /* [6]  */
        unsigned int    reserved_0                          : 25; /* [31:7]  */
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
        unsigned int    cfg_mem_ctrl                        : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_mem_ctrl;

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
        unsigned int    intf1_vt_int_0_sta                  : 1; /* [4]  */
        unsigned int    intf1_vt_int_1_sta                  : 1; /* [5]  */
        unsigned int    intf1_vt_int_2_sta                  : 1; /* [6]  */
        unsigned int    intf1_vt_int_3_sta                  : 1; /* [7]  */
        unsigned int    reserved_3                          : 8; /* [15:8]  */
        unsigned int    intf_underflow_sta                  : 1; /* [16]  */
        unsigned int    intf1_underflow_sta                 : 1; /* [17]  */
        unsigned int    reserved_2                          : 2; /* [19:18]  */
        unsigned int    low_delay_c_err_sta                 : 1; /* [20]  */
        unsigned int    low_delay_y_err_sta                 : 1; /* [21]  */
        unsigned int    reserved_1                          : 2; /* [23:22]  */
        unsigned int    axi_bus_rerr_sta                    : 1; /* [24]  */
        unsigned int    axi_bus_werr_sta                    : 1; /* [25]  */
        unsigned int    reserved_0                          : 6; /* [31:26]  */
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
        unsigned int    cfg_intf1_vt_int_0_clr              : 1; /* [4]  */
        unsigned int    cfg_intf1_vt_int_1_clr              : 1; /* [5]  */
        unsigned int    cfg_intf1_vt_int_2_clr              : 1; /* [6]  */
        unsigned int    cfg_intf1_vt_int_3_clr              : 1; /* [7]  */
        unsigned int    reserved_3                          : 8; /* [15:8]  */
        unsigned int    cfg_intf_underflow_clr              : 1; /* [16]  */
        unsigned int    cfg_intf1_underflow_clr             : 1; /* [17]  */
        unsigned int    reserved_2                          : 2; /* [19:18]  */
        unsigned int    cfg_low_delay_c_err_clr             : 1; /* [20]  */
        unsigned int    cfg_low_delay_y_err_clr             : 1; /* [21]  */
        unsigned int    reserved_1                          : 2; /* [23:22]  */
        unsigned int    cfg_axi_bus_rerr_clr                : 1; /* [24]  */
        unsigned int    cfg_axi_bus_werr_clr                : 1; /* [25]  */
        unsigned int    reserved_0                          : 6; /* [31:26]  */
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
        unsigned int    cfg_intf1_vt_int_0_msk              : 1; /* [4]  */
        unsigned int    cfg_intf1_vt_int_1_msk              : 1; /* [5]  */
        unsigned int    cfg_intf1_vt_int_2_msk              : 1; /* [6]  */
        unsigned int    cfg_intf1_vt_int_3_msk              : 1; /* [7]  */
        unsigned int    reserved_3                          : 8; /* [15:8]  */
        unsigned int    cfg_intf_underflow_msk              : 1; /* [16]  */
        unsigned int    cfg_intf1_underflow_msk             : 1; /* [17]  */
        unsigned int    reserved_2                          : 2; /* [19:18]  */
        unsigned int    cfg_low_delay_c_err_msk             : 1; /* [20]  */
        unsigned int    cfg_low_delay_y_err_msk             : 1; /* [21]  */
        unsigned int    reserved_1                          : 2; /* [23:22]  */
        unsigned int    cfg_axi_bus_rerr_msk                : 1; /* [24]  */
        unsigned int    cfg_axi_bus_werr_msk                : 1; /* [25]  */
        unsigned int    reserved_0                          : 6; /* [31:26]  */
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
        unsigned int    intf1_vt_int_0_src_sta              : 1; /* [4]  */
        unsigned int    intf1_vt_int_1_src_sta              : 1; /* [5]  */
        unsigned int    intf1_vt_int_2_src_sta              : 1; /* [6]  */
        unsigned int    intf1_vt_int_3_src_sta              : 1; /* [7]  */
        unsigned int    reserved_3                          : 8; /* [15:8]  */
        unsigned int    intf_underflow_src_sta              : 1; /* [16]  */
        unsigned int    intf1_underflow_src_sta             : 1; /* [17]  */
        unsigned int    reserved_2                          : 2; /* [19:18]  */
        unsigned int    low_delay_c_err_src_sta             : 1; /* [20]  */
        unsigned int    low_delay_y_err_src_sta             : 1; /* [21]  */
        unsigned int    reserved_1                          : 2; /* [23:22]  */
        unsigned int    axi_bus_rerr_src_sta                : 1; /* [24]  */
        unsigned int    axi_bus_werr_src_sta                : 1; /* [25]  */
        unsigned int    reserved_0                          : 6; /* [31:26]  */
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
        unsigned int    rdma_busy_v1_y                      : 1; /* [5]  */
        unsigned int    rdma_busy_v1_c                      : 1; /* [6]  */
        unsigned int    rdma1_busy_v1_y                     : 1; /* [7]  */
        unsigned int    rdma1_busy_v1_c                     : 1; /* [8]  */
        unsigned int    rdma_busy_g1                        : 1; /* [9]  */
        unsigned int    reserved_0                          : 22; /* [31:10]  */
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

/* Define the union u_vout_cbm1_mixer */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mixer1_prio0                    : 2; /* [1:0]  */
        unsigned int    reserved_1                          : 2; /* [3:2]  */
        unsigned int    cfg_mixer1_prio1                    : 2; /* [5:4]  */
        unsigned int    reserved_0                          : 26; /* [31:6]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_cbm1_mixer;

/* Define the union u_vout_dma_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_cross_256_proc_en_r             : 1; /* [0]  */
        unsigned int    cfg_continuous_req_en_r             : 1; /* [1]  */
        unsigned int    reserved_1                          : 2; /* [3:2]  */
        unsigned int    cfg_rd_weight_v0_y                  : 4; /* [7:4]  */
        unsigned int    cfg_rd_weight_v0_c                  : 4; /* [11:8]  */
        unsigned int    cfg_rd_weight_v0_y_1                : 4; /* [15:12]  */
        unsigned int    cfg_rd_weight_v0_c_1                : 4; /* [19:16]  */
        unsigned int    cfg_rd_weight_g0                    : 4; /* [23:20]  */
        unsigned int    reserved_0                          : 8; /* [31:24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_dma_ctrl;

/* Define the union u_vout_dma_ost_num */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_axi_rd_ost_num                  : 5; /* [4:0]  */
        unsigned int    reserved_0                          : 3; /* [7:5]  */
        unsigned int    cfg_rd_ost_num_v0_y                 : 5; /* [12:8]  */
        unsigned int    reserved_1                          : 3; /* [15:13] */
        unsigned int    cfg_rd_ost_num_v0_c                 : 5; /* [20:16] */
        unsigned int    reserved_2                          : 3; /* [23:21] */
        unsigned int    cfg_rd_ost_num_v0_y_1               : 5; /* [28:24] */
        unsigned int    reserved_3                          : 3; /* [31:29] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_dma_ost_num;

/* Define the union u_vout_dma_ost_num_1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_rd_ost_num_v0_c_1               : 5; /* [4:0]  */
        unsigned int    reserved_0                          : 3; /* [7:5]  */
        unsigned int    cfg_rd_ost_num_g0                   : 5; /* [12:8]  */
        unsigned int    reserved_1                          : 3; /* [15:13] */
        unsigned int    cfg_rd_ost_num_v1_y                 : 5; /* [20:16] */
        unsigned int    reserved_2                          : 3; /* [23:21] */
        unsigned int    cfg_rd_ost_num_v1_c                 : 5; /* [28:24] */
        unsigned int    reserved_3                          : 3; /* [31:29] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_dma_ost_num_1;


/* Define the union u_vout_dma_ost_num_2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_rd_ost_num_v1_y_1               : 5; /* [4:0]  */
        unsigned int    reserved_0                          : 3; /* [7:5]  */
        unsigned int    cfg_rd_ost_num_v1_c_1               : 5; /* [12:8]  */
        unsigned int    reserved_1                          : 3; /* [15:13] */
        unsigned int    cfg_rd_ost_num_g1                   : 5; /* [20:16] */
        unsigned int    reserved_2                          : 11; /* [31:21] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_dma_ost_num_2;


/* Define the union u_vout_dma_ctrl_1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_rd_weight_v1_y                  : 4; /* [3:0]  */
        unsigned int    cfg_rd_weight_v1_c                  : 4; /* [7:4]  */
        unsigned int    cfg_rd_weight_v1_y_1                : 4; /* [11:8]  */
        unsigned int    cfg_rd_weight_v1_c_1                : 4; /* [15:12]  */
        unsigned int    cfg_rd_weight_g1                    : 4; /* [19:16]  */
        unsigned int    reserved                            : 12; /* [31:20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_dma_ctrl_1;

/* Define the union u_vout_dma_timeout_t */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_dma_timeout_th                  : 32; /* [31:0] */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_dma_timeout_th;

/* Define the union u_vout_para_update */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_para_0_update                   : 1; /* [0 : 0]  */
        unsigned int    cfg_para_1_update                   : 1; /* [1]  */
        unsigned int    cfg_para_2_update                   : 1; /* [2]  */
        unsigned int    cfg_para_3_update                   : 1; /* [3]  */
        unsigned int    reserved_0                          : 28; /* [31:4]  */
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

/* Define the union u_vout_para_1_addr_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_para_1_addr_h                   : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_para_1_addr_h;

/* Define the union u_vout_para_1_addr_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_para_1_addr_l                   : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_para_1_addr_l;

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

/* Define the union u_vout_para_3_addr_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_para_3_addr_h                   : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_para_3_addr_h;

/* Define the union u_vout_para_3_addr_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_para_3_addr_l                   : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_para_3_addr_l;

/* Define the union u_vout_lvds_phy_update */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_lvds_phy_update                 : 1; /* [0 : 0]  */
        unsigned int    reserved_0                          : 31; /* [31:1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_lvds_phy_update;

/* Define the union u_vout_lvds_phy_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_lvds_rf                         : 1; /* [0]  */
        unsigned int    reserved_5                          : 3; /* [3:1]  */
        unsigned int    cfg_lvds_phy_pn_inv                 : 5; /* [8:4]  */
        unsigned int    reserved_4                          : 1; /* [9]  */
        unsigned int    cfg_lvds_phy_ldo                    : 2; /* [11:10]  */
        unsigned int    cfg_lvds_vos                        : 3; /* [14:12]  */
        unsigned int    reserved_3                          : 1; /* [15]  */
        unsigned int    cfg_lvds_iref                       : 2; /* [17:16]  */
        unsigned int    reserved_2                          : 2; /* [19:18]  */
        unsigned int    cfg_lvds_phy_voc_sw                 : 5; /* [24:20]  */
        unsigned int    reserved_1                          : 2; /* [27:25]  */
        unsigned int    cfg_lvds_rterm_en                   : 1; /* [28]  */
        unsigned int    cfg_lvds_rterm_sel                  : 2; /* [30:29]  */
        unsigned int    reserved_0                          : 1; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;
} u_vout_lvds_phy_ctrl;

/* Define the union u_vout_lvds_pll_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_lvds_pll_frange                   : 5; /* [4:0]  */
        unsigned int    reserved_1                            : 1; /* [5]  */
        unsigned int    cfg_lvds_pll_ssc_en                   : 1; /* [6]  */
        unsigned int    cfg_lvds_pll_en                       : 1; /* [7]  */
        unsigned int    cfg_lvds_pll_ssc_freq                 : 2; /* [9:8]  */
        unsigned int    cfg_lvds_pll_ssc_spread               : 2; /* [11:10]  */
        unsigned int    cfg_lvds_pll_option                   : 4; /* [15:12]  */
        unsigned int    cfg_lvds_pll_cpi1                     : 4; /* [19:16]  */
        unsigned int    cfg_lvds_pll_cpi2                     : 4; /* [23:20]  */
        unsigned int    reserved_0                            : 8; /* [31:24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_lvds_pll_ctrl;

/* Define the union u_vout_lvds_phy_dbg */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_lvds_pd_en                   : 1; /* [0]  */
        unsigned int    reserved_1                       : 3; /* [3:1]  */
        unsigned int    cfg_lvds_lane_en                 : 5; /* [8:4]  */
        unsigned int    reserved_0                       : 23; /* [31:9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

}u_vout_lvds_phy_dbg;

/* Define the union u_vout_lvds_phy_sta */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    lvds_bist_ok                   : 5; /* [4:0]  */
        unsigned int    reserved_1                     : 3; /* [7:5]  */
        unsigned int    lvds_phy_rdy                   : 1; /* [8]  */
        unsigned int    reserved_0                     : 23; /* [31:9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

}u_vout_lvds_phy_sta;


/* Define the union u_vout_lvds_phy_en */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_lvds_pd_en                   : 1; /* [0]  */
        unsigned int    reserved_1                       : 3; /* [3:1]  */
        unsigned int    cfg_lvds_lane_en                 : 5; /* [8:4]  */
        unsigned int    reserved_0                       : 23; /* [31:9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

}u_vout_lvds_phy_en;


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
        unsigned int    cfg_mrg_en                          : 1; /* [8 : 8]  */
        unsigned int    reserved_2                          : 7; /* [15:9]  */
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
        unsigned int    cfg_mute_en                         : 1; /* [0 : 0]  */
        unsigned int    reserved_2                          : 3; /* [3 : 1]  */
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

/* Define the union u_vout_v0_mute_color */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mute_color                      : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mute_color;

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

/* Define the union u_vout_v0_disp_pos */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_disp_xpos                       : 16; /* [15 : 0]  */
        unsigned int    cfg_disp_ypos                       : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_disp_pos;

/* Define the union u_vout_v0_disp */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_disp_h                          : 16; /* [15 : 0]  */
        unsigned int    cfg_disp_v                          : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_disp;

/* Define the union u_vout_v0_addr_y_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_src_addr_y_h                    : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_addr_y_h;

/* Define the union u_vout_v0_addr_y_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_src_addr_y_l                    : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_addr_y_l;

/* Define the union u_vout_v0_stride_y */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_src_stride_y                    : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_stride_y;

/* Define the union u_vout_v0_addr_c_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_src_addr_c_h                    : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_addr_c_h;

/* Define the union u_vout_v0_addr_c_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_src_addr_c_l                    : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_addr_c_l;

/* Define the union u_vout_v0_stride_c */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_src_stride_c                    : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_stride_c;

/* Define the union u_vout_v0_mrg_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_en                         : 1; /* [0]  */
        unsigned int    cfg_mrg1_en                         : 1; /* [1]  */
        unsigned int    cfg_mrg2_en                         : 1; /* [2]  */
        unsigned int    cfg_mrg3_en                         : 1; /* [3]  */
        unsigned int    reserved_4                          : 4; /* [7:4]  */
        unsigned int    cfg_mrg0_mute_en                    : 1; /* [8]  */
        unsigned int    cfg_mrg1_mute_en                    : 1; /* [9]  */
        unsigned int    cfg_mrg2_mute_en                    : 1; /* [10]  */
        unsigned int    cfg_mrg3_mute_en                    : 1; /* [11]  */
        unsigned int    reserved_3                          : 4; /* [15:12]  */
        unsigned int    cfg_rdma1_en                        : 1; /* [16]  */
        unsigned int    reserved_2                          : 3; /* [19:17]  */
        unsigned int    cfg_mrg_link_rdma1                  : 2; /* [21:20]  */
        unsigned int    reserved_1                          : 2; /* [23:22]  */
        unsigned int    cfg_mixer_prio                      : 1; /* [24]  */
        unsigned int    reserved_0                          : 7; /* [31:25]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg_ctrl;

/* Define the union u_vout_v0_mrg_sort */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mixer_sort                      : 16; /* [15:0]  */
        unsigned int    reserved_0                          : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg_sort;

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

/* Define the union u_vout_v0_mrg2_addr_y_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_src_addr_y_h               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg2_addr_y_h;

/* Define the union u_vout_v0_mrg2_addr_y_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_src_addr_y_l               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg2_addr_y_l;

/* Define the union u_vout_v0_mrg2_stride_y */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_src_stride_y               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg2_stride_y;

/* Define the union u_vout_v0_mrg2_addr_c_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_src_addr_c_h               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg2_addr_c_h;

/* Define the union u_vout_v0_mrg2_addr_c_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_src_addr_c_l               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg2_addr_c_l;

/* Define the union u_vout_v0_mrg2_stride_c */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_src_stride_c               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg2_stride_c;

/* Define the union u_vout_v0_mrg2_src_pos */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_src_xpos                   : 16; /* [15 : 0]  */
        unsigned int    cfg_mrg2_src_ypos                   : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg2_src_pos;

/* Define the union u_vout_v0_mrg2_src */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_src_h                      : 16; /* [15 : 0]  */
        unsigned int    cfg_mrg2_src_v                      : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg2_src;

/* Define the union u_vout_v0_mrg2_mute_color */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_mute_color                 : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg2_mute_color;

/* Define the union u_vout_v0_mrg3_addr_y_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_src_addr_y_h               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg3_addr_y_h;

/* Define the union u_vout_v0_mrg3_addr_y_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_src_addr_y_l               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg3_addr_y_l;

/* Define the union u_vout_v0_mrg3_stride_y */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_src_stride_y               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg3_stride_y;

/* Define the union u_vout_v0_mrg3_addr_c_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_src_addr_c_h               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg3_addr_c_h;

/* Define the union u_vout_v0_mrg3_addr_c_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_src_addr_c_l               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg3_addr_c_l;

/* Define the union u_vout_v0_mrg3_stride_c */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_src_stride_c               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg3_stride_c;

/* Define the union u_vout_v0_mrg3_src_pos */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_src_xpos                   : 16; /* [15 : 0]  */
        unsigned int    cfg_mrg3_src_ypos                   : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg3_src_pos;

/* Define the union u_vout_v0_mrg3_src */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_src_h                      : 16; /* [15 : 0]  */
        unsigned int    cfg_mrg3_src_v                      : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg3_src;

/* Define the union u_vout_v0_mrg3_mute_color */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_mute_color                 : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg3_mute_color;

/* Define the union u_vout_v0_cus_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_vcus_mode                       : 1; /* [0]  */
        unsigned int    cfg_hcus_mode                       : 1; /* [1]  */
        unsigned int    reserved_0                          : 30; /* [31:2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_cus_ctrl;

/* Define the union u_vout_v0_vcus_coef */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_vcus_coef_0                     : 8; /* [7:0]  */
        unsigned int    cfg_vcus_coef_1                     : 8; /* [15:8]  */
        unsigned int    cfg_vcus_coef_2                     : 8; /* [23:16]  */
        unsigned int    cfg_vcus_coef_3                     : 8; /* [31 : 24]  */
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
        unsigned int    cfg_hcus_coef_0                     : 8; /* [7:0]  */
        unsigned int    cfg_hcus_coef_1                     : 8; /* [15:8]  */
        unsigned int    cfg_hcus_coef_2                     : 8; /* [23:16]  */
        unsigned int    cfg_hcus_coef_3                     : 8; /* [31 : 24]  */
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
        unsigned int    cfg_mrg0_csc_offset_in_0            : 8; /* [7:0]  */
        unsigned int    cfg_mrg0_csc_offset_in_1            : 8; /* [15:8]  */
        unsigned int    cfg_mrg0_csc_offset_in_2            : 8; /* [23:16]  */
        unsigned int    reserved_0                          : 7; /* [30:24]  */
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
        unsigned int    cfg_mrg1_csc_offset_in_0            : 8; /* [7:0]  */
        unsigned int    cfg_mrg1_csc_offset_in_1            : 8; /* [15:8]  */
        unsigned int    cfg_mrg1_csc_offset_in_2            : 8; /* [23:16]  */
        unsigned int    reserved_0                          : 8; /* [31:24]  */
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

/* Define the union u_vout_v0_mrg2_csc_offset_in */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_csc_offset_in_0            : 8; /* [7:0]  */
        unsigned int    cfg_mrg2_csc_offset_in_1            : 8; /* [15:8]  */
        unsigned int    cfg_mrg2_csc_offset_in_2            : 8; /* [23:16]  */
        unsigned int    reserved_0                          : 8; /* [31:24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg2_csc_offset_in;

/* Define the union u_vout_v0_mrg2_csc_matrix_01 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_csc_matrix_0               : 16; /* [15:0]  */
        unsigned int    cfg_mrg2_csc_matrix_1               : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg2_csc_matrix_01;

/* Define the union u_vout_v0_mrg2_csc_matrix_23 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_csc_matrix_2               : 16; /* [15:0]  */
        unsigned int    cfg_mrg2_csc_matrix_3               : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg2_csc_matrix_23;

/* Define the union u_vout_v0_mrg2_csc_matrix_45 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_csc_matrix_4               : 16; /* [15:0]  */
        unsigned int    cfg_mrg2_csc_matrix_5               : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg2_csc_matrix_45;

/* Define the union u_vout_v0_mrg2_csc_matrix_67 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_csc_matrix_6               : 16; /* [15:0]  */
        unsigned int    cfg_mrg2_csc_matrix_7               : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg2_csc_matrix_67;

/* Define the union u_vout_v0_mrg2_csc_matrix_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_csc_matrix_8               : 16; /* [15:0]  */
        unsigned int    reserved_0                          : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg2_csc_matrix_8;

/* Define the union u_vout_v0_mrg2_csc_offset_out */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_csc_offset_out_0           : 9; /* [8:0]  */
        unsigned int    reserved_2                          : 1; /* [9]  */
        unsigned int    cfg_mrg2_csc_offset_out_1           : 9; /* [18:10]  */
        unsigned int    reserved_1                          : 1; /* [19]  */
        unsigned int    cfg_mrg2_csc_offset_out_2           : 9; /* [28:20]  */
        unsigned int    reserved_0                          : 3; /* [31:29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg2_csc_offset_out;

/* Define the union u_vout_v0_mrg3_csc_offset_in */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_csc_offset_in_0            : 8; /* [7:0]  */
        unsigned int    cfg_mrg3_csc_offset_in_1            : 8; /* [15:8]  */
        unsigned int    cfg_mrg3_csc_offset_in_2            : 8; /* [23:16]  */
        unsigned int    reserved_0                          : 8; /* [31:24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg3_csc_offset_in;

/* Define the union u_vout_v0_mrg3_csc_matrix_01 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_csc_matrix_0               : 16; /* [15:0]  */
        unsigned int    cfg_mrg3_csc_matrix_1               : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg3_csc_matrix_01;

/* Define the union u_vout_v0_mrg3_csc_matrix_23 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_csc_matrix_2               : 16; /* [15:0]  */
        unsigned int    cfg_mrg3_csc_matrix_3               : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg3_csc_matrix_23;

/* Define the union u_vout_v0_mrg3_csc_matrix_45 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_csc_matrix_4               : 16; /* [15:0]  */
        unsigned int    cfg_mrg3_csc_matrix_5               : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg3_csc_matrix_45;

/* Define the union u_vout_v0_mrg3_csc_matrix_67 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_csc_matrix_6               : 16; /* [15:0]  */
        unsigned int    cfg_mrg3_csc_matrix_7               : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg3_csc_matrix_67;

/* Define the union u_vout_v0_mrg3_csc_matrix_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_csc_matrix_8               : 16; /* [15:0]  */
        unsigned int    reserved_0                          : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg3_csc_matrix_8;

/* Define the union u_vout_v0_mrg3_csc_offset_out */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_csc_offset_out_0           : 9; /* [8:0]  */
        unsigned int    reserved_2                          : 1; /* [9]  */
        unsigned int    cfg_mrg3_csc_offset_out_1           : 9; /* [18:10]  */
        unsigned int    reserved_1                          : 1; /* [19]  */
        unsigned int    cfg_mrg3_csc_offset_out_2           : 9; /* [28:20]  */
        unsigned int    reserved_0                          : 3; /* [31:29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v0_mrg3_csc_offset_out;

/* Define the union u_vout_g0_ctrl */
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
        unsigned int    reserved_2                          : 1; /* [7 : 7]  */
        unsigned int    cfg_rb_order                        : 1; /* [8 : 8]  */
        unsigned int    cfg_a_order                         : 1; /* [9 : 9]  */
        unsigned int    cfg_bitext                          : 2; /* [11 : 10]  */
        unsigned int    reserved_1                          : 4; /* [15 : 12]  */
        unsigned int    cfg_blend_mode                      : 1; /* [16 : 16]  */
        unsigned int    reserved_0                          : 3; /* [19 : 17]  */
        unsigned int    cfg_galpha_en                       : 1; /* [20 : 20]  */
        unsigned int    cfg_palpha_en                       : 1; /* [21 : 21]  */
        unsigned int    cfg_gpremul_en                      : 1; /* [22 : 22]  */
        unsigned int    cfg_ppremul_en                      : 1; /* [23 : 23]  */
        unsigned int    cfg_galpha                          : 8; /* [31 : 24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g0_ctrl;

/* Define the union u_vout_g0_upd */
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

} u_vout_g0_upd;

/* Define the union u_vout_g0_bk */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_bk_color                        : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g0_bk;

/* Define the union u_vout_g0_alpha_ext */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_alpha_0                         : 8; /* [7 : 0]  */
        unsigned int    cfg_alpha_1                         : 8; /* [15 : 8]  */
        unsigned int    reserved_0                          : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g0_alpha_ext;

/* Define the union u_vout_g0_mute_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mute_en                         : 1; /* [0 : 0]  */
        unsigned int    reserved_2                          : 3; /* [3 : 1]  */
        unsigned int    cfg_mute_mode                       : 1; /* [4 : 4]  */
        unsigned int    reserved_1                          : 3; /* [7 : 5]  */
        unsigned int    cfg_mute_pat                        : 1; /* [8 : 8]  */
        unsigned int    reserved_0                          : 7; /* [15 : 9]  */
        unsigned int    cfg_checker_h                       : 8; /* [23 : 16]  */
        unsigned int    cfg_checker_v                       : 8; /* [31 : 24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g0_mute_ctrl;

/* Define the union u_vout_g0_mute_color */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mute_color                      : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g0_mute_color;

/* Define the union u_vout_g0_oreso */
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

} u_vout_g0_oreso;

/* Define the union u_vout_g0_src_pos */
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

} u_vout_g0_src_pos;

/* Define the union u_vout_g0_src */
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

} u_vout_g0_src;

/* Define the union u_vout_g0_disp_pos */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_disp_xpos                       : 16; /* [15 : 0]  */
        unsigned int    cfg_disp_ypos                       : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g0_disp_pos;

/* Define the union u_vout_g0_disp */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_disp_h                          : 16; /* [15 : 0]  */
        unsigned int    cfg_disp_v                          : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g0_disp;

/* Define the union u_vout_g0_addr_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_src_addr_h                      : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g0_addr_h;

/* Define the union u_vout_g0_addr_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_src_addr_l                      : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g0_addr_l;

/* Define the union u_vout_g0_stride */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_src_stride                      : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g0_stride;

/* Define the union u_vout_g0_clut_rd_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_clut_rd_en                      : 1; /* [0 : 0]  */
        unsigned int    reserved_1                          : 7; /* [7 : 1]  */
        unsigned int    cfg_clut_rd_addr                    : 8; /* [15 : 8]  */
        unsigned int    reserved_0                          : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g0_clut_rd_ctrl;

/* Define the union u_vout_g0_clut_rd_data */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clut_rd_data                        : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g0_clut_rd_data;

/* Define the union u_vout_g0_key_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_key_en                          : 1; /* [0 : 0]  */
        unsigned int    cfg_key_sel                         : 1; /* [1 : 1]  */
        unsigned int    reserved_1                          : 6; /* [7 : 2]  */
        unsigned int    cfg_key_a_mode                      : 2; /* [9 : 8]  */
        unsigned int    cfg_key_y_mode                      : 2; /* [11 : 10]  */
        unsigned int    cfg_key_u_mode                      : 2; /* [13 : 12]  */
        unsigned int    cfg_key_v_mode                      : 2; /* [15 : 14]  */
        unsigned int    reserved_0                          : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g0_key_ctrl;

/* Define the union u_vout_g0_key_max */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_key_a_max                       : 8; /* [7 : 0]  */
        unsigned int    cfg_key_y_max                       : 8; /* [15 : 8]  */
        unsigned int    cfg_key_u_max                       : 8; /* [23 : 16]  */
        unsigned int    cfg_key_v_max                       : 8; /* [31 : 24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g0_key_max;

/* Define the union u_vout_g0_key_min */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_key_a_min                       : 8; /* [7 : 0]  */
        unsigned int    cfg_key_y_min                       : 8; /* [15 : 8]  */
        unsigned int    cfg_key_u_min                       : 8; /* [23 : 16]  */
        unsigned int    cfg_key_v_min                       : 8; /* [31 : 24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g0_key_min;

/* Define the union u_vout_g0_key_msk */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_key_a_msk                       : 8; /* [7 : 0]  */
        unsigned int    cfg_key_y_msk                       : 8; /* [15 : 8]  */
        unsigned int    cfg_key_u_msk                       : 8; /* [23 : 16]  */
        unsigned int    cfg_key_v_msk                       : 8; /* [31 : 24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g0_key_msk;

/* Define the union u_vout_g0_csc_offset_in */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_offset_in_0                 : 8; /* [7:0]  */
        unsigned int    cfg_csc_offset_in_1                 : 8; /* [15:8]  */
        unsigned int    cfg_csc_offset_in_2                 : 8; /* [23:16]  */
        unsigned int    reserved_0                          : 7; /* [30:24]  */
        unsigned int    cfg_csc_en                          : 1; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g0_csc_offset_in;

/* Define the union u_vout_g0_csc_matrix_01 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_matrix_0                    : 16; /* [15:0]  */
        unsigned int    cfg_csc_matrix_1                    : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g0_csc_matrix_01;

/* Define the union u_vout_g0_csc_matrix_23 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_matrix_2                    : 16; /* [15:0]  */
        unsigned int    cfg_csc_matrix_3                    : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g0_csc_matrix_23;

/* Define the union u_vout_g0_csc_matrix_45 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_matrix_4                    : 16; /* [15:0]  */
        unsigned int    cfg_csc_matrix_5                    : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g0_csc_matrix_45;

/* Define the union u_vout_g0_csc_matrix_67 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_matrix_6                    : 16; /* [15:0]  */
        unsigned int    cfg_csc_matrix_7                    : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g0_csc_matrix_67;

/* Define the union u_vout_g0_csc_matrix_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_matrix_8                    : 16; /* [15:0]  */
        unsigned int    reserved_0                          : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g0_csc_matrix_8;

/* Define the union u_vout_g0_csc_offset_out */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_offset_out_0                : 9; /* [8:0]  */
        unsigned int    reserved_2                          : 1; /* [9]  */
        unsigned int    cfg_csc_offset_out_1                : 9; /* [18:10]  */
        unsigned int    reserved_1                          : 1; /* [19]  */
        unsigned int    cfg_csc_offset_out_2                : 9; /* [28:20]  */
        unsigned int    reserved_0                          : 3; /* [31:29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g0_csc_offset_out;

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

/* Define the union u_vout_intf_lvds_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_lvds_mode                       : 3; /* [2 : 0]  */
        unsigned int    reserved_1                          : 1; /* [3 : 3]  */
        unsigned int    cfg_lvds_lane_0_sel                 : 4; /* [7:4]  */
        unsigned int    cfg_lvds_lane_1_sel                 : 4; /* [11:8]  */
        unsigned int    cfg_lvds_lane_2_sel                 : 4; /* [15:12]  */
        unsigned int    cfg_lvds_lane_3_sel                 : 4; /* [19:16]  */
        unsigned int    cfg_lvds_lane_4_sel                 : 4; /* [23:20]  */
        unsigned int    reserved_0                          : 8; /* [31:24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_lvds_ctrl;

/* Define the union u_vout_intf_mipi_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_dpi_mode                        : 3; /* [2 : 0]  */
        unsigned int    cfg_dpi_uv_order                    : 1; /* [3]  */
        unsigned int    reserved_0                          : 28; /* [31:4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_mipi_ctrl;

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

/* Define the union u_vout_intf_csc_offset_in */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_offset_in_0                 : 8; /* [7:0]  */
        unsigned int    cfg_csc_offset_in_1                 : 8; /* [15:8]  */
        unsigned int    cfg_csc_offset_in_2                 : 8; /* [23:16]  */
        unsigned int    reserved_0                          : 7; /* [30:24]  */
        unsigned int    cfg_csc_en                          : 1; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_csc_offset_in;

/* Define the union u_vout_intf_csc_matrix_01 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_matrix_0                    : 16; /* [15:0]  */
        unsigned int    cfg_csc_matrix_1                    : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_csc_matrix_01;

/* Define the union u_vout_intf_csc_matrix_23 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_matrix_2                    : 16; /* [15:0]  */
        unsigned int    cfg_csc_matrix_3                    : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_csc_matrix_23;

/* Define the union u_vout_intf_csc_matrix_45 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_matrix_4                    : 16; /* [15:0]  */
        unsigned int    cfg_csc_matrix_5                    : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_csc_matrix_45;

/* Define the union u_vout_intf_csc_matrix_67 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_matrix_6                    : 16; /* [15:0]  */
        unsigned int    cfg_csc_matrix_7                    : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_csc_matrix_67;

/* Define the union u_vout_intf_csc_matrix_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_matrix_8                    : 16; /* [15:0]  */
        unsigned int    reserved_0                          : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_csc_matrix_8;

/* Define the union u_vout_intf_csc_offset_out */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_offset_out_0                : 9; /* [8:0]  */
        unsigned int    reserved_2                          : 1; /* [9]  */
        unsigned int    cfg_csc_offset_out_1                : 9; /* [18:10]  */
        unsigned int    reserved_1                          : 1; /* [19]  */
        unsigned int    cfg_csc_offset_out_2                : 9; /* [28:20]  */
        unsigned int    reserved_0                          : 3; /* [31:29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf_csc_offset_out;

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

/* Define the union u_vout_v1_ctrl */
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
        unsigned int    cfg_mrg_en                          : 1; /* [8 : 8]  */
        unsigned int    reserved_2                          : 7; /* [15:9]  */
        unsigned int    cfg_blend_mode                      : 1; /* [16 : 16]  */
        unsigned int    reserved_1                          : 3; /* [19 : 17]  */
        unsigned int    cfg_galpha_en                       : 1; /* [20 : 20]  */
        unsigned int    reserved_0                          : 3; /* [23 : 21]  */
        unsigned int    cfg_galpha                          : 8; /* [31 : 24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_ctrl;

/* Define the union u_vout_v1_upd */
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

} u_vout_v1_upd;

/* Define the union u_vout_v1_bk */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_bk_color                        : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_bk;

/* Define the union u_vout_v1_mute_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mute_en                         : 1; /* [0 : 0]  */
        unsigned int    reserved_2                          : 3; /* [3 : 1]  */
        unsigned int    cfg_mute_mode                       : 1; /* [4 : 4]  */
        unsigned int    reserved_1                          : 3; /* [7 : 5]  */
        unsigned int    cfg_mute_pat                        : 1; /* [8 : 8]  */
        unsigned int    reserved_0                          : 7; /* [15 : 9]  */
        unsigned int    cfg_checker_h                       : 8; /* [23 : 16]  */
        unsigned int    cfg_checker_v                       : 8; /* [31 : 24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mute_ctrl;

/* Define the union u_vout_v1_mute_color */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mute_color                      : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mute_color;

/* Define the union u_vout_v1_oreso */
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

} u_vout_v1_oreso;

/* Define the union u_vout_v1_src_pos */
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

} u_vout_v1_src_pos;

/* Define the union u_vout_v1_src */
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

} u_vout_v1_src;

/* Define the union u_vout_v1_disp_pos */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_disp_xpos                       : 16; /* [15 : 0]  */
        unsigned int    cfg_disp_ypos                       : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_disp_pos;

/* Define the union u_vout_v1_disp */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_disp_h                          : 16; /* [15 : 0]  */
        unsigned int    cfg_disp_v                          : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_disp;

/* Define the union u_vout_v1_addr_y_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_src_addr_y_h                    : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_addr_y_h;

/* Define the union u_vout_v1_addr_y_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_src_addr_y_l                    : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_addr_y_l;

/* Define the union u_vout_v1_stride_y */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_src_stride_y                    : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_stride_y;

/* Define the union u_vout_v1_addr_c_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_src_addr_c_h                    : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_addr_c_h;

/* Define the union u_vout_v1_addr_c_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_src_addr_c_l                    : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_addr_c_l;

/* Define the union u_vout_v1_stride_c */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_src_stride_c                    : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_stride_c;

/* Define the union u_vout_v1_mrg_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_en                         : 1; /* [0]  */
        unsigned int    cfg_mrg1_en                         : 1; /* [1]  */
        unsigned int    cfg_mrg2_en                         : 1; /* [2]  */
        unsigned int    cfg_mrg3_en                         : 1; /* [3]  */
        unsigned int    reserved_4                          : 4; /* [7:4]  */
        unsigned int    cfg_mrg0_mute_en                    : 1; /* [8]  */
        unsigned int    cfg_mrg1_mute_en                    : 1; /* [9]  */
        unsigned int    cfg_mrg2_mute_en                    : 1; /* [10]  */
        unsigned int    cfg_mrg3_mute_en                    : 1; /* [11]  */
        unsigned int    reserved_3                          : 4; /* [15:12]  */
        unsigned int    cfg_rdma1_en                        : 1; /* [16]  */
        unsigned int    reserved_2                          : 3; /* [19:17]  */
        unsigned int    cfg_mrg_link_rdma1                  : 2; /* [21:20]  */
        unsigned int    reserved_1                          : 2; /* [23:22]  */
        unsigned int    cfg_mixer_prio                      : 1; /* [24]  */
        unsigned int    reserved_0                          : 7; /* [31:25]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg_ctrl;

/* Define the union u_vout_v1_mrg_sort */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mixer_sort                      : 16; /* [15:0]  */
        unsigned int    reserved_0                          : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg_sort;

/* Define the union u_vout_v1_mrg0_addr_y_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_src_addr_y_h               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg0_addr_y_h;

/* Define the union u_vout_v1_mrg0_addr_y_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_src_addr_y_l               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg0_addr_y_l;

/* Define the union u_vout_v1_mrg0_stride_y */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_src_stride_y               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg0_stride_y;

/* Define the union u_vout_v1_mrg0_addr_c_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_src_addr_c_h               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg0_addr_c_h;

/* Define the union u_vout_v1_mrg0_addr_c_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_src_addr_c_l               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg0_addr_c_l;

/* Define the union u_vout_v1_mrg0_stride_c */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_src_stride_c               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg0_stride_c;

/* Define the union u_vout_v1_mrg0_src_pos */
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

} u_vout_v1_mrg0_src_pos;

/* Define the union u_vout_v1_mrg0_src */
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

} u_vout_v1_mrg0_src;

/* Define the union u_vout_v1_mrg0_mute_color */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_mute_color                 : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg0_mute_color;

/* Define the union u_vout_v1_mrg1_addr_y_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_src_addr_y_h               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg1_addr_y_h;

/* Define the union u_vout_v1_mrg1_addr_y_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_src_addr_y_l               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg1_addr_y_l;

/* Define the union u_vout_v1_mrg1_stride_y */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_src_stride_y               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg1_stride_y;

/* Define the union u_vout_v1_mrg1_addr_c_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_src_addr_c_h               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg1_addr_c_h;

/* Define the union u_vout_v1_mrg1_addr_c_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_src_addr_c_l               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg1_addr_c_l;

/* Define the union u_vout_v1_mrg1_stride_c */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_src_stride_c               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg1_stride_c;

/* Define the union u_vout_v1_mrg1_src_pos */
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

} u_vout_v1_mrg1_src_pos;

/* Define the union u_vout_v1_mrg1_src */
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

} u_vout_v1_mrg1_src;

/* Define the union u_vout_v1_mrg1_mute_color */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_mute_color                 : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg1_mute_color;

/* Define the union u_vout_v1_mrg2_addr_y_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_src_addr_y_h               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg2_addr_y_h;

/* Define the union u_vout_v1_mrg2_addr_y_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_src_addr_y_l               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg2_addr_y_l;

/* Define the union u_vout_v1_mrg2_stride_y */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_src_stride_y               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg2_stride_y;

/* Define the union u_vout_v1_mrg2_addr_c_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_src_addr_c_h               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg2_addr_c_h;

/* Define the union u_vout_v1_mrg2_addr_c_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_src_addr_c_l               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg2_addr_c_l;

/* Define the union u_vout_v1_mrg2_stride_c */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_src_stride_c               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg2_stride_c;

/* Define the union u_vout_v1_mrg2_src_pos */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_src_xpos                   : 16; /* [15 : 0]  */
        unsigned int    cfg_mrg2_src_ypos                   : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg2_src_pos;

/* Define the union u_vout_v1_mrg2_src */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_src_h                      : 16; /* [15 : 0]  */
        unsigned int    cfg_mrg2_src_v                      : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg2_src;

/* Define the union u_vout_v1_mrg2_mute_color */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_mute_color                 : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg2_mute_color;

/* Define the union u_vout_v1_mrg3_addr_y_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_src_addr_y_h               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg3_addr_y_h;

/* Define the union u_vout_v1_mrg3_addr_y_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_src_addr_y_l               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg3_addr_y_l;

/* Define the union u_vout_v1_mrg3_stride_y */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_src_stride_y               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg3_stride_y;

/* Define the union u_vout_v1_mrg3_addr_c_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_src_addr_c_h               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg3_addr_c_h;

/* Define the union u_vout_v1_mrg3_addr_c_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_src_addr_c_l               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg3_addr_c_l;

/* Define the union u_vout_v1_mrg3_stride_c */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_src_stride_c               : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg3_stride_c;

/* Define the union u_vout_v1_mrg3_src_pos */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_src_xpos                   : 16; /* [15 : 0]  */
        unsigned int    cfg_mrg3_src_ypos                   : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg3_src_pos;

/* Define the union u_vout_v1_mrg3_src */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_src_h                      : 16; /* [15 : 0]  */
        unsigned int    cfg_mrg3_src_v                      : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg3_src;

/* Define the union u_vout_v1_mrg3_mute_color */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_mute_color                 : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg3_mute_color;

/* Define the union u_vout_v1_cus_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_vcus_mode                       : 1; /* [0]  */
        unsigned int    cfg_hcus_mode                       : 1; /* [1]  */
        unsigned int    reserved_0                          : 30; /* [31:2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_cus_ctrl;

/* Define the union u_vout_v1_vcus_coef */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_vcus_coef_0                     : 8; /* [7:0]  */
        unsigned int    cfg_vcus_coef_1                     : 8; /* [15:8]  */
        unsigned int    cfg_vcus_coef_2                     : 8; /* [23:16]  */
        unsigned int    cfg_vcus_coef_3                     : 8; /* [31 : 24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_vcus_coef;

/* Define the union u_vout_v1_hcus_coef */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_hcus_coef_0                     : 8; /* [7:0]  */
        unsigned int    cfg_hcus_coef_1                     : 8; /* [15:8]  */
        unsigned int    cfg_hcus_coef_2                     : 8; /* [23:16]  */
        unsigned int    cfg_hcus_coef_3                     : 8; /* [31 : 24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_hcus_coef;

/* Define the union u_vout_v1_mrg0_csc_offset_in */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg0_csc_offset_in_0            : 8; /* [7:0]  */
        unsigned int    cfg_mrg0_csc_offset_in_1            : 8; /* [15:8]  */
        unsigned int    cfg_mrg0_csc_offset_in_2            : 8; /* [23:16]  */
        unsigned int    reserved_0                          : 7; /* [30:24]  */
        unsigned int    cfg_csc_en                          : 1; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg0_csc_offset_in;

/* Define the union u_vout_v1_mrg0_csc_matrix_01 */
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

} u_vout_v1_mrg0_csc_matrix_01;

/* Define the union u_vout_v1_mrg0_csc_matrix_23 */
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

} u_vout_v1_mrg0_csc_matrix_23;

/* Define the union u_vout_v1_mrg0_csc_matrix_45 */
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

} u_vout_v1_mrg0_csc_matrix_45;

/* Define the union u_vout_v1_mrg0_csc_matrix_67 */
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

} u_vout_v1_mrg0_csc_matrix_67;

/* Define the union u_vout_v1_mrg0_csc_matrix_8 */
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

} u_vout_v1_mrg0_csc_matrix_8;

/* Define the union u_vout_v1_mrg0_csc_offset_out */
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

} u_vout_v1_mrg0_csc_offset_out;

/* Define the union u_vout_v1_mrg1_csc_offset_in */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg1_csc_offset_in_0            : 8; /* [7:0]  */
        unsigned int    cfg_mrg1_csc_offset_in_1            : 8; /* [15:8]  */
        unsigned int    cfg_mrg1_csc_offset_in_2            : 8; /* [23:16]  */
        unsigned int    reserved_0                          : 8; /* [31:24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg1_csc_offset_in;

/* Define the union u_vout_v1_mrg1_csc_matrix_01 */
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

} u_vout_v1_mrg1_csc_matrix_01;

/* Define the union u_vout_v1_mrg1_csc_matrix_23 */
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

} u_vout_v1_mrg1_csc_matrix_23;

/* Define the union u_vout_v1_mrg1_csc_matrix_45 */
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

} u_vout_v1_mrg1_csc_matrix_45;

/* Define the union u_vout_v1_mrg1_csc_matrix_67 */
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

} u_vout_v1_mrg1_csc_matrix_67;

/* Define the union u_vout_v1_mrg1_csc_matrix_8 */
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

} u_vout_v1_mrg1_csc_matrix_8;

/* Define the union u_vout_v1_mrg1_csc_offset_out */
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

} u_vout_v1_mrg1_csc_offset_out;

/* Define the union u_vout_v1_mrg2_csc_offset_in */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_csc_offset_in_0            : 8; /* [7:0]  */
        unsigned int    cfg_mrg2_csc_offset_in_1            : 8; /* [15:8]  */
        unsigned int    cfg_mrg2_csc_offset_in_2            : 8; /* [23:16]  */
        unsigned int    reserved_0                          : 8; /* [31:24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg2_csc_offset_in;

/* Define the union u_vout_v1_mrg2_csc_matrix_01 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_csc_matrix_0               : 16; /* [15:0]  */
        unsigned int    cfg_mrg2_csc_matrix_1               : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg2_csc_matrix_01;

/* Define the union u_vout_v1_mrg2_csc_matrix_23 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_csc_matrix_2               : 16; /* [15:0]  */
        unsigned int    cfg_mrg2_csc_matrix_3               : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg2_csc_matrix_23;

/* Define the union u_vout_v1_mrg2_csc_matrix_45 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_csc_matrix_4               : 16; /* [15:0]  */
        unsigned int    cfg_mrg2_csc_matrix_5               : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg2_csc_matrix_45;

/* Define the union u_vout_v1_mrg2_csc_matrix_67 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_csc_matrix_6               : 16; /* [15:0]  */
        unsigned int    cfg_mrg2_csc_matrix_7               : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg2_csc_matrix_67;

/* Define the union u_vout_v1_mrg2_csc_matrix_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_csc_matrix_8               : 16; /* [15:0]  */
        unsigned int    reserved_0                          : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg2_csc_matrix_8;

/* Define the union u_vout_v1_mrg2_csc_offset_out */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg2_csc_offset_out_0           : 9; /* [8:0]  */
        unsigned int    reserved_2                          : 1; /* [9]  */
        unsigned int    cfg_mrg2_csc_offset_out_1           : 9; /* [18:10]  */
        unsigned int    reserved_1                          : 1; /* [19]  */
        unsigned int    cfg_mrg2_csc_offset_out_2           : 9; /* [28:20]  */
        unsigned int    reserved_0                          : 3; /* [31:29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg2_csc_offset_out;

/* Define the union u_vout_v1_mrg3_csc_offset_in */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_csc_offset_in_0            : 8; /* [7:0]  */
        unsigned int    cfg_mrg3_csc_offset_in_1            : 8; /* [15:8]  */
        unsigned int    cfg_mrg3_csc_offset_in_2            : 8; /* [23:16]  */
        unsigned int    reserved_0                          : 8; /* [31:24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg3_csc_offset_in;

/* Define the union u_vout_v1_mrg3_csc_matrix_01 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_csc_matrix_0               : 16; /* [15:0]  */
        unsigned int    cfg_mrg3_csc_matrix_1               : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg3_csc_matrix_01;

/* Define the union u_vout_v1_mrg3_csc_matrix_23 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_csc_matrix_2               : 16; /* [15:0]  */
        unsigned int    cfg_mrg3_csc_matrix_3               : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg3_csc_matrix_23;

/* Define the union u_vout_v1_mrg3_csc_matrix_45 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_csc_matrix_4               : 16; /* [15:0]  */
        unsigned int    cfg_mrg3_csc_matrix_5               : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg3_csc_matrix_45;

/* Define the union u_vout_v1_mrg3_csc_matrix_67 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_csc_matrix_6               : 16; /* [15:0]  */
        unsigned int    cfg_mrg3_csc_matrix_7               : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg3_csc_matrix_67;

/* Define the union u_vout_v1_mrg3_csc_matrix_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_csc_matrix_8               : 16; /* [15:0]  */
        unsigned int    reserved_0                          : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg3_csc_matrix_8;

/* Define the union u_vout_v1_mrg3_csc_offset_out */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mrg3_csc_offset_out_0           : 9; /* [8:0]  */
        unsigned int    reserved_2                          : 1; /* [9]  */
        unsigned int    cfg_mrg3_csc_offset_out_1           : 9; /* [18:10]  */
        unsigned int    reserved_1                          : 1; /* [19]  */
        unsigned int    cfg_mrg3_csc_offset_out_2           : 9; /* [28:20]  */
        unsigned int    reserved_0                          : 3; /* [31:29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_v1_mrg3_csc_offset_out;

/* Define the union u_vout_g1_ctrl */
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
        unsigned int    reserved_2                          : 1; /* [7 : 7]  */
        unsigned int    cfg_rb_order                        : 1; /* [8 : 8]  */
        unsigned int    cfg_a_order                         : 1; /* [9 : 9]  */
        unsigned int    cfg_bitext                          : 2; /* [11 : 10]  */
        unsigned int    reserved_1                          : 4; /* [15 : 12]  */
        unsigned int    cfg_blend_mode                      : 1; /* [16 : 16]  */
        unsigned int    reserved_0                          : 3; /* [19 : 17]  */
        unsigned int    cfg_galpha_en                       : 1; /* [20 : 20]  */
        unsigned int    cfg_palpha_en                       : 1; /* [21 : 21]  */
        unsigned int    cfg_gpremul_en                      : 1; /* [22 : 22]  */
        unsigned int    cfg_ppremul_en                      : 1; /* [23 : 23]  */
        unsigned int    cfg_galpha                          : 8; /* [31 : 24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g1_ctrl;

/* Define the union u_vout_g1_upd */
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

} u_vout_g1_upd;

/* Define the union u_vout_g1_bk */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_bk_color                        : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g1_bk;

/* Define the union u_vout_g1_alpha_ext */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_alpha_0                         : 8; /* [7 : 0]  */
        unsigned int    cfg_alpha_1                         : 8; /* [15 : 8]  */
        unsigned int    reserved_0                          : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g1_alpha_ext;

/* Define the union u_vout_g1_mute_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mute_en                         : 1; /* [0 : 0]  */
        unsigned int    reserved_2                          : 3; /* [3 : 1]  */
        unsigned int    cfg_mute_mode                       : 1; /* [4 : 4]  */
        unsigned int    reserved_1                          : 3; /* [7 : 5]  */
        unsigned int    cfg_mute_pat                        : 1; /* [8 : 8]  */
        unsigned int    reserved_0                          : 7; /* [15 : 9]  */
        unsigned int    cfg_checker_h                       : 8; /* [23 : 16]  */
        unsigned int    cfg_checker_v                       : 8; /* [31 : 24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g1_mute_ctrl;

/* Define the union u_vout_g1_mute_color */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mute_color                      : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g1_mute_color;

/* Define the union u_vout_g1_oreso */
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

} u_vout_g1_oreso;

/* Define the union u_vout_g1_src_pos */
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

} u_vout_g1_src_pos;

/* Define the union u_vout_g1_src */
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

} u_vout_g1_src;

/* Define the union u_vout_g1_disp_pos */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_disp_xpos                       : 16; /* [15 : 0]  */
        unsigned int    cfg_disp_ypos                       : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g1_disp_pos;

/* Define the union u_vout_g1_disp */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_disp_h                          : 16; /* [15 : 0]  */
        unsigned int    cfg_disp_v                          : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g1_disp;

/* Define the union u_vout_g1_addr_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_src_addr_h                      : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g1_addr_h;

/* Define the union u_vout_g1_addr_l */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_src_addr_l                      : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g1_addr_l;

/* Define the union u_vout_g1_stride */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_src_stride                      : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g1_stride;

/* Define the union u_vout_g1_clut_rd_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_clut_rd_en                      : 1; /* [0 : 0]  */
        unsigned int    reserved_1                          : 7; /* [7 : 1]  */
        unsigned int    cfg_clut_rd_addr                    : 8; /* [15 : 8]  */
        unsigned int    reserved_0                          : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g1_clut_rd_ctrl;

/* Define the union u_vout_g1_clut_rd_data */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clut_rd_data                        : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g1_clut_rd_data;

/* Define the union u_vout_g1_key_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_key_en                          : 1; /* [0 : 0]  */
        unsigned int    cfg_key_sel                         : 1; /* [1 : 1]  */
        unsigned int    reserved_1                          : 6; /* [7 : 2]  */
        unsigned int    cfg_key_a_mode                      : 2; /* [9 : 8]  */
        unsigned int    cfg_key_y_mode                      : 2; /* [11 : 10]  */
        unsigned int    cfg_key_u_mode                      : 2; /* [13 : 12]  */
        unsigned int    cfg_key_v_mode                      : 2; /* [15 : 14]  */
        unsigned int    reserved_0                          : 16; /* [31 : 16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g1_key_ctrl;

/* Define the union u_vout_g1_key_max */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_key_a_max                       : 8; /* [7 : 0]  */
        unsigned int    cfg_key_y_max                       : 8; /* [15 : 8]  */
        unsigned int    cfg_key_u_max                       : 8; /* [23 : 16]  */
        unsigned int    cfg_key_v_max                       : 8; /* [31 : 24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g1_key_max;

/* Define the union u_vout_g1_key_min */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_key_a_min                       : 8; /* [7 : 0]  */
        unsigned int    cfg_key_y_min                       : 8; /* [15 : 8]  */
        unsigned int    cfg_key_u_min                       : 8; /* [23 : 16]  */
        unsigned int    cfg_key_v_min                       : 8; /* [31 : 24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g1_key_min;

/* Define the union u_vout_g1_key_msk */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_key_a_msk                       : 8; /* [7 : 0]  */
        unsigned int    cfg_key_y_msk                       : 8; /* [15 : 8]  */
        unsigned int    cfg_key_u_msk                       : 8; /* [23 : 16]  */
        unsigned int    cfg_key_v_msk                       : 8; /* [31 : 24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g1_key_msk;

/* Define the union u_vout_g1_csc_offset_in */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_offset_in_0                 : 8; /* [7:0]  */
        unsigned int    cfg_csc_offset_in_1                 : 8; /* [15:8]  */
        unsigned int    cfg_csc_offset_in_2                 : 8; /* [23:16]  */
        unsigned int    reserved_0                          : 7; /* [30:24]  */
        unsigned int    cfg_csc_en                          : 1; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g1_csc_offset_in;

/* Define the union u_vout_g1_csc_matrix_01 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_matrix_0                    : 16; /* [15:0]  */
        unsigned int    cfg_csc_matrix_1                    : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g1_csc_matrix_01;

/* Define the union u_vout_g1_csc_matrix_23 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_matrix_2                    : 16; /* [15:0]  */
        unsigned int    cfg_csc_matrix_3                    : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g1_csc_matrix_23;

/* Define the union u_vout_g1_csc_matrix_45 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_matrix_4                    : 16; /* [15:0]  */
        unsigned int    cfg_csc_matrix_5                    : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g1_csc_matrix_45;

/* Define the union u_vout_g1_csc_matrix_67 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_matrix_6                    : 16; /* [15:0]  */
        unsigned int    cfg_csc_matrix_7                    : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g1_csc_matrix_67;

/* Define the union u_vout_g1_csc_matrix_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_matrix_8                    : 16; /* [15:0]  */
        unsigned int    reserved_0                          : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g1_csc_matrix_8;

/* Define the union u_vout_g1_csc_offset_out */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_offset_out_0                : 9; /* [8:0]  */
        unsigned int    reserved_2                          : 1; /* [9]  */
        unsigned int    cfg_csc_offset_out_1                : 9; /* [18:10]  */
        unsigned int    reserved_1                          : 1; /* [19]  */
        unsigned int    cfg_csc_offset_out_2                : 9; /* [28:20]  */
        unsigned int    reserved_0                          : 3; /* [31:29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_g1_csc_offset_out;

/* Define the union u_vout_intf1_ctrl */
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

} u_vout_intf1_ctrl;

/* Define the union u_vout_intf1_upd */
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

} u_vout_intf1_upd;

/* Define the union u_vout_intf1_regup_pos */
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

} u_vout_intf1_regup_pos;

/* Define the union u_vout_intf1_start_pos */
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

} u_vout_intf1_start_pos;

/* Define the union u_vout_intf1_act */
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

} u_vout_intf1_act;

/* Define the union u_vout_intf1_vfb */
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

} u_vout_intf1_vfb;

/* Define the union u_vout_intf1_bvfb */
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

} u_vout_intf1_bvfb;

/* Define the union u_vout_intf1_hfb */
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

} u_vout_intf1_hfb;

/* Define the union u_vout_intf1_hs_w */
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

} u_vout_intf1_hs_w;

/* Define the union u_vout_intf1_field */
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

} u_vout_intf1_field;

/* Define the union u_vout_intf1_vs_h */
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

} u_vout_intf1_vs_h;

/* Define the union u_vout_intf1_vs_v */
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

} u_vout_intf1_vs_v;

/* Define the union u_vout_intf1_bvs_h */
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

} u_vout_intf1_bvs_h;

/* Define the union u_vout_intf1_bvs_v */
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

} u_vout_intf1_bvs_v;

/* Define the union u_vout_intf1_sfifo_thd */
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

} u_vout_intf1_sfifo_thd;

/* Define the union u_vout_intf1_vt_int */
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

} u_vout_intf1_vt_int;

/* Define the union u_vout_intf1_vt_intf1_thd0 */
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

} u_vout_intf1_vt_intf1_thd0;

/* Define the union u_vout_intf1_vt_intf1_thd2 */
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

} u_vout_intf1_vt_intf1_thd2;

/* Define the union u_vout_intf1_sta */
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

} u_vout_intf1_sta;

/* Define the union u_vout_intf1_sta2 */
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

} u_vout_intf1_sta2;

/* Define the union u_vout_intf1_uf_line */
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

} u_vout_intf1_uf_line;

/* Define the union u_vout_intf1_y_sum */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    intf_y_sum                          : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf1_y_sum;

/* Define the union u_vout_intf1_u_sum */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    intf_u_sum                          : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf1_u_sum;

/* Define the union u_vout_intf1_v_sum */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    intf_v_sum                          : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf1_v_sum;

/* Define the union u_vout_intf1_bt_ctrl */
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

} u_vout_intf1_bt_ctrl;

/* Define the union u_vout_intf1_bt_vbit */
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

} u_vout_intf1_bt_vbit;

/* Define the union u_vout_intf1_bt_bvbit */
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

} u_vout_intf1_bt_bvbit;

/* Define the union u_vout_intf1_lcd_ctrl */
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

} u_vout_intf1_lcd_ctrl;

/* Define the union u_vout_intf1_lvds_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_lvds_mode                       : 3; /* [2 : 0]  */
        unsigned int    reserved_1                          : 1; /* [3 : 3]  */
        unsigned int    cfg_lvds_lane_0_sel                 : 4; /* [7:4]  */
        unsigned int    cfg_lvds_lane_1_sel                 : 4; /* [11:8]  */
        unsigned int    cfg_lvds_lane_2_sel                 : 4; /* [15:12]  */
        unsigned int    cfg_lvds_lane_3_sel                 : 4; /* [19:16]  */
        unsigned int    cfg_lvds_lane_4_sel                 : 4; /* [23:20]  */
        unsigned int    reserved_0                          : 8; /* [31:24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf1_lvds_ctrl;

/* Define the union u_vout_intf1_mipi_ctrl */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_dpi_mode                        : 3; /* [2 : 0]  */
        unsigned int    cfg_dpi_uv_order                    : 1; /* [3]  */
        unsigned int    reserved_0                          : 28; /* [31:4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf1_mipi_ctrl;

/* Define the union u_vout_intf1_mute_ctrl */
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

} u_vout_intf1_mute_ctrl;

/* Define the union u_vout_intf1_mute_color */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_mute_color                      : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf1_mute_color;

/* Define the union u_vout_intf1_mute_bk */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_bk_color                        : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf1_mute_bk;

/* Define the union u_vout_intf1_ink_ctrl */
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

} u_vout_intf1_ink_ctrl;

/* Define the union u_vout_intf1_ink_pos */
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

} u_vout_intf1_ink_pos;

/* Define the union u_vout_intf1_ink_color */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_cross_color                     : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf1_ink_color;

/* Define the union u_vout_intf1_ink_data */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ink_data                            : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf1_ink_data;

/* Define the union u_vout_intf1_clip_l */
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

} u_vout_intf1_clip_l;

/* Define the union u_vout_intf1_clip_h */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_clip_yuv_h                      : 32; /* [31 : 0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf1_clip_h;

/* Define the union u_vout_intf1_csc_offset_in */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_offset_in_0                 : 8; /* [7:0]  */
        unsigned int    cfg_csc_offset_in_1                 : 8; /* [15:8]  */
        unsigned int    cfg_csc_offset_in_2                 : 8; /* [23:16]  */
        unsigned int    reserved_0                          : 7; /* [30:24]  */
        unsigned int    cfg_csc_en                          : 1; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf1_csc_offset_in;

/* Define the union u_vout_intf1_csc_matrix_01 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_matrix_0                    : 16; /* [15:0]  */
        unsigned int    cfg_csc_matrix_1                    : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf1_csc_matrix_01;

/* Define the union u_vout_intf1_csc_matrix_23 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_matrix_2                    : 16; /* [15:0]  */
        unsigned int    cfg_csc_matrix_3                    : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf1_csc_matrix_23;

/* Define the union u_vout_intf1_csc_matrix_45 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_matrix_4                    : 16; /* [15:0]  */
        unsigned int    cfg_csc_matrix_5                    : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf1_csc_matrix_45;

/* Define the union u_vout_intf1_csc_matrix_67 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_matrix_6                    : 16; /* [15:0]  */
        unsigned int    cfg_csc_matrix_7                    : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf1_csc_matrix_67;

/* Define the union u_vout_intf1_csc_matrix_8 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_matrix_8                    : 16; /* [15:0]  */
        unsigned int    reserved_0                          : 16; /* [31:16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf1_csc_matrix_8;

/* Define the union u_vout_intf1_csc_offset_out */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cfg_csc_offset_out_0                : 9; /* [8:0]  */
        unsigned int    reserved_2                          : 1; /* [9]  */
        unsigned int    cfg_csc_offset_out_1                : 9; /* [18:10]  */
        unsigned int    reserved_1                          : 1; /* [19]  */
        unsigned int    cfg_csc_offset_out_2                : 9; /* [28:20]  */
        unsigned int    reserved_0                          : 3; /* [31:29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} u_vout_intf1_csc_offset_out;

/* Define the union u_vout_intf1_hcds_ctrl */
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

} u_vout_intf1_hcds_ctrl;

/* Define the union u_vout_intf1_hcds_coef */
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

} u_vout_intf1_hcds_coef;

/* Define the union u_vout_intf1_dither_ctrl */
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

} u_vout_intf1_dither_ctrl;

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
    volatile xmedia_u32                           reserved1                     ;/*0x1c*/
    volatile u_vout_int_sta                       vout_int_sta                  ;/*0x20*/
    volatile u_vout_int_clr                       vout_int_clr                  ;/*0x24*/
    volatile u_vout_int_msk                       vout_int_msk                  ;/*0x28*/
    volatile u_vout_int_src_sta                   vout_int_src_sta              ;/*0x2c*/
    volatile xmedia_u32                           reserved2[4]                  ;/*0x30*/
    volatile u_vout_sta_clr                       vout_sta_clr                  ;/*0x40*/
    volatile u_vout_debug                         vout_debug                    ;/*0x44*/
    volatile xmedia_u32                           reserved3[18]                 ;/*0x48*/
    volatile u_vout_dma_dfx_ctrl                  vout_dma_dfx_ctrl             ;/*0x90*/
    volatile xmedia_u32                           reserved4[15]                 ;/*0x94*/
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
    volatile xmedia_u32                           reserved5                     ;/*0xfc*/
    volatile u_vout_cbm_mixer                     vout_cbm_mixer                ;/*0x100*/
    volatile xmedia_u32                           vout_cbm_bk                   ;/*0x104*/
    volatile xmedia_u32                           reserved6[2]                  ;/*0x108*/
    volatile u_vout_cbm1_mixer                    vout_cbm1_mixer               ;/*0x110*/
    volatile xmedia_u32                           vout_cbm1_bk                  ;/*0x114*/
    volatile xmedia_u32                           reserved7[26]                 ;/*0x118*/
    volatile u_vout_dma_ctrl                      vout_dma_ctrl                 ;/*0x180*/
    volatile u_vout_dma_ctrl_1                    vout_dma_ctrl_1               ;/*0x184*/
    volatile u_vout_dma_ost_num                   vout_dma_ost_num              ;/*0x188*/
    volatile u_vout_dma_ost_num_1                 vout_dma_ost_num_1            ;/*0x18c*/
    volatile u_vout_dma_ost_num_2                 vout_dma_ost_num_2            ;/*0x190*/
    volatile u_vout_dma_timeout_th                vout_dma_timeout_th           ;/*0x194*/
    volatile xmedia_u32                           reserved8[26]                 ;/*0x19c*/
    volatile u_vout_para_update                   vout_para_update              ;/*0x200*/
    volatile u_vout_para_0_addr_h                 vout_para_0_addr_h            ;/*0x204*/
    volatile u_vout_para_0_addr_l                 vout_para_0_addr_l            ;/*0x208*/
    volatile u_vout_para_1_addr_h                 vout_para_1_addr_h            ;/*0x20c*/
    volatile u_vout_para_1_addr_l                 vout_para_1_addr_l            ;/*0x210*/
    volatile xmedia_u32                           reserved9[60]                 ;/*0x214*/
    volatile u_vout_para_2_addr_h                 vout_para_2_addr_h            ;/*0x304*/
    volatile u_vout_para_2_addr_l                 vout_para_2_addr_l            ;/*0x308*/
    volatile u_vout_para_3_addr_h                 vout_para_3_addr_h            ;/*0x30c*/
    volatile u_vout_para_3_addr_l                 vout_para_3_addr_l            ;/*0x310*/
    volatile xmedia_u32                           reserved105[315]              ;/*0x314*/
    volatile u_vout_lvds_phy_update               vout_lvds_phy_update          ;/*0x800*/
    volatile u_vout_lvds_phy_ctrl                 vout_lvds_phy_ctrl            ;/*0x804*/
    volatile u_vout_lvds_pll_ctrl                 vout_lvds_pll_ctrl            ;/*0x808*/
    volatile u_vout_lvds_phy_dbg                  vout_lvds_phy_dbg             ;/*0x80c*/
    volatile u_vout_lvds_phy_sta                  vout_lvds_phy_sta             ;/*0x810*/
    volatile xmedia_u32                           reserved104[11]               ;/*0x814*/
    volatile u_vout_lvds_phy_en                   vout_lvds_phy_en              ;/*0x840*/
    volatile xmedia_u32                           reserved10[495]               ;/*0x844*/
    volatile u_vout_v0_ctrl                       vout_v0_ctrl                  ;/*0x1000*/
    volatile u_vout_v0_upd                        vout_v0_upd                   ;/*0x1004*/
    volatile xmedia_u32                           reserved11[2]                 ;/*0x1008*/
    volatile u_vout_v0_bk                         vout_v0_bk                    ;/*0x1010*/
    volatile xmedia_u32                           reserved12[3]                 ;/*0x1014*/
    volatile u_vout_v0_mute_ctrl                  vout_v0_mute_ctrl             ;/*0x1020*/
    volatile u_vout_v0_mute_color                 vout_v0_mute_color            ;/*0x1024*/
    volatile xmedia_u32                           reserved13[2]                 ;/*0x1028*/
    volatile u_vout_v0_oreso                      vout_v0_oreso                 ;/*0x1030*/
    volatile xmedia_u32                           reserved14[3]                 ;/*0x1034*/
    volatile u_vout_v0_src_pos                    vout_v0_src_pos               ;/*0x1040*/
    volatile u_vout_v0_src                        vout_v0_src                   ;/*0x1044*/
    volatile u_vout_v0_disp_pos                   vout_v0_disp_pos              ;/*0x1048*/
    volatile u_vout_v0_disp                       vout_v0_disp                  ;/*0x104c*/
    volatile u_vout_v0_addr_y_h                   vout_v0_addr_y_h              ;/*0x1050*/
    volatile u_vout_v0_addr_y_l                   vout_v0_addr_y_l              ;/*0x1054*/
    volatile u_vout_v0_stride_y                   vout_v0_stride_y              ;/*0x1058*/
    volatile xmedia_u32                           reserved15                    ;/*0x105c*/
    volatile u_vout_v0_addr_c_h                   vout_v0_addr_c_h              ;/*0x1060*/
    volatile u_vout_v0_addr_c_l                   vout_v0_addr_c_l              ;/*0x1064*/
    volatile u_vout_v0_stride_c                   vout_v0_stride_c              ;/*0x1068*/
    volatile xmedia_u32                           reserved16[37]                ;/*0x106c*/
    volatile u_vout_v0_mrg_ctrl                   vout_v0_mrg_ctrl              ;/*0x1100*/
    volatile u_vout_v0_mrg_sort                   vout_v0_mrg_sort              ;/*0x1104*/
    volatile xmedia_u32                           reserved17[14]                ;/*0x1108*/
    volatile u_vout_v0_mrg0_addr_y_h              vout_v0_mrg0_addr_y_h         ;/*0x1140*/
    volatile u_vout_v0_mrg0_addr_y_l              vout_v0_mrg0_addr_y_l         ;/*0x1144*/
    volatile u_vout_v0_mrg0_stride_y              vout_v0_mrg0_stride_y         ;/*0x1148*/
    volatile xmedia_u32                           reserved18                    ;/*0x114c*/
    volatile u_vout_v0_mrg0_addr_c_h              vout_v0_mrg0_addr_c_h         ;/*0x1150*/
    volatile u_vout_v0_mrg0_addr_c_l              vout_v0_mrg0_addr_c_l         ;/*0x1154*/
    volatile u_vout_v0_mrg0_stride_c              vout_v0_mrg0_stride_c         ;/*0x1158*/
    volatile xmedia_u32                           reserved19                    ;/*0x115c*/
    volatile u_vout_v0_mrg0_src_pos               vout_v0_mrg0_src_pos          ;/*0x1160*/
    volatile u_vout_v0_mrg0_src                   vout_v0_mrg0_src              ;/*0x1164*/
    volatile u_vout_v0_mrg0_mute_color            vout_v0_mrg0_mute_color       ;/*0x1168*/
    volatile xmedia_u32                           reserved20                    ;/*0x116c*/
    volatile u_vout_v0_mrg1_addr_y_h              vout_v0_mrg1_addr_y_h         ;/*0x1170*/
    volatile u_vout_v0_mrg1_addr_y_l              vout_v0_mrg1_addr_y_l         ;/*0x1174*/
    volatile u_vout_v0_mrg1_stride_y              vout_v0_mrg1_stride_y         ;/*0x1178*/
    volatile xmedia_u32                           reserved21                    ;/*0x117c*/
    volatile u_vout_v0_mrg1_addr_c_h              vout_v0_mrg1_addr_c_h         ;/*0x1180*/
    volatile u_vout_v0_mrg1_addr_c_l              vout_v0_mrg1_addr_c_l         ;/*0x1184*/
    volatile u_vout_v0_mrg1_stride_c              vout_v0_mrg1_stride_c         ;/*0x1188*/
    volatile xmedia_u32                           reserved22                    ;/*0x118c*/
    volatile u_vout_v0_mrg1_src_pos               vout_v0_mrg1_src_pos          ;/*0x1190*/
    volatile u_vout_v0_mrg1_src                   vout_v0_mrg1_src              ;/*0x1194*/
    volatile u_vout_v0_mrg1_mute_color            vout_v0_mrg1_mute_color       ;/*0x1198*/
    volatile xmedia_u32                           reserved23                    ;/*0x119c*/
    volatile u_vout_v0_mrg2_addr_y_h              vout_v0_mrg2_addr_y_h         ;/*0x11a0*/
    volatile u_vout_v0_mrg2_addr_y_l              vout_v0_mrg2_addr_y_l         ;/*0x11a4*/
    volatile u_vout_v0_mrg2_stride_y              vout_v0_mrg2_stride_y         ;/*0x11a8*/
    volatile xmedia_u32                           reserved24                    ;/*0x11ac*/
    volatile u_vout_v0_mrg2_addr_c_h              vout_v0_mrg2_addr_c_h         ;/*0x11b0*/
    volatile u_vout_v0_mrg2_addr_c_l              vout_v0_mrg2_addr_c_l         ;/*0x11b4*/
    volatile u_vout_v0_mrg2_stride_c              vout_v0_mrg2_stride_c         ;/*0x11b8*/
    volatile xmedia_u32                           reserved25                    ;/*0x11bc*/
    volatile u_vout_v0_mrg2_src_pos               vout_v0_mrg2_src_pos          ;/*0x11c0*/
    volatile u_vout_v0_mrg2_src                   vout_v0_mrg2_src              ;/*0x11c4*/
    volatile u_vout_v0_mrg2_mute_color            vout_v0_mrg2_mute_color       ;/*0x11c8*/
    volatile xmedia_u32                           reserved26                    ;/*0x11cc*/
    volatile u_vout_v0_mrg3_addr_y_h              vout_v0_mrg3_addr_y_h         ;/*0x11d0*/
    volatile u_vout_v0_mrg3_addr_y_l              vout_v0_mrg3_addr_y_l         ;/*0x11d4*/
    volatile u_vout_v0_mrg3_stride_y              vout_v0_mrg3_stride_y         ;/*0x11d8*/
    volatile xmedia_u32                           reserved27                    ;/*0x11dc*/
    volatile u_vout_v0_mrg3_addr_c_h              vout_v0_mrg3_addr_c_h         ;/*0x11e0*/
    volatile u_vout_v0_mrg3_addr_c_l              vout_v0_mrg3_addr_c_l         ;/*0x11e4*/
    volatile u_vout_v0_mrg3_stride_c              vout_v0_mrg3_stride_c         ;/*0x11e8*/
    volatile xmedia_u32                           reserved28                    ;/*0x11ec*/
    volatile u_vout_v0_mrg3_src_pos               vout_v0_mrg3_src_pos          ;/*0x11f0*/
    volatile u_vout_v0_mrg3_src                   vout_v0_mrg3_src              ;/*0x11f4*/
    volatile u_vout_v0_mrg3_mute_color            vout_v0_mrg3_mute_color       ;/*0x11f8*/
    volatile xmedia_u32                           reserved29                    ;/*0x11fc*/
    volatile u_vout_v0_cus_ctrl                   vout_v0_cus_ctrl              ;/*0x1200*/
    volatile u_vout_v0_vcus_coef                  vout_v0_vcus_coef             ;/*0x1204*/
    volatile u_vout_v0_hcus_coef                  vout_v0_hcus_coef             ;/*0x1208*/
    volatile xmedia_u32                           reserved30[29]                ;/*0x120c*/
    volatile u_vout_v0_mrg0_csc_offset_in         vout_v0_mrg0_csc_offset_in    ;/*0x1280*/
    volatile u_vout_v0_mrg0_csc_matrix_01         vout_v0_mrg0_csc_matrix_01    ;/*0x1284*/
    volatile u_vout_v0_mrg0_csc_matrix_23         vout_v0_mrg0_csc_matrix_23    ;/*0x1288*/
    volatile u_vout_v0_mrg0_csc_matrix_45         vout_v0_mrg0_csc_matrix_45    ;/*0x128c*/
    volatile u_vout_v0_mrg0_csc_matrix_67         vout_v0_mrg0_csc_matrix_67    ;/*0x1290*/
    volatile u_vout_v0_mrg0_csc_matrix_8          vout_v0_mrg0_csc_matrix_8     ;/*0x1294*/
    volatile xmedia_u32                           reserved31                    ;/*0x1298*/
    volatile u_vout_v0_mrg0_csc_offset_out        vout_v0_mrg0_csc_offset_out   ;/*0x129c*/
    volatile u_vout_v0_mrg1_csc_offset_in         vout_v0_mrg1_csc_offset_in    ;/*0x12a0*/
    volatile u_vout_v0_mrg1_csc_matrix_01         vout_v0_mrg1_csc_matrix_01    ;/*0x12a4*/
    volatile u_vout_v0_mrg1_csc_matrix_23         vout_v0_mrg1_csc_matrix_23    ;/*0x12a8*/
    volatile u_vout_v0_mrg1_csc_matrix_45         vout_v0_mrg1_csc_matrix_45    ;/*0x12ac*/
    volatile u_vout_v0_mrg1_csc_matrix_67         vout_v0_mrg1_csc_matrix_67    ;/*0x12b0*/
    volatile u_vout_v0_mrg1_csc_matrix_8          vout_v0_mrg1_csc_matrix_8     ;/*0x12b4*/
    volatile xmedia_u32                           reserved32                    ;/*0x12b8*/
    volatile u_vout_v0_mrg1_csc_offset_out        vout_v0_mrg1_csc_offset_out   ;/*0x12bc*/
    volatile u_vout_v0_mrg2_csc_offset_in         vout_v0_mrg2_csc_offset_in    ;/*0x12c0*/
    volatile u_vout_v0_mrg2_csc_matrix_01         vout_v0_mrg2_csc_matrix_01    ;/*0x12c4*/
    volatile u_vout_v0_mrg2_csc_matrix_23         vout_v0_mrg2_csc_matrix_23    ;/*0x12c8*/
    volatile u_vout_v0_mrg2_csc_matrix_45         vout_v0_mrg2_csc_matrix_45    ;/*0x12cc*/
    volatile u_vout_v0_mrg2_csc_matrix_67         vout_v0_mrg2_csc_matrix_67    ;/*0x12d0*/
    volatile u_vout_v0_mrg2_csc_matrix_8          vout_v0_mrg2_csc_matrix_8     ;/*0x12d4*/
    volatile xmedia_u32                           reserved33                    ;/*0x12d8*/
    volatile u_vout_v0_mrg2_csc_offset_out        vout_v0_mrg2_csc_offset_out   ;/*0x12dc*/
    volatile u_vout_v0_mrg3_csc_offset_in         vout_v0_mrg3_csc_offset_in    ;/*0x12e0*/
    volatile u_vout_v0_mrg3_csc_matrix_01         vout_v0_mrg3_csc_matrix_01    ;/*0x12e4*/
    volatile u_vout_v0_mrg3_csc_matrix_23         vout_v0_mrg3_csc_matrix_23    ;/*0x12e8*/
    volatile u_vout_v0_mrg3_csc_matrix_45         vout_v0_mrg3_csc_matrix_45    ;/*0x12ec*/
    volatile u_vout_v0_mrg3_csc_matrix_67         vout_v0_mrg3_csc_matrix_67    ;/*0x12f0*/
    volatile u_vout_v0_mrg3_csc_matrix_8          vout_v0_mrg3_csc_matrix_8     ;/*0x12f4*/
    volatile xmedia_u32                           reserved34                    ;/*0x12f8*/
    volatile u_vout_v0_mrg3_csc_offset_out        vout_v0_mrg3_csc_offset_out   ;/*0x12fc*/
    volatile xmedia_u32                           reserved35[832]               ;/*0x1300*/
    volatile u_vout_g0_ctrl                       vout_g0_ctrl                  ;/*0x2000*/
    volatile u_vout_g0_upd                        vout_g0_upd                   ;/*0x2004*/
    volatile xmedia_u32                           reserved36[2]                 ;/*0x2008*/
    volatile u_vout_g0_bk                         vout_g0_bk                    ;/*0x2010*/
    volatile u_vout_g0_alpha_ext                  vout_g0_alpha_ext             ;/*0x2014*/
    volatile xmedia_u32                           reserved37[2]                 ;/*0x2018*/
    volatile u_vout_g0_mute_ctrl                  vout_g0_mute_ctrl             ;/*0x2020*/
    volatile u_vout_g0_mute_color                 vout_g0_mute_color            ;/*0x2024*/
    volatile xmedia_u32                           reserved38[2]                 ;/*0x2028*/
    volatile u_vout_g0_oreso                      vout_g0_oreso                 ;/*0x2030*/
    volatile xmedia_u32                           reserved39[3]                 ;/*0x2034*/
    volatile u_vout_g0_src_pos                    vout_g0_src_pos               ;/*0x2040*/
    volatile u_vout_g0_src                        vout_g0_src                   ;/*0x2044*/
    volatile u_vout_g0_disp_pos                   vout_g0_disp_pos              ;/*0x2048*/
    volatile u_vout_g0_disp                       vout_g0_disp                  ;/*0x204c*/
    volatile u_vout_g0_addr_h                     vout_g0_addr_h                ;/*0x2050*/
    volatile u_vout_g0_addr_l                     vout_g0_addr_l                ;/*0x2054*/
    volatile u_vout_g0_stride                     vout_g0_stride                ;/*0x2058*/
    volatile xmedia_u32                           reserved40[9]                 ;/*0x205c*/
    volatile u_vout_g0_clut_rd_ctrl               vout_g0_clut_rd_ctrl          ;/*0x2080*/
    volatile u_vout_g0_clut_rd_data               vout_g0_clut_rd_data          ;/*0x2084*/
    volatile xmedia_u32                           reserved41[30]                ;/*0x2088*/
    volatile u_vout_g0_key_ctrl                   vout_g0_key_ctrl              ;/*0x2100*/
    volatile u_vout_g0_key_max                    vout_g0_key_max               ;/*0x2104*/
    volatile u_vout_g0_key_min                    vout_g0_key_min               ;/*0x2108*/
    volatile u_vout_g0_key_msk                    vout_g0_key_msk               ;/*0x210c*/
    volatile xmedia_u32                           reserved42[12]                ;/*0x2110*/
    volatile u_vout_g0_csc_offset_in              vout_g0_csc_offset_in         ;/*0x2140*/
    volatile u_vout_g0_csc_matrix_01              vout_g0_csc_matrix_01         ;/*0x2144*/
    volatile u_vout_g0_csc_matrix_23              vout_g0_csc_matrix_23         ;/*0x2148*/
    volatile u_vout_g0_csc_matrix_45              vout_g0_csc_matrix_45         ;/*0x214c*/
    volatile u_vout_g0_csc_matrix_67              vout_g0_csc_matrix_67         ;/*0x2150*/
    volatile u_vout_g0_csc_matrix_8               vout_g0_csc_matrix_8          ;/*0x2154*/
    volatile xmedia_u32                           reserved43                    ;/*0x2158*/
    volatile u_vout_g0_csc_offset_out             vout_g0_csc_offset_out        ;/*0x215c*/
    volatile xmedia_u32                           reserved44[936]               ;/*0x2160*/
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
    volatile xmedia_u32                           reserved45[17]                ;/*0x303c*/
    volatile u_vout_intf_vt_int                   vout_intf_vt_int              ;/*0x3080*/
    volatile u_vout_intf_vt_intf_thd0             vout_intf_vt_intf_thd0        ;/*0x3084*/
    volatile u_vout_intf_vt_intf_thd2             vout_intf_vt_intf_thd2        ;/*0x3088*/
    volatile xmedia_u32                           reserved46                    ;/*0x308c*/
    volatile u_vout_intf_sta                      vout_intf_sta                 ;/*0x3090*/
    volatile u_vout_intf_sta2                     vout_intf_sta2                ;/*0x3094*/
    volatile u_vout_intf_uf_line                  vout_intf_uf_line             ;/*0x3098*/
    volatile u_vout_intf_y_sum                    vout_intf_y_sum               ;/*0x309c*/
    volatile u_vout_intf_u_sum                    vout_intf_u_sum               ;/*0x30a0*/
    volatile u_vout_intf_v_sum                    vout_intf_v_sum               ;/*0x30a4*/
    volatile xmedia_u32                           reserved47[22]                ;/*0x30a8*/
    volatile u_vout_intf_bt_ctrl                  vout_intf_bt_ctrl             ;/*0x3100*/
    volatile u_vout_intf_bt_vbit                  vout_intf_bt_vbit             ;/*0x3104*/
    volatile u_vout_intf_bt_bvbit                 vout_intf_bt_bvbit            ;/*0x3108*/
    volatile xmedia_u32                           reserved48[5]                 ;/*0x310c*/
    volatile u_vout_intf_lcd_ctrl                 vout_intf_lcd_ctrl            ;/*0x3120*/
    volatile xmedia_u32                           reserved49[7]                 ;/*0x3124*/
    volatile u_vout_intf_lvds_ctrl                vout_intf_lvds_ctrl           ;/*0x3140*/
    volatile xmedia_u32                           reserved50[15]                ;/*0x3144*/
    volatile u_vout_intf_mipi_ctrl                vout_intf_mipi_ctrl           ;/*0x3180*/
    volatile xmedia_u32                           reserved51[31]                ;/*0x3184*/
    volatile u_vout_intf_mute_ctrl                vout_intf_mute_ctrl           ;/*0x3200*/
    volatile u_vout_intf_mute_color               vout_intf_mute_color          ;/*0x3204*/
    volatile u_vout_intf_mute_bk                  vout_intf_mute_bk             ;/*0x3208*/
    volatile xmedia_u32                           reserved52                    ;/*0x320c*/
    volatile u_vout_intf_ink_ctrl                 vout_intf_ink_ctrl            ;/*0x3210*/
    volatile u_vout_intf_ink_pos                  vout_intf_ink_pos             ;/*0x3214*/
    volatile u_vout_intf_ink_color                vout_intf_ink_color           ;/*0x3218*/
    volatile u_vout_intf_ink_data                 vout_intf_ink_data            ;/*0x321c*/
    volatile u_vout_intf_clip_l                   vout_intf_clip_l              ;/*0x3220*/
    volatile u_vout_intf_clip_h                   vout_intf_clip_h              ;/*0x3224*/
    volatile xmedia_u32                           reserved53[54]                ;/*0x3228*/
    volatile u_vout_intf_csc_offset_in            vout_intf_csc_offset_in       ;/*0x3300*/
    volatile u_vout_intf_csc_matrix_01            vout_intf_csc_matrix_01       ;/*0x3304*/
    volatile u_vout_intf_csc_matrix_23            vout_intf_csc_matrix_23       ;/*0x3308*/
    volatile u_vout_intf_csc_matrix_45            vout_intf_csc_matrix_45       ;/*0x330c*/
    volatile u_vout_intf_csc_matrix_67            vout_intf_csc_matrix_67       ;/*0x3310*/
    volatile u_vout_intf_csc_matrix_8             vout_intf_csc_matrix_8        ;/*0x3314*/
    volatile xmedia_u32                           reserved54                    ;/*0x3318*/
    volatile u_vout_intf_csc_offset_out           vout_intf_csc_offset_out      ;/*0x331c*/
    volatile xmedia_u32                           reserved55[8]                 ;/*0x3320*/
    volatile u_vout_intf_hcds_ctrl                vout_intf_hcds_ctrl           ;/*0x3340*/
    volatile u_vout_intf_hcds_coef                vout_intf_hcds_coef           ;/*0x3344*/
    volatile xmedia_u32                           reserved56[6]                 ;/*0x3348*/
    volatile u_vout_intf_dither_ctrl              vout_intf_dither_ctrl         ;/*0x3360*/
    volatile xmedia_u32                           reserved57[807]               ;/*0x3364*/
    volatile u_vout_v1_ctrl                       vout_v1_ctrl                  ;/*0x4000*/
    volatile u_vout_v1_upd                        vout_v1_upd                   ;/*0x4004*/
    volatile xmedia_u32                           reserved58[2]                 ;/*0x4008*/
    volatile u_vout_v1_bk                         vout_v1_bk                    ;/*0x4010*/
    volatile xmedia_u32                           reserved59[3]                 ;/*0x4014*/
    volatile u_vout_v1_mute_ctrl                  vout_v1_mute_ctrl             ;/*0x4020*/
    volatile u_vout_v1_mute_color                 vout_v1_mute_color            ;/*0x4024*/
    volatile xmedia_u32                           reserved60[2]                 ;/*0x4028*/
    volatile u_vout_v1_oreso                      vout_v1_oreso                 ;/*0x4030*/
    volatile xmedia_u32                           reserved61[3]                 ;/*0x4034*/
    volatile u_vout_v1_src_pos                    vout_v1_src_pos               ;/*0x4040*/
    volatile u_vout_v1_src                        vout_v1_src                   ;/*0x4044*/
    volatile u_vout_v1_disp_pos                   vout_v1_disp_pos              ;/*0x4048*/
    volatile u_vout_v1_disp                       vout_v1_disp                  ;/*0x404c*/
    volatile u_vout_v1_addr_y_h                   vout_v1_addr_y_h              ;/*0x4050*/
    volatile u_vout_v1_addr_y_l                   vout_v1_addr_y_l              ;/*0x4054*/
    volatile u_vout_v1_stride_y                   vout_v1_stride_y              ;/*0x4058*/
    volatile xmedia_u32                           reserved62                    ;/*0x405c*/
    volatile u_vout_v1_addr_c_h                   vout_v1_addr_c_h              ;/*0x4060*/
    volatile u_vout_v1_addr_c_l                   vout_v1_addr_c_l              ;/*0x4064*/
    volatile u_vout_v1_stride_c                   vout_v1_stride_c              ;/*0x4068*/
    volatile xmedia_u32                           reserved63[37]                ;/*0x406c*/
    volatile u_vout_v1_mrg_ctrl                   vout_v1_mrg_ctrl              ;/*0x4100*/
    volatile u_vout_v1_mrg_sort                   vout_v1_mrg_sort              ;/*0x4104*/
    volatile xmedia_u32                           reserved64[14]                ;/*0x4108*/
    volatile u_vout_v1_mrg0_addr_y_h              vout_v1_mrg0_addr_y_h         ;/*0x4140*/
    volatile u_vout_v1_mrg0_addr_y_l              vout_v1_mrg0_addr_y_l         ;/*0x4144*/
    volatile u_vout_v1_mrg0_stride_y              vout_v1_mrg0_stride_y         ;/*0x4148*/
    volatile xmedia_u32                           reserved65                    ;/*0x414c*/
    volatile u_vout_v1_mrg0_addr_c_h              vout_v1_mrg0_addr_c_h         ;/*0x4150*/
    volatile u_vout_v1_mrg0_addr_c_l              vout_v1_mrg0_addr_c_l         ;/*0x4154*/
    volatile u_vout_v1_mrg0_stride_c              vout_v1_mrg0_stride_c         ;/*0x4158*/
    volatile xmedia_u32                           reserved66                    ;/*0x415c*/
    volatile u_vout_v1_mrg0_src_pos               vout_v1_mrg0_src_pos          ;/*0x4160*/
    volatile u_vout_v1_mrg0_src                   vout_v1_mrg0_src              ;/*0x4164*/
    volatile u_vout_v1_mrg0_mute_color            vout_v1_mrg0_mute_color       ;/*0x4168*/
    volatile xmedia_u32                           reserved67                    ;/*0x416c*/
    volatile u_vout_v1_mrg1_addr_y_h              vout_v1_mrg1_addr_y_h         ;/*0x4170*/
    volatile u_vout_v1_mrg1_addr_y_l              vout_v1_mrg1_addr_y_l         ;/*0x4174*/
    volatile u_vout_v1_mrg1_stride_y              vout_v1_mrg1_stride_y         ;/*0x4178*/
    volatile xmedia_u32                           reserved68                    ;/*0x417c*/
    volatile u_vout_v1_mrg1_addr_c_h              vout_v1_mrg1_addr_c_h         ;/*0x4180*/
    volatile u_vout_v1_mrg1_addr_c_l              vout_v1_mrg1_addr_c_l         ;/*0x4184*/
    volatile u_vout_v1_mrg1_stride_c              vout_v1_mrg1_stride_c         ;/*0x4188*/
    volatile xmedia_u32                           reserved69                    ;/*0x418c*/
    volatile u_vout_v1_mrg1_src_pos               vout_v1_mrg1_src_pos          ;/*0x4190*/
    volatile u_vout_v1_mrg1_src                   vout_v1_mrg1_src              ;/*0x4194*/
    volatile u_vout_v1_mrg1_mute_color            vout_v1_mrg1_mute_color       ;/*0x4198*/
    volatile xmedia_u32                           reserved70                    ;/*0x419c*/
    volatile u_vout_v1_mrg2_addr_y_h              vout_v1_mrg2_addr_y_h         ;/*0x41a0*/
    volatile u_vout_v1_mrg2_addr_y_l              vout_v1_mrg2_addr_y_l         ;/*0x41a4*/
    volatile u_vout_v1_mrg2_stride_y              vout_v1_mrg2_stride_y         ;/*0x41a8*/
    volatile xmedia_u32                           reserved71                    ;/*0x41ac*/
    volatile u_vout_v1_mrg2_addr_c_h              vout_v1_mrg2_addr_c_h         ;/*0x41b0*/
    volatile u_vout_v1_mrg2_addr_c_l              vout_v1_mrg2_addr_c_l         ;/*0x41b4*/
    volatile u_vout_v1_mrg2_stride_c              vout_v1_mrg2_stride_c         ;/*0x41b8*/
    volatile xmedia_u32                           reserved72                    ;/*0x41bc*/
    volatile u_vout_v1_mrg2_src_pos               vout_v1_mrg2_src_pos          ;/*0x41c0*/
    volatile u_vout_v1_mrg2_src                   vout_v1_mrg2_src              ;/*0x41c4*/
    volatile u_vout_v1_mrg2_mute_color            vout_v1_mrg2_mute_color       ;/*0x41c8*/
    volatile xmedia_u32                           reserved73                    ;/*0x41cc*/
    volatile u_vout_v1_mrg3_addr_y_h              vout_v1_mrg3_addr_y_h         ;/*0x41d0*/
    volatile u_vout_v1_mrg3_addr_y_l              vout_v1_mrg3_addr_y_l         ;/*0x41d4*/
    volatile u_vout_v1_mrg3_stride_y              vout_v1_mrg3_stride_y         ;/*0x41d8*/
    volatile xmedia_u32                           reserved74                    ;/*0x41dc*/
    volatile u_vout_v1_mrg3_addr_c_h              vout_v1_mrg3_addr_c_h         ;/*0x41e0*/
    volatile u_vout_v1_mrg3_addr_c_l              vout_v1_mrg3_addr_c_l         ;/*0x41e4*/
    volatile u_vout_v1_mrg3_stride_c              vout_v1_mrg3_stride_c         ;/*0x41e8*/
    volatile xmedia_u32                           reserved75                    ;/*0x41ec*/
    volatile u_vout_v1_mrg3_src_pos               vout_v1_mrg3_src_pos          ;/*0x41f0*/
    volatile u_vout_v1_mrg3_src                   vout_v1_mrg3_src              ;/*0x41f4*/
    volatile u_vout_v1_mrg3_mute_color            vout_v1_mrg3_mute_color       ;/*0x41f8*/
    volatile xmedia_u32                           reserved76                    ;/*0x41fc*/
    volatile u_vout_v1_cus_ctrl                   vout_v1_cus_ctrl              ;/*0x4200*/
    volatile u_vout_v1_vcus_coef                  vout_v1_vcus_coef             ;/*0x4204*/
    volatile u_vout_v1_hcus_coef                  vout_v1_hcus_coef             ;/*0x4208*/
    volatile xmedia_u32                           reserved77[29]                ;/*0x420c*/
    volatile u_vout_v1_mrg0_csc_offset_in         vout_v1_mrg0_csc_offset_in    ;/*0x4280*/
    volatile u_vout_v1_mrg0_csc_matrix_01         vout_v1_mrg0_csc_matrix_01    ;/*0x4284*/
    volatile u_vout_v1_mrg0_csc_matrix_23         vout_v1_mrg0_csc_matrix_23    ;/*0x4288*/
    volatile u_vout_v1_mrg0_csc_matrix_45         vout_v1_mrg0_csc_matrix_45    ;/*0x428c*/
    volatile u_vout_v1_mrg0_csc_matrix_67         vout_v1_mrg0_csc_matrix_67    ;/*0x4290*/
    volatile u_vout_v1_mrg0_csc_matrix_8          vout_v1_mrg0_csc_matrix_8     ;/*0x4294*/
    volatile xmedia_u32                           reserved78                    ;/*0x4298*/
    volatile u_vout_v1_mrg0_csc_offset_out        vout_v1_mrg0_csc_offset_out   ;/*0x429c*/
    volatile u_vout_v1_mrg1_csc_offset_in         vout_v1_mrg1_csc_offset_in    ;/*0x42a0*/
    volatile u_vout_v1_mrg1_csc_matrix_01         vout_v1_mrg1_csc_matrix_01    ;/*0x42a4*/
    volatile u_vout_v1_mrg1_csc_matrix_23         vout_v1_mrg1_csc_matrix_23    ;/*0x42a8*/
    volatile u_vout_v1_mrg1_csc_matrix_45         vout_v1_mrg1_csc_matrix_45    ;/*0x42ac*/
    volatile u_vout_v1_mrg1_csc_matrix_67         vout_v1_mrg1_csc_matrix_67    ;/*0x42b0*/
    volatile u_vout_v1_mrg1_csc_matrix_8          vout_v1_mrg1_csc_matrix_8     ;/*0x42b4*/
    volatile xmedia_u32                           reserved79                    ;/*0x42b8*/
    volatile u_vout_v1_mrg1_csc_offset_out        vout_v1_mrg1_csc_offset_out   ;/*0x42bc*/
    volatile u_vout_v1_mrg2_csc_offset_in         vout_v1_mrg2_csc_offset_in    ;/*0x42c0*/
    volatile u_vout_v1_mrg2_csc_matrix_01         vout_v1_mrg2_csc_matrix_01    ;/*0x42c4*/
    volatile u_vout_v1_mrg2_csc_matrix_23         vout_v1_mrg2_csc_matrix_23    ;/*0x42c8*/
    volatile u_vout_v1_mrg2_csc_matrix_45         vout_v1_mrg2_csc_matrix_45    ;/*0x42cc*/
    volatile u_vout_v1_mrg2_csc_matrix_67         vout_v1_mrg2_csc_matrix_67    ;/*0x42d0*/
    volatile u_vout_v1_mrg2_csc_matrix_8          vout_v1_mrg2_csc_matrix_8     ;/*0x42d4*/
    volatile xmedia_u32                           reserved80                    ;/*0x42d8*/
    volatile u_vout_v1_mrg2_csc_offset_out        vout_v1_mrg2_csc_offset_out   ;/*0x42dc*/
    volatile u_vout_v1_mrg3_csc_offset_in         vout_v1_mrg3_csc_offset_in    ;/*0x42e0*/
    volatile u_vout_v1_mrg3_csc_matrix_01         vout_v1_mrg3_csc_matrix_01    ;/*0x42e4*/
    volatile u_vout_v1_mrg3_csc_matrix_23         vout_v1_mrg3_csc_matrix_23    ;/*0x42e8*/
    volatile u_vout_v1_mrg3_csc_matrix_45         vout_v1_mrg3_csc_matrix_45    ;/*0x42ec*/
    volatile u_vout_v1_mrg3_csc_matrix_67         vout_v1_mrg3_csc_matrix_67    ;/*0x42f0*/
    volatile u_vout_v1_mrg3_csc_matrix_8          vout_v1_mrg3_csc_matrix_8     ;/*0x42f4*/
    volatile xmedia_u32                           reserved81                    ;/*0x42f8*/
    volatile u_vout_v1_mrg3_csc_offset_out        vout_v1_mrg3_csc_offset_out   ;/*0x42fc*/
    volatile xmedia_u32                           reserved82[832]               ;/*0x4300*/
    volatile u_vout_g1_ctrl                       vout_g1_ctrl                  ;/*0x5000*/
    volatile u_vout_g1_upd                        vout_g1_upd                   ;/*0x5004*/
    volatile xmedia_u32                           reserved83[2]                 ;/*0x5008*/
    volatile u_vout_g1_bk                         vout_g1_bk                    ;/*0x5010*/
    volatile u_vout_g1_alpha_ext                  vout_g1_alpha_ext             ;/*0x5014*/
    volatile xmedia_u32                           reserved84[2]                 ;/*0x5018*/
    volatile u_vout_g1_mute_ctrl                  vout_g1_mute_ctrl             ;/*0x5020*/
    volatile u_vout_g1_mute_color                 vout_g1_mute_color            ;/*0x5024*/
    volatile xmedia_u32                           reserved85[2]                 ;/*0x5028*/
    volatile u_vout_g1_oreso                      vout_g1_oreso                 ;/*0x5030*/
    volatile xmedia_u32                           reserved86[3]                 ;/*0x5034*/
    volatile u_vout_g1_src_pos                    vout_g1_src_pos               ;/*0x5040*/
    volatile u_vout_g1_src                        vout_g1_src                   ;/*0x5044*/
    volatile u_vout_g1_disp_pos                   vout_g1_disp_pos              ;/*0x5048*/
    volatile u_vout_g1_disp                       vout_g1_disp                  ;/*0x504c*/
    volatile u_vout_g1_addr_h                     vout_g1_addr_h                ;/*0x5050*/
    volatile u_vout_g1_addr_l                     vout_g1_addr_l                ;/*0x5054*/
    volatile u_vout_g1_stride                     vout_g1_stride                ;/*0x5058*/
    volatile xmedia_u32                           reserved87[9]                 ;/*0x505c*/
    volatile u_vout_g1_clut_rd_ctrl               vout_g1_clut_rd_ctrl          ;/*0x5080*/
    volatile u_vout_g1_clut_rd_data               vout_g1_clut_rd_data          ;/*0x5084*/
    volatile xmedia_u32                           reserved88[30]                ;/*0x5088*/
    volatile u_vout_g1_key_ctrl                   vout_g1_key_ctrl              ;/*0x5100*/
    volatile u_vout_g1_key_max                    vout_g1_key_max               ;/*0x5104*/
    volatile u_vout_g1_key_min                    vout_g1_key_min               ;/*0x5108*/
    volatile u_vout_g1_key_msk                    vout_g1_key_msk               ;/*0x510c*/
    volatile xmedia_u32                           reserved89[12]                ;/*0x5110*/
    volatile u_vout_g1_csc_offset_in              vout_g1_csc_offset_in         ;/*0x5140*/
    volatile u_vout_g1_csc_matrix_01              vout_g1_csc_matrix_01         ;/*0x5144*/
    volatile u_vout_g1_csc_matrix_23              vout_g1_csc_matrix_23         ;/*0x5148*/
    volatile u_vout_g1_csc_matrix_45              vout_g1_csc_matrix_45         ;/*0x514c*/
    volatile u_vout_g1_csc_matrix_67              vout_g1_csc_matrix_67         ;/*0x5150*/
    volatile u_vout_g1_csc_matrix_8               vout_g1_csc_matrix_8          ;/*0x5154*/
    volatile xmedia_u32                           reserved90                    ;/*0x5158*/
    volatile u_vout_g1_csc_offset_out             vout_g1_csc_offset_out        ;/*0x515c*/
    volatile xmedia_u32                           reserved91[936]               ;/*0x5160*/
    volatile u_vout_intf1_ctrl                    vout_intf1_ctrl               ;/*0x6000*/
    volatile u_vout_intf1_upd                     vout_intf1_upd                ;/*0x6004*/
    volatile u_vout_intf1_regup_pos               vout_intf1_regup_pos          ;/*0x6008*/
    volatile u_vout_intf1_start_pos               vout_intf1_start_pos          ;/*0x600c*/
    volatile u_vout_intf1_act                     vout_intf1_act                ;/*0x6010*/
    volatile u_vout_intf1_vfb                     vout_intf1_vfb                ;/*0x6014*/
    volatile u_vout_intf1_bvfb                    vout_intf1_bvfb               ;/*0x6018*/
    volatile u_vout_intf1_hfb                     vout_intf1_hfb                ;/*0x601c*/
    volatile u_vout_intf1_hs_w                    vout_intf1_hs_w               ;/*0x6020*/
    volatile u_vout_intf1_field                   vout_intf1_field              ;/*0x6024*/
    volatile u_vout_intf1_vs_h                    vout_intf1_vs_h               ;/*0x6028*/
    volatile u_vout_intf1_vs_v                    vout_intf1_vs_v               ;/*0x602c*/
    volatile u_vout_intf1_bvs_h                   vout_intf1_bvs_h              ;/*0x6030*/
    volatile u_vout_intf1_bvs_v                   vout_intf1_bvs_v              ;/*0x6034*/
    volatile u_vout_intf1_sfifo_thd               vout_intf1_sfifo_thd          ;/*0x6038*/
    volatile xmedia_u32                           reserved92[17]                ;/*0x603c*/
    volatile u_vout_intf1_vt_int                  vout_intf1_vt_int             ;/*0x6080*/
    volatile u_vout_intf1_vt_intf1_thd0           vout_intf1_vt_intf1_thd0      ;/*0x6084*/
    volatile u_vout_intf1_vt_intf1_thd2           vout_intf1_vt_intf1_thd2      ;/*0x6088*/
    volatile xmedia_u32                           reserved93                    ;/*0x608c*/
    volatile u_vout_intf1_sta                     vout_intf1_sta                ;/*0x6090*/
    volatile u_vout_intf1_sta2                    vout_intf1_sta2               ;/*0x6094*/
    volatile u_vout_intf1_uf_line                 vout_intf1_uf_line            ;/*0x6098*/
    volatile u_vout_intf1_y_sum                   vout_intf1_y_sum              ;/*0x609c*/
    volatile u_vout_intf1_u_sum                   vout_intf1_u_sum              ;/*0x60a0*/
    volatile u_vout_intf1_v_sum                   vout_intf1_v_sum              ;/*0x60a4*/
    volatile xmedia_u32                           reserved94[22]                ;/*0x60a8*/
    volatile u_vout_intf1_bt_ctrl                 vout_intf1_bt_ctrl            ;/*0x6100*/
    volatile u_vout_intf1_bt_vbit                 vout_intf1_bt_vbit            ;/*0x6104*/
    volatile u_vout_intf1_bt_bvbit                vout_intf1_bt_bvbit           ;/*0x6108*/
    volatile xmedia_u32                           reserved95[5]                 ;/*0x610c*/
    volatile u_vout_intf1_lcd_ctrl                vout_intf1_lcd_ctrl           ;/*0x6120*/
    volatile xmedia_u32                           reserved96[7]                 ;/*0x6124*/
    volatile u_vout_intf1_lvds_ctrl               vout_intf1_lvds_ctrl          ;/*0x6140*/
    volatile xmedia_u32                           reserved97[15]                ;/*0x6144*/
    volatile u_vout_intf1_mipi_ctrl               vout_intf1_mipi_ctrl          ;/*0x6180*/
    volatile xmedia_u32                           reserved98[31]                ;/*0x6184*/
    volatile u_vout_intf1_mute_ctrl               vout_intf1_mute_ctrl          ;/*0x6200*/
    volatile u_vout_intf1_mute_color              vout_intf1_mute_color         ;/*0x6204*/
    volatile u_vout_intf1_mute_bk                 vout_intf1_mute_bk            ;/*0x6208*/
    volatile xmedia_u32                           reserved99                    ;/*0x620c*/
    volatile u_vout_intf1_ink_ctrl                vout_intf1_ink_ctrl           ;/*0x6210*/
    volatile u_vout_intf1_ink_pos                 vout_intf1_ink_pos            ;/*0x6214*/
    volatile u_vout_intf1_ink_color               vout_intf1_ink_color          ;/*0x6218*/
    volatile u_vout_intf1_ink_data                vout_intf1_ink_data           ;/*0x621c*/
    volatile u_vout_intf1_clip_l                  vout_intf1_clip_l             ;/*0x6220*/
    volatile u_vout_intf1_clip_h                  vout_intf1_clip_h             ;/*0x6224*/
    volatile xmedia_u32                           reserved100[54]               ;/*0x6228*/
    volatile u_vout_intf1_csc_offset_in           vout_intf1_csc_offset_in      ;/*0x6300*/
    volatile u_vout_intf1_csc_matrix_01           vout_intf1_csc_matrix_01      ;/*0x6304*/
    volatile u_vout_intf1_csc_matrix_23           vout_intf1_csc_matrix_23      ;/*0x6308*/
    volatile u_vout_intf1_csc_matrix_45           vout_intf1_csc_matrix_45      ;/*0x630c*/
    volatile u_vout_intf1_csc_matrix_67           vout_intf1_csc_matrix_67      ;/*0x6310*/
    volatile u_vout_intf1_csc_matrix_8            vout_intf1_csc_matrix_8       ;/*0x6314*/
    volatile xmedia_u32                           reserved101                   ;/*0x6318*/
    volatile u_vout_intf1_csc_offset_out          vout_intf1_csc_offset_out     ;/*0x631c*/
    volatile xmedia_u32                           reserved102[8]                ;/*0x6320*/
    volatile u_vout_intf1_hcds_ctrl               vout_intf1_hcds_ctrl          ;/*0x6340*/
    volatile u_vout_intf1_hcds_coef               vout_intf1_hcds_coef          ;/*0x6344*/
    volatile xmedia_u32                           reserved103[6]                ;/*0x6348*/
    volatile u_vout_intf1_dither_ctrl             vout_intf1_dither_ctrl        ;/*0x6360*/
}s_vout_regs_type;

extern s_vout_regs_type *g_p_vout_reg;

xmedia_u8 hal_vout_get_cfg_lvds_pll_ssc_freq(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_lvds_pll_ssc_freq(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_cfg_lvds_pll_ssc_spread(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_lvds_pll_ssc_spread(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_cfg_lvds_pll_cpi2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_lvds_pll_cpi2(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_lvds_rterm_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_lvds_rterm_sel(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_cfg_lvds_rterm_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_lvds_rterm_en(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_void hal_vout_set_cfg_lvds_iref(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_lvds_iref(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_vout_get_cfg_lvds_vos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_lvds_vos(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_void hal_vout_set_cfg_lvds_phy_pn_inv(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_lvds_phy_pn_inv(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_lvds_phy_rdy(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_vout_get_lvds_phy_bist_ok(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_cfg_lvds_pd_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_lvds_pd_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_cfg_lvds_lane_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_lvds_lane_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);

xmedia_u32 hal_vout_reg_read(volatile xmedia_uintptr_t addr);
xmedia_void hal_vout_reg_write(volatile xmedia_uintptr_t addr, xmedia_u32 val);
xmedia_u8 hal_vout_get_cfg_testpin_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_testpin_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_cfg_regup_debug(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_regup_debug(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_ck_dyn_gt_en_dma(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_ck_dyn_gt_en_dma(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_ck_dyn_gt_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_ck_dyn_gt_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_vout_set_cfg_regup_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_intf1_regup_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_g1_regup_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_v1_regup_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_intf_regup_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_g0_regup_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_v0_regup_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_top_regup_sta(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_vout_get_vout_ver_0(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_vout_get_vout_ver_1(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_vout_get_cfg_mem_ctrl(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mem_ctrl(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_bool hal_vout_get_axi_bus_werr_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_axi_bus_rerr_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_low_delay_y_err_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_low_delay_c_err_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_intf1_underflow_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_intf_underflow_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_intf1_vt_int_3_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_intf1_vt_int_2_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_intf1_vt_int_1_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_intf1_vt_int_0_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_intf_vt_int_3_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_intf_vt_int_2_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_intf_vt_int_1_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_intf_vt_int_0_sta(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_axi_bus_werr_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_vout_set_cfg_axi_bus_rerr_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_vout_set_cfg_low_delay_y_err_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_vout_set_cfg_low_delay_c_err_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_vout_set_cfg_intf1_underflow_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_vout_set_cfg_intf_underflow_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_vout_set_cfg_intf1_vt_int_3_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_vout_set_cfg_intf1_vt_int_2_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_vout_set_cfg_intf1_vt_int_1_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_vout_set_cfg_intf1_vt_int_0_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_vout_set_cfg_intf_vt_int_3_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_vout_set_cfg_intf_vt_int_2_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_vout_set_cfg_intf_vt_int_1_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_vout_set_cfg_intf_vt_int_0_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_axi_bus_werr_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_axi_bus_werr_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_axi_bus_rerr_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_axi_bus_rerr_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_low_delay_y_err_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_low_delay_y_err_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_low_delay_c_err_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_low_delay_c_err_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_intf1_underflow_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_intf1_underflow_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_intf_underflow_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_intf_underflow_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_intf1_vt_int_3_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_intf1_vt_int_3_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_intf1_vt_int_2_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_intf1_vt_int_2_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_intf1_vt_int_1_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_intf1_vt_int_1_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_intf1_vt_int_0_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_intf1_vt_int_0_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_intf_vt_int_3_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_intf_vt_int_3_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_intf_vt_int_2_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_intf_vt_int_2_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_intf_vt_int_1_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_intf_vt_int_1_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_intf_vt_int_0_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_intf_vt_int_0_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_axi_bus_werr_src_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_axi_bus_rerr_src_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_low_delay_y_err_src_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_low_delay_c_err_src_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_intf1_underflow_src_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_intf_underflow_src_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_intf1_vt_int_3_src_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_intf1_vt_int_2_src_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_intf1_vt_int_1_src_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_intf1_vt_int_0_src_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_intf_vt_int_3_src_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_intf_vt_int_2_src_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_intf_vt_int_1_src_sta(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_intf_vt_int_0_src_sta(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_uf_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_rdma_busy_g1(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_rdma1_busy_v1_c(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_rdma1_busy_v1_y(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_rdma_busy_v1_c(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_rdma_busy_v1_y(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_rdma_busy_g0(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_rdma1_busy_v0_c(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_rdma1_busy_v0_y(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_rdma_busy_v0_c(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_rdma_busy_v0_y(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_vout_get_cfg_rd_bw_window_num_pow(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_rd_bw_window_num_pow(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_rd_bw_ch_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_rd_bw_ch_sel(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_rd_latency_ch_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_rd_latency_ch_sel(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_cfg_rd_bw_enable(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_rd_bw_enable(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_rd_latency_enable(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_rd_latency_enable(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_axi_sta_enable(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_axi_sta_enable(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u32 hal_vout_get_dfx_axi_rd_burst_num(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_vout_get_dfx_axi_rd_ost_max(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_vout_get_dfx_axi_rd_ost_sum_h(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_vout_get_dfx_axi_rd_ost_sum_l(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_vout_get_dfx_rd_latency_min(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_vout_get_dfx_rd_latency_max(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_vout_get_dfx_rd_latency_sum_h(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_vout_get_dfx_rd_latency_sum_l(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_vout_get_dfx_rd_latency_burst_num(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_vout_get_dfx_rd_bw_trans_max(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_vout_get_dfx_rd_bw_trans_min(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_vout_get_dfx_rd_bw_trans_avg(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_vout_get_dfx_rd_bw_trans_window_cycle_avg(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_vout_get_cfg_mixer_prio1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mixer_prio1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_mixer_prio0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mixer_prio0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u32 hal_vout_get_cfg_bk_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_bk_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u8 hal_vout_get_cfg_mixer1_prio1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mixer1_prio1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_mixer1_prio0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mixer1_prio0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u32 hal_vout_get_cfg_bk1_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_bk1_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u8 hal_vout_get_cfg_rd_weight_g0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_rd_weight_g0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_rd_weight_v0_c_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_rd_weight_v0_c_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_rd_weight_v0_y_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_rd_weight_v0_y_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_rd_weight_v0_c(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_rd_weight_v0_c(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_rd_weight_v0_y(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_rd_weight_v0_y(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_cfg_continuous_req_en_r(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_continuous_req_en_r(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_cross_256_proc_en_r(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_cross_256_proc_en_r(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_cfg_axi_rd_ost_num(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_axi_rd_ost_num(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_void hal_vout_set_cfg_rd_ost_num_v0_y(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_void hal_vout_set_cfg_rd_ost_num_v0_c(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_void hal_vout_set_cfg_rd_ost_num_v0_y_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_void hal_vout_set_cfg_rd_ost_num_v0_c_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_void hal_vout_set_cfg_rd_ost_num_g0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_void hal_vout_set_cfg_rd_ost_num_v1_y(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_void hal_vout_set_cfg_rd_ost_num_v1_c(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_void hal_vout_set_cfg_rd_ost_num_g1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_void hal_vout_set_cfg_rd_ost_num_v1_c_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_void hal_vout_set_cfg_rd_ost_num_v1_y_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_rd_weight_g1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_rd_weight_g1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_rd_weight_v1_c_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_rd_weight_v1_c_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_rd_weight_v1_y_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_rd_weight_v1_y_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_rd_weight_v1_c(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_rd_weight_v1_c(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_rd_weight_v1_y(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_rd_weight_v1_y(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_void hal_vout_set_cfg_para_3_update(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_vout_set_cfg_para_2_update(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_vout_set_cfg_para_1_update(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_vout_set_cfg_para_0_update(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u32 hal_vout_get_cfg_para_0_addr_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_para_0_addr_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_para_0_addr_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_para_0_addr_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_para_1_addr_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_para_1_addr_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_para_1_addr_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_para_1_addr_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_para_2_addr_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_para_2_addr_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_para_2_addr_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_para_2_addr_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_para_3_addr_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_para_3_addr_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_para_3_addr_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_para_3_addr_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_void hal_vout_set_cfg_lvds_phy_update(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_cfg_lvds_pll_frange(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_lvds_pll_frange(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_cfg_lvds_pll_ssc_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_lvds_pll_ssc_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_lvds_pll_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_lvds_pll_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_cfg_lvds_pll_option(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_lvds_pll_option(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_lvds_phy_voc_sw(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_lvds_phy_voc_sw(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_lvds_phy_ldo(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_lvds_phy_ldo(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_cfg_lvds_phy_rf(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_lvds_phy_rf(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_lvds_phy_pll_lock(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_vout_get_lvds_phy_bist_ok(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_vout_get_cfg_galpha(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_galpha(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_cfg_galpha_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_galpha_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_blend_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_blend_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_mrg_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_src_uv_order(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_src_uv_order(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_cfg_src_fmt(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_src_fmt(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_cfg_regup_field(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_regup_field(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_regup_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_regup_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_layer_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_layer_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_vout_set_v0upd_cfg_regup_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u32 hal_vout_get_v0bk_cfg_bk_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_v0bk_cfg_bk_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u8 hal_vout_get_cfg_checker_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_checker_v(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_checker_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_checker_h(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_cfg_mute_pat(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mute_pat(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_mute_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mute_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_mute_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mute_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u32 hal_vout_get_cfg_mute_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mute_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u16 hal_vout_get_out_v(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_vout_get_out_h(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_vout_get_cfg_src_ypos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_src_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_src_xpos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_src_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_src_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_src_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_src_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_src_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_disp_ypos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_disp_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_disp_xpos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_disp_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_disp_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_disp_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_disp_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_disp_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u32 hal_vout_get_cfg_src_addr_y_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_src_addr_y_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_src_addr_y_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_src_addr_y_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_src_stride_y(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_src_stride_y(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_src_addr_c_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_src_addr_c_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_src_addr_c_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_src_addr_c_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_src_stride_c(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_src_stride_c(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_bool hal_vout_get_cfg_mixer_prio(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mixer_prio(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_cfg_mrg_link_rdma1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg_link_rdma1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_cfg_rdma1_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_rdma1_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_mrg3_mute_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_mute_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_mrg2_mute_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_mute_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_mrg1_mute_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_mute_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_mrg0_mute_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_mute_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_mrg3_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_mrg2_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_mrg1_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_mrg0_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u16 hal_vout_get_cfg_mixer_sort(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mixer_sort(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u32 hal_vout_get_cfg_mrg0_src_addr_y_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_src_addr_y_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_mrg0_src_addr_y_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_src_addr_y_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_mrg0_src_stride_y(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_src_stride_y(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_mrg0_src_addr_c_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_src_addr_c_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_mrg0_src_addr_c_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_src_addr_c_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_mrg0_src_stride_c(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_src_stride_c(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u16 hal_vout_get_cfg_mrg0_src_ypos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_src_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg0_src_xpos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_src_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg0_src_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_src_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg0_src_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_src_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u32 hal_vout_get_cfg_mrg0_mute_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_mute_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_mrg1_src_addr_y_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_src_addr_y_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_mrg1_src_addr_y_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_src_addr_y_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_mrg1_src_stride_y(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_src_stride_y(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_mrg1_src_addr_c_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_src_addr_c_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_mrg1_src_addr_c_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_src_addr_c_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_mrg1_src_stride_c(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_src_stride_c(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u16 hal_vout_get_cfg_mrg1_src_ypos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_src_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg1_src_xpos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_src_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg1_src_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_src_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg1_src_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_src_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u32 hal_vout_get_cfg_mrg1_mute_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_mute_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_mrg2_src_addr_y_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_src_addr_y_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_mrg2_src_addr_y_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_src_addr_y_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_mrg2_src_stride_y(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_src_stride_y(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_mrg2_src_addr_c_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_src_addr_c_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_mrg2_src_addr_c_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_src_addr_c_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_mrg2_src_stride_c(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_src_stride_c(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u16 hal_vout_get_cfg_mrg2_src_ypos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_src_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg2_src_xpos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_src_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg2_src_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_src_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg2_src_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_src_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u32 hal_vout_get_cfg_mrg2_mute_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_mute_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_mrg3_src_addr_y_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_src_addr_y_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_mrg3_src_addr_y_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_src_addr_y_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_mrg3_src_stride_y(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_src_stride_y(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_mrg3_src_addr_c_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_src_addr_c_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_mrg3_src_addr_c_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_src_addr_c_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_mrg3_src_stride_c(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_src_stride_c(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u16 hal_vout_get_cfg_mrg3_src_ypos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_src_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg3_src_xpos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_src_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg3_src_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_src_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg3_src_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_src_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u32 hal_vout_get_cfg_mrg3_mute_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_mute_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_bool hal_vout_get_cfg_hcus_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_hcus_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_vcus_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vcus_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_cfg_vcus_coef_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vcus_coef_3(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_vcus_coef_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vcus_coef_2(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_vcus_coef_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vcus_coef_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_vcus_coef_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vcus_coef_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_hcus_coef_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_hcus_coef_3(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_hcus_coef_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_hcus_coef_2(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_hcus_coef_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_hcus_coef_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_hcus_coef_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_hcus_coef_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_cfg_csc_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_csc_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_cfg_mrg0_csc_offset_in_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_csc_offset_in_2(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_mrg0_csc_offset_in_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_csc_offset_in_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_mrg0_csc_offset_in_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_csc_offset_in_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u16 hal_vout_get_cfg_mrg0_csc_matrix_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_csc_matrix_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg0_csc_matrix_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_csc_matrix_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg0_csc_matrix_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_csc_matrix_3(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg0_csc_matrix_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_csc_matrix_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg0_csc_matrix_5(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_csc_matrix_5(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg0_csc_matrix_4(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_csc_matrix_4(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg0_csc_matrix_7(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_csc_matrix_7(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg0_csc_matrix_6(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_csc_matrix_6(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg0_csc_matrix_8(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_csc_matrix_8(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg0_csc_offset_out_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_csc_offset_out_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg0_csc_offset_out_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_csc_offset_out_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg0_csc_offset_out_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg0_csc_offset_out_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u8 hal_vout_get_cfg_mrg1_csc_offset_in_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_csc_offset_in_2(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_mrg1_csc_offset_in_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_csc_offset_in_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_mrg1_csc_offset_in_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_csc_offset_in_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u16 hal_vout_get_cfg_mrg1_csc_matrix_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_csc_matrix_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg1_csc_matrix_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_csc_matrix_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg1_csc_matrix_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_csc_matrix_3(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg1_csc_matrix_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_csc_matrix_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg1_csc_matrix_5(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_csc_matrix_5(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg1_csc_matrix_4(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_csc_matrix_4(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg1_csc_matrix_7(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_csc_matrix_7(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg1_csc_matrix_6(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_csc_matrix_6(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg1_csc_matrix_8(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_csc_matrix_8(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg1_csc_offset_out_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_csc_offset_out_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg1_csc_offset_out_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_csc_offset_out_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg1_csc_offset_out_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg1_csc_offset_out_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u8 hal_vout_get_cfg_mrg2_csc_offset_in_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_csc_offset_in_2(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_mrg2_csc_offset_in_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_csc_offset_in_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_mrg2_csc_offset_in_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_csc_offset_in_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u16 hal_vout_get_cfg_mrg2_csc_matrix_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_csc_matrix_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg2_csc_matrix_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_csc_matrix_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg2_csc_matrix_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_csc_matrix_3(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg2_csc_matrix_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_csc_matrix_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg2_csc_matrix_5(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_csc_matrix_5(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg2_csc_matrix_4(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_csc_matrix_4(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg2_csc_matrix_7(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_csc_matrix_7(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg2_csc_matrix_6(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_csc_matrix_6(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg2_csc_matrix_8(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_csc_matrix_8(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg2_csc_offset_out_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_csc_offset_out_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg2_csc_offset_out_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_csc_offset_out_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg2_csc_offset_out_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg2_csc_offset_out_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u8 hal_vout_get_cfg_mrg3_csc_offset_in_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_csc_offset_in_2(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_mrg3_csc_offset_in_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_csc_offset_in_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_mrg3_csc_offset_in_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_csc_offset_in_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u16 hal_vout_get_cfg_mrg3_csc_matrix_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_csc_matrix_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg3_csc_matrix_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_csc_matrix_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg3_csc_matrix_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_csc_matrix_3(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg3_csc_matrix_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_csc_matrix_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg3_csc_matrix_5(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_csc_matrix_5(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg3_csc_matrix_4(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_csc_matrix_4(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg3_csc_matrix_7(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_csc_matrix_7(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg3_csc_matrix_6(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_csc_matrix_6(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg3_csc_matrix_8(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_csc_matrix_8(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg3_csc_offset_out_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_csc_offset_out_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg3_csc_offset_out_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_csc_offset_out_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_mrg3_csc_offset_out_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_mrg3_csc_offset_out_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u8 hal_vout_get_voutg_cfg_galpha(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg_cfg_galpha(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_cfg_ppremul_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_ppremul_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_gpremul_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_gpremul_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_palpha_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_palpha_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_voutg_cfg_galpha_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg_cfg_galpha_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_voutg_cfg_blend_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg_cfg_blend_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_cfg_bitext(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_bitext(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_cfg_a_order(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_a_order(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_rb_order(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_rb_order(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_voutg_cfg_src_fmt(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg_cfg_src_fmt(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_voutg_cfg_regup_field(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg_cfg_regup_field(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_voutg_cfg_regup_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg_cfg_regup_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_voutg_cfg_layer_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg_cfg_layer_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_vout_set_g0upd_cfg_regup_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u32 hal_vout_get_g0bk_cfg_bk_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_g0bk_cfg_bk_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u8 hal_vout_get_cfg_alpha_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_alpha_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_alpha_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_alpha_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutg_cfg_checker_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg_cfg_checker_v(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutg_cfg_checker_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg_cfg_checker_h(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_voutg_cfg_mute_pat(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg_cfg_mute_pat(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_voutg_cfg_mute_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg_cfg_mute_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_voutg_cfg_mute_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg_cfg_mute_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u32 hal_vout_get_voutg_cfg_mute_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg_cfg_mute_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u16 hal_vout_get_voutg_out_v(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_vout_get_voutg_out_h(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_vout_get_voutg_cfg_src_ypos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg_cfg_src_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutg_cfg_src_xpos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg_cfg_src_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_g0src_cfg_src_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_g0src_cfg_src_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_g0src_cfg_src_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_g0src_cfg_src_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutg_cfg_disp_ypos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg_cfg_disp_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutg_cfg_disp_xpos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg_cfg_disp_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutg_cfg_disp_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg_cfg_disp_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutg_cfg_disp_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg_cfg_disp_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u32 hal_vout_get_cfg_src_addr_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_src_addr_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_src_addr_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_src_addr_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_src_stride(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_src_stride(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_void hal_vout_set_cfg_clut_rd_addr(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_void hal_vout_set_cfg_clut_rd_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u32 hal_vout_get_clut_rd_data(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_vout_get_cfg_key_v_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_key_v_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_key_u_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_key_u_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_key_y_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_key_y_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_key_a_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_key_a_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_cfg_key_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_key_sel(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_key_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_key_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_cfg_key_v_max(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_key_v_max(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_key_u_max(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_key_u_max(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_key_y_max(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_key_y_max(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_key_a_max(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_key_a_max(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_key_v_min(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_key_v_min(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_key_u_min(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_key_u_min(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_key_y_min(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_key_y_min(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_key_a_min(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_key_a_min(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_key_v_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_key_v_msk(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_key_u_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_key_u_msk(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_key_y_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_key_y_msk(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_key_a_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_key_a_msk(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_vout_cfg_csc_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_vout_cfg_csc_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_cfg_csc_offset_in_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_csc_offset_in_2(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_csc_offset_in_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_csc_offset_in_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_csc_offset_in_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_csc_offset_in_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u16 hal_vout_get_cfg_csc_matrix_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_csc_matrix_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_csc_matrix_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_csc_matrix_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_csc_matrix_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_csc_matrix_3(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_csc_matrix_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_csc_matrix_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_csc_matrix_5(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_csc_matrix_5(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_csc_matrix_4(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_csc_matrix_4(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_csc_matrix_7(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_csc_matrix_7(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_csc_matrix_6(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_csc_matrix_6(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_csc_matrix_8(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_csc_matrix_8(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_csc_offset_out_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_csc_offset_out_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_csc_offset_out_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_csc_offset_out_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_csc_offset_out_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_csc_offset_out_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_bool hal_vout_get_cfg_chk_sum_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_chk_sum_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_cfg_hs_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_hs_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_cfg_de_inv(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_de_inv(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_hs_inv(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_hs_inv(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_vs_inv(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vs_inv(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_field_inv(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_field_inv(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_clock_edge_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_clock_edge_sel(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_interlaced(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_interlaced(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_cfg_intf_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_intf_sel(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_cfg_intf_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_intf_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_vout_set_intfupd_cfg_regup_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u16 hal_vout_get_cfg_para_pos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_para_pos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_regup_pos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_regup_pos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_start_pos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_start_pos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_prefetch_pos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_prefetch_pos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_hact(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_hact(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_vact(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vact(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_vbb(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vbb(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_vfb(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vfb(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_bvbb(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_bvbb(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_bvfb(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_bvfb(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_hbb(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_hbb(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_hfb(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_hfb(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_hs_width(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_hs_width(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_bfield_tog(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_bfield_tog(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_field_tog(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_field_tog(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_vs_end_h_field0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vs_end_h_field0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_vs_st_h_field0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vs_st_h_field0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_vs_end_v_field0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vs_end_v_field0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_vs_st_v_field0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vs_st_v_field0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_vs_end_h_field1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vs_end_h_field1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_vs_st_h_field1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vs_st_h_field1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_vs_end_v_field1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vs_end_v_field1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_vs_st_v_field1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vs_st_v_field1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_intf_sfifo_thd(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_intf_sfifo_thd(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_bool hal_vout_get_cfg_vt_int_cnt_sel_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vt_int_cnt_sel_3(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_vt_int_cnt_sel_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vt_int_cnt_sel_2(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_vt_int_cnt_sel_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vt_int_cnt_sel_1(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_vt_int_cnt_sel_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vt_int_cnt_sel_0(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_vt_int_mode_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vt_int_mode_3(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_vt_int_mode_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vt_int_mode_2(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_vt_int_mode_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vt_int_mode_1(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_vt_int_mode_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vt_int_mode_0(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u16 hal_vout_get_cfg_vt_int_thd_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vt_int_thd_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_vt_int_thd_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vt_int_thd_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_vt_int_thd_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vt_int_thd_3(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_vt_int_thd_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vt_int_thd_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_frm_cnt(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_field_flag_ppc(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_vout_get_vstate(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_vout_get_vcnt_st_vact(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_vout_get_vcnt_st_vblk(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_vout_get_intf_uf_line(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_vout_get_intf_y_sum(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_vout_get_intf_u_sum(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_vout_get_intf_v_sum(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_cfg_bt_bit_inv(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_bt_bit_inv(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_vbit_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vbit_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_bt_uv_order(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_bt_uv_order(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_bt_yc_order(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_bt_yc_order(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_bt_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_bt_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u16 hal_vout_get_cfg_vbit_end(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vbit_end(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_vbit_st(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_vbit_st(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_bvbit_end(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_bvbit_end(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_bvbit_st(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_bvbit_st(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_bool hal_vout_get_cfg_lcd_bit_inv(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_lcd_bit_inv(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_cfg_lcd_rgb_order_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_lcd_rgb_order_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_lcd_rgb_order_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_lcd_rgb_order_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_lcd_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_lcd_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_lvds_lane_4_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_lvds_lane_4_sel(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_lvds_lane_3_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_lvds_lane_3_sel(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_lvds_lane_2_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_lvds_lane_2_sel(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_lvds_lane_1_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_lvds_lane_1_sel(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_lvds_lane_0_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_lvds_lane_0_sel(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_lvds_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_lvds_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_cfg_dpi_uv_order(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_dpi_uv_order(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_cfg_dpi_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_dpi_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutintf_cfg_checker_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf_cfg_checker_v(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutintf_cfg_checker_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf_cfg_checker_h(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_cfg_cbar_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_cbar_sel(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_voutintf_cfg_mute_pat(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf_cfg_mute_pat(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_voutintf_cfg_mute_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf_cfg_mute_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_voutintf_cfg_mute_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf_cfg_mute_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u32 hal_vout_get_voutintf_cfg_mute_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf_cfg_mute_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_intfmutebk_cfg_bk_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_intfmutebk_cfg_bk_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_bool hal_vout_get_cfg_cross_color_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_cross_color_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_cross_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_cross_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_ink_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_ink_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u16 hal_vout_get_cfg_y_pos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_y_pos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_cfg_x_pos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_x_pos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u32 hal_vout_get_cfg_cross_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_cross_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_ink_data(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_cfg_clip_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_clip_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u32 hal_vout_get_cfg_clip_yuv_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_clip_yuv_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_cfg_clip_yuv_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_clip_yuv_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_bool hal_vout_get_voutintf_cfg_csc_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf_cfg_csc_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_voutintf_cfg_csc_offset_in_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf_cfg_csc_offset_in_2(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutintf_cfg_csc_offset_in_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf_cfg_csc_offset_in_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutintf_cfg_csc_offset_in_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf_cfg_csc_offset_in_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u16 hal_vout_get_voutintf_cfg_csc_matrix_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf_cfg_csc_matrix_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf_cfg_csc_matrix_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf_cfg_csc_matrix_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf_cfg_csc_matrix_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf_cfg_csc_matrix_3(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf_cfg_csc_matrix_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf_cfg_csc_matrix_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf_cfg_csc_matrix_5(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf_cfg_csc_matrix_5(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf_cfg_csc_matrix_4(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf_cfg_csc_matrix_4(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf_cfg_csc_matrix_7(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf_cfg_csc_matrix_7(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf_cfg_csc_matrix_6(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf_cfg_csc_matrix_6(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf_cfg_csc_matrix_8(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf_cfg_csc_matrix_8(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf_cfg_csc_offset_out_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf_cfg_csc_offset_out_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf_cfg_csc_offset_out_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf_cfg_csc_offset_out_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf_cfg_csc_offset_out_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf_cfg_csc_offset_out_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_bool hal_vout_get_cfg_hcds_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_hcds_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_hcds_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_hcds_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_cfg_hcds_coef_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_hcds_coef_3(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_hcds_coef_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_hcds_coef_2(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_hcds_coef_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_hcds_coef_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_hcds_coef_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_hcds_coef_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_dither_thd_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_dither_thd_h(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_cfg_dither_thd_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_dither_thd_l(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_cfg_dither_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_dither_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_cfg_gamma_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_cfg_gamma_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_1ctrl_cfg_galpha(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1ctrl_cfg_galpha(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_1ctrl_cfg_galpha_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1ctrl_cfg_galpha_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1ctrl_cfg_blend_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1ctrl_cfg_blend_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1ctrl_cfg_mrg_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1ctrl_cfg_mrg_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1ctrl_cfg_src_uv_order(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1ctrl_cfg_src_uv_order(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_1ctrl_cfg_src_fmt(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1ctrl_cfg_src_fmt(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_1ctrl_cfg_regup_field(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1ctrl_cfg_regup_field(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1ctrl_cfg_regup_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1ctrl_cfg_regup_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1ctrl_cfg_layer_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1ctrl_cfg_layer_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_vout_set_v1upd_cfg_regup_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u32 hal_vout_get_v1bk_cfg_bk_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_v1bk_cfg_bk_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u8 hal_vout_get_voutv1_cfg_checker_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_checker_v(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutv1_cfg_checker_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_checker_h(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_voutv1_cfg_mute_pat(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mute_pat(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_voutv1_cfg_mute_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mute_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_voutv1_cfg_mute_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mute_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u32 hal_vout_get_voutv1_cfg_mute_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mute_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u16 hal_vout_get_1oreso_out_v(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_vout_get_1oreso_out_h(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_vout_get_voutv1_cfg_src_ypos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_src_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_src_xpos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_src_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1src_cfg_src_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1src_cfg_src_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1src_cfg_src_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1src_cfg_src_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_disp_ypos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_disp_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_disp_xpos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_disp_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1disp_cfg_disp_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1disp_cfg_disp_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1disp_cfg_disp_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1disp_cfg_disp_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u32 hal_vout_get_voutv1_cfg_src_addr_y_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_src_addr_y_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_src_addr_y_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_src_addr_y_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_src_stride_y(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_src_stride_y(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_src_addr_c_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_src_addr_c_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_src_addr_c_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_src_addr_c_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_src_stride_c(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_src_stride_c(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_bool hal_vout_get_voutv1_cfg_mixer_prio(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mixer_prio(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_voutv1_cfg_mrg_link_rdma1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg_link_rdma1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_voutv1_cfg_rdma1_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_rdma1_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_voutv1_cfg_mrg3_mute_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_mute_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_voutv1_cfg_mrg2_mute_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_mute_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_voutv1_cfg_mrg1_mute_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_mute_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_voutv1_cfg_mrg0_mute_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_mute_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_voutv1_cfg_mrg3_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_voutv1_cfg_mrg2_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_voutv1_cfg_mrg1_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_voutv1_cfg_mrg0_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u16 hal_vout_get_voutv1_cfg_mixer_sort(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mixer_sort(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg0_src_addr_y_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_src_addr_y_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg0_src_addr_y_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_src_addr_y_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg0_src_stride_y(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_src_stride_y(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg0_src_addr_c_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_src_addr_c_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg0_src_addr_c_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_src_addr_c_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg0_src_stride_c(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_src_stride_c(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg0_src_ypos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_src_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg0_src_xpos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_src_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg0_src_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_src_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg0_src_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_src_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg0_mute_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_mute_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg1_src_addr_y_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_src_addr_y_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg1_src_addr_y_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_src_addr_y_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg1_src_stride_y(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_src_stride_y(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg1_src_addr_c_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_src_addr_c_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg1_src_addr_c_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_src_addr_c_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg1_src_stride_c(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_src_stride_c(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg1_src_ypos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_src_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg1_src_xpos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_src_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg1_src_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_src_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg1_src_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_src_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg1_mute_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_mute_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg2_src_addr_y_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_src_addr_y_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg2_src_addr_y_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_src_addr_y_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg2_src_stride_y(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_src_stride_y(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg2_src_addr_c_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_src_addr_c_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg2_src_addr_c_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_src_addr_c_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg2_src_stride_c(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_src_stride_c(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg2_src_ypos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_src_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg2_src_xpos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_src_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg2_src_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_src_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg2_src_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_src_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg2_mute_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_mute_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg3_src_addr_y_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_src_addr_y_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg3_src_addr_y_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_src_addr_y_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg3_src_stride_y(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_src_stride_y(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg3_src_addr_c_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_src_addr_c_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg3_src_addr_c_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_src_addr_c_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg3_src_stride_c(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_src_stride_c(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg3_src_ypos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_src_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg3_src_xpos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_src_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg3_src_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_src_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg3_src_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_src_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u32 hal_vout_get_voutv1_cfg_mrg3_mute_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_mute_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_bool hal_vout_get_voutv1_cfg_hcus_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_hcus_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_voutv1_cfg_vcus_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_vcus_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_voutv1_cfg_vcus_coef_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_vcus_coef_3(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutv1_cfg_vcus_coef_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_vcus_coef_2(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutv1_cfg_vcus_coef_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_vcus_coef_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutv1_cfg_vcus_coef_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_vcus_coef_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutv1_cfg_hcus_coef_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_hcus_coef_3(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutv1_cfg_hcus_coef_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_hcus_coef_2(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutv1_cfg_hcus_coef_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_hcus_coef_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutv1_cfg_hcus_coef_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_hcus_coef_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_voutv1_cfg_csc_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_csc_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_voutv1_cfg_mrg0_csc_offset_in_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_csc_offset_in_2(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutv1_cfg_mrg0_csc_offset_in_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_csc_offset_in_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutv1_cfg_mrg0_csc_offset_in_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_csc_offset_in_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg0_csc_matrix_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_csc_matrix_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg0_csc_matrix_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_csc_matrix_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg0_csc_matrix_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_csc_matrix_3(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg0_csc_matrix_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_csc_matrix_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg0_csc_matrix_5(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_csc_matrix_5(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg0_csc_matrix_4(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_csc_matrix_4(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg0_csc_matrix_7(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_csc_matrix_7(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg0_csc_matrix_6(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_csc_matrix_6(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg0_csc_matrix_8(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_csc_matrix_8(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg0_csc_offset_out_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_csc_offset_out_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg0_csc_offset_out_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_csc_offset_out_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg0_csc_offset_out_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg0_csc_offset_out_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u8 hal_vout_get_voutv1_cfg_mrg1_csc_offset_in_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_csc_offset_in_2(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutv1_cfg_mrg1_csc_offset_in_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_csc_offset_in_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutv1_cfg_mrg1_csc_offset_in_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_csc_offset_in_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg1_csc_matrix_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_csc_matrix_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg1_csc_matrix_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_csc_matrix_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg1_csc_matrix_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_csc_matrix_3(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg1_csc_matrix_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_csc_matrix_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg1_csc_matrix_5(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_csc_matrix_5(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg1_csc_matrix_4(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_csc_matrix_4(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg1_csc_matrix_7(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_csc_matrix_7(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg1_csc_matrix_6(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_csc_matrix_6(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg1_csc_matrix_8(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_csc_matrix_8(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg1_csc_offset_out_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_csc_offset_out_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg1_csc_offset_out_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_csc_offset_out_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg1_csc_offset_out_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg1_csc_offset_out_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u8 hal_vout_get_voutv1_cfg_mrg2_csc_offset_in_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_csc_offset_in_2(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutv1_cfg_mrg2_csc_offset_in_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_csc_offset_in_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutv1_cfg_mrg2_csc_offset_in_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_csc_offset_in_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg2_csc_matrix_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_csc_matrix_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg2_csc_matrix_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_csc_matrix_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg2_csc_matrix_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_csc_matrix_3(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg2_csc_matrix_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_csc_matrix_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg2_csc_matrix_5(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_csc_matrix_5(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg2_csc_matrix_4(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_csc_matrix_4(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg2_csc_matrix_7(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_csc_matrix_7(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg2_csc_matrix_6(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_csc_matrix_6(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg2_csc_matrix_8(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_csc_matrix_8(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg2_csc_offset_out_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_csc_offset_out_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg2_csc_offset_out_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_csc_offset_out_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg2_csc_offset_out_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg2_csc_offset_out_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u8 hal_vout_get_voutv1_cfg_mrg3_csc_offset_in_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_csc_offset_in_2(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutv1_cfg_mrg3_csc_offset_in_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_csc_offset_in_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutv1_cfg_mrg3_csc_offset_in_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_csc_offset_in_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg3_csc_matrix_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_csc_matrix_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg3_csc_matrix_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_csc_matrix_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg3_csc_matrix_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_csc_matrix_3(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg3_csc_matrix_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_csc_matrix_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg3_csc_matrix_5(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_csc_matrix_5(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg3_csc_matrix_4(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_csc_matrix_4(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg3_csc_matrix_7(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_csc_matrix_7(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg3_csc_matrix_6(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_csc_matrix_6(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg3_csc_matrix_8(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_csc_matrix_8(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg3_csc_offset_out_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_csc_offset_out_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg3_csc_offset_out_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_csc_offset_out_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutv1_cfg_mrg3_csc_offset_out_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutv1_cfg_mrg3_csc_offset_out_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u8 hal_vout_get_g1ctrl_cfg_galpha(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_g1ctrl_cfg_galpha(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_1ctrl_cfg_ppremul_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1ctrl_cfg_ppremul_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1ctrl_cfg_gpremul_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1ctrl_cfg_gpremul_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1ctrl_cfg_palpha_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1ctrl_cfg_palpha_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_g1ctrl_cfg_galpha_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_g1ctrl_cfg_galpha_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_g1ctrl_cfg_blend_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_g1ctrl_cfg_blend_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_1ctrl_cfg_bitext(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1ctrl_cfg_bitext(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_1ctrl_cfg_a_order(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1ctrl_cfg_a_order(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1ctrl_cfg_rb_order(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1ctrl_cfg_rb_order(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_g1ctrl_cfg_src_fmt(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_g1ctrl_cfg_src_fmt(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_g1ctrl_cfg_regup_field(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_g1ctrl_cfg_regup_field(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_g1ctrl_cfg_regup_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_g1ctrl_cfg_regup_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_g1ctrl_cfg_layer_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_g1ctrl_cfg_layer_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_vout_set_g1upd_cfg_regup_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u32 hal_vout_get_g1bk_cfg_bk_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_g1bk_cfg_bk_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u8 hal_vout_get_voutg1_cfg_alpha_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_alpha_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutg1_cfg_alpha_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_alpha_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutg1_cfg_checker_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_checker_v(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutg1_cfg_checker_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_checker_h(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_voutg1_cfg_mute_pat(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_mute_pat(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_voutg1_cfg_mute_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_mute_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_voutg1_cfg_mute_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_mute_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u32 hal_vout_get_voutg1_cfg_mute_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_mute_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u16 hal_vout_get_voutg1_out_v(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_vout_get_voutg1_out_h(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_vout_get_voutg1_cfg_src_ypos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_src_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutg1_cfg_src_xpos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_src_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_g1src_cfg_src_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_g1src_cfg_src_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_g1src_cfg_src_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_g1src_cfg_src_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutg1_cfg_disp_ypos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_disp_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutg1_cfg_disp_xpos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_disp_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_g1disp_cfg_disp_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_g1disp_cfg_disp_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_g1disp_cfg_disp_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_g1disp_cfg_disp_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u32 hal_vout_get_voutg1_cfg_src_addr_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_src_addr_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutg1_cfg_src_addr_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_src_addr_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_voutg1_cfg_src_stride(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_src_stride(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_void hal_vout_set_voutg1_cfg_clut_rd_addr(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_void hal_vout_set_voutg1_cfg_clut_rd_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u32 hal_vout_get_voutg1_clut_rd_data(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_vout_get_voutg1_cfg_key_v_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_key_v_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutg1_cfg_key_u_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_key_u_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutg1_cfg_key_y_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_key_y_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutg1_cfg_key_a_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_key_a_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_voutg1_cfg_key_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_key_sel(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_voutg1_cfg_key_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_key_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_voutg1_cfg_key_v_max(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_key_v_max(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutg1_cfg_key_u_max(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_key_u_max(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutg1_cfg_key_y_max(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_key_y_max(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutg1_cfg_key_a_max(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_key_a_max(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutg1_cfg_key_v_min(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_key_v_min(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutg1_cfg_key_u_min(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_key_u_min(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutg1_cfg_key_y_min(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_key_y_min(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutg1_cfg_key_a_min(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_key_a_min(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutg1_cfg_key_v_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_key_v_msk(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutg1_cfg_key_u_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_key_u_msk(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutg1_cfg_key_y_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_key_y_msk(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutg1_cfg_key_a_msk(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_key_a_msk(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_voutg1_cfg_csc_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_csc_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_voutg1_cfg_csc_offset_in_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_csc_offset_in_2(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutg1_cfg_csc_offset_in_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_csc_offset_in_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutg1_cfg_csc_offset_in_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_csc_offset_in_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u16 hal_vout_get_voutg1_cfg_csc_matrix_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_csc_matrix_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutg1_cfg_csc_matrix_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_csc_matrix_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutg1_cfg_csc_matrix_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_csc_matrix_3(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutg1_cfg_csc_matrix_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_csc_matrix_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutg1_cfg_csc_matrix_5(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_csc_matrix_5(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutg1_cfg_csc_matrix_4(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_csc_matrix_4(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutg1_cfg_csc_matrix_7(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_csc_matrix_7(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutg1_cfg_csc_matrix_6(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_csc_matrix_6(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutg1_cfg_csc_matrix_8(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_csc_matrix_8(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutg1_cfg_csc_offset_out_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_csc_offset_out_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutg1_cfg_csc_offset_out_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_csc_offset_out_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutg1_cfg_csc_offset_out_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutg1_cfg_csc_offset_out_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_bool hal_vout_get_1ctrl_cfg_chk_sum_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1ctrl_cfg_chk_sum_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_1ctrl_cfg_hs_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1ctrl_cfg_hs_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_1ctrl_cfg_de_inv(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1ctrl_cfg_de_inv(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1ctrl_cfg_hs_inv(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1ctrl_cfg_hs_inv(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1ctrl_cfg_vs_inv(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1ctrl_cfg_vs_inv(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1ctrl_cfg_field_inv(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1ctrl_cfg_field_inv(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1ctrl_cfg_clock_edge_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1ctrl_cfg_clock_edge_sel(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1ctrl_cfg_interlaced(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1ctrl_cfg_interlaced(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_1ctrl_cfg_intf_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1ctrl_cfg_intf_sel(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_1ctrl_cfg_intf_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1ctrl_cfg_intf_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_vout_set_intf1upd_cfg_regup_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u16 hal_vout_get_voutintf1_cfg_para_pos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_para_pos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf1_cfg_regup_pos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_regup_pos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf1_cfg_start_pos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_start_pos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf1_cfg_prefetch_pos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_prefetch_pos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1act_cfg_hact(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1act_cfg_hact(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1act_cfg_vact(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1act_cfg_vact(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1vfb_cfg_vbb(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1vfb_cfg_vbb(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1vfb_cfg_vfb(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1vfb_cfg_vfb(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1bvfb_cfg_bvbb(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1bvfb_cfg_bvbb(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1bvfb_cfg_bvfb(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1bvfb_cfg_bvfb(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1hfb_cfg_hbb(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1hfb_cfg_hbb(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1hfb_cfg_hfb(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1hfb_cfg_hfb(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1hsw_cfg_hs_width(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1hsw_cfg_hs_width(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1field_cfg_bfield_tog(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1field_cfg_bfield_tog(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1field_cfg_field_tog(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1field_cfg_field_tog(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1vsh_cfg_vs_end_h_field0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1vsh_cfg_vs_end_h_field0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1vsh_cfg_vs_st_h_field0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1vsh_cfg_vs_st_h_field0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1vsv_cfg_vs_end_v_field0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1vsv_cfg_vs_end_v_field0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1vsv_cfg_vs_st_v_field0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1vsv_cfg_vs_st_v_field0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1bvsh_cfg_vs_end_h_field1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1bvsh_cfg_vs_end_h_field1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1bvsh_cfg_vs_st_h_field1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1bvsh_cfg_vs_st_h_field1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1bvsv_cfg_vs_end_v_field1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1bvsv_cfg_vs_end_v_field1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1bvsv_cfg_vs_st_v_field1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1bvsv_cfg_vs_st_v_field1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf1_cfg_intf_sfifo_thd(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_intf_sfifo_thd(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_bool hal_vout_get_1vtint_cfg_vt_int_cnt_sel_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1vtint_cfg_vt_int_cnt_sel_3(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1vtint_cfg_vt_int_cnt_sel_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1vtint_cfg_vt_int_cnt_sel_2(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1vtint_cfg_vt_int_cnt_sel_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1vtint_cfg_vt_int_cnt_sel_1(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1vtint_cfg_vt_int_cnt_sel_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1vtint_cfg_vt_int_cnt_sel_0(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1vtint_cfg_vt_int_mode_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1vtint_cfg_vt_int_mode_3(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1vtint_cfg_vt_int_mode_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1vtint_cfg_vt_int_mode_2(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1vtint_cfg_vt_int_mode_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1vtint_cfg_vt_int_mode_1(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1vtint_cfg_vt_int_mode_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1vtint_cfg_vt_int_mode_0(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u16 hal_vout_get_1vtintf1thd0_cfg_vt_int_thd_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1vtintf1thd0_cfg_vt_int_thd_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1vtintf1thd0_cfg_vt_int_thd_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1vtintf1thd0_cfg_vt_int_thd_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1vtintf1thd2_cfg_vt_int_thd_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1vtintf1thd2_cfg_vt_int_thd_3(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1vtintf1thd2_cfg_vt_int_thd_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1vtintf1thd2_cfg_vt_int_thd_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1sta_frm_cnt(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_1sta_field_flag_ppc(s_vout_regs_type *p_vout_reg);
xmedia_u8 hal_vout_get_1sta_vstate(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_vout_get_1sta2_vcnt_st_vact(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_vout_get_1sta2_vcnt_st_vblk(s_vout_regs_type *p_vout_reg);
xmedia_u16 hal_vout_get_1ufline_intf_uf_line(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_vout_get_1ysum_intf_y_sum(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_vout_get_1usum_intf_u_sum(s_vout_regs_type *p_vout_reg);
xmedia_u32 hal_vout_get_1vsum_intf_v_sum(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_1btctrl_cfg_bt_bit_inv(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1btctrl_cfg_bt_bit_inv(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1btctrl_cfg_vbit_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1btctrl_cfg_vbit_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1btctrl_cfg_bt_uv_order(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1btctrl_cfg_bt_uv_order(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1btctrl_cfg_bt_yc_order(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1btctrl_cfg_bt_yc_order(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1btctrl_cfg_bt_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1btctrl_cfg_bt_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u16 hal_vout_get_1btvbit_cfg_vbit_end(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1btvbit_cfg_vbit_end(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1btvbit_cfg_vbit_st(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1btvbit_cfg_vbit_st(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1btbvbit_cfg_bvbit_end(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1btbvbit_cfg_bvbit_end(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1btbvbit_cfg_bvbit_st(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1btbvbit_cfg_bvbit_st(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_bool hal_vout_get_1lcdctrl_cfg_lcd_bit_inv(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1lcdctrl_cfg_lcd_bit_inv(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_1lcdctrl_cfg_lcd_rgb_order_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1lcdctrl_cfg_lcd_rgb_order_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_1lcdctrl_cfg_lcd_rgb_order_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1lcdctrl_cfg_lcd_rgb_order_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_1lcdctrl_cfg_lcd_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1lcdctrl_cfg_lcd_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutintf1_cfg_lvds_lane_4_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_lvds_lane_4_sel(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutintf1_cfg_lvds_lane_3_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_lvds_lane_3_sel(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutintf1_cfg_lvds_lane_2_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_lvds_lane_2_sel(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutintf1_cfg_lvds_lane_1_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_lvds_lane_1_sel(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutintf1_cfg_lvds_lane_0_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_lvds_lane_0_sel(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutintf1_cfg_lvds_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_lvds_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_voutintf1_cfg_dpi_uv_order(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_dpi_uv_order(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_voutintf1_cfg_dpi_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_dpi_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutintf1_cfg_checker_v(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_checker_v(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutintf1_cfg_checker_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_checker_h(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_voutintf1_cfg_cbar_sel(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_cbar_sel(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_voutintf1_cfg_mute_pat(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_mute_pat(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_voutintf1_cfg_mute_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_mute_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_voutintf1_cfg_mute_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_mute_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u32 hal_vout_get_voutintf1_cfg_mute_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_mute_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_intf1mutebk_cfg_bk_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_intf1mutebk_cfg_bk_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_bool hal_vout_get_1inkctrl_cfg_cross_color_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1inkctrl_cfg_cross_color_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1inkctrl_cfg_cross_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1inkctrl_cfg_cross_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_1inkctrl_cfg_ink_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1inkctrl_cfg_ink_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u16 hal_vout_get_1inkpos_cfg_y_pos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1inkpos_cfg_y_pos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_1inkpos_cfg_x_pos(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1inkpos_cfg_x_pos(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u32 hal_vout_get_voutintf1_cfg_cross_color(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_cross_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_1inkdata_ink_data(s_vout_regs_type *p_vout_reg);
xmedia_bool hal_vout_get_1clipl_cfg_clip_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1clipl_cfg_clip_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u32 hal_vout_get_1clipl_cfg_clip_yuv_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1clipl_cfg_clip_yuv_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_u32 hal_vout_get_1cliph_cfg_clip_yuv_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_1cliph_cfg_clip_yuv_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value);
xmedia_bool hal_vout_get_voutintf1_cfg_csc_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_csc_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_voutintf1_cfg_csc_offset_in_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_csc_offset_in_2(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutintf1_cfg_csc_offset_in_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_csc_offset_in_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutintf1_cfg_csc_offset_in_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_csc_offset_in_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u16 hal_vout_get_voutintf1_cfg_csc_matrix_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_csc_matrix_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf1_cfg_csc_matrix_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_csc_matrix_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf1_cfg_csc_matrix_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_csc_matrix_3(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf1_cfg_csc_matrix_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_csc_matrix_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf1_cfg_csc_matrix_5(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_csc_matrix_5(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf1_cfg_csc_matrix_4(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_csc_matrix_4(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf1_cfg_csc_matrix_7(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_csc_matrix_7(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf1_cfg_csc_matrix_6(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_csc_matrix_6(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf1_cfg_csc_matrix_8(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_csc_matrix_8(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf1_cfg_csc_offset_out_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_csc_offset_out_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf1_cfg_csc_offset_out_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_csc_offset_out_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_u16 hal_vout_get_voutintf1_cfg_csc_offset_out_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_csc_offset_out_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value);
xmedia_bool hal_vout_get_voutintf1_cfg_hcds_mode(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_hcds_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_voutintf1_cfg_hcds_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_hcds_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_u8 hal_vout_get_voutintf1_cfg_hcds_coef_3(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_hcds_coef_3(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutintf1_cfg_hcds_coef_2(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_hcds_coef_2(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutintf1_cfg_hcds_coef_1(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_hcds_coef_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutintf1_cfg_hcds_coef_0(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_hcds_coef_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutintf1_cfg_dither_thd_h(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_dither_thd_h(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_u8 hal_vout_get_voutintf1_cfg_dither_thd_l(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_dither_thd_l(s_vout_regs_type *p_vout_reg,xmedia_u8 value);
xmedia_bool hal_vout_get_voutintf1_cfg_dither_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_dither_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_bool hal_vout_get_voutintf1_cfg_gamma_en(s_vout_regs_type *p_vout_reg);
xmedia_void hal_vout_set_voutintf1_cfg_gamma_en(s_vout_regs_type *p_vout_reg,xmedia_bool value);
xmedia_void hal_vout_reg_bit_write(volatile xmedia_uintptr_t addr, xmedia_u32 val,
                xmedia_ulong mask, xmedia_ulong offset);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_VOUT_H__ */
