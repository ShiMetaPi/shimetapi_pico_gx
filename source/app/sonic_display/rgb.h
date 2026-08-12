/*
 * rgb.h - RGB LED 控制（3 路 GPIO：R/G/B，共阴高电平点亮）。
 *
 * 接线：R=GPIO6_6，G=GPIO6_5，B=GPIO5_3。共阴模块的公共脚接 GND，拉高控制脚=亮。
 *   3 个脚默认都是 func0(GPIO)，开箱可用（pad 复用在 main.c 的 padmux_init 做）。
 * 无 PWM，颜色靠 R/G/B 三路 on/off 组合（红/绿/蓝/黄=红+绿/青/紫/白/灭）。
 */
#ifndef RGB_H
#define RGB_H

#include <stdint.h>

/* 申请 R/G/B 三个 GPIO 为输出（pad 复用在 main 做）。成功 0，失败 -1。 */
int  rgb_init(void);
void rgb_deinit(void);

/* 全灭。 */
void rgb_off(void);

/* r/g/b 取 0/1，共阴下 1=点亮该色。直接设三脚电平。 */
void rgb_set(int r, int g, int b);

/*
 * 按距离设色（倒车雷达式）并返回颜色名，用于屏上显示：
 *   d < 0            -> 蓝（无回波/超量程）
 *   d < 20 cm        -> 红（很近，危险）
 *   20 <= d < 50 cm  -> 黄（较近，注意）
 *   d >= 50 cm       -> 绿（安全）
 * 阈值在 rgb.c 的宏里改。
 */
const char *rgb_by_distance(float d_cm);

#endif /* RGB_H */
