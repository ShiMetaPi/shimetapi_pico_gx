// SPDX-License-Identifier: GPL-2.0
/* Copyright (c) LOTUS. All rights reserved. */

#ifndef __UPDATE_COMM_H__
#define __UPDATE_COMM_H__

#define UP_TRUE  1
#define UP_FALSE 0

#define UP_SUCCESS 0
#define UP_FAILURE -1

#define AU_DEBUG 1
#undef debug_print
#ifdef AU_DEBUG
#define debug_print(fmt, args...) printf(fmt, ##args)
#else
#define debug_print(fmt, args...)
#endif  /* AU_DEBUG */

#endif
