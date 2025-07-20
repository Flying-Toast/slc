#include <err.h>
#include "da.h"
#include "parse.h"

void parser_init(struct parser *p, FILE *in) {
	lexer_init(&p->l, in);
}

void parser_dispose(struct parser *p) {
	lexer_dispose(&p->l);
}

static struct token expect(struct parser *p, enum token_type type) {
	struct token ret;
	str_t s1, s2;

	if (lexer_next(&p->l, &ret)) {
		s1 = toktype2str(type);
		errx(1, "%d:%d: got EOF when expecting %.*s", p->l.line, p->l.col, PRSTR(s1));
	}

	if (ret.type != type) {
		s1 = toktype2str(ret.type);
		s2 = toktype2str(type);
		errx(1, "%d:%d: got %.*s when expecting %.*s", p->l.line, p->l.col, PRSTR(s1), PRSTR(s2));
	}

	return ret;
}

static _Bool peekis(struct parser *p, enum token_type tok) {
	struct token peeked;
	if (lexer_peek(&p->l, &peeked))
		return 0;
	return peeked.type == tok;
}

static void consume1(struct parser *p) {
	struct token t;
	lexer_next(&p->l, &t);
}

static struct decl parse_decl(struct parser *p) {
	struct decl ret;

	ret.ty = expect(p, TOKEN_TY).as.ty;
	ret.name = expect(p, TOKEN_IDENT).as.ident;

	return ret;
}

static void parse_func(struct parser *p, struct func_item *func) {
	struct token rettok, nametok;
	da_init(func->args);
	da_init(func->stackvars);

	expect(p, TOKEN_FN);

	rettok = expect(p, TOKEN_TY);
	func->return_type = rettok.as.ty;

	nametok = expect(p, TOKEN_IDENT);
	func->name = nametok.as.ident;

	expect(p, TOKEN_LPAREN);
	if (!peekis(p, TOKEN_RPAREN)) {
		for (;;) {
			da_append(func->args, parse_decl(p));

			if (peekis(p, TOKEN_COMMA))
				consume1(p);
			else
				break;
		}
	}
	expect(p, TOKEN_RPAREN);

	while (!peekis(p, TOKEN_LBRACE)) {
		da_append(func->stackvars, parse_decl(p));
		expect(p, TOKEN_SEMICOLON);
	}

	expect(p, TOKEN_LBRACE);
	///////////////while (!peekis(p, TOKEN_RBRACE)) {
	///////////////}
	expect(p, TOKEN_RBRACE);
}

int parser_next(struct parser *p, struct item *out) {
	struct token peek;

	if (lexer_peek(&p->l, &peek))
		return 1;

	switch (peek.type) {
	case TOKEN_FN:
		out->type = ITEM_FUNC;
		parse_func(p, &out->as.func);
		break;
	default:
		errx(1, "not start of item: %.*s", toktype2str(peek.type));
		break;
	};

	return 1;
}
