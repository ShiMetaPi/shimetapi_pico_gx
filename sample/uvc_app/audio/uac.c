/*
 * Copyright (c) XMEDIA. All rights reserved.
 */
#include <stdio.h>

#include "uac.h"
#include "uac_hal.h"
#include "audio_stream.h"

#include "uac_log.h"

static uac_dev_t* uac_dev = NULL;

static int __uac_init(void)
{
    int err = 0;
    audio_stream_operate_t* audio_stream = NULL;

    audio_stream = get_audio_stream();
    if (audio_stream == NULL) {
        uac_loge("Create audio stream dev failed.\n");
        return -1;
    }

    uac_dev = get_uac_dev();
    if (uac_dev == NULL) {
        uac_loge("Careate uac dev failed.\n");
        return -1;
    }

    err = uac_stream_register(audio_stream);
    if (err < 0) {
        uac_loge("UAC register stream dev failed.\n");
        return -1;
    }

    if (uac_dev->init) {
        err = uac_dev->init();
    } else {
        err = -1;
        uac_loge("Invalid uac_dev.\n");
    }

    return err;
}

static int __uac_deinit(void)
{
    if (uac_dev && uac_dev->deinit) {
        uac_dev->deinit();
        uac_stream_unregister();

        return 0;
    }

    return -1;
}

static int __uac_open(void)
{
    if (uac_dev && uac_dev->open) {
        return uac_dev->open();
    } else {
        uac_loge("Invalid uac_dev.\n");
    }

    return -1;
}

static int __uac_close(void)
{
    if (uac_dev && uac_dev->close) {
        return uac_dev->close();
    } else {
        uac_loge("Invalid uac_dev.\n");
    }

    return -1;
}

static int __uac_run(void)
{
    if (uac_dev && uac_dev->run) {
        return uac_dev->run();
    } else {
        uac_loge("Invalid uac_dev.\n");
    }

    return 0;
}

static int __uac_stop(void)
{
    if (uac_dev && uac_dev->stop) {
        return uac_dev->stop();
    } else {
        uac_loge("Invalid uac_dev.\n");
    }

    return 0;
}

/* ---------------------------------------------------------------------- */

static uac_t __uac = {
    .init = &__uac_init,
    .deinit = &__uac_deinit,
    .open = &__uac_open,
    .close = &__uac_close,
    .run = &__uac_run,
    .stop = &__uac_stop,
};

uac_t *get_uac(void)
{
    return &__uac;
}
