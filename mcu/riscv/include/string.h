
#ifndef _STRING_H_
#define _STRING_H_
/******************************************************************************/

#include <stddef.h>

extern char const hex2ascii_data[];

#define hex2ascii(hex) (hex2ascii_data[hex])

#define imax(_a, _b) ((_a) > (_b) ? (_a) : (_b))

#define bcpy(src, dst, len) memcpy((dst), (src), (len))
#define bzero(buf, size)    memset((buf), 0, (size))
#define bcmp(b1, b2, len)   (memcmp((b1), (b2), (len)) != 0)

void *memmove(void *dest, const void *src, size_t size);
void *memcpy(void *dst, const void *src, size_t len);
void *memset(void *b, int c, size_t len);
int memcmp(const void *b1, const void *b2, size_t len);
int strncmp(const char *s1, const char *s2, size_t len);
uint32_t strnlen(const char *s, uint32_t len);
unsigned long strtoul(const char *nptr, char **endptr, int base);
uint64_t strtoull(const char *nptr, char **endptr, int base);
char *strncpy(char * dst, const char * src, size_t n);
uint64_t memparse(const char *ptr, char **retptr);
char *strndup(const char *str, size_t n);
char *strtok(char *s, const char *delim);

int strcmp(const char *s1, const char *s2);
void strcpy(char *dst, const char *src);
void strcat(char *dst, const char *src);
char *strncat(char *dst, const char *src, size_t n);
char *strchr(const char *s, char ch);
char *strdup(const char *s);
size_t strlen(const char *s);
char *strstr(const char *s1, const char *s2);

/******************************************************************************/
#endif /* _STRING_H_ */
