/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#ifndef __RUN_UAC_H__
#define __RUN_UAC_H__

typedef struct uac {
    int (*init)(void);
    int (*deinit)(void);
    int (*open)(void);
    int (*close)(void);
    int (*run)(void);
    int (*stop)(void);
} uac_t;

#if UAC_COMPILE
uac_t* get_uac(void);
#else
uac_t* get_uac(void)
{
    return NULL;
}
#endif

#endif
