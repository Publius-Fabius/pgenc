#ifndef PGENC_ERROR_H
#define PGENC_ERROR_H

#include "selc/error.h"

/** Type for returning errors */
enum pgc_err 
{
        PGC_ERR_OK      = SEL_ERR_OK,           /** All OK */
        PGC_ERR_SYS     = SEL_ERR_SYS,          /** Syscall Error */

        PGC_ERR_OOB     = -1600,                /** Out of Bounds */
        PGC_ERR_CMP     = -1601,                /** Comparison Failed */
        PGC_ERR_ENC     = -1602,                /** Encoding Error */
        PGC_ERR_EOF     = -1603,                /** End of File */
        PGC_ERR_SYN     = -1604,                /** Syntax Error */
        PGC_ERR_FLO     = -1605,                /** Numeric Overflow */
        PGC_ERR_OOM     = -1606,                /** Out of Memory */
        PGC_ERR_SSL     = -1607                 /** SSL Error */
};

/**
 * Initialize error information.
 */
void pgc_err_init();

#endif