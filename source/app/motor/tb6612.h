/*
 * tb6612.h - TB6612FNG 双路直流电机驱动（用户态）。
 *
 * TB6612 是一颗常见的双路 H 桥驱动，每路三个控制信号：
 *   IN1/IN2 选择方向（00=coast, 01=反转, 10=正转, 11=brake）
 *   PWM    速度（0..100% 占空比）
 *
 * 模块提供开箱即用的封装：初始化 7 个 GPIO（5 路方向+STBY+2 路 PWM 软件生成），
 * 控制 API 是 `tb6612_set_motor_a/b(dir, speed_pct)`，屏外只关心方向和占空比。
 *
 * 软件 PWM 说明：本平台 xmorca 没有 PWM 控制器驱动（dts 没有 pwm 子节点），
 * 所以 PWMA/PWMB 通过两个独立 pthread 做位带 PWM。频率默认 1kHz，0..100%
 * duty 分辨率 1%。
 */
#ifndef TB6612_H
#define TB6612_H

#include <stdint.h>

/* 电机方向（IN1/IN2 状态组合）。COAST 高阻、BRKE 主动刹车。 */
typedef enum {
    TB6612_COAST  = 0,   /* IN1=0, IN2=0  - 高阻，电机自由滑行 */
    TB6612_REV    = 1,   /* IN1=0, IN2=1  - 反转 */
    TB6612_FWD    = 2,   /* IN1=1, IN2=0  - 正转 */
    TB6612_BRAKE  = 3,   /* IN1=1, IN2=1  - 短路刹车 */
} tb6612_dir_t;

/* 初始化：申请 7 路 GPIO（AIN1/2, BIN1/2, PWMA/B, STBY），启动 2 路 PWM 软件线程。 */
int  tb6612_init(void);

/* 单电机控制：方向 + 占空比（0..100）。speed=0 时电机停在 COAST。 */
void tb6612_set_motor_a(tb6612_dir_t dir, int speed_pct);
void tb6612_set_motor_b(tb6612_dir_t dir, int speed_pct);

/* 同时设置两路（方便坦克左右一致动作）。 */
void tb6612_set_both(tb6612_dir_t a, int a_pct, tb6612_dir_t b, int b_pct);

/* STBY：高=驱动使能，低=整个 TB6612 进入 standby（输出高阻，静态电流 ~1µA）。 */
void tb6612_standby(int enable);

/* 关闭 PWM 线程 + 释放 GPIO。 */
void tb6612_deinit(void);

#endif /* TB6612_H */
