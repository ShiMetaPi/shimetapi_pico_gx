/*
 * ssd1306.h - SSD1306 OLED 显示驱动（128x64, I2C）。
 *
 * 采用「显存帧缓冲 + 整屏刷新」模型：所有绘图 API 只改内存帧缓冲，
 * 调用 ssd1306_flush() 时一次性把整屏 1024 字节经 I2C 写入 OLED。
 */
#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>

/* ---- 硬件配置（按板子接线修改）---- */
#ifndef OLED_I2C_BUS
#define OLED_I2C_BUS       3       /* GPIO4_1(SCL)/GPIO4_2(SDA) = i2c_bus3 -> /dev/i2c-3 */
#endif
#ifndef OLED_I2C_ADDR
#define OLED_I2C_ADDR      0x3C    /* 7 位从地址，SA0=0（SA0=1 则为 0x3D） */
#endif

#define OLED_WIDTH         128
#define OLED_HEIGHT        64
#define OLED_PAGES         (OLED_HEIGHT / 8)   /* 8 */

/* 画点颜色：白=亮，黑=灭，反相=翻转 */
#define OLED_COLOR_BLACK   0
#define OLED_COLOR_WHITE   1
#define OLED_COLOR_INVERT  2

/* 打开 /dev/i2c-<bus> 并按 SSD1306 128x64 标准序列初始化。成功 0，失败 -1。 */
int  ssd1306_init(void);

/* 释放 I2C 句柄（不主动关显示，屏上保留最后一帧）。 */
void ssd1306_deinit(void);

/* 开/关显示（省电）。on=1 亮，on=0 灭。 */
void ssd1306_display_on(int on);

/* 整屏填充并立即刷新。color: OLED_COLOR_WHITE 全亮，其余全灭。 */
void ssd1306_fill(uint8_t color);

/* 清显存为黑（仅改帧缓冲，需 flush 才上屏）。 */
void ssd1306_clear(void);

/* 画点。 */
void ssd1306_draw_pixel(int x, int y, uint8_t color);

/* 画水平/垂直线段。 */
void ssd1306_draw_hline(int x, int y, int len, uint8_t color);
void ssd1306_draw_vline(int x, int y, int len, uint8_t color);

/* 画矩形框 / 实心矩形。 */
void ssd1306_draw_rect(int x, int y, int w, int h, uint8_t color);
void ssd1306_fill_rect(int x, int y, int w, int h, uint8_t color);

/* 画一个 8x16 字符，返回字符宽度。非可打印字符用空格替代。背景恒为黑。 */
int  ssd1306_draw_char(int x, int y, char ch, uint8_t color);

/* 画字符串，支持 '\n' 换行与行末自动换行。 */
void ssd1306_draw_string(int x, int y, const char *s, uint8_t color);

/* 把整屏帧缓冲一次性刷新到 OLED。 */
void ssd1306_flush(void);

#endif /* SSD1306_H */
