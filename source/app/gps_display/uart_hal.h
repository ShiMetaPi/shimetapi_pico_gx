/*
 * uart_hal.h - 用户态 UART 读写封装（termios raw，按行读 NMEA）。
 *
 * 平台：GK7602V11A，GPS 接 UART1（GPIO3_5=TX, GPIO3_6=RX -> /dev/ttyAMA1），
 *       PL011 控制器，9600 8N1（GPS 模块默认）。
 */
#ifndef UART_HAL_H
#define UART_HAL_H

#define GPS_UART_DEV  "/dev/ttyAMA1"

/* 打开并配置 UART（raw 8N1，VMIN=0 VTIME=2 即 200ms 超时读）。成功返回 fd，失败 -1。 */
int  uart_open(const char *path);

/*
 * 读一行（到 '\n'）。返回读到的字节数；
 *   返回 0           = 超时无数据；
 *   返回 >0 且以 \n 结尾 = 完整一行（可解析）；
 *   返回 >0 不以 \n 结尾 = 半行（调用方按需丢弃）。
 */
int  uart_read_line(int fd, char *buf, int maxlen);

void uart_close(int fd);

#endif /* UART_HAL_H */
