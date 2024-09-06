
#include "pgenc/buffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

uint8_t bytes[256];

void test_seek() {
        puts("   ...test_seek()");
        struct pgc_buf buf; pgc_buf_init(&buf, bytes, 16, 0);
        assert(pgc_buf_seek(&buf, 1) == PGC_ERR_OOB);
        assert(pgc_buf_seek(&buf, (size_t)-1) == PGC_ERR_OOB);
        assert(pgc_buf_seek(&buf, 0) == PGC_ERR_OK);
}

void test_put()
{
        puts("   ...test_put()");
        struct pgc_buf b; pgc_buf_init(&b, bytes, 8, 0);
        assert(pgc_buf_put(&b, "abcd", 4) == PGC_ERR_OK);
        assert(pgc_buf_end(&b) == 4);
        assert(pgc_buf_tell(&b) == 0);
        assert(pgc_buf_put(&b, "xyz", 3) == PGC_ERR_OK);
        assert(pgc_buf_end(&b) == 7);
        assert(pgc_buf_tell(&b) == 0);
        assert(pgc_buf_put(&b, "ij", 2) == PGC_ERR_OOB);
        assert(pgc_buf_end(&b) == 7);
        assert(pgc_buf_tell(&b) == 0);
        pgc_buf_seek(&b, 4);
        assert(pgc_buf_put(&b, "ij", 2) == PGC_ERR_OK);
        assert(pgc_buf_end(&b) == 9);
        assert(pgc_buf_seek(&b, 1) == PGC_ERR_OOB);
}

void test_get()
{
        puts("   ...test_get()");
        struct pgc_buf b; pgc_buf_init(&b, bytes, 8, 0);
        char str[128];
        assert(pgc_buf_put(&b, "abc", 3) == PGC_ERR_OK);
        assert(pgc_buf_put(&b, "de", 2) == PGC_ERR_OK);
        assert(pgc_buf_get(&b, str, 2) == PGC_ERR_OK);
        assert(pgc_buf_tell(&b) == 2);
        assert(memcmp(str, "ab", 2) == 0);
        assert(pgc_buf_get(&b, str, 2) == PGC_ERR_OK);
        assert(pgc_buf_tell(&b) == 4);
        assert(memcmp(str, "cd", 2) == 0);
        assert(pgc_buf_put(&b, "fgh", 3) == PGC_ERR_OK);
        assert(pgc_buf_end(&b) == 8);
        assert(pgc_buf_get(&b, str, 3) == PGC_ERR_OK);
        assert(pgc_buf_tell(&b) == 7);
        assert(memcmp(str, "efg", 3) == 0);
        assert(pgc_buf_get(&b, str, 2) == PGC_ERR_OOB);
        assert(pgc_buf_get(&b, str, 1) == PGC_ERR_OK);
        assert(pgc_buf_tell(&b) == 8);
        assert(pgc_buf_end(&b) == 8);
        assert(memcmp(str, "h", 1) == 0);
        assert(pgc_buf_put(&b, "123456", 6) == PGC_ERR_OK);
        assert(pgc_buf_get(&b, str, 6) == PGC_ERR_OK);
        assert(memcmp(str, "123456", 6) == 0);
}

void test_cmp()
{
        puts("   ...test_cmp()");
        struct pgc_buf b; pgc_buf_init(&b, bytes, 8, 0);
        assert(pgc_buf_put(&b, "123456", 6) == PGC_ERR_OK);
        assert(pgc_buf_cmp(&b, "123", 3) == PGC_ERR_OK);
        assert(pgc_buf_tell(&b) == 3);
        assert(pgc_buf_cmp(&b, "456", 3) == PGC_ERR_OK);
        assert(pgc_buf_tell(&b) == 6);
        assert(pgc_buf_put(&b, "abcdef", 6) == PGC_ERR_OK);
        assert(pgc_buf_end(&b) == 12);
        assert(pgc_buf_cmp(&b, "abcdef", 6) == PGC_ERR_OK);
        assert(pgc_buf_tell(&b) == 12);
}

int char_pred(void *st, const int c) {
        return *((char*)st) == c;
}

void test_match()
{
        puts("   ...test_match()");
        char c = 'c';
        struct pgc_buf b; pgc_buf_init(&b, bytes, 8, 0);
        assert(pgc_buf_put(&b, "cat", 3) == PGC_ERR_OK);
        assert(pgc_buf_match(&b, char_pred, &c) == PGC_ERR_OK);
        assert(pgc_buf_tell(&b) == 1);
        c = 'a';
        assert(pgc_buf_match(&b, char_pred, &c) == PGC_ERR_OK);
        assert(pgc_buf_tell(&b) == 2);
        c = 'b';
        assert(pgc_buf_match(&b, char_pred, &c) == PGC_ERR_CMP);
        assert(pgc_buf_tell(&b) == 2);
        c = 't';
        assert(pgc_buf_match(&b, char_pred, &c) == PGC_ERR_OK);
        assert(pgc_buf_tell(&b) == 3);
        assert(pgc_buf_match(&b, char_pred, &c) == PGC_ERR_OOB);
        assert(pgc_buf_tell(&b) == 3);
}

int utf8_pred(void *st, const uint32_t c) {
        return *((uint32_t*)st) == c;
}

void test_match_utf8()
{
        puts("   ...test_match_utf8()");
        uint32_t c = 'a';
        struct pgc_buf b; pgc_buf_init(&b, bytes, 8, 0);

        // Test an ASCII char
        assert(pgc_buf_put(&b, "a", 1) == PGC_ERR_OK);
        assert(pgc_buf_matchutf8(&b, utf8_pred, &c) == PGC_ERR_OK);
        assert(pgc_buf_tell(&b) == 1);

        // Test a character that should take 2 bytes.
        c = 0x00A3;
        assert(pgc_buf_put(&b, "£", 2) == PGC_ERR_OK);
        assert(pgc_buf_matchutf8(&b, utf8_pred, &c) == PGC_ERR_OK);
        assert(pgc_buf_tell(&b) == 3);

        // Test a char that should take 3 bytes.
        c = 0x276E;
        assert(pgc_buf_put(&b, "❮", 3) == PGC_ERR_OK);
        assert(pgc_buf_matchutf8(&b, utf8_pred, &c) == PGC_ERR_OK);
        assert(pgc_buf_tell(&b) == 6);

        // Test a char that should take 4 bytes.
        c = 0x1D400;
        assert(pgc_buf_put(&b, "𝐀", 4) == PGC_ERR_OK);
        assert(pgc_buf_matchutf8(&b, utf8_pred, &c) == PGC_ERR_OK);
        assert(pgc_buf_tell(&b) == 10);

        // Let's test for OOB.
        assert(pgc_buf_matchutf8(&b, utf8_pred, &c) == PGC_ERR_OOB);
        
}

void test_read()
{
        puts("   ...test_read()");

        int pfds[2];
        pipe(pfds);
        fcntl(pfds[0], F_SETFL, O_NONBLOCK); 
        fcntl(pfds[1], F_SETFL, O_NONBLOCK); 
        struct pgc_buf b; pgc_buf_init(&b, bytes, 8, 0);

        write(pfds[1], "abcd", 4);
        assert(pgc_buf_read(&b, pfds[0], 4) == PGC_ERR_OK);
        assert(pgc_buf_tell(&b) == 0);
        assert(pgc_buf_end(&b) == 4);
        assert(pgc_buf_cmp(&b, "abcd", 4) == PGC_ERR_OK);
        write(pfds[1], "12345", 5);
        assert(pgc_buf_read(&b, pfds[0], 9) == PGC_ERR_OOB);
        assert(pgc_buf_read(&b, pfds[0], 5) == PGC_ERR_OK);
        assert(pgc_buf_cmp(&b, "12345", 5) == PGC_ERR_OK);
        assert(pgc_buf_read(&b, pfds[0], 1) == PGC_ERR_SYS);
        assert(errno == EAGAIN);

        close(pfds[0]);
        close(pfds[1]);
}

void test_write()
{
        puts("   ...test_write()");

        int pfds[2];
        pipe(pfds);
        fcntl(pfds[0], F_SETFL, O_NONBLOCK); 
        fcntl(pfds[1], F_SETFL, O_NONBLOCK); 
        struct pgc_buf b; pgc_buf_init(&b, bytes, 8, 0);
        char str[64];

        assert(pgc_buf_put(&b, "xyz", 3) == PGC_ERR_OK);
        assert(pgc_buf_tell(&b) == 0);
        assert(pgc_buf_end(&b) == 3);
        assert(pgc_buf_write(&b, pfds[1], 3) == PGC_ERR_OK);
        assert(pgc_buf_tell(&b) == 3);
        read(pfds[0], str, 3);
        assert(memcmp(str, "xyz", 3) == 0);
        assert(pgc_buf_write(&b, pfds[1], 1) == PGC_ERR_OOB);

        close(pfds[0]);
        close(pfds[1]);
}

void test_printf()
{
        puts("   ...test_printf()");
        struct pgc_buf b; pgc_buf_init(&b, bytes, 8, 0);

        assert(pgc_buf_printf(&b, "%i", -12345) == PGC_ERR_OK);
        assert(pgc_buf_cmp(&b, "-12345", 6) == PGC_ERR_OK);
        assert(pgc_buf_printf(&b, "%x", 0xABCD) == PGC_ERR_OK);
        assert(pgc_buf_printf(&b, "%i", -123456) == PGC_ERR_OOB);
        assert(pgc_buf_cmp(&b, "abcd", 4) == PGC_ERR_OK);
}

void test_getchar()
{
        puts("   ...test_getchar()");
        struct pgc_buf b; pgc_buf_init(&b, bytes, 8, 0);
        char c;
        assert(pgc_buf_put(&b, "hi", 2) == PGC_ERR_OK);
        assert(pgc_buf_getchar(&b, &c) == PGC_ERR_OK);
        assert(c == 'h');
        assert(pgc_buf_getchar(&b, &c) == PGC_ERR_OK);
        assert(c == 'i');
        assert(pgc_buf_getchar(&b, &c) == PGC_ERR_OOB);
}

void test_getutf8()
{
        puts("   ...test_getutf8()");
        struct pgc_buf b; pgc_buf_init(&b, bytes, 64, 0);
        uint32_t c;

        // Test an ASCII char
        assert(pgc_buf_put(&b, "a", 1) == PGC_ERR_OK);
        assert(pgc_buf_getutf8(&b, &c) == PGC_ERR_OK);
        assert(c == 'a');
        assert(pgc_buf_tell(&b) == 1);

        // Test a character that should take 2 bytes.
        // c == 0x00A3;
        assert(pgc_buf_put(&b, "£", 2) == PGC_ERR_OK);
        assert(pgc_buf_getutf8(&b,  &c) == PGC_ERR_OK);
        assert(c == 0x00A3);
        assert(pgc_buf_tell(&b) == 3);

        // Test a char that should take 3 bytes.
        // c == 0x276E;
        assert(pgc_buf_put(&b, "❮", 3) == PGC_ERR_OK);
        assert(pgc_buf_getutf8(&b, &c) == PGC_ERR_OK);
        assert(c == 0x276E);
        assert(pgc_buf_tell(&b) == 6);

        // Test a char that should take 4 bytes.
        // c = 0x1D400;
        assert(pgc_buf_put(&b, "𝐀", 4) == PGC_ERR_OK);
        assert(pgc_buf_getutf8(&b, &c) == PGC_ERR_OK);
        assert(c == 0x1D400);
        assert(pgc_buf_tell(&b) == 10);

        // Let's test for OOB.
        assert(pgc_buf_getutf8(&b, &c) == PGC_ERR_OOB);
}

void test_encode_dec()
{
        puts("   ...test_encode_dec()");
        uint8_t sym;
        assert(pgc_buf_encode_dec(0, &sym) == PGC_ERR_OK);
        assert(sym == '0');
        assert(pgc_buf_encode_dec(1, &sym) == PGC_ERR_OK);
        assert(sym == '1');
        assert(pgc_buf_encode_dec(9, &sym) == PGC_ERR_OK);
        assert(sym == '9');
        assert(pgc_buf_encode_dec(11, &sym) == PGC_ERR_ENC);
}

void test_encode_hex()
{
        puts("   ...test_encode_hex()");
        uint8_t sym;
        assert(pgc_buf_encode_hex(0, &sym) == PGC_ERR_OK);
        assert(sym == '0');
        assert(pgc_buf_encode_hex(1, &sym) == PGC_ERR_OK);
        assert(sym == '1');
        assert(pgc_buf_encode_hex(9, &sym) == PGC_ERR_OK);
        assert(sym == '9');
        assert(pgc_buf_encode_hex(10, &sym) == PGC_ERR_OK);
        assert(sym == 'a');
        assert(pgc_buf_encode_hex(15, &sym) == PGC_ERR_OK);
        assert(sym == 'f');
        assert(pgc_buf_encode_hex(16, &sym) == PGC_ERR_ENC);
}

void test_encode_uint8()
{
        puts("   ...test_encode_uint8()");

        uint8_t val, dig;

        val = 123;
        assert(pgc_buf_encode_uint8(10, &val, &dig) != 0);
        assert(dig == 3);
        assert(pgc_buf_encode_uint8(10, &val, &dig) != 0);
        assert(dig == 2);
        assert(pgc_buf_encode_uint8(10, &val, &dig) == 0);
        assert(dig == 1);
}

void test_encode()
{
        puts("   ...test_encode()");
        struct pgc_buf b; pgc_buf_init(&b, bytes, 64, 0);

        uint8_t val8;

        val8 = 123;
        assert(pgc_buf_encode(
                &b, 
                10, 
                &val8, 
                pgc_buf_encode_uint8, 
                pgc_buf_encode_dec) == PGC_ERR_OK);
        assert(pgc_buf_end(&b) == 3);
        assert(bytes[0] == '1');
        assert(bytes[1] == '2');
        assert(bytes[2] == '3');

        val8 = 0xF1;
        assert(pgc_buf_encode(
                &b, 
                16, 
                &val8, 
                pgc_buf_encode_uint8, 
                pgc_buf_encode_hex) == PGC_ERR_OK);
        assert(pgc_buf_end(&b) == 5);
        assert(bytes[3] == 'f');
        assert(bytes[4] == '1');
}

void test_decode_dec()
{
        puts("   ...test_decode_dec()");
        uint8_t val = 0xAA;
        assert(pgc_buf_decode_dec('0', &val) == PGC_ERR_OK);
        assert(val == 0);
        assert(pgc_buf_decode_dec('9', &val) == PGC_ERR_OK);
        assert(val == 9);
        assert(pgc_buf_decode_dec('a', &val) == PGC_ERR_ENC);
}

void test_decode_hex()
{
        puts("   ...test_decode_hex()");
        uint8_t val = 0xAA;
        assert(pgc_buf_decode_hex('0', &val) == PGC_ERR_OK);
        assert(val == 0);
        assert(pgc_buf_decode_hex('9', &val) == PGC_ERR_OK);
        assert(val == 9);
        assert(pgc_buf_decode_hex('a', &val) == PGC_ERR_OK);
        assert(val == 10);
        assert(pgc_buf_decode_hex('f', &val) == PGC_ERR_OK);
        assert(val == 15);
        assert(pgc_buf_decode_hex('g', &val) == PGC_ERR_ENC);     
}

void test_decode_uint8()
{
        puts("   ...test_decode_uint8()");
        uint8_t accum8 = 0;
        assert(pgc_buf_decode_uint8(10, 1, &accum8) == PGC_ERR_OK);
        assert(accum8 == 1);
        assert(pgc_buf_decode_uint8(10, 2, &accum8) == PGC_ERR_OK);
        assert(accum8 == 12);
        assert(pgc_buf_decode_uint8(10, 3, &accum8) == PGC_ERR_OK);
        assert(accum8 == 123);
}

void test_decode()
{
        puts("   ...test_decode()");
        struct pgc_buf b; pgc_buf_init(&b, bytes, 64, 0);
        uint8_t val8 = 0;
        assert(pgc_buf_put(&b, "123", 3) == PGC_ERR_OK);
        assert(pgc_buf_decode(
                &b, 
                3, 
                10, 
                pgc_buf_decode_dec, 
                pgc_buf_decode_uint8, 
                &val8) == PGC_ERR_OK);
        assert(val8 == 123);
        assert(pgc_buf_put(&b, "a1", 2) == PGC_ERR_OK);
        assert(pgc_buf_decode(
                &b, 
                2, 
                16, 
                pgc_buf_decode_hex, 
                pgc_buf_decode_uint8, 
                &val8) == PGC_ERR_OK);
        assert(val8 == 0xA1);
}

int main(int argc, char **args) 
{
        puts("running test_buffer...");

        test_seek();
        test_put();
        test_get();
        test_cmp();
        test_match();
        test_match_utf8();
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