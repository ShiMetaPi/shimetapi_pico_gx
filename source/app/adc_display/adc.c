/*
 * adc.c - LSADC GPIO1_1 / LSADC_CH1 实时电压采样（mq2.c 同款套路：/dev/lsadc 一直 open 不关）。
 *
 * 关键点：
 *   - 全局 g_lsadc_fd 在整个采集期间保持 open。这样 xm_lsadc.ko 的 lsadc_open
 *     会把 CRG 时钟开起来 + 退出 reset + 拉 ENABLE.bit9 = 1，close 之前都不会
 *     再被 lsadc_release 拉回 reset。
 *   - 用户态 /dev/mem mmap LSADC 寄存器后只做一件事：兜底把 CONFIG.bit13 清成
 *     single-step（避免驱动残留的 continuous 状态）。
 *   - 每次读之前都重新 rmw CONFIG.bit9 = 1 开 CH1（参考 mq2.c 里 CH0 bit8 的
 *     写法），这样即使中途被人/驱动动了 CONFIG，下一帧也能拉回来。
 *
 * 寄存器定义（参考 source/gmp/drv/lsadc/adc.c）：
 *   LSADC @ 0x120a0000
 *   bit 9  = CH1 enable (in CONFIG @ +0x00)
 *   bit 13 = mode (0=single, 1=continuous)
 *   bit 15 = reset state
 *   CHNDATA @ +0x2C, CH0=+0, CH1=+4, CH2=+8
 *   START @ +0x1C, STOP @ +0x20
 *   ENABLE @ +0x3C bit9 = lsadc controller enable（由驱动开/关，我们不动）
 */
#include "adc.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>

#define PAGE_SIZE       0x1000u
#define LSADC_BASE      0x120a0000u
#define LSADC_REG_SIZE  0x100u

/* 寄存器偏移 */
#define OFS_CONFIG      0x00u
#define OFS_START       0x1Cu
#define OFS_STOP        0x20u
#define OFS_CHNDATA     0x2Cu   /* CH0 = +0, CH1 = +4, CH2 = +8 */

/* CONFIG 位 */
#define BIT_CH0_EN      (1u << 8)   /* bit8: CH0 使能 */
#define BIT_CH1_EN      (1u << 9)   /* bit9: CH1 使能 */
#define BIT_CH2_EN      (1u << 10)  /* bit10: CH2 使能 */
#define BIT_MODE_CONT   (1u << 13)  /* bit13: 0=single, 1=continuous */
#define BIT_DEGLITCH_BYPASS  (1u << 17)  /* bit17: 1=bypass deglitch (single-step 必设) */

#define ALL_CH_EN_MASK  (BIT_CH0_EN | BIT_CH1_EN | BIT_CH2_EN)

/* LSADC 设备路径 */
#define LSADC_DEV       "/dev/lsadc"

static int                g_lsadc_fd = -1;   /* /dev/lsadc：让驱动 enable clock + exit reset */
static int                g_mem_fd   = -1;   /* /dev/mem */
static volatile uint32_t *g_reg      = NULL; /* mmap 到 LSADC 基址 */

/* 读 / 写 32-bit LSADC 寄存器（带 volatile）。 */
static inline uint32_t lsadc_r(uint32_t ofs)
{
    return g_reg[ofs >> 2];
}
static inline void lsadc_w(uint32_t ofs, uint32_t val)
{
    g_reg[ofs >> 2] = val;
}
static inline void lsadc_rmw(uint32_t ofs, uint32_t val, uint32_t mask)
{
    uint32_t t = g_reg[ofs >> 2];
    t = (t & ~mask) | (val & mask);
    g_reg[ofs >> 2] = t;
}

/* 开 /dev/lsadc（驱动 enable clock + exit reset + ENABLE.bit9=1）+ /dev/mem mmap。
 * 如果 /dev/lsadc 不存在（xm_lsadc.ko 没 insmod），自动尝试 insmod。
 * fd 全程不关，lsadc_release 不会被触发。 */
static int lsadc_open(void)
{
    g_lsadc_fd = open(LSADC_DEV, O_RDWR);
    if (g_lsadc_fd < 0 && errno == ENOENT) {
        /* 节点不存在，尝试 insmod xm_lsadc.ko */
        if (access("/opt/ko/xm_lsadc.ko", R_OK) == 0) {
            printf("[adc] /dev/lsadc 不存在，自动 insmod xm_lsadc.ko ...\n");
            if (system("insmod /opt/ko/xm_lsadc.ko") != 0) {
                fprintf(stderr, "[adc] insmod xm_lsadc.ko 失败\n");
                return -1;
            }
            /* udev/mdev 创建设备节点可能要几百 ms，等一下 */
            usleep(300000);
            g_lsadc_fd = open(LSADC_DEV, O_RDWR);
        }
    }
    if (g_lsadc_fd < 0) {
        fprintf(stderr, "[adc] open %s: %s（%s）\n",
                LSADC_DEV, strerror(errno),
                errno == ENOENT ? "尝试 insmod /opt/ko/xm_lsadc.ko 也失败" : "");
        return -1;
    }
    /* 不做任何 ioctl——避免驱动改 CONFIG。CH1 使能由我们自己 mmap 后写。 */

    g_mem_fd = open("/dev/mem", O_RDWR);
    if (g_mem_fd < 0) {
        fprintf(stderr, "[adc] open /dev/mem: %s\n", strerror(errno));
        return -1;
    }
    volatile uint8_t *m = (volatile uint8_t *)mmap(NULL, PAGE_SIZE,
            PROT_READ | PROT_WRITE, MAP_SHARED, g_mem_fd,
            LSADC_BASE & ~(PAGE_SIZE - 1u));
    if (m == MAP_FAILED) {
        fprintf(stderr, "[adc] mmap LSADC: %s\n", strerror(errno));
        return -1;
    }
    g_reg = (volatile uint32_t *)(m + (LSADC_BASE & (PAGE_SIZE - 1u)));

    /* 强制切到 single-step 模式（避免驱动残留的 continuous 状态）。 */
    lsadc_rmw(OFS_CONFIG, 0u, BIT_MODE_CONT);

    /* single-step 必设：bypass deglitch，否则 boot 默认的 ACTBIT/GLITCH/TIMESCAN
     * 会把通道切换时的电压差当 glitch 扔掉，CHNDATA 卡在上一次的值不更新。
     * 参考 source/gmp/drv/lsadc/adc.c:163 驱动的写法。 */
    lsadc_rmw(OFS_CONFIG, BIT_DEGLITCH_BYPASS, BIT_DEGLITCH_BYPASS);

    /* 显式禁掉 CH0 / CH2，只留 CH1 —— 防止多通道扫描导致 CHNDATA+4 不是真 CH1。 */
    lsadc_rmw(OFS_CONFIG, BIT_CH1_EN, ALL_CH_EN_MASK);
    return 0;
}

static void lsadc_close(void)
{
    if (g_reg != NULL) {
        /* 关闭前确保 CH1 已禁能 */
        lsadc_rmw(OFS_CONFIG, 0u, BIT_CH1_EN);
        munmap((void *)((uintptr_t)g_reg - (LSADC_BASE & (PAGE_SIZE - 1u))),
               PAGE_SIZE);
        g_reg = NULL;
    }
    if (g_mem_fd >= 0) {
        close(g_mem_fd);
        g_mem_fd = -1;
    }
    if (g_lsadc_fd >= 0) {
        close(g_lsadc_fd);    /* 这里才让驱动跑 lsadc_release：进 reset、关 clock */
        g_lsadc_fd = -1;
    }
}

/* 单次 SAR 转换（不推荐外部直接调）。失败返回 -1。 */
static int lsadc_read_once(void)
{
    if (g_reg == NULL) {
        return -1;
    }
    /* 1) 显式只开 CH1（防 CH0/CH2 残留使能） */
    lsadc_rmw(OFS_CONFIG, BIT_CH1_EN, ALL_CH_EN_MASK);
    /* 2) 强制重置 deglitch bypass（bit17=1）—— 防驱动/硬件在前一帧后
     *    不小心清掉 CONFIG.bit17（参考 source/gmp/drv/lsadc/adc.c:163）。
     *    这一步是解决"按键1/2 raw 都是 360+"的关键。 */
    lsadc_rmw(OFS_CONFIG, BIT_DEGLITCH_BYPASS, BIT_DEGLITCH_BYPASS);
    /* 3) 触发 start（single-step：写 1） */
    lsadc_w(OFS_START, 1u);
    /* 4) 等转换完成（参考 sample 1ms，给 2ms 余量；R_b 大时再加 1ms） */
    usleep(3000);
    /* 5) 读 CH1 数据（CHNDATA 偏移 +1<<2 = +4），12-bit mask 0xFFF */
    int v = (int)(lsadc_r(OFS_CHNDATA + (1u << 2)) & 0xFFFu);
    /* 6) 触发 stop */
    lsadc_w(OFS_STOP, 1u);
    return v;
}

/* N 次采样 + 去极值平均（trim 首尾各 1 个）。
 * 默认 8 次；可用 -DADC_AVG_SAMPLES=N 覆盖。 */
#ifndef ADC_AVG_SAMPLES
#define ADC_AVG_SAMPLES  8
#endif
#ifndef ADC_AVG_TRIM     /* 两端各去几个最大值/最小值 */
#define ADC_AVG_TRIM     1
#endif

static int int_cmp(const void *a, const void *b)
{
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

static int lsadc_read_raw(void)
{
    if (g_reg == NULL) {
        return -1;
    }
    int s[ADC_AVG_SAMPLES];
    int n = 0;
    for (int i = 0; i < ADC_AVG_SAMPLES; i++) {
        int v = lsadc_read_once();
        if (v >= 0) {
            s[n++] = v;
        }
        usleep(200);  /* 帧间小延时，给 pad 上的 R_pu+R_b 时间稳定 */
    }
    if (n == 0) return -1;

    if (n <= 2 * ADC_AVG_TRIM) {
        /* 样本太少就简单平均，不去极值 */
        int sum = 0;
        for (int i = 0; i < n; i++) sum += s[i];
        return sum / n;
    }
    /* 排序 + 去掉首尾各 ADC_AVG_TRIM 个 + 平均 */
    qsort(s, n, sizeof(int), int_cmp);
    int sum = 0, cnt = 0;
    for (int i = ADC_AVG_TRIM; i < n - ADC_AVG_TRIM; i++) {
        sum += s[i];
        cnt++;
    }
    return sum / cnt;
}

/* 诊断用：单次 CH1 读（不经 8 次平均，给一个"原始"快照）。失败 -1。 */
static int lsadc_read_diag_ch1(void)
{
    if (g_reg == NULL) {
        return -1;
    }
    lsadc_rmw(OFS_CONFIG, BIT_CH1_EN, ALL_CH_EN_MASK);
    lsadc_rmw(OFS_CONFIG, 0u, BIT_MODE_CONT);
    lsadc_rmw(OFS_CONFIG, BIT_DEGLITCH_BYPASS, BIT_DEGLITCH_BYPASS);
    lsadc_w(OFS_START, 1u);
    usleep(3000);
    int v = (int)(lsadc_r(OFS_CHNDATA + (1u << 2)) & 0xFFFu);
    lsadc_w(OFS_STOP, 1u);
    return v;
}

int adc_init(void)
{
    return lsadc_open();
}

int adc_read(adc_data_t *out)
{
    int v;

    if (out == NULL) {
        return -1;
    }
    v = lsadc_read_raw();
    if (v < 0) {
        out->raw = -1;
        out->mv_linear = -1;
        out->mv = -1;
        return -1;
    }
    out->raw = v;
    /* 线性换算（实测 VREF ≈ 3.15V 时拟合最好，12-bit）。
     * 用 raw*3300/4095 会持续高估 ~10%（因为 LSADC 内部 VREF scaling 不是 1:1），
     * 改成 3150 之后：
     *   raw= 385  → 296 mV   (vs 万用表 300)
     *   raw=1392  →1071 mV   (vs 1000, +7%)
     *   raw=2641  →2031 mV   (vs 1900, +7%)
     *   raw=3640  →2800 mV   (vs 2700, +4%)
     *   raw=4095  →3150 mV   (vs 3300, -5%)
     * "linear" 仅供和万用表对趋势，**精确值看 cal**（cal 走 5 锚点分段线性）。 */
    out->mv_linear = (v * 3150 + 2047) / 4095;

    /* 5 锚点 4 段分段线性标定：把 raw 映射到 R_pu=10kΩ + R_b 模型下的实测毫伏。
     * 锚点见 adc.h：
     *   (386,  300)  按键 1 (R_b=1k)   → 0.300 V
     *   (443, 1055)  按键 2 (R_b=4.7k) → 1.055 V (段 1 内插)
     *   (512, 1980)  按键 3 (R_b=15k)  → 1.980 V (段 2 内插)
     *   (568, 2721)  按键 4 (R_b=47k)  → 2.721 V
     *   (4095,3300)  无按键（上拉到 3.3V，实测 raw=4095）
     *   (3640,2700)  按键 4（R_b=47kΩ）
     *   (2641,1900)  按键 3（R_b=15kΩ）
     *   (1392,1000)  按键 2（R_b=4.7kΩ）
     *   ( 385, 300)  按键 1（R_b=1kΩ）
     * 段 1(386-443)≈13.21 mV/count，段 2(443-512)≈13.36 mV/count，
     * 段 1(385-1392)≈0.69 mV/count，段 2(1392-2641)≈0.72 mV/count，
     * 段 3(2641-3640)≈0.80 mV/count，段 4(3640-4095)≈1.32 mV/count。
     * 注：LSADC 内部 VREF scaling 不严格线性，4 段斜率各不相同是正常的。
     * 段 1-3 斜率近似一致（≈ 13.3 mV/count），段 4 在 SARADC 接近满量程时斜率掉。 */
    {
        int mv;

        if (v <= ADC_CAL_RAW_LO) {
            mv = ADC_CAL_MV_LO;
        } else if (v >= ADC_CAL_RAW_HI) {
            mv = ADC_CAL_MV_HI;
        } else if (v <= ADC_CAL_RAW_M2) {
            /* 段 1：raw_lo..raw_m2 → mv_lo..mv_m2 */
            mv = ADC_CAL_MV_LO +
                 (v - ADC_CAL_RAW_LO) * (ADC_CAL_MV_M2 - ADC_CAL_MV_LO) /
                 (ADC_CAL_RAW_M2 - ADC_CAL_RAW_LO);
        } else if (v <= ADC_CAL_RAW_M3) {
            /* 段 2：raw_m2..raw_m3 → mv_m2..mv_m3 */
            mv = ADC_CAL_MV_M2 +
                 (v - ADC_CAL_RAW_M2) * (ADC_CAL_MV_M3 - ADC_CAL_MV_M2) /
                 (ADC_CAL_RAW_M3 - ADC_CAL_RAW_M2);
        } else if (v <= ADC_CAL_RAW_MID) {
            /* 段 3：raw_m3..raw_mid → mv_m3..mv_mid */
            mv = ADC_CAL_MV_M3 +
                 (v - ADC_CAL_RAW_M3) * (ADC_CAL_MV_MID - ADC_CAL_MV_M3) /
                 (ADC_CAL_RAW_MID - ADC_CAL_RAW_M3);
        } else {
            /* 段 4：raw_mid..raw_hi → mv_mid..mv_hi */
            mv = ADC_CAL_MV_MID +
                 (v - ADC_CAL_RAW_MID) * (ADC_CAL_MV_HI - ADC_CAL_MV_MID) /
                 (ADC_CAL_RAW_HI - ADC_CAL_RAW_MID);
        }
        out->mv = mv;
    }
    return 0;
}

int adc_read_diag(void)
{
    return lsadc_read_diag_ch1();
}

void adc_deinit(void)
{
    lsadc_close();
}
