/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "xmedia_type.h"
#include "xmedia_cipher.h"
#include "sample_common.h"

int sample_rng(xmedia_void)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 index = 0;
    xmedia_u32 random_number = 0;

    ret = xmedia_cipher_init();
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("xmedia_cipher_init ERROR!\n");
        return ret;
    }

    for (index = 0; index < 10; index++) {
        ret = xmedia_cipher_get_random_number(&random_number);
        if (XMEDIA_SUCCESS != ret) {
            ERR_CODE_CIPHER("There is No ramdom number available now. Try again!\n");
            index--;
            continue;
        }

        printf("Random number: %08x\n", random_number);
    }

    ret = xmedia_cipher_exit();
    if (XMEDIA_SUCCESS != ret) {
        ERR_CODE_CIPHER("xmedia_cipher_exit error! return value = %d.\n", ret);
        return ret;
    }

    return ret;
}

