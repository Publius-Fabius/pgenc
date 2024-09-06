#ifndef PGENC_AST_H
#define PGENC_AST_H

#include <stddef.h>
#include <stdint.h>

/** AST Type */
enum pgc_ast_tag {                   
        PGC_AST_INT8,                           /** 8bit int */
        PGC_AST_UINT8,                          /** 8bit unsigned int */
        PGC_AST_INT16,                          /** 16bit int */
        PGC_AST_UINT16,                         /** 16bit unsigned int */
        PGC_AST_INT32,                          /** 32bit int */
        PGC_AST_UINT32,                         /** 32bit unsigned int */
        PGC_AST_INT64,                          /** 64bit int */
        PGC_AST_UINT64,                         /** 64bit unsigned int */
        PGC_AST_FLOAT32,                        /** 32bit float */
        PGC_AST_FLOAT64,                        /** 64bit float */
        PGC_AST_STR,                            /** string value */
        PGC_AST_LST                             /** list value */
};

struct pgc_ast_lst;

/** (A)bstract (S)yntax (T)ree Node */
struct pgc_ast {                            
        int16_t atag;                           /** AST tag */
        int16_t utag;                           /** User tag */
        union {
                int8_t int8;                    /** int8_t value */
                uint8_t uint8;                  /** uint8_t value */
                int16_t int16;                  /** int16_t value */
                uint16_t uint16;                /** uint16_t value */
                int32_t int32;                  /** int32_t value */
                uint32_t uint32;                /** uint32_t value */
                int64_t int64;                  /** int64_t value */
                uint64_t uint64;                /** uint64_t value */
                float float32;                  /** float value */
                double float64;                 /** double value */
                char *str;                      /** string value */
                struct pgc_ast_lst *lst;        /** list value */
                struct {
                        uint8_t bits[8];
                } any;                          /** Any size. */
        } u;
};

/** AST Linked List */
struct pgc_ast_lst {                            
        struct pgc_ast *val;                    /** Value */
        struct pgc_ast_lst *nxt;                /** Next node */
};

/**
 * Get the node's base type.
 * @param ast The node.
 * @return The node's type.
 */
int16_t pgc_ast_typeof(struct pgc_ast *ast);

/**
 * Get the node's user type.
 */
int16_t pgc_ast_utype(struct pgc_ast *ast);

/**
 * Initialize a generic node, setting tags and zeroing the variant.
 * @param node The node to initialize.
 * @param atag The AST level tag.
 * @param utag The user tag.
 * @return node
 */
struct pgc_ast *pgc_ast_initany(
        struct pgc_ast *node,
        const int16_t atag,
        const int16_t utag);

/**
 * Initializes a `pgc_ast` node with an int8 value.
 *
 * @param node Pointer to the AST node to initialize. 
 * @param value The value to store in the node.
 * @param tag The node's user tag.
 * @return A pointer to the initialized node.
 */
struct pgc_ast *pgc_ast_initint8(
        struct pgc_ast *node, 
        const int8_t value,
        const int16_t tag);

/**
 * Initializes a `pgc_ast` node with an uint8 value.
 *
 * @param node Pointer to the AST node to initialize. 
 * @param value The value to store in the node.
 * @param tag The node's user tag.
 * @return A pointer to the initialized node.
 */
struct pgc_ast *pgc_ast_inituint8(
        struct pgc_ast *node, 
        const uint8_t value,
        const int16_t tag);

/**
 * Initializes a `pgc_ast` node with an int16 value.
 *
 * @param node Pointer to the AST node to initialize. 
 * @param value The value to store in the node.
 * @param tag The node's user tag.
 * @return A pointer to the initialized node.
 */
struct pgc_ast *pgc_ast_initint16(
        struct pgc_ast *node, 
        const int16_t value,
        const int16_t tag);

/**
 * Initializes a `pgc_ast` node with an uint16 value.
 *
 * @param node Pointer to the AST node to initialize. 
 * @param value The value to store in the node.
 * @param tag The node's user tag.
 * @return A pointer to the initialized node.
 */
struct pgc_ast *pgc_ast_inituint16(
        struct pgc_ast *node, 
        const uint16_t value,
        const int16_t tag);

/**
 * Initializes a `pgc_ast` node with an int32 value.
 *
 * @param node Pointer to the AST node to initialize. 
 * @param value The value to store in the node.
 * @param tag The node's user tag.
 * @return A pointer to the initialized node.
 */
struct pgc_ast *pgc_ast_initint32(
        struct pgc_ast *node, 
        const int32_t value,
        const int16_t tag);

/**
 * Initializes a `pgc_ast` node with an uint32 value.
 *
 * @param node Pointer to the AST node to initialize. 
 * @param value The value to store in the node.
 * @param tag The node's user tag.
 * @return A pointer to the initialized node.
 */
struct pgc_ast *pgc_ast_inituint32(
        struct pgc_ast *node, 
        const uint32_t value,
        const int16_t tag);

/**
 * Initializes a `pgc_ast` node with an int64 value.
 *
 * @param node Pointer to the AST node to initialize. 
 * @param value The value to store in the node.
 * @param tag The node's user tag.
 * @return A pointer to the initialized node.
 */
struct pgc_ast *pgc_ast_initint64(
        struct pgc_ast *node, 
        const int64_t value,
        const int16_t tag);

/**
 * Initializes a `pgc_ast` node with an uint64 value.
 *
 * @param node Pointer to the AST node to initialize. 
 * @param value The value to store in the node.
 * @param tag The node's user tag.
 * @return A pointer to the initialized node.
 */
struct pgc_ast *pgc_ast_inituint64(
        struct pgc_ast *node, 
        const uint64_t value,
        const int16_t tag);

/**
 * Initializes a `pgc_ast` node with an float32 value.
 *
 * @param node Pointer to the AST node to initialize. 
 * @param value The value to store in the node.
 * @param tag The node's user tag.
 * @return A pointer to the initialized node.
 */
struct pgc_ast *pgc_ast_initfloat32(
        struct pgc_ast *node, 
        const float value,
        const int16_t tag);

/**
 * Initializes a `pgc_ast` node with an float64 value.
 *
 * @param node Pointer to the AST node to initialize. 
 * @param value The value to store in the node.
 * @param tag The node's user tag.
 * @return A pointer to the initialized node.
 */
struct pgc_ast *pgc_ast_initfloat64(
        struct pgc_ast *node, 
        const double value,
        const int16_t tag);

/**
 * Initializes a `pgc_ast` node with a string value.  This routine stores the
 * string as is, not performing a copy.
 *
 * @param node Pointer to the AST node to initialize. 
 * @param value The string value to store in the node.
 * @param tag The node's user tag.
 * @return A pointer to the initialized node.
 */
struct pgc_ast *pgc_ast_initstr(
        struct pgc_ast *node, 
        char *value,
        const int16_t tag);

/**
 * Initializes a `pgc_ast` node with a list value.  
 *
 * @param a Pointer to the AST node to initialize. 
 * @param l The list value to store in the node.
 * @param tag The node's user tag.
 * @return A pointer to the initialized node.
 */
struct pgc_ast *pgc_ast_initlst(
        struct pgc_ast *a, 
        struct pgc_ast_lst *l,
        const int16_t tag);

/**
 * Convert the `pgc_ast` node to a int8, potentially aborting the process 
 * if the type doesn't match.
 * @param a Pointer to the AST node to convert.
 * @return A double value.
 */
int8_t pgc_ast_toint8(struct pgc_ast *a);

/**
 * Convert the `pgc_ast` node to a uint8, potentially aborting the process 
 * if the type doesn't match.
 * @param a Pointer to the AST node to convert.
 * @return A double value.
 */
uint8_t pgc_ast_touint8(struct pgc_ast *a);

/**
 * Convert the `pgc_ast` node to a int16, potentially aborting the process 
 * if the type doesn't match.
 * @param a Pointer to the AST node to convert.
 * @return A double value.
 */
int16_t pgc_ast_toint16(struct pgc_ast *a);

/**
 * Convert the `pgc_ast` node to a uint16, potentially aborting the process 
 * if the type doesn't match.
 * @param a Pointer to the AST node to convert.
 * @return A double value.
 */
uint16_t pgc_ast_touint16(struct pgc_ast *a);

/**
 * Convert the `pgc_ast` node to a int32, potentially aborting the process 
 * if the type doesn't match.
 * @param a Pointer to the AST node to convert.
 * @return A double value.
 */
int32_t pgc_ast_toint32(struct pgc_ast *a);

/**
 * Convert the `pgc_ast` node to a uint32, potentially aborting the process 
 * if the type doesn't match.
 * @param a Pointer to the AST node to convert.
 * @return A double value.
 */
uint32_t pgc_ast_touint32(struct pgc_ast *a);

/**
 * Convert the `pgc_ast` node to a int64, potentially aborting the process 
 * if the type doesn't match.
 * @param a Pointer to the AST node to convert.
 * @return A double value.
 */
int64_t pgc_ast_toint64(struct pgc_ast *a);

/**
 * Convert the `pgc_ast` node to a uint64, potentially aborting the process 
 * if the type doesn't match.
 * @param a Pointer to the AST node to convert.
 * @return A double value.
 */
uint64_t pgc_ast_touint64(struct pgc_ast *a);

/**
 * Convert the `pgc_ast` node to a float32, potentially aborting the process 
 * if the type doesn't match.
 * @param a Pointer to the AST node to convert.
 * @return A double value.
 */
float pgc_ast_tofloat32(struct pgc_ast *a);

/**
 * Convert the `pgc_ast` node to a float64, potentially aborting the process 
 * if the type doesn't match.
 * @param a Pointer to the AST node to convert.
 * @return A double value.
 */
double pgc_ast_tofloat64(struct pgc_ast *a);

/**
 * Convert the `pgc_ast` node to a string, potentially aborting the process 
 * if the type doesn't match.
 * @param a Pointer to the AST node to convert.
 * @return A string value
 */
char * pgc_ast_tostr(struct pgc_ast *a);

/**
 * Convert the `pgc_ast` node to a list, potentially aborting the process 
 * if the type doesn't match.
 * @param a Pointer to the AST node to convert.
 * @return A list value
 */
struct pgc_ast_lst * pgc_ast_tolst(struct pgc_ast *a);

/**
 * Get `pgc_ast_list` node at index.
 * @param a The list.
 * @param i The index.
 * @return The node at the given index.
 */
struct pgc_ast_lst * pgc_ast_at(struct pgc_ast_lst *a, const size_t i);

/**
 * Reverse the `pgc_ast_list` in place.
 * @param l Pointer to the AST list.
 * @return The reversed list.
 */
struct pgc_ast_lst * pgc_ast_rev(struct pgc_ast_lst *l);

/**
 * Get the list's length. 
 * @param l The list.
 * @return The list's length.
 */
size_t pgc_ast_len(struct pgc_ast_lst *l);

/**
 * Concatenate the lists.
 * @param a The first list.
 * @param b The second list.
 * @return The concatenated list.
 */
struct pgc_ast_lst * pgc_ast_cat(
        struct pgc_ast_lst *a, 
        struct pgc_ast_lst *b);

#endif