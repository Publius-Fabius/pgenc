#include "pgenc/lang.h"
#include "pgenc/table.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/** C Gen State */
struct pgc_lang_genst {
        FILE *out;
        const char *dict;
        struct pgc_tbl *sets;
        size_t uid;
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

static const char MARGIN[] = "        ";

/**
 * Generate the statement dictionary as a C struct.
 * @stmts The list of statments.
 * @param st The gen state.
 * @return A potential error.
 */
static enum pgc_err pgc_lang_gen_dict(
        struct pgc_ast_lst *stmts, 
        struct pgc_lang_genst *st)
{
        fprintf(st->out, "struct %s \r\n{ \r\n", st->dict);

        for(struct pgc_ast_lst *i = stmts; i; i = i->nxt) {
                char *name = pgc_ast_tostr(pgc_syn_getname(i->val));
                if(pgc_syn_typeof(i->val) == PGC_SYN_DEF) {
                        continue;
                } else {
                        fprintf(st->out, 
                                "%sstruct pgc_par *%s;\r\n", 
                                MARGIN, name);
                }
        }

        fputs("};\r\n", st->out);

        return PGC_ERR_OK;
}

/**
 * Scan the expression for any hook or call expressions and add their names
 * to the provided table.
 * @param syn the syntax.
 * @param tbl a name table
 * @return A possible error.
 */
static enum pgc_err pgc_lang_gen_hooks_scan(
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
static enum pgc_err pgc_lang_gen_hooks(
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
                        "extern enum pgc_err %s"
                        "(struct pgc_buf*,void*,struct pgc_par*);\r\n", 
                        pgc_tbl_key(i));
                } else {
                        /* hook declaration */
                        fprintf(st->out, 
                        "extern enum pgc_err %s(struct pgc_buf*,void*);\r\n", 
                        pgc_tbl_key(i));
                }   
        }

        pgc_tbl_destroy(tbl);

        return PGC_ERR_OK;
}

/**
 * Generate getters.
 */
static enum pgc_err pgc_lang_gen_gets(
        struct pgc_ast_lst *stmts, 
        struct pgc_lang_genst *st)
{
        for(struct pgc_ast_lst *i = stmts; i; i = i->nxt) {
                char *name;
                switch(pgc_syn_typeof(i->val)) {
                        case PGC_SYN_DEC:
                        case PGC_SYN_LET:
                        case PGC_SYN_SET:
                                name = pgc_ast_tostr(pgc_syn_getname(i->val));
                                fprintf(st->out,
                                        "struct pgc_par *%s_%s(struct %s *x)"
                                        "{\r\n"
                                        "       return x->%s;\r\n"
                                        "}\r\n",
                                        st->dict, name, st->dict, name);
                        default: break;
                }
        }
        return PGC_ERR_OK;
}

static enum pgc_err pgc_lang_gen_exp(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st,
        struct pgc_lang_genid *var);

static enum pgc_err pgc_lang_gen_id(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st,
        struct pgc_lang_genid *var)
{
        pgc_lang_genid_initstr(var, pgc_ast_tostr(syn));
        return PGC_ERR_OK;
}

static enum pgc_err pgc_lang_gen_lit(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st,
        struct pgc_lang_genid *var)
{
        char *lit = pgc_ast_tostr(syn);
        pgc_lang_genid_initnum(var, st->uid++);
        PGC_IO(fprintf(st->out, 
                "%sstatic struct pgc_par p%zx_p; "
                "pgc_par_cmp(&p%zx_p, \"%s\", %zu);\r\n", 
                MARGIN, var->u.num, var->u.num, lit, strlen(lit)))
        return PGC_ERR_OK;
}

static enum pgc_err pgc_lang_gen_char(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st,
        struct pgc_lang_genid *var)
{
        const char byte = pgc_ast_toint8(syn);
        pgc_lang_genid_initnum(var, st->uid++);
        PGC_IO(fprintf(st->out, 
                "%sstatic struct pgc_par p%zx_p; "
                "pgc_par_byte(&p%zx_p, (uint8_t)%i);\r\n", 
                MARGIN, var->u.num, var->u.num, byte));
        return PGC_ERR_OK;
}

static enum pgc_err pgc_lang_genid_fprint(
        FILE *out,
        struct pgc_lang_genid *var)
{
        switch(var->tag) {
                case PGC_BLD_GENID_STR:
                        PGC_IO(fputs(var->u.str, out));
                        break;
                case PGC_BLD_GENID_NUM:
                        PGC_IO(fprintf(out, "&p%zx_p", var->u.num));
                        break;
                default: return PGC_ABORT();
        }
        return PGC_ERR_OK;
}

static enum pgc_err pgc_lang_gen_and(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st,
        struct pgc_lang_genid *var0)
{
        pgc_lang_genid_initnum(var0, st->uid++);
        struct pgc_lang_genid var1;
        struct pgc_lang_genid var2;
        pgc_lang_gen_exp(pgc_syn_getfst(syn), st, &var1);
        pgc_lang_gen_exp(pgc_syn_getsnd(syn), st, &var2);
        PGC_IO(fprintf(st->out, 
                "%sstatic struct pgc_par p%zx_p; "
                "pgc_par_and(&p%zx_p, ",
                MARGIN, var0->u.num, var0->u.num));
        PGC_TRACE(pgc_lang_genid_fprint(st->out, &var1));
        PGC_IO(fputs(", ", st->out));
        PGC_TRACE(pgc_lang_genid_fprint(st->out, &var2));
        PGC_IO(fputs(");\r\n", st->out));
        return PGC_ERR_OK;
}

static enum pgc_err pgc_lang_gen_or(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st,
        struct pgc_lang_genid *var0)
{
        pgc_lang_genid_initnum(var0, st->uid++);
        struct pgc_lang_genid var1;
        struct pgc_lang_genid var2;
        pgc_lang_gen_exp(pgc_syn_getfst(syn), st, &var1);
        pgc_lang_gen_exp(pgc_syn_getsnd(syn), st, &var2);
        PGC_IO(fprintf(st->out, 
                "%sstatic struct pgc_par p%zx_p; "
                "pgc_par_or(&p%zx_p, ",
                MARGIN, var0->u.num, var0->u.num));
        PGC_TRACE(pgc_lang_genid_fprint(st->out, &var1));
        PGC_IO(fputs(", ", st->out));
        PGC_TRACE(pgc_lang_genid_fprint(st->out, &var2));
        PGC_IO(fputs(");\r\n", st->out));
        return PGC_ERR_OK;
}

static enum pgc_err pgc_lang_gen_rep(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st,
        struct pgc_lang_genid *var0)
{
        struct pgc_ast *pair = pgc_syn_getrange(syn);
        const uint32_t min = pgc_ast_touint32(pgc_syn_getfst(pair));
        const uint32_t max = pgc_ast_touint32(pgc_syn_getsnd(pair));
        pgc_lang_genid_initnum(var0, st->uid++);
        struct pgc_lang_genid var1;
        PGC_TRACE(pgc_lang_gen_exp(pgc_syn_getsubex(syn), st, &var1));
        PGC_IO(fprintf(st->out,
                "%sstatic struct pgc_par p%zx_p; "
                "pgc_par_rep(&p%zx_p, ",
                MARGIN, var0->u.num, var0->u.num));
        PGC_TRACE(pgc_lang_genid_fprint(st->out, &var1));
        PGC_IO(fprintf(st->out, ", %u, %u);\r\n", min, max));
        return PGC_ERR_OK;
}

static enum pgc_err pgc_lang_gen_utf(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st,
        struct pgc_lang_genid *var0)
{
        const uint32_t min = pgc_ast_touint32(pgc_syn_getfst(syn));
        const uint32_t max = pgc_ast_touint32(pgc_syn_getsnd(syn));
        pgc_lang_genid_initnum(var0, st->uid++);
        PGC_IO(fprintf(st->out,
                "%sstatic struct pgc_par p%zx_p;"
                "pgc_par_utf8(&p%zx_p, %u, %u); \r\n",
                MARGIN, var0->u.num, var0->u.num, min, max));
        return PGC_ERR_OK;
}

static enum pgc_err pgc_lang_gen_hook(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st,
        struct pgc_lang_genid *var0)
{
        pgc_lang_genid_initnum(var0, st->uid++);
        PGC_IO(fprintf(st->out, 
                "%sstatic struct pgc_par p%zx_p; "
                "pgc_par_hook(&p%zx_p, %s);\r\n", 
                MARGIN, var0->u.num, var0->u.num, pgc_ast_tostr(syn)));
        return PGC_ERR_OK;
}

static enum pgc_err pgc_lang_gen_call(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st,
        struct pgc_lang_genid *var0)
{
        pgc_lang_genid_initnum(var0, st->uid++);
        struct pgc_lang_genid var1;
        pgc_lang_gen_exp(pgc_syn_getvar(syn), st, &var1);
        char *name = pgc_ast_tostr(pgc_syn_getfun(syn));
        PGC_IO(fprintf(st->out, 
                "%sstatic struct pgc_par p%zx_p; "
                "pgc_par_call(&p%zx_p, %s, ", 
                MARGIN, var0->u.num, var0->u.num, name));
        PGC_TRACE(pgc_lang_genid_fprint(st->out, &var1));
        PGC_IO(fprintf(st->out, ");\r\n"));
        return PGC_ERR_OK;
}


static enum pgc_err pgc_lang_gen_exp(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st,
        struct pgc_lang_genid *var)
{
        switch(pgc_syn_typeof(syn)) {
                case PGC_SYN_ID: return pgc_lang_gen_id(syn, st, var);
                case PGC_SYN_LIT: return pgc_lang_gen_lit(syn, st, var);
                case PGC_SYN_CHAR: return pgc_lang_gen_char(syn, st, var);
                case PGC_SYN_AND: return pgc_lang_gen_and(syn, st, var);
                case PGC_SYN_OR: return pgc_lang_gen_or(syn, st, var);
                case PGC_SYN_REP: return pgc_lang_gen_rep(syn, st, var);
                case PGC_SYN_HOOK: return pgc_lang_gen_hook(syn, st, var);
                case PGC_SYN_UTF: return pgc_lang_gen_utf(syn, st, var);
                case PGC_SYN_CALL: return pgc_lang_gen_call(syn, st, var);
                default: return PGC_ABORT();
        }
}

static enum pgc_err pgc_lang_gen_setexp(
        struct pgc_ast *syn,
        struct pgc_tbl *tbl,
        struct pgc_cset *set);

static pgc_cset_pred_t pgc_lang_gen_setpred(const char *name)
{
        if(!strcmp(name, "isalnum")) {
                return isalnum;
        } else if(!strcmp(name, "isalpha")) {
                return isalpha;
        } else if(!strcmp(name, "iscntrl")) {
                return iscntrl;
        } else if(!strcmp(name, "isdigit")) {
                return isdigit;
        } else if(!strcmp(name, "isgraph")) {
                return isgraph;
        } else if(!strcmp(name, "islower")) {
                return islower;
        } else if(!strcmp(name, "isprint")) {
                return isprint;
        } else if(!strcmp(name, "ispunct")) {
                return ispunct;
        } else if(!strcmp(name, "isspace")) {
                return isspace;
        } else if(!strcmp(name, "isupper")) {
                return isupper;
        } else if(!strcmp(name, "isxdigit")) {
                return isxdigit;
        } else {
                return (pgc_cset_pred_t)0;
        }
}

static enum pgc_err pgc_lang_gen_setid(
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
                        PGC_THROW(PGC_ERR_SYN);
                }
                pgc_cset_cpy(set, (struct pgc_cset*)pgc_tbl_value(i));
                return PGC_ERR_OK;
        }
}

static enum pgc_err pgc_lang_gen_setbyte(
        struct pgc_ast *syn,
        struct pgc_cset *set)
{
        pgc_cset_zero(set);
        pgc_cset_set(set, pgc_ast_toint8(syn));
        return PGC_ERR_OK;
}

static enum pgc_err pgc_lang_gen_union(
        struct pgc_ast *syn,
        struct pgc_tbl *tbl,
        struct pgc_cset *set)
{
        struct pgc_cset left;
        struct pgc_cset right;
        PGC_TRACE(pgc_lang_gen_setexp(pgc_syn_getfst(syn), tbl, &left));
        PGC_TRACE(pgc_lang_gen_setexp(pgc_syn_getsnd(syn), tbl, &right));
        pgc_cset_union(set, &left, &right);
        return PGC_ERR_OK;
}

static enum pgc_err pgc_lang_gen_diff(
        struct pgc_ast *syn,
        struct pgc_tbl *tbl,
        struct pgc_cset *set)
{
        struct pgc_cset left;
        struct pgc_cset right;
        PGC_TRACE(pgc_lang_gen_setexp(pgc_syn_getfst(syn), tbl, &left));
        PGC_TRACE(pgc_lang_gen_setexp(pgc_syn_getsnd(syn), tbl, &right));
        pgc_cset_diff(set, &left, &right);
        return PGC_ERR_OK;
}

static enum pgc_err pgc_lang_gen_setexp(
        struct pgc_ast *syn,
        struct pgc_tbl *tbl,
        struct pgc_cset *set)
{
        switch(pgc_syn_typeof(syn)) {
                case PGC_SYN_ID: return pgc_lang_gen_setid(syn, tbl, set);
                case PGC_SYN_CHAR: return pgc_lang_gen_setbyte(syn, set);
                case PGC_SYN_UNION: return pgc_lang_gen_union(syn, tbl, set);
                case PGC_SYN_DIFF: return pgc_lang_gen_diff(syn, tbl, set);
                default: return PGC_ABORT();
        }
}

static enum pgc_err pgc_lang_gen_dec(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st)
{
        char *name = pgc_ast_tostr(pgc_syn_getname(syn));
        size_t c1 = st->uid++;
        PGC_IO(fprintf(st->out,
                "%sstatic struct pgc_par p%zx_p; "
                "pgc_par_lnk(&p%zx_p, NULL);\r\n"
                "%sstatic struct pgc_par *%s = &p%zx_p;\r\n",
                MARGIN, c1, c1, MARGIN, name, c1));
        return PGC_ERR_OK;
}

static enum pgc_err pgc_lang_gen_def(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st)
{
        char *name = pgc_ast_tostr(pgc_syn_getname(syn));
        struct pgc_lang_genid var;
        PGC_TRACE(pgc_lang_gen_exp(pgc_syn_getexpr(syn), st, &var));
        PGC_IO(fprintf(st->out, "%s%s->u.lnk = ", MARGIN, name));
        PGC_TRACE(pgc_lang_genid_fprint(st->out, &var));
        PGC_IO(fputs(";\r\n", st->out));
        return PGC_ERR_OK;
}

static enum pgc_err pgc_lang_gen_let(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st)
{
        char *name = pgc_ast_tostr(pgc_syn_getname(syn));
        struct pgc_lang_genid var;
        PGC_TRACE(pgc_lang_gen_exp(pgc_syn_getexpr(syn), st, &var));
        PGC_IO(fprintf(st->out, 
                "%sstatic struct pgc_par * %s = ",
                MARGIN, name));
        PGC_TRACE(pgc_lang_genid_fprint(st->out, &var));
        PGC_IO(fputs(";\r\n", st->out));
        return PGC_ERR_OK;
}

static enum pgc_err pgc_lang_gen_set_try(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st,
        struct pgc_cset *set)
{
        char *name = pgc_ast_tostr(pgc_syn_getname(syn));
        PGC_TRACE(pgc_lang_gen_setexp(pgc_syn_getexpr(syn), st->sets, set));
        
        const size_t c1 = st->uid++;
        const size_t c2 = st->uid++;
        
        PGC_IO(fprintf(st->out, 
                "%sstatic struct pgc_cset p%zx_p = { .words = { 0x%x ",
                MARGIN, c1, set->words[0]));
        for(int x = 1; x < 8; ++x) {
                PGC_IO(fprintf(st->out, ", 0x%x", set->words[x]));
        }
        PGC_IO(fputs(" } };\r\n", st->out));

        PGC_IO(fprintf(st->out, 
                "%sstatic struct pgc_par p%zx_p; "
                "pgc_par_set(&p%zx_p, &p%zx_p);\r\n"
                "%sstatic struct pgc_par *%s = &p%zx_p;\r\n",
                MARGIN, c2, c2, c1, MARGIN, name, c2));

        pgc_tbl_insert(st->sets, name, set);

        return PGC_ERR_OK;
}

static enum pgc_err pgc_lang_gen_set(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st)
{
        struct pgc_cset *set = malloc(sizeof(struct pgc_cset));
        const enum pgc_err err = pgc_lang_gen_set_try(syn, st, set);
        if(err != PGC_ERR_OK) {
                free(set);
                PGC_TRACE(err);
        }
        return PGC_ERR_OK;
}

static enum pgc_err pgc_lang_gen_stmt(
        struct pgc_ast *syn,
        struct pgc_lang_genst *st)
{
        switch(pgc_syn_typeof(syn)) {
                case PGC_SYN_DEC: return pgc_lang_gen_dec(syn, st);
                case PGC_SYN_DEF: return pgc_lang_gen_def(syn, st);
                case PGC_SYN_LET: return pgc_lang_gen_let(syn, st);
                case PGC_SYN_SET: return pgc_lang_gen_set(syn, st);
                default: PGC_ABORT();
        }
        return PGC_ERR_OK;
}

static enum pgc_err pgc_lang_gen_result(
        struct pgc_ast_lst *lst, 
        struct pgc_lang_genst *st)
{
        PGC_IO(fprintf(st->out, 
                "%sstatic struct %s result;\r\n", 
                MARGIN, st->dict));
        for(struct pgc_ast_lst *l = lst; l; l = l->nxt) {
                struct pgc_ast *stmt = l->val;
                if(!stmt) {
                        PGC_ABORT();
                }
                char *name = pgc_ast_tostr(pgc_syn_getname(stmt));
                switch(pgc_syn_typeof(stmt)) {
                        case PGC_SYN_DEC:
                        case PGC_SYN_LET:
                        case PGC_SYN_SET: PGC_IO(
                                fprintf(st->out, 
                                "%sresult.%s = %s;\r\n",
                                MARGIN, name, name));
                        case PGC_SYN_DEF: break;
                        default: PGC_ABORT();
                }
        }
        PGC_IO(fprintf(st->out, "%sreturn &result;\r\n", MARGIN));
        return PGC_ERR_OK;
}

static enum pgc_err pgc_lang_gen_fun_try(
        struct pgc_ast_lst *lst,
        struct pgc_lang_genst *st)
{
        PGC_IO(fprintf(st->out,
                "struct %s *export_%s()\r\n{\r\n",
                st->dict, st->dict));
        for(struct pgc_ast_lst *l = lst; l; l = l->nxt) {
                PGC_TRACE(pgc_lang_gen_stmt(l->val, st));
        }
        PGC_TRACE(pgc_lang_gen_result(lst, st));
        PGC_IO(fputs("}\r\n", st->out));
        return PGC_ERR_OK;
}

static enum pgc_err pgc_lang_gen_fun(
        struct pgc_ast_lst *lst,
        struct pgc_lang_genst *st)
{
        st->sets = pgc_tbl_create(8);
        const enum pgc_err err = pgc_lang_gen_fun_try(lst, st);
        struct pgc_tbl_i imem, *i;
        for(i = pgc_tbl_begin(st->sets, &imem); i; i = pgc_tbl_next(i)) {
                free(pgc_tbl_value(i));
        }
        pgc_tbl_destroy(st->sets);
        if(err != PGC_ERR_OK) {
                PGC_TRACE(err);
        }
        return err;
}

enum pgc_err pgc_lang_gen(
        FILE *out,
        struct pgc_ast_lst *lst,
        const char *dictname)
{
        struct pgc_lang_genst st = { 
                .uid = 0, 
                .dict = dictname, 
                .out = out,
                .sets = NULL };
        PGC_IO(fputs("#include \"pgenc/parser.h\"\r\n", st.out));
        PGC_TRACE(pgc_lang_gen_dict(lst, &st));
        PGC_TRACE(pgc_lang_gen_hooks(lst, &st));
        PGC_TRACE(pgc_lang_gen_gets(lst, &st));
        PGC_TRACE(pgc_lang_gen_fun(lst, &st));
        return PGC_ERR_OK;
}
