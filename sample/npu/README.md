文件说明

1.npu/xmm/test_xmm.c
    常规模型测试样例

2.npu/xmm_stat/xmm_stat.c
    输出模型的性能数据测试样例

3.npu/xmm_submit/xmm_submit.c
    调用回调函数测试样例

4.npu/xmm_batch/xmm_batch.c
    动态batch功能测试样例

5.npu/xmm_profiling/xmm_profiling.c
    启用profiling工具测试样例,需要模型匹配的cycle_bin_file.bin

6.npu/xmm_workspace/xmm_workspace.c
    多模型workspace共用测试样例

7.npu/xmm_memory_reuse/xmm_memory_reuse.c
    内存复用测试样例
    含四种模式：0，不复用
                1，workspace与input复用
                2，workspace与output复用
                3, workspace,input,output三者复用

8.npu/xmm_private_data/xmm_private_data.c
    获取模型私有数据测试样例
    
