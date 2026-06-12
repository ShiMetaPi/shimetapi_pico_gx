/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "xmedia_type.h"
#include "xmedia_debug.h"
#include "xmedia_vgs.h"
#include "sample_comm.h"

xmedia_s32 sample_comm_vgs_init(xmedia_void)
{
    return xmedia_vgs_init();
}

xmedia_void sample_comm_vgs_exit(xmedia_void)
{
    xmedia_vgs_exit();
}



