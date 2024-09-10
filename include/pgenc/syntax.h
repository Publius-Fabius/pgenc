#ifndef PGENC_SYNTAX_H
#define PGENC_SYNTAX_H

#include "pgenc/ast.h"
#include "pgenc/stack.h"
#include "pgenc/error.h"
#include <stdio.h>

/** Syntax Types */
enum pgc_syn_tag                                
{
        PGC_SYN_ID,                             /** Identifier */
        PGC_SYN_NUM,                            /** Number Literal */
        PGC_SYN_CHAR,                           /** Char Literal */
        PGC_SYN_UNION,                          /** Set Union */
        PGC_SYN_DIFF,                           /** Set Difference */
        PGC_SYN_UTF,                            /** UTF Span */
        PGC_SYN_LIT,                            /** String Literal */
        PGC_SYN_AND,                            /** And Expression */
        PGC_SYN_OR,                             /** Or Expression */
        PGC_SYN_RANGE,                          /** Numeric Range */
        PGC_SYN_REP,                            /** Repeat Expression */
        PGC_SYN_HOOK,                           /** Hook Expression */
        PGC_SYN_CALL,                           /** Call Expression */
        PGC_SYN_SET,                            /** Set Definition */
        PGC_SYN_LET,                            /** Let Definition */
        PGC_SYN_DEC,                            /** Declaration */
        PGC_SYN_DEF,                            /** Definition */
        PGC_SYN_SRC                             /** Source Node */
};

/**
 * Construct a new identifier node.
 * @param value The string value.
 * @return A new syntax node.
 */
struct pgc_ast *pgc_syn_newid(char *value);

/**
 * Construct a new number literal node.
 * @param value The integer value.
 * @return A new syntax node.
 */
struct pgc_ast *pgc_syn_newnum(const uint32_t value);

/**
 * Construct a new char literal node.
 * @param value The byte value.
 * @return A new syntax node.
 */
struct pgc_ast *pgc_syn_newchar(const char value);

/**
 * Construct a new set union node.
 * @param fst The first argument.
 * @param snd The second argument.
 * @return A new syntax node.
 */
struct pgc_ast *pgc_syn_newunion(
        struct pgc_ast *fst, 
        struct pgc_ast *snd);

/**
 * Construct a new set difference node.
 * @param fst The first argument.
 * @param snd The second argument.
 * @return A new syntax node.
 */
struct pgc_ast *pgc_syn_newdiff(
        struct pgc_ast *fst, 
        struct pgc_ast *snd);

/**
 * Construct a new UTF node.
 * @param min The minimum decoded UTF value.
 * @param max The maximum decoded UTF value.
 * @return A new syntax node.
*/
struct pgc_ast *pgc_syn_newutf(
        struct pgc_ast *min, 
        struct pgc_ast *max);

/**
 * Construct a new string literal node.
 * @param str The string value.
 * @return A new syntax node.
 */
struct pgc_ast *pgc_syn_newlit(char *str);

/**
 * Construct a new 'and' node.
 * @param fst The first argument.
 * @param snd The second argument.
 * @return A new syntax node.
 */
struct pgc_ast *pgc_syn_newand(
        struct pgc_ast *fst, 
        struct pgc_ast *snd);

/**
 * Construct a new 'or' node.
 * @param fst The first argument.
 * @param snd The second argument.
 * @return A new syntax node.
 */
struct pgc_ast *pgc_syn_newor(
        struct pgc_ast *fst, 
        struct pgc_ast *snd);

/**
 * Construct a new range node.
 * @param min The minimum value.
 * @param max The maximum value.
 * @return A new syntax node.
 */
struct pgc_ast *pgc_syn_newrange(
        struct pgc_ast *min, 
        struct pgc_ast *max);

/**
 * Construct a new repeat node.
 * @param range The numeric range.
 * @param exp The sub-expression.
 * @return A new syntax node.
 */
struct pgc_ast *pgc_syn_newrep(
        struct pgc_ast *range, 
        struct pgc_ast *exp);

/**
 * Construct a new hook node.
 * @param iden The function's identifier.
 * @return A new syntax node.
 */
struct pgc_ast *pgc_syn_newhook(char *iden);

/**
 * Construct a new call node.
 * @param fun The function.
 * @param var The variable.
 * @return A new syntax node.
 */
struct pgc_ast *pgc_syn_newcall(
        struct pgc_ast *fun,
        struct pgc_ast *var);

/**
 * Construct a new 'set' statement node.
 * @param iden The statement's identifier.
 * @param expr The statement's expression.
 * @return A new syntax node.
 */
struct pgc_ast *pgc_syn_newset(
        struct pgc_ast *iden,
        struct pgc_ast *expr);

/**
 * Construct a new 'let' statement node.
 * @param iden The statement's identifier.
 * @param expr The statement's expression.
 * @return A new syntax node.
 */
struct pgc_ast *pgc_syn_newlet(
        struct pgc_ast *iden,
        struct pgc_ast *expr);

/**
 * Construct a new 'dec' statement node.
 * @param iden The statement's identifier.
 * @param expr The statement's expression.
 * @return A new syntax node.
 */
struct pgc_ast *pgc_syn_newdec(struct pgc_ast *iden);

/**
 * Construct a new 'def' statement node.
 * @param iden The statement's identifier.
 * @param expr The statement's expression.
 * @return A new syntax node.
 */
struct pgc_ast *pgc_syn_newdef(
        struct pgc_ast *iden,
        struct pgc_ast *expr);

/**
 * Construct a new 'src' node.
 * @param alloc The allocator.
 * @param list The list of statements.
 * @return A new 'src' node.
 */
struct pgc_ast *pgc_syn_newsrc(struct pgc_ast_lst *list);

/** 
 * Free the tree's memory.
 * @param ast The syntax tree.
 */
void pgc_syn_free(struct pgc_ast *ast);

/**
 * Get the syntax node's type. 
 * @param syn The syntax node.
 * @return The node's type.
 */
int pgc_syn_typeof(struct pgc_ast *syn);

/**
 * Get the first argument.
 * @param syn The syntax node.
 * @return The first argument.
 */
struct pgc_ast *pgc_syn_getfst(struct pgc_ast *syn);

/**
 * Get the second argument.
 * @param syn The syntax node.
 * @return The second argument.
 */
struct pgc_ast *pgc_syn_getsnd(struct pgc_ast *syn);

/**
 * Get the statement's name.
 * @param syn The syntax node.
 * @return The statement's name.
 */
struct pgc_ast *pgc_syn_getname(struct pgc_ast *syn);

/**
 * Get the statement's expression.
 * @param syn The syntax node.
 * @return The statement's expression.
 */
struct pgc_ast *pgc_syn_getexpr(struct pgc_ast *syn);

/**
 * Get the repetition's range component. 
 * @param syn The syntax node.
 * @return The expression's range component.
 */
struct pgc_ast *pgc_syn_getrange(struct pgc_ast *syn);

/**
 * Get the node's sub-expression. 
 * @param syn The syntax node.
 * @return The expression's sub-expression.
 */
struct pgc_ast *pgc_syn_getsubex(struct pgc_ast *syn);

/**
 * Get the call node's function.
 */
struct pgc_ast *pgc_syn_getfun(struct pgc_ast *syn);

/**
 * Get the call node's var.
 */
struct pgc_ast *pgc_syn_getvar(struct pgc_ast *syn);

/**
 * Find the first statement with the given name.
 * @param syn The syntax tree.
 * @param name The statement's name
 * @return The statement, or NULL if it can not be found.
 */
struct pgc_ast *pgc_syn_findstmt(struct pgc_ast *syn, const char *name);

/**
 * Print out syntax to output stream.
 * @param file The output stream.
 * @param syn The syntax node.
 * @return -1 indicates errno error.
 */
int pgc_syn_fprint(FILE *file, struct pgc_ast *syn);

#endif