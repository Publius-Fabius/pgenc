#include "pgenc/buffer.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

struct pgc_buf *pgc_buf_init(
        struct pgc_buf *buf,
        void *addr,
        const size_t length,
        const size_t end)
{
        buf->addr = addr;
        buf->max = length;
        buf->end = end;
        buf->offset = 0;
        buf->begin = 0;
        return buf;
}

struct pgc_buf *pgc_buf_lens(
        struct pgc_buf *buf,
        struct pgc_buf *src,
        const size_t nbytes)
{
        PGC_ASSERT(src->begin <= src->offset);
        PGC_ASSERT(src->offset <= src->end);
        PGC_ASSERT(nbytes <= src->end - src->offset)
        buf->addr = (uint8_t*)src->addr + (src->offset - src->begin);
        buf->max = nbytes;
        buf->end = nbytes;
        buf->offset = 0;
        buf->begin = 0;
        return buf;
}

size_t pgc_buf_max(struct pgc_buf *buf)
{
        return buf->max;
}

size_t pgc_buf_end(struct pgc_buf *buf)
{
        return buf->end;
}

size_t pgc_buf_tell(struct pgc_buf *buf)
{
        return buf->offset;
}

enum pgc_err pgc_buf_seek(struct pgc_buf *buf, const size_t offset)
{
        if((offset < buf->begin) || (buf->end < offset)) {
                return PGC_ERR_OOB;
        } else {
                buf->offset = offset;
                return PGC_ERR_OK;
        }
}

static void pgc_buf_backcopy(struct pgc_buf *buf) 
{
        uint8_t *addr = buf->addr;
        memmove(addr, 
                addr + buf->offset - buf->begin, 
                buf->end - buf->offset);
        buf->begin = buf->offset;
}

/** 
 * Ensure buffer has enough available space. 
 * NOTE: PGC_BUF_ENSURE can modify BUF->begin 
 */
#define PGC_BUF_ENSURE(BUF, NEW_END) \
        if(BUF->max < ((NEW_END) - BUF->offset)) { \
                return PGC_ERR_OOB; } \
        if(BUF->max < ((NEW_END) - BUF->begin)) { \
                pgc_buf_backcopy(BUF); }

enum pgc_err pgc_buf_put(struct pgc_buf *b, void *src, const size_t nb)
{
        const size_t new_end = b->end + nb;
        /* NOTE: PGC_BUF_ENSURE can modify b->begin */
        PGC_BUF_ENSURE(b, new_end);
        memcpy(((uint8_t*)b->addr) + (b->end - b->begin), src, nb);
        b->end = new_end;
        return PGC_ERR_OK;
}

enum pgc_err pgc_buf_get(
        struct pgc_buf *b, 
        void *dest, 
        const size_t nb)
{
        const size_t new_offset = b->offset + nb;
        uint8_t *addr = ((uint8_t*)b->addr) + (b->offset - b->begin);
        if(b->end < new_offset) {
                return PGC_ERR_OOB;
        } else {
                memcpy(dest, addr, nb);
                b->offset = new_offset;
                return PGC_ERR_OK;
        }
}

enum pgc_err pgc_buf_cmp(
        struct pgc_buf *b, 
        void *str, 
        const size_t nb)
{
        const size_t new_offset = b->offset + nb;
        uint8_t *addr = ((uint8_t*)b->addr) + (b->offset - b->begin);
        if(b->end < new_offset) {
                return PGC_ERR_OOB;
        } else if(memcmp(addr, str, nb) == 0) {
                b->offset = new_offset;
                return PGC_ERR_OK;
        } else {
                return PGC_ERR_CMP;
        }   
}

enum pgc_err pgc_buf_match(
        struct pgc_buf *b, 
        int (*pred)(void *st, const int c), 
        void *st)
{
        const size_t new_offset = b->offset + 1;
        uint8_t *c = ((uint8_t*)b->addr) + (b->offset - b->begin);
        if(b->end < new_offset) {
                return PGC_ERR_OOB;
        } else if(pred(st, *c)) {
                b->offset = new_offset;
                return PGC_ERR_OK;
        } else {
                return PGC_ERR_CMP;
        }
}

/*
        Begin decode_utf8 section.

        Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>

        See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
*/

#define UTF8_ACCEPT 0
#define UTF8_REJECT 1

static const uint8_t utf8d[] = 
{
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 00..1f */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 20..3f */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 40..5f */
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 60..7f */
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, /* 80..9f */
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, /* a0..bf */
  8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, /* c0..df */
  0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, /* e0..ef */
  0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, /* f0..ff */
  0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, /* s0..s0 */
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, /* s1..s2 */
  1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, /* s3..s4 */
  1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, /* s5..s6 */
  1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* s7..s8 */
};

static uint32_t decode_utf8(
        uint32_t *state, 
        uint32_t *codep, 
        uint32_t byte) 
{
        uint32_t type = utf8d[byte];

        *codep = (*state != UTF8_ACCEPT) ?
        (byte & 0x3fu) | (*codep << 6) :
        (0xff >> type) & (byte);

        *state = utf8d[256 + *state*16 + type];
        return *state;
}

/*
        End decode_utf8 section.  Thanks Bjoern!
*/

enum pgc_err pgc_buf_matchutf8(
        struct pgc_buf *b,
        int (*pred)(void *st, const uint32_t c), 
        void *st)
{
        uint8_t *addr = ((uint8_t*)b->addr) + (b->offset - b->begin);
        uint32_t utf_state = UTF8_ACCEPT;
        uint32_t utf_codep = 0;
        for(size_t i = 0; i < 4; ++i) {
                const size_t new_offset = b->offset + i + 1;
                if(b->end < new_offset) {
                        return PGC_ERR_OOB;
                }
                decode_utf8(&utf_state, &utf_codep, addr[i]);
                if(utf_state == UTF8_ACCEPT) {
                        if(pred(st, utf_codep)) {
                                b->offset = new_offset;
                                return PGC_ERR_OK;
                        } else {
                                return PGC_ERR_CMP;
                        }
                } else if(utf_state == UTF8_REJECT) {
                        return PGC_ERR_ENC;
                } 
        }
        return PGC_ABORT();
}

enum pgc_err pgc_buf_scan(
        struct pgc_buf *buf,
        void *bytes,
        const size_t nbytes)
{
        for(;;) {
                size_t beg = pgc_buf_tell(buf);
                enum pgc_err err = pgc_buf_cmp(buf, bytes, nbytes);
                switch(err) {
                        case PGC_ERR_OK:
                                PGC_TRY_QUIETLY(pgc_buf_seek(buf, beg));
                                return PGC_ERR_OK;
                        case PGC_ERR_CMP: 
                                PGC_TRY_QUIETLY(pgc_buf_seek(buf, beg + 1));
                                break;
                        default: 
                                PGC_TRY_QUIETLY(pgc_buf_seek(buf, beg));
                                return err;
                }
        }
}

enum pgc_err pgc_buf_read(
        struct pgc_buf *b, 
        int fd, 
        const size_t nb)
{
        const size_t new_end = b->end + nb;
        /* NOTE: PGC_BUF_ENSURE can modify b->begin */
        PGC_BUF_ENSURE(b, new_end);
        uint8_t *addr = ((uint8_t*)b->addr) + (b->end - b->begin);
        const ssize_t result = read(fd, addr, nb);
        if(result < 0) {
                return PGC_ERR_SYS;
        } else if(result < 1) {
                return PGC_ERR_EOF;
        } else {
                b->end += (size_t)result;
                return PGC_ERR_OK;
        }
}

enum pgc_err pgc_buf_fread(
        struct pgc_buf *b, 
        FILE *file, 
        const size_t nb)
{
        const size_t new_end = b->end + nb;
        /* NOTE: PGC_BUF_ENSURE can modify b->begin */
        PGC_BUF_ENSURE(b, new_end);
        uint8_t *addr = ((uint8_t*)b->addr) + (b->end - b->begin);
        const size_t result = fread(addr, 1, nb, file);
        if(ferror(file)) {
                return PGC_ERR_SYS;
        } if((result == 0) && feof(file)) {
                return PGC_ERR_EOF;
        } else {
                b->end += result;
                return PGC_ERR_OK;
        }
}

enum pgc_err pgc_buf_write(
        struct pgc_buf *b, 
        int fd, 
        const size_t nb)
{
        const size_t new_offset = b->offset + nb;
        uint8_t *addr = ((uint8_t*)b->addr) + (b->offset - b->begin);
        if(b->end < new_offset) {
                return PGC_ERR_OOB;
        } 
        const ssize_t result = write(fd, addr, nb);
        if(result < 0) {
                return PGC_ERR_SYS;
        } else {
                b->offset += (size_t)result;
                return PGC_ERR_OK;
        }
}

enum pgc_err pgc_buf_fwrite(
        struct pgc_buf *b, 
        FILE *file, 
        const size_t nb)
{
        const size_t new_offset = b->offset + nb;
        uint8_t *addr = ((uint8_t*)b->addr) + (b->offset - b->begin);
        if(b->end < new_offset) {
                return PGC_ERR_OOB;
        } 
        const size_t result = fwrite(addr, 1, nb, file);
        if(ferror(file)) {
                return PGC_ERR_SYS;
        } else {
                b->offset += (size_t)result;
                return PGC_ERR_OK;
        }
}

enum pgc_err pgc_buf_printf(
        struct pgc_buf *buf,
        const char *format,
        ...)
{
        va_list ap;
        va_start(ap, format);
        return pgc_buf_vprintf(buf, format, ap);
}

enum pgc_err pgc_buf_vprintf(
        struct pgc_buf *b,
        const char *format,
        va_list ap)
{
        if(b->begin < b->offset) {
                pgc_buf_backcopy(b);
        }
        const size_t chunk = b->end - b->begin;
        const size_t nbytes = b->max - chunk;
        uint8_t *addr = ((uint8_t*)b->addr) + chunk;
        const int result = vsnprintf((void*)addr, nbytes, format, ap);
        if(result < 0) {
                return PGC_ERR_SYS;
        } else if(result > nbytes) {
                /* Output was truncated, vsnprintf returns the number of 
                   bytes it wanted to print.  Also, vsnprintf attempts 
                   to write a trailing NULL char, but the value of result 
                   doesn't include that. */
                return PGC_ERR_OOB;
        } else {
                /* Increment by result to forget the trailing NULL 
                   char (if it wasn't truncated). */
                b->end += (size_t)result;
                return PGC_ERR_OK;
        }
}

enum pgc_err pgc_buf_getchar(
        struct pgc_buf *buf, 
        char *result)
{
        const size_t offset = buf->offset;
        if(buf->end < offset + 1) {
                return PGC_ERR_OOB;
        }
        char *addr = ((char*)buf->addr) + (offset - buf->begin);
        *result = *addr;
        buf->offset += 1;
        return PGC_ERR_OK;
}

enum pgc_err pgc_buf_getutf8(
        struct pgc_buf *b,
        uint32_t *result)
{
        uint8_t *addr = ((uint8_t*)b->addr) + (b->offset - b->begin);
        uint32_t utf_state = UTF8_ACCEPT;
        uint32_t utf_codep = 0;
        for(size_t i = 0; i < 4; ++i) {
                const size_t new_offset = b->offset + i + 1;
                if(b->end < new_offset) {
                        return PGC_ERR_OOB;
                }
                decode_utf8(&utf_state, &utf_codep, addr[i]);
                if(utf_state == UTF8_ACCEPT) {
                        *result = utf_codep;
                        b->offset = new_offset;
                        return PGC_ERR_OK;
                } else if(utf_state == UTF8_REJECT) {
                        return PGC_ERR_ENC;
                } 
        }
        return PGC_ABORT();
}

enum pgc_err pgc_buf_encode(
        struct pgc_buf *buf,
        const size_t base,
        void *value,
        pgc_buf_encode_iter_t next,
        pgc_buf_encode_dict_t dict)
{
        const size_t end = buf->end;
        uint8_t *addr = ((uint8_t*)buf->addr) + (end - buf->begin);
        uint8_t digit;
        size_t len = 0;

        /* Iterate through value until it reaches zero. */
        int notzero = 1;
        do {
                PGC_BUF_ENSURE(buf, end + (len + 1));
                /* Get numeric representation of the next "digit."" */
                notzero = next(base, value, &digit);
                /* Lookup the "digit's" symbol and store it. */
                PGC_TRY_QUIETLY(dict(digit, addr + len++));
        } while(notzero); 

        if(!len) {
                /* Something went wrong. */
                PGC_ABORT();
        }

        /* Reverse the encoded string's ordering. */
        for (size_t j = 0, k = len - 1; j < k; ++j, --k) {
                uint8_t temp = addr[j];
                addr[j] = addr[k];
                addr[k] = temp;
        }

        buf->end = end + len;
        return PGC_ERR_OK;
}

enum pgc_err pgc_buf_encode_dec(const uint8_t digit, uint8_t *symbol)
{
        if(0 <= digit && digit <= 9) {
                 *symbol = '0' + digit;
                 return PGC_ERR_OK;
        }
        return PGC_ERR_ENC;
}

enum pgc_err pgc_buf_encode_hex(const uint8_t digit, uint8_t *symbol)
{
        if(0 <= digit && digit <= 9) {
                 *symbol = '0' + digit;
                 return PGC_ERR_OK;
        } else if(10 <= digit && digit <= 15) {
                *symbol = 'a' + digit - 10;
                return PGC_ERR_OK;
        } 
        return PGC_ERR_ENC;  
}

int pgc_buf_encode_uint8(const size_t base, void *value, uint8_t *digit)
{
        uint8_t *typed_value = value;
        uint8_t typed_base = (uint8_t)base;
        *digit = *typed_value % typed_base;
        *typed_value /= typed_base;
        return *typed_value != 0;
}

int pgc_buf_encode_uint16(const size_t base, void *value, uint8_t *digit)
{
        uint16_t *typed_value = value;
        uint16_t typed_base = (uint16_t)base;
        *digit = (uint8_t)(*typed_value % typed_base);
        *typed_value /= typed_base;
        return *typed_value != 0;
}

int pgc_buf_encode_uint32(const size_t base, void *value, uint8_t *digit)
{
        uint32_t *typed_value = value;
        uint32_t typed_base = (uint32_t)base;
        *digit = (uint8_t)(*typed_value % typed_base);
        *typed_value /=typed_base;
        return *typed_value != 0;
}

int pgc_buf_encode_uint64(const size_t base, void *value, uint8_t *digit)
{
        uint64_t *typed_value = value;
        uint64_t typed_base = (uint64_t)base;
        *digit = (uint8_t)(*typed_value % typed_base);
        *typed_value /= typed_base;
        return *typed_value != 0;
}

enum pgc_err pgc_buf_decode(
        struct pgc_buf *buf,
        const size_t len,
        const size_t base,
        pgc_buf_decode_dict_t dict,
        pgc_buf_decode_accum_t accum,
        void *result)
{
        const size_t offset = buf->offset;
        const size_t begin = buf->begin;
        const size_t end = buf->end;
        if((end - offset) < len) {
                return PGC_ERR_OOB;
        }
        uint8_t *addr = ((uint8_t*)buf->addr) + (offset - begin);
        for(size_t n = 0; n < len; ++n) {
                uint8_t value;
                PGC_TRY_QUIETLY(dict(addr[n], &value));
                PGC_TRY_QUIETLY(accum(base, value, result));
        }
        buf->offset += len;
        return PGC_ERR_OK;
}

enum pgc_err pgc_buf_decode_dec(const uint8_t symbol, uint8_t *value)
{
        if('0' <= symbol && symbol <= '9') {
                *value = symbol - '0';
                return PGC_ERR_OK;
        } else {
                return PGC_ERR_ENC;
        }
}

enum pgc_err pgc_buf_decode_hex(const uint8_t symbol, uint8_t *value)
{
        const uint8_t c = (uint8_t)tolower(symbol);
        if('0' <= c && c <= '9') {
                *value = c - '0';
                return PGC_ERR_OK;
        } else if('a' <= c && c <= 'f') {
                *value = c - 'a' + 10;
                return PGC_ERR_OK;
        } else {
                return PGC_ERR_ENC;
        }
}

enum pgc_err pgc_buf_decode_uint8(
        const size_t base, 
        const uint8_t value, 
        void *result)
{
        uint8_t *typed_result = result;
        *typed_result *= (uint8_t)base;
        *typed_result += value;
        return PGC_ERR_OK;
}

enum pgc_err pgc_buf_decode_uint16(
        const size_t base, 
        const uint8_t value, 
        void *result)
{
        uint16_t *typed_result = result;
        *typed_result *= (uint16_t)base;
        *typed_result += value;
        return PGC_ERR_OK;
}

enum pgc_err pgc_buf_decode_uint32(
        const size_t base, 
        const uint8_t value, 
        void *result)
{
        uint32_t *typed_result = result;
        *typed_result *= (uint32_t)base;
        *typed_result += value;
        return PGC_ERR_OK;
}

enum pgc_err pgc_buf_decode_uint64(
        const size_t base, 
        const uint8_t value, 
        void *result)
{
        uint64_t *typed_result = result;
        *typed_result *= (uint64_t)base;
        *typed_result += value;
        return PGC_ERR_OK;
}
