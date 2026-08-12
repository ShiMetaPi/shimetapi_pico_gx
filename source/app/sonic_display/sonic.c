/*
 * sonic.c - HC-SR04 超声波测距实现。
 *
 * 时序：TRIG 高 ≥10us 触发；ECHO 上升沿到下降沿的高电平时间 = 声波往返时间。
 * 距离 cm = t(us) / 58（HC-SR04 datasheet：1cm ≈ 58us，声速 340m/s 往返）。
 *
 * 精度：busy-wait 轮询 + clock_gettime(CLOCK_MONOTONIC_RAW)，用户态分辨率 us 级，
 *       对应 mm~cm 级距离；受系统调度抖动影响，板子空闲时足够。
 * 超时：上升沿/下降沿等待均 30ms 超时（量程 4m ≈ 23ms，30ms 覆盖），防 ECHO 不回卡死。
 */
#include "sonic.h"
#include "gpio_hal.h"

#include <stdio.h>
#include <stdint.h>
#include <time.h>

#define ECHO_TIMEOUT_US  30000u   /* 30ms 超时保护 */

static gpio_handle_t g_trig = {
    .chip_path      = SONIC_TRIG_CHIP,
    .line_offset    = SONIC_TRIG_LINE,
    .gpio_mode      = GPIOHANDLE_REQUEST_OUTPUT,
    .default_value  = 0,
    .consumer_label = "sonic-trig",
    .chip_fd        = -1,
    .line_fd        = -1,
};
static gpio_handle_t g_echo = {
    .chip_path      = SONIC_ECHO_CHIP,
    .line_offset    = SONIC_ECHO_LINE,
    .gpio_mode      = GPIOHANDLE_REQUEST_INPUT,
    .default_value  = 0,
    .consumer_label = "sonic-echo",
    .chip_fd        = -1,
    .line_fd        = -1,
};

/* CLOCK_MONOTONIC_RAW 当前时刻，单位微秒（不受 NTP 调整影响，分辨率最高）。 */
static int64_t now_us(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (int64_t)ts.tv_sec * 1000000 + (int64_t)ts.tv_nsec / 1000;
}

/* busy-wait 自旋延时 us 微秒（不依赖 usleep/jiffy 粒度，适合 <1ms 的短脉冲触发）。 */
static void spin_us(uint32_t us)
{
    int64_t end = now_us() + (int64_t)us;
    while (now_us() < end) {
        /* spin */
    }
}

int sonic_init(void)
{
    if (gpio_handle_init(&g_trig) < 0) {
        fprintf(stderr, "[sonic] TRIG(%s line%u) 申请输出失败。\n",
                g_trig.chip_path, g_trig.line_offset);
        return -1;
    }
    if (gpio_handle_init(&g_echo) < 0) {
        fprintf(stderr, "[sonic] ECHO(%s line%u) 申请输入失败。\n",
                g_echo.chip_path, g_echo.line_offset);
        gpio_handle_close(&g_trig);
        return -1;
    }
    gpio_set_value(&g_trig, 0);   /* TRIG 默认低，防误触发 */
    printf("[sonic] TRIG=%s line%u, ECHO=%s line%u 就绪。\n",
           g_trig.chip_path, g_trig.line_offset, g_echo.chip_path, g_echo.line_offset);
    return 0;
}

float sonic_measure_cm(void)
{
    int64_t t_start, t_rise;

    /* 1) TRIG 触发脉冲：≥10us 高电平。 */
    gpio_set_value(&g_trig, 1);
    spin_us(20);
    gpio_set_value(&g_trig, 0);

    /* 2) 等 ECHO 上升沿（变高）。 */
    t_start = now_us();
    while (gpio_get_value(&g_echo) == 0) {
        if ((uint64_t)(now_us() - t_start) > ECHO_TIMEOUT_US) {
            return SONIC_ERR_NO_ECHO;   /* 一直没拉高：无回波 */
        }
    }
    t_rise = now_us();

    /* 3) 等 ECHO 下降沿（变低）；脉宽 = 此时刻 - 上升沿时刻。 */
    while (gpio_get_value(&g_echo) == 1) {
        if ((uint64_t)(now_us() - t_rise) > ECHO_TIMEOUT_US) {
            return SONIC_ERR_TIMEOUT;   /* 一直没拉低：超量程 */
        }
    }
    int64_t width_us = now_us() - t_rise;

    /* 4) 距离 cm = us / 58（声速 340m/s 往返，1cm≈58us）。 */
    return (float)width_us / 58.0f;
}

void sonic_deinit(void)
{
    gpio_handle_close(&g_trig);
    gpio_handle_close(&g_echo);
}
