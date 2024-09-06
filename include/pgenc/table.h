#ifndef PGENC_TABLE_H
#define PGENC_TABLE_H

#include <stddef.h>

/** Hashtable */
struct pgc_tbl;

/** Hashtable Node */
struct pgc_tbl_n;

/** Hashtable Iterator */
struct pgc_tbl_i
{
        struct pgc_tbl *table;
        struct pgc_tbl_n *node;
        size_t bucket;
};

/**
 * Creates a new pgc_tbl with the given initial capacity.
 * 
 * @param capacity Initial capacity of the table. 
 * @return Pointer to the created table.
*/  
extern struct pgc_tbl *pgc_tbl_create(const size_t capacity);

/** 
 * Destroys the given table, freeing all memory.
 * 
 * @param table The table to destroy.
*/
extern void pgc_tbl_destroy(struct pgc_tbl *table);

/**
 * Gets the number of entries in the table.
 * 
 * @param table The table.
 * @return Number of entries.
*/
extern size_t pgc_tbl_size(struct pgc_tbl *table);

/**
 * Gets the capacity of the table.
 * 
 * @param table The table. 
 * @return Capacity of the table.
*/
extern size_t pgc_tbl_capacity(struct pgc_tbl *table);

/**
 * Inserts a key-value pair into the table.
 * 
 * @param table The table.
 * @param key The key to insert.
 * @param value The value to insert.
 * @return The table pointer.
*/
extern struct pgc_tbl *pgc_tbl_insert(
        struct pgc_tbl *table, 
        const char *key, 
        void *value);

/**
 * Looks up a key and returns an iterator to the entry.
 * 
 * @param table The table.
 * @param iter The iterator to (maybe) initialize.
 * @param key The key to look up.
 * @return An iterator to the found entry, or end iterator if not found.
*/ 
extern struct pgc_tbl_i *pgc_tbl_lookup(
        struct pgc_tbl *table,
        struct pgc_tbl_i *iter,
        const char *key);

/**
 * Removes an element matching the given key.
 * 
 * @param table The table.
 * @param key The key of the element to remove.
 * @return The table pointer.
*/
extern struct pgc_tbl *pgc_tbl_remove(
        struct pgc_tbl *table,
        const char *key);

/**
 * Returns an iterator to the beginning of the table.
 * 
 * @param table The table.
 * @param iter The iterator to (maybe) initialize.
 * @return Iterator to the first entry.
*/
extern struct pgc_tbl_i *pgc_tbl_begin(
        struct pgc_tbl *table,
        struct pgc_tbl_i *iter);

/**
 * Advances the iterator to the next element.
 * 
 * @param iter The iterator.
 * @return Next iterator.
*/  
extern struct pgc_tbl_i *pgc_tbl_next(struct pgc_tbl_i *iter);

/**
 * Gets the value of the entry for the iterator.
 *
 * @param iter The iterator. 
 * @return The value.
*/
extern void *pgc_tbl_value(struct pgc_tbl_i *iter);


/**
 * Gets the key of the entry for the iterator.
 *
 * @param iter The iterator.
 * @return The key. 
*/
extern char *pgc_tbl_key(struct pgc_tbl_i *iter);

#endif