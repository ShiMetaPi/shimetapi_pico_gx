/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include "xmedia_cipher_osal.h"
#include "xmedia_cipher_reg.h"
//#include "safety.h"

/* spacc symc int entry struct which is defined by hardware, you can't change it */
struct spacc_symc_in_entry_t {
    unsigned int spacc_cmd: 2;
    unsigned int rev1: 6;
    unsigned int sym_ctrl: 7;
    unsigned int rev2: 1;
    unsigned int gcm_iv_len: 4;
    unsigned int rev3: 12;
    unsigned int sym_start_addr_high;
    unsigned int sym_start_addr;
    unsigned int sym_alg_length;
    unsigned int symc_iv[4]; /* 4 iv len */
};

/* spacc digest in entry struct which is defined by hardware, you can't change it */
struct spacc_digest_in_entry_t {
    unsigned int spacc_cmd: 2;
    unsigned int rev1: 6;
    unsigned int hash_ctrl: 6;
    unsigned int rev2: 18;
    unsigned int hash_start_addr;
    unsigned int hash_alg_length;
    unsigned int hash_start_addr_high;
};

/* spacc symc out entry struct which is defined by hardware, you can't change it */
struct spacc_symc_out_entry_t {
    unsigned int rev1: 8;
    unsigned int aes_ctrl: 4;
    unsigned int rev2: 20;
    unsigned int sym_start_addr;
    unsigned int sym_alg_length;
    unsigned int hash_rslt_start_addr;
    unsigned int tag[4]; /* 4 tag len */
};

struct spacc_digest_context {
    digest_alg_en  digest_alg;
    digest_mode_en digest_mode;
    unsigned int digest_len;
    unsigned int digest_blen;

    struct spacc_digest_in_entry_t* entry_digest_in;
    unsigned int entry_digest_in_depth;
    unsigned int digest_cur_in_nodes;
};

struct spacc_symc_context {
    symc_alg_en  symc_alg;
    symc_mode_en symc_mode;
    unsigned int symc_iv[4]; /* 4 iv len */
    unsigned int symc_ivlen;

    struct spacc_symc_in_entry_t*   entry_symc_in;
    struct spacc_symc_out_entry_t*  entry_symc_out;

    unsigned int entry_symc_in_depth;
    unsigned int entry_symc_out_depth;
    unsigned int symc_cur_in_nodes;
    unsigned int symc_cur_out_nodes;
};

#define SAPCC_SYMC_IN_ENTRY_TOTAL_SIZE (sizeof(struct spacc_symc_in_entry_t) * SPACC_MAX_DEPTH)
#define SAPCC_SYMC_OUT_ENTRY_TOTAL_SIZE (sizeof(struct spacc_symc_out_entry_t) * SPACC_MAX_DEPTH)
#define SAPCC_DIGEST_IN_ENTRY_TOTAL_SIZE (sizeof(struct spacc_digest_in_entry_t) * SPACC_MAX_DEPTH)

void *g_spacc_reg_base = 0;
static struct spacc_digest_context *g_digest_context = XMEDIA_NULL;
static struct spacc_symc_context   *g_symc_context = XMEDIA_NULL;

static xmedia_void* g_cipher_reg_base = XMEDIA_NULL;
static xmedia_void* cipher_entry_addr = XMEDIA_NULL;

void cipher_debug_log(unsigned short errflag)
{
    char errStr[12];

    errStr[0] = 'c';
    errStr[1] = 'i';
    errStr[2] = 'p';
    errStr[3] = 'h';
    errStr[4] = 'e';
    errStr[5] = 'r';
    errStr[6] = '#';
    errStr[7] = 'E';
    errStr[8] = (char)(errflag/10) + '0';
    errStr[9] = (char)(errflag%10) + '0';
    errStr[10] = '\n';
    errStr[11] = 0;
    xmedia_err_cipher(errStr);
}

/*
 * spacc_digest_configure - configure the hash ctrl register.
 */
int spacc_sha256_config(unsigned int chn_num,
                        digest_alg_en digest_alg,
                        digest_mode_en digest_mode,
                        unsigned char hard_key)
{
    struct spacc_digest_context *info = g_digest_context;
    u_chann_hash_ctrl hash_ctrl;

    info->digest_alg = digest_alg;
    info->digest_mode = digest_mode;
    info->digest_len = SHA256_RESULT_SIZE;   /* 32 sha1 digest len */
    info->digest_blen = SHA256_BLOCK_SIZE;  /* 64 sha1 digest blen */

    hash_ctrl.u32 = spacc_read(chn_n_hash_ctrl(chn_num));
    hash_ctrl.bits.hash_chn_mode = digest_mode;
    hash_ctrl.bits.hash_chn_agl_sel = digest_alg;
    spacc_write(chn_n_hash_ctrl(chn_num), hash_ctrl.u32);

    info->entry_digest_in_depth = 0;

    return XMEDIA_SUCCESS;
}

/**
 * spacc_digest_addbuf - filling the buf addr and length of
 *   data into nodes list.
 *
 */
int spacc_sha256_addbuf(unsigned int chn_num,
                        unsigned long buf_phy,
                        unsigned int buf_size,
                        unsigned int ctrl)
{
    struct spacc_digest_context *info = g_digest_context;
    unsigned int id, size;
    void *addr = XMEDIA_NULL;

    id = info->digest_cur_in_nodes++;
    addr = &info->entry_digest_in[id];
    size = sizeof(struct spacc_digest_in_entry_t);
    crypto_memset(addr, sizeof(info->entry_digest_in[id]), 0, size);
    info->entry_digest_in[id].spacc_cmd = 0x00;
    info->entry_digest_in[id].hash_start_addr = get_ulong_low(buf_phy);
    info->entry_digest_in[id].hash_alg_length = buf_size;
    info->entry_digest_in[id].hash_ctrl = ctrl;
    info->entry_digest_in[id].hash_start_addr_high = get_ulong_high(buf_phy);
    info->entry_digest_in_depth++;
    info->digest_cur_in_nodes %= SPACC_MAX_DEPTH;

    return XMEDIA_SUCCESS;
}

/**
 * spacc_digest_get - get hash result.
 *
 */
int spacc_sha256_get(unsigned int chn_num, unsigned int *digest)
{
    unsigned int i;

    for (i = 0; i < SHA256_RESULT_IN_WORD; i++) {
        spacc_write(chn_n_hash_state_val_addr(chn_num), i);
        digest[i] = spacc_read(chn_n_hash_state_val(chn_num));
    }

    return XMEDIA_SUCCESS;
}

/*
 * spacc_digest_start - action the hash start to processing the node list.
 */
int spacc_sha256_start(unsigned int chn_num, spacc_ctrl_en spacc_ctrl, unsigned int *state)
{
    unsigned int i;
    u_chann_hash_in_node_cfg in_node_cfg;
    struct spacc_digest_context *info = g_digest_context;
    unsigned int ptr;

    /* Write last state */
    for (i = 0; i < SHA256_RESULT_IN_WORD; i++) {
        spacc_write(chn_n_hash_state_val_addr(chn_num), i);
        spacc_write(chn_n_hash_state_val(chn_num), state[i]);
    }

    if (info->entry_digest_in_depth == 0)
        return XMEDIA_SUCCESS;

    /* configure in-node */
    in_node_cfg.u32 = spacc_read(chn_n_hash_in_node_cfg(chn_num));
    if (in_node_cfg.bits.hash_in_node_wptr != in_node_cfg.bits.hash_in_node_rptr) {
        cipher_debug_log(CIPHER_ERR_SHA256_START);
        return XMEDIA_FAILURE;
    }

    ptr = in_node_cfg.bits.hash_in_node_wptr + info->entry_digest_in_depth;
    in_node_cfg.bits.hash_in_node_wptr = ptr % SPACC_MAX_DEPTH;
    in_node_cfg.bits.hash_in_node_mpackage_int_level = 1;

    /* Start */
    spacc_write(chn_n_hash_in_node_cfg(chn_num), in_node_cfg.u32);

    return XMEDIA_SUCCESS;
}

/*
 * spacc_digest_done_try - test the int status of hash channel.
 */
unsigned int spacc_sha256_done_try(unsigned int chn_num)
{
    u_hash_int_raw int_raw;
    unsigned int chn_mask;

    int_raw.u32 = spacc_read(HASH_INT_RAW);
    int_raw.bits.hash_chn_oram_raw &= 0x01 << chn_num;
    chn_mask = int_raw.bits.hash_chn_oram_raw;

    /* Clean raw int */
    spacc_write(HASH_INT_RAW, int_raw.u32);

    return chn_mask;
}

/*
 * spacc_digest_get_err_code - get the error code of hash.
 */
unsigned int spacc_sha256_get_err_code(unsigned int chn_num, unsigned int *src_addr)
{
    *src_addr = spacc_read(chn_n_hash_in_buf_rptr(chn_num));

    return spacc_read(CALC_ERR);
}

static xmedia_void spacc_config_start_addr(unsigned long entry_phy_addr, xmedia_void *entry_via_addr)
{
    xmedia_size_t page_phy;
    xmedia_void *page_via = XMEDIA_NULL;
    u_chann_cipher_in_node_cfg cipher_in_cfg;
    u_chann_cipher_out_node_cfg cipher_out_cfg;
    u_chann_hash_in_node_cfg hash_in_cfg;

    page_phy = entry_phy_addr;
    page_via = entry_via_addr;

    /* set total num and start addr for cipher in node */
    cipher_in_cfg.u32  = spacc_read(chn_n_cipher_in_node_cfg(SPACC_CHN_AES));
    cipher_in_cfg.bits.cipher_in_node_total_num = SPACC_MAX_DEPTH;
    spacc_write(chn_n_cipher_in_node_cfg(SPACC_CHN_AES), cipher_in_cfg.u32);
    spacc_write(chn_n_cipher_in_node_start_addr(SPACC_CHN_AES), get_ulong_low(page_phy));
    spacc_write(chn_n_cipher_in_node_start_addr_high(SPACC_CHN_AES), get_ulong_high(page_phy));
    g_symc_context->entry_symc_in = (struct spacc_symc_in_entry_t*)page_via;
    g_symc_context->symc_cur_in_nodes = cipher_in_cfg.bits.cipher_in_node_wptr;
    g_symc_context->entry_symc_in_depth = 0;
    page_via += SAPCC_SYMC_IN_ENTRY_TOTAL_SIZE;
    page_phy += SAPCC_SYMC_IN_ENTRY_TOTAL_SIZE;

    /* set total num and start addr for cipher out node */
    cipher_out_cfg.u32 = spacc_read(chn_n_cipher_out_node_cfg(SPACC_CHN_AES));
    cipher_out_cfg.bits.cipher_out_node_total_num = SPACC_MAX_DEPTH;
    spacc_write(chn_n_cipher_out_node_cfg(SPACC_CHN_AES), cipher_out_cfg.u32);
    spacc_write(chn_n_cipher_out_node_start_addr(SPACC_CHN_AES), get_ulong_low(page_phy));
    spacc_write(chn_n_cipher_out_node_start_addr_high(SPACC_CHN_AES), get_ulong_high(page_phy));
    g_symc_context->entry_symc_out = (struct spacc_symc_out_entry_t*)page_via;
    g_symc_context->symc_cur_out_nodes = cipher_out_cfg.bits.cipher_out_node_wptr;
    g_symc_context->entry_symc_out_depth = 0;
    page_via += SAPCC_SYMC_OUT_ENTRY_TOTAL_SIZE;
    page_phy += SAPCC_SYMC_OUT_ENTRY_TOTAL_SIZE;

    /* set total num and start addr for hash in node */
    hash_in_cfg.u32 = spacc_read(chn_n_hash_in_node_cfg(SPACC_CHN_SHA256));
    hash_in_cfg.bits.hash_in_node_total_num = SPACC_MAX_DEPTH;
    spacc_write(chn_n_hash_in_node_cfg(SPACC_CHN_SHA256), hash_in_cfg.u32);
    spacc_write(chn_n_hash_in_node_start_addr(SPACC_CHN_SHA256), get_ulong_low(page_phy));
    spacc_write(chn_n_hash_in_node_start_addr_high(SPACC_CHN_SHA256), get_ulong_high(page_phy));
    g_digest_context->entry_digest_in = (struct spacc_digest_in_entry_t*)page_via;
    g_digest_context->digest_cur_in_nodes = hash_in_cfg.bits.hash_in_node_wptr;
    g_digest_context->entry_digest_in_depth = 0;
}

/**
 * spacc_get_node_list_size - return the total size of nodes lists memory required by the drive.
 */
unsigned int spacc_get_node_list_size(void)
{
    /*********************************************************************
    0x40 + 0x40 + 0x20
    *********************************************************************/
    return (SAPCC_SYMC_IN_ENTRY_TOTAL_SIZE + SAPCC_SYMC_OUT_ENTRY_TOTAL_SIZE + SAPCC_DIGEST_IN_ENTRY_TOTAL_SIZE);
}

/**
 * spacc_init - spacc hardware initialization.
 * @reg_base: virtual address of spacc module which be accessed by CPU
 * @mmu_table_addr: mmu base table physical addr, if disable mmu, set it to 0
 * @entry_phy_addr: a consecutive physical memory, used for nodes
 *                  list of symc-in, symc-out and hash-in, the size
 *                   must large than spacc_get_node_list_size().
 * @entry_via_addr: virtual address of entry_phy_addr.
 *
 * Description:
 *   spacc hardware initialization as follows:
 *   - reset global var.
 *   - enable interrupt
 *   - set nodes list addr
 *   - set mmu table addr
 *   - configureure hardware register
 *
 * Context:
 *   this function must be called one time in the beginning.
 */
int spacc_init(xmedia_void *reg_base, unsigned long entry_phy_addr, xmedia_void *entry_via_addr)
{
    crypto_memset(g_symc_context, sizeof(struct spacc_symc_context), 0, sizeof(struct spacc_symc_context));
    crypto_memset(g_digest_context, sizeof(struct spacc_digest_context), 0, sizeof(struct spacc_digest_context));

    g_spacc_reg_base = reg_base;

    //not enable smmu
    //not enable int

    /* configure start addr for in-node and out-node */
    spacc_config_start_addr(entry_phy_addr, entry_via_addr);

    return XMEDIA_SUCCESS;
}

/**
 * spacc_deinit - spacc hardware deinit.
 */
int spacc_deinit(void)
{
    return XMEDIA_SUCCESS;
}

/*
 * spacc_symc_configure - configure logic register, such as alg, mode, key len and so on.
 */
int spacc_symc_config(unsigned int chn_num, spacc_symc_config_s *symc_cfg, unsigned char hard_key)
{
    struct spacc_symc_context *info = g_symc_context;
    u_chann_cipher_ctrl cipher_ctrl;

    cipher_ctrl.u32 = spacc_read(chn_n_cipher_ctrl(chn_num));
    cipher_ctrl.bits.sym_chn_key_sel = hard_key;
    cipher_ctrl.bits.sym_chn_key_length = 2;   //bit[11:10] 2:256bit
    cipher_ctrl.bits.sym_chn_dat_width = symc_cfg->symc_width;
    cipher_ctrl.bits.sym_chn_decrypt = 0x00;
    cipher_ctrl.bits.sym_chn_alg_sel = symc_cfg->symc_alg;
    cipher_ctrl.bits.sym_chn_alg_mode = symc_cfg->symc_mode;
    spacc_write(chn_n_cipher_ctrl(chn_num), cipher_ctrl.u32);

    info->symc_alg = symc_cfg->symc_alg;
    info->symc_mode = symc_cfg->symc_mode;
    info->entry_symc_in_depth = 0;
    info->entry_symc_out_depth = 0;

    return XMEDIA_SUCCESS;
}
/**
 * spacc_symc_setiv - set iv for symc.
 * @chn_num: the logic channel number, must 1~7.
 * @iv: the initialization vector
 * @ivlen:  length of iv.
 *
 * Description:
 *   here store the iv to global structure of channel, don't set to logic,
 *   because the IV must be set in the nodes list.
 *
 */
int spacc_symc_setiv(unsigned int chn_num, unsigned char *iv, unsigned int ivlen)
{
    struct spacc_symc_context *info = g_symc_context;

    crypto_memset(info->symc_iv, sizeof(info->symc_iv), 0, sizeof(info->symc_iv));
    crypto_memcpy(info->symc_iv, sizeof(info->symc_iv), iv, ivlen);
    info->symc_ivlen = ivlen;

    return XMEDIA_SUCCESS;
}

/*
 * spacc_symc_addbuf - filling the buf addr and length of
 *   encrypt/decrypt data into nodes list.
 */
int spacc_symc_addbuf(unsigned int chn_num, unsigned long buf_phy,
    unsigned int buf_size, spacc_buf_type_en type, unsigned int ctrl)
{
    struct spacc_symc_context *info = g_symc_context;
    unsigned int id, size;
    void *addr = XMEDIA_NULL;

    switch (type) {
        case SPACC_BUF_TYPE_SYMC_IN:
            id = info->symc_cur_in_nodes++;
            addr = &info->entry_symc_in[id];
            size = sizeof(struct spacc_symc_in_entry_t);
            crypto_memset(addr, sizeof(info->entry_symc_in[id]), 0, size);
            info->entry_symc_in[id].spacc_cmd = 0x00;
            info->entry_symc_in[id].sym_start_addr = get_ulong_low(buf_phy);
            info->entry_symc_in[id].sym_start_addr_high = get_ulong_high(buf_phy);
            info->entry_symc_in[id].sym_alg_length = buf_size;
            info->entry_symc_in[id].sym_ctrl = ctrl;
            info->entry_symc_in_depth++;
            info->symc_cur_in_nodes %= SPACC_MAX_DEPTH;
            break;
        case SPACC_BUF_TYPE_SYMC_OUT:
            id = info->symc_cur_out_nodes++;
            addr = &info->entry_symc_out[id];
            size = sizeof(struct spacc_symc_out_entry_t);
            crypto_memset(addr, sizeof(info->entry_symc_out[id]), 0, size);
            info->entry_symc_out[id].sym_start_addr = get_ulong_low(buf_phy);
            info->entry_symc_out[id].tag[0] = get_ulong_high(buf_phy);
            info->entry_symc_out[id].sym_alg_length = buf_size;
            info->entry_symc_out[id].aes_ctrl = ctrl;
            info->entry_symc_out_depth++;
            info->symc_cur_out_nodes %= SPACC_MAX_DEPTH;
            break;
        default:
            return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

static int spacc_config_out_node(unsigned int chn_num, struct spacc_symc_context *info)
{
    u_chann_cipher_out_node_cfg out_node_cfg;
    unsigned int ptr;

    out_node_cfg.u32 = spacc_read(chn_n_cipher_out_node_cfg(chn_num));
    if (out_node_cfg.bits.cipher_out_node_wptr != out_node_cfg.bits.cipher_out_node_rptr) {
        cipher_debug_log(CIPHER_ERR_CIPHER_CFG_OUTNODE);
        return XMEDIA_FAILURE;
    }
    ptr = out_node_cfg.bits.cipher_out_node_wptr + info->entry_symc_out_depth;
    out_node_cfg.bits.cipher_out_node_wptr = ptr % SPACC_MAX_DEPTH;
    out_node_cfg.bits.cipher_out_node_mpackage_int_level = info->entry_symc_out_depth;
    spacc_write(chn_n_cipher_out_node_cfg(chn_num), out_node_cfg.u32);

    return XMEDIA_SUCCESS;
}

static void spacc_cipher_chn_crypt(unsigned int chn_num, unsigned int decrypt)
{
    u_chann_cipher_ctrl cipher_ctrl;

    cipher_ctrl.u32 = spacc_read(chn_n_cipher_ctrl(chn_num));
    cipher_ctrl.bits.sym_chn_decrypt = decrypt;
    spacc_write(chn_n_cipher_ctrl(chn_num), cipher_ctrl.u32);
}

static void spacc_config_in_node(unsigned int chn_num, struct spacc_symc_context *info)
{
    u_chann_cipher_in_node_cfg in_node_cfg;
    unsigned int ptr;

    in_node_cfg.u32 = spacc_read(chn_n_cipher_in_node_cfg(chn_num));
    ptr = in_node_cfg.bits.cipher_in_node_wptr + info->entry_symc_in_depth;
    in_node_cfg.bits.cipher_in_node_wptr = ptr % SPACC_MAX_DEPTH;
    in_node_cfg.bits.cipher_in_node_mpackage_int_level = info->entry_symc_in_depth;

    /* move forward the in-node ptr to action the symc working */
    spacc_write(chn_n_cipher_in_node_cfg(chn_num), in_node_cfg.u32);
}

/*
 * spacc_symc_done_try - test the int status of symc channel.
 */
unsigned int spacc_symc_done_try(unsigned int chn_num)
{
    u_cipher_int_raw int_raw;
    unsigned int chn_mask;

    int_raw.u32 = spacc_read(CIPHER_INT_RAW);
    int_raw.bits.cipher_chn_obuf_raw &= 0x01 << chn_num;
    chn_mask = int_raw.bits.cipher_chn_obuf_raw;

    /* Clean raw int */
    int_raw.u32 = 0x00;
    int_raw.bits.cipher_chn_obuf_raw = chn_mask;
    spacc_write(CIPHER_INT_RAW, int_raw.u32);

    return chn_mask ? 1 : 0;
}

/*
 * spacc_symc_get_err_code - get the error code of symc.
 */
unsigned int spacc_symc_get_err_code(unsigned int chn_num,
                                     unsigned int *src_addr,
                                     unsigned int *dst_addr)
{
    *src_addr = spacc_read(chn_n_cipher_in_buf_rptr(chn_num));
    *dst_addr = spacc_read(chn_n_cipher_out_buf_rptr(chn_num));

    return spacc_read(CALC_ERR);
}

/* check error code
* bit0: klad_key_use_err
* bit1: alg_len_err
* bit2: smmu_page_unvlid
*/
static xmedia_s32 spacc_cipher_check_error_code(xmedia_u32 hard_num, xmedia_u32 wait, xmedia_u32 src_addr)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;

    if (wait & 0x01) {
        cipher_debug_log(CIPHER_ERR_CIPHER_ERRCODE_1);
        ret = XMEDIA_FAILURE;
    }

    if (wait & 0x02) {
        cipher_debug_log(CIPHER_ERR_CIPHER_ERRCODE_2);
        ret = XMEDIA_FAILURE;
    }

    if (wait & 0x04) {
        cipher_debug_log(CIPHER_ERR_CIPHER_ERRCODE_4);
        ret = XMEDIA_FAILURE;
    }

    return ret;
}

/*
 * spacc_symc_start - action the symc start to processing the node list.
 */
int spacc_symc_start(unsigned int chn_num, unsigned int decrypt)
{
    unsigned int cur, i, j, node;
    u_chann_cipher_in_node_cfg in_node_cfg;
    struct spacc_symc_context *info = g_symc_context;

    in_node_cfg.u32 = spacc_read(chn_n_cipher_in_node_cfg(chn_num));
    cur = in_node_cfg.bits.cipher_in_node_rptr;

    for (j = 0; j < info->entry_symc_in_depth; j++) {
        if (info->symc_ivlen > 0) {
            /* Write iv to all nodes */
            node = (cur + j) % SPACC_MAX_DEPTH;
            for (i = 0; i < 4; i++) { /* 4 iv len */
                info->entry_symc_in[node].symc_iv[i] = info->symc_iv[i];
            }

            info->entry_symc_in[node].gcm_iv_len = 0;
            info->entry_symc_in[node].sym_ctrl |= SPACC_CTRL_SYMC_IN_FIRST;
        }
    }

    if (spacc_config_out_node(chn_num, info) != XMEDIA_SUCCESS) {
        return XMEDIA_FAILURE;
    }
    /* encrypt or decrypt */
    spacc_cipher_chn_crypt(chn_num, decrypt);

    /* spacc_config_in_node must be placed after flush_cache, otherwwise it may be crypt timeout */
    spacc_config_in_node(chn_num, info);

    /* all the nodes are processing, reset the depth to 0 */
    info->entry_symc_in_depth = 0;
    info->entry_symc_out_depth = 0;

    return XMEDIA_SUCCESS;
}

static xmedia_s32 drv_cipher_sha256_config(cipher_hash_data_s *cipher_hash_data, spacc_ctrl_en *spacc_ctrl)
{
    xmedia_s32 ret;
    digest_alg_en digest_alg;
    digest_mode_en digest_mode;

    *spacc_ctrl = SPACC_CTRL_NONE;
    digest_alg = DIGEST_ALG_SHA256;
    digest_mode = DIGEST_MODE_HASH;

    ret = spacc_sha256_config(cipher_hash_data->hard_chn, digest_alg, digest_mode, XMEDIA_FALSE);

    *spacc_ctrl = ((xmedia_u32)*spacc_ctrl) | SPACC_CTRL_HASH_IN_FIRST;
    *spacc_ctrl = ((xmedia_u32)*spacc_ctrl) | SPACC_CTRL_HASH_IN_LAST;

    return ret;

}
static xmedia_s32 drv_cipher_sha256_wait_done(unsigned int channel)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_u32 wait;
    xmedia_u32 time_out = SPACC_TIME_OUT;
    xmedia_u32 src_addr;

    time_out = 0;
    while (time_out++ < SPACC_TIME_OUT) {
        if (spacc_sha256_done_try(channel))
            break;
        cipher_usleep(10);
    }
    if (time_out >= SPACC_TIME_OUT) {
        cipher_debug_log(CIPHER_ERR_SHA256_WAIT_TIMEOUT);
        ret = XMEDIA_FAILURE;
    }

    /* check error code
    * bit0: klad_key_use_err
    * bit1: alg_len_err
    * bit2: smmu_page_unvlid
    */
    wait = spacc_sha256_get_err_code(channel, &src_addr);
    if (spacc_cipher_check_error_code(channel, wait, src_addr) != XMEDIA_SUCCESS) {
        ret = XMEDIA_FAILURE;
    }

    return ret;
}

xmedia_s32 drv_cipher_sha256_update(cipher_hash_data_s *cipher_hash_data)
{
    xmedia_s32 ret;
    spacc_ctrl_en spacc_ctrl;

    /* configure hash register */
    ret = drv_cipher_sha256_config(cipher_hash_data, &spacc_ctrl);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    /* Add the phy of data to nodes list */
    ret = spacc_sha256_addbuf(cipher_hash_data->hard_chn, make_ulong(cipher_hash_data->data_phy, cipher_hash_data->data_phy_high),
            cipher_hash_data->data_len, spacc_ctrl);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    /* Start working */
    ret = spacc_sha256_start(cipher_hash_data->hard_chn, spacc_ctrl, cipher_hash_data->sha_val);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    /* Waiting hardware computing finished */
    ret = drv_cipher_sha256_wait_done(cipher_hash_data->hard_chn);
    if (ret == XMEDIA_SUCCESS) /* Read hash result */
        spacc_sha256_get(cipher_hash_data->hard_chn, cipher_hash_data->sha_val);

    return ret;
}

xmedia_s32 drv_cipher_sha256_final(cipher_hash_data_s *cipher_hash_data)
{
    return drv_cipher_sha256_update(cipher_hash_data);
}

static xmedia_s32 drv_cipher_reset(xmedia_void)
{
    xmedia_u32 *pvirt = XMEDIA_NULL;
    xmedia_u32 spacc_stat = 0;

    pvirt = cipher_ioremap_nocache(CIPHER_SPACC_CRG_ADDR_PHY, 16); /* 16 */

    /* open clock, reset */
    spacc_stat = hal_cipher_read_reg(pvirt);
    spacc_stat |= SPACC_CRG_CLOCK_BIT;
    spacc_stat |= SPACC_CRG_RESET_BIT;
    hal_cipher_write_reg(pvirt, spacc_stat);
    cipher_usleep(10); /* 10us */

    /* cancel reset */
    spacc_stat &= ~SPACC_CRG_RESET_BIT;
    hal_cipher_write_reg(pvirt, spacc_stat);

    //enable_spacc_safety();

    cipher_iounmap(pvirt);
    pvirt = XMEDIA_NULL;

    return XMEDIA_SUCCESS;
}

static xmedia_s32 drv_cipher_symc_wait_done(xmedia_u32 hw_chan, xmedia_u32 time_out)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_u32 wait;
    xmedia_u32 src_addr, dst_addr;

    time_out = 0;
    while (time_out++ < SPACC_TIME_OUT) {
        if (spacc_symc_done_try(hw_chan))
            break;
        cipher_usleep(10);
    }
    if (time_out >= SPACC_TIME_OUT) {
        cipher_debug_log(CIPHER_ERR_CIPHER_WAIT_TIMEOUT);
        ret = XMEDIA_FAILURE;
    }

    wait = spacc_symc_get_err_code(hw_chan, &src_addr, &dst_addr);
    if (spacc_cipher_check_error_code(hw_chan, wait, src_addr) != XMEDIA_SUCCESS) {
        ret = XMEDIA_FAILURE;
    }

    return ret;
}

xmedia_s32 drv_cipher_init(xmedia_void)
{
    xmedia_u32 malloc_size;

    dcache_disable();

    g_cipher_reg_base = cipher_ioremap_nocache(CIPHER_CIPHER_REG_BASE_ADDR_PHY, 0x2000);

    if (drv_cipher_reset() != XMEDIA_SUCCESS) {
        return XMEDIA_FAILURE;
    }

    malloc_size = spacc_get_node_list_size();
    cipher_entry_addr = cipher_malloc(malloc_size);
    if (cipher_entry_addr == XMEDIA_NULL) {
        cipher_debug_log(CIPHER_ERR_CIPHER_MALLOC);
        return XMEDIA_FAILURE;
    }

    /*sizeof(struct spacc_digest_context) == 0x1C*/
    g_digest_context = cipher_malloc(sizeof(struct spacc_digest_context));
    if (g_digest_context == XMEDIA_NULL) {
        cipher_debug_log(CIPHER_ERR_CIPHER_MALLOC);
        cipher_free(cipher_entry_addr);
        cipher_entry_addr = XMEDIA_NULL;
        return XMEDIA_FAILURE;
    }

    /*sizeof(struct spacc_digest_context) == 0x34*/
    g_symc_context = cipher_malloc(sizeof(struct spacc_symc_context));
    if (g_symc_context == XMEDIA_NULL) {
        cipher_free(cipher_entry_addr);
        cipher_entry_addr = XMEDIA_NULL;
        cipher_free(g_digest_context);
        g_digest_context = XMEDIA_NULL;
        cipher_debug_log(CIPHER_ERR_CIPHER_MALLOC);
        return XMEDIA_FAILURE;
    }

    crypto_memset(cipher_entry_addr, malloc_size, 0, malloc_size);
    spacc_init(g_cipher_reg_base, (unsigned long)cipher_entry_addr, cipher_entry_addr);

    return XMEDIA_SUCCESS;
}

xmedia_s32 drv_cipher_deinit(xmedia_void)
{
    spacc_deinit();

    cipher_free(cipher_entry_addr);
    cipher_entry_addr = XMEDIA_NULL;

    cipher_iounmap(g_cipher_reg_base);

    cipher_free(g_digest_context);
    g_digest_context = XMEDIA_NULL;
    cipher_free(g_symc_context);
    g_symc_context = XMEDIA_NULL;

    dcache_enable();

    return XMEDIA_SUCCESS;
}

xmedia_s32 drv_cipher_config_aes_chn(cipher_ctrl *config)
{
    xmedia_s32 ret;
    spacc_symc_config_s symc_cfg;

    symc_cfg.symc_alg = SYMC_ALG_AES;
    symc_cfg.symc_mode = SYMC_MODE_CBC;
    symc_cfg.symc_width = SYMC_DAT_WIDTH_128;
    symc_cfg.key_len = 32;

    ret = spacc_symc_config(SPACC_CHN_AES, &symc_cfg, XMEDIA_TRUE);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    //AES256 CBC
    ret = drv_cipher_klad_load_key(SPACC_CHN_AES, config->key, 32); /* 16 key len */
    if (ret != XMEDIA_SUCCESS) {
        cipher_debug_log(CIPHER_ERR_CIPHER_LOAD_KEY);
        return ret;
    }

    ret = spacc_symc_setiv(SPACC_CHN_AES, (xmedia_u8*)config->iv, sizeof(config->iv));

    return ret;
}

xmedia_s32 drv_cipher_aes_decrypt(cipher_data_s *ci_data)
{
    xmedia_s32 ret;
    xmedia_size_t src_phy_addr, dest_phy_addr;

    src_phy_addr = make_ulong(ci_data->src_phy_addr, ci_data->src_phy_addr_high);
    dest_phy_addr = make_ulong(ci_data->dest_phy_addr, ci_data->dest_phy_addr_high);

    /* Add in buffer */
    ret = spacc_symc_addbuf(SPACC_CHN_AES, src_phy_addr, ci_data->data_length,
        SPACC_BUF_TYPE_SYMC_IN, SPACC_CTRL_SYMC_IN_LAST);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    /* Add out buffer */
    ret = spacc_symc_addbuf(SPACC_CHN_AES, dest_phy_addr, ci_data->data_length,
        SPACC_BUF_TYPE_SYMC_OUT, SPACC_CTRL_SYMC_OUT_LAST);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    /* Start working */
    spacc_symc_start(SPACC_CHN_AES, XMEDIA_TRUE);

    /* Waiting hardware computing finished */
    ret = drv_cipher_symc_wait_done(SPACC_CHN_AES, SPACC_TIME_OUT);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    return XMEDIA_SUCCESS;
}
