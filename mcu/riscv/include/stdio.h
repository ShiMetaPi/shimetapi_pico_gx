
#ifndef _STDIO_H_
#define _STDIO_H_
/******************************************************************************/

#include <config.h>
#include <stddef.h>

int sprintf(char *buf, const char *cfmt, ...);
int snprintf(char *str, size_t size, const char *format, ...);

int getchar(void);
void puts_always(const char *s);
void puthex_always(unsigned int hex);

#ifdef CONFIG_PRINT
#ifdef CONFIG_PRINTF
int printf(const char *cfmt, ...);
#else
#  define printf(_cfmt, ...)
#endif
int putchar(int c);
int puts(const char *s);
void puthex(unsigned int hex);
void putdec(unsigned int dec);
#else
#  define printf(_cfmt, ...)
#  define putchar(c)
#  define puts(_s)
#  define puthex(hex)
#  define putdec(dec)
#endif

#ifndef MOULE_NAME
#define MOULE_NAME ""
#endif

#define pr_error(_fmt, args...)  printf(MOULE_NAME _fmt, ##args)
#define pr_warn(_fmt, args...)   printf(MOULE_NAME _fmt, ##args)
#define pr_info(_fmt, args...)   printf(MOULE_NAME _fmt, ##args)

#ifdef PR_DEBUG
#  define pr_debug(_fmt, args...)  printf(MOULE_NAME _fmt, ##args)
#else
#  define pr_debug(_fmt, args...)
#endif /* pr_debug */

#define bug(_fmt, args...)  { \
	printf(MOULE_NAME "%s(%d): "_fmt, __FILE__, __LINE__, ##args); \
	while (1); }

/******************************************************************************/
#endif /* _STDIO_H_ */

