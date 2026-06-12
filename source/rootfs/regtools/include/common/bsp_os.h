/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __XMEDIA_OS_H__
#define __XMEDIA_OS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#if defined(OS_LINUX)

#endif

#if defined(OS_WINCE)

#endif

#if defined(OS_WIN32)

#endif

#if defined(OS_LINUX)
#define  _UNUSED_FUNC __attribute__ ((unused))
#define  _UNUSED_VAR  __attribute__ ((unused))

/*
int xxx() _NOWARN_UNUSED;

int _NOWARN_UNUSED YYY()
{
    return 0;
}
*/
#define _NOWARN_UNUSED _UNUSED_FUNC
#endif

#if defined(OS_VXWORKS)

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __XMEDIA_OS_H__ */
