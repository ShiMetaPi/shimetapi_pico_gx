/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#include <common.h>
#include <command.h>
#include "drv_vo.h"
#include "xmedia_vo.h"
#ifndef CFG_MAXARGS
#define CFG_MAXARGS 10
#endif

#define CFG_MAXARGS_SETVOBG 3
#define CFG_MAXARGS_STARTVO 4
#define CFG_MAXARGS_STOPVO 2
#define CFG_MAXARGS_STARTVL 8
#define CFG_MAXARGS_STOPVL 2
#define CFG_MAXARGS_STARTGX 9
#define CFG_MAXARGS_STOPGX 2
#define CMD_VO_ARGS_BASE10 10
#define CMD_VO_ARGS_BASE16 16
#define CMD_VO_ARGS_BASE_ALL 0

extern int init_vo(xmedia_void);
extern int deinit_vo(xmedia_void);
extern int set_vobg(unsigned int dev, unsigned int rgb);
extern int start_vo(unsigned int dev, xmedia_s32 screen_type);
extern int stop_vo(unsigned int dev);
extern int start_videolayer(unsigned int layer, unsigned long addr, unsigned int strd, xmedia_video_rect layer_rect);
extern int stop_videolayer(unsigned int layer);
extern int start_gx(unsigned int layer, unsigned long addr, unsigned int strd, xmedia_video_rect gx_rect);
extern int stop_gx(unsigned int layer);
extern int check_vo_support(unsigned int dev, xmedia_s32 screen_type);

static int do_initvo(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
    init_vo();
    return 0;
}

static int do_deinitvo(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
    deinit_vo();
    return 0;
}

static int do_startvo(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
    xmedia_vo_dev dev = 0;
    xmedia_s32 screen_type;

    if (argc < 3) {
        printf("insufficient parameter!\n");
        printf("usage:\n%s\n", cmdtp->usage);
        return -1;
    }

    dev = (unsigned int)simple_strtoul(argv[1], NULL, CMD_VO_ARGS_BASE10);
    screen_type = (unsigned int)simple_strtoul(argv[2], NULL, CMD_VO_ARGS_BASE10);

    if (check_vo_support(dev, screen_type)) {
        printf("unsupported parameter!\n");
        return -1;
    }

    start_vo(dev, screen_type);
    return 0;
}

static int do_stopvo(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
    xmedia_vo_dev dev = 0;
    if (argc < 2) {
        printf("insufficient parameter!\n");
        printf("usage:\n%s\n", cmdtp->usage);
        return -1;
    }

    dev = (unsigned int)simple_strtoul(argv[1], NULL, CMD_VO_ARGS_BASE10);

    stop_vo(dev);
    return 0;
}

static int start_layer_parse(char *const argv[], unsigned int *layer,
    unsigned long *addr, unsigned int *strd, xmedia_video_rect *layer_rect)
{
    int ret;
    unsigned long layer_tmp, addr_tmp, strd_tmp, x_tmp, y_tmp, width_tmp, height_tmp;
    ret = strict_strtoul(argv[1], CMD_VO_ARGS_BASE10, &layer_tmp);  /* 1st arg */
    if (ret != 0) {
        printf("parse layer failed.\n");
        return -1;
    }
    ret = strict_strtoul(argv[2], CMD_VO_ARGS_BASE16, &addr_tmp);  /* 2nd arg */
    if (ret != 0) {
        printf("parse addr failed.\n");
        return -1;
    }
    ret = strict_strtoul(argv[3], CMD_VO_ARGS_BASE10, &strd_tmp);  /* 3rd arg */
    if (ret != 0) {
        printf("parse strd failed.\n");
        return -1;
    }
    ret = strict_strtoul(argv[4], CMD_VO_ARGS_BASE10, &x_tmp);  /* 4th arg */
    if (ret != 0) {
        printf("parse x failed.\n");
        return -1;
    }
    ret = strict_strtoul(argv[5], CMD_VO_ARGS_BASE10, &y_tmp);  /* 5th arg */
    if (ret != 0) {
        printf("parse y failed.\n");
        return -1;
    }
    ret = strict_strtoul(argv[6], CMD_VO_ARGS_BASE10, &width_tmp);  /* 6th arg */
    if (ret != 0) {
        printf("parse width failed.\n");
        return -1;
    }
    ret = strict_strtoul(argv[7], CMD_VO_ARGS_BASE10, &height_tmp);  /* 7th arg */
    if (ret != 0) {
        printf("parse height failed.\n");
        return -1;
    }
    *layer = (unsigned int)layer_tmp;
    *addr = addr_tmp;
    *strd = (unsigned int)strd_tmp;
    layer_rect->x = (unsigned int)x_tmp;
    layer_rect->y = (unsigned int)y_tmp;
    layer_rect->width = (unsigned int)width_tmp;
    layer_rect->height = (unsigned int)height_tmp;
    return 0;
}

static int do_startvl(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
    int ret;
    unsigned int layer, strd;
    unsigned long addr;
    xmedia_video_rect layer_rect;

    if (argc < 8) { /* max 8 args */
        printf("insufficient parameter!\n");
        printf("usage:\n%s\n", cmdtp->usage);
        return -1;
    }

    ret = start_layer_parse(argv, &layer, &addr, &strd, &layer_rect);
    if (ret != 0) {
        printf("insufficient parameter!\n");
        return -1;
    }

    ret = start_videolayer(layer, addr, strd, layer_rect);
    if (ret != 0) {
        printf("start_videolayer error!\n");
        return -1;
    }

    printf("video layer %u opened!\n", layer);

    return 0;
}

static int do_stopvl(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
    int ret;
    unsigned long layer_tmp;
    if (argc < 2) { /* max 8 args */
        printf("insufficient parameter!\n");
        printf("usage:\n%s\n", cmdtp->usage);
        return -1;
    }
    ret = strict_strtoul(argv[1], CMD_VO_ARGS_BASE10, &layer_tmp);  /* 1st arg */
    if (ret != 0) {
        printf("strict_strtoul error\n");
        return -1;
    }

    stop_videolayer(layer_tmp);
    return 0;
}

static int do_startgx(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
    //todo
    return 0;
}

static int do_stopgx(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
    //todo
    return 0;
}

U_BOOT_CMD(initvo, CFG_MAXARGS, 1, do_initvo,
       "initvo", "initvo");

U_BOOT_CMD(deinitvo, CFG_MAXARGS, 1, do_deinitvo,
       "deinitvo", "deinitvo");


U_BOOT_CMD(startvo, CFG_MAXARGS, 1, do_startvo,
       "startvo   - open vo device with a certain output interface.\n"
       "\t- startvo [dev screen_type]",
       "\nargs: [dev screen_type]\n"
       "\t-<dev> : 0: DHD0, 1: DHD1\n"
       "\t-<screen_type>: 0:MICROTECH_MTF050_LCD_800X480\n"
       "\t-<screen_type>: 1:SATOZ_SAT935_MIPI_800X1280_RGB8888_24BIT\n"
       "\t-<screen_type>: 2:SIL9024_BT1120_1920X1080P60\n"
       "\t-<screen_type>: 3:MICROTECH_MTF101_FT01_LVDS_800X1280_RGB666_18BIT\n"
       "\t-<screen_type>: 4:MICROTECH_MTF101_FS16_LVDS_1280X800_RGB888_24BIT\n"
       "\t- ....");

U_BOOT_CMD(stopvo, CFG_MAXARGS, 1, do_stopvo,
       "stopvo   - close interface of vo device.\n"
       "\t- stopvo [dev]",
       "\nargs: [dev]\n"
       "\t-<dev> : 0(HD0), 1: DHD1\n");

U_BOOT_CMD(startvl, CFG_MAXARGS, 1, do_startvl,
       "startvl   - open video layer.\n"
       "\t- startvl [layer addr stride x y w h]\n",
       "\nargs: [layer, addr, stride, x, y, w, h]\n"
       "\t-<layer>   : 0(V0)\n"
       "\t-<addr>    : picture address\n"
       "\t-<stride>  : picture stride\n"
       "\t-<x,y,w,h> : display area\n");

U_BOOT_CMD(stopvl, CFG_MAXARGS, 1, do_stopvl,
       "stopvl   - close video layer.\n"
       "\t- stopvl [layer]",
       "\nargs: [layer]\n"
       "\t-<layer> : 0(V0) 1(V1)\n");

U_BOOT_CMD(startgx, CFG_MAXARGS, 1, do_startgx,
       "startgx   - open graphics layer.\n"
       "\t- startgx [layer addr stride x y w h]\n",
       "\nargs: [layer, addr, stride, x, y, w, h]\n"
       "\t-<layer>   : 0(G0)\n"
       "\t-<addr>    : picture address\n"
       "\t-<stride>  : picture stride\n"
       "\t-<x,y,w,h> : display area\n");

U_BOOT_CMD(stopgx, CFG_MAXARGS, 1, do_stopgx,
       "stopgx   - close graphics layer.\n"
       "\t- stopgx [layer]",
       "\nargs: [layer]\n"
       "\t-<layer> : 0(G0)\n");
