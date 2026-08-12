/*
 * sonic.h - HC-SR04 超声波测距驱动（GPIO：TRIG 输出脉冲 + ECHO 输入测脉宽）。
 *
 * 接线：TRIG = GPIO6_7（/dev/gpiochip6 line7，输出），ECHO = GPIO7_0（/dev/gpiochip7 line0，输入）。
 * 原理：TRIG 给 ≥10us 高电平 -> 模块发 8×40kHz -> ECHO 拉高 -> 收到回波拉低；
 *       距离 cm = ECHO 高电平时间(us) / 58（声速 340m/s 往返，1cm≈58us）。
 * 测脉宽用 clock_gettime(CLOCK_MONOTONIC_RAW) busy-wait 轮询 ECHO，含超时保护防卡死。
 */
#ifndef SONIC_H
#define SONIC_H

#include <stdint.h>

/* ---- 引脚配置（按板子接线修改）---- */
#define SONIC_TRIG_CHIP  "/dev/gpiochip6"
#define SONIC_TRIG_LINE  7       /* GPIO6_7 */
#define SONIC_ECHO_CHIP  "/dev/gpiochip7"
#define SONIC_ECHO_LINE  0       /* GPIO7_0 */

/* 测距错误码（sonic_measure_cm 返回 <0 即错误）。 */
#define SONIC_ERR_NO_ECHO  (-1.0f)   /* 超时无上升沿：前方无障碍 / 太近(<2cm) / ECHO 未接 */
#define SONIC_ERR_TIMEOUT  (-2.0f)   /* 上升沿后超时无下降沿：超量程(>4m)或声波发散 */

/* 申请 TRIG(输出)/ECHO(输入) chardev（pad 复用在 main 里做）。成功 0，失败 -1。 */
int  sonic_init(void);
void sonic_deinit(void);

/*
 * 测一次距离。返回 cm（float，含小数）；<0 见 SONIC_ERR_*。
 * 内部含超时保护，最坏约 30ms 返回，不会卡死。
 * HC-SR04 建议两次测量间隔 ≥60ms（防上次回波串扰）。
 */
float sonic_measure_cm(void);

#endif /* SONIC_H */
