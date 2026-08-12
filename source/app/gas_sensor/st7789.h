/*
 * st7789.h - ST7789 TFT 显示驱动（240x240 RGB, 4 线 SPI + DC/RES）。
 *
 * 与 SSD1309 的根本区别：
 *   - 240x240 RGB（RGB565 16bit/像素），不是 128x64 单色。
 *   - 无"页"概念；显存按行/列窗口寻址：CASET(0x2A) 设列范围、RASET(0x2B)
 *     设行范围、RAMWR(0x2C) 之后连续送像素，写指针在窗口内自增。
 *   - 上电处于 sleep，必须 SLPOUT(0x11)+120ms 再 DISPON(0x29) 才点亮。
 *
 * 传输仍走 spi_hal（DC=0 命令 / DC=1 数据），故本驱动不含任何硬件细节。
 */
#ifndef ST7789_H
#define ST7789_H

#include <stdint.h>

#define TFT_WIDTH   240
#define TFT_HEIGHT  240

/*
 * 偏移：240x240 原生玻璃用 0/0。
 * 若画面整体偏移/被裁切（多见于 240x320 裁切型模组），改 y=80 再试。
 */
#define TFT_OFFSET_X 0
#define TFT_OFFSET_Y 0

/* RGB565 颜色（R5G6B5，逻辑序：高字节先发）。 */
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0
#define COLOR_CYAN    0x07FF
#define COLOR_MAGENTA 0xF81F

/* 8bit RGB -> RGB565。 */
#define RGB565(r, g, b) \
    ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | (((b) & 0xF8) >> 3))

/* 初始化（内部依赖 spi_hal 已复位）。 */
int  st7789_init(void);
void st7789_deinit(void);

void st7789_display_on(int on);
void st7789_invert(int on);

/* 清帧缓冲为黑（不立即刷新）；fill 立即整屏刷一色。 */
void st7789_clear(void);
void st7789_fill(uint16_t color);

void st7789_draw_pixel(int x, int y, uint16_t color);
void st7789_draw_hline(int x, int y, int len, uint16_t color);
void st7789_draw_vline(int x, int y, int len, uint16_t color);
void st7789_draw_rect(int x, int y, int w, int h, uint16_t color);
void st7789_fill_rect(int x, int y, int w, int h, uint16_t color);

/* fg=字形色，bg=字背景色（TFT 上文字需显式背景，否则拖影）。 */
int  st7789_draw_char(int x, int y, char ch, uint16_t fg, uint16_t bg);
void st7789_draw_string(int x, int y, const char *s, uint16_t fg, uint16_t bg);

/* 把帧缓冲整屏推到屏（CASET/RASET 全屏 + RAMWR）。 */
void st7789_flush(void);

#endif /* ST7789_H */
