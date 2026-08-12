/*
 * ssd1306.c - SSD1306 OLED 显示驱动实现（128x64, I2C）。
 *
 * 显存布局（SSD1306 页寻址）：s_gram[page][col]，每字节对应某列连续 8 行，
 * bit0 = 最上一行。绘制时 (x, y) -> s_gram[y/8][x] 的 bit (y&7)。
 * 刷新时切到水平寻址，一次写满 8 页 x 128 列共 1024 字节。
 */
#include "ssd1306.h"
#include "i2c_hal.h"
#include "font8x16.h"

#include <stdio.h>
#include <string.h>

/* SSD1306 I2C 控制字节：bit6=Co(后续控制字节)，bit0=D/C(0=命令,1=数据) */
#define SSD1306_CTRL_CMD      0x00    /* Co=0, D/C=0：本字节后全是命令 */
#define SSD1306_CTRL_DATA     0x40    /* Co=0, D/C=1：本字节后全是数据 */

static int      g_fd = -1;
static uint8_t  g_gram[OLED_PAGES][OLED_WIDTH];

/* SSD1306 128x64 标准初始化序列（0.96" 模组通用）。 */
static const uint8_t g_init_cmds[] = {
    0xAE,           /* display off */
    0xD5, 0x80,     /* display clock divide ratio / oscillator */
    0xA8, 0x3F,     /* multiplex ratio = 64 (HEIGHT - 1) */
    0xD3, 0x00,     /* display offset = 0 */
    0x40,           /* set display start line = 0 */
    0x8D, 0x14,     /* enable charge pump（0.96" 必须，否则不亮）*/
    0x20, 0x02,     /* memory addressing mode = page（SH1106 逐页写）*/
    0xA1,           /* segment remap: col127 = SEG0 */
    0xC8,           /* COM output scan direction remapped */
    0xDA, 0x12,     /* COM pins hardware config（128x64 = 0x12）*/
    0x81, 0xCF,     /* contrast */
    0xD9, 0xF1,     /* pre-charge period */
    0xDB, 0x40,     /* VCOMH deselect level */
    0xA4,           /* display follows RAM content */
    0xA6,           /* normal display (not inverted) */
    0xAF,           /* display ON */
};

/* 写一条单字节命令。 */
static int write_cmd(uint8_t cmd)
{
    uint8_t buf[2] = { SSD1306_CTRL_CMD, cmd };
    return i2c_hal_write(g_fd, OLED_I2C_ADDR, buf, 2);
}

/* 写一组命令（一次 I2C 事务，Co=0 后整段为命令流，SSD1306 顺序解析）。 */
static int write_cmds(const uint8_t *cmds, int n)
{
    uint8_t buf[40];

    if (n + 1 > (int)sizeof(buf)) {
        return -1;
    }
    buf[0] = SSD1306_CTRL_CMD;
    if (n > 0) {
        memcpy(buf + 1, cmds, (size_t)n);
    }
    return i2c_hal_write(g_fd, OLED_I2C_ADDR, buf, n + 1);
}

/* 写一批显存数据。 */
static int write_data(const uint8_t *data, int n)
{
    static uint8_t buf[1 + OLED_PAGES * OLED_WIDTH]; /* 复用，避免每次占栈 */

    if (n + 1 > (int)sizeof(buf)) {
        return -1;
    }
    buf[0] = SSD1306_CTRL_DATA;
    memcpy(buf + 1, data, (size_t)n);
    return i2c_hal_write(g_fd, OLED_I2C_ADDR, buf, n + 1);
}

int ssd1306_init(void)
{
    g_fd = i2c_hal_open(OLED_I2C_BUS, OLED_I2C_ADDR);
    if (g_fd < 0) {
        return -1;
    }
    /* 写初始化序列；若 OLED 未应答，I2C_RDWR 返回 -ENXIO，这里据此报错。 */
    if (write_cmds(g_init_cmds, (int)sizeof(g_init_cmds)) < 0) {
        fprintf(stderr, "[oled] OLED(0x%02x) on /dev/i2c-%d 无应答："
                "检查接线 / 地址 / 该 pad 是否已切到 I2C 功能。\n",
                OLED_I2C_ADDR, OLED_I2C_BUS);
        i2c_hal_close(g_fd);
        g_fd = -1;
        return -1;
    }
    ssd1306_clear();
    ssd1306_flush();
    return 0;
}

void ssd1306_deinit(void)
{
    if (g_fd >= 0) {
        i2c_hal_close(g_fd);
        g_fd = -1;
    }
}

void ssd1306_display_on(int on)
{
    if (g_fd >= 0) {
        write_cmd(on ? 0xAF : 0xAE);
    }
}

void ssd1306_clear(void)
{
    memset(g_gram, 0, sizeof(g_gram));
}

void ssd1306_fill(uint8_t color)
{
    memset(g_gram, (color == OLED_COLOR_WHITE) ? 0xFF : 0x00, sizeof(g_gram));
    ssd1306_flush();
}

void ssd1306_draw_pixel(int x, int y, uint8_t color)
{
    uint8_t *byte;
    uint8_t  mask;

    if (x < 0 || x >= OLED_WIDTH || y < 0 || y >= OLED_HEIGHT) {
        return;
    }
    mask = (uint8_t)(1u << (y & 7));
    byte = &g_gram[y >> 3][x];
    switch (color) {
    case OLED_COLOR_WHITE:
        *byte |= mask;
        break;
    case OLED_COLOR_BLACK:
        *byte &= (uint8_t)~mask;
        break;
    default: /* OLED_COLOR_INVERT */
        *byte ^= mask;
        break;
    }
}

void ssd1306_draw_hline(int x, int y, int len, uint8_t color)
{
    for (int i = 0; i < len; i++) {
        ssd1306_draw_pixel(x + i, y, color);
    }
}

void ssd1306_draw_vline(int x, int y, int len, uint8_t color)
{
    for (int i = 0; i < len; i++) {
        ssd1306_draw_pixel(x, y + i, color);
    }
}

void ssd1306_draw_rect(int x, int y, int w, int h, uint8_t color)
{
    if (w <= 0 || h <= 0) {
        return;
    }
    ssd1306_draw_hline(x, y, w, color);
    ssd1306_draw_hline(x, y + h - 1, w, color);
    ssd1306_draw_vline(x, y, h, color);
    ssd1306_draw_vline(x + w - 1, y, h, color);
}

void ssd1306_fill_rect(int x, int y, int w, int h, uint8_t color)
{
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            ssd1306_draw_pixel(x + i, y + j, color);
        }
    }
}

int ssd1306_draw_char(int x, int y, char ch, uint8_t color)
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
            /* MSB = 最左像素；字符背景恒为黑，前景取 color。 */
            if (line & (uint8_t)(0x80u >> col)) {
                ssd1306_draw_pixel(x + col, y + row, color);
            } else {
                ssd1306_draw_pixel(x + col, y + row, OLED_COLOR_BLACK);
            }
        }
    }
    return FONT8X16_W;
}

void ssd1306_draw_string(int x, int y, const char *s, uint8_t color)
{
    int cx = x;
    int cy = y;

    for (; s != NULL && *s != '\0'; s++) {
        if (*s == '\n') {
            cx = x;
            cy += FONT8X16_H_;
            continue;
        }
        if (cx + FONT8X16_W > OLED_WIDTH) {
            cx = x;
            cy += FONT8X16_H_;
        }
        ssd1306_draw_char(cx, cy, *s, color);
        cx += FONT8X16_W;
    }
}

void ssd1306_flush(void)
{
    if (g_fd < 0) {
        return;
    }
    /*
     * 1.3" OLED 实为 SH1106（非 SSD1306）：GDDRAM 132 列，可见区从列 2 开始；
     * 不用 0x21/0x22 范围命令（SH1106 不支持），改用页寻址传统命令：
     *   0xB0+页     设页地址
     *   0x10        列地址高位=0
     *   0x02        列地址低位=2（SH1106 左侧 2 像素偏移）
     * 页寻址模式下逐页设地址后写 128 字节，列从 2 自增到 129 = 物理 0..127。
     */
    for (int p = 0; p < OLED_PAGES; p++) {
        uint8_t setup[3] = {
            (uint8_t)(0xB0 + p),   /* 页地址 p */
            0x10,                   /* 列地址高位 = 0 */
            0x02,                   /* 列地址低位 = 2（SH1106 偏移）*/
        };
        if (write_cmds(setup, 3) < 0) {
            break;
        }
        if (write_data(&g_gram[p][0], OLED_WIDTH) < 0) {
            break;
        }
    }
}
