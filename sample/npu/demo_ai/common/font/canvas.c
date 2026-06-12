/*!
*****************************************************************************
** file        canvas.c
**
** version     2016-05-08 16:21 heyong -> v1.0
**
** brief       the implementation of canvas operations.
**
** attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2016-2020 BY GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
//#include "type.h"
#include "canvas.h"

typedef struct bitmap_file_header_s {
	char type[2]; // "BM" (0x4d42)
    unsigned int file_size;
    unsigned int reserved_zero;
    unsigned int off_bits; // data area offset to the file set (unit. byte)
	unsigned int info_size;
	unsigned int width;
	unsigned int height;
	unsigned short planes; // 0 - 1
	unsigned short bit_count; // 0 - 1
	unsigned int compression; // 0 - 1
	unsigned int size_image; // 0 - 1
	unsigned int xpels_per_meter;
	unsigned int ypels_per_meter;
	unsigned int clr_used;
	unsigned int clr_important;
} __attribute__((packed)) bitmap_file_header_t; //


static canvas_t g_canvas[MAX_CANVAS_NUMS];

canvas_t *canvas_init(int width, int height)
{
    int cnt = 0;
    canvas_t *canvaTemp = NULL;

    /*check input parameters.*/
    if ((width <= 0) || (height <= 0)) {
        printf("invalid params %d-%d\n", width, height);
        return NULL;
    }

    for (cnt = 0; cnt < MAX_CANVAS_NUMS; cnt++) {
        canvaTemp = &(g_canvas[cnt]);
        if ((canvaTemp->width == 0) && (canvaTemp->height == 0) && (canvaTemp->pixels == NULL)) {
            canvaTemp->width  = BIG_ENDIAN_ALIGNED(width, 4); // aligned to 2 pixel
            canvaTemp->height = BIG_ENDIAN_ALIGNED(height, 4);

            /*canva buffer.*/
            canvaTemp->pixels = calloc(canvaTemp->width * canvaTemp->height * sizeof(int), 1);
            if(!canvaTemp->pixels)
            {
                printf("canvas->pixels malloc failed!\n");
                canvaTemp->width  = 0;
                canvaTemp->height = 0;
                return NULL;
            }
            return canvaTemp;
        }
    }

    printf("create canvas failed: not free canvas.\n");
    return NULL;
}

int canvas_init_obj(canvas_t *canvas, int width, int height)
{
    /*check input parameters.*/
    if ((width <= 0) || (height <= 0)) {
        printf("invalid params %d-%d\n", width, height);
        return -1;
    }
    canvas->width  = BIG_ENDIAN_ALIGNED(width, 2); // aligned to 2 pixel
    canvas->height = BIG_ENDIAN_ALIGNED(height, 2);

    /*canva buffer.*/
    canvas->pixels = calloc(canvas->width * canvas->height * sizeof(int), 1);
    return 0;
}

int canvas_deinit(canvas_t *canvas)
{
    if (canvas) {
		canvas->width = 0;
		canvas->height = 0;
		if (canvas->pixels) {
            /*the canvas buffer calloc form memory, so it need to free in this API.*/
			free(canvas->pixels);
			canvas->pixels = NULL;
		}
	}
	return 0;
}

int canvas_put_pixel(canvas_t *canvas, int x, int y, canvas_pixel_t pixel)
{
	unsigned int *pixelAddr = NULL;

    /*check input parameters.*/
    if ((canvas == NULL) || ((canvas->pixels == NULL)) ||
       ((x >= canvas->width) && (y >= canvas->height))) {
        printf("input invalid params\n");
        return -1;
    }

	pixelAddr = (unsigned int*)canvas->pixels;

	*(pixelAddr + canvas->width * y + x) = pixel.argb8888;
	return 0;
}

int canvas_get_pixel(canvas_t *canvas, int x, int y, canvas_pixel_t *pixel)
{
	unsigned int *pixelAddr = NULL;
    unsigned int argb8888;

    /*check input parameters.*/
    if ((canvas == NULL) || (canvas->pixels == NULL) || (pixel == NULL) ||
       ((x >= canvas->width) && (y >= canvas->height))) {
        printf("input invalid params.\n");
        return -1;
    }

    pixelAddr  = (unsigned int*)canvas->pixels;
    argb8888   = *(pixelAddr + canvas->width * y + x);
    pixel->argb8888  = argb8888;
    return 0;
}

int canvas_match_pixel(canvas_t *canvas, canvas_pixel_t pixel1, canvas_pixel_t pixel2)
{
    return (pixel1.argb8888 == pixel2.argb8888);
}

void canvas_clear(canvas_t *canvas)
{
    int i = 0;
	unsigned int *pixelAddr = (unsigned int *)canvas->pixels;
    for (i = 0; i < canvas->height * canvas->width; i++) {
        pixelAddr[i] = 0x00000000;// clear canvas.
    }
}

void canvas_null_rect(canvas_t *canvas,canvas_pixel_t pixel2)
{
    int i = 0,j = 0;
	int argb8888 = pixel2.argb8888;
	unsigned int *pixelAddr = (unsigned int *)canvas->pixels;
    for (i = 0; i < canvas->height ; i++) {
			pixelAddr =  (unsigned int *)canvas->pixels + (i*canvas->width);
		for(j = 0; j < canvas->width; j++)
		{
			if(i < 2 || j < 2 || (i > (canvas->height -3)) || (j > (canvas->width-3)))
				pixelAddr[j] = argb8888;// clear canvas.
			else
				pixelAddr[j] = 0;// clear canvas.
		}
    }
}

int canvas_copy(canvas_t *canvas_from, canvas_t *canvas_to)
{

	int i ;

	int from_width = canvas_from->width*sizeof(int);
	int to_width = canvas_to->width*sizeof(int);
	char *from = canvas_from->pixels;
	char *to = canvas_to->pixels;
	for(i = 0 ;i < canvas_from->height; i++)
	{
		memcpy(to,from,from_width);
		to += to_width;
		from += from_width;
	}
	return 0;
}

canvas_t *canvas_init_by_bmp24file(const char *bmp24_path)
{
    int ret = 0;
    int i = 0, j = 0, bmpWidth = 0, bmpHeight = 0;
	unsigned int dataSize = 0;
    char *lineOffset = NULL;
    FILE *bmpFd = NULL;
    char *canvas_cache = NULL;
    bitmap_file_header_t bmpHeader;

    canvas_t *canvas = NULL;
    canvas_pixel_t canvas_pixel;

    /*check input parameters.*/
    if(bmp24_path == NULL)
    {
        printf("canvas_init_by_bmp24file: input invalid parameters.\n");
        return NULL;
    }

    printf("canvas_init_by_bmp24file: fileName: %s.\n",bmp24_path);

    bmpFd = fopen(bmp24_path, "rb");

    if(bmpFd ==  NULL)
    {
        printf("canvas_init_by_bmp24file: can not open fileName:%s.\n",bmp24_path);
        return NULL;
    }

    ret = fread(&bmpHeader, 1, sizeof(bmpHeader), bmpFd);

    if(sizeof(bmpHeader) != ret)
    {
        fclose(bmpFd);
        printf("canvas_init_by_bmp24file: load fileName:%s error.\n",bmp24_path);
        return NULL;
    }

    if((bmpHeader.type[0] == 'B') && (bmpHeader.type[1] == 'M') &&
        ((bmpHeader.bit_count == 24) || (bmpHeader.bit_count == 32)))
    {
        printf("IMAGE %dx%d size=%d offset=%d filesize=%d info=%d\n", \
                 bmpWidth, bmpHeight, bmpHeader.size_image, \
                 bmpHeader.off_bits, bmpHeader.file_size,\
                 bmpHeader.info_size);

        bmpWidth = bmpHeader.width;
        bmpHeight = bmpHeader.height;

        if(bmpHeader.size_image == 0)
        {
            dataSize = bmpHeader.file_size - bmpHeader.off_bits;
        }
        else
        {
            dataSize = bmpHeader.size_image;
        }
        canvas_cache = calloc(dataSize, 1);

        /*load image to buf.*/
        if(0 == fseek(bmpFd, bmpHeader.off_bits, SEEK_SET))
        {
            ret = fread(canvas_cache, 1, dataSize, bmpFd);
        }
        fclose(bmpFd);
        bmpFd = NULL;

        /*load to canvas.*/
        canvas = canvas_init(bmpWidth, bmpHeight);
        for(i = 0; i < bmpHeight; ++i)
        {
            if(bmpHeader.bit_count == 24)
            {
                lineOffset = canvas_cache + BIG_ENDIAN_ALIGNED(3 *
                                             bmpWidth, 4) * (bmpHeight - 1 - i) + 2;
                for(j = 0; j < bmpWidth; ++j)
                {
                    char *const column_offset = lineOffset + 3 * j;

                    canvas_pixel.alpha = 0xff;
                    canvas_pixel.red   = column_offset[0];
                    canvas_pixel.green = column_offset[1];
                    canvas_pixel.blue  = column_offset[2];

                    canvas_put_pixel(canvas, j, i, canvas_pixel);
                }
            }
            else/*32*/
            {
                lineOffset = canvas_cache + BIG_ENDIAN_ALIGNED(4 *
                            bmpWidth, 4) * (bmpHeight - 1 - i) + 2;
                for(j = 0; j < bmpWidth; ++j)
                {
                    char *const column_offset = lineOffset + 4 * j;

                    canvas_pixel.alpha = column_offset[0];
                    canvas_pixel.red = column_offset[1];
                    canvas_pixel.green = column_offset[2];
                    canvas_pixel.blue = column_offset[3];

                    canvas_put_pixel(canvas, j, i, canvas_pixel);
                }
            }
        }

        /*free the canvas cache.*/
        free(canvas_cache);
        canvas_cache = NULL;

        return canvas;
    }

    fclose(bmpFd);
    printf("canvas_init_by_bmp24file: fileName: %s error.\n",bmp24_path);
    return NULL;
}


