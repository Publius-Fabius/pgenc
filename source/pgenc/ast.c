#include "pgenc/ast.h"
#include "pgenc/error.h"

int16_t pgc_ast_typeof(struct pgc_ast *ast)
{
        return ast->atag;
}

int16_t pgc_ast_utype(struct pgc_ast *ast)
{
        return ast->utag;
}

struct pgc_ast *pgc_ast_initany(
        struct pgc_ast *node,
        const int16_t atag,
        const int16_t utag)
{
        node->atag = atag;
        node->utag = utag;
        for(int x = 0; x < 8; ++x) {
                node->u.any.bits[x] = 0;
        }
        return node;
}

struct pgc_ast *pgc_ast_initint8(
        struct pgc_ast *node, 
        const int8_t value, 
        const int16_t tag)
{
        node->atag = PGC_AST_INT8;
        node->utag = tag;
        node->u.int8 = value;
        return node;
}

struct pgc_ast *pgc_ast_inituint8(
        struct pgc_ast *node, 
        const uint8_t value, 
        const int16_t tag)
{
        node->atag = PGC_AST_UINT8;
        node->utag = tag;
        node->u.uint8 = value;
        return node;
}

struct pgc_ast *pgc_ast_initint16(
        struct pgc_ast *node, 
        const int16_t value, 
        const int16_t tag)
{
        node->atag = PGC_AST_INT16;
        node->utag = tag;
        node->u.int16 = value;
        return node;
}

struct pgc_ast *pgc_ast_inituint16(
        struct pgc_ast *node, 
        const uint16_t value, 
        const int16_t tag)
{
        node->atag = PGC_AST_UINT16;
        node->utag = tag;
        node->u.uint16 = value;
        return node;
}

struct pgc_ast *pgc_ast_initint32(
        struct pgc_ast *node, 
        const int32_t value, 
        const int16_t tag)
{
        node->atag = PGC_AST_INT32;
        node->utag = tag;
        node->u.int32 = value;
        return node;
}

struct pgc_ast *pgc_ast_inituint32(
        struct pgc_ast *node, 
        const uint32_t value, 
        const int16_t tag)
{
        node->atag = PGC_AST_UINT32;
        node->utag = tag;
        node->u.uint32 = value;
        return node;
}

struct pgc_ast *pgc_ast_initint64(
        struct pgc_ast *node, 
        const int64_t value, 
        const int16_t tag)
{
        node->atag = PGC_AST_INT64;
        node->utag = tag;
        node->u.int64 = value;
        return node;
}

struct pgc_ast *pgc_ast_inituint64(
        struct pgc_ast *node, 
        const uint64_t value, 
        const int16_t tag)
{
        node->atag = PGC_AST_UINT64;
        node->utag = tag;
        node->u.uint64 = value;
        return node;
}

struct pgc_ast *pgc_ast_initfloat32(
        struct pgc_ast *node, 
        const float value, 
        const int16_t tag)
{
        node->atag = PGC_AST_FLOAT32;
        node->utag = tag;
        node->u.float32 = value;
        return node;
}

struct pgc_ast *pgc_ast_initfloat64(
        struct pgc_ast *node, 
        const double value, 
        const int16_t tag)
{
        node->atag = PGC_AST_FLOAT64;
        node->utag = tag;
        node->u.float64 = value;
        return node;
}

struct pgc_ast *pgc_ast_initstr(
        struct pgc_ast *node, 
        char *value, 
        const int16_t tag)
{
        node->atag = PGC_AST_STR;
        node->utag = tag;
        node->u.str = value;
        return node;
}

struct pgc_ast *pgc_ast_initlst(
        struct pgc_ast *node, 
        struct pgc_ast_lst *value, 
        const int16_t tag)
{
        node->atag = PGC_AST_LST;
        node->utag = tag;
        node->u.lst = value;
        return node;
}

int8_t pgc_ast_toint8(struct pgc_ast *a)
{
        PGC_ASSERT(a->atag == PGC_AST_INT8);
        return a->u.int8;
}

uint8_t pgc_ast_touint8(struct pgc_ast *a)
{
        PGC_ASSERT(a->atag == PGC_AST_UINT8);
        return a->u.uint8;
}

int16_t pgc_ast_toint16(struct pgc_ast *a)
{
        PGC_ASSERT(a->atag == PGC_AST_INT16);
        return a->u.int16;
}

uint16_t pgc_ast_touint16(struct pgc_ast *a)
{
        PGC_ASSERT(a->atag == PGC_AST_UINT16);
        return a->u.uint16;
}

int32_t pgc_ast_toint32(struct pgc_ast *a)
{
        PGC_ASSERT(a->atag == PGC_AST_INT32);
        return a->u.int32;
}

uint32_t pgc_ast_touint32(struct pgc_ast *a)
{
        PGC_ASSERT(a->atag == PGC_AST_UINT32);
        return a->u.uint32;
}

int64_t pgc_ast_toint64(struct pgc_ast *a)
{
        PGC_ASSERT(a->atag == PGC_AST_INT64);
        return a->u.int64;
}

uint64_t pgc_ast_touint64(struct pgc_ast *a)
{
        PGC_ASSERT(a->atag == PGC_AST_UINT64);
        return a->u.uint64;
}

float pgc_ast_tofloat32(struct pgc_ast *a)
{
        PGC_ASSERT(a->atag == PGC_AST_FLOAT32);
        return a->u.float32;
}

double pgc_ast_tofloat64(struct pgc_ast *a)
{
        PGC_ASSERT(a->atag == PGC_AST_FLOAT64);
        return a->u.float64;
}

char *pgc_ast_tostr(struct pgc_ast *a)
{
        PGC_ASSERT(a->atag == PGC_AST_STR);
        return a->u.str;
}

struct pgc_ast_lst *pgc_ast_tolst(struct pgc_ast *a)
{
        PGC_ASSERT(a->atag == PGC_AST_LST);
        return a->u.lst;
}

struct pgc_ast_lst *pgc_ast_rev(struct pgc_ast_lst *lst)
{
        struct pgc_ast_lst *res = NULL;
        while(lst) {
                struct pgc_ast_lst *tmp = lst->nxt;
                lst->nxt = res;
                res = lst;
                lst = tmp;
        }
        return res;
}

struct pgc_ast_lst *pgc_ast_cat(
        struct pgc_ast_lst *a, 
        struct pgc_ast_lst *b)
{
        if(!a) {
                return b;
        } 
        while(a->nxt) {
                a = a->nxt;
        }
        a->nxt = b;
        return a;
}

struct pgc_ast_lst *pgc_ast_at(struct pgc_ast_lst *a, const size_t i)
{
        for(size_t n = 0; n < i; ++n) {
                PGC_ASSERT(a);
                a = a->nxt;
        }
        return a;
}

size_t pgc_ast_len(struct pgc_ast_lst *l)
{
        size_t n;
        for(n = 0; l; l = l->nxt) {
                n += 1;
        }
        return n;
}