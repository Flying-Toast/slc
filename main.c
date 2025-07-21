#include <err.h>
#include "gen.h"

int main(int argc, char **argv) {
	FILE *in, *out;
	struct codegen cg;

	if (argc != 2 && argc != 3)
		errx(1, "usage: slc <in.c> [out.asm]");

	if ((in = fopen(argv[1], "r")) == NULL)
		err(1, "%s", argv[1]);

	if (argc == 2)
		out = stdout;
	else if ((out = fopen(argv[2], "w")) == NULL)
		err(1, "%s", argv[2]);

	codegen_init(&cg, in, out);
	codegen_gen(&cg);
	codegen_dispose(&cg);
}
