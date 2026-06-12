/*!
*****************************************************************************
** file        canvas_font.h
**
** version     2015-04-25 14:58 unknow -> v1.0
**             2016-04-25 16:21 heyong -> v2.0
**
** brief       the font operations api define.
**
** attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2016-2020 BY GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#ifndef _OSD_FONT_H_
#define _OSD_FONT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "canvas.h"

#define CANVAS_FONT_DIR "./common/font/fnt"

typedef enum font_size_ {
	FONT_SIZE_16 = (16),
	FONT_SIZE_20 = (20),
	FONT_SIZE_24 = (24),
	FONT_SIZE_32 = (32),
	FONT_SIZE_40 = (40),
	FONT_SIZE_48 = (48),
	FONT_SIZE_56 = (56),
	FONT_SIZE_64 = (64),
}font_size_e;

#define RGB_PIXEL_FILL(_pixel, _a, _r, _g, _b) \
	do{\
		_pixel.alpha = (_a), _pixel.red = (_r), _pixel.green = (_g), _pixel.blue = (_b);\
	}while(0)

typedef unsigned int text_style_t;

#define TEXT_STYLE_FOREGROUND_TRANSPARENT (1<<0)
#define TEXT_STYLE_FOREGROUND_WHRITE      (1<<1)
#define TEXT_STYLE_FOREGROUND_BLACK       (1<<2)
#define TEXT_STYLE_FOREGROUND_RED         (1<<3)
#define TEXT_STYLE_BACKGROUND_TRANSPARENT (1<<4)
#define TEXT_STYLE_BACKGROUND_WHRITE      (1<<5)
#define TEXT_STYLE_BACKGROUND_BLACK       (1<<6)
#define TEXT_STYLE_ENCLOSED_TRANSPARENT   (1<<8)
#define TEXT_STYLE_ENCLOSED_WHRITE        (1<<9)
#define TEXT_STYLE_ENCLOSED_BLACK         (1<<10)

#define TEXT_STYLE_ENCLOSED_ENABLE        (1<<12)
#define TEXT_STYLE_FOREGROUND_BIT_AREA    0x0007
#define TEXT_STYLE_BACKGROUND_BIT_AREA    0x0070
#define TEXT_STYLE_ENCLOSED_BIT_AREA      0x0700

#define TEXT_STYLE_DEFAULT \
		(TEXT_STYLE_FOREGROUND_WHRITE\
		|TEXT_STYLE_BACKGROUND_TRANSPARENT\
		|TEXT_STYLE_ENCLOSED_BLACK)

void canvas_font_set_alpha(unsigned char alpha);
int canvas_font_available(font_size_e font_size);

int canvas_font_load(font_size_e font_size, const char* asc_font, const char* gb2312_font);

int canvas_font_init();
void canvas_font_deinit();

/*!
*******************************************************************************
** brief load font to canvas.
**
** param[in]  canvas    the point of dest canvas.
** param[in]  x         canvas x start address.
** param[in]  y         canvas y start address.
** param[in]  font_size the current font size.
** param[in]  text      the point of text.
** param[in]  style     font style.
**
** return
** - #>=0 , successed, and return the write put size.
** - #<0  , failed.
**
*******************************************************************************
*/
int canvas_fill_text(canvas_t *canvas, int x, int y, const char *text,
                            font_size_e font_size, text_style_t style);
int canvas_get_fill_text_width(canvas_t *canvas, const char *text, int font_size);

#ifdef __cplusplus
};
#endif
#endif /* _OSD_FONT_H_ */

