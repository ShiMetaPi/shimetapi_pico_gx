/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include "xmedia_cipher_osal.h"
#define SHA256_H0               0x6a09e667
#define SHA256_H1               0xbb67ae85
#define SHA256_H2               0x3c6ef372
#define SHA256_H3               0xa54ff53a
#define SHA256_H4               0x510e527f
#define SHA256_H5               0x9b05688c
#define SHA256_H6               0x1f83d9ab
#define SHA256_H7               0x5be0cd19

typedef struct {
    xmedia_u8 block_size;
    xmedia_u8 sha_len;
    xmedia_u8 last_block_size;
    xmedia_u8 reserve;
    xmedia_u32 total_data_len;
    xmedia_u32 sha_val[SHA256_RESULT_IN_WORD];
    xmedia_u8 last_block[SHA256_PADDING_MAX_SIZE];
} hash_info_s;

static hash_info_s* g_cipher_hash_data = XMEDIA_NULL;
static xmedia_s32 g_cipher_init_flag = 0;

static xmedia_u32 hash_msg_padding(xmedia_u8 *msg, xmedia_u32 byte_len, xmedia_u32 total_len, xmedia_u32 block_size)
{
    xmedia_u32 tmp;
    xmedia_u32 padding_len;

    if (block_size == 0) {
        return XMEDIA_FAILURE;
    }

    tmp = total_len % block_size;

    padding_len = (tmp < 56) ? (56 - tmp) : (120 - tmp); /* 56, 120 */
    padding_len += 8; /* 8 padding len */

    /* Format(binary): {data|1000...00| fix_data_len(bits)} */
    msg[byte_len++] = 0x80;
    crypto_memset(&msg[byte_len], SHA256_PADDING_MAX_SIZE, 0, padding_len - 1 - 8); /* 2, 8 */
    byte_len += padding_len - 1 - 8; /* 8 */

    /* write 8 bytes fix data length */
    msg[byte_len++] = 0x00;
    msg[byte_len++] = 0x00;
    msg[byte_len++] = 0x00;
    msg[byte_len++] = (xmedia_u8)((total_len >> 29) & 0x07); /* 29 right shift */
    msg[byte_len++] = (xmedia_u8)((total_len >> 21) & 0xff); /* 21 right shift */
    msg[byte_len++] = (xmedia_u8)((total_len >> 13) & 0xff); /* 13 right shift */
    msg[byte_len++] = (xmedia_u8)((total_len >> 5) & 0xff);  /* 5  right shift */
    msg[byte_len++] = (xmedia_u8)((total_len << 3) & 0xff);  /* 3  right shift */

    return byte_len;
}

static xmedia_s32 cipher_hash_update_block(hash_info_s *hash_info, xmedia_u8 *input_data, xmedia_u32 input_data_len)
{
    xmedia_s32 ret;
    cipher_hash_data_s hash_data;

    crypto_memset(&hash_data, sizeof(cipher_hash_data_s), 0, sizeof(cipher_hash_data_s));
    hash_data.hard_chn = SPACC_CHN_SHA256;
    crypto_memcpy(hash_data.sha_val, sizeof(hash_data.sha_val), hash_info->sha_val, sizeof(hash_info->sha_val));

    hash_data.data_len = input_data_len;
    hash_data.data_phy = get_ulong_low(input_data);
    hash_data.data_phy_high = get_ulong_high(input_data);
    ret = drv_cipher_sha256_update(&hash_data);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    crypto_memcpy(hash_info->sha_val, sizeof(hash_info->sha_val), hash_data.sha_val, sizeof(hash_data.sha_val));

    return XMEDIA_SUCCESS;
}

static xmedia_s32 cipher_hash_updata_tail(hash_info_s *hash_info,
    xmedia_u8 *data_phy, xmedia_u8 *input_data, xmedia_u32 input_data_len)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;

    /* process the tail of last update */
    if (hash_info->last_block_size > 0) {
        crypto_memcpy(hash_info->last_block + hash_info->last_block_size,
                        SHA256_PADDING_MAX_SIZE - hash_info->last_block_size,
                        input_data,
                        hash_info->block_size - hash_info->last_block_size);
        crypto_memcpy(data_phy, SHA256_BLOCK_SIZE, hash_info->last_block, hash_info->block_size);
        ret = cipher_hash_update_block(hash_info, data_phy, hash_info->block_size);
        if (ret != XMEDIA_SUCCESS) {
            return ret;
        }
        input_data_len -= hash_info->block_size - hash_info->last_block_size;
        input_data += hash_info->block_size - hash_info->last_block_size;
    }

    if (input_data_len >= hash_info->block_size) {
        xmedia_u32 size;
        size = input_data_len - (input_data_len % hash_info->block_size);
        ret = cipher_hash_update_block(hash_info, input_data, size);
        if (ret != XMEDIA_SUCCESS) {
            return ret;
        }
        input_data_len -= size;
        input_data += size;
    }

    /* save tail data */
    crypto_memset(hash_info->last_block, SHA256_PADDING_MAX_SIZE, 0, SHA256_PADDING_MAX_SIZE);
    hash_info->last_block_size = input_data_len;
    crypto_memcpy(hash_info->last_block, SHA256_PADDING_MAX_SIZE, input_data, input_data_len);

    return ret;
}

xmedia_s32 xmedia_cipher_deinit()
{
    if (g_cipher_hash_data != XMEDIA_NULL) {
        cipher_free(g_cipher_hash_data);
        g_cipher_hash_data = XMEDIA_NULL;
    }

    if (g_cipher_init_flag == 0) {
        return XMEDIA_SUCCESS;
    }

    if (drv_cipher_deinit() != XMEDIA_SUCCESS) {
        return XMEDIA_FAILURE;
    } else {
        g_cipher_init_flag = 0;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 xmedia_cipher_aes256_decrypt(cipher_ctrl *ctrl, xmedia_size_t src_phy_addr, xmedia_size_t dest_phy_addr, xmedia_u32 byte_len)
{
    cipher_data_s ci_data;
    xmedia_s32 ret;

    if (g_cipher_init_flag == 0) {
        if (drv_cipher_init() != XMEDIA_SUCCESS) {
            cipher_debug_log(CIPHER_ERR_CIPHER_INIT);
            return XMEDIA_FAILURE;
        }

        g_cipher_init_flag = 1;
    }

    ret = drv_cipher_config_aes_chn(ctrl);
    if (ret != XMEDIA_SUCCESS) {
        cipher_debug_log(CIPHER_ERR_CIPHER_CONFIG_CHAN);
        return ret;
    }

    ci_data.src_phy_addr = get_ulong_low(src_phy_addr);
    ci_data.src_phy_addr_high = get_ulong_high(src_phy_addr);
    ci_data.dest_phy_addr = get_ulong_low(dest_phy_addr);
    ci_data.dest_phy_addr_high = get_ulong_high(dest_phy_addr);
    ci_data.data_length = byte_len;

    ret = drv_cipher_aes_decrypt(&ci_data);
    if (ret != XMEDIA_SUCCESS) {
        cipher_debug_log(CIPHER_ERR_CIPHER_DECRYPT);
        return ret;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 xmedia_cipher_sha256_init()
{
    if (g_cipher_init_flag == 0) {
        if (drv_cipher_init() != XMEDIA_SUCCESS) {
            cipher_debug_log(CIPHER_ERR_CIPHER_INIT);
            return XMEDIA_FAILURE;
        }
        g_cipher_init_flag = 1;
    }

    if (g_cipher_hash_data == XMEDIA_NULL) {
        /*sizeof(hash_info_s) = 0xA8*/
        g_cipher_hash_data = cipher_malloc(sizeof(hash_info_s));
        if (g_cipher_hash_data == XMEDIA_NULL) {
            cipher_debug_log(CIPHER_ERR_SHA256_MALLOC);
            return XMEDIA_FAILURE;
        }
    }

    crypto_memset(g_cipher_hash_data, sizeof(hash_info_s), 0, sizeof(hash_info_s));
    g_cipher_hash_data->block_size = SHA256_BLOCK_SIZE; /* 64 sha256 block size */
    g_cipher_hash_data->sha_len = SHA256_RESULT_SIZE;
    g_cipher_hash_data->sha_val[0] = cipher_cpu_to_be32(SHA256_H0); /* 0 sha256 index */
    g_cipher_hash_data->sha_val[1] = cipher_cpu_to_be32(SHA256_H1); /* 1 sha256 index */
    g_cipher_hash_data->sha_val[2] = cipher_cpu_to_be32(SHA256_H2); /* 2 sha256 index */
    g_cipher_hash_data->sha_val[3] = cipher_cpu_to_be32(SHA256_H3); /* 3 sha256 index */
    g_cipher_hash_data->sha_val[4] = cipher_cpu_to_be32(SHA256_H4); /* 4 sha256 index */
    g_cipher_hash_data->sha_val[5] = cipher_cpu_to_be32(SHA256_H5); /* 5 sha256 index */
    g_cipher_hash_data->sha_val[6] = cipher_cpu_to_be32(SHA256_H6); /* 6 sha256 index */
    g_cipher_hash_data->sha_val[7] = cipher_cpu_to_be32(SHA256_H7); /* 7 sha256 index */

    return XMEDIA_SUCCESS;
}

xmedia_s32 xmedia_cipher_sha256_update(xmedia_u8 *input_data, xmedia_u32 input_data_len)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_u8 *data_phy = XMEDIA_NULL;
    hash_info_s *hash_info = g_cipher_hash_data;
    xmedia_u32 size;

    if (input_data_len == 0)
        return XMEDIA_SUCCESS;

    if (hash_info == XMEDIA_NULL) {
        return XMEDIA_FAILURE;
    }

    data_phy = (xmedia_u8 *)cipher_malloc(SHA256_BLOCK_SIZE);
    if (data_phy == XMEDIA_NULL) {
        cipher_debug_log(CIPHER_ERR_SHA256_MALLOC);
        return XMEDIA_FAILURE;
    }
    crypto_memset(data_phy, SHA256_BLOCK_SIZE, 0, SHA256_BLOCK_SIZE);

    hash_info->total_data_len += input_data_len;
    size = hash_info->last_block_size + input_data_len;
    if (size < hash_info->block_size) {
        crypto_memcpy(hash_info->last_block + hash_info->last_block_size,
            (SHA256_PADDING_MAX_SIZE - hash_info->last_block_size), input_data, input_data_len);
        hash_info->last_block_size += input_data_len;
        goto free_data_phy;
    }

    ret = cipher_hash_updata_tail(hash_info, data_phy, input_data, input_data_len);
    if (ret != XMEDIA_SUCCESS) {
        cipher_debug_log(CIPHER_ERR_SHA256_UPDATE);
    }

free_data_phy:
    cipher_free(data_phy);
    data_phy = XMEDIA_NULL;
    return ret;
}

xmedia_s32 xmedia_cipher_sha256_final(xmedia_u8 *output_hash)
{
    xmedia_s32 ret;
    hash_info_s *hash_info = g_cipher_hash_data;
    cipher_hash_data_s hash_data;
    xmedia_u32 tmp;
    xmedia_u8 *data_phy = XMEDIA_NULL;

    if (hash_info == XMEDIA_NULL) {
        return XMEDIA_FAILURE;
    }

    data_phy = (xmedia_u8 *)cipher_malloc(SHA256_PADDING_MAX_SIZE);
    if (data_phy == XMEDIA_NULL) {
        cipher_debug_log(CIPHER_ERR_SHA256_MALLOC);
        return XMEDIA_FAILURE;
    }

    crypto_memset(data_phy, SHA256_PADDING_MAX_SIZE, 0, SHA256_PADDING_MAX_SIZE);
    crypto_memset(&hash_data, sizeof(cipher_hash_data_s), 0, sizeof(cipher_hash_data_s));

    tmp = hash_msg_padding(hash_info->last_block, hash_info->last_block_size, hash_info->total_data_len, hash_info->block_size);

    crypto_memcpy(data_phy, SHA256_PADDING_MAX_SIZE, hash_info->last_block, tmp);

    hash_data.data_len = tmp;
    hash_data.data_phy = get_ulong_low(data_phy);
    hash_data.data_phy_high = get_ulong_high(data_phy);
    crypto_memcpy(hash_data.sha_val, sizeof(hash_data.sha_val), hash_info->sha_val, sizeof(hash_info->sha_val));
    hash_data.hard_chn = SPACC_CHN_SHA256;

    ret = drv_cipher_sha256_final(&hash_data);
    if (ret != XMEDIA_SUCCESS) {
        cipher_debug_log(CIPHER_ERR_SHA256_FINAL);
    }
    crypto_memcpy(output_hash, SHA256_RESULT_SIZE, hash_data.sha_val, hash_info->sha_len);

    cipher_free(data_phy);
    data_phy = XMEDIA_NULL;

    return ret;
}
