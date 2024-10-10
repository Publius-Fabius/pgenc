#include "pgenc/lang.h"
#include "pgenc/table.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/** C Gen State */
struct pgc_lang_genst {
        FILE *out;
        const char *prefix;
        struct pgc_tbl *sets;
        size_t uid;
        const char *fname;
};

/** C Gen Id Tag */
enum pgc_lang_genid_tag {
        PGC_BLD_GENID_NUM,
        PGC_BLD_GENID_STR
};

/** C Gen Identifier */
struct pgc_lang_genid {
        enum pgc_lang_genid_tag tag;
        union {
                size_t num;
                char *str;
        } u;
};

static inline struct pgc_lang_genid *pgc_lang_genid_initnum(
        struct pgc_lang_genid *var, 
        const size_t num)
{
        var->tag = PGC_BLD_GENID_NUM;
        var->u.num = num;
        return var;
}

static inline struct pgc_lang_genid *pgc_lang_genid_initstr(
        struct pgc_lang_genid *var, 
        char *str)
{
        var->tag = PGC_BLD_GENID_STR;
        var->u.str = str;
        return var;
}

/**
 * Generate the statement dictionary as a C struct.
 * @stmts The list of statments.
 * @param st The gen state.
 * @return A potential error.
 */
// static sel_err_t pgc_lang_gen_dict(
//         struct pgc_ast_lst *stmts, 
//         struct pgc_lang_genst *st)
// {
//         fprintf(st->out, "struct %s \r\n{ \r\n", st->dict);

//         for(struct pgc_ast_lst *i = stmts; i; i = i->nxt) {
//                 char *name = pgc_ast_tostr(pgc_syn_getname(i->val));
//                 if(pgc_syn_typeof(i->val) == PGC_SYN_DEF) {
//                         continue;
//                 } else {
//                         fprintf(st->out, 
//                                 "%sstruct pgc_par *%s;\r\n", 
//                                 MARGIN, name);
//                 }
//         }

//         fputs("};\r\n", st->out);

//         return PGC_ERR_OK;
// }

/**
 * Scan the expression for any hook or call expressions and add their names
 * to the provided table.
 * @param syn the syntax.
 * @param tbl a name table
 * @return A possible error.
 */
static sel_err_t pgc_lang_gen_hooks_scan(
        struct pgc_ast *syn,
        struct pgc_tbl *tbl)
{
        char *name;
        struct pgc_tbl_i iter;
        switch(pgc_syn_typeof(syn)) {
                case PGC_SYN_AND:
                case PGC_SYN_OR:
                        pgc_lang_gen_hooks_scan(pgc_syn_getfst(syn), tbl);
                        pgc_lang_gen_hooks_scan(pgc_syn_getsnd(syn), tbl);
                        return PGC_ERR_OK;
                case PGC_SYN_REP:
                        pgc_lang_gen_hooks_scan(pgc_syn_getsubex(syn), tbl);
                        return PGC_ERR_OK;
                case PGC_SYN_CALL:
                        pgc_lang_gen_hooks_scan(pgc_syn_getvar(syn), tbl);
                        name = pgc_ast_tostr(pgc_syn_getfun(syn));
                        if(!pgc_tbl_lookup(tbl, &iter, name)) {
                                pgc_tbl_insert(tbl, name, (void*)1);
                        }
                        return PGC_ERR_OK;
                case PGC_SYN_HOOK:
                        name = pgc_ast_tostr(syn);
                        if(!pgc_tbl_lookup(tbl, &iter, name)) {
                                pgc_tbl_insert(tbl, name, (void*)0);
                        }
                        return PGC_ERR_OK;
                default: return PGC_ERR_OK;
        }
}

/**
 * Generate forward declarations for call and hook.
 */
static sel_err_t pgc_lang_gen_hooks(
        struct pgc_ast_lst *stmts, 
        struct pgc_lang_genst *st)
{
        struct pgc_tbl *tbl = pgc_tbl_create(8);

        for(struct pgc_ast_lst *i = stmts; i; i = i->nxt) {
                switch(pgc_syn_typeof(i->val)) {
                        case PGC_SYN_DEF:
                        case PGC_SYN_LET:
                                pgc_lang_gen_hooks_scan(
                                        pgc_syn_getexpr(i->val), 
                                        tbl);
                        default: break;
                }
        }

        struct pgc_tbl_i imem, *i;
        for(i = pgc_tbl_begin(tbl, &imem); i; i = pgc_tbl_next(i)) 
        {
                if(pgc_tbl_value(i)){
                        /* call declaration */
                        fprintf(st->out, 
                                "sel_err_t %s"
                                "(struct pgc_buf*," 
                                "void*, " 
                                "const struct pgc_par*); \r\n", 
                        pgc_tbl_key(i));
                } else {
                        /* hook declaration */
                        fprintf(st->out, 
                                "sel_err_t %s(struct pgc_buf*,void*); \r\n", 
                        pgc_tbl_key(i));
                }   
        }

        pgc_tbl_destroy(tbl);

        return PGC_ERR_OK;
}

/**
 * Generate getters.
 */
// static sel_err_t pgc_lang_gen_gets(
//         struct pgc_ast_lst *stmts, 
//         struct pgc_lang_genst *st)
// {
//         for(struct pgc_ast_lst *i = stmts; i; i = i->nxt) {
//                 char *name;
//                 switch(pgc_syn_typeof(i->val)) {
//                         case PGC_SYN_DEC:
//                         case PGC_SYN_LET:
//                         case PGC_SYN_SET:
//                                 name = pgc_ast_tostr(pgc_syn_getname(i->val));
//                                 fprintf(st->out,
//                                         "struct pgc_par *%s_%s(struct %s *x)"
//                                         "{\r\n"
//                                         "       return x->%s;\r\n"
//                                         "}\r\n",
//                                         st->dict, name, st->dict, name);
//                         default: break;
//                 }
//         }
//         return PGC_ERR_OK;
// }

static sel_err_t pgc_lang_gen_sig(
        struct pgc_lang_genst *st,
        struct pgc_lang_genid *var0, 
        const int depth)
{
        if(depth == 0) {
                SEL_IO(fprintf(st->out,
                        "const struct pgc_par %s_%s",
                        st->prefix, st->fname));
        } else {
                pgc_lang_genid_initnum(var0, st->uid++);
                SEL_IO(fprintf(st->out,
                        "static const struct pgc_par p%zx_p",
                        var0->u.num))
        }
        return PGC_ERR_OK;
}

static sel_err_t pgc_lang_gen_exp(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st,
        struct pgc_lang_genid *var,
        const int depth);

static sel_err_t pgc_lang_gen_id(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st,
        struct pgc_lang_genid *var,
        const int depth)
{
        if(depth == 0) {
                SEL_IO(fprintf(st->out, 
                        "static const pgc_par %s_%s = %s; \r\n",
                        st->prefix, st->fname, pgc_ast_tostr(syn)));
        } else {
                pgc_lang_genid_initstr(var, pgc_ast_tostr(syn));
        }
        return PGC_ERR_OK;
}

static sel_err_t pgc_lang_gen_lit(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st,
        struct pgc_lang_genid *var0,
        const int depth)
{
        char *lit = pgc_ast_tostr(syn);
        SEL_TRY_QUIETLY(pgc_lang_gen_sig(st, var0, depth));
        SEL_IO(fprintf(st->out, 
                " = PGC_PAR_CMP(\"%s\", %zu); \r\n",
                lit, strlen(lit)))
        return PGC_ERR_OK;
}

static sel_err_t pgc_lang_gen_char(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st,
        struct pgc_lang_genid *var0,
        const int depth)
{
        const uint8_t byte = (uint8_t)pgc_ast_toint8(syn);
        SEL_TRY_QUIETLY(pgc_lang_gen_sig(st, var0, depth));
        SEL_IO(fprintf(st->out, " = PGC_PAR_BYTE(%u); \r\n", byte));
        return PGC_ERR_OK;
}

static sel_err_t pgc_lang_genid_fprint(
        FILE *out,
        const char *prefix,
        struct pgc_lang_genid *var)
{
        switch(var->tag) {
                case PGC_BLD_GENID_STR:
                        SEL_IO(fprintf(out, "&%s_%s", prefix, var->u.str));
                        break;
                case PGC_BLD_GENID_NUM:
                        SEL_IO(fprintf(out, "&p%zx_p", var->u.num));
                        break;
                default: 
                        return SEL_ABORT();
        }
        return PGC_ERR_OK;
}

static sel_err_t pgc_lang_gen_and(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st,
        struct pgc_lang_genid *var0,
        const int depth)
{
        struct pgc_lang_genid var1;
        struct pgc_lang_genid var2;
        const int nd = depth + 1;
        SEL_TRY_QUIETLY(pgc_lang_gen_exp(pgc_syn_getfst(syn), st, &var1, nd));
        SEL_TRY_QUIETLY(pgc_lang_gen_exp(pgc_syn_getsnd(syn), st, &var2, nd));
        SEL_TRY_QUIETLY(pgc_lang_gen_sig(st, var0, depth));
        SEL_IO(fprintf(st->out, " = PGC_PAR_AND("));
        SEL_TRY_QUIETLY(pgc_lang_genid_fprint(st->out, st->prefix, &var1));
        SEL_IO(fputs(", ", st->out));
        SEL_TRY_QUIETLY(pgc_lang_genid_fprint(st->out, st->prefix, &var2));
        SEL_IO(fputs("); \r\n", st->out));
        return PGC_ERR_OK;
}

static sel_err_t pgc_lang_gen_or(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st,
        struct pgc_lang_genid *var0,
        const int depth)
{
        struct pgc_lang_genid var1;
        struct pgc_lang_genid var2;
        const int nd = depth + 1;
        SEL_TRY_QUIETLY(pgc_lang_gen_exp(pgc_syn_getfst(syn), st, &var1, nd));
        SEL_TRY_QUIETLY(pgc_lang_gen_exp(pgc_syn_getsnd(syn), st, &var2, nd));
        SEL_TRY_QUIETLY(pgc_lang_gen_sig(st, var0, depth));
        SEL_IO(fprintf(st->out, " = PGC_PAR_OR("));
        SEL_TRY_QUIETLY(pgc_lang_genid_fprint(st->out, st->prefix, &var1));
        SEL_IO(fputs(", ", st->out));
        SEL_TRY_QUIETLY(pgc_lang_genid_fprint(st->out, st->prefix, &var2));
        SEL_IO(fputs("); \r\n", st->out));
        return PGC_ERR_OK;
}

static sel_err_t pgc_lang_gen_rep(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st,
        struct pgc_lang_genid *var0,
        const int depth)
{
        struct pgc_ast *pair = pgc_syn_getrange(syn);
        const uint32_t min = pgc_ast_touint32(pgc_syn_getfst(pair));
        const uint32_t max = pgc_ast_touint32(pgc_syn_getsnd(pair));
        struct pgc_lang_genid var1;
        SEL_TRY_QUIETLY(pgc_lang_gen_exp(
                pgc_syn_getsubex(syn), st, &var1, depth + 1));
        SEL_TRY_QUIETLY(pgc_lang_gen_sig(st, var0, depth));
        SEL_IO(fprintf(st->out, " = PGC_PAR_REP("));
        SEL_TRY_QUIETLY(pgc_lang_genid_fprint(st->out, st->prefix, &var1));
        SEL_IO(fprintf(st->out, ", %u, %u); \r\n", min, max));
        return PGC_ERR_OK;
}

static sel_err_t pgc_lang_gen_utf(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st,
        struct pgc_lang_genid *var0,
        const int depth)
{
        const uint32_t min = pgc_ast_touint32(pgc_syn_getfst(syn));
        const uint32_t max = pgc_ast_touint32(pgc_syn_getsnd(syn));
        SEL_TRY_QUIETLY(pgc_lang_gen_sig(st, var0, depth));
        SEL_IO(fprintf(st->out,
                " = PGC_PAR_UTF8(%u, %u); \r\n",
                min, max));
        return PGC_ERR_OK;
}

static sel_err_t pgc_lang_gen_hook(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st,
        struct pgc_lang_genid *var0,
        const int depth)
{
        SEL_TRY_QUIETLY(pgc_lang_gen_sig(st, var0, depth));
        SEL_IO(fprintf(st->out, 
                " = PGC_PAR_HOOK(%s); \r\n",
                pgc_ast_tostr(syn)));
        return PGC_ERR_OK;
}

static sel_err_t pgc_lang_gen_call(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st,
        struct pgc_lang_genid *var0,
        const int depth)
{
        char *name = pgc_ast_tostr(pgc_syn_getfun(syn));
        struct pgc_lang_genid var1;
        const int nd = depth + 1;
        SEL_TRY_QUIETLY(pgc_lang_gen_exp(pgc_syn_getvar(syn), st, &var1, nd));
        SEL_TRY_QUIETLY(pgc_lang_gen_sig(st, var0, depth));
        SEL_IO(fprintf(st->out, " = PGC_PAR_CALL(%s, ", name));
        SEL_TRY_QUIETLY(pgc_lang_genid_fprint(st->out, st->prefix, &var1));
        SEL_IO(fprintf(st->out, "); \r\n"));
        return PGC_ERR_OK;
}

static sel_err_t pgc_lang_gen_exp(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st,
        struct pgc_lang_genid *var,
        const int depth)
{
        switch(pgc_syn_typeof(syn)) {
                case PGC_SYN_ID: 
                        return pgc_lang_gen_id(syn, st, var, depth);
                case PGC_SYN_LIT: 
                        return pgc_lang_gen_lit(syn, st, var, depth);
                case PGC_SYN_CHAR: 
                        return pgc_lang_gen_char(syn, st, var, depth);
                case PGC_SYN_AND: 
                        return pgc_lang_gen_and(syn, st, var, depth);
                case PGC_SYN_OR: 
                        return pgc_lang_gen_or(syn, st, var, depth);
                case PGC_SYN_REP: 
                        return pgc_lang_gen_rep(syn, st, var, depth);
                case PGC_SYN_HOOK: 
                        return pgc_lang_gen_hook(syn, st, var, depth);
                case PGC_SYN_UTF: 
                        return pgc_lang_gen_utf(syn, st, var, depth);
                case PGC_SYN_CALL: 
                        return pgc_lang_gen_call(syn, st, var, depth);
                default: 
                        return SEL_ABORT();
        }
}

static sel_err_t pgc_lang_gen_setexp(
        struct pgc_ast *syn,
        struct pgc_tbl *tbl,
        struct pgc_cset *set);

static pgc_cset_pred_t pgc_lang_gen_setpred(const char *name)
{
        if(!strcmp(name, "isalnum")) {
                return pgc_cset_isalnum;
        } else if(!strcmp(name, "isalpha")) {
                return pgc_cset_isalpha;
        } else if(!strcmp(name, "iscntrl")) {
                return pgc_cset_iscntrl;
        } else if(!strcmp(name, "isdigit")) {
                return pgc_cset_isdigit;
        } else if(!strcmp(name, "isgraph")) {
                return pgc_cset_isgraph;
        } else if(!strcmp(name, "islower")) {
                return pgc_cset_islower;
        } else if(!strcmp(name, "isprint")) {
                return pgc_cset_isprint;
        } else if(!strcmp(name, "ispunct")) {
                return pgc_cset_ispunct;
        } else if(!strcmp(name, "isspace")) {
                return pgc_cset_isspace;
        } else if(!strcmp(name, "isupper")) {
                return pgc_cset_isupper;
        } else if(!strcmp(name, "isxdigit")) {
                return pgc_cset_isxdigit;
        } else {
                return (pgc_cset_pred_t)0;
        }
}

static sel_err_t pgc_lang_gen_setid(
        struct pgc_ast *syn,
        struct pgc_tbl *tbl,
        struct pgc_cset *set)
{
        char *name = pgc_ast_tostr(syn);
        pgc_cset_pred_t pred = pgc_lang_gen_setpred(name);
        if(pred) {
                pgc_cset_iso(set, pred);
                return PGC_ERR_OK;
        } else {
                struct pgc_tbl_i imem, *i;
                i = pgc_tbl_lookup(tbl, &imem, name);
                if(!i) {
                        return SEL_REPORT(PGC_ERR_SYN);
                }
                pgc_cset_cpy(set, (struct pgc_cset*)pgc_tbl_value(i));
                return PGC_ERR_OK;
        }
}

static sel_err_t pgc_lang_gen_setbyte(
        struct pgc_ast *syn,
        struct pgc_cset *set)
{
        pgc_cset_zero(set);
        pgc_cset_set(set, (uint8_t)pgc_ast_toint8(syn));
        return PGC_ERR_OK;
}

static sel_err_t pgc_lang_gen_union(
        struct pgc_ast *syn,
        struct pgc_tbl *tbl,
        struct pgc_cset *set)
{
        struct pgc_cset left;
        struct pgc_cset right;
        SEL_TRY_QUIETLY(pgc_lang_gen_setexp(pgc_syn_getfst(syn), tbl, &left));
        SEL_TRY_QUIETLY(pgc_lang_gen_setexp(pgc_syn_getsnd(syn), tbl, &right));
        pgc_cset_union(set, &left, &right);
        return PGC_ERR_OK;
}

static sel_err_t pgc_lang_gen_diff(
        struct pgc_ast *syn,
        struct pgc_tbl *tbl,
        struct pgc_cset *set)
{
        struct pgc_cset left;
        struct pgc_cset right;
        SEL_TRY_QUIETLY(pgc_lang_gen_setexp(pgc_syn_getfst(syn), tbl, &left));
        SEL_TRY_QUIETLY(pgc_lang_gen_setexp(pgc_syn_getsnd(syn), tbl, &right));
        pgc_cset_diff(set, &left, &right);
        return PGC_ERR_OK;
}

static sel_err_t pgc_lang_gen_setexp(
        struct pgc_ast *syn,
        struct pgc_tbl *tbl,
        struct pgc_cset *set)
{
        switch(pgc_syn_typeof(syn)) {
                case PGC_SYN_ID: 
                        return pgc_lang_gen_setid(syn, tbl, set);
                case PGC_SYN_CHAR: 
                        return pgc_lang_gen_setbyte(syn, set);
                case PGC_SYN_UNION: 
                        return pgc_lang_gen_union(syn, tbl, set);
                case PGC_SYN_DIFF: 
                        return pgc_lang_gen_diff(syn, tbl, set);
                default: 
                        return SEL_ABORT();
        }
}

static sel_err_t pgc_lang_gen_dec(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st)
{
        char *name = pgc_ast_tostr(pgc_syn_getname(syn));
        SEL_IO(fprintf(st->out,
                "extern const struct pgc_par %s_%s; \r\n", 
                st->prefix, name));
        return PGC_ERR_OK;
}

static sel_err_t pgc_lang_gen_def(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st)
{
        char *name = pgc_ast_tostr(pgc_syn_getname(syn));
        st->fname = name;
        struct pgc_lang_genid var;
        SEL_TRY_QUIETLY(pgc_lang_gen_exp(pgc_syn_getexpr(syn), st, &var, 0));
        return PGC_ERR_OK;
}

static sel_err_t pgc_lang_gen_let(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st)
{
        char *name = pgc_ast_tostr(pgc_syn_getname(syn));
        st->fname = name;
        struct pgc_lang_genid var;
        SEL_TRY_QUIETLY(pgc_lang_gen_exp(pgc_syn_getexpr(syn), st, &var, 0));
        return PGC_ERR_OK;
}

static sel_err_t pgc_lang_gen_set_try(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st,
        struct pgc_cset *set)
{
        char *name = pgc_ast_tostr(pgc_syn_getname(syn));

        SEL_TRY_QUIETLY(pgc_lang_gen_setexp(
                pgc_syn_getexpr(syn), st->sets, set));
        
        const size_t c1 = st->uid++;

        SEL_IO(fprintf(st->out, 
                "static const struct pgc_cset p%zx_p = { .words = { 0x%x ",
                c1, set->words[0]));
        for(int x = 1; x < 8; ++x) {
                SEL_IO(fprintf(st->out, ", 0x%x", set->words[x]));
        }
        SEL_IO(fputs(" } }; \r\n", st->out));

        SEL_IO(fprintf(st->out, 
               "const struct pgc_par %s_%s = PGC_PAR_SET(&p%zx_p); \r\n",
                st->prefix, name, c1));
        
        pgc_tbl_insert(st->sets, name, set);

        return PGC_ERR_OK;
}

static sel_err_t pgc_lang_gen_set(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st)
{
        struct pgc_cset *set = malloc(sizeof(struct pgc_cset));
        const sel_err_t err = pgc_lang_gen_set_try(syn, st, set);
        if(err != PGC_ERR_OK) {
                free(set);
                return err;
        }
        return PGC_ERR_OK;
}

static sel_err_t pgc_lang_gen_stmt(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st)
{
        switch(pgc_syn_typeof(syn)) {
                case PGC_SYN_DEC: return pgc_lang_gen_dec(syn, st);
                case PGC_SYN_DEF: return pgc_lang_gen_def(syn, st);
                case PGC_SYN_LET: return pgc_lang_gen_let(syn, st);
                case PGC_SYN_SET: return pgc_lang_gen_set(syn, st);
                default: SEL_ABORT();
        }
        return PGC_ERR_OK;
}

// static sel_err_t pgc_lang_gen_result(
//         struct pgc_ast_lst *lst, 
//         struct pgc_lang_genst *st)
// {
//         SEL_IO(fprintf(st->out, 
//                 "%sstatic struct %s result;\r\n", 
//                 MARGIN, st->dict));
//         for(struct pgc_ast_lst *l = lst; l; l = l->nxt) {
//                 struct pgc_ast *stmt = l->val;
//                 if(!stmt) {
//                         SEL_ABORT();
//                 }
//                 char *name = pgc_ast_tostr(pgc_syn_getname(stmt));
//                 switch(pgc_syn_typeof(stmt)) {
//                         case PGC_SYN_DEC:
//                         case PGC_SYN_LET:
//                         case PGC_SYN_SET: SEL_IO(
//                                 fprintf(st->out, 
//                                 "%sresult.%s = %s;\r\n",
//                                 MARGIN, name, name));
//                         case PGC_SYN_DEF: break;
//                         default: SEL_ABORT();
//                 }
//         }
//         SEL_IO(fprintf(st->out, "%sreturn &result;\r\n", MARGIN));
//         return PGC_ERR_OK;
// }

static sel_err_t pgc_lang_gen_stmts_try(
        struct pgc_ast_lst *lst,
        struct pgc_lang_genst *st)
{
        // SEL_IO(fprintf(st->out,
        //         "struct %s *export_%s()\r\n{\r\n",
        //         st->dict, st->dict));
        for(struct pgc_ast_lst *l = lst; l; l = l->nxt) {
                SEL_TRY_QUIETLY(pgc_lang_gen_stmt(l->val, st));
        }
        // PGC_TRACE(pgc_lang_gen_result(lst, st));
        // SEL_IO(fputs("}\r\n", st->out));
        return PGC_ERR_OK;
}

static sel_err_t pgc_lang_gen_stmts(
        struct pgc_ast_lst *lst,
        struct pgc_lang_genst *st)
{
        st->sets = pgc_tbl_create(8);
        const sel_err_t err = pgc_lang_gen_stmts_try(lst, st);
        struct pgc_tbl_i imem, *i;
        for(i = pgc_tbl_begin(st->sets, &imem); i; i = pgc_tbl_next(i)) {
                free(pgc_tbl_value(i));
        }
        pgc_tbl_destroy(st->sets);
        return err;
}

sel_err_t pgc_lang_gen(
        FILE *out,
        struct pgc_ast_lst *lst,
        const char *prefix)
{
        struct pgc_lang_genst st = { 
                .uid = 0, 
                .prefix = prefix, 
                .out = out,
                .sets = NULL };
        SEL_IO(fputs("#include \"pgenc/parser.h\"\r\n", st.out));
        SEL_TRY_QUIETLY(pgc_lang_gen_hooks(lst, &st));
        SEL_TRY_QUIETLY(pgc_lang_gen_stmts(lst, &st));
        return PGC_ERR_OK;
}
