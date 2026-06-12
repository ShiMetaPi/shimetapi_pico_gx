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

#define U32_TO_POINT(addr)  ((xmedia_void*)((xmedia_ulong)(addr)))
#define POINT_TO_U32(addr)  ((xmedia_u32)((xmedia_ulong)(addr)))

static xmedia_u8 aes_key[16] = {0x2B,0x7E,0x15,0x16,0x28,0xAE,0xD2,0xA6,0xAB,0xF7,0x15,0x88,0x09,0xCF,0x4F,0x3C};
static xmedia_u8 aes_iv[16]  = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};
static xmedia_u8 aes_cbc_enc_src_buf[32] = {0x6B,0xC1,0xBE,0xE2,0x2E,0x40,0x9F,0x96,0xE9,0x3D,0x7E,0x11,0x73,0x93,0x17,0x2A,
                                            0x6B,0xC1,0xBE,0xE2,0x2E,0x40,0x9F,0x96,0xE9,0x3D,0x7E,0x11,0x73,0x93,0x17,0x2A};
static xmedia_u8 aes_cbc_enc_dst_buf[32] = {0x76,0x49,0xAB,0xAC,0x81,0x19,0xB2,0x46,0xCE,0xE9,0x8E,0x9B,0x12,0xE9,0x19,0x7D,
                                            0x4C,0xbb,0xc8,0x58,0x75,0x6b,0x35,0x81,0x25,0x52,0x9e,0x96,0x98,0xa3,0x8f,0x44};
static xmedia_u8 aes_cbc_dec_src_buf[16] = {0x76,0x49,0xAB,0xAC,0x81,0x19,0xB2,0x46,0xCE,0xE9,0x8E,0x9B,0x12,0xE9,0x19,0x7D};
static xmedia_u8 aes_cbc_dec_dst_buf[16] = {0x6B,0xC1,0xBE,0xE2,0x2E,0x40,0x9F,0x96,0xE9,0x3D,0x7E,0x11,0x73,0x93,0x17,0x2A};

static xmedia_s32 set_config_info(xmedia_handle handle,
                                        xmedia_bool key_by_ca,
                                        xmedia_cipher_alg alg,
                                        xmedia_cipher_work_mode mode,
                                        xmedia_cipher_key_length key_len,
                                        const xmedia_u8 *key,
                                        const xmedia_u8 iv[AES_IV_LENGTH])
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
            if (ctrl.mode != XMEDIA_CIPHER_WORK_MODE_ECB) {
                ctrl.aes_ctrl.iv_usage = CIPHER_IV_CHANGE_ALL_PKG;  //must set for CBC , CFB mode
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

static xmedia_s32 multicipher_aes_cbc_enc(xmedia_void)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_u32 data_len = 32;
    xmedia_u32 cached = 0;
    xmedia_u8 *input_addr_vir[3] = {0};
    xmedia_u8 *output_addr_vir[3] = {0};
    xmedia_handle chnid = 0;
    xmedia_cipher_package cipher_data_array[3];
    xmedia_u32 i;

    memset(&cipher_data_array, 0, sizeof(cipher_data_array));

    ret = xmedia_cipher_init();
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Cipher init failed.\n");
        return XMEDIA_FAILURE;
    }

    ret = xmedia_cipher_create_handle(&chnid);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Cipher create handle failed.\n");
        goto __CIPHER_EXIT__;
    }

    for(i = 0; i < 3; i++) {
        cipher_data_array[i].src_phy_addr = POINT_TO_U32(xmedia_mmz_alloc(XMEDIA_NULL, "CIPHER_BufIn", data_len));
        if (0 == cipher_data_array[i].src_phy_addr) {
            ERR_CODE_CIPHER("Error: Get phyaddr for input failed!\n");
            ret = XMEDIA_FAILURE;
            goto __CIPHER_EXIT__;
        }
        input_addr_vir[i] = xmedia_mmz_map(cipher_data_array[i].src_phy_addr, data_len, cached);

        cipher_data_array[i].dest_phy_addr = POINT_TO_U32(xmedia_mmz_alloc(XMEDIA_NULL, "CIPHER_BufOut", data_len));
        if (0 == cipher_data_array[i].dest_phy_addr) {
            ERR_CODE_CIPHER("Error: Get phyaddr for outPut failed!\n");
            ret = XMEDIA_FAILURE;
            goto __CIPHER_EXIT__;
        }
        output_addr_vir[i] = xmedia_mmz_map(cipher_data_array[i].dest_phy_addr, data_len, cached);

        cipher_data_array[i].byte_length = data_len;
        memset(input_addr_vir[i], 0x0, data_len);
        memcpy(input_addr_vir[i], aes_cbc_enc_src_buf, data_len);
        memset(output_addr_vir[i], 0x0, data_len);
    }

    ret = set_config_info(chnid,
                          XMEDIA_FALSE,
                          XMEDIA_CIPHER_ALG_AES,
                          XMEDIA_CIPHER_WORK_MODE_CBC,
                          XMEDIA_CIPHER_KEY_AES_128BIT,
                          aes_key,
                          aes_iv);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Cipher setconfiginfo failed.\n");
        goto __CIPHER_EXIT__;
    }

    ret = xmedia_cipher_encrypt_multi(chnid, cipher_data_array, 3);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Cipher encryptMulti failed.\n");
        goto __CIPHER_EXIT__;
    }

    print_buffer("GOLD", aes_cbc_enc_dst_buf, data_len);

    for (i = 0; i < 3; i++) {
        if (0 != memcmp(output_addr_vir[i], aes_cbc_enc_dst_buf, data_len)) {
            ERR_CODE_CIPHER("MultiCipher AES CBC Encryption run failed on array %d!\n", i);
            print_buffer("ENC", output_addr_vir[i], data_len);
            ret = XMEDIA_FAILURE;
            goto __CIPHER_EXIT__;
        }
    }

    TEST_END_PASS();

__CIPHER_EXIT__:

    for (i = 0; i < 3; i++) {
        if (cipher_data_array[i].src_phy_addr > 0) {
            xmedia_mmz_unmap(input_addr_vir[i]);
            xmedia_mmz_free(cipher_data_array[i].src_phy_addr);
        }

        if (cipher_data_array[i].dest_phy_addr > 0) {
            xmedia_mmz_unmap(output_addr_vir[i]);
            xmedia_mmz_free(cipher_data_array[i].dest_phy_addr);
        }
    }

    xmedia_cipher_destroy_handle(chnid);
    xmedia_cipher_exit();

    return ret;
}


static xmedia_s32 multicipher_aes_cbc_dec(xmedia_void)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_u32 data_len = 16;
    xmedia_u32 cached = 0;
    xmedia_u8 *input_addr_vir[3] = {0};
    xmedia_u8 *output_addr_vir[3] = {0};
    xmedia_handle chnid = 0;
    xmedia_cipher_package cipher_data_array[3];
    xmedia_u32 i;

    memset(&cipher_data_array, 0, sizeof(cipher_data_array));

    ret = xmedia_cipher_init();
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Cipher init failed.\n");
        return XMEDIA_FAILURE;
    }

    ret = xmedia_cipher_create_handle(&chnid);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Cipher Create handle failed.\n");
        goto __CIPHER_EXIT__;
    }

    for (i = 0; i < 3; i++) {
        cipher_data_array[i].src_phy_addr = POINT_TO_U32(xmedia_mmz_alloc(XMEDIA_NULL, "cipherIn", data_len));
        if (0 == cipher_data_array[i].src_phy_addr) {
            ERR_CODE_CIPHER("Error: Get phyaddr for input failed!\n");
            ret = XMEDIA_FAILURE;
            goto __CIPHER_EXIT__;
        }
        input_addr_vir[i] = xmedia_mmz_map(cipher_data_array[i].src_phy_addr, data_len, cached);

        cipher_data_array[i].dest_phy_addr = POINT_TO_U32(xmedia_mmz_alloc(XMEDIA_NULL, "cipherOut", data_len));
        if (0 == cipher_data_array[i].dest_phy_addr) {
            ERR_CODE_CIPHER("Error: Get phyaddr for outPut failed!\n");
            ret = XMEDIA_FAILURE;
            goto __CIPHER_EXIT__;
        }
        output_addr_vir[i] = xmedia_mmz_map(cipher_data_array[i].dest_phy_addr, data_len, cached);

        cipher_data_array[i].byte_length = data_len;
        memset(input_addr_vir[i], 0x0, data_len);
        memcpy(input_addr_vir[i], aes_cbc_dec_src_buf, data_len);
        memset(output_addr_vir[i], 0x0, data_len);
    }

    ret = set_config_info(chnid,
                          XMEDIA_FALSE,
                          XMEDIA_CIPHER_ALG_AES,
                          XMEDIA_CIPHER_WORK_MODE_CBC,
                          XMEDIA_CIPHER_KEY_AES_128BIT,
                          aes_key,
                          aes_iv);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Cipher setconfiginfo failed.\n");
        goto __CIPHER_EXIT__;
    }

    ret = xmedia_cipher_decrypt_multi(chnid, cipher_data_array, 3);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Cipher DecryptMulti failed.\n");
        goto __CIPHER_EXIT__;
    }

    for (i = 0; i < 3; i++) {
        if (0 != memcmp(output_addr_vir[i], aes_cbc_dec_dst_buf, data_len)) {
            ERR_CODE_CIPHER("MultiCipher AES CBC Decryption run failed on array %d!\n", i);
            print_buffer("DEC", output_addr_vir[i], data_len);
            ret = XMEDIA_FAILURE;
            goto __CIPHER_EXIT__;
        }
    }

    TEST_END_PASS();

__CIPHER_EXIT__:

    for (i = 0; i < 3; i++) {
        if (cipher_data_array[i].src_phy_addr > 0) {
            xmedia_mmz_unmap(input_addr_vir[i]);
            xmedia_mmz_free(cipher_data_array[i].src_phy_addr);
        }

        if (cipher_data_array[i].dest_phy_addr > 0) {
            xmedia_mmz_unmap(output_addr_vir[i]);
            xmedia_mmz_free(cipher_data_array[i].dest_phy_addr);
        }
    }

    xmedia_cipher_destroy_handle(chnid);
    xmedia_cipher_exit();

    return ret;
}

int sample_multicipher(xmedia_void)
{

    xmedia_s32 ret = XMEDIA_SUCCESS;

    ret = multicipher_aes_cbc_enc();
    if (ret != XMEDIA_SUCCESS) {
        ERR_CODE_CIPHER("Multi cipher ex aes cbc encrypt failed.\n");
        return ret;
    }

    ret = multicipher_aes_cbc_dec();
    if (ret != XMEDIA_SUCCESS) {
        ERR_CODE_CIPHER("Multi cipher ex aes cbc decrypt failed.\n");
        return ret;
    }

    return XMEDIA_SUCCESS;
}
