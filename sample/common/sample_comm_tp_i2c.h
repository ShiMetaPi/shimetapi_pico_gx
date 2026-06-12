#ifndef __TP_I2C_H__
#define __TP_I2C_H__

#include "xmedia_type.h"

xmedia_s32 sample_comm_tp_i2c_init(xmedia_u32 dev);
xmedia_void sample_comm_tp_i2c_exit(xmedia_void);
xmedia_s32 sample_comm_tp_i2c_read(xmedia_u32 addr, xmedia_u32 *data);
xmedia_s32 sample_comm_tp_i2c_write(xmedia_u32 addr, xmedia_u32 data);

#endif