#include <err.h>
#include <inttypes.h>
#include "gen.h"

void codegen_init(struct codegen *cg, FILE *in, FILE *out) {
	parser_init(&cg->p, in);
	cg->out = out;
}

void codegen_dispose(struct codegen *cg) {
	fclose(cg->out);
	parser_dispose(&cg->p);
}

static uint64_t tysize(const struct ty *ty) {
	switch (ty->tag) {
	case TY_I32:
		return 4;
	case TY_VOID:
		errx(1, "can't take tysize(TY_VOID)");
	}
}

static void check_func(const struct func_item *f) {
	if (f->return_type.tag != TY_I32)
		errx(1, "TODO: support non-i32 return type");

	for (size_t i = 0; i < f->args.len; i++) {
		for (size_t j = 0; j < f->args.len; j++) {
			if (i != j && str_eq(f->args.ptr[i].name, f->args.ptr[j].name))
				errx(1, "duplicate arg name: %.*s", PRSTR(f->args.ptr[i].name));
		}

		for (size_t svi = 0; svi < f->stackvars.len; svi++) {
			if (str_eq(f->stackvars.ptr[svi].name, f->args.ptr[i].name))
				errx(1, "stackvar/arg name collision: %.*s", PRSTR(f->args.ptr[i].name));
		}
	}

	for (size_t i = 0; i < f->stackvars.len; i++) {
		for (size_t j = 0; j < f->stackvars.len; j++) {
			if (i != j && str_eq(f->stackvars.ptr[i].name, f->stackvars.ptr[j].name))
				errx(1, "duplicate stackvar name .%.*s.", PRSTR(f->stackvars.ptr[i].name));
		}
	}
}

static void gen_func(struct codegen *cg, const struct func_item *f) {
	check_func(f);

	uint64_t stacksize = 0;
	for (size_t i = 0; i < f->stackvars.len; i++)
		stacksize += tysize(&f->stackvars.ptr[i].ty);
	// align sp to 16
	while (stacksize % 16)
		stacksize += 1;

	fprintf(cg->out,
		".globl %.*s\n"
		"%.*s:\n"
		, PRSTR(f->name)
		, PRSTR(f->name)
	);

	if (stacksize)
		fprintf(cg->out, "\taddi sp, sp, -0x%"PRIx64"\n", stacksize);
}

static void gen_item(struct codegen *cg, const struct item *itm) {
	switch (itm->type) {
	case ITEM_FUNC:
		gen_func(cg, &itm->as.func);
		break;
	}
}

void codegen_gen(struct codegen *cg) {
	struct item it;

	while ((parser_next(&cg->p, &it)) == 0) {
		gen_item(cg, &it);
	}
}
