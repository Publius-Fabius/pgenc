
#include <stddef.h>
#include "pgenc/parser.h"
#include "pgenc/self.h"

// struct pgc_self;

// struct pgc_self *export_pgc_self()
// {
//         return NULL;
// }

// struct pgc_par *pgc_self_src(struct pgc_self *dict)
// {
//         return NULL;
// }

const struct pgc_par pgc_self_src = PGC_PAR_BYTE('a');
