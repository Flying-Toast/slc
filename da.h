#ifndef __DA_H
#define __DA_H

#include <stddef.h>

#define da_init(DA) \
	do { \
		(DA).ptr = NULL; \
		(DA).len = 0; \
		(DA).cap = 0; \
	} while (0)

#define da_grow(DA, MINCAP) \
	do { \
		size_t __mincap = MINCAP; \
		if ((DA).cap >= __mincap) \
			break; \
		size_t __newcap = (DA).cap * 2; \
		if (__newcap == 0) \
			__newcap = 4; \
		if (__newcap < __mincap) \
			__newcap = __mincap; \
		(DA).ptr = realloc((DA).ptr, sizeof((DA).ptr[0]) * __newcap); \
	} while (0)

#define da_append(DA, ITEM) \
	do { \
		da_grow(DA, (DA).len + 1); \
		(DA).ptr[(DA).len] = ITEM; \
		(DA).len += 1; \
	} while (0)

#endif
