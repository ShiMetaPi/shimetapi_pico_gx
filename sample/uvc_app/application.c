/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#include "camera.h"
#include "frame_cache.h"
#include "config_svc.h"

#include "log_base.h"

unsigned int g_uvc = 1;
unsigned int g_bulk = 0;
unsigned int g_uac = 0;
unsigned int g_loglevel = LOG_LEVEL_INFO;

volatile sig_atomic_t g_need_quit_flag = 0;

static void sample_uvc_usage(char *process)
{
    printf("Usage : %s <param>\n", process);
    printf("param:\n");
    printf("\t -h            --for help.\n");
    printf("\t -bulkmode     --use uvc bulkmode.\n");
    printf("\t -uac          --enable uac.\n");
    printf("\t -loglevel x   --set the log level to x (0 ~ 5).\n");
    printf("\n");
}

void sample_uvc_handle_signal(int signo)
{

    switch (signo) {
        case SIGINT:
        case SIGTERM:
            signal(SIGINT, SIG_IGN);
            signal(SIGTERM, SIG_IGN);
            signal(SIGUSR1, SIG_IGN);
            signal(SIGUSR2, SIG_IGN);
            g_need_quit_flag = 1;
            return;

        case SIGUSR1:
            if (g_loglevel > 0)
                g_loglevel--;
            break;

        case SIGUSR2:
            if (g_loglevel < LOG_LEVEL_DEBUG)
                g_loglevel++;
            break;

        default:
            break;
    }

    printf("[UVC_APP] Change the log level to %d\n", g_loglevel);
}

static int create_cache(void)
{
    if (create_uvc_cache() != 0) {
        return -1;
    }

    return 0;
}

static void destroy_cache(void)
{
    destroy_uvc_cache();

    return;
}

static int run_camera(void)
{
    if (get_camera()->init() != 0) {
        return -1;
    }

    if (get_camera()->open() != 0) {
        get_camera()->deinit();

        return -1;
    }

    if (get_camera()->run() != 0) {
        get_camera()->close();
        get_camera()->deinit();

        return -1;
    }

    return 0;
}

void stop_camera(void)
{
    get_camera()->stop();
    get_camera()->close();
    get_camera()->deinit();

    return;
}

int main(int argc, char *argv[])
{
    int i = 1;

    printf("\n@@@@@ UVC App Sample @@@@@\n\n");

    while (i < argc) {
        if (strcmp(argv[i], "-bulkmode") == 0) {
            g_bulk = 1;
        }

        if (strcmp(argv[i], "-uac") == 0) {
            g_uac = 1;
        }

        if (strcmp(argv[i], "-loglevel") == 0) {
            i++;
            g_loglevel = atoi(argv[i]);
        }

        if (strcmp(argv[i], "-h") == 0) {
            sample_uvc_usage(argv[0]);
            exit(0);
        }

        i++;
    }

    signal(SIGINT, sample_uvc_handle_signal);
    signal(SIGTERM, sample_uvc_handle_signal);
    signal(SIGUSR1, sample_uvc_handle_signal);
    signal(SIGUSR2, sample_uvc_handle_signal);

    if (create_config_svc("./uvc_app.conf") != 0) {
        goto EXIT0;
    }

    if (create_cache() != 0) {
        goto EXIT1;
    }

    if (run_camera() != 0) {
        goto EXIT2;
    }

    while(!g_need_quit_flag) {
        sleep(2);
    }

EXIT2:
    stop_camera();
    destroy_cache();

EXIT1:
    release_cofnig_svc();

EXIT0:
    printf("uvc_app exit!\n");

    return 0;
}
