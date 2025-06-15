
#define _POSIX_C_SOURCE 200809L

#include "pgenc/lang.h"
#include "pgenc/self.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static sel_err_t pgc_parse_syntax(
        struct pgc_stk *alloc,
        const char *fname, 
        struct pgc_ast_lst **syntax)
{
        const struct pgc_par *src_parser = &pgc_self_src;

        FILE *file = fopen(fname, "r");
        if(!file) {
                fputs("unable to open grammar file\n", stderr);
                fprintf(stderr, "errno=%s\n", strerror(errno));
                return SEL_REPORT(PGC_ERR_SYS);
        }

        SEL_IO(fseek(file, 0, SEEK_END));
        const size_t flen = (size_t)ftell(file);
        SEL_IO(fseek(file, 0, SEEK_SET));

        void *addr = malloc(flen);
        if(!addr) {
                fclose(file);
                fputs("file failed to read (out of memory)", stderr);
                return SEL_REPORT(PGC_ERR_SYS);
        }
        
        struct pgc_buf buf; pgc_buf_init(&buf, addr, flen, 0);
        sel_err_t e = pgc_buf_fread(&buf, file, flen);
        fclose(file);
        if(e != PGC_ERR_OK) {
                fputs("file failed to read\n", stderr);
                return SEL_REPORT(PGC_ERR_SYS);
        }
        
        e = pgc_lang_parse(src_parser, &buf, alloc, syntax);
        free(addr);

        if(e != PGC_ERR_OK) {
                fputs("grammar failed to parse\n", stderr);
                return SEL_REPORT(PGC_ERR_SYS);
        }

        const size_t offset = pgc_buf_tell(&buf);
        const size_t end = pgc_buf_end(&buf);

        SEL_ASSERT(offset <= end);

        if(offset != end) {
                static const size_t BUFLEN = 127;
                char msgbuf[BUFLEN + 1];
                fputs("parsing was incomplete starting at:\n", stderr);
                size_t msglen = end - offset;
                msglen = msglen <= BUFLEN ? msglen : BUFLEN;
                msgbuf[msglen] = 0;
                SEL_TRY(pgc_buf_get(&buf, msgbuf, msglen));
                fprintf(stderr, "%s\n", msgbuf);
                return SEL_REPORT(PGC_ERR_SYN);
        }

        return PGC_ERR_OK;
}

int main(int argc, char **args)
{
        pgc_err_init();

        const char *src = "autogen.c";
        const char *dict = "autogen";
        const char *gram = NULL;

        size_t blen = 0xFFFFF;

        if(argc < 2) {
                puts("USAGE: <OUTPUT_FILE> <NAMESPACE> <INPUT_FILE>");
                return EXIT_SUCCESS;
        } else if(argc > 2) {
                src = args[1];
                dict = args[2];
                gram = args[3];
        }

        FILE *src_file = fopen(src, "w");

        if(!src_file) {
                fputs("unable to open source file for writing\n", stderr);
                fprintf(stderr, "errno=%s\n", strerror(errno));
                return EXIT_FAILURE;
        }

        sel_err_t e = 0;

        if(gram) {
                struct pgc_stk *alloc = malloc(sizeof(struct pgc_stk));
                struct pgc_ast_lst *syntax = NULL;
                pgc_stk_init(alloc, malloc(blen), blen);
                e = pgc_parse_syntax(alloc, gram, &syntax);
                if(e != PGC_ERR_OK) {
                        free(alloc->base);
                        free(alloc);
                        fprintf(stderr, "error parsing syntax\n");
                        fclose(src_file);
                        return EXIT_FAILURE;
                } else {
                        e = pgc_lang_gen(src_file, syntax, dict);
                        free(alloc->base);
                        free(alloc);
                }
        } else {
                struct pgc_ast *prot = pgc_lang_proto();
                e = pgc_lang_gen(src_file, pgc_ast_tolst(prot), dict);
                pgc_syn_free(prot);
        }
        
        fclose(src_file);
        
        if(e != PGC_ERR_OK) {
                fputs("error generating parsers\n", stderr);
                fprintf(stderr, "pgc_err=%s\n", sel_strerror(e));
                if(e == PGC_ERR_SYS) {
                        fprintf(stderr, "errno=%s\n", strerror(errno));
                }
                exit(EXIT_FAILURE);
        }
        
        return EXIT_SUCCESS;
}