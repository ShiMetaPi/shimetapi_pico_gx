#ifndef _GH_NPU_H
#define _GH_NPU_H

#ifdef __cplusplus
extern "C" {
#endif
typedef int s32;
typedef unsigned int u32;

s32 gh_npu_set_wt_latency_enable(u32 dev_fd, u32 enble);
s32 gh_npu_set_rd_latency_enable(u32 dev_fd, u32 enble);
s32 gh_npu_set_wt_bw_enable(u32 dev_fd, u32 enble);
s32 gh_npu_set_rd_bw_enable(u32 dev_fd, u32 enble);
s32 gh_npu_get_wt_latency_sum(u32 dev_fd, u32 *latency);
s32 gh_npu_get_wt_latency_burst_sum(u32 dev_fd);
s32 gh_npu_get_rd_latency_sum(u32 dev_fd, u32 *latency);
s32 gh_npu_get_rd_latency_burst_sum(u32 dev_fd);
s32 gh_npu_get_wt_bw_sum(u32 dev_fd, u32 *latency);
s32 gh_npu_get_rd_bw_sum(u32 dev_fd, u32 *latency);
s32 gh_npu_get_cfg_running_cycle_cnt(u32 index);

#ifdef __cplusplus
}
#endif

#endif 
