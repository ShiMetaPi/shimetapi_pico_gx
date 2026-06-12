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


#define DEV_FILE "/dev/lsadc"

#define LSADC_GLITCH_SAMPLE_MS (7)

typedef enum {
    LSADC_SCAN_MODE_SINGLE_STEP,
    LSADC_SCAN_MODE_CONTINUOUS,
    LSADC_SCAN_MODE_BUTT
} lsadc_scan_mode;

static xmedia_s32 lsadc_check_glitch
(xmedia_s32 value_arr[13], xmedia_s32 start_pos, xmedia_s32 end_pos, xmedia_s32 *lsadc_equ_vaule)
{
    xmedia_s32 j;
    xmedia_s32 last_sum = 0;
    xmedia_s32 start_value;
    const xmedia_s32 LSADC_DATA_DELTA = 0xc;
    xmedia_s32 arr_len = 13;
    xmedia_s32 len = end_pos - start_pos + 1;
    if (start_pos < 0 || start_pos >= arr_len || len <= 0 || end_pos >= arr_len || NULL == lsadc_equ_vaule) {
        fprintf(stderr, "fail to check! start_pos:%d end_pos:%d\n", start_pos, end_pos);
        return 0;
    }
    start_value = value_arr[start_pos];
    for(j = start_pos; j <= end_pos; j++) {
        last_sum += value_arr[j];
    }

    *lsadc_equ_vaule = last_sum / len;
    if (*lsadc_equ_vaule - start_value > LSADC_DATA_DELTA
     || start_value - *lsadc_equ_vaule > LSADC_DATA_DELTA) {
        return 1;
    }
    return 0;
}

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

    printf("[%s] LSADC_GLITCH_SAMPLE_MS=%d\n", __FUNCTION__, LSADC_GLITCH_SAMPLE_MS);

    for(i=0; i < 20; i++)
    {
        xmedia_s32 j, k;
        const xmedia_s32 gnum = LSADC_GLITCH_SAMPLE_MS;
        xmedia_s32 last_value[gnum + 6];
        xmedia_s32 lsadc_equ_vaule;
        xmedia_s32 len = sizeof(last_value) / sizeof(last_value[0]);
        for(j = 0; j < len; j++) {
            if(ioctl(fd, LSADC_IOC_START) < 0)  {
                fprintf(stderr, "start lsadc error.\n");
                goto exit;
            }

            value = ioctl(fd, LSADC_IOC_GET_CHNVAL, &chn);
            last_value[j] = value;
            if (0 == j)
                printf("[%d][%d] get value:%d,chn[%d]\n", i, j, value, chn);
            if (j >= (gnum - 1)) {
                k = j - (gnum - 1);
                if (lsadc_check_glitch(last_value, k, j, &lsadc_equ_vaule)) {
                    printf("[%d][%d] is glitch, value:%d equ_vaule:%d chn[%d]\n", i, k, last_value[k], lsadc_equ_vaule, chn);
                }
            }

            if(ioctl(fd, LSADC_IOC_STOP) <0)  {
                fprintf(stderr, "stop lsadc error.\n");
            }
        }
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
        xmedia_s32 j, k;
        const xmedia_s32 gnum = LSADC_GLITCH_SAMPLE_MS;
        xmedia_s32 last_value[gnum + 6];
        xmedia_s32 lsadc_equ_vaule;
        xmedia_s32 len = sizeof(last_value) / sizeof(last_value[0]);
        for(j = 0; j < len; j++) {
            value = ioctl(fd, LSADC_IOC_GET_CHNVAL, &chn);
            last_value[j] = value;
            if (0 == j)
                printf("[%d][%d] get value:%d,chn[%d]\n", i, j, value, chn);
            if (j >= (gnum - 1)) {
                k = j - (gnum - 1);
                if (lsadc_check_glitch(last_value, k, j, &lsadc_equ_vaule)) {
                    printf("[%d][%d] is glitch, value:%d equ_vaule:%d chn[%d]\n", i, k, last_value[k], lsadc_equ_vaule, chn);
                }
            }

            usleep(1*1000);
        }
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
    printf(" - [0] is CHN[0]\n");
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

