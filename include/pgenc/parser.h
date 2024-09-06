#ifndef PGENC_PARSER_H
#define PGENC_PARSER_H

#include "pgenc/error.h"
#include "pgenc/charset.h"
#include "pgenc/buffer.h"
#include <sys/types.h>

struct pgc_par;

/** Function pointer for parser hooks. */
typedef enum pgc_err (*pgc_par_hook_t)(
        struct pgc_buf *buffer,
        void *state);

/** Function pointer for custom parser calls. */
typedef enum pgc_err (*pgc_par_call_t)(
        struct pgc_buf *buffer,
        void *state,
        struct pgc_par *var);

/** Parser type discriminator */
enum pgc_par_tag 
{
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
        enum pgc_par_tag tag;                   /** Discriminator */
        union 
        {
                struct {
                        struct pgc_par *arg1;   /** First argument */
                        struct pgc_par *arg2;   /** Second argument */
                } pair;
                struct {
                        struct pgc_par *sub;    /** Sub-parser */
                        uint32_t min;           /** Minimum repetitions */
                        uint32_t max;           /** Maximum repetitions */
                } trip;
                struct {
                        char *val;              /** String Literal */
                        size_t len;             /** String Length */
                } str;
                struct {
                        pgc_par_call_t fun;     /** Call function */
                        struct pgc_par *var;    /** Call variable */
                } call;
                struct pgc_par *lnk;            /** Link */
                pgc_par_hook_t hook;            /** Hook function */
                struct pgc_cset *set;           /** Charset */
                int byte;                       /** Byte */
        } u;
};

/** 
 * Initialize a comparison parser that compares the next sequence of 
 * characters against the string.  This function does not allocate 
 * new memory, so the string must be available when the parser runs.
 * This parser does not compare against the trailing 'zero' character
 * if it exists.
 * @param par The parser to initialize.
 * @param str The string to compare against.
 * @param len The number of characters to compare.
 * @return par
 */
struct pgc_par *pgc_par_cmp(
        struct pgc_par *par,
        char *str, 
        const size_t len);

/** 
 * Initialize a char parser that succeeds when the next character matches 
 * the given octet.
 * @param par The parser to initialize.
 * @param oct The octet to match input against.
 * @return par
 */
struct pgc_par *pgc_par_byte(
        struct pgc_par *par,
        const int byte);

/** 
 * Initialize a UTF8 parser that succeeds when a UTF8 value within the 
 * specified interval is parsed. 
 * @param par The parser to initialize.
 * @param min The minimum UTF8 value.
 * @param max The maximum UTF8 value.
 * @return par
 */
struct pgc_par *pgc_par_utf8(
        struct pgc_par *par,
        const uint32_t min,
        const uint32_t max);

/**
 * Initialize a set membership parser that succeeds when the next character 
 * is a member of the given set.  This function does not allocate any new
 * memory.
 * @param par The parser to initialize.
 * @param set The character set.
 * @return par
 */
struct pgc_par *pgc_par_set(
        struct pgc_par *par,
        struct pgc_cset *set);

/**
 * Initialize a product parser that succeeds when both of its sub-parsers
 * succeed.
 * @param par The parser to initialize.
 * @param fst The first projection.
 * @param snd The second projection.
 * @return par
 */
struct pgc_par *pgc_par_and(
        struct pgc_par *par,
        struct pgc_par *fst,
        struct pgc_par *snd);

/**
 * Initialize a choice parser that succeeds when one of the sub-parsers
 * succeeds.
 * @param par The parser to initialize.
 * @param inl The left injection.
 * @param inr The right injection.
 * @return par
 */
struct pgc_par *pgc_par_or(
        struct pgc_par *par,
        struct pgc_par *inl,
        struct pgc_par *inr);

/**
 * Initialize a repetition parser that succeeds when the given child parser 
 * succeeds within a bounded number of repetitions.
 * @param par The parser to initialize.
 * @param sub The sub-parser.
 * @param min The minimum number of repetitions.
 * @param max The maximum number of repetitions.
 * @return par
 **/
struct pgc_par *pgc_par_rep(
        struct pgc_par *par,
        struct pgc_par *sub,
        const uint32_t min,
        const uint32_t max);

/**
 * Initialize a hook parser that calls the given function.
 * @param parser The parser to initialize.
 * @param callback The function to call when the parser runs.
 * @return parser
 */
struct pgc_par *pgc_par_hook(
        struct pgc_par *par,
        pgc_par_hook_t callback);

/**
 * Initialize a call parser that calls the given function with an argument.
 * @param parser The parser to initialize.
 * @param callback The function to call when the parser runs.
 * @return parser
 */
struct pgc_par *pgc_par_call(
        struct pgc_par *par,
        pgc_par_call_t callback,
        struct pgc_par *var);

/**
 * Initialize a link parser that simply runs the parser it points to.
 * @param parser The parser to initialize.
 * @param lnk The parser the parser will link to.
 * @return parser
 */
struct pgc_par *pgc_par_lnk(
        struct pgc_par *par,
        struct pgc_par *lnk);

/** 
 * Run a parser by taking a character buffer and a state.
 * @param parser The parser to run.
 * @param buffer The buffer to parse.
 * @param state The parser's user defined state.
 * @return A negative error code on failure, otherwise PGC_ERR_OK.
 */
enum pgc_err pgc_par_run(
        struct pgc_par *parser, 
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
        struct pgc_par *parser,
        char *str,
        void *state);

/**
 * Scan the buffer until the parser is successful.  Return the offset to
 * the beginning of the parsed sequence.
 */
ssize_t pgc_par_scan(
        struct pgc_par *parser,
        struct pgc_buf *buffer,
        void *state);

#endif