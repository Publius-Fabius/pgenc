#ifndef PGENC_STACK_H
#define PGENC_STACK_H

#include <stddef.h>

/** Byte Stack */
struct pgc_stk {
        void *base;
        size_t length, offset;
};

/**
 * Get the stack's length.
 */
size_t pgc_stk_length(struct pgc_stk *stk);


/**
 * Get the stack's offset.
 */
size_t pgc_stk_offset(struct pgc_stk *stk);

/**
 * Initialize the byte stack.
 */
struct pgc_stk *pgc_stk_init(
        struct pgc_stk *stack, 
        void *base,
        const size_t length);

/**
 * Push the stack, incrementing its size by nbytes, returning a pointer to the 
 * beginning of the pushed region.  Returns NULL if the stack is full.
 */
void *pgc_stk_push(struct pgc_stk *stack, const size_t nbytes);

/**
 * Pop the stack, decrementing its size by nbytes, returning a pointer to the 
 * beginning of the popped off region.  Returns NULL if the stack is empty.
 */
void *pgc_stk_pop(struct pgc_stk *stack, const size_t nbytes);

/**
 * Peek the stack, returning a pointer to the beginning of the region nbytes
 * from the stack's current offset.
 */
void *pgc_stk_peek(struct pgc_stk *stack, const size_t nbytes);

#endif