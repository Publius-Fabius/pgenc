
#include "pgenc/ast.h"
#include "pgenc/error.h"
#include <assert.h>
#include <stdio.h>

void test_initint8() 
{
       struct pgc_ast ast;
       pgc_ast_initint8(&ast, 0x3F, 123);
       PGC_TEST(ast.utag == 123);
       PGC_TEST(ast.atag == PGC_AST_INT8);
       PGC_TEST(pgc_ast_toint8(&ast) == 0x3F);
}

void test_inituint8() 
{
       struct pgc_ast ast;
       pgc_ast_inituint8(&ast, 0x1F, 123);
       PGC_TEST(ast.utag == 123);
       PGC_TEST(ast.atag == PGC_AST_UINT8);
       PGC_TEST(pgc_ast_touint8(&ast) == 0x1F);
}

void test_initint16() 
{
       struct pgc_ast ast;
       pgc_ast_initint16(&ast, 0x1234, 123);
       PGC_TEST(ast.utag == 123);
       PGC_TEST(ast.atag == PGC_AST_INT16);
       PGC_TEST(pgc_ast_toint16(&ast) == 0x1234);
}

void test_inituint16() 
{
       struct pgc_ast ast;
       pgc_ast_inituint16(&ast, 0x1234, 123);
       PGC_TEST(ast.utag == 123);
       PGC_TEST(ast.atag == PGC_AST_UINT16);
       PGC_TEST(pgc_ast_touint16(&ast) == 0x1234);
}

void test_initint32() 
{
       struct pgc_ast ast;
       pgc_ast_initint32(&ast, 0xBEEF, 123);
       PGC_TEST(ast.utag == 123);
       PGC_TEST(ast.atag == PGC_AST_INT32);
       PGC_TEST(pgc_ast_toint32(&ast) == 0xBEEF);
}

void test_inituint32() 
{
       struct pgc_ast ast;
       pgc_ast_inituint32(&ast, 0xBEEF, 123);
       PGC_TEST(ast.utag == 123);
       PGC_TEST(ast.atag == PGC_AST_UINT32);
       PGC_TEST(pgc_ast_touint32(&ast) == 0xBEEF);
}

void test_initint64() 
{
       struct pgc_ast ast;
       pgc_ast_initint64(&ast, 0xBEEF, 123);
       PGC_TEST(ast.utag == 123);
       PGC_TEST(ast.atag == PGC_AST_INT64);
       PGC_TEST(pgc_ast_toint64(&ast) == 0xBEEF);
}

void test_inituint64() 
{
       struct pgc_ast ast;
       pgc_ast_inituint64(&ast, 0xBEEF, 123);
       PGC_TEST(ast.utag == 123);
       PGC_TEST(ast.atag == PGC_AST_UINT64);
       PGC_TEST(pgc_ast_touint64(&ast) == 0xBEEF);
}

void test_initfloat32() 
{
       struct pgc_ast ast;
       pgc_ast_initfloat32(&ast, -3.14F, 123);
       PGC_TEST(ast.utag == 123);
       PGC_TEST(ast.atag == PGC_AST_FLOAT32);
       PGC_TEST(pgc_ast_tofloat32(&ast) == -3.14F);
}

void test_initfloat64() 
{
       struct pgc_ast ast;
       pgc_ast_initfloat64(&ast, -3.14, 123);
       PGC_TEST(ast.utag == 123);
       PGC_TEST(ast.atag == PGC_AST_FLOAT64);
       PGC_TEST(pgc_ast_tofloat64(&ast) == -3.14);
}

void test_initstr() 
{
       struct pgc_ast ast;
       char *dog = "dog";
       pgc_ast_initstr(&ast, dog, 123);
       PGC_TEST(ast.utag == 123);
       PGC_TEST(ast.atag == PGC_AST_STR);
       PGC_TEST(pgc_ast_tostr(&ast) == dog);
}

void test_initlst() 
{
       struct pgc_ast ast;
       struct pgc_ast_lst lst;
       pgc_ast_initlst(&ast, &lst, 123);
       PGC_TEST(ast.utag == 123);
       PGC_TEST(ast.atag == PGC_AST_LST);
       PGC_TEST(pgc_ast_tolst(&ast) == &lst);
}

void test_at()
{
       struct pgc_ast_lst head, tail;
       struct pgc_ast one, two;
       pgc_ast_initint32(&one, 1, 123);
       pgc_ast_initint32(&two, 2, 123);
       tail.nxt = NULL;
       tail.val = &two;
       head.nxt = &tail;
       head.val = &one;
       PGC_TEST(pgc_ast_at(&head, 0)->val == &one);
       PGC_TEST(pgc_ast_at(&head, 1)->val == &two);
}

void test_rev()
{
       struct pgc_ast_lst head, tail;
       struct pgc_ast one, two;
       pgc_ast_initint32(&one, 1, 123);
       pgc_ast_initint32(&two, 2, 123);
       tail.nxt = NULL;
       tail.val = &two;
       head.nxt = &tail;
       head.val = &one;
       struct pgc_ast_lst *ptr = pgc_ast_rev(&head);
       PGC_TEST(ptr);
       PGC_TEST(pgc_ast_at(ptr, 0)->val == &two);
       PGC_TEST(pgc_ast_at(ptr, 1)->val == &one);
       PGC_TEST(pgc_ast_at(ptr, 1)->nxt == NULL);
}

void test_cat()
{
       struct pgc_ast_lst head, tail;
       struct pgc_ast one, two;
       pgc_ast_initint32(&one, 1, 123);
       pgc_ast_initint32(&two, 2, 123);
       tail.nxt = NULL;
       tail.val = &two;
       head.nxt = NULL;
       head.val = &one;
       struct pgc_ast_lst *ptr = pgc_ast_cat(&head, &tail);
       PGC_TEST(ptr);
       PGC_TEST(pgc_ast_at(ptr, 0)->val == &one);
       PGC_TEST(pgc_ast_at(ptr, 1)->val == &two);
       PGC_TEST(pgc_ast_at(ptr, 1)->nxt == NULL);
       ptr = pgc_ast_rev(ptr);
       PGC_TEST(ptr);
       PGC_TEST(pgc_ast_at(ptr, 0)->val == &two);
       PGC_TEST(pgc_ast_at(ptr, 1)->val == &one);
       PGC_TEST(pgc_ast_at(ptr, 1)->nxt == NULL);
}

int main(int argc, char **argv)
{
       puts("running test_ast...");
       test_initint8();
       test_inituint8();
       test_initint16();
       test_inituint16();
       test_inituint32();
       test_initint64();
       test_inituint64();
       test_initfloat32();
       test_initfloat64();
       test_initstr();
       test_initlst();
       test_at();
       test_rev();
       test_cat();
}