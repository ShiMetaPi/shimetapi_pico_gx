/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef _SPACC_BODY_H_
#define _SPACC_BODY_H_

#define SHA256_RESULT_SIZE              32
#define SHA256_RESULT_IN_WORD           8
#define SHA256_BLOCK_SIZE               64
#define SHA256_PADDING_MAX_SIZE         128

#define SPACC_CHN_MASK                  0xFE
#define SPACC_CHN_AES                   0x1
#define SPACC_CHN_SHA256                0x1

#define SPACC_MAX_DEPTH                 2

#define get_ulong_low(dw)               (unsigned int)(dw)
#define get_ulong_high(dw)              0
#define make_ulong(low, high)           (low)
#define make_size(low)                  (((unsigned long)(low##High) << 32) | (low))

#define SPACC_TIME_OUT                  300000
#define RSA_TIMEOUT_CNT                 300000

typedef enum {
    DIGEST_MODE_HASH,
    DIGEST_MODE_HMAC,
    DIGEST_MODE_COUNT,
} digest_mode_en;

typedef enum {
    DIGEST_ALG_SHA1,
    DIGEST_ALG_SHA224,
    DIGEST_ALG_SHA256,
    DIGEST_ALG_SHA384,
    DIGEST_ALG_SHA512,
    DIGEST_ALG_SM3,
    DIGEST_ALG_COUNT,
} digest_alg_en;

typedef enum {
    SYMC_ALG_DES = 0,
    SYMC_ALG_3DES,
    SYMC_ALG_AES,
    SYMC_ALG_SM4,
    SYMC_ALG_SM1,
    SYMC_ALG_NULL_CIPHER,
    SYMC_ALG_COUNT,
} symc_alg_en;

typedef enum {
    SYMC_DAT_WIDTH_128 = 0,
    SYMC_DAT_WIDTH_64 = 0,
    SYMC_DAT_WIDTH_8,
    SYMC_DAT_WIDTH_1,
    SYMC_DAT_WIDTH_COUNT,
} symc_dat_width_en;

typedef enum {
    SYMC_MODE_ECB = 0,
    SYMC_MODE_CBC,
    SYMC_MODE_CFB,
    SYMC_MODE_OFB,
    SYMC_MODE_CTR,
    SYMC_MODE_CCM,
    SYMC_MODE_GCM,
    SYMC_MODE_COUNT,
} symc_mode_en;

typedef enum {
    SPACC_BUF_TYPE_SYMC_IN,
    SPACC_BUF_TYPE_SYMC_OUT,
    SPACC_BUF_TYPE_DIGEST_IN,
    SPACC_BUF_TYPE_COUNT,
} spacc_buf_type_en;

typedef enum {
    SPACC_CTRL_NONE             = 0x00,
    SPACC_CTRL_SYMC_IN_GCM_A    = 0x00,
    SPACC_CTRL_SYMC_IN_GCM_P    = 0x08,
    SPACC_CTRL_SYMC_IN_GCM_LEN  = 0x10,
    SPACC_CTRL_SYMC_IN_CCM_N    = 0x00,
    SPACC_CTRL_SYMC_IN_CCM_A    = 0x08,
    SPACC_CTRL_SYMC_IN_CCM_P    = 0x10,
    SPACC_CTRL_SYMC_IN_CBC_OUTPUT_DISABLE = 0x04,
    SPACC_CTRL_SYMC_IN_FIRST    = 0x01,
    SPACC_CTRL_SYMC_IN_LAST     = 0x02,
    SPACC_CTRL_HASH_IN_PAD      = 0x04,
    SPACC_CTRL_HASH_IN_FIRST    = 0x01,
    SPACC_CTRL_HASH_IN_LAST     = 0x02,
    SPACC_CTRL_HASH_IN_AUTO_PADDING = 0x04,
    SPACC_CTRL_HASH_IN_HMAC_END = 0x08,
    SPACC_CTRL_SYMC_OUT_LAST    = 0x02,
    SPACC_CTRL_SYMC_CCM_LAST    = 0x20,
    SPACC_CTRL_SYMC_ODD_KEY     = 0x40,
    SPACC_CTRL_SYMC_EVEN_KEY    = 0x00,
    SPACC_CTRL_COUNT,
} spacc_ctrl_en;

typedef struct {
    xmedia_u32 hard_chn;
    xmedia_u32 sha_val[SHA256_RESULT_IN_WORD]; /* 16 size */
    xmedia_u32 data_phy;
    xmedia_u32 data_phy_high;
    xmedia_u32 data_len;
} cipher_hash_data_s;

typedef struct {
    xmedia_u32 src_phy_addr;
    xmedia_u32 src_phy_addr_high;
    xmedia_u32 dest_phy_addr;
    xmedia_u32 dest_phy_addr_high;
    xmedia_u32 data_length;
} cipher_data_s;

typedef struct {
    symc_alg_en symc_alg;
    symc_mode_en symc_mode;
    symc_dat_width_en symc_width;
    xmedia_u32 key_len;
} spacc_symc_config_s;

xmedia_s32 drv_cipher_sha256_update(cipher_hash_data_s *cipher_hash_data);
xmedia_s32 drv_cipher_sha256_final(cipher_hash_data_s *cipher_hash_data);
xmedia_s32 drv_cipher_init(xmedia_void);
xmedia_s32 drv_cipher_deinit(xmedia_void);
xmedia_s32 drv_cipher_config_aes_chn(cipher_ctrl *config);
xmedia_s32 drv_cipher_aes_decrypt(cipher_data_s *ci_data);
xmedia_s32 drv_cipher_klad_load_key(xmedia_u32 chn_id, xmedia_u32* data_in, xmedia_u32 key_len);
void cipher_debug_log(unsigned short errflag);
#endif
