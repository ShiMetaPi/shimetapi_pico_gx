/*
 * buzzer.c - 无源蜂鸣器控制实现（GPIO5_5 软件方波）。
 *
 * 方波靠 GPIO 翻转 + clock_gettime busy-wait 实现半周期延时，频率是近似的
 * （用户态有抖动），但无源蜂鸣器对频率不敏感，2~4kHz 都能正常响。
 */
#include "buzzer.h"
#include "gpio_hal.h"

#include <stdio.h>
#include <stdint.h>
#include <time.h>

static gpio_handle_t g_buz = {
    .chip_path      = "/dev/gpiochip5",
    .line_offset    = 5,                  /* GPIO5_5 */
    .gpio_mode      = GPIOHANDLE_REQUEST_OUTPUT,
    .default_value  = 0,
    .consumer_label = "buzzer",
    .chip_fd        = -1,
    .line_fd        = -1,
};

static int64_t now_us(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (int64_t)ts.tv_sec * 1000000 + (int64_t)ts.tv_nsec / 1000;
}

static int64_t now_ms(void)
{
    return now_us() / 1000;
}

/* busy-wait 自旋延时 us 微秒。 */
static void spin_us(uint32_t us)
{
    int64_t end = now_us() + (int64_t)us;
    while (now_us() < end) {
        /* spin */
    }
}

int buzzer_init(void)
{
    if (gpio_handle_init(&g_buz) < 0) {
        fprintf(stderr, "[buzzer] GPIO5_5(%s line%u) 申请输出失败。\n",
                g_buz.chip_path, g_buz.line_offset);
        return -1;
    }
    gpio_set_value(&g_buz, 0);
    printf("[buzzer] GPIO5_5 就绪。\n");
    return 0;
}

void buzzer_off(void)
{
    gpio_set_value(&g_buz, 0);
}

void buzzer_beep(int duration_ms, int freq_hz)
{
    uint32_t half_us;
    int64_t end_ms;
    int v = 0;

    if (g_buz.line_fd < 0 || freq_hz <= 0 || duration_ms <= 0) {
        return;
    }
    half_us = 1000000u / (uint32_t)freq_hz / 2u;   /* 半周期（us）*/
    end_ms = now_ms() + duration_ms;
    while (now_ms() < end_ms) {
        gpio_set_value(&g_buz, v);
        v ^= 1;
        spin_us(half_us);
    }
    gpio_set_value(&g_buz, 0);   /* 结束置低，彻底关闭 */
}

void buzzer_deinit(void)
{
    buzzer_off();
    gpio_handle_close(&g_buz);
}
