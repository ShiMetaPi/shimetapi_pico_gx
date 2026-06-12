/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <assert.h>

#include "xmedia_type.h"
#include "xmedia_cipher.h"
#include "xmedia_memory.h"
#include "sample_common.h"

#define KEY_IS_ZERO

#define AES_KEY_MAX_LENGTH      32
#define AES128_KEY_LENGTH       16
#define AES_IV_LENGTH           16

static xmedia_u8 aes_128_cbc_key[16] = {0x00,0xe9,0x85,0xde,0x41,0x67,0x6f,0x00,0x96,0x94,0x41,0x54,0x02,0x7d,0xe7,0xc5};
#ifdef KEY_IS_ZERO
// 使用的AES硬件key（OTP中的key）需要为全0(默认为全0)
static xmedia_u8 aes_128_enc_key[16] = {0x00,0x86,0x30,0x1E,0xAF,0x79,0x10,0x06,0x14,0x65,0x6C,0x08,0xE5,0xEE,0x7C,0x19};
#else
/*
 * 使用的AES硬件key（OTP中的key）需要为00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff,
 * 烧写AES硬件key的方法请参考《OTP API参考.docx》
 */
static xmedia_u8 aes_128_enc_key[16] = {0x53,0x2d,0x81,0x71,0x24,0x3d,0x70,0xcc,0x55,0xad,0x4f,0xb7,0x2a,0x52,0x06,0x8d};
#endif
static xmedia_u8 aes_128_cbc_IV[16]  = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};
static xmedia_u8 aes_128_src_buf[16] = {0x6B,0xC1,0xBE,0xE2,0x2E,0x40,0x9F,0x96,0xE9,0x3D,0x7E,0x11,0x73,0x93,0x17,0x2A};
static xmedia_u8 aes_128_dst_buf[16] = {0xF0,0XB7,0X6A,0X0C,0X16,0X06,0XD4,0XDE,0X79,0XCB,0X5C,0X66,0XD3,0X51,0X0C,0XAB};

static xmedia_s32 set_config(xmedia_handle handle, xmedia_cipher_ca_type type, xmedia_cipher_alg alg, xmedia_cipher_work_mode mode,
                                 xmedia_cipher_key_length key_len, const xmedia_u8 *key, const xmedia_u8 iv[AES_IV_LENGTH])
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_cipher_ctrl ctrl;
    memset(&ctrl, 0, sizeof(xmedia_cipher_ctrl));

    ctrl.alg = alg;
    ctrl.mode = mode;
    ctrl.key_by_ca = XMEDIA_TRUE;
    ctrl.ca_type = type;

    if (ctrl.alg == XMEDIA_CIPHER_ALG_AES) {
        /*only support AES ECB/CBC/CFB/OFB/CTR*/
        if (ctrl.mode <= XMEDIA_CIPHER_WORK_MODE_CTR && ctrl.mode >= XMEDIA_CIPHER_WORK_MODE_ECB) {
            ctrl.aes_ctrl.bit_width = XMEDIA_CIPHER_BIT_WIDTH_128BIT;
            ctrl.aes_ctrl.key_len = key_len;
            if (ctrl.mode != XMEDIA_CIPHER_WORK_MODE_ECB) {
                ctrl.aes_ctrl.iv_usage = 1;  // must set for CBC , CFB mode
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

xmedia_s32 sample_cipher_efuse_vir(xmedia_void)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_u32 data_len = 16;
    xmedia_handle chnid = 0;
    xmedia_cipher_ca_type key_src;
    xmedia_u8 u8Key[AES128_KEY_LENGTH];
    xmedia_u8 enc_result[16];
    xmedia_u8 dec_result[16];

    ret = xmedia_cipher_init();
    if (XMEDIA_SUCCESS != ret) {
        return XMEDIA_FAILURE;
    }

    memset(u8Key, 0x0, AES128_KEY_LENGTH);

    key_src = XMEDIA_CIPHER_KEY_SRC_KLAD_1;

    ret = xmedia_cipher_klad_encrypt_key(XMEDIA_CIPHER_KEY_SRC_KLAD_1, XMEDIA_CIPHER_KLAD_TARGET_AES, aes_128_cbc_key, u8Key, AES128_KEY_LENGTH);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Error: Klad Encrypt Key failed!\n");
        return XMEDIA_FAILURE;
    }
    if (0 != memcmp(u8Key, aes_128_enc_key, 16)) {
        ERR_CODE_CIPHER("Memcmp failed!\n");
        print_buffer("clean key", aes_128_cbc_key, 16);
        print_buffer("encrypt key", u8Key, 16);
        return XMEDIA_FAILURE;
    }

    ret = xmedia_cipher_create_handle(&chnid);
    if (XMEDIA_SUCCESS != ret) {
        xmedia_cipher_exit();
        return XMEDIA_FAILURE;
    }

    ret = set_config(chnid,
                     key_src,
                     XMEDIA_CIPHER_ALG_AES,
                     XMEDIA_CIPHER_WORK_MODE_CBC,
                     XMEDIA_CIPHER_KEY_AES_128BIT,
                     u8Key,
                     aes_128_cbc_IV);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Set config info failed.\n");
        goto __CIPHER_EXIT__;
    }

    print_buffer("clear text:", aes_128_src_buf, sizeof(aes_128_src_buf));

    memset(enc_result, 0x0, data_len);

    ret = xmedia_cipher_encrypt_vir(chnid, aes_128_src_buf, enc_result, data_len);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Cipher encrypt failed.\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

    print_buffer("encrypted text:", enc_result, data_len);

    /* compare */
    if (0 != memcmp(enc_result, aes_128_dst_buf, data_len)) {
        ERR_CODE_CIPHER("Memcmp failed!\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

    memset(dec_result, 0x0, data_len);

    ret = set_config(chnid,
                     key_src,
                     XMEDIA_CIPHER_ALG_AES,
                     XMEDIA_CIPHER_WORK_MODE_CBC,
                     XMEDIA_CIPHER_KEY_AES_128BIT,
                     u8Key,
                     aes_128_cbc_IV);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Set config info failed.\n");
        goto __CIPHER_EXIT__;
    }

    print_buffer("before decrypt:", enc_result, data_len);
    ret = xmedia_cipher_decrypt_vir(chnid, enc_result, dec_result, data_len);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Cipher decrypt failed.\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

    print_buffer("decrypted text:", dec_result, data_len);
    /* compare */
    if (0 != memcmp(dec_result, aes_128_src_buf, data_len)) {
        ERR_CODE_CIPHER("Memcmp failed!\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

    TEST_END_PASS();

__CIPHER_EXIT__:

    xmedia_cipher_destroy_handle(chnid);
    xmedia_cipher_exit();

    return ret;
}

xmedia_s32 sample_cipher_efuse_phy(xmedia_void)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_u32 data_len = 16;
    xmedia_u32 input_addr_phy = 0;
    xmedia_u32 output_addr_phy = 0;
    xmedia_u32 cached = 0;
    xmedia_u8 *input_addr_vir = XMEDIA_NULL;
    xmedia_u8 *output_addr_vir = XMEDIA_NULL;
    xmedia_handle chnid = 0;
    xmedia_cipher_ca_type key_src;
    xmedia_u8 u8Key[AES128_KEY_LENGTH];

    ret = xmedia_cipher_init();
    if (XMEDIA_SUCCESS != ret) {
        return XMEDIA_FAILURE;
    }

    memset(u8Key, 0x0, AES128_KEY_LENGTH);

    key_src = XMEDIA_CIPHER_KEY_SRC_KLAD_1;

    ret = xmedia_cipher_klad_encrypt_key(XMEDIA_CIPHER_KEY_SRC_KLAD_1, XMEDIA_CIPHER_KLAD_TARGET_AES, aes_128_cbc_key, u8Key, AES128_KEY_LENGTH);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Error: Klad Encrypt Key failed!\n");
        return XMEDIA_FAILURE;
    }
    if (0 != memcmp(u8Key, aes_128_enc_key, 16)) {
        ERR_CODE_CIPHER("Memcmp failed!\n");
        print_buffer("clean key", aes_128_cbc_key, 16);
        print_buffer("encrypt key", u8Key, 16);
        return XMEDIA_FAILURE;
    }

    ret = xmedia_cipher_create_handle(&chnid);
    if (XMEDIA_SUCCESS != ret) {
        xmedia_cipher_exit();
        return XMEDIA_FAILURE;
    }

    input_addr_phy = (xmedia_u32)xmedia_mmz_alloc(XMEDIA_NULL, "CIPHER_BufIn",data_len);
    if (0 == input_addr_phy) {
        ERR_CODE_CIPHER("Error: Get phyaddr for input failed!\n");
        goto __CIPHER_EXIT__;
    }
    input_addr_vir = xmedia_mmz_map(input_addr_phy, data_len, cached);
    output_addr_phy = (xmedia_u32)xmedia_mmz_alloc(XMEDIA_NULL, "CIPHER_BufOut", data_len);
    if (0 == output_addr_phy) {
        ERR_CODE_CIPHER("Error: Get phyaddr for outPut failed!\n");
        goto __CIPHER_EXIT__;
    }
    output_addr_vir = xmedia_mmz_map(output_addr_phy, data_len, cached);
    if (0 == output_addr_vir) {
        ERR_CODE_CIPHER("Errror: Get Viraddr for outPut failed!\n");
        goto __CIPHER_EXIT__;
    }

    // For encrypt
    ret = set_config(chnid,
                     key_src,
                     XMEDIA_CIPHER_ALG_AES,
                     XMEDIA_CIPHER_WORK_MODE_CBC,
                     XMEDIA_CIPHER_KEY_AES_128BIT,
                     u8Key,
                     aes_128_cbc_IV);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Set config info failed.\n");
        goto __CIPHER_EXIT__;
    }

    memset(input_addr_vir, 0x0, data_len);
    memcpy(input_addr_vir, aes_128_src_buf, data_len);
    print_buffer("clear text:", aes_128_src_buf, sizeof(aes_128_src_buf));

    memset(output_addr_vir, 0x0, data_len);

    ret = xmedia_cipher_encrypt(chnid, input_addr_phy, output_addr_phy, data_len);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Cipher encrypt failed.\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

    print_buffer("encrypted text:", output_addr_vir, sizeof(aes_128_dst_buf));

    // compare
    if (0 != memcmp(output_addr_vir, aes_128_dst_buf, data_len)) {
        ERR_CODE_CIPHER("Memcmp failed!\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

   //For decrypt
    memcpy(input_addr_vir, aes_128_dst_buf, data_len);
    memset(output_addr_vir, 0x0, data_len);

    ret = set_config(chnid,
                     key_src,
                     XMEDIA_CIPHER_ALG_AES,
                     XMEDIA_CIPHER_WORK_MODE_CBC,
                     XMEDIA_CIPHER_KEY_AES_128BIT,
                     u8Key,
                     aes_128_cbc_IV);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Set config info failed.\n");
        goto __CIPHER_EXIT__;
    }

    print_buffer("before decrypt:", aes_128_dst_buf, sizeof(aes_128_dst_buf));
    ret = xmedia_cipher_decrypt(chnid, input_addr_phy, output_addr_phy, data_len);
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("Cipher decrypt failed.\n");
        ret = XMEDIA_FAILURE;
        goto __CIPHER_EXIT__;
    }

    print_buffer("decrypted text:", output_addr_vir, data_len);
    // compare
    if (0 != memcmp(output_addr_vir, aes_128_src_buf, data_len)) {
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

int sample_cipher_efuse(xmedia_void)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;

    ret = sample_cipher_efuse_vir();
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    ret = sample_cipher_efuse_phy();
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    return ret;
}
