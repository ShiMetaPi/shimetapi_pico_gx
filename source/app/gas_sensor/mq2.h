/*
 * mq2.h - MQ2 气体传感器驱动（板端运行）。
 *
 * MQ2 模块输出：
 *   DO  数字输出：浓度超阈值（板上电位器调）拉低/拉高，单 GPIO 读即可。
 *   AO  模拟输出：电压随浓度变化，需 SoC 的 SARADC 读。
 *
 * 板端资源：
 *   DO  接 GPIO0_1  -> /dev/gpiochip0 line 1（chardev 读）
 *   AO  接 GPIO1_0 / LSADC_CH0 -> /dev/mem mmap LSADC 寄存器直读
 *       （板端 xm_lsadc.ko 没开 USE_LSADC_CHANNEL_0，ioctl chn=0 走 default 分支
 *        报 "error chn:0"。绕过驱动，自己写 CONFIG bit8 = enable + START + read CHNDATA。）
 *
 * ADC 数据是裸值：xmorca LSADC 是 12-bit（0..4095），对应 0..Vref（默认 Vref=1.8V 或 3.3V
 * 看板子配置，这里不假设具体电压，只把原始值和换算的"百分比"都给出来）。
 */
#ifndef MQ2_H
#define MQ2_H

#include <stdint.h>

typedef struct {
    int      do_level;   /* DO 数字电平：0 或 1（1 = 浓度超阈值，板卡上电平极性以模块为准） */
    int      ao_raw;     /* LSADC 原始值（12-bit, 0..4095），-1 = 读失败 */
    int      ao_pct;     /* 百分比 = ao_raw * 100 / 4095，-1 = 读失败 */
    int      ao_err;     /* AO 读取错误码（0 = OK） */
} mq2_data_t;

/* 打开 LSADC + 配 GPIO0_1 输入。成功 0，失败 -1。 */
int  mq2_init(void);

/* 读一次 DO + AO。成功 0，失败 -1（一般只有 AO 失败，DO 永远能读）。 */
int  mq2_read(mq2_data_t *out);

void mq2_deinit(void);

#endif /* MQ2_H */