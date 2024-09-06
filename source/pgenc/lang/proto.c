
#include "pgenc/lang.h"
#include <stdlib.h>

static inline struct pgc_ast *pgc_syn_newand3(
        struct pgc_ast *arg1, 
        struct pgc_ast *arg2,
        struct pgc_ast *arg3)
{
        return pgc_syn_newand(arg1, pgc_syn_newand(arg2, arg3));
}

static inline struct pgc_ast *pgc_syn_newand4(
        struct pgc_ast *arg1, 
        struct pgc_ast *arg2,
        struct pgc_ast *arg3,
        struct pgc_ast *arg4)
{
        return pgc_syn_newand3(arg1, arg2, pgc_syn_newand(arg3, arg4));
}

static inline struct pgc_ast *pgc_syn_newand5(
        struct pgc_ast *arg1, 
        struct pgc_ast *arg2,
        struct pgc_ast *arg3,
        struct pgc_ast *arg4,
        struct pgc_ast *arg5)
{
        return pgc_syn_newand4(arg1, arg2, arg3, pgc_syn_newand(arg4, arg5));
}

static inline struct pgc_ast *pgc_syn_newor3(
        struct pgc_ast *arg1, 
        struct pgc_ast *arg2,
        struct pgc_ast *arg3)
{
        return pgc_syn_newor(arg1, pgc_syn_newor(arg2, arg3));
}

static inline struct pgc_ast *pgc_syn_newor4(
        struct pgc_ast *arg1, 
        struct pgc_ast *arg2,
        struct pgc_ast *arg3,
        struct pgc_ast *arg4)
{
        return pgc_syn_newor3(arg1, arg2, pgc_syn_newor(arg3, arg4));
}

static inline struct pgc_ast *pgc_syn_newor5(
        struct pgc_ast *arg1, 
        struct pgc_ast *arg2,
        struct pgc_ast *arg3,
        struct pgc_ast *arg4,
        struct pgc_ast *arg5)
{
        return pgc_syn_newor4(arg1, arg2, arg3, pgc_syn_newor(arg4, arg5));
}

static inline struct pgc_ast_lst *pgc_syn_cons(
        struct pgc_ast *elem,
        struct pgc_ast_lst *tail)
{
        struct pgc_ast_lst *head = malloc(sizeof(struct pgc_ast_lst));
        head->val = elem;
        head->nxt = tail;
        return head;
}

#define CONS(elem, tail) pgc_syn_cons(elem, tail)
#define SET(name, expr) pgc_syn_newset(name, expr)
#define ID(str) pgc_syn_newid(str)
#define LET(name, expr) pgc_syn_newlet(name, expr)
#define REP(range, subex) pgc_syn_newrep(range, subex)
#define NUM(val) pgc_syn_newnum(val)
#define RANGE(min, max) pgc_syn_newrange(min, max)
#define UNION(fst, snd) pgc_syn_newunion(fst, snd)
#define BYTE(value) pgc_syn_newchar(value)
#define AND(a, b) pgc_syn_newand(a, b)
#define AND3(a, b, c) pgc_syn_newand3(a, b, c)
#define AND4(a, b, c, d) pgc_syn_newand4(a, b, c, d)
#define AND5(a, b, c, d, e) pgc_syn_newand5(a, b, c, d, e)
#define OR(a, b) pgc_syn_newor(a, b)
#define OR3(a, b, c) pgc_syn_newor3(a, b, c)
#define OR4(a, b, c, d) pgc_syn_newor4(a, b, c, d)
#define OR5(a, b, c, d, e) pgc_syn_newor5(a, b, c, d, e)
#define HOOK(iden) pgc_syn_newhook(iden)
#define DEC(iden) pgc_syn_newdec(iden)
#define DEF(iden, expr) pgc_syn_newdef(iden, expr)
#define LIT(iden) pgc_syn_newlit(iden)
#define CALL(a, b) pgc_syn_newcall(a, b)

struct pgc_ast *pgc_lang_proto() 
{
        struct pgc_ast_lst *stmts = NULL;

        /* set wsc = isspace; */ 
        stmts = CONS(SET(ID("wsc"), ID("isspace")), stmts);

        /* let ws = 0_128wsc; */
        stmts = CONS(LET(ID("ws"), 
                REP(RANGE(NUM(0), NUM(128)), ID("wsc"))), stmts);
        
        /* set alpha = isalpha; */
        stmts = CONS(SET(ID("alpha"), ID("isalpha")), stmts);

        /* set idc = alpha + '_'; */
        stmts = CONS(SET(ID("idc"), UNION(ID("alpha"), BYTE('_'))), stmts);

        /* let idnh = 1_128idc; */
        stmts = CONS(LET(ID("idnh"), 
                REP(RANGE(NUM(1), NUM(128)), ID("idc"))), stmts);

        /* let id = pgc_lang_capid $ idnh; */
        stmts = CONS(LET(ID("id"), 
                CALL(ID("pgc_lang_capid"), ID("idnh"))), stmts);
        
        /* set xdigit = isxdigit; */
        stmts = CONS(SET(ID("xdigit"), ID("isxdigit")), stmts);

        /* let xdigits = 1_2xdigit; */
        stmts = CONS(LET(ID("xdigits"), 
                REP(RANGE(NUM(1), NUM(2)), ID("xdigit"))), stmts);

        /* let xbyte = pgc_lang_capxbyte $ xdigits; */
        stmts = CONS(LET(ID("xbyte"), 
                CALL(ID("pgc_lang_capxbyte"), ID("xdigits"))), stmts);
        
        /* let pctbyte = '%' xbyte; */
        stmts = CONS(LET(ID("pctbyte"), AND(BYTE('%'), ID("xbyte"))), stmts);

        /* set print = isprint; */
        stmts = CONS(SET(ID("print"), ID("isprint")), stmts);
      
        /* let charlit = ''' (pgc_lang_capcharlit $ print) '''; */
        stmts = CONS(LET(ID("charlit"), AND3(
                BYTE('\''),
                CALL(ID("pgc_lang_capchar"), ID("print")),
                BYTE('\'')
        )), stmts);

        /* set digit = isdigit; */
        stmts = CONS(SET(ID("digit"), ID("isdigit")), stmts);

        /* let digits = 1_10digit; */
        stmts = CONS(LET(ID("digits"), 
                REP(RANGE(NUM(1), NUM(10)), ID("digit"))), stmts);

        /* let numlit = pgc_lang_capnum $ digits; */
        stmts = CONS(LET(ID("num"), 
                CALL(ID("pgc_lang_capnum"), ID("digits"))), stmts);

        /* let range = pgc_lang_caprange $ numlit _ numlit; */
        stmts = CONS(LET(ID("range"), CALL(ID("pgc_lang_caprange"), 
                AND3(ID("num"), BYTE('_'), ID("num")))), stmts);
        
        /* let utfvalue = pgc_lang_caputf $ 1_6xdigit; */
        stmts = CONS(LET(ID("utfvalue"), CALL(ID("pgc_lang_caputf"),
                REP(RANGE(NUM(1), NUM(6)), ID("xdigit")))), stmts);
        
        /* let utfrange = pgc_lang_caputfpair $ '&' utfvalue '_' utfvalue; */
        stmts = CONS(LET(ID("utfrange"), CALL(ID("pgc_lang_caputfrange"), AND4(
                BYTE('&'), 
                ID("utfvalue"), 
                BYTE('_'), 
                ID("utfvalue")))), stmts);

        /* dec setexp; */
        stmts = CONS(DEC(ID("setexp")), stmts);

         /* let setparen = '(' setexp ')'; */
        stmts = CONS(LET(ID("setparen"), AND3(
                BYTE('('), 
                ID("setexp"), 
                BYTE(')'))), stmts);

        /* let setterm = setparen | xbytelit | charlit | id; */
        stmts = CONS(LET(ID("setterm"), OR4(
                ID("setparen"), 
                ID("pctbyte"), 
                ID("charlit"), 
                ID("id"))), stmts);

        /* let union = '+' setexp @pgc_lang_popunion; */
        stmts = CONS(LET(ID("setunion"), AND(
                AND(BYTE('+'), ID("setexp")),
                HOOK("pgc_lang_setunion"))), stmts);

        /* let diff = '-' setexp @pgc_lang_popdiff; */
        stmts = CONS(LET(ID("setdiff"), AND(
                AND(BYTE('-'), ID("setexp")),
                HOOK("pgc_lang_setdiff"))), stmts);

        /* let setop = union | diff; */
        stmts = CONS(LET(ID("setop"), OR(
                ID("setunion"), 
                ID("setdiff"))), stmts);
        
        /* def setexp = pgc_lang_capsetexp $ ws setterm ws setop; */
        stmts = CONS(DEF(ID("setexp"), CALL(ID("pgc_lang_capsetexp"), AND4(
                ID("ws"),
                ID("setterm"),
                ID("ws"),
                REP(RANGE(NUM(0), NUM(1)), ID("setop"))
        ))), stmts);
        
        /* dec exp; */
        stmts = CONS(DEC(ID("exp")), stmts);

        /* let paren = '(' exp ')' */
        stmts = CONS(LET(ID("paren"), AND3(
                BYTE('('), 
                ID("exp"), 
                BYTE(')'))), stmts);

        /* let term = xbytelit | charlit | paren | id; */
        stmts = CONS(LET(ID("term"), OR4(
                ID("pctbyte"), 
                ID("charlit"), 
                ID("paren"), 
                ID("id"))), stmts);

        /* let rep = range term @pgc_lang_poprep; */
        stmts = CONS(LET(ID("rep"), AND3(
                ID("range"), 
                ID("term"), 
                HOOK("pgc_lang_setrep"))), stmts);

        /* let hook = '@' pgc_lang_caphook $ idnh ; */
        stmts = CONS(LET(ID("hook"), AND(
                BYTE('@'),
                CALL(ID("pgc_lang_caphook"), ID("idnh")))), stmts);
        
        /* let phrase = rep | hook | utf | term; */
        stmts = CONS(LET(ID("phrase"), OR4(
                ID("rep"), 
                ID("hook"), 
                ID("utfrange"), 
                ID("term"))), stmts);

        /* let or = '|' exp @pgc_lang_setor; */
        stmts = CONS(LET(ID("or"), AND3(
                BYTE('|'), 
                ID("exp"), 
                HOOK("pgc_lang_setor"))), stmts);
        
        /* let and = exp @pgc_lang_setand;  */
        stmts = CONS(LET(ID("and"), AND(
                ID("exp"), 
                HOOK("pgc_lang_setand"))), stmts);
        
        /* let call = '$' exp @pgc_lang_setcall; */
        stmts = CONS(LET(ID("call"), AND3(
                BYTE('$'),
                ID("exp"),
                HOOK("pgc_lang_setcall"))), stmts);
        
        /* let op = or | and; */
        stmts = CONS(LET(ID("op"), OR3(
                ID("call"),
                ID("or"),
                ID("and"))), stmts);

        /* def exp = ws phrase ws 0_1(op)*/
        stmts = CONS(DEF(ID("exp"), CALL(ID("pgc_lang_capexp"), AND4(
                ID("ws"),
                ID("phrase"),
                ID("ws"),
                REP(RANGE(NUM(0), NUM(1)), ID("op"))
        ))), stmts);

        /* let wsidws = ws id ws; */
        stmts = CONS(LET(ID("wsidws"), AND3(
                ID("ws"), 
                ID("id"), 
                ID("ws"))), stmts);
        
        /* let dec = "dec" wsidws ';' @pgc_lang_popdec; */
        stmts = CONS(LET(ID("dec"), AND4(
                LIT("dec"), 
                ID("wsidws"), 
                BYTE(';'), 
                HOOK("pgc_lang_setdec"))), stmts);
        
        /* let wsidwseq = wsidws '='; */
        stmts = CONS(LET(ID("wsidwseq"), AND(
                ID("wsidws"), 
                BYTE('='))), stmts);

        /* let def = "def" wsidwseq exp ';' @pgc_lang_popdef; */
        stmts = CONS(LET(ID("def"), AND5(
                LIT("def"), 
                ID("wsidwseq"), 
                ID("exp"), 
                BYTE(';'), 
                HOOK("pgc_lang_setdef"))), stmts);

        /* let set = "set" wsidwseq setexp ';' @pgc_lang_popset; */
        stmts = CONS(LET(ID("set"), AND5(
                LIT("set"), 
                ID("wsidwseq"), 
                ID("setexp"), 
                BYTE(';'), 
                HOOK("pgc_lang_setset"))), stmts);

        /* let let = "let" wsidwseq exp ';' @pgc_lang_poplet; */
        stmts = CONS(LET(ID("let"), AND5(
                LIT("let"),
                ID("wsidwseq"),
                ID("exp"),
                BYTE(';'),
                HOOK("pgc_lang_setlet"))), stmts);

        /* let stmt = pgc_lang_capstmt $ def | dec | let | set; */
        stmts = CONS(LET(ID("stmt"), CALL(ID("pgc_lang_capstmt"), OR4(
                ID("def"), 
                ID("dec"), 
                ID("let"), 
                ID("set")))), stmts);

        /* let cmt = '#' 0_4096 print; */
        stmts = CONS(LET(ID("cmt"), AND(
                BYTE('#'),
                REP(RANGE(NUM(0), NUM(1024)), ID("print")))), stmts);

        /* let src = ws 0_50000((cmt | cat) ws); */
        stmts = CONS(LET(ID("src"), AND(
                ID("ws"), 
                REP(RANGE(NUM(0), NUM(1000000)), AND(
                        OR(ID("cmt"), ID("stmt")),
                        ID("ws")
                )))), stmts);

        return pgc_syn_newsrc(pgc_ast_rev(stmts));
}

#undef CONS
#undef SET
#undef ID
#undef LET
#undef REP
#undef NUM
#undef RANGE
#undef UNION
#undef BYTE
#undef AND
#undef AND3
#undef AND4
#undef AND5
#undef OR
#undef OR3
#undef OR4
#undef OR5
#undef HOOK
#undef DEC
#undef DEF
#undef LIT
#undef CALL