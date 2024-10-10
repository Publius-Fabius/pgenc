#ifndef PGENC_PARSER_H
#define PGENC_PARSER_H

#include "pgenc/error.h"
#include "pgenc/charset.h"
#include "pgenc/buffer.h"
#include <sys/types.h>

struct pgc_par;

/** Function pointer for parser hooks. */
typedef sel_err_t (*pgc_par_hook_t)(
        struct pgc_buf *buffer,
        void *state);

/** Function pointer for custom parser calls. */
typedef sel_err_t (*pgc_par_call_t)(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *var);

/** Parser type discriminator */
enum pgc_par_tag {
        PGC_PAR_CMP,                            /** Comparison Parser */
        PGC_PAR_UTF8,                           /** UTF8 Parser */
        PGC_PAR_BYTE,                           /** Byte Parser */
        PGC_PAR_SET,                            /** Charset Parser */
        PGC_PAR_AND,                            /** Product Parser */
        PGC_PAR_OR,                             /** Choice Parser */
        PGC_PAR_REP,                            /** Repetition Parser */
        PGC_PAR_HOOK,                           /** Hook Parser */
        PGC_PAR_LNK,                            /** Link parser */
        PGC_PAR_CALL                            /** Custom Call */
};

/** Parser */
struct pgc_par 
{
        const enum pgc_par_tag tag;
        union 
        {
                struct {
                        const struct pgc_par *const arg1;
                        const struct pgc_par *const arg2;
                } const pair;
                struct {
                        const struct pgc_par *const sub;
                        const uint32_t min;
                        const uint32_t max;
                } const trip;
                struct {
                        const char *const val;
                        const size_t len;
                } const str;
                struct {
                        const pgc_par_call_t fun;
                        const struct pgc_par *const var;
                } const call;
                const struct pgc_par *const lnk;
                const pgc_par_hook_t hook;
                const struct pgc_cset *const set;
                const int byte;
        } const u;
};

/** 
 * Initialize a comparison parser that compares the next sequence of 
 * characters against the string.  This function does not allocate 
 * new memory, so the string must be available when the parser runs.
 * This parser does not compare against the trailing 'zero' character
 * if it exists.
 * @param STR The string to compare against.
 * @param LEN The number of characters to compare.
 */
#define PGC_PAR_CMP(STR, LEN) { \
        .tag = PGC_PAR_CMP, \
        .u.str.val = STR, \
        .u.str.len = LEN }

/** 
 * Initialize a char parser that succeeds when the next character matches 
 * the given octet.
 * @param OCTET The octet to match input against.
 */
#define PGC_PAR_BYTE(OCTET) { \
        .tag = PGC_PAR_BYTE, \
        .u.byte = OCTET }

/** 
 * Initialize a UTF8 parser that succeeds when a UTF8 value within the 
 * specified interval is parsed. 
 * @param MIN The minimum UTF8 value.
 * @param MAX The maximum UTF8 value.
 */
#define PGC_PAR_UTF8(MIN, MAX) { \
        .tag = PGC_PAR_UTF8, \
        .u.trip.min = MIN, \
        .u.trip.max = MAX }

/**
 * Initialize a set membership parser that succeeds when the next character 
 * is a member of the given set.  This function does not allocate any new
 * memory.
 * @param SET The character set.
 */
#define PGC_PAR_SET(SET) { \
        .tag = PGC_PAR_SET, \
        .u.set = SET }

/**
 * Initialize a product parser that succeeds when both of its sub-parsers
 * succeed.
 * @param ARG1 The first argument.
 * @param ARG2 The second argument.
 */
#define PGC_PAR_AND(ARG1, ARG2) { \
        .tag = PGC_PAR_AND, \
        .u.pair.arg1 = ARG1, \
        .u.pair.arg2 = ARG2 }

/**
 * Initialize a choice parser that succeeds when one of the sub-parsers
 * succeeds.
 * @param ARG1 The first argument.
 * @param ARG2 The second argument.
 */
#define PGC_PAR_OR(ARG1, ARG2) { \
        .tag = PGC_PAR_OR, \
        .u.pair.arg1 = ARG1, \
        .u.pair.arg2 = ARG2 }

/**
 * Initialize a repetition parser that succeeds when the given child parser 
 * succeeds within a bounded number of repetitions.
 * @param SUB The sub-parser.
 * @param MIN The minimum number of repetitions.
 * @param MAX The maximum number of repetitions.
 **/
#define PGC_PAR_REP(SUB, MIN, MAX) { \
        .tag = PGC_PAR_REP, \
        .u.trip.sub = SUB, \
        .u.trip.min = MIN, \
        .u.trip.max = MAX }

/**
 * Initialize a hook parser that calls the given function.
 * @param callback The function to call when the parser runs.
 */
#define PGC_PAR_HOOK(CALLBACK) { \
        .tag = PGC_PAR_HOOK, \
        .u.hook = CALLBACK }

/**
 * Initialize a call parser that calls the given function with an argument.
 * @param FUN The function to call when the parser runs.
 * @param VAR The parser argument
 */
#define PGC_PAR_CALL(FUN, VAR) { \
        .tag = PGC_PAR_CALL, \
        .u.call.fun = FUN, \
        .u.call.var = VAR }

/**
 * Initialize a link parser that simply runs the parser it points to.
 * @param lnk The parser the parser will link to.
 */
#define PGC_PAR_LNK(LNK) { \
        .tag = PGC_PAR_LNK, \
        .u.lnk = LNK }

/** 
 * Run a parser by taking a character buffer and a state.
 * @param parser The parser to run.
 * @param buffer The buffer to parse.
 * @param state The parser's user defined state.
 * @return A negative error code on failure, otherwise PGC_ERR_OK.
 */
sel_err_t pgc_par_run(
        const struct pgc_par *parser, 
        struct pgc_buf *buffer, 
        void *state);

/**
 * Run the parser on the string.
 * @param parser The parser to run.
 * @param str The string to parse.
 * @param state The parse state.
 * @return A value 0 or more indicates how many characters were successfully
 * parsed.  A negative value corresponds to a PGC_ERR_X code.
 */
ssize_t pgc_par_runs(
        const struct pgc_par *parser,
        char *str,
        void *state);

#endif