#ifndef LEXER_H
#define LEXER_H

struct lexem
{
    int type;
    double val;
};

struct lexarr
{
    lexem* ptr;
    size_t size;
};

lexarr init_lexem_array (const char* filename);
#endif
