# pgenc - Parser Generators for C

This project provides a robust option for generating static parsers in standard C99.  Parsers are defined as a grammar in a language inspired by EBNF, but with several powerful extensions.  These grammar files can then be used to generate parsers in standard C that can be used for a variety of parsing applications.

Example Grammar:
```
    let dog = 'd' 'o' 'g';
    let cat = 'c' 'a' 't';
    let animal = dog | cat;
```

Example Use:
```
    #include "pgenc/parser.h"

    struct my_grammar *export_my_grammar();
    struct pgc_par *my_grammar_dog(struct my_grammar *);

    void do_stuff() {
        struct my_grammar *gram = export_my_grammar();
        struct pgc_par *dog = my_grammar_dog(gram);
        pgc_par_run(... example code, see headers for necessary arguments ...);
        
	...

    }
```
