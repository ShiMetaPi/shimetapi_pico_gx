#ifndef __SAMPLE_API_H__
#define __SAMPLE_API_H__

#include "xmedia_type.h"

#ifdef __cplusplus
extern "C" {
#endif

xmedia_s32 sample_cipher(xmedia_void);
xmedia_s32 sample_cipher_via(xmedia_void);
xmedia_s32 sample_multicipher(xmedia_void);
xmedia_s32 sample_cipher_efuse(xmedia_void);
xmedia_s32 sample_hash(xmedia_void);
xmedia_s32 sample_rng(xmedia_void);
xmedia_s32 sample_rsa_sign(xmedia_void);
xmedia_s32 sample_rsa_enc(xmedia_void);

#ifdef __cplusplus
}
#endif

#endif
