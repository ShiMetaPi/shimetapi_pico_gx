/*
 * rgb.c - RGB LED 控制实现（3 路 GPIO 输出，共阴）。
 *
 * 引脚：R=GPIO6_6(chip6 line6)，G=GPIO6_5(chip6 line5)，B=GPIO5_3(chip5 line3)。
 * 共阴：gpio_set_value(1)=亮，(0)=灭。
 */
#include "rgb.h"
#include "gpio_hal.h"

#include <stdio.h>

/* 距离→颜色阈值（cm）。 */
#define RGB_NEAR_RED_CM     20.0f    /* < 此距离显示红 */
#define RGB_MID_YELLOW_CM   50.0f    /* < 此距离显示黄；>= 显示绿 */

static gpio_handle_t g_r = {
    .chip_path      = "/dev/gpiochip6",
    .line_offset    = 6,                 /* GPIO6_6 = R */
    .gpio_mode      = GPIOHANDLE_REQUEST_OUTPUT,
    .default_value  = 0,
    .consumer_label = "rgb-r",
    .chip_fd        = -1,
    .line_fd        = -1,
};
static gpio_handle_t g_g = {
    .chip_path      = "/dev/gpiochip6",
    .line_offset    = 5,                 /* GPIO6_5 = G */
    .gpio_mode      = GPIOHANDLE_REQUEST_OUTPUT,
    .default_value  = 0,
    .consumer_label = "rgb-g",
    .chip_fd        = -1,
    .line_fd        = -1,
};
static gpio_handle_t g_b = {
    .chip_path      = "/dev/gpiochip5",
    .line_offset    = 3,                 /* GPIO5_3 = B */
    .gpio_mode      = GPIOHANDLE_REQUEST_OUTPUT,
    .default_value  = 0,
    .consumer_label = "rgb-b",
    .chip_fd        = -1,
    .line_fd        = -1,
};

int rgb_init(void)
{
    if (gpio_handle_init(&g_r) < 0) {
        fprintf(stderr, "[rgb] R(%s line%u) 申请输出失败。\n", g_r.chip_path, g_r.line_offset);
        return -1;
    }
    if (gpio_handle_init(&g_g) < 0) {
        fprintf(stderr, "[rgb] G(%s line%u) 申请输出失败。\n", g_g.chip_path, g_g.line_offset);
        return -1;
    }
    if (gpio_handle_init(&g_b) < 0) {
        fprintf(stderr, "[rgb] B(%s line%u) 申请输出失败。\n", g_b.chip_path, g_b.line_offset);
        return -1;
    }
    rgb_off();
    printf("[rgb] R=GPIO6_6, G=GPIO6_5, B=GPIO5_3 就绪（共阴）。\n");
    return 0;
}

void rgb_set(int r, int g, int b)
{
    gpio_set_value(&g_r, r ? 1 : 0);
    gpio_set_value(&g_g, g ? 1 : 0);
    gpio_set_value(&g_b, b ? 1 : 0);
}

void rgb_off(void)
{
    rgb_set(0, 0, 0);
}

const char *rgb_by_distance(float d)
{
    if (d < 0.0f) {
        rgb_set(0, 0, 1);                 /* 蓝：无回波/超量程 */
        return "BLUE";
    }
    if (d < RGB_NEAR_RED_CM) {
        rgb_set(1, 0, 0);                 /* 红：很近 */
        return "RED";
    }
    if (d < RGB_MID_YELLOW_CM) {
        rgb_set(1, 1, 0);                 /* 黄：较近（红+绿）*/
        return "YELLOW";
    }
    rgb_set(0, 1, 0);                     /* 绿：安全 */
    return "GREEN";
}

void rgb_deinit(void)
{
    rgb_off();
    gpio_handle_close(&g_r);
    gpio_handle_close(&g_g);
    gpio_handle_close(&g_b);
}
