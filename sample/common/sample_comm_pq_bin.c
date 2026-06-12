/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "xmedia_sys.h"
#include "xmedia_isp.h"
#include "xmedia_vi.h"
#include "sample_comm.h"
#include "sample_comm_pq_bin.h"

xmedia_u32 g_isp_len = 0;

/**************************** isp bin data content ***************************
 *
 *  magic_isp : 16byte
 *  pipe      : sizeof(xmedia_s32)
 *  isp_data  : sizeof(xmedia_isp_module_info * 4) +
 *              xmedia_isp_module_info.length[i] * XMEDIA_ISP_MODULE_MAX_NUM
 *
 *****************************************************************************/
xmedia_u32 get_isp_export_len(xmedia_void)
{
    return g_isp_len;
}

xmedia_u32 pq_bin_isp_export(bin_module_info *module, xmedia_isp_module_info *isp_mod_info)
{
    xmedia_s32 ret;

    isp_mod_info[0].module = XMEDIA_ISP_MODULE_ID_ISP;
    isp_mod_info[1].module = XMEDIA_ISP_MODULE_ID_AWB;
    isp_mod_info[2].module = XMEDIA_ISP_MODULE_ID_AE;
    //isp_mod_info[3].module = XMEDIA_ISP_MODULE_ID_AF;

    ret = xmedia_isp_export(module->vi.pipe, isp_mod_info);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_isp_export err(%#x)\n", ret);
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

xmedia_u32 get_isp_data_len(bin_module_info *module)
{
    xmedia_s32 ret;
    xmedia_isp_module_info isp_mod_info[XMEDIA_ISP_MODULE_MAX_NUM] = { 0 };

    g_isp_len = MAGIC_DATA_LEN;
    g_isp_len += sizeof(xmedia_s32);
    g_isp_len += sizeof(xmedia_isp_module_info) * XMEDIA_ISP_MODULE_MAX_NUM;

    ret = pq_bin_isp_export(module, isp_mod_info);
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    for (xmedia_s32 i = 0; i < XMEDIA_ISP_MODULE_MAX_NUM; i++) {
        SAMPLE_PRT("isp[%d].length = %d\n", i, isp_mod_info[i].length);
        g_isp_len += isp_mod_info[i].length;
    }

    return g_isp_len;
}

xmedia_u32 get_isp_import_len(xmedia_u8* buffer)
{
    xmedia_u32 len;
    xmedia_u8 *pos = buffer;
    xmedia_isp_module_info isp_mod_info[XMEDIA_ISP_MODULE_MAX_NUM] = { 0 };

    pos += MAGIC_DATA_LEN;
    pos += sizeof(xmedia_s32);

    memcpy(&isp_mod_info, pos, sizeof(isp_mod_info));
    pos += sizeof(isp_mod_info);

    for (xmedia_s32 index = 0; index < XMEDIA_ISP_MODULE_MAX_NUM; index++) {
        SAMPLE_PRT("isp[%d].length=%d\n", index, isp_mod_info[index].length);
        pos += isp_mod_info[index].length;
    }

    len = pos - buffer;

    return len;
}

xmedia_s32 pq_bin_export_isp_data(bin_module_info *module, xmedia_u8 *buffer, xmedia_u32 isp_data_len)
{
    xmedia_u8 *pos = buffer;
    xmedia_isp_module_info isp_mod_info[XMEDIA_ISP_MODULE_MAX_NUM] = { 0 };
    xmedia_s32 ret;

    CHECK_NULL_PTR(module);
    CHECK_NULL_PTR(buffer);

    memcpy(pos, magic_isp, MAGIC_DATA_LEN);
    pos += MAGIC_DATA_LEN;

    memcpy(pos, &module->vi.pipe, sizeof(xmedia_s32));
    pos += sizeof(xmedia_s32);

    ret = pq_bin_isp_export(module, isp_mod_info);
    if (ret != XMEDIA_SUCCESS) {
        return XMEDIA_FAILURE;
    }

    memcpy(pos, &isp_mod_info, sizeof(isp_mod_info));
    pos += sizeof(isp_mod_info);

    for (xmedia_s32 index = 0; index < XMEDIA_ISP_MODULE_MAX_NUM; index++)    {
        memcpy(pos, isp_mod_info[index].addr, isp_mod_info[index].length);
        pos += isp_mod_info[index].length;
    }

    if (isp_data_len != (pos - buffer)) {
        SAMPLE_ERR("The exported length does not match the expected length.\n");
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 pq_bin_import_isp_data(bin_module_info *module, xmedia_u8* buffer)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_u8 *pos = buffer;
    xmedia_isp_module_info isp_mod_info[XMEDIA_ISP_MODULE_MAX_NUM] = { 0 };

    pos += MAGIC_DATA_LEN;
    pos += sizeof(xmedia_s32);

    memcpy(&isp_mod_info, pos, sizeof(isp_mod_info));
    pos += sizeof(isp_mod_info);

    for (xmedia_s32 index = 0; index < XMEDIA_ISP_MODULE_MAX_NUM; index++) {
        isp_mod_info[index].addr = pos;
        pos += isp_mod_info[index].length;
    }

    ret = xmedia_isp_import(module->vi.pipe, isp_mod_info);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("xmedia_isp_import failed.\n");
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 pq_bin_data_export(bin_module_info *module, xmedia_u8 *buffer, xmedia_u32 total_len)
{
    xmedia_s32 ret;
    xmedia_u32 len;
    xmedia_s32 result = -1;
    xmedia_u8 *pos = buffer;

    xmedia_sys_get_tuning_connect(&result);
    if (result == 0 || result == -1) {
        SAMPLE_ERR("xmedia_sys_get_tuning_connect failed\n");
        return XMEDIA_FAILURE;
    }

    len = get_isp_export_len();

    // Export ISP Data
    ret = pq_bin_export_isp_data(module, buffer, len);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("pq_bin_export_isp_data failed\n");
        return ret;
    }

    buffer += len;

    if (total_len != buffer - pos) {
        SAMPLE_ERR("The exported length does not match the expected length.\n");
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

xmedia_s32 pq_bin_data_import(bin_module_info *module, xmedia_u8 *buffer, xmedia_u32 total_len)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_u32 len;
    xmedia_s32 result = -1;

    if (buffer == XMEDIA_NULL) {
        SAMPLE_ERR("buffer is a null pointer.\n");
        return XMEDIA_FAILURE;
    }

    xmedia_sys_get_tuning_connect(&result);
    if (result == 0 || result == -1) {
        SAMPLE_ERR("xmedia_sys_get_tuning_connect failed\n");
        return XMEDIA_FAILURE;
    }

    //isp data
    if (total_len > sizeof(magic_isp) && memcmp(&magic_isp, buffer, sizeof(magic_isp)) == 0) {
        len = get_isp_import_len(buffer);
        total_len -= len;
        SAMPLE_ERR("pq_bin isp data size: %d\n", len);

        if (total_len < 0) {
            SAMPLE_ERR("pq_bin data is incomplete!\n");
            return XMEDIA_FAILURE;
        }

        ret = pq_bin_import_isp_data(module, buffer);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_ERR("pq_bin import isp data failed\n");
            return ret;
        }
        SAMPLE_ERR("pq_bin import isp data success\n");

        buffer += len;
    } else {
        SAMPLE_ERR("BIN file error.\n");
        return XMEDIA_FAILURE;
    }

    return XMEDIA_SUCCESS;
}

xmedia_u32 get_pq_bin_length(xmedia_u8 *para)
{
    xmedia_u32 offSet = 0;
    xmedia_u32 length = 0;
    bin_module_info mod = { 0 };

    mod.vi.pipe = *(xmedia_s32 *)(para + offSet);
    offSet += sizeof(xmedia_s32);

    length = get_isp_data_len(&mod);
    SAMPLE_PRT("Export ISP  data total len: %u\n", length);

    return length;
}

xmedia_s32 sample_comm_pq_bin_export(bin_module_info *module, xmedia_u8 *buffer,
    xmedia_u32 total_length, xmedia_char *file_name)
{
    xmedia_s32 ret;
    xmedia_u32 tmp_len;
    FILE *fd = XMEDIA_NULL;

    fd = fopen(file_name, "wb+");
    if (fd == XMEDIA_NULL) {
        SAMPLE_ERR("pq bin file open failed.\n");
        ret = -1;
        goto err;
    }

    ret = pq_bin_data_export(module, buffer, total_length);
    if (ret != 0) {
        SAMPLE_ERR("pq_bin_data_export err(%#x)!\n", ret);
    } else {
        tmp_len = fwrite(buffer, 1, total_length, fd);
        if (tmp_len != total_length) {
            SAMPLE_ERR("write pq bin file err!\n");
            ret = -1;
            goto err;
        }

        SAMPLE_ERR("\nexport bin data success!\n");
    }

err:
    if (fd != XMEDIA_NULL) {
        fclose(fd);
    }

    return ret;
}

xmedia_s32 sample_comm_pq_bin_import(bin_module_info *module, xmedia_char *file_name)
{
    xmedia_s32 ret = XMEDIA_FAILURE;
    xmedia_u32 size;
    xmedia_u32 tmp_len;
    xmedia_u8 *buffer = XMEDIA_NULL;
    FILE *fd = XMEDIA_NULL;

    fd = fopen(file_name, "r");
    if (fd == XMEDIA_NULL) {
        SAMPLE_ERR("No such file.\n");
        return XMEDIA_FAILURE;
    }

    fseek(fd, 0, SEEK_END);
    size = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    buffer = (xmedia_u8 *)malloc(size);
    if (buffer == XMEDIA_NULL) {
        SAMPLE_ERR("buffer malloc fail\n");
        fclose(fd);
        return XMEDIA_FAILURE;
    }

    tmp_len = fread(buffer, sizeof(xmedia_u8), size, fd);
    if (tmp_len == 0) {
        SAMPLE_ERR("read error\n");
        goto err;
    }

    ret = pq_bin_data_import(module, buffer, size);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_ERR("pq_bin_data_import err(%#x)\n", ret);
    } else {
        SAMPLE_ERR("\nimport bin data success!\n");
    }

err:
    fclose(fd);
    free(buffer);
    buffer = XMEDIA_NULL;

    return ret;
}
