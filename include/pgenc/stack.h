#ifndef PGENC_STACK_H
#define PGENC_STACK_H

#include <stddef.h>

/** Byte Stack */
struct pgc_stk {
        size_t max;
        size_t size;
        void *bytes;
};

/**
 * Get the stack's current size.
 * @param stk The stack.
 * @return The stack's current size.
 */
size_t pgc_stk_size(struct pgc_stk *stk);

/**
 * Get the stack's maximum size.
 * @param stk The stack.
 * @return The stack's maximum size.
 */
size_t pgc_stk_max(struct pgc_stk *stk);

/**
 * Initialize the byte stack.
 * @param stk The structure to initialize.
 * @param length The length of the stack.
 * @param bytes Memory address.
 * @return stk 
 */
struct pgc_stk *pgc_stk_init(
        struct pgc_stk *stk, 
        const size_t length, 
        void *bytes);

/**
 * Push the stack, incrementing the offset by nbytes, returning
 * a pointer to the current offset. 
 * @param stack The stack to push
 * @param nbytes The number of bytes to push.
 * @return A pointer to the current offset.
 */
void *pgc_stk_push(struct pgc_stk *stack, const size_t nbytes);

/**
 * Pop stack, decrementing the offset by nbytes, returning a
 * pointer to the new offset.
 * @param stack The stack to pop.
 * @param nbytes The number of bytes to pop.
 * @return A pointer to the new offset.
 */
void *pgc_stk_pop(struct pgc_stk *stack, const size_t nbytes);

#endif