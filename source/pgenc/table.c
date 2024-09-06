
#include "pgenc/table.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct pgc_tbl {
        struct pgc_tbl_n **nodes;
        size_t capacity;
        size_t size;
};

struct pgc_tbl_n {
        char *key;
        void *value;
        struct pgc_tbl_n *next;
};

static struct pgc_tbl *pgc_tbl_init(
        struct pgc_tbl *tbl, 
        const size_t capacity)
{
        tbl->size = 0;
        tbl->capacity = capacity;
        tbl->nodes = calloc(tbl->capacity, sizeof(void*));
        return tbl;
}

struct pgc_tbl *pgc_tbl_create(const size_t capacity)
{
        struct pgc_tbl *tbl = malloc(sizeof(struct pgc_tbl));
        return pgc_tbl_init(tbl, capacity);
}

void pgc_tbl_destroy(struct pgc_tbl *tbl)
{
        struct pgc_tbl_i imem, nxtmem;
        struct pgc_tbl_i *i = pgc_tbl_begin(tbl, &imem);
        for(nxtmem = imem; i; imem = nxtmem) {
                i = pgc_tbl_next(&nxtmem);
                free(imem.node->key);
                free(imem.node);
        }
        free(tbl->nodes);
        free(tbl);
}

size_t pgc_tbl_size(struct pgc_tbl *table)
{
        return table->size;
}

size_t pgc_tbl_capacity(struct pgc_tbl *table)
{
        return table->capacity;
}

static size_t pgc_tbl_hash(const char *str)
{
        static const size_t FNV_PRIME = 0x100000001b3UL;
        static const size_t FNV_BASIS = 0xcbf29ce484222325UL;

        size_t hash = FNV_BASIS;

        for(const char *c = str; *c; ++c) {
                hash ^= *c;
                hash *= FNV_PRIME;
        }

        return hash;
}

static struct pgc_tbl *pgc_tbl_insert_n(
        struct pgc_tbl *tbl, 
        struct pgc_tbl_n *node)
{
        const size_t hash = pgc_tbl_hash(node->key);
        const size_t bucket = hash % tbl->capacity;
        node->next = tbl->nodes[bucket];
        tbl->nodes[bucket] = node;
        tbl->size += 1;
        return tbl;
}

#include <stdio.h>
static void pgc_tbl_resize(struct pgc_tbl *tbl)
{
        struct pgc_tbl new_tbl;
        pgc_tbl_init(&new_tbl, tbl->capacity * 2);

        struct pgc_tbl_i imem, nxtmem;
        struct pgc_tbl_i *i = pgc_tbl_begin(tbl, &imem);

        for(nxtmem = imem; i; imem = nxtmem) {
                i = pgc_tbl_next(&nxtmem);
                pgc_tbl_insert_n(&new_tbl, imem.node);
        }

        free(tbl->nodes);
        *tbl = new_tbl;
}


struct pgc_tbl *pgc_tbl_insert(
        struct pgc_tbl *tbl, 
        const char *key, 
        void *value)
{
        if((double)tbl->size / (double)tbl->capacity > 0.75) {
               pgc_tbl_resize(tbl);
        }

        struct pgc_tbl_n *node = malloc(sizeof(struct pgc_tbl_n));
        node->key = strcpy(malloc(strlen(key) + 1), key);
        node->value = value;
        return pgc_tbl_insert_n(tbl, node);
}

struct pgc_tbl_i *pgc_tbl_lookup(
        struct pgc_tbl *table,
        struct pgc_tbl_i *iter,
        const char *key)
{
        const size_t hash = pgc_tbl_hash(key);
        const size_t bucket = hash % table->capacity;
        
        for(    struct pgc_tbl_n *node = table->nodes[bucket];
                node; 
                node = node->next) 
        {
                if(!strcmp(node->key, key)) {
                        iter->bucket = bucket;
                        iter->node = node;
                        iter->table = table;
                        return iter;
                }
        }
        
        return NULL;
}

static struct pgc_tbl *pgc_tbl_remove_n(
        struct pgc_tbl *table,
        struct pgc_tbl_n *node) 
{
        free(node->key);
        free(node);
        table->size -= 1;
        return table;
}

struct pgc_tbl *pgc_tbl_remove(
        struct pgc_tbl *table,
        const char *key)
{
        const size_t hash = pgc_tbl_hash(key);
        const size_t bucket = hash % table->capacity;
        struct pgc_tbl_n *node = table->nodes[bucket];

        if(!node) {
                return table;
        }

        if(!strcmp(node->key, key)) {
                table->nodes[bucket] = node->next;
                return pgc_tbl_remove_n(table, node);
        }

        struct pgc_tbl_n *pred = node;
        for(node = node->next; node; node = node->next) {
                if(!strcmp(node->key, key)) {
                        pred->next = node->next;
                        return pgc_tbl_remove_n(table, node);
                }
                pred = node;
        }

        return table;
}

struct pgc_tbl_i *pgc_tbl_begin(
        struct pgc_tbl *table,
        struct pgc_tbl_i *iter)
{
        for(size_t n = 0; n < table->capacity; ++n) {
                struct pgc_tbl_n *node = table->nodes[n];
                if(node) {
                        iter->bucket = n;
                        iter->node = node;
                        iter->table = table;
                        return iter;
                }
        }

        return NULL;
}

struct pgc_tbl_i *pgc_tbl_next(struct pgc_tbl_i *iter)
{
        struct pgc_tbl *table = iter->table;

        if(!iter->node) {
                return NULL;
        }

        struct pgc_tbl_n *node = iter->node->next;

        if(node) {
                iter->node = node;
                return iter;
        }

        for(size_t n = iter->bucket + 1; n < table->capacity; ++n) {
                node = table->nodes[n];
                if(node) {
                        iter->bucket = n;
                        iter->node = node;
                        return iter;
                }
        }

        return NULL;
}

void *pgc_tbl_value(struct pgc_tbl_i *iter)
{
        if(!iter->node) {
                return NULL;
        }
        return iter->node->value;
}

char *pgc_tbl_key(struct pgc_tbl_i *iter)
{
        if(!iter->node) {
                return NULL;
        }
        return iter->node->key;
}       

