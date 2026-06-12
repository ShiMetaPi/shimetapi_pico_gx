/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#ifndef __RUN_UVC_H__
#define __RUN_UVC_H__

typedef struct uvc {
    int (*init)(void);
    int (*deinit)(void);
    int (*open)(void);
    int (*close)(void);
    int (*run)(void);
    int (*stop)(void);
} uvc_t;

#if UVC_COMPILE
uvc_t *get_uvc(void);
#else
uvc_t *get_uvc(void)
{
    return NULL;
}
#endif

#endif
