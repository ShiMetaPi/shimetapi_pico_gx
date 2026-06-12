#include"xmedia_type.h"
#include"xmedia_errcode.h"
#include"i2c.h"
#include "compile.h"

#define REG_ADDR_MAX_WIDTH 2//16BIT

STAGE1_FUNC xmedia_s32 i2c_read(xmedia_s32 fd, xmedia_u16 slave_addr, xmedia_u32 read_addr,
                        xmedia_u8 addr_width, xmedia_u8 *buff, xmedia_u32 length)
{
    struct i2c_client client;

    if(addr_width > REG_ADDR_MAX_WIDTH){
        return XMEDIA_ERRCODE_NOT_SUPPORT;
    }

    client.i2c_num   = fd;
    client.dev_addr  = slave_addr >> 1;
    client.reg_addr  = read_addr;
    client.reg_width = addr_width;

    return i2c_recv(&client, buff, length);
}

STAGE1_FUNC xmedia_s32 i2c_write(xmedia_s32 fd, xmedia_u16 slave_addr, xmedia_u8 *buff, xmedia_u32 length)
{
    return i2c_send(fd, slave_addr >> 1, (xmedia_void *)buff, length);
}


