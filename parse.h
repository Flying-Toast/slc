#ifndef __PARSE_H
#define __PARSE_H

#include "lex.h"

struct decl {
	// XXX: leaked
	str_t name;
	struct ty ty;
};

struct lvalue {
	str_t ident;
};

enum expr_tag {
	EXPR_INTLIT,
	EXPR_ADD,
	EXPR_IDENT,
};

struct expr {
	enum expr_tag tag;
	union {
		int64_t intlit;
		struct {
			struct expr *lhs;
			struct expr *rhs;
		} add;
		str_t ident;
	} as;
};

enum stmt_tag {
	STMT_RETURN,
	STMT_ASSIGN,
};

struct stmt {
	enum stmt_tag tag;
	union {
		struct expr *return_;
		struct {
			struct lvalue lvalue;
			struct expr *rvalue;
		} assign;
	} as;
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

	struct da_stmt {
		struct stmt *ptr;
		size_t len;
		size_t cap;
	} stmts;
};

enum item_type {
	ITEM_FUNC,
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
