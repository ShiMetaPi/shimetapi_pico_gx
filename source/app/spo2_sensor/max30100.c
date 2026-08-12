/*
 * max30100.c - MAX30100 血氧/心率传感器驱动（I2C）。
 *
 * 寄存器（参考 MAX30100 datasheet Rev 1.4）：
 *   0x00 INT_STATUS1   [0]=PWR_RDY, [1..4]=reserved, [5]=FIFO_A_FULL, [6]=TMP_RDY, [7]=reserved
 *   0x01 INT_STATUS2   [0]=reserved, [1..6]=reserved, [7]=FIFO_OVF
 *   0x02 INT_ENABLE1   bit0=29ms 心率就绪（不用）
 *   0x03 INT_ENABLE2   bit0=FIFO 溢出（建议开）
 *   0x04 FIFO_WR_PTR   0..31
 *   0x05 OVF_COUNTER   0..31
 *   0x06 FIFO_RD_PTR   0..31
 *   0x07 FIFO_DATA     读 4 字节：IR 高/低 + R 高/低（每条 sample）
 *   0x08 FIFO_CONFIG   SMP_AVE[7:5] + FIFO_ROLLOVER[4] + A_FULL[3:0]
 *   0x09 MODE_CONFIG   [6]=SHDN, [2]=RESET, [1:0]=MODE: 0x02=HR only, 0x03=SpO2
 *   0x0A SPO2_CONFIG   [5]=reserved, [4:2]=ADC_RGE(0..3), [1:0]=SR(0..3) ; 0x0B LED_PW[1:0]
 *   0x0C LED1_PA(IR)   0x00..0x7F（按 ADC 满量程电流档位）
 *   0x0D LED2_PA(R)    同上
 *   0x0E..0x10         reserved
 *   0x11 DIE_TEMP      两字节整数
 *   0x12..0x15         reserved
 *   0x16 REVISION_ID
 *
 * SpO2 标准配置：
 *   FIFO_CFG    = 0x4F  （不平均，A_FULL=15）
 *   MODE_CFG    = 0x03  （SpO2 模式：HR + SpO2）
 *   SPO2_CFG    = 0x27  （LED_PW=411us, SR=100Hz, ADC_RGE=4096nA）
 *   LED1_PA(IR) = 0x1F  （中等电流档）
 *   LED2_PA(R)  = 0x1F
 *
 * 读 FIFO 流程（标准 burst read）：
 *   1) 读 0x00 INT_STATUS（可选，主要清状态）
 *   2) 读 0x04..0x06 三字节：WR_PTR, OVF, RD_PTR
 *   3) sample 数 = (WR_PTR - RD_PTR) mod 32（也可能 = OVF_COUNTER 上限 17）
 *   4) 从 FIFO_DATA(0x07) 连读 sample*4 字节；每 4 字节 = IR(18-bit) + R(18-bit)
 */
#include "max30100.h"
#include "i2c_hal.h"
#include "gpio_hal.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

/* 寄存器（MAX30100 datasheet Table 3，PART_ID=0x11）
 * 注意：和 MAX30102/30105 的寄存器图完全不同，别混用。 */
#define REG_INT_STATUS    0x00   /* [7]A_FULL [6]TEMP_RDY [5]HR_RDY [4]SPO2_RDY [0]PWR_RDY，读后自清 */
#define REG_INT_ENABLE    0x01   /* [7]ENB_A_FULL [6]ENB_TEMP_RDY [5]ENB_HR_RDY [4]ENB_SPO2_RDY */
#define REG_FIFO_WR_PTR   0x02   /* [3:0] */
#define REG_OVF_COUNTER   0x03   /* [3:0] */
#define REG_FIFO_RD_PTR   0x04   /* [3:0] */
#define REG_FIFO_DATA     0x05   /* burst 读时寄存器指针不递增，连续吐 FIFO 字节 */
#define REG_MODE_CONFIG   0x06   /* [7]SHDN [6]RESET [3]TEMP_EN [2:0]MODE */
#define REG_SPO2_CONFIG   0x07   /* [6]SPO2_HI_RES_EN [4:2]SPO2_SR [1:0]LED_PW */
#define REG_LED_CONFIG    0x09   /* [7:4]RED_PA [3:0]IR_PA —— 单寄存器双 nibble */
#define REG_TEMP_INT      0x16
#define REG_TEMP_FRAC     0x17
#define REG_REV_ID        0xFE   /* revision id（值不固定，仅打印用）*/
#define REG_PART_ID       0xFF   /* part id：MAX30100=0x11，MAX30102/30105=0x15 */

#define PART_ID_MAX30100  0x11u
#define PART_ID_MAX30102  0x15u

/* 配置 */
#define CFG_MODE_SPO2     0x03u   /* MODE[2:0]=011 SpO2（HR+SpO2）；010=HR only */
#define CFG_SPO2_CFG      0x43u   /* HI_RES_EN=1(bit6) | SR=50Hz([4:2]=000) | LED_PW=1600us([1:0]=11)
                                   * → 16-bit 分辨率。配 main.c 100ms 轮询 = 5 sample/轮，
                                   * 16 深 FIFO 留出余量给刷屏时间。 */
#define CFG_LED_CONFIG    0x99u   /* RED_PA=0x9(30.6mA) | IR_PA=0x9(30.6mA)
                                   * 档位（手册 Table 6）：
                                   *   0x7 =24.0mA   0x8 =27.1mA   0x9 =30.6mA
                                   *   0xA =33.8mA   0xB =37.0mA   0xC =40.2mA
                                   *   0xD =43.6mA   0xE =46.8mA   0xF =50.0mA
                                   * 调参记录：
                                   *   0x77 (24mA) → R 只到 30k，脉搏幅度 ~700，太弱
                                   *   0xBB (37mA) → R 频繁打满 65535（饱和），但能看到 ~30k 的脉搏幅度
                                   *   0x99 (30.6mA) ← 当前：期望 R 在 50k~63k 不饱和，脉搏幅度 5k~10k
                                   * 若 R 仍打满 → 退到 0x77(24mA) 或换 800µs LED_PW 减积分时间。 */

#define FIFO_PTR_MASK     0x0Fu   /* MAX30100 FIFO 指针是 4-bit（16 深），不是 MAX30102 的 5-bit */
#define ADC_MASK          0xFFFFu /* HI_RES_EN + LED_PW=1600us → 16-bit ADC（不是 18-bit） */

static int         g_fd = -1;
static gpio_handle_t g_int;
static int         g_int_ok = 0;

static int write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    return i2c_hal_write(g_fd, MAX30100_ADDR, buf, 2);
}

static int read_regs(uint8_t reg, uint8_t *buf, int n)
{
    return i2c_hal_read(g_fd, MAX30100_ADDR, reg, buf, n);
}

static int read_reg(uint8_t reg, uint8_t *val)
{
    return read_regs(reg, val, 1);
}

int max30100_init(void)
{
    uint8_t rev = 0, part = 0, mode = 0;

    g_fd = i2c_hal_open(MAX30100_BUS, MAX30100_ADDR);
    if (g_fd < 0) {
        return -1;
    }

    /* 先读 PART_ID(0xFF) 认芯片。ID 寄存器只读，不受 reset 影响。
     * 本驱动按 MAX30100(0x11) 的寄存器图写；MAX30102/30105(0x15) 寄存器图不同，不兼容。 */
    if (read_reg(REG_PART_ID, &part) < 0) {
        fprintf(stderr, "[max] 读 PART_ID(0xFF) 失败：总线通了但器件不响应读？\n");
        goto fail;
    }
    (void)read_reg(REG_REV_ID, &rev);   /* rev 只作参考，任何值都合法，不做判断 */
    printf("[max] PART_ID=0x%02X REV_ID=0x%02X\n", part, rev);

    if (part == PART_ID_MAX30102) {
        fprintf(stderr,
                "[max] 检测到 MAX30102/30105(0x15)，本驱动是 MAX30100 寄存器图，不兼容：\n"
                "      MAX30102 的 MODE_CONFIG 在 0x09(本驱动用 0x06)、FIFO_DATA 在 0x07(本驱动用 0x05)，\n"
                "      且 LED 电流是 0x0C/0x0D 两个独立寄存器、FIFO 32 深、ADC 18-bit。\n");
        goto fail;
    }
    if (part != PART_ID_MAX30100) {
        fprintf(stderr, "[max] PART_ID=0x%02X 不是 MAX3010x（期望 0x11/0x15）。\n", part);
        goto fail;
    }

    /* reset：MODE_CONFIG bit6 = RESET，复位完成后该位由硬件自清 */
    if (write_reg(REG_MODE_CONFIG, 0x40) < 0) {
        fprintf(stderr, "[max] reset 失败\n");
        goto fail;
    }
    usleep(300000);   /* 等 300ms：芯片内部状态机 / 振荡器稳定，避免后续 read 拿到 NACK */

    /* 配置：MODE(SpO2) / SPO2_CFG(SR+LED_PW+HI_RES) / LED_CONFIG(RED+IR 电流) */
    if (write_reg(REG_SPO2_CONFIG, CFG_SPO2_CFG)  < 0) goto fail;
    if (write_reg(REG_LED_CONFIG,  CFG_LED_CONFIG) < 0) goto fail;
    if (write_reg(REG_MODE_CONFIG, CFG_MODE_SPO2) < 0) goto fail;   /* MODE 最后写，配完才开始采样 */

    /* 再多等 200ms 让 LED / ADC 启动 + FIFO 开始攒数据 */
    usleep(200000);

    /* FIFO 三个指针清零，从干净状态开始（reset 后本该是 0，写一次求稳） */
    (void)write_reg(REG_FIFO_WR_PTR, 0x00);
    (void)write_reg(REG_OVF_COUNTER, 0x00);
    (void)write_reg(REG_FIFO_RD_PTR, 0x00);

    /* 验证：读回 MODE[2:0] 应是 0x03 */
    if (read_reg(REG_MODE_CONFIG, &mode) < 0 || (mode & 0x07) != CFG_MODE_SPO2) {
        fprintf(stderr, "[max] MODE 读回 0x%02X（期望 [2:0]=0x03）\n", mode);
        goto fail;
    }

    /* 申请 GPIO4_4 输入（INT）—— 故意禁用。
     * 原因：pin89 (LCD_DE) 同时被 SPI 屏的 RES 占用（spi_hal_init 先跑拿到 chardev），
     *       MAX30100 INT 接这根线会把屏 RES 拉低导致屏不亮，且 kernel dts 也没注册
     *       MAX30100 的 IRQ 节点，**就算接了 INT 也触发不了中断**。
     * 解决：硬件上 INT 引脚悬空不接，软件上不申请、不 padmux，强制走 FIFO 轮询。 */
    g_int_ok = 0;
    /* 旧逻辑保留注释以备需要时回退：
     * memset(&g_int, 0, sizeof(g_int));
     * g_int.chip_path      = "/dev/gpiochip4";
     * g_int.line_offset    = 4;
     * g_int.gpio_mode      = GPIOHANDLE_REQUEST_INPUT;
     * g_int.default_value  = 0;
     * snprintf(g_int.consumer_label, sizeof(g_int.consumer_label), "max-int");
     * if (gpio_handle_init(&g_int) == 0) { g_int_ok = 1; }
     * else { fprintf(stderr, "[max] GPIO4_4 申请输入失败：%s（不影响 FIFO 轮询）\n", strerror(errno)); }
     */

    printf("[max] @ /dev/i2c-%d addr 0x%02x (rev=0x%02X, INT=disabled, polling only)\n",
           MAX30100_BUS, MAX30100_ADDR, rev);
    return 0;

fail:
    i2c_hal_close(g_fd);
    g_fd = -1;
    return -1;
}

int max30100_read_fifo(uint32_t *ir, uint32_t *r, int *n_out, int *overflow)
{
    uint8_t buf[3];
    uint8_t raw[4 * MAX30100_FIFO_DEPTH];
    int wr, ovf, rd, n;

    if (g_fd < 0 || ir == NULL || r == NULL || n_out == NULL) {
        return -1;
    }

    /* 清 INT_STATUS（MAX30100 只有一个中断状态寄存器 0x00，读后自清） */
    (void)read_reg(REG_INT_STATUS, &buf[0]);

    /* 读 WR/OVF/RD 三字节（0x02/0x03/0x04 连续，一次 burst 拿完）
     * 偶尔会拿到 EIO（总线瞬时尖峰 / 芯片 reset 后尚未完全稳定），retry 3 次，
     * 每次间隔 50ms。3 次仍失败才放弃返回 -1。 */
    int retry = 3;
    while (retry-- > 0) {
        if (read_regs(REG_FIFO_WR_PTR, buf, 3) >= 0) break;
        usleep(50000);
        (void)read_reg(REG_INT_STATUS, &buf[0]);  /* 重试前再清一次状态 */
    }
    if (retry < 0) {
        return -1;
    }
    wr  = buf[0] & FIFO_PTR_MASK;
    ovf = buf[1] & FIFO_PTR_MASK;
    rd  = buf[2] & FIFO_PTR_MASK;

    /* OVF_COUNTER 不会自动清零，必须写 0 才能重置——否则下一轮还会读到上次的值 */
    (void)write_reg(REG_OVF_COUNTER, 0x00);
    /* RD_PTR 不用手动写：burst read 时芯片自动推进 RD_PTR。手动写反而可能跟硬件状态机冲突。 */

    /* 溢出时 WR==RD，用 (wr-rd)&mask 会算成 0，得靠 OVF 判满 */
    if (ovf > 0) {
        n = MAX30100_FIFO_DEPTH;
    } else {
        n = (wr - rd) & FIFO_PTR_MASK;     /* 0..15 */
    }
    if (n == 0) {
        *n_out = 0;
        *overflow = ovf;
        return 0;
    }
    if (n > MAX30100_FIFO_DEPTH) {
        n = MAX30100_FIFO_DEPTH;
    }

    /* 一次性 burst 读 n*4 字节（读 0x05 时寄存器指针不递增，连续吐 FIFO） */
    if (read_regs(REG_FIFO_DATA, raw, n * 4) < 0) {
        return -1;
    }

    for (int i = 0; i < n; i++) {
        /* 每样本 4 字节（手册 Page 13 Table 2）：byte0=IR[15:8] byte1=IR[7:0] byte2=R[15:8] byte3=R[7:0]
         *
         * 本模块的 LED 物理极性反了——byte0/1（datasheet 标 IR 脚）实际接的是 RED LED die，
         * byte2/3（datasheet 标 R 脚）实际接的是 IR LED die。表现为手指贴紧时 R 计数 > IR 计数。
         * 在软件里把两个通道颠倒过来：硬件 IR → 软件 RED，硬件 RED → 软件 IR。
         * 后续算法（BPM/SpO2）都按软件通道走，不再关心硬件极性。
         */
        r[i]  = (((uint32_t)raw[i*4 + 0] << 8) | raw[i*4 + 1]) & ADC_MASK;  /* 硬件 IR，软件 RED */
        ir[i] = (((uint32_t)raw[i*4 + 2] << 8) | raw[i*4 + 3]) & ADC_MASK;  /* 硬件 RED，软件 IR */
    }
    *n_out = n;
    *overflow = ovf;
    return 0;
}

/*
 * 峰值检测：滑窗均值 baseline + 阈值 amp*20% + 峰间去抖 500ms。
 * 心率公式：BPM = peak 数 * 60 / 窗口秒数。
 * 只用于演示，医学级需要 FFT 或更精细的算法。
 *
 * 关键设计：
 *   1) baseline 用最近 WINDOW_SEC 秒所有 sample 的算术平均（不是 (vmin+vmax)/2）。
 *      原因：PPG 信号有「呼吸调制」—— 吸气时脉搏幅度大、呼气时幅度小。
 *      用 (vmin+vmax)/2 当 baseline 会被大峰 vmax 拉高，呼气期的小峰过不了阈值 → 漏峰 → BPM 折半。
 *      用滑动均值当 baseline，阈值跟着信号 DC 走，呼吸调制的小峰也能稳定数到。
 *   2) 阈值 = baseline + amp * 20%（30% 太严会漏小峰；10% 太松会被噪声尖刺骗）。
 *   3) 峰间最小 500ms（= 120 BPM 上限）：真实静息心率 < 100 BPM 够用；
 *      同时把 dicrotic notch（脉搏后小反弹）和手指微动的假峰都挡掉。
 */
int max30100_est_bpm(const uint32_t *ir_buf, int len, int sps)
{
    if (ir_buf == NULL || len < sps) {   /* 至少 1 秒数据 */
        return -1;
    }
    if (len > 512) len = 512;

    /* 取最近 WINDOW_SEC 秒数据计算 amp / baseline。 */
    const int WINDOW_SEC = 2;
    int win = WINDOW_SEC * sps;
    if (win > len) win = len;
    const uint32_t *p = ir_buf + (len - win);  /* 滑窗起点 */

    uint64_t sum = 0;
    uint32_t vmin = p[0], vmax = p[0];
    for (int i = 0; i < win; i++) {
        sum += p[i];
        if (p[i] < vmin) vmin = p[i];
        if (p[i] > vmax) vmax = p[i];
    }
    uint32_t baseline = (uint32_t)(sum / win);
    uint32_t amp = vmax - vmin;
    /* 信号太弱：可能没贴手，或信号被双层均值磨平了。返回 -1。 */
    if (amp < 30) {
        return -1;
    }

    /* thresh = baseline + amp * 20%。呼吸调制的小峰也能数到。 */
    uint32_t thresh = baseline + amp / 5;
    if (thresh >= vmax) thresh = vmax - 1;
    if (thresh <= baseline) thresh = baseline + 1;

    /* 峰间去抖：peak 最小间隔 500ms（= 120 BPM 上限），挡 dicrotic notch + 抖动假峰。 */
    int min_peak_samples = (sps * 500) / 1000;
    if (min_peak_samples < 1) min_peak_samples = 1;

    int peaks = 0;
    int in_peak = 0;
    int since_last_peak = min_peak_samples;  /* 起点允许立即出第一个峰 */
    for (int i = 1; i < len; i++) {
        since_last_peak++;
        if (!in_peak && since_last_peak >= min_peak_samples &&
            ir_buf[i] > thresh && ir_buf[i] > ir_buf[i-1]) {
            in_peak = 1;
            peaks++;
            since_last_peak = 0;
        } else if (in_peak && ir_buf[i] < thresh) {
            in_peak = 0;
        }
    }
    if (peaks < 2) {   /* 至少 2 个峰才可信（10 秒 hist，72 BPM ≈ 12 峰） */
        return -1;
    }
    int secs = len / sps;
    if (secs <= 0) secs = 1;
    int bpm = (peaks * 60) / secs;
    /* 合理范围 30..120（去抖 min=500ms 物理上限就是 120 BPM） */
    if (bpm < 30 || bpm > 120) {
        return -1;
    }
    return bpm;
}

void max30100_deinit(void)
{
    if (g_int_ok) {
        gpio_handle_close(&g_int);
        g_int_ok = 0;
    }
    if (g_fd >= 0) {
        /* 关掉 sensor 省电：MODE = SHDN(bit6) */
        write_reg(REG_MODE_CONFIG, 0x80);
        i2c_hal_close(g_fd);
        g_fd = -1;
    }
}