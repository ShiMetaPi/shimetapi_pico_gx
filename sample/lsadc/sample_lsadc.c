/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "adc.h"

//#define USE_LSADC_CHANNEL_0

#define DEV_FILE "/dev/lsadc"

typedef enum {
    LSADC_SCAN_MODE_SINGLE_STEP,
    LSADC_SCAN_MODE_CONTINUOUS,
    LSADC_SCAN_MODE_BUTT
} lsadc_scan_mode;

xmedia_s32 sample_lsadc_single_step_scan_mode(lsadc_scan_mode mode, xmedia_s32 chn)
{
    xmedia_s32 i, value;
    xmedia_s32 fd  = open(DEV_FILE, O_RDWR);

    if (fd < 0) {
        fprintf(stderr, "fail to open file:%s\n", DEV_FILE);
        return -1;
    }

    if(ioctl(fd, LSADC_IOC_MODEL_SEL, &mode) < 0) {
        fprintf(stderr, "adc model select error.\n");
        goto exit;
    }

    if(ioctl(fd, LSADC_IOC_CHN_ENABLE, &chn) < 0) {
        fprintf(stderr, "enable chn %d error.\n", chn);
        goto exit;
    }


    for(i=0; i < 20; i++) {
        if(ioctl(fd, LSADC_IOC_START) < 0) {
            fprintf(stderr, "start lsadc error.\n");
            goto exit;
        }

        value = ioctl(fd, LSADC_IOC_GET_CHNVAL, &chn);
        printf("get value:%d,chn[%d]\n", value,chn);
        usleep(1000*1000);

        if(ioctl(fd, LSADC_IOC_STOP) < 0) {
            fprintf(stderr, "stop lsadc error.\n");
        }
    }

exit:
    if(ioctl(fd, LSADC_IOC_STOP) < 0) {
        fprintf(stderr, "stop lsadc error.\n");
    }

    if(ioctl(fd, LSADC_IOC_CHN_DISABLE, &chn) < 0) {
        fprintf(stderr, "disable chn %d error.\n", chn);
    }

    close(fd);
    return 0;
}

xmedia_s32 sample_lsadc_continuous_scan_mode(lsadc_scan_mode mode, xmedia_s32 chn)
{
    xmedia_s32 i, value;
    xmedia_s32 fd  = open(DEV_FILE, O_RDWR);

    if (fd < 0) {
        fprintf(stderr, "fail to open file:%s\n", DEV_FILE);
        return -1;
    }

    if(ioctl(fd, LSADC_IOC_MODEL_SEL, &mode) < 0) {
        fprintf(stderr, "adc model select error.\n");
        goto exit;
    }

    if(ioctl(fd, LSADC_IOC_CHN_ENABLE, &chn) < 0) {
        fprintf(stderr, "enable chn %d error.\n", chn);
        goto exit;
    }

    if(ioctl(fd, LSADC_IOC_START) < 0) {
        fprintf(stderr, "start lsadc error.\n");
        goto exit;
    }

    for(i = 0; i < 20; i++) {
        value = ioctl(fd, LSADC_IOC_GET_CHNVAL, &chn);
        printf("get value:%d,chn[%d]\n", value,chn);
        usleep(1000*1000);
    }

exit:
    if(ioctl(fd, LSADC_IOC_STOP) < 0) {
        fprintf(stderr, "stop lsadc error.\n");
    }

    if(ioctl(fd, LSADC_IOC_CHN_DISABLE, &chn) < 0) {
        fprintf(stderr, "disable chn %d error.\n", chn);
    }

    close(fd);
    return 0;
}

#ifdef __LITEOS__
xmedia_s32 app_main(xmedia_s32 argc, char *argv[])
#else
xmedia_s32 main(xmedia_s32 argc, char* argv[])
#endif
{
    lsadc_scan_mode mode = LSADC_SCAN_MODE_BUTT;
    xmedia_s32 chn = 0;

    printf("select scan mode:\n");
    printf(" - [0] is single step scan mode\n");
    printf(" - [1] is Continuous scan mode\n");

    mode = getchar() - '0';

    getchar();
    printf("select ADC CHN:\n");
#ifdef USE_LSADC_CHANNEL_0
    printf(" - [0] is CHN[0]\n");
#endif
    printf(" - [1] is CHN[1]\n");
    printf(" - [2] is CHN[2]\n");

    chn = getchar() - '0';

    switch (mode) {
        case LSADC_SCAN_MODE_SINGLE_STEP:
            sample_lsadc_single_step_scan_mode(mode, chn);
            break;
        case LSADC_SCAN_MODE_CONTINUOUS:
            sample_lsadc_continuous_scan_mode(mode, chn);
            break;
        default:
            printf("error scan mode\n");
            break;
    }
    return 0;
}

