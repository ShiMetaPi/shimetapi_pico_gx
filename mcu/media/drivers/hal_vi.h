#ifndef __HAL_VI_H_
#define __HAL_VI_H_

#include "xmedia_type.h"
#include "xmedia_vi.h"
#include "compile.h"

#define VI_CAP_FRAME_START_INT      (0x1 << 6)
#define VI_CAP_FRAME_LOWDELAY_INT   (0x1 << 1)
#define VI_CAP_FRAME_EARLY_INT      (0x1 << 0)
#define VI_CAP_FRAME_END_INT        (0x1 << 7)
#define VI_CAP_FRAME_END_INT_TIMING (0x1 << 11)
#define VI_CAP_FRAME_ERR_INT        ((0x1 << 8) | (0x1 << 12) | (0x1 << 3) | (0x1 << 4) | (0x1 << 13))
#define VI_CAP_FRAME_TIMEOUT_INT    ((0x1 << 9) | (0x1 << 10))

#define VI_PROC_FRAME_LIST_INT      (0x1 << 5)
#define VI_PROC_FRAME_START_INT     (0x1 << 3)
#define VI_PROC_FRAME_EARLY_INT     (0x1)
#define VI_PROC_LOWDELAY_INT        (0x1 << 1)
#define VI_PROC_FRAME_END_INT       (0x1 << 4)
#define VI_PROC_FRAME_ERR_INT                                                                                        \
    ((0x1 << 6) | (0x1 << 7) | (0x1 << 8) | (0x1 << 9) | (0x1 << 13) | (0x1 << 14) | (0x1 << 15) | (0x1 << 16) |     \
     (0x1 << 17) | (0x1 << 18) | (0x1 << 19) | (0x1 << 20) | (0x1 << 21) | (0x1 << 22) | (0x1 << 27) | (0x1 << 28) | \
     (0x1 << 29) | (0x1 << 30) | (0x1 << 31))
#define VI_PROC_FRAME_TIMEOUT_INT    ((0x1 << 19) | (0x1 << 20))
#define VI_PROC_FRAME_ERR_IGNORE_INT (0x7 << 10)

#define sleep_us(x)                          \
    do {                                     \
        for (xmedia_s32 i = x; i > 0; i--) { \
            asm("nop");                      \
            asm("nop");                      \
            asm("nop");                      \
            asm("nop");                      \
            asm("nop");                      \
        }                                    \
    } while (0)

typedef enum {
    VI_PACK_8BIT = 8,
    VI_PACK_10BIT = 10,
    VI_PACK_12BIT = 12,
    VI_PACK_14BIT = 14,
    VI_PACK_16BIT = 16,
    VI_PACK_MAX,
} vi_pack_bit_width;

xmedia_void hal_vi_vicap_reg_newer(xmedia_s32 fe_id);
xmedia_void hal_vi_vicap_online_config(xmedia_s32 fe_id, xmedia_vi_pipe_config *pipe_config);
xmedia_u32 hal_vi_vicap_get_top_int_status(xmedia_u32 vicap_id);
xmedia_u32 hal_vi_vicap_get_top_int_mask(xmedia_u32 vicap_id);
xmedia_u32 hal_vi_vicap_get_chn_int_status(xmedia_s32 fe_id);
xmedia_void hal_vi_vicap_clear_int(xmedia_s32 fe_id, xmedia_u32 mask);
xmedia_s32 hal_vi_get_vicap_irq_num(xmedia_u32 vicap_id);
xmedia_s32 hal_vi_get_viproc_irq_num(xmedia_u32 viproc_id);
xmedia_u32 hal_viproc_get_reg_public_int_status(xmedia_u32 viproc_id);
xmedia_u32 hal_viproc_get_reg_public_int_mask(xmedia_u32 viproc_id);
xmedia_u32 hal_viproc_common_get_reg_early_mask(xmedia_u32 viproc_id);
xmedia_void hal_viproc_clr_reg_int(xmedia_u32 viproc_id, xmedia_u32 mask);
xmedia_void hal_vi_init(xmedia_void);
xmedia_void hal_vi_exit(xmedia_void);
xmedia_s32 hal_vi_enable_dev(xmedia_s32 dev);
xmedia_void hal_mipi_init(xmedia_void);
xmedia_s32 hal_mipi_enable(xmedia_s32 dev, xmedia_vi_dev_config *dev_config);
xmedia_void hal_vi_disable_dev(xmedia_s32 dev, xmedia_vi_dev_config *dev_config);
xmedia_s32 hal_vi_set_dev_bind_pipe(xmedia_s32 dev, xmedia_s32 fe_id);
xmedia_void hal_vi_set_dev_unbind_pipe(xmedia_s32 dev, xmedia_s32 fe_id);
xmedia_s32 hal_vi_start_vicap(xmedia_s32 fe_id, xmedia_vi_pipe_config *pipe_config);
xmedia_void hal_vi_viproc_online_config(xmedia_s32 fe_id, xmedia_vi_pipe_config *pipe_config);
xmedia_void hal_vi_viproc_reg_start(xmedia_u32 viproc_id);
xmedia_s32 hal_vi_start_viproc(xmedia_s32 fe_id, xmedia_vi_pipe_config *pipe_config);
xmedia_void hal_vi_stop_viproc(xmedia_s32 fe_id);
xmedia_void hal_vi_stop_vicap(xmedia_s32 fe_id);

#endif
