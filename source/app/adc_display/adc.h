/*
 * adc.h - LSADC 通用 ADC 驱动（用户态）。
 *
 * 板端资源（xmorca LSADC @ 0x120a0000）：
 *   CH0 = GPIO1_0 / iocfg_reg20 @ 0x12090000  (驱动没开 USE_LSADC_CHANNEL_0，ioctl 不可用)
 *   CH1 = GPIO1_1 / iocfg_reg21 @ 0x12090004  ← 本程序用这个
 *   CH2 = GPIO1_2 / iocfg_reg22 @ 0x12090008  (驱动支持但未在本程序验证)
 *
 * 数据：12-bit 原始值（0..4095），用 5 锚点 4 段分段线性标定映射到毫伏。
 *
 * 锚点由用户实测：每个按键按下时用万用表量 IO 电压，同时读 raw。
 *   idle    (无按键)    → 3.300 V    raw=4095
 *   按键 4  (R_b=47k)   → 2.700 V    raw=3640
 *   按键 3  (R_b=15k)   → 1.900 V    raw=2641
 *   按键 2  (R_b=4.7k)  → 1.000 V    raw=1392
 *   按键 1  (R_b=1k)    → 0.300 V    raw= 385
 *
 * 公式（4 段分段线性）：
 *   raw ≤  385        → mv =  300   (钳下界)
 *   385  < raw ≤ 1392 → mv =  300 + (raw- 385) * (1000- 300) / (1392- 385)
 *   1392 < raw ≤ 2641 → mv = 1000 + (raw-1392) * (1900-1000) / (2641-1392)
 *   2641 < raw ≤ 3640 → mv = 1900 + (raw-2641) * (2700-1900) / (3640-2641)
 *   3640 < raw ≤ 4095 → mv = 2700 + (raw-3640) * (3300-2700) / (4095-3640)
 *   raw > 4095        → mv = 3300  (钳上界)
 *
 * 段 1-3 斜率 ≈ 13.4 mV/count，段 4（满量程段）≈ 1.28 mV/count。
 *
 * 驱动接口参考 source/gmp/drv/lsadc/adc.c，应用层把 6 个 ioctl 直接复制定义。
 */
#ifndef ADC_H
#define ADC_H

#include <stdint.h>

/* 默认通道 = 1（GPIO1_1 / LSADC_CH1）。要看其它通道改这里 */
#ifndef ADC_CHN
#define ADC_CHN         1
#endif

/* 5 锚点标定（R_pu=10kΩ，按键 1/2/3/4/上拉）： */
#ifndef ADC_CAL_RAW_LO         /* 按键 1 (1kΩ) → 0.300 V (实测锚点) */
#define ADC_CAL_RAW_LO   385
#endif
#ifndef ADC_CAL_MV_LO
#define ADC_CAL_MV_LO     300
#endif

#ifndef ADC_CAL_RAW_M2         /* 按键 2 (4.7kΩ) → 1.000 V (实测) */
#define ADC_CAL_RAW_M2   1392
#endif
#ifndef ADC_CAL_MV_M2
#define ADC_CAL_MV_M2    1000
#endif

#ifndef ADC_CAL_RAW_M3         /* 按键 3 (15kΩ) → 1.900 V (实测) */
#define ADC_CAL_RAW_M3   2641
#endif
#ifndef ADC_CAL_MV_M3
#define ADC_CAL_MV_M3    1900
#endif

#ifndef ADC_CAL_RAW_MID        /* 按键 4 (47kΩ) → 2.700 V (实测) */
#define ADC_CAL_RAW_MID  3640
#endif
#ifndef ADC_CAL_MV_MID
#define ADC_CAL_MV_MID   2700
#endif

#ifndef ADC_CAL_RAW_HI         /* 无按键（上拉到 VCC） → 3.300 V */
#define ADC_CAL_RAW_HI   4095
#endif
#ifndef ADC_CAL_MV_HI
#define ADC_CAL_MV_HI    3300
#endif

typedef struct {
    int      raw;        /* 12-bit 原始值，0..4095；< 0 = 读失败 */
    int      mv_linear;  /* 线性换算毫伏（VREF≈3.15V 拟合）：raw * 3150 / 4095
                          *   用于直接和万用表对：若 V_REF 真是 3.3V，
                          *   linear ≈ 万用表读数（差几十 mV 正常）。
                          *   若 linear 与万用表差几百 mV，说明:
                          *     (a) VREF 不是 3.3V
                          *     (b) 按键实际接在 CH0/CH2 而不是 CH1
                          *     (c) 分压电阻与标定假设的不一样 */
    int      mv;         /* 5 锚点 4 段分段线性标定后的毫伏
                          *   按键 1 → 300mV / 0.300V
                          *   按键 2 → 1055mV / 1.055V
                          *   按键 3 → 1980mV / 1.980V
                          *   按键 4 → 2721mV / 2.721V
                          *   无按键 → 3300mV / 3.300V
                          *   此值依赖标定表，仅在硬件/分压完全匹配时正确 */
} adc_data_t;

/* 打开 /dev/lsadc（保持 open 不关，让驱动帮忙 enable clock + exit reset）
 * + /dev/mem mmap LSADC 寄存器，兜底切 single-step 模式。成功 0，失败 -1。
 * 自动 insmod /opt/ko/xm_lsadc.ko 如果设备节点不存在（兜底）。 */
int adc_init(void);

/* 读一次 ADC 原始值并用 5 锚点分段线性标定换算毫伏。每帧自动重新使能 CH1 兜底
 * 状态丢失。成功 0，失败 -1。 */
int adc_read(adc_data_t *out);

/* 诊断读：CH1 单次 SAR 读（不经 8 次平均），快速给一个 raw 快照。
 * 返回 0..4095 = 原始 raw；< 0 = 读失败。 */
int adc_read_diag(void);

void adc_deinit(void);

#endif /* ADC_H */
