
#include "pgenc/table.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

void test_create_destroy()
{
        puts(" ... test_create_destroy()");
        pgc_tbl_destroy(pgc_tbl_create(4));
}

void test_insert_lookup()
{
        puts(" ... test_insert_lookup()");
        struct pgc_tbl *tbl = pgc_tbl_create(4);
        
        pgc_tbl_insert(tbl, "dog", NULL);

        struct pgc_tbl_i imem;
        struct pgc_tbl_i *i = pgc_tbl_lookup(tbl, &imem, "dog");
        assert(i);
        pgc_tbl_insert(tbl, "cat", NULL);
        i = pgc_tbl_lookup(tbl, &imem, "cat");
        assert(i);
        assert(!strcmp(pgc_tbl_key(i), "cat"));

        i = pgc_tbl_lookup(tbl, &imem, "rat");
        assert(!i);

        pgc_tbl_destroy(tbl);
}

void test_resize()
{
        puts(" ... test_resize()");
        struct pgc_tbl *tbl = pgc_tbl_create(4);

        pgc_tbl_insert(tbl, "dog", NULL);
        pgc_tbl_insert(tbl, "cat", NULL);
        pgc_tbl_insert(tbl, "rat", NULL);
        pgc_tbl_insert(tbl, "frog", NULL);
        pgc_tbl_insert(tbl, "snail", NULL);

        assert(pgc_tbl_size(tbl) == 5);
        assert(pgc_tbl_capacity(tbl) > 4);

        pgc_tbl_destroy(tbl);
}

void test_remove()
{
        puts(" ... test_remove()");
        struct pgc_tbl *tbl = pgc_tbl_create(4);

        pgc_tbl_insert(tbl, "dog", NULL);
        pgc_tbl_insert(tbl, "cat", NULL);
        pgc_tbl_insert(tbl, "rat", NULL);
        pgc_tbl_insert(tbl, "frog", NULL);
        pgc_tbl_insert(tbl, "snail", NULL);

        pgc_tbl_remove(tbl, "cat");

        assert(pgc_tbl_size(tbl) == 4);

        struct pgc_tbl_i i;

        assert(!pgc_tbl_lookup(tbl, &i, "cat"));
        assert(pgc_tbl_lookup(tbl, &i, "rat"));
        assert(pgc_tbl_lookup(tbl, &i, "frog"));
        assert(pgc_tbl_lookup(tbl, &i, "snail"));
        assert(pgc_tbl_lookup(tbl, &i, "dog"));

        pgc_tbl_destroy(tbl);
}

int main(int argc, char **args)
{
        puts("running test/pgenc/table...");
        test_create_destroy();
        test_insert_lookup();
        test_resize();
        test_remove();
        return EXIT_SUCCESS;

}