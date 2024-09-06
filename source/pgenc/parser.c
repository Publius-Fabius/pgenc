
#include "pgenc/parser.h"
#include <string.h>
#include <assert.h>

struct pgc_par *pgc_par_cmp(
        struct pgc_par *par,
        char *str, 
        const size_t len)
{
        par->tag = PGC_PAR_CMP;
        par->u.str.val = str;
        par->u.str.len = len;
        return par;
}

struct pgc_par *pgc_par_byte(
        struct pgc_par *par,
        const int byte)
{
        par->tag = PGC_PAR_BYTE;
        par->u.byte = byte;
        return par;
}

struct pgc_par *pgc_par_utf8(
        struct pgc_par *par,
        const uint32_t min,
        const uint32_t max)
{
        par->tag = PGC_PAR_UTF8;
        par->u.trip.min = min;
        par->u.trip.max = max;
        return par;
}

struct pgc_par *pgc_par_set(
        struct pgc_par *par,
        struct pgc_cset *set)
{
        par->tag = PGC_PAR_SET;
        par->u.set = set;
        return par;
}

struct pgc_par *pgc_par_and(
        struct pgc_par *par,
        struct pgc_par *fst,
        struct pgc_par *snd)
{
        par->tag = PGC_PAR_AND;
        par->u.pair.arg1 = fst;
        par->u.pair.arg2 = snd;
        return par;
}

struct pgc_par *pgc_par_or(
        struct pgc_par *par,
        struct pgc_par *inl,
        struct pgc_par *inr)
{
        par->tag = PGC_PAR_OR;
        par->u.pair.arg1 = inl;
        par->u.pair.arg2 = inr;
        return par;
}

struct pgc_par *pgc_par_rep(
        struct pgc_par *par,
        struct pgc_par *sub,
        const uint32_t min,
        const uint32_t max)
{
        par->tag = PGC_PAR_REP;
        par->u.trip.sub = sub;
        par->u.trip.min = min;
        par->u.trip.max = max;
        return par;
}

struct pgc_par *pgc_par_hook(
        struct pgc_par *par,
        pgc_par_hook_t callback)
{
        par->tag = PGC_PAR_HOOK;
        par->u.hook = callback;
        return par;
}

struct pgc_par *pgc_par_lnk(
        struct pgc_par *par,
        struct pgc_par *lnk)
{
        par->tag = PGC_PAR_LNK;
        par->u.lnk = lnk;
        return par;
}

struct pgc_par *pgc_par_call(
        struct pgc_par *par,
        pgc_par_call_t callback,
        struct pgc_par *var)
{
        par->tag = PGC_PAR_CALL;
        par->u.call.fun = callback;
        par->u.call.var = var;
        return par;
}

static int pgc_par_run_byte_match(void *ptr, const int c)
{
        return *(char*)ptr == c;
}

static enum pgc_err pgc_par_run_byte(
        struct pgc_par *par, 
        struct pgc_buf *buf)
{
        return pgc_buf_match(buf, pgc_par_run_byte_match, &par->u.byte);
}

static int pgc_par_run_utf8_match(void *ptr, const uint32_t c)
{
        uint32_t *range = (uint32_t*)ptr;
        return range[0] <= c && c <= range[1];
}

static enum pgc_err pgc_par_run_utf8(
        struct pgc_par *par,
        struct pgc_buf *buf)
{
        uint32_t range[2];
        range[0] = par->u.trip.min;
        range[1] = par->u.trip.max;
        return pgc_buf_matchutf8(buf, pgc_par_run_utf8_match, range);
}

static int pgc_par_run_set_match(void *set, const int c) 
{
        return pgc_cset_in(set, c);
}

static enum pgc_err pgc_par_run_set(
        struct pgc_par *par,
        struct pgc_buf *buf)
{
        return pgc_buf_match(buf, pgc_par_run_set_match, par->u.set);
}

static enum pgc_err pgc_par_run_cmp(
        struct pgc_par *par,
        struct pgc_buf *buf)
{
        return pgc_buf_cmp(buf, par->u.str.val, par->u.str.len);
}

static enum pgc_err pgc_par_run_and(
        struct pgc_par *par,
        struct pgc_buf *buf,
        void *state)
{
        do {
                /* Perform parse of arg1. */
                const enum pgc_err err = pgc_par_run(
                        par->u.pair.arg1, 
                        buf, 
                        state);

                if(err != PGC_ERR_OK) {
                        /* Anything other than OK is returned. */
                        return err;
                } else {
                        /* Advance par iterator to arg2. */
                        par = par->u.pair.arg2;
                }

                /* When arg2 is a product, associative laws allow for a short
                   jump here. */
        } while(par->tag == PGC_PAR_AND);

        /* Iterator fell through type check, return last parse. */
        return pgc_par_run(par, buf, state);
}

static enum pgc_err pgc_par_run_or(
        struct pgc_par *par,
        struct pgc_buf *buf,
        void *state)
{
        do {
                /* Copy the offset. */
                const size_t buf_offset = pgc_buf_tell(buf);

                /* Perform the parse of arg1. */
                const enum pgc_err err = pgc_par_run(
                        par->u.pair.arg1, 
                        buf, 
                        state);

                if(err == PGC_ERR_CMP || err == PGC_ERR_OOB) {
                        /* Restore the offset. */
                        pgc_buf_seek(buf, buf_offset);

                        /* Advance iterator to arg2. */
                        par = par->u.pair.arg2;
                } else {
                        /* Anything other than cmp/oob is returned. */
                        return err;
                }

                /* When arg2 is a coproduct, the associative laws allow
                   for a short jump here. */
        } while(par->tag == PGC_PAR_OR);

        /* Iterator fell through, offset is restored and ready to go. */
        return pgc_par_run(par, buf, state);
}

static enum pgc_err pgc_par_run_rep(
        struct pgc_par *par,
        struct pgc_buf *buf,
        void *state)
{
        const uint32_t min = par->u.trip.min;           /** Min reps */
        const uint32_t max = par->u.trip.max;           /** Max reps */
        struct pgc_par *sub = par->u.trip.sub;          /** Sub-parser */

        /* Values for min and max need to make sense. */
        assert((0 <= min) && (min <= max));
        
        /* Copy the current state. */
        size_t buf_offset = pgc_buf_tell(buf);

        /* Pre-increment n for sanity. */
        for(int n = 1; ; ++n) {

                /* Perform the parse of the sub-parser. */
                const enum pgc_err err = pgc_par_run(sub, buf, state);

                if(err == PGC_ERR_OK) {
                        if(n >= max) {
                                /* Maximum constraint met, return OK. */
                                return PGC_ERR_OK;
                        } else {
                                /* Copy the current state and continue. */
                                buf_offset = pgc_buf_tell(buf);
                                continue;
                        }
                } else if(err != PGC_ERR_CMP && err != PGC_ERR_OOB) {
                        /* Errors other than cmp/oob fall through. */
                        return err;
                } else if(n <= min) {
                        /* Minimum constraint not met. */
                        return err;
                } else {
                        /* Last parse failed, but met minimum constraint. */
                        pgc_buf_seek(buf, buf_offset);
                        return PGC_ERR_OK;
                }
        }
        return PGC_ABORT();
}

static enum pgc_err pgc_par_run_hook(
        struct pgc_par *par,
        struct pgc_buf *buf,
        void *state)
{
        return par->u.hook(buf, state);
}

static enum pgc_err pgc_par_run_call(
        struct pgc_par *par,
        struct pgc_buf *buf,
        void *state)
{
        return par->u.call.fun(buf, state, par->u.call.var);
}

static enum pgc_err pgc_par_run_lnk(
        struct pgc_par *par,
        struct pgc_buf *buf,
        void *state)
{
        return pgc_par_run(par->u.lnk, buf, state);
}

enum pgc_err pgc_par_run(
        struct pgc_par *par,
        struct pgc_buf *buf,
        void *state)
{
        assert(par);
        switch(par->tag) {
                case PGC_PAR_BYTE: 
                        return pgc_par_run_byte(par, buf);
                case PGC_PAR_UTF8:
                        return pgc_par_run_utf8(par, buf);
                case PGC_PAR_SET: 
                        return pgc_par_run_set(par, buf);
                case PGC_PAR_CMP: 
                        return pgc_par_run_cmp(par, buf);
                case PGC_PAR_AND: 
                        return pgc_par_run_and(par, buf, state);
                case PGC_PAR_OR: 
                        return pgc_par_run_or(par, buf, state);
                case PGC_PAR_REP: 
                        return pgc_par_run_rep(par, buf, state);
                case PGC_PAR_HOOK: 
                        return pgc_par_run_hook(par, buf, state);
                case PGC_PAR_CALL:
                        return pgc_par_run_call(par, buf, state);
                case PGC_PAR_LNK: 
                        return pgc_par_run_lnk(par, buf, state);
                default: return PGC_ABORT();
        };
}

ssize_t pgc_par_runs(
        struct pgc_par *par,
        char *str,
        void *st)
{
        const size_t len = strlen(str);
        struct pgc_buf buf; pgc_buf_init(&buf, str, strlen(str), 0);
        buf.end = len;
        enum pgc_err e = pgc_par_run(par, &buf, st);
        if(e == PGC_ERR_OK) {
                return (ssize_t)pgc_buf_tell(&buf);
        } else {
                return e;
        }
}