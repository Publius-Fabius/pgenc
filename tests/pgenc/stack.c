
#include "pgenc/stack.h"
#include "pgenc/error.h"
#include <stdio.h>
#include <stdlib.h>

void test_push_pop_char()
{
        SEL_INFO();
        struct pgc_stk stk;
        const size_t buflen = 3;
        void* buf = malloc(buflen);
        pgc_stk_init(&stk, buf, buflen);
        char *ptr;
        SEL_TEST(ptr = pgc_stk_push(&stk, 1));
        *ptr = 'a';
        SEL_TEST(*((char*)pgc_stk_peek(&stk, 0)) == 'a');
        SEL_TEST(ptr = pgc_stk_push(&stk, 1));
        *ptr = 'b';
        SEL_TEST(*((char*)pgc_stk_peek(&stk, 0)) == 'b');
        SEL_TEST(ptr = pgc_stk_push(&stk, 1));
        *ptr = 'c';
        SEL_TEST(*((char*)pgc_stk_peek(&stk, 0)) == 'c');
        SEL_TEST(!pgc_stk_push(&stk, 1));
        SEL_TEST(*((char*)pgc_stk_pop(&stk, 1)) == 'c');
        SEL_TEST(*((char*)pgc_stk_pop(&stk, 1)) == 'b');
        SEL_TEST(*((char*)pgc_stk_pop(&stk, 1)) == 'a');
        SEL_TEST(!pgc_stk_pop(&stk, 1));
        SEL_TEST(!pgc_stk_peek(&stk, 0));
        free(buf);
}

void test_push_pop_ptr()
{
        SEL_INFO();
        struct pgc_stk stk;
        const size_t buflen = 3 * sizeof(void**);
        void* buf = malloc(buflen);
        pgc_stk_init(&stk, buf, buflen);
        char a = 'a', b = 'b', c = 'c';
        char **ptr;
        SEL_TEST(ptr = pgc_stk_push(&stk, sizeof(void**)));
        *ptr = &a;
        SEL_TEST(*((void**)pgc_stk_peek(&stk, 0)) == &a);
        SEL_TEST(ptr = pgc_stk_push(&stk, sizeof(void**)));
        *ptr = &b;
        SEL_TEST(*((void**)pgc_stk_peek(&stk, 0)) == &b);
        SEL_TEST(ptr = pgc_stk_push(&stk, sizeof(void**)));
        *ptr = &c;
        SEL_TEST(*((void**)pgc_stk_peek(&stk, 0)) == &c);
        SEL_TEST(!pgc_stk_push(&stk, sizeof(void**)));
        SEL_TEST(*((void**)pgc_stk_pop(&stk, sizeof(void**))) == &c);
        SEL_TEST(*((void**)pgc_stk_pop(&stk, sizeof(void**))) == &b);
        SEL_TEST(*((void**)pgc_stk_pop(&stk, sizeof(void**))) == &a);
        SEL_TEST(!pgc_stk_pop(&stk, 1));
        SEL_TEST(!pgc_stk_peek(&stk, 0));
        free(buf);
}

int main(int argc, char **args) 
{
        SEL_INFO();
        test_push_pop_char();
        test_push_pop_ptr();
}