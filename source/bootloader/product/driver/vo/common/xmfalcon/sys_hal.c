/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include "drv_reg_crg.h"
#include "drv_reg_sys.h"
#include "sys_hal.h"
#include "xmedia_vo.h"
#include "drv_vo_comm.h"

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */


#define DDRC0_REG_ADDR            0x120d0000 /* base addr of DDRCB */
#define DDRC_REGS_SIZE            0x10000

#define SCT_REGS_ADDR             0x12050000 /* base addr of syscnt */
#define SCT_REGS_SIZE             0x100

#define OTP_REGS_ADDR             0x100A0000
#define OTP_REGS_SIZE             0X10000

#define APLL_VO_VCO_MAX 3700
#define APLL_VO_VCO_MIN 1000

void *g_reg_crg_base_va = (void *)CRG_REGS_ADDR;
void *g_reg_sys_base_va = (void *)SYS_REGS_ADDR;
void *g_reg_ddr0_base_va = (void *)DDRC0_REG_ADDR;
void *g_reg_misc_base_va = (void *)MISC_REGS_ADDR;
void *g_reg_otp_base_va = (void *)OTP_REGS_ADDR;
void *g_sct_addr = (void *)SCT_REGS_ADDR;

xmedia_u32 g_sys_efuse_addr = 0;
xmedia_u32 g_sys_chip_addr = 0;

#define IO_CRG_ADDRESS(x) ((uintptr_t)g_reg_crg_base_va + ((x) - (CRG_REGS_ADDR)))
#define IO_SYS_ADDRESS(x) ((uintptr_t)g_reg_sys_base_va + ((x) - (SYS_REGS_ADDR)))
#define IO_DDR0_ADDRESS(x) ((uintptr_t)g_reg_ddr0_base_va + ((x) - (DDRC0_REG_ADDR)))
#define IO_MISC_ADDRESS(x) ((uintptr_t)g_reg_misc_base_va + ((x) - (MISC_REGS_ADDR)))

static xmedia_u32 hal_vo_sys_reg_read(volatile xmedia_uintptr_t addr)
{
    return *(volatile xmedia_u32 *)(addr);
}

static xmedia_void hal_vo_sys_reg_write(volatile xmedia_uintptr_t addr, xmedia_u32 val)
{
    *(volatile xmedia_u32 *)(addr) = val;
    return;
}


static inline void sys_reg_set_bit(unsigned long value, unsigned long offset, unsigned long addr)
{
    unsigned long t, mask;

    mask = 1 << offset;
    t = hal_vo_sys_reg_read((xmedia_uintptr_t)addr);
    t &= ~mask;
    t |= (value << offset) & mask;
    hal_vo_sys_reg_write ( (xmedia_uintptr_t)addr, t);
}

static inline void sys_reg_write_32(unsigned long value, unsigned long mask, unsigned long addr)
{
    unsigned long t;

    t = hal_vo_sys_reg_read((xmedia_uintptr_t)addr);
    t &= ~mask;
    t |= value & mask;
    hal_vo_sys_reg_write((xmedia_uintptr_t)addr, t);
}

static inline void sys_reg_read(unsigned int *pvalue, unsigned long addr)
{
    *pvalue = hal_vo_sys_reg_read((uintptr_t)addr);
}

xmedia_s32 sys_hal_usrdef_slv_enable(xmedia_bool enable)
{
    if (enable == XMEDIA_TRUE) {
        sys_reg_set_bit(1, 1, IO_CRG_ADDRESS(CRG_PERCTL50_ADDR)); // 打开时钟
        sys_reg_set_bit(0, 0, IO_CRG_ADDRESS(CRG_PERCTL50_ADDR)); // 撤消复位
    } else if (enable == XMEDIA_FALSE) {
        sys_reg_set_bit(0, 1, IO_CRG_ADDRESS(CRG_PERCTL50_ADDR)); // 关闭时钟
    } else {
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}


xmedia_void sys_hal_vo_vou_hd_sel(xmedia_s32 vo_dev, xmedia_u32 vou_hd_sel)
{
    if(vo_dev == 0) {
        sys_reg_write_32(vou_hd_sel << 1 , 0xf << 1, IO_CRG_ADDRESS(CRG_PERCTL67_ADDR));
    } else if (vo_dev == 1) {
        sys_reg_write_32(vou_hd_sel << 13 , 0xf << 13, IO_CRG_ADDRESS(CRG_PERCTL67_ADDR));
    } else {
    }
}

xmedia_void sys_hal_vo_vou_ppc_cksel(xmedia_u32 vou_ppc_cksel)
{
    sys_reg_write_32(vou_ppc_cksel << 24 , 0x3 << 24, IO_CRG_ADDRESS(CRG_PERCTL67_ADDR));
}

xmedia_void sys_hal_vo_lvds_por_srst(xmedia_bool lvd_por_srst)
{
    xmedia_u32 tmp = (lvd_por_srst == XMEDIA_TRUE) ? 1 : 0;

    sys_reg_set_bit(tmp, 21, IO_CRG_ADDRESS(CRG_PERCTL66_ADDR));

}
xmedia_void sys_hal_vo_mipitx_pll_ref_cksel(xmedia_bool mipitx_pll_ref_cksel)
{
    xmedia_u32 tmp = (mipitx_pll_ref_cksel == XMEDIA_TRUE) ? 1 : 0;

    sys_reg_set_bit(tmp, 20, IO_CRG_ADDRESS(CRG_PERCTL66_ADDR));
}

static inline xmedia_u64 vo_div64_u64(xmedia_u64 dividend, xmedia_u64 divisor)
{
	return dividend / divisor;
}

xmedia_void sys_hal_vo_lcd_div_cfg(xmedia_u32 lcd_div_cfg)
{
    xmedia_u32 lcd_clk_div = (xmedia_u32)vo_div64_u64(vo_div64_u64((xmedia_u64)lcd_div_cfg , 1375) * (1 << 27),
        1000*1000);
    sys_reg_set_bit(0, 28, IO_CRG_ADDRESS(CRG_PERCTL65_ADDR));

    sys_reg_write_32(lcd_clk_div, 0x7ffffff, IO_CRG_ADDRESS(CRG_PERCTL65_ADDR));
}

static xmedia_s32 hal_vo_find_fout_div(xmedia_u32 *fout0_div, xmedia_u32 *fout1_div, xmedia_u32 pll_div,
    xmedia_u32 *pll_vco){
    xmedia_u32 fout0;
    xmedia_u32 fout1;
    xmedia_u32 vco;

    //fout0 fout1 取值范围都是0 - 15
    for(fout0 = 0;fout0 < 16;fout0++) {
        for(fout1 = 0;fout1 < 16;fout1++) {
            vco = pll_div * (fout0 + 1) * (fout1 + 1);
            //vco的取值范围是950 - 3800，但ic要求不要取临近值，所以这里使用1000 - 3700
            if((vco > (APLL_VO_VCO_MIN * 1000)) && (vco < (APLL_VO_VCO_MAX * 1000))) {
                *fout0_div = fout0;
                *fout1_div = fout1;
                *pll_vco = vco;
                return XMEDIA_SUCCESS;
            }
        }
    }
    return XMEDIA_FAILURE;
}

xmedia_s32 sys_hal_vo_pll_div_cfg(xmedia_u32 pll_div_cfg)
{
    xmedia_u32 apll_frac;
    xmedia_u32 apll_fbdiv;
    xmedia_u32 fout0_div;
    xmedia_u32 fout1_div;
    xmedia_u32 vco;
    xmedia_u32 pll_div;

    //为避免超出u32范围 对1000取整计算
    pll_div = pll_div_cfg / 1000;

    //理论不会存在获取不到分频因子的情况
    if(hal_vo_find_fout_div(&fout0_div, &fout1_div, pll_div, &vco) != XMEDIA_SUCCESS){
        return XMEDIA_FAILURE;
    }

    //apll倍频整数部分
    apll_fbdiv = (vco / 24) / 1000;
    //apll倍频小数部分
    apll_frac = (vco / 24) % 1000;
    apll_frac = (apll_frac * ((1 << 24) / 1000));

    sys_reg_write_32(apll_frac, 0xffffff, IO_CRG_ADDRESS(CRG_PERCTL0_ADDR));

    // apll 小数/整数分频控制 默认为0 小数分频
    sys_reg_set_bit(0, 23, IO_CRG_ADDRESS(CRG_PERCTL1_ADDR));

    //除了fout1 fout0 其他都bypass
    sys_reg_write_32(0xc << 18, 0xf << 18, IO_CRG_ADDRESS(CRG_PERCTL1_ADDR));

    //参考时钟分频系数默认设置为1 apll参考时钟则为24mhz
    sys_reg_write_32(0x1 << 12, 0x3f << 12, IO_CRG_ADDRESS(CRG_PERCTL1_ADDR));

    sys_reg_write_32(apll_fbdiv, 0xfff, IO_CRG_ADDRESS(CRG_PERCTL1_ADDR));

    //APLL 配置fout0 fout1 分频因子
    sys_reg_write_32(fout0_div | (fout1_div << 4), 0xffff, IO_CRG_ADDRESS(CRG_PERCTL2_ADDR));
    return XMEDIA_SUCCESS;
}

xmedia_void sys_hal_vo_lvds_cksel(xmedia_s32 vo_dev, xmedia_bool lvds_cksel)
{
    xmedia_u32 tmp = (lvds_cksel == XMEDIA_TRUE) ? 1 : 0;

    if(vo_dev == 0) {
        sys_reg_set_bit(tmp, 22, IO_CRG_ADDRESS(CRG_PERCTL67_ADDR));
    } else if (vo_dev == 1){
        sys_reg_set_bit(tmp, 23, IO_CRG_ADDRESS(CRG_PERCTL67_ADDR));
    } else {
    }
}

xmedia_void sys_hal_vo_vou_hd_cken(xmedia_s32 vo_dev, xmedia_bool vou_hd_cken)
{
    xmedia_u32 tmp = (vou_hd_cken == XMEDIA_TRUE) ? 1 : 0;
    if(vo_dev == 0) {
        sys_reg_set_bit(tmp, 0, IO_CRG_ADDRESS(CRG_PERCTL67_ADDR));
    } else if (vo_dev == 1){
        sys_reg_set_bit(tmp, 12, IO_CRG_ADDRESS(CRG_PERCTL67_ADDR));
    } else {
    }
}
xmedia_void sys_hal_vo_vou_srst_req(xmedia_bool vou_srst_req)
{
    xmedia_u32 tmp = (vou_srst_req == XMEDIA_TRUE) ? 1 : 0;

    sys_reg_set_bit(tmp, 28, IO_CRG_ADDRESS(CRG_PERCTL67_ADDR));
}

xmedia_s32 sys_hal_vo_vou_srst_get(xmedia_bool* vou_srst)
{
    xmedia_u32 reg;

    if (vou_srst == XMEDIA_NULL) {
        return XMEDIA_FAILURE;
    }

    sys_reg_read(&reg, IO_CRG_ADDRESS(CRG_PERCTL121_ADDR));
    *vou_srst = (reg & 0x1); // bit[0]: vou reset state
    return XMEDIA_SUCCESS;
}

xmedia_void sys_hal_vo_lcd_div_cken(xmedia_bool lcd_div_cken)
{
    xmedia_u32 tmp = (lcd_div_cken == XMEDIA_TRUE) ? 1 : 0;

    sys_reg_set_bit(tmp, 27, IO_CRG_ADDRESS(CRG_PERCTL65_ADDR));

}
xmedia_void sys_hal_vo_pll_div_cken(xmedia_bool pll_div_cken)
{
    //0表示输出时钟,只使能或去使能fout1 fout0
    xmedia_u32 apll_foutpostdivpd = ((!pll_div_cken) << 1) | ((!pll_div_cken) << 0);
    sys_reg_write_32(apll_foutpostdivpd << 24, 0xf << 24, IO_CRG_ADDRESS(CRG_PERCTL1_ADDR));
    //APLL Power Down控制
    sys_reg_set_bit(!pll_div_cken, 29, IO_CRG_ADDRESS(CRG_PERCTL1_ADDR));
    //APLL VCO输出Power Down控制
    sys_reg_set_bit(!pll_div_cken, 28, IO_CRG_ADDRESS(CRG_PERCTL1_ADDR));
    //APLL 测试信号控制
    sys_reg_set_bit(!pll_div_cken, 22, IO_CRG_ADDRESS(CRG_PERCTL1_ADDR));
}
xmedia_void sys_hal_vo_bt_hd_cksel(xmedia_bool bt_hd_cksel)
{
    xmedia_u32 tmp = (bt_hd_cksel == XMEDIA_TRUE) ? 1 : 0;

    sys_reg_set_bit(tmp, 17, IO_CRG_ADDRESS(CRG_PERCTL66_ADDR));
}
xmedia_void sys_hal_vo_lcd_hd_cksel(xmedia_bool lcd_hd_cksel)
{
    xmedia_u32 tmp = (lcd_hd_cksel == XMEDIA_TRUE) ? 1 : 0;

    sys_reg_set_bit(tmp, 16, IO_CRG_ADDRESS(CRG_PERCTL66_ADDR));
}
xmedia_void sys_hal_vo_mipitx_hd_cksel(xmedia_bool mipitx_hd_cksel)
{
    xmedia_u32 tmp = (mipitx_hd_cksel == XMEDIA_TRUE) ? 1 : 0;

    sys_reg_set_bit(tmp, 19, IO_CRG_ADDRESS(CRG_PERCTL66_ADDR));
}
xmedia_void sys_hal_vo_lvdstx_hd_cksel(xmedia_bool lvdstx_hd_cksel)
{
    xmedia_u32 tmp = (lvdstx_hd_cksel == XMEDIA_TRUE) ? 1 : 0;

    sys_reg_set_bit(tmp, 18, IO_CRG_ADDRESS(CRG_PERCTL66_ADDR));
}
xmedia_void sys_hal_vo_bt_pctrl(xmedia_bool bt_pctrl)
{
    xmedia_u32 tmp = (bt_pctrl == XMEDIA_TRUE) ? 1 : 0;

    sys_reg_set_bit(tmp, 9, IO_CRG_ADDRESS(CRG_PERCTL66_ADDR));
}
xmedia_void sys_hal_vo_lcd_pctrl(xmedia_bool lcd_pctrl)
{
    xmedia_u32 tmp = (lcd_pctrl == XMEDIA_TRUE) ? 1 : 0;

    sys_reg_set_bit(tmp, 8, IO_CRG_ADDRESS(CRG_PERCTL66_ADDR));
}
xmedia_void sys_hal_vo_mipitx_pctrl(xmedia_bool mipitx_pctrl)
{
    xmedia_u32 tmp = (mipitx_pctrl == XMEDIA_TRUE) ? 1 : 0;

    sys_reg_set_bit(tmp, 11, IO_CRG_ADDRESS(CRG_PERCTL66_ADDR));
}
xmedia_void sys_hal_vo_lvdstx_pctrl(xmedia_bool lvdstx_pctrl)
{
    xmedia_u32 tmp = (lvdstx_pctrl == XMEDIA_TRUE) ? 1 : 0;

    sys_reg_set_bit(tmp, 10, IO_CRG_ADDRESS(CRG_PERCTL66_ADDR));
}
xmedia_void sys_hal_vo_bt_cken(xmedia_bool bt_cken)
{
    xmedia_u32 tmp = (bt_cken == XMEDIA_TRUE) ? 1 : 0;

    sys_reg_set_bit(tmp, 1, IO_CRG_ADDRESS(CRG_PERCTL66_ADDR));
}
xmedia_void sys_hal_vo_lcd_cken(xmedia_bool lcd_cken)
{
    xmedia_u32 tmp = (lcd_cken == XMEDIA_TRUE) ? 1 : 0;

    sys_reg_set_bit(tmp, 0, IO_CRG_ADDRESS(CRG_PERCTL66_ADDR));
}
xmedia_void sys_hal_vo_mipitx_cken(xmedia_bool mipitx_cken)
{
    xmedia_u32 tmp = (mipitx_cken == XMEDIA_TRUE) ? 1 : 0;

    sys_reg_set_bit(tmp, 3, IO_CRG_ADDRESS(CRG_PERCTL66_ADDR));
}
xmedia_void sys_hal_vo_lvdstx_cken(xmedia_bool lvdstx_cken)
{
    xmedia_u32 tmp = (lvdstx_cken == XMEDIA_TRUE) ? 1 : 0;

    sys_reg_set_bit(tmp, 2, IO_CRG_ADDRESS(CRG_PERCTL66_ADDR));
}


xmedia_s32 sys_hal_init(xmedia_void)
{
#ifdef VO_KERNEL
    if (osal_spin_lock_init(&g_crg_spin_lock) < 0) {
        osal_printk("spinlock init fail, line: %d. \n", __LINE__);
        return -1;
    }

    if (g_reg_crg_base_va == XMEDIA_NULL) {
        g_reg_crg_base_va = (void *)osal_ioremap(CRG_REGS_ADDR, (xmedia_u32)CRG_REGS_SIZE);
        if (g_reg_crg_base_va == XMEDIA_NULL) {
            osal_printk("remap crg reg fail, line: %d. \n", __LINE__);
            goto fail;
        }
    }

    if (g_reg_sys_base_va == XMEDIA_NULL) {
        g_reg_sys_base_va = (void *)osal_ioremap(SYS_REGS_ADDR, (xmedia_u32)SYS_REGS_SIZE);
        if (g_reg_sys_base_va == XMEDIA_NULL) {
            osal_printk("remap sys reg fail, line: %d. \n", __LINE__);
            goto fail;
        }
    }

    if (g_reg_ddr0_base_va == XMEDIA_NULL) {
        g_reg_ddr0_base_va = (void *)osal_ioremap(DDRC0_REG_ADDR, (xmedia_u32)DDRC_REGS_SIZE);
        if (g_reg_ddr0_base_va == XMEDIA_NULL) {
            osal_printk("remap ddr0 reg fail, line: %d. \n", __LINE__);
            goto fail;
        }
    }

    if (g_reg_misc_base_va == XMEDIA_NULL) {
        g_reg_misc_base_va = (void *)osal_ioremap(MISC_REGS_ADDR, (xmedia_u32)MISC_REGS_SIZE);
        if (g_reg_misc_base_va == XMEDIA_NULL) {
            osal_printk("remap MISC reg fail, line: %d. \n", __LINE__);
            goto fail;
        }
    }

    /* version_id与性能限制相关，为了避免通过修改osal_ioremap来更改version_id,使用特殊的iomap函数 */
    if (g_reg_otp_base_va == XMEDIA_NULL) {
#ifdef VO_KERNEL
        g_reg_otp_base_va = (void *)osal_ioremap(OTP_REGS_ADDR, (xmedia_u32)OTP_REGS_SIZE);
#else
        g_reg_otp_base_va = sys_hal_zonediv(OTP_REGS_ADDR, (xmedia_u32)OTP_REGS_SIZE);
#endif
        if (g_reg_otp_base_va == XMEDIA_NULL) {
            osal_printk("remap OTP reg fail, line: %d. \n", __LINE__);
            goto fail;
        }
    }

    if (g_sct_addr == XMEDIA_NULL) {
        g_sct_addr = (void *)osal_ioremap(SCT_REGS_ADDR, (xmedia_u32)SCT_REGS_SIZE);
        if(g_sct_addr == XMEDIA_NULL) {
            osal_printk("remap SCT reg fail, line: %d. \n", __LINE__);
            goto fail;
        }
    }

    return 0;

fail:
    sys_hal_exit();
    return -1;
#endif
    return 0;
}

xmedia_void sys_hal_exit(xmedia_void)
{
#ifdef VO_KERNEL
    if (g_reg_crg_base_va != XMEDIA_NULL) {
        osal_iounmap(g_reg_crg_base_va);
        g_reg_crg_base_va = XMEDIA_NULL;
    }
    if (g_reg_sys_base_va != XMEDIA_NULL) {
        osal_iounmap(g_reg_sys_base_va);
        g_reg_sys_base_va = XMEDIA_NULL;
    }
    if (g_reg_ddr0_base_va != XMEDIA_NULL) {
        osal_iounmap(g_reg_ddr0_base_va);
        g_reg_ddr0_base_va = XMEDIA_NULL;
    }

    if (g_reg_misc_base_va != XMEDIA_NULL) {
        osal_iounmap(g_reg_misc_base_va);
        g_reg_misc_base_va = XMEDIA_NULL;
    }

    if (g_reg_otp_base_va != XMEDIA_NULL) {
#ifdef VO_KERNEL
        osal_iounmap(g_reg_otp_base_va);
#else
        sys_hal_atmos(g_reg_otp_base_va);
#endif
        g_reg_otp_base_va = XMEDIA_NULL;
    }

    if (g_sct_addr != XMEDIA_NULL) {
        osal_iounmap(g_sct_addr);
        g_sct_addr = XMEDIA_NULL;
    }

    osal_spin_lock_destroy(&g_crg_spin_lock);

    return;
#endif
    return;
}

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

