/*
 * st7789.c - ST7789 TFT 显示驱动实现（240x240 RGB565, 4 线 SPI）。
 *
 * 帧缓冲：g_fb 为全屏像素，按"高字节、低字节"的 wire 序存放（ST7789 要求
 * 16bit 像素 MSB 先发），故 flush 可直接整段送出，无需字节序转换。
 * 绘图 (x,y) -> g_fb[(y*W + x)*2 .. +1]。
 */
#include "st7789.h"
#include "spi_hal.h"
#include "font8x16.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

static uint8_t g_fb[TFT_WIDTH * TFT_HEIGHT * 2];

/* 调试用：给每次 flush 编号，便于定位第几次刷新卡住。 */
static int s_flush_cnt = 0;

/* ST7789 常用命令。 */
#define ST7789_SLPOUT  0x11
#define ST7789_INVOFF  0x20
#define ST7789_INVON   0x21
#define ST7789_DISPOFF 0x28
#define ST7789_DISPON  0x29
#define ST7789_CASET   0x2A
#define ST7789_RASET   0x2B
#define ST7789_RAMWR   0x2C
#define ST7789_MADCTL  0x36
#define ST7789_COLMOD  0x3A
#define ST7789_PORCTRL 0xB2
#define ST7789_GCTRL   0xB7
#define ST7789_VCOMS   0xBB
#define ST7789_LCMCTRL 0xC0
#define ST7789_VDVVRHEN 0xC2
#define ST7789_VRHS    0xC3
#define ST7789_VDVS    0xC4
#define ST7789_FRCTRL2 0xC6
#define ST7789_PWCTRL1 0xD0
#define ST7789_GMCTRP1 0xE0
#define ST7789_GMCTRN1 0xE1

/*
 * spidev 单笔事务上限：com-mode=POLLING 下实测 128B 触发 1s 超时返回 EAGAIN——
 * 驱动按 vendor_lotus.fifodepth=256 一次性把整片塞进 TX FIFO，但本 SoC 实际 TX FIFO
 * 不足 128，溢出后只有前若干字节真正发出、RX 永远收不满 → 超时(dmesg 可见 timeout
 * dump)。init 的小事务(≤14B)均正常，故分片到 16B。flush 共 7200 片，1MHz 下约 2s/帧。
 * 待确认 16B 稳定后，可往上试 32/64 找真实 FIFO 上限。
 */
#define ST7789_TX_CHUNK 16

static inline void fb_set(int x, int y, uint16_t color)
{
    int i;
    if ((unsigned)x >= TFT_WIDTH || (unsigned)y >= TFT_HEIGHT) {
        return;
    }
    i = (y * TFT_WIDTH + x) * 2;
    g_fb[i]     = (uint8_t)(color >> 8);   /* 高字节先发：R5G3 */
    g_fb[i + 1] = (uint8_t)(color & 0xFF); /* 低字节后发：G3B5 */
}

static void cmd1(uint8_t c)
{
    spi_hal_write_cmd(c);
}

static void cmd(uint8_t c, const uint8_t *d, int n)
{
    spi_hal_write_cmd(c);
    if (n > 0) {
        spi_hal_write_data(d, n);
    }
}

/* 设写入窗口（含偏移），之后 RAMWR 的像素落在该窗口内自增填充。 */
static void set_window(int x0, int y0, int x1, int y1)
{
    uint8_t cx[4];
    uint8_t cy[4];

    cx[0] = (uint8_t)((x0 + TFT_OFFSET_X) >> 8);
    cx[1] = (uint8_t)((x0 + TFT_OFFSET_X) & 0xFF);
    cx[2] = (uint8_t)((x1 + TFT_OFFSET_X) >> 8);
    cx[3] = (uint8_t)((x1 + TFT_OFFSET_X) & 0xFF);
    cy[0] = (uint8_t)((y0 + TFT_OFFSET_Y) >> 8);
    cy[1] = (uint8_t)((y0 + TFT_OFFSET_Y) & 0xFF);
    cy[2] = (uint8_t)((y1 + TFT_OFFSET_Y) >> 8);
    cy[3] = (uint8_t)((y1 + TFT_OFFSET_Y) & 0xFF);
    cmd(ST7789_CASET, cx, 4);
    cmd(ST7789_RASET, cy, 4);
}

int st7789_init(void)
{
    uint8_t madctl = 0x00;   /* RGB 序；若红蓝互换改 0x08(BGR) */
    uint8_t colmod = 0x05;   /* DBI=101 -> 16bit RGB565（对照 TFT 例程用 0x05）*/

    /* 硬件复位已由 spi_hal_init 完成。 */
    printf("[tft] init: SLPOUT\n");
    cmd1(ST7789_SLPOUT);     /* 退出 sleep */
    usleep(120000);          /* datasheet：SLPOUT 后须等 120ms 再发后续命令 */
    printf("[tft] init: SLPOUT +120ms ok\n");

    cmd(ST7789_MADCTL, &madctl, 1);
    cmd(ST7789_COLMOD, &colmod, 1);

    cmd(ST7789_PORCTRL, (const uint8_t[]){0x0C, 0x0C, 0x00, 0x33, 0x33}, 5);
    cmd(ST7789_GCTRL,   (const uint8_t[]){0x35}, 1);
    cmd(ST7789_VCOMS,   (const uint8_t[]){0x19}, 1);
    cmd(ST7789_LCMCTRL, (const uint8_t[]){0x2C}, 1);
    cmd(ST7789_VDVVRHEN,(const uint8_t[]){0x01}, 1);
    cmd(ST7789_VRHS,    (const uint8_t[]){0x0B}, 1);
    cmd(ST7789_VDVS,    (const uint8_t[]){0x20}, 1);
    cmd(ST7789_FRCTRL2, (const uint8_t[]){0x0F}, 1);
    cmd(ST7789_PWCTRL1, (const uint8_t[]){0xA4, 0xA1}, 2);
    cmd(ST7789_GMCTRP1, (const uint8_t[]){0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B,
                                          0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B,
                                          0x1F, 0x23}, 14);
    cmd(ST7789_GMCTRN1, (const uint8_t[]){0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C,
                                          0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F,
                                          0x20, 0x23}, 14);

    printf("[tft] init: config cmds ok\n");
    cmd1(ST7789_INVON);      /* 反色开：多数 ST7789 模组需要，否则偏暗/反色 */

    printf("[tft] init: DISPON\n");
    cmd1(ST7789_DISPON);     /* 点亮 */
    usleep(20000);
    printf("[tft] init: DISPON ok\n");

    printf("[tft] init: clear-flush start\n");
    st7789_clear();
    st7789_flush();
    printf("[tft] init: clear-flush done\n");
    return 0;
}

void st7789_deinit(void)
{
    spi_hal_deinit();
}

void st7789_display_on(int on)
{
    cmd1(on ? ST7789_DISPON : ST7789_DISPOFF);
}

void st7789_invert(int on)
{
    cmd1(on ? ST7789_INVON : ST7789_INVOFF);
}

void st7789_clear(void)
{
    memset(g_fb, 0, sizeof(g_fb));
}

void st7789_fill(uint16_t color)
{
    for (int y = 0; y < TFT_HEIGHT; y++) {
        for (int x = 0; x < TFT_WIDTH; x++) {
            fb_set(x, y, color);
        }
    }
    st7789_flush();
}

void st7789_draw_pixel(int x, int y, uint16_t color)
{
    fb_set(x, y, color);
}

void st7789_draw_hline(int x, int y, int len, uint16_t color)
{
    for (int i = 0; i < len; i++) {
        fb_set(x + i, y, color);
    }
}

void st7789_draw_vline(int x, int y, int len, uint16_t color)
{
    for (int i = 0; i < len; i++) {
        fb_set(x, y + i, color);
    }
}

void st7789_draw_rect(int x, int y, int w, int h, uint16_t color)
{
    if (w <= 0 || h <= 0) {
        return;
    }
    st7789_draw_hline(x, y, w, color);
    st7789_draw_hline(x, y + h - 1, w, color);
    st7789_draw_vline(x, y, h, color);
    st7789_draw_vline(x + w - 1, y, h, color);
}

void st7789_fill_rect(int x, int y, int w, int h, uint16_t color)
{
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            fb_set(x + i, y + j, color);
        }
    }
}

int st7789_draw_char(int x, int y, char ch, uint16_t fg, uint16_t bg)
{
    unsigned int idx = (unsigned char)ch;
    const uint8_t *glyph;
    uint8_t line;

    if (idx < FONT8X16_FIRST || idx >= FONT8X16_FIRST + FONT8X16_COUNT) {
        ch = ' ';
        idx = FONT8X16_FIRST;
    }
    glyph = font8x16[idx - FONT8X16_FIRST];

    for (int row = 0; row < FONT8X16_H_; row++) {
        line = glyph[row];
        for (int col = 0; col < FONT8X16_W; col++) {
            fb_set(x + col, y + row, (line & (uint8_t)(0x80u >> col)) ? fg : bg);
        }
    }
    return FONT8X16_W;
}

void st7789_draw_string(int x, int y, const char *s, uint16_t fg, uint16_t bg)
{
    int cx = x;
    int cy = y;

    for (; s != NULL && *s != '\0'; s++) {
        if (*s == '\n') {
            cx = x;
            cy += FONT8X16_H_;
            continue;
        }
        if (cx + FONT8X16_W > TFT_WIDTH) {
            cx = x;
            cy += FONT8X16_H_;
        }
        st7789_draw_char(cx, cy, *s, fg, bg);
        cx += FONT8X16_W;
    }
}

void st7789_flush(void)
{
    int total = (int)sizeof(g_fb);
    int off = 0;
    int id = ++s_flush_cnt;

    printf("[tft] flush #%d start\n", id);
    set_window(0, 0, TFT_WIDTH - 1, TFT_HEIGHT - 1);
    spi_hal_write_cmd(ST7789_RAMWR);   /* 之后连续写像素，CS 可在分片间翻转 */
    while (off < total) {
        int n = total - off;
        if (n > ST7789_TX_CHUNK) {
            n = ST7789_TX_CHUNK;
        }
        spi_hal_write_data(g_fb + off, n);   /* 仍报错会在 spi_hal 打 EIO */
        off += n;
        usleep(100);   /* 片间 100us：给 lotus PL022 控制器喘息，防连续刷 wedge */
    }
    printf("[tft] flush #%d done\n", id);
}
