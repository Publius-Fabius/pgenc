
#include "pgenc/parser.h"
#include <string.h>

static int pgc_par_run_byte_match(const void *ptr, const uint8_t c)
{
        return *((const int*)ptr) == c;
}

static sel_err_t pgc_par_run_byte(
        const struct pgc_par *par, 
        struct pgc_buf *buf)
{
        return pgc_buf_match(buf, pgc_par_run_byte_match, &par->u.byte);
}

static int pgc_par_run_utf8_match(const void *ptr, const uint32_t c)
{
        const uint32_t *range = (const uint32_t*)ptr;
        return range[0] <= c && c <= range[1];
}

static sel_err_t pgc_par_run_utf8(
        const struct pgc_par *par,
        struct pgc_buf *buf)
{
        uint32_t range[2];
        range[0] = par->u.trip.min;
        range[1] = par->u.trip.max;
        return pgc_buf_matchutf8(buf, pgc_par_run_utf8_match, range);
}

static int pgc_par_run_set_match(const void *set, const uint8_t c) 
{
        return pgc_cset_in(set, c);
}

static sel_err_t pgc_par_run_set(
        const struct pgc_par *par,
        struct pgc_buf *buf)
{
        return pgc_buf_match(buf, pgc_par_run_set_match, par->u.set);
}

static sel_err_t pgc_par_run_cmp(
        const struct pgc_par *par,
        struct pgc_buf *buf)
{
        return pgc_buf_cmp(buf, par->u.str.val, par->u.str.len);
}

static sel_err_t pgc_par_run_and(
        const struct pgc_par *par,
        struct pgc_buf *buf,
        void *state)
{
        do {
                /* Perform parse of arg1. */
                const sel_err_t err = pgc_par_run(
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

        } while(par->tag == PGC_PAR_AND);

        /* Iterator fell through type check, return last parse. */
        return pgc_par_run(par, buf, state);
}

static sel_err_t pgc_par_run_or(
        const struct pgc_par *par,
        struct pgc_buf *buf,
        void *state)
{
        do {
                /* Copy the offset. */
                const size_t buf_offset = pgc_buf_tell(buf);

                /* Perform the parse of arg1. */
                const sel_err_t err = pgc_par_run(
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

        } while(par->tag == PGC_PAR_OR);

        /* Iterator fell through, offset is restored and ready to go. */
        return pgc_par_run(par, buf, state);
}

static sel_err_t pgc_par_run_rep(
        const struct pgc_par *par,
        struct pgc_buf *buf,
        void *state)
{
        const uint32_t min = par->u.trip.min;           /** Min reps */
        const uint32_t max = par->u.trip.max;           /** Max reps */
        const struct pgc_par *sub = par->u.trip.sub;    /** Sub-parser */

        /* Values for min and max need to make sense. */
        SEL_ASSERT((0 <= min) && (min <= max));
        
        /* Copy the current state. */
        size_t buf_offset = pgc_buf_tell(buf);

        /* Pre-increment n for sanity. */
        for(int n = 1; ; ++n) {

                /* Perform the parse of the sub-parser. */
                const sel_err_t err = pgc_par_run(sub, buf, state);

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
        return SEL_HALT();
}

static sel_err_t pgc_par_run_hook(
        const struct pgc_par *par,
        struct pgc_buf *buf,
        void *state)
{
        return par->u.hook(buf, state);
}

static sel_err_t pgc_par_run_call(
        const struct pgc_par *par,
        struct pgc_stk *stk,
        struct pgc_buf *buf,
        void *state)
{
        return par->u.call.fun(stk, buf, state, par->u.call.var);
}

sel_err_t pgc_par_run(
        const struct pgc_par *par,
        struct pgc_buf *buf,
        void *state)
{
        SEL_ASSERT(par);
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
                /* ERROR!!! */
                        return pgc_par_run_call(par, NULL, buf, state);
                default: 
                        return SEL_HALT();
        };
}

ssize_t pgc_par_runs(
        const struct pgc_par *par,
        const char *str,
        void *st)
{
        const size_t len = strlen(str);
        struct pgc_buf buf; pgc_buf_init(&buf, str, strlen(str), 0);
        buf.end = len;
        sel_err_t e = pgc_par_run(par, &buf, st);
        if(e == PGC_ERR_OK) {
                return (ssize_t)pgc_buf_tell(&buf);
        } else {
                return e;
        }
}