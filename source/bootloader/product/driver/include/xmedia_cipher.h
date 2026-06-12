/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __XMEDIA_CIPHER_H__
#define __XMEDIA_CIPHER_H__

#include "xmedia_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* Klad target. */
typedef enum {
    XMEDIA_CIPHER_KLAD_TARGET_AES   = 0x0,  /* Klad for AES. */
    XMEDIA_CIPHER_KLAD_TARGET_RSA,          /* Klad for RSA. */
    XMEDIA_CIPHER_KLAD_TARGET_BUTT,
} cipher_klad_target;

/* Hash algrithm type. */
typedef enum {
    XMEDIA_CIPHER_HASH_TYPE_SHA1,
    XMEDIA_CIPHER_HASH_TYPE_SHA224,
    XMEDIA_CIPHER_HASH_TYPE_SHA256,
    XMEDIA_CIPHER_HASH_TYPE_SHA384,
    XMEDIA_CIPHER_HASH_TYPE_SHA512,
    XMEDIA_CIPHER_HASH_TYPE_HMAC_SHA1,
    XMEDIA_CIPHER_HASH_TYPE_HMAC_SHA224,
    XMEDIA_CIPHER_HASH_TYPE_HMAC_SHA256,
    XMEDIA_CIPHER_HASH_TYPE_HMAC_SHA384,
    XMEDIA_CIPHER_HASH_TYPE_HMAC_SHA512,
    XMEDIA_CIPHER_HASH_TYPE_SM3,
    XMEDIA_CIPHER_HASH_TYPE_BUTT,
    XMEDIA_CIPHER_HASH_TYPE_INVALID = 0xffffffff,
} cipher_hash_type;

/* Cipher algorithm. */
typedef enum {
    XMEDIA_CIPHER_ALG_AES           = 0x0,  /* Advanced encryption standard (AES) algorithm */
    XMEDIA_CIPHER_ALG_SM1           = 0x1,  /* SM1 algorithm. */
    XMEDIA_CIPHER_ALG_SM4           = 0x2,  /* SM4 algorithm. */
    XMEDIA_CIPHER_ALG_DMA           = 0x3,  /* DMA copy. */
    XMEDIA_CIPHER_ALG_BUTT          = 0x4,
    XMEDIA_CIPHER_ALG_INVALID       = 0xffffffff,
} cipher_alg;

/* Cipher bit width. */
typedef enum {
    XMEDIA_CIPHER_BIT_WIDTH_128BIT  = 0x0,  /* 128-bit width */
    XMEDIA_CIPHER_BIT_WIDTH_64BIT   = 0x1,  /* 64-bit width */
    XMEDIA_CIPHER_BIT_WIDTH_8BIT    = 0x2,  /* 8-bit width */
    XMEDIA_CIPHER_BIT_WIDTH_1BIT    = 0x3,  /* 1-bit width */
    XMEDIA_CIPHER_BIT_WIDTH_BUTT    = 0x4,
    XMEDIA_CIPHER_BIT_WIDTH_INVALID = 0xffffffff,
} cipher_bit_width;

/* enum typedef */
/* Cipher work mode. */
typedef enum {
    XMEDIA_CIPHER_WORK_MODE_ECB     = 0x0,  /* Electronic codebook (ECB) mode, ECB has been considered insecure and it is
                                           recommended not to use it. */
    XMEDIA_CIPHER_WORK_MODE_CBC,            /* Cipher block chaining (CBC) mode. */
    XMEDIA_CIPHER_WORK_MODE_CFB,            /* Cipher feedback (CFB) mode. */
    XMEDIA_CIPHER_WORK_MODE_OFB,            /* Output feedback (OFB) mode. */
    XMEDIA_CIPHER_WORK_MODE_CTR,            /* Counter (CTR) mode. */
    XMEDIA_CIPHER_WORK_MODE_CCM,            /* Counter (CCM) mode. */
    XMEDIA_CIPHER_WORK_MODE_GCM,            /* Counter (GCM) mode. */
    XMEDIA_CIPHER_WORK_MODE_CBC_CTS,        /* Cipher block chaining CipherStealing mode. */
    XMEDIA_CIPHER_WORK_MODE_BUTT,
    XMEDIA_CIPHER_WORK_MODE_INVALID = 0xffffffff,
} cipher_work_mode;

/* Key length. */
typedef enum {
    XMEDIA_CIPHER_KEY_DEFAULT       = 0x0,  /* Default key length, AES-16, SM1-48, SM4-16 */
    XMEDIA_CIPHER_KEY_AES_128BIT    = 0x0,  /* 128-bit key for the AES algorithm */
    XMEDIA_CIPHER_KEY_AES_192BIT    = 0x1,  /* 192-bit key for the AES algorithm */
    XMEDIA_CIPHER_KEY_AES_256BIT    = 0x2,  /* 256-bit key for the AES algorithm */
    XMEDIA_CIPHER_KEY_LEN_BUTT      = 0x3,
    XMEDIA_CIPHER_KEY_INVALID       = 0xffffffff,
} cipher_key_len;

/* Structure of the cipher control information */
typedef struct {
    xmedia_u32 key[8];                   /* Key input */
    xmedia_u32 iv[4];                    /* Initialization vector (IV) */
} cipher_ctrl;

/* RSA public key struct */
typedef struct {
    xmedia_u8 *n;         /* Point to public modulus N */
    xmedia_u8 *e;         /* Point to public exponent E */
    xmedia_u16 n_len;     /* Length of public modulus N, max value is 512Byte */
    xmedia_u16 e_len;     /* Length of public exponent E, max value is 512Byte */
} cipher_rsa_pub_key;

/* RSA signature verify struct input */
typedef struct {
    cipher_rsa_pub_key pub_key;          /* RSA public key struct */
} cipher_rsa_verify;

typedef struct {
    xmedia_u8 *hash_data;             /* Hash data. */
    xmedia_u32 hash_data_len;               /* Length of hash data. */
    xmedia_u8 *sign;                        /* Input sign data. */
    xmedia_u32 sign_len;                    /* Length of input sign data. */
} cipher_verify_data;

xmedia_s32 xmedia_cipher_deinit(xmedia_void);
xmedia_s32 xmedia_cipher_aes256_decrypt(cipher_ctrl *ctrl, xmedia_size_t src_phy_addr, xmedia_size_t dest_phy_addr, xmedia_u32 byte_len);
xmedia_s32 xmedia_cipher_sha256_init(xmedia_void);
xmedia_s32 xmedia_cipher_sha256_update(xmedia_u8 *input_data, xmedia_u32 input_data_len);
xmedia_s32 xmedia_cipher_sha256_final(xmedia_u8 *output_hash);
xmedia_s32 xmedia_cipher_rsa_deinit(xmedia_void);
xmedia_s32 xmedia_cipher_rsa4096_psssha256_verify(const cipher_rsa_verify *rsa_verify, cipher_verify_data *verify_data);
xmedia_void xmedia_cipher_klad_deinit(xmedia_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __XMEDIA_CIPHER_H__ */

