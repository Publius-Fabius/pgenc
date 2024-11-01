
#include "pgenc/syntax.h"
#include "pgenc/error.h"
#include <string.h>
#include <stdlib.h>

struct pgc_ast *pgc_syn_newid(char *value) 
{
        return pgc_ast_initstr(
                malloc(sizeof(struct pgc_ast)), 
                value, 
                PGC_SYN_ID);
}

struct pgc_ast *pgc_syn_newnum(const uint32_t value) 
{
        return pgc_ast_inituint32(
                malloc(sizeof(struct pgc_ast)), 
                value, 
                PGC_SYN_NUM);
}

struct pgc_ast *pgc_syn_newchar(const char value) 
{
        return pgc_ast_initint8(
                malloc(sizeof(struct pgc_ast)), 
                value, 
                PGC_SYN_CHAR);
}

static struct pgc_ast *pgc_syn_newpair(
        struct pgc_ast *fst,
        struct pgc_ast *snd,
        enum pgc_syn_tag tag)
{
        struct pgc_ast_lst *head = malloc(sizeof(struct pgc_ast_lst));
        struct pgc_ast_lst *tail = malloc(sizeof(struct pgc_ast_lst));
        tail->val = snd;
        tail->nxt = NULL;
        head->val = fst;
        head->nxt = tail;
        return pgc_ast_initlst(
                malloc(sizeof(struct pgc_ast)), 
                head, 
                tag);
}

static struct pgc_ast *pgc_syn_newsingleton(
        struct pgc_ast *value,
        enum pgc_syn_tag tag)
{
        struct pgc_ast_lst *list = malloc(sizeof(struct pgc_ast_lst));
        list->val = value;
        list->nxt = NULL;
        return pgc_ast_initlst(
                malloc(sizeof(struct pgc_ast)), 
                list, 
                tag);
}

struct pgc_ast *pgc_syn_newunion(struct pgc_ast *fst, struct pgc_ast *snd) 
{
        return pgc_syn_newpair(fst, snd, PGC_SYN_UNION);
}

struct pgc_ast *pgc_syn_newdiff(struct pgc_ast *fst, struct pgc_ast *snd) 
{
        return pgc_syn_newpair(fst, snd, PGC_SYN_DIFF);
}

struct pgc_ast *pgc_syn_newutf(struct pgc_ast *min, struct pgc_ast *max) 
{
        return pgc_syn_newpair(min, max, PGC_SYN_UTF);
}

struct pgc_ast *pgc_syn_newlit(char *str) 
{
        return pgc_ast_initstr(
                malloc(sizeof(struct pgc_ast)), 
                str, 
                PGC_SYN_LIT);
}

struct pgc_ast *pgc_syn_newand(struct pgc_ast *fst, struct pgc_ast *snd) 
{
        return pgc_syn_newpair(fst, snd, PGC_SYN_AND);
}

struct pgc_ast *pgc_syn_newor(struct pgc_ast *fst, struct pgc_ast *snd) 
{
        return pgc_syn_newpair(fst, snd, PGC_SYN_OR);
}

struct pgc_ast *pgc_syn_newrange(struct pgc_ast *min, struct pgc_ast *max) 
{
        return pgc_syn_newpair(min, max, PGC_SYN_RANGE);
}

struct pgc_ast *pgc_syn_newrep(
        struct pgc_ast *range, 
        struct pgc_ast *subex) 
{
        return pgc_syn_newpair(range, subex, PGC_SYN_REP);
}

struct pgc_ast *pgc_syn_newhook(char *iden) 
{
        return pgc_ast_initstr(
                malloc(sizeof(struct pgc_ast)),
                iden, 
                PGC_SYN_HOOK);
}

struct pgc_ast *pgc_syn_newcall(struct pgc_ast *fun, struct pgc_ast *var) 
{
        return pgc_syn_newpair(fun, var, PGC_SYN_CALL);
}

struct pgc_ast *pgc_syn_newset(struct pgc_ast *id, struct pgc_ast *exp) 
{
        return pgc_syn_newpair(id, exp, PGC_SYN_SET);
}

struct pgc_ast *pgc_syn_newlet(struct pgc_ast *id, struct pgc_ast *exp) 
{
        return pgc_syn_newpair(id, exp, PGC_SYN_LET);
}

struct pgc_ast *pgc_syn_newdec(struct pgc_ast *id) 
{
        return pgc_syn_newsingleton(id, PGC_SYN_DEC);
}

struct pgc_ast *pgc_syn_newdef(struct pgc_ast *id, struct pgc_ast *exp) 
{
        return pgc_syn_newpair(id, exp, PGC_SYN_DEF);
}

struct pgc_ast *pgc_syn_newsrc(struct pgc_ast_lst *list)
{
        return pgc_ast_initlst(
                malloc(sizeof(struct pgc_ast)), 
                list, 
                PGC_SYN_SRC);
}

static inline void pgc_syn_freelst(struct pgc_ast_lst *l)
{
        while(l) {
                pgc_syn_free(l->val);
                struct pgc_ast_lst *tmp = l;
                l = l->nxt;
                free(tmp);
        }
}

void pgc_syn_free(struct pgc_ast *ast)
{
      switch(pgc_syn_typeof(ast)) {
                case PGC_SYN_ID: 
                case PGC_SYN_NUM: 
                case PGC_SYN_CHAR: 
                case PGC_SYN_LIT:
                case PGC_SYN_HOOK:
                        free(ast);
                        return;
                case PGC_SYN_CALL:
                case PGC_SYN_UTF: 
                case PGC_SYN_RANGE: 
                case PGC_SYN_UNION: 
                case PGC_SYN_DIFF: 
                case PGC_SYN_AND: 
                case PGC_SYN_OR: 
                case PGC_SYN_REP: 
                case PGC_SYN_SET: 
                case PGC_SYN_LET: 
                case PGC_SYN_DEF: 
                case PGC_SYN_DEC: 
                case PGC_SYN_SRC: 
                        pgc_syn_freelst(pgc_ast_tolst(ast));
                        free(ast);
                        return;
                default: SEL_HALT();
        }  
}

int pgc_syn_typeof(struct pgc_ast *syn)
{
        return syn->utag;
}

static struct pgc_ast *pgc_syn_getfst_unsafe(struct pgc_ast *syn)
{
        struct pgc_ast_lst *head = pgc_ast_tolst(syn);
        SEL_ASSERT(head);
        return head->val;
}

static struct pgc_ast *pgc_syn_getsnd_unsafe(struct pgc_ast *syn)
{
        struct pgc_ast_lst *head = pgc_ast_tolst(syn);
        SEL_ASSERT(head);
        struct pgc_ast_lst *tail = head->nxt;
        SEL_ASSERT(tail);
        return tail->val;
}

struct pgc_ast *pgc_syn_getfst(struct pgc_ast *syn) 
{
        SEL_ASSERT(syn);
        switch(syn->utag) {
                case PGC_SYN_OR: break;
                case PGC_SYN_AND: break;
                case PGC_SYN_RANGE: break;
                case PGC_SYN_UNION: break;
                case PGC_SYN_DIFF: break;
                case PGC_SYN_UTF: break;
                default: SEL_HALT();
        }
        return pgc_syn_getfst_unsafe(syn);
}

struct pgc_ast *pgc_syn_getsnd(struct pgc_ast *syn) 
{
        SEL_ASSERT(syn);
        switch(syn->utag) {
                case PGC_SYN_OR: break;
                case PGC_SYN_AND: break;
                case PGC_SYN_RANGE: break;
                case PGC_SYN_UNION: break;
                case PGC_SYN_DIFF: break;
                case PGC_SYN_UTF: break;
                default: SEL_HALT();
        }
        return pgc_syn_getsnd_unsafe(syn);
}

struct pgc_ast *pgc_syn_getname(struct pgc_ast *syn) 
{
        SEL_ASSERT(syn);
        switch(syn->utag) {
                case PGC_SYN_DEF: break;
                case PGC_SYN_DEC: break;
                case PGC_SYN_LET: break;
                case PGC_SYN_SET: break; 
                default: SEL_HALT();
        } 
        return pgc_syn_getfst_unsafe(syn);
}

struct pgc_ast *pgc_syn_getexpr(struct pgc_ast *syn) 
{
        SEL_ASSERT(syn);
        switch(syn->utag) {
                case PGC_SYN_DEF: break;
                case PGC_SYN_LET: break;
                case PGC_SYN_SET: break;
                default: SEL_HALT();
        } 
        return pgc_syn_getsnd_unsafe(syn);
}

struct pgc_ast *pgc_syn_getrange(struct pgc_ast *syn) 
{
        SEL_ASSERT(syn);
        switch(syn->utag) {
                case PGC_SYN_REP: break;
                default: SEL_HALT();
        }
        return pgc_syn_getfst_unsafe(syn);
}

struct pgc_ast *pgc_syn_getsubex(struct pgc_ast *syn) 
{
        SEL_ASSERT(syn);
        switch(syn->utag) {
                case PGC_SYN_REP: break;
                default: SEL_HALT();
        }
        return pgc_syn_getsnd_unsafe(syn);
}

struct pgc_ast *pgc_syn_getfun(struct pgc_ast *syn)
{
        SEL_ASSERT(syn);
        switch(syn->utag) {
                case PGC_SYN_CALL: break;
                default: SEL_HALT();
        }
        return pgc_syn_getfst_unsafe(syn);
}

struct pgc_ast *pgc_syn_getvar(struct pgc_ast *syn)
{
        SEL_ASSERT(syn);
        switch(syn->utag) {
                case PGC_SYN_CALL: break;
                default: SEL_HALT();
        }
        return pgc_syn_getsnd_unsafe(syn);
}

struct pgc_ast *pgc_syn_findstmt(struct pgc_ast *syn, const char *name)
{
        if(pgc_syn_typeof(syn) != PGC_SYN_SRC) {
                SEL_HALT();
        }
        for(struct pgc_ast_lst *l = pgc_ast_tolst(syn); l; l = l->nxt)
        {
                if(!strcmp(pgc_ast_tostr(pgc_syn_getname(l->val)), name)) {
                        return l->val;
                }
        }
        return NULL;
}

static int pgc_syn_fprintid(FILE *file, struct pgc_ast *syn) 
{
        return fputs(pgc_ast_tostr(syn), file);
}

static int pgc_syn_fprintnum(FILE *file, struct pgc_ast *syn) 
{
        return fprintf(file, "%u", pgc_ast_touint32(syn));
}

static int pgc_syn_fprintbyte(FILE *file, struct pgc_ast *syn) 
{
        return fprintf(file, "%%%X", pgc_ast_toint8(syn));
}

static int pgc_syn_fprintlit(FILE *file, struct pgc_ast *syn) 
{
        return fprintf(file, "\"%s\"", pgc_ast_tostr(syn));
}

static int pgc_syn_fprinthook(FILE *file, struct pgc_ast *syn) 
{
        return fprintf(file, "@%s", pgc_ast_tostr(syn));
}

static int pgc_syn_fprintutf(FILE *file, struct pgc_ast *syn) 
{
        struct pgc_ast *min = pgc_syn_getfst(syn);
        struct pgc_ast *max = pgc_syn_getsnd(syn);
        return fprintf(file, 
                "&%X_%X", 
                pgc_ast_touint32(min), 
                pgc_ast_touint32(max));
}

static int pgc_syn_fprintrange(FILE *file, struct pgc_ast *syn) 
{
        struct pgc_ast *min = pgc_syn_getfst(syn);
        struct pgc_ast *max = pgc_syn_getsnd(syn);
        return fprintf(file, 
                "%u_%u", 
                pgc_ast_touint32(min), 
                pgc_ast_touint32(max));
}

#define TRY_IO(EXPR) if(EXPR < 0) { return -1; } 

static int pgc_syn_fprintpair(
        FILE *file, 
        struct pgc_ast *syn, 
        const char *delim)
{
        struct pgc_ast *fst = pgc_syn_getfst(syn);
        struct pgc_ast *snd = pgc_syn_getsnd(syn);
        TRY_IO(fputc('(', file));
        TRY_IO(pgc_syn_fprint(file, fst));
        TRY_IO(fputs(delim, file));
        TRY_IO(pgc_syn_fprint(file, snd));
        return fputc(')', file);
}

static int pgc_syn_fprintunion(FILE *file, struct pgc_ast *syn) 
{
        return pgc_syn_fprintpair(file, syn, " + ");
}

static int pgc_syn_fprintdiff(FILE *file, struct pgc_ast *syn) 
{
        return pgc_syn_fprintpair(file, syn, " - ");
}

static int pgc_syn_fprintand(FILE *file, struct pgc_ast *syn) 
{
        return pgc_syn_fprintpair(file, syn, " ");
}

static int pgc_syn_fprintor(FILE *file, struct pgc_ast *syn) 
{
        return pgc_syn_fprintpair(file, syn, " | ");
}

static int pgc_syn_fprintrep(FILE *file, struct pgc_ast *syn) 
{
        struct pgc_ast *range = pgc_syn_getrange(syn);
        struct pgc_ast *subex = pgc_syn_getsubex(syn);
        TRY_IO(pgc_syn_fprint(file, range));
        return pgc_syn_fprint(file, subex);
}

static int pgc_syn_fprintstmtpair(
        FILE *file, 
        struct pgc_ast *syn,
        const char *type)
{
        struct pgc_ast *name = pgc_syn_getname(syn);
        struct pgc_ast *expr = pgc_syn_getexpr(syn);
        TRY_IO(fprintf(file, "%s ", type));
        TRY_IO(pgc_syn_fprint(file, name));
        TRY_IO(fputs(" = ", file));
        TRY_IO(pgc_syn_fprint(file, expr));
        return fputc(';', file);
}

static int pgc_syn_fprintset(FILE *file, struct pgc_ast *syn) 
{
        return pgc_syn_fprintstmtpair(file, syn, "set");
}

static int pgc_syn_fprintlet(FILE *file, struct pgc_ast *syn) 
{
        return pgc_syn_fprintstmtpair(file, syn, "let");
}

static int pgc_syn_fprintdef(FILE *file, struct pgc_ast *syn) 
{
        return pgc_syn_fprintstmtpair(file, syn, "def");
}

static int pgc_syn_fprintdec(FILE *file, struct pgc_ast *syn)
{
        struct pgc_ast *name = pgc_syn_getname(syn);
        TRY_IO(fputs("dec ", file));
        TRY_IO(pgc_syn_fprint(file, name));
        return fputc(';', file);
}

static int pgc_syn_fprintsrc(FILE *file, struct pgc_ast *syn)
{
        for(struct pgc_ast_lst *l = pgc_ast_tolst(syn); l; l = l->nxt) {
                TRY_IO(pgc_syn_fprint(file, l->val));
                TRY_IO(fputs("\r\n", file));
        }
        return 0;
}

int pgc_syn_fprint(FILE *file, struct pgc_ast *syn) 
{
        switch(pgc_syn_typeof(syn)) {
                case PGC_SYN_ID: return pgc_syn_fprintid(file, syn);
                case PGC_SYN_NUM: return pgc_syn_fprintnum(file, syn);
                case PGC_SYN_CHAR: return pgc_syn_fprintbyte(file, syn);
                case PGC_SYN_LIT: return pgc_syn_fprintlit(file, syn);
                case PGC_SYN_HOOK: return pgc_syn_fprinthook(file, syn);
                case PGC_SYN_UTF: return pgc_syn_fprintutf(file, syn);
                case PGC_SYN_RANGE: return pgc_syn_fprintrange(file, syn);
                case PGC_SYN_UNION: return pgc_syn_fprintunion(file, syn);
                case PGC_SYN_DIFF: return pgc_syn_fprintdiff(file, syn); 
                case PGC_SYN_AND: return pgc_syn_fprintand(file, syn);
                case PGC_SYN_OR: return pgc_syn_fprintor(file, syn);
                case PGC_SYN_REP: return pgc_syn_fprintrep(file, syn);
                case PGC_SYN_SET: return pgc_syn_fprintset(file, syn);
                case PGC_SYN_LET: return pgc_syn_fprintlet(file, syn);
                case PGC_SYN_DEF: return pgc_syn_fprintdef(file, syn);
                case PGC_SYN_DEC: return pgc_syn_fprintdec(file, syn);
                case PGC_SYN_SRC: return pgc_syn_fprintsrc(file, syn);
                default: SEL_HALT();
        }
        return -1;
}

#undef TRY_IO
