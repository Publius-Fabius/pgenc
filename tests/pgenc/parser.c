
#include "pgenc/parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

void test_byte()
{
        SEL_INFO();
        const struct pgc_par p = PGC_PAR_BYTE('x');
        SEL_TEST(pgc_par_runs(&p, "xyz", NULL) == 1);
        SEL_TEST(pgc_par_runs(&p, "x", NULL) == 1);
        SEL_TEST(pgc_par_runs(&p, "abc", NULL) == PGC_ERR_CMP);
        SEL_TEST(pgc_par_runs(&p, "b", NULL) == PGC_ERR_CMP);
        SEL_TEST(pgc_par_runs(&p, "", NULL) == PGC_ERR_OOB);
}

void test_cmp()
{
        SEL_INFO();
        const struct pgc_par p = PGC_PAR_CMP("cat", 3);
        SEL_TEST(pgc_par_runs(&p, "catty", NULL) == 3);
        SEL_TEST(pgc_par_runs(&p, "cat", NULL) == 3);
        SEL_TEST(pgc_par_runs(&p, "ca", NULL) == PGC_ERR_OOB);
        SEL_TEST(pgc_par_runs(&p, "", NULL) == PGC_ERR_OOB);
        SEL_TEST(pgc_par_runs(&p, "do", NULL) == PGC_ERR_OOB);
        SEL_TEST(pgc_par_runs(&p, "dog", NULL) == PGC_ERR_CMP);
        SEL_TEST(pgc_par_runs(&p, "doggy", NULL) == PGC_ERR_CMP);
}

void test_utf8()
{
        SEL_INFO();
        const uint32_t min = 0x0390;
        const uint32_t max = 0x039F;
        const struct pgc_par p = PGC_PAR_UTF8(min, max);
        SEL_TEST(pgc_par_runs(&p, "ΐ", NULL) == 2);
        SEL_TEST(pgc_par_runs(&p, "Ο", NULL) == 2);
        SEL_TEST(pgc_par_runs(&p, "Π", NULL) == PGC_ERR_CMP);
        SEL_TEST(pgc_par_runs(&p, "", NULL) == PGC_ERR_OOB);

        const uint32_t thai_min = 0x0E09;
        const uint32_t thai_max = 0x0E10;
        const struct pgc_par tp = PGC_PAR_UTF8(thai_min, thai_max);
        SEL_TEST(pgc_par_runs(&tp, "ฉ", NULL) == 3);
        SEL_TEST(pgc_par_runs(&tp, "ฐ", NULL) == 3);
        SEL_TEST(pgc_par_runs(&tp, "จ", NULL) == PGC_ERR_CMP);
        SEL_TEST(pgc_par_runs(&tp, "", NULL) == PGC_ERR_OOB);

        const uint32_t retro_min = 0x1FB30;
        const uint32_t retro_max = 0x1FB3F;
        const struct pgc_par rp = PGC_PAR_UTF8(retro_min, retro_max);
        SEL_TEST(pgc_par_runs(&rp, "🬱", NULL) == 4);
        SEL_TEST(pgc_par_runs(&rp, "🬾", NULL) == 4);
        SEL_TEST(pgc_par_runs(&rp, "🭀", NULL) == PGC_ERR_CMP);
        SEL_TEST(pgc_par_runs(&rp, "", NULL) == PGC_ERR_OOB);
}

void test_set()
{
        SEL_INFO();
        struct pgc_cset s; 
        pgc_cset_iso(&s, pgc_cset_isupper);
        const struct pgc_cset cs = s;
        const struct pgc_par p = PGC_PAR_SET(&cs);
        SEL_TEST(pgc_par_runs(&p, "IJK", NULL) == 1);
        SEL_TEST(pgc_par_runs(&p, "A", NULL) == 1);
        SEL_TEST(pgc_par_runs(&p, "ijk", NULL) == PGC_ERR_CMP);
        SEL_TEST(pgc_par_runs(&p, "a", NULL) == PGC_ERR_CMP);
        SEL_TEST(pgc_par_runs(&p, "", NULL) == PGC_ERR_OOB);
}

void test_and()
{
        SEL_INFO();
        const struct pgc_par a = PGC_PAR_BYTE('a');
        const struct pgc_par b = PGC_PAR_BYTE('b');
        const struct pgc_par p = PGC_PAR_AND(&a, &b);
        SEL_TEST(pgc_par_runs(&p, "ab", NULL) == 2);
        SEL_TEST(pgc_par_runs(&p, "ax", NULL) == PGC_ERR_CMP);
        SEL_TEST(pgc_par_runs(&p, "a", NULL) == PGC_ERR_OOB);
        const struct pgc_par left_left = PGC_PAR_BYTE('a');
        const struct pgc_par left_right = PGC_PAR_BYTE('b');
        const struct pgc_par right_left = PGC_PAR_BYTE('c');
        const struct pgc_par right_right = PGC_PAR_BYTE('d');
        const struct pgc_par left = PGC_PAR_AND(&left_left, &left_right);
        const struct pgc_par right = PGC_PAR_AND(&right_left, &right_right);
        const struct pgc_par and = PGC_PAR_AND(&left, &right);
        SEL_TEST(pgc_par_runs(&and, "abcde", NULL) == 4);
        SEL_TEST(pgc_par_runs(&and, "abcd", NULL) == 4);
        SEL_TEST(pgc_par_runs(&and, "abc", NULL) == PGC_ERR_OOB);
        SEL_TEST(pgc_par_runs(&and, "ab", NULL) == PGC_ERR_OOB);
        SEL_TEST(pgc_par_runs(&and, "a", NULL) == PGC_ERR_OOB);
        SEL_TEST(pgc_par_runs(&and, "", NULL) == PGC_ERR_OOB);
        SEL_TEST(pgc_par_runs(&and, "abcx", NULL) == PGC_ERR_CMP);
        SEL_TEST(pgc_par_runs(&and, "abx", NULL) == PGC_ERR_CMP);
        SEL_TEST(pgc_par_runs(&and, "ax", NULL) == PGC_ERR_CMP);
        SEL_TEST(pgc_par_runs(&and, "x", NULL) == PGC_ERR_CMP);
}

void test_or()
{
        SEL_INFO();
        const struct pgc_par a = PGC_PAR_BYTE('a');
        const struct pgc_par b = PGC_PAR_BYTE('b');
        const struct pgc_par p = PGC_PAR_OR(&a, &b);
        SEL_TEST(pgc_par_runs(&p, "a", NULL) == 1);
        SEL_TEST(pgc_par_runs(&p, "b", NULL) == 1);
        SEL_TEST(pgc_par_runs(&p, "c", NULL) == PGC_ERR_CMP);
        SEL_TEST(pgc_par_runs(&p, "", NULL) == PGC_ERR_OOB);
        const struct pgc_par left_left = PGC_PAR_BYTE('a');
        const struct pgc_par left_right = PGC_PAR_BYTE('b');
        const struct pgc_par right_left = PGC_PAR_BYTE('c');
        const struct pgc_par right_right = PGC_PAR_BYTE('d');
        const struct pgc_par left = PGC_PAR_OR(&left_left, &left_right);
        const struct pgc_par right = PGC_PAR_OR(&right_left, &right_right);
        const struct pgc_par or = PGC_PAR_OR(&left, &right);
        SEL_TEST(pgc_par_runs(&or, "", NULL) == PGC_ERR_OOB);
        SEL_TEST(pgc_par_runs(&or, "x", NULL) == PGC_ERR_CMP);
        SEL_TEST(pgc_par_runs(&or, "a", NULL) == 1);
        SEL_TEST(pgc_par_runs(&or, "b", NULL) == 1);
        SEL_TEST(pgc_par_runs(&or, "c", NULL) == 1);
        SEL_TEST(pgc_par_runs(&or, "d", NULL) == 1);
}

void test_rep()
{
        SEL_INFO();
        struct pgc_cset s; 
        pgc_cset_iso(&s, pgc_cset_isupper);
        const struct pgc_cset cs = s;
        const struct pgc_par sub = PGC_PAR_SET(&cs);
        const struct pgc_par p = PGC_PAR_REP(&sub, 1, 3);
        SEL_TEST(pgc_par_runs(&p, "", NULL) == PGC_ERR_OOB);
        SEL_TEST(pgc_par_runs(&p, "a", NULL) == PGC_ERR_CMP);
        SEL_TEST(pgc_par_runs(&p, "A", NULL) == 1);
        SEL_TEST(pgc_par_runs(&p, "Aa", NULL) == 1);
        SEL_TEST(pgc_par_runs(&p, "AAa", NULL) == 2);
        SEL_TEST(pgc_par_runs(&p, "AAAa", NULL) == 3);
        SEL_TEST(pgc_par_runs(&p, "AAAAa", NULL) == 3);
        const struct pgc_par p2 = PGC_PAR_REP(&sub, 0, 3);
        SEL_TEST(pgc_par_runs(&p2, "", NULL) == 0);
        SEL_TEST(pgc_par_runs(&p2, "a", NULL) == 0);
        SEL_TEST(pgc_par_runs(&p2, "A", NULL) == 1);
        SEL_TEST(pgc_par_runs(&p2, "Aa", NULL) == 1);
        SEL_TEST(pgc_par_runs(&p2, "AAa", NULL) == 2);
        SEL_TEST(pgc_par_runs(&p2, "AAAa", NULL) == 3);
        SEL_TEST(pgc_par_runs(&p2, "AAAAa", NULL) == 3);
        const struct pgc_par a = PGC_PAR_BYTE('a');
        const struct pgc_par b = PGC_PAR_BYTE('b');
        const struct pgc_par ab = PGC_PAR_AND(&a, &b);
        const struct pgc_par p3 = PGC_PAR_REP(&ab, 0, 2);
        SEL_TEST(pgc_par_runs(&p3, "", NULL) == 0);
        SEL_TEST(pgc_par_runs(&p3, "a", NULL) == 0);
        SEL_TEST(pgc_par_runs(&p3, "ab", NULL) == 2);
        SEL_TEST(pgc_par_runs(&p3, "aba", NULL) == 2);
        SEL_TEST(pgc_par_runs(&p3, "abab", NULL) == 4);
        SEL_TEST(pgc_par_runs(&p3, "ababa", NULL) == 4);
        SEL_TEST(pgc_par_runs(&p3, "ababab", NULL) == 4);
}

enum pgc_err test_hook_f(struct pgc_buf *buf, void *st)
{
        (*(int*)st) += 5;
        return PGC_ERR_OK;
}

void test_hook()
{
        SEL_INFO();
        int st = 0;
        struct pgc_cset s; pgc_cset_iso(&s, pgc_cset_isupper);
        const struct pgc_cset cs = s;
        const struct pgc_par left = PGC_PAR_SET(&cs);
        const struct pgc_par right = PGC_PAR_HOOK(test_hook_f);
        const struct pgc_par p = PGC_PAR_AND(&left, &right);
        SEL_TEST(pgc_par_runs(&p, "A", &st) == 1);
        SEL_TEST(st == 5);
        SEL_TEST(pgc_par_runs(&p, "a", &st) == PGC_ERR_CMP);
        SEL_TEST(st == 5);
        SEL_TEST(pgc_par_runs(&p, "", &st) == PGC_ERR_OOB);
        SEL_TEST(st == 5);
}

int main(int argc, char **args) 
{
        SEL_INFO();
        test_byte();
        test_cmp();
        test_utf8();
        test_set();
        test_and();
        test_or();
        test_rep();
        test_hook();
}