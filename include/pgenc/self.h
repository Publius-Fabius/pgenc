#ifndef PGENC_SELF_H
#define PGENC_SELF_H

#include "pgenc/parser.h"

/** Dictionary for self-parsing syntax */
struct pgc_self;

/** 
 * Get the pgc_self dictionary. 
 * @return the dictionary
 */
struct pgc_self *export_pgc_self();

/** 
 * Get 'src' parser definition from the dictionary.
 * @param dict The pgc_self dictionary.
 * @return the 'src' parser
 */
struct pgc_par *pgc_self_src(struct pgc_self *dict);

#endif