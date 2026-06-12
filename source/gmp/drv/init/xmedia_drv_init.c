/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/version.h>
#include <linux/of_platform.h>

#include "common.h"
#include "osal.h"

#ifndef MODULE

extern int __init osal_driver_init(void);
extern int __init common_driver_init(void);
extern int __init pmoc_driver_init(void);
extern int __init tde_driver_init(void);
extern int __init vgs_driver_init(void);
extern int __init rgn_driver_init(void);
extern int __init isp_driver_init(void);
extern int __init vpss_driver_init(void);
extern int __init vi_driver_init(void);
extern int __init chnl_driver_init(void);
extern int __init vedu_driver_init(void);
extern int __init rc_driver_init(void);
extern int __init jpege_driver_init(void);
extern int __init h264e_driver_init(void);
extern int __init h265e_driver_init(void);
extern int __init venc_driver_init(void);
extern int __init vo_driver_init(void);
extern int __init gfbg_driver_init(void);
extern int __init cipher_driver_init(void);
extern int __init acomm_driver_init(void);
extern int __init aiao_driver_init(void);
extern int __init ao_driver_init(void);
extern int __init ai_driver_init(void);
extern int __init aenc_driver_init(void);
extern int __init adec_driver_init(void);
extern int __init npu_driver_init(void);
extern int __init ir_driver_init(void);
extern int __init adc_driver_init(void);
extern int __init ive_driver_init(void);
extern int __init misc_driver_init(void);


static int __init xmedia_driver_init(void)
{
    osal_driver_init();
    common_driver_init();
    pmoc_driver_init();
    vgs_driver_init();
    tde_driver_init();
    rgn_driver_init();
    isp_driver_init();
    vpss_driver_init();
    npu_driver_init();
    ive_driver_init();
    vi_driver_init();
    //misc_driver_init();
    chnl_driver_init();
    vedu_driver_init();
    rc_driver_init();
    jpege_driver_init();
    h264e_driver_init();
    h265e_driver_init();
    venc_driver_init();
    vo_driver_init();
    gfbg_driver_init();
    cipher_driver_init();
    acomm_driver_init();
    aiao_driver_init();
    ao_driver_init();
    ai_driver_init();
    aenc_driver_init();
    adec_driver_init();

    /*IR_IN confilicts with SDIO_CARD_DETECT pin function,
    SDIO_CARD_DETECT function is retained by default.*/
    //ir_driver_init();
    //adc_driver_init();

    return 0;
}

late_initcall(xmedia_driver_init);
MODULE_LICENSE("GPL");

#endif

