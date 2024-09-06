
#include "pgenc/lang.h"
#include "pgenc/error.h"
#include <stdlib.h>
#include <string.h>

enum pgc_err test_parse(
        struct pgc_par *par, 
        char *str, 
        struct pgc_ast_lst **syn)
{
        static char allocbytes[1024];

        const size_t len = strlen(str);
        struct pgc_buf buf;
        pgc_buf_init(&buf, str, len, len);

        struct pgc_stk alloc;
        pgc_stk_init(&alloc, 1024, allocbytes);

        return pgc_lang_parse(par, &buf, &alloc,  syn);
}

void test_readnothing()
{
        puts("   test_readnothing()...");
        struct pgc_par c;
        struct pgc_ast_lst *syn;
        pgc_par_byte(&c, 'c');
        PGC_TEST(test_parse(&c, "cab", &syn) == PGC_ERR_OK);
        PGC_TEST(pgc_ast_len(syn) == 0);
}

void test_capchar()
{
        puts("   test_capchar()...");
        struct pgc_par c, hook;
        pgc_par_byte(&c, 'c');
        pgc_par_call(&hook, pgc_lang_capchar, &c);
        struct pgc_ast_lst *lst;
        PGC_TEST(test_parse(&hook, "cab", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_ast_len(lst) == 1);
        PGC_TEST(pgc_ast_typeof(lst->val) == PGC_AST_INT8);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_CHAR);
        PGC_TEST(pgc_ast_toint8(lst->val) == 'c');
}

void test_capid()
{
        puts("   test_capid()...");
        struct pgc_par cat, call;
        pgc_par_cmp(&cat, "cat", 3);
        pgc_par_call(&call, pgc_lang_capid, &cat);
        struct pgc_ast_lst *lst;
        PGC_TEST(test_parse(&call, "catty", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_ast_len(lst) == 1);
        PGC_TEST(pgc_ast_typeof(lst->val) == PGC_AST_STR);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_ID);
        PGC_TEST(!strcmp(pgc_ast_tostr(lst->val), "cat"));
        PGC_TEST(!strcmp("cat", pgc_ast_tostr(lst->val)));
}

void test_capnum()
{
        puts("   test_capnum()...");
        struct pgc_par num, hook;
        pgc_par_cmp(&num, "123", 3);
        pgc_par_call(&hook, pgc_lang_capnum, &num);
        struct pgc_ast_lst *lst;
        PGC_TEST(test_parse(&hook, "123abc", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_ast_len(lst) == 1);
        PGC_TEST(pgc_ast_typeof(lst->val) == PGC_AST_UINT32);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_NUM);
        PGC_TEST(pgc_ast_touint32(lst->val) == 123);
}

void test_capxbyte()
{
        puts("   test_capxbyte()...");
        struct pgc_par num, hook;
        pgc_par_cmp(&num, "1A", 2);
        pgc_par_call(&hook, pgc_lang_capxbyte, &num);
        struct pgc_ast_lst *lst;
        PGC_TEST(test_parse(&hook, "1A  ", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_ast_len(lst) == 1);
        PGC_TEST(pgc_ast_typeof(lst->val) == PGC_AST_INT8);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_CHAR);
        PGC_TEST(pgc_ast_toint8(lst->val) == 0x1A);
}

void test_caputf8()
{
        puts("   test_caputf8()...");
        struct pgc_par num, hook;
        pgc_par_cmp(&num, "abc", 3);
        pgc_par_call(&hook, pgc_lang_caputf, &num);
        struct pgc_ast_lst *lst;
        PGC_TEST(test_parse(&hook, "abc  ", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_ast_len(lst) == 1);
        PGC_TEST(pgc_ast_typeof(lst->val) == PGC_AST_UINT32);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_NUM);
        PGC_TEST(pgc_ast_touint32(lst->val) == 0xABC);
}

int main(int argc, char **args)
{
        puts("running test_build...");
        test_readnothing();
        test_capchar();
        test_capid();
        test_capnum();
        test_capxbyte();
        test_caputf8();
}