
#include "pgenc/stack.h"
#include <stdint.h>

struct pgc_stk *pgc_stk_init(
        struct pgc_stk *stack, 
        void *bytes,
        const size_t length)
{
        stack->bytes = bytes;
        stack->max = length;
        stack->size = 0;
        return stack;
}

size_t pgc_stk_size(struct pgc_stk *stk)
{
        return stk->size;
}

size_t pgc_stk_max(struct pgc_stk *stk)
{
        return stk->max;
}

void *pgc_stk_push(struct pgc_stk *stack, const size_t nbytes)
{
        const size_t offset = stack->size;
        const size_t new_offset = offset + nbytes;
        if(stack->max < new_offset) {
                return NULL;
        }
        stack->size = new_offset;
        return ((char*)stack->bytes) + offset;
}

void *pgc_stk_pop(struct pgc_stk *stack, const size_t nbytes)
{
        if(stack->size < nbytes) {
                return NULL;
        }
        stack->size -= nbytes;
        return ((char*)stack->bytes) + stack->size;
}
