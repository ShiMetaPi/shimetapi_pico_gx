/*
 * uart_hal.c - 用户态 UART 读写实现（基于 termios）。
 *
 * cfmakeraw 关闭所有 line discipline（裸字节流），再设 8N1 + 9600。
 * VMIN=0 VTIME=2：read 最多等 200ms 的下一个字符，有字符立即返回；GPS 持续 1Hz 发，
 * 主循环靠这个超时节流，无数据时不忙等。
 */
#include "uart_hal.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

int uart_open(const char *path)
{
    int fd;
    struct termios tio;

    fd = open(path, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        fprintf(stderr, "[uart] open %s 失败: %s\n", path, strerror(errno));
        return -1;
    }
    if (tcgetattr(fd, &tio) < 0) {
        fprintf(stderr, "[uart] tcgetattr 失败: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    cfmakeraw(&tio);
    cfsetispeed(&tio, B9600);
    cfsetospeed(&tio, B9600);
    tio.c_cflag &= ~(PARENB | CSTOPB | CSIZE);
    tio.c_cflag |= (CS8 | CLOCAL | CREAD);    /* 8 数据位、无校验、1 停止位、使能收发 */
    tio.c_iflag &= ~(IXON | IXOFF | IXANY);   /* 无软件流控 */
    tio.c_cc[VMIN]  = 0;
    tio.c_cc[VTIME] = 2;                       /* 200ms 字符间超时 */

    tcsetattr(fd, TCSANOW, &tio);
    tcflush(fd, TCIOFLUSH);

    printf("[uart] %s open, 9600 8N1 raw (VMIN=0 VTIME=2)\n", path);
    return fd;
}

int uart_read_line(int fd, char *buf, int maxlen)
{
    int n = 0;

    if (fd < 0 || buf == NULL || maxlen <= 1) {
        return 0;
    }
    while (n < maxlen - 1) {
        char c;
        ssize_t r = read(fd, &c, 1);
        if (r == 1) {
            buf[n++] = c;
            if (c == '\n') {
                break;            /* 一行结束 */
            }
        } else {
            break;                /* 超时(0) 或出错：结束本次读取 */
        }
    }
    buf[n] = '\0';
    return n;
}

void uart_close(int fd)
{
    if (fd >= 0) {
        close(fd);
    }
}
