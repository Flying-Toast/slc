#ifndef __LEX_H
#define __LEX_H

#include <stdint.h>
#include <stdio.h>
#include "str.h"

#define ENUMERATE_TOKEN_TYPES(X) \
	X(TOKEN_FN) \
	X(TOKEN_VOID) \
	X(TOKEN_I32) \
	X(TOKEN_UINTLIT) \
	X(TOKEN_IDENT) \
	X(TOKEN_RETURN) \
	X(TOKEN_LPAREN) \
	X(TOKEN_RPAREN) \
	X(TOKEN_LBRACE) \
	X(TOKEN_RBRACE) \
	X(TOKEN_COMMA) \
	X(TOKEN_PLUS) \
	X(TOKEN_EQUALS) \
	X(TOKEN_SEMICOLON) \
	X(INVALID_TOKEN)

enum token_type {
#define X(T) T,
	ENUMERATE_TOKEN_TYPES(X)
#undef X
};

struct token {
	enum token_type type;
	union {
		str_t ident;
		uint64_t uintlit;
	} as;
};

struct lexer {
	FILE *in;
	struct token peeked;
	size_t line, col;
};

void lexer_init(struct lexer *l, FILE *in);
void lexer_dispose(struct lexer *l);
int lexer_next(struct lexer *l, struct token *out);
int lexer_peek(struct lexer *l, struct token *out);
str_t toktype2str(enum token_type);
void token_dispose(struct token *t);

#endif
