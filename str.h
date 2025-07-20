#ifndef __STR_H
#define __STR_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define STR(LIT) (str_t){ .ptr = ""LIT"", .len = sizeof(LIT) - 1 }
#define PRSTR(S) (int)((S).len), ((S).ptr)

typedef struct {
	char *ptr;
	size_t len;
} str_t;

static inline str_t str_dup(str_t s) {
	str_t ret;

	ret.len = s.len;
	ret.ptr = malloc(s.len);
	memcpy(ret.ptr, s.ptr, s.len);

	return ret;
}

static inline _Bool str_eq(str_t a, str_t b) {
	if (a.len != b.len)
		return 0;

	for (size_t i = 0; i < a.len; i++) {
		if (a.ptr[i] != b.ptr[i])
			return 0;
	}

	return 1;
}

#endif
