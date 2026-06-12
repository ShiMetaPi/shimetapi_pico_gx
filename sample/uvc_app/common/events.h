/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __EVENTS_H_
#define __EVENTS_H_

int events_init(void);
int events_deinit(void);
int events_wait(void);
int events_wait_timeout_ms(unsigned long long timeout_ms);
int events_notity_one(void);

int node_wait(void);
int node_wait_timeout_ms(unsigned long long timeout_ms);
int node_notity_one(void);

#endif
