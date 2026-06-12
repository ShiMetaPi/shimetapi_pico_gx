#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "xmedia_cl.h"
#include "xmedia_mmz.h"
#include "xmedia_sys.h"
#include <sys/time.h>

#define ALIGN_BYTE 8
#define DEFAULT_INOUT_NUM 16
#define ALIGN_FUNC(A, ALIGN) \
    ((((A) % (ALIGN)) == 0) ? (A) : ((A) + (ALIGN) - ((A) % (ALIGN))))

int abs(int num)
{
    return num < 0 ? -num : num;
}

static xmedia_cl_double
common_get_type_get_bytes_extension(xmedia_cl_data_type dtype)
{
    switch (dtype) {
    case XMEDIA_CL_INT8:
    case XMEDIA_CL_INT4:
    case XMEDIA_CL_UINT8:
        return 1.0;
    case XMEDIA_CL_UINT12:
        return 1.5;
    case XMEDIA_CL_INT16:
    case XMEDIA_CL_UINT16:
    case XMEDIA_CL_FP16:
        return 2.0;
    case XMEDIA_CL_INT32:
    case XMEDIA_CL_FP32:
        return 4.0;
    default:
        return 0.0;
    }
}

static xmedia_cl_s32 gnn_tensor_cmp_fixed(void *p_exp, void *p_got,
                                          xmedia_cl_tensor_shape *exp,
                                          xmedia_cl_tensor_shape *got,
                                          const xmedia_cl_s8 *info_label,
                                          xmedia_cl_s32 delta)
{
    xmedia_cl_s32 first_error_c_idx = -1, first_error_h_idx = 0,
                  first_error_w_idx = 0, first_expect_value = 0,
                  first_got_value = 0;
    xmedia_cl_s32 max_error_c_idx = -1, max_error_h_idx = 0,
                  max_error_w_idx = 0, max_expect_value = 0, max_got_value = 0;
    xmedia_cl_s32 max_error_value = 0, mismatch_cnt = 0;
    xmedia_cl_s32 error = 0;
    xmedia_cl_s32 exp_int = 0;
    xmedia_cl_s32 got_int = 0;
    xmedia_cl_s32 ndims = 0;

    xmedia_cl_u32 n = 0, c = 0, h = 0, w = 0;
    xmedia_cl_u32 exp_n_stride = 0, exp_c_stride = 0, exp_h_stride = 0,
                  got_n_stride = 0, got_c_stride = 0, got_h_stride = 0;
    xmedia_cl_double w_stride = 0;
    xmedia_cl_u32 exp_idx = 0;
    xmedia_cl_u32 got_idx = 0;
    w_stride = common_get_type_get_bytes_extension(exp->type);

    ndims = exp->ndims;

    if (XMEDIA_CL_UINT12 == exp->type) {
        exp->type = XMEDIA_CL_UINT8;
        if (ndims == 2) {
            exp->dims[1] = exp->dims[1] * w_stride;
        } else if (ndims == 3) {
            exp->dims[2] = exp->dims[2] * w_stride;
        } else if (ndims == 4) {
            exp->dims[3] = exp->dims[3] * w_stride;
        }
        w_stride = common_get_type_get_bytes_extension(exp->type);
    }

    if (ndims == 2) {
        n = 1;
        c = 1;
        h = exp->dims[0];
        w = exp->dims[1];

        exp_h_stride = exp->pch[0];
        exp_c_stride = exp_h_stride * exp->dims[0];
        exp_n_stride = exp_c_stride;
    } else if (ndims == 3) {
        n = 1;
        c = exp->dims[0];
        h = exp->dims[1];
        w = exp->dims[2];

        exp_c_stride = exp->pch[0];
        exp_n_stride = exp_c_stride * c;
        exp_h_stride = exp->pch[1];
    } else if (ndims == 4) {
        n = exp->dims[0];
        c = exp->dims[1];
        h = exp->dims[2];
        w = exp->dims[3];

        exp_n_stride = exp->pch[0];
        exp_c_stride = exp->pch[1];
        exp_h_stride = exp->pch[2];
    }

    ndims = got->ndims;

    if (ndims == 2) {
        got_h_stride = got->pch[0];
        got_c_stride = got->dims[0] * got_h_stride;
        got_n_stride = got_c_stride;
    } else if (ndims == 3) {
        got_c_stride = got->pch[0];
        got_n_stride = got_c_stride;
        got_h_stride = got->pch[1];
    } else if (ndims == 4) {
        got_n_stride = got->pch[0];
        got_c_stride = got->pch[1];
        got_h_stride = got->pch[2];
    }

    for (xmedia_cl_u32 nidx = 0; nidx < n; nidx++) {
        for (xmedia_cl_u32 cidx = 0; cidx < c; cidx++) {
            for (xmedia_cl_u32 hidx = 0; hidx < h; hidx++) {
                for (xmedia_cl_u32 widx = 0; widx < w; widx++) {
                    exp_idx = nidx * exp_n_stride + cidx * exp_c_stride +
                              hidx * exp_h_stride + widx * w_stride;
                    got_idx = nidx * got_n_stride + cidx * got_c_stride +
                              hidx * got_h_stride + widx * w_stride;

                    if (exp->type == XMEDIA_CL_INT8) {
                        exp_int =
                            (xmedia_cl_s32)(*((xmedia_cl_s8 *)p_exp + exp_idx));
                        got_int =
                            (xmedia_cl_s32)(*((xmedia_cl_s8 *)p_got + got_idx));

                    } else if (exp->type == XMEDIA_CL_UINT8) {
                        exp_int =
                            (xmedia_cl_s32)(*((xmedia_cl_u8 *)p_exp + exp_idx));
                        got_int =
                            (xmedia_cl_s32)(*((xmedia_cl_u8 *)p_got + got_idx));
                    } else if (exp->type == XMEDIA_CL_INT16) {
                        exp_int = (xmedia_cl_s32)(*((xmedia_cl_s16 *)p_exp +
                                                    (exp_idx >> 1)));
                        got_int = (xmedia_cl_s32)(*((xmedia_cl_s16 *)p_got +
                                                    (got_idx >> 1)));
                    } else if (exp->type == XMEDIA_CL_UINT16) {
                        exp_int = (xmedia_cl_s32)(*((xmedia_cl_u16 *)p_exp +
                                                    (exp_idx >> 1)));
                        got_int = (xmedia_cl_s32)(*((xmedia_cl_u16 *)p_got +
                                                    (got_idx >> 1)));
                    } else if (exp->type == XMEDIA_CL_INT32) {
                        exp_int = (xmedia_cl_s32)(*((xmedia_cl_s32 *)p_exp +
                                                    (exp_idx >> 2)));
                        got_int = (xmedia_cl_s32)(*((xmedia_cl_s32 *)p_got +
                                                    (got_idx >> 2)));
                    } else {
                        //assert(0);
                    }
                    error = abs(exp_int - got_int);
                    if (error > 0) {
                        if (first_error_c_idx == -1) {
                            first_error_c_idx = cidx;
                            first_error_h_idx = hidx;
                            first_error_w_idx = widx;
                            first_expect_value = exp_int;
                            first_got_value = got_int;
                        }
                        if (error > max_error_value) {
                            max_error_c_idx = cidx;
                            max_error_h_idx = hidx;
                            max_error_w_idx = widx;
                            max_error_value = error;
                            max_expect_value = exp_int;
                            max_got_value = got_int;
                        }
                        mismatch_cnt++;
                    }
                    if (error > delta) {
                        printf(
                            "%s:     error      at c_index %d h_index %d w_index %d exp %d got %d\n",
                            info_label, cidx, hidx, widx, exp_int, got_int);
                        printf(
                            "%s: first mismatch at c_index %d h_index %d w_index %d exp %d got %d (delta %d)\n",
                            info_label, first_error_c_idx, first_error_h_idx,
                            first_error_w_idx, first_expect_value,
                            first_got_value, delta);
                        printf("%s: total mismatch count %d (delta %d) \n",
                               info_label, mismatch_cnt, delta);
                        printf("compare failed!!!\n");
                        return -1;
                    }
                }
            }
        }
    }
    if (max_error_c_idx != -1) {
        printf(
            "%s: first mismatch at c_index %d h_index %d w_index %d exp %d got %d (delta %d)\n",
            info_label, first_error_c_idx, first_error_h_idx, first_error_w_idx,
            first_expect_value, first_got_value, delta);
        printf(
            "%s:  max  mismatch at c_index %d h_index %d w_index %d exp %d got %d (delta %d)\n",
            info_label, max_error_c_idx, max_error_h_idx, max_error_w_idx,
            max_expect_value, max_got_value, delta);
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

    if (inout->tensor == NULL || inout->current_batch == NULL ||
        inout->tensor_batch == NULL) {
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

int main()
{
    void *buff[DEFAULT_INOUT_NUM] = { 0 };
    xmedia_cl_s8 name[64] = { 0 };
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
    xmedia_cl_memory_reuse_type memory_reuse_type;

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
    if (worksize) {
        //根据查询到的大小申请work和weight的内存
        ret = XMEDIA_API_SYS_MmzAlloc_Cached(
            &u64SrcPhyAddr[0], (void **)(&nocache_buffer_start[0]), 
            "npu_workspace", NULL, worksize);
        if (XMEDIA_CL_SUCCESS != ret) {
            goto ERROR;
        }
    } else {
        nocache_buffer_start[0] = NULL;
    }
    if (weightsize) {
        ret = XMEDIA_API_SYS_MmzAlloc_Cached(
            &u64SrcPhyAddr[1], (void **)(&nocache_buffer_start[1]), 
            "npu_weight", NULL, weightsize);
        if (XMEDIA_CL_SUCCESS != ret) {
            goto ERROR;
        }
    } else {
        nocache_buffer_start[1] = NULL;
    }
    //加载解析模型文件，若是从文件中加载，则model为文件路径，若从内存中加载，则model为内存地址
    ret = xmedia_cl_graph_loadmodel_from_file_withmem(
        &context, model, nocache_buffer_start[0], worksize,
        nocache_buffer_start[1], weightsize, &graph);
    if (ret != XMEDIA_CL_SUCCESS) {
        printf("xmedia_cl_graph_loadmodel err, errno %d\n", ret);
        goto ERROR;
    }
    ret = xmedia_cl_graph_get_memory_reuse_type(graph, &memory_reuse_type);
    if (ret != XMEDIA_CL_SUCCESS) {
        printf("xmedia_cl_graph_get_memory_reuse_type, errno %d\n", ret);
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
    if (memory_reuse_type != XMEDIA_CL_WORKSPACE_INPUT &&
        memory_reuse_type != XMEDIA_CL_WORKSPACE_INPUT_OUTPUT) {
        ret = XMEDIA_API_SYS_MmzAlloc_Cached(
            &u64SrcPhyAddr[2], (void **)(&nocache_buffer_start[2]), 
            "npu_input", NULL,inputsize);
        if (XMEDIA_CL_SUCCESS != ret) {
            goto ERROR;
        }
    }

    //计算输出的大小并申请内存
    for (i = 0; i < output.num; i++) {
        output_size[i] = output.tensor[i].size;
        outputsize += ALIGN_FUNC(output_size[i], ALIGN_BYTE);
    }
    if (memory_reuse_type != XMEDIA_CL_WORKSPACE_OUTPUT &&
        memory_reuse_type != XMEDIA_CL_WORKSPACE_INPUT_OUTPUT) {
        ret = XMEDIA_API_SYS_MmzAlloc_Cached(
            &u64SrcPhyAddr[3], (void **)(&nocache_buffer_start[3]), 
            "npu_output", NULL, outputsize);
        if (XMEDIA_CL_SUCCESS != ret) {
            goto ERROR;
        }
    }

    for (j = 0; j < 10; j++) { //内存复用时，多次process必须每次都重新设置输入输出地址
        //设置输入，把输入golden数据拷贝到输入地址,每个输入必须设置一次
        for (i = 0; i < input.num; i++) {
            if (memory_reuse_type == XMEDIA_CL_WORKSPACE ||
                memory_reuse_type == XMEDIA_CL_WORKSPACE_OUTPUT) {
                if (i > 0) {
                    input.tensor[i].addr =
                        input.tensor[i - 1].addr + ALIGN_FUNC(size, ALIGN_BYTE);
                } else {
                    input.tensor[i].addr = nocache_buffer_start[2];
                }
            }
            size = input.tensor[i].size;

            memset(name, 0, sizeof(name));
            sprintf(name, "data/input_data%d.bin", i);
            ret = copy_file_to_buff(name, input.tensor[i].addr, 0, size);
            if (ret != XMEDIA_CL_SUCCESS) {
                printf("read input_data%d.bin failed!\n", i);
                goto ERROR;
            }
            printf("input.tensor[%d].addr = %p input size = 0x%x\n", i,
                   input.tensor[i].addr, size);
        }

        //把输出的golden数据拷贝到内存，用来与npu的输出做对比
        for (i = 0; i < output.num; i++) {
            if (memory_reuse_type == XMEDIA_CL_WORKSPACE ||
                memory_reuse_type == XMEDIA_CL_WORKSPACE_INPUT) {
                if (i > 0) {
                    output.tensor[i].addr =
                        output.tensor[i - 1].addr +
                        ALIGN_FUNC(output_size[i - 1], ALIGN_BYTE);
                } else {
                    output.tensor[i].addr = nocache_buffer_start[3];
                }
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
            printf("output.tensor[%d].addr = %p output size = 0x%x\n", i,
                   output.tensor[i].addr, output_size[i]);
        }

        //在xmm2下，用户需要申请输入地址，输出地址可以动态改变，把输入与输出设置为推理的输入、输出

        if (memory_reuse_type == XMEDIA_CL_WORKSPACE) {
            ret = xmedia_cl_graph_set_inout(graph, &input, &output);
            if (ret != XMEDIA_CL_SUCCESS) {
                printf("xmedia_cl_graph_set_inout, errno %d\n", ret);
                goto ERROR;
            }
        } else if (memory_reuse_type == XMEDIA_CL_WORKSPACE_INPUT) {
            ret = xmedia_cl_graph_set_inout(graph, NULL, &output);
            if (ret != XMEDIA_CL_SUCCESS) {
                printf("xmedia_cl_graph_set_inout, errno %d\n", ret);
                goto ERROR;
            }
        } else if (memory_reuse_type == XMEDIA_CL_WORKSPACE_OUTPUT) {
            ret = xmedia_cl_graph_set_inout(graph, &input, NULL);
            if (ret != XMEDIA_CL_SUCCESS) {
                printf("xmedia_cl_graph_set_inout, errno %d\n", ret);
                goto ERROR;
            }
        }
        printf("xmedia_cl_graph_pocess\n");
        //执行推理
        struct timeval t_start, t_end;
        gettimeofday(&t_start, NULL);
        ret = xmedia_cl_graph_process(graph);
        gettimeofday(&t_end, NULL);
        printf("xmedia_cl_graph_process start time = [%ld]\n", t_start.tv_usec);
        printf("xmedia_cl_graph_process end time = [%ld]\n", t_end.tv_usec);
        printf("xmedia_cl_graph_process timeval = [%ld]\n",
               ((t_end.tv_sec - t_start.tv_sec) * 1000000) + t_end.tv_usec -
                   t_start.tv_usec);
        if (ret != XMEDIA_CL_SUCCESS) {
            printf("xmedia_cl_graph_process err, errno %d\n", ret);
            break;
        }

        for (i = 0; i < output.num; i++) {
            printf("#################################\n");
            gnn_tensor_cmp_fixed(output.tensor[i].addr, buff[i],
                                 &output.tensor[i].shape,
                                 &output.tensor[i].shape, (char *)__FILE__, 0);
            printf("#################################\n");
        }
        for (i = 0; i < output.num; i++) {
            if (buff[i] != NULL) {
                free(buff[i]);
                buff[i] = NULL;
            }
        }
    }
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
