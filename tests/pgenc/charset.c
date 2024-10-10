
#include "pgenc/charset.h"
#include "pgenc/error.h"
#include <stdlib.h>
#include <stdio.h>

void test_iso()
{
        SEL_INFO();
        struct pgc_cset set;
        pgc_cset_iso(&set, pgc_cset_isalnum);
        SEL_TEST(pgc_cset_in(&set, 'a'));
        SEL_TEST(pgc_cset_in(&set, 'z'));
        SEL_TEST(pgc_cset_in(&set, '0'));
        SEL_TEST(pgc_cset_in(&set, '9'));
        SEL_TEST(!pgc_cset_in(&set, '$'));
}

void test_unset()
{
        SEL_INFO();
        struct pgc_cset set;
        pgc_cset_iso(&set, pgc_cset_isalnum);
        SEL_TEST(pgc_cset_in(&set, 'a'));
        SEL_TEST(pgc_cset_in(&set, 'z'));
        SEL_TEST(pgc_cset_in(&set, '0'));
        SEL_TEST(pgc_cset_in(&set, '9'));
        pgc_cset_unset(&set, 'a');
        SEL_TEST(!pgc_cset_in(&set, 'a'));
        SEL_TEST(!pgc_cset_in(&set, '$'));
}

void test_union()
{
        SEL_INFO();
        struct pgc_cset set1, set2, set3;
        pgc_cset_iso(&set1, pgc_cset_isalpha);
        pgc_cset_iso(&set2, pgc_cset_isdigit);
        pgc_cset_union(&set3, &set1, &set2);
        SEL_TEST(pgc_cset_in(&set3, 'a'));
        SEL_TEST(pgc_cset_in(&set3, 'z'));
        SEL_TEST(pgc_cset_in(&set3, '0'));
        SEL_TEST(pgc_cset_in(&set3, '9'));
}

void test_isect()
{
        SEL_INFO();
        struct pgc_cset set1, set2, set3;
        pgc_cset_iso(&set1, pgc_cset_isalnum);
        pgc_cset_iso(&set2, pgc_cset_isdigit);
        pgc_cset_isect(&set3, &set1, &set2);
        SEL_TEST(!pgc_cset_in(&set3, 'a'));
        SEL_TEST(!pgc_cset_in(&set3, 'z'));
        SEL_TEST(pgc_cset_in(&set3, '0'));
        SEL_TEST(pgc_cset_in(&set3, '9'));
}

void test_diff()
{
        SEL_INFO();
        struct pgc_cset set1, set2, set3;
        pgc_cset_iso(&set1, pgc_cset_isalnum);
        pgc_cset_iso(&set2, pgc_cset_isdigit);
        pgc_cset_diff(&set3, &set1, &set2);
        SEL_TEST(pgc_cset_in(&set3, 'a'));
        SEL_TEST(pgc_cset_in(&set3, 'z'));
        SEL_TEST(!pgc_cset_in(&set3, '0'));
        SEL_TEST(!pgc_cset_in(&set3, '9'));
}

void print_set(struct pgc_cset *set)
{
        int x;
        printf("{ %i", set->words[0]);
        for(x = 1; x < 8; ++x) {
                printf(", %i", set->words[x]);
        }
        printf(" }");
}

int main(int argc, char ** args)
{
        SEL_INFO();
        test_iso();
        test_unset();
        test_union();
        test_isect();
        test_diff();
        return EXIT_SUCCESS;
}