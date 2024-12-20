#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <sys/stat.h>
#include "lexer.h"
#include "tree/tree.h"
#include "tree_transforms.h"

int pr (FILE* fp, const void* ptr, int type);
static int read_lex (FILE* fp, lexem* lex);
static void read_name (FILE* src, char* dest);
static double get_num(FILE* fp);

lexarr init_lexem_array (const char* filename)
{
    struct lexarr result = {};
    struct stat fileinfo = {};
    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "FAILED TO OPEN FILE %s\n", filename);
        return result;
    }
    stat(filename, &fileinfo);

    size_t lex_size = sizeof(lexem);
    result.ptr = (lexem*) calloc(lex_size, (size_t)fileinfo.st_size);
    if (result.ptr == NULL)
    {
        fprintf(stderr, "ERROR IN ALLOCATING LEXEM ARRAY\n");
        return lexarr {};
    }

    size_t i = 0;
    for (i = 0; i < (size_t)fileinfo.st_size; i++)
    {
        int code = read_lex(fp, result.ptr + i);
        if (code == 1)
        {
            fprintf(stderr, "error while reading lexem; p = %zu\n", i);
            free(result.ptr);
            return lexarr {};
        }
        if (code == 2)
            break;
    }
    result.size = i;
    fclose(fp);

    for (int i = 0; i < result.size; i++)
        fprintf(stderr, "type = %d; val = %f\n", result.ptr[i].type, result.ptr[i].val);

    return result;
}

static int read_lex (FILE* fp, lexem* lex)
{
    char ch = (char)getc(fp);
    while (ch == ' ')
        ch = (char)getc(fp);
    if (ch == EOF)
        return 2;
    if (isdigit(ch))
    {
        ungetc(ch, fp);
        lex->type = ARG_TYPE_NUM;
        lex->val = get_num(fp);
        return 0;
    }

    char arg[32] = {ch, '\0'};

    for (size_t i = 0; i < OPS_COUNT; i++)
    {
        if (strcmp(OPS[i].str, arg) == 0)
        {
            lex->val = OPS[i].arg_code;
            lex->type = ARG_TYPE_OP;
            return 0;
        }
    }

    read_name(fp, arg + 1);

    for (size_t i = 0; i < FUNCS_COUNT; i++)
    {
        if (strcmp(FUNCS[i].str, arg) == 0)
        {
            lex->val = FUNCS[i].arg_code;
            lex->type = ARG_TYPE_FUNC;
            return 0;
        }
    }

    for (size_t i = 0; i < VARS_COUNT; i++)
    {
        if (strcmp(VARS[i].str, arg) == 0)
        {
            lex->val = VARS[i].arg_code;
            lex->type = ARG_TYPE_VAR;
            return 0;
        }
    }

    return 1;
}

static double get_num(FILE* fp)
{
    double val = 0;
    FILE* fp_0 = fp;
    int less_zero = 0, point = 0;
    int ch = getc(fp);
    int k = 1;
    if (ch == '-')
    {
        less_zero = 1;
        ch = getc(fp);
    }
    while (('0' <= ch && ch <= '9') || ch == '.')
    {
        if (ch == '.')
        {
            point = 1;
            /*if (fp == fp_0)//FIXME пофиксить точку без числа
                return NAN;*/
            ch = getc(fp);
            continue;
        }
        if (!point)
            val = val*10 + ch - '0';
        else
            val += (double)(ch - '0') / pow(10, k++);
        ch = getc(fp);
    }
    ungetc(ch, fp);
    if (less_zero)
        val = -val;
    return val;
}

static void read_name (FILE* src, char* dest)
{
    int ch = getc(src);
    int i = 0;
    while (isalpha(ch))
    {
        dest[i++] = (char)ch;
        ch = getc(src);
    }
    ungetc(ch, src);
    dest[i] = 0;
}
