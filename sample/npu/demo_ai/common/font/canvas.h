/*!
*****************************************************************************
** file        osd.h
**
** version     2016-05-08 18:43 heyong -> v1.0
**
** brief       the canvas operations define.
**
** attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2016-2020 BY GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#ifndef _CANVAS_H_
#define _CANVAS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CANVAS_NUMS     (3)

#define LITTLE_ENDIAN_ALIGNED(__val, __align) ( (__val) & ~((__align)-1u) )
#define BIG_ENDIAN_ALIGNED(__val, __align) LITTLE_ENDIAN_ALIGNED((__val) + (__align) - 1, (__align))


#ifndef ROUND_UP
#define ROUND_UP(x, n)  ( ((x)+(n)-1u) & ~((n)-1u) )
#endif

typedef union canvas_pixel_format_s {
	int argb8888;
	struct {
		unsigned char blue, green, red, alpha;
	};
} canvas_pixel_t;

typedef struct canvas_s {
	int width;
	int height;
	void* pixels;    
    int width_min, height_min, height_max;
    unsigned char* image;
} canvas_t;

canvas_t *canvas_init(int width, int height);
int canvas_init_obj(canvas_t *canvas, int width, int height);
int canvas_deinit(canvas_t *canvas);

int canvas_put_pixel(canvas_t *canvas, int x, int y, canvas_pixel_t pixel);
int canvas_get_pixel(canvas_t *canvas, int x, int y, canvas_pixel_t *pixel);

int canvas_match_pixel(canvas_t *canvas, canvas_pixel_t pixel1, canvas_pixel_t pixel2);

void canvas_clear(canvas_t *canvas);

canvas_t *canvas_init_by_bmp24file(const char *bmp24_path);

#ifdef __cplusplus
};
#endif
#endif /* _CANVAS_H_ */

