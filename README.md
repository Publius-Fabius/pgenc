# pgenc - Parser Generators for C

This project provides a robust option for generating highly optimized static parsers in standard C99.  Parsers are defined as a grammar in a language inspired by EBNF, but with several powerful extensions.  These grammar files can then be used to generate parsers in standard C that can be used for a variety of parsing applications.

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
