
#include "pgenc/error.h"

#include <stdlib.h>
#include <stdio.h>

const char *pgc_strerror(const enum pgc_err error)
{
        static const char *PGC_ERR_OK_STR      = "PGC_ERR_OK";
        static const char *PGC_ERR_SYS_STR     = "PGC_ERR_SYS";
        static const char *PGC_ERR_OOB_STR     = "PGC_ERR_OOB";
        static const char *PGC_ERR_CMP_STR     = "PGC_ERR_CMP";
        static const char *PGC_ERR_ENC_STR     = "PGC_ERR_ENC";
        static const char *PGC_ERR_EOF_STR     = "PGC_ERR_EOF";
        static const char *PGC_ERR_SYN_STR     = "PGC_ERR_SYN";
        static const char *PGC_ERR_FLO_STR     = "PGC_ERR_FLO";
        static const char *PGC_ERR_FLO_UNK     = "undefined";

        switch(error) {
                case PGC_ERR_OK: return PGC_ERR_OK_STR;
                case PGC_ERR_SYS: return PGC_ERR_SYS_STR;
                case PGC_ERR_OOB: return PGC_ERR_OOB_STR;
                case PGC_ERR_CMP: return PGC_ERR_CMP_STR;
                case PGC_ERR_ENC: return PGC_ERR_ENC_STR;
                case PGC_ERR_EOF: return PGC_ERR_EOF_STR;
                case PGC_ERR_SYN: return PGC_ERR_SYN_STR;
                case PGC_ERR_FLO: return PGC_ERR_FLO_STR;
                default: return PGC_ERR_FLO_UNK;
        }
}

enum pgc_err pgc_abort(
        const char *file,
        const char *func,
        const int line)
{
        fprintf(stderr, 
                "\n%s: %s(..) line %i: pgc_abort(..) \n",
                file,
                func,
                line);
        abort();
}

enum pgc_err pgc_report(
        const enum pgc_err error,
        const char *file,
        const char *func,
        const int line)
{
        fprintf(stderr, 
                "\n%s: %s(..) line %i: pgc_report(..): error %i %s \n", 
                file,
                func,
                line,
                error,
                pgc_strerror(error));
        return error;
}

void pgc_trace(
        const char *file,
        const char *func,
        const int line)
{
        fprintf(stderr, 
                "\n%s: %s(..) line %i: pgc_trace(..) \n", 
                file, 
                func,
                line);
}

void pgc_info(
        const char *file,
        const char *func,
        const int line)
{
        printf( "%s: %s(..) line %i \n", 
                file, 
                func,
                line);
}