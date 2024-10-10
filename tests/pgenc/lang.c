
#include "pgenc/lang.h"
#include "pgenc/error.h"
#include <stdlib.h>
#include <string.h>

enum pgc_err test_parse(
        const struct pgc_par *par, 
        char *str, 
        struct pgc_ast_lst **syn)
{
        static char allocbytes[1024];

        const size_t len = strlen(str);
        struct pgc_buf buf;
        pgc_buf_init(&buf, str, len, len);

        struct pgc_stk alloc;
        pgc_stk_init(&alloc, allocbytes, 1024);

        return pgc_lang_parse(par, &buf, &alloc,  syn);
}

void test_readnothing()
{
        SEL_INFO();
        const struct pgc_par c = PGC_PAR_BYTE('c');
        struct pgc_ast_lst *syn;
        SEL_TEST(test_parse(&c, "cab", &syn) == PGC_ERR_OK);
        SEL_TEST(pgc_ast_len(syn) == 0);
}

void test_capchar()
{
        SEL_INFO();
        const struct pgc_par c = PGC_PAR_BYTE('c');
        const struct pgc_par hook = PGC_PAR_CALL(pgc_lang_capchar, &c);
        struct pgc_ast_lst *lst;
        SEL_TEST(test_parse(&hook, "cab", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_ast_len(lst) == 1);
        SEL_TEST(pgc_ast_typeof(lst->val) == PGC_AST_INT8);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_CHAR);
        SEL_TEST(pgc_ast_toint8(lst->val) == 'c');
}

void test_capid()
{
        SEL_INFO();
        const struct pgc_par cat = PGC_PAR_CMP("cat", 3);
        const struct pgc_par call = PGC_PAR_CALL(pgc_lang_capid, &cat);
        struct pgc_ast_lst *lst;
        SEL_TEST(test_parse(&call, "catty", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_ast_len(lst) == 1);
        SEL_TEST(pgc_ast_typeof(lst->val) == PGC_AST_STR);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_ID);
        SEL_TEST(!strcmp(pgc_ast_tostr(lst->val), "cat"));
        SEL_TEST(!strcmp("cat", pgc_ast_tostr(lst->val)));
}

void test_capnum()
{
        SEL_INFO();
        const struct pgc_par num = PGC_PAR_CMP("123", 3);
        const struct pgc_par hook = PGC_PAR_CALL(pgc_lang_capnum, &num);
        struct pgc_ast_lst *lst;
        SEL_TEST(test_parse(&hook, "123abc", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_ast_len(lst) == 1);
        SEL_TEST(pgc_ast_typeof(lst->val) == PGC_AST_UINT32);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_NUM);
        SEL_TEST(pgc_ast_touint32(lst->val) == 123);
}

void test_capxbyte()
{
        SEL_INFO();
        const struct pgc_par num = PGC_PAR_CMP("1A", 2);
        const struct pgc_par hook = PGC_PAR_CALL(pgc_lang_capxbyte, &num);
        struct pgc_ast_lst *lst;
        SEL_TEST(test_parse(&hook, "1A  ", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_ast_len(lst) == 1);
        SEL_TEST(pgc_ast_typeof(lst->val) == PGC_AST_INT8);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_CHAR);
        SEL_TEST(pgc_ast_toint8(lst->val) == 0x1A);
}

void test_caputf8()
{
        SEL_INFO();
        const struct pgc_par num = PGC_PAR_CMP("abc", 3);
        const struct pgc_par hook = PGC_PAR_CALL(pgc_lang_caputf, &num);
        struct pgc_ast_lst *lst;
        SEL_TEST(test_parse(&hook, "abc  ", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_ast_len(lst) == 1);
        SEL_TEST(pgc_ast_typeof(lst->val) == PGC_AST_UINT32);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_NUM);
        SEL_TEST(pgc_ast_touint32(lst->val) == 0xABC);
}

int main(int argc, char **args)
{
        SEL_INFO();
        test_readnothing();
        test_capchar();
        test_capid();
        test_capnum();
        test_capxbyte();
        test_caputf8();
}