/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/spi/spidev.h>
#include <pthread.h>
#include <signal.h>

#include "xmedia_msensor.h"
#include "xmedia_sys.h"
#include "xmedia_vi.h"
#include "xmedia_venc.h"
#include "xmedia_mmz.h"

#include "sample_comm.h"
#include "sample_comm_sys.h"
#include "sample_comm_vi.h"
#include "sample_comm_vpss.h"
#include "sample_comm_vgs.h"
#include "sample_comm_venc.h"
#include "sample_comm_isp.h"
#include "sample_msensor.h"


#define NUMS_IN_BUF              4000 /* max numbers of gyro data or acc data in buffer */
#define GYRO_BUF_SIZE            (sizeof(xmedia_msensor_sample_data) * NUMS_IN_BUF)
#define ACC_BUF_SIZE             (sizeof(xmedia_msensor_sample_data) * NUMS_IN_BUF)

#define GYRO_DATA_RECORDER_RANGE 1000
#define GYRO_DATA_DV_RANGE       1000
#define GYRO_DATA_ODR            800
#define ACC_DATA_RANGE           16
#define ACC_DATA_ODR             800

#define INT_MAX  0x7fffffff        /* max value for an int */
#define GYRO_BIN_FILE "./gyro_data.bin"
#define GYRO_TXT_FILE "./gyro_data.txt"
#define GYRO_PARSE_FILE "./gyro_parse_data.txt"

typedef struct msensor_get_data_info_ {
    xmedia_bool thread_start;
    xmedia_msensor_data_type data_type;
    xmedia_u32 time_intval;
    xmedia_s32 msensor_dev;
    xmedia_s32 venc_chn;
} msensor_get_data_info;

typedef struct msensor_data_info_ {
    xmedia_s32 msensor_dev;
    xmedia_s32 user_id;
    FILE *gyro_bin_fp;
    FILE *gyro_txt_fp;
} msensor_data_info;

static xmedia_bool g_force_exit = XMEDIA_FALSE;

xmedia_s32 g_msensor_dev_fd = -1;
xmedia_msensor_buf_attr g_msensor_attr[XMEDIA_MSENSOR_MAX_DEV_NUM] = { 0 };
xmedia_s32 g_gyro_data_range = GYRO_DATA_DV_RANGE;

static pthread_t g_msensor_pid[XMEDIA_MSENSOR_MAX_DEV_NUM];
static msensor_get_data_info g_get_data_info[XMEDIA_MSENSOR_MAX_DEV_NUM] = { 0 };
static sample_comm_sensor_type sample_msensor_sensor_type[MAX_SENSOR_NUM] = {
    SENSOR0_TYPE,SENSOR1_TYPE,SENSOR2_TYPE,SENSOR3_TYPE,SENSOR4_TYPE
};

xmedia_void sample_msensor_handle_sig(xmedia_s32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo || SIGTERM == signo) {
        g_force_exit = XMEDIA_TRUE;
    }
}

xmedia_s32 sample_msensor_sys_init(sample_sys_config *sys_config)
{
    xmedia_s32 ret = 0;

    ret = sample_comm_sys_init(sys_config);
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_sys_init failed!\n");
        return ret;
    }

    //in online-online mode,vi and vpss must be reset at the same time
    ret = sample_comm_vi_init();
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vi_init failed!\n");
        return ret;
    }

    ret = sample_comm_vpss_init();
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_vpss_init failed!\n");
        return ret;
    }

    ret = sample_comm_venc_init();
    if(ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sample_comm_venc_init failed!\n");
        return ret;
    }

    return XMEDIA_SUCCESS;
}

xmedia_void sample_msensor_sys_exit(void)
{
    sample_comm_venc_exit();
    sample_comm_vpss_exit();
    sample_comm_vi_exit();
    sample_comm_sys_exit();
}

xmedia_void sample_msensor_usage(xmedia_char* args)
{
    printf("Usage1 : %s [index] \n", args);
    printf("index:\n");
    printf("\t  0) h265  + msensor.\n");
    printf("\t  1) parse msensor data.\n");
    return;
}

xmedia_u32 h265_encode_sei(xmedia_void *dest_buf, xmedia_void *src_buf, xmedia_u32 in_len_bytes,
                      xmedia_u32 *out_len_bytes)
{
    xmedia_u8 *dest = (xmedia_u8 *)dest_buf;
    xmedia_u8 *src = (xmedia_u8 *)src_buf;
    xmedia_s32 tmp_len = 0;
    xmedia_u32 size;
    xmedia_u32 tmp_code;
    xmedia_u32 byte_cnt = 0;
    xmedia_s32 zero_cnt = 0;
    xmedia_s32 k;

    size = in_len_bytes;

	dest[byte_cnt++] = 0x00;
	dest[byte_cnt++] = 0x00;
	dest[byte_cnt++] = 0x00;
	dest[byte_cnt++] = 0x1;
	dest[byte_cnt++] = 0x4e;
	dest[byte_cnt++] = 0x1;
	dest[byte_cnt++] = 0xF0;	

    while (size >= 255) {
        size -= 255;
		dest[byte_cnt++] = 0xFF;
    }

    tmp_code = size;
	dest[byte_cnt++] = tmp_code;
    tmp_len = byte_cnt;

    if (size == 0) {
        zero_cnt = 1;
    }

    for (k = 0; k < (xmedia_s32)in_len_bytes; k++) {
        /* true when 0x000000|0x000001|0x000002|0x000003 */
        if ((zero_cnt == 2) && !(src[k] & 0xFC)) {
            dest[byte_cnt++] = 0x03;
            zero_cnt = 0;
        }

        dest[byte_cnt++] = src[k];

        if (src[k] == 0x00) {
            zero_cnt++;
        } else {
            zero_cnt = 0;
        }
    }

    dest[byte_cnt++] = 0x80;
    *out_len_bytes = byte_cnt;

    return tmp_len;
}


xmedia_u32 h264_encode_sei(xmedia_void *dest_buf, xmedia_void *src_buf, xmedia_u32 in_len_bytes,
                      xmedia_u32 *out_len_bytes)
{
    xmedia_u8 *dest = (xmedia_u8 *)dest_buf;
    xmedia_u8 *src = (xmedia_u8 *)src_buf;
    xmedia_s32 tmp_len = 0;
    xmedia_u32 size;
    xmedia_u32 tmp_code;
    xmedia_u32 byte_cnt = 0;
    xmedia_s32 zero_cnt = 0;
    xmedia_s32 k;

    size = in_len_bytes;

	dest[byte_cnt++] = 0x00;
	dest[byte_cnt++] = 0x00;
	dest[byte_cnt++] = 0x00;
	dest[byte_cnt++] = 0x1;
	dest[byte_cnt++] = 0x6;
	dest[byte_cnt++] = 0xF0;	

    while (size >= 255) {
        size -= 255;
		dest[byte_cnt++] = 0xFF;
    }

    tmp_code = size;
	dest[byte_cnt++] = tmp_code;
    tmp_len = byte_cnt;
	
    if (size == 0) {
        zero_cnt = 1;
    }

    for (k = 0; k < (xmedia_s32)in_len_bytes; k++) {
        /* true when 0x000000|0x000001|0x000002|0x000003 */
        if ((zero_cnt == 2) && !(src[k] & 0xFC)) {
            dest[byte_cnt++] = 0x03;
            zero_cnt = 0;
        }

        dest[byte_cnt++] = src[k];

        if (src[k] == 0x00) {
            zero_cnt++;
        } else {
            zero_cnt = 0;
        }
    }

    dest[byte_cnt++] = 0x80;
    *out_len_bytes = byte_cnt;

    return tmp_len;
}

static void save_gyro_data(xmedia_u8 *data, xmedia_s32 datalen, FILE *pfile)
{
    xmedia_s32 x;
    xmedia_s32 y;
    xmedia_s32 z;
    xmedia_s32 temp;
    xmedia_u64 pts;
    xmedia_u8 * temp_ptr = data;
    xmedia_s32 tmplen = 0;
    xmedia_char buf[128] = {0};

    if (datalen % 24 != 0) {
        return ;
    }
    while (tmplen + 24 <= datalen) {
        x = *(xmedia_s32 *)temp_ptr;
        temp_ptr += sizeof(xmedia_s32);
        y = *(xmedia_s32 *)temp_ptr;
        temp_ptr += sizeof(xmedia_s32);
        z = *(xmedia_s32 *)temp_ptr;
        temp_ptr += sizeof(xmedia_s32);
        temp = *(xmedia_s32 *)temp_ptr;
        temp_ptr += sizeof(xmedia_s32);
        pts = *(xmedia_u64 *)temp_ptr;
        temp_ptr += sizeof(xmedia_u64);

        tmplen += 24;

        sprintf(buf, "%12d,%12d,%12d,%12d,%22llu, \n", x, y, z, temp, pts);
        fwrite(buf, strlen(buf), 1, pfile);
    }
}

// 查找4字节起始码 0x00000001
int find_4byte_start_code(const xmedia_u8* data, int size, int start_pos)
{
    int i;

    for (i = start_pos; i < size - 4; i++) {
        if (data[i] == 0x00 && data[i+1] == 0x00 && 
            data[i+2] == 0x00 && data[i+3] == 0x01) {
            return i;
        }
    }
    return -1;
}

// 防竞争字节处理 - 移除0x03字节
// 在H.264/H.265中，0x000000, 0x000001, 0x000002, 0x000003前面会插入0x03防止竞争
xmedia_u8* remove_emulation_prevention_bytes(const xmedia_u8* data, int size, int* new_size)
{
    xmedia_u8* output;
    int output_index = 0;
    int zero_count = 0;
    
    if (size <= 0) {
        *new_size = 0;
        return NULL;
    }
    
    // 分配足够大的缓冲区
    output = (xmedia_u8*)malloc(size);
    if (!output) {
        *new_size = 0;
        return NULL;
    }
    
    for (int i = 0; i < size; i++) {
        if (zero_count == 2 && data[i] == 0x03) {
            // 找到防竞争字节0x03，跳过它
            zero_count = 0;
            continue;
        }
        
        if (data[i] == 0x00) {
            zero_count++;
        } else {
            zero_count = 0;
        }
        
        output[output_index++] = data[i];
    }
    
    *new_size = output_index;
    return output;
}

// 解析H.264 SEI NAL单元（带防竞争处理）
int parse_h264_sei_nal(const xmedia_u8* nal_data, int nal_size, FILE *file)
{
    int rbsp_size;
    xmedia_u8* rbsp_data;
    
    int payload_type = 0;
    xmedia_u8 byte;

    int pos = 1; // 跳过NAL头（1字节）
    
    if (nal_size < 2) {
        return 0;
    }
    
    // 首先进行防竞争字节处理
    rbsp_data = remove_emulation_prevention_bytes(nal_data, nal_size, &rbsp_size);
    if (!rbsp_data) {
        return 0;
    }
    
    pos = 1; // 跳过NAL头（1字节）
    
    while (pos < rbsp_size) {
        // 解析payload_type（使用类似UTF-8的变长编码）
        do {
            if (pos >= rbsp_size) {
                break;
            }
            byte = rbsp_data[pos++];
            payload_type += byte;
        } while (byte == 0xFF);
        
        if (pos >= rbsp_size) {
            break;
        }
        
        // 解析payload_size（使用类似UTF-8的变长编码）
        int payload_size = 0;
        do {
            if (pos >= rbsp_size) {
                break;
            }
            byte = rbsp_data[pos++];
            payload_size += byte;
        } while (byte == 0xFF);
        
        if (pos + payload_size > rbsp_size) {
            printf("Warning: SEI payload size exceeds RBSP size\n");
            break;
        }

        save_gyro_data(rbsp_data+pos, payload_size, file);
        
        pos += payload_size;
        
        // 跳过rbsp_trailing_bits（如果有）
        if (pos < rbsp_size) {
            // rbsp_trailing_bits以1开始，后面跟0
            if (rbsp_data[pos] == 0x80) {
                pos++;
            }
        }
    }
    
    free(rbsp_data);
    return 0;
}

// 解析H.265 SEI NAL单元（带防竞争处理）
int parse_h265_sei_nal(const xmedia_u8* nal_data, int nal_size, FILE *file)
{
    int rbsp_size;
    xmedia_u8* rbsp_data;
    
    int payload_type = 0;
    xmedia_u8 byte;

    int pos = 2; // 跳过NAL头（1字节）

    if (nal_size < 3) {
        return 0;
    }
    
    // 首先进行防竞争字节处理
    rbsp_data = remove_emulation_prevention_bytes(nal_data, nal_size, &rbsp_size);
    if (!rbsp_data) {
        return 0;
    }
    
    pos = 2; // 跳过NAL头（2字节）
    
    while (pos < rbsp_size) {
        // 解析payload_type（使用类似UTF-8的变长编码）
        do {
            if (pos >= rbsp_size) {
                break;
            }
            byte = rbsp_data[pos++];
            payload_type += byte;
        } while (byte == 0xFF);
        
        if (pos >= rbsp_size) {
            break;
        }
        
        // 解析payload_size（使用类似UTF-8的变长编码）
        int payload_size = 0;
        do {
            if (pos >= rbsp_size) {
                break;
            }
            byte = rbsp_data[pos++];
            payload_size += byte;
        } while (byte == 0xFF);
        
        if (pos + payload_size > rbsp_size) {
            printf("Warning: SEI payload size exceeds RBSP size\n");
            break;
        }

        save_gyro_data(rbsp_data+pos, payload_size, file);
        
        pos += payload_size;

        // 跳过rbsp_trailing_bits（如果有）
        if (pos < rbsp_size) {
            // rbsp_trailing_bits以1开始，后面跟0
            if (rbsp_data[pos] == 0x80) {
                pos++;
            }
        }
    }
    
    free(rbsp_data);
    return 0;
}


// 从码流中解析SEI数据（带防竞争处理）
void parse_stream_sei_with_emulation_prevention(const xmedia_u8* stream_data, int stream_size, int is_h265, FILE *file)
{
    int pos = 0;
    int next_start;    
    int nal_size;
    xmedia_u8 nal_header;
    xmedia_u8 nal_unit_type;
    int is_sei_nal = 0;
    int start_code_pos;

    while (pos < stream_size - 4) {
        // 查找4字节起始码
        start_code_pos = find_4byte_start_code(stream_data, stream_size, pos);
        if (start_code_pos == -1) {
            break;
        }
        
        pos = start_code_pos + 4; // 移动到起始码后
        
        if (pos >= stream_size) {
            break;
        }
        
        // 查找下一个起始码以确定NAL单元大小
        next_start = find_4byte_start_code(stream_data, stream_size, pos);
        if (next_start == -1) {
            nal_size = stream_size - pos;
        } else {
            nal_size = next_start - pos;
        }
        
        if (nal_size <= 0) {
            pos += 1;
            continue;
        }
        
        // 获取NAL单元类型
        nal_header = stream_data[pos];
        
        if (is_h265) {
            nal_unit_type = (nal_header >> 1) & 0x3F; // H.265: 6位类型
        } else {
            nal_unit_type = nal_header & 0x1F; // H.264: 5位类型
        }
        
        // 检查是否为SEI NAL单元
        if (is_h265) {
            // H.265: 前缀SEI(39)和后缀SEI(40)
            is_sei_nal = (nal_unit_type == 39 || nal_unit_type == 40);
        } else {
            // H.264: SEI类型为6
            is_sei_nal = (nal_unit_type == 6);
        }
        //printf("  NAL size: %d bytes, Type: %d\n", nal_size, nal_unit_type);
        if (is_sei_nal) {
            if (is_h265) {
                parse_h265_sei_nal(stream_data + pos, nal_size, file);
            } else {
                parse_h264_sei_nal(stream_data + pos, nal_size, file);
            }        
        }
        
        pos += nal_size;
    }
    
}

xmedia_s32 msensor_spi_init(void)
{
    xmedia_s32 fd, ret;
    xmedia_s32 mode = SPI_MODE_3; /* | SPI_LSB_FIRST | SPI_LOOP | SPI_CS_HIGH */
    xmedia_s32 bits = 8;
    xmedia_u64 speed = 10000000;
    xmedia_char *dev = "/dev/spidev2.0";

    fd = open(dev, O_RDWR);
    if (fd < 0) {
        SAMPLE_PRT("open spi def failed\n");
        return XMEDIA_FAILURE;
    }

    /* set spi mode */
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode); /* SPI_IOC_WR_MODE32 */
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("can't set spi mode\n");
        goto end;
    }

    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode); /* SPI_IOC_RD_MODE32 */
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("can't get spi mode\n");
        goto end;
    }

    /*
     * bits per word
     */
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("can't set bits per word\n");
        goto end;
    }

    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("can't get bits per word\n");
        goto end;
    }

    /* set spi max speed */
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("can't set bits max speed HZ\n");
        goto end;
    }

    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("can't set bits max speed HZ\n");
        goto end;
    }

    printf("spi mode: 0x%x\n", mode);
    printf("bits per word: %d\n", bits);
    printf("max speed: %lld Hz (%lld KHz)\n", speed, speed / 1000); /* speed with precision of 1000 */

end:
    close(fd);
    return ret;
}


xmedia_s32 msensor_init(xmedia_s32 dev)
{
    xmedia_s32 ret;
    xmedia_u32 buf_size = GYRO_BUF_SIZE + ACC_BUF_SIZE;
    xmedia_char buf_name[20] = { 0 };
    xmedia_msensor_param param;

    snprintf(buf_name, sizeof(buf_name), "msensor_dev%d", dev);
    g_msensor_attr[dev].phys_addr = xmedia_mmz_alloc(NULL, buf_name, buf_size);
    if (g_msensor_attr[dev].phys_addr == 0) {
        SAMPLE_PRT("alloc mmz for Msensor failed\n");
        return XMEDIA_FAILURE;
    }

    g_msensor_attr[dev].virt_addr = xmedia_mmz_map(g_msensor_attr[dev].phys_addr, buf_size, XMEDIA_FALSE);
    if (g_msensor_attr[dev].virt_addr == 0) {
        SAMPLE_PRT("msensor malloc mmz map failed\n");
        xmedia_mmz_free(g_msensor_attr[dev].phys_addr);
        return XMEDIA_FAILURE;
    }

    (xmedia_void) memset(g_msensor_attr[dev].virt_addr, 0, buf_size);

    g_msensor_attr[dev].buf_len = buf_size;

    /* set device work mode */
    param.attr.device_mask = XMEDIA_MSENSOR_DEVICE_GYRO | XMEDIA_MSENSOR_DEVICE_ACC;
    param.attr.temperature_mask = XMEDIA_MSENSOR_TEMP_GYRO | XMEDIA_MSENSOR_TEMP_ACC;

    /* set gyro samplerate and full scale range */
    param.config.gyro_config.odr = GYRO_DATA_ODR * XMEDIA_MSENSOR_GRADIENT;
    param.config.gyro_config.fsr = g_gyro_data_range * XMEDIA_MSENSOR_GRADIENT;
    /* set accel samplerate and full scale range */
    param.config.acc_config.odr = ACC_DATA_ODR * XMEDIA_MSENSOR_GRADIENT;
    param.config.acc_config.fsr = ACC_DATA_RANGE * XMEDIA_MSENSOR_GRADIENT;

    (xmedia_void) memcpy(&param.buf_attr, &g_msensor_attr[dev], sizeof(xmedia_msensor_buf_attr));

    ret = xmedia_msensor_create_dev(dev, &param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("msensor_create failed\n");
        return XMEDIA_FAILURE;
    }

    return ret;
}

xmedia_void msensor_deinit(xmedia_s32 dev)
{
    xmedia_s32 ret;

    ret = xmedia_msensor_destroy_dev(dev);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("msensor deinit failed , ret:0x%x !\n", ret);
    }

    ret = xmedia_mmz_unmap(g_msensor_attr[dev].virt_addr);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("msensor mmz map failed, ret:0x%x !\n", ret);
    }

    ret = xmedia_mmz_free(g_msensor_attr[dev].phys_addr);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("msensor mmz free failed, ret:0x%x !\n", ret);
    }

    g_msensor_attr[dev].phys_addr = 0;
    g_msensor_attr[dev].virt_addr = NULL;

    return;
}

xmedia_s32 msensor_start(xmedia_s32 dev)
{
    xmedia_s32 ret;

    printf("spi init\n");

    ret = msensor_spi_init();
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("init spi fail.ret:0x%x !\n", ret);
        return ret;
    }

    printf("spi init ok\n");

    ret = msensor_init(dev);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("init gyro fail.ret:0x%x !\n", ret);
        return ret;
    }

    ret = xmedia_msensor_start_dev(dev);;
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("start gyro fail.ret:0x%x !\n", ret);
        goto msensor_start_fail;
    }

    printf("motion sensor start ok\n");
    return XMEDIA_SUCCESS;

msensor_start_fail:
    msensor_deinit(dev);

    return ret;
}

xmedia_void msensor_stop(xmedia_s32 dev)
{
    xmedia_msensor_stop_dev(dev);;
    msensor_deinit(dev);

    return;
}

xmedia_venc_stream *msensor_mix_gyro_stream(xmedia_payload_type payload_type, FILE* pFd, xmedia_venc_stream* pstStream, xmedia_u32 stream_idex, msensor_data_info *pstdata_info)
{
    xmedia_s32 i;
    xmedia_msensor_data_info gyro_data;
    static xmedia_venc_stream *last_stream = XMEDIA_NULL; 
    static xmedia_u32 delta_pts = 0;
    xmedia_venc_stream *temp_stream;
    xmedia_s32 addr_offset;
    xmedia_s32 x;
    xmedia_s32 y;
    xmedia_s32 z;
    xmedia_s32 temp;
    xmedia_u64 pts;
    static xmedia_char *gyro_data_buf = XMEDIA_NULL;
    static xmedia_char *sei_data_buf = XMEDIA_NULL;
    xmedia_u32 gyro_data_len = 0;
    xmedia_u32 sei_data_len = 0;
    xmedia_char * txt_buf;
    xmedia_char txt_tmp[128] = {0};
    xmedia_u32 txt_pos = 0;
    xmedia_char * temp_ptr;
    int cyclic;
    xmedia_s32 ret;
    xmedia_s32 msensor_dev;
    xmedia_s32 user_id;

    if (payload_type != PT_H264 && payload_type != PT_H265) {
        return pstStream;
    }

    if (stream_idex == 0) {
        last_stream = pstStream;
        if (gyro_data_buf == XMEDIA_NULL) {
            gyro_data_buf = malloc(4*1024);
        }

        if (sei_data_buf == XMEDIA_NULL) {
            sei_data_buf = malloc(6*1024);
        }
        
        return XMEDIA_NULL;
    }

    if ((gyro_data_buf == XMEDIA_NULL) || (sei_data_buf == XMEDIA_NULL)) {
        return pstStream;
    }

    msensor_dev = pstdata_info->msensor_dev;
    user_id = pstdata_info->user_id;

    gyro_data.id = user_id;
    gyro_data.data_type = XMEDIA_MSENSOR_DATA_TYPE_GYRO;
    if (stream_idex != 0xFFFFFFFF) {
        delta_pts = pstStream->pack[0].pts - last_stream->pack[0].pts;
        gyro_data.begin_pts = last_stream->pack[0].pts;
        gyro_data.end_pts = pstStream->pack[0].pts;
    } else {
        gyro_data.begin_pts = last_stream->pack[0].pts;
        gyro_data.end_pts = gyro_data.begin_pts + delta_pts;
    }
    
    ret = xmedia_msensor_get_data(msensor_dev, &gyro_data);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("msensor get data failed, pts[%lld]-pts[%lld] ret:0x%x !\n", gyro_data.begin_pts, gyro_data.end_pts, ret);
        temp_stream = last_stream;
        last_stream = pstStream;

        if (stream_idex == 0xFFFFFFFF) {
            temp_stream = last_stream;
            last_stream = XMEDIA_NULL;

            if (gyro_data_buf) {
                free(gyro_data_buf);
                gyro_data_buf = XMEDIA_NULL;
            }

            if (sei_data_buf) {
                free(sei_data_buf);
                sei_data_buf = XMEDIA_NULL;
            }
        }
        return temp_stream;
    }
    
    
    //SAMPLE_PRT("get gyro data start pts %llu, end pts %llu, diff %llu, num %d %d\n", gyro_data.begin_pts,
    //            gyro_data.end_pts, gyro_data.end_pts - gyro_data.begin_pts, gyro_data.data[0].num,
    //            gyro_data.data[1].num);usleep(100000);

    gyro_data_len = (gyro_data.data[0].num + gyro_data.data[1].num) * 24;
    temp_ptr = gyro_data_buf;
    addr_offset = g_msensor_attr[msensor_dev].phys_addr - (xmedia_u64)(xmedia_uintptr_t)g_msensor_attr[msensor_dev].virt_addr;
    txt_buf = (xmedia_char *)malloc(1024*1024);
    txt_pos = 0;
    memset(txt_tmp, 0, sizeof(txt_tmp));
    
    for (cyclic = 0; cyclic < 2; cyclic++) {
        for (i = 0; i < gyro_data.data[cyclic].num; i++) {
            x = *(xmedia_s32 *)(xmedia_uintptr_t)((xmedia_u64)(xmedia_uintptr_t)(gyro_data.data[cyclic].x_phys_addr + i) -
                                               addr_offset);
            y = *(xmedia_s32 *)(xmedia_uintptr_t)((xmedia_u64)(xmedia_uintptr_t)(gyro_data.data[cyclic].y_phys_addr + i) -
                                               addr_offset);
            z = *(xmedia_s32 *)(xmedia_uintptr_t)((xmedia_u64)(xmedia_uintptr_t)(gyro_data.data[cyclic].z_phys_addr + i) -
                                               addr_offset);
            temp = *(xmedia_s32 *)(xmedia_uintptr_t)((xmedia_u64)(xmedia_uintptr_t)(gyro_data.data[cyclic].temperature_phys_addr + i) -
                                               addr_offset);
            pts =
             *(xmedia_u64 *)(xmedia_uintptr_t)((xmedia_u64)(xmedia_uintptr_t)(gyro_data.data[cyclic].pts_phys_addr + i) -
                                               addr_offset);

            *(xmedia_s32 *)temp_ptr = x;
            temp_ptr += sizeof(xmedia_s32);
            *(xmedia_s32 *)temp_ptr = y;
            temp_ptr += sizeof(xmedia_s32);
            *(xmedia_s32 *)temp_ptr = z;
            temp_ptr += sizeof(xmedia_s32);
            *(xmedia_s32 *)temp_ptr = temp;
            temp_ptr += sizeof(xmedia_s32);
            *(xmedia_u64 *)temp_ptr = pts;
            temp_ptr += sizeof(xmedia_u64);

            if (txt_buf) {
                sprintf(txt_tmp, "%12d,%12d,%12d,%12d,%22llu, \n", x, y, z, temp, pts);
                memcpy(txt_buf+txt_pos, txt_tmp, strlen(txt_tmp));
                txt_pos += strlen(txt_tmp);
            }
            
        }
    }

    if (txt_buf) {
        fwrite(txt_buf, txt_pos, 1, pstdata_info->gyro_txt_fp);
        free(txt_buf);
        txt_buf = XMEDIA_NULL;
    }

    if (payload_type == PT_H264) {
        h264_encode_sei(sei_data_buf, gyro_data_buf, gyro_data_len, &sei_data_len);
        fwrite(sei_data_buf, sei_data_len, 1, pFd);
        if (pstdata_info->gyro_bin_fp) {
            fwrite(sei_data_buf, sei_data_len, 1, pstdata_info->gyro_bin_fp);
        }
    } else if (payload_type == PT_H265) {
        h265_encode_sei(sei_data_buf, gyro_data_buf, gyro_data_len, &sei_data_len);
        fwrite(sei_data_buf, sei_data_len, 1, pFd);
        if (pstdata_info->gyro_bin_fp) {
            fwrite(sei_data_buf, sei_data_len, 1, pstdata_info->gyro_bin_fp);
        }
    }

    for (i = 0; i < last_stream->pack_count; i++) {
        fwrite(last_stream->pack[i].vir_addr + last_stream->pack[i].offset,
               last_stream->pack[i].len - last_stream->pack[i].offset, 1, pFd);
    }

    if (stream_idex == 0xFFFFFFFF) {
        temp_stream = last_stream;
        last_stream = XMEDIA_NULL;

        if (gyro_data_buf) {
            free(gyro_data_buf);
            gyro_data_buf = XMEDIA_NULL;
        }

        if (sei_data_buf) {
            free(sei_data_buf);
            sei_data_buf = XMEDIA_NULL;
        }
    } else {
        
        temp_stream = last_stream;
        last_stream = pstStream;
    }
    return temp_stream;

}

xmedia_void *msensor_get_data_thread(xmedia_void *args)
{
    xmedia_s32 ret;
    xmedia_s32 msensor_dev;
    xmedia_s32 user_id;
    msensor_data_info msensor_info;
    msensor_get_data_info *get_data_info = (msensor_get_data_info *)args;
    FILE *gyro_bin_fp = XMEDIA_NULL;
    FILE *gyro_txt_fp = XMEDIA_NULL;

    xmedia_u32 stream_index = 0;

    //xmedia_u16 temp_pts = 0;
    xmedia_venc_chn_attr venc_chn_attr;
    xmedia_payload_type payload_type;

    xmedia_char file_name[512];
    xmedia_char *file_path="./";
    FILE *file_ptr;

    struct timeval time_out;
    xmedia_venc_stream *venc_stream;
    xmedia_venc_chn_status stat;
    xmedia_u32 venc_mask = 0;
    xmedia_venc_stream *temp_stream;

    printf("start dump gyro data\n");

    gyro_bin_fp = fopen(GYRO_BIN_FILE, "w+");
    if (gyro_bin_fp == XMEDIA_NULL) {
        SAMPLE_PRT("Error: cannot open imu data\n");
        return XMEDIA_NULL;
    }

    gyro_txt_fp = fopen(GYRO_TXT_FILE, "w+");
    if (gyro_txt_fp == XMEDIA_NULL) {
        fclose(gyro_bin_fp);
        SAMPLE_PRT("Error: cannot open imu data\n");
        return XMEDIA_NULL;
    }

    ret = xmedia_venc_get_chn_attr(get_data_info->venc_chn, &venc_chn_attr);
    if (XMEDIA_SUCCESS != ret) {
        SAMPLE_PRT("xmedia_venc_get_chn_attr chn[%d] failed with %#x!\n", get_data_info->venc_chn, ret);
        return XMEDIA_NULL;
    }

    payload_type = venc_chn_attr.venc_attr.en_type;

    if (payload_type != PT_H264 && payload_type != PT_H265) {
        goto exit;
    }

    if (payload_type == PT_H264) {
        snprintf(file_name, 512, "%s/stream_chn%d%s", file_path, get_data_info->venc_chn, ".h264");
    } else {
        snprintf(file_name, 512, "%s/stream_chn%d%s", file_path, get_data_info->venc_chn, ".h265");
    }
    file_ptr = fopen(file_name, "wb");
    if (!file_ptr) {
        SAMPLE_PRT("open file %s failed!\n", file_name);
        return XMEDIA_NULL;
    }
    chmod(file_name, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    
    venc_mask |= 1 << get_data_info->venc_chn;

    msensor_dev = get_data_info->msensor_dev;
    ret = xmedia_msensor_add_user(msensor_dev, &user_id);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("msensor add user failed, ret:0x%x !\n", ret);
        goto exit;
    }

    msensor_info.msensor_dev = msensor_dev;
    msensor_info.user_id = user_id;
    msensor_info.gyro_txt_fp = gyro_txt_fp;
    msensor_info.gyro_bin_fp = gyro_bin_fp;

    while (get_data_info->thread_start) {
        time_out.tv_sec  = 2;
        time_out.tv_usec = 0;
        ret = xmedia_venc_select(venc_mask, &time_out);
        if (ret == XMEDIA_ERRCODE_INVALID_PARAM || ret == XMEDIA_FAILURE) {
            SAMPLE_PRT("select err\n");
            break;
        } else if (ret == XMEDIA_ERRCODE_TIMEOUT) {
            usleep(100 * 1000);
            continue;
        }

        ret = xmedia_venc_query_status(get_data_info->venc_chn, &stat);
        if (XMEDIA_SUCCESS != ret) {
            SAMPLE_PRT("xmedia_venc_query_status chn[%d] failed with %#x!\n", get_data_info->venc_chn, ret);
            break;
        }

        if (0 == stat.cur_packs) {
            continue;
        }

        venc_stream = (xmedia_venc_stream *)malloc(sizeof(xmedia_venc_stream));
        if (XMEDIA_NULL == venc_stream) {
            SAMPLE_PRT("malloc stream pack failed!\n");
            break;
        }
        memset(venc_stream, 0, sizeof(xmedia_venc_stream));
        venc_stream->pack = (xmedia_venc_pack*)malloc(sizeof(xmedia_venc_pack) * stat.cur_packs);
        if (XMEDIA_NULL == venc_stream->pack) {
            SAMPLE_PRT("malloc stream pack failed!\n");
            free(venc_stream);
            break;
        }

        venc_stream->pack_count = stat.cur_packs;
        ret = xmedia_venc_get_stream(get_data_info->venc_chn, venc_stream, -1);
        if (XMEDIA_SUCCESS != ret) {
            free(venc_stream->pack);
            venc_stream->pack = XMEDIA_NULL;
            free(venc_stream);
            SAMPLE_PRT("xmedia_venc_get_stream failed with %#x!\n", ret);
            break;
        }

        if (PT_JPEG != payload_type) {
            temp_stream = msensor_mix_gyro_stream(payload_type, file_ptr, venc_stream, stream_index, &msensor_info);
            stream_index++;
            if (XMEDIA_NULL != temp_stream) {
                xmedia_venc_release_stream(get_data_info->venc_chn, temp_stream);                
                free(temp_stream->pack);
                temp_stream->pack = XMEDIA_NULL;
                free(temp_stream);
            }
        }

    }

    temp_stream = msensor_mix_gyro_stream(payload_type, file_ptr, XMEDIA_NULL, 0xFFFFFFFF, &msensor_info);
    if (XMEDIA_NULL != temp_stream) {
        xmedia_venc_release_stream(get_data_info->venc_chn, temp_stream);        
        free(temp_stream->pack);
        temp_stream->pack = XMEDIA_NULL;
        free(temp_stream);
    }

    ret = xmedia_msensor_delete_user(msensor_dev, user_id);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("msensor delete user failed, ret:0x%x !\n", ret);
    }

exit:
    fclose(gyro_bin_fp);
    fclose(gyro_txt_fp);

    return XMEDIA_NULL;
}

xmedia_s32 sample_msensor_parse_data(xmedia_void)
{
    FILE *file = fopen(GYRO_BIN_FILE, "rb");
    FILE *gyro_parse_fp = XMEDIA_NULL;

    gyro_parse_fp = fopen(GYRO_PARSE_FILE, "w+");
    if (gyro_parse_fp == XMEDIA_NULL) {
        SAMPLE_PRT("Error: cannot open imu data\n");
        return XMEDIA_NULL;
    }
    if (file == NULL) {
        fclose(gyro_parse_fp);
        perror("无法打开文件");
        return 1;
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // 分配内存
    unsigned char *buffer = (unsigned char*)malloc(file_size + 1);
    if (buffer == NULL) {
        perror("内存分配失败");
        fclose(file);
        return 1;
    }

    // 读取整个文件
    size_t bytes_read = fread(buffer, 1, file_size, file);
    buffer[bytes_read] = '\0'; // 添加字符串结束符
    fclose(file);

    parse_stream_sei_with_emulation_prevention(buffer, bytes_read, 1, gyro_parse_fp);
    fclose(gyro_parse_fp);

    // 清理
    free(buffer);

    return 0;
}

xmedia_s32 sample_msensor_start_dump_data(xmedia_s32 dev)
{

    memset(&g_get_data_info[dev], 0, sizeof(msensor_get_data_info));

    g_get_data_info[dev].thread_start = XMEDIA_TRUE;
    g_get_data_info[dev].data_type = XMEDIA_MSENSOR_DATA_TYPE_GYRO;
    g_get_data_info[dev].time_intval = 4;
    g_get_data_info[dev].msensor_dev = dev;
    pthread_create(&g_msensor_pid[dev], 0, msensor_get_data_thread, (xmedia_void *)&g_get_data_info[dev]);

    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_msensor_stop_dump_data(xmedia_s32 dev)
{
    if (XMEDIA_TRUE == g_get_data_info[dev].thread_start) {
        g_get_data_info[dev].thread_start = XMEDIA_FALSE;

        if (g_msensor_pid[dev] > 0) {
            pthread_join(g_msensor_pid[dev], 0);
        }
    }

    return XMEDIA_SUCCESS;
}


xmedia_s32 sample_msensor_init(xmedia_s32 dev_cnt, xmedia_s32 *dev)
{
    xmedia_s32 i;
    xmedia_s32 ret = XMEDIA_SUCCESS;

    xmedia_msensor_init();

    for (i = 0; i < dev_cnt; i++) {
        ret = msensor_start(dev[i]);
    }

    return ret;
}

xmedia_void sample_msensor_exit(xmedia_s32 dev_cnt, xmedia_s32 *dev)
{
    xmedia_s32 i;

    for (i = 0; i < dev_cnt; i++) {
        msensor_stop(dev[i]);
    }
    xmedia_msensor_exit();
}

xmedia_s32 sample_venc_h265_msensor(xmedia_bool set_roi)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 blk_size = 0;
    xmedia_video_size pic_size = { 0 };
    xmedia_video_size sub_pic_size = { 0 };
    sample_comm_video_param video_param = { 0 };
    sample_sys_config sys_config = { 0 };
    xmedia_s32 msensor_dev = 0;
    xmedia_s32 vi_dev = 0;
    xmedia_s32 vi_pipe = 0;
    xmedia_s32 vi_chn = 0;
    sample_vi_config vi_config = { 0 };

    xmedia_s32 sensor_type = sample_msensor_sensor_type[0];
    xmedia_u32 framerate = 0;
    vi_sensor_info sensor_info = { 0 };
    sample_isp_param isp_param = { 0 };
    xmedia_isp_config isp_config = { 0 };
    xmedia_bool mirror = XMEDIA_FALSE;
    xmedia_bool flip = XMEDIA_FALSE;

    xmedia_s32 vpss_chnl = 0;
    xmedia_s32 vpss_pipe = 0;
    xmedia_s32 vpss_ichn = 0;
    xmedia_s32 vpss_ochn[1] = { 0 };
    sample_vpss_config vpss_config = { 0 };
    xmedia_video_size vpss_ochn_size[VPSS_MAX_OCHN_NUM] = { 0 };

    xmedia_s32 venc_chnl = 0;
    xmedia_s32 venc_chn[1] = { 0 };
    xmedia_s32 venc_chn_cnt = 1;
    sample_venc_config venc_config = { 0 };
    xmedia_venc_gop_mode gop_mode;

    video_param.video_fmt = XMEDIA_VIDEO_FMT_LINEAR;
    video_param.pixel_fmt = XMEDIA_VIDEO_PIXEL_FMT_YVU_SEMIPLANAR_420;
    video_param.data_width = XMEDIA_VIDEO_DATA_WIDTH_8;
    video_param.compress_mode = XMEDIA_VIDEO_COMPRESS_MODE_NONE;

    sample_comm_vi_get_sensor_info(sensor_type, &sensor_info);
    sample_comm_vi_get_framerate_by_sensor(sensor_type, &framerate);

    gop_mode = VENC_GOPMODE_NORMALP;

    // sys init
    sys_config.sys_conf.pipe_mode[vi_pipe].vicap_viproc_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[vi_pipe].viproc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;
    sys_config.sys_conf.pipe_mode[vi_pipe].gdc_vpss_mode = XMEDIA_WORK_MODE_OFFLINE;

    sys_config.vb_conf.max_pool_cnt = 25;

    pic_size.width = sensor_info.width;
    pic_size.height = sensor_info.height;
    blk_size = sample_comm_sys_get_buffer_size(pic_size, video_param.video_fmt, sensor_info.pixel_format,
                                               sensor_info.bit_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[0].block_size = blk_size;
    sys_config.vb_conf.common_pool[0].block_cnt = 2;

    blk_size = sample_comm_sys_get_buffer_size(pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[1].block_size = blk_size;
    sys_config.vb_conf.common_pool[1].block_cnt = 4;

    sub_pic_size.width = 640;
    sub_pic_size.height = 480;
    blk_size = sample_comm_sys_get_buffer_size(sub_pic_size, video_param.video_fmt, video_param.pixel_fmt,
                                               video_param.data_width, video_param.compress_mode);
    sys_config.vb_conf.common_pool[2].block_size = blk_size;
    sys_config.vb_conf.common_pool[2].block_cnt = 2;

    sys_config.vb_conf.supplement_config = 1;

    ret = sample_msensor_sys_init(&sys_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("sys init failed !\n");
        return ret;
    }

    sample_msensor_init(1, &msensor_dev);

    vi_config.dev_info[vi_dev].dev_en = XMEDIA_TRUE;
    vi_config.dev_info[vi_dev].dev_no = vi_dev;
    vi_config.dev_info[vi_dev].sensor_type = sensor_type;
    vi_config.pipe_info[vi_pipe].pipe_en = XMEDIA_TRUE;
    vi_config.pipe_info[vi_pipe].pipe_no = vi_pipe;
    vi_config.pipe_info[vi_pipe].chn_info[vi_chn].chn_en = XMEDIA_TRUE;
    vi_config.pipe_info[vi_pipe].chn_info[vi_chn].chn_no = vi_chn;
    vi_config.dev_bind_pipe[vi_dev].pipe[0] = vi_pipe;
    vi_config.dev_bind_pipe[vi_dev].pipe[1] = -1;

    isp_config.fps = framerate;
    isp_config.mode_config.work_mode = XMEDIA_ISP_WORK_MODE_MASTER;
    isp_config.mode_config.master_mode.blend_stat_enable = XMEDIA_FALSE;
    isp_config.mode_config.master_mode.slave_num = 0;
    isp_config.pixel_fmt = sensor_info.pixel_format;
    isp_config.size.height = sensor_info.height;
    isp_config.size.width = sensor_info.width;
    isp_config.wdr_mode = sensor_info.wdr_mode;

    // isp pipe init
    isp_param.pipe[vi_pipe] = vi_pipe;
    isp_param.isp_info[vi_pipe].isp_pipe_en = XMEDIA_TRUE;
    isp_param.isp_info[vi_pipe].isp_sensor_en = XMEDIA_TRUE;
    isp_param.isp_info[vi_pipe].sensor_type = sensor_type;
    isp_param.isp_info[vi_pipe].mirror = mirror;
    isp_param.isp_info[vi_pipe].flip = flip;
    memcpy(&(isp_param.isp_info[vi_pipe].isp_config), &isp_config, sizeof(xmedia_isp_config));

    // isp init
    ret = sample_comm_isp_init(&isp_param, &vi_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("isp init failed!\n");
        goto exit0;
    }

    // vi start
    ret = sample_comm_vi_start(&vi_config, &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("start vi failed!\n");
        goto exit1;
    }

    // isp start
    ret = sample_comm_isp_start(&isp_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("start isp failed!\n");
        goto exit2;
    }

    // vpss config
    ret = sample_comm_vpss_get_default_pipe_cfg(&vpss_config.pipe_info[vpss_pipe].pipe_config, pic_size, &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get default pipe cfg failed !\n");
        goto exit2;
    }

    vpss_ochn_size[0].width = sensor_info.width;
    vpss_ochn_size[0].height = sensor_info.height;

    vpss_config.pipe_info[vpss_pipe].pipe_en = XMEDIA_TRUE;
    vpss_config.pipe_info[vpss_pipe].pipe_no = vpss_pipe;

    vpss_chnl = vpss_ochn[0];
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_en = XMEDIA_TRUE;
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_no = vpss_ochn[0];
    ret = sample_comm_vpss_get_default_ochn_cfg(&vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config,
                                                vpss_ochn_size[vpss_chnl], &video_param);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("get default pipe cfg failed !\n");
        goto exit2;
    }
    vpss_config.pipe_info[vpss_pipe].chn_info[vpss_chnl].chn_config.depth = 0;

    // vpss start
    ret = sample_comm_vpss_start(&vpss_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vpss start failed !\n");
        goto exit2;
    }

    ret = sample_comm_sys_vi_bind_vpss(vi_pipe, vi_chn, vpss_pipe, vpss_ichn);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vi bind vpss failed !\n");
        goto exit3;
    }

    venc_chnl = venc_chn[0];
    venc_config.chn_info[venc_chnl].venc_en = XMEDIA_TRUE;
    venc_config.chn_info[venc_chnl].venc_chn = venc_chnl;
    venc_config.chn_info[venc_chnl].payload_type = PT_H265;
    venc_config.chn_info[venc_chnl].rc_mode = VENC_RC_MODE_H265CBR;
    venc_config.chn_info[venc_chnl].venc_gop_attr.gop_mode = gop_mode;
    sample_comm_venc_get_default_chn_info(vpss_ochn_size[0], framerate, &venc_config.chn_info[venc_chnl]);

    ret = sample_comm_venc_start(&venc_config);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("venc start failed !\n");
        goto exit4;
    }

    if (set_roi == XMEDIA_TRUE) {
        ret = sample_comm_venc_set_roi(venc_chn, venc_chn_cnt);
        if (ret != XMEDIA_SUCCESS) {
            SAMPLE_PRT("venc start get stream failed !\n");
            goto exit4;
        }
    }

    ret = sample_comm_sys_vpss_bind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
    if (ret != XMEDIA_SUCCESS) {
        SAMPLE_PRT("vpss bind venc failed !\n");
        goto exit5;
    }

    sample_msensor_start_dump_data(msensor_dev);

    PAUSE(g_force_exit);

    sample_msensor_stop_dump_data(msensor_dev);

    sample_comm_sys_vpss_unbind_venc(vpss_pipe, vpss_ochn[0], venc_chn[0]);
exit5:
    sample_comm_venc_stop(&venc_config);
exit4:
    sample_comm_sys_vi_unbind_vpss(vi_pipe, vi_chn, vpss_pipe, vpss_ichn);
exit3:
    sample_comm_vpss_stop(&vpss_config);
exit2:
    sample_comm_isp_stop(&isp_param);
exit1:
    sample_comm_vi_stop(&vi_config);
exit0:
    sample_comm_isp_exit(&isp_param);
    sample_msensor_exit(1, &msensor_dev);
    sample_msensor_sys_exit();

    return ret;
}

int main(int argc,char **argv)
{
    xmedia_s32 ret = XMEDIA_SUCCESS;
    xmedia_s32 index = 0;
    xmedia_bool set_roi = XMEDIA_FALSE;

    signal(SIGINT, sample_msensor_handle_sig);
    signal(SIGTERM, sample_msensor_handle_sig);

    if (argc < 2) {
        sample_msensor_usage(argv[0]);
        return 0;
    }

    index = atoi(argv[1]);
    if (argc == 2) {
        switch (index) {
            case 0:
                ret = sample_venc_h265_msensor(set_roi);
                break;
            case 1:
                sample_msensor_parse_data();
                break;
            default:
                sample_msensor_usage(argv[0]);
                return 0;
        }
    } 

    if (XMEDIA_SUCCESS == ret) {
        SAMPLE_PRT("program exit normally!\n");
    } else {
        SAMPLE_PRT("program exit abnormally!\n");
    }

    return 0;
}



