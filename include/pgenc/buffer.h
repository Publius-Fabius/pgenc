#ifndef PGENC_BUFFER_H
#define PGENC_BUFFER_H

#include "pgenc/error.h"
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <openssl/ssl.h>

/** 
 * I/O Buffer
 * DEFINE: [ begin, .., offset, .., end ] 
 * WHERE: (end - begin <= max) AND (begin <= offset)
 */
struct pgc_buf 
{
        void *addr;                     /** Storage region. */
        size_t begin;                   /** Beginning of buffer storage. */
        size_t offset;                  /** Current offset. */
        size_t end;                     /** End of buffer storage. */
        size_t max;                     /** Max (end - begin) length. */
};

/**
 * Initialize a buffer with the given memory.
 * 
 * @param buf The buffer to initialize
 * @param addr The address to the buffer's internal memory.
 * @param max The length of the buffer's internal memory.
 * @param end The new end value for the buffer (usually 0).
 * @return buf
 */
struct pgc_buf *pgc_buf_init(
        struct pgc_buf *buf,
        void *addr,
        const size_t max,
        const size_t end);

/**
 * Initialize a lens, nbytes in length starting at the source's offset.
 * @param buffer The buffer to initialize.
 * @param source The source buffer.
 * @param nbytes The number of bytes to view.
 * @return buffer
 */
struct pgc_buf *pgc_buf_lens(
        struct pgc_buf *buffer,
        struct pgc_buf *source,
        const size_t nbytes);

/**
 * Get the maximum number of storable bytes for the buffer.
 * @param buffer The buffer.
 * @return The maximum number of storable bytes.
 */
size_t pgc_buf_max(struct pgc_buf *buffer);

/**
 * Get the total number of bytes written to this buffer over its lifetime.
 * @param buffer The buffer.
 * @return The buffer's size.
 */
size_t pgc_buf_end(struct pgc_buf *buffer);

/** 
 * Get the buffer's current offset.
 * @param buffer The buffer.
 * @return The buffer's current offset. 
 */
size_t pgc_buf_tell(struct pgc_buf *buffer);

/** 
 * Move the buffer's offset to the given position. 
 * @param buffer The buffer.
 * @param offset The buffer's new offset.
 * @return 
 *      PGC_ERR_OK - All ok.
 *      PGC_ERR_OOB - Argument is not in storable memory.
 */
enum pgc_err pgc_buf_seek(
        struct pgc_buf *buffer, 
        const size_t offset);

/**
 * Append binary data to the end of the buffer.
 * @param buffer The buffer to write bytes into.
 * @param source The source of bytes.
 * @param nbytes The number of bytes to copy.
 * @return 
 *      PGC_ERR_OK      - All ok.
 *      PGC_ERR_OOB     - Operation would result in buffer overflow.
 **/
enum pgc_err pgc_buf_put(
        struct pgc_buf *buffer, 
        void *source, 
        const size_t nbytes);

/**
 * Copy nbytes from the buffer to the destination starting at the buffer's 
 * offset, advancing the offset by nbytes bytes.
 * @param buffer The buffer to copy bytes from.
 * @param dest The destination region to copy bytes into.
 * @param nbytes The number of bytes to copy.
 * @return 
 *      PGC_ERR_OK      - All ok.
 *      PGC_ERR_OOB     - Attempt to read past available data.
 **/
enum pgc_err pgc_buf_get(
        struct pgc_buf *buffer, 
        void *dest, 
        const size_t nbytes);

/**
 * Compare the next sequence of bytes in the buffer.  Upon success, the 
 * offset is advanced by nbytes bytes.
 * @param buffer The buffer.
 * @param bytes The bytes to compare against.
 * @param nbytes The number of bytes to compare.
 * @return 
 *      PGC_ERR_OK      - All ok.
 *      PGC_ERR_CMP     - Comparison operation failed.
 *      PGC_ERR_OOB     - Attempt to read past available data.
 **/
enum pgc_err pgc_buf_cmp(
        struct pgc_buf *buffer, 
        void *bytes, 
        const size_t nbytes);

/**
 * Match the next octet in the buffer against the predicate.  Upon success,
 * the offset is incremented by one.
 * @param buffer The buffer.
 * @param pred The matching function.
 * @param state The matching function's state.
 * @return 
 *      PGC_ERR_OK      - All ok.
 *      PGC_ERR_CMP     - The character didn't match the predicate.
 *      PGC_ERR_OOB     - Attempt to read past available data.
 */
enum pgc_err pgc_buf_match(
        struct pgc_buf *buffer, 
        int (*pred)(void *state, const int value), 
        void *state);

/**
 * Decode and match UTF8 value.  Upon success, the offset is advanced
 * by a variable number of bytes, depending on the UTF8 value's length.
 * @param buffer The buffer.
 * @param pred The matching function.
 * @param state The matching function's state.
 * @return
 *      PGC_ERR_OK      - All ok.
 *      PGC_ERR_CMP     - The predicate failed to match the UTF8 value.
 *      PGC_ERR_ENC     - Failed to decode the UTF8 value.
 *      PGC_ERR_OOB     - Attempt to read past available data.
 */
enum pgc_err pgc_buf_matchutf8(
        struct pgc_buf *buffer,
        int (*pred)(void *state, const uint32_t value), 
        void *state);

/**
 * Scan the buffer for the sequence of bytes.  The buffer's offset will be 
 * set to the beginning of the matching byte sequence if found.
 * @param buffer The buffer to scan.
 * @param bytes The bytes to scan for.
 * @param nbytes The number of bytes to scan for.
 * @return 
 *      PGC_ERR_OK      - Sequence found
 */
enum pgc_err pgc_buf_scan(
        struct pgc_buf *buffer,
        void *bytes,
        const size_t nbytes);

/**
 * Read up to nbytes bytes from a file descriptor and append them to the 
 * end of the buffer.
 * @param buffer The buffer.
 * @param fd The file descriptor to read bytes from.
 * @param nbytes The maximum number of bytes to read.
 * @return
 * PGC_ERR_OK - All ok.
 * PGC_ERR_OOB - Attempt to write past buffer.
 * PGC_ERR_EOF - The EOF was encountered.
 * PGC_ERR_SYS - System Error
 * PGC_ERR_WNT - Wants more time.
 */
enum pgc_err pgc_buf_read(
        struct pgc_buf *buffer, 
        int fd, 
        const size_t nbytes);

/**
 * Read up to nbytes bytes from a secure connection and append them to the 
 * end of the buffer.
 * @return
 * PGC_ERR_OK - All OK.
 * PGC_ERR_OOB - Attempt to write past buffer.
 * PGC_ERR_EOF - The EOF was encountered.
 * PGC_ERR_SSL - SSL Error
 */
enum pgc_err pgc_buf_sread(
        struct pgc_buf *buffer, 
        SSL *ssl, 
        const size_t nbytes,
        int *ssl_error);

/**
 * Read up to nbytes bytes from a file stream and append them to the 
 * end of the buffer.
 * @param buffer The buffer.
 * @param fd The file descriptor to read bytes from.
 * @param nbytes The maximum number of bytes to read.
 * @return
 *      PGC_ERR_OK      - All ok.
 *      PGC_ERR_OOB     - Attempt to write past storable memory.
 *      PGC_ERR_EOF     - The EOF was encountered.
 *      PGC_ERR_SYS     - System error (check errno).
 */
enum pgc_err pgc_buf_fread(
        struct pgc_buf *buffer, 
        FILE *file, 
        const size_t nbytes);

/**
 * Write up to nbytes bytes to a file descriptor.
 * @param buffer The buffer.
 * @param fd The file descriptor to write bytes to.
 * @param nbytes The number of bytes to write.
 * @return
 *      PGC_ERR_OK      - All ok.
 *      PGC_ERR_OOB     - Attempt to read past available data.
 *      PGC_ERR_SYS     - System Error
 *      PGC_ERR_WNT     - Wants more time
 */
enum pgc_err pgc_buf_write(
        struct pgc_buf *buffer, 
        int fd, 
        const size_t nbytes);

/**
 * Write up to nbytes bytes to a secure connection.
 * @return
 * PGC_ERR_OK - All OK.
 * PGC_ERR_OOB - Attempt to read past available data.
 * PGC_ERR_SSL - SSL Error
 */
enum pgc_err pgc_buf_swrite(
        struct pgc_buf *buffer, 
        SSL *ssl, 
        const size_t nbytes,
        int *ssl_error);

/**
 * Write up to nbytes bytes to a FILE stream.
 * @param buffer The buffer.
 * @param file The file to write bytes to.
 * @param nbytes The number of bytes to write.
 *      PGC_ERR_OK      - All ok.
 *      PGC_ERR_OOB     - Attempt to read past available data.
 *      PGC_ERR_SYS     - System error (check errno).
 */
enum pgc_err pgc_buf_fwrite(
        struct pgc_buf *buffer, 
        FILE *file, 
        const size_t nbytes);

/**
 * Print formatted data into buffer.  See man page on printf for details.
 * @param buffer The buffer to print formatted data to.
 * @param format The printf format.
 * @return 
 *      PGC_ERR_OK      - All ok.
 *      PGC_ERR_OOB     - Attempt to write past storable memory.
 *      PGC_ERR_SYS     - System error (check errno).
 */
enum pgc_err pgc_buf_printf(
        struct pgc_buf *buffer,
        const char *format,
        ...);

/**
 * Print formatted data into buffer.  See man page on vprintf for details.
 * @param buffer The buffer to print formatted data to.
 * @param format The printf format.
 * @param args The argument list.
 * @return 
 *      PGC_ERR_OK      - All ok.
 *      PGC_ERR_OOB     - Attempt to write past storable memory.
 *      PGC_ERR_SYS     - System error (check errno).
 */
enum pgc_err pgc_buf_vprintf(
        struct pgc_buf *buffer,
        const char *format,
        va_list args);

/**
 * Read next byte. 
 * @param buffer The buffer.
 * @param result A pointer to result storage.
 * @return 
 * PGC_ERR_OOB: Attempt to read past end of buffer.
 */
enum pgc_err pgc_buf_getchar(
        struct pgc_buf *buffer, 
        char *result);

/**
 * Read next UTF8 character.
 */
enum pgc_err pgc_buf_getutf8(
        struct pgc_buf *buffer,
        uint32_t *result);

/** Encode Dictionary */
typedef enum pgc_err (*pgc_buf_encode_dict_t)(
        const uint8_t digit,
        uint8_t *symbol);

/** Encode Iterator */
typedef int (*pgc_buf_encode_iter_t)(
        const size_t base,
        void *value,
        uint8_t *digit);

/**
 * Encode a numeric value into the given base.
 * @param buffer The buffer.
 * @param base The numeric base to encode the value into.
 * @param value The unsigned value to encode.
 * @param next The encode iterator.
 * @param dict The encode dictionary.
 * @return 
 *      PGC_ERR_OOB: Attempt to write past end of buffer.
 *      PGC_ERR_OK: All OKAY.
 */
enum pgc_err pgc_buf_encode(
        struct pgc_buf *buffer,
        const size_t base,
        void *value,
        pgc_buf_encode_iter_t next,
        pgc_buf_encode_dict_t dict);

enum pgc_err pgc_buf_encode_dec(const uint8_t digit, uint8_t *symbol);
enum pgc_err pgc_buf_encode_hex(const uint8_t digit, uint8_t *symbol);
int pgc_buf_encode_uint8(const size_t base, void *value, uint8_t *digit);
int pgc_buf_encode_uint16(const size_t base, void *value, uint8_t *digit);
int pgc_buf_encode_uint32(const size_t base, void *value, uint8_t *digit);
int pgc_buf_encode_uint64(const size_t base, void *value, uint8_t *digit);

/**
 * Decode Dictionary
 */
typedef enum pgc_err (pgc_buf_decode_dict_t)(
        const uint8_t symbol, 
        uint8_t *value);

/** 
 * Decode Accumulator
 */
typedef enum pgc_err (*pgc_buf_decode_accum_t)(
        const size_t base, const uint8_t value, void *result);

/**
 * Generic function for decoding ASCII encoded numeric values.
 * @param buffer The buffer.
 * @param len The number of symbols to decode (one byte is one symbol).
 * @param base The numeric base.
 * @param dict A dictionary mapping symbols to numeric values.
 * @param accum The accumulator.
 * @param result The result's storage.
 * @return
 *      PGC_ERR_OOB: Attempt to read past end of buffer.
 *      PGC_ERR_ENC: Improperly encoded text (unexpected symbol).
 *      PGC_ERR_OK: All OKAY.
 */
enum pgc_err pgc_buf_decode(
        struct pgc_buf *buf,
        const size_t len,
        const size_t base,
        pgc_buf_decode_dict_t dict,
        pgc_buf_decode_accum_t accum,
        void *result);

enum pgc_err pgc_buf_decode_dec(const uint8_t symbol, uint8_t *value);
enum pgc_err pgc_buf_decode_hex(const uint8_t symbol, uint8_t *value);
enum pgc_err pgc_buf_decode_uint8(
        const size_t base, const uint8_t value, void *result);
enum pgc_err pgc_buf_decode_uint16(
        const size_t base, const uint8_t value, void *result);
enum pgc_err pgc_buf_decode_uint32(
        const size_t base, const uint8_t value, void *result);
enum pgc_err pgc_buf_decode_uint64(
        const size_t base, const uint8_t value, void *result);

#endif