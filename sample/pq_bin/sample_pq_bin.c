/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <limits.h>
#include <time.h>

#include "xmedia_sys.h"
#include "xmedia_isp.h"
#include "xmedia_vi.h"

#include "sample_comm_pq_bin.h"

xmedia_s32 sample_pq_bin_module_init(xmedia_void)
{
    xmedia_s32 ret;
    ret = xmedia_sys_init(NULL);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_sys_init err(%#x)!\n", ret);
    }

    ret = xmedia_vi_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_vi_init err(%#x)!\n", ret);
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_pq_bin_module_exit(xmedia_void)
{
    xmedia_s32 ret;

    ret = xmedia_vi_exit();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_vi_exit err(%#x)!\n", ret);
    }

    xmedia_sys_exit();

    return XMEDIA_SUCCESS;
}

xmedia_void sample_pq_bin_create_file_name(xmedia_char *name, bin_module_info mod)
{
    time_t now;
    struct tm *local;
    xmedia_s32 ret;
    xmedia_char date[80];

    time(&now);
    local = localtime(&now);
    strftime(date, sizeof(date), "%Y%m%d%H%M%S", local);

    ret = sprintf(name, "PQBin_");
    if (mod.vi.enable)
        ret += sprintf(name + ret, "isp_");

    if (mod.vpss.enable)
        ret += sprintf(name + ret, "vpss_");

    if (mod.mcf.enable)
        ret += sprintf(name + ret, "mcf_");

    sprintf(name + ret, "%s.bin", date);

    return;
}

xmedia_void sample_pq_bin_usage(xmedia_char *args)
{
    SAMPLE_ERR("\n"
           "****************************************************************************************\n"
           "Usage: %s [pipe] [file]\n"
           "pipe:     vi pipe.\n"
           "file:     bin data file path.\n\n"
           "e.g:\n"
           "export: %s 0\n"
           "import: %s 0 PQBin_xxx.bin\n"
           "***************************************************************************************\n"
           "\n", args, args, args);
    return;
}

xmedia_s32 sample_pq_bin_cmd_args_analysis(xmedia_s32 argc, xmedia_char **argv, xmedia_u8 cmd_args[],
    bin_module_info *module)
{
    xmedia_u32 offSet = 0;

    if (argc < 2 || argc > 3) {
        sample_pq_bin_usage(argv[0]);
        return XMEDIA_FAILURE;
    }

    module->vi.enable = XMEDIA_TRUE;
    module->vpss.enable = XMEDIA_FALSE;
    module->mcf.enable = XMEDIA_FALSE;

    module->vi.pipe = atoi(argv[1]);
    module->vpss.pipe = 0;
    module->mcf.pipe = 0;
    module->mcf.ichn = 0;

    *(xmedia_s32 *)(cmd_args + offSet) = module->vi.pipe;
    offSet += sizeof(xmedia_s32);

    *(xmedia_s32 *)(cmd_args + offSet) = module->vpss.pipe;
    offSet += sizeof(xmedia_s32);

    *(xmedia_s32 *)(cmd_args + offSet) = module->mcf.pipe;
    offSet += sizeof(xmedia_s32);

    *(xmedia_s32 *)(cmd_args + offSet) = module->mcf.ichn;
    offSet += sizeof(xmedia_s32);

    *(xmedia_s32 *)(cmd_args + offSet) = module->vi.enable;
    offSet += sizeof(xmedia_s32);

    *(xmedia_s32 *)(cmd_args + offSet) = module->vpss.enable;
    offSet += sizeof(xmedia_s32);

    *(xmedia_s32 *)(cmd_args + offSet) = module->mcf.enable;

    return XMEDIA_SUCCESS;
}

int main(int argc, char **argv)
{
    xmedia_s32 ret;
    xmedia_u32 total_length;
    xmedia_char file_name[200];
    xmedia_u8 *buffer;
    xmedia_u8 cmd_args[64];
    bin_ops import;
    bin_module_info module = { 0 };

    if (argv[2]) {
        import = PQ_BIN_IMPORT;
    } else {
        import = PQ_BIN_EXPORT;
    }

    ret = sample_pq_bin_cmd_args_analysis(argc, argv, cmd_args, &module);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    (xmedia_void)sample_pq_bin_module_init();

    if (import == PQ_BIN_IMPORT) {
        ret = sample_comm_pq_bin_import(&module, argv[2]);
        if (ret != XMEDIA_SUCCESS) {
            goto err;
        }
    } else if(import == PQ_BIN_EXPORT) {
        total_length = get_pq_bin_length(cmd_args);
        if(total_length <= 0) {
            (xmedia_void)sample_pq_bin_module_exit();
            SAMPLE_ERR("total length error, must be greater the 0.\n");
            return XMEDIA_FAILURE;
        }

        buffer = (xmedia_u8 *)malloc(total_length);
        if (buffer == XMEDIA_NULL) {
            (xmedia_void)sample_pq_bin_module_exit();
            SAMPLE_ERR("malloc err!\n");
            return XMEDIA_FAILURE;
        }

        sample_pq_bin_create_file_name(&file_name[0], module);
        memset(buffer, 0, total_length);

        ret = sample_comm_pq_bin_export(&module, buffer, total_length, file_name);
        if (ret != XMEDIA_SUCCESS) {
            goto err;
        }
    } else {
        SAMPLE_ERR("Unsupported operation.\n");
    }

err:
    (xmedia_void)sample_pq_bin_module_exit();

    if(import == PQ_BIN_EXPORT) {
        free(buffer);
        buffer = XMEDIA_NULL;
    }

    return ret;
}
