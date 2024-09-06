#include "pgenc/self.h"
#include "pgenc/lang.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

struct pgc_par *pgc_self_wsc(struct pgc_self *x);
struct pgc_par *pgc_self_ws(struct pgc_self *x);
struct pgc_par *pgc_self_alpha(struct pgc_self *x);
struct pgc_par *pgc_self_idc(struct pgc_self *x);
struct pgc_par *pgc_self_id(struct pgc_self *x);
struct pgc_par *pgc_self_xdigit(struct pgc_self *x);
struct pgc_par *pgc_self_xdigits(struct pgc_self *x);
struct pgc_par *pgc_self_xbyte(struct pgc_self *x);
struct pgc_par *pgc_self_pctbyte(struct pgc_self *x);
struct pgc_par *pgc_self_charlit(struct pgc_self *x);
struct pgc_par *pgc_self_digits(struct pgc_self *x);
struct pgc_par *pgc_self_num(struct pgc_self *x);
struct pgc_par *pgc_self_range(struct pgc_self *x);
struct pgc_par *pgc_self_utfvalue(struct pgc_self *x);
struct pgc_par *pgc_self_utfrange(struct pgc_self *x);
struct pgc_par *pgc_self_setexp(struct pgc_self *x);
struct pgc_par *pgc_self_exp(struct pgc_self *x);
struct pgc_par *pgc_self_stmt(struct pgc_self *x);

enum pgc_err pgc_tst_par(
        struct pgc_par *parser, 
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

void test_wsc()
{
        PGC_INFO();
        struct pgc_self *self = export_pgc_self();
        struct pgc_par *wsc = pgc_self_wsc(self);
        PGC_TEST(pgc_par_runs(wsc, " ", NULL) == 1);
        PGC_TEST(pgc_par_runs(wsc, "\t", NULL) == 1);
        PGC_TEST(pgc_par_runs(wsc, "\r", NULL) == 1);
        PGC_TEST(pgc_par_runs(wsc, "\n", NULL) == 1);
        PGC_TEST(pgc_par_runs(wsc, "b", NULL) == PGC_ERR_CMP);
}

void test_ws()
{
        PGC_INFO();
        struct pgc_self *self = export_pgc_self();
        struct pgc_par *ws = pgc_self_ws(self);
        PGC_TEST(pgc_par_runs(ws, "  \t\ns", NULL) == 4);
        PGC_TEST(pgc_par_runs(ws, "b", NULL) == PGC_ERR_OK);
        PGC_TEST(pgc_par_runs(ws, "", NULL) == PGC_ERR_OK);
}

void test_alpha()
{
        PGC_INFO();
        struct pgc_self *self = export_pgc_self();
        struct pgc_par *as = pgc_self_alpha(self);
        PGC_TEST(pgc_par_runs(as, "a", NULL) == 1);
        PGC_TEST(pgc_par_runs(as, "A", NULL) == 1);
        PGC_TEST(pgc_par_runs(as, "1", NULL) == PGC_ERR_CMP);
        PGC_TEST(pgc_par_runs(as, "", NULL) == PGC_ERR_OOB);
}

void test_idc()
{
        PGC_INFO();
        struct pgc_self *self = export_pgc_self();
        struct pgc_par *idc = pgc_self_idc(self);
        PGC_TEST(pgc_par_runs(idc, "a", NULL) == 1);
        PGC_TEST(pgc_par_runs(idc, "A", NULL) == 1);
        PGC_TEST(pgc_par_runs(idc, "z", NULL) == 1);
        PGC_TEST(pgc_par_runs(idc, "Z", NULL) == 1);
        PGC_TEST(pgc_par_runs(idc, "_", NULL) == 1);
}

void test_id()
{
        PGC_INFO();
        struct pgc_self *self = export_pgc_self();
        struct pgc_par *id = pgc_self_id(self);
        struct pgc_ast_lst *lst = NULL;
        PGC_TEST(pgc_tst_par(id, "x_z;", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_ast_len(lst) == 1);
        PGC_TEST(pgc_ast_typeof(lst->val) == PGC_AST_STR);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_ID);
        PGC_TEST(!strcmp(pgc_ast_tostr(lst->val), "x_z"));
}

void test_xdigit()
{
        PGC_INFO();
        struct pgc_self *self = export_pgc_self();
        struct pgc_par *xdigit = pgc_self_xdigit(self);
        struct pgc_ast_lst *lst = NULL;
        PGC_TEST(pgc_tst_par(xdigit, "0;", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_tst_par(xdigit, "9;", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_tst_par(xdigit, "a;", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_tst_par(xdigit, "f;", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_tst_par(xdigit, "A;", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_tst_par(xdigit, "F;", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_tst_par(xdigit, "g;", &lst) == PGC_ERR_CMP);
        PGC_TEST(pgc_tst_par(xdigit, "G;", &lst) == PGC_ERR_CMP);
}

void test_xdigits()
{
        PGC_INFO();
        struct pgc_self *self = export_pgc_self();
        struct pgc_par *xdigits = pgc_self_xdigits(self);
        PGC_TEST(pgc_par_runs(xdigits, "0a", NULL) == 2);
        PGC_TEST(pgc_par_runs(xdigits, "99", NULL) == 2);
        PGC_TEST(pgc_par_runs(xdigits, "Ag", NULL) == 1);
        PGC_TEST(pgc_par_runs(xdigits, "AF", NULL) == 2);
        PGC_TEST(pgc_par_runs(xdigits, "_", NULL) == PGC_ERR_CMP);
}

void test_xbyte()
{
        PGC_INFO();
        struct pgc_self *self = export_pgc_self();
        struct pgc_par *xbyte = pgc_self_xbyte(self);
        struct pgc_ast_lst *lst = NULL;
        PGC_TEST(pgc_tst_par(xbyte, "0a;;", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_ast_typeof(lst->val) == PGC_AST_INT8);
        PGC_TEST(pgc_ast_toint8(lst->val) == 0x0A);
        PGC_TEST(pgc_tst_par(xbyte, "7f;;", &lst) == PGC_ERR_OK); 
        PGC_TEST(pgc_ast_typeof(lst->val) == PGC_AST_INT8);
        PGC_TEST(pgc_ast_toint8(lst->val) == 0x7F);
        PGC_TEST(pgc_tst_par(xbyte, "b;;", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_ast_typeof(lst->val) == PGC_AST_INT8);
        PGC_TEST(pgc_ast_toint8(lst->val) == 11);
        PGC_TEST(pgc_tst_par(xbyte, "0;;", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_ast_typeof(lst->val) == PGC_AST_INT8);
        PGC_TEST(pgc_ast_toint8(lst->val) == 0);
        PGC_TEST(pgc_tst_par(xbyte, "G;;", &lst) == PGC_ERR_CMP);
}

void test_pctbyte()
{
        PGC_INFO();
        struct pgc_self *self = export_pgc_self();
        struct pgc_par *xbyte = pgc_self_pctbyte(self);
        struct pgc_ast_lst *lst = NULL;
        PGC_TEST(pgc_tst_par(xbyte, "%0a;;", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_ast_typeof(lst->val) == PGC_AST_INT8);
        PGC_TEST(pgc_ast_toint8(lst->val) == 0x0A);
        PGC_TEST(pgc_tst_par(xbyte, "%7f;;", &lst) == PGC_ERR_OK); 
        PGC_TEST(pgc_ast_typeof(lst->val) == PGC_AST_INT8);
        PGC_TEST(pgc_ast_toint8(lst->val) == 0x7F);
        PGC_TEST(pgc_tst_par(xbyte, "%b;;", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_ast_typeof(lst->val) == PGC_AST_INT8);
        PGC_TEST(pgc_ast_toint8(lst->val) == 11);
        PGC_TEST(pgc_tst_par(xbyte, "%0;;", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_ast_typeof(lst->val) == PGC_AST_INT8);
        PGC_TEST(pgc_ast_toint8(lst->val) == 0);
        PGC_TEST(pgc_tst_par(xbyte, "%G;;", &lst) == PGC_ERR_CMP);
}

void test_charlit()
{
        PGC_INFO();
        struct pgc_self *self = export_pgc_self();
        struct pgc_par *blit = pgc_self_charlit(self);
        struct pgc_ast_lst *lst = NULL;
        PGC_TEST(pgc_tst_par(blit, "'a'", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_ast_typeof(lst->val) == PGC_AST_INT8);
        PGC_TEST(pgc_ast_toint8(lst->val) == 'a'); 
}

void test_digits()
{
        PGC_INFO();
        struct pgc_self *self = export_pgc_self();
        struct pgc_par *digits = pgc_self_digits(self);
        PGC_TEST(pgc_par_runs(digits, "0a", NULL) == 1);
        PGC_TEST(pgc_par_runs(digits, "99", NULL) == 2);
        PGC_TEST(pgc_par_runs(digits, "01", NULL) == 2);
        PGC_TEST(pgc_par_runs(digits, "_", NULL) == PGC_ERR_CMP);
}

void test_num()
{
        PGC_INFO();
        struct pgc_self *self = export_pgc_self();
        struct pgc_par *num = pgc_self_num(self);
        struct pgc_ast_lst *lst = NULL;
        PGC_TEST(pgc_tst_par(num, "3a", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_NUM);
        PGC_TEST(pgc_ast_touint32(lst->val) == 3);
        PGC_TEST(pgc_tst_par(num, "37a", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_NUM);
        PGC_TEST(pgc_ast_touint32(lst->val) == 37);
}

void test_range()
{
        PGC_INFO();
        struct pgc_self *self = export_pgc_self();
        struct pgc_par *range = pgc_self_range(self);
        struct pgc_ast_lst *lst = NULL;
        PGC_TEST(pgc_tst_par(range, "12_34a", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_ast_len(lst) == 1);
        PGC_TEST(pgc_ast_typeof(lst->val) == PGC_AST_LST);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_RANGE);
        lst = pgc_ast_tolst(lst->val);
        PGC_TEST(pgc_ast_len(lst) == 2);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_NUM);
        PGC_TEST(pgc_ast_touint32(lst->val) == 12);
        lst = lst->nxt;
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_NUM);
        PGC_TEST(pgc_ast_touint32(lst->val) == 34);
}

void test_utfvalue()
{
        PGC_INFO();
        struct pgc_self *self = export_pgc_self();
        struct pgc_par *utf = pgc_self_utfvalue(self);
        struct pgc_ast_lst *lst = NULL;


        PGC_TEST(pgc_tst_par(utf, "123", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_NUM);
        PGC_TEST(pgc_ast_touint32(lst->val) == 0x123);
}

void test_utfrange()
{
        PGC_INFO();
        struct pgc_self *self = export_pgc_self();
        struct pgc_par *range = pgc_self_utfrange(self);
        struct pgc_ast_lst *lst = NULL;
        PGC_TEST(pgc_tst_par(range, "&12_34a", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_ast_len(lst) == 1);
        PGC_TEST(pgc_ast_typeof(lst->val) == PGC_AST_LST);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_UTF);
        lst = pgc_ast_tolst(lst->val);
        PGC_TEST(pgc_ast_len(lst) == 2);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_NUM);
        PGC_TEST(pgc_ast_touint32(lst->val) == 0x12);
        lst = lst->nxt;
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_NUM);
        PGC_TEST(pgc_ast_touint32(lst->val) == 0x34A);
}

void test_setexp()
{
        PGC_INFO();
        struct pgc_self *self = export_pgc_self();
        struct pgc_par *exp = pgc_self_setexp(self);
        struct pgc_ast_lst *lst = NULL;
        PGC_TEST(pgc_tst_par(exp, "'a'", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_CHAR);
        PGC_TEST(pgc_tst_par(exp, "'a' + %1a", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_UNION);
        lst = pgc_ast_tolst(lst->val);
        PGC_TEST(pgc_ast_len(lst) == 2);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_CHAR);
        PGC_TEST(pgc_ast_toint8(lst->val) == 'a');
        lst = lst->nxt;
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_CHAR);
        PGC_TEST(pgc_ast_toint8(lst->val) == 0x1A);
}

void test_exp()
{
        PGC_INFO();
        struct pgc_self *self = export_pgc_self();
        struct pgc_par *exp = pgc_self_exp(self);
        struct pgc_ast_lst *lst = NULL;
        PGC_TEST(pgc_tst_par(exp, "'a'", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_CHAR);
        PGC_TEST(pgc_tst_par(exp, "'a' %1a", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_AND);
        lst = pgc_ast_tolst(lst->val);
        PGC_TEST(pgc_ast_len(lst) == 2);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_CHAR);
        PGC_TEST(pgc_ast_toint8(lst->val) == 'a');
        lst = lst->nxt;
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_CHAR);
        PGC_TEST(pgc_ast_toint8(lst->val) == 0x1A);
}

void test_dec()
{
        PGC_INFO();
        struct pgc_self *self = export_pgc_self();
        struct pgc_par *stmt = pgc_self_stmt(self);
        struct pgc_ast_lst *lst = NULL;
        PGC_TEST(pgc_tst_par(stmt, "dec hi;", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_ast_typeof(lst->val) == PGC_AST_LST);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_DEC);
        lst = pgc_ast_tolst(lst->val);
        PGC_TEST(pgc_ast_len(lst) == 1);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_ID);
}

void test_def()
{
        PGC_INFO();
        struct pgc_self *self = export_pgc_self();
        struct pgc_par *stmt = pgc_self_stmt(self);
        struct pgc_ast_lst *lst = NULL;
        PGC_TEST(pgc_tst_par(stmt, "def hi = 'a';", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_ast_typeof(lst->val) == PGC_AST_LST);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_DEF);
        lst = pgc_ast_tolst(lst->val);
        PGC_TEST(pgc_ast_len(lst) == 2);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_ID);
}

void test_set()
{
        PGC_INFO();
        struct pgc_self *self = export_pgc_self();
        struct pgc_par *stmt = pgc_self_stmt(self);
        struct pgc_ast_lst *lst = NULL;
        PGC_TEST(pgc_tst_par(stmt, "set hi = 'a';", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_ast_typeof(lst->val) == PGC_AST_LST);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_SET);
        lst = pgc_ast_tolst(lst->val);
        PGC_TEST(pgc_ast_len(lst) == 2);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_ID);
}

void test_let()
{
        PGC_INFO();
        struct pgc_self *self = export_pgc_self();
        struct pgc_par *stmt = pgc_self_stmt(self);
        struct pgc_ast_lst *lst = NULL;
        PGC_TEST(pgc_tst_par(stmt, "let hi = 'a';", &lst) == PGC_ERR_OK);
        PGC_TEST(pgc_ast_typeof(lst->val) == PGC_AST_LST);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_LET);
        lst = pgc_ast_tolst(lst->val);
        PGC_TEST(pgc_ast_len(lst) == 2);
        PGC_TEST(pgc_syn_typeof(lst->val) == PGC_SYN_ID);
}

int main(int argc, char **argv)
{
        PGC_INFO();
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
        // test_src();
}