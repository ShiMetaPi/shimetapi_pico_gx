/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include "bsp.h"
#include "memmap.h"

#define MAX_ROW (0x8000000) /* =max_len/16 */

/*memory dump*/
XMEDIA_RET bsp_md(IN VOID* pStart, IN U32 ulLen, IN FD fd)
{
    if (NULL == pStart) {
        return ERR_GEN_INVALID_POINTER;
    }

    size_t wl = write(fd, pStart, ulLen);
    if (wl != ulLen) {
        WRITE_LOG_DEBUG("Dump memory error!wish len: %u, dump len:%u\n", ulLen, wl);
    }
    return XMEDIA_SUCCESS;
}

/*Memory Dump 2 file*/
XMEDIA_RET bsp_md2file(IN VOID* pStart, IN U32 ulLen, IN const char* strLabel, IN char* fn)
{
    char path[PATH_MAX + 1] = {0x00};

    if (NULL == fn || NULL == pStart) {
        return ERR_GEN_INVALID_POINTER;
    }

    if ((strlen(fn) > PATH_MAX) || (NULL == realpath(fn, path))) {
        printf("error: input path error.\n");
        return ERR_GEN_INVALID_POINTER;
    }

    FILE* fMd = fopen(path, "a");
    if (NULL == fMd) {
        WRITE_LOG_DEBUG("Open File %s to dump memory error.\n", fn);
        return XMEDIA_FAILURE;
    }
    if (strLabel) {
        fwrite(strLabel, strlen(strLabel), 1, fMd);
    }
    fwrite(pStart, ulLen, 1, fMd);
    fclose(fMd);
    return XMEDIA_SUCCESS;
}

VOID bsp_hexdump(OUT FILE* stream, IN const void* src, IN size_t len, IN size_t width)
{
    unsigned int rows, pos, c, i;
    const unsigned char* start = NULL;
    const unsigned char* rowpos = NULL;
    const unsigned char* data = NULL;

    if (width == 0) {
        return;
    }

    data = src;
    start = data;
    pos = 0;
    rows = (len % width) == 0 ? len / width : len / width + 1;
    for (i = 0; i < rows; i++) {
        rowpos = data;
        fprintf(stream, "%05x: ", pos);
        do {
            c = *data++ & 0xff;
            if ((size_t)(data - start) <= len) {
                fprintf(stream, " %02x", c);
            } else {
                fprintf(stream, "   ");
            }
        } while (((data - rowpos) % width) != 0);

        fprintf(stream, "  |");
        data -= width;
        do {
            c = *data++;
            if (isprint(c) == 0 || c == '\t') {
                c = '.';
            }
            if ((size_t)(data - start) <= len) {
                fprintf(stream, "%c", c);
            } else {
                fprintf(stream, " ");
            }
        } while (((data - rowpos) % width) != 0);
        fprintf(stream, "|\n");
        pos += width;
    }
    fflush(stream);
}

VOID bsp_hexdump2(OUT FILE* stream, IN const void* src, IN size_t len, IN size_t width)
{
    unsigned int c, i, rows;
    const unsigned int* rowpos = NULL;
    const unsigned int* data = NULL;

    if (width == 0) {
        return;
    }

    data = src;
    rows = (len % width) == 0 ? len / width : len / width + 1;

    if (MAX_ROW < rows) {
        printf("error:Input param(len) is greater than 2GB!\n");
        return;
    }

    for (i = 0; i < rows; i++) {
        rowpos = data;
        fprintf(stream, "%04x: ", i * 0x10);
        do {
            c = *data++;
            fprintf(stream, " %08x", c);
        } while (((data - rowpos) % 4) != 0);
        fprintf(stream, "\n");
    }
    fflush(stream);
}

VOID bsp_hexdump_log(IN const void* src, IN size_t len, IN size_t width)
{
    if (LOG_GETLEVEL() < LOG_LEVEL_DEBUG2)
        return;

    unsigned int rows, pos, c, i;
    const unsigned char* start = NULL;
    const unsigned char* rowpos = NULL;
    const unsigned char* data = NULL;

    if (width == 0) {
        return;
    }

    data = src;
    start = data;
    pos = 0;
    rows = (len % width) == 0 ? len / width : len / width + 1;
    for (i = 0; i < rows; i++) {
        rowpos = data;
        WRITE_LOG_NORMAL("%05x: ", pos);
        do {
            c = *data++ & 0xff;
            if ((size_t)(data - start) <= len) {
                WRITE_LOG_NORMAL(" %02x", c);
            } else {
                WRITE_LOG_NORMAL("   ");
            }
        } while (((data - rowpos) % width) != 0);

        WRITE_LOG_NORMAL("  |");
        data -= width;
        do {
            c = *data++;
            if (isprint(c) == 0 || c == '\t') {
                c = '.';
            }
            if ((size_t)(data - start) <= len) {
                WRITE_LOG_NORMAL("%c", c);
            } else {
                WRITE_LOG_NORMAL(" ");
            }
        } while (((data - rowpos) % width) != 0);
        WRITE_LOG_NORMAL("|\n");
        pos += width;
    }
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
