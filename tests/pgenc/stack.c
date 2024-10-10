
#include "pgenc/stack.h"
#include "pgenc/error.h"
#include <stdio.h>

int main(int argc, char **args) 
{
        puts("running test_stack...");
        struct pgc_stk stk;
        char buf[16];
        pgc_stk_init(&stk, buf, 16);
        SEL_TEST(buf ==  pgc_stk_push(&stk, 8));
        SEL_TEST(pgc_stk_push(&stk, 10) == NULL);
        SEL_TEST(pgc_stk_push(&stk, 8) == (buf + 8));
        SEL_TEST(pgc_stk_pop(&stk, 4) == (buf + 12));
        SEL_TEST(pgc_stk_push(&stk, 5) == NULL);
        SEL_TEST(pgc_stk_push(&stk, 4) == (buf + 12));
        SEL_TEST(pgc_stk_push(&stk, 1) == NULL);
        SEL_TEST(pgc_stk_push(&stk, 0) == (buf + 16));
        SEL_TEST(pgc_stk_pop(&stk, 17) == NULL);
        SEL_TEST(pgc_stk_pop(&stk, 16) == buf);
}