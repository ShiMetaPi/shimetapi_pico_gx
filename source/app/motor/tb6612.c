/*
 * tb6612.c - TB6612FNG 驱动实现（软件 PWM via pthread）。
 *
 * 7 路 GPIO（参考 7206V11A PIN_OUT 表3）：
 *   PWMA   = GPIO5_4 / iocfg_reg59 @ 0x100C003C  func0(GPIO) default 0x1000
 *   AIN2   = GPIO7_4 / iocfg_reg75 @ 0x100C007C  func0(GPIO) default 0x1000
 *   AIN1   = GPIO7_2 / iocfg_reg73 @ 0x100C0074  func0(GPIO) default 0x1000
 *   STBY   = GPIO5_2 / iocfg_reg57 @ 0x100C0034  func0(GPIO) default 0x1000
 *   BIN1   = GPIO6_5 / iocfg_reg68 @ 0x100C0060  func0(GPIO) default 0x1000
 *   BIN2   = GPIO6_6 / iocfg_reg69 @ 0x100C0064  func0(GPIO) default 0x1000
 *   PWMB   = GPIO5_5 / iocfg_reg60 @ 0x100C0040  func0(GPIO) default 0x1000
 *
 * 所有 7 个 pad 默认就是 GPIO 功能（func=0, 0x1000），写一次显式 set_pad_func
 * 即可确保电源状态变化后状态稳定。
 *
 * PWM：xmorca 没有 hardware PWM 控制器驱动（dts &pwm 没有具体节点），
 * 所以开两个 pthread 做 software bit-bang PWM，频率 1kHz，duty 0..100%。
 */
#include "tb6612.h"
#include "gpio_hal.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <pthread.h>
#include <signal.h>

#define PAGE_SIZE  0x1000u

/* 各 pad 的物理地址 + func 值（func 0 默认 GPIO + bit12 输入使能）。 */
static const struct {
    const char *name;
    const char *chip;
    unsigned int line;
    uint32_t    pad_addr;
    uint32_t    func_val;
} g_pins[] = {
    { "PWMA",  "/dev/gpiochip5", 4,  0x100C003Cu, 0x1000u },
    { "AIN2",  "/dev/gpiochip7", 4,  0x100C007Cu, 0x1000u },
    { "AIN1",  "/dev/gpiochip7", 2,  0x100C0074u, 0x1000u },
    { "STBY",  "/dev/gpiochip5", 2,  0x100C0034u, 0x1000u },
    { "BIN1",  "/dev/gpiochip6", 5,  0x100C0060u, 0x1000u },
    { "BIN2",  "/dev/gpiochip6", 6,  0x100C0064u, 0x1000u },
    { "PWMB",  "/dev/gpiochip5", 5,  0x100C0040u, 0x1000u },
};

enum {
    IDX_PWMA = 0,
    IDX_AIN2 = 1,
    IDX_AIN1 = 2,
    IDX_STBY = 3,
    IDX_BIN1 = 4,
    IDX_BIN2 = 5,
    IDX_PWMB = 6,
    N_PINS   = 7,
};

static gpio_handle_t g_h[N_PINS];

/* 状态（pthread 之间共享，用 mutex 保护）。 */
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_t       g_thr_a, g_thr_b;
static volatile int    g_run = 0;
static int             g_pwm_a_pct = 0;  /* 0..100 */
static int             g_pwm_b_pct = 0;
static int             g_stby     = 0;  /* 0/1 */

/* ---------- pad 切到 GPIO（func 写 0x1000） ---------- */
static void set_pad_func(uint32_t phys, uint32_t func_val)
{
    int fd = open("/dev/mem", O_RDWR);
    if (fd < 0) {
        perror("[tb6612] open /dev/mem");
        return;
    }
    volatile uint8_t *m = (volatile uint8_t *)mmap(NULL, PAGE_SIZE,
            PROT_READ | PROT_WRITE, MAP_SHARED, fd,
            phys & ~(PAGE_SIZE - 1u));
    if (m == MAP_FAILED) {
        perror("[tb6612] mmap pad");
        close(fd);
        return;
    }
    volatile uint32_t *pad = (volatile uint32_t *)(m + (phys & (PAGE_SIZE - 1u)));
    *pad = func_val;
    printf("[tb6612] pad 0x%08X -> 0x%08X\n", phys, *pad);
    munmap((void *)m, PAGE_SIZE);
    close(fd);
}

/* ---------- 软件 PWM 线程 ----------
 * 1kHz 周期（1000us），duty 0..100，对应 0..1000us 高电平，剩余低电平。
 * usleep 精度有限但 1% 在这个速率下电机表现稳定。 */
#define PWM_PERIOD_US  1000
#define PWM_UNIT_US    10     /* 1% = 10us */

static void *pwm_thread(void *arg)
{
    int idx = (int)(intptr_t)arg;          /* IDX_PWMA 或 IDX_PWMB */
    int *pct = (idx == IDX_PWMA) ? &g_pwm_a_pct : &g_pwm_b_pct;
    int last_pct = -1;

    while (g_run) {
        int cur;
        pthread_mutex_lock(&g_lock);
        cur = *pct;
        pthread_mutex_unlock(&g_lock);

        if (cur != last_pct) {
            /* duty 0：直接拉低，sleep 一个周期就跳过 */
            if (cur <= 0) {
                gpio_set_value(&g_h[idx], 0);
                usleep(PWM_PERIOD_US);
                last_pct = cur;
                continue;
            }
            if (cur >= 100) {
                /* duty 100：拉高（占空比持续，需要在循环里维持） */
                gpio_set_value(&g_h[idx], 1);
                usleep(PWM_PERIOD_US);
                last_pct = cur;
                continue;
            }
            last_pct = cur;
        }

        if (cur > 0 && cur < 100) {
            int on_us  = cur * PWM_UNIT_US;
            int off_us = PWM_PERIOD_US - on_us;
            if (on_us > 0)  gpio_set_value(&g_h[idx], 1);
            if (on_us > 0)  usleep(on_us);
            gpio_set_value(&g_h[idx], 0);
            if (off_us > 0) usleep(off_us);
        } else if (cur == 0) {
            gpio_set_value(&g_h[idx], 0);
            usleep(PWM_PERIOD_US);
        } else {
            gpio_set_value(&g_h[idx], 1);
            usleep(PWM_PERIOD_US);
        }
    }
    return NULL;
}

/* ---------- 把 TB6612 方向枚举翻译成 IN1/IN2 ---------- */
static void apply_dir_a(tb6612_dir_t d)
{
    int in1 = (d == TB6612_FWD || d == TB6612_BRAKE) ? 1 : 0;
    int in2 = (d == TB6612_REV || d == TB6612_BRAKE) ? 1 : 0;
    gpio_set_value(&g_h[IDX_AIN1], in1);
    gpio_set_value(&g_h[IDX_AIN2], in2);
}

static void apply_dir_b(tb6612_dir_t d)
{
    int in1 = (d == TB6612_FWD || d == TB6612_BRAKE) ? 1 : 0;
    int in2 = (d == TB6612_REV || d == TB6612_BRAKE) ? 1 : 0;
    gpio_set_value(&g_h[IDX_BIN1], in1);
    gpio_set_value(&g_h[IDX_BIN2], in2);
}

static int clamp_pct(int pct)
{
    if (pct < 0)   return 0;
    if (pct > 100) return 100;
    return pct;
}

/* ---------- 公开 API ---------- */

int tb6612_init(void)
{
    int i;

    /* 1) 全部 pad 显式写一次 0x1000（默认就是 GPIO，写稳一下） */
    for (i = 0; i < N_PINS; i++) {
        set_pad_func(g_pins[i].pad_addr, g_pins[i].func_val);
    }

    /* 2) 申请 GPIO，全部输出、默认低 */
    for (i = 0; i < N_PINS; i++) {
        memset(&g_h[i], 0, sizeof(g_h[i]));
        g_h[i].chip_path   = g_pins[i].chip;
        g_h[i].line_offset = g_pins[i].line;
        g_h[i].gpio_mode   = GPIOHANDLE_REQUEST_OUTPUT;
        g_h[i].default_value = 0;
        snprintf(g_h[i].consumer_label, sizeof(g_h[i].consumer_label),
                 "tb6612-%s", g_pins[i].name);
        if (gpio_handle_init(&g_h[i]) < 0) {
            fprintf(stderr, "[tb6612] GPIO %s init 失败: %s\n",
                    g_pins[i].name, strerror(errno));
            return -1;
        }
        printf("[tb6612] %s ok (chip=%s line=%u)\n",
               g_pins[i].name, g_pins[i].chip, g_pins[i].line);
    }

    /* 3) STBY 默认高（驱动使能） */
    g_stby = 1;
    gpio_set_value(&g_h[IDX_STBY], 1);

    /* 4) 启动两个 PWM 线程（g_run 由 mutex + flag 同步） */
    g_run = 1;
    if (pthread_create(&g_thr_a, NULL, pwm_thread,
                       (void *)(intptr_t)IDX_PWMA) != 0) {
        fprintf(stderr, "[tb6612] pthread PWMA 失败\n");
        return -1;
    }
    if (pthread_create(&g_thr_b, NULL, pwm_thread,
                       (void *)(intptr_t)IDX_PWMB) != 0) {
        fprintf(stderr, "[tb6612] pthread PWMB 失败\n");
        return -1;
    }

    /* 5) 初始状态：两路 COAST + 0 速 */
    apply_dir_a(TB6612_COAST);
    apply_dir_b(TB6612_COAST);
    pthread_mutex_lock(&g_lock);
    g_pwm_a_pct = 0;
    g_pwm_b_pct = 0;
    pthread_mutex_unlock(&g_lock);

    printf("[tb6612] init ok, 两路 1kHz 软件 PWM 已启动\n");
    return 0;
}

void tb6612_set_motor_a(tb6612_dir_t dir, int speed_pct)
{
    apply_dir_a(dir);
    pthread_mutex_lock(&g_lock);
    g_pwm_a_pct = clamp_pct(speed_pct);
    pthread_mutex_unlock(&g_lock);
}

void tb6612_set_motor_b(tb6612_dir_t dir, int speed_pct)
{
    apply_dir_b(dir);
    pthread_mutex_lock(&g_lock);
    g_pwm_b_pct = clamp_pct(speed_pct);
    pthread_mutex_unlock(&g_lock);
}

void tb6612_set_both(tb6612_dir_t a, int a_pct, tb6612_dir_t b, int b_pct)
{
    apply_dir_a(a);
    apply_dir_b(b);
    pthread_mutex_lock(&g_lock);
    g_pwm_a_pct = clamp_pct(a_pct);
    g_pwm_b_pct = clamp_pct(b_pct);
    pthread_mutex_unlock(&g_lock);
}

void tb6612_standby(int enable)
{
    g_stby = enable ? 1 : 0;
    gpio_set_value(&g_h[IDX_STBY], g_stby);
    if (!g_stby) {
        /* 进 standby：占空比归 0，避免松开 STBY 后电机突然暴走 */
        pthread_mutex_lock(&g_lock);
        g_pwm_a_pct = 0;
        g_pwm_b_pct = 0;
        pthread_mutex_unlock(&g_lock);
    }
}

void tb6612_deinit(void)
{
    /* 停 PWM 线程 */
    g_run = 0;
    pthread_join(g_thr_a, NULL);
    pthread_join(g_thr_b, NULL);

    /* 进 STBY 拉低 */
    gpio_set_value(&g_h[IDX_STBY], 0);

    /* 释放 GPIO */
    for (int i = 0; i < N_PINS; i++) {
        gpio_handle_close(&g_h[i]);
    }
    printf("[tb6612] deinit ok\n");
}
