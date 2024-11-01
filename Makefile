# PGENC Makefile

CFLAGS = -g -std=c99 -pedantic -Wconversion -Wall -I include
CC = gcc

../selc/lib/libselc.a: 
	make -C ../selc lib/libselc.a 

lib/libselc.a: ../selc/lib/libselc.a 
	ln --force -s ../$< $@ 

include/selc: 
	ln --force -s ../../selc/include/selc $@

includes: include/selc

# error.h
build/pgenc/error.o : source/pgenc/error.c include/pgenc/error.h includes
	$(CC) $(CFLAGS) -c -o $@ $<

# charset.h 
build/pgenc/charset.o : source/pgenc/charset.c include/pgenc/charset.h includes
	$(CC) $(CFLAGS) -c -o $@ $<
bin/test_charset : tests/pgenc/charset.c build/pgenc/charset.o lib/libselc.a
	$(CC) $(CFLAGS) -o $@ $^
grind_test_charset : bin/test_charset
	valgrind -q --error-exitcode=1 --leak-check=full $^ 1>/dev/null

# buffer.h
build/pgenc/buffer.o : source/pgenc/buffer.c include/pgenc/buffer.h includes
	$(CC) $(CFLAGS) -c -o $@ $<
bin/test_buffer : tests/pgenc/buffer.c build/pgenc/buffer.o \
	build/pgenc/error.o \
	lib/libselc.a
	$(CC) $(CFLAGS) -o $@ $^ -lssl
grind_test_buffer : bin/test_buffer
	valgrind -q --error-exitcode=1 --leak-check=full $^ 1>/dev/null

# stack.h
build/pgenc/stack.o : source/pgenc/stack.c include/pgenc/stack.h includes
	$(CC) $(CFLAGS) -c -o $@ $<
bin/test_stack : tests/pgenc/stack.c build/pgenc/stack.o \
	build/pgenc/error.o \
	lib/libselc.a
	$(CC) $(CFLAGS) -o $@ $^
grind_test_stack : bin/test_stack
	valgrind -q --error-exitcode=1 --leak-check=full $^ 1>/dev/null

# parser.h
#build/pgenc/parser.o : source/pgenc/parser.c include/pgenc/parser.h includes
#	$(CC) $(CFLAGS) -c -o $@ $<
build/pgenc/parser_nr.o : source/pgenc/parser_nr.c include/pgenc/parser.h includes 
	$(CC) $(CFLAGS) -c -o $@ $<
bin/test_parser : tests/pgenc/parser.c build/pgenc/parser_nr.o \
	build/pgenc/error.o \
	build/pgenc/charset.o \
	build/pgenc/buffer.o \
	build/pgenc/stack.o \
	lib/libselc.a
	$(CC) $(CFLAGS) -o $@ $^ -lssl
grind_test_parser : bin/test_parser
	valgrind -q --error-exitcode=1 --leak-check=full $^ 1>/dev/null

lib/libpgenc_pristine.a : \
	build/pgenc/parser_nr.o \
	build/pgenc/stack.o \
	build/pgenc/buffer.o \
	build/pgenc/charset.o \
	build/pgenc/error.o 
	ar -crs $@ $^

# table.h
build/pgenc/table.o : source/pgenc/table.c include/pgenc/table.h includes
	$(CC) $(CFLAGS) -c -o $@ $<
bin/test_table : tests/pgenc/table.c \
	build/pgenc/table.o \
	lib/libselc.a
	$(CC) $(CFLAGS) -o $@ $^
grind_test_table : bin/test_table
	valgrind -q --error-exitcode=1 --leak-check=full $^ 1>/dev/null

# ast.h
build/pgenc/ast.o : source/pgenc/ast.c include/pgenc/ast.h includes
	$(CC) $(CFLAGS) -c -o $@ $<
bin/test_ast : tests/pgenc/ast.c \
	build/pgenc/ast.o \
	build/pgenc/error.o \
	lib/libselc.a
	$(CC) $(CFLAGS) -o $@ $^
grind_test_ast : bin/test_ast
	valgrind -q --error-exitcode=1 --leak-check=full $^ 1>/dev/null

# syntax.h
build/pgenc/syntax.o : source/pgenc/syntax.c include/pgenc/syntax.h includes
	$(CC) $(CFLAGS) -c -o $@ $<
bin/test_syntax : tests/pgenc/syntax.c build/pgenc/syntax.o \
	build/pgenc/error.o \
	build/pgenc/ast.o \
	build/pgenc/stack.o \
	lib/libselc.a
	$(CC) $(CFLAGS) -o $@ $^
grind_test_syntax : bin/test_syntax
	valgrind -q --error-exitcode=1 --leak-check=full $^ 1>/dev/null

# lang.h
build/pgenc/lang_gen.o : source/pgenc/lang/gen.c include/pgenc/lang.h includes
	$(CC) $(CFLAGS) -c -o $@ $<
build/pgenc/lang_parse.o : source/pgenc/lang/parse.c include/pgenc/lang.h 
	$(CC) $(CFLAGS) -c -o $@ $<
build/pgenc/lang_proto.o : source/pgenc/lang/proto.c include/pgenc/lang.h 
	$(CC) $(CFLAGS) -c -o $@ $<
bin/test_lang : tests/pgenc/lang.c \
	build/pgenc/lang_parse.o \
	build/pgenc/lang_gen.o \
	build/pgenc/lang_proto.o \
	build/pgenc/ast.o \
	build/pgenc/syntax.o \
	build/pgenc/table.o \
	lib/libpgenc_pristine.a \
	lib/libselc.a
	$(CC) $(CFLAGS) -o $@ $^ -lssl
grind_test_lang : bin/test_lang
	valgrind -q --error-exitcode=1 --leak-check=full $^ 1>/dev/null

lib/libpgenc.a : \
	build/pgenc/error.o \
	build/pgenc/charset.o \
	build/pgenc/buffer.o \
	build/pgenc/parser_nr.o \
	build/pgenc/ast.o \
	build/pgenc/syntax.o \
	build/pgenc/stack.o \
	build/pgenc/table.o \
	build/pgenc/lang_parse.o \
	build/pgenc/lang_gen.o \
	build/pgenc/lang_proto.o
	ar -crs $@ $^

# dummy build
bin/pgenc_dummy : source/pgenc/main.c \
	source/pgenc/dummy.c \
	lib/libpgenc.a \
	lib/libselc.a
	$(CC) $(CFLAGS) -o $@ $^ -lssl

# self_proto.c
tmp/pgenc/self_proto.c : bin/pgenc_dummy 
	bin/pgenc_dummy -s $@ -d pgc_self 
build/pgenc/self_proto.o : tmp/pgenc/self_proto.c include/pgenc/self.h
	$(CC) $(CFLAGS) -c -o $@ $<
bin/test_self_proto : tests/pgenc/self_proto.c \
	build/pgenc/self_proto.o \
	lib/libpgenc.a \
	lib/libselc.a
	$(CC) $(CFLAGS) -o $@ $^ -lssl
grind_test_self_proto : bin/test_self_proto
	valgrind -q --error-exitcode=1 --leak-check=full $^ 1>/dev/null

# self.c
bin/pgenc_proto : source/pgenc/main.c \
	tmp/pgenc/self_proto.c \
	lib/libpgenc.a \
	lib/libselc.a
	$(CC) $(CFLAGS) -o $@ $^ -lssl
tmp/pgenc/self.c : bin/pgenc_proto grammar/self.g
	bin/pgenc_proto -g grammar/self.g -s $@ -d pgc_self
build/pgenc/self.o : tmp/pgenc/self.c include/pgenc/self.h
	$(CC) $(CFLAGS) -c -o $@ $<
bin/test_self : tests/pgenc/self.c \
	build/pgenc/self.o \
	lib/libpgenc.a \
	lib/libselc.a
	$(CC) $(CFLAGS) -o $@ $^ -lssl
grind_test_self: bin/test_self
	valgrind -q --error-exitcode=1 --leak-check=full $^ 1>/dev/null

# executable
bin/pgenc : source/pgenc/main.c \
	tmp/pgenc/self.c \
	lib/libpgenc.a \
	lib/libselc.a
	$(CC) $(CFLAGS) -o $@ $^ -lssl
tmp/pgenc/self_exec.c : bin/pgenc grammar/self.g
	bin/pgenc -g grammar/self.g -s $@ -d pgc_self
bin/test_exec : tests/pgenc/self.c \
	tmp/pgenc/self_exec.c \
	lib/libpgenc.a \
	lib/libselc.a
	$(CC) $(CFLAGS) -o $@ $^ -lssl
grind_test_exec: bin/test_exec
	valgrind -q --error-exitcode=1 --leak-check=full $^ 1>/dev/null

# test suite
suite: \
	grind_test_charset \
	grind_test_buffer \
	grind_test_parser \
	grind_test_stack \
	grind_test_table \
	grind_test_ast \
	grind_test_syntax  \
	grind_test_lang \
	grind_test_self_proto \
	grind_test_self \
	grind_test_exec

clean :
	rm build/pgenc/*.o || true 
	rm bin/test_* || true 
	rm tmp/pgenc/*.c || true
	rm bin/pgenc || true
	rm bin/pgenc_dummy || true
	rm bin/pgenc_proto || true 
	rm lib/libpgenc.a || true 
	rm lib/libpgenc_pristine.a || true 
	rm include/selc || true 
	rm lib/libselc.a || true

