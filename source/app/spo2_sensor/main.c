/*
 * spo2_sensor - MAX30100 血氧/心率 + ST7789 SPI 屏显示（板端运行）。
 *
 * 硬件：GK7602V11A
 *   MAX30100 接 I2C3（GPIO4_1=SCL, GPIO4_2=SDA -> /dev/i2c-3），地址 0x57，
 *   与 OLED(0x3C)/MPU6050(0x68)/SHT20(0x40) 共享总线。
 *   INT 接 GPIO4_4（/dev/gpiochip4 line 4，本程序仅作输入检测/可选）。
 *   IRD/RD 悬空（sensor 用内部 LED）。
 *   数据显示在 SPI 屏（ST7789 240x240，/dev/spidev2.0）。
 *
 * 屏上：大字 IR 原始值 + R 原始值 + HR(BPM) + SpO2(%)。每 100ms 刷屏，Ctrl+C 退出。
 *
 * 注：本程序只读 FIFO 出原始 IR/R 值 + 简单峰值检测算 HR，SpO2 是 R/IR 的简单比值
 *     估算。医学级判断需要更复杂的算法 + 个体校准，这里仅作演示。
 */
#include "st7789.h"
#include "spi_hal.h"
#include "max30100.h"
#include "font8x16.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>

/*
 * pad 复用（查 7206V11A PIN_OUT 表3）：
 *   I2C3_SCL   = GPIO4_1 = iocfg_reg48 @ 0x100C0010  func2 -> 0x1002
 *   I2C3_SDA   = GPIO4_2 = iocfg_reg49 @ 0x100C0014  func2 -> 0x1002
 *   GPIO4_4    = iocfg_reg51 @ 0x100C001C  func5 -> 0x1005（默认 JTAG_TDI，要写 func5）
 *
 * SPI 屏的 pad 由 spi_hal_init() 内部切，这里不管。
 */
#define PAGE_SIZE     0x1000u
#define I2C3_SCL_PAD  0x100C0010u
#define I2C3_SDA_PAD  0x100C0014u
#define I2C3_FUNC_VAL 0x1002u
#define INT_PAD       0x100C001Cu
#define INT_FUNC_VAL  0x1005u

static void set_pad_func(uint32_t phys, uint32_t func_val)
{
    int fd = open("/dev/mem", O_RDWR);
    if (fd < 0) {
        perror("[max] open /dev/mem");
        return;
    }
    volatile uint8_t *m = (volatile uint8_t *)mmap(NULL, PAGE_SIZE,
            PROT_READ | PROT_WRITE, MAP_SHARED, fd, phys & ~(PAGE_SIZE - 1u));
    if (m == MAP_FAILED) {
        perror("[max] mmap pad");
        close(fd);
        return;
    }
    volatile uint32_t *pad = (volatile uint32_t *)(m + (phys & (PAGE_SIZE - 1u)));
    *pad = func_val;
    printf("[max] pad 0x%08X -> 0x%08X\n", phys, *pad);
    munmap((void *)m, PAGE_SIZE);
    close(fd);
}

static void padmux_init(void)
{
    set_pad_func(I2C3_SCL_PAD, I2C3_FUNC_VAL);
    set_pad_func(I2C3_SDA_PAD, I2C3_FUNC_VAL);
    /* INT_PAD 不切：pin89 (LCD_DE) 默认 func=0 状态（JTAG_TDI），保持原样。
     * 不切到 func5(GPIO) 的话，MAX30100 INT 这边也不会去申请 line 4 之外的冲突。
     * 物理上 MAX30100 的 INT 引脚请悬空或根本不接，避免拉到屏的 RES。 */
    /* set_pad_func(INT_PAD, INT_FUNC_VAL);   ← 故意不切，注释保留方便回看 */
}

static volatile sig_atomic_t g_exit = 0;
static void on_signal(int sig) { (void)sig; g_exit = 1; }

/* ---------- 采样线程 ↔ 屏刷线程共享数据 ----------
 * 架构：单 sample ring buffer（512 个 slot），不平均、不累积。
 *   采样线程：20ms 读 FIFO，每个 raw sample 直接 push 到 ring
 *   屏刷线程：200ms 醒来，把 ring 里的所有 sample 灌进 hist（hist 是 50 SPS 原始数据）
 *
 * 之前用「每 slot 累积 5 sample 平均」的架构把 1Hz 脉搏信号抹平了——50Hz 脉搏经过 100ms+200ms 两级
 * 平均，幅度基本被吃光。改成原始 sample 后 hist 有真信号可算。 */
#define SAMPLE_RING_SIZE  512
typedef struct {
    uint32_t ir[SAMPLE_RING_SIZE];
    uint32_t r [SAMPLE_RING_SIZE];
    volatile int wr;       /* sample_thread 写 */
    volatile int rd;       /* main 线程读 */
    volatile int ovf;      /* 累计 OVF */
} sample_ring_t;

static sample_ring_t g_ring = { .wr = 0, .rd = 0, .ovf = 0 };
static pthread_mutex_t g_ring_lock = PTHREAD_MUTEX_INITIALIZER;

static void ring_push(int n, const uint32_t *ir_src, const uint32_t *r_src, int ovf)
{
    pthread_mutex_lock(&g_ring_lock);
    for (int i = 0; i < n; i++) {
        int next = (g_ring.wr + 1) % SAMPLE_RING_SIZE;
        if (next == g_ring.rd) {
            /* ring 满：丢最早一个（推进 rd） */
            g_ring.rd = (g_ring.rd + 1) % SAMPLE_RING_SIZE;
        }
        g_ring.ir[g_ring.wr] = ir_src[i];
        g_ring.r [g_ring.wr] = r_src[i];
        g_ring.wr = next;
    }
    g_ring.ovf += ovf;
    pthread_mutex_unlock(&g_ring_lock);
}

/* 从 ring 弹出至多 max_out 个 sample 到 out_ir/out_r。返回实际弹出数。 */
static int ring_pop(int max_out, uint32_t *out_ir, uint32_t *out_r, int *out_ovf)
{
    pthread_mutex_lock(&g_ring_lock);
    int n = 0;
    while (n < max_out && g_ring.wr != g_ring.rd) {
        out_ir[n] = g_ring.ir[g_ring.rd];
        out_r [n] = g_ring.r [g_ring.rd];
        g_ring.rd = (g_ring.rd + 1) % SAMPLE_RING_SIZE;
        n++;
    }
    *out_ovf = g_ring.ovf;
    g_ring.ovf = 0;
    pthread_mutex_unlock(&g_ring_lock);
    return n;
}

/* 采样线程：100ms 节奏读 FIFO，每次 burst 拿到 ~5 个 sample（50Hz SR 下），
 * 直接 push 到 ring（不平均）。100ms 周期兼顾了 burst 读效率（n>1）
 * 和屏刷节奏（主线程 200ms 一次），I2C 总线压力比之前 20ms 轮询降 5 倍。 */
static void *sample_thread(void *arg)
{
    (void)arg;
    int log_cnt = 0;

    while (!g_exit) {
        uint32_t ir_tmp[MAX30100_FIFO_DEPTH];
        uint32_t r_tmp[MAX30100_FIFO_DEPTH];
        int n = 0, ovf = 0;
        int rc = max30100_read_fifo(ir_tmp, r_tmp, &n, &ovf);
        if (rc == 0 && n > 0) {
            ring_push(n, ir_tmp, r_tmp, ovf);
        }
        if (++log_cnt % 5 == 0) {  /* 5×100ms = 500ms 打印一次 */
            printf("[max.dbg] read_fifo rc=%d n=%d ovf=%d (ring wr=%d rd=%d)\n",
                   rc, n, ovf, g_ring.wr, g_ring.rd);
        }
        usleep(100000);
    }
    return NULL;
}

/* 大字辅助（复用 sht20_display 的画法）。 */
static void draw_big_char(int x, int y, char ch, uint16_t fg, uint16_t bg, int scale)
{
    unsigned int idx = (unsigned int)(unsigned char)ch;
    const uint8_t *glyph;
    if (idx < FONT8X16_FIRST || idx >= FONT8X16_FIRST + FONT8X16_COUNT) {
        idx = FONT8X16_FIRST;
    }
    glyph = font8x16[idx - FONT8X16_FIRST];
    for (int row = 0; row < FONT8X16_H_; row++) {
        uint8_t line = glyph[row];
        for (int col = 0; col < FONT8X16_W; col++) {
            uint16_t c = (line & (uint8_t)(0x80u >> col)) ? fg : bg;
            st7789_fill_rect(x + col * scale, y + row * scale, scale, scale, c);
        }
    }
}
static void draw_big_string(int x, int y, const char *s, uint16_t fg, uint16_t bg, int scale)
{
    int cx = x;
    for (; s != NULL && *s != '\0'; s++) {
        draw_big_char(cx, y, *s, fg, bg, scale);
        cx += FONT8X16_W * scale;
    }
}

/* 屏显：标题 + IR/R 大字 + HR / SpO2 简表。
 * n == 0 时 IR/R 画 "--"（数据未攒出来，避免误判为传感器坏了）。 */
static void draw_data(int n, uint32_t ir_avg, uint32_t r_avg, int bpm, int spo2_pct)
{
    char buf[24];
    const int scale = 2;

    st7789_clear();
    st7789_draw_string(0, 0, "MAX30100", COLOR_CYAN, COLOR_BLACK);
    snprintf(buf, sizeof(buf), "I2C3 0x57 n=%d", n);
    st7789_draw_string(0, 18, buf, COLOR_WHITE, COLOR_BLACK);
    st7789_draw_hline(0, 36, TFT_WIDTH, COLOR_WHITE);

    /* IR 大字 */
    st7789_draw_string(0, 44, "IR", COLOR_YELLOW, COLOR_BLACK);
    if (n > 0) {
        snprintf(buf, sizeof(buf), "%lu", (unsigned long)ir_avg);
    } else {
        snprintf(buf, sizeof(buf), "--");
    }
    {
        int w = (int)strlen(buf) * FONT8X16_W * scale;
        draw_big_string((TFT_WIDTH - w) / 2, 60, buf, COLOR_WHITE, COLOR_BLACK, scale);
    }

    /* R 大字 */
    st7789_draw_string(0, 100, "R ", COLOR_YELLOW, COLOR_BLACK);
    if (n > 0) {
        snprintf(buf, sizeof(buf), "%lu", (unsigned long)r_avg);
    } else {
        snprintf(buf, sizeof(buf), "--");
    }
    {
        int w = (int)strlen(buf) * FONT8X16_W * scale;
        draw_big_string((TFT_WIDTH - w) / 2, 116, buf, COLOR_CYAN, COLOR_BLACK, scale);
    }

    /* HR + SpO2 一行 */
    st7789_draw_string(0, 156, "HR:", COLOR_YELLOW, COLOR_BLACK);
    if (bpm > 0) {
        snprintf(buf, sizeof(buf), "%d BPM", bpm);
    } else {
        snprintf(buf, sizeof(buf), "--");
    }
    st7789_draw_string(40, 156, buf, COLOR_WHITE, COLOR_BLACK);

    st7789_draw_string(0, 178, "SpO2:", COLOR_YELLOW, COLOR_BLACK);
    if (spo2_pct > 0) {
        snprintf(buf, sizeof(buf), "%d%%", spo2_pct);
    } else {
        snprintf(buf, sizeof(buf), "--");
    }
    st7789_draw_string(60, 178, buf, COLOR_GREEN, COLOR_BLACK);

    st7789_draw_string(0, 224, "Ctrl+C quit", COLOR_BLUE, COLOR_BLACK);
    st7789_flush();
}

static void draw_error(const char *msg)
{
    st7789_clear();
    st7789_draw_string(0, 0, "MAX30100 FAIL", COLOR_RED, COLOR_BLACK);
    st7789_draw_string(0, 24, msg, COLOR_WHITE, COLOR_BLACK);
    st7789_draw_string(0, 60, "check:", COLOR_YELLOW, COLOR_BLACK);
    st7789_draw_string(0, 80, "SCL=GPIO4_1  SDA=GPIO4_2", COLOR_WHITE, COLOR_BLACK);
    st7789_draw_string(0, 98, "INT=GPIO4_4", COLOR_WHITE, COLOR_BLACK);
    st7789_draw_string(0, 116, "ADDR 0x57  VCC 3.3V", COLOR_WHITE, COLOR_BLACK);
    st7789_flush();
}

static void usage(const char *prog)
{
    printf("用法: %s [-h]\n"
           "  采样线程每 100ms 读一次 FIFO（50Hz SR，每次 ~5 个 sample），\n"
           "  主线程每 200ms 刷一次屏，Ctrl+C 退出。\n"
           "  地址在 max30100.h、引脚在 main.c 顶部修改后重新编译。\n",
           prog);
}

int main(int argc, char **argv)
{
    struct sigaction sa;
    uint32_t ir_buf[MAX30100_FIFO_DEPTH];
    uint32_t r_buf[MAX30100_FIFO_DEPTH];
    uint32_t hist_ir[512];
    uint32_t hist_r [512];
    int hist_n = 0;
    int hist_sp;
    pthread_t thr;

    (void)ir_buf;  /* 主线程不再直接 read_fifo，引用清零 */
    (void)r_buf;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            usage(argv[0]);
            return 0;
        }
    }
    setvbuf(stdout, NULL, _IONBF, 0);
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    printf("[max] pad 复用：I2C3(4_1/4_2)->func2，GPIO4_4->func5\n");
    padmux_init();

    printf("[max] 初始化 SPI 屏（/dev/spidev2.0）...\n");
    if (spi_hal_init() < 0) {
        fprintf(stderr, "[max] SPI 传输层初始化失败。\n");
        return 1;
    }
    st7789_init();

    printf("[max] 初始化 MAX30100（/dev/i2c-%d addr 0x%02x）...\n",
           MAX30100_BUS, MAX30100_ADDR);
    if (max30100_init() < 0) {
        fprintf(stderr, "[max] MAX30100 初始化失败，屏上显示排查提示。\n");
        draw_error("read revision failed");
        sleep(3);
        st7789_clear(); st7789_flush();
        st7789_display_on(0); st7789_deinit();
        return 1;
    }

    hist_sp = 50;  /* SpO2 取最近 1 秒（50 sample @ 50 SPS）算 R/IR 均值；BPM 用全 hist（10 秒） */
    printf("[max] 采集中，50 SPS；采样线程 100ms/次，屏刷 200ms/次，Ctrl+C 退出。\n");

    /* 启动采样线程（100ms 节奏读 FIFO） */
    if (pthread_create(&thr, NULL, sample_thread, NULL) != 0) {
        fprintf(stderr, "[max] 采样线程启动失败\n");
        max30100_deinit();
        st7789_clear(); st7789_flush();
        st7789_display_on(0); st7789_deinit();
        return 1;
    }

    /* 主线程：每 200ms 醒一次，从 ring 把所有待取 sample 灌进 hist。
     * hist 是 50 SPS 原始数据，est_bpm 直接处理。 */
    while (!g_exit) {
        uint32_t tmp_ir[64], tmp_r[64];
        int ovf = 0;
        int n_pop = ring_pop(64, tmp_ir, tmp_r, &ovf);
        if (n_pop <= 0) {
            /* ring 空，等下一个 200ms */
            usleep(200000);
            draw_data(0, 0, 0, -1, -1);
            continue;
        }
        /* 把所有 sample 灌进 hist（超过 hist 容量时滚窗） */
        for (int i = 0; i < n_pop; i++) {
            if (hist_n < (int)(sizeof(hist_ir)/sizeof(hist_ir[0]))) {
                hist_ir[hist_n] = tmp_ir[i];
                hist_r [hist_n] = tmp_r [i];
                hist_n++;
            } else {
                memmove(hist_ir, hist_ir + 1, (hist_n - 1) * sizeof(hist_ir[0]));
                memmove(hist_r,  hist_r  + 1, (hist_n - 1) * sizeof(hist_r[0]));
                hist_ir[hist_n - 1] = tmp_ir[i];
                hist_r [hist_n - 1] = tmp_r [i];
            }
        }
        /* 屏上画最新一个 sample 的瞬时值（不平均） */
        uint32_t ir_disp = tmp_ir[n_pop - 1];
        uint32_t r_disp  = tmp_r [n_pop - 1];

        int bpm = -1, spo2_pct = -1;
        if (hist_n >= hist_sp) {
            /* hist 是 50 SPS 原始数据，sps=50 */
            bpm = max30100_est_bpm(hist_ir, hist_n, 50);

            /* R/IR 用最近 hist_sp 个 sample 的均值算 SpO2（滤掉单 sample 噪声） */
            uint32_t ir_sum = 0, r_sum = 0;
            int n = hist_n < hist_sp ? hist_n : hist_sp;
            for (int i = hist_n - n; i < hist_n; i++) {
                ir_sum += hist_ir[i];
                r_sum  += hist_r [i];
            }
            double ir_avg = (double)ir_sum / (double)n;
            double r_avg  = (double)r_sum  / (double)n;
            /* 屏上 SpO2 用均值 R/IR 算的简单公式，仅作演示 */
            double ratio = r_avg / (ir_avg + 1.0);
            int pct = (int)(110.0 - 30.0 * ratio);
            if (pct < 70)  pct = 70;
            if (pct > 100) pct = 100;
            spo2_pct = pct;
        }
        draw_data(n_pop, ir_disp, r_disp, bpm, spo2_pct);
        if (ovf > 0) {
            printf("[max] ring ovf=%d (n_pop=%d)\n", ovf, n_pop);
        }
        if (bpm > 0) {
            printf("[max] IR=%lu R=%lu  BPM=%d SpO2=%d%% (hist_n=%d)\n",
                   (unsigned long)ir_disp, (unsigned long)r_disp, bpm, spo2_pct, hist_n);
        }
    }

    pthread_join(thr, NULL);
    max30100_deinit();
    st7789_clear(); st7789_flush();
    st7789_display_on(0); st7789_deinit();
    printf("[max] 已清屏关显示，退出。\n");
    return 0;
}