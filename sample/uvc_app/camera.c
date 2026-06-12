/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#include <stdio.h>
#include <pthread.h>

#include "camera.h"
#include "uvc.h"
#include "uac.h"

/* -------------------------------------------------------------------------- */
extern unsigned int g_uvc;
extern unsigned int g_uac;

static uvc_t* g_uvc_dev = NULL;
static uac_t* g_uac_dev = NULL;

static int __camera_init(void)
{
    if (g_uvc) {
        g_uvc_dev = get_uvc();
        if (g_uvc_dev && g_uvc_dev->init) {
            g_uvc_dev->init();
        }
    }

    if (g_uac) {
        g_uac_dev = get_uac();
        if (g_uac_dev && g_uac_dev->init) {
            g_uac_dev->init();
        }
    }

    return 0;
}

static int __camera_deinit(void)
{
    if (g_uac) {
        if (g_uac_dev && g_uac_dev->deinit) {
            g_uac_dev->deinit();
        }
    }

    if (g_uvc) {
        if (g_uvc_dev && g_uvc_dev->deinit) {
            g_uvc_dev->deinit();
        }
    }

    g_uvc_dev = NULL;
    g_uac_dev = NULL;

    return 0;
}

static int __camera_open(void)
{
    if (g_uvc) {
        if (g_uvc_dev && g_uvc_dev->open) {
            g_uvc_dev->open();
        }
    }

    if (g_uac) {
        if (g_uac_dev && g_uac_dev->open) {
            g_uac_dev->open();
        }
    }

    return 0;
}

static int __camera_close(void)
{
    if (g_uac) {
        if (g_uac_dev && g_uac_dev->close) {
            g_uac_dev->close();
        }
    }

    if (g_uvc) {
        if (g_uvc_dev && g_uvc_dev->close) {
            g_uvc_dev->close();
        }
    }

    return 0;
}

static int __camera_run(void)
{
    if (g_uvc) {
        if (g_uvc_dev && g_uvc_dev->run) {
            g_uvc_dev->run();
        }
    }

    if (g_uac) {
        if (g_uac_dev && g_uac_dev->run) {
            g_uac_dev->run();
        }
    }
    return 0;
}

static int __camera_stop(void)
{
    if (g_uvc) {
        if (g_uvc_dev && g_uvc_dev->stop) {
            g_uvc_dev->stop();
        }
    }

    if (g_uac) {
        if (g_uac_dev && g_uac_dev->stop) {
            g_uac_dev->stop();
        }
    }

    return 0;
}
/* -------------------------------------------------------------------------- */

static camera __camera =
{
    .init = &__camera_init,
    .deinit = &__camera_deinit,
    .open = &__camera_open,
    .close = &__camera_close,
    .run = &__camera_run,
    .stop = &__camera_stop,
};

camera *get_camera(void)
{
    return &__camera;
}
