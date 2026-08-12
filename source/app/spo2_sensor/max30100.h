/*
 * max30100.h - MAX30100 血氧/心率传感器驱动（I2C）。
 *
 * MAX30100 通过 I2C 暴露 0x00..0x17 寄存器（按本芯片实际承认的寄存器图写，
 * 不是新 datasheet 的 0x07 FIFO_DATA / 0x09 MODE / 0x0C LED1_PA 那一套）。
 * FIFO 每条 sample = 4 字节（IR 高/低 + R 高/低），配置 SpO2 模式后 sensor
 * 自动以设定采样率把 ADC 结果塞 FIFO，超阈值触发 INT。
 *
 * 实际寄存器地址（按本驱动 .c 用的为准，新 datasheet 与之冲突）：
 *   0x00 INT_STATUS  （读后自清）
 *   0x02 FIFO_WR_PTR [3:0]
 *   0x03 OVF_COUNTER [3:0]
 *   0x04 FIFO_RD_PTR [3:0]
 *   0x05 FIFO_DATA   （burst 读，寄存器指针不递增）
 *   0x06 MODE_CONFIG [7]SHDN [6]RESET [3]TEMP_EN [2:0]MODE
 *   0x07 SPO2_CONFIG [6]HI_RES_EN [4:2]SR [1:0]LED_PW
 *   0x09 LED_CONFIG  [7:4]RED_PA [3:0]IR_PA —— 单寄存器双 nibble
 *   0x16 DIE_TEMP_INT
 *   0x17 DIE_TEMP_FRAC
 *   0xFE REV_ID
 *   0xFF PART_ID  (0x11 = MAX30100, 0x15 = MAX30102/30105)
 *
 * 板端资源（与 OLED/MPU6050/SHT20 共享 I2C3）：
 *   SCL = GPIO4_1 @ 0x100C0010 func2 = I2C3_SCL（默认 func0=GPIO,func2=I2C）
 *   SDA = GPIO4_2 @ 0x100C0014 func2 = I2C3_SDA（默认 func5=GPIO,func2=I2C）
 *   INT = GPIO4_4 @ 0x100C001C func5（GPIO 输入）→ /dev/gpiochip4 line 4
 *        ⚠ 与 spi_hal 的屏 RES 撞同一根线，spi_hal_init 先跑会占住 → INT 申请失败，退化为轮询
 *   IRD, RD 悬空（用 sensor 内部 LED）
 *
 * 另有一组可选 I2C3 引脚：pin117/118 @ 0x100C0080/0x100C0084 func4 = GPIO7_5/GPIO7_6，
 * 两组不能同时开。
 *
 * 注意：本驱动只读 FIFO 出原始 IR/R 值 + 简单峰值检测估算 HR。
 *       真正的血氧饱和度 SpO2 需要 R/IR 比值的非线性校正 + 个体校准，
 *       这里只给"指示性百分比"（基于 R/IR 的简单比值），不做医学级判断。
 */
#ifndef MAX30100_H
#define MAX30100_H

#include <stdint.h>

#ifndef MAX30100_BUS
#define MAX30100_BUS  3       /* /dev/i2c-3 */
#endif
#ifndef MAX30100_ADDR
#define MAX30100_ADDR 0x57    /* MAX30100 7-bit 从地址；少数板用 0x5A */
#endif

#define MAX30100_FIFO_DEPTH  16     /* MAX30100 硬件 FIFO 深度 = 16 样本（指针 4-bit）。
                                     * 注意：MAX30102 才是 32 深、指针 5-bit，别照抄。 */

typedef struct {
    uint32_t ir[MAX30100_FIFO_DEPTH];   /* 红外 ADC 原始值（16-bit，HI_RES_EN 开启） */
    uint32_t r [MAX30100_FIFO_DEPTH];   /* 红光 ADC 原始值（16-bit） */
    int      n;                          /* 本次实际读到的 sample 数 (0..16) */
    int      fifo_overflow;             /* OVF_COUNTER 读数，>0 说明轮询太慢丢样本 */
} max30100_fifo_t;

/*
 * 初始化：开 I2C + I2C_SLAVE 绑地址 + GPIO4_4 申请输入 + 配 MODE=SpO2, SPO2_CFG=100Hz/411us,
 * LED_PA=0x1F。返回 0 成功，-1 失败。
 */
int max30100_init(void);

/*
 * 读一次 FIFO：先看 INT_STATUS，有 FIFO 数据就读出来。
 *   ir/r: 各 MAX30100_FIFO_DEPTH 长数组（输出）
 *   n: 实际读到的 sample 数（输出）
 *   overflow: FIFO 溢出计数（输出）
 * 成功 0，失败 -1。
 */
int max30100_read_fifo(uint32_t *ir, uint32_t *r, int *n, int *overflow);

/*
 * 峰值检测算心率：基于最近 len 个 IR 样本，用滑窗（最近 2 秒）算滑动均值 baseline
 * + amp*20% 阈值，峰间最小 500ms 去抖（上限 120 BPM）。
 *   ir_buf: 最近的 IR 值数组
 *   len:    长度
 *   sps:    采样率 Hz（按本驱动配的 CFG_SPO2_CFG=0x43 实测为 50 传入）
 * 返回每分钟心跳数 BPM（< 0 表示算不出来）。
 */
int max30100_est_bpm(const uint32_t *ir_buf, int len, int sps);

void max30100_deinit(void);

#endif /* MAX30100_H */