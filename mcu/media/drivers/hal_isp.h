#ifndef _HAL_ISP_H_
#define _HAL_ISP_H_

xmedia_void delay(xmedia_u32 us);
xmedia_s32 hal_sensor_reset(xmedia_u32 ch, xmedia_bool enable);
xmedia_s32 hal_sensor_read_mclk(xmedia_u32 ch, xmedia_u32 *clk);
xmedia_s32 hal_sensor_set_mclk(xmedia_u32 ch, xmedia_u32 clk);
xmedia_s32 hal_isp_init(xmedia_u32 pipe, xmedia_isp_config *cfg, xmedia_sensor_black_level *blc);
xmedia_s32 hal_isp_init_expander(xmedia_u32 pipe, const xmedia_isp_expander_attr *attr);
xmedia_s32 hal_isp_stat(xmedia_u32 pipe, xmedia_isp_statistics *stat);
xmedia_s32 hal_isp_update(xmedia_u32 pipe, xmedia_isp_result *result, xmedia_sensor_regs_info *reg_info);
xmedia_s32 hal_isp_format_statistics(xmedia_u32 pipe);
xmedia_s32 hal_isp_config(xmedia_u32 pipe);
xmedia_s32 hal_isp_sync(xmedia_u32 pipe);

#endif

