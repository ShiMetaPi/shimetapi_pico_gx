/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#include "config_svc.h"
#include "iniparser.h"
#include "log_base.h"

#define CONFIG_MAX_PATH 256

typedef struct {
    char* config_file;
} config_svc_t;

static config_svc_t* __config_svc = NULL;

int create_config_svc(char *config_path)
{
    assert(config_path != NULL);

    if (__config_svc != NULL) {
        free(__config_svc);
    }

    __config_svc = (config_svc_t*)malloc(sizeof(config_svc_t));
    if (__config_svc == NULL) {
        LOGE("create config svc failure\n");
        goto ERR;
    }

    __config_svc->config_file = malloc(sizeof(char) * CONFIG_MAX_PATH);
    if (__config_svc->config_file == NULL) {
        LOGE("malloc config file memory failure\n");
        goto ERR;
    }

    if (strlen(config_path) > CONFIG_MAX_PATH) {
        LOGE("config path is too length\n");
        goto ERR;
    }

    strcpy(__config_svc->config_file, config_path);

    return 0;

ERR:
    if (__config_svc) {
        if (__config_svc->config_file) {
            free(__config_svc->config_file);
        }

        free(__config_svc);
    }

    __config_svc = NULL;

    return -1;
}

void release_cofnig_svc()
{
    if (__config_svc) {
        if (__config_svc->config_file) {
            free(__config_svc->config_file);
        }

        free(__config_svc);
    }

    __config_svc = NULL;
}

int get_config_value(const char *key, int default_value)
{
    dictionary *config_dic;
    int value = default_value;

    assert(key != NULL);
    assert(__config_svc != NULL);

    config_dic = iniparser_load(__config_svc->config_file);

    if (config_dic == NULL) {
        LOGE("open config file failuer[%s], returen default value:%d\n", __config_svc->config_file, default_value);
        return value;
    }

    value = iniparser_getint(config_dic, key, default_value);
    iniparser_freedict(config_dic);

    return value;
}

int get_config_string(const char *key, char *buf, int size)
{
    dictionary *config_dic;
    int copy_size;
    int ret = -1;
    char* value = NULL;

    assert(key != NULL);
    assert(buf != NULL);
    assert(__config_svc != NULL);

    config_dic = iniparser_load(__config_svc->config_file);
    if (config_dic == NULL) {
        LOGE("open config file failuer[%s], returen NULL\n", __config_svc->config_file);
        return ret;
    }

    value = iniparser_getstring(config_dic, key, NULL);
    if (value != NULL) {
        copy_size = strlen(value);
        if (copy_size <= size) {
            memcpy(buf, value, copy_size);
            ret = 0;
        } else {
            LOGE("buffer is small too get %s.\n", key);
        }
    } else {
        LOGD("getstring (%s) fail.\n", key);
    }

    iniparser_freedict(config_dic);

    return ret;
}
