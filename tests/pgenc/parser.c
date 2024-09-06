
#include "pgenc/parser.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

void test_byte()
{
        puts(" ... test_byte()...");
        struct pgc_par p; pgc_par_byte(&p, 'x');
        assert(pgc_par_runs(&p, "xyz", NULL) == 1);
        assert(pgc_par_runs(&p, "abc", NULL) == PGC_ERR_CMP);
}

void test_cmp()
{
        puts(" ... test_cmp()...");
        struct pgc_par p; pgc_par_cmp(&p, "cat", 3);
        assert(pgc_par_runs(&p, "catty", NULL) == 3);
        assert(pgc_par_runs(&p, "cat", NULL) == 3);
        assert(pgc_par_runs(&p, "ca", NULL) == PGC_ERR_OOB);
        assert(pgc_par_runs(&p, "dog", NULL) == PGC_ERR_CMP);
}

void test_utf8()
{
        puts(" ... test_utf8()...");
        const uint32_t min = 0x0390;
        const uint32_t max = 0x039F;
        struct pgc_par p; pgc_par_utf8(&p, min, max);
        assert(pgc_par_runs(&p, "ΐ", NULL) == 2);
        assert(pgc_par_runs(&p, "Ο", NULL) == 2);
        assert(pgc_par_runs(&p, "Π", NULL) == PGC_ERR_CMP);

        const uint32_t thai_min = 0x0E09;
        const uint32_t thai_max = 0x0E10;
        pgc_par_utf8(&p, thai_min, thai_max);
        assert(pgc_par_runs(&p, "ฉ", NULL) == 3);
        assert(pgc_par_runs(&p, "ฐ", NULL) == 3);
        assert(pgc_par_runs(&p, "จ", NULL) == PGC_ERR_CMP);

        const uint32_t retro_min = 0x1FB30;
        const uint32_t retro_max = 0x1FB3F;
        pgc_par_utf8(&p, retro_min, retro_max);
        assert(pgc_par_runs(&p, "🬱", NULL) == 4);
        assert(pgc_par_runs(&p, "🬾", NULL) == 4);
        assert(pgc_par_runs(&p, "🭀", NULL) == PGC_ERR_CMP);
}

void test_set()
{
        puts(" ... test_set()...");

        struct pgc_cset s; pgc_cset_iso(&s, isupper);
        struct pgc_par p; pgc_par_set(&p, &s);
        assert(pgc_par_runs(&p, "IJK", NULL) == 1);
        assert(pgc_par_runs(&p, "ijk", NULL) == PGC_ERR_CMP);
}

void test_and()
{
        puts(" ... test_and()...");
        struct pgc_par left; pgc_par_byte(&left, 'a');
        struct pgc_par right; pgc_par_byte(&right, 'b');
        struct pgc_par p; pgc_par_and(&p, &left, &right);
        assert(pgc_par_runs(&p, "ab", NULL) == 2);
        assert(pgc_par_runs(&p, "ax", NULL) == PGC_ERR_CMP);
        assert(pgc_par_runs(&p, "a", NULL) == PGC_ERR_OOB);
}

void test_or()
{
        puts(" ... test_or()...");
        struct pgc_par left; pgc_par_byte(&left, 'a');
        struct pgc_par right; pgc_par_byte(&right, 'b');
        struct pgc_par p; pgc_par_or(&p, &left, &right);
        assert(pgc_par_runs(&p, "a", NULL) == 1);
        assert(pgc_par_runs(&p, "b", NULL) == 1);
        assert(pgc_par_runs(&p, "c", NULL) == PGC_ERR_CMP);
        assert(pgc_par_runs(&p, "", NULL) == PGC_ERR_OOB);
}

void test_rep()
{
        puts(" ... test_rep()...");
        struct pgc_cset s; pgc_cset_iso(&s, isupper);
        struct pgc_par sub; pgc_par_set(&sub, &s);
        struct pgc_par p; pgc_par_rep(&p, &sub, 1, 3);
        assert(pgc_par_runs(&p, "Aaa", NULL) == 1);
        assert(pgc_par_runs(&p, "aaa", NULL) == PGC_ERR_CMP);
        assert(pgc_par_runs(&p, "", NULL) == PGC_ERR_OOB);
        assert(pgc_par_runs(&p, "ABcd", NULL) == 2);
        assert(pgc_par_runs(&p, "ABCd", NULL) == 3);
}

enum pgc_err test_hook_f(struct pgc_buf *buf, void *st)
{
        (*(int*)st) += 5;
        return PGC_ERR_OK;
}

void test_hook()
{
        puts(" ... test_hook()...");

        int st = 0;

        struct pgc_cset s; pgc_cset_iso(&s, isupper);
        struct pgc_par left; pgc_par_set(&left, &s);
        struct pgc_par right; pgc_par_hook(&right, test_hook_f);
        struct pgc_par p; pgc_par_and(&p, &left, &right);
        assert(pgc_par_runs(&p, "A", &st) == 1);
        assert(st == 5);
        assert(pgc_par_runs(&p, "a", &st) == PGC_ERR_CMP);
        assert(st == 5);
}

int main(int argc, char **args) 
{
        puts("running test_parser...");
        test_byte();
        test_cmp();
        test_utf8();
        test_set();
        test_and();
        test_or();
        test_rep();
        test_hook();
}