
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <div64.h>
#include <serial.h>

extern void serial_put_hex(u32 hex);
extern void serial_put_dec(u32 dec);
/*****************************************************************************/

int putchar(int c)
{
	if (c == '\n')
		serial_putc('\r');
	serial_putc(c);
	return c;
}

int puts(const char *s)
{
	const char *ptr = s;

	while (*ptr)
		putchar(*ptr++);

	return (ptr - s);
}

void puts_always(const char *s)
{
	serial_puts_always(s);
}

void puthex(unsigned int hex)
{
	serial_put_hex(hex);
}

void puthex_always(unsigned int hex)
{
	serial_put_hex_always(hex);
}

void putdec(unsigned int dec)
{
	serial_put_dec(dec);
}

int getchar(void)
{
	return serial_getc();
}
