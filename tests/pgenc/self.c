#include "pgenc/self.h"
#include "pgenc/lang.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

enum pgc_err pgc_tst_par(
        const struct pgc_par *parser, 
        char *string, 
        struct pgc_ast_lst **syntax)
{
        static uint8_t bytes[0xFFFF];
        struct pgc_stk alloc;
        pgc_stk_init(&alloc, 0xFFFF, bytes);
        struct pgc_buf buf;
        size_t len = strlen(string);
        pgc_buf_init(&buf, string, len, len);
        return pgc_lang_parse(parser, &buf, &alloc, syntax);
}

extern const struct pgc_par pgc_self_wsc;
void test_wsc()
{
        SEL_INFO();
        SEL_TEST(pgc_par_runs(&pgc_self_wsc, " ", NULL) == 1);
        SEL_TEST(pgc_par_runs(&pgc_self_wsc, "\t", NULL) == 1);
        SEL_TEST(pgc_par_runs(&pgc_self_wsc, "\r", NULL) == 1);
        SEL_TEST(pgc_par_runs(&pgc_self_wsc, "\n", NULL) == 1);
        SEL_TEST(pgc_par_runs(&pgc_self_wsc, "b", NULL) == PGC_ERR_CMP);
}

extern const struct pgc_par pgc_self_ws;
void test_ws()
{
        SEL_INFO();
        SEL_TEST(pgc_par_runs(&pgc_self_ws, "  \t\ns", NULL) == 4);
        SEL_TEST(pgc_par_runs(&pgc_self_ws, "b", NULL) == PGC_ERR_OK);
        SEL_TEST(pgc_par_runs(&pgc_self_ws, "", NULL) == PGC_ERR_OK);
}

extern const struct pgc_par pgc_self_alpha;
void test_alpha()
{
        SEL_INFO();
        SEL_TEST(pgc_par_runs(&pgc_self_alpha, "a", NULL) == 1);
        SEL_TEST(pgc_par_runs(&pgc_self_alpha, "A", NULL) == 1);
        SEL_TEST(pgc_par_runs(&pgc_self_alpha, "1", NULL) == PGC_ERR_CMP);
        SEL_TEST(pgc_par_runs(&pgc_self_alpha, "", NULL) == PGC_ERR_OOB);
}

extern const struct pgc_par pgc_self_idc;
void test_idc()
{
        SEL_INFO();
        SEL_TEST(pgc_par_runs(&pgc_self_idc, "a", NULL) == 1);
        SEL_TEST(pgc_par_runs(&pgc_self_idc, "A", NULL) == 1);
        SEL_TEST(pgc_par_runs(&pgc_self_idc, "z", NULL) == 1);
        SEL_TEST(pgc_par_runs(&pgc_self_idc, "Z", NULL) == 1);
        SEL_TEST(pgc_par_runs(&pgc_self_idc, "_", NULL) == 1);
}

extern const struct pgc_par pgc_self_id;
void test_id()
{
        SEL_INFO();
        struct pgc_ast_lst *lst = NULL;
        SEL_TEST(pgc_tst_par(&pgc_self_id, "x_z;", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_ast_len(lst) == 1);
        SEL_TEST(pgc_ast_typeof(lst->val) == PGC_AST_STR);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_ID);
        SEL_TEST(!strcmp(pgc_ast_tostr(lst->val), "x_z"));
}

extern const struct pgc_par pgc_self_xdigit;
void test_xdigit()
{
        SEL_INFO();
        struct pgc_ast_lst *lst = NULL;
        SEL_TEST(pgc_tst_par(&pgc_self_xdigit, "0;", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_tst_par(&pgc_self_xdigit, "9;", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_tst_par(&pgc_self_xdigit, "a;", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_tst_par(&pgc_self_xdigit, "f;", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_tst_par(&pgc_self_xdigit, "A;", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_tst_par(&pgc_self_xdigit, "F;", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_tst_par(&pgc_self_xdigit, "g;", &lst) == PGC_ERR_CMP);
        SEL_TEST(pgc_tst_par(&pgc_self_xdigit, "G;", &lst) == PGC_ERR_CMP);
}

extern const struct pgc_par pgc_self_xdigits;
void test_xdigits()
{
        SEL_INFO();
        SEL_TEST(pgc_par_runs(&pgc_self_xdigits, "0a", NULL) == 2);
        SEL_TEST(pgc_par_runs(&pgc_self_xdigits, "99", NULL) == 2);
        SEL_TEST(pgc_par_runs(&pgc_self_xdigits, "Ag", NULL) == 1);
        SEL_TEST(pgc_par_runs(&pgc_self_xdigits, "AF", NULL) == 2);
        SEL_TEST(pgc_par_runs(&pgc_self_xdigits, "_", NULL) == PGC_ERR_CMP);
}

extern const struct pgc_par pgc_self_xbyte;
void test_xbyte()
{
        SEL_INFO();
        struct pgc_ast_lst *lst = NULL;
        SEL_TEST(pgc_tst_par(&pgc_self_xbyte, "0a;;", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_ast_typeof(lst->val) == PGC_AST_INT8);
        SEL_TEST(pgc_ast_toint8(lst->val) == 0x0A);
        SEL_TEST(pgc_tst_par(&pgc_self_xbyte, "7f;;", &lst) == PGC_ERR_OK); 
        SEL_TEST(pgc_ast_typeof(lst->val) == PGC_AST_INT8);
        SEL_TEST(pgc_ast_toint8(lst->val) == 0x7F);
        SEL_TEST(pgc_tst_par(&pgc_self_xbyte, "b;;", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_ast_typeof(lst->val) == PGC_AST_INT8);
        SEL_TEST(pgc_ast_toint8(lst->val) == 11);
        SEL_TEST(pgc_tst_par(&pgc_self_xbyte, "0;;", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_ast_typeof(lst->val) == PGC_AST_INT8);
        SEL_TEST(pgc_ast_toint8(lst->val) == 0);
        SEL_TEST(pgc_tst_par(&pgc_self_xbyte, "G;;", &lst) == PGC_ERR_CMP);
}

extern const struct pgc_par pgc_self_pctbyte;
void test_pctbyte()
{
        SEL_INFO();
        struct pgc_ast_lst *lst = NULL;
        SEL_TEST(pgc_tst_par(&pgc_self_pctbyte, "%0a;;", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_ast_typeof(lst->val) == PGC_AST_INT8);
        SEL_TEST(pgc_ast_toint8(lst->val) == 0x0A);
        SEL_TEST(pgc_tst_par(&pgc_self_pctbyte, "%7f;;", &lst) == PGC_ERR_OK); 
        SEL_TEST(pgc_ast_typeof(lst->val) == PGC_AST_INT8);
        SEL_TEST(pgc_ast_toint8(lst->val) == 0x7F);
        SEL_TEST(pgc_tst_par(&pgc_self_pctbyte, "%b;;", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_ast_typeof(lst->val) == PGC_AST_INT8);
        SEL_TEST(pgc_ast_toint8(lst->val) == 11);
        SEL_TEST(pgc_tst_par(&pgc_self_pctbyte, "%0;;", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_ast_typeof(lst->val) == PGC_AST_INT8);
        SEL_TEST(pgc_ast_toint8(lst->val) == 0);
        SEL_TEST(pgc_tst_par(&pgc_self_pctbyte, "%G;;", &lst) == PGC_ERR_CMP);
}

extern const struct pgc_par pgc_self_charlit;
void test_charlit()
{
        SEL_INFO();
        struct pgc_ast_lst *lst = NULL;
        SEL_TEST(pgc_tst_par(&pgc_self_charlit, "'a'", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_ast_typeof(lst->val) == PGC_AST_INT8);
        SEL_TEST(pgc_ast_toint8(lst->val) == 'a'); 
}

extern const struct pgc_par pgc_self_digits;
void test_digits()
{
        SEL_INFO();
        SEL_TEST(pgc_par_runs(&pgc_self_digits, "0a", NULL) == 1);
        SEL_TEST(pgc_par_runs(&pgc_self_digits, "99", NULL) == 2);
        SEL_TEST(pgc_par_runs(&pgc_self_digits, "01", NULL) == 2);
        SEL_TEST(pgc_par_runs(&pgc_self_digits, "_", NULL) == PGC_ERR_CMP);
}

extern const struct pgc_par pgc_self_num;
void test_num()
{
        SEL_INFO();
        struct pgc_ast_lst *lst = NULL;
        SEL_TEST(pgc_tst_par(&pgc_self_num, "3a", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_NUM);
        SEL_TEST(pgc_ast_touint32(lst->val) == 3);
        SEL_TEST(pgc_tst_par(&pgc_self_num, "37a", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_NUM);
        SEL_TEST(pgc_ast_touint32(lst->val) == 37);
}

extern const struct pgc_par pgc_self_range;
void test_range()
{
        SEL_INFO();
        struct pgc_ast_lst *lst = NULL;
        SEL_TEST(pgc_tst_par(&pgc_self_range, "12_34a", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_ast_len(lst) == 1);
        SEL_TEST(pgc_ast_typeof(lst->val) == PGC_AST_LST);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_RANGE);
        lst = pgc_ast_tolst(lst->val);
        SEL_TEST(pgc_ast_len(lst) == 2);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_NUM);
        SEL_TEST(pgc_ast_touint32(lst->val) == 12);
        lst = lst->nxt;
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_NUM);
        SEL_TEST(pgc_ast_touint32(lst->val) == 34);
}

extern const struct pgc_par pgc_self_utfvalue;
void test_utfvalue()
{
        SEL_INFO();
        struct pgc_ast_lst *lst = NULL;
        SEL_TEST(pgc_tst_par(&pgc_self_utfvalue, "123", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_NUM);
        SEL_TEST(pgc_ast_touint32(lst->val) == 0x123);
}

extern const struct pgc_par pgc_self_utfrange;
void test_utfrange()
{
        SEL_INFO();
        struct pgc_ast_lst *lst = NULL;
        SEL_TEST(pgc_tst_par(
                &pgc_self_utfrange, "&12_34a", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_ast_len(lst) == 1);
        SEL_TEST(pgc_ast_typeof(lst->val) == PGC_AST_LST);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_UTF);
        lst = pgc_ast_tolst(lst->val);
        SEL_TEST(pgc_ast_len(lst) == 2);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_NUM);
        SEL_TEST(pgc_ast_touint32(lst->val) == 0x12);
        lst = lst->nxt;
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_NUM);
        SEL_TEST(pgc_ast_touint32(lst->val) == 0x34A);
}

extern const struct pgc_par pgc_self_setexp;
void test_setexp()
{
        SEL_INFO();
        struct pgc_ast_lst *lst = NULL;
        SEL_TEST(pgc_tst_par(&pgc_self_setexp, "'a'", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_CHAR);
        SEL_TEST(pgc_tst_par(
                &pgc_self_setexp, "'a' + %1a", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_UNION);
        lst = pgc_ast_tolst(lst->val);
        SEL_TEST(pgc_ast_len(lst) == 2);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_CHAR);
        SEL_TEST(pgc_ast_toint8(lst->val) == 'a');
        lst = lst->nxt;
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_CHAR);
        SEL_TEST(pgc_ast_toint8(lst->val) == 0x1A);
}

extern const struct pgc_par pgc_self_exp;
void test_exp()
{
        SEL_INFO();
        struct pgc_ast_lst *lst = NULL;
        SEL_TEST(pgc_tst_par(&pgc_self_exp, "'a'", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_CHAR);
        SEL_TEST(pgc_tst_par(&pgc_self_exp, "'a' %1a", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_AND);
        lst = pgc_ast_tolst(lst->val);
        SEL_TEST(pgc_ast_len(lst) == 2);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_CHAR);
        SEL_TEST(pgc_ast_toint8(lst->val) == 'a');
        lst = lst->nxt;
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_CHAR);
        SEL_TEST(pgc_ast_toint8(lst->val) == 0x1A);
}

extern const struct pgc_par pgc_self_stmt;
void test_dec()
{
        SEL_INFO();
        struct pgc_ast_lst *lst = NULL;
        SEL_TEST(pgc_tst_par(&pgc_self_stmt, "dec hi;", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_ast_typeof(lst->val) == PGC_AST_LST);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_DEC);
        lst = pgc_ast_tolst(lst->val);
        SEL_TEST(pgc_ast_len(lst) == 1);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_ID);
}

void test_def()
{
        SEL_INFO();
        struct pgc_ast_lst *lst = NULL;
        SEL_TEST(pgc_tst_par(
                &pgc_self_stmt, "def hi = 'a';", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_ast_typeof(lst->val) == PGC_AST_LST);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_DEF);
        lst = pgc_ast_tolst(lst->val);
        SEL_TEST(pgc_ast_len(lst) == 2);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_ID);
}

void test_set()
{
        SEL_INFO();
        struct pgc_ast_lst *lst = NULL;
        SEL_TEST(pgc_tst_par(
                &pgc_self_stmt, "set hi = 'a';", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_ast_typeof(lst->val) == PGC_AST_LST);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_SET);
        lst = pgc_ast_tolst(lst->val);
        SEL_TEST(pgc_ast_len(lst) == 2);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_ID);
}

void test_let()
{
        SEL_INFO();
        struct pgc_ast_lst *lst = NULL;
        SEL_TEST(pgc_tst_par(
                &pgc_self_stmt, "let hi = 'a';", &lst) == PGC_ERR_OK);
        SEL_TEST(pgc_ast_typeof(lst->val) == PGC_AST_LST);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_LET);
        lst = pgc_ast_tolst(lst->val);
        SEL_TEST(pgc_ast_len(lst) == 2);
        SEL_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_ID);
}

int main(int argc, char **argv)
{
        SEL_INFO();
        test_wsc();
        test_ws();
        test_alpha();
        test_idc();
        test_id();
        test_xdigit();
        test_xdigits();
        test_xbyte();
        test_pctbyte();
        test_charlit();
        test_digits();
        test_num();
        test_range();
        test_utfvalue();
        test_utfrange();
        test_setexp();
        test_exp();
        test_dec();
        test_def();
        test_set();
        test_let();
}