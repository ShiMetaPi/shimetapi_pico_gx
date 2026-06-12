/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#include <pthread.h>
#include <stdio.h>
#include <signal.h>

#include "uvc.h"
#include "uvc_hal.h"
#include "video_stream.h"

#include "uvc_log.h"

extern void video_stream_register(void);

static int g_uvc_run = 0;

static int __uvc_init(void)
{
    video_stream_register();
    video_stream_init();
    return 0;
}

static int __uvc_deinit(void)
{

    video_stream_deinit();
    video_stream_unregister();
    return 0;
}

static int __uvc_open(void)
{
    return open_uvc_device();
}

static int __uvc_close(void)
{
    return close_uvc_device();
}

static pthread_t g_pid_stream;
static pthread_t g_pid_contrl;

static void *uvc_stream_process(void *args)
{
    uvc_logt("UVC stream process start.\n");
    while (g_uvc_run) {
        run_uvc_data();
    }

    uvc_logt("UVC stream process exit.\n");

    return NULL;
}

static void *uvc_contrl_process(void *args)
{
    uvc_logt("UVC contrl process start.\n");
    while (g_uvc_run) {
        run_uvc_device();
    }

    uvc_logt("UVC contrl process exit.\n");

    return NULL;
}
static int __uvc_run(void)
{
    int err;
    g_uvc_run = 1;

    uvc_logt("UVC run entry.\n");
    err = pthread_create(&g_pid_stream, NULL, &uvc_stream_process, NULL);
    if (err != 0) {
        uvc_loge("Create uvc_stream_process failed.\n");
    }

    err = pthread_create(&g_pid_contrl, NULL, &uvc_contrl_process, NULL);
    if (err != 0) {
        uvc_loge("Create uvc_contrl_process failed.\n");
    }

    uvc_logt("UVC run ok.\n");

    return 0;
}

static int __uvc_stop(void)
{
    uvc_logt("UVC stop entry.\n");
    g_uvc_run = 0;
    pthread_join(g_pid_contrl, NULL);
    pthread_join(g_pid_stream, NULL);
    uvc_logt("UVC stop ok.\n");

    return 0;
}

/* ---------------------------------------------------------------------- */
static uvc_t __uvc = {
    .init = &__uvc_init,
    .deinit = &__uvc_deinit,
    .open = &__uvc_open,
    .close = &__uvc_close,
    .run = &__uvc_run,
    .stop = &__uvc_stop,
};

uvc_t *get_uvc()
{
    return &__uvc;
}
