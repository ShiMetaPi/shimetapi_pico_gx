/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __CONFIG_SERVICE_H__
#define __CONFIG_SERVICE_H__

int  create_config_svc(char* config_path);
void release_cofnig_svc();
int get_config_value(const char *key, int default_value);
int get_config_string(const char *key, char *buf, int size);

#endif //__CONFIG_SERVICE_H__
