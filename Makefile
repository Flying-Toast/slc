.POSIX:
.SUFFIXES:
.SUFFIXES: .c .o

CC = clang
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -g

obj = \
	lex.o \
	parse.o \
	main.o

headers = \
	da.h \
	lex.h \
	parse.h \
	str.h

slc: $(obj)
	@printf 'LD\t$@\n'
	@$(CC) $(CFLAGS) $(obj) -o $@

$(obj): $(headers)

.c.o:
	@printf 'CC\t$@\n'
	@$(CC) $(CFLAGS) -c $< -o $*.o

clean:
	rm -f *.o
	rm -f slc
