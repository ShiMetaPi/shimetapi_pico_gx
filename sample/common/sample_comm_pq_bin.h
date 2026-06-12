#ifndef _SAMPLE_COMM_PQ_BIN_H_
#define _SAMPLE_COMM_PQ_BIN_H_

#include "sample_comm.h"

#define MAGIC_DATA_LEN 16

typedef struct vi_info {
    xmedia_s32 enable;
    xmedia_s32 pipe;
} bin_vi_info;

typedef struct vpss_info {
    xmedia_s32 enable;
    xmedia_s32 pipe;
} bin_vpss_info;

typedef struct mcf_info {
    xmedia_s32 enable;
    xmedia_s32 pipe;
    xmedia_s32 ichn;
} bin_mcf_info;

typedef struct module_info {
    bin_vi_info vi;
    bin_vpss_info vpss;
    bin_mcf_info mcf;
} bin_module_info;

typedef enum opration_select {
    PQ_BIN_EXPORT = 0,
    PQ_BIN_IMPORT,
    PQ_BIN_BUTT
} bin_ops;

extern xmedia_u32 g_isp_len;

static const xmedia_u8 magic_isp[MAGIC_DATA_LEN] = "ISP_BIN";

xmedia_u32 get_pq_bin_length(xmedia_u8 *para);
xmedia_s32 pq_bin_data_export(bin_module_info *module, xmedia_u8 *buffer, xmedia_u32 total_len);
xmedia_s32 pq_bin_data_import(bin_module_info *module, xmedia_u8 *buffer, xmedia_u32 total_len);
xmedia_s32 sample_comm_pq_bin_import(bin_module_info *module, xmedia_char *file_name);
xmedia_s32 sample_comm_pq_bin_export(bin_module_info *module, xmedia_u8 *buffer,
    xmedia_u32 total_length, xmedia_char *file_name);

#endif // _SAMPLE_COMM_PQ_BIN_H_

