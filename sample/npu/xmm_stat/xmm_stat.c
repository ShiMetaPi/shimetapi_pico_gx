#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "xmedia_cl.h"
#include "xmedia_mmz.h"
#include "xmedia_sys.h"
#include <sys/time.h>
#include "ghal_npu.h"
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/mman.h>

#define ALIGN_BYTE 8
#define DEFAULT_INOUT_NUM 16
#define ALIGN_FUNC(A, ALIGN) \
    ((((A) % (ALIGN)) == 0) ? (A) : ((A) + (ALIGN) - ((A) % (ALIGN))))

int abs(int num)
{
    return num < 0 ? -num : num;
}

static int array_cmp_fixed(void *p_exp, void *p_got,
                           xmedia_cl_data_type dtype, // 0: unsigned, 1: signed
                           int len, char *info_label, int delta)
{
    int idx = 0;
    int first_error_idx = -1, first_expect_value = 0, first_got_value = 0;
    int max_error_idx = -1, max_expect_value = 0, max_got_value = 0;
    int max_error_value = 0, mismatch_cnt = 0;
    for (idx = 0; idx < len; idx++) {
        int error = 0;
        int exp_int = 0;
        int got_int = 0;
        if (dtype == XMEDIA_CL_INT8) {
            exp_int = (int)(*((char *)p_exp + idx));
            got_int = (int)(*((char *)p_got + idx));
        } else if (dtype == XMEDIA_CL_UINT8) {
            exp_int = (int)(*((unsigned char *)p_exp + idx));
            got_int = (int)(*((unsigned char *)p_got + idx));
        } else if (dtype == XMEDIA_CL_INT32) {
            exp_int = (int)(*((int *)p_exp + idx));
            got_int = (int)(*((int *)p_got + idx));
        } else {
        }
        error = abs(exp_int - got_int);
        if (error > 0) {
            if (first_error_idx == -1) {
                first_error_idx = idx;
                first_expect_value = exp_int;
                first_got_value = got_int;
            }
            if (error > max_error_value) {
                max_error_idx = idx;
                max_error_value = error;
                max_expect_value = exp_int;
                max_got_value = got_int;
            }
            mismatch_cnt++;
            //printf("%s mismatch at index %d exp %d got %d (delta %d)\n", info_label, idx, exp_int, got_int, delta);
        }
        if (error > delta) {
            printf("%s:     error      at index %d exp %d got %d\n", info_label,
                   idx, exp_int, got_int);
            printf("%s: first mismatch at index %d exp %d got %d (delta %d)\n",
                   info_label, first_error_idx, first_expect_value,
                   first_got_value, delta);
            printf("%s: total mismatch count %d (delta %d) \n", info_label,
                   mismatch_cnt, delta);
            printf("compare failed!!!\n");
            return -1;
        }
    }
    if (max_error_idx != -1) {
        printf("%s: first mismatch at index %d exp %d got %d (delta %d)\n",
               info_label, first_error_idx, first_expect_value, first_got_value,
               delta);
        printf("%s:  max  mismatch at index %d exp %d got %d (delta %d)\n",
               info_label, max_error_idx, max_expect_value, max_got_value,
               delta);
        printf("%s: total mismatch count %d (delta %d) \n", info_label,
               mismatch_cnt, delta);
    }
    printf("compare succeed!!!\n");
    return 0;
}

static xmedia_cl_s32 copy_file_to_buff(const char *src_file, char *des,
                                       unsigned int offset, unsigned int size)
{
    int ret;
    xmedia_cl_u32 count;
    FILE *fp = fopen(src_file, "r");

    if (fp == NULL) {
        return -1;
    }

    ret = fseek(fp, offset, SEEK_SET);
    if (ret != 0) {
        fclose(fp);
        return -1;
    }

    count = fread(des, 1, size, fp);
    if (count != size) {
        printf("want to read size = 0x%x, in fact count = 0x%x\n", size, count);
        fclose(fp);
        return -1;
    }
    fclose(fp);

    return XMEDIA_CL_SUCCESS;
}

static xmedia_cl_s32 malloc_inout_tensor_mem(xmedia_cl_tensor_info_inout *inout)
{
    inout->tensor = malloc(sizeof(xmedia_cl_tensor) * inout->num);
    inout->current_batch = malloc(sizeof(xmedia_cl_u32) * inout->num);
    inout->tensor_batch = malloc(sizeof(xmedia_cl_tensor_batch) * inout->num);

    if (inout->tensor == NULL || inout->current_batch == NULL 
        || inout->tensor_batch == NULL) {
        return XMEDIA_CL_OUT_OF_HOST_MEMORY;
    }

    return XMEDIA_CL_SUCCESS;
}

static void free_inout_tensor_mem(xmedia_cl_tensor_info_inout inout)
{
    if (inout.tensor != NULL) {
        free(inout.tensor);
    }
    if (inout.current_batch != NULL) {
        free(inout.current_batch);
    }
    if (inout.tensor_batch != NULL) {
        free(inout.tensor_batch);
    }
}


xmedia_s32 XMEDIA_API_SYS_MmzAlloc_Cached(xmedia_u64 *pu64PhyAddr,
                                          xmedia_void **ppVirAddr,
                                          const xmedia_char *strMmb,
                                          const xmedia_char *strZone,
                                          xmedia_u32 u32Len)
{
    *pu64PhyAddr = xmedia_mmz_alloc(strZone, strMmb, u32Len);
    if (*pu64PhyAddr == 0) {
        return XMEDIA_CL_OUT_OF_HOST_MEMORY;
    }

    *ppVirAddr = xmedia_mmz_map(*pu64PhyAddr, u32Len, 1);
    if (*ppVirAddr == NULL) {
        return XMEDIA_CL_OUT_OF_HOST_MEMORY;
    }

    return XMEDIA_CL_SUCCESS;
}

xmedia_s32 XMEDIA_API_SYS_MmzFlushCache(xmedia_u64 u64PhyAddr,
                                        xmedia_void *pVirAddr,
                                        xmedia_u32 u32Size)
{
    return xmedia_mmz_flush_cache(u64PhyAddr, pVirAddr, u32Size);
}

xmedia_s32 XMEDIA_API_SYS_MmzFree(xmedia_u64 u64PhyAddr, xmedia_void *pVirAddr)
{
    xmedia_cl_s32 ret;

    ret = xmedia_mmz_unmap(pVirAddr);
    if (ret != XMEDIA_CL_SUCCESS) {
        return -1;
    }

    return xmedia_mmz_free(u64PhyAddr);
}

xmedia_s32 get_freq(xmedia_cl_s32 *npu_freq) { 
    xmedia_cl_s32 ddr_fd = -1;
    xmedia_cl_u32 phy_addr = 0x12010000;
    xmedia_cl_u32 map_size = 0x100;
    
    ddr_fd = open("/dev/mem",O_RDWR | O_SYNC);
    if (ddr_fd == -1) {
        printf("open /dev/mem error\n");
        return -1;
    }
    xmedia_cl_u32 *ddr_reg = NULL;
    ddr_reg = (xmedia_cl_u32*)mmap(NULL,map_size, PROT_READ, MAP_PRIVATE, ddr_fd, phy_addr);
    if (ddr_reg == (void*)-1) {
        printf("mmap ddr error \n");
        close(ddr_fd);
        return -1;
    }
    int value = (*(ddr_reg + 0x28) >> 23) &0x7;
    switch (value) {
    case 0:
        *npu_freq = 496000;
        break;
    case 1:
        *npu_freq = 450000;
        break;
    case 2:
        *npu_freq = 396000;
        break;
    case 3:
        *npu_freq = 350000;
        break;
    case 4:
        *npu_freq = 263000;
        break;
    case 5:
        *npu_freq = 525000;
        break;
    default:
        *npu_freq = -1;
    }
   
    munmap(ddr_reg, map_size);
    close(ddr_fd);
    if (*npu_freq == -1)
        return -1;
    else 
        return XMEDIA_CL_SUCCESS;
}

int main(int argc,char **argv)
{
    void *buff[DEFAULT_INOUT_NUM] = { 0 };
    xmedia_cl_s8 name[64] = {0};
    xmedia_cl_u64 u64SrcPhyAddr[4] = { 0 };
    xmedia_char *nocache_buffer_start[4] = { 0 };
    xmedia_cl_s32 output_size[DEFAULT_INOUT_NUM] = { 0 };
    xmedia_cl_s8 *model = "data/neuron_network.xmm";
    xmedia_cl_s32 i = 0, j, size = 0, ret, err;
    xmedia_cl_graph graph = XMEDIA_NULL;
    xmedia_cl_u32 num_devices = 0, input_num = 0, output_num = 0;
    xmedia_cl_tensor_info_inout input = { 0 }, output = { 0 };
    xmedia_cl_s32 err_code = 0;
    xmedia_cl_context context = XMEDIA_NULL;
    xmedia_cl_device_id *devices = XMEDIA_NULL;
    xmedia_cl_u32 worksize, weightsize, inputsize = 0, outputsize = 0;
    xmedia_cl_u32 rd_bt_sum = 0, wt_bt_sum = 0, bt_sum = 0;
    xmedia_cl_u32 rd_latency = 0, wt_latency = 0;
    xmedia_cl_u32 rd_burst_num =0, wt_burst_num = 0 ;
    xmedia_cl_u32 run_cycle = 0;
    xmedia_cl_u32 time_val = 0, min_time_val = 0xffffffff, max_time_val = 0, tot_time_val = 0;
    xmedia_cl_float f_tot_rd_latency = 0, f_min_rd_latency = 1000000.0, f_max_rd_latency = 0, f_rd_latency = 0;
    xmedia_cl_float f_tot_wt_latency = 0, f_min_wt_latency = 1000000.0, f_max_wt_latency = 0, f_wt_latency = 0;
    xmedia_cl_s32 fd = -1;
    xmedia_cl_u64 tot_run_cycle = 0;
    xmedia_cl_s32 npu_freq = -1;

    xmedia_cl_u32 process_cnt = 100;
    if (argc == 2) {
        process_cnt = atoi(argv[1]);
    }
    if (process_cnt == 0) {
        printf("please input process run times\n");
        return XMEDIA_SUCCESS;
    }
   //公共模块初始化
    ret = xmedia_sys_init(XMEDIA_NULL);
    if (ret != XMEDIA_CL_SUCCESS) {
        printf("xmedia_sys_init err, errno %d\n", ret);
        return ret;
    }
    //运行时初始化
    ret = xmedia_cl_init();
    if (ret != XMEDIA_CL_SUCCESS) {
        printf("xmedia_cl_init err, errno %d\n", ret);
        goto SYS_EXIT;
    }
    //第一次调用该接口获取设备数量
    ret = xmedia_cl_get_device_ids(XMEDIA_CL_DEVICE_ALL, NULL, &num_devices);
    if (ret != XMEDIA_CL_SUCCESS) {
        printf("xmedia_cl_get_device_ids err, errno %d\n", ret);
        goto ERROR;
    }
    devices =
        (xmedia_cl_device_id *)calloc(num_devices, sizeof(xmedia_cl_device_id));
    if (devices == NULL) {
        printf("calloc err\n");
        ret = XMEDIA_CL_OUT_OF_HOST_MEMORY;
        goto ERROR;
    }
    //第二次调用该接口传递获取到的设备数量申请设备资源
    ret = xmedia_cl_get_device_ids(XMEDIA_CL_DEVICE_ALL, devices, &num_devices);
    if (ret != XMEDIA_CL_SUCCESS) {
        printf("xmedia_cl_get_device_ids err, errno %d\n", ret);
        free(devices);
        xmedia_cl_uninit();
        return ret;
    }
    //创建资源管理的对象
    context = xmedia_cl_create_context(num_devices, devices, &err_code);
    if (err_code != XMEDIA_CL_SUCCESS) {
        printf("xmedia_cl_create_context err, errno %d\n", ret);
        goto ERROR;
    }

    //从模型文件中查询work和weight的大小
    ret = xmedia_cl_graph_querysize_from_file(model, &worksize, &weightsize);
    if (XMEDIA_CL_SUCCESS != ret) {
        printf("xmedia_cl_graph_querysize_from_file, errno %d\n", ret);
        goto ERROR;
    }
    //根据查询到的大小申请work和weight的内存
    ret = XMEDIA_API_SYS_MmzAlloc_Cached(&u64SrcPhyAddr[0],
                                  (void **)(&nocache_buffer_start[0]), 
                                  "npu_workspace", NULL, worksize);
    if (XMEDIA_CL_SUCCESS != ret) {
        goto ERROR;
    }
    ret = XMEDIA_API_SYS_MmzAlloc_Cached(&u64SrcPhyAddr[1],
                                  (void **)(&nocache_buffer_start[1]), 
                                  "npu_weight", NULL, weightsize);
    if (XMEDIA_CL_SUCCESS != ret) {
        goto ERROR;
    }

    //加载解析模型文件，若是从文件中加载，则model为文件路径，若从内存中加载，则model为内存地址
    ret = xmedia_cl_graph_loadmodel_from_file_withmem(
        &context, model, nocache_buffer_start[0], worksize,
        nocache_buffer_start[1], weightsize, &graph);
    if (ret != XMEDIA_CL_SUCCESS) {
        printf("xmedia_cl_graph_loadmodel err, errno %d\n", ret);
        goto ERROR;
    }

    //第一次调用该接口获取输入数量
    ret = xmedia_cl_graph_get_input(graph, input_num, &input);
    if (ret != XMEDIA_CL_SUCCESS) {
        printf("xmedia_cl_graph_get_input, errno %d\n", ret);
        goto ERROR;
    }
    if (input.num > DEFAULT_INOUT_NUM) {
        printf("model input num is greater than default value!\n");
        goto ERROR;
    }
    //根据输入数量申请内存
    malloc_inout_tensor_mem(&input);
    input_num = input.num;
    //第二次调用该接口获取输入信息
    ret = xmedia_cl_graph_get_input(graph, input_num, &input);
    if (ret != XMEDIA_CL_SUCCESS) {
        printf("xmedia_cl_graph_get_input, errno %d\n", ret);
        goto ERROR;
    }

    //第一次调用该接口获取输出数量
    ret = xmedia_cl_graph_get_output(graph, output_num, &output);
    if (ret != XMEDIA_CL_SUCCESS) {
        printf("xmedia_cl_graph_get_output, errno %d\n", ret);
        goto ERROR;
    }
    if (output.num > DEFAULT_INOUT_NUM) {
        printf("model output num is greater than default value!\n");
        goto ERROR;
    }
    //根据输出数量申请内存
    malloc_inout_tensor_mem(&output);
    output_num = output.num;
    //第二次调用该接口获取输出信息
    ret = xmedia_cl_graph_get_output(graph, output_num, &output);
    if (ret != XMEDIA_CL_SUCCESS) {
        printf("xmedia_cl_graph_get_output, errno %d\n", ret);
        goto ERROR;
    }

    //计算输入的大小并申请内存
    for (i = 0; i < input.num; i++) {
        size = input.tensor[i].size;
        inputsize += ALIGN_FUNC(size, ALIGN_BYTE);
    }

    ret = XMEDIA_API_SYS_MmzAlloc_Cached(&u64SrcPhyAddr[2],
                                  (void **)(&nocache_buffer_start[2]), 
                                  "npu_input", NULL, inputsize);
    if (XMEDIA_CL_SUCCESS != ret) {
        goto ERROR;
    }

    //计算输出的大小并申请内存
    for (i = 0; i < output.num; i++) {
        output_size[i] = output.tensor[i].size;
        outputsize += ALIGN_FUNC(output_size[i], ALIGN_BYTE);
    }
    ret = XMEDIA_API_SYS_MmzAlloc_Cached(&u64SrcPhyAddr[3],
                                  (void **)(&nocache_buffer_start[3]), 
                                  "npu_output", NULL, outputsize);
    if (XMEDIA_CL_SUCCESS != ret) {
        goto ERROR;
    }

    //设置输入，把输入golden数据拷贝到输入地址,每个输入必须设置一次
    for (i = 0; i < input.num; i++) {
      if (i > 0) {
            input.tensor[i].addr =
                input.tensor[i - 1].addr + ALIGN_FUNC(size, ALIGN_BYTE);
        } else {
            input.tensor[i].addr = nocache_buffer_start[2];
        }
        size = input.tensor[i].size;

        memset(input.tensor[i].addr, 0,
               size); //清0防止运行拷贝模式对非拷贝模式的影响
        memset(name, 0, sizeof(name));
        sprintf(name, "data/input_data%d.bin", i);
        ret = copy_file_to_buff(name, input.tensor[i].addr, 0, size);
        if (ret != XMEDIA_CL_SUCCESS) {
            printf("read input_data%d.bin failed!\n", i);
            goto ERROR;
        }
        //printf("input.tensor[%d].addr = %p input size = 0x%x\n", i,
        //       input.tensor[i].addr, size);
    }

    //把输出的golden数据拷贝到内存，用来与npu的输出做对比
    for (i = 0; i < output.num; i++) {
        if (i > 0) {
            output.tensor[i].addr = output.tensor[i - 1].addr +
                                    ALIGN_FUNC(output_size[i - 1], ALIGN_BYTE);
        } else {
            output.tensor[i].addr = nocache_buffer_start[3];
        }
        memset(name, 0, sizeof(name));
        output_size[i] = output.tensor[i].size;
        buff[i] = malloc(output_size[i]);
        if (buff[i] == NULL) {
            printf("malloc err\n");
            ret = XMEDIA_CL_OUT_OF_HOST_MEMORY;
            goto ERROR;
        }
        sprintf(name, "data/output_data%d.bin", i);
        ret = copy_file_to_buff(name, buff[i], 0, output_size[i]);
        if (ret != XMEDIA_CL_SUCCESS) {
            printf("read output_data%d.bin failed!\n", i);
            goto ERROR;
        }
        memset(output.tensor[i].addr, 0, output_size[i]);
        //printf("output.tensor[%d].addr = %p output size = 0x%x\n", i,
        //       output.tensor[i].addr, output_size[i]);
    }

    //在xmm2下，用户需要申请输入地址，输出地址可以动态改变，把输入与输出设置为推理的输入、输出
    ret = xmedia_cl_graph_set_inout(graph, &input, &output);
    if (ret != XMEDIA_CL_SUCCESS) {
        printf("xmedia_cl_graph_set_inout, errno %d\n", ret);
        goto ERROR;
    }
    fd = open("/dev/npu.0",O_RDWR | O_SYNC);
    if (fd == -1) {
        printf("open /dev/npu.0 error\n");
        goto ERROR;
    }

    ret = get_freq(&npu_freq);
    if (ret != XMEDIA_CL_SUCCESS){
        printf("get npu freq error\n");
        goto ERROR;
    }

    gh_npu_set_wt_bw_enable(fd, XMEDIA_TRUE);
    gh_npu_set_rd_bw_enable(fd, XMEDIA_TRUE);
    gh_npu_set_wt_latency_enable(fd, XMEDIA_TRUE);
    gh_npu_set_rd_latency_enable(fd, XMEDIA_TRUE);
    printf("xmedia_cl_graph_process %d times\n", process_cnt);
    for (j = 0; j < process_cnt; j++) {
        //执行推理
        struct timeval t_start, t_end;
        gettimeofday(&t_start, NULL);
        ret = xmedia_cl_graph_process(graph);
        gettimeofday(&t_end, NULL);
        time_val = ((t_end.tv_sec-t_start.tv_sec)*1000000) + t_end.tv_usec-t_start.tv_usec;
        min_time_val = min_time_val < time_val ? min_time_val : time_val; 
        max_time_val = max_time_val > time_val ? max_time_val : time_val; 
        tot_time_val += time_val;
        if (ret != XMEDIA_CL_SUCCESS) {
            printf("xmedia_cl_graph_process err, errno %d\n", ret);
            break;
        }
        system("xmmm 0x120100b4 2 > /dev/NULL");
        ret = gh_npu_get_rd_bw_sum(fd, &rd_bt_sum);
        ret = gh_npu_get_wt_bw_sum(fd, &wt_bt_sum);
        ret = gh_npu_get_rd_latency_sum(fd, &rd_latency);
        ret = gh_npu_get_wt_latency_sum(fd, &wt_latency);
        rd_burst_num = gh_npu_get_rd_latency_burst_sum(fd);
        wt_burst_num = gh_npu_get_wt_latency_burst_sum(fd);
        f_rd_latency = rd_latency * 1.0 / rd_burst_num;
        f_wt_latency = wt_latency * 1.0 / wt_burst_num;
        f_tot_rd_latency += f_rd_latency;
        f_tot_wt_latency += f_wt_latency;
        run_cycle = gh_npu_get_cfg_running_cycle_cnt(fd);
        bt_sum = rd_bt_sum + wt_bt_sum;
        f_min_rd_latency = f_min_rd_latency < f_rd_latency ? f_min_rd_latency : f_rd_latency;
        f_max_rd_latency = f_max_rd_latency > f_rd_latency ? f_max_rd_latency : f_rd_latency;
        f_min_wt_latency = f_min_wt_latency < f_wt_latency ? f_min_wt_latency : f_wt_latency;
        f_max_wt_latency = f_max_wt_latency > f_wt_latency ? f_max_wt_latency : f_wt_latency;
        tot_run_cycle += run_cycle;
        if (j < 1) {
            for (i = 0; i < 1; i++) {
                //printf("#################################\n");
                array_cmp_fixed(output.tensor[i].addr, buff[i],
                                output.tensor[i].shape.type, output_size[i],
                                (char *)__FILE__, 0);
                //printf("#################################\n");
            }
        }
    }
    float fps = 1000/(tot_run_cycle * 1.0/ process_cnt /npu_freq);
    char proc[256];
    
    printf("Latency Cycle :\n");
    for (i = 0 ; i <64;i++) 
      printf("—");
    printf("\n");
    sprintf(proc,"|%20s|%20s|%20s|\n", "ave_read","min_read","max_read");
    printf("%s",proc);
    sprintf(proc,"|%20.3f|%20.3f|%20.3f|\n", f_tot_rd_latency / process_cnt, f_min_rd_latency, f_max_rd_latency);
    printf("%s",proc);
    for (i = 0 ; i <64;i++) 
      printf("—");
    printf("\n");
    
    sprintf(proc,"|%20s|%20s|%20s|\n", "ave_write","min_write","max_write");
    printf("%s",proc);
    sprintf(proc,"|%20.3f|%20.3f|%20.3f|\n", f_tot_wt_latency / process_cnt, f_min_wt_latency, f_max_wt_latency);
    printf("%s",proc);
    for (i = 0 ; i <64;i++) 
      printf("—");
    printf("\n");

    printf("Bandwidth : \n");
    for (i = 0 ; i <64;i++) 
      printf("—");
    printf("\n");
    sprintf(proc, "|%20s|%20s|%20s|\n","read(kb/frame)","write(kb/frame)","total(kb/frame)");
    printf("%s", proc);
    sprintf(proc, "|%20u|%20u|%20u|\n",rd_bt_sum * 8 / 1024, wt_bt_sum * 8 / 1024, bt_sum * 8 / 1024);
    printf("%s", proc);
    for (i = 0 ; i <64;i++) 
      printf("—");
    printf("\n");
    sprintf(proc, "|%20s|%20s|%20s|\n","read(mb/s)","write(mb/s)","total(mb/s)");
    printf("%s", proc);
    sprintf(proc, "|%20.3f|%20.3f|%20.3f|\n",rd_bt_sum * fps * 8 / 1024 / 1024, wt_bt_sum * fps * 8 / 1024 / 1024, bt_sum * fps* 8 / 1024 / 1024);
    printf("%s", proc);
    for (i = 0 ; i <64;i++) 
      printf("—");
    printf("\n");

    printf("Model Performance :\n");
    for (i = 0 ; i <64;i++) 
      printf("—");
    printf("\n");
    sprintf(proc,"|%20s|%20s|%20s|\n", "ai_core_time(ms)","execute_time(ms)","FPS"  );
    printf("%s",proc);
    sprintf(proc,"|%20.3f|%20.3f|%20.3f|\n", tot_run_cycle * 1.0/ process_cnt / npu_freq,tot_time_val / process_cnt / 1000.0, fps);
    printf("%s",proc);
    for (i = 0 ; i <64;i++) 
      printf("—");
    printf("\n");
    

    printf("Memory Usage :\n");
    for (i = 0 ; i <85;i++) 
      printf("—");
    printf("\n");
    sprintf(proc, "|%20s|%20s|%20s|%20s|\n","work_size(kb)","weight_size(kb)","input_size(kb)","output_size(kb)");
    printf("%s", proc);
    sprintf(proc, "|%20.3f|%20.3f|%20.3f|%20.3f|\n",worksize / 1024.0, weightsize / 1024.0,inputsize / 1024.0, outputsize / 1024.0);
    printf("%s", proc);
    for (i = 0 ; i <85;i++) 
      printf("—");
    printf("\n");
    close(fd);
ERROR:
    //释放资源
    free_inout_tensor_mem(input);
    free_inout_tensor_mem(output);
    for (i = 0; i < 4; i++) {
        if (nocache_buffer_start[i] != NULL) {
            XMEDIA_API_SYS_MmzFree(u64SrcPhyAddr[i], nocache_buffer_start[i]);
        }
    }
    for (i = 0; i < output.num; i++) {
        if (buff[i] != NULL) {
            free(buff[i]);
        }
    }
    if (graph != NULL) {
        err = xmedia_cl_graph_unload(graph);
        if (err != XMEDIA_CL_SUCCESS) {
            printf("xmedia_cl_graph_unload err, errno %d\n", err);
            return err;
        }
    }
    if (context != NULL) {
        err = xmedia_cl_release_context(context);
        if (err != XMEDIA_CL_SUCCESS) {
            printf("xmedia_cl_release_context err, errno %d\n", err);
            return err;
        }
    }
    if (devices != NULL) {
        err = xmedia_cl_release_device_ids(devices, &num_devices);
        if (err != XMEDIA_CL_SUCCESS) {
            printf("xmedia_cl_release_device_ids err, errno %d\n", err);
            return err;
        }
        free(devices);
    }

    err = xmedia_cl_uninit();
    if (err != XMEDIA_CL_SUCCESS) {
        printf("xmedia_cl_uninit err, errno %d\n", err);
        return err;
    }
SYS_EXIT:
    err = xmedia_sys_exit();
    if (err != XMEDIA_CL_SUCCESS) {
        printf("xmedia_sys_exit err, errno %d\n", err);
        return err;
    }

    return ret;
}
