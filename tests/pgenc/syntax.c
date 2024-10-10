#define _POSIX_C_SOURCE 200809L

#include "pgenc/syntax.h"
#include "pgenc/error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_newid()
{
        puts("   test_newid()...");
        char hello[] = "hello";
        struct pgc_ast *n = pgc_syn_newid(hello);
        SEL_TEST(n);
        SEL_TEST(pgc_ast_typeof(n) == PGC_AST_STR);
        SEL_TEST(pgc_syn_typeof(n) == PGC_SYN_ID);
        SEL_TEST(pgc_ast_tostr(n) == hello);
        pgc_syn_free(n);
}

void test_newlit()
{
        puts("   test_newlit()...");
        char hello[] = "hello";
        struct pgc_ast *n = pgc_syn_newlit(hello);
        SEL_TEST(n);
        SEL_TEST(pgc_ast_typeof(n) == PGC_AST_STR);
        SEL_TEST(pgc_syn_typeof(n) == PGC_SYN_LIT);
        SEL_TEST(pgc_ast_tostr(n) == hello);
        pgc_syn_free(n);
}

void test_newhook()
{
        puts("   test_newhook()...");
        char hello[] = "hello";
        struct pgc_ast *n = pgc_syn_newhook(hello);
        SEL_TEST(n);
        SEL_TEST(pgc_ast_typeof(n) == PGC_AST_STR);
        SEL_TEST(pgc_syn_typeof(n) == PGC_SYN_HOOK);
        SEL_TEST(pgc_ast_tostr(n) == hello);
        pgc_syn_free(n);
}

void test_newnum()
{
        puts("   test_newnum()...");
        struct pgc_ast *n = pgc_syn_newnum(0xBBAAFF);
        SEL_TEST(n);
        SEL_TEST(pgc_ast_typeof(n) == PGC_AST_UINT32);
        SEL_TEST(pgc_syn_typeof(n) == PGC_SYN_NUM);
        SEL_TEST(pgc_ast_touint32(n) == 0xBBAAFF);
        pgc_syn_free(n);
}

void test_newchar()
{
        puts("   test_newchar()...");
        struct pgc_ast *n = pgc_syn_newchar('x');
        SEL_TEST(n);
        SEL_TEST(pgc_ast_typeof(n) == PGC_AST_INT8);
        SEL_TEST(pgc_syn_typeof(n) == PGC_SYN_CHAR);
        SEL_TEST(pgc_ast_toint8(n) == 'x');
        pgc_syn_free(n);
}

void test_newunion()
{
        puts("   test_newunion()...");
        struct pgc_ast *x = pgc_syn_newchar('x');
        struct pgc_ast *y = pgc_syn_newchar('y');
        struct pgc_ast *u = pgc_syn_newunion(x, y);
        SEL_TEST(x && y && u);
        SEL_TEST(pgc_ast_typeof(u) == PGC_AST_LST);
        SEL_TEST(pgc_syn_typeof(u) == PGC_SYN_UNION);
        SEL_TEST(pgc_ast_toint8(pgc_syn_getfst(u)) == 'x');
        SEL_TEST(pgc_ast_toint8(pgc_syn_getsnd(u)) == 'y');
        pgc_syn_free(u);
}

void test_newdiff()
{
        puts("   test_newdiff()...");
        struct pgc_ast *x = pgc_syn_newchar('x');
        struct pgc_ast *y = pgc_syn_newchar('y');
        struct pgc_ast *u = pgc_syn_newdiff(x, y);
        SEL_TEST(x && y && u);
        SEL_TEST(pgc_ast_typeof(u) == PGC_AST_LST);
        SEL_TEST(pgc_syn_typeof(u) == PGC_SYN_DIFF);
        SEL_TEST(pgc_ast_toint8(pgc_syn_getfst(u)) == 'x');
        SEL_TEST(pgc_ast_toint8(pgc_syn_getsnd(u)) == 'y');
        pgc_syn_free(u);
}

void test_newutf()
{
        puts("   test_newutf()...");
        struct pgc_ast *x = pgc_syn_newnum(0xBADBEE);
        struct pgc_ast *y = pgc_syn_newnum(0xFABDAB);
        struct pgc_ast *u = pgc_syn_newutf(x, y);
        SEL_TEST(x && y && u);
        SEL_TEST(pgc_ast_typeof(u) == PGC_AST_LST);
        SEL_TEST(pgc_syn_typeof(u) == PGC_SYN_UTF);
        SEL_TEST(pgc_ast_touint32(pgc_syn_getfst(u)) == 0xBADBEE);
        SEL_TEST(pgc_ast_touint32(pgc_syn_getsnd(u)) == 0xFABDAB);
        pgc_syn_free(u);
}

void test_newand()
{
        puts("   test_newand()...");
        struct pgc_ast *x = pgc_syn_newnum(0xBADBEE);
        struct pgc_ast *y = pgc_syn_newnum(0xFABDAB);
        struct pgc_ast *u = pgc_syn_newand(x, y);
        SEL_TEST(x && y && u);
        SEL_TEST(pgc_ast_typeof(u) == PGC_AST_LST);
        SEL_TEST(pgc_syn_typeof(u) == PGC_SYN_AND);
        SEL_TEST(pgc_ast_touint32(pgc_syn_getfst(u)) == 0xBADBEE);
        SEL_TEST(pgc_ast_touint32(pgc_syn_getsnd(u)) == 0xFABDAB);
        pgc_syn_free(u);
}

void test_newor()
{
        puts("   test_newor()...");
        struct pgc_ast *x = pgc_syn_newnum(0xBADBEE);
        struct pgc_ast *y = pgc_syn_newnum(0xFABDAB);
        struct pgc_ast *u = pgc_syn_newor(x, y);
        SEL_TEST(x && y && u);
        SEL_TEST(pgc_ast_typeof(u) == PGC_AST_LST);
        SEL_TEST(pgc_syn_typeof(u) == PGC_SYN_OR);
        SEL_TEST(pgc_ast_touint32(pgc_syn_getfst(u)) == 0xBADBEE);
        SEL_TEST(pgc_ast_touint32(pgc_syn_getsnd(u)) == 0xFABDAB);
        pgc_syn_free(u);
}

void test_newrange()
{
        puts("   test_newrange()...");
        struct pgc_ast *x = pgc_syn_newnum(0xBADBEE);
        struct pgc_ast *y = pgc_syn_newnum(0xFABDAB);
        struct pgc_ast *u = pgc_syn_newrange(x, y);
        SEL_TEST(x && y && u);
        SEL_TEST(pgc_ast_typeof(u) == PGC_AST_LST);
        SEL_TEST(pgc_syn_typeof(u) == PGC_SYN_RANGE);
        SEL_TEST(pgc_ast_touint32(pgc_syn_getfst(u)) == 0xBADBEE);
        SEL_TEST(pgc_ast_touint32(pgc_syn_getsnd(u)) == 0xFABDAB);
        pgc_syn_free(u);
}

void test_newrep()
{
        puts("   test_newrep()...");
        struct pgc_ast *min = pgc_syn_newnum(0xBADBEE);
        struct pgc_ast *max = pgc_syn_newnum(0xFABDAB);
        struct pgc_ast *range = pgc_syn_newrange(min, max);
        struct pgc_ast *exp = pgc_syn_newchar('a');
        struct pgc_ast *rep = pgc_syn_newrep(range, exp);
        SEL_TEST(min && max && range && exp && rep);
        SEL_TEST(pgc_syn_getrange(rep) == range);
        SEL_TEST(pgc_syn_getsubex(rep) == exp);
        SEL_TEST(pgc_ast_typeof(rep) == PGC_AST_LST);
        SEL_TEST(pgc_syn_typeof(rep) == PGC_SYN_REP);
        pgc_syn_free(rep);
}

void test_newset()
{
        puts("   test_newset()...");
        struct pgc_ast *name = pgc_syn_newid("cat");
        struct pgc_ast *exp = pgc_syn_newchar('c');
        struct pgc_ast *stmt = pgc_syn_newset(name, exp);
        SEL_TEST(name && exp && stmt);
        SEL_TEST(pgc_syn_getname(stmt) == name);
        SEL_TEST(pgc_syn_getexpr(stmt) == exp);
        SEL_TEST(pgc_ast_typeof(stmt) == PGC_AST_LST);
        SEL_TEST(pgc_syn_typeof(stmt) == PGC_SYN_SET);
        pgc_syn_free(stmt);
}

void test_newdef()
{
        puts("   test_newdef()...");
        struct pgc_ast *name = pgc_syn_newid("cat");
        struct pgc_ast *exp = pgc_syn_newchar('c');
        struct pgc_ast *stmt = pgc_syn_newdef(name, exp);
        SEL_TEST(name && exp && stmt);
        SEL_TEST(pgc_syn_getname(stmt) == name);
        SEL_TEST(pgc_syn_getexpr(stmt) == exp);
        SEL_TEST(pgc_ast_typeof(stmt) == PGC_AST_LST);
        SEL_TEST(pgc_syn_typeof(stmt) == PGC_SYN_DEF);
        pgc_syn_free(stmt);
}

void test_newlet()
{
        puts("   test_newlet()...");
        struct pgc_ast *name = pgc_syn_newid("cat");
        struct pgc_ast *exp = pgc_syn_newchar('c');
        struct pgc_ast *stmt = pgc_syn_newlet(name, exp);
        SEL_TEST(name && exp && stmt);
        SEL_TEST(pgc_syn_getname(stmt) == name);
        SEL_TEST(pgc_syn_getexpr(stmt) == exp);
        SEL_TEST(pgc_ast_typeof(stmt) == PGC_AST_LST);
        SEL_TEST(pgc_syn_typeof(stmt) == PGC_SYN_LET);
        pgc_syn_free(stmt);
}

void test_newdec()
{
        puts("   test_newdec()...");
        struct pgc_ast *name = pgc_syn_newid("cat");
        struct pgc_ast *stmt = pgc_syn_newdec(name);
        SEL_TEST(name && stmt);
        SEL_TEST(pgc_syn_getname(stmt) == name);
        SEL_TEST(pgc_ast_typeof(stmt) == PGC_AST_LST);
        SEL_TEST(pgc_syn_typeof(stmt) == PGC_SYN_DEC);
        pgc_syn_free(stmt);
}

void test_fprintid()
{
        puts("   test_fprintid()...");
        char *buf;
        size_t len;
        FILE *file = open_memstream(&buf, &len);
        struct pgc_ast *syn = pgc_syn_newid("cat");
        SEL_TEST(pgc_syn_fprint(file, syn) >= 0);
        fflush(file);
        SEL_TEST(!strcmp(buf, "cat"));
        fclose(file);
        free(buf);
        pgc_syn_free(syn);
}

void test_fprintnum()
{
        puts("   test_fprintnum()...");
        char *buf;
        size_t len;
        FILE *file = open_memstream(&buf, &len);
        struct pgc_ast *syn = pgc_syn_newnum(12345);
        SEL_TEST(pgc_syn_fprint(file, syn) >= 0);
        fflush(file);
        SEL_TEST(!strcmp(buf, "12345"));
        fclose(file);
        free(buf);
        pgc_syn_free(syn);
}

void test_fprintbyte()
{
        puts("   test_fprintbyte()...");
        char *buf;
        size_t len;
        FILE *file = open_memstream(&buf, &len);
        struct pgc_ast *syn = pgc_syn_newchar( 0x1A);
        SEL_TEST(pgc_syn_fprint(file, syn) >= 0);
        fflush(file);
        SEL_TEST(!strcmp(buf, "%1A"));
        fclose(file);
        free(buf);
        pgc_syn_free(syn);
}

void test_fprintlit()
{
        puts("   test_fprintlit()...");
        char *buf;
        size_t len;
        FILE *file = open_memstream(&buf, &len);
        struct pgc_ast *syn = pgc_syn_newlit("set");
        SEL_TEST(pgc_syn_fprint(file, syn) >= 0);
        fflush(file);
        SEL_TEST(!strcmp(buf, "\"set\""));
        fclose(file);
        free(buf);
        pgc_syn_free(syn);
}

void test_fprinthook()
{
        puts("   test_fprinthook()...");
        char *buf;
        size_t len;
        FILE *file = open_memstream(&buf, &len);
        struct pgc_ast *syn = pgc_syn_newhook("set");
        SEL_TEST(pgc_syn_fprint(file, syn) >= 0);
        fflush(file);
        SEL_TEST(!strcmp(buf, "@set"));
        fclose(file);
        free(buf);
        pgc_syn_free(syn);
}

void test_fprintutf()
{
        puts("   test_fprintutf()...");
        char *buf;
        size_t len;
        FILE *file = open_memstream(&buf, &len);
        struct pgc_ast *min = pgc_syn_newnum(0xABC);
        struct pgc_ast *max = pgc_syn_newnum(0xDEF);
        struct pgc_ast *syn = pgc_syn_newutf(min, max);
        SEL_TEST(pgc_syn_fprint(file, syn) >= 0);
        fflush(file);
        SEL_TEST(!strcmp(buf, "&ABC_DEF"));
        fclose(file);
        free(buf);
        pgc_syn_free(syn);
}

void test_fprintrange()
{
        puts("   test_fprintrange()...");
        char *buf;
        size_t len;
        FILE *file = open_memstream(&buf, &len);
        struct pgc_ast *min = pgc_syn_newnum(123);
        struct pgc_ast *max = pgc_syn_newnum(456);
        struct pgc_ast *syn = pgc_syn_newrange(min, max);
        SEL_TEST(pgc_syn_fprint(file, syn) >= 0);
        fflush(file);
        SEL_TEST(!strcmp(buf, "123_456"));
        fclose(file);
        free(buf);
        pgc_syn_free(syn);
}

void test_fprintunion()
{
        puts("   test_fprintunion()...");
        char *buf;
        size_t len;
        FILE *file = open_memstream(&buf, &len);
        struct pgc_ast *fst = pgc_syn_newnum(1);
        struct pgc_ast *snd = pgc_syn_newnum(2);
        struct pgc_ast *syn = pgc_syn_newunion(fst, snd);
        SEL_TEST(pgc_syn_fprint(file, syn) >= 0);
        fflush(file);
        SEL_TEST(!strcmp(buf, "(1 + 2)"));
        fclose(file);
        free(buf);
        pgc_syn_free(syn);
}

void test_fprintdiff()
{
        puts("   test_fprintdiff()...");
        char *buf;
        size_t len;
        FILE *file = open_memstream(&buf, &len);
        struct pgc_ast *fst = pgc_syn_newnum(1);
        struct pgc_ast *snd = pgc_syn_newnum(2);
        struct pgc_ast *syn = pgc_syn_newdiff(fst, snd);
        SEL_TEST(pgc_syn_fprint(file, syn) >= 0);
        fflush(file);
        SEL_TEST(!strcmp(buf, "(1 - 2)"));
        fclose(file);
        free(buf);
        pgc_syn_free(syn);
}

void test_fprintand()
{
        puts("   test_fprintand()...");
        char *buf;
        size_t len;
        FILE *file = open_memstream(&buf, &len);
        struct pgc_ast *fst = pgc_syn_newnum(1);
        struct pgc_ast *snd = pgc_syn_newnum(2);
        struct pgc_ast *syn = pgc_syn_newand(fst, snd);
        SEL_TEST(pgc_syn_fprint(file, syn) >= 0);
        fflush(file);
        SEL_TEST(!strcmp(buf, "(1 2)"));
        fclose(file);
        free(buf);
        pgc_syn_free(syn);
}

void test_fprintor()
{
        puts("   test_fprintor()...");
        char *buf;
        size_t len;
        FILE *file = open_memstream(&buf, &len);
        struct pgc_ast *fst = pgc_syn_newnum(1);
        struct pgc_ast *snd = pgc_syn_newnum(2);
        struct pgc_ast *syn = pgc_syn_newor(fst, snd);
        SEL_TEST(pgc_syn_fprint(file, syn) >= 0);
        fflush(file);
        SEL_TEST(!strcmp(buf, "(1 | 2)"));
        fclose(file);
        free(buf);
        pgc_syn_free(syn);
}

void test_fprintrep()
{
        puts("   test_fprintrep()...");
        char *buf;
        size_t len;
        FILE *file = open_memstream(&buf, &len);
        struct pgc_ast *fst = pgc_syn_newnum(1);
        struct pgc_ast *snd = pgc_syn_newnum(2);
        struct pgc_ast *range = pgc_syn_newrange(fst, snd);
        struct pgc_ast *subex = pgc_syn_newlit("a");
        struct pgc_ast *syn = pgc_syn_newrep(range, subex);
        SEL_TEST(pgc_syn_fprint(file, syn) >= 0);
        fflush(file);
        SEL_TEST(!strcmp(buf, "1_2\"a\""));
        fclose(file);
        free(buf);
        pgc_syn_free(syn);
}

void test_fprintset()
{
        puts("   test_fprintset()...");
        char *buf;
        size_t len;
        FILE *file = open_memstream(&buf, &len);
        struct pgc_ast *name = pgc_syn_newid("alphas");
        struct pgc_ast *expr = pgc_syn_newnum(1);
        struct pgc_ast *syn = pgc_syn_newset(name, expr);
        SEL_TEST(pgc_syn_fprint(file, syn) >= 0);
        fflush(file);
        SEL_TEST(!strcmp(buf, "set alphas = 1;"));
        fclose(file);
        free(buf);
        pgc_syn_free(syn);
}

void test_fprintlet()
{
        puts("   test_fprintlet()...");
        char *buf;
        size_t len;
        FILE *file = open_memstream(&buf, &len);
        struct pgc_ast *name = pgc_syn_newid("alphas");
        struct pgc_ast *expr = pgc_syn_newnum(1);
        struct pgc_ast *syn = pgc_syn_newlet(name, expr);
        SEL_TEST(pgc_syn_fprint(file, syn) >= 0);
        fflush(file);
        SEL_TEST(!strcmp(buf, "let alphas = 1;"));
        fclose(file);
        free(buf);
        pgc_syn_free(syn);
}

void test_fprintdef()
{
        puts("   test_fprintdef()...");
        char *buf;
        size_t len;
        FILE *file = open_memstream(&buf, &len);
        struct pgc_ast *name = pgc_syn_newid("alphas");
        struct pgc_ast *expr = pgc_syn_newnum(1);
        struct pgc_ast *syn = pgc_syn_newdef(name, expr);
        SEL_TEST(pgc_syn_fprint(file, syn) >= 0);
        fflush(file);
        SEL_TEST(!strcmp(buf, "def alphas = 1;"));
        fclose(file);
        free(buf);
        pgc_syn_free(syn);
}

void test_fprintdec()
{
        puts("   test_fprintdec()...");
        char *buf;
        size_t len;
        FILE *file = open_memstream(&buf, &len);
        struct pgc_ast *name = pgc_syn_newid("alphas");
        struct pgc_ast *syn = pgc_syn_newdec(name);
        SEL_TEST(pgc_syn_fprint(file, syn) >= 0);
        fflush(file);
        SEL_TEST(!strcmp(buf, "dec alphas;"));
        fclose(file);
        free(buf);
        pgc_syn_free(syn);
}

int main(int argc, char **args)
{
        test_newid();
        test_newlit();
        test_newhook();
        test_newnum();
        test_newchar();
        test_newunion();
        test_newdiff();
        test_newutf();
        test_newand();
        test_newor();
        test_newrange();
        test_newrep();
        test_newset();
        test_newdef();
        test_newlet();
        test_newdec();
        test_fprintid();
        test_fprintnum();
        test_fprintbyte();
        test_fprintlit();
        test_fprinthook();
        test_fprintutf();
        test_fprintrange();
        test_fprintunion();
        test_fprintdiff();
        test_fprintand();
        test_fprintor();
        test_fprintrep();
        test_fprintset();
        test_fprintlet();
        test_fprintdef();
        test_fprintdec();
        // test_self();
}