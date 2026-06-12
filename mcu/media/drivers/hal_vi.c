#include "stdio.h"
#include "xmedia_type.h"
#include "xmedia_vi.h"
#include "defines.h"
#include "hal_vi.h"

/*
 * 系统控制寄存器，MISC_CTRL81
 * BIT [0] :
 * 0:MIPI_RX_PHY1连接MIPI_2L_RX0控制器
 * 1:MIPI_RX_PHY1连接MIPI_4L_RX0控制器
 * BIT [1] :
 * 0:MIPI_RX_PHY3连接MIPI_2L_RX1控制器
 * 1:MIPI_RX_PHY3连接MIPI_4L_RX1控制器
 * */
#define MIPI_MISC_CTRL                 0x12028900
#define VIPROC_EXTERNAL_TIMING_EN_ADDR 0x1202899C
#define CRG_REGS_ADDR                  0x12010000

#define VICAP_IRQ_NUM                  66
#define VIPROC0_IRQ_NUM                63

#define VI_RESET_WAIT_CNT              100

typedef struct {
    xmedia_u32 phy0_data_lane0;
    xmedia_u32 phy0_data_lane1;
    xmedia_u32 phy1_data_lane0;
    xmedia_u32 phy1_data_lane1;
    xmedia_u32 phy0_clk_sel;
    xmedia_u32 phy1_clk_sel;
} mipi_lane_swap_attr;

volatile xmedia_u32 g_vicap_all_reg[VICAP_IP_NUM] = { 0x11000000 };
volatile xmedia_u32 g_viproc_all_reg[VIPROC_IP_NUM] = { 0x11200000 };
STAGE1_GLOBAL volatile xmedia_u32 g_mipi_rx_reg[VI_MIPI_RX_NUM] = { 0x10fe0000, 0x11010000 };
STAGE1_GLOBAL volatile xmedia_u32 g_mipi_phy_reg_addr = 0x12028900;

STAGE1_FUNC static xmedia_u32 vi_read_reg(volatile xmedia_uintptr_t addr)
{
    return *(volatile xmedia_u32 *)(addr);
}

STAGE1_FUNC static xmedia_void vi_write_reg(volatile xmedia_uintptr_t addr, xmedia_u32 val)
{
    *(volatile xmedia_u32 *)(addr) = val;
}

STAGE1_FUNC xmedia_void vi_reg_write_bit(xmedia_u32 addr, xmedia_u32 value, xmedia_u32 bit_pos)
{
    xmedia_u32 t, mask;

    mask = 1 << bit_pos;
    t = vi_read_reg(addr);
    t &= ~mask;
    t |= (value << bit_pos) & mask;
    vi_write_reg(addr, t);
}

STAGE1_FUNC xmedia_void vi_reg_write_32(xmedia_u32 addr, xmedia_u32 value, xmedia_u32 bit_pos, xmedia_u32 bit_count)
{
    xmedia_u32 t;
    xmedia_u32 mask;

    t = vi_read_reg(addr);

    mask = ((1U << bit_count) - 1) << bit_pos;
    t &= ~mask;
    t |= (value << bit_pos) & mask;

    vi_write_reg(addr, t);
}

static xmedia_s32 vi_dev_reset(xmedia_s32 dev, xmedia_bool reset)
{
    xmedia_bool reset_state = 0;
    xmedia_u32 wait_cnt = 0;
    xmedia_u32 reg;

    vi_reg_write_bit(CRG_REGS_ADDR + 0xf4, reset, 4 + dev); // 4 : pt0 offset

    reg = vi_read_reg(CRG_REGS_ADDR + 0x1e4);
    reset_state = (reg & (0x1 << (8 + dev))) >> (8 + dev); // bit[8:12]: pt0-pt4 reset state

    while (reset_state != reset) {
        sleep_us(1);
        wait_cnt++;
        reg = vi_read_reg(CRG_REGS_ADDR + 0x1e4);
        reset_state = (reg & (0x1 << (8 + dev))) >> (8 + dev); // bit[8:12]: pt0-pt4 reset state
        if (reset_state != reset && wait_cnt > VI_RESET_WAIT_CNT) {
            puts("dev reset err\n");
            return XMEDIA_FAILURE;
        }
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 vi_vicap_fe_reset(xmedia_s32 fe_id, xmedia_bool reset)
{
    xmedia_bool reset_state = 0;
    xmedia_u32 reg;
    xmedia_u32 wait_cnt = 0;

    vi_reg_write_bit(CRG_REGS_ADDR + 0xf4, reset, 9 + fe_id); // 9 : fe0 offset

    reg = vi_read_reg(CRG_REGS_ADDR + 0x1e4);
    reset_state = (reg & (0x1 << (13 + fe_id))) >> (13 + fe_id); // bit[13:20]: fe0-fe7 reset state

    while (reset_state != reset) {
        sleep_us(1);
        wait_cnt++;
        reg = vi_read_reg(CRG_REGS_ADDR + 0x1e4);
        reset_state = (reg & (0x1 << (13 + fe_id))) >> (13 + fe_id); // bit[13:20]: fe0-fe7 reset state
        if (reset_state != reset && wait_cnt > VI_RESET_WAIT_CNT) {
            puts("vicap reset failed\n");
            return XMEDIA_FAILURE;
        }
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 vi_viproc_reset_set(xmedia_u32 proc_id, xmedia_bool reset)
{
    xmedia_bool reset_state = 0;
    xmedia_u32 wait_cnt = 0;
    xmedia_u32 reg;

    vi_reg_write_bit(CRG_REGS_ADDR + 0xfc, reset, 4); // 4 : viproc reset offset

    reg = vi_read_reg(CRG_REGS_ADDR + 0x1e4);
    reset_state = (reg & 0x80) >> 7; // bit[7]: viproc reset state

    while (reset_state != reset) {
        sleep_us(1);
        wait_cnt++;
        reg = vi_read_reg(CRG_REGS_ADDR + 0x1e4);
        reset_state = (reg & 0x80) >> 7; // bit[7]: viproc reset state
        if (reset_state != reset && wait_cnt > VI_RESET_WAIT_CNT) {
            puts("viproc reset failed\n");
            return XMEDIA_FAILURE;
        }
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 vi_vicap_all_reset(xmedia_bool reset)
{
    xmedia_bool reset_state = 0;
    xmedia_u32 reg;
    xmedia_u32 wait_cnt = 0;

    vi_reg_write_bit(CRG_REGS_ADDR + 0xf4, reset, 17); // [17] : vicap总软复位请求

    reg = vi_read_reg(CRG_REGS_ADDR + 0x1e4);
    reset_state = (reg & (0x1 << 21)) >> 21; // bit[21]: vicap all state

    while (reset_state != reset) {
        sleep_us(1);
        wait_cnt++;
        reg = vi_read_reg(CRG_REGS_ADDR + 0x1e4);
        reset_state = (reg & (0x1 << 21)) >> 21; // bit[21]: vicap all state
        if (reset_state != reset && wait_cnt > VI_RESET_WAIT_CNT) {
            puts("vicap reset failed\n");
            return XMEDIA_FAILURE;
        }
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 vi_vicap_clk_reset_init(xmedia_u32 cap_id)
{
    xmedia_s32 dev;
    xmedia_s32 fe;

    vi_reg_write_bit(CRG_REGS_ADDR + 0xf4, XMEDIA_TRUE, 0); // enable vicap clk

    sleep_us(5); // 等待时钟稳定

    if (vi_vicap_all_reset(XMEDIA_FALSE) != XMEDIA_SUCCESS) {
        return XMEDIA_FAILURE;
    }

    for (dev = 0; dev < VI_MAX_DEV_NUM; dev++) {
        if (vi_dev_reset(dev, XMEDIA_TRUE) != 0) {
            return XMEDIA_FAILURE;
        }
    }

    for (fe = 0; fe < VI_MAX_PHY_PIPE_NUM; fe++) {
        if (vi_vicap_fe_reset(fe, XMEDIA_TRUE) != 0) {
            return XMEDIA_FAILURE;
        }
    }

    for (dev = 0; dev < VI_MAX_DEV_NUM; dev++) {
        if (vi_dev_reset(dev, XMEDIA_FALSE) != 0) {
            return XMEDIA_FAILURE;
        }
    }

    for (fe = 0; fe < VI_MAX_PHY_PIPE_NUM; fe++) {
        if (vi_vicap_fe_reset(fe, XMEDIA_FALSE) != 0) {
            return XMEDIA_FAILURE;
        }
    }

    return XMEDIA_SUCCESS;
}

static xmedia_void vi_vicap_clk_reset_deinit(xmedia_u32 cap_id)
{
    xmedia_s32 dev;
    xmedia_s32 fe;

    for (dev = 0; dev < VI_MAX_DEV_NUM; dev++) {
        vi_dev_reset(dev, XMEDIA_TRUE);
    }

    for (fe = 0; fe < VI_MAX_PHY_PIPE_NUM; fe++) {
        vi_vicap_fe_reset(fe, XMEDIA_TRUE);
    }

    vi_vicap_all_reset(XMEDIA_TRUE);

    vi_reg_write_bit(CRG_REGS_ADDR + 0xf4, 0, 0);
}

static xmedia_s32 vi_viproc_clk_reset_init(xmedia_u32 proc_id)
{
    vi_reg_write_bit(CRG_REGS_ADDR + 0xfc, XMEDIA_TRUE, 0);

    sleep_us(5); // 等待时钟稳定

    if (vi_viproc_reset_set(proc_id, XMEDIA_FALSE) != 0) {
        puts("viproc clk reset false failed\n");
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_void vi_viproc_clk_reset_deinit(xmedia_u32 proc_id)
{
    vi_viproc_reset_set(proc_id, XMEDIA_TRUE);
    vi_viproc_reset_set(proc_id, XMEDIA_FALSE);
    vi_reg_write_bit(CRG_REGS_ADDR + 0xfc, XMEDIA_FALSE, 0);
}

STAGE1_FUNC static xmedia_void mipi_rx_set_misc_sel(xmedia_s32 dev_id, xmedia_vi_mipi_lane_divide_mode mipi_lane_num)
{
    volatile xmedia_u32 mipi_misc_reg = MIPI_MISC_CTRL;

    if (mipi_lane_num == XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE) {
        if (dev_id == 0) {
            vi_reg_write_bit(mipi_misc_reg, 1, 0);
        } else {
            vi_reg_write_bit(mipi_misc_reg, 1, 1);
        }
    } else {
        if (dev_id == 0 || dev_id == 1) {
            vi_reg_write_bit(mipi_misc_reg, 0, 0);
        } else {
            vi_reg_write_bit(mipi_misc_reg, 0, 1);
        }
    }
}

static xmedia_void mipi_rx_phy_get_lane_swap(xmedia_s32 dev_id, const xmedia_vi_dev_config *dev_config,
    mipi_lane_swap_attr *lane_swap_mode)
{
    /*
     * 默认phy与lane的链接关系:
     * mipi0-phy0-data0-lane0
     * mipi0-phy0-data1-lane1
     * mipi1-phy1-data0-lane2
     * mipi1-phy1-data1-lane3
     */
    if (dev_config->mipi_lane_num == XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE) {
        lane_swap_mode->phy0_data_lane0 = 0;
        lane_swap_mode->phy0_data_lane1 = 1;
        lane_swap_mode->phy1_data_lane0 = 2;
        lane_swap_mode->phy1_data_lane1 = 3;
        lane_swap_mode->phy0_clk_sel = 0;
        lane_swap_mode->phy1_clk_sel = 1;
    } else {
        if (dev_id == 0) {
            lane_swap_mode->phy0_data_lane0 = 0;
            lane_swap_mode->phy0_data_lane1 = 1;
            lane_swap_mode->phy0_clk_sel = 0;
        } else { // VI_DEV1
            lane_swap_mode->phy1_data_lane0 = 2;
            lane_swap_mode->phy1_data_lane1 = 3;
            lane_swap_mode->phy1_clk_sel = 0;
        }
    }

    if (dev_config->lane_config.enable != XMEDIA_TRUE) {
        return;
    }

    // 用户任意配置链接关系
    if (dev_config->mipi_lane_num == XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE) {
        lane_swap_mode->phy0_data_lane0 = dev_config->lane_config.lane_cfg[0];
        lane_swap_mode->phy0_data_lane1 = dev_config->lane_config.lane_cfg[1];
        lane_swap_mode->phy1_data_lane0 = dev_config->lane_config.lane_cfg[2];
        lane_swap_mode->phy1_data_lane1 = dev_config->lane_config.lane_cfg[3];
        lane_swap_mode->phy0_clk_sel = 0;
        lane_swap_mode->phy1_clk_sel = 1;
    } else {
        if (dev_id == 0) {
            lane_swap_mode->phy0_data_lane0 = dev_config->lane_config.lane_cfg[0];
            lane_swap_mode->phy0_data_lane1 = dev_config->lane_config.lane_cfg[1];
            lane_swap_mode->phy0_clk_sel = (lane_swap_mode->phy0_data_lane0 > 1) ? 1 : 0;
        } else {
            lane_swap_mode->phy1_data_lane0 = dev_config->lane_config.lane_cfg[0];
            lane_swap_mode->phy1_data_lane1 = dev_config->lane_config.lane_cfg[1];
            lane_swap_mode->phy1_clk_sel = (lane_swap_mode->phy1_data_lane0 > 1) ? 0 : 1;
        }
    }
}

static xmedia_void mipi_rx_phy_lane_swap(xmedia_s32 dev_id, xmedia_vi_mipi_lane_divide_mode lane_mode,
    mipi_lane_swap_attr *lane_swap)
{
    volatile xmedia_u32 mipi_phy_addr = g_mipi_phy_reg_addr;

    if (lane_mode == XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE) {
        vi_reg_write_32(mipi_phy_addr + 0xac, lane_swap->phy0_data_lane0, 0, 2);
        vi_reg_write_32(mipi_phy_addr + 0xac, lane_swap->phy0_data_lane1, 2, 2);
        vi_reg_write_32(mipi_phy_addr + 0xac, lane_swap->phy1_data_lane0, 4, 2);
        vi_reg_write_32(mipi_phy_addr + 0xac, lane_swap->phy1_data_lane1, 6, 2);
        vi_reg_write_32(mipi_phy_addr + 0xac, lane_swap->phy0_clk_sel, 8, 1);
        vi_reg_write_32(mipi_phy_addr + 0xac, lane_swap->phy1_clk_sel, 9, 1);
    } else if (lane_mode == XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE) {
        if (dev_id == 0) {
            vi_reg_write_32(mipi_phy_addr + 0xac, lane_swap->phy0_data_lane0, 0, 2);
            vi_reg_write_32(mipi_phy_addr + 0xac, lane_swap->phy0_data_lane1, 2, 2);
            vi_reg_write_32(mipi_phy_addr + 0xac, lane_swap->phy0_clk_sel, 8, 1);
        } else { // VI_DEV1
            vi_reg_write_32(mipi_phy_addr + 0xac, lane_swap->phy1_data_lane0, 4, 2);
            vi_reg_write_32(mipi_phy_addr + 0xac, lane_swap->phy1_data_lane1, 6, 2);
            vi_reg_write_32(mipi_phy_addr + 0xac, lane_swap->phy1_clk_sel, 9, 1);
        }
    } else {
        if (dev_id == 0) {
            vi_reg_write_32(mipi_phy_addr + 0xac, lane_swap->phy0_data_lane0, 0, 2);
            vi_reg_write_32(mipi_phy_addr + 0xac, lane_swap->phy0_clk_sel, 8, 1);
        } else { // VI_DEV1
            vi_reg_write_32(mipi_phy_addr + 0xac, lane_swap->phy1_data_lane0, 4, 2);
            vi_reg_write_32(mipi_phy_addr + 0xac, lane_swap->phy1_clk_sel, 9, 1);
        }
    }
}

STAGE1_FUNC static xmedia_void mipi_rx_phy_init(xmedia_s32 dev_id, xmedia_u32 mipi_rate,
                                                xmedia_vi_mipi_lane_divide_mode mipi_lane_num,
                                                mipi_lane_swap_attr *lane_swap)
{
    volatile xmedia_u32 mipi_phy_addr = g_mipi_phy_reg_addr;
    xmedia_u32 data_rate;
    xmedia_u32 td_term_en_max;
    xmedia_u32 td_term_en;
    xmedia_u32 ths_settle_max;
    xmedia_u32 ths_settle;
    xmedia_u32 tclk_term_en_max;
    xmedia_u32 tclk_term_en;
    xmedia_u32 tclk_settle_max;
    xmedia_u32 tclk_settle;
    xmedia_u32 tx_clk_esc = 24; // 24MHz 晶振配置 (T-tx_clk_esc = 41.7 ns)

    if (mipi_rate == 0) {
        puts("mipi_rate invalid");
        puts("\n");
    }

    if (mipi_lane_num == XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE) {
        vi_reg_write_bit(mipi_phy_addr, 1, 0); // mipirxphy_sel
    } else {
        vi_reg_write_bit(mipi_phy_addr, 0, 0); // mipirxphy_sel
    }

    mipi_rx_phy_lane_swap(dev_id, mipi_lane_num, lane_swap);

    data_rate = mipi_rate;
    td_term_en_max = 35 + 4000 / data_rate;
    td_term_en = td_term_en_max * data_rate / 2000 - 1;
    ths_settle_max = 145 + 10 * 1000 / data_rate;
    ths_settle = (ths_settle_max - td_term_en * 2000 / data_rate) * data_rate / 2000 - 1;
    tclk_term_en_max = 38;
    tclk_term_en = (tx_clk_esc * tclk_term_en_max) / 1000;
    tclk_settle_max = 300;
    tclk_settle = (tclk_settle_max - tclk_term_en * 1000 / tx_clk_esc) * tx_clk_esc / 1000 - 1;

    if (dev_id == 0) {
        vi_reg_write_32(mipi_phy_addr + 0xb4, td_term_en, 24, 8); // time_hs_term_en
        vi_reg_write_32(mipi_phy_addr + 0xb4, ths_settle, 16, 8); // time_hs_settle
        vi_reg_write_32(mipi_phy_addr + 0xb4, tclk_term_en, 8, 8); // time_ck_term_en
        vi_reg_write_32(mipi_phy_addr + 0xb4, tclk_settle, 0, 8); // time_ck_settle
        vi_reg_write_bit(mipi_phy_addr + 0xb0, XMEDIA_TRUE, 0); // cr_csi_en
        vi_reg_write_bit(mipi_phy_addr + 0xb0, XMEDIA_TRUE, 1); // dl0_enable
        vi_reg_write_bit(mipi_phy_addr + 0xb0, XMEDIA_TRUE, 2); // dl1_enable
    } else if (dev_id == 1) {
        vi_reg_write_32(mipi_phy_addr + 0xd4, td_term_en, 24, 8); // time_hs_term_en
        vi_reg_write_32(mipi_phy_addr + 0xd4, ths_settle, 16, 8); // time_hs_settle
        vi_reg_write_32(mipi_phy_addr + 0xd4, tclk_term_en, 8, 8); // time_ck_term_en
        vi_reg_write_32(mipi_phy_addr + 0xd4, tclk_settle, 0, 8); // time_ck_settle
        vi_reg_write_bit(mipi_phy_addr + 0xd0, XMEDIA_TRUE, 0); // cr_csi_en
        vi_reg_write_bit(mipi_phy_addr + 0xd0, XMEDIA_TRUE, 1); // dl0_enable
        vi_reg_write_bit(mipi_phy_addr + 0xd0, XMEDIA_TRUE, 2); // dl1_enable
    } else {
        puts("mipi phy num err, dev_id ");
        putdec(dev_id);
        puts("\n");
    }
}

STAGE1_GLOBAL static xmedia_u32 ret_mipi_date_type[10] = { 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x27, 0x18, 0x1a, 0x1e };

STAGE1_FUNC xmedia_u32 mipi_rx_get_data_type(xmedia_intf_mipi_csi_data_type data_type)
{
    return ret_mipi_date_type[data_type];
}

STAGE1_FUNC xmedia_void mipi_rx_set_ipi_reg(xmedia_s32 dev_id, xmedia_intf_mipi_csi_data_type data_type)
{
    volatile unsigned int mipi_rx_addr = g_mipi_rx_reg[dev_id];
    xmedia_u32 ipi_data_type;

    vi_reg_write_bit(mipi_rx_addr + 0x80, 1, 24); // ipi enable
    vi_reg_write_bit(mipi_rx_addr + 0x80, 1, 16); // ipi_cut_through

    // ipi_color_com
    if (data_type <= XMEDIA_INTF_MIPI_CSI_DATA_TYPE_RAW_16BIT) {
        vi_reg_write_bit(mipi_rx_addr + 0x80, 1, 8);
    } else {
        vi_reg_write_bit(mipi_rx_addr + 0x80, 0, 8);
    }

    vi_reg_write_32(mipi_rx_addr + 0x84, 0, 0, 2); // ipi vc id

    ipi_data_type = mipi_rx_get_data_type(data_type);
    vi_reg_write_32(mipi_rx_addr + 0x88, ipi_data_type, 0, 6); // ipi data type
    vi_reg_write_bit(mipi_rx_addr + 0x88, 1, 8); // embedded_data[A]

    vi_reg_write_bit(mipi_rx_addr + 0x8c, 1, 8); // ipi mem auto flush
    vi_reg_write_32(mipi_rx_addr + 0x90, 0xa, 0, 12); // hsa
    vi_reg_write_32(mipi_rx_addr + 0x94, 0xa, 0, 12); // hbp
    vi_reg_write_32(mipi_rx_addr + 0x98, 0xa, 0, 12); // hsd

    vi_reg_write_32(mipi_rx_addr + 0x9c, 0x898, 0, 15); // hline_time
    vi_reg_write_bit(mipi_rx_addr + 0xa0, 1, 0); // soft reset

    // advanced feature
    vi_reg_write_bit(mipi_rx_addr + 0xac, 0, 0);
    vi_reg_write_32(mipi_rx_addr + 0xac, 0, 8, 6);
    vi_reg_write_bit(mipi_rx_addr + 0xac, 1, 16);
    vi_reg_write_bit(mipi_rx_addr + 0xac, 1, 17);
    vi_reg_write_bit(mipi_rx_addr + 0xac, 1, 18);
    vi_reg_write_bit(mipi_rx_addr + 0xac, 1, 19);
    vi_reg_write_bit(mipi_rx_addr + 0xac, 1, 20);
    vi_reg_write_bit(mipi_rx_addr + 0xac, 1, 21);
    vi_reg_write_bit(mipi_rx_addr + 0xac, 1, 22);
    vi_reg_write_bit(mipi_rx_addr + 0xac, 1, 24);
}

xmedia_u32 vi_get_bit_width(xmedia_video_data_width data_width)
{
    xmedia_u32 bit_width;

    switch (data_width) {
        case XMEDIA_VIDEO_DATA_WIDTH_8:
            bit_width = 8;
            break;

        case XMEDIA_VIDEO_DATA_WIDTH_10:
            bit_width = 10;
            break;

        case XMEDIA_VIDEO_DATA_WIDTH_12:
            bit_width = 12;
            break;

        case XMEDIA_VIDEO_DATA_WIDTH_14:
            bit_width = 14;
            break;

        case XMEDIA_VIDEO_DATA_WIDTH_16:
            bit_width = 16;
            break;

        default:
            bit_width = 16;
            break;
    }

    return bit_width;
}

xmedia_u32 vi_viproc_get_bit_deepth(xmedia_video_data_width bit_width, xmedia_video_wdr_mode wdr_mode)
{
    if (wdr_mode == XMEDIA_VIDEO_WDR_MODE_NONE) {
        return VI_PACK_12BIT;
    } else if (wdr_mode == XMEDIA_VIDEO_WDR_MODE_BUILT_IN) {
        if (bit_width == XMEDIA_VIDEO_DATA_WIDTH_8) {
            return VI_PACK_8BIT;
        } else if (bit_width == XMEDIA_VIDEO_DATA_WIDTH_10) {
            return VI_PACK_10BIT;
        } else if (bit_width == XMEDIA_VIDEO_DATA_WIDTH_12) {
            return VI_PACK_12BIT;
        } else if (bit_width == XMEDIA_VIDEO_DATA_WIDTH_14) {
            return VI_PACK_14BIT;
        } else if (bit_width == XMEDIA_VIDEO_DATA_WIDTH_16) {
            return VI_PACK_16BIT;
        } else {
            puts("viproc built-in wdr mode not support bit_width ");
            putdec(bit_width);
            puts("\n");
            return VI_PACK_12BIT;
        }
    } else {
        puts("viproc not support wdr_mode ");
        putdec(wdr_mode);
        puts("\n");
        return VI_PACK_12BIT;
    }
}

xmedia_void hal_vi_vicap_reg_newer(xmedia_s32 fe_id)
{
    xmedia_u32 vicap_addr = g_vicap_all_reg[0];

    vi_reg_write_bit(vicap_addr + ((fe_id + 6) * 0x1000 + 0x48), 1, 0);
}

xmedia_void hal_vi_vicap_online_config(xmedia_s32 fe_id, xmedia_vi_pipe_config *pipe_config)
{
    xmedia_u32 vicap_addr = g_vicap_all_reg[0];
    xmedia_u32 bit_width;

    // input_size
    vi_reg_write_32(vicap_addr + ((fe_id + 6) * 0x1000 + 0x4), pipe_config->width, 0, 14);
    vi_reg_write_32(vicap_addr + ((fe_id + 6) * 0x1000 + 0x4), pipe_config->height, 16, 14);

    // bit_width
    bit_width = vi_get_bit_width(pipe_config->bit_width);
    vi_reg_write_32(vicap_addr + ((fe_id + 6) * 0x1000 + 0x28), bit_width, 16, 6);

    // output_size
    vi_reg_write_32(vicap_addr + ((fe_id + 6) * 0x1000 + 0x2c), pipe_config->width, 0, 14);
    vi_reg_write_32(vicap_addr + ((fe_id + 6) * 0x1000 + 0x2c), pipe_config->height, 16, 14);

    // vicap reg newer
    hal_vi_vicap_reg_newer(fe_id);

    // todo offline set_output_memory
    if (pipe_config->vi_work_mode == 1) {
    }
}

xmedia_u32 hal_vi_vicap_get_top_int_status(xmedia_u32 vicap_id)
{
    xmedia_u32 top_int_status;
    xmedia_u32 vicap_addr = g_vicap_all_reg[vicap_id];

    top_int_status = vi_read_reg(vicap_addr + 0x10);

    return top_int_status;
}

xmedia_u32 hal_vi_vicap_get_top_int_mask(xmedia_u32 vicap_id)
{
    xmedia_u32 top_int_mask;
    xmedia_u32 vicap_addr = g_vicap_all_reg[vicap_id];

    top_int_mask = vi_read_reg(vicap_addr + 0x14);

    return top_int_mask;
}

xmedia_u32 hal_vi_vicap_get_chn_int_status(xmedia_s32 fe_id)
{
    xmedia_u32 irq_status;
    xmedia_u32 vicap_addr = g_vicap_all_reg[0];

    irq_status = vi_read_reg(vicap_addr + ((fe_id + 6) * 0x1000 + 0x40));
    return irq_status;
}

xmedia_void hal_vi_vicap_clear_int(xmedia_s32 fe_id, xmedia_u32 mask)
{
    xmedia_u32 vicap_addr = g_vicap_all_reg[0];

    vi_write_reg(vicap_addr + ((fe_id + 6) * 0x1000 + 0x40), mask); // clear int
}
xmedia_s32 hal_vi_get_vicap_irq_num(xmedia_u32 vicap_id)
{
    return VICAP_IRQ_NUM;
}

xmedia_s32 hal_vi_get_viproc_irq_num(xmedia_u32 viproc_id)
{
    return VIPROC0_IRQ_NUM;
}

xmedia_u32 hal_viproc_get_reg_public_int_status(xmedia_u32 viproc_id)
{
    xmedia_u32 int_status;
    xmedia_u32 viproc_addr = g_viproc_all_reg[viproc_id];

    int_status = vi_read_reg(viproc_addr + 0x48);
    return int_status;
}

xmedia_u32 hal_viproc_get_reg_public_int_mask(xmedia_u32 viproc_id)
{
    xmedia_u32 int_mask;
    xmedia_u32 viproc_addr = g_viproc_all_reg[viproc_id];

    int_mask = vi_read_reg(viproc_addr + 0x2c);
    return int_mask;
}

xmedia_u32 hal_viproc_common_get_reg_early_mask(xmedia_u32 viproc_id)
{
    xmedia_u32 int_mask;
    xmedia_u32 viproc_addr = g_viproc_all_reg[viproc_id];

    int_mask = vi_read_reg(viproc_addr + 0x1e8);
    return int_mask;
}

xmedia_void hal_viproc_clr_reg_int(xmedia_u32 viproc_id, xmedia_u32 mask)
{
    xmedia_u32 viproc_addr = g_viproc_all_reg[viproc_id];

    vi_write_reg(viproc_addr + 0x48, mask);
}

STAGE1_FUNC xmedia_void hal_mipi_init(xmedia_void)
{
    vi_reg_write_32(CRG_REGS_ADDR + 0xa0, 0, 0, 3); // bit[0:2]: mipi_rx_clk_sel
}

STAGE1_FUNC xmedia_s32 hal_mipi_enable(xmedia_s32 dev, xmedia_vi_dev_config *dev_config)
{
    xmedia_u32 mipi_lane_num = 0;
    xmedia_u32 mipi_rx_addr = g_mipi_rx_reg[dev];
    xmedia_u32 sys_config_addr = CRG_REGS_ADDR + 0xf8;
    mipi_lane_swap_attr lane_swap_mode = { 0 };

    // mipi_rx mipi_phy clk enable
    vi_reg_write_bit(sys_config_addr, XMEDIA_TRUE, dev); // mipi_rx_cken
    if (dev_config->mipi_lane_num == XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE) {
        vi_reg_write_bit(sys_config_addr, XMEDIA_TRUE, 3); // mipi_rx0_4line_cken
    }
    vi_reg_write_bit(sys_config_addr, XMEDIA_TRUE, 9); // mipi_phy0_cken

    // 延时5 us，等待时钟稳定
    sleep_us(5);

    // mipi_rx复位
    vi_reg_write_bit(mipi_rx_addr + 0x8, XMEDIA_FALSE, 0);
    sleep_us(5);
    vi_reg_write_bit(mipi_rx_addr + 0x8, XMEDIA_TRUE, 0);

    if (dev_config->mipi_lane_num == XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_1_LANE) {
        mipi_lane_num = 0x0;
    } else if (dev_config->mipi_lane_num == XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_2_LANE) {
        mipi_lane_num = 0x1;
    } else if (dev_config->mipi_lane_num == XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE) {
        mipi_lane_num = 0x3;
    } else {
        // mipi_lane错误参数
        puts("vi mipi lane is invalid\n");
    }

    vi_reg_write_32(mipi_rx_addr + 0x4, mipi_lane_num, 0, 3); // mipi_lane
    vi_reg_write_bit(mipi_rx_addr + 0x44, 1, 0); // mipi0_dphy_rstz
    vi_reg_write_bit(mipi_rx_addr + 0x40, 1, 0); // mipi0_phy_shutdownz

    mipi_rx_set_misc_sel(dev, dev_config->mipi_lane_num);

    mipi_rx_phy_get_lane_swap(dev, dev_config, &lane_swap_mode);
    mipi_rx_phy_init(dev, dev_config->mipi_rate, dev_config->mipi_lane_num, &lane_swap_mode);
    if (dev_config->mipi_lane_num == XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE) {
        mipi_rx_phy_init(dev + 1, dev_config->mipi_rate, dev_config->mipi_lane_num, &lane_swap_mode);
    }

    sleep_us(5);

    mipi_rx_set_ipi_reg(dev, dev_config->data_type); // ipi cfg, todo:暂不考虑wdr
    vi_reg_write_32(mipi_rx_addr + 0x640, 0, 0, 3); // hdr_data_mode,0:VC
    vi_reg_write_32(mipi_rx_addr + 0x654, 0x142, 0, 16); // rft_speeder

    return XMEDIA_SUCCESS;
}

xmedia_void hal_vi_init(xmedia_void)
{
    xmedia_u32 vicap_addr = g_vicap_all_reg[0];
    xmedia_u32 viproc_addr = g_viproc_all_reg[0];

    // 初始化mipi_rx、vicap、viproc时钟
    //    vi_reg_write_32(CRG_REGS_ADDR + 0xa0, 0, 0, 3); // bit[0:2]: mipi_rx_clk_sel
    vi_reg_write_32(CRG_REGS_ADDR + 0xa0, 0, 3, 3); // bit[3:5]: vicap_clk_sel
    vi_reg_write_32(CRG_REGS_ADDR + 0xa0, 0, 6, 3); // bit[6:8]: viproc_clk_sel

    // vicap时钟使能，dev、fe复位
    vi_vicap_clk_reset_init(0);

    // viproc时钟使能、复位，init开启时钟，isp_init需要配置viproc算法寄存器
    vi_viproc_clk_reset_init(0);

    // vicap viproc outstanding
    vi_reg_write_32(vicap_addr + 0x74, 0x88, 0, 9);
    vi_reg_write_32(viproc_addr + 0x10, 0x10, 8, 5); // wr outstanding
    vi_reg_write_32(viproc_addr + 0x10, 0x20, 16, 6); // rd outstanding

    // 屏蔽中断
    vi_reg_write_32(vicap_addr + 0x14, 0x7fffff, 0, 23); // top mask
}

xmedia_void hal_vi_exit(xmedia_void)
{
    // viproc时钟去使能，dev、fe复位
    //    vi_viproc_clk_reset_deinit(0);

    // vicap时钟去使能，dev、fe复位
    vi_vicap_clk_reset_deinit(0);
}

xmedia_s32 hal_vi_enable_dev(xmedia_s32 dev)
{
    xmedia_u32 vicap_addr = g_vicap_all_reg[0];

    // pt intf mode，todo ：其他接口待适配
    if (dev == 0) {
        vi_reg_write_32(vicap_addr + 0xc, 0, 0, 2);
    } else if (dev == 1) {
        vi_reg_write_32(vicap_addr + 0xc, 0, 2, 2);
    } else if (dev == 2) {
        vi_reg_write_32(vicap_addr + 0xc, 0, 4, 3);
    } else {
        puts("dev_id is err ");
        putdec(dev);
        puts("\n");
    }

    // enable pt
    vi_reg_write_bit(vicap_addr, XMEDIA_TRUE, dev);

    if (dev < 2) {
        vi_reg_write_bit(vicap_addr + ((dev + 1) * 0x1000), XMEDIA_TRUE, 0); // pt_timing_en
        vi_reg_write_bit(vicap_addr + ((dev + 1) * 0x1000), XMEDIA_TRUE, 4); // debug_en
    } else {
        vi_reg_write_bit(vicap_addr + ((dev + 1) * 0x1000), XMEDIA_TRUE, 0); // pt_timing_en
    }

    return XMEDIA_SUCCESS;
}

xmedia_void hal_vi_disable_dev(xmedia_s32 dev, xmedia_vi_dev_config *dev_config)
{
    xmedia_u32 vicap_addr = g_vicap_all_reg[0];

#if 0
    xmedia_u32 mipi_rx_addr = g_mipi_rx_reg[dev];
    xmedia_u32 sys_config_addr = CRG_REGS_ADDR + 0xf8;

    vi_reg_write_bit(mipi_rx_addr + 0x44, 0, 0); //mipi0_dphy_rstz
    vi_reg_write_bit(mipi_rx_addr + 0x40, 0, 0); //mipi0_phy_shutdownz

    // mipi_rx mipi_phy clk disable
    vi_reg_write_bit(sys_config_addr, 0, dev); // diable mipi_rx clk
    vi_reg_write_bit(sys_config_addr, 0, 9 + dev); // diable mipi_rx phy clk
    if (dev_config->mipi_lane_num == XMEDIA_VI_MIPI_LANE_DIVIDE_MODE_4_LANE) {
        vi_reg_write_bit(sys_config_addr, 0, 9 + dev + 1);
    }
#endif

    // disable pt
    vi_reg_write_bit(vicap_addr, XMEDIA_FALSE, dev);

    if (dev < 2) {
        vi_reg_write_bit(vicap_addr + ((dev + 1) * 0x1000), XMEDIA_FALSE, 0); // pt_timing_en
        vi_reg_write_bit(vicap_addr + ((dev + 1) * 0x1000), XMEDIA_FALSE, 4); // debug_en
        vi_reg_write_bit(vicap_addr + ((dev + 1) * 0x1000), XMEDIA_FALSE, 2); // ck_dyn_gt_en
    } else {
        vi_reg_write_bit(vicap_addr + ((dev + 1) * 0x1000), XMEDIA_FALSE, 0); // pt_timing_en
        vi_reg_write_bit(vicap_addr + ((dev + 1) * 0x1000) + 0x2c, XMEDIA_FALSE, 0); // ck_dyn_gt_en
    }
}

xmedia_s32 hal_vi_set_dev_bind_pipe(xmedia_s32 dev, xmedia_s32 fe_id)
{
    xmedia_u32 vicap_addr = g_vicap_all_reg[0];

    vi_reg_write_32(vicap_addr + 0x4, dev, (fe_id * 3), 3);

    return XMEDIA_SUCCESS;
}

xmedia_void hal_vi_set_dev_unbind_pipe(xmedia_s32 dev, xmedia_s32 fe_id)
{
    xmedia_u32 vicap_addr = g_vicap_all_reg[0];

    vi_reg_write_32(vicap_addr + 0x4, 0, (fe_id * 3), 3);
}

xmedia_s32 hal_vi_start_vicap(xmedia_s32 fe_id, xmedia_vi_pipe_config *pipe_config)
{
    xmedia_u32 vicap_addr = g_vicap_all_reg[0];

    vi_vicap_fe_reset(fe_id, XMEDIA_FALSE);

    // 打开对应fe上报中断开关、中断掩码
    vi_reg_write_bit(vicap_addr + 0x14, 0, fe_id + 5);
    vi_reg_write_32(vicap_addr + ((fe_id + 6) * 0x1000 + 0x3c), 0x6, 0, 16);

    // 在线离线工作模式
    if (pipe_config->vi_work_mode == 0) {
        vi_reg_write_bit(vicap_addr + ((fe_id + 6) * 0x1000), XMEDIA_TRUE, 0); // online_en
        vi_reg_write_bit(VIPROC_EXTERNAL_TIMING_EN_ADDR, XMEDIA_TRUE, 1); // viproc_external_timing_en
    } else {
        vi_reg_write_bit(vicap_addr + ((fe_id + 6) * 0x1000), 0, 0);
        vi_reg_write_bit(VIPROC_EXTERNAL_TIMING_EN_ADDR, XMEDIA_FALSE, 1);
    }

    // 设置early行号、early位置
    vi_reg_write_32(vicap_addr + ((fe_id + 6) * 0x1000 + 0x1c), 1, 0, 2);
    vi_reg_write_32(vicap_addr + ((fe_id + 6) * 0x1000 + 0x1c), (pipe_config->height * 4 / 5), 8, 13);

    // clear_int
    vi_reg_write_32(vicap_addr + ((fe_id + 6) * 0x1000 + 0x40), 0xffff, 0, 16);

    hal_vi_vicap_online_config(fe_id, pipe_config);

    // enable fe
    vi_reg_write_bit(vicap_addr, XMEDIA_TRUE, fe_id + 5);

    return XMEDIA_SUCCESS;
}

xmedia_void hal_vi_viproc_reg_start(xmedia_u32 viproc_id)
{
    xmedia_u32 viproc_addr = g_viproc_all_reg[viproc_id];

    vi_reg_write_bit(viproc_addr + 0x14, XMEDIA_TRUE, 0); // ctl_up_finish_enable
    vi_reg_write_bit(viproc_addr + 0x14, XMEDIA_TRUE, 4); // filter_up_finish_enable
}

xmedia_void hal_vi_viproc_online_config(xmedia_s32 fe_id, xmedia_vi_pipe_config *pipe_config)
{
    xmedia_u32 bit_deepth;
    xmedia_u32 viproc_addr = g_viproc_all_reg[0];

    // inch0_path_sel
    vi_reg_write_32(viproc_addr + 0x174, 0, 1, 2);

    // list_mode_enable false
    vi_reg_write_bit(viproc_addr + 0x14, 0, 8);

    // early line/location
    vi_reg_write_32(viproc_addr + 0x1dc, 1, 4, 2);
    vi_reg_write_32(viproc_addr + 0x1e0, (pipe_config->height * 4 / 5), 16, 14);

    // inch0_en
    vi_reg_write_bit(viproc_addr + 0x174, XMEDIA_TRUE, 0);

    // inch0_bit_deepth
    bit_deepth = vi_viproc_get_bit_deepth(pipe_config->bit_width, pipe_config->wdr_mode);
    vi_reg_write_32(viproc_addr + 0x174, bit_deepth, 16, 5);

    // input_data_mode
    vi_reg_write_bit(viproc_addr + 0x16c, 0, 3);

    // valid_in_width/height
    vi_reg_write_32(viproc_addr + 0x190, pipe_config->width, 0, 14);
    vi_reg_write_32(viproc_addr + 0x190, pipe_config->height, 16, 14);

    // early mask
    vi_reg_write_32(viproc_addr + 0x1e8, 0x6, 0, 3);
    // todo:node id
}

xmedia_s32 hal_vi_start_viproc(xmedia_s32 fe_id, xmedia_vi_pipe_config *pipe_config)
{
    xmedia_u32 viproc_addr = g_viproc_all_reg[0];

//    vi_viproc_clk_reset_init(0);

    // viproc 中断掩码
    vi_reg_write_32(viproc_addr + 0x2c, 0x20, 0, 32);

    // viproc clear int
    vi_reg_write_32(viproc_addr + 0x48, 0xffffffff, 0, 32);

    hal_vi_viproc_online_config(fe_id, pipe_config);

    // ctl_up_finish_enable/filter_up_finish_enable
    hal_vi_viproc_reg_start(0);

    return XMEDIA_SUCCESS;
}

xmedia_void hal_vi_stop_viproc(xmedia_s32 fe_id)
{
    vi_viproc_clk_reset_deinit(0);
}

xmedia_void hal_vi_stop_vicap(xmedia_s32 fe_id)
{
    xmedia_u32 vicap_addr = g_vicap_all_reg[0];

    // disable fe
    vi_reg_write_bit(vicap_addr, XMEDIA_FALSE, fe_id + 5);

    // ck_dyn_gt_en_enable
    vi_reg_write_bit(vicap_addr + ((fe_id + 6) * 0x1000 + 0xa0), XMEDIA_FALSE, 0);

    // int none mask
    vi_reg_write_32(vicap_addr + ((fe_id + 6) * 0x1000 + 0x3c), 0xffff, 0, 16);

    // clear int
    vi_reg_write_32(vicap_addr + ((fe_id + 6) * 0x1000 + 0x40), 0xffff, 0, 16);

    vi_vicap_fe_reset(fe_id, XMEDIA_TRUE);
    vi_vicap_fe_reset(fe_id, XMEDIA_FALSE);
}