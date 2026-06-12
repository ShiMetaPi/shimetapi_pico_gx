/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <sys/types.h>

/*-----------------------------------------------------------------
 * serial interface
------------------------------------------------------------------*/
void serial_init(void);
void serial_putc(const char c);
void serial_puts(const char *s);
void serial_puts_always(const char *s);
void serial_put_dec(u32 dec);
void serial_put_hex(u32 hex);
void serial_put_hex_always(u32 hex);
char serial_getc(void);

#endif
