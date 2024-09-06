
#include "pgenc/charset.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

void test_iso()
{
        struct pgc_cset set;

        puts("   ...test_iso()");
        pgc_cset_iso(&set, isalnum);
        assert(pgc_cset_in(&set, 'a'));
        assert(pgc_cset_in(&set, 'z'));
        assert(pgc_cset_in(&set, '0'));
        assert(pgc_cset_in(&set, '9'));
        assert(!pgc_cset_in(&set, '$'));
}

void test_unset()
{
        struct pgc_cset set;

        puts("   ...test_unset()");

        pgc_cset_iso(&set, isalnum);
        assert(pgc_cset_in(&set, 'a'));
        assert(pgc_cset_in(&set, 'z'));
        assert(pgc_cset_in(&set, '0'));
        assert(pgc_cset_in(&set, '9'));

        pgc_cset_unset(&set, 'a');

        assert(!pgc_cset_in(&set, 'a'));
        assert(!pgc_cset_in(&set, '$'));
}

void test_union()
{
        struct pgc_cset set1, set2, set3;

        puts("   ...test_union()");
        
        pgc_cset_iso(&set1, isalpha);
        pgc_cset_iso(&set2, isdigit);
        pgc_cset_union(&set3, &set1, &set2);

        assert(pgc_cset_in(&set3, 'a'));
        assert(pgc_cset_in(&set3, 'z'));
        assert(pgc_cset_in(&set3, '0'));
        assert(pgc_cset_in(&set3, '9'));
}

void test_isect()
{
        struct pgc_cset set1, set2, set3;

        puts("   ...test_isect()");
        
        pgc_cset_iso(&set1, isalnum);
        pgc_cset_iso(&set2, isdigit);
        pgc_cset_isect(&set3, &set1, &set2);

        assert(!pgc_cset_in(&set3, 'a'));
        assert(!pgc_cset_in(&set3, 'z'));
        assert(pgc_cset_in(&set3, '0'));
        assert(pgc_cset_in(&set3, '9'));
}

void test_diff()
{
        struct pgc_cset set1, set2, set3;

        puts("   ...test_diff()");
        
        pgc_cset_iso(&set1, isalnum);
        pgc_cset_iso(&set2, isdigit);
        pgc_cset_diff(&set3, &set1, &set2);

        assert(pgc_cset_in(&set3, 'a'));
        assert(pgc_cset_in(&set3, 'z'));
        assert(!pgc_cset_in(&set3, '0'));
        assert(!pgc_cset_in(&set3, '9'));
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
        puts("running test_charset...");
        test_iso();
        test_unset();
        test_union();
        test_isect();
        test_diff();
        return EXIT_SUCCESS;
}