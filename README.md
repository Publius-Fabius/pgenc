# pgenc - Parser Generator for C

This project provides a robust option for generating efficient parsers in 
standard C99.  Parsers are defined as a grammar in a language inspired by 
EBNF, but with several powerful extensions, including UTF8!  PGENC is fully 
self parsing and comes with its own definition defined in its own language 
(see grammar/self.g).

PGENC relies on no OS specific code, so it should be perfectly portable to any
architecture/system with a C99 compliant compiler.  

Clone PGENC and SELC:
```
    git clone https://github.com/Publius-Fabius/selc.git
    git clone https://github.com/Publius-Fabius/pgenc.git
    cd pgenc
    make bin/pgenc
```

In a file named input.grammar:
```
    let dog = 'd' 'o' 'g';
    let cat = 'c' 'a' 't';
    let animal = dog | cat;
```

Generate the parsers:
```
    bin/pgenc my_parsers.c my_parsers input.grammar 
```

Use the parsers in your code like so:
```
    #include "pgenc/parser.h"

    /* Forward declarations, be sure to include 'const'. */
    extern const struct pgc_par my_parsers_dog;
    extern const struct pgc_par my_parsers_cat;
    extern const struct pgc_par my_parsers_animal;

    int example_function() {
        if(pgc_par_runs(&my_parsers_dog, "dog!!!", NULL) != 3) {
            return -1;
        }
    }
```

Compile and link in the standard manner:
```
    gcc -o my_program my_code.c my_parsers.c
```
