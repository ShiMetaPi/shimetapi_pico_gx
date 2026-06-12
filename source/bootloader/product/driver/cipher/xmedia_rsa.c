/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include "xmedia_cipher_osal.h"
//#include "safety.h"

/************************************************************/
static xmedia_void *g_rsa_reg_base  = XMEDIA_NULL;
static xmedia_void *g_rsa_reg_crg   = XMEDIA_NULL;
static xmedia_u32 rsa_init_flag = 0;

/************************************************************/
#define RSA_REG_BASE_RSA                          g_rsa_reg_base
#define SEC_RSA_BUSY_REG                          (RSA_REG_BASE_RSA + 0x010C)
#define SEC_RSA_MOD_REG                           (RSA_REG_BASE_RSA + 0x0110)
#define SEC_RSA_WSEC_REG                          (RSA_REG_BASE_RSA + 0x0104)
#define SEC_RSA_WDAT_REG                          (RSA_REG_BASE_RSA + 0x0108)
#define SEC_RSA_RRSLT_REG                         (RSA_REG_BASE_RSA + 0x0120)
#define SEC_RSA_START_REG                         (RSA_REG_BASE_RSA + 0x0114)
#define SEC_RSA_ERROR_REG                         (RSA_REG_BASE_RSA + 0x0124)

#define RSA_DATA_CLR_KEY           (1 << 2)
#define RSA_DATA_CLR_INPUT         (2 << 2)
#define RSA_DATA_CLR_OUTPUT        (4 << 2)
#define RSA_MOD_SEL                (3 << 0)
#define RSA_MOD_SEL_OPT            (0 << 0)
#define RSA_MOD_SEL_RAM_CLAER      (2 << 0)
#define RSA_BUSY                   (1 << 0)
#define RSA_START                  (1 << 0)

#define RSA_MAX_KEY_LEN         512
#define CRC16_POLYNOMIAL        0x1021

#define RSA_PKCS1_PSS_MGF1_SHA256_MALLOC_SIZE   0x44
#define RSA_PKCS1_PSS_PADDING_CHECK_MALLOC_SIZE 0x48

typedef enum {
    RSA_DATA_TYPE_CONTEXT,
    RSA_DATA_TYPE_MODULE,
    RSA_DATA_TYPE_KEY,
} rsa_data_type_e;

typedef enum {
    RSA_KEY_WIDTH_1K   = 0x00,
    RSA_KEY_WIDTH_2K   = 0x01,
    RSA_KEY_WIDTH_4K   = 0x02,
    RSA_KEY_WIDTH_3K   = 0x03,
    RSA_KEY_WIDTH_BUTT = 0xff,
} rsa_key_width_e;

typedef struct {
    xmedia_u32 hlen;
    xmedia_u32 klen;
    xmedia_u32 em_bit;
    xmedia_u8 *in_data;
    xmedia_u32 in_len;
} rsa_padding_s;

typedef struct {
    xmedia_u8 *masked_db;
    xmedia_u8 *masked_seed;
    xmedia_u8 salt[RSA_MAX_KEY_LEN];
    xmedia_u32 msb_bits;
    xmedia_u32 slen;
    xmedia_u32 key_len;
} rsa_pkcs1_pss_s;

typedef struct {
    xmedia_u8 *input_data;
    xmedia_u8 *output_data;
    xmedia_u32 data_len;
    xmedia_u8 *rsa_n;
    xmedia_u8 *rsa_k;
    xmedia_u16 rsa_n_len;
    xmedia_u16 rsa_k_len;
} rsa_data_s;

static xmedia_void hal_rsa_start(xmedia_void)
{
    hal_cipher_write_reg(SEC_RSA_START_REG, 0x01);
}

static xmedia_s32 hal_rsa_wait_free(xmedia_void)
{
    xmedia_u32 value;
    xmedia_u32 try_count = 0;

    do {
        value = hal_cipher_read_reg(SEC_RSA_BUSY_REG);
        if ((value & RSA_BUSY) == 0)
            return XMEDIA_SUCCESS;
        try_count++;
        cipher_usleep(10);
    } while (try_count < RSA_TIMEOUT_CNT);

    return XMEDIA_FAILURE;
}

static xmedia_void hal_rsa_clear_ram(xmedia_void)
{
    xmedia_u32 value;

    value = hal_cipher_read_reg(SEC_RSA_MOD_REG);
    value &= 0x60;
    value |= RSA_DATA_CLR_INPUT | RSA_DATA_CLR_OUTPUT | RSA_DATA_CLR_KEY | RSA_MOD_SEL_RAM_CLAER;
    hal_cipher_write_reg(SEC_RSA_MOD_REG, value);
}

static xmedia_void hal_rsa_config_mode(rsa_key_width_e ken_width)
{
    xmedia_u32 value;

    value = ((xmedia_u32)ken_width << 5) | RSA_MOD_SEL_OPT; /* 5 left shift */
    hal_cipher_write_reg(SEC_RSA_MOD_REG, value);
}

static xmedia_void hal_rsa_write_data(rsa_data_type_e data_type,
                                  xmedia_u8 *data,
                                  xmedia_u32 data_len,
                                  xmedia_u32 length,
                                  xmedia_u32 random[2]) /* 2 random size */
{
    xmedia_u32 *reg = XMEDIA_NULL;
    xmedia_u8 *pos = XMEDIA_NULL;
    xmedia_u32 i, value;
    xmedia_bool id = 0;

    if (data_type == RSA_DATA_TYPE_CONTEXT)
        reg = SEC_RSA_WDAT_REG;
    else
        reg = SEC_RSA_WSEC_REG;

    pos = data;
    for (i = 0; i < length; i += 4) { /* 4 groups */
        value  = (xmedia_u32)pos[0];
        value |= ((xmedia_u32)pos[1]) << 8;  /* 1 index, 8  left shift */
        value |= ((xmedia_u32)pos[2]) << 16; /* 2 index, 16 left shift */
        value |= ((xmedia_u32)pos[3]) << 24; /* 3 index, 24 left shift */
        if (data_type != RSA_DATA_TYPE_CONTEXT)
            value ^= random[id];

        hal_cipher_write_reg(reg, value);
        pos += 4; /* 4 groups */
        id = (xmedia_u32)id ^ 0x01;
    }
}

static xmedia_void hal_rsa_read_data(xmedia_u8 *data, xmedia_u32 data_len, xmedia_u32 klen)
{
    xmedia_u32 value;
    xmedia_u8 *pos = XMEDIA_NULL;
    xmedia_u32 i;

    pos = data;
    for (i = 0; i < klen; i += 4) { /* 4 groups */
        value = hal_cipher_read_reg(SEC_RSA_RRSLT_REG);
        pos[0] = (xmedia_u8)(value & 0xFF);
        pos[1] = (xmedia_u8)((value >> 8) & 0xFF);  /* 1 index, 8  right shift */
        pos[2] = (xmedia_u8)((value >> 16) & 0xFF); /* 2 index, 16 right shift */
        pos[3] = (xmedia_u8)((value >> 24) & 0xFF); /* 3 index, 24 right shift */
        pos += 4; /* 4 groups */
    }
}

static xmedia_u32 hal_rsa_get_error_code(xmedia_void)
{
    xmedia_u32 value;

    value = hal_cipher_read_reg(SEC_RSA_ERROR_REG);

    return value;
}

static xmedia_void hal_rsa_enable(xmedia_void)
{
    xmedia_u32 value;

    value = hal_cipher_read_reg(g_rsa_reg_crg);
    hal_set_bit(value, 5); /* 5bit clock opened */
    hal_cipher_write_reg(g_rsa_reg_crg, value);
    cipher_usleep(10);

    hal_clear_bit(value, 4); /* 4bit cancel reset */
    hal_cipher_write_reg(g_rsa_reg_crg, value);
    cipher_usleep(10);

    //enable_rsa_safety();
}

static xmedia_void hal_rsa_disable(xmedia_void)
{
    xmedia_u32 value;

    value = hal_cipher_read_reg(g_rsa_reg_crg);

    hal_set_bit(value, 4); /* 4bit reset */
    hal_cipher_write_reg(g_rsa_reg_crg, value);
    cipher_usleep(10);

    hal_clear_bit(value, 5); /* 5bit clock closed */
    hal_cipher_write_reg(g_rsa_reg_crg, value);
    cipher_usleep(10);
}

static xmedia_u32 rsa_get_bit_num(xmedia_u8 *big_num, xmedia_u32 num_len)
{
    static const xmedia_s8 bits[16] = {0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4}; /* 16 bits size */
    xmedia_u32 i;

    for (i = 0; i < num_len; i++) {
        xmedia_u32 num;
        num = bits[(big_num[i] & 0xF0) >> 4]; /* 4 right shift */

        if (num > 0)
            return (num_len - i - 1) * 8 + num + 4; /* 8, 4 */

        num = bits[big_num[i] & 0xF];
        if (num > 0)
            return (num_len - i - 1) * 8 + num; /* 8 */
    }
    return 0;
}

static xmedia_s32 rsa_pkcs1_mgf1(xmedia_u8 *seed, xmedia_u32 seed_len, xmedia_u8 *mask, xmedia_u32 mask_len)
{
    xmedia_s32 ret = XMEDIA_FAILURE;
    xmedia_u32 i, out_len;
    xmedia_u8 *ptr_cnt = XMEDIA_NULL;
    xmedia_u8 *ptr_md = XMEDIA_NULL;
    xmedia_u8 *ptr_seed = XMEDIA_NULL;
    xmedia_u8 *ptr_malloc = XMEDIA_NULL;

    ptr_malloc = (xmedia_u8 *)cipher_malloc(RSA_PKCS1_PSS_MGF1_SHA256_MALLOC_SIZE);
    if (ptr_malloc == XMEDIA_NULL) {
        cipher_debug_log(CIPHER_ERR_RSA_MALLOC);
        return XMEDIA_FAILURE;
    }
    crypto_memset(ptr_malloc, RSA_PKCS1_PSS_MGF1_SHA256_MALLOC_SIZE, 0, RSA_PKCS1_PSS_MGF1_SHA256_MALLOC_SIZE);

    ptr_cnt = ptr_malloc;
    ptr_md = ptr_malloc + 4;
    ptr_seed = (ptr_malloc + 4 + SHA256_RESULT_SIZE);

    crypto_memcpy(ptr_seed, (RSA_PKCS1_PSS_MGF1_SHA256_MALLOC_SIZE - SHA256_RESULT_SIZE - 4), seed, seed_len);

    /* PKCS#1 V2.1*/
    for (i = 0, out_len = 0; out_len < mask_len; i++) {
        xmedia_u32 j, md_len;

        //sha256 fixed
        md_len = 32;

        ptr_cnt[0] = (xmedia_u8)((i >> 24) & 0xFF); /* 0 ptr_cnt index, 24 right shift */
        ptr_cnt[1] = (xmedia_u8)((i >> 16) & 0xFF); /* 1 ptr_cnt index, 16 right shift */
        ptr_cnt[2] = (xmedia_u8)((i >>  8) & 0xFF); /* 2 ptr_cnt index, 8  right shift */
        ptr_cnt[3] = (xmedia_u8)(i & 0xFF);         /* 3 ptr_cnt index */

        ret = xmedia_cipher_sha256_init();
        ret |= xmedia_cipher_sha256_update(ptr_seed, seed_len);
        ret |= xmedia_cipher_sha256_update(ptr_cnt, 4);    /* 4 ptr_cnt size */
        ret |= xmedia_cipher_sha256_final(ptr_md);
        if(ret != XMEDIA_SUCCESS)
        {
            cipher_debug_log(CIPHER_ERR_RSA_SHA256);    /*sha256 failed*/
            goto free_ptr;
        }

        for (j = 0; (j < md_len) && (out_len < mask_len); j++)
            mask[out_len++] ^= ptr_md[j];
    }

free_ptr:
    cipher_free(ptr_malloc);
    ptr_malloc = XMEDIA_NULL;
    return ret;
}

static xmedia_s32 rsa_padding_check_pkcs1_pss_hash(rsa_padding_s *pad, xmedia_u8 *mhash, rsa_pkcs1_pss_s *pss)
{
    xmedia_s32 ret;
    xmedia_u32 mlen;
    xmedia_u8 *ptr_m = XMEDIA_NULL;
    xmedia_u8 arr_h[SHA256_RESULT_SIZE];

    /*8+32+32*/
    mlen = pss->slen + pad->hlen + 8; /* 8 */
    ptr_m = (xmedia_u8 *)cipher_malloc(RSA_PKCS1_PSS_PADDING_CHECK_MALLOC_SIZE);
    if(ptr_m == XMEDIA_NULL)
    {
        return XMEDIA_FAILURE;
    }

    crypto_memset(arr_h, SHA256_RESULT_SIZE, 0, SHA256_RESULT_SIZE);
    crypto_memset(ptr_m, RSA_PKCS1_PSS_PADDING_CHECK_MALLOC_SIZE, 0, mlen);

    /* M' = (0x)00 00 00 00 00 00 00 00 || mHash || salt */
    crypto_memset(ptr_m, mlen, 0x00, 8); /* 8, 0 counts */
    crypto_memcpy(&ptr_m[8], mlen - 8, mhash, pad->hlen); /* 8, 0 counts */
    crypto_memcpy(&ptr_m[8 + pad->hlen], mlen - 8 - pad->hlen, pss->salt, pss->slen); /* 8, 0 counts */

    ret = xmedia_cipher_sha256_init();
    ret |= xmedia_cipher_sha256_update(ptr_m, mlen);
    ret |= xmedia_cipher_sha256_final(arr_h);

    cipher_free(ptr_m); /* Must free ptr_m befort return */
    ptr_m = XMEDIA_NULL;
    if(ret != XMEDIA_SUCCESS)
    {
        cipher_debug_log(CIPHER_ERR_RSA_SHA256);    /*sha256 failed*/
        return XMEDIA_FAILURE;
    }

    if(crypto_memcmp(arr_h, pss->masked_seed, pad->hlen) != XMEDIA_SUCCESS)
    {
        cipher_debug_log(CIPHER_ERR_RSA_VERIFY_HASH_CMP);    /*masked_seed cmp failed*/
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 rsa_padding_check_pkcs1_pss(rsa_padding_s *pad, xmedia_u8 *mhash)
{
    xmedia_u32 ret;
    xmedia_u32 index, tmp_len;
    rsa_pkcs1_pss_s pss;

    crypto_memset(&pss, sizeof(rsa_pkcs1_pss_s), 0, sizeof(rsa_pkcs1_pss_s));
    pss.slen = pad->hlen;
    pss.key_len = (pad->em_bit + 7) / 8; /* 7, 8 */
    pss.msb_bits = (pad->em_bit - 1) & 0x07;

    if (pss.key_len < (pad->hlen + pss.slen + 2)) { /* 2 */
        cipher_debug_log(CIPHER_ERR_RSA_VERIFY_M_LEN);    /*message too long*/
        return XMEDIA_FAILURE;
    }

    if (pad->in_data[0] & (0xFF << pss.msb_bits)) {
        cipher_debug_log(CIPHER_ERR_RSA_VERIFY_EM0);    /*inconsistent, EM[0] invalid*/
        return XMEDIA_FAILURE;
    }

    if (pss.msb_bits == 0) {
        pad->in_data++;
        pss.key_len--;
    }

    pss.masked_db = pad->in_data;
    pss.masked_seed = pad->in_data + pss.key_len - pad->hlen - 1;

    if (pad->in_data[pss.key_len - 1] != 0xBC) {
        cipher_debug_log(CIPHER_ERR_RSA_VERIFY_BC);    /*inconsistent, EM[key_len - 1] != 0xBC*/
        return XMEDIA_FAILURE;
    }

    /* formula: maskedDB = DB xor dbMask, DB = PS || 0x01 || salt */
    ret = rsa_pkcs1_mgf1(pss.masked_seed, pad->hlen, pss.masked_db, pss.key_len - pad->hlen - 1);
    if(ret != XMEDIA_SUCCESS)
    {
        cipher_debug_log(CIPHER_ERR_RSA_MGF1);    /*rsa_pkcs1_mgf1 failed*/
        return XMEDIA_FAILURE;
    }

    if (pss.msb_bits)
        pss.masked_db[0] &= 0xFF >> (8 - pss.msb_bits); /* 8 */

    tmp_len = pss.key_len - pss.slen - pad->hlen - 2; /* 2 */
    if (tmp_len >= RSA_MAX_KEY_LEN - 1) { /* -1 is for index++, avoid masked_db overflow */
        cipher_debug_log(CIPHER_ERR_RSA_VERIFY_MASKEDDB_LEN);    /*operate masked_db maybe overflow*/
        return XMEDIA_FAILURE;
    }

    for (index = 0; index < tmp_len; index++) {
        if (pss.masked_db[index] != 0x00) {
            break;
        }
    }

    pss.slen = pss.key_len - pad->hlen - index - 2;

    if(pss.masked_db[index] != 0x1)
    {
        cipher_debug_log(CIPHER_ERR_RSA_VERIFY_DB_1);    /*check masked_db failed*/
        return XMEDIA_FAILURE;
    }

    index++;
    crypto_memcpy(pss.salt, sizeof(pss.salt), &pss.masked_db[index], pss.slen);

    return rsa_padding_check_pkcs1_pss_hash(pad, mhash, &pss);
}

static xmedia_s32 drv_cipher_calc_rsa(rsa_data_s *rsa_data);
static xmedia_s32 rsa_public(const cipher_rsa_pub_key *pub_key, const xmedia_u8 *input, xmedia_u8 *output)
{
    rsa_data_s rsa_data;

    rsa_data.rsa_n = pub_key->n;
    rsa_data.rsa_k = pub_key->e;
    rsa_data.rsa_n_len = pub_key->n_len;
    rsa_data.rsa_k_len = pub_key->e_len;
    rsa_data.input_data = (xmedia_u8 *)input;
    rsa_data.output_data = output;
    rsa_data.data_len = pub_key->n_len;

    return drv_cipher_calc_rsa(&rsa_data);
}

static xmedia_s32 rsa_verify_pad_init(rsa_padding_s *pad, const cipher_rsa_verify *rsa_verify,
    cipher_verify_data *verify_data, xmedia_u8 *arr_em)
{
    /*fix here: only support sha256*/
    pad->hlen = SHA256_RESULT_SIZE; /* 32 pad hlen */
    pad->klen = rsa_verify->pub_key.n_len;
    pad->in_data = arr_em;
    pad->in_len = verify_data->sign_len;
    if(verify_data->sign_len != pad->klen)
    {
        return XMEDIA_FAILURE;
    }

    return rsa_public(&rsa_verify->pub_key, verify_data->sign, pad->in_data);
}

static xmedia_s32 drv_rsa_init(xmedia_void)
{
    g_rsa_reg_crg = cipher_ioremap_nocache(CIPHER_RSA_CRG_ADDR_PHY, 16); /* 16 */
    g_rsa_reg_base = cipher_ioremap_nocache(CIPHER_RSA_REG_BASE_ADDR_PHY, 0x1000);

    return XMEDIA_SUCCESS;
}

static xmedia_void drv_rsa_deinit(xmedia_void)
{
    if (g_rsa_reg_base != XMEDIA_NULL) {
        cipher_iounmap(g_rsa_reg_base);
        g_rsa_reg_base = XMEDIA_NULL;
    }

    if (g_rsa_reg_crg != XMEDIA_NULL) {
        cipher_iounmap(g_rsa_reg_crg);
        g_rsa_reg_crg = XMEDIA_NULL;
    }
}

static xmedia_s32 drv_rsa_wait_done(xmedia_void)
{
    return hal_rsa_wait_free();
}

static xmedia_s32 drv_cipher_check_rsa_data(xmedia_u8 *rsa_n, xmedia_u8 *rsa_e, xmedia_u8 *rsa_mc, xmedia_u32 length)
{
    xmedia_u32 i;

    /* formula: rsa_mc > 0 */
    for (i = 0; i < length; i++) {
        if (rsa_mc[i] > 0)
            break;
    }
    if (i >= length) {
        cipher_debug_log(CIPHER_ERR_RSA_M_ZERO);   /*RSA M/C is zero, error*/
        return XMEDIA_FAILURE;
    }

    /* formula: rsa_mc < rsa_n */
    for (i = 0; i < length; i++) {
        if(rsa_mc[i] < rsa_n[i])
            break;

        if(rsa_mc[i] == rsa_n[i])
            continue;

        if (rsa_mc[i] > rsa_n[i])
        {
            cipher_debug_log(CIPHER_ERR_RSA_M_LARGER_N);    /*RSA M/C is larger than rsa_n, error!*/
            return XMEDIA_FAILURE;
        }
    }
    if (i >= length) {
        cipher_debug_log(CIPHER_ERR_RSA_M_LARGER_N);    /*RSA M/C is larger than rsa_n, error!*/
        return XMEDIA_FAILURE;
    }

    /* formula: rsa_e > 1 */
    for (i = 0; i < length; i++) {
        if (rsa_e[i] > 0)
            break;
    }

    if ((i == length -1) && (rsa_e[i] == 1)) {
        cipher_debug_log(CIPHER_ERR_RSA_E_EQUAL_1);    /*RSA D/rsa_e is 1, error!*/
        return XMEDIA_FAILURE;
    }

    if (i >= length) {
        cipher_debug_log(CIPHER_ERR_RSA_E_EQUAL_0);    /*RSA D/rsa_e is zero, error!*/
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 drv_cipher_clear_rsa_ram(xmedia_void)
{
    if (hal_rsa_wait_free() != XMEDIA_SUCCESS) {
        cipher_debug_log(CIPHER_ERR_RSA_BUSY_TIMEOUT);    /*RSA is busy and timeout,error!*/
        return XMEDIA_FAILURE;
    }

    hal_rsa_clear_ram();
    hal_rsa_start();

    if (drv_rsa_wait_done() != XMEDIA_SUCCESS) {
        cipher_debug_log(CIPHER_ERR_RSA_BUSY_TIMEOUT);    /*RSA is busy and timeout,error!*/
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_void drv_rsa_rand_mask(rsa_data_s *rsa_data,
                                 xmedia_u32 key_len,
                                 xmedia_u32 *random)
{
    return;
}

static void drv_rsa_cipher_klad(rsa_data_s *rsa_data, xmedia_u32 key_len, xmedia_u32 *random)
{
    hal_rsa_write_data(RSA_DATA_TYPE_KEY, rsa_data->rsa_k,
        rsa_data->rsa_n_len, key_len, random);

    return;
}

static xmedia_s32 drv_rsa_key_info(rsa_data_s *rsa_data,
                               xmedia_u32 *key_len,
                               rsa_key_width_e *key_width)
{
    /* Only support the key width of 1024, 2048 and 4096 */
    if (rsa_data->rsa_n_len <= 128) { /* key n size 128 */
        *key_len = 128; /* key n size 128 */
        *key_width = RSA_KEY_WIDTH_1K;
    } else if (rsa_data->rsa_n_len <= 256) { /* key n size 256 */
        *key_len = 256; /* key n size 256 */
        *key_width = RSA_KEY_WIDTH_2K;
    } else if (rsa_data->rsa_n_len <= 384) { /* key n size 384 */
        *key_len = 384; /* key n size 384 */
        *key_width = RSA_KEY_WIDTH_3K;
    } else if (rsa_data->rsa_n_len <= 512) { /* key n size 512 */
        *key_len = 512; /* key n size 512 */
        *key_width = RSA_KEY_WIDTH_4K;
    } else {
        cipher_debug_log(CIPHER_ERR_RSA_KEY_LEN);
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

static xmedia_s32 drv_cipher_calc_rsa_ex(rsa_data_s *rsa_data,
                                     xmedia_u32 key_len,
                                     rsa_key_width_e key_width)
{
    xmedia_s32 ret = XMEDIA_FAILURE;
    xmedia_u32 err_code = 0;
    xmedia_u64 random = 0;

    ret = drv_cipher_check_rsa_data(rsa_data->rsa_n, rsa_data->rsa_k, rsa_data->input_data, key_len);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    hal_rsa_enable();

    ret = hal_rsa_wait_free();
    if (ret != XMEDIA_SUCCESS) {
        cipher_debug_log(CIPHER_ERR_RSA_BUSY_TIMEOUT);    /*RSA is busy!*/
        goto exit;
    }

    /* Config Mode */
    hal_rsa_config_mode(key_width);

    //V500 not support,reserved!
    drv_rsa_rand_mask(rsa_data, key_len, (xmedia_u32 *)&random);

    /* Write rsa_n, rsa_e, rsa_m */
    hal_rsa_write_data(RSA_DATA_TYPE_MODULE,
        rsa_data->rsa_n, rsa_data->rsa_n_len, key_len, (xmedia_u32 *)&random);

    /* V500 not support rand mask,random reserved! */
    drv_rsa_cipher_klad(rsa_data, key_len, (xmedia_u32 *)&random);

    hal_rsa_write_data(RSA_DATA_TYPE_CONTEXT,
        rsa_data->input_data, rsa_data->rsa_n_len, key_len, (xmedia_u32 *)&random);

    /* Sart */
    hal_rsa_start();

    ret = drv_rsa_wait_done();
    if (ret != XMEDIA_SUCCESS) {
        cipher_debug_log(CIPHER_ERR_RSA_BUSY_TIMEOUT);    /*RSA is busy and timeout,error!*/
        goto exit;
    }

    /* Get result */
    hal_rsa_read_data(rsa_data->output_data, rsa_data->rsa_n_len, key_len);

    err_code = hal_rsa_get_error_code();
    if (err_code == 0) {
        ret = XMEDIA_SUCCESS;
    } else {
        cipher_debug_log(CIPHER_ERR_RSA_ERRORCODE);    /*RSA is err!*/
        ret = XMEDIA_FAILURE;
    }

exit:
    (void)drv_cipher_clear_rsa_ram();
    hal_rsa_disable();

    return ret;
}

static xmedia_s32 drv_cipher_calc_rsa(rsa_data_s *rsa_data)
{
    xmedia_s32 ret;
    xmedia_u32 key_len = 0;
    rsa_data_s cipher_rsa_data;
    rsa_key_width_e key_width = RSA_KEY_WIDTH_BUTT;

    ret = drv_rsa_key_info(rsa_data, &key_len, &key_width);
    if (ret != XMEDIA_SUCCESS)
        return ret;

    crypto_memset(&cipher_rsa_data, sizeof(rsa_data_s), 0, sizeof(rsa_data_s));
    cipher_rsa_data.rsa_n = rsa_data->rsa_n;
    cipher_rsa_data.rsa_k = rsa_data->rsa_k;
    cipher_rsa_data.rsa_n_len = key_len;
    cipher_rsa_data.rsa_k_len = key_len;
    cipher_rsa_data.input_data = rsa_data->input_data;
    cipher_rsa_data.data_len = key_len;
    cipher_rsa_data.output_data = rsa_data->output_data;

    ret = drv_cipher_calc_rsa_ex(&cipher_rsa_data, key_len, key_width);
    return ret;
}

xmedia_s32 xmedia_cipher_rsa_deinit(xmedia_void)
{
    if (rsa_init_flag == 1) {
        drv_rsa_deinit();
        rsa_init_flag = 0;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 xmedia_cipher_rsa4096_psssha256_verify(const cipher_rsa_verify *rsa_verify, cipher_verify_data *verify_data)
{
    xmedia_s32 ret;
    rsa_padding_s pad;
    xmedia_u8 arr_em[RSA_MAX_KEY_LEN];

    if (rsa_init_flag == 0) {
        if (drv_rsa_init() != XMEDIA_SUCCESS) {
            return XMEDIA_FAILURE;
        }

        rsa_init_flag = 1;
    }

    crypto_memset(arr_em, sizeof(arr_em), 0, sizeof(arr_em));
    crypto_memset(&pad, sizeof(rsa_padding_s), 0, sizeof(rsa_padding_s));
    ret = rsa_verify_pad_init(&pad, rsa_verify, verify_data, arr_em);
    if (ret != XMEDIA_SUCCESS) {
        cipher_debug_log(CIPHER_ERR_RSA_DEC);
        return XMEDIA_FAILURE;
    }

    pad.em_bit = rsa_get_bit_num(rsa_verify->pub_key.n, pad.klen);
    ret = rsa_padding_check_pkcs1_pss(&pad, verify_data->hash_data);
    if (ret != XMEDIA_SUCCESS) {
        cipher_debug_log(CIPHER_ERR_RSA_CHECK_PADDING);
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}
