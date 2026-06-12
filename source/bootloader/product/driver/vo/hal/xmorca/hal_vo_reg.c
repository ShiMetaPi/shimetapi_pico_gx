#include "hal_vo_reg.h"

xmedia_u32 hal_reg_vout_reg_read(volatile xmedia_uintptr_t addr)
{
    return *(volatile xmedia_u32 *)(addr);
}

xmedia_void hal_reg_vout_reg_write(volatile xmedia_uintptr_t addr, xmedia_u32 val)
{
    *(volatile xmedia_u32 *)(addr) = val;
    return;
}

/********************************************************
 testpin 输出选择
 0: 全0 
 1：intf field,vsync,hsync,de
 2：mcu_valid,mcu_op_type
 3：mcu_start,mcu_valid,mcu_last,mcu_
 rd_en
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_testpin_mode(s_vout_regs_type *p_vout_reg)
{
    u_vout_top_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_top_ctrl.u32)));
    return (xmedia_u8)val.bits.cfg_testpin_mode;
}

xmedia_void hal_reg_vout_set_cfg_testpin_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_top_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_top_ctrl.u32));
    val.bits.cfg_testpin_mode = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_top_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 VOUT 所有寄存器更新控制。 跟随INTF更新
 0 : 寄存器更新由更新使能控制 cfg_regup_en;
 1 : 寄存器每帧更新（调试模式）。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_regup_debug(s_vout_regs_type *p_vout_reg)
{
    u_vout_top_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_top_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_regup_debug;
}

xmedia_void hal_reg_vout_set_cfg_regup_debug(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_top_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_top_ctrl.u32));
    val.bits.cfg_regup_debug = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_top_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 DMA 模块动态时钟门控使能。 跟随INTF更新
 0 : 不使能；
 1：使能；
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_ck_dyn_gt_en_dma(s_vout_regs_type *p_vout_reg)
{
    u_vout_top_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_top_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_ck_dyn_gt_en_dma;
}

xmedia_void hal_reg_vout_set_cfg_ck_dyn_gt_en_dma(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_top_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_top_ctrl.u32));
    val.bits.cfg_ck_dyn_gt_en_dma = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_top_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 动态时钟门控使能（除 DMA 模块）。跟随INTF更新
 0 : 不使能；
 1：使能；
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_ck_dyn_gt_en(s_vout_regs_type *p_vout_reg)
{
    u_vout_top_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_top_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_ck_dyn_gt_en;
}

xmedia_void hal_reg_vout_set_cfg_ck_dyn_gt_en(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_top_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_top_ctrl.u32));
    val.bits.cfg_ck_dyn_gt_en = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_top_ctrl.u32)),val.u32);
    return;
}

xmedia_void hal_reg_vout_set_cfg_regup_en(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_top_upd val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_top_upd.u32));
    val.bits.cfg_regup_en = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_top_upd.u32)),val.u32);
    return;
}

/********************************************************
 1：intf 寄存器未更新。
 ********************************************************/
xmedia_bool hal_reg_vout_get_intf_regup_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_state val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_state.u32)));
    return (xmedia_bool)val.bits.intf_regup_sta;
}

/********************************************************
 1：g0 寄存器未更新。
 ********************************************************/
xmedia_bool hal_reg_vout_get_g0_regup_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_state val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_state.u32)));
    return (xmedia_bool)val.bits.g0_regup_sta;
}

/********************************************************
 1：v0 寄存器未更新。
 ********************************************************/
xmedia_bool hal_reg_vout_get_v0_regup_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_state val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_state.u32)));
    return (xmedia_bool)val.bits.v0_regup_sta;
}

/********************************************************
 1：top 寄存器未更新。
 ********************************************************/
xmedia_bool hal_reg_vout_get_top_regup_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_state val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_state.u32)));
    return (xmedia_bool)val.bits.top_regup_sta;
}

/********************************************************
 VOUT 版本。
 [31:24] 年；[23:20] 月；[19:12] 日；[11:4
 ]  时；[3:0] 其他。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_vout_ver_0(s_vout_regs_type *p_vout_reg)
{
    return hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_version_0)));
}

/********************************************************
 VOUT SVN tag 号。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_vout_ver_1(s_vout_regs_type *p_vout_reg)
{
    return hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_version_1)));
}

/********************************************************
 RAS 类型SRAM 调速接口配置:
 [16]STOV,默认值'b0
 [19:17]EMA,默认值'b011
 [21:20]EMAW,默认值'b00
 [22]EMAS,默认值'b0
 [23]RET1N,默认值'b1
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_ras_sram_ctrl(s_vout_regs_type *p_vout_reg)
{
    u_vout_mem_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mem_ctrl.u32)));
    return (xmedia_u16)val.bits.cfg_ras_sram_ctrl;
}

xmedia_void hal_reg_vout_set_cfg_ras_sram_ctrl(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_mem_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_mem_ctrl.u32));
    val.bits.cfg_ras_sram_ctrl = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mem_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 RFS 类型SRAM 调速接口配置：
 [2:0]EMA,默认值'b011
 [4:3]EMAW,默认值'b01
 [5]reserved
 [6]RET1N,默认值'b1
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_rfs_sram_ctrl(s_vout_regs_type *p_vout_reg)
{
    u_vout_mem_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mem_ctrl.u32)));
    return (xmedia_u16)val.bits.cfg_rfs_sram_ctrl;
}

xmedia_void hal_reg_vout_set_cfg_rfs_sram_ctrl(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_mem_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_mem_ctrl.u32));
    val.bits.cfg_rfs_sram_ctrl = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mem_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 RAT 类型SRAM 调速接口配置:
 [16]STOV,默认值'b0
 [17]STOVAB,默认值'b0
 [20:18]EMA,默认值'b011
 [22:21]EMAW,默认值'b01
 [23]EMAS,默认值'b0
 [24]EMAP,默认值'b0
 [25]RET1N,默认值'b1
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_rat_sram_ctrl(s_vout_regs_type *p_vout_reg)
{
    u_vout_mem_ctrl_1 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mem_ctrl_1.u32)));
    return (xmedia_u16)val.bits.cfg_rat_sram_ctrl;
}

xmedia_void hal_reg_vout_set_cfg_rat_sram_ctrl(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_mem_ctrl_1 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_mem_ctrl_1.u32));
    val.bits.cfg_rat_sram_ctrl = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mem_ctrl_1.u32)),val.u32);
    return;
}

/********************************************************
 RFT 类型SRAM 调速接口配置:
 [0]COLLDISN,默认值'b1
 [3:1]EMAA,默认值'b100
 [4]EMASA,默认值'b0
 [7:5]EMAB,默认值'b100
 [8]RET1N,默认值'b1
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_rft_sram_ctrl(s_vout_regs_type *p_vout_reg)
{
    u_vout_mem_ctrl_1 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mem_ctrl_1.u32)));
    return (xmedia_u16)val.bits.cfg_rft_sram_ctrl;
}

xmedia_void hal_reg_vout_set_cfg_rft_sram_ctrl(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_mem_ctrl_1 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_mem_ctrl_1.u32));
    val.bits.cfg_rft_sram_ctrl = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mem_ctrl_1.u32)),val.u32);
    return;
}

/********************************************************
 MCU屏操作冲突错误中断。使能cfg_intf_mcu_operr_c
 lr清零。
 ********************************************************/
xmedia_bool hal_reg_vout_get_intf_mcu_operr_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_sta val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_sta.u32)));
    return (xmedia_bool)val.bits.intf_mcu_operr_sta;
}

/********************************************************
 MCU屏写display错误中断。使能cfg_intf_mcu_wre
 rr_clr清零。
 ********************************************************/
xmedia_bool hal_reg_vout_get_intf_mcu_wrerr_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_sta val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_sta.u32)));
    return (xmedia_bool)val.bits.intf_mcu_wrerr_sta;
}

/********************************************************
 总线错误中断。使能 cfg_axi_bus_werr_clr 清零。
 ********************************************************/
xmedia_bool hal_reg_vout_get_axi_bus_werr_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_sta val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_sta.u32)));
    return (xmedia_bool)val.bits.axi_bus_werr_sta;
}

/********************************************************
 总线错误中断。使能 cfg_axi_bus_rerr_clr 清零。
 ********************************************************/
xmedia_bool hal_reg_vout_get_axi_bus_rerr_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_sta val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_sta.u32)));
    return (xmedia_bool)val.bits.axi_bus_rerr_sta;
}

/********************************************************
 低延时Y分量申请数据的行号大于 帧存保存的行号。使能 cfg_low_
 delay_y_err_clr 清零。
 ********************************************************/
xmedia_bool hal_reg_vout_get_low_delay_y_err_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_sta val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_sta.u32)));
    return (xmedia_bool)val.bits.low_delay_y_err_sta;
}

/********************************************************
 低延时C分量申请数据的行号大于 帧存保存的行号。使能 cfg_low_
 delay_c_err_clr 清零。
 ********************************************************/
xmedia_bool hal_reg_vout_get_low_delay_c_err_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_sta val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_sta.u32)));
    return (xmedia_bool)val.bits.low_delay_c_err_sta;
}

/********************************************************
 Intf 0 低带宽中断。使能 cfg_intf_underflow_
 clr 清零。
 ********************************************************/
xmedia_bool hal_reg_vout_get_intf_underflow_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_sta val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_sta.u32)));
    return (xmedia_bool)val.bits.intf_underflow_sta;
}

/********************************************************
 Intf 0 垂直时序中断3。使能 cfg_intf_vt_int_3
 _clr 清零。
 ********************************************************/
xmedia_bool hal_reg_vout_get_intf_vt_int_3_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_sta val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_sta.u32)));
    return (xmedia_bool)val.bits.intf_vt_int_3_sta;
}

/********************************************************
 Intf 0 垂直时序中断2。使能 cfg_intf_vt_int_2
 _clr 清零。
 ********************************************************/
xmedia_bool hal_reg_vout_get_intf_vt_int_2_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_sta val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_sta.u32)));
    return (xmedia_bool)val.bits.intf_vt_int_2_sta;
}

/********************************************************
 Intf 0 垂直时序中断1。使能 cfg_intf_vt_int_1
 _clr 清零。
 ********************************************************/
xmedia_bool hal_reg_vout_get_intf_vt_int_1_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_sta val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_sta.u32)));
    return (xmedia_bool)val.bits.intf_vt_int_1_sta;
}

/********************************************************
 Intf 0 垂直时序中断0。使能 cfg_intf_vt_int_0
 _clr 清零。
 ********************************************************/
xmedia_bool hal_reg_vout_get_intf_vt_int_0_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_sta val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_sta.u32)));
    return (xmedia_bool)val.bits.intf_vt_int_0_sta;
}

xmedia_void hal_reg_vout_set_cfg_intf_mcu_operr_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_int_clr val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_int_clr.u32));
    val.bits.cfg_intf_mcu_operr_clr = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_clr.u32)),val.u32);
    return;
}

xmedia_void hal_reg_vout_set_cfg_intf_mcu_wrerr_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_int_clr val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_int_clr.u32));
    val.bits.cfg_intf_mcu_wrerr_clr = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_clr.u32)),val.u32);
    return;
}

xmedia_void hal_reg_vout_set_cfg_axi_bus_werr_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_int_clr val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_int_clr.u32));
    val.bits.cfg_axi_bus_werr_clr = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_clr.u32)),val.u32);
    return;
}

xmedia_void hal_reg_vout_set_cfg_axi_bus_rerr_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_int_clr val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_int_clr.u32));
    val.bits.cfg_axi_bus_rerr_clr = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_clr.u32)),val.u32);
    return;
}

xmedia_void hal_reg_vout_set_cfg_low_delay_y_err_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_int_clr val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_int_clr.u32));
    val.bits.cfg_low_delay_y_err_clr = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_clr.u32)),val.u32);
    return;
}

xmedia_void hal_reg_vout_set_cfg_low_delay_c_err_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_int_clr val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_int_clr.u32));
    val.bits.cfg_low_delay_c_err_clr = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_clr.u32)),val.u32);
    return;
}

xmedia_void hal_reg_vout_set_cfg_intf_underflow_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_int_clr val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_int_clr.u32));
    val.bits.cfg_intf_underflow_clr = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_clr.u32)),val.u32);
    return;
}

xmedia_void hal_reg_vout_set_cfg_intf_vt_int_3_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_int_clr val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_int_clr.u32));
    val.bits.cfg_intf_vt_int_3_clr = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_clr.u32)),val.u32);
    return;
}

xmedia_void hal_reg_vout_set_cfg_intf_vt_int_2_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_int_clr val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_int_clr.u32));
    val.bits.cfg_intf_vt_int_2_clr = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_clr.u32)),val.u32);
    return;
}

xmedia_void hal_reg_vout_set_cfg_intf_vt_int_1_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_int_clr val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_int_clr.u32));
    val.bits.cfg_intf_vt_int_1_clr = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_clr.u32)),val.u32);
    return;
}

xmedia_void hal_reg_vout_set_cfg_intf_vt_int_0_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_int_clr val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_int_clr.u32));
    val.bits.cfg_intf_vt_int_0_clr = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_clr.u32)),val.u32);
    return;
}

/********************************************************
 MCU屏操作冲突错误中断掩码。0: 屏蔽中断；1： 使能中断产生；
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_intf_mcu_operr_msk(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_msk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_msk.u32)));
    return (xmedia_bool)val.bits.cfg_intf_mcu_operr_msk;
}

xmedia_void hal_reg_vout_set_cfg_intf_mcu_operr_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_int_msk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_int_msk.u32));
    val.bits.cfg_intf_mcu_operr_msk = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_msk.u32)),val.u32);
    return;
}

/********************************************************
 MCU屏写display错误中断掩码。0: 屏蔽中断；1： 使能中断产
 生；
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_intf_mcu_wrerr_msk(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_msk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_msk.u32)));
    return (xmedia_bool)val.bits.cfg_intf_mcu_wrerr_msk;
}

xmedia_void hal_reg_vout_set_cfg_intf_mcu_wrerr_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_int_msk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_int_msk.u32));
    val.bits.cfg_intf_mcu_wrerr_msk = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_msk.u32)),val.u32);
    return;
}

/********************************************************
 总线错误中断。0: 屏蔽中断；1： 使能中断产生；
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_axi_bus_werr_msk(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_msk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_msk.u32)));
    return (xmedia_bool)val.bits.cfg_axi_bus_werr_msk;
}

xmedia_void hal_reg_vout_set_cfg_axi_bus_werr_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_int_msk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_int_msk.u32));
    val.bits.cfg_axi_bus_werr_msk = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_msk.u32)),val.u32);
    return;
}

/********************************************************
 总线错误中断。0: 屏蔽中断；1： 使能中断产生；
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_axi_bus_rerr_msk(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_msk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_msk.u32)));
    return (xmedia_bool)val.bits.cfg_axi_bus_rerr_msk;
}

xmedia_void hal_reg_vout_set_cfg_axi_bus_rerr_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_int_msk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_int_msk.u32));
    val.bits.cfg_axi_bus_rerr_msk = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_msk.u32)),val.u32);
    return;
}

/********************************************************
 低延时Y分量申请数据的行号大于 帧存保存的行号。
 0: 屏蔽中断；1： 使能中断产生；
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_low_delay_y_err_msk(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_msk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_msk.u32)));
    return (xmedia_bool)val.bits.cfg_low_delay_y_err_msk;
}

xmedia_void hal_reg_vout_set_cfg_low_delay_y_err_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_int_msk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_int_msk.u32));
    val.bits.cfg_low_delay_y_err_msk = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_msk.u32)),val.u32);
    return;
}

/********************************************************
 低延时C分量申请数据的行号大于 帧存保存的行号。
 0: 屏蔽中断；1： 使能中断产生；
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_low_delay_c_err_msk(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_msk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_msk.u32)));
    return (xmedia_bool)val.bits.cfg_low_delay_c_err_msk;
}

xmedia_void hal_reg_vout_set_cfg_low_delay_c_err_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_int_msk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_int_msk.u32));
    val.bits.cfg_low_delay_c_err_msk = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_msk.u32)),val.u32);
    return;
}

/********************************************************
 Intf  0 低带宽中断。0: 屏蔽中断；1： 使能中断产生；
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_intf_underflow_msk(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_msk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_msk.u32)));
    return (xmedia_bool)val.bits.cfg_intf_underflow_msk;
}

xmedia_void hal_reg_vout_set_cfg_intf_underflow_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_int_msk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_int_msk.u32));
    val.bits.cfg_intf_underflow_msk = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_msk.u32)),val.u32);
    return;
}

/********************************************************
 Intf  0 垂直时序中断3。0: 屏蔽中断；1： 使能中断产生；
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_intf_vt_int_3_msk(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_msk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_msk.u32)));
    return (xmedia_bool)val.bits.cfg_intf_vt_int_3_msk;
}

xmedia_void hal_reg_vout_set_cfg_intf_vt_int_3_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_int_msk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_int_msk.u32));
    val.bits.cfg_intf_vt_int_3_msk = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_msk.u32)),val.u32);
    return;
}

/********************************************************
 Intf  0 垂直时序中断2。0: 屏蔽中断；1： 使能中断产生；
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_intf_vt_int_2_msk(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_msk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_msk.u32)));
    return (xmedia_bool)val.bits.cfg_intf_vt_int_2_msk;
}

xmedia_void hal_reg_vout_set_cfg_intf_vt_int_2_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_int_msk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_int_msk.u32));
    val.bits.cfg_intf_vt_int_2_msk = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_msk.u32)),val.u32);
    return;
}

/********************************************************
 Intf  0 垂直时序中断1。0: 屏蔽中断；1： 使能中断产生；
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_intf_vt_int_1_msk(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_msk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_msk.u32)));
    return (xmedia_bool)val.bits.cfg_intf_vt_int_1_msk;
}

xmedia_void hal_reg_vout_set_cfg_intf_vt_int_1_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_int_msk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_int_msk.u32));
    val.bits.cfg_intf_vt_int_1_msk = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_msk.u32)),val.u32);
    return;
}

/********************************************************
 Intf  0 垂直时序中断0。0: 屏蔽中断；1： 使能中断产生；
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_intf_vt_int_0_msk(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_msk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_msk.u32)));
    return (xmedia_bool)val.bits.cfg_intf_vt_int_0_msk;
}

xmedia_void hal_reg_vout_set_cfg_intf_vt_int_0_msk(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_int_msk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_int_msk.u32));
    val.bits.cfg_intf_vt_int_0_msk = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_msk.u32)),val.u32);
    return;
}

/********************************************************
 MCU屏操作冲突错误中断。
 ********************************************************/
xmedia_bool hal_reg_vout_get_intf_mcu_operr_src_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_src_sta val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_src_sta.u32)));
    return (xmedia_bool)val.bits.intf_mcu_operr_src_sta;
}

/********************************************************
 MCU屏写display错误中断。
 ********************************************************/
xmedia_bool hal_reg_vout_get_intf_mcu_wrerr_src_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_src_sta val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_src_sta.u32)));
    return (xmedia_bool)val.bits.intf_mcu_wrerr_src_sta;
}

/********************************************************
 总线错误中断。使能 cfg_axi_bus_werr_clr 清零。
 ********************************************************/
xmedia_bool hal_reg_vout_get_axi_bus_werr_src_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_src_sta val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_src_sta.u32)));
    return (xmedia_bool)val.bits.axi_bus_werr_src_sta;
}

/********************************************************
 总线错误中断。使能 cfg_axi_bus_rerr_clr 清零。
 ********************************************************/
xmedia_bool hal_reg_vout_get_axi_bus_rerr_src_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_src_sta val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_src_sta.u32)));
    return (xmedia_bool)val.bits.axi_bus_rerr_src_sta;
}

/********************************************************
 低延时Y分量申请数据的行号大于 帧存保存的行号。使能 cfg_low_
 delay_y_err_clr 清零。
 ********************************************************/
xmedia_bool hal_reg_vout_get_low_delay_y_err_src_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_src_sta val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_src_sta.u32)));
    return (xmedia_bool)val.bits.low_delay_y_err_src_sta;
}

/********************************************************
 低延时C分量申请数据的行号大于 帧存保存的行号。使能 cfg_low_
 delay_c_err_clr 清零。
 ********************************************************/
xmedia_bool hal_reg_vout_get_low_delay_c_err_src_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_src_sta val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_src_sta.u32)));
    return (xmedia_bool)val.bits.low_delay_c_err_src_sta;
}

/********************************************************
 Intf 0 低带宽中断。配置 cfg_uf_clr，逻辑在cfg_p
 refetch_pos清零。
 ********************************************************/
xmedia_bool hal_reg_vout_get_intf_underflow_src_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_src_sta val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_src_sta.u32)));
    return (xmedia_bool)val.bits.intf_underflow_src_sta;
}

/********************************************************
 Intf 0 垂直时序中断3。使能 cfg_intf_vt_int_3
 _clr 清零。
 ********************************************************/
xmedia_bool hal_reg_vout_get_intf_vt_int_3_src_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_src_sta val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_src_sta.u32)));
    return (xmedia_bool)val.bits.intf_vt_int_3_src_sta;
}

/********************************************************
 Intf 0 垂直时序中断2。使能 cfg_intf_vt_int_2
 _clr 清零。
 ********************************************************/
xmedia_bool hal_reg_vout_get_intf_vt_int_2_src_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_src_sta val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_src_sta.u32)));
    return (xmedia_bool)val.bits.intf_vt_int_2_src_sta;
}

/********************************************************
 Intf 0 垂直时序中断1。使能 cfg_intf_vt_int_1
 _clr 清零。
 ********************************************************/
xmedia_bool hal_reg_vout_get_intf_vt_int_1_src_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_src_sta val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_src_sta.u32)));
    return (xmedia_bool)val.bits.intf_vt_int_1_src_sta;
}

/********************************************************
 Intf 0 垂直时序中断0。使能 cfg_intf_vt_int_0
 _clr 清零。
 ********************************************************/
xmedia_bool hal_reg_vout_get_intf_vt_int_0_src_sta(s_vout_regs_type *p_vout_reg)
{
    u_vout_int_src_sta val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_int_src_sta.u32)));
    return (xmedia_bool)val.bits.intf_vt_int_0_src_sta;
}

xmedia_void hal_reg_vout_set_cfg_uf_clr(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_sta_clr val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_sta_clr.u32));
    val.bits.cfg_uf_clr = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_sta_clr.u32)),val.u32);
    return;
}

/********************************************************
 DMA G0 通道 busy 状态。
 ********************************************************/
xmedia_bool hal_reg_vout_get_rdma_busy_g0(s_vout_regs_type *p_vout_reg)
{
    u_vout_debug val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_debug.u32)));
    return (xmedia_bool)val.bits.rdma_busy_g0;
}

/********************************************************
 DMA1 V0 C 通道 busy 状态。
 ********************************************************/
xmedia_bool hal_reg_vout_get_rdma1_busy_v0_c(s_vout_regs_type *p_vout_reg)
{
    u_vout_debug val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_debug.u32)));
    return (xmedia_bool)val.bits.rdma1_busy_v0_c;
}

/********************************************************
 DMA1  V0 Y 通道 busy 状态。
 ********************************************************/
xmedia_bool hal_reg_vout_get_rdma1_busy_v0_y(s_vout_regs_type *p_vout_reg)
{
    u_vout_debug val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_debug.u32)));
    return (xmedia_bool)val.bits.rdma1_busy_v0_y;
}

/********************************************************
 DMA  V0 C 通道 busy 状态。
 ********************************************************/
xmedia_bool hal_reg_vout_get_rdma_busy_v0_c(s_vout_regs_type *p_vout_reg)
{
    u_vout_debug val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_debug.u32)));
    return (xmedia_bool)val.bits.rdma_busy_v0_c;
}

/********************************************************
 DMA  V0 Y 通道 busy 状态。
 ********************************************************/
xmedia_bool hal_reg_vout_get_rdma_busy_v0_y(s_vout_regs_type *p_vout_reg)
{
    u_vout_debug val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_debug.u32)));
    return (xmedia_bool)val.bits.rdma_busy_v0_y;
}

/********************************************************
 读带宽统计时间窗个数（指数）， 
 若配置为N，则统计时间窗个数等于2^N 个节点。
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_rd_bw_window_num_pow(s_vout_regs_type *p_vout_reg)
{
    u_vout_dma_dfx_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_dfx_ctrl.u32)));
    return (xmedia_u8)val.bits.cfg_rd_bw_window_num_pow;
}

xmedia_void hal_reg_vout_set_cfg_rd_bw_window_num_pow(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_dma_dfx_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_dma_dfx_ctrl.u32));
    val.bits.cfg_rd_bw_window_num_pow = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_dfx_ctrl.u32)),val.u32);
    return;
}

/********************************************************
  读带宽 统计通道选择。 
 0: V0 Y;   1: V0 C; 2: V0 Y1; 3 : V
 0 C1; 4: G0;5:所有通道总带宽
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_rd_bw_ch_sel(s_vout_regs_type *p_vout_reg)
{
    u_vout_dma_dfx_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_dfx_ctrl.u32)));
    return (xmedia_u8)val.bits.cfg_rd_bw_ch_sel;
}

xmedia_void hal_reg_vout_set_cfg_rd_bw_ch_sel(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_dma_dfx_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_dma_dfx_ctrl.u32));
    val.bits.cfg_rd_bw_ch_sel = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_dfx_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 读latency 统计通道选择。
 0: V0 Y;   1: V0 C; 2: V0 Y1; 3 : V
 0 C1; 4: G0;
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_rd_latency_ch_sel(s_vout_regs_type *p_vout_reg)
{
    u_vout_dma_dfx_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_dfx_ctrl.u32)));
    return (xmedia_u8)val.bits.cfg_rd_latency_ch_sel;
}

xmedia_void hal_reg_vout_set_cfg_rd_latency_ch_sel(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_dma_dfx_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_dma_dfx_ctrl.u32));
    val.bits.cfg_rd_latency_ch_sel = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_dfx_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 读带宽统计。
 需要选择通道和统计窗口 cfg_rd_bw_ch_sel & cfg_
 rd_bw_window_num_pow。
 每2^N个节点刷新统计数据：
 dfx_rd_bw_trans_max/dfx_rd_bw_trans
 _min/
 dfx_rd_bw_trans_avg/dfx_rd_bw_trans
 _window_cycle_avg。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_rd_bw_enable(s_vout_regs_type *p_vout_reg)
{
    u_vout_dma_dfx_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_dfx_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_rd_bw_enable;
}

xmedia_void hal_reg_vout_set_cfg_rd_bw_enable(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_dma_dfx_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_dma_dfx_ctrl.u32));
    val.bits.cfg_rd_bw_enable = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_dfx_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 读latency 统计使能。需要选择通道 cfg_rd_latency
 _ch_sel。
 每个节点运行完刷新统计数据：
 dfx_rd_latency_min/dfx_rd_latency_m
 ax/
 dfx_rd_latency_sum/dfx_rd_latency_b
 urst_num。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_rd_latency_enable(s_vout_regs_type *p_vout_reg)
{
    u_vout_dma_dfx_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_dfx_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_rd_latency_enable;
}

xmedia_void hal_reg_vout_set_cfg_rd_latency_enable(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_dma_dfx_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_dma_dfx_ctrl.u32));
    val.bits.cfg_rd_latency_enable = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_dfx_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 统计使能。
 在统计使能关闭后刷新统计数据：
 axi_wr_burst_num/axi_wr_ost_max/axi
 _wr_ost_sum/
 axi_rd_burst_num/axi_rd_ost_max/axi
 _rd_ost_sum。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_axi_sta_enable(s_vout_regs_type *p_vout_reg)
{
    u_vout_dma_dfx_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_dfx_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_axi_sta_enable;
}

xmedia_void hal_reg_vout_set_cfg_axi_sta_enable(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_dma_dfx_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_dma_dfx_ctrl.u32));
    val.bits.cfg_axi_sta_enable = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_dfx_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 cfg_axi_sta_enable 统计使能期间burst总数，在统
 计使能关闭后刷新。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_dfx_axi_rd_burst_num(s_vout_regs_type *p_vout_reg)
{
    return hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_dfx_rd_0)));
}

/********************************************************
 cfg_axi_sta_enable 统计使能期间outstandin
 g最大值，在统计使能关闭后刷新。
 ********************************************************/
xmedia_u8 hal_reg_vout_get_dfx_axi_rd_ost_max(s_vout_regs_type *p_vout_reg)
{
    u_vout_dma_dfx_rd_1 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_dfx_rd_1.u32)));
    return (xmedia_u8)val.bits.dfx_axi_rd_ost_max;
}

/********************************************************
 cfg_axi_sta_enable 统计使能期间outstandin
 g累加和，在统计使能关闭后刷新
 ********************************************************/
xmedia_u8 hal_reg_vout_get_dfx_axi_rd_ost_sum_h(s_vout_regs_type *p_vout_reg)
{
    u_vout_dma_dfx_rd_1 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_dfx_rd_1.u32)));
    return (xmedia_u8)val.bits.dfx_axi_rd_ost_sum_h;
}

/********************************************************
 cfg_axi_sta_enable 统计使能期间outstandin
 g累加和，在统计使能关闭后刷新
 ********************************************************/
xmedia_u32 hal_reg_vout_get_dfx_axi_rd_ost_sum_l(s_vout_regs_type *p_vout_reg)
{
    return hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_dfx_rd_2)));
}

/********************************************************
 统计一帧内latency最小值。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_dfx_rd_latency_min(s_vout_regs_type *p_vout_reg)
{
    u_vout_dma_dfx_rd_3 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_dfx_rd_3.u32)));
    return (xmedia_u16)val.bits.dfx_rd_latency_min;
}

/********************************************************
 统计一帧内latency最大值。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_dfx_rd_latency_max(s_vout_regs_type *p_vout_reg)
{
    u_vout_dma_dfx_rd_3 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_dfx_rd_3.u32)));
    return (xmedia_u16)val.bits.dfx_rd_latency_max;
}

/********************************************************
 统计一帧内latency累计和。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_dfx_rd_latency_sum_h(s_vout_regs_type *p_vout_reg)
{
    u_vout_dma_dfx_rd_4 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_dfx_rd_4.u32)));
    return (xmedia_u16)val.bits.dfx_rd_latency_sum_h;
}

/********************************************************
 统计一帧内latency累计和。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_dfx_rd_latency_sum_l(s_vout_regs_type *p_vout_reg)
{
    return hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_dfx_rd_5)));
}

/********************************************************
 统计一帧内burst 总个数。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_dfx_rd_latency_burst_num(s_vout_regs_type *p_vout_reg)
{
    return hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_dfx_rd_6)));
}

/********************************************************
 带宽统计：2^N个统计时间窗内transfer个数最大值。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_dfx_rd_bw_trans_max(s_vout_regs_type *p_vout_reg)
{
    return hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_dfx_rd_7)));
}

/********************************************************
 带宽统计：2^N个统计时间窗内transfer个数最小值。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_dfx_rd_bw_trans_min(s_vout_regs_type *p_vout_reg)
{
    return hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_dfx_rd_8)));
}

/********************************************************
 带宽统计：2^N个统计时间窗内transfer个数的平均值。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_dfx_rd_bw_trans_avg(s_vout_regs_type *p_vout_reg)
{
    return hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_dfx_rd_9)));
}

/********************************************************
 带宽统计：2^N个统计时间窗内时钟cycle个数的平均值。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_dfx_rd_bw_trans_window_cycle_avg(s_vout_regs_type *p_vout_reg)
{
    return hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_dfx_rd_10)));
}

/********************************************************
 叠加层优先级 。（第1层） 跟随INTF 更新
 0：无层驱动 （背景色）
 1：V0 （需要使能层开关）
 2：G0 （需要使能层开关）
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_mixer_prio1(s_vout_regs_type *p_vout_reg)
{
    u_vout_cbm_mixer val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_cbm_mixer.u32)));
    return (xmedia_u8)val.bits.cfg_mixer_prio1;
}

xmedia_void hal_reg_vout_set_cfg_mixer_prio1(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_cbm_mixer val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_cbm_mixer.u32));
    val.bits.cfg_mixer_prio1 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_cbm_mixer.u32)),val.u32);
    return;
}

/********************************************************
 叠加层优先级 。（第0层）  跟随INTF 更新
 0：无层驱动 （背景色）
 1：V0 （需要使能层开关）
 2：G0 （需要使能层开关）
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_mixer_prio0(s_vout_regs_type *p_vout_reg)
{
    u_vout_cbm_mixer val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_cbm_mixer.u32)));
    return (xmedia_u8)val.bits.cfg_mixer_prio0;
}

xmedia_void hal_reg_vout_set_cfg_mixer_prio0(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_cbm_mixer val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_cbm_mixer.u32));
    val.bits.cfg_mixer_prio0 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_cbm_mixer.u32)),val.u32);
    return;
}

/********************************************************
 CBM 叠加背景色 。  跟随INTF 更新
 [31:24] 保留
 [23:16] Y/R
 [15:8] U/G
 [7:0] V/B
 ********************************************************/
xmedia_u32 hal_reg_vout_get_cfg_bk_color(s_vout_regs_type *p_vout_reg)
{
    return hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_cbm_bk)));
}

xmedia_void hal_reg_vout_set_cfg_bk_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_cbm_bk)),value);
    return;
}

/********************************************************
 读 DMA outstanding 配置。最大8；跟随INTF更新
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_axi_rd_ost_num(s_vout_regs_type *p_vout_reg)
{
    u_vout_dma_ost_num val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_ost_num.u32)));
    return (xmedia_u8)val.bits.cfg_axi_rd_ost_num;
}

xmedia_void hal_reg_vout_set_cfg_axi_rd_ost_num(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_dma_ost_num val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_dma_ost_num.u32));
    val.bits.cfg_axi_rd_ost_num = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_dma_ost_num.u32)),val.u32);
    return;
}

xmedia_void hal_reg_vout_set_cfg_para_2_update(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_para_update val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_para_update.u32));
    val.bits.cfg_para_2_update = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_para_update.u32)),val.u32);
    return;
}

xmedia_void hal_reg_vout_set_cfg_para_0_update(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_para_update val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_para_update.u32));
    val.bits.cfg_para_0_update = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_para_update.u32)),val.u32);
    return;
}

/********************************************************
 G0 CLUT 申请数据 地址。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_cfg_para_0_addr_h(s_vout_regs_type *p_vout_reg)
{
    u_vout_para_0_addr_h val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_para_0_addr_h.u32)));
    return (xmedia_u32)val.bits.cfg_para_0_addr_h;
}

xmedia_void hal_reg_vout_set_cfg_para_0_addr_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_para_0_addr_h val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_para_0_addr_h.u32));
    val.bits.cfg_para_0_addr_h = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_para_0_addr_h.u32)),val.u32);
    return;
}

/********************************************************
 G0 CLUT 申请数据 地址。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_cfg_para_0_addr_l(s_vout_regs_type *p_vout_reg)
{
    u_vout_para_0_addr_l val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_para_0_addr_l.u32)));
    return (xmedia_u32)val.bits.cfg_para_0_addr_l;
}

xmedia_void hal_reg_vout_set_cfg_para_0_addr_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_para_0_addr_l val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_para_0_addr_l.u32));
    val.bits.cfg_para_0_addr_l = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_para_0_addr_l.u32)),val.u32);
    return;
}

/********************************************************
 INTF0 Gamma 申请数据 地址。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_cfg_para_2_addr_h(s_vout_regs_type *p_vout_reg)
{
    u_vout_para_2_addr_h val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_para_2_addr_h.u32)));
    return (xmedia_u32)val.bits.cfg_para_2_addr_h;
}

xmedia_void hal_reg_vout_set_cfg_para_2_addr_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_para_2_addr_h val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_para_2_addr_h.u32));
    val.bits.cfg_para_2_addr_h = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_para_2_addr_h.u32)),val.u32);
    return;
}

/********************************************************
 INTF0 Gamma申请数据 地址。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_cfg_para_2_addr_l(s_vout_regs_type *p_vout_reg)
{
    u_vout_para_2_addr_l val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_para_2_addr_l.u32)));
    return (xmedia_u32)val.bits.cfg_para_2_addr_l;
}

xmedia_void hal_reg_vout_set_cfg_para_2_addr_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_para_2_addr_l val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_para_2_addr_l.u32));
    val.bits.cfg_para_2_addr_l = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_para_2_addr_l.u32)),val.u32);
    return;
}

/********************************************************
 全局 alpha 值。
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_galpha(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_ctrl.u32)));
    return (xmedia_u8)val.bits.cfg_galpha;
}

xmedia_void hal_reg_vout_set_cfg_galpha(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_v0_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_ctrl.u32));
    val.bits.cfg_galpha = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 全局 alpha 使能。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_galpha_en(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_galpha_en;
}

xmedia_void hal_reg_vout_set_cfg_galpha_en(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_v0_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_ctrl.u32));
    val.bits.cfg_galpha_en = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 层叠加模式。即背景色 + 视频层
 0: SRC  （直接覆盖，典型应用）
 1: SRC OVER （alpha blend)
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_blend_mode(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_blend_mode;
}

xmedia_void hal_reg_vout_set_cfg_blend_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_v0_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_ctrl.u32));
    val.bits.cfg_blend_mode = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 UV 顺序  
 0 UV ; 1 VU;
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_src_uv_order(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_src_uv_order;
}

xmedia_void hal_reg_vout_set_cfg_src_uv_order(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_v0_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_ctrl.u32));
    val.bits.cfg_src_uv_order = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 层输入像素格式 
 3'd0：YUV400
 3'd1：YUV420
 3'd2：YUV422
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_src_fmt(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_ctrl.u32)));
    return (xmedia_u8)val.bits.cfg_src_fmt;
}

xmedia_void hal_reg_vout_set_cfg_src_fmt(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_v0_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_ctrl.u32));
    val.bits.cfg_src_fmt = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 寄存器系数场更新模式（即时寄存器）。
 0：顶场更新； 
 1：底场更新。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_regup_field(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_regup_field;
}

xmedia_void hal_reg_vout_set_cfg_regup_field(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_v0_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_ctrl.u32));
    val.bits.cfg_regup_field = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 寄存器系数更新模式（即时寄存器）。
 0：帧更新； 
 1：场更新。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_regup_mode(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_regup_mode;
}

xmedia_void hal_reg_vout_set_cfg_regup_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_v0_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_ctrl.u32));
    val.bits.cfg_regup_mode = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 层使能。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_layer_en(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_layer_en;
}

xmedia_void hal_reg_vout_set_cfg_layer_en(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_v0_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_ctrl.u32));
    val.bits.cfg_layer_en = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_ctrl.u32)),val.u32);
    return;
}

xmedia_void hal_reg_vout_set_v0upd_cfg_regup_en(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_v0_upd val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_upd.u32));
    val.bits.cfg_regup_en = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_upd.u32)),val.u32);
    return;
}

/********************************************************
 层背景色。 
 多区域场景，CBM 叠加时，背景色会替换为 CBM 背景色。
 mute 棋盘格 黑色块 颜色设置。
 颜色格式按照intf输出格式配置
 ********************************************************/
xmedia_u32 hal_reg_vout_get_v0bk_cfg_bk_color(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_bk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_bk.u32)));
    return (xmedia_u32)val.bits.cfg_bk_color;
}

xmedia_void hal_reg_vout_set_v0bk_cfg_bk_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_v0_bk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_bk.u32));
    val.bits.cfg_bk_color = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_bk.u32)),val.u32);
    return;
}

/********************************************************
 mute 棋盘格高，减1配置。（单区域/多区域共用）
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_checker_v(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mute_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mute_ctrl.u32)));
    return (xmedia_u8)val.bits.cfg_checker_v;
}

xmedia_void hal_reg_vout_set_cfg_checker_v(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_v0_mute_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mute_ctrl.u32));
    val.bits.cfg_checker_v = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mute_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 mute 棋盘格宽，减1配置。（单区域/多区域共用）
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_checker_h(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mute_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mute_ctrl.u32)));
    return (xmedia_u8)val.bits.cfg_checker_h;
}

xmedia_void hal_reg_vout_set_cfg_checker_h(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_v0_mute_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mute_ctrl.u32));
    val.bits.cfg_checker_h = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mute_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 0 : 纯色。
 1 : 使能棋盘格。（单区域/多区域共用）
 颜色格式按照intf输出格式配置
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_mute_pat(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mute_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mute_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_mute_pat;
}

xmedia_void hal_reg_vout_set_cfg_mute_pat(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_v0_mute_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mute_ctrl.u32));
    val.bits.cfg_mute_pat = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mute_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 mute 模式。（单区域/多区域共用）
 0:使能 DMA 申请数据，替换视频数据。
 1:不使能DMA 申请数据，自建 valid/data。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_mute_mode(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mute_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mute_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_mute_mode;
}

xmedia_void hal_reg_vout_set_cfg_mute_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_v0_mute_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mute_ctrl.u32));
    val.bits.cfg_mute_mode = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mute_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 层输出高。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_out_v(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_oreso val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_oreso.u32)));
    return (xmedia_u16)val.bits.out_v;
}

/********************************************************
 层输出宽。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_out_h(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_oreso val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_oreso.u32)));
    return (xmedia_u16)val.bits.out_h;
}

/********************************************************
 层真实内容在显示窗口的Y坐标。 (无区域使能时以此输出背景图)
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_src_ypos(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_src_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_src_pos.u32)));
    return (xmedia_u16)val.bits.cfg_src_ypos;
}

xmedia_void hal_reg_vout_set_cfg_src_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_src_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_src_pos.u32));
    val.bits.cfg_src_ypos = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_src_pos.u32)),val.u32);
    return;
}

/********************************************************
 层真实内容在显示窗口的X坐标。  (无区域使能时以此输出背景图)
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_src_xpos(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_src_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_src_pos.u32)));
    return (xmedia_u16)val.bits.cfg_src_xpos;
}

xmedia_void hal_reg_vout_set_cfg_src_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_src_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_src_pos.u32));
    val.bits.cfg_src_xpos = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_src_pos.u32)),val.u32);
    return;
}

/********************************************************
 层真实内容高，像素单位，减1配置。  (无区域使能时以此输出背景图)
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_src_v(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_src val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_src.u32)));
    return (xmedia_u16)val.bits.cfg_src_v;
}

xmedia_void hal_reg_vout_set_cfg_src_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_src val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_src.u32));
    val.bits.cfg_src_v = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_src.u32)),val.u32);
    return;
}

/********************************************************
 层真实内容宽，像素单位，减1配置。  (无区域使能时以此输出背景图)
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_src_h(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_src val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_src.u32)));
    return (xmedia_u16)val.bits.cfg_src_h;
}

xmedia_void hal_reg_vout_set_cfg_src_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_src val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_src.u32));
    val.bits.cfg_src_h = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_src.u32)),val.u32);
    return;
}

/********************************************************
 叠加层优先级 （底层）。 （cfg_mrg1_en 使能有效）
 0：rdma0 ，即  rdma0 上面叠 rdma1。
 1：rdma1 ，即  rdma1 上面叠 rdma0。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_mixer_prio(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_mixer_prio;
}

xmedia_void hal_reg_vout_set_cfg_mixer_prio(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_v0_mrg_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg_ctrl.u32));
    val.bits.cfg_mixer_prio = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 区域1 MUTE 使能；
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_mrg1_mute_en(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_mrg1_mute_en;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_mute_en(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_v0_mrg_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg_ctrl.u32));
    val.bits.cfg_mrg1_mute_en = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 区域0 MUTE 使能；
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_mrg0_mute_en(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_mrg0_mute_en;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_mute_en(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_v0_mrg_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg_ctrl.u32));
    val.bits.cfg_mrg0_mute_en = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 区域1 使能，若使能则绑定rdma1通道；
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_mrg1_en(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_mrg1_en;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_en(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_v0_mrg_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg_ctrl.u32));
    val.bits.cfg_mrg1_en = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 区域0 使能，若使能则绑定rdma0通道；
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_mrg0_en(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_mrg0_en;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_en(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_v0_mrg_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg_ctrl.u32));
    val.bits.cfg_mrg0_en = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 视频区域0 Y分量 DDR 地址，单位Byte，Byte对齐。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_cfg_mrg0_src_addr_y_h(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_addr_y_h val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_addr_y_h.u32)));
    return (xmedia_u32)val.bits.cfg_mrg0_src_addr_y_h;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_src_addr_y_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_v0_mrg0_addr_y_h val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_addr_y_h.u32));
    val.bits.cfg_mrg0_src_addr_y_h = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_addr_y_h.u32)),val.u32);
    return;
}

/********************************************************
 视频区域0 Y分量 DDR 地址，单位Byte，Byte对齐。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_cfg_mrg0_src_addr_y_l(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_addr_y_l val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_addr_y_l.u32)));
    return (xmedia_u32)val.bits.cfg_mrg0_src_addr_y_l;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_src_addr_y_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_v0_mrg0_addr_y_l val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_addr_y_l.u32));
    val.bits.cfg_mrg0_src_addr_y_l = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_addr_y_l.u32)),val.u32);
    return;
}

/********************************************************
 视频区域0 Y分量 DDR 行间距，单位Byte，8Byte对齐。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_cfg_mrg0_src_stride_y(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_stride_y val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_stride_y.u32)));
    return (xmedia_u32)val.bits.cfg_mrg0_src_stride_y;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_src_stride_y(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_v0_mrg0_stride_y val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_stride_y.u32));
    val.bits.cfg_mrg0_src_stride_y = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_stride_y.u32)),val.u32);
    return;
}

/********************************************************
 视频区域0 C分量 DDR 地址，单位Byte，Byte对齐。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_cfg_mrg0_src_addr_c_h(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_addr_c_h val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_addr_c_h.u32)));
    return (xmedia_u32)val.bits.cfg_mrg0_src_addr_c_h;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_src_addr_c_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_v0_mrg0_addr_c_h val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_addr_c_h.u32));
    val.bits.cfg_mrg0_src_addr_c_h = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_addr_c_h.u32)),val.u32);
    return;
}

/********************************************************
 视频区域0 C分量 DDR 地址，单位Byte，Byte对齐。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_cfg_mrg0_src_addr_c_l(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_addr_c_l val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_addr_c_l.u32)));
    return (xmedia_u32)val.bits.cfg_mrg0_src_addr_c_l;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_src_addr_c_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_v0_mrg0_addr_c_l val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_addr_c_l.u32));
    val.bits.cfg_mrg0_src_addr_c_l = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_addr_c_l.u32)),val.u32);
    return;
}

/********************************************************
 视频区域0 C分量 DDR 行间距，单位Byte，8Byte对齐。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_cfg_mrg0_src_stride_c(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_stride_c val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_stride_c.u32)));
    return (xmedia_u32)val.bits.cfg_mrg0_src_stride_c;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_src_stride_c(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_v0_mrg0_stride_c val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_stride_c.u32));
    val.bits.cfg_mrg0_src_stride_c = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_stride_c.u32)),val.u32);
    return;
}

/********************************************************
 视频区域0 在显示窗口的Y坐标。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg0_src_ypos(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_src_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_src_pos.u32)));
    return (xmedia_u16)val.bits.cfg_mrg0_src_ypos;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_src_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg0_src_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_src_pos.u32));
    val.bits.cfg_mrg0_src_ypos = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_src_pos.u32)),val.u32);
    return;
}

/********************************************************
 视频区域0 在显示窗口的X坐标。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg0_src_xpos(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_src_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_src_pos.u32)));
    return (xmedia_u16)val.bits.cfg_mrg0_src_xpos;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_src_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg0_src_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_src_pos.u32));
    val.bits.cfg_mrg0_src_xpos = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_src_pos.u32)),val.u32);
    return;
}

/********************************************************
 视频区域0 高，像素单位，减1配置。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg0_src_v(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_src val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_src.u32)));
    return (xmedia_u16)val.bits.cfg_mrg0_src_v;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_src_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg0_src val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_src.u32));
    val.bits.cfg_mrg0_src_v = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_src.u32)),val.u32);
    return;
}

/********************************************************
 视频区域0 宽，像素单位，减1配置。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg0_src_h(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_src val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_src.u32)));
    return (xmedia_u16)val.bits.cfg_mrg0_src_h;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_src_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg0_src val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_src.u32));
    val.bits.cfg_mrg0_src_h = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_src.u32)),val.u32);
    return;
}

/********************************************************
 视频区域0 mute 颜色。
 棋盘格 白色块 颜色设置。
 颜色格式按照intf输出格式配置
 ********************************************************/
xmedia_u32 hal_reg_vout_get_cfg_mrg0_mute_color(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_mute_color val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_mute_color.u32)));
    return (xmedia_u32)val.bits.cfg_mrg0_mute_color;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_mute_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_v0_mrg0_mute_color val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_mute_color.u32));
    val.bits.cfg_mrg0_mute_color = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_mute_color.u32)),val.u32);
    return;
}

/********************************************************
 视频区域1 Y分量 DDR 地址，单位Byte，Byte对齐。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_cfg_mrg1_src_addr_y_h(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_addr_y_h val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_addr_y_h.u32)));
    return (xmedia_u32)val.bits.cfg_mrg1_src_addr_y_h;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_src_addr_y_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_v0_mrg1_addr_y_h val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_addr_y_h.u32));
    val.bits.cfg_mrg1_src_addr_y_h = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_addr_y_h.u32)),val.u32);
    return;
}

/********************************************************
 视频区域1 Y分量 DDR 地址，单位Byte，Byte对齐。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_cfg_mrg1_src_addr_y_l(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_addr_y_l val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_addr_y_l.u32)));
    return (xmedia_u32)val.bits.cfg_mrg1_src_addr_y_l;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_src_addr_y_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_v0_mrg1_addr_y_l val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_addr_y_l.u32));
    val.bits.cfg_mrg1_src_addr_y_l = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_addr_y_l.u32)),val.u32);
    return;
}

/********************************************************
 视频区域1 Y分量 DDR 行间距，单位Byte，8Byte对齐。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_cfg_mrg1_src_stride_y(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_stride_y val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_stride_y.u32)));
    return (xmedia_u32)val.bits.cfg_mrg1_src_stride_y;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_src_stride_y(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_v0_mrg1_stride_y val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_stride_y.u32));
    val.bits.cfg_mrg1_src_stride_y = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_stride_y.u32)),val.u32);
    return;
}

/********************************************************
 视频区域1 C分量 DDR 地址，单位Byte，Byte对齐。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_cfg_mrg1_src_addr_c_h(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_addr_c_h val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_addr_c_h.u32)));
    return (xmedia_u32)val.bits.cfg_mrg1_src_addr_c_h;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_src_addr_c_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_v0_mrg1_addr_c_h val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_addr_c_h.u32));
    val.bits.cfg_mrg1_src_addr_c_h = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_addr_c_h.u32)),val.u32);
    return;
}

/********************************************************
 视频区域1 C分量 DDR 地址，单位Byte，Byte对齐。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_cfg_mrg1_src_addr_c_l(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_addr_c_l val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_addr_c_l.u32)));
    return (xmedia_u32)val.bits.cfg_mrg1_src_addr_c_l;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_src_addr_c_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_v0_mrg1_addr_c_l val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_addr_c_l.u32));
    val.bits.cfg_mrg1_src_addr_c_l = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_addr_c_l.u32)),val.u32);
    return;
}

/********************************************************
 视频区域1 C分量 DDR 行间距，单位Byte，8Byte对齐。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_cfg_mrg1_src_stride_c(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_stride_c val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_stride_c.u32)));
    return (xmedia_u32)val.bits.cfg_mrg1_src_stride_c;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_src_stride_c(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_v0_mrg1_stride_c val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_stride_c.u32));
    val.bits.cfg_mrg1_src_stride_c = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_stride_c.u32)),val.u32);
    return;
}

/********************************************************
 视频区域1 在显示窗口的Y坐标。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg1_src_ypos(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_src_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_src_pos.u32)));
    return (xmedia_u16)val.bits.cfg_mrg1_src_ypos;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_src_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg1_src_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_src_pos.u32));
    val.bits.cfg_mrg1_src_ypos = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_src_pos.u32)),val.u32);
    return;
}

/********************************************************
 视频区域1 在显示窗口的X坐标。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg1_src_xpos(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_src_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_src_pos.u32)));
    return (xmedia_u16)val.bits.cfg_mrg1_src_xpos;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_src_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg1_src_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_src_pos.u32));
    val.bits.cfg_mrg1_src_xpos = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_src_pos.u32)),val.u32);
    return;
}

/********************************************************
 视频区域1 高，像素单位，减1配置。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg1_src_v(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_src val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_src.u32)));
    return (xmedia_u16)val.bits.cfg_mrg1_src_v;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_src_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg1_src val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_src.u32));
    val.bits.cfg_mrg1_src_v = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_src.u32)),val.u32);
    return;
}

/********************************************************
 视频区域1 宽，像素单位，减1配置。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg1_src_h(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_src val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_src.u32)));
    return (xmedia_u16)val.bits.cfg_mrg1_src_h;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_src_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg1_src val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_src.u32));
    val.bits.cfg_mrg1_src_h = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_src.u32)),val.u32);
    return;
}

/********************************************************
 视频区域1 mute 颜色。
 棋盘格 白色块 颜色设置。
 颜色格式按照intf输出格式配置
 ********************************************************/
xmedia_u32 hal_reg_vout_get_cfg_mrg1_mute_color(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_mute_color val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_mute_color.u32)));
    return (xmedia_u32)val.bits.cfg_mrg1_mute_color;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_mute_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_v0_mrg1_mute_color val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_mute_color.u32));
    val.bits.cfg_mrg1_mute_color = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_mute_color.u32)),val.u32);
    return;
}

/********************************************************
 奇数行垂直上采样系数。
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_vcus_coef_1(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_vcus_coef val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_vcus_coef.u32)));
    return (xmedia_u8)val.bits.cfg_vcus_coef_1;
}

xmedia_void hal_reg_vout_set_cfg_vcus_coef_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_v0_vcus_coef val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_vcus_coef.u32));
    val.bits.cfg_vcus_coef_1 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_vcus_coef.u32)),val.u32);
    return;
}

/********************************************************
 偶数行垂直上采样系数。
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_vcus_coef_0(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_vcus_coef val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_vcus_coef.u32)));
    return (xmedia_u8)val.bits.cfg_vcus_coef_0;
}

xmedia_void hal_reg_vout_set_cfg_vcus_coef_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_v0_vcus_coef val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_vcus_coef.u32));
    val.bits.cfg_vcus_coef_0 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_vcus_coef.u32)),val.u32);
    return;
}

/********************************************************
 奇数列垂直上采样系数。
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_hcus_coef_1(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_hcus_coef val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_hcus_coef.u32)));
    return (xmedia_u8)val.bits.cfg_hcus_coef_1;
}

xmedia_void hal_reg_vout_set_cfg_hcus_coef_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_v0_hcus_coef val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_hcus_coef.u32));
    val.bits.cfg_hcus_coef_1 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_hcus_coef.u32)),val.u32);
    return;
}

/********************************************************
 偶数列垂直上采样系数。
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_hcus_coef_0(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_hcus_coef val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_hcus_coef.u32)));
    return (xmedia_u8)val.bits.cfg_hcus_coef_0;
}

xmedia_void hal_reg_vout_set_cfg_hcus_coef_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_v0_hcus_coef val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_hcus_coef.u32));
    val.bits.cfg_hcus_coef_0 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_hcus_coef.u32)),val.u32);
    return;
}

/********************************************************
 色域色调饱和度对比度调整使能。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_csc_en(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_csc_offset_in val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_offset_in.u32)));
    return (xmedia_bool)val.bits.cfg_csc_en;
}

xmedia_void hal_reg_vout_set_cfg_csc_en(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_v0_mrg0_csc_offset_in val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_csc_offset_in.u32));
    val.bits.cfg_csc_en = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_offset_in.u32)),val.u32);
    return;
}

/********************************************************
 CSC模式
 0:输入格式是RGB
 1:输入格式是YUV
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_csc_mode(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_csc_offset_in val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_offset_in.u32)));
    return (xmedia_bool)val.bits.cfg_csc_mode;
}

xmedia_void hal_reg_vout_set_cfg_csc_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_v0_mrg0_csc_offset_in val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_csc_offset_in.u32));
    val.bits.cfg_csc_mode = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_offset_in.u32)),val.u32);
    return;
}

/********************************************************
 V/B 分量输入偏置，有符号数。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_offset_in_2(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_csc_offset_in val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_offset_in.u32)));
    return (xmedia_u16)val.bits.cfg_mrg0_csc_offset_in_2;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_csc_offset_in_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg0_csc_offset_in val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_csc_offset_in.u32));
    val.bits.cfg_mrg0_csc_offset_in_2 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_offset_in.u32)),val.u32);
    return;
}

/********************************************************
 U/G 分量输入偏置，有符号数。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_offset_in_1(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_csc_offset_in val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_offset_in.u32)));
    return (xmedia_u16)val.bits.cfg_mrg0_csc_offset_in_1;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_csc_offset_in_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg0_csc_offset_in val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_csc_offset_in.u32));
    val.bits.cfg_mrg0_csc_offset_in_1 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_offset_in.u32)),val.u32);
    return;
}

/********************************************************
 Y/R  分量输入偏置，有符号数。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_offset_in_0(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_csc_offset_in val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_offset_in.u32)));
    return (xmedia_u16)val.bits.cfg_mrg0_csc_offset_in_0;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_csc_offset_in_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg0_csc_offset_in val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_csc_offset_in.u32));
    val.bits.cfg_mrg0_csc_offset_in_0 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_offset_in.u32)),val.u32);
    return;
}

/********************************************************
 转换矩阵，有符号数(s1+5+10)。U/G
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_matrix_1(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_csc_matrix_01 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_matrix_01.u32)));
    return (xmedia_u16)val.bits.cfg_mrg0_csc_matrix_1;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_csc_matrix_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg0_csc_matrix_01 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_csc_matrix_01.u32));
    val.bits.cfg_mrg0_csc_matrix_1 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_matrix_01.u32)),val.u32);
    return;
}

/********************************************************
 转换矩阵，有符号数。Y/R
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_matrix_0(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_csc_matrix_01 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_matrix_01.u32)));
    return (xmedia_u16)val.bits.cfg_mrg0_csc_matrix_0;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_csc_matrix_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg0_csc_matrix_01 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_csc_matrix_01.u32));
    val.bits.cfg_mrg0_csc_matrix_0 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_matrix_01.u32)),val.u32);
    return;
}

/********************************************************
 转换矩阵，有符号数。Y/R
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_matrix_3(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_csc_matrix_23 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_matrix_23.u32)));
    return (xmedia_u16)val.bits.cfg_mrg0_csc_matrix_3;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_csc_matrix_3(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg0_csc_matrix_23 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_csc_matrix_23.u32));
    val.bits.cfg_mrg0_csc_matrix_3 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_matrix_23.u32)),val.u32);
    return;
}

/********************************************************
 转换矩阵，有符号数。V/B
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_matrix_2(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_csc_matrix_23 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_matrix_23.u32)));
    return (xmedia_u16)val.bits.cfg_mrg0_csc_matrix_2;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_csc_matrix_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg0_csc_matrix_23 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_csc_matrix_23.u32));
    val.bits.cfg_mrg0_csc_matrix_2 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_matrix_23.u32)),val.u32);
    return;
}

/********************************************************
 转换矩阵，有符号数。V/B
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_matrix_5(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_csc_matrix_45 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_matrix_45.u32)));
    return (xmedia_u16)val.bits.cfg_mrg0_csc_matrix_5;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_csc_matrix_5(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg0_csc_matrix_45 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_csc_matrix_45.u32));
    val.bits.cfg_mrg0_csc_matrix_5 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_matrix_45.u32)),val.u32);
    return;
}

/********************************************************
 转换矩阵，有符号数。U/G
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_matrix_4(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_csc_matrix_45 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_matrix_45.u32)));
    return (xmedia_u16)val.bits.cfg_mrg0_csc_matrix_4;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_csc_matrix_4(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg0_csc_matrix_45 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_csc_matrix_45.u32));
    val.bits.cfg_mrg0_csc_matrix_4 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_matrix_45.u32)),val.u32);
    return;
}

/********************************************************
 转换矩阵，有符号数。U/G
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_matrix_7(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_csc_matrix_67 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_matrix_67.u32)));
    return (xmedia_u16)val.bits.cfg_mrg0_csc_matrix_7;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_csc_matrix_7(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg0_csc_matrix_67 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_csc_matrix_67.u32));
    val.bits.cfg_mrg0_csc_matrix_7 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_matrix_67.u32)),val.u32);
    return;
}

/********************************************************
 转换矩阵，有符号数。Y/R
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_matrix_6(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_csc_matrix_67 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_matrix_67.u32)));
    return (xmedia_u16)val.bits.cfg_mrg0_csc_matrix_6;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_csc_matrix_6(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg0_csc_matrix_67 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_csc_matrix_67.u32));
    val.bits.cfg_mrg0_csc_matrix_6 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_matrix_67.u32)),val.u32);
    return;
}

/********************************************************
 转换矩阵，有符号数。V/B
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_matrix_8(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_csc_matrix_8 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_matrix_8.u32)));
    return (xmedia_u16)val.bits.cfg_mrg0_csc_matrix_8;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_csc_matrix_8(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg0_csc_matrix_8 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_csc_matrix_8.u32));
    val.bits.cfg_mrg0_csc_matrix_8 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_matrix_8.u32)),val.u32);
    return;
}

/********************************************************
 V/B 分量输出偏置，有符号数。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_offset_out_2(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_csc_offset_out val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_offset_out.u32)));
    return (xmedia_u16)val.bits.cfg_mrg0_csc_offset_out_2;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_csc_offset_out_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg0_csc_offset_out val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_csc_offset_out.u32));
    val.bits.cfg_mrg0_csc_offset_out_2 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_offset_out.u32)),val.u32);
    return;
}

/********************************************************
 U/G 分量输出偏置，有符号数。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_offset_out_1(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_csc_offset_out val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_offset_out.u32)));
    return (xmedia_u16)val.bits.cfg_mrg0_csc_offset_out_1;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_csc_offset_out_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg0_csc_offset_out val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_csc_offset_out.u32));
    val.bits.cfg_mrg0_csc_offset_out_1 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_offset_out.u32)),val.u32);
    return;
}

/********************************************************
 Y/R  分量输出偏置，有符号数。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg0_csc_offset_out_0(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_csc_offset_out val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_offset_out.u32)));
    return (xmedia_u16)val.bits.cfg_mrg0_csc_offset_out_0;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_csc_offset_out_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg0_csc_offset_out val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_csc_offset_out.u32));
    val.bits.cfg_mrg0_csc_offset_out_0 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_csc_offset_out.u32)),val.u32);
    return;
}

/********************************************************
 V/B 分量输入偏置，有符号数。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_offset_in_2(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_csc_offset_in val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_offset_in.u32)));
    return (xmedia_u16)val.bits.cfg_mrg1_csc_offset_in_2;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_csc_offset_in_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg1_csc_offset_in val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_csc_offset_in.u32));
    val.bits.cfg_mrg1_csc_offset_in_2 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_offset_in.u32)),val.u32);
    return;
}

/********************************************************
 U/G 分量输入偏置，有符号数。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_offset_in_1(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_csc_offset_in val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_offset_in.u32)));
    return (xmedia_u16)val.bits.cfg_mrg1_csc_offset_in_1;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_csc_offset_in_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg1_csc_offset_in val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_csc_offset_in.u32));
    val.bits.cfg_mrg1_csc_offset_in_1 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_offset_in.u32)),val.u32);
    return;
}

/********************************************************
 Y/R  分量输入偏置，有符号数。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_offset_in_0(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_csc_offset_in val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_offset_in.u32)));
    return (xmedia_u16)val.bits.cfg_mrg1_csc_offset_in_0;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_csc_offset_in_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg1_csc_offset_in val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_csc_offset_in.u32));
    val.bits.cfg_mrg1_csc_offset_in_0 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_offset_in.u32)),val.u32);
    return;
}

/********************************************************
 转换矩阵，有符号数(s1+5+10)。U/G
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_matrix_1(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_csc_matrix_01 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_matrix_01.u32)));
    return (xmedia_u16)val.bits.cfg_mrg1_csc_matrix_1;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_csc_matrix_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg1_csc_matrix_01 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_csc_matrix_01.u32));
    val.bits.cfg_mrg1_csc_matrix_1 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_matrix_01.u32)),val.u32);
    return;
}

/********************************************************
 转换矩阵，有符号数。Y/R
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_matrix_0(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_csc_matrix_01 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_matrix_01.u32)));
    return (xmedia_u16)val.bits.cfg_mrg1_csc_matrix_0;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_csc_matrix_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg1_csc_matrix_01 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_csc_matrix_01.u32));
    val.bits.cfg_mrg1_csc_matrix_0 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_matrix_01.u32)),val.u32);
    return;
}

/********************************************************
 转换矩阵，有符号数。Y/R
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_matrix_3(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_csc_matrix_23 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_matrix_23.u32)));
    return (xmedia_u16)val.bits.cfg_mrg1_csc_matrix_3;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_csc_matrix_3(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg1_csc_matrix_23 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_csc_matrix_23.u32));
    val.bits.cfg_mrg1_csc_matrix_3 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_matrix_23.u32)),val.u32);
    return;
}

/********************************************************
 转换矩阵，有符号数。V/B
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_matrix_2(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_csc_matrix_23 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_matrix_23.u32)));
    return (xmedia_u16)val.bits.cfg_mrg1_csc_matrix_2;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_csc_matrix_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg1_csc_matrix_23 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_csc_matrix_23.u32));
    val.bits.cfg_mrg1_csc_matrix_2 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_matrix_23.u32)),val.u32);
    return;
}

/********************************************************
 转换矩阵，有符号数。V/B
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_matrix_5(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_csc_matrix_45 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_matrix_45.u32)));
    return (xmedia_u16)val.bits.cfg_mrg1_csc_matrix_5;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_csc_matrix_5(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg1_csc_matrix_45 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_csc_matrix_45.u32));
    val.bits.cfg_mrg1_csc_matrix_5 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_matrix_45.u32)),val.u32);
    return;
}

/********************************************************
 转换矩阵，有符号数。U/G
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_matrix_4(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_csc_matrix_45 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_matrix_45.u32)));
    return (xmedia_u16)val.bits.cfg_mrg1_csc_matrix_4;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_csc_matrix_4(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg1_csc_matrix_45 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_csc_matrix_45.u32));
    val.bits.cfg_mrg1_csc_matrix_4 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_matrix_45.u32)),val.u32);
    return;
}

/********************************************************
 转换矩阵，有符号数。U/G
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_matrix_7(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_csc_matrix_67 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_matrix_67.u32)));
    return (xmedia_u16)val.bits.cfg_mrg1_csc_matrix_7;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_csc_matrix_7(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg1_csc_matrix_67 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_csc_matrix_67.u32));
    val.bits.cfg_mrg1_csc_matrix_7 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_matrix_67.u32)),val.u32);
    return;
}

/********************************************************
 转换矩阵，有符号数。Y/R
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_matrix_6(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_csc_matrix_67 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_matrix_67.u32)));
    return (xmedia_u16)val.bits.cfg_mrg1_csc_matrix_6;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_csc_matrix_6(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg1_csc_matrix_67 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_csc_matrix_67.u32));
    val.bits.cfg_mrg1_csc_matrix_6 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_matrix_67.u32)),val.u32);
    return;
}

/********************************************************
 转换矩阵，有符号数。V/B
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_matrix_8(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_csc_matrix_8 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_matrix_8.u32)));
    return (xmedia_u16)val.bits.cfg_mrg1_csc_matrix_8;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_csc_matrix_8(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg1_csc_matrix_8 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_csc_matrix_8.u32));
    val.bits.cfg_mrg1_csc_matrix_8 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_matrix_8.u32)),val.u32);
    return;
}

/********************************************************
 V/B 分量输出偏置，有符号数。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_offset_out_2(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_csc_offset_out val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_offset_out.u32)));
    return (xmedia_u16)val.bits.cfg_mrg1_csc_offset_out_2;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_csc_offset_out_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg1_csc_offset_out val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_csc_offset_out.u32));
    val.bits.cfg_mrg1_csc_offset_out_2 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_offset_out.u32)),val.u32);
    return;
}

/********************************************************
 U/G 分量输出偏置，有符号数。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_offset_out_1(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_csc_offset_out val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_offset_out.u32)));
    return (xmedia_u16)val.bits.cfg_mrg1_csc_offset_out_1;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_csc_offset_out_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg1_csc_offset_out val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_csc_offset_out.u32));
    val.bits.cfg_mrg1_csc_offset_out_1 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_offset_out.u32)),val.u32);
    return;
}

/********************************************************
 Y/R  分量输出偏置，有符号数。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg1_csc_offset_out_0(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_csc_offset_out val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_offset_out.u32)));
    return (xmedia_u16)val.bits.cfg_mrg1_csc_offset_out_0;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_csc_offset_out_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg1_csc_offset_out val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_csc_offset_out.u32));
    val.bits.cfg_mrg1_csc_offset_out_0 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_csc_offset_out.u32)),val.u32);
    return;
}

/********************************************************
 区域0的边框颜色。
 颜色格式按照intf输出格式配置
 ********************************************************/
xmedia_u32 hal_reg_vout_get_cfg_lba0_color(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_lba0 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_lba0.u32)));
    return (xmedia_u32)val.bits.cfg_lba0_color;
}

xmedia_void hal_reg_vout_set_cfg_lba0_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_v0_lba0 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_lba0.u32));
    val.bits.cfg_lba0_color = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_lba0.u32)),val.u32);
    return;
}

/********************************************************
 区域1的边框颜色。
 颜色格式按照intf输出格式配置
 ********************************************************/
xmedia_u32 hal_reg_vout_get_cfg_lba1_color(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_lba1 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_lba1.u32)));
    return (xmedia_u32)val.bits.cfg_lba1_color;
}

xmedia_void hal_reg_vout_set_cfg_lba1_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_v0_lba1 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_lba1.u32));
    val.bits.cfg_lba1_color = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_lba1.u32)),val.u32);
    return;
}

/********************************************************
 区域0在显示窗口的Y坐标。 （添加边框使用）
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg0_disp_ypos(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_disp_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_disp_pos.u32)));
    return (xmedia_u16)val.bits.cfg_mrg0_disp_ypos;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_disp_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg0_disp_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_disp_pos.u32));
    val.bits.cfg_mrg0_disp_ypos = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_disp_pos.u32)),val.u32);
    return;
}

/********************************************************
 区域0在显示窗口的X坐标。 （添加边框使用）
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg0_disp_xpos(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_disp_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_disp_pos.u32)));
    return (xmedia_u16)val.bits.cfg_mrg0_disp_xpos;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_disp_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg0_disp_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_disp_pos.u32));
    val.bits.cfg_mrg0_disp_xpos = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_disp_pos.u32)),val.u32);
    return;
}

/********************************************************
 区域0高，像素单位，减1配置。 （添加边框使用）
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg0_disp_v(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_disp val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_disp.u32)));
    return (xmedia_u16)val.bits.cfg_mrg0_disp_v;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_disp_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg0_disp val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_disp.u32));
    val.bits.cfg_mrg0_disp_v = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_disp.u32)),val.u32);
    return;
}

/********************************************************
 区域0宽，像素单位，减1配置。（添加边框使用）
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg0_disp_h(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg0_disp val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_disp.u32)));
    return (xmedia_u16)val.bits.cfg_mrg0_disp_h;
}

xmedia_void hal_reg_vout_set_cfg_mrg0_disp_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg0_disp val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg0_disp.u32));
    val.bits.cfg_mrg0_disp_h = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg0_disp.u32)),val.u32);
    return;
}

/********************************************************
 区域1在显示窗口的Y坐标。 （添加边框使用）
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg1_disp_ypos(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_disp_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_disp_pos.u32)));
    return (xmedia_u16)val.bits.cfg_mrg1_disp_ypos;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_disp_ypos(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg1_disp_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_disp_pos.u32));
    val.bits.cfg_mrg1_disp_ypos = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_disp_pos.u32)),val.u32);
    return;
}

/********************************************************
 区域1在显示窗口的X坐标。 （添加边框使用）
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg1_disp_xpos(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_disp_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_disp_pos.u32)));
    return (xmedia_u16)val.bits.cfg_mrg1_disp_xpos;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_disp_xpos(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg1_disp_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_disp_pos.u32));
    val.bits.cfg_mrg1_disp_xpos = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_disp_pos.u32)),val.u32);
    return;
}

/********************************************************
 区域1高，像素单位，减1配置。 （添加边框使用）
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg1_disp_v(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_disp val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_disp.u32)));
    return (xmedia_u16)val.bits.cfg_mrg1_disp_v;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_disp_v(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg1_disp val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_disp.u32));
    val.bits.cfg_mrg1_disp_v = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_disp.u32)),val.u32);
    return;
}

/********************************************************
 区域1宽，像素单位，减1配置。（添加边框使用）
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_mrg1_disp_h(s_vout_regs_type *p_vout_reg)
{
    u_vout_v0_mrg1_disp val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_disp.u32)));
    return (xmedia_u16)val.bits.cfg_mrg1_disp_h;
}

xmedia_void hal_reg_vout_set_cfg_mrg1_disp_h(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_v0_mrg1_disp val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_v0_mrg1_disp.u32));
    val.bits.cfg_mrg1_disp_h = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_v0_mrg1_disp.u32)),val.u32);
    return;
}

/********************************************************
 接口检验和使能。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_chk_sum_en(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_chk_sum_en;
}

xmedia_void hal_reg_vout_set_cfg_chk_sum_en(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_ctrl.u32));
    val.bits.cfg_chk_sum_en = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 hsync 模式。(mipi 场景使用，TBD)
 0：vsync上升沿和下降沿对应的hsync都有效。
 1：vsync上升沿和下降沿对应的hsync无效。
 2：vsync 上降沿对应的hsync无效。
 3：vsync下降沿对应的hsync无效。
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_hs_mode(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ctrl.u32)));
    return (xmedia_u8)val.bits.cfg_hs_mode;
}

xmedia_void hal_reg_vout_set_cfg_hs_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_intf_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_ctrl.u32));
    val.bits.cfg_hs_mode = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 输出接口 de 信号取反。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_de_inv(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_de_inv;
}

xmedia_void hal_reg_vout_set_cfg_de_inv(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_ctrl.u32));
    val.bits.cfg_de_inv = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 输出接口 hs 信号取反。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_hs_inv(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_hs_inv;
}

xmedia_void hal_reg_vout_set_cfg_hs_inv(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_ctrl.u32));
    val.bits.cfg_hs_inv = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 输出接口 vs 信号取反。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_vs_inv(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_vs_inv;
}

xmedia_void hal_reg_vout_set_cfg_vs_inv(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_ctrl.u32));
    val.bits.cfg_vs_inv = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 输出接口 field 信号取反。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_field_inv(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_field_inv;
}

xmedia_void hal_reg_vout_set_cfg_field_inv(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_ctrl.u32));
    val.bits.cfg_field_inv = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 BT/LCD-RGB 使能随路时钟取反。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_clock_edge_sel(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_clock_edge_sel;
}

xmedia_void hal_reg_vout_set_cfg_clock_edge_sel(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_ctrl.u32));
    val.bits.cfg_clock_edge_sel = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 隔行显示使能。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_interlaced(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_interlaced;
}

xmedia_void hal_reg_vout_set_cfg_interlaced(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_ctrl.u32));
    val.bits.cfg_interlaced = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 0：BT 656 /BT1120
 1：LCD
 4：MCU屏接口
 其余：保留。
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_intf_sel(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ctrl.u32)));
    return (xmedia_u8)val.bits.cfg_intf_sel;
}

xmedia_void hal_reg_vout_set_cfg_intf_sel(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_intf_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_ctrl.u32));
    val.bits.cfg_intf_sel = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 接口使能。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_intf_en(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_intf_en;
}

xmedia_void hal_reg_vout_set_cfg_intf_en(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_ctrl.u32));
    val.bits.cfg_intf_en = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ctrl.u32)),val.u32);
    return;
}

xmedia_void hal_reg_vout_set_intfupd_cfg_regup_en(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_upd val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_upd.u32));
    val.bits.cfg_regup_en = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_upd.u32)),val.u32);
    return;
}

/********************************************************
 系数更新点配置，从进入消隐区开始计数，单位行。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_para_pos(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_regup_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_regup_pos.u32)));
    return (xmedia_u16)val.bits.cfg_para_pos;
}

xmedia_void hal_reg_vout_set_cfg_para_pos(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_regup_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_regup_pos.u32));
    val.bits.cfg_para_pos = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_regup_pos.u32)),val.u32);
    return;
}

/********************************************************
 寄存器更新点配置，从进入消隐区开始计数，单位行。
 约束 cfg_regup_pos < cfg_regup_pos < 
 cfg_prefetch_pos。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_regup_pos(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_regup_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_regup_pos.u32)));
    return (xmedia_u16)val.bits.cfg_regup_pos;
}

xmedia_void hal_reg_vout_set_cfg_regup_pos(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_regup_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_regup_pos.u32));
    val.bits.cfg_regup_pos = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_regup_pos.u32)),val.u32);
    return;
}

/********************************************************
 通路数据流帧启动点配置，从进入消隐区开始计数，单位行。
 低延时场景，从进入VBB 开始计数；
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_start_pos(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_start_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_start_pos.u32)));
    return (xmedia_u16)val.bits.cfg_start_pos;
}

xmedia_void hal_reg_vout_set_cfg_start_pos(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_start_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_start_pos.u32));
    val.bits.cfg_start_pos = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_start_pos.u32)),val.u32);
    return;
}

/********************************************************
 DMA 启动数据申请配置，从进入消隐区开始计数，单位行。
 低延时场景，从进入VBB 开始计数；
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_prefetch_pos(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_start_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_start_pos.u32)));
    return (xmedia_u16)val.bits.cfg_prefetch_pos;
}

xmedia_void hal_reg_vout_set_cfg_prefetch_pos(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_start_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_start_pos.u32));
    val.bits.cfg_prefetch_pos = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_start_pos.u32)),val.u32);
    return;
}

/********************************************************
 显示窗口 H有效区，减1配置。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_hact(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_act val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_act.u32)));
    return (xmedia_u16)val.bits.cfg_hact;
}

xmedia_void hal_reg_vout_set_cfg_hact(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_act val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_act.u32));
    val.bits.cfg_hact = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_act.u32)),val.u32);
    return;
}

/********************************************************
 显示窗口 V有效区，减1配置。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_vact(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_act val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_act.u32)));
    return (xmedia_u16)val.bits.cfg_vact;
}

xmedia_void hal_reg_vout_set_cfg_vact(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_act val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_act.u32));
    val.bits.cfg_vact = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_act.u32)),val.u32);
    return;
}

/********************************************************
 显示窗口 V后消隐（逐行/隔行顶场），减1配置。
 低延时场景，vbb 配置最小9；
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_vbb(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_vfb val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vfb.u32)));
    return (xmedia_u16)val.bits.cfg_vbb;
}

xmedia_void hal_reg_vout_set_cfg_vbb(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_vfb val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_vfb.u32));
    val.bits.cfg_vbb = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vfb.u32)),val.u32);
    return;
}

/********************************************************
 显示窗口 V前消隐（逐行/隔行顶场），减1配置。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_vfb(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_vfb val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vfb.u32)));
    return (xmedia_u16)val.bits.cfg_vfb;
}

xmedia_void hal_reg_vout_set_cfg_vfb(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_vfb val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_vfb.u32));
    val.bits.cfg_vfb = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vfb.u32)),val.u32);
    return;
}

/********************************************************
 显示窗口 V后消隐（隔行底场），减1配置。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_bvbb(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_bvfb val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bvfb.u32)));
    return (xmedia_u16)val.bits.cfg_bvbb;
}

xmedia_void hal_reg_vout_set_cfg_bvbb(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_bvfb val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_bvfb.u32));
    val.bits.cfg_bvbb = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bvfb.u32)),val.u32);
    return;
}

/********************************************************
 显示窗口 V前消隐（隔行底场），减1配置。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_bvfb(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_bvfb val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bvfb.u32)));
    return (xmedia_u16)val.bits.cfg_bvfb;
}

xmedia_void hal_reg_vout_set_cfg_bvfb(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_bvfb val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_bvfb.u32));
    val.bits.cfg_bvfb = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bvfb.u32)),val.u32);
    return;
}

/********************************************************
 显示窗口 H后消隐，减1配置。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_hbb(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_hfb val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_hfb.u32)));
    return (xmedia_u16)val.bits.cfg_hbb;
}

xmedia_void hal_reg_vout_set_cfg_hbb(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_hfb val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_hfb.u32));
    val.bits.cfg_hbb = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_hfb.u32)),val.u32);
    return;
}

/********************************************************
 显示窗口 H前消隐，减1配置。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_hfb(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_hfb val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_hfb.u32)));
    return (xmedia_u16)val.bits.cfg_hfb;
}

xmedia_void hal_reg_vout_set_cfg_hfb(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_hfb val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_hfb.u32));
    val.bits.cfg_hfb = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_hfb.u32)),val.u32);
    return;
}

/********************************************************
 输出接口HS 宽度，单位像素。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_hs_width(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_hs_w val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_hs_w.u32)));
    return (xmedia_u16)val.bits.cfg_hs_width;
}

xmedia_void hal_reg_vout_set_cfg_hs_width(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_hs_w val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_hs_w.u32));
    val.bits.cfg_hs_width = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_hs_w.u32)),val.u32);
    return;
}

/********************************************************
 显示窗口 field 信号翻转点，
 从进入消隐区开始计数，单位行（隔行底场）。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_bfield_tog(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_field val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_field.u32)));
    return (xmedia_u16)val.bits.cfg_bfield_tog;
}

xmedia_void hal_reg_vout_set_cfg_bfield_tog(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_field val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_field.u32));
    val.bits.cfg_bfield_tog = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_field.u32)),val.u32);
    return;
}

/********************************************************
 显示窗口 field 信号翻转点，
 从进入消隐区开始计数，单位行（隔行顶场）。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_field_tog(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_field val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_field.u32)));
    return (xmedia_u16)val.bits.cfg_field_tog;
}

xmedia_void hal_reg_vout_set_cfg_field_tog(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_field val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_field.u32));
    val.bits.cfg_field_tog = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_field.u32)),val.u32);
    return;
}

/********************************************************
 显示窗口 VS 信号 H 结束点，
 从进入消隐区开始计数，单位像素（逐行/隔行顶场）。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_vs_end_h_field0(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_vs_h val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vs_h.u32)));
    return (xmedia_u16)val.bits.cfg_vs_end_h_field0;
}

xmedia_void hal_reg_vout_set_cfg_vs_end_h_field0(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_vs_h val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_vs_h.u32));
    val.bits.cfg_vs_end_h_field0 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vs_h.u32)),val.u32);
    return;
}

/********************************************************
 显示窗口 VS 信号 H 起始点，
 从进入消隐区开始计数，单位像素（逐行/隔行顶场）。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_vs_st_h_field0(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_vs_h val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vs_h.u32)));
    return (xmedia_u16)val.bits.cfg_vs_st_h_field0;
}

xmedia_void hal_reg_vout_set_cfg_vs_st_h_field0(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_vs_h val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_vs_h.u32));
    val.bits.cfg_vs_st_h_field0 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vs_h.u32)),val.u32);
    return;
}

/********************************************************
 显示窗口 VS 信号 V 结束点，
 从进入消隐区开始计数，单位行（逐行/隔行顶场）。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_vs_end_v_field0(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_vs_v val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vs_v.u32)));
    return (xmedia_u16)val.bits.cfg_vs_end_v_field0;
}

xmedia_void hal_reg_vout_set_cfg_vs_end_v_field0(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_vs_v val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_vs_v.u32));
    val.bits.cfg_vs_end_v_field0 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vs_v.u32)),val.u32);
    return;
}

/********************************************************
 显示窗口 VS 信号 V 起始点，
 从进入消隐区开始计数，单位行（逐行/隔行顶场）。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_vs_st_v_field0(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_vs_v val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vs_v.u32)));
    return (xmedia_u16)val.bits.cfg_vs_st_v_field0;
}

xmedia_void hal_reg_vout_set_cfg_vs_st_v_field0(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_vs_v val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_vs_v.u32));
    val.bits.cfg_vs_st_v_field0 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vs_v.u32)),val.u32);
    return;
}

/********************************************************
 显示窗口 VS 信号 H 结束点，
 从进入消隐区开始计数，单位像素（隔行底场）。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_vs_end_h_field1(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_bvs_h val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bvs_h.u32)));
    return (xmedia_u16)val.bits.cfg_vs_end_h_field1;
}

xmedia_void hal_reg_vout_set_cfg_vs_end_h_field1(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_bvs_h val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_bvs_h.u32));
    val.bits.cfg_vs_end_h_field1 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bvs_h.u32)),val.u32);
    return;
}

/********************************************************
 显示窗口 VS 信号 H 起始点，
 从进入消隐区开始计数，单位像素（隔行底场）。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_vs_st_h_field1(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_bvs_h val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bvs_h.u32)));
    return (xmedia_u16)val.bits.cfg_vs_st_h_field1;
}

xmedia_void hal_reg_vout_set_cfg_vs_st_h_field1(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_bvs_h val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_bvs_h.u32));
    val.bits.cfg_vs_st_h_field1 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bvs_h.u32)),val.u32);
    return;
}

/********************************************************
 显示窗口 VS 信号 V 结束点，
 从进入消隐区开始计数，单位行（隔行底场）。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_vs_end_v_field1(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_bvs_v val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bvs_v.u32)));
    return (xmedia_u16)val.bits.cfg_vs_end_v_field1;
}

xmedia_void hal_reg_vout_set_cfg_vs_end_v_field1(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_bvs_v val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_bvs_v.u32));
    val.bits.cfg_vs_end_v_field1 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bvs_v.u32)),val.u32);
    return;
}

/********************************************************
 显示窗口 VS 信号 V 起始点，
 从进入消隐区开始计数，单位行（隔行底场）。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_vs_st_v_field1(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_bvs_v val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bvs_v.u32)));
    return (xmedia_u16)val.bits.cfg_vs_st_v_field1;
}

xmedia_void hal_reg_vout_set_cfg_vs_st_v_field1(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_bvs_v val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_bvs_v.u32));
    val.bits.cfg_vs_st_v_field1 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bvs_v.u32)),val.u32);
    return;
}

/********************************************************
 接口同步FIFO 深度。
 V0层 ：最大值959， 典型值 cfg_hact/2。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_intf_sfifo_thd(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_sfifo_thd val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_sfifo_thd.u32)));
    return (xmedia_u16)val.bits.cfg_intf_sfifo_thd;
}

xmedia_void hal_reg_vout_set_cfg_intf_sfifo_thd(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_sfifo_thd val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_sfifo_thd.u32));
    val.bits.cfg_intf_sfifo_thd = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_sfifo_thd.u32)),val.u32);
    return;
}

/********************************************************
 接口垂直时序中断计数器起始行配置。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_vt_int_cnt_sel_3(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_vt_int val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vt_int.u32)));
    return (xmedia_bool)val.bits.cfg_vt_int_cnt_sel_3;
}

xmedia_void hal_reg_vout_set_cfg_vt_int_cnt_sel_3(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_vt_int val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_vt_int.u32));
    val.bits.cfg_vt_int_cnt_sel_3 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vt_int.u32)),val.u32);
    return;
}

/********************************************************
 接口垂直时序中断计数器起始行配置。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_vt_int_cnt_sel_2(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_vt_int val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vt_int.u32)));
    return (xmedia_bool)val.bits.cfg_vt_int_cnt_sel_2;
}

xmedia_void hal_reg_vout_set_cfg_vt_int_cnt_sel_2(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_vt_int val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_vt_int.u32));
    val.bits.cfg_vt_int_cnt_sel_2 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vt_int.u32)),val.u32);
    return;
}

/********************************************************
 接口垂直时序中断计数器起始行配置。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_vt_int_cnt_sel_1(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_vt_int val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vt_int.u32)));
    return (xmedia_bool)val.bits.cfg_vt_int_cnt_sel_1;
}

xmedia_void hal_reg_vout_set_cfg_vt_int_cnt_sel_1(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_vt_int val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_vt_int.u32));
    val.bits.cfg_vt_int_cnt_sel_1 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vt_int.u32)),val.u32);
    return;
}

/********************************************************
 接口垂直时序中断计数器起始行配置。
 0: 消隐区 
 1: 有效区
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_vt_int_cnt_sel_0(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_vt_int val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vt_int.u32)));
    return (xmedia_bool)val.bits.cfg_vt_int_cnt_sel_0;
}

xmedia_void hal_reg_vout_set_cfg_vt_int_cnt_sel_0(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_vt_int val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_vt_int.u32));
    val.bits.cfg_vt_int_cnt_sel_0 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vt_int.u32)),val.u32);
    return;
}

/********************************************************
 接口垂直时序中断模式
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_vt_int_mode_3(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_vt_int val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vt_int.u32)));
    return (xmedia_bool)val.bits.cfg_vt_int_mode_3;
}

xmedia_void hal_reg_vout_set_cfg_vt_int_mode_3(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_vt_int val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_vt_int.u32));
    val.bits.cfg_vt_int_mode_3 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vt_int.u32)),val.u32);
    return;
}

/********************************************************
 接口垂直时序中断模式
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_vt_int_mode_2(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_vt_int val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vt_int.u32)));
    return (xmedia_bool)val.bits.cfg_vt_int_mode_2;
}

xmedia_void hal_reg_vout_set_cfg_vt_int_mode_2(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_vt_int val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_vt_int.u32));
    val.bits.cfg_vt_int_mode_2 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vt_int.u32)),val.u32);
    return;
}

/********************************************************
 接口垂直时序中断模式
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_vt_int_mode_1(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_vt_int val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vt_int.u32)));
    return (xmedia_bool)val.bits.cfg_vt_int_mode_1;
}

xmedia_void hal_reg_vout_set_cfg_vt_int_mode_1(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_vt_int val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_vt_int.u32));
    val.bits.cfg_vt_int_mode_1 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vt_int.u32)),val.u32);
    return;
}

/********************************************************
 接口垂直时序中断模式
 0：帧模式，以帧为单位做门限计数；（隔行顶场）
 1：场模式，在隔行显示时以场为单位做门限计数。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_vt_int_mode_0(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_vt_int val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vt_int.u32)));
    return (xmedia_bool)val.bits.cfg_vt_int_mode_0;
}

xmedia_void hal_reg_vout_set_cfg_vt_int_mode_0(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_vt_int val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_vt_int.u32));
    val.bits.cfg_vt_int_mode_0 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vt_int.u32)),val.u32);
    return;
}

/********************************************************
 接口垂直时序中断门限值，单位行。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_vt_int_thd_1(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_vt_intf_thd0 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vt_intf_thd0.u32)));
    return (xmedia_u16)val.bits.cfg_vt_int_thd_1;
}

xmedia_void hal_reg_vout_set_cfg_vt_int_thd_1(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_vt_intf_thd0 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_vt_intf_thd0.u32));
    val.bits.cfg_vt_int_thd_1 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vt_intf_thd0.u32)),val.u32);
    return;
}

/********************************************************
 接口垂直时序中断门限值，单位行。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_vt_int_thd_0(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_vt_intf_thd0 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vt_intf_thd0.u32)));
    return (xmedia_u16)val.bits.cfg_vt_int_thd_0;
}

xmedia_void hal_reg_vout_set_cfg_vt_int_thd_0(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_vt_intf_thd0 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_vt_intf_thd0.u32));
    val.bits.cfg_vt_int_thd_0 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vt_intf_thd0.u32)),val.u32);
    return;
}

/********************************************************
 接口垂直时序中断门限值，单位行。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_vt_int_thd_3(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_vt_intf_thd2 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vt_intf_thd2.u32)));
    return (xmedia_u16)val.bits.cfg_vt_int_thd_3;
}

xmedia_void hal_reg_vout_set_cfg_vt_int_thd_3(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_vt_intf_thd2 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_vt_intf_thd2.u32));
    val.bits.cfg_vt_int_thd_3 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vt_intf_thd2.u32)),val.u32);
    return;
}

/********************************************************
 接口垂直时序中断门限值，单位行。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_vt_int_thd_2(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_vt_intf_thd2 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vt_intf_thd2.u32)));
    return (xmedia_u16)val.bits.cfg_vt_int_thd_2;
}

xmedia_void hal_reg_vout_set_cfg_vt_int_thd_2(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_vt_intf_thd2 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_vt_intf_thd2.u32));
    val.bits.cfg_vt_int_thd_2 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_vt_intf_thd2.u32)),val.u32);
    return;
}

/********************************************************
 接口帧计数器。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_frm_cnt(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_sta val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_sta.u32)));
    return (xmedia_u16)val.bits.frm_cnt;
}

/********************************************************
 接口场状态，0 顶场；1 底场。
 ********************************************************/
xmedia_bool hal_reg_vout_get_field_flag_ppc(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_sta val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_sta.u32)));
    return (xmedia_bool)val.bits.field_flag_ppc;
}

/********************************************************
 接口 vstate状态
  0 VFB ; 1 VBB ; 2: VACT; 3:VWAIT
 ********************************************************/
xmedia_u8 hal_reg_vout_get_vstate(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_sta val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_sta.u32)));
    return (xmedia_u8)val.bits.vstate;
}

/********************************************************
 接口行计数器，从进入有效区开始计数。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_vcnt_st_vact(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_sta2 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_sta2.u32)));
    return (xmedia_u16)val.bits.vcnt_st_vact;
}

/********************************************************
 接口行计数器，从进入消隐区开始计数。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_vcnt_st_vblk(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_sta2 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_sta2.u32)));
    return (xmedia_u16)val.bits.vcnt_st_vblk;
}

/********************************************************
 接口首次低带宽有效区行号。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_intf_uf_line(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_uf_line val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_uf_line.u32)));
    return (xmedia_u16)val.bits.intf_uf_line;
}

/********************************************************
 接口 Y/R 分量累加和。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_intf_y_sum(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_y_sum val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_y_sum.u32)));
    return (xmedia_u32)val.bits.intf_y_sum;
}

/********************************************************
 接口 U/G 分量累加和。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_intf_u_sum(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_u_sum val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_u_sum.u32)));
    return (xmedia_u32)val.bits.intf_u_sum;
}

/********************************************************
 接口 V/B  分量累加和。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_intf_v_sum(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_v_sum val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_v_sum.u32)));
    return (xmedia_u32)val.bits.intf_v_sum;
}

/********************************************************
 BT 接口输出bit 倒序
 0: 高bit-> 低bit；
 1: 低bit-> 高bit；
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_bt_bit_inv(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_bt_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bt_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_bt_bit_inv;
}

xmedia_void hal_reg_vout_set_cfg_bt_bit_inv(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_bt_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_bt_ctrl.u32));
    val.bits.cfg_bt_bit_inv = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bt_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 BT 接口VBIT 信号产生模式：
 0: 场消隐区
 1: 自定义 
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_vbit_mode(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_bt_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bt_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_vbit_mode;
}

xmedia_void hal_reg_vout_set_cfg_vbit_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_bt_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_bt_ctrl.u32));
    val.bits.cfg_vbit_mode = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bt_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 BT 接口 CbCr输出顺序
 0: Cb Cr; 
 1: Cr Cb;
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_bt_uv_order(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_bt_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bt_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_bt_uv_order;
}

xmedia_void hal_reg_vout_set_cfg_bt_uv_order(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_bt_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_bt_ctrl.u32));
    val.bits.cfg_bt_uv_order = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bt_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 BT 接口 YC 输出顺序
 0: CYCY ; 
 1: YCYC ;
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_bt_yc_order(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_bt_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bt_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_bt_yc_order;
}

xmedia_void hal_reg_vout_set_cfg_bt_yc_order(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_bt_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_bt_ctrl.u32));
    val.bits.cfg_bt_yc_order = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bt_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 BT 接口模式
 0：串行 8bit （BT656)
 1：并行 16bit （BT1120）
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_bt_mode(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_bt_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bt_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_bt_mode;
}

xmedia_void hal_reg_vout_set_cfg_bt_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_bt_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_bt_ctrl.u32));
    val.bits.cfg_bt_mode = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bt_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 BT656 vbit 顶场 自定义时序 
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_vbit_end(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_bt_vbit val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bt_vbit.u32)));
    return (xmedia_u16)val.bits.cfg_vbit_end;
}

xmedia_void hal_reg_vout_set_cfg_vbit_end(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_bt_vbit val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_bt_vbit.u32));
    val.bits.cfg_vbit_end = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bt_vbit.u32)),val.u32);
    return;
}

/********************************************************
 BT656 vbit 顶场 自定义时序 
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_vbit_st(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_bt_vbit val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bt_vbit.u32)));
    return (xmedia_u16)val.bits.cfg_vbit_st;
}

xmedia_void hal_reg_vout_set_cfg_vbit_st(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_bt_vbit val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_bt_vbit.u32));
    val.bits.cfg_vbit_st = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bt_vbit.u32)),val.u32);
    return;
}

/********************************************************
 BT656 vbit 低场 自定义时序 
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_bvbit_end(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_bt_bvbit val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bt_bvbit.u32)));
    return (xmedia_u16)val.bits.cfg_bvbit_end;
}

xmedia_void hal_reg_vout_set_cfg_bvbit_end(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_bt_bvbit val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_bt_bvbit.u32));
    val.bits.cfg_bvbit_end = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bt_bvbit.u32)),val.u32);
    return;
}

/********************************************************
 BT656 vbit 低场 自定义时序 
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_bvbit_st(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_bt_bvbit val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bt_bvbit.u32)));
    return (xmedia_u16)val.bits.cfg_bvbit_st;
}

xmedia_void hal_reg_vout_set_cfg_bvbit_st(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_bt_bvbit val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_bt_bvbit.u32));
    val.bits.cfg_bvbit_st = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_bt_bvbit.u32)),val.u32);
    return;
}

/********************************************************
 LCD 接口输出bit 倒序
 0: 高bit-> 低bit；
 1: 低bit-> 高bit；
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_lcd_bit_inv(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_lcd_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_lcd_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_lcd_bit_inv;
}

xmedia_void hal_reg_vout_set_cfg_lcd_bit_inv(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_lcd_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_lcd_ctrl.u32));
    val.bits.cfg_lcd_bit_inv = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_lcd_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 LCD 接口奇数行RGB的排列顺序
 0: RGB; 1: RBG ;
 2: GRB; 3: GBR;
 4: BRG; 5: BGR;
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_lcd_rgb_order_1(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_lcd_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_lcd_ctrl.u32)));
    return (xmedia_u8)val.bits.cfg_lcd_rgb_order_1;
}

xmedia_void hal_reg_vout_set_cfg_lcd_rgb_order_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_intf_lcd_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_lcd_ctrl.u32));
    val.bits.cfg_lcd_rgb_order_1 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_lcd_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 LCD 接口偶数行RGB的排列顺序
 0: RGB; 1: RBG ;
 2: GRB; 3: GBR;
 4: BRG; 5: BGR;
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_lcd_rgb_order_0(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_lcd_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_lcd_ctrl.u32)));
    return (xmedia_u8)val.bits.cfg_lcd_rgb_order_0;
}

xmedia_void hal_reg_vout_set_cfg_lcd_rgb_order_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_intf_lcd_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_lcd_ctrl.u32));
    val.bits.cfg_lcd_rgb_order_0 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_lcd_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 LCD 接口输出模式：
 0：串行 6bit
 1：串行 8bit
 2：并行 16bit RGB565
 3：并行 18bit RGB666 
 4：并行 24bit RGB888
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_lcd_mode(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_lcd_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_lcd_ctrl.u32)));
    return (xmedia_u8)val.bits.cfg_lcd_mode;
}

xmedia_void hal_reg_vout_set_cfg_lcd_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_intf_lcd_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_lcd_ctrl.u32));
    val.bits.cfg_lcd_mode = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_lcd_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 接口mute 棋盘格高，减1配置。
 ********************************************************/
xmedia_u8 hal_reg_vout_get_voutintf_cfg_checker_v(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_mute_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mute_ctrl.u32)));
    return (xmedia_u8)val.bits.cfg_checker_v;
}

xmedia_void hal_reg_vout_set_voutintf_cfg_checker_v(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_intf_mute_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_mute_ctrl.u32));
    val.bits.cfg_checker_v = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mute_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 接口mute 棋盘格宽，减1配置。
 ********************************************************/
xmedia_u8 hal_reg_vout_get_voutintf_cfg_checker_h(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_mute_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mute_ctrl.u32)));
    return (xmedia_u8)val.bits.cfg_checker_h;
}

xmedia_void hal_reg_vout_set_voutintf_cfg_checker_h(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_intf_mute_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_mute_ctrl.u32));
    val.bits.cfg_checker_h = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mute_ctrl.u32)),val.u32);
    return;
}

/********************************************************
  colorbar 色域选择
 0：RGB；1：YUV。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_cbar_sel(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_mute_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mute_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_cbar_sel;
}

xmedia_void hal_reg_vout_set_cfg_cbar_sel(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_mute_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_mute_ctrl.u32));
    val.bits.cfg_cbar_sel = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mute_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 接口 mute pattern
 0: 纯色
 1: 棋盘格
 2: colorbar
 3: 黑底白框,框颜色可配
 ********************************************************/
xmedia_u8 hal_reg_vout_get_voutintf_cfg_mute_pat(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_mute_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mute_ctrl.u32)));
    return (xmedia_u8)val.bits.cfg_mute_pat;
}

xmedia_void hal_reg_vout_set_voutintf_cfg_mute_pat(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_intf_mute_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_mute_ctrl.u32));
    val.bits.cfg_mute_pat = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mute_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 接口 mute模式。
 0:使能DMA申请数据，替换视频数据。
 1:不使能DMA申请数据，自建 valid/data。
 ********************************************************/
xmedia_bool hal_reg_vout_get_voutintf_cfg_mute_mode(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_mute_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mute_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_mute_mode;
}

xmedia_void hal_reg_vout_set_voutintf_cfg_mute_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_mute_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_mute_ctrl.u32));
    val.bits.cfg_mute_mode = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mute_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 接口MUTE 使能
 ********************************************************/
xmedia_bool hal_reg_vout_get_voutintf_cfg_mute_en(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_mute_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mute_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_mute_en;
}

xmedia_void hal_reg_vout_set_voutintf_cfg_mute_en(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_mute_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_mute_ctrl.u32));
    val.bits.cfg_mute_en = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mute_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 接口棋盘格 ，白色块 颜色设置。
 黑底白框，白框颜色设置。
 8bit 模式  [23:16] Y /R; [15:8] U/G; 
 [7:0] V/B;
 ********************************************************/
xmedia_u32 hal_reg_vout_get_voutintf_cfg_mute_color(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_mute_color val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mute_color.u32)));
    return (xmedia_u32)val.bits.cfg_mute_color;
}

xmedia_void hal_reg_vout_set_voutintf_cfg_mute_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_intf_mute_color val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_mute_color.u32));
    val.bits.cfg_mute_color = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mute_color.u32)),val.u32);
    return;
}

/********************************************************
 接口棋盘格 ，黑色块 颜色设置。
 黑底白框 ，黑底颜色设置。
 8bit 模式  [23:16] Y /R; [15:8] U/G; 
 [7:0] V/B;
 ********************************************************/
xmedia_u32 hal_reg_vout_get_intfmutebk_cfg_bk_color(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_mute_bk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mute_bk.u32)));
    return (xmedia_u32)val.bits.cfg_bk_color;
}

xmedia_void hal_reg_vout_set_intfmutebk_cfg_bk_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_intf_mute_bk val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_mute_bk.u32));
    val.bits.cfg_bk_color = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mute_bk.u32)),val.u32);
    return;
}

/********************************************************
 接口INK 十字光标颜色显示模式。
 0：取反；
 1：自定义；
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_cross_color_mode(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_ink_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ink_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_cross_color_mode;
}

xmedia_void hal_reg_vout_set_cfg_cross_color_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_ink_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_ink_ctrl.u32));
    val.bits.cfg_cross_color_mode = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ink_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 接口INK 十字光标显示使能。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_cross_en(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_ink_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ink_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_cross_en;
}

xmedia_void hal_reg_vout_set_cfg_cross_en(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_ink_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_ink_ctrl.u32));
    val.bits.cfg_cross_en = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ink_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 接口INK 使能。
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_ink_en(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_ink_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ink_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_ink_en;
}

xmedia_void hal_reg_vout_set_cfg_ink_en(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_ink_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_ink_ctrl.u32));
    val.bits.cfg_ink_en = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ink_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 接口INK 十字光标 Y坐标。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_y_pos(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_ink_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ink_pos.u32)));
    return (xmedia_u16)val.bits.cfg_y_pos;
}

xmedia_void hal_reg_vout_set_cfg_y_pos(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_ink_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_ink_pos.u32));
    val.bits.cfg_y_pos = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ink_pos.u32)),val.u32);
    return;
}

/********************************************************
 接口INK 十字光标 X坐标。
 ********************************************************/
xmedia_u16 hal_reg_vout_get_cfg_x_pos(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_ink_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ink_pos.u32)));
    return (xmedia_u16)val.bits.cfg_x_pos;
}

xmedia_void hal_reg_vout_set_cfg_x_pos(s_vout_regs_type *p_vout_reg,xmedia_u16 value)
{
    u_vout_intf_ink_pos val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_ink_pos.u32));
    val.bits.cfg_x_pos = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ink_pos.u32)),val.u32);
    return;
}

/********************************************************
 接口INK 十字光标 颜色。
 8bit 模式  [23:16] Y /R; [15:8] U/G; 
 [7:0] V/B;
 ********************************************************/
xmedia_u32 hal_reg_vout_get_cfg_cross_color(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_ink_color val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ink_color.u32)));
    return (xmedia_u32)val.bits.cfg_cross_color;
}

xmedia_void hal_reg_vout_set_cfg_cross_color(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_intf_ink_color val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_ink_color.u32));
    val.bits.cfg_cross_color = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ink_color.u32)),val.u32);
    return;
}

/********************************************************
 接口INK 数据回读。
 8bit 模式  [23:16] Y /R; [15:8] U/G; 
 [7:0] V/B;
 ********************************************************/
xmedia_u32 hal_reg_vout_get_ink_data(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_ink_data val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_ink_data.u32)));
    return (xmedia_u32)val.bits.ink_data;
}

/********************************************************
 接口输出数据钳位使能，主要应用在 BT 接口场景。
 Y 钳位范围 16~235；C 钳位范围 16~240；
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_clip_en(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_clip_l val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_clip_l.u32)));
    return (xmedia_bool)val.bits.cfg_clip_en;
}

xmedia_void hal_reg_vout_set_cfg_clip_en(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_clip_l val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_clip_l.u32));
    val.bits.cfg_clip_en = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_clip_l.u32)),val.u32);
    return;
}

/********************************************************
 接口输出数据钳位最低门限。
 8bit 模式  [23:16] Y ; [15:8] U; [7:0
 ] V;
 ********************************************************/
xmedia_u32 hal_reg_vout_get_cfg_clip_yuv_l(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_clip_l val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_clip_l.u32)));
    return (xmedia_u32)val.bits.cfg_clip_yuv_l;
}

xmedia_void hal_reg_vout_set_cfg_clip_yuv_l(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_intf_clip_l val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_clip_l.u32));
    val.bits.cfg_clip_yuv_l = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_clip_l.u32)),val.u32);
    return;
}

/********************************************************
 接口输出数据钳位最高门限。
 8bit 模式  [23:16] Y ; [15:8] U; [7:0
 ] V;
 ********************************************************/
xmedia_u32 hal_reg_vout_get_cfg_clip_yuv_h(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_clip_h val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_clip_h.u32)));
    return (xmedia_u32)val.bits.cfg_clip_yuv_h;
}

xmedia_void hal_reg_vout_set_cfg_clip_yuv_h(s_vout_regs_type *p_vout_reg,xmedia_u32 value)
{
    u_vout_intf_clip_h val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_clip_h.u32));
    val.bits.cfg_clip_yuv_h = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_clip_h.u32)),val.u32);
    return;
}

/********************************************************
 MCU写display数据格式类型
 2'd0:RGB444(不支持)
 2'd1:RGB565
 2'd2:RGB666
 2'd3:保留
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_mcu_wr_fmt(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_mcu_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mcu_ctrl.u32)));
    return (xmedia_u8)val.bits.cfg_mcu_wr_fmt;
}

xmedia_void hal_reg_vout_set_cfg_mcu_wr_fmt(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_intf_mcu_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_mcu_ctrl.u32));
    val.bits.cfg_mcu_wr_fmt = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mcu_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 当前帧写display data到屏幕frame ram命令使能
 0：不使能
 1：使能
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_mcu_wrram_en(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_mcu_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mcu_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_mcu_wrram_en;
}

xmedia_void hal_reg_vout_set_cfg_mcu_wrram_en(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_mcu_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_mcu_ctrl.u32));
    val.bits.cfg_mcu_wrram_en = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mcu_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 屏幕时序同步模式
 0：不使用TE同步
 1：使用TE同步
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_mcu_sync_type(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_mcu_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mcu_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_mcu_sync_type;
}

xmedia_void hal_reg_vout_set_cfg_mcu_sync_type(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_mcu_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_mcu_ctrl.u32));
    val.bits.cfg_mcu_sync_type = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mcu_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 写display data到屏frame ram命令
 每帧第一个数据包
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_mcu_wrram_cmd(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_mcu_wrram_cmd val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mcu_wrram_cmd.u32)));
    return (xmedia_u8)val.bits.cfg_mcu_wrram_cmd;
}

xmedia_void hal_reg_vout_set_cfg_mcu_wrram_cmd(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_intf_mcu_wrram_cmd val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_mcu_wrram_cmd.u32));
    val.bits.cfg_mcu_wrram_cmd = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mcu_wrram_cmd.u32)),val.u32);
    return;
}

/********************************************************
 写display data到屏frame ram命令
 每帧第一个数据包
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_mcu_wrramc_cmd(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_mcu_wrramc_cmd val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mcu_wrramc_cmd.u32)));
    return (xmedia_u8)val.bits.cfg_mcu_wrramc_cmd;
}

xmedia_void hal_reg_vout_set_cfg_mcu_wrramc_cmd(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_intf_mcu_wrramc_cmd val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_mcu_wrramc_cmd.u32));
    val.bits.cfg_mcu_wrramc_cmd = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_mcu_wrramc_cmd.u32)),val.u32);
    return;
}

/********************************************************
 色度水平下采样模式
 0:丢点模式;
 1:2阶滤波；
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_hcds_mode(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_hcds_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_hcds_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_hcds_mode;
}

xmedia_void hal_reg_vout_set_cfg_hcds_mode(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_hcds_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_hcds_ctrl.u32));
    val.bits.cfg_hcds_mode = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_hcds_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 色度水平下采样使能
 (YUV444到YUV422)
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_hcds_en(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_hcds_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_hcds_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_hcds_en;
}

xmedia_void hal_reg_vout_set_cfg_hcds_en(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_hcds_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_hcds_ctrl.u32));
    val.bits.cfg_hcds_en = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_hcds_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 列数列水平下采样系数。
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_hcds_coef_3(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_hcds_coef val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_hcds_coef.u32)));
    return (xmedia_u8)val.bits.cfg_hcds_coef_3;
}

xmedia_void hal_reg_vout_set_cfg_hcds_coef_3(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_intf_hcds_coef val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_hcds_coef.u32));
    val.bits.cfg_hcds_coef_3 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_hcds_coef.u32)),val.u32);
    return;
}

/********************************************************
 列数列水平下采样系数。
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_hcds_coef_2(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_hcds_coef val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_hcds_coef.u32)));
    return (xmedia_u8)val.bits.cfg_hcds_coef_2;
}

xmedia_void hal_reg_vout_set_cfg_hcds_coef_2(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_intf_hcds_coef val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_hcds_coef.u32));
    val.bits.cfg_hcds_coef_2 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_hcds_coef.u32)),val.u32);
    return;
}

/********************************************************
 偶数列水平下采样系数。
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_hcds_coef_1(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_hcds_coef val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_hcds_coef.u32)));
    return (xmedia_u8)val.bits.cfg_hcds_coef_1;
}

xmedia_void hal_reg_vout_set_cfg_hcds_coef_1(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_intf_hcds_coef val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_hcds_coef.u32));
    val.bits.cfg_hcds_coef_1 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_hcds_coef.u32)),val.u32);
    return;
}

/********************************************************
 偶数列水平下采样系数。
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_hcds_coef_0(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_hcds_coef val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_hcds_coef.u32)));
    return (xmedia_u8)val.bits.cfg_hcds_coef_0;
}

xmedia_void hal_reg_vout_set_cfg_hcds_coef_0(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_intf_hcds_coef val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_hcds_coef.u32));
    val.bits.cfg_hcds_coef_0 = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_hcds_coef.u32)),val.u32);
    return;
}

/********************************************************
 做Dither 处理像素上限。分量共用。
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_dither_thd_h(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_dither_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_dither_ctrl.u32)));
    return (xmedia_u8)val.bits.cfg_dither_thd_h;
}

xmedia_void hal_reg_vout_set_cfg_dither_thd_h(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_intf_dither_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_dither_ctrl.u32));
    val.bits.cfg_dither_thd_h = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_dither_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 做Dither 处理像素下限。分量共用。
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_dither_thd_l(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_dither_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_dither_ctrl.u32)));
    return (xmedia_u8)val.bits.cfg_dither_thd_l;
}

xmedia_void hal_reg_vout_set_cfg_dither_thd_l(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_intf_dither_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_dither_ctrl.u32));
    val.bits.cfg_dither_thd_l = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_dither_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 Gamma 使能
 只支持 RGB 格式；
 
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_gamma_en(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_dither_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_dither_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_gamma_en;
}

xmedia_void hal_reg_vout_set_cfg_gamma_en(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_dither_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_dither_ctrl.u32));
    val.bits.cfg_gamma_en = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_dither_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 Dither 使能；
 支持输入RGB888,输出RGB565/RGB666;
 
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_dither_en(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_dither_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_dither_ctrl.u32)));
    return (xmedia_bool)val.bits.cfg_dither_en;
}

xmedia_void hal_reg_vout_set_cfg_dither_en(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_dither_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_dither_ctrl.u32));
    val.bits.cfg_dither_en = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_dither_ctrl.u32)),val.u32);
    return;
}

xmedia_void hal_reg_vout_set_cfg_gamma_rd_addr(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_intf_gammalut_rd_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_gammalut_rd_ctrl.u32));
    val.bits.cfg_gamma_rd_addr = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_gammalut_rd_ctrl.u32)),val.u32);
    return;
}

xmedia_void hal_reg_vout_set_cfg_gamma_rd_en(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_intf_gammalut_rd_ctrl val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_intf_gammalut_rd_ctrl.u32));
    val.bits.cfg_gamma_rd_en = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_gammalut_rd_ctrl.u32)),val.u32);
    return;
}

/********************************************************
 INTF GAMMA 表读数据。
 ********************************************************/
xmedia_u32 hal_reg_vout_get_gamma_rd_data(s_vout_regs_type *p_vout_reg)
{
    u_vout_intf_gammalut_rd_data val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_intf_gammalut_rd_data.u32)));
    return (xmedia_u32)val.bits.gamma_rd_data;
}

/********************************************************
 MCU输出reset信号
 ********************************************************/
xmedia_bool hal_reg_vout_get_cfg_mcu_rst_ctrl(s_vout_regs_type *p_vout_reg)
{
    u_vout_mcu_screen_ctrl_0 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_ctrl_0.u32)));
    return (xmedia_bool)val.bits.cfg_mcu_rst_ctrl;
}

xmedia_void hal_reg_vout_set_cfg_mcu_rst_ctrl(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_mcu_screen_ctrl_0 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_mcu_screen_ctrl_0.u32));
    val.bits.cfg_mcu_rst_ctrl = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_ctrl_0.u32)),val.u32);
    return;
}

/********************************************************
 MCU屏幕接口类型和像素编码格式
 8bit并口
 8'd0：8bit-RGB565-8080-Ⅰ
 8'd1：8bit-RGB666-8080-Ⅰ
 8'd2：8bit-RGB565-8080-Ⅱ
 8'd3：8bit-RGB666-8080-Ⅱ
 9bit并口
 8'd4：9bit-RGB565-8080-Ⅰ
 8'd5：9bit-RGB666-8080-Ⅰ-option1
 8'd6：9bit-RGB666-8080-Ⅰ-option2
 8'd7：9bit-RGB565-8080-Ⅱ
 8'd8：9bit-RGB666-8080-Ⅱ-option1
 8'd9：9bit-RGB666-8080-Ⅱ-option2
 16bit并口
 8'd10：16bit-RGB565-8080-Ⅰ
 8'd11：16bit-RGB666-8080-Ⅰ-option1
 8'd12：16bit-RGB666-8080-Ⅰ-option2
 8'd13：16bit-RGB666-8080-Ⅰ-option3
 8'd14：16bit-RGB666-8080-Ⅰ-option4
 8'd15：16bit-RGB666-8080-Ⅰ-option5
 8'd16：16bit-RGB565-8080-Ⅱ
 8'd17：16bit-RGB666-8080-Ⅱ-option1
 8'd18：16bit-RGB666-8080-Ⅱ-option2
 8'd19：16bit-RGB666-8080-Ⅱ-option3
 8'd20：16bit-RGB666-8080-Ⅱ-option4
 18bit并口
 8'd21：18bit-RGB565-8080-Ⅰ
 8'd22：18bit-RGB666-8080-Ⅰ
 8'd23：18bit-RGB565-8080-Ⅱ
 8'd24：18bit-RGB666-8080-Ⅱ
 串口
 8'd25：3线-RGB444-option1
 8'd26：3线-RGB565-option1
 8'd27：3线-RGB666-option1
 8'd28：3线-RGB444-option2
 8'd29：3线-RGB565-option2
 8'd30：3线-RGB666-option2
 8'd31：4线-RGB444
 8'd32：4线-RGB565
 8'd33：4线-RGB666
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_mcu_mode(s_vout_regs_type *p_vout_reg)
{
    u_vout_mcu_screen_ctrl_0 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_ctrl_0.u32)));
    return (xmedia_u8)val.bits.cfg_mcu_mode;
}

xmedia_void hal_reg_vout_set_cfg_mcu_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_mcu_screen_ctrl_0 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_mcu_screen_ctrl_0.u32));
    val.bits.cfg_mcu_mode = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_ctrl_0.u32)),val.u32);
    return;
}

xmedia_void hal_reg_vout_set_cfg_mcu_para_raddr(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_mcu_screen_ctrl_1 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_mcu_screen_ctrl_1.u32));
    val.bits.cfg_mcu_para_raddr = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_ctrl_1.u32)),val.u32);
    return;
}

xmedia_void hal_reg_vout_set_cfg_mcu_para_rd(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_mcu_screen_ctrl_1 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_mcu_screen_ctrl_1.u32));
    val.bits.cfg_mcu_para_rd = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_ctrl_1.u32)),val.u32);
    return;
}

xmedia_void hal_reg_vout_set_cfg_mcu_wrcmd_start(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_mcu_screen_ctrl_1 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_mcu_screen_ctrl_1.u32));
    val.bits.cfg_mcu_wrcmd_start = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_ctrl_1.u32)),val.u32);
    return;
}

xmedia_void hal_reg_vout_set_cfg_mcu_cmd_stop(s_vout_regs_type *p_vout_reg,xmedia_bool value)
{
    u_vout_mcu_screen_ctrl_1 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_mcu_screen_ctrl_1.u32));
    val.bits.cfg_mcu_cmd_stop = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_ctrl_1.u32)),val.u32);
    return;
}

/********************************************************
 屏写参数个数，减1配置
 取值范围：0~63
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_mcu_wrcmd_size(s_vout_regs_type *p_vout_reg)
{
    u_vout_mcu_screen_ctrl_2 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_ctrl_2.u32)));
    return (xmedia_u8)val.bits.cfg_mcu_wrcmd_size;
}

xmedia_void hal_reg_vout_set_cfg_mcu_wrcmd_size(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_mcu_screen_ctrl_2 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_mcu_screen_ctrl_2.u32));
    val.bits.cfg_mcu_wrcmd_size = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_ctrl_2.u32)),val.u32);
    return;
}

/********************************************************
 屏命令类型
 2'd0：无操作
 2'd1：写参数，缓存到参数RAM，64x32bit
 2'd2：读参数，每次最多读16byte，映射到寄存器'h0040~'
 h004f
 2'd3：读显示数据，每次最多读64pixel，缓存到参数RAM
 2'd4：读LUT数据，，每次最多256byte，缓存到参数RAM
 ********************************************************/
xmedia_u8 hal_reg_vout_get_cfg_mcu_cmd_mode(s_vout_regs_type *p_vout_reg)
{
    u_vout_mcu_screen_ctrl_2 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_ctrl_2.u32)));
    return (xmedia_u8)val.bits.cfg_mcu_cmd_mode;
}

xmedia_void hal_reg_vout_set_cfg_mcu_cmd_mode(s_vout_regs_type *p_vout_reg,xmedia_u8 value)
{
    u_vout_mcu_screen_ctrl_2 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)&(p_vout_reg->vout_mcu_screen_ctrl_2.u32));
    val.bits.cfg_mcu_cmd_mode = value;
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_ctrl_2.u32)),val.u32);
    return;
}

/********************************************************
 参数RAM回读数据
 ********************************************************/
xmedia_u32 hal_reg_vout_get_mcu_para_rdata(s_vout_regs_type *p_vout_reg)
{
    return hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_ram_rdata)));
}

/********************************************************
 屏读LUT/显示数据状态
 0：空闲状态
 1：读显示数据/读LUT中
 ********************************************************/
xmedia_bool hal_reg_vout_get_mcu_rd_pixel_state(s_vout_regs_type *p_vout_reg)
{
    u_vout_mcu_screen_state val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_state.u32)));
    return (xmedia_bool)val.bits.mcu_rd_pixel_state;
}

/********************************************************
 屏读参数
 0：空闲状态
 1：读参数中
 ********************************************************/
xmedia_bool hal_reg_vout_get_mcu_rd_cmd_state(s_vout_regs_type *p_vout_reg)
{
    u_vout_mcu_screen_state val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_state.u32)));
    return (xmedia_bool)val.bits.mcu_rd_cmd_state;
}

/********************************************************
 屏写参数状态
 2'd0：空闲状态
 2'd1：写参数已经全部存到参数RAM中，但还未发送到MCU屏幕
 2'd2：写参数从RAM中发送到MCU中
 ********************************************************/
xmedia_u8 hal_reg_vout_get_mcu_wr_cmd_state(s_vout_regs_type *p_vout_reg)
{
    u_vout_mcu_screen_state val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_state.u32)));
    return (xmedia_u8)val.bits.mcu_wr_cmd_state;
}

/********************************************************
 MCU屏读参数
 ********************************************************/
xmedia_u8 hal_reg_vout_get_mcu_rd_para_3(s_vout_regs_type *p_vout_reg)
{
    u_vout_mcu_screen_para_rdata_0 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_para_rdata_0.u32)));
    return (xmedia_u8)val.bits.mcu_rd_para_3;
}

/********************************************************
 
 ********************************************************/
xmedia_u8 hal_reg_vout_get_mcu_rd_para_2(s_vout_regs_type *p_vout_reg)
{
    u_vout_mcu_screen_para_rdata_0 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_para_rdata_0.u32)));
    return (xmedia_u8)val.bits.mcu_rd_para_2;
}

/********************************************************
 
 ********************************************************/
xmedia_u8 hal_reg_vout_get_mcu_rd_para_1(s_vout_regs_type *p_vout_reg)
{
    u_vout_mcu_screen_para_rdata_0 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_para_rdata_0.u32)));
    return (xmedia_u8)val.bits.mcu_rd_para_1;
}

/********************************************************
 
 ********************************************************/
xmedia_u8 hal_reg_vout_get_mcu_rd_para_0(s_vout_regs_type *p_vout_reg)
{
    u_vout_mcu_screen_para_rdata_0 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_para_rdata_0.u32)));
    return (xmedia_u8)val.bits.mcu_rd_para_0;
}

/********************************************************
 MCU屏读参数
 ********************************************************/
xmedia_u8 hal_reg_vout_get_mcu_rd_para_7(s_vout_regs_type *p_vout_reg)
{
    u_vout_mcu_screen_para_rdata_1 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_para_rdata_1.u32)));
    return (xmedia_u8)val.bits.mcu_rd_para_7;
}

/********************************************************
 
 ********************************************************/
xmedia_u8 hal_reg_vout_get_mcu_rd_para_6(s_vout_regs_type *p_vout_reg)
{
    u_vout_mcu_screen_para_rdata_1 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_para_rdata_1.u32)));
    return (xmedia_u8)val.bits.mcu_rd_para_6;
}

/********************************************************
 
 ********************************************************/
xmedia_u8 hal_reg_vout_get_mcu_rd_para_5(s_vout_regs_type *p_vout_reg)
{
    u_vout_mcu_screen_para_rdata_1 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_para_rdata_1.u32)));
    return (xmedia_u8)val.bits.mcu_rd_para_5;
}

/********************************************************
 
 ********************************************************/
xmedia_u8 hal_reg_vout_get_mcu_rd_para_4(s_vout_regs_type *p_vout_reg)
{
    u_vout_mcu_screen_para_rdata_1 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_para_rdata_1.u32)));
    return (xmedia_u8)val.bits.mcu_rd_para_4;
}

/********************************************************
 MCU屏读参数
 ********************************************************/
xmedia_u8 hal_reg_vout_get_mcu_rd_para_11(s_vout_regs_type *p_vout_reg)
{
    u_vout_mcu_screen_para_rdata_2 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_para_rdata_2.u32)));
    return (xmedia_u8)val.bits.mcu_rd_para_11;
}

/********************************************************
 
 ********************************************************/
xmedia_u8 hal_reg_vout_get_mcu_rd_para_10(s_vout_regs_type *p_vout_reg)
{
    u_vout_mcu_screen_para_rdata_2 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_para_rdata_2.u32)));
    return (xmedia_u8)val.bits.mcu_rd_para_10;
}

/********************************************************
 
 ********************************************************/
xmedia_u8 hal_reg_vout_get_mcu_rd_para_9(s_vout_regs_type *p_vout_reg)
{
    u_vout_mcu_screen_para_rdata_2 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_para_rdata_2.u32)));
    return (xmedia_u8)val.bits.mcu_rd_para_9;
}

/********************************************************
 
 ********************************************************/
xmedia_u8 hal_reg_vout_get_mcu_rd_para_8(s_vout_regs_type *p_vout_reg)
{
    u_vout_mcu_screen_para_rdata_2 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_para_rdata_2.u32)));
    return (xmedia_u8)val.bits.mcu_rd_para_8;
}

/********************************************************
 MCU屏读参数
 ********************************************************/
xmedia_u8 hal_reg_vout_get_mcu_rd_para_15(s_vout_regs_type *p_vout_reg)
{
    u_vout_mcu_screen_para_rdata_3 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_para_rdata_3.u32)));
    return (xmedia_u8)val.bits.mcu_rd_para_15;
}

/********************************************************
 
 ********************************************************/
xmedia_u8 hal_reg_vout_get_mcu_rd_para_14(s_vout_regs_type *p_vout_reg)
{
    u_vout_mcu_screen_para_rdata_3 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_para_rdata_3.u32)));
    return (xmedia_u8)val.bits.mcu_rd_para_14;
}

/********************************************************
 
 ********************************************************/
xmedia_u8 hal_reg_vout_get_mcu_rd_para_13(s_vout_regs_type *p_vout_reg)
{
    u_vout_mcu_screen_para_rdata_3 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_para_rdata_3.u32)));
    return (xmedia_u8)val.bits.mcu_rd_para_13;
}

/********************************************************
 
 ********************************************************/
xmedia_u8 hal_reg_vout_get_mcu_rd_para_12(s_vout_regs_type *p_vout_reg)
{
    u_vout_mcu_screen_para_rdata_3 val;

    val.u32 = hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_para_rdata_3.u32)));
    return (xmedia_u8)val.bits.mcu_rd_para_12;
}

/********************************************************
 MCU屏幕写参数，i inside [0:63]
 ********************************************************/
xmedia_u32 hal_reg_vout_get_mcu_wr_para(s_vout_regs_type *p_vout_reg,xmedia_u32 i)
{
    return hal_reg_vout_reg_read((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_wr_para) + (i * 0x4)));
}

xmedia_void hal_reg_vout_set_mcu_wr_para(s_vout_regs_type *p_vout_reg,xmedia_u32 i,xmedia_u32 value)
{
    hal_reg_vout_reg_write((xmedia_uintptr_t)((xmedia_u8 *)&(p_vout_reg->vout_mcu_screen_wr_para) + (i * 0x4)),value);
    return;
}

