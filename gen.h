#ifndef __GEN_H
#define __GEN_H

#include "parse.h"

struct codegen {
	struct parser p;
	FILE *out;
};

void codegen_init(struct codegen *cg, FILE *in, FILE *out);
void codegen_dispose(struct codegen *cg);
void codegen_gen(struct codegen *cg);

#endif
