#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <linux/fb.h>
#include <sys/stat.h>

#include "osal.h"
#include "sample_comm_tp_i2c.h"
#include "sample_comm.h"
#include "xmedia_errcode.h"

#define TP9963_I2C_ADDR  0x88

#define TP9963_ADDR_BYTE 1
#define TP9963_DATA_BYTE 1

#define INVALID_FD       (-1)

static xmedia_s32 g_i2c_fd = INVALID_FD;

xmedia_s32 sample_comm_tp_i2c_init(xmedia_u32 dev)
{
    xmedia_s32 ret;
    xmedia_char i2c_dev_file[16];

    snprintf(i2c_dev_file, sizeof(i2c_dev_file), "/dev/i2c-%u", dev);
    g_i2c_fd = open(i2c_dev_file, O_RDWR, S_IRUSR | S_IWUSR);
    if (g_i2c_fd < 0) {
        SAMPLE_PRT("Open /dev/i2c-%u error!\n", dev);
        return XMEDIA_ERRCODE_OPEN_FAILED;
    }

    ret = ioctl(g_i2c_fd, I2C_SLAVE_FORCE, (TP9963_I2C_ADDR >> 1));
    if (ret < 0) {
        SAMPLE_PRT("I2C_SLAVE_FORCE error!\n");
        close(g_i2c_fd);
        g_i2c_fd = INVALID_FD;
        return ret;
    }

    SAMPLE_PRT("====== i2c[%d] init success!=======\n", dev);

    return XMEDIA_SUCCESS;
}

xmedia_void sample_comm_tp_i2c_exit(xmedia_void)
{
    if (g_i2c_fd >= 0) {
        close(g_i2c_fd);
        g_i2c_fd = INVALID_FD;
    }
}

xmedia_s32 sample_comm_tp_i2c_read(xmedia_u32 addr, xmedia_u32 *data)
{
    xmedia_s32 ret;
    xmedia_u8 recv_buf[4];
    struct i2c_rdwr_ioctl_data ioctl_data;
    struct i2c_msg msg[2];
    xmedia_u32 i2c_addr = (TP9963_I2C_ADDR >> 1);

    if (g_i2c_fd < 0) {
        SAMPLE_PRT("i2c is not init!\n");
        return XMEDIA_ERRCODE_NOT_INIT;
    }

    memset(recv_buf, 0x0, sizeof(recv_buf));
    memset(&ioctl_data, 0x0, sizeof(ioctl_data));
    memset(msg, 0x0, sizeof(msg[0]));

    msg[0].addr = i2c_addr;
    msg[0].flags = 0;
    msg[0].len = TP9963_ADDR_BYTE;
    msg[0].buf = recv_buf;

    msg[1].addr = i2c_addr;
    msg[1].flags = 0;
    msg[1].flags |= I2C_M_RD;
    msg[1].len = TP9963_DATA_BYTE;
    msg[1].buf = recv_buf;

    ioctl_data.msgs = &msg[0];
    ioctl_data.nmsgs = 2;

    if (TP9963_ADDR_BYTE == 2) {
        recv_buf[0] = (addr >> 8) & 0xff;
        recv_buf[1] = addr & 0xff;
    } else {
        recv_buf[0] = addr & 0xff;
    }
    ret = ioctl(g_i2c_fd, I2C_RDWR, &ioctl_data);
    if (ret < 0) {
        SAMPLE_PRT("tp9963 i2c read failed!\n");
        return XMEDIA_FAILURE;
    }

    if (TP9963_DATA_BYTE == 2) {
        *data = recv_buf[0] | (recv_buf[1] << 8);
    } else {
        *data = recv_buf[0];
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_tp_i2c_write(xmedia_u32 addr, xmedia_u32 data)
{
    xmedia_u32 idx = 0;
    xmedia_s32 ret;
    xmedia_u8 buf[8];

    if (g_i2c_fd < 0) {
        SAMPLE_PRT("i2c is not init!\n");
        return XMEDIA_ERRCODE_NOT_INIT;
    }

    if (TP9963_ADDR_BYTE == 2) {
        buf[idx] = (addr >> 8) & 0xff;
        idx++;
        buf[idx] = addr & 0xff;
        idx++;
    } else {
        buf[idx] = addr & 0xff;
        idx++;
    }

    if (TP9963_DATA_BYTE == 2) {
        buf[idx] = (data >> 8) & 0xff;
        idx++;
        buf[idx] = data & 0xff;
        idx++;
    } else {
        buf[idx] = data & 0xff;
        idx++;
    }

    ret = write(g_i2c_fd, buf, TP9963_ADDR_BYTE + TP9963_DATA_BYTE);
    if (ret < 0) {
        SAMPLE_PRT("tp9963 i2c write failed!\n");
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}