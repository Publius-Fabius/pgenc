
#include "pgenc/buffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

uint8_t bytes[256];

void test_seek() {
        SEL_INFO();
        struct pgc_buf buf; pgc_buf_init(&buf, bytes, 16, 0);
        SEL_TEST(pgc_buf_seek(&buf, 1) == PGC_ERR_OOB);
        SEL_TEST(pgc_buf_seek(&buf, (size_t)-1) == PGC_ERR_OOB);
        SEL_TEST(pgc_buf_seek(&buf, 0) == PGC_ERR_OK);
}

void test_put()
{
        SEL_INFO();
        struct pgc_buf b; pgc_buf_init(&b, bytes, 8, 0);
        SEL_TEST(pgc_buf_put(&b, "abcd", 4) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_end(&b) == 4);
        SEL_TEST(pgc_buf_tell(&b) == 0);
        SEL_TEST(pgc_buf_put(&b, "xyz", 3) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_end(&b) == 7);
        SEL_TEST(pgc_buf_tell(&b) == 0);
        SEL_TEST(pgc_buf_put(&b, "ij", 2) == PGC_ERR_OOB);
        SEL_TEST(pgc_buf_end(&b) == 7);
        SEL_TEST(pgc_buf_tell(&b) == 0);
        pgc_buf_seek(&b, 4);
        SEL_TEST(pgc_buf_put(&b, "ij", 2) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_end(&b) == 9);
        SEL_TEST(pgc_buf_seek(&b, 1) == PGC_ERR_OOB);
}

void test_get()
{
        SEL_INFO();
        struct pgc_buf b; pgc_buf_init(&b, bytes, 8, 0);
        char str[128];
        SEL_TEST(pgc_buf_put(&b, "abc", 3) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_put(&b, "de", 2) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_get(&b, str, 2) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_tell(&b) == 2);
        SEL_TEST(memcmp(str, "ab", 2) == 0);
        SEL_TEST(pgc_buf_get(&b, str, 2) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_tell(&b) == 4);
        SEL_TEST(memcmp(str, "cd", 2) == 0);
        SEL_TEST(pgc_buf_put(&b, "fgh", 3) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_end(&b) == 8);
        SEL_TEST(pgc_buf_get(&b, str, 3) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_tell(&b) == 7);
        SEL_TEST(memcmp(str, "efg", 3) == 0);
        SEL_TEST(pgc_buf_get(&b, str, 2) == PGC_ERR_OOB);
        SEL_TEST(pgc_buf_get(&b, str, 1) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_tell(&b) == 8);
        SEL_TEST(pgc_buf_end(&b) == 8);
        SEL_TEST(memcmp(str, "h", 1) == 0);
        SEL_TEST(pgc_buf_put(&b, "123456", 6) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_get(&b, str, 6) == PGC_ERR_OK);
        SEL_TEST(memcmp(str, "123456", 6) == 0);
}

void test_cmp()
{
        SEL_INFO();
        struct pgc_buf b; pgc_buf_init(&b, bytes, 8, 0);
        SEL_TEST(pgc_buf_put(&b, "123456", 6) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_cmp(&b, "123", 3) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_tell(&b) == 3);
        SEL_TEST(pgc_buf_cmp(&b, "456", 3) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_tell(&b) == 6);
        SEL_TEST(pgc_buf_put(&b, "abcdef", 6) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_end(&b) == 12);
        SEL_TEST(pgc_buf_cmp(&b, "abcdef", 6) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_tell(&b) == 12);
}

int char_pred(const void *st, const uint8_t c) {
        return *((const uint8_t*)st) == c;
}

void test_match()
{
        SEL_INFO();
        uint8_t c = 'c';
        struct pgc_buf b; pgc_buf_init(&b, bytes, 8, 0);
        SEL_TEST(pgc_buf_put(&b, "cat", 3) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_match(&b, char_pred, &c) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_tell(&b) == 1);
        c = 'a';
        SEL_TEST(pgc_buf_match(&b, char_pred, &c) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_tell(&b) == 2);
        c = 'b';
        SEL_TEST(pgc_buf_match(&b, char_pred, &c) == PGC_ERR_CMP);
        SEL_TEST(pgc_buf_tell(&b) == 2);
        c = 't';
        SEL_TEST(pgc_buf_match(&b, char_pred, &c) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_tell(&b) == 3);
        SEL_TEST(pgc_buf_match(&b, char_pred, &c) == PGC_ERR_OOB);
        SEL_TEST(pgc_buf_tell(&b) == 3);
}

int utf8_pred(const void *st, const uint32_t c) {
        return *((const uint32_t*)st) == c;
}

void test_match_utf8()
{
        SEL_INFO();
        uint32_t c = 'a';
        struct pgc_buf b; pgc_buf_init(&b, bytes, 8, 0);

        // Test an ASCII char
        SEL_TEST(pgc_buf_put(&b, "a", 1) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_matchutf8(&b, utf8_pred, &c) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_tell(&b) == 1);

        // Test a character that should take 2 bytes.
        c = 0x00A3;
        SEL_TEST(pgc_buf_put(&b, "£", 2) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_matchutf8(&b, utf8_pred, &c) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_tell(&b) == 3);

        // Test a char that should take 3 bytes.
        c = 0x276E;
        SEL_TEST(pgc_buf_put(&b, "❮", 3) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_matchutf8(&b, utf8_pred, &c) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_tell(&b) == 6);

        // Test a char that should take 4 bytes.
        c = 0x1D400;
        SEL_TEST(pgc_buf_put(&b, "𝐀", 4) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_matchutf8(&b, utf8_pred, &c) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_tell(&b) == 10);

        // Let's test for OOB.
        SEL_TEST(pgc_buf_matchutf8(&b, utf8_pred, &c) == PGC_ERR_OOB);
        
}

void test_scan()
{
        SEL_INFO();

        struct pgc_buf buf;
        pgc_buf_init(&buf, "abcdefg", 7, 7);

        enum pgc_err err = pgc_buf_scan(&buf, "de", 2);
        SEL_TEST(err == PGC_ERR_OK);
        SEL_TEST(pgc_buf_tell(&buf) == 5);

        struct pgc_buf lens;
        SEL_TEST(pgc_buf_seek(&buf, 0) == PGC_ERR_OK);
        pgc_buf_lens(&lens, &buf, 4);

        err = pgc_buf_scan(&lens, "de", 2);
        SEL_TEST(err == PGC_ERR_OOB);
        SEL_TEST(pgc_buf_tell(&lens) == 3);

        pgc_buf_lens(&lens, &buf, 5);
        err = pgc_buf_scan(&lens, "cde", 3);
        SEL_TEST(err == PGC_ERR_OK);
        SEL_TEST(pgc_buf_tell(&lens) == 5);
}

void test_read()
{
        SEL_INFO();

        int pfds[2];
        pipe(pfds);
        fcntl(pfds[0], F_SETFL, O_NONBLOCK); 
        fcntl(pfds[1], F_SETFL, O_NONBLOCK); 
        struct pgc_buf b; pgc_buf_init(&b, bytes, 8, 0);

        write(pfds[1], "abcd", 4);
        SEL_TEST(pgc_buf_read(&b, pfds[0], 4) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_tell(&b) == 0);
        SEL_TEST(pgc_buf_end(&b) == 4);
        SEL_TEST(pgc_buf_cmp(&b, "abcd", 4) == PGC_ERR_OK);
        write(pfds[1], "12345", 5);
        SEL_TEST(pgc_buf_read(&b, pfds[0], 9) == PGC_ERR_OOB);
        SEL_TEST(pgc_buf_read(&b, pfds[0], 5) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_cmp(&b, "12345", 5) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_read(&b, pfds[0], 1) == PGC_ERR_SYS);
        SEL_TEST(errno == EAGAIN);

        close(pfds[0]);
        close(pfds[1]);
}

void test_write()
{
        SEL_INFO();

        int pfds[2];
        pipe(pfds);
        fcntl(pfds[0], F_SETFL, O_NONBLOCK); 
        fcntl(pfds[1], F_SETFL, O_NONBLOCK); 
        struct pgc_buf b; pgc_buf_init(&b, bytes, 8, 0);
        char str[64];

        SEL_TEST(pgc_buf_put(&b, "xyz", 3) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_tell(&b) == 0);
        SEL_TEST(pgc_buf_end(&b) == 3);
        SEL_TEST(pgc_buf_write(&b, pfds[1], 3) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_tell(&b) == 3);
        read(pfds[0], str, 3);
        SEL_TEST(memcmp(str, "xyz", 3) == 0);
        SEL_TEST(pgc_buf_write(&b, pfds[1], 1) == PGC_ERR_OOB);

        close(pfds[0]);
        close(pfds[1]);
}

void test_printf()
{
        SEL_INFO();
        struct pgc_buf b; pgc_buf_init(&b, bytes, 8, 0);

        SEL_TEST(pgc_buf_printf(&b, "%i", -12345) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_cmp(&b, "-12345", 6) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_printf(&b, "%x", 0xABCD) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_printf(&b, "%i", -123456) == PGC_ERR_OOB);
        SEL_TEST(pgc_buf_cmp(&b, "abcd", 4) == PGC_ERR_OK);
}

void test_getchar()
{
        SEL_INFO();
        struct pgc_buf b; pgc_buf_init(&b, bytes, 8, 0);
        char c;
        SEL_TEST(pgc_buf_put(&b, "hi", 2) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_getchar(&b, &c) == PGC_ERR_OK);
        SEL_TEST(c == 'h');
        SEL_TEST(pgc_buf_getchar(&b, &c) == PGC_ERR_OK);
        SEL_TEST(c == 'i');
        SEL_TEST(pgc_buf_getchar(&b, &c) == PGC_ERR_OOB);
}

void test_getutf8()
{
        SEL_INFO();
        struct pgc_buf b; pgc_buf_init(&b, bytes, 64, 0);
        uint32_t c;

        // Test an ASCII char
        SEL_TEST(pgc_buf_put(&b, "a", 1) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_getutf8(&b, &c) == PGC_ERR_OK);
        SEL_TEST(c == 'a');
        SEL_TEST(pgc_buf_tell(&b) == 1);

        // Test a character that should take 2 bytes.
        // c == 0x00A3;
        SEL_TEST(pgc_buf_put(&b, "£", 2) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_getutf8(&b,  &c) == PGC_ERR_OK);
        SEL_TEST(c == 0x00A3);
        SEL_TEST(pgc_buf_tell(&b) == 3);

        // Test a char that should take 3 bytes.
        // c == 0x276E;
        SEL_TEST(pgc_buf_put(&b, "❮", 3) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_getutf8(&b, &c) == PGC_ERR_OK);
        SEL_TEST(c == 0x276E);
        SEL_TEST(pgc_buf_tell(&b) == 6);

        // Test a char that should take 4 bytes.
        // c = 0x1D400;
        SEL_TEST(pgc_buf_put(&b, "𝐀", 4) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_getutf8(&b, &c) == PGC_ERR_OK);
        SEL_TEST(c == 0x1D400);
        SEL_TEST(pgc_buf_tell(&b) == 10);

        // Let's test for OOB.
        SEL_TEST(pgc_buf_getutf8(&b, &c) == PGC_ERR_OOB);
}

void test_encode_dec()
{
        SEL_INFO();
        uint8_t sym;
        SEL_TEST(pgc_buf_encode_dec(0, &sym) == PGC_ERR_OK);
        SEL_TEST(sym == '0');
        SEL_TEST(pgc_buf_encode_dec(1, &sym) == PGC_ERR_OK);
        SEL_TEST(sym == '1');
        SEL_TEST(pgc_buf_encode_dec(9, &sym) == PGC_ERR_OK);
        SEL_TEST(sym == '9');
        SEL_TEST(pgc_buf_encode_dec(11, &sym) == PGC_ERR_ENC);
}

void test_encode_hex()
{
        SEL_INFO();
        uint8_t sym;
        SEL_TEST(pgc_buf_encode_hex(0, &sym) == PGC_ERR_OK);
        SEL_TEST(sym == '0');
        SEL_TEST(pgc_buf_encode_hex(1, &sym) == PGC_ERR_OK);
        SEL_TEST(sym == '1');
        SEL_TEST(pgc_buf_encode_hex(9, &sym) == PGC_ERR_OK);
        SEL_TEST(sym == '9');
        SEL_TEST(pgc_buf_encode_hex(10, &sym) == PGC_ERR_OK);
        SEL_TEST(sym == 'a');
        SEL_TEST(pgc_buf_encode_hex(15, &sym) == PGC_ERR_OK);
        SEL_TEST(sym == 'f');
        SEL_TEST(pgc_buf_encode_hex(16, &sym) == PGC_ERR_ENC);
}

void test_encode_uint8()
{
        SEL_INFO();

        uint8_t val, dig;

        val = 123;
        SEL_TEST(pgc_buf_encode_uint8(10, &val, &dig) != 0);
        SEL_TEST(dig == 3);
        SEL_TEST(pgc_buf_encode_uint8(10, &val, &dig) != 0);
        SEL_TEST(dig == 2);
        SEL_TEST(pgc_buf_encode_uint8(10, &val, &dig) == 0);
        SEL_TEST(dig == 1);
}

void test_encode()
{
        SEL_INFO();
        struct pgc_buf b; pgc_buf_init(&b, bytes, 64, 0);

        uint8_t val8;

        val8 = 123;
        SEL_TEST(pgc_buf_encode(
                &b, 
                10, 
                &val8, 
                pgc_buf_encode_uint8, 
                pgc_buf_encode_dec) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_end(&b) == 3);
        SEL_TEST(bytes[0] == '1');
        SEL_TEST(bytes[1] == '2');
        SEL_TEST(bytes[2] == '3');

        val8 = 0xF1;
        SEL_TEST(pgc_buf_encode(
                &b, 
                16, 
                &val8, 
                pgc_buf_encode_uint8, 
                pgc_buf_encode_hex) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_end(&b) == 5);
        SEL_TEST(bytes[3] == 'f');
        SEL_TEST(bytes[4] == '1');
}

void test_decode_dec()
{
        SEL_INFO();
        uint8_t val = 0xAA;
        SEL_TEST(pgc_buf_decode_dec('0', &val) == PGC_ERR_OK);
        SEL_TEST(val == 0);
        SEL_TEST(pgc_buf_decode_dec('9', &val) == PGC_ERR_OK);
        SEL_TEST(val == 9);
        SEL_TEST(pgc_buf_decode_dec('a', &val) == PGC_ERR_ENC);
}

void test_decode_hex()
{
        SEL_INFO();
        uint8_t val = 0xAA;
        SEL_TEST(pgc_buf_decode_hex('0', &val) == PGC_ERR_OK);
        SEL_TEST(val == 0);
        SEL_TEST(pgc_buf_decode_hex('9', &val) == PGC_ERR_OK);
        SEL_TEST(val == 9);
        SEL_TEST(pgc_buf_decode_hex('a', &val) == PGC_ERR_OK);
        SEL_TEST(val == 10);
        SEL_TEST(pgc_buf_decode_hex('f', &val) == PGC_ERR_OK);
        SEL_TEST(val == 15);
        SEL_TEST(pgc_buf_decode_hex('g', &val) == PGC_ERR_ENC);     
}

void test_decode_uint8()
{
        SEL_INFO();
        uint8_t accum8 = 0;
        SEL_TEST(pgc_buf_decode_uint8(10, 1, &accum8) == PGC_ERR_OK);
        SEL_TEST(accum8 == 1);
        SEL_TEST(pgc_buf_decode_uint8(10, 2, &accum8) == PGC_ERR_OK);
        SEL_TEST(accum8 == 12);
        SEL_TEST(pgc_buf_decode_uint8(10, 3, &accum8) == PGC_ERR_OK);
        SEL_TEST(accum8 == 123);
}

void test_decode()
{
        SEL_INFO();
        struct pgc_buf b; pgc_buf_init(&b, bytes, 64, 0);
        uint8_t val8 = 0;
        SEL_TEST(pgc_buf_put(&b, "123", 3) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_decode(
                &b, 
                3, 
                10, 
                pgc_buf_decode_dec, 
                pgc_buf_decode_uint8, 
                &val8) == PGC_ERR_OK);
        SEL_TEST(val8 == 123);
        SEL_TEST(pgc_buf_put(&b, "a1", 2) == PGC_ERR_OK);
        SEL_TEST(pgc_buf_decode(
                &b, 
                2, 
                16, 
                pgc_buf_decode_hex, 
                pgc_buf_decode_uint8, 
                &val8) == PGC_ERR_OK);
        SEL_TEST(val8 == 0xA1);
}

int main(int argc, char **args) 
{
        SEL_INFO();

        test_seek();
        test_put();
        test_get();
        test_cmp();
        test_match();
        test_match_utf8();
        test_scan();
        test_read();
        test_write();
        test_printf();
        test_getchar();
        test_getutf8();
        test_encode_dec();
        test_encode_hex();
        test_encode_uint8();
        test_encode();
        test_decode_dec();
        test_decode_hex();
        test_decode_uint8();
        test_decode();
}