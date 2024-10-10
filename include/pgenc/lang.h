#ifndef PGC_BUILD_H
#define PGC_BUILD_H

#include "pgenc/error.h"
#include "pgenc/syntax.h"
#include "pgenc/stack.h"
#include "pgenc/parser.h"
#include <stdio.h>

/** 
 * Generate a syntax prototype for parsing PGENC grammars.
 * @return The syntax.
 */
struct pgc_ast *pgc_lang_proto();

/**
 * Generate PGENC parsers in standard C.
 * @param out The output stream.
 * @param list A list of syntax nodes.
 * @param prefix The prefix to the generated parsers.
 * @return A possible error, or PGC_ERR_OK.
 */
sel_err_t pgc_lang_gen(
        FILE *out,
        struct pgc_ast_lst *list,
        const char *prefix);

/**
 * Parse the buffer's contents into a syntax tree. 
 * @param parser The parser to run.
 * @param buffer The buffer to parse.
 * @param alloc Memory allocator.
 * @param list Resulting list of syntax nodes.
 * @return A possible error.
 */
sel_err_t pgc_lang_parse(
        const struct pgc_par *parser,
        struct pgc_buf *buffer,
        struct pgc_stk *alloc,
        struct pgc_ast_lst **list);

/**
 * Read a character literal.
 * @param buffer The buffer to read from
 * @param state Parsing state
 * @param parser The byte parser.
 * @param tag User defined tag.
 * @return Error code
 */
sel_err_t pgc_lang_readchar(
        struct pgc_buf *buf, 
        void *state,
        const size_t start,
        const int16_t tag);

/**
 * Read UTF8 character.
 * @param buffer The buffer to read from
 * @param state Parsing state
 * @param parser UTF8 parser.
 * @param tag User defined tag.
 * @return Error code
 */
sel_err_t pgc_lang_readutf8(
        struct pgc_buf *buffer, 
        void *state,
        const size_t start,
        const int16_t tag);

/**
 * Read a string literal.
 * @param buffer The buffer to read from
 * @param state An (opaque) parsing state
 * @param tag A user defined tag.
 * @return A possible error.
 */
sel_err_t pgc_lang_readstr(
        struct pgc_buf *buffer, 
        void *state,
        const size_t start,
        const int16_t tag);

/**
 * Read an encoded number.
 */
sel_err_t pgc_lang_readenc(
        struct pgc_buf *buffer,
        void *state,
        const size_t start,
        const int16_t atag,
        const int16_t utag,
        const size_t base,
        pgc_buf_decode_dict_t dict,
        pgc_buf_decode_accum_t accum);

/**
 * Set internal state's user tag.
 */
void pgc_lang_setutag(void *state, const int16_t tag);

/** 
 * Options for parsing an expression.
 */
enum pgc_lang_readop {
        PGC_BLD_MERGE       = 1,            /* Merge singletons */
        PGC_BLD_POLY        = 2             /* Poly type */
};

/** 
 * Parse an expression. 
 * PGC_BLD_READMERGE will merge singleton lists.
 * PGC_BLD_READPOLY will use the internal utag for the node's type.
 */
sel_err_t pgc_lang_readexp(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *parser,
        const int16_t utag,
        enum pgc_lang_readop ops);

/**
 * Parse a term.
 */
sel_err_t pgc_lang_readterm(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *arg,
        const int16_t utag,
        sel_err_t (*reader)(struct pgc_buf*, void*, const size_t, int16_t));

/**
 * Capture an identifier.
 */
sel_err_t pgc_lang_capid(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *arg);

/**
 * Capture a hex encoded byte.
 */
sel_err_t pgc_lang_capxbyte(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *arg);

/**
 * Capture a char literal.
 */
sel_err_t pgc_lang_capchar(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *arg);

/** 
 * Capture a number literal.
 */
sel_err_t pgc_lang_capnum(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *arg);

/**
 * Capture a numeric range.
 */
sel_err_t pgc_lang_caprange(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *arg);

/**
 * Capture a UTF8 encoded value.  Note that this function only captures a UTF
 * pair as encoded in the PGENC grammar (base16).  A true UTF8 capture may be
 * constructed with pgc_lang_readutf8.
 */
sel_err_t pgc_lang_caputf(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *arg);

/**
 * Capture a UTF8 encoded pair.
 */
sel_err_t pgc_lang_caputfrange(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *arg);

/**
 * Set internal utag to PGC_SYN_UNION.
 */
sel_err_t pgc_lang_setunion(struct pgc_buf *buffer, void *state);

/**
 * Set internal utag to PGC_SYN_DIFF.
 */
sel_err_t pgc_lang_setdiff(struct pgc_buf *buffer, void *state);

/**
 * Capture set expression
 */
sel_err_t pgc_lang_capsetexp(
        struct pgc_buf *buffer, 
        void *state, 
        const struct pgc_par *arg);

/**
 * Set internal utag to PGC_SYN_REP.
 */
sel_err_t pgc_lang_setrep(struct pgc_buf *buffer, void *state);

/**
 * Set internal utag to PGC_SYN_AND.
 */
sel_err_t pgc_lang_setand(struct pgc_buf *buffer, void *state);

/**
 * Set internal utag to PGC_SYN_OR.
 */
sel_err_t pgc_lang_setor(struct pgc_buf *buffer, void *state);

/**
 * Set internal utag to PGC_SYN_CALL.
 */
sel_err_t pgc_lang_setcall(struct pgc_buf *buffer, void *state);

/** Capture repitition */
sel_err_t pgc_lang_caprep(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *arg);
        
/**
 * Capture hook.
 */
sel_err_t pgc_lang_caphook(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *arg);

/**
 * Capture an expression.
 */
sel_err_t pgc_lang_capexp(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *parser);

/**
 * Set internal utag to PGC_SYN_DEC.
 */
sel_err_t pgc_lang_setdec(struct pgc_buf *buffer, void *state);

/**
 * Set internal utag to PGC_SYN_EXT.
 */
sel_err_t pgc_lang_setext(struct pgc_buf *buffer, void *state);

/**
 * Set internal utag to PGC_SYN_DEC.
 */
sel_err_t pgc_lang_setdef(struct pgc_buf *buffer, void *state);

/**
 * Set internal utag to PGC_SYN_SET.
 */
sel_err_t pgc_lang_setset(struct pgc_buf *buffer, void *state);

/**
 * Set internal utag to PGC_SYN_LET.
 */
sel_err_t pgc_lang_setlet(struct pgc_buf *buffer, void *state);

/**
 * Capture a statement.
 */
sel_err_t pgc_lang_capstmt(
        struct pgc_buf *buffer,
        void *state,
        const struct pgc_par *parser);

#endif