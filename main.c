#include <err.h>
#include "lex.h"

int main(int argc, char **argv) {
	FILE *in;
	struct lexer l;

	if (argc != 2)
		errx(1, "usage: slc <in.c>");

	if ((in = fopen(argv[1], "r")) == NULL)
		err(1, "%s", argv[1]);

	lexer_init(&l, in);

	for (struct token t; lexer_next(&l, &t) == 0;) {
		str_t s = toktype2str(t.type);
		printf("%.*s", PRSTR(s));
		if (t.type == TOKEN_UINTLIT)
			printf(": %zu", t.as.uintlit);
		printf("\n");
	}

	lexer_dispose(&l);
}
