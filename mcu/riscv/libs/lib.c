
#include <stdio.h>
#include <lib.h>
#include <serial.h>

int memset_s(void* dest, size_t destMax, unsigned char c, size_t count)
{
    unsigned char *__dest = (unsigned char *)dest;

    if (destMax == 0 || destMax > SECUREC_MEM_MAX_LEN) {
        return FAILURE;
    }

    if (__dest == NULL) {
        return FAILURE;
    }

    if (count > destMax) {
        while (destMax--)
		    *__dest++ = (unsigned char)c;   /*set entire buffer to value c*/
        return FAILURE;
    }

    while (count--)
		*__dest++ = (unsigned char)c;

    return SUCCESS;
}

int memset(void* dest, unsigned char c, size_t count)
{
	return memset_s(dest, SECUREC_MEM_MAX_LEN, c, count);
}

int memcpy_s(void* dest, size_t destMax, const void* src, size_t count)
{
    unsigned char *__dest = (unsigned char *)dest;
    unsigned char *__src = (unsigned char *)src;

    if (destMax == 0 || destMax > SECUREC_MEM_MAX_LEN ) {
        return FAILURE;
    }

    if (__dest == NULL || __src == NULL) {
        if (__dest != NULL ) {
            while (destMax--)
	        *__dest++ = 0;   /*set entire buffer to value 0 */
            return FAILURE;
        }
        return FAILURE;
    }

    if (count > destMax) {
        while (destMax--)
	    *__dest++ = 0;       /*set entire buffer to value 0 */
        return FAILURE;
    }

    if (__dest == __src) {
        return SUCCESS;
    }

    if ((__dest > __src && __dest < ((u8*)__src + count)) ||
        (__src > __dest && __src < ((u8*)__dest + count)) ) {
        while (destMax--)
		    *__dest++ = 0;   /*set entire buffer to value 0 */

        return FAILURE;
    }

	while (count--)
		*__dest++ = *__src++;

    return SUCCESS;
}

void *memcpy(void *dst, const void *src, size_t len)
{
	if(SUCCESS == memcpy_s(dst, len, src, len))
		return dst;
	return NULL;
}

int memcmp(const void *b1, const void *b2, size_t len)
{
	int res = 0;
	const char *d = b1;
	const char *s = b2;

	if (b1 == NULL || b2 == NULL) {
		puts("memcmp : Invalid args\n");
		return res;
	}

	while (len > 0) {
		res = *d - *s;
		if (res != 0)
			break;
		d++;
		s++;
		len--;
	}

	return res;
}

u32 strlen(const char * s)
{
	const char *sc;

	for (sc = s; *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}

char *strcat(char *dest, const char *src)
{
	char *tmp = dest;

	while (*dest)
		dest++;
	while ((*dest++ = *src++) != '\0')
		;

	return tmp;
}

char *strcpy(char *dest, const char*src)
{
	char *tmp = dest;

	while (*src != '\0') {
		*dest = *src;
		dest++;
		src++;
	}

	*dest = '\0';

	return tmp;
}


char *strncpy(char *dest, char *src, size_t n)
{
	char *tmp = dest;

	while (n > 0 && *src != '\0') {
		*dest = *src;
		dest++;
		src++;
		n--;
	}

	while (n > 0) {
		*dest = '\0';
		dest++;
		n--;
	}

	return tmp;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    if (n == 0)
        return 0;
    do {
        if (*s1 != *s2++)
            return *(const unsigned char*)s1 - *(const unsigned char*)--s2;
        if (*s1++ == 0)
            break;
    } while (--n != 0);
    return 0;
}

/*****************************************************************************/
/* for gcc -fstack-protector-all */
unsigned long __stack_chk_guard = 0x000a0dff;

void __stack_chk_fail(void)
{
        serial_puts("Stack is corrupted\n");
        while (1) {
                reset_cpu();
        }
}

unsigned short crc16(unsigned char *data, unsigned int length)
{
	unsigned int i;
	unsigned short crc_table;
	unsigned short crc = 0xffff;
	unsigned char inter1;
	unsigned char inter2;

	if (data == NULL)
		return (crc);

	for (i = 0; i < length; ++i) {
		inter1 = data[i] ^ ((unsigned char) crc);
		inter2 = (unsigned char) (inter1 ^ (inter1 << 4)); /* bit size 4 */
		crc_table = (inter2 << 8) ^ (inter2 << 3) ^ (inter2 >> 4); /* bit size 8 3 4 */
		crc = (crc >> 8) ^ crc_table; /* bit size 8 */
	}
	crc ^= 0xffff;
	return (crc);
}

