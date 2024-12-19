#include <stdio.h>
#include <stdlib.h>
#include "tree_transforms.h"
#include "tree/tree.h"
#include "recursive_reader.h"
#include "lexer.h"

static tree_node_t* get_E (lexem* lex_arr);
static tree_node_t* get_T (lexem* lex_arr);
static tree_node_t* get_P (lexem* lex_arr);
static tree_node_t* get_N (lexem* lex_arr);

static void syntax_error (lexem* lexarr);

#define PTR(a, b) ((tree_node_t*)((char*)a + node_size*b))

static const size_t node_size = 40;
static int p = 0;

tree_node_t* get_grammar (lexem* lex_arr)
{
    p = 0;
    tree_node_t* node = get_E(lex_arr);
    fprintf(stderr, "(int)lex_arr[p].type = %d\n", (int)lex_arr[p].type);
    if ((int)lex_arr[p].val != DOLLAR)
        syntax_error(lex_arr);
    return node;
}

static tree_node_t* get_E (lexem* lex_arr)
{
    tree_node_t* node = get_T(lex_arr);
    long op = (long)lex_arr[p].val;
    fprintf(stderr, "op = %d\n", op);
    while((op == ADD || op == SUB) && lex_arr[p].type == OP)
    {
        p++;
        tree_node_t* val2 = get_T(lex_arr);
        node = new_node(&op, sizeof(op), OP, node, val2);
        op = (long)lex_arr[p].val;
    }
    return node;
}

static tree_node_t* get_T (lexem* lex_arr)
{
    tree_node_t* node = get_P(lex_arr);
    long op = (long)lex_arr[p].val;
    fprintf(stderr, "op = %d\n", op);
    while((op == MULT || op == DIV) && lex_arr[p].type == OP)
    {
        p++;
        tree_node_t* val2 = get_P(lex_arr);
        node = new_node(&op, sizeof(op), OP, node, val2);
        op = (long)lex_arr[p].val;
        fprintf(stderr, "p = %d, val = %f, type = %d, op = %d\n", p, lex_arr[p].val, lex_arr[p].type, op);
    }
    return node;
}

static tree_node_t* get_P (lexem* lex_arr)
{
    long val = (long)lex_arr[p].val;
    if (val == '(')
    {
        p++;
        tree_node_t* node = get_E(lex_arr);
        val = (long)lex_arr[p].val;
        if (val != ')')
            exit(EXIT_FAILURE);
        p++;
        return node;
    }
    else
    {
        return get_N(lex_arr);
    }
}

static tree_node_t* get_N (lexem* lex_arr)//TODO: дробные числа
{
    int type = lex_arr[p].type;
    if (type == NUM)
    {
        fprintf(stderr, "type = %f\n", lex_arr[p].val);
        tree_node_t* node = new_node(&lex_arr[p].val, sizeof(double), NUM, NULL, NULL);
        p++;
        return node;
    }
    else
        return NULL;
}

static void syntax_error (lexem* lex_arr)
{
    fprintf(stderr, "SYNTAX ERROR NEAR IN P = %d\n", p);
    exit(EXIT_FAILURE);
}
