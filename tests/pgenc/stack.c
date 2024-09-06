
#include "pgenc/stack.h"
#include "pgenc/error.h"
#include <stdio.h>

int main(int argc, char **args) 
{
        puts("running test_stack...");
        struct pgc_stk stk;
        char buf[16];
        pgc_stk_init(&stk, 16, buf);
        PGC_TEST(buf ==  pgc_stk_push(&stk, 8));
        PGC_TEST(pgc_stk_push(&stk, 10) == NULL);
        PGC_TEST(pgc_stk_push(&stk, 8) == (buf + 8));
        PGC_TEST(pgc_stk_pop(&stk, 4) == (buf + 12));
        PGC_TEST(pgc_stk_push(&stk, 5) == NULL);
        PGC_TEST(pgc_stk_push(&stk, 4) == (buf + 12));
        PGC_TEST(pgc_stk_push(&stk, 1) == NULL);
        PGC_TEST(pgc_stk_push(&stk, 0) == (buf + 16));
        PGC_TEST(pgc_stk_pop(&stk, 17) == NULL);
        PGC_TEST(pgc_stk_pop(&stk, 16) == buf);
}