#include <err.h>
#include <inttypes.h>
#include <stdio.h>
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

static struct decl parse_decl(struct parser *p) {
	struct decl ret;

	ret.ty = expect(p, TOKEN_TY).as.ty;
	ret.name = str_dup(expect(p, TOKEN_IDENT).as.ident);

	return ret;
}

static struct expr *parse_expr(struct parser *p) {
	// XXX: leak
	struct expr *ret = malloc(sizeof(*ret));

	if (peekis(p, TOKEN_INTLIT)) {
		ret->tag = EXPR_INTLIT;
		ret->as.intlit = expect(p, TOKEN_INTLIT).as.intlit;
		return ret;
	}

	if (peekis(p, TOKEN_PLUS)) {
		expect(p, TOKEN_PLUS);
		ret->tag = EXPR_ADD;
		expect(p, TOKEN_LPAREN);
		ret->as.add.lhs = parse_expr(p);
		expect(p, TOKEN_COMMA);
		ret->as.add.rhs = parse_expr(p);
		expect(p, TOKEN_RPAREN);
		return ret;
	}

	if (peekis(p, TOKEN_IDENT)) {
		ret->tag = EXPR_IDENT;
		ret->as.ident = str_dup(expect(p, TOKEN_IDENT).as.ident);
		return ret;
	}

	errx(1, "parse error at %d:%d\n", p->l.line, p->l.col);
}

static struct expr *parse_return_stmt(struct parser *p) {
	expect(p, TOKEN_RETURN);
	struct expr *ret = parse_expr(p);
	expect(p, TOKEN_SEMICOLON);
	return ret;
}

static struct lvalue parse_lvalue(struct parser *p) {
	struct lvalue ret;
	ret.ident = str_dup(expect(p, TOKEN_IDENT).as.ident);
	return ret;
}

static struct stmt parse_assign(struct parser *p) {
	struct stmt ret;
	ret.tag = STMT_ASSIGN;

	ret.as.assign.lvalue = parse_lvalue(p);
	ret.as.assign.rvalue = parse_expr(p);

	return ret;
}

static struct stmt parse_stmt(struct parser *p) {
	struct stmt ret;

	if (peekis(p, TOKEN_RETURN)) {
		ret.tag = STMT_RETURN;
		ret.as.return_ = parse_return_stmt(p);
		return ret;
	}

	// anything else is assumed an assignment
	return parse_assign(p);
}

static void parse_func(struct parser *p, struct func_item *func) {
	struct token rettok, nametok;
	da_init(func->args);
	da_init(func->stackvars);
	da_init(func->stmts);

	expect(p, TOKEN_FN);

	rettok = expect(p, TOKEN_TY);
	func->return_type = rettok.as.ty;

	nametok = expect(p, TOKEN_IDENT);
	func->name = str_dup(nametok.as.ident);

	expect(p, TOKEN_LPAREN);
	if (!peekis(p, TOKEN_RPAREN)) {
		for (;;) {
			da_append(func->args, parse_decl(p));

			if (peekis(p, TOKEN_COMMA))
				expect(p, TOKEN_COMMA);
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
	while (!peekis(p, TOKEN_RBRACE))
		da_append(func->stmts, parse_stmt(p));
	expect(p, TOKEN_RBRACE);
}

int parser_next(struct parser *p, struct item *out) {
	struct token peek;
	str_t s;

	if (lexer_peek(&p->l, &peek))
		return 1;

	switch (peek.type) {
	case TOKEN_FN:
		out->type = ITEM_FUNC;
		parse_func(p, &out->as.func);
		break;
	default:
		s = toktype2str(peek.type);
		errx(1, "not start of item: %.*s", PRSTR(s));
		break;
	};

	return 0;
}

static void print_type(const struct ty *ty) {
	switch (ty->tag) {
	case TY_I32:
		printf("i32");
		break;
	case TY_VOID:
		printf("void");
		break;
	default:
		errx(1, "unhandled type %d", ty->tag);
		break;
	}
}

static void print_decl(const struct decl *d) {
	print_type(&d->ty);
	printf(" %.*s", PRSTR(d->name));
}

static void print_expr(const struct expr *e) {
	switch (e->tag) {
	case EXPR_ADD:
		printf("(");
		print_expr(e->as.add.lhs);
		printf(") + (");
		print_expr(e->as.add.rhs);
		printf(")");
		break;
	case EXPR_IDENT:
		printf("%.*s", PRSTR(e->as.ident));
		break;
	case EXPR_INTLIT:
		printf("%"PRIi64, e->as.intlit);
		break;
	default:
		errx(1, "unhandled expr %d", (int)e->tag);
		break;
	}
}

static void print_lvalue(const struct lvalue *l) {
	printf("%.*s", PRSTR(l->ident));
}

static void print_stmt(const struct stmt *s) {
	switch (s->tag) {
	case STMT_ASSIGN:
		print_lvalue(&s->as.assign.lvalue);
		printf(" = ");
		print_expr(s->as.assign.rvalue);
		break;
	case STMT_RETURN:
		printf("return ");
		print_expr(s->as.return_);
		break;
	default:
		errx(1, "unhandled stmt %d", (int)s->tag);
		break;
	}
}

static void print_func(const struct func_item *f) {
	printf("fn ");
	print_type(&f->return_type);
	printf(" %.*s(", PRSTR(f->name));
	for (size_t i = 0; i < f->args.len; i++) {
		print_decl(&f->args.ptr[i]);
		if (i < f->args.len - 1)
			printf(", ");
	}
	printf(")\n");
	for (size_t i = 0; i <f->stackvars.len; i++) {
		printf("\t");
		print_decl(&f->stackvars.ptr[i]);
		printf(";\n");
	}
	printf("{\n");
	for (size_t i = 0; i < f->stmts.len; i++) {
		printf("\t");
		print_stmt(&f->stmts.ptr[i]);
		printf(";\n");
	}
	printf("}\n");
}

void print_item(const struct item *i) {
	switch (i->type) {
	case ITEM_FUNC:
		print_func(&i->as.func);
		break;
	default:
		errx(1, "unhandled item %d", (int)i->type);
		break;
	}
}
