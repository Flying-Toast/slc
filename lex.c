#include <err.h>
#include <ctype.h>
#include "lex.h"

void lexer_init(struct lexer *l, FILE *in) {
	l->in = in;
	l->peeked.type = INVALID_TOKEN;
	l->line = 1;
	l->col = 1;
	l->ungot = 0;
}

void lexer_dispose(struct lexer *l) {
	fclose(l->in);
	token_dispose(&l->peeked);
}

void token_dispose(struct token *t) {
	if (t->type == TOKEN_IDENT) {
		free(t->as.ident.ptr);
	}
}

static int lgetc(struct lexer *l) {
	int ret;

	if (l->ungot != 0) {
		ret = l->ungot;
		l->ungot = 0;
	} else {
		ret = fgetc(l->in);
	}

	if (ret == '\n') {
		l->line += 1;
		l->col = 1;
	} else if (ret != EOF) {
		l->col += 1;
	}
	return ret;
}

str_t toktype2str(enum token_type ty) {
#define X(T) if (ty == T) { return STR(#T); }
	ENUMERATE_TOKEN_TYPES(X)
#undef X
	errx(1, "UNREACHABLE");
}

static int isidentheadchar(int ch) {
	return ch == '_' || isalpha(ch);
}

static int isidenttailchar(int ch) {
	return isidentheadchar(ch) || isdigit(ch);
}

static void lungetc(struct lexer *l, int ch) {
	if (l->ungot != 0) {
		ungetc(l->ungot, l->in);
	}
	l->ungot = ch;

	if (ch == '\n') {
		l->line -= 1;
		l->col = 123456;
	} else if (ch != EOF) {
		l->col -= 1;
	}
}

static int lpeekc(struct lexer *l) {
	int ret = lgetc(l);
	lungetc(l, ret);
	return ret;
}

static int lgetword(struct lexer *l, str_t *out_buf) {
	size_t idx = 0;
	int ch;

	if (!isidentheadchar(lpeekc(l)))
		return 1;

	while (idx < out_buf->len) {
		ch = lgetc(l);

		if (!isidenttailchar(ch)) {
			lungetc(l, ch);
			out_buf->len = idx;
			return 0;
		}

		out_buf->ptr[idx++] = ch;
	}

	errx(1, "word too long: %.*s", PRSTR(*out_buf));
}

static void word2tok(str_t word, struct token *out) {
	if (str_eq(word, STR("fn"))) {
		out->type = TOKEN_FN;
	} else if (str_eq(word, STR("void"))) {
		out->type = TOKEN_TY;
		out->as.ty.tag = TY_VOID;
	} else if (str_eq(word, STR("i32"))) {
		out->type = TOKEN_TY;
		out->as.ty.tag = TY_I32;
	} else if (str_eq(word, STR("return"))) {
		out->type = TOKEN_RETURN;
	} else {
		out->type = TOKEN_IDENT;
		out->as.ident = str_dup(word);
	}
}

static void lskipspace(struct lexer *l) {
	int ch;

	while (isspace(ch = lgetc(l)))
		;

	lungetc(l, ch);
}

static int lgetsymb(struct lexer *l, struct token *out) {
	int ch;
	switch (ch = lgetc(l)) {
	case '{':
		out->type = TOKEN_LBRACE;
		return 0;
	case '}':
		out->type = TOKEN_RBRACE;
		return 0;
	case '(':
		out->type = TOKEN_LPAREN;
		return 0;
	case ')':
		out->type = TOKEN_RPAREN;
		return 0;
	case ',':
		out->type = TOKEN_COMMA;
		return 0;
	case '+':
		out->type = TOKEN_PLUS;
		return 0;
	case ';':
		out->type = TOKEN_SEMICOLON;
		return 0;
	case '=':
		out->type = TOKEN_EQUALS;
		return 0;
	default:
		lungetc(l, ch);
		return 1;
	}
}

static int lgetintlit(struct lexer *l, struct token *out) {
	int peek = lpeekc(l);

	if (!isdigit(peek) && peek != '-')
		return 1;

	_Bool negative = 0;
	int64_t lit = 0, prev = 0;

	if (peek == '-') {
		lgetc(l);
		negative = 1;
	}

	while (isdigit(lpeekc(l))) {
		char dig = lgetc(l);

		lit *= 10;
		lit += dig - '0';

		if (lit < prev)
			errx(1, "literal overflow at %d:%d", l->line, l->col);
		prev = lit;
	}

	if (negative)
		lit *= -1;

	out->type = TOKEN_INTLIT;
	out->as.intlit = lit;
	return 0;
}

static int lskipcomment(struct lexer *l) {
	int c1, c2;

	c1 = lgetc(l);
	c2 = lgetc(l);

	if (c1 == EOF || c2 == EOF)
		return 1;

	if (c1 != '/' || c2 != '/') {
		lungetc(l, c2);
		lungetc(l, c1);
		return 1;
	}

	while ((c1 = lgetc(l)) != EOF && c1 != '\n')
		;

	return 0;
}

int lexer_next(struct lexer *l, struct token *out) {
	char wordbufbuf[100];
	str_t wordbuf = {
		.ptr = wordbufbuf,
		.len = sizeof(wordbufbuf),
	};

	if (l->peeked.type != INVALID_TOKEN) {
		*out = l->peeked;
		l->peeked.type = INVALID_TOKEN;
		return 0;
	}

	lskipspace(l);
	if (lskipcomment(l) == 0)
		return lexer_next(l, out);

	if (feof(l->in))
		return 1;

	if (lgetword(l, &wordbuf) == 0) {
		word2tok(wordbuf, out);
		return 0;
	}

	if (lgetsymb(l, out) == 0)
		return 0;

	if (lgetintlit(l, out) == 0) {
		return 0;
	}

	errx(1, "lex error at %d:%d", l->line, l->col);
}

int lexer_peek(struct lexer *l, struct token *out) {
	if (l->peeked.type == INVALID_TOKEN && lexer_next(l, &l->peeked))
		return 1;

	*out = l->peeked;
	return 0;
}
