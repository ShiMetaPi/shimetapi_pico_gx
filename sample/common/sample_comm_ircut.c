
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "sample_comm_ircut.h"

static xmedia_s32  g_mem_dev = 0;

static xmedia_s32 sample_comm_ircut_memopen(xmedia_void)
{
    if (g_mem_dev <= 0) {
        g_mem_dev = open ("/dev/mem", O_CREAT | O_RDWR | O_SYNC);
        if (g_mem_dev <= 0) {
            return XMEDIA_FAILURE;
        }
    }
    return XMEDIA_SUCCESS;
}

static xmedia_void sample_comm_ircut_memclose(xmedia_void)
{
    if (g_mem_dev > 0) {
        close(g_mem_dev);
        g_mem_dev = 0;
    }
}

static xmedia_void* sample_comm_ircut_memmap(xmedia_u32 phy_addr, xmedia_u32 size)
{
    xmedia_u32 diff;
    xmedia_u32 page_phy;
    xmedia_u32 page_size;
    xmedia_u8* page_addr;

    page_phy = phy_addr & 0xfffff000;
    diff = phy_addr - page_phy;

    page_size = ((size + diff - 1) & 0xfffff000) + 0x1000;
    page_addr = mmap((void *)0, page_size, PROT_READ|PROT_WRITE, MAP_SHARED, g_mem_dev, page_phy);
    if (MAP_FAILED == page_addr) {
        perror("mmap error\n");
        return NULL;
    }

    return (xmedia_void *) (page_addr + diff);
}

static xmedia_s32 sample_comm_ircut_memunmap(xmedia_void* vir_addr, xmedia_u32 size)
{
    xmedia_u32 page_addr;
    xmedia_u32 page_size;
    xmedia_u32 diff;

    page_addr = (((xmedia_u32)vir_addr) & 0xfffff000);
    diff      = (xmedia_u32)vir_addr - page_addr;
    page_size = ((size + diff - 1) & 0xfffff000) + 0x1000;

    return munmap((xmedia_void*)page_addr, page_size);
}

static xmedia_void sample_comm_ircut_set_reg(xmedia_u32 addr,xmedia_u32 value)
{
    xmedia_u32 *port_vir_addr;
    port_vir_addr  = sample_comm_ircut_memmap(addr,sizeof(value));
    *port_vir_addr = value;
    sample_comm_ircut_memunmap(port_vir_addr,sizeof(value));
}

xmedia_s32 sample_comm_ircut_set_status_ir(sample_comm_board_type board_type, xmedia_s32 vi_dev)
{
    xmedia_s32 ret = 0;

    if (vi_dev >= VI_MAX_DEV_NUM || board_type >= BOARD_TYPE_MAX) {
        return XMEDIA_FAILURE;
    }

    ret = sample_comm_ircut_memopen();
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    switch (board_type) {
        case BOARD_TYPE_XM7206_V10_RB:{
            //pin_mux-- GPIO4_6,GPIO4_3
            sample_comm_ircut_set_reg(0x100C0024,0x05); // mutex
            sample_comm_ircut_set_reg(0x100C0018,0x05); // mutex
            sample_comm_ircut_set_reg(0x120B4400,0x48); // dir
            sample_comm_ircut_set_reg(0x120B4120,0x40); // data
            break;
        }
        case BOARD_TYPE_XM7206_V10B_RBA:{
            //pin_mux-- GPIO3_5, GPIO3_6
            sample_comm_ircut_set_reg(0x100C0000,0x00); // mutex
            sample_comm_ircut_set_reg(0x100C0004,0x00); // mutex
            sample_comm_ircut_set_reg(0x120B3400,0x60); // dir
            sample_comm_ircut_set_reg(0x120B3180,0x20); // data
            break;
        }
        case BOARD_TYPE_XM7206_V10B_EVB:{
            //pin_mux-- GPIO5_4, GPIO5_5
            sample_comm_ircut_set_reg(0x100C003C,0x00); // mutex
            sample_comm_ircut_set_reg(0x100C0040,0x00); // mutex
            sample_comm_ircut_set_reg(0x120B5400,0x30); // dir
            sample_comm_ircut_set_reg(0x120B50C0,0x10); // data
            break;
        }
        case BOARD_TYPE_XM7206_V11A_RBA:{
            //pin_mux-- GPIO7_5, GPIO7_6
            sample_comm_ircut_set_reg(0x100C0080,0x00); // mutex
            sample_comm_ircut_set_reg(0x100C0084,0x00); // mutex
            sample_comm_ircut_set_reg(0x120B7400,0x60); // dir
            sample_comm_ircut_set_reg(0x120B7180,0x20); // data
            break;
        }
        case BOARD_TYPE_XM7206_V11A_EVB:{
            //pin_mux-- GPIO6_5, GPIO6_6
            sample_comm_ircut_set_reg(0x100C0060,0x10); // mutex
            sample_comm_ircut_set_reg(0x100C0064,0x10); // mutex
            sample_comm_ircut_set_reg(0x120B6400,0x60); // dir
            sample_comm_ircut_set_reg(0x120B6180,0x20); // data
            break;
        }
        default:
            break;
    }

    sample_comm_ircut_memclose();
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_ircut_set_status_normal(sample_comm_board_type board_type, xmedia_s32 vi_dev)
{
    xmedia_s32 ret = 0;

    if (vi_dev >= VI_MAX_DEV_NUM || board_type >= BOARD_TYPE_MAX) {
        return XMEDIA_FAILURE;
    }

    ret = sample_comm_ircut_memopen();
    if (ret != XMEDIA_SUCCESS) {
        return ret;
    }

    switch (board_type) {
        case BOARD_TYPE_XM7206_V10_RB:{
            //pin_mux-- GPIO4_6,GPIO4_3
            sample_comm_ircut_set_reg(0x100C0024,0x05); // mutex
            sample_comm_ircut_set_reg(0x100C0018,0x05); // mutex
            sample_comm_ircut_set_reg(0x120B4400,0x48); // dir
            sample_comm_ircut_set_reg(0x120B4120,0x08); // data
            break;
        }
        case BOARD_TYPE_XM7206_V10B_RBA:{
            //pin_mux-- GPIO3_5, GPIO3_6
            sample_comm_ircut_set_reg(0x100C0000,0x00); // mutex
            sample_comm_ircut_set_reg(0x100C0004,0x00); // mutex
            sample_comm_ircut_set_reg(0x120B3400,0x60); // dir
            sample_comm_ircut_set_reg(0x120B3180,0x40); // data
            break;
        }
        case BOARD_TYPE_XM7206_V10B_EVB:{
            //pin_mux-- GPIO5_4, GPIO5_5
            sample_comm_ircut_set_reg(0x100C003C,0x00); // mutex
            sample_comm_ircut_set_reg(0x100C0040,0x00); // mutex
            sample_comm_ircut_set_reg(0x120B5400,0x30); // dir
            sample_comm_ircut_set_reg(0x120B50C0,0x20); // data
            break;
        }
        case BOARD_TYPE_XM7206_V11A_RBA:{
            //pin_mux-- GPIO7_5, GPIO7_6
            sample_comm_ircut_set_reg(0x100C0080,0x00); // mutex
            sample_comm_ircut_set_reg(0x100C0084,0x00); // mutex
            sample_comm_ircut_set_reg(0x120B7400,0x60); // dir
            sample_comm_ircut_set_reg(0x120B7180,0x40); // data
            break;
        }
        case BOARD_TYPE_XM7206_V11A_EVB:{
            //pin_mux-- GPIO6_5, GPIO6_6
            sample_comm_ircut_set_reg(0x100C0060,0x10); // mutex
            sample_comm_ircut_set_reg(0x100C0064,0x10); // mutex
            sample_comm_ircut_set_reg(0x120B6400,0x60); // dir
            sample_comm_ircut_set_reg(0x120B6180,0x40); // data
            break;
        }
        default:
            break;
    }
    
    sample_comm_ircut_memclose();
    return XMEDIA_SUCCESS;
}



