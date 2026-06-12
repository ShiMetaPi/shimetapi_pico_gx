/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include "xmedia_cipher_osal.h"

/*KLAD*/
#define KLAD_REG_BASE_ADDR                      g_klad_base
#define KLAD_REG_KLAD_CTRL                      (KLAD_REG_BASE_ADDR + 0x0420)
#define KLAD_REG_DAT_IN                         (KLAD_REG_BASE_ADDR + 0x0400)

#define KLAD_KEY_LEN                            4
#define CIPHER_WAIT_IDEL_TIMES                  300000

/*OTP USER*/
#define OTP_USER_IF_BASE                        g_efuse_otp_reg_base
#define OTP_USER_WORK_MODE                      (OTP_USER_IF_BASE + 0x0000)
#define OTP_USER_OP_START                       (OTP_USER_IF_BASE + 0x0004)
#define OTP_USER_KEY_INDEX                      (OTP_USER_IF_BASE + 0x0008)
#define OTP_USER_CTRL_STA                       (OTP_USER_IF_BASE + 0x003C)

#define REG_SYS_OTP_CLK_ADDR_PHY                0x120101BC
#define OTP_CRG_CLOCK_BIT                       (0x01 << 1)

typedef enum {
    OTP_USER_KEY0,
    OTP_USER_KEY1,
    OTP_USER_KEY2,
    OTP_USER_KEY3,
    OTP_USER_KEY_ROOTKEY,
    OTP_USER_KEY_HMACKEY,
    OTP_USER_KEY_UNKNOWN,
} otp_user_key_index_e;

typedef enum {
    OTP_IDEL_WORK_MODE,
    OTP_READ_LOCK_STA_MODE,
    OTP_LOAD_CIPHER_KEY_MODE,
    OTP_WRITE_KEY_ID_OR_PASSWD_MODE,
    OTP_KEY_ID_OR_PASSWD_CRC_MODE,
    OTP_SET_FLAG_ENABLE_MODE,
    OTP_WRITE_USER_ROOM_MODE,
    OTP_READ_USER_ROOM_MODE,
    OTP_UNKOOWN_MODE,
} otp_user_work_mode_e;
typedef enum {
    OTP_KEY_LENGTH_64BIT,
    OTP_KEY_LENGTH_128BIT,
    OTP_KEY_LENGTH_256BIT,
    OTP_KEY_LENGTH_UNSUPPORT,
} otp_user_key_length_e;

static xmedia_void *g_klad_base = XMEDIA_NULL;
static xmedia_void *g_efuse_otp_reg_base = XMEDIA_NULL;
static xmedia_u32 g_klad_init_flag = 0;
/* OTP init */
static xmedia_s32 hal_efuse_otp_init(xmedia_void)
{
    xmedia_u32 crg_value = 0;
    xmedia_u32 *sys_addr = XMEDIA_NULL;

    sys_addr = cipher_ioremap_nocache(REG_SYS_OTP_CLK_ADDR_PHY, 0x100);

    crg_value = hal_cipher_read_reg(sys_addr);
    crg_value |= OTP_CRG_CLOCK_BIT;   /* set the bit 0, clock opened */
    hal_cipher_write_reg(sys_addr, crg_value);

    cipher_iounmap(sys_addr);
    sys_addr = XMEDIA_NULL;

    g_efuse_otp_reg_base = cipher_ioremap_nocache(CIPHER_OTP_REG_BASE_ADDR_PHY, 0x100);

    return XMEDIA_SUCCESS;
}

static xmedia_void hal_klad_init(xmedia_void)
{
    xmedia_u32 crg_value;
    xmedia_u32 *sys_addr;

    sys_addr = cipher_ioremap_nocache(CIPHER_KLAD_CRG_ADDR_PHY, 0x100);

    crg_value = hal_cipher_read_reg(sys_addr);
    crg_value |= KLAD_CRG_RESET_BIT;   /* reset */
    crg_value |= KLAD_CRG_CLOCK_BIT;   /* set the bit 0, clock opened */
    hal_cipher_write_reg(sys_addr, crg_value);

    cipher_usleep(10);

    /* clock select and cancel reset 0x30100 */
    crg_value &= (~KLAD_CRG_RESET_BIT); /* cancel reset */
    crg_value |= KLAD_CRG_CLOCK_BIT;    /* set the bit 0, clock opened */
    hal_cipher_write_reg(sys_addr, crg_value);

    cipher_iounmap(sys_addr);
}

xmedia_void xmedia_cipher_klad_deinit(xmedia_void)
{
    if (g_klad_base != XMEDIA_NULL) {
        cipher_iounmap(g_klad_base);
        g_klad_base = XMEDIA_NULL;
    }

    if (g_efuse_otp_reg_base != XMEDIA_NULL) {
        cipher_iounmap(g_efuse_otp_reg_base);
        g_efuse_otp_reg_base = XMEDIA_NULL;
    }

    g_klad_init_flag = 0;

    return ;
}

static xmedia_s32 hal_otp_wait_free(xmedia_void)
{
    xmedia_u32 timeout_cnt = 0;
    xmedia_u32 reg_value = 0;

    while (1) {
        reg_value = hal_cipher_read_reg(OTP_USER_CTRL_STA);
        if ((reg_value & 0x1) == 0) /* bit0:otp_op_busy 0:idle, 1:busy */
            return XMEDIA_SUCCESS;

        timeout_cnt++;
        if (timeout_cnt >= CIPHER_WAIT_IDEL_TIMES) {
            cipher_debug_log(CIPHER_ERR_OTP_WAIT_TIMEOUT);    /*OTP_WaitFree TimeOut!*/
            break;
        }

        cipher_usleep(10);
    }

    return XMEDIA_FAILURE;
}

static xmedia_s32 hal_otp_wait_op_done(xmedia_void)
{
    xmedia_u32 timeout_cnt = 0;
    xmedia_u32 reg_value = 0;

    while (1) {
        reg_value = hal_cipher_read_reg(OTP_USER_CTRL_STA);
        if (reg_value & 0x2) { /* bit[1] otp_user_cmd_finish */
            return XMEDIA_SUCCESS;
        }

        timeout_cnt++;
        if (timeout_cnt >= CIPHER_WAIT_IDEL_TIMES) {
            cipher_debug_log(CIPHER_ERR_OTP_WAIT_TIMEOUT);    /*OTP_Wait_OP_done TimeOut!*/
            break;
        }

        cipher_usleep(10);
    }

    return XMEDIA_FAILURE;
}

static xmedia_s32 hal_cipher_wait_klad_done(void)
{
    xmedia_u32 timeout_cnt = 0;
    xmedia_u32 reg_value;


    while (1) {
        reg_value = hal_cipher_read_reg(KLAD_REG_KLAD_CTRL);
        if ((reg_value & 0x2) == 0x00) /* bit[1] start: 1 busy; 0 free */
            return XMEDIA_SUCCESS;

        timeout_cnt++;
        if (timeout_cnt >= CIPHER_WAIT_IDEL_TIMES) {
            cipher_debug_log(CIPHER_ERR_KLAD_WAIT_TIMEOUT);    /*Klad time out!*/
            break;
        }

        cipher_usleep(10);
    }

    return XMEDIA_FAILURE;
}

xmedia_s32 drv_cipher_klad_load_key(xmedia_u32 chn_id, xmedia_u32* data_in, xmedia_u32 key_len)
{
    xmedia_u32 i = 0, j = 0, ctrl = 0, gh = 0;

    if (g_klad_init_flag == 0) {
        g_klad_base = cipher_ioremap_nocache(CIPHER_KLAD_REG_BASE_ADDR_PHY, 0x100);

        if (hal_efuse_otp_init() != XMEDIA_SUCCESS) {
            cipher_iounmap(g_klad_base);
            return XMEDIA_FAILURE;
        }

        hal_klad_init();
        g_klad_init_flag = 1;
    }

    /*1: set otp key to klad*/
    /*wait otp idle*/
    if (XMEDIA_SUCCESS != hal_otp_wait_free())
        return XMEDIA_FAILURE;

    /*select key 0*/
    hal_cipher_write_reg(OTP_USER_KEY_INDEX, (xmedia_u32)OTP_USER_KEY0);

    /*mode*/
    hal_cipher_write_reg(OTP_USER_WORK_MODE, (xmedia_u32)OTP_LOAD_CIPHER_KEY_MODE);

    /*op start*/
    hal_cipher_write_reg(OTP_USER_OP_START, (xmedia_u32)0x1acce551);

    if (XMEDIA_SUCCESS != hal_otp_wait_op_done())
        return XMEDIA_FAILURE;

    /*
    *klad config
    *bit[18:16]: Klad2ci addr
    *bit[4:3]: klad type, 00:cipher's klad, 01:rsa's klad
    *bit[2]: high/low 128bit flag. 0:low 128bit  1:high 128bit
    *bit[1]: 1:start
    *bit[0]: 1:decrypt
    */
    ctrl  = chn_id << 16;
    ctrl |= (xmedia_u32)XMEDIA_CIPHER_KLAD_TARGET_AES << 3;
    ctrl |= (xmedia_u32)1;
    ctrl |= 0x00;
    hal_cipher_write_reg(KLAD_REG_KLAD_CTRL, ctrl);

    for (i = 0; i < key_len / 16; i++) {
        gh = (i == 1 ? 1 : 0);

        for (j = 0; j < 4; j++) {
            hal_cipher_write_reg(KLAD_REG_DAT_IN + j * KLAD_KEY_LEN, data_in[i * 4 + j]);
        }

        /* start */
        ctrl = hal_cipher_read_reg(KLAD_REG_KLAD_CTRL);
        ctrl &= ~(0x01 << 2);
        ctrl |= gh << 2;
        ctrl |= 0x2;
        hal_cipher_write_reg(KLAD_REG_KLAD_CTRL, ctrl);

        //wait lock done
        if (XMEDIA_SUCCESS != hal_cipher_wait_klad_done())
            return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}
