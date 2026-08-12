/*
 * sht20.c - SHT20/SI7021/HTU21 温湿度驱动实现（I2C，no-hold-master）。
 *
 * 时序：发测量命令(0xF3/0xF5) -> 传感器 NACK 直到转换完成 -> 主控等够时间后直接读。
 *   本驱动用 i2c_hal_write 发命令 + usleep 等转换 + i2c_hal_read_noreg 读结果，
 *   不依赖 I2C clock-stretching（lotus i2c-lotus 是否支持 stretch 未知，no-hold 最稳）。
 * 读回 3 字节：MSB, LSB, CRC。低 2 位是状态位，& 0xFFFC 清掉再用公式换算。
 *
 * 初始化：效仿 oled_display，开 fd + I2C_SLAVE 绑地址 + 触发一次温度测量 + 读 3B。
 *   任一步 ioctl 失败即视为设备不在（参考 ssd1306_init 的写法）。
 */
#include "sht20.h"
#include "i2c_hal.h"

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

static int g_fd = -1;

/* no-hold-master 测量一次：发命令 -> 等 delay_ms -> 读 3 字节。 */
static int measure(uint8_t cmd, int delay_ms, uint16_t *raw)
{
    uint8_t buf[3];

    if (i2c_hal_write(g_fd, SHT20_ADDR, &cmd, 1) < 0) {
        return -1;
    }
    usleep((useconds_t)delay_ms * 1000u);
    if (i2c_hal_read_noreg(g_fd, SHT20_ADDR, buf, 3) < 0) {
        return -1;
    }
    *raw = (uint16_t)((((uint16_t)buf[0] << 8) | buf[1]) & 0xFFFCu);
    return 0;
}

int sht20_init(void)
{
    g_fd = i2c_hal_open(SHT20_BUS, SHT20_ADDR);   /* 同 oled：open + I2C_SLAVE 绑地址 */
    if (g_fd < 0) {
        return -1;
    }
    /* 触发一次温度测量 + 读 3B，能完成就算设备在。同 ssd1306_init 写 init 序列。 */
    uint16_t raw;
    if (measure(0xF3, 85, &raw) < 0) {
        fprintf(stderr,
                "[sht20] /dev/i2c-%d addr 0x%02x 无应答：检查接线/地址/pad 复用。\n",
                SHT20_BUS, SHT20_ADDR);
        i2c_hal_close(g_fd);
        g_fd = -1;
        return -1;
    }
    printf("[sht20] @ /dev/i2c-%d addr 0x%02x 就绪\n", SHT20_BUS, SHT20_ADDR);
    return 0;
}

int sht20_read(sht20_data_t *out)
{
    uint16_t traw, hraw;

    if (g_fd < 0 || out == NULL) {
        return -1;
    }
    if (measure(0xF3, 85, &traw) < 0) {     /* 温度 no-hold */
        fprintf(stderr, "[sht20] 读温度失败。\n");
        return -1;
    }
    if (measure(0xF5, 85, &hraw) < 0) {     /* 湿度 no-hold */
        fprintf(stderr, "[sht20] 读湿度失败。\n");
        return -1;
    }
    out->temp = -46.85f + 175.72f * (float)traw / 65536.0f;
    out->rh   = -6.0f   + 125.0f   * (float)hraw / 65536.0f;
    if (out->rh < 0.0f) {
        out->rh = 0.0f;
    }
    if (out->rh > 100.0f) {
        out->rh = 100.0f;
    }
    return 0;
}

void sht20_deinit(void)
{
    if (g_fd >= 0) {
        i2c_hal_close(g_fd);
        g_fd = -1;
    }
}