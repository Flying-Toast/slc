#include <err.h>
#include "parse.h"

int main(int argc, char **argv) {
	FILE *in;
	struct parser p;

	if (argc != 2)
		errx(1, "usage: slc <in.c>");

	if ((in = fopen(argv[1], "r")) == NULL)
		err(1, "%s", argv[1]);

	parser_init(&p, in);

	for (struct item it; parser_next(&p, &it) == 0;) {
		print_item(&it);
	}

	parser_dispose(&p);
}
