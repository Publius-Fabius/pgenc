
#include "pgenc/charset.h"
#include <ctype.h>

struct pgc_cset *pgc_cset_zero(struct pgc_cset *set)
{
        for(int n = 0; n < 8; ++n) {
                set->words[n] = 0;
        }
        return set;
}

struct pgc_cset *pgc_cset_cpy(struct pgc_cset *dest, struct pgc_cset *src)
{
        for(int n = 0; n < 8; ++n) {
                dest->words[n] = src->words[n];
        }
        return dest;
}

int pgc_cset_in(
        const struct pgc_cset *set, 
        const uint8_t ele)
{
        const uint32_t c = ele;
        const uint32_t word = c >> 5;
        const uint32_t bit = 1 << (c & 31);
        return (set->words[word] & bit) != 0;
}

struct pgc_cset *pgc_cset_set(
        struct pgc_cset *set, 
        const uint8_t ele)
{
        const uint32_t c = ele;
        const uint32_t word = c >> 5;
        const uint32_t bit = 1 << (c & 31);
        set->words[word] |= bit;
        return set;
}

struct pgc_cset *pgc_cset_unset(
        struct pgc_cset *set, 
        const uint8_t element)
{
        const uint32_t c = element;
        const uint32_t word = c >> 5;
        const uint32_t bit = 1 << (c & 31);
        const uint32_t mask = ~bit;
        set->words[word] &= mask;
        return set;
}

#define PGC_CSET_FOR(VAR) for(uint16_t VAR = 0; VAR < 0x100; ++VAR)

struct pgc_cset *pgc_cset_iso(
        struct pgc_cset *set,
        pgc_cset_pred_t pred)
{
        pgc_cset_zero(set);
        PGC_CSET_FOR(c) {
                if(pred((uint8_t)c)) {
                        pgc_cset_set(set, (uint8_t)c);
                }
        }
        return set;
}

struct pgc_cset *pgc_cset_isect(
        struct pgc_cset *set, 
        struct pgc_cset *fst, 
        struct pgc_cset *snd)
{
        pgc_cset_zero(set);
        PGC_CSET_FOR(c) {
                if(     pgc_cset_in(fst, (uint8_t)c) && 
                        pgc_cset_in(snd, (uint8_t)c)) 
                {
                        pgc_cset_set(set, (uint8_t)c);
                }
        }
        return set;
}

struct pgc_cset *pgc_cset_union(
        struct pgc_cset *set,
        struct pgc_cset *fst, 
        struct pgc_cset *snd)
{
        pgc_cset_zero(set);
        PGC_CSET_FOR(c) {
                if(     pgc_cset_in(fst, (uint8_t)c) || 
                        pgc_cset_in(snd, (uint8_t)c)) 
                {
                        pgc_cset_set(set, (uint8_t)c);
                }
        }
        return set;
}

struct pgc_cset *pgc_cset_diff(
        struct pgc_cset *set,
        struct pgc_cset *fst, 
        struct pgc_cset *snd)
{
        pgc_cset_zero(set);
        PGC_CSET_FOR(c) {
                if(     pgc_cset_in(fst, (uint8_t)c) && 
                        !pgc_cset_in(snd, (uint8_t)c)) 
                {
                        pgc_cset_set(set, (uint8_t)c);
                }
        }
        return set;
}

#define PGC_CSET_CTYPE(fun) \
int pgc_cset_ ## fun (const uint8_t c) { \
        return fun(c); \
}

PGC_CSET_CTYPE(isalnum)
PGC_CSET_CTYPE(isalpha)
PGC_CSET_CTYPE(isblank)
PGC_CSET_CTYPE(iscntrl)
PGC_CSET_CTYPE(isdigit)
PGC_CSET_CTYPE(isgraph)
PGC_CSET_CTYPE(islower)
PGC_CSET_CTYPE(isprint)
PGC_CSET_CTYPE(ispunct)
PGC_CSET_CTYPE(isspace)
PGC_CSET_CTYPE(isupper)
PGC_CSET_CTYPE(isxdigit)
