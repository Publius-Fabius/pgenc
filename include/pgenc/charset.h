#ifndef PGENC_CHARSET_H
#define PGENC_CHARSET_H

#include <stddef.h>
#include <stdint.h>

/** A 256bit character set used for efficient string parsing. */
struct pgc_cset 
{
        uint32_t words[8];                      /** Bits */
};

/**
 * Initialize the set to zero.
 * @param set The set to make zero.
 * @return set
 **/
extern struct pgc_cset *pgc_cset_zero(struct pgc_cset *set);

/**
 * Copy all bits from src to dest.
 * @param dest The destination set.
 * @param src The source set.
 * @return dest
 **/
extern struct pgc_cset *pgc_cset_cpy(
        struct pgc_cset *dest, 
        struct pgc_cset *src);

/**
 * Is the element a member of the character set?
 * @param set The character set that may contain the element.
 * @param ele The element that may be a member of the set.
 * @return A value other than zero indicates 'ele' is in 'set'.
 **/
extern int pgc_cset_in(struct pgc_cset *set,  const int ele);

/**
 * Add an element to the character set.
 * @param set The character set that will now include the element.
 * @param element The character we want to include in the set.
 * @return set.
 **/
extern struct pgc_cset *pgc_cset_set(
        struct pgc_cset *set, 
        const int element);

/**
 * Remove an element from the character set.
 * @param set The character set that will now exclude the element.
 * @param ele The character to exclude from the set.
 * @return set
 **/
extern struct pgc_cset *pgc_cset_unset(
        struct pgc_cset *set, 
        const int ele);

/** Type for predicate functions defined in ctype.h. */
typedef int (*pgc_cset_pred_t)(int value);

/**
 * Construct a set that is isomorphic to the predicate.
 * @param dest The set that will be made isomorphic to the predicate.
 * @param pred The predicate.
 * @return dest
 **/
extern struct pgc_cset *pgc_cset_iso(
        struct pgc_cset *dest,
        pgc_cset_pred_t pred);

/**
 * Construct a set that is the intersection of the two sets.
 * @param dest The destination set.
 * @param first The first set.
 * @param second The second set.
 * @return dest
 **/
extern struct pgc_cset *pgc_cset_isect(
        struct pgc_cset *dest, 
        struct pgc_cset *first, 
        struct pgc_cset *second);

/**
 * Construct a set that is the union of the two sets.
 * @param dest The destination set.
 * @param first The first set.
 * @param second The second set.
 * @return dest
 **/
extern struct pgc_cset *pgc_cset_union(
        struct pgc_cset *dest,
        struct pgc_cset *first, 
        struct pgc_cset *second);

/**
 * Construct a set that is the difference of two sets. Note that this function 
 * is not commutative, ie law_cset_dif(a, b) != law_cset_dif(b, a).
 * @param dest The destination set.
 * @param first The first set.
 * @param second The second set.
 * @return dest
 **/
extern struct pgc_cset *pgc_cset_diff(
        struct pgc_cset *dest,
        struct pgc_cset *first, 
        struct pgc_cset *second);

#endif