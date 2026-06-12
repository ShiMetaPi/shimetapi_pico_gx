/*
 * Copyright (c) XMEDIA. All rights reserved.
 */

#ifndef _SYS_CTYPE_H_
#define _SYS_CTYPE_H_

#define isspace(c)  ((c) == ' ' || ((c) >= '\t' && (c) <= '\r'))
#define isascii(c)  (((c) & ~0x7f) == 0)
#define isupper(c)  ((c) >= 'A' && (c) <= 'Z')
#define islower(c)  ((c) >= 'a' && (c) <= 'z')
#define isalpha(c)  (isupper(c) || islower(c))
#define isdigit(c)  ((c) >= '0' && (c) <= '9')
#define isxdigit(c) (isdigit(c) \
		     || ((c) >= 'A' && (c) <= 'F') \
		     || ((c) >= 'a' && (c) <= 'f'))
#define isprint(c)  ((c) >= ' ' && (c) <= '~')
#define toupper(c)  ((c) - 0x20 * (((c) >= 'a') && ((c) <= 'z')))
#define tolower(c)  ((c) + 0x20 * (((c) >= 'A') && ((c) <= 'Z')))

#define min(x, y)	((x) < (y) ? (x) : (y))
#define min_t(t, x, y)	((t)((t)(x) < (t)(y) ? (x) : (y)))

#define tohex(c)	(((c) >= '0' && (c) <= '9') ? ((c) - '0') : \
			  (((c) >= 'a' && (c) <= 'f') ? ((c) - 'a' + 10) : \
			  (((c) >= 'A' && (c) <= 'F') ? ((c) - 'A' + 10) : -1)))

#endif /* !_SYS_CTYPE_H_ */
