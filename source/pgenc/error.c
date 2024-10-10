
#include "pgenc/error.h"

void pgc_err_init() 
{
        SEL_BIND(PGC_ERR_OK, "All OK");
        SEL_BIND(PGC_ERR_SYS, "System Call Error");
        SEL_BIND(PGC_ERR_OOB, "Index Out Of Bounds");
        SEL_BIND(PGC_ERR_CMP, "Comparison Failed");
        SEL_BIND(PGC_ERR_ENC, "Encoding Error");
        SEL_BIND(PGC_ERR_EOF, "End of File");
        SEL_BIND(PGC_ERR_SYN, "Syntax Error");   
        SEL_BIND(PGC_ERR_FLO, "Numeric Overflow");
        SEL_BIND(PGC_ERR_OOM, "Out of Memory");
        SEL_BIND(PGC_ERR_SSL, "SSL Error")

}
