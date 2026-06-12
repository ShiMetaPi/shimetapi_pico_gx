/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __BTOOLS_H__
#define __BTOOLS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

XMEDIA_RET xmmc(int argc , char* argv[]);

XMEDIA_RET xmmd_l(int argc , char* argv[]);

XMEDIA_RET xmmd(int argc , char* argv[]);

XMEDIA_RET xmmm(int argc , char* argv[]);

XMEDIA_RET bsper(int argc , char* argv[]);

XMEDIA_RET bspew(int argc , char* argv[]);

XMEDIA_RET i2c_read(int argc , char* argv[]);

XMEDIA_RET i2c_write(int argc , char* argv[]);

XMEDIA_RET ssp_read(int argc , char* argv[]);

XMEDIA_RET ssp_write(int argc , char* argv[]);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __BTOOLS_H__ */
