#ifndef __PARSE_H
#define __PARSE_H

#include "lex.h"

struct decl {
	// TODO: don't leak
	str_t name;
	struct ty ty;
};

enum item_type {
	ITEM_FUNC,
};

struct func_item {
	// TODO: don't leak
	str_t name;
	struct ty return_type;

	struct da_decl {
		struct decl *ptr;
		size_t len;
		size_t cap;
	} args;

	struct da_decl stackvars;
};

struct item {
	enum item_type type;
	union {
		struct func_item func;
	} as;
};

struct parser {
	struct lexer l;
};

void parser_init(struct parser *p, FILE *in);
int parser_next(struct parser *p, struct item *out);
void parser_dispose(struct parser *p);

#endif
