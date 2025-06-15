
#include "pgenc/parser.h"

#include <string.h>

struct pgc_par_choice_frame {
        size_t step, offset;
};

struct pgc_par_and_frame {
        char step;
};

struct pgc_stk *pgc_par_push(struct pgc_stk *stk, const struct pgc_par *par)
{
        struct pgc_par_choice_frame *cf;
        struct pgc_par_and_frame *af;
        const struct pgc_par **ptr;
        switch(par->tag) {
                case PGC_PAR_BYTE: 
                case PGC_PAR_UTF8:
                case PGC_PAR_SET:
                case PGC_PAR_CMP: 
                case PGC_PAR_HOOK: 
                case PGC_PAR_CALL:
                        if(!(ptr = pgc_stk_push(stk, sizeof(*ptr))))
                                return NULL;
                        *ptr = par;
                        return stk;
                case PGC_PAR_AND: 
                        if(!(af = pgc_stk_push(stk, sizeof(*af)))) 
                                return NULL;
                        af->step = 0;
                        if(!(ptr = pgc_stk_push(stk, sizeof(*ptr)))) {
                                SEL_TEST(pgc_stk_pop(stk, sizeof(*af)));
                                return NULL;
                        }
                        *ptr = par;
                        return stk;
                case PGC_PAR_OR: 
                case PGC_PAR_REP: 
                        if(!(cf = pgc_stk_push(stk, sizeof(*cf))))
                                return NULL;
                        cf->step = cf->offset = 0;
                        if(!(ptr = pgc_stk_push(stk, sizeof(*ptr)))) {
                                SEL_TEST(pgc_stk_pop(stk, sizeof(*cf)));
                                return NULL;
                        }
                        *ptr = par;
                        return stk;
                default: 
                        SEL_HALT();
        }
        return NULL;
}

struct pgc_stk *pgc_par_pop_term(struct pgc_stk *stk)
{
        if(!pgc_stk_pop(stk, sizeof(void*))) {
                return NULL;
        }
        return stk;
}

struct pgc_stk *pgc_par_pop_choice(struct pgc_stk *stk)
{
        if(!pgc_stk_pop(stk, sizeof(void*))) {
                return NULL;
        } else if(!pgc_stk_pop(stk, sizeof(struct pgc_par_choice_frame))) {
                return NULL;
        }
        return stk;
}

struct pgc_stk *pgc_par_pop_and(struct pgc_stk *stk)
{
        if(!pgc_stk_pop(stk, sizeof(void*))) {
                return NULL;
        } else if(!pgc_stk_pop(stk, sizeof(struct pgc_par_and_frame))) {
                return NULL;
        }
        return stk;
}

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

static sel_err_t pgc_par_run_and(
        const struct pgc_par *par,
        struct pgc_stk *stk,
        const sel_err_t eax)
{
        struct pgc_par_and_frame *frame = pgc_stk_peek(stk, sizeof(par));
        SEL_TEST(frame);
        switch(frame->step) {
                case 0:
                        if(!pgc_par_push(stk, par->u.pair.arg1)) {
                                SEL_TEST(pgc_par_pop_and(stk));
                                return PGC_ERR_OOB;
                        }
                        frame->step = 1;
                        return PGC_ERR_OK;
                case 1:
                        SEL_TEST(pgc_par_pop_and(stk));
                        if(eax != PGC_ERR_OK) 
                                return eax;
                        else if(!pgc_par_push(stk, par->u.pair.arg2))
                                return PGC_ERR_OOB;
                        return PGC_ERR_OK;
                default:
                        return SEL_HALT();   
        }
}

static sel_err_t pgc_par_run_or(
        const struct pgc_par *par,
        struct pgc_stk *stk,
        struct pgc_buf *buf,
        const sel_err_t eax)
{
        struct pgc_par_choice_frame *frame = pgc_stk_peek(stk, sizeof(par));
        SEL_TEST(frame);
        size_t offset;
        switch(frame->step) {
                case 0:
                        if(!pgc_par_push(stk, par->u.pair.arg1)) {
                                SEL_TEST(pgc_par_pop_choice(stk));
                                return PGC_ERR_OOB;
                        }
                        frame->offset = pgc_buf_tell(buf);
                        frame->step = 1;
                        return PGC_ERR_OK;
                case 1:
                        offset = frame->offset;
                        SEL_TEST(pgc_par_pop_choice(stk));
                        if(eax == PGC_ERR_OK) 
                                return PGC_ERR_OK;
                        else if(eax != PGC_ERR_CMP && eax != PGC_ERR_OOB) 
                                return eax;
                        else if(pgc_buf_seek(buf, offset) != PGC_ERR_OK)
                                return PGC_ERR_OOB;
                        else if(!pgc_par_push(stk, par->u.pair.arg2))
                                return PGC_ERR_OOB;
                        return PGC_ERR_OK;
                default: 
                        return SEL_HALT();
        }
}

static sel_err_t pgc_par_run_rep(
        const struct pgc_par *par,
        struct pgc_stk *stk,
        struct pgc_buf *buf,
        const sel_err_t res)
{
        struct pgc_par_choice_frame *frame = pgc_stk_peek(stk, sizeof(par));
        SEL_TEST(frame);
        const uint32_t max = par->u.trip.max;
        const uint32_t min = par->u.trip.min;
        SEL_ASSERT(min <= max);
        if(frame->step == 0) {
                goto NEXT;
        } else if(res != PGC_ERR_OK) {
                const size_t offset = frame->offset;
                SEL_TEST(pgc_par_pop_choice(stk))
                if(res != PGC_ERR_CMP && res != PGC_ERR_OOB) {
                        return res;
                } else if(min < frame->step) {
                        if(pgc_buf_seek(buf, offset) != PGC_ERR_OK) 
                                return PGC_ERR_OOB;
                        return PGC_ERR_OK;
                } else {
                        return res;
                }
        } else if(frame->step == max) {
                SEL_TEST(pgc_par_pop_choice(stk));
                return PGC_ERR_OK;
        } 
        NEXT:
        if(!pgc_par_push(stk, par->u.trip.sub)) {
                SEL_TEST(pgc_par_pop_choice(stk));
                return PGC_ERR_OOB;
        }
        frame->offset = pgc_buf_tell(buf);
        frame->step += 1;
        return PGC_ERR_OK;
}

sel_err_t pgc_par_run_ex(
        const struct pgc_par *parser,
        struct pgc_stk *stk,
        struct pgc_buf *buf,
        void *state)
{
        SEL_ASSERT(parser);
        const struct pgc_par **ptr;
        sel_err_t res = PGC_ERR_EOF;
        const size_t base = pgc_stk_offset(stk);
        if(!pgc_par_push(stk, parser)) 
                return PGC_ERR_OOB;
        while((ptr = pgc_stk_peek(stk, 0)) && pgc_stk_offset(stk) < base) {
                const struct pgc_par *p = *ptr;
                switch(p->tag) {
                        case PGC_PAR_BYTE: 
                                SEL_TEST(pgc_par_pop_term(stk));
                                res = pgc_par_run_byte(p, buf);
                                break;
                        case PGC_PAR_UTF8:
                                SEL_TEST(pgc_par_pop_term(stk));
                                res = pgc_par_run_utf8(p, buf);
                                break;
                        case PGC_PAR_SET:
                                SEL_TEST(pgc_par_pop_term(stk));
                                res = pgc_par_run_set(p, buf);
                                break;
                        case PGC_PAR_CMP: 
                                SEL_TEST(pgc_par_pop_term(stk));
                                res = pgc_par_run_cmp(p, buf);
                                break;
                        case PGC_PAR_HOOK: 
                                SEL_TEST(pgc_par_pop_term(stk));
                                res = pgc_par_run_hook(p, buf, state);
                                break;
                        case PGC_PAR_CALL:
                                SEL_TEST(pgc_par_pop_term(stk));
                                res = pgc_par_run_call(p, stk, buf, state);
                                break;
                        case PGC_PAR_AND: 
                                res = pgc_par_run_and(p, stk, res);
                                break;
                        case PGC_PAR_OR: 
                                res = pgc_par_run_or(p, stk, buf, res);
                                break;
                        case PGC_PAR_REP: 
                                res = pgc_par_run_rep(p, stk, buf, res);
                                break;
                        default: 
                                return SEL_HALT();
                }
        }
        return res;
}

sel_err_t pgc_par_run(
        const struct pgc_par *par, 
        struct pgc_buf *buf,
        void *state)
{
        char base[2048];
        struct pgc_stk stk; pgc_stk_init(&stk, base, 2048);
        return pgc_par_run_ex(par, &stk, buf, state);
}

intptr_t pgc_par_runs(
        const struct pgc_par *par,
        const char *str,
        void *state)
{
        char base[2048];
        struct pgc_stk stk; pgc_stk_init(&stk, base, 2048);
        return pgc_par_runs_ex(par, &stk, str, state);
}

intptr_t pgc_par_runs_ex(
        const struct pgc_par *par,
        struct pgc_stk *stk,
        const char *str,
        void *state)
{
        const size_t len = strlen(str);
        struct pgc_buf buf; pgc_buf_init(&buf, (void*)str, strlen(str), 0);
        buf.end = len;
        sel_err_t e = pgc_par_run_ex(par, stk, &buf, state);
        if(e == PGC_ERR_OK) {
                return (intptr_t)pgc_buf_tell(&buf);
        } else {
                return e;
        }
}