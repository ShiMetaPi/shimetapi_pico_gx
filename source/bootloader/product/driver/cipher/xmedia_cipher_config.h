/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __XMDEDIA_CIPHER_CONFIG_H_
#define __XMDEDIA_CIPHER_CONFIG_H_

#define CIPHER_CIPHER_REG_BASE_ADDR_PHY         (0x10050000)
#define CIPHER_RSA_REG_BASE_ADDR_PHY            (0x10070000)

#define CIPHER_RSA_CRG_ADDR_PHY                 (0x120101A0)
#define RSA_CRG_CLOCK_BIT                       (0x01 << 5)
#define RSA_CRG_RESET_BIT                       (0x01 << 4)
#define CIPHER_SPACC_CRG_ADDR_PHY               (0x120101A0)
#define SPACC_CRG_CLOCK_BIT                     (0x01 << 9)
#define SPACC_CRG_RESET_BIT                     (0x01 << 8)

#define CIPHER_KLAD_REG_BASE_ADDR_PHY           (0x10060000)
#define CIPHER_OTP_REG_BASE_ADDR_PHY            (0x100A0000)
#define CIPHER_KLAD_CRG_ADDR_PHY                (0x120101A0)

#define KLAD_CRG_CLOCK_BIT                      (0x01 << 1)
#define KLAD_CRG_RESET_BIT                      (0x01 << 0)


#define CIPHER_ERR_CIPHER_INIT                    0x1
#define CIPHER_ERR_CIPHER_CONFIG_CHAN             0x2
#define CIPHER_ERR_CIPHER_DECRYPT                 0x3
#define CIPHER_ERR_CIPHER_ERRCODE_1               0x4
#define CIPHER_ERR_CIPHER_ERRCODE_2               0x5
#define CIPHER_ERR_CIPHER_ERRCODE_4               0x6
#define CIPHER_ERR_CIPHER_CFG_OUTNODE             0x7
#define CIPHER_ERR_CIPHER_WAIT_TIMEOUT            0x8
#define CIPHER_ERR_CIPHER_MALLOC                  0x9
#define CIPHER_ERR_CIPHER_LOAD_KEY                0xA

#define CIPHER_ERR_SHA256_MALLOC                  0x10
#define CIPHER_ERR_SHA256_UPDATE                  0x11
#define CIPHER_ERR_SHA256_FINAL                   0x12
#define CIPHER_ERR_SHA256_START                   0x13
#define CIPHER_ERR_SHA256_WAIT_TIMEOUT            0x14

#define CIPHER_ERR_RSA_MALLOC                     0x20
#define CIPHER_ERR_RSA_KEY_LEN                    0x21
#define CIPHER_ERR_RSA_DEC                        0x22
#define CIPHER_ERR_RSA_CHECK_PADDING              0x23
#define CIPHER_ERR_RSA_BUSY_TIMEOUT               0x24
#define CIPHER_ERR_RSA_ERRORCODE                  0x25
#define CIPHER_ERR_RSA_M_ZERO                     0x26
#define CIPHER_ERR_RSA_M_LARGER_N                 0x27
#define CIPHER_ERR_RSA_E_EQUAL_1                  0x28
#define CIPHER_ERR_RSA_E_EQUAL_0                  0x29
#define CIPHER_ERR_RSA_SHA256                     0x2A
#define CIPHER_ERR_RSA_MGF1                       0x2B
#define CIPHER_ERR_RSA_VERIFY_HASH_CMP            0x2C
#define CIPHER_ERR_RSA_VERIFY_M_LEN               0x2D
#define CIPHER_ERR_RSA_VERIFY_EM0                 0x2E
#define CIPHER_ERR_RSA_VERIFY_BC                  0x2F
#define CIPHER_ERR_RSA_VERIFY_DB_1                0x30
#define CIPHER_ERR_RSA_VERIFY_MASKEDDB_LEN        0x31

#define CIPHER_ERR_OTP_WAIT_TIMEOUT               0x40
#define CIPHER_ERR_KLAD_WAIT_TIMEOUT              0x41

#endif
