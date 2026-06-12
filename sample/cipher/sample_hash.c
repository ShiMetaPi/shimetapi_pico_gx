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
#include <linux/fb.h>
#include <assert.h>

#include "xmedia_type.h"
#include "xmedia_cipher.h"
#include "sample_common.h"

#define MAX_HASH_HANDLE 4
#define MAX_HMAC_HANDLE 3

#define LONG_DATA_SIZE 10000

static unsigned char sha1_buf[3][128] = {
    {"abc"},
    {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"},
    {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopqabcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"}
};
static const int sha1_buflen[4] ={3, 56, 112, 1000000};
static const unsigned char sha1_sum[5][20] =
{
    {0xA9, 0x99, 0x3E, 0x36, 0x47, 0x06, 0x81, 0x6A, 0xBA, 0x3E, 0x25, 0x71, 0x78, 0x50, 0xC2, 0x6C, 0x9C, 0xD0, 0xD8, 0x9D},
    {0x84, 0x98, 0x3E, 0x44, 0x1C, 0x3B, 0xD2, 0x6E, 0xBA, 0xAE, 0x4A, 0xA1, 0xF9, 0x51, 0x29, 0xE5, 0xE5, 0x46, 0x70, 0xF1},
    {0xaf, 0xc5, 0x3a, 0x4e, 0xa2, 0x08, 0x56, 0xf9, 0x8e, 0x08, 0xdc, 0x6f, 0x3a, 0x5c, 0x98, 0x33, 0x13, 0x77, 0x68, 0xed},
    {0x34, 0xaa, 0x97, 0x3c, 0xd4, 0xc4, 0xda, 0xa4, 0xf6, 0x1e, 0xeb, 0x2b, 0xdb, 0xad, 0x27, 0x31, 0x65, 0x34, 0x01, 0x6f},
    {0x7d, 0xf9, 0x62, 0x1f, 0x17, 0xad, 0x18, 0xc5, 0x8a, 0x5a, 0xf7, 0x99, 0x1d, 0x12, 0x62, 0x20, 0x0f, 0xaf, 0xa9, 0x0f}
};

static const unsigned char sha2_sum[4][32] ={
    /* SHA-256 sample vectors */
    {0xBA, 0x78, 0x16, 0xBF, 0x8F, 0x01, 0xCF, 0xEA, 0x41, 0x41, 0x40, 0xDE, 0x5D, 0xAE, 0x22, 0x23, 0xB0, 0x03, 0x61, 0xA3, 0x96, 0x17, 0x7A, 0x9C, 0xB4, 0x10, 0xFF, 0x61, 0xF2, 0x00, 0x15, 0xAD},
    {0x24, 0x8D, 0x6A, 0x61, 0xD2, 0x06, 0x38, 0xB8, 0xE5, 0xC0, 0x26, 0x93, 0x0C, 0x3E, 0x60, 0x39, 0xA3, 0x3C, 0xE4, 0x59, 0x64, 0xFF, 0x21, 0x67, 0xF6, 0xEC, 0xED, 0xD4, 0x19, 0xDB, 0x06, 0xC1},
    {0x59, 0xf1, 0x09, 0xd9, 0x53, 0x3b, 0x2b, 0x70, 0xe7, 0xc3, 0xb8, 0x14, 0xa2, 0xbd, 0x21, 0x8f, 0x78, 0xea, 0x5d, 0x37, 0x14, 0x45, 0x5b, 0xc6, 0x79, 0x87, 0xcf, 0x0d, 0x66, 0x43, 0x99 ,0xcf},
    {0xcd, 0xc7, 0x6e, 0x5c, 0x99, 0x14, 0xfb, 0x92, 0x81, 0xa1, 0xc7, 0xe2, 0x84, 0xd7, 0x3e, 0x67, 0xf1, 0x80, 0x9a, 0x48, 0xa4, 0x97, 0x20, 0x0e, 0x04, 0x6d, 0x39, 0xcc, 0xc7, 0x11, 0x2c, 0xd0}
};

static const unsigned char sha224_sum[4][32] ={
    /* SHA-256 sample vectors */
    {0x23, 0x09, 0x7d, 0x22, 0x34, 0x05, 0xd8, 0x22, 0x86, 0x42, 0xa4, 0x77, 0xbd, 0xa2, 0x55, 0xb3, 0x2a, 0xad, 0xbc, 0xe4, 0xbd, 0xa0, 0xb3, 0xf7, 0xe3, 0x6c, 0x9d, 0xa7},
    {0x75, 0x38, 0x8b, 0x16, 0x51, 0x27, 0x76, 0xcc, 0x5d, 0xba, 0x5d, 0xa1, 0xfd, 0x89, 0x01, 0x50, 0xb0, 0xc6, 0x45, 0x5c, 0xb4, 0xf5, 0x8b, 0x19, 0x52, 0x52, 0x25, 0x25},
    {0x7d, 0xe2, 0xf9, 0x3b, 0x0d, 0x0a, 0x1f, 0x5c, 0xaf, 0x83, 0x77, 0x39, 0xda, 0x74, 0x16, 0x7a, 0x03, 0xbd, 0x64, 0xb7, 0x93, 0x06, 0x7e, 0xbd, 0x40, 0x73, 0xd0, 0xdc},
    {0x20, 0x79, 0x46, 0x55, 0x98, 0x0c, 0x91, 0xd8, 0xbb, 0xb4, 0xc1, 0xea, 0x97, 0x61, 0x8a, 0x4b, 0xf0, 0x3f, 0x42, 0x58, 0x19, 0x48, 0xb2, 0xee, 0x4e, 0xe7, 0xad, 0x67}
};

static xmedia_u8 sha1_hmac_test_key[3][26] =
{
    { "This is xx There" },//16 bytes
    { "There is the key" },
    { "This is your dog" }
};

static xmedia_u8 sha1_hmac_test_sum[3][20] =
{
    { 0xac, 0xd4, 0x9a, 0xdd, 0x87, 0xba, 0x3b, 0xe2, 0x93, 0xdc, 0x39, 0xc1, 0x72, 0x06, 0xbf, 0xab, 0x8b, 0xde, 0x59, 0xe7 }, // one block
    { 0xFC, 0x66, 0xA4, 0xCB, 0x9E, 0xF4, 0xE6, 0x88, 0x19, 0xF3, 0xE9, 0x40, 0xAE, 0xF0, 0x7A, 0x59, 0x5B, 0x72, 0x7A, 0x0F }, // two blocks
    { 0x73, 0xD3, 0x86, 0xE3, 0x0E, 0x42, 0x1A, 0x27, 0xC7, 0x53, 0xFA, 0x45, 0xA4, 0x78, 0x31, 0x34, 0xC0, 0x2A, 0x71, 0x2B }  // 10000 'a'
};

static xmedia_u8 sha2_hmac_test_sum[3][32] =
{
    { 0xf1, 0x2b, 0xdb, 0x85, 0xbd, 0x31, 0xaa, 0x46,
      0x84, 0x3d, 0xca, 0x9b, 0xde, 0x9c, 0xbf, 0x63,
      0xba, 0x00, 0x55, 0x02, 0x60, 0xdb, 0xbf, 0xc4,
      0xad, 0x03, 0xc4, 0xdf, 0x98, 0x82, 0x2c, 0x57 }, // one block
    { 0xEF, 0x03, 0xC8, 0xB5, 0x2F, 0xFC, 0x02, 0x3B,
      0x7A, 0x99, 0x69, 0xF7, 0x0E, 0xF6, 0x03, 0x22,
      0x90, 0x2B, 0x47, 0x98, 0x1F, 0x9B, 0x8F, 0xAF,
      0xFE, 0x43, 0xE5, 0x61, 0x33, 0x8A, 0xE3, 0x44 }, // two blocks
    { 0x4B, 0x3B, 0xF5, 0xA2, 0x57, 0xB6, 0xF9, 0x10,
      0x35, 0x9E, 0xF4, 0x8D, 0x8A, 0x3B, 0xE1, 0x6E,
      0x0A, 0x09, 0x3A, 0x96, 0x53, 0x51, 0x72, 0xBD,
      0x6F, 0x82, 0x0D, 0xBC, 0xFB, 0x1D, 0x9B, 0x00 }  // 10000 'a'
};

static xmedia_u8 au8Buf[LONG_DATA_SIZE];

xmedia_s32 sha1(void)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_u8 hash[20];
    xmedia_u32 i = 0,j = 0;
    xmedia_handle handle[MAX_HASH_HANDLE];
    xmedia_cipher_hash_attr hash_attr;

    ret = xmedia_cipher_init();
    if (XMEDIA_SUCCESS != ret) {
        ret = XMEDIA_FAILURE;
        goto EXIT;
    }

    memset(hash, 0, 20);

    for (i = 0; i < MAX_HASH_HANDLE; i++) {
        memset(&hash_attr, 0, sizeof(xmedia_cipher_hash_attr));
        hash_attr.type = XMEDIA_CIPHER_HASH_TYPE_SHA1;

        ret = xmedia_cipher_hash_init(&hash_attr, &handle[i]);

        if (XMEDIA_SUCCESS != ret) {
            ret = XMEDIA_FAILURE;
            goto EXIT;
        }
     }

     for (i = 0; i < MAX_HASH_HANDLE; i++) {
         if (i == 3) {
            memset(au8Buf, 'a', LONG_DATA_SIZE);
            for (j=0; j < 1000000 / LONG_DATA_SIZE; j++) {
                ret = xmedia_cipher_hash_update(handle[i], au8Buf, LONG_DATA_SIZE);
                if (XMEDIA_SUCCESS != ret) {
                    ret = XMEDIA_FAILURE;
                    goto EXIT;
                }
            }
        } else {
            ret = xmedia_cipher_hash_update(handle[i], sha1_buf[i], sha1_buflen[i]);
            if (XMEDIA_SUCCESS != ret) {
                ret = XMEDIA_FAILURE;
                goto EXIT;
            }
        }
     }

     for (i = 0; i < MAX_HASH_HANDLE; i++) {
         ret = xmedia_cipher_hash_final(handle[i], hash);
         if (XMEDIA_SUCCESS != ret) {
             ret = XMEDIA_FAILURE;
             goto EXIT;
         }

         if (memcmp(hash, sha1_sum[i], 20) != 0) {
             ERR_CODE_CIPHER("sha1 run failed, sample %d!\n", i);
             print_buffer("Sha1 result:", hash, 20);
             print_buffer("golden data:", sha1_sum[i], 20);
             ret = XMEDIA_FAILURE;
             goto EXIT;
         }
     }

EXIT:

    TEST_RESULT_PRINT();

    xmedia_cipher_exit();

    return ret;
}

xmedia_s32 sha256(void)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_u8 hash[32];
    xmedia_u32 i = 0, j = 0;
    xmedia_handle handle[MAX_HASH_HANDLE];
    xmedia_cipher_hash_attr hash_attr;

    ret = xmedia_cipher_init();
    if (XMEDIA_SUCCESS != ret) {
        ret = XMEDIA_FAILURE;
        goto EXIT;
    }

    memset(hash, 0x0, 32);

    for (i = 0; i < MAX_HASH_HANDLE; i++) {
        memset(&hash_attr, 0, sizeof(xmedia_cipher_hash_attr));
        hash_attr.type = XMEDIA_CIPHER_HASH_TYPE_SHA256;

        ret = xmedia_cipher_hash_init(&hash_attr, &handle[i]);
        if (XMEDIA_SUCCESS != ret) {
            ret = XMEDIA_FAILURE;
            goto EXIT;
        }
    }

    for (i = 0; i < MAX_HASH_HANDLE; i++) {
        if (i == 3) {
            memset(au8Buf, 'a', LONG_DATA_SIZE);
            for (j=0; j < 1000000 / LONG_DATA_SIZE; j++) {
                ret = xmedia_cipher_hash_update(handle[i], au8Buf, LONG_DATA_SIZE);
                if (XMEDIA_SUCCESS != ret) {
                    ret = XMEDIA_FAILURE;
                    goto EXIT;
                }
            }
        } else {
            ret = xmedia_cipher_hash_update(handle[i], sha1_buf[i], sha1_buflen[i]);
            if (XMEDIA_SUCCESS != ret) {
                ret = XMEDIA_FAILURE;
                goto EXIT;
            }
        }
    }

    for (i = 0; i < MAX_HASH_HANDLE; i++) {
        ret = xmedia_cipher_hash_final(handle[i], hash);
        if (XMEDIA_SUCCESS != ret) {
            ret = XMEDIA_FAILURE;
            goto EXIT;
        }

        if (memcmp(hash, sha2_sum[i], 32) != 0) {
            ERR_CODE_CIPHER("sha256 run failed, sample %d!\n", i);
            print_buffer("Sha256 result:", hash, 32);
            print_buffer("golden data:", sha2_sum[i], 32);
            ret = XMEDIA_FAILURE;
            goto EXIT;
        }
    }

EXIT:

    TEST_RESULT_PRINT();
    xmedia_cipher_exit();

    return ret;
}

xmedia_s32 sha224(void)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_u8 hash[28];
    xmedia_u32 i = 0, j=0;
    xmedia_handle handle[MAX_HASH_HANDLE];
    xmedia_cipher_hash_attr hash_attr;

    ret = xmedia_cipher_init();
    if (XMEDIA_SUCCESS != ret) {
        ret = XMEDIA_FAILURE;
        goto EXIT;
    }

    memset(hash, 0, 28);

    for (i = 0; i < MAX_HASH_HANDLE; i++) {
        memset(&hash_attr, 0, sizeof(xmedia_cipher_hash_attr));
        hash_attr.type = XMEDIA_CIPHER_HASH_TYPE_SHA224;

        ret = xmedia_cipher_hash_init(&hash_attr, &handle[i]);
        if (XMEDIA_SUCCESS != ret) {
            ret = XMEDIA_FAILURE;
            goto EXIT;
        }
    }

    for (i = 0; i < MAX_HASH_HANDLE; i++) {
        if (i == 3) {
            memset(au8Buf, 'a', LONG_DATA_SIZE);
            for (j = 0; j < 1000000 / LONG_DATA_SIZE; j++) {
                ret = xmedia_cipher_hash_update(handle[i], au8Buf, LONG_DATA_SIZE);
                if (XMEDIA_SUCCESS != ret) {
                    ret = XMEDIA_FAILURE;
                    goto EXIT;
                }
            }
        } else {
            ret = xmedia_cipher_hash_update(handle[i], sha1_buf[i], sha1_buflen[i]);
            if (XMEDIA_SUCCESS != ret) {
                ret = XMEDIA_FAILURE;
                goto EXIT;
            }
        }
    }

    for (i = 0; i < MAX_HASH_HANDLE; i++) {
        ret = xmedia_cipher_hash_final(handle[i], hash);
        if (XMEDIA_SUCCESS != ret) {
            ret = XMEDIA_FAILURE;
            goto EXIT;
        }

        if (memcmp(hash, sha224_sum[i], 28) != 0) {
            ERR_CODE_CIPHER("sha224 run failed, sample %d!\n", i);
            print_buffer("Sha224 result:", hash, 28);
            print_buffer("golden data:", sha224_sum[i], 28);
            ret = XMEDIA_FAILURE;
            goto EXIT;
        }
    }

EXIT:

    TEST_RESULT_PRINT();
    xmedia_cipher_exit();

    return ret;
}

xmedia_s32 hmac_sha1(void)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_u8 hash[20];
    xmedia_u32 i = 0;
    xmedia_handle handle[MAX_HMAC_HANDLE];
    xmedia_cipher_hash_attr hash_attr;

    ret = xmedia_cipher_init();
    if (XMEDIA_SUCCESS != ret) {
        ret = XMEDIA_FAILURE;
        goto EXIT;
    }

    memset(&hash_attr, 0, sizeof(xmedia_cipher_hash_attr));
    hash_attr.hmac_key = malloc(16);
    hash_attr.hmac_key_len = 16;
    if (XMEDIA_NULL == hash_attr.hmac_key) {
        ret = XMEDIA_FAILURE;
        goto EXIT;
    }

    memset(hash, 0, 20);

    for (i = 0; i < MAX_HMAC_HANDLE; i++) {
        hash_attr.type = XMEDIA_CIPHER_HASH_TYPE_HMAC_SHA1;
        memcpy(hash_attr.hmac_key, sha1_hmac_test_key[i], 16);

        ret = xmedia_cipher_hash_init(&hash_attr, &handle[i]);
        if (XMEDIA_SUCCESS != ret) {
            ERR_CODE_CIPHER("hash init failed!\n");
            ret = XMEDIA_FAILURE;
            goto EXIT;
        }
    }

    for (i = 0; i < MAX_HMAC_HANDLE; i++) {
        if (i == 2) {
            memset(au8Buf, 'a', 10000);
            ret = xmedia_cipher_hash_update(handle[i], au8Buf, 10000);
        } else {
            ret = xmedia_cipher_hash_update(handle[i], sha1_buf[i], sha1_buflen[i]);
        }

        if (XMEDIA_SUCCESS != ret) {
            ERR_CODE_CIPHER("hash update failed!\n");
            ret = XMEDIA_FAILURE;
            goto EXIT;
        }
    }

    for (i = 0; i < MAX_HMAC_HANDLE; i++) {
        ret = xmedia_cipher_hash_final(handle[i], hash);
        if (XMEDIA_SUCCESS != ret) {
            ERR_CODE_CIPHER("hash final failed!\n");
            ret = XMEDIA_FAILURE;
            goto EXIT;
        }
        print_buffer("hmac-Sha1 result:", hash, 20);
        if (memcmp(hash, sha1_hmac_test_sum[i], 20) != 0) {
            ERR_CODE_CIPHER("\033[0;31m" "hmac_sha1 run failed, sample %d!\n" "\033[0m", i);
            print_buffer("HMAC1 result:", hash, 20);
            print_buffer("golden data:", sha1_hmac_test_sum[i], 20);
            ret = XMEDIA_FAILURE;
            goto EXIT;
        }
    }

EXIT:

    TEST_RESULT_PRINT();

    free(hash_attr.hmac_key);
    hash_attr.hmac_key = XMEDIA_NULL;
    xmedia_cipher_exit();

    return ret;
}

xmedia_s32 hmac_sha256(void)
{
    xmedia_s32 ret = XMEDIA_FAILURE;
    xmedia_u8 hash[32];
    xmedia_u32 i = 0;
    xmedia_handle handle[MAX_HMAC_HANDLE];
    xmedia_cipher_hash_attr hash_attr;

    ret = xmedia_cipher_init();
    if (XMEDIA_SUCCESS != ret) {
        ret = XMEDIA_FAILURE;
        goto EXIT;
    }

    memset(hash, 0, 32);

    memset(&hash_attr, 0, sizeof(xmedia_cipher_hash_attr));
    hash_attr.hmac_key = malloc(16);
    hash_attr.hmac_key_len = 16;
    if (XMEDIA_NULL == hash_attr.hmac_key) {
        ret = XMEDIA_FAILURE;
        goto EXIT;
    }

    for (i = 0; i < MAX_HMAC_HANDLE; i++) {
        hash_attr.type = XMEDIA_CIPHER_HASH_TYPE_HMAC_SHA256;
        memcpy(hash_attr.hmac_key, sha1_hmac_test_key[i], 16);

        ret = xmedia_cipher_hash_init(&hash_attr, &handle[i]);
        if (XMEDIA_SUCCESS != ret) {
            goto EXIT;
        }
    }

    for (i = 0; i < MAX_HMAC_HANDLE; i++) {
        if (i == 2) {
            memset(au8Buf, 'a', 10000);
            ret = xmedia_cipher_hash_update(handle[i], au8Buf, 10000);
        } else {
            ret = xmedia_cipher_hash_update(handle[i], sha1_buf[i], sha1_buflen[i]);
        }
        if (XMEDIA_SUCCESS != ret) {
            goto EXIT;
        }
    }

    for(i = 0; i < MAX_HMAC_HANDLE; i++) {
        ret = xmedia_cipher_hash_final(handle[i], hash);
        if (XMEDIA_SUCCESS != ret) {
            goto EXIT;
        }

        print_buffer("hmac-Sha256 result:", hash, 32);
        if (memcmp(hash, sha2_hmac_test_sum[i], 32) != 0) {
            ERR_CODE_CIPHER("hmac_sha256 run failed, sample %d!\n", i);
            print_buffer("HMAC256 result:", hash, 32);
            print_buffer("golden data:", sha2_hmac_test_sum[i], 20);
            ret = XMEDIA_FAILURE;
            goto EXIT;
        }
    }

EXIT:

    TEST_RESULT_PRINT();

    free(hash_attr.hmac_key);
    hash_attr.hmac_key = XMEDIA_NULL;
    xmedia_cipher_exit();

    return ret;
}

int sample_hash(int argc, char* argv[])
{
    xmedia_s32 ret = XMEDIA_SUCCESS;

    ret = sha1();
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("sha1 run failed!\n");
        return ret;
    }

    ret = sha256();
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("sha256 run failed!\n");
        return ret;
    }

    ret = hmac_sha1();
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("hmac_sha1 run failed!\n");
        return ret;
    }

    ret = hmac_sha256();
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("hmac_sha256 run failed!\n");
        return ret;
    }

    ret = sha224();
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("sha224 run failed!\n");
        return ret;
    }

    return XMEDIA_SUCCESS;
}

