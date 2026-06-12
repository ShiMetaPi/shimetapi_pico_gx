/*!
*****************************************************************************
** file        canvas_font.c
**
** version     2015-04-25 14:58 unknow -> v1.0
**             2016-04-25 16:21 heyong -> v2.0
**
** brief       the implementation of canvas_font.h.
**
** attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2016-2020 BY GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include "canvas_font.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>

static unsigned char _def_alpha = 0;
static text_style_t _def_sytle = TEXT_STYLE_DEFAULT;

static unsigned char *_asc_16 = NULL; // bytes 128 x 16 x 1 size 8x16
static unsigned char *_asc_20 = NULL; // bytes 128 x 20 x 2 size 10x20
static unsigned char *_asc_24 = NULL; // bytes 128 x 24 x 2 size 12x24
static unsigned char *_asc_32 = NULL; // bytes 128 x 32 x 2 size 16x32
static unsigned char *_asc_40 = NULL; // bytes 128 x 40 x 3 size 20x40
static unsigned char *_asc_48 = NULL; // bytes 128 x 48 x 3 size 24x48
static unsigned char *_asc_56 = NULL; // bytes 128 x 56 x 4 size 28x56

static unsigned char* _gb2312_16 = NULL; // bytes 87 x 94 x 16 x 2 size 16x16
static unsigned char* _gb2312_20 = NULL; // bytes 87 x 94 x 20 x 3 size 20x20
static unsigned char* _gb2312_24 = NULL; // bytes 87 x 94 x 24 x 3 size 24x24
static unsigned char* _gb2312_32 = NULL; // bytes 87 x 94 x 32 x 4 size 32x32
static unsigned char* _gb2312_40 = NULL; // bytes 87 x 94 x 40 x 5 size 40x40
static unsigned char* _gb2312_48 = NULL; // bytes 87 x 94 x 48 x 6 size 48x48
static unsigned char* _gb2312_56 = NULL; // bytes 87 x 94 x 56 x 7 size 56x56

static int font_offset_byte = 0;
static int font_asc_offset_byte = 0;

static int _canvas_font_load_mem(const char* file_name, unsigned char **ret_mem)
{
	struct stat file_stat = {0};
	FILE* fid = NULL;
	char font=0;
	if(0 == stat(file_name, &file_stat)){
		fid = fopen(file_name, "rb");
		if(NULL != fid){
			if(NULL != *ret_mem){
				free(*ret_mem);
			}
			*ret_mem = calloc(file_stat.st_size, 1);
			fread(*ret_mem, 1, file_stat.st_size, fid);
            fseek(fid,0,SEEK_SET);
			fread(&font, 1, 1, fid);
			if(font=='M')
			{
    			if(strstr(file_name, "hzk")!=NULL)
    			{
                    font_offset_byte = 16;
    			}
                else
                    font_asc_offset_byte = 16;
			}
			fclose(fid);
			fid = NULL;
			return 0;
		}
	}
	return -1;
}

static void _canvas_font_free_mem(unsigned char **mem)
{
    if (mem && *mem) {
        free(*mem);
        *mem = NULL;
    }
}

int canvas_font_load(font_size_e font_size, const char* asc_font, const char* gb2312_font)
{
	unsigned char **asc_mem = NULL, **gb2312_mem = NULL;

	switch(font_size){
		case FONT_SIZE_16:
			asc_mem = &_asc_16;
			gb2312_mem = &_gb2312_16;
			break;

		case FONT_SIZE_20:
			asc_mem = &_asc_20;
			gb2312_mem = &_gb2312_20;
			break;

		case FONT_SIZE_24:
			asc_mem = &_asc_24;
			gb2312_mem = &_gb2312_24;
			break;

		case FONT_SIZE_32:
			asc_mem = &_asc_32;
			gb2312_mem = &_gb2312_32;
			break;

		case FONT_SIZE_40:
			asc_mem = &_asc_40;
			gb2312_mem = &_gb2312_40;
			break;

		case FONT_SIZE_48:
			asc_mem = &_asc_48;
			gb2312_mem = &_gb2312_48;
			break;

		case FONT_SIZE_56:
			asc_mem = &_asc_56;
			gb2312_mem = &_gb2312_56;
			break;

		default:
			return -1;
	}

	if(0 == _canvas_font_load_mem(asc_font, asc_mem)
		&& 0 == _canvas_font_load_mem(gb2312_font, gb2312_mem))
	{
		return 0;
	}
	else
	{
		printf("Load asc_font or gb2313_font fail\n");
	}

	free(*asc_mem);
	free(*gb2312_mem);
	return -1;

}

void canvas_font_set_alpha(unsigned char alpha)
{
	_def_alpha = alpha;
}

int canvas_font_available(font_size_e font_size)
{
	switch(font_size){
		case FONT_SIZE_16:
			return (NULL != _asc_16 && (NULL != _gb2312_16));
		case FONT_SIZE_20:
			return (NULL != _asc_20 && (NULL != _gb2312_20));
		case FONT_SIZE_24:
			return (NULL != _asc_24 && (NULL != _gb2312_24));
		case FONT_SIZE_32:
			return (NULL != _asc_32 && (NULL != _gb2312_32));
		case FONT_SIZE_40:
			return (NULL != _asc_40 && (NULL != _gb2312_40));
		case FONT_SIZE_48:
			return (NULL != _asc_48 && (NULL != _gb2312_48));
		case FONT_SIZE_56:
			return (NULL != _asc_56 && (NULL != _gb2312_56));
		default: ;
	}
	return 0;
}

int canvas_font_init()
{
    char file_path_asc[128] = {0}, file_path_hzk[128] = {0};
    snprintf(file_path_asc, sizeof(file_path_asc), "%s/%s%d", CANVAS_FONT_DIR, "asc", FONT_SIZE_32);
    snprintf(file_path_hzk, sizeof(file_path_hzk), "%s/%s%d", CANVAS_FONT_DIR, "hzk", FONT_SIZE_32);
    canvas_font_load(FONT_SIZE_32, file_path_asc, file_path_hzk);

    snprintf(file_path_asc, sizeof(file_path_asc), "%s/%s%d", CANVAS_FONT_DIR, "asc", FONT_SIZE_16);
    snprintf(file_path_hzk, sizeof(file_path_hzk), "%s/%s%d", CANVAS_FONT_DIR, "hzk", FONT_SIZE_16);
    canvas_font_load(FONT_SIZE_16, file_path_asc, file_path_hzk);
	return 0;
}

void canvas_font_deinit()
{
	_canvas_font_free_mem(&_asc_16);
	_canvas_font_free_mem(&_asc_20);
	_canvas_font_free_mem(&_asc_24);
	_canvas_font_free_mem(&_asc_32);
	_canvas_font_free_mem(&_asc_40);
	_canvas_font_free_mem(&_asc_48);
	_canvas_font_free_mem(&_asc_56);
	_canvas_font_free_mem(&_gb2312_16);
	_canvas_font_free_mem(&_gb2312_20);
	_canvas_font_free_mem(&_gb2312_24);
	_canvas_font_free_mem(&_gb2312_32);
	_canvas_font_free_mem(&_gb2312_40);
	_canvas_font_free_mem(&_gb2312_48);
	_canvas_font_free_mem(&_gb2312_56);
}

int canvas_fill_text(canvas_t *canvas, int x, int y, const char *text,
                            font_size_e font_size, text_style_t style)
{
	int i = 0, ii = 0, iii = 0;
	int const x_base = x;
	int const y_base = y;
    unsigned char *asc = NULL;
    unsigned char *gb2312 = NULL;

	char *ch = (char *)text; // at the beginning of the text
	unsigned int asc_width = 0;
	unsigned int gb2312_width = 0;
	unsigned int text_width = 0, text_height = 0;

	canvas_pixel_t fg_color, bg_color, enclosed_color;
	bool is_enclosed = false;

	if(!canvas){
		return -1;
	}

	switch(font_size){
		case FONT_SIZE_16:
			asc_width = 8;
			gb2312_width = 16;
            asc    = _asc_16;
            gb2312 = _gb2312_16;
			break;

		case FONT_SIZE_20:
			asc_width = 10;
			gb2312_width = 20;
            asc    = _asc_20;
            gb2312 = _gb2312_20;
			break;

		case FONT_SIZE_24:
			asc_width = 12;
			gb2312_width = 24;
            asc    = _asc_24;
            gb2312 = _gb2312_24;
			break;

		case FONT_SIZE_32:
			asc_width = 16;
			gb2312_width = 32;
            asc    = _asc_32;
            gb2312 = _gb2312_32;
			break;

		case FONT_SIZE_40:
			asc_width = 20;
			gb2312_width = 40;
            asc    = _asc_40;
            gb2312 = _gb2312_40;
			break;

		case FONT_SIZE_48:
			asc_width = 24;
			gb2312_width = 48;
            asc    = _asc_48;
            gb2312 = _gb2312_48;
			break;

		case FONT_SIZE_56:
			asc_width = 28;
			gb2312_width = 56;
            asc    = _asc_56;
            gb2312 = _gb2312_56;
			break;

		default:
			// not support font size
			return -1;
	}

    if(!asc || !gb2312){
		return -1;
	}

	text_height = font_size < canvas->height ? font_size : canvas->height;

	if(!style){
		style = _def_sytle;
	}

	// check style
	if(style & TEXT_STYLE_BACKGROUND_TRANSPARENT){
		RGB_PIXEL_FILL(bg_color, 0, 255, 255, 0);
	}else if(style & TEXT_STYLE_BACKGROUND_WHRITE){
		RGB_PIXEL_FILL(bg_color, _def_alpha, 255, 255, 255);
	}else if(style & TEXT_STYLE_BACKGROUND_BLACK){
		RGB_PIXEL_FILL(bg_color, _def_alpha, 0, 0, 0);
	}

	if(style & TEXT_STYLE_FOREGROUND_TRANSPARENT){
		RGB_PIXEL_FILL(fg_color, 0, 255, 0, 255);
	}else if(style & TEXT_STYLE_FOREGROUND_WHRITE){
		//RGB_PIXEL_FILL(fg_color, _def_alpha ^ 0x11, 255, 255, 255);
		RGB_PIXEL_FILL(fg_color, 255, 255, 255, 255);
	}else if(style & TEXT_STYLE_FOREGROUND_BLACK){
		RGB_PIXEL_FILL(fg_color, _def_alpha ^ 0x11, 0, 0, 0);
	}else if(style & TEXT_STYLE_FOREGROUND_RED){
		RGB_PIXEL_FILL(fg_color, 255, 255, 0, 0);
	}

	if(style & TEXT_STYLE_ENCLOSED_TRANSPARENT){
		is_enclosed = true;
		RGB_PIXEL_FILL(enclosed_color, 0, 0, 255, 255);
	}else if(style & TEXT_STYLE_ENCLOSED_WHRITE){
		is_enclosed = true;
		RGB_PIXEL_FILL(enclosed_color, _def_alpha ^ 0x22, 255, 255, 255);
	}else if(style & TEXT_STYLE_ENCLOSED_BLACK){
		is_enclosed = true;
		RGB_PIXEL_FILL(enclosed_color, _def_alpha ^ 0x22, 0, 0, 0);
	}
	//printf("bg_color: 0x%08x, fg_color: 0x%08x\n", bg_color.argb8888, fg_color.argb8888);
	// the font item stride bytes number, align 8bits / 1byte
#if 0
	int convert_the_color = 0;
#endif
	while('\0' != *ch)
	{
#if 0
	    if (*ch == '[')
		    RGB_PIXEL_FILL(fg_color, _def_alpha ^ 0x11, 255, 0, 0);
        else if (*ch == ']')
            convert_the_color = 2;

        convert_the_color--;
	    if (convert_the_color == 0)
		    RGB_PIXEL_FILL(fg_color, _def_alpha ^ 0x11, 255, 255, 255);
#endif

		if(*ch < 0x7f)
		{
			// ascii
			int const stride_bytes = (asc_width + 7) / 8;
			int const asc_code = *ch;
            unsigned long offset = 0;
            if(font_asc_offset_byte)
            {
                unsigned long dwCharInfo = 0;
                memcpy(&dwCharInfo, &asc[font_asc_offset_byte + asc_code*4], sizeof(dwCharInfo));
                offset = dwCharInfo & 0x3ffffff;
            }
            else
                offset = asc_code * font_size * stride_bytes;

			// check width limited
			if(x + asc_width > canvas->width){
				break;
			}

			for(i = 0; i < font_size; ++i){
				int stride_pixel = 0;
				//printf("\n");
				for(ii = 0; ii < stride_bytes && stride_pixel < asc_width; ++ii){
					off_t const offset_byte = offset + i * stride_bytes + ii;
					for(iii = 0; iii < 8 && stride_pixel < asc_width; ++iii){
						unsigned char const actived_px = 0x80>>iii;//(1<<(8-1-iii));

						if(asc[offset_byte] & actived_px){
							//printf("1");
							canvas_put_pixel(canvas, x + ii * 8 + iii, y + i, fg_color);
						}else{
							//printf("0");
							canvas_put_pixel(canvas, x + ii * 8 + iii, y + i, bg_color);
						}
						++stride_pixel;
					}
				}
			}
			//printf("\n");
			x += asc_width;
			ch += 1;
		}
		else if(*ch > 0xa0)
		{
			int const stride_bytes = (gb2312_width + 7) / 8;
			// get qu code and wei code
			int const qu_code = ch[0] - 0xa0 - 1; // 87
			int const wei_code = ch[1] - 0xa0 - 1; // 94
			unsigned long offset = font_offset_byte+ (qu_code * 94 + wei_code) * font_size * stride_bytes;

			if(6 == qu_code){
				// russian
			}
			else
			{
				for(i = 0; i < font_size; ++i)
				{
					int stride_pixel = 0;
					for(ii = 0; ii < stride_bytes && stride_pixel < gb2312_width; ++ii)
					{
						int const offset_byte = offset + i * stride_bytes + ii;
						for(iii = 0; iii < 8 && stride_pixel < gb2312_width; ++iii)
						{
							unsigned char const actived_px = (0x80>>iii);

							if(gb2312[offset_byte] & actived_px){
								canvas_put_pixel(canvas, x + ii * 8 + iii, y + i, fg_color);
							}else{
								canvas_put_pixel(canvas, x + ii * 8 + iii, y + i, bg_color);
							}
							++stride_pixel;
						}
					}
				}
			}
			x += gb2312_width;
			ch += 2;
		}
	}

	text_width = x - x_base;

	if(is_enclosed){
		for(i = 0; i < text_height - 1 ; ++i){
			for(ii = 0; ii < text_width - 1 ; ++ii){
				canvas_pixel_t center_color;
				canvas_get_pixel(canvas, x_base + ii, y_base + i, &center_color);
				if(0 == i || text_height - 1 == i
					|| 0 == ii || text_width - 1 == ii){
					// on the edage
					if(canvas_match_pixel(canvas, fg_color, center_color)){
						canvas_put_pixel(canvas, x_base + ii, y_base + i, enclosed_color);
					}
				}else{
					canvas_pixel_t right_color, bottom_color;

					canvas_get_pixel(canvas, x_base + ii + 1, y_base + i, &right_color); // to the right
					canvas_get_pixel(canvas, x_base + ii, y_base + i + 1, &bottom_color); // to the bottom

					if(canvas_match_pixel(canvas, bg_color, center_color)){
						// background
						if(canvas_match_pixel(canvas, fg_color, right_color)
							|| canvas_match_pixel(canvas, fg_color, bottom_color)){
							canvas_put_pixel(canvas, x_base + ii, y_base + i, enclosed_color);
						}
					}else if(canvas_match_pixel(canvas, fg_color, center_color)){
						// forground
						if(canvas_match_pixel(canvas, bg_color, right_color)){
							canvas_put_pixel(canvas, x_base + ii + 1, y_base + i, enclosed_color);
						}
						if(canvas_match_pixel(canvas, bg_color, bottom_color)){
							canvas_put_pixel(canvas, x_base + ii, y_base + i + 1, enclosed_color);
						}
					}
				}
			}
		}
	}

	return x - x_base; // write put size
}


