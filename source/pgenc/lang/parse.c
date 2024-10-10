#include "pgenc/lang.h"
#include "pgenc/table.h"
#include <stdlib.h>
#include <string.h>

struct pgc_lang_parst {
        struct pgc_stk *alloc;
        struct pgc_ast_lst *list;
        int16_t utag;
};

/**
 * A utility function that will attempt to push the stack, returning 
 * PGC_ERR_OOM if push fails.
 * @param VAR The variable to assign.
 * @param STK The stack to push.
 * @param NBYTES The number of bytes to push.
 */
#define PGC_STK_PUSH(VAR, STK, NBYTES) \
        VAR = pgc_stk_push(STK, NBYTES); \
        if(!(VAR)) { \
                return PGC_ERR_OOM; \
        }

/**
 * A utility function that will attempt to pop the stack, returning 
 * PGC_ERR_OOB if pop fails.
 * @param VAR The pointer to assign.
 * @param STK The stack to pop.
 * @param NBYTES The number of bytes to pop.
 */
#define PGC_STK_POP(VAR, STK, NBYTES) \
        VAR = pgc_stk_pop(STK, NBYTES); \
        if(!(VAR)) { \
                return PGC_ERR_OOB; \
        }

sel_err_t pgc_lang_parse(
        const struct pgc_par *par,
        struct pgc_buf *buf,
        struct pgc_stk *alloc,
        struct pgc_ast_lst **syn)
{
        struct pgc_lang_parst parst;
        parst.alloc = alloc;
        parst.list = NULL;
        SEL_TRY_QUIETLY(pgc_par_run(par, buf, &parst));
        *syn = pgc_ast_rev(parst.list);
        return PGC_ERR_OK;
}

sel_err_t pgc_lang_readchar(
        struct pgc_buf *buf, 
        void *st,
        const size_t offset,
        const int16_t tag)
{
        struct pgc_lang_parst *parst = st;
        char result;

        SEL_TRY_QUIETLY(pgc_buf_seek(buf, offset));
        SEL_TRY_QUIETLY(pgc_buf_getchar(buf, &result));

        struct pgc_ast *node;
        PGC_STK_PUSH(node, parst->alloc, sizeof(struct pgc_ast));

        struct pgc_ast_lst *head;
        PGC_STK_PUSH(head, parst->alloc, sizeof(struct pgc_ast_lst));

        head->val = pgc_ast_initint8(node, result, tag);
        head->nxt = parst->list;
        parst->list = head;

        return PGC_ERR_OK;
}

sel_err_t pgc_lang_readstr(
        struct pgc_buf *buf, 
        void *state,
        const size_t start,
        const int16_t tag)
{
        struct pgc_lang_parst *parst = state;
        const size_t stop = pgc_buf_tell(buf);

        SEL_ASSERT(start <= stop);
      
        const size_t len = stop - start;
        char *str;
        PGC_STK_PUSH(str, parst->alloc, len + 1);
        str[len] = 0;
        
        SEL_TRY_QUIETLY(pgc_buf_seek(buf, start));
        SEL_TRY_QUIETLY(pgc_buf_get(buf, str, len));

        struct pgc_ast *node;
        PGC_STK_PUSH(node, parst->alloc, sizeof(struct pgc_ast));
        struct pgc_ast_lst *head;
        PGC_STK_PUSH(head, parst->alloc, sizeof(struct pgc_ast_lst));

        head->val = pgc_ast_initstr(node, str, tag);
        head->nxt = parst->list;
        parst->list = head;

        return PGC_ERR_OK;
}

sel_err_t pgc_lang_readutf8(
        struct pgc_buf *buf, 
        void *state,
        const size_t start,
        const int16_t tag)
{
        struct pgc_lang_parst *parst = state;
        const size_t stop = pgc_buf_tell(buf);

        SEL_ASSERT(start <= stop);

        uint32_t value;

        SEL_TRY_QUIETLY(pgc_buf_seek(buf, start));
        SEL_TRY_QUIETLY(pgc_buf_getutf8(buf, &value));

        struct pgc_ast *node;
        PGC_STK_PUSH(node, parst->alloc, sizeof(struct pgc_ast));
        struct pgc_ast_lst *head;
        PGC_STK_PUSH(head, parst->alloc, sizeof(struct pgc_ast_lst));

        head->val = pgc_ast_inituint32(node, value, tag);
        head->nxt = parst->list;
        parst->list = head;

        return PGC_ERR_OK;
}

sel_err_t pgc_lang_readenc(
        struct pgc_buf *buf,
        void *state,
        const size_t start,
        const int16_t atag,
        const int16_t utag,
        const size_t base,
        pgc_buf_decode_dict_t dict,
        pgc_buf_decode_accum_t accum)
{
        struct pgc_lang_parst *parst = state;

        const size_t stop = pgc_buf_tell(buf);

        SEL_ASSERT(start <= stop);

        struct pgc_ast *node;
        PGC_STK_PUSH(node, parst->alloc, sizeof(struct pgc_ast));
        pgc_ast_initany(node, atag, utag);

        SEL_TRY_QUIETLY(pgc_buf_seek(buf, start));
        SEL_TRY_QUIETLY(pgc_buf_decode(
                buf, stop - start, base, dict, accum, &node->u.any));

        struct pgc_ast_lst *head;
        PGC_STK_PUSH(head, parst->alloc, sizeof(struct pgc_ast_lst));

        head->val = node;
        head->nxt = parst->list;
        parst->list = head;

        return PGC_ERR_OK;
}

void pgc_lang_setutag(void *state, const int16_t tag)
{
        struct pgc_lang_parst *parst = state;
        parst->utag = tag;
}

sel_err_t pgc_lang_readexp(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *parser,
        const int16_t utag,
        enum pgc_lang_readop ops)
{
        struct pgc_lang_parst *parst = state;

        /*  Save allocator's size. */
        const size_t saved_size = pgc_stk_size(parst->alloc);

        /* Save the list. */
        struct pgc_ast_lst *saved_list = parst->list;

        /* Provide the sub-parser with an empty list. */
        parst->list = NULL;

        /* Run the sub-parser. */
        sel_err_t err = pgc_par_run(parser, buffer, state);

        /* Handle a parsing failure. */
        if(err != PGC_ERR_OK) {  

                const size_t current_size = pgc_stk_size(parst->alloc);

                SEL_ASSERT(saved_size <= current_size);
            
                /* Restore the allocator's state. */
                pgc_stk_pop(parst->alloc, current_size - saved_size);

                /* Restore the list state. */
                parst->list = saved_list;

                return err;
        } 

        /* Behavior for merging option. */
        if(ops & PGC_BLD_MERGE) {
                /* There's nothing to do when the list is empty. */
                if(!parst->list) {
                        return PGC_ERR_OK;
                } 
                /* If the list is a singleton, simply merge it in. */
                else if(pgc_ast_len(parst->list) == 1) {
                        parst->list->nxt = saved_list;
                        return PGC_ERR_OK;
                } 
        }

        struct pgc_ast *node;
        PGC_STK_PUSH(node, parst->alloc, sizeof(struct pgc_ast));
        
        struct pgc_ast_lst *head;
        PGC_STK_PUSH(head, parst->alloc, sizeof(struct pgc_ast_lst));

        /* Use internal utag when polymorphic behavior is specified. */
        const int16_t type = ops & PGC_BLD_POLY ? 
                parst->utag :
                utag;

        pgc_ast_initlst(node, pgc_ast_rev(parst->list), type);

        head->val = node;
        head->nxt = saved_list;
        parst->list = head;

        return PGC_ERR_OK;
}

sel_err_t pgc_lang_readterm(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *arg,
        const int16_t utag,
        sel_err_t (*reader)(struct pgc_buf*, void*, const size_t, int16_t))
{
        /* Mark the offset. */
        const size_t offset = pgc_buf_tell(buffer);

        /* Run the sub-parser. */
        SEL_TRY_QUIETLY(pgc_par_run(arg, buffer, state));

        /* Read the value. */
        return reader(buffer, state, offset, utag);
}

sel_err_t pgc_lang_capid(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *arg)
{
        return pgc_lang_readterm(
                buffer, state, arg, PGC_SYN_ID, pgc_lang_readstr);
}

static sel_err_t pgc_lang_readhex8(
        struct pgc_buf *buffer,
        void *state,
        const size_t offset,
        int16_t tag)
{
        return pgc_lang_readenc(
                buffer, 
                state, 
                offset,
                PGC_AST_INT8, 
                tag, 
                16, 
                pgc_buf_decode_hex,
                pgc_buf_decode_uint8);
}

sel_err_t pgc_lang_capxbyte(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *arg)
{
        return pgc_lang_readterm(
                buffer, state, arg, PGC_SYN_CHAR, pgc_lang_readhex8);
}

sel_err_t pgc_lang_capchar(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *arg)
{
        return pgc_lang_readterm(
                buffer, state, arg, PGC_SYN_CHAR, pgc_lang_readchar);
}

static sel_err_t pgc_lang_readuint32(
        struct pgc_buf *buffer,
        void *state,
        const size_t offset,
        int16_t tag)
{
        return pgc_lang_readenc(
                buffer, 
                state, 
                offset,
                PGC_AST_UINT32, 
                tag, 
                10, 
                pgc_buf_decode_dec,
                pgc_buf_decode_uint32);
}

sel_err_t pgc_lang_capnum(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *arg)
{
        return pgc_lang_readterm(
                buffer, state, arg, PGC_SYN_NUM, pgc_lang_readuint32);
}

sel_err_t pgc_lang_caprange(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *parser)
{
        return pgc_lang_readexp(buffer, state, parser, PGC_SYN_RANGE, 0);
}

static sel_err_t pgc_lang_readhex32(
        struct pgc_buf *buffer,
        void *state,
        const size_t offset,
        int16_t tag)
{
        return pgc_lang_readenc(
                buffer, 
                state, 
                offset,
                PGC_AST_UINT32, 
                tag, 
                16, 
                pgc_buf_decode_hex,
                pgc_buf_decode_uint32);
}

sel_err_t pgc_lang_caputf(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *arg)
{
        return pgc_lang_readterm(
                buffer, state, arg, PGC_SYN_NUM, pgc_lang_readhex32);
}

sel_err_t pgc_lang_caputfrange(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *parser)
{
        return pgc_lang_readexp(buffer, state, parser, PGC_SYN_UTF, 0);
}

sel_err_t pgc_lang_setunion(struct pgc_buf *buffer, void *state)
{
        pgc_lang_setutag(state, PGC_SYN_UNION);
        return PGC_ERR_OK;
}

sel_err_t pgc_lang_setdiff(struct pgc_buf *buffer, void *state)
{
        pgc_lang_setutag(state, PGC_SYN_DIFF);
        return PGC_ERR_OK;
}

sel_err_t pgc_lang_capsetexp(
        struct pgc_buf *buffer, 
        void *state, 
        const struct pgc_par *arg)
{
        return pgc_lang_readexp(
                buffer, state, arg, 0, PGC_BLD_POLY | PGC_BLD_MERGE);
}

sel_err_t pgc_lang_setrep(struct pgc_buf *buffer, void *state)
{
        pgc_lang_setutag(state, PGC_SYN_REP);
        return PGC_ERR_OK;
}

sel_err_t pgc_lang_setand(struct pgc_buf *buffer, void *state)
{
        pgc_lang_setutag(state, PGC_SYN_AND);
        return PGC_ERR_OK;
}

sel_err_t pgc_lang_setor(struct pgc_buf *buffer, void *state)
{
        pgc_lang_setutag(state, PGC_SYN_OR);
        return PGC_ERR_OK;
}

sel_err_t pgc_lang_setcall(struct pgc_buf *buffer, void *state)
{
        pgc_lang_setutag(state, PGC_SYN_CALL);
        return PGC_ERR_OK;
}

sel_err_t pgc_lang_caprep(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *arg)
{
        return pgc_lang_readexp(buffer, state, arg, PGC_SYN_REP, 0);
}

sel_err_t pgc_lang_caphook(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *arg)
{
        return pgc_lang_readterm(
                buffer, state, arg, PGC_SYN_HOOK, pgc_lang_readstr);
}

sel_err_t pgc_lang_capexp(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *arg)
{
        return pgc_lang_readexp(
                buffer, state, arg, 0, PGC_BLD_POLY | PGC_BLD_MERGE);
}

sel_err_t pgc_lang_setdec(struct pgc_buf *buffer, void *state)
{
        pgc_lang_setutag(state, PGC_SYN_DEC);
        return PGC_ERR_OK;
}

sel_err_t pgc_lang_setdef(struct pgc_buf *buffer, void *state)
{
        pgc_lang_setutag(state, PGC_SYN_DEF);
        return PGC_ERR_OK;
}


sel_err_t pgc_lang_setset(struct pgc_buf *buffer, void *state)
{
        pgc_lang_setutag(state, PGC_SYN_SET);
        return PGC_ERR_OK;
}

sel_err_t pgc_lang_setlet(struct pgc_buf *buffer, void *state)
{
        pgc_lang_setutag(state, PGC_SYN_LET);
        return PGC_ERR_OK;
}

sel_err_t pgc_lang_capstmt(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *arg)
{
        return pgc_lang_readexp(
                buffer, state, arg, 0, PGC_BLD_POLY);
}
