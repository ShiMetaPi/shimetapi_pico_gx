/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "xmedia_type.h"
#include "xmedia_cipher.h"
#include "xmedia_memory.h"
#include "sample_common.h"

#define AES_KEY_MAX_LENGTH      32
#define AES128_KEY_LENGTH       16
#define AES_IV_LENGTH           16

static xmedia_s32 set_config_info(xmedia_handle handle, xmedia_bool key_by_ca, xmedia_cipher_alg alg, xmedia_cipher_work_mode mode,
                                        xmedia_cipher_key_length key_len, const xmedia_u8 *key, const xmedia_u8 iv[AES_IV_LENGTH])
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_cipher_ctrl ctrl;
    memset(&ctrl, 0, sizeof(xmedia_cipher_ctrl));

    ctrl.alg = alg;
    ctrl.mode = mode;
    ctrl.key_by_ca = key_by_ca;

    if (ctrl.alg == XMEDIA_CIPHER_ALG_AES) {
        /* only support ECB/CBC/CFB/OFB/CTR */
        if (ctrl.mode <= XMEDIA_CIPHER_WORK_MODE_CTR && ctrl.mode >= XMEDIA_CIPHER_WORK_MODE_ECB) {
            ctrl.aes_ctrl.bit_width = XMEDIA_CIPHER_BIT_WIDTH_128BIT;
            ctrl.aes_ctrl.key_len = key_len;
            ctrl.aes_ctrl.iv_usage = 1;
            if (ctrl.mode != XMEDIA_CIPHER_WORK_MODE_ECB) {
                memcpy(ctrl.aes_ctrl.iv, iv, AES_IV_LENGTH);
            }

            if (key_len == XMEDIA_CIPHER_KEY_AES_128BIT) {
                memcpy(ctrl.aes_ctrl.key, key, AES128_KEY_LENGTH);
            } else {
                memcpy(ctrl.aes_ctrl.key, key, AES_KEY_MAX_LENGTH);
            }
        }
    }

    ret = xmedia_cipher_config_handle(handle, &ctrl);
    if (XMEDIA_SUCCESS != ret) {
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 ecb_aes256(xmedia_void)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_u32 data_len = 32;
    xmedia_ulong input_addr_phy = 0;
    xmedia_ulong output_addr_phy = 0;
    xmedia_u32 cached = 0;
    xmedia_u8 *input_addr_vir = XMEDIA_NULL;
    xmedia_u8 *output_addr_vir = XMEDIA_NULL;
    xmedia_handle chnid = 0;
    xmedia_u8 aes_key[32] = {0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
                             0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11};
    xmedia_u8 aes_src[32] = {0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
                             0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11};
    xmedia_u8 aes_dst[32] = {0x86,0x3a,0xe9,0x1f,0xd4,0xcf,0x51,0x13,0x2d,0xc3,0xe4,0xed,0xf0,0x9d,0xbc,0x6e,
                             0x86,0x3a,0xe9,0x1f,0xd4,0xcf,0x51,0x13,0x2d,0xc3,0xe4,0xed,0xf0,0x9d,0xbc,0x6e};
    xmedia_u8 aes_iv[16] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};

    ret = xmedia_cipher_init();
    if (XMEDIA_SUCCESS != ret) {
        return ret;
    }

    ret = xmedia_cipher_create_handle(&chnid);
    if (XMEDIA_SUCCESS != ret) {
        xmedia_cipher_exit();
        return ret;
    }

    printf("chnid 0x%d\n", chnid);

    input_addr_phy = xmedia_mmz_alloc(XMEDIA_NULL, "CIPHER_BufIn", data_len);
    if (0 == input_addr_phy) {
        ERR_CODE_CIPHER("Error: Get phyaddr for input failed!\n");
        goto __CIPHER_EXIT__;
    }
    input_addr_vir = xmedia_mmz_map(input_addr_phy, data_len, cached);

    output_addr_phy = xmedia_mmz_alloc(XMEDIA_NULL, "CIPHER_BufOut", data_len);
    if (0 == output_addr_phy) {
        ERR_CODE_CIPHER("Error: Get phyaddr for output failed!\n");
        goto __CIPHER_EXIT__;
    }
    output_addr_vir = xmedia_mmz_map(output_addr_phy, data_len, cached);

    // For encrypt
    ret = set_config_info(chnid,
                          XMEDIA_FALSE,
                          XMEDIA_CIPHER_ALG_AES,
                          XMEDIA_CIPHER_WORK_MODE_ECB,
                          XMEDIA_CIPHER_KEY_AES_256BIT,
                          aes_key,
                          aes_iv);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Set config info failed.\n");
        goto __CIPHER_EXIT__;
    }

    memset(input_addr_vir, 0x0, data_len);
    memcpy(input_addr_vir, aes_src, data_len);
    print_buffer("ECB-AES-256-ORI:", aes_src, sizeof(aes_src));

    memset(output_addr_vir, 0x0, data_len);

    ret = xmedia_cipher_encrypt(chnid, input_addr_phy, output_addr_phy, data_len);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Cipher encrypt failed.\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

    print_buffer("ECB-AES-256-ENC:", output_addr_vir, data_len);

    // compare
    if (0 != memcmp(output_addr_vir, aes_dst, data_len)) {
        ERR_CODE_CIPHER("cipher encrypt, memcmp failed!\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

   // For decrypt
    memcpy(input_addr_vir, aes_dst, data_len);
    memset(output_addr_vir, 0x0, data_len);

    ret = set_config_info(chnid,
                          XMEDIA_FALSE,
                          XMEDIA_CIPHER_ALG_AES,
                          XMEDIA_CIPHER_WORK_MODE_ECB,
                          XMEDIA_CIPHER_KEY_AES_256BIT,
                          aes_key,
                          aes_iv);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Set config info failed.\n");
        goto __CIPHER_EXIT__;
    }

    ret = xmedia_cipher_decrypt(chnid, input_addr_phy, output_addr_phy, data_len);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Cipher decrypt failed.\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

    print_buffer("ECB-AES-256-DEC:", output_addr_vir, data_len);
    // compare
    if (0 != memcmp(output_addr_vir, aes_src, data_len)) {
        ERR_CODE_CIPHER("cipher decrypt, memcmp failed!\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

    TEST_END_PASS();

__CIPHER_EXIT__:

    if (input_addr_phy > 0) {
        xmedia_mmz_unmap(input_addr_vir);
        xmedia_mmz_free(input_addr_phy);
    }

    if (output_addr_phy > 0) {
        xmedia_mmz_unmap(output_addr_vir);
        xmedia_mmz_free(output_addr_phy);
    }

    xmedia_cipher_destroy_handle(chnid);
    xmedia_cipher_exit();

    return ret;
}

static xmedia_s32 cbc_aes256(xmedia_void)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_u32 data_len = 16;
    xmedia_ulong input_addr_phy = 0;
    xmedia_ulong output_addr_phy = 0;
    xmedia_u32 cached = 0;
    xmedia_u8 *input_addr_vir = XMEDIA_NULL;
    xmedia_u8 *output_addr_vir = XMEDIA_NULL;
    xmedia_handle chnid = 0;
    xmedia_u8 aes_key[32] = {0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
                             0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11};
    xmedia_u8 aes_iv[16]  = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};
    xmedia_u8 aes_src[16] = {0x6B,0xC1,0xBE,0xE2,0x2E,0x40,0x9F,0x96,0xE9,0x3D,0x7E,0x11,0x73,0x93,0x17,0x2A};
    xmedia_u8 aes_dst[16] = {0xe0,0xfa,0xab,0x42,0xd5,0xa5,0x5c,0x18,0xb0,0x13,0x1f,0x47,0x6c,0xa5,0xd0,0x0a};

    ret = xmedia_cipher_init();
    if (XMEDIA_SUCCESS != ret) {
        return ret;
    }

    ret = xmedia_cipher_create_handle(&chnid);
    if (XMEDIA_SUCCESS != ret) {
        xmedia_cipher_exit();
        return ret;
    }

    printf("chnid 0x%d\n", chnid);

    input_addr_phy = xmedia_mmz_alloc(XMEDIA_NULL, "CIPHER_BufIn", data_len);
    if (0 == input_addr_phy) {
        ERR_CODE_CIPHER("Error: Get phyaddr for input failed!\n");
        goto __CIPHER_EXIT__;
    }
    input_addr_vir = xmedia_mmz_map(input_addr_phy, data_len, cached);

    output_addr_phy = xmedia_mmz_alloc(XMEDIA_NULL, "CIPHER_BufOut", data_len);
    if (0 == output_addr_phy) {
        ERR_CODE_CIPHER("Error: Get phyaddr for output failed!\n");
        goto __CIPHER_EXIT__;
    }
    output_addr_vir = xmedia_mmz_map(output_addr_phy, data_len, cached);

    // For encrypt
    ret = set_config_info(chnid,
                          XMEDIA_FALSE,
                          XMEDIA_CIPHER_ALG_AES,
                          XMEDIA_CIPHER_WORK_MODE_CBC,
                          XMEDIA_CIPHER_KEY_AES_256BIT,
                          aes_key,
                          aes_iv);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Set config info failed.\n");
        goto __CIPHER_EXIT__;
    }

    memset(input_addr_vir, 0x0, data_len);
    memcpy(input_addr_vir, aes_src, data_len);
    print_buffer("CBC-AES-256-ORI:", aes_src, sizeof(aes_src));

    memset(output_addr_vir, 0x0, data_len);

    ret = xmedia_cipher_encrypt(chnid, input_addr_phy, output_addr_phy, data_len);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Cipher encrypt failed.\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

    print_buffer("CBC-AES-256-ENC:", output_addr_vir, sizeof(aes_dst));

    if (0 != memcmp(output_addr_vir, aes_dst, data_len)) {
        ERR_CODE_CIPHER("cipher encrypt, memcmp failed!\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

   // For decrypt
    memcpy(input_addr_vir, aes_dst, data_len);
    memset(output_addr_vir, 0x0, data_len);

    ret = set_config_info(chnid,
                          XMEDIA_FALSE,
                          XMEDIA_CIPHER_ALG_AES,
                          XMEDIA_CIPHER_WORK_MODE_CBC,
                          XMEDIA_CIPHER_KEY_AES_256BIT,
                          aes_key,
                          aes_iv);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Set config info failed.\n");
        goto __CIPHER_EXIT__;
    }

    ret = xmedia_cipher_decrypt(chnid, input_addr_phy, output_addr_phy, data_len);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Cipher decrypt failed.\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

    print_buffer("CBC-AES-256-DEC:", output_addr_vir, data_len);
    // compare
    if (0 != memcmp(output_addr_vir, aes_src, data_len)) {
        ERR_CODE_CIPHER("cipher decrypt, memcmp failed!\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

    TEST_END_PASS();

__CIPHER_EXIT__:

    if (input_addr_phy > 0) {
        xmedia_mmz_unmap(input_addr_vir);
        xmedia_mmz_free(input_addr_phy);
    }

    if (output_addr_phy > 0) {
        xmedia_mmz_unmap(output_addr_vir);
        xmedia_mmz_free(output_addr_phy);
    }

    xmedia_cipher_destroy_handle(chnid);
    xmedia_cipher_exit();

    return ret;
}

static xmedia_s32 cbc_aes192(xmedia_void)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_u32 data_len = 16;
    xmedia_ulong input_addr_phy = 0;
    xmedia_ulong output_addr_phy = 0;
    xmedia_u32 cached = 0;
    xmedia_u8 *input_addr_vir = XMEDIA_NULL;
    xmedia_u8 *output_addr_vir = XMEDIA_NULL;
    xmedia_handle chnid = 0;
    xmedia_u8 aes_key[32] = {0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
                             0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11};
    xmedia_u8 aes_iv[16]  = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};
    xmedia_u8 aes_src[16] = {0x6B,0xC1,0xBE,0xE2,0x2E,0x40,0x9F,0x96,0xE9,0x3D,0x7E,0x11,0x73,0x93,0x17,0x2A};
    xmedia_u8 aes_dst[16] = {0x69,0x25,0x8f,0xc2,0x8d,0x58,0x8b,0x84,0x67,0x87,0x76,0x02,0x2f,0x9c,0x1c,0x91};

    ret = xmedia_cipher_init();
    if (XMEDIA_SUCCESS != ret) {
        return ret;
    }

    ret = xmedia_cipher_create_handle(&chnid);
    if (XMEDIA_SUCCESS != ret) {
        xmedia_cipher_exit();
        return ret;
    }

    printf("chnid 0x%d\n", chnid);

    input_addr_phy = xmedia_mmz_alloc(XMEDIA_NULL, "CIPHER_BufIn", data_len);
    if (0 == input_addr_phy) {
        ERR_CODE_CIPHER("Error: Get phyaddr for input failed!\n");
        goto __CIPHER_EXIT__;
    }
    input_addr_vir = xmedia_mmz_map(input_addr_phy, data_len, cached);

    output_addr_phy = xmedia_mmz_alloc(XMEDIA_NULL, "CIPHER_BufOut", data_len);
    if (0 == output_addr_phy) {
        ERR_CODE_CIPHER("Error: Get phyaddr for output failed!\n");
        goto __CIPHER_EXIT__;
    }
    output_addr_vir = xmedia_mmz_map(output_addr_phy, data_len, cached);

    // For encrypt
    ret = set_config_info(chnid,
                          XMEDIA_FALSE,
                          XMEDIA_CIPHER_ALG_AES,
                          XMEDIA_CIPHER_WORK_MODE_CBC,
                          XMEDIA_CIPHER_KEY_AES_192BIT,
                          aes_key,
                          aes_iv);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Set config info failed.\n");
        goto __CIPHER_EXIT__;
    }

    memset(input_addr_vir, 0x0, data_len);
    memcpy(input_addr_vir, aes_src, data_len);
    print_buffer("CBC-AES-192-ORI:", aes_src, sizeof(aes_src));

    memset(output_addr_vir, 0x0, data_len);

    ret = xmedia_cipher_encrypt(chnid, input_addr_phy, output_addr_phy, data_len);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Cipher encrypt failed.\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

    print_buffer("CBC-AES-192-ENC:", output_addr_vir, data_len);

    // compare
    if (0 != memcmp(output_addr_vir, aes_dst, data_len)) {
        ERR_CODE_CIPHER("cipher encrypt, memcmp failed!\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

    // For decrypt
    memcpy(input_addr_vir, aes_dst, data_len);
    memset(output_addr_vir, 0x0, data_len);

    ret = set_config_info(chnid,
                          XMEDIA_FALSE,
                          XMEDIA_CIPHER_ALG_AES,
                          XMEDIA_CIPHER_WORK_MODE_CBC,
                          XMEDIA_CIPHER_KEY_AES_192BIT,
                          aes_key,
                          aes_iv);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Set config info failed.\n");
        goto __CIPHER_EXIT__;
    }

    ret = xmedia_cipher_decrypt(chnid, input_addr_phy, output_addr_phy, data_len);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Cipher decrypt failed.\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

    print_buffer("CBC-AES-192-DEC:", output_addr_vir, data_len);
    // compare
    if (0 != memcmp(output_addr_vir, aes_src, data_len)) {
        ERR_CODE_CIPHER("cipher decrypt, memcmp failed!\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

    TEST_END_PASS();

__CIPHER_EXIT__:

    if (input_addr_phy > 0) {
        xmedia_mmz_unmap(input_addr_vir);
        xmedia_mmz_free(input_addr_phy);
    }

    if (output_addr_phy > 0) {
        xmedia_mmz_unmap(output_addr_vir);
        xmedia_mmz_free(output_addr_phy);
    }

    xmedia_cipher_destroy_handle(chnid);
    xmedia_cipher_exit();

    return ret;
}

static xmedia_s32 cbc_aes128(xmedia_void)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_u32 data_len = 16;
    xmedia_ulong input_addr_phy = 0;
    xmedia_ulong output_addr_phy = 0;
    xmedia_u32 cached = 0;
    xmedia_u8 *input_addr_vir = XMEDIA_NULL;
    xmedia_u8 *output_addr_vir = XMEDIA_NULL;
    xmedia_handle chnid = 0;
    xmedia_u8 aes_key[32] = {0x2B,0x7E,0x15,0x16,0x28,0xAE,0xD2,0xA6,0xAB,0xF7,0x15,0x88,0x09,0xCF,0x4F,0x3C};
    xmedia_u8 aes_iv[16]  = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};
    xmedia_u8 aes_src[16] = {0x6B,0xC1,0xBE,0xE2,0x2E,0x40,0x9F,0x96,0xE9,0x3D,0x7E,0x11,0x73,0x93,0x17,0x2A};
    xmedia_u8 aes_dst[16] = {0x76,0x49,0xAB,0xAC,0x81,0x19,0xB2,0x46,0xCE,0xE9,0x8E,0x9B,0x12,0xE9,0x19,0x7D};

    ret = xmedia_cipher_init();
    if (XMEDIA_SUCCESS != ret) {
        return ret;
    }

    ret = xmedia_cipher_create_handle(&chnid);
    if (XMEDIA_SUCCESS != ret) {
        xmedia_cipher_exit();
        return ret;
    }

    printf("chnid 0x%d\n", chnid);

    input_addr_phy = xmedia_mmz_alloc(XMEDIA_NULL, "CIPHER_BufIn", data_len);
    if (0 == input_addr_phy)
    {
        ERR_CODE_CIPHER("Error: Get phyaddr for input failed!\n");
        goto __CIPHER_EXIT__;
    }
    input_addr_vir = xmedia_mmz_map(input_addr_phy, data_len, cached);

    output_addr_phy = xmedia_mmz_alloc(XMEDIA_NULL, "CIPHER_BufOut", data_len);
    if (0 == output_addr_phy)
    {
        ERR_CODE_CIPHER("Error: Get phyaddr for output failed!\n");
        goto __CIPHER_EXIT__;
    }
    output_addr_vir = xmedia_mmz_map(output_addr_phy, data_len, cached);

    // For encrypt
    ret = set_config_info(chnid,
                          XMEDIA_FALSE,
                          XMEDIA_CIPHER_ALG_AES,
                          XMEDIA_CIPHER_WORK_MODE_CBC,
                          XMEDIA_CIPHER_KEY_AES_128BIT,
                          aes_key,
                          aes_iv);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Set config info failed.\n");
        goto __CIPHER_EXIT__;
    }

    memset(input_addr_vir, 0x0, data_len);
    memcpy(input_addr_vir, aes_src, data_len);
    print_buffer("CBC-AES-128-ORI:", aes_src, sizeof(aes_src));

    memset(output_addr_vir, 0x0, data_len);

    ret = xmedia_cipher_encrypt(chnid, input_addr_phy, output_addr_phy, data_len);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Cipher encrypt failed.\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

    print_buffer("CBC-AES-128-ENC:", output_addr_vir, sizeof(aes_dst));

    // compare
    if (0 != memcmp(output_addr_vir, aes_dst, data_len)) {
        ERR_CODE_CIPHER("cipher encrypt, memcmp failed!\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

    // For decrypt
    memcpy(input_addr_vir, aes_dst, data_len);
    memset(output_addr_vir, 0x0, data_len);

    ret = set_config_info(chnid,
                          XMEDIA_FALSE,
                          XMEDIA_CIPHER_ALG_AES,
                          XMEDIA_CIPHER_WORK_MODE_CBC,
                          XMEDIA_CIPHER_KEY_AES_128BIT,
                          aes_key,
                          aes_iv);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Set config info failed.\n");
        goto __CIPHER_EXIT__;
    }

    ret = xmedia_cipher_decrypt(chnid, input_addr_phy, output_addr_phy, data_len);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Cipher decrypt failed.\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

    print_buffer("CBC-AES-128-DEC:", output_addr_vir, data_len);
    // compare
    if (0 != memcmp(output_addr_vir, aes_src, data_len)) {
        ERR_CODE_CIPHER("cipher decrypt, memcmp failed!\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

    TEST_END_PASS();

__CIPHER_EXIT__:

    if (input_addr_phy > 0) {
        xmedia_mmz_unmap(input_addr_vir);
        xmedia_mmz_free(input_addr_phy);
    }

    if (output_addr_phy > 0) {
        xmedia_mmz_unmap(output_addr_vir);
        xmedia_mmz_free(output_addr_phy);
    }

    xmedia_cipher_destroy_handle(chnid);
    xmedia_cipher_exit();

    return ret;
}

static xmedia_s32 cfb_aes128(xmedia_void)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_u32 data_len = 32;
    xmedia_ulong input_addr_phy = 0;
    xmedia_ulong output_addr_phy = 0;
    xmedia_u32 cached = 0;
    xmedia_u8 *input_addr_vir = XMEDIA_NULL;
    xmedia_u8 *output_addr_vir = XMEDIA_NULL;
    xmedia_handle chnid = 0;
    xmedia_u8 aes_key[32] = {"\x2B\x7E\x15\x16\x28\xAE\xD2\xA6\xAB\xF7\x15\x88\x09\xCF\x4F\x3C"};
    xmedia_u8 aes_iv[16]  = {"\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"};
    xmedia_u8 aes_src[32] = {"\x6B\xC1\xBE\xE2\x2E\x40\x9F\x96\xE9\x3D\x7E\x11\x73\x93\x17\x2A\xAE\x2D\x8A\x57\x1E\x03\xAC\x9C\x9E\xB7\x6F\xAC\x45\xAF\x8E\x51"};
    xmedia_u8 aes_dst[32] = {"\x3B\x3F\xD9\x2E\xB7\x2D\xAD\x20\x33\x34\x49\xF8\xE8\x3C\xFB\x4A\xC8\xA6\x45\x37\xA0\xB3\xA9\x3F\xCD\xE3\xCD\xAD\x9F\x1C\xE5\x8B"};

    ret = xmedia_cipher_init();
    if (XMEDIA_SUCCESS != ret) {
        return ret;
    }

    ret = xmedia_cipher_create_handle(&chnid);
    if (XMEDIA_SUCCESS != ret) {
        xmedia_cipher_exit();
        return ret;
    }
    input_addr_phy = xmedia_mmz_alloc(XMEDIA_NULL, "CIPHER_BufIn", data_len);
    if (0 == input_addr_phy) {
        ERR_CODE_CIPHER("Error: Get phyaddr for input failed!\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }
    input_addr_vir = xmedia_mmz_map(input_addr_phy, data_len, cached);
    output_addr_phy = xmedia_mmz_alloc(XMEDIA_NULL, "CIPHER_BufOut", data_len);
    if (0 == output_addr_phy) {
        ERR_CODE_CIPHER("Error: Get phyaddr for output failed!\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }
    output_addr_vir = xmedia_mmz_map(output_addr_phy, data_len, cached);

    // For encrypt
    ret = set_config_info(chnid,
                          XMEDIA_FALSE,
                          XMEDIA_CIPHER_ALG_AES,
                          XMEDIA_CIPHER_WORK_MODE_CFB,
                          XMEDIA_CIPHER_KEY_AES_128BIT,
                          aes_key,
                          aes_iv);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Set config info failed.\n");
        goto __CIPHER_EXIT__;
    }

    memset(input_addr_vir, 0x0, data_len);
    memcpy(input_addr_vir, aes_src, data_len);
    print_buffer("CFB-AES-128-ORI:", aes_src, data_len);

    memset(output_addr_vir, 0x0, data_len);

    ret = xmedia_cipher_encrypt(chnid, input_addr_phy, output_addr_phy, data_len);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Cipher encrypt failed.\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

    print_buffer("CFB-AES-128-ENC:", output_addr_vir, data_len);

    //compare
    if (0 != memcmp(output_addr_vir, aes_dst, data_len)) {
        ERR_CODE_CIPHER("Memcmp failed!\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

   // For decrypt
    memcpy(input_addr_vir, aes_dst, data_len);
    memset(output_addr_vir, 0x0, data_len);

    ret = set_config_info(chnid,
                          XMEDIA_FALSE,
                          XMEDIA_CIPHER_ALG_AES,
                          XMEDIA_CIPHER_WORK_MODE_CFB,
                          XMEDIA_CIPHER_KEY_AES_128BIT,
                          aes_key,
                          aes_iv);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Set config info failed.\n");
        goto __CIPHER_EXIT__;
    }

    ret = xmedia_cipher_decrypt(chnid, input_addr_phy, output_addr_phy, data_len);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Cipher decrypt failed.\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

    print_buffer("CFB-AES-128-DEC", output_addr_vir, data_len);
    // compare
    if (0 != memcmp(output_addr_vir, aes_src, data_len)) {
        ERR_CODE_CIPHER("Memcmp failed!\n");
        goto __CIPHER_EXIT__;
    }

    TEST_END_PASS();

__CIPHER_EXIT__:

    if (input_addr_phy > 0) {
        xmedia_mmz_unmap(input_addr_vir);
        xmedia_mmz_free(input_addr_phy);
    }

    if (output_addr_phy > 0) {
        xmedia_mmz_unmap(output_addr_vir);
        xmedia_mmz_free(output_addr_phy);
    }

    xmedia_cipher_destroy_handle(chnid);
    xmedia_cipher_exit();

    return ret;
}

static xmedia_s32 ctr_aes128(xmedia_void)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_u32 data_len = 32;
    xmedia_ulong input_addr_phy = 0;
    xmedia_ulong output_addr_phy = 0;
    xmedia_u32 cached = 0;
    xmedia_u8 *input_addr_vir = XMEDIA_NULL;
    xmedia_u8 *output_addr_vir = XMEDIA_NULL;
    xmedia_handle chnid = 0;
    xmedia_u8 aes_key[32] = {"\x7E\x24\x06\x78\x17\xFA\xE0\xD7\x43\xD6\xCE\x1F\x32\x53\x91\x63"};
    xmedia_u8 aes_iv[16]  = {"\x00\x6C\xB6\xDB\xC0\x54\x3B\x59\xDA\x48\xD9\x0B\x00\x00\x00\x01"};
    xmedia_u8 aes_src[32] = {"\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"};
    xmedia_u8 aes_dst[32] = {"\x51\x04\xA1\x06\x16\x8A\x72\xD9\x79\x0D\x41\xEE\x8E\xDA\xD3\x88\xEB\x2E\x1E\xFC\x46\xDA\x57\xC8\xFC\xE6\x30\xDF\x91\x41\xBE\x28"};

    ret = xmedia_cipher_init();
    if (XMEDIA_SUCCESS != ret) {
        return ret;
    }

    ret = xmedia_cipher_create_handle(&chnid);
    if (XMEDIA_SUCCESS != ret) {
        xmedia_cipher_exit();
        return ret;
    }
    input_addr_phy = xmedia_mmz_alloc(XMEDIA_NULL, "CIPHER_BufIn", data_len);
    if (0 == input_addr_phy) {
        ERR_CODE_CIPHER("Error: Get phyaddr for input failed!\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }
    input_addr_vir = xmedia_mmz_map(input_addr_phy, data_len, cached);
    output_addr_phy = xmedia_mmz_alloc(XMEDIA_NULL, "CIPHER_BufOut", data_len);
    if (0 == output_addr_phy) {
        ERR_CODE_CIPHER("Error: Get phyaddr for outPut failed!\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }
    output_addr_vir = xmedia_mmz_map(output_addr_phy, data_len, cached);

    // For encrypt
    ret = set_config_info(chnid,
                          XMEDIA_FALSE,
                          XMEDIA_CIPHER_ALG_AES,
                          XMEDIA_CIPHER_WORK_MODE_CTR,
                          XMEDIA_CIPHER_KEY_AES_128BIT,
                          aes_key,
                          aes_iv);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Set config info failed.\n");
        goto __CIPHER_EXIT__;
    }

    memset(input_addr_vir, 0x0, data_len);
    memcpy(input_addr_vir, aes_src, data_len);
    print_buffer("CTR-AES-128-ORI:", aes_src, data_len);

    memset(output_addr_vir, 0x0, data_len);

    ret = xmedia_cipher_encrypt(chnid, input_addr_phy, output_addr_phy, data_len);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Cipher encrypt failed.\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

    print_buffer("CTR-AES-128-ENC:", output_addr_vir, data_len);

    // compare
    if (0 != memcmp(output_addr_vir, aes_dst, data_len)) {
        ERR_CODE_CIPHER("Memcmp failed!\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

   // For decrypt
    memcpy(input_addr_vir, aes_dst, data_len);
    memset(output_addr_vir, 0x0, data_len);

    ret = set_config_info(chnid,
                          XMEDIA_FALSE,
                          XMEDIA_CIPHER_ALG_AES,
                          XMEDIA_CIPHER_WORK_MODE_CTR,
                          XMEDIA_CIPHER_KEY_AES_128BIT,
                          aes_key,
                          aes_iv);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Set config info failed.\n");
        goto __CIPHER_EXIT__;
    }

    ret = xmedia_cipher_decrypt(chnid, input_addr_phy, output_addr_phy, data_len);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Cipher decrypt failed.\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

    print_buffer("CTR-AES-128-DEC", output_addr_vir, data_len);
    // compare
    if (0 != memcmp(output_addr_vir, aes_src, data_len)) {
        ERR_CODE_CIPHER("Memcmp failed!\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

    TEST_END_PASS();

__CIPHER_EXIT__:

    if (input_addr_phy > 0) {
        xmedia_mmz_unmap(input_addr_vir);
        xmedia_mmz_free(input_addr_phy);
    }

    if (output_addr_phy > 0) {
        xmedia_mmz_unmap(output_addr_vir);
        xmedia_mmz_free(output_addr_phy);
    }

    xmedia_cipher_destroy_handle(chnid);
    xmedia_cipher_exit();

    return ret;
}

int sample_cipher(xmedia_void)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;

    ret = ecb_aes256();
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    ret = cbc_aes256();
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    ret = cbc_aes192();
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    ret = cbc_aes128();
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    ret = cfb_aes128();
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    ret = ctr_aes128();
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    return XMEDIA_SUCCESS;
}

