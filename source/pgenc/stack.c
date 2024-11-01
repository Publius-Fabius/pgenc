
#include "pgenc/stack.h"
#include "pgenc/error.h"
#include <stdint.h>

size_t pgc_stk_offset(struct pgc_stk *stk)
{
        return stk->offset;
}

size_t pgc_stk_length(struct pgc_stk *stk)
{
        return stk->length;
}

struct pgc_stk *pgc_stk_init(
        struct pgc_stk *stack, 
        void *base,
        const size_t length)
{
        stack->base = base;
        stack->length = length;
        stack->offset = length;
        return stack;
}

void *pgc_stk_push(struct pgc_stk *stack, const size_t nbytes)
{
        SEL_ASSERT(stack && stack->offset <= stack->length);
        if(stack->offset < nbytes)
                return NULL;
        stack->offset -= nbytes;
        return ((char*)stack->base) + stack->offset;
}

void *pgc_stk_pop(struct pgc_stk *stack, const size_t nbytes)
{
        SEL_ASSERT(stack && stack->offset <= stack->length);
        const size_t old_offset = stack->offset;
        const size_t new_offset = old_offset + nbytes;
        if(stack->length < new_offset) 
                return NULL;
        stack->offset = new_offset;
        return ((char*)stack->base) + old_offset;
}

void *pgc_stk_peek(struct pgc_stk *stack, const size_t nbytes)
{
        SEL_ASSERT(stack && stack->offset <= stack->length);
        const size_t index = stack->offset + nbytes;
        if(stack->length <= index) 
                return NULL;
        return ((char*)stack->base) + index;
}
