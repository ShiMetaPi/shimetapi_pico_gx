/*
 * buzzer.h - 无源蜂鸣器控制（GPIO5_5 软件方波）。
 *
 * 无源蜂鸣器没有内部振荡器，必须用方波驱动（给直流只咔一声不响）。
 * 本驱动用 GPIO5_5 输出软件方波（busy-wait 翻转），频率/时长可设。
 *   pad 复用在 main 里做（GPIO5_5 = iocfg_reg60 @ 0x100C0040，默认 func0(GPIO)）。
 *   GPIO5_5 也能复用 func1=PWM0，需要精确频率/长响可改硬件 PWM。
 */
#ifndef BUZZER_H
#define BUZZER_H

/* 申请 GPIO5_5 为输出（pad 复用在 main 做）。成功 0，失败 -1。 */
int  buzzer_init(void);
void buzzer_deinit(void);

/* 置低（静音）。 */
void buzzer_off(void);

/*
 * 响 duration_ms 毫秒：用 freq_hz 方波驱动 GPIO5_5，结束置低。
 * 典型无源蜂鸣器频率 2~4kHz（2000~4000），过低闷、过高尖。busy-wait 占 CPU。
 */
void buzzer_beep(int duration_ms, int freq_hz);

#endif /* BUZZER_H */
