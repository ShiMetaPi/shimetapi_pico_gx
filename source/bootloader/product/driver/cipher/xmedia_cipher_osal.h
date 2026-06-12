/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __XMDEDIA_CIPHER_OSAL_H_
#define __XMDEDIA_CIPHER_OSAL_H_

#include "linux/lotus/securec.h"
#include "linux/delay.h"
#include "common.h"
#include "xmedia_type.h"
#include "xmedia_cipher.h"
#include "xmedia_cipher_drv.h"
#include "xmedia_cipher_config.h"

/**************************** M A C R O ****************************/
#define cipher_min(a, b) ((a) < (b) ? (a) : (b))

#define get_ulong_low(dw) (unsigned int)(dw)
#define get_ulong_high(dw) 0
#define make_ulong(low, high) (low)

#define u32_to_point(addr)                  ((xmedia_void*)((xmedia_size_t)(addr)))
#define point_to_u32(addr)                  ((xmedia_u32)((xmedia_size_t)(addr)))


#define hal_cipher_read_reg(addr)           (*(volatile unsigned int *)(addr))
#define hal_cipher_write_reg(addr, result)  (*(volatile unsigned int *)(addr) = (result))
//#define hal_cipher_read_reg(addr)             reg_get(addr)
//#define hal_cipher_write_reg(addr, result)    reg_set(addr, result)


#define hal_set_bit(src, bit)               ((src) |= (1 << (bit)))
#define hal_clear_bit(src, bit)             ((src) &= ~(1 << (bit)))
#define cipher_cpu_to_be16(v) (((v) << 8) | ((v) >> 8))
//#define cipher_cpu_to_be32(v) (((v) >> 24) | (((v) >> 8) & 0xff00) | (((v) << 8) & 0xff0000) | ((v) << 24))
#define cipher_cpu_to_be32(v) (((v) >> 24) | (((v) >> 8) & 0xff00) | (((v)&0x0000ff00)<<8) | (((v)&0x000000ff)<<24))
#define cipher_cpu_to_be64(x) ((xmedia_u64)(                         \
    (((x) & 0x00000000000000ffULL) << 56) | (((x) & 0x000000000000ff00ULL) << 40) |   \
    (((x) & 0x0000000000ff0000ULL) << 24) | (((x) & 0x00000000ff000000ULL) <<  8) |   \
    (((x) & 0x000000ff00000000ULL) >>  8) | (((x) & 0x0000ff0000000000ULL) >> 24) |   \
    (((x) & 0x00ff000000000000ULL) >> 40) | (((x) & 0xff00000000000000ULL) >> 56)))
/**************************** S T D L I B ****************************/
#define cipher_ioremap_nocache(addr, size)  (xmedia_void*)(addr)
#define cipher_iounmap(x)

#define CIPHER_MUTEX                        xmedia_void *
#define cipher_mutex_init(x)
#define cipher_mutex_lock(x)
#define cipher_mutex_unlock(x)
#define cipher_mutex_destroy(x)

#define CIPHER_QUEUE_HEAD                   xmedia_void *
#define cipher_queue_init(x)
#define cipher_queue_wait_up(x)
#define cipher_queue_wait_timeout(head, con, time)

#define cipher_malloc(x)                    malloc(x)
#define cipher_free(x)                      free(x)

#define crypto_memset(d,dmax,c,l)           memset_s(d,dmax,c,l)
#define crypto_memcmp(a,b,c)                ((0 == memcmp(a,b,c))?(XMEDIA_SUCCESS):(XMEDIA_FAILURE))
#define crypto_memcpy(d,dmax,s,l)           memcpy_s(d,dmax,s,l)

#define cipher_msleep(msec)                 udelay((msec)*1000)
#define cipher_usleep(usec)                 udelay(usec)

#define xmedia_err_cipher(a)                    printf("%s", a)
#endif
