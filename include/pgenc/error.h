#ifndef PGENC_ERROR_H
#define PGENC_ERROR_H

/** Type for returning errors */
enum pgc_err 
{
        PGC_ERR_OK      = 0,            /** No error. */
        PGC_ERR_SYS     = -1,           /** System error (check errno). */
        PGC_ERR_OOB     = -2,           /** Buffer index went out of bounds. */
        PGC_ERR_CMP     = -3,           /** Comparison operation failed. */
        PGC_ERR_ENC     = -4,           /** Encoding error. */
        PGC_ERR_EOF     = -5,           /** End of file was encountered. */
        PGC_ERR_SYN     = -6,           /** Syntax error. */
        PGC_ERR_FLO     = -7,           /** Numeric overflow error. */
        PGC_ERR_OOM     = -8            /** Allocator ran out of memory. */
};

/**
 * Get the error's string value.
 * @param error The error
 * @return The error's string value.
 */
const char *pgc_strerror(const enum pgc_err error);

/**
 * Abort the process. 
 * @param file The file where the process was aborted.
 * @param func The function where the process was aborted.
 * @param line The line number where the process was aborted.
 * @return pgc_abort(..) doesn't actually return, but its type returns an
 * error so pgc_abort(..) can be used as a return value.
 */
enum pgc_err pgc_abort(
        const char *file,
        const char *func,
        const int line);

/**
 * Abort the process, printing out some error information.
 */
#define PGC_ABORT() pgc_abort(__FILE__, __func__, __LINE__)

/**
 * If the expression evaluates to false, abort the process.  This macro
 * expands to nothing when __OPTIMIZE__ is defined.
 * @param EXPR The expression
 */
#ifndef __OPTIMIZE__
#define PGC_ASSERT(EXPR) \
        if(!(EXPR)) { \
                pgc_abort(__FILE__, __func__, __LINE__); \
        }
#else
#define PGC_ASSERT(EXPR) 
#endif

/**
 * Does the same thing as PGC_ASSERT, but the macro definition is 
 * not affected by the __OPTIMIZE__ flag.
 * @param EXPR The expression
 */
#define PGC_TEST(EXPR) \
        if(!(EXPR)) { \
                pgc_abort(__FILE__, __func__, __LINE__); \
        }

/**
 * Report an error, printing out useful information to stderr.
 * @param file The file where the error occurred.
 * @param func The function where the error occurred.
 * @param line The line number where the error occurred.
 * @return error
 */
enum pgc_err pgc_report(
        const enum pgc_err error,
        const char *file,
        const char *func,
        const int line);

/**
 * Report trace information to stderr.
 * @param file The file where the trace occurred.
 * @param func The function where the trace occurred.
 * @param line The line number where the trace occurred.
 */
void pgc_trace(
        const char *file,
        const char *func,
        const int line);

/**
 * Report and return an error.
 */
#define PGC_THROW(ERROR) \
        return pgc_report(ERROR, __FILE__, __func__, __LINE__); 

/**
 * If expr evaluates to true throw error.
 */
#define PGC_IF_THROW(EXPR, ERROR) \
        if(EXPR) {\
                return pgc_report(ERROR, __FILE__, __func__, __LINE__);\
        } 

#define PGC_IO(EXPR) { \
        const int PGC_IO_ERR = EXPR; \
        if(PGC_IO_ERR < 0) { \
                return pgc_report(PGC_ERR_SYS, __FILE__, __func__, __LINE__); \
        } \
}

#define PGC_IO_QUIETLY(EXPR) { \
        const int PGC_IO_ERR = EXPR; \
        if(PGC_IO_ERR < 0) { \
                return PGC_ERR_SYS; \
        } \
}

/**
 * If the expression evaluates to an error throw it. 
 * @param EXPR An expression that may produce an error.
 */
#define PGC_TRY(EXPR) {\
        const enum pgc_err PGC_TRY_ERR = EXPR;\
        if((PGC_TRY_ERR) != PGC_ERR_OK) {\
                return pgc_report(PGC_TRY_ERR, __FILE__, __func__, __LINE__);\
        }\
}

/**
 * If the expression evaluates to an error return it. 
 * @param EXPR An expression that may produce an error.
 */
#define PGC_TRY_QUIETLY(EXPR) { \
        const enum pgc_err PGC_TRY_ERR = EXPR; \
        if(PGC_TRY_ERR != PGC_ERR_OK) { \
                return PGC_TRY_ERR; \
        } \
}

/**
 * If the expression returns an error, print out tracing information.
 * @param EXPR The expression to try (usually a function call).
 */
#define PGC_TRACE(EXPR) {\
        const enum pgc_err PGC_TRACE_ERR = EXPR;\
        if((PGC_TRACE_ERR) != PGC_ERR_OK) {\
                pgc_trace(__FILE__, __func__, __LINE__);\
                return PGC_TRACE_ERR;\
        }\
}

void pgc_info(
        const char *file,
        const char *func,
        const int line);

#define PGC_INFO() pgc_info(__FILE__, __func__, __LINE__)

#endif