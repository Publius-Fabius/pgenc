# pgenc - Parser Generator for C

This project provides a robust option for generating highly optimized static 
parsers in standard C99.  Parsers are defined as a grammar in a language 
inspired by EBNF, but with several powerful extensions.  These grammar files 
can then be used to generate text parsers in standard C.  PGENC is fully self 
parsing and comes with its own definition defined in its own language 
(see grammar/self.g).

In a file named input.grammar:
```
    let dog = 'd' 'o' 'g';
    let cat = 'c' 'a' 't';
    let animal = dog | cat;
```

Generate the parsers:
```
    bin/pgenc -g input.grammar -s my_parsers.c -d my_parsers
```

Use the parsers in your code like so:
```
    #include "pgenc/parser.h"

    extern const struct pgc_par my_parsers_dog;
    extern const struct pgc_par my_parsers_cat;
    extern const struct pgc_par my_parsers_animal;
```

Compile in the standard manner:
```
    gcc -o my_program my_code.c my_parsers.c
```

Here's an example of some generated code to parse the grammar itself:
```
static const struct pgc_cset p0_p = { .words = { 0x3e00 , 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 } }; 
const struct pgc_par pgc_self_wsc = PGC_PAR_SET(&p0_p); 
const struct pgc_par pgc_self_ws = PGC_PAR_REP(&pgc_self_wsc, 0, 512); 
static const struct pgc_cset p1_p = { .words = { 0x0 , 0x0, 0x7fffffe, 0x7fffffe, 0x0, 0x0, 0x0, 0x0 } }; 
const struct pgc_par pgc_self_alpha = PGC_PAR_SET(&p1_p); 
static const struct pgc_cset p2_p = { .words = { 0x0 , 0x3ff0000, 0x87fffffe, 0x7fffffe, 0x0, 0x0, 0x0, 0x0 } }; 
const struct pgc_par pgc_self_idc = PGC_PAR_SET(&p2_p); 
static const struct pgc_par p3_p = PGC_PAR_REP(&pgc_self_idc, 0, 256); 
const struct pgc_par pgc_self_idpat = PGC_PAR_AND(&pgc_self_alpha, &p3_p); 
const struct pgc_par pgc_self_id = PGC_PAR_CALL(pgc_lang_capid, &pgc_self_idpat); 
static const struct pgc_cset p4_p = { .words = { 0x0 , 0x3ff0000, 0x7e, 0x7e, 0x0, 0x0, 0x0, 0x0 } }; 
const struct pgc_par pgc_self_xdigit = PGC_PAR_SET(&p4_p); 
const struct pgc_par pgc_self_xdigits = PGC_PAR_REP(&pgc_self_xdigit, 1, 2); 

```