/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __XMEDIA_DEFS_H__
#define __XMEDIA_DEFS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#ifndef __GNUC__
#define __asm__    asm
#define __inline__ inline
#endif

#define DO_NOTHING

#define INLINE inline

/*use for parameter INPUT, *DO NOT Modify the value* */
#define IN
/* use for parameter OUTPUT, the value maybe change when return from the function
 * the init value is ingore in the function.*/
#define OUT
/*use for parameter INPUT and OUTPUT*/
#define IO

/* --------------------------------  */
#define EXTERN extern
#define STATIC static

#define LOCALVAR static
#define GLOBALVAR extern

#define DECLARE_GLOBALVAR

#define USE_GLOBALVAR extern

#define LOCALFUNC    static
#define EXTERNFUNC   extern

#define LAPI
#define HAPI
/*Multimedia Frame API*/
#define MMFAPI

/* -------- Standard input/output/err *****/
#define STDIN  stdin
#define STDOUT stdout
#define STDERR stderr

#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))

#define ALIGN_START(v,a) (((v)) & (~((a)-1)))

#define ALIGN_END(v,a) (((v) & ~((a)-1)) + ((a)-1) )

#define ALIGN_NEXT(v,a) ((((v) + ((a)-1)) & (~((a)-1))))

#define ALIGN_LENGTH(l, a) ALIGN_NEXT(l, a)

#define ALIGNTYPE_1BYTE  1
/*zsp*/
#define ALIGNTYPE_2BYTE  2
/*x86... default*/
#define ALIGNTYPE_4BYTE  4

#define ALIGNTYPE_8BYTE  8
/*1 Page*/
#define ALIGNTYPE_4K     4096

#define ALIGNTYPE_ZSP    ALIGNTYPE_2BYTE

#define ALIGNTYPE_VIDEO  ALIGNTYPE_8BYTE

#define PRINT_OUT(args...) printf(##args)

#define PACK_ONE_BYTE  __attribute__((__packed__))

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __XMEDIA_DEFS_H__ */
