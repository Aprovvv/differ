#include <stdio.h>
#include <stdlib.h>
#include "tree_transforms.h"
#include "tree/tree.h"
#include "recursive_reader.h"
#include "lexer.h"

static tree_node_t* get_PM  (lexem* lex_arr);
static tree_node_t* get_MD  (lexem* lex_arr);
static tree_node_t* get_BR  (lexem* lex_arr);
static tree_node_t* get_POW (lexem* lex_arr);
static tree_node_t* get_NUM (lexem* lex_arr);
static tree_node_t* get_VAR (lexem* lex_arr);
static tree_node_t* get_FUNC(lexem* lex_arr);

static void syntax_error (lexem* lexarr);

#define PTR(a, b) ((tree_node_t*)((char*)a + NODE_SIZE*b))

static const size_t NODE_SIZE = NODE_BASE_SIZE + sizeof(double);
static int p = 0;

tree_node_t* get_grammar (lexem* lex_arr)
{
    p = 0;
    tree_node_t* node = get_PM(lex_arr);

    if (node == NULL || (int)lex_arr[p].val != OP_CODE_ENDOFEQ)
        syntax_error(lex_arr);
    return node;
}

static tree_node_t* get_PM (lexem* lex_arr)
{
    tree_node_t* node = get_MD(lex_arr);
    tree_node_t* node2 = NULL;
    long op = 0;
    if (!node)
        goto sntxerr;
    if (node == NULL)
    {
        long zero = 0;
        node = new_node(&zero, sizeof(zero), ARG_TYPE_NUM, NULL, NULL);
    }
    op = (long)lex_arr[p].val;
    while((op == OP_CODE_ADD || op == OP_CODE_SUB)
            && lex_arr[p].type == ARG_TYPE_OP)
    {
        p++;
        node2 = get_MD(lex_arr);
        if (!node2)
            goto sntxerr;
        node = new_node(&op, sizeof(op), ARG_TYPE_OP, node, node2);
        node = simplify(node);
        op = (long)lex_arr[p].val;
    }
    return node;
sntxerr:
    branch_delete(node);
    branch_delete(node2);
    return NULL;
}

static tree_node_t* get_MD (lexem* lex_arr)
{
    tree_node_t* node = get_POW(lex_arr);
    tree_node_t* node2 = NULL;
    long op = 0;
    if (!node)
        goto sntxerr;
    op = (long)lex_arr[p].val;
    while((op == OP_CODE_MULT || op == OP_CODE_DIV)
            && lex_arr[p].type == ARG_TYPE_OP)
    {
        p++;
        node2 = get_POW(lex_arr);
        if (!node2)
            goto sntxerr;
        node = new_node(&op, sizeof(op), ARG_TYPE_OP, node, node2);
        op = (long)lex_arr[p].val;
    }
    return node;
sntxerr:
    branch_delete(node);
    branch_delete(node2);
    return NULL;
}

static tree_node_t* get_POW (lexem* lex_arr)
{
    tree_node_t* node = get_BR(lex_arr);
    tree_node_t* node2 = NULL;
    long op = 0;

    if (!node)
        goto sntxerr;
    op = (long)lex_arr[p].val;
    while(op == OP_CODE_POW && lex_arr[p].type == ARG_TYPE_OP)
    {
        p++;
        node2 = get_BR(lex_arr);
        if (!node2)
            goto sntxerr;
        node = new_node(&op, sizeof(op), ARG_TYPE_OP, node, node2);
        op = (long)lex_arr[p].val;
    }
    return node;
sntxerr:
    branch_delete(node);
    branch_delete(node2);
    return NULL;
}

static tree_node_t* get_BR (lexem* lex_arr)
{
    long val = (long)lex_arr[p].val;
    tree_node_t* node = NULL;
    if (val == OP_CODE_LBRACKET && lex_arr[p].type == ARG_TYPE_OP)
    {
        p++;
        node = get_PM(lex_arr);
        if (!node)
            goto sntxerr;
        val = (long)lex_arr[p].val;
        if (val != OP_CODE_RBRACKET)
            goto sntxerr;
        p++;
        return node;
    }
    else
    {
        switch (lex_arr[p].type)
        {
        case ARG_TYPE_NUM:
            return get_NUM(lex_arr);
        case ARG_TYPE_VAR:
            return get_VAR(lex_arr);
        case ARG_TYPE_FUNC:
            return get_FUNC(lex_arr);
        default:
            return NULL;
        }
    }
sntxerr:
    branch_delete(node);
    return NULL;
}

static tree_node_t* get_NUM (lexem* lex_arr)
{
    return new_node(&lex_arr[p++].val, sizeof(double),
                    ARG_TYPE_NUM, NULL, NULL);
}

static tree_node_t* get_VAR(lexem* lex_arr)
{
    long var = (long)lex_arr[p++].val;
    return new_node(&var, sizeof(var), ARG_TYPE_VAR, NULL, NULL);
}

static tree_node_t* get_FUNC(lexem* lex_arr)
{
    long func = (long)lex_arr[p++].val;
    return new_node(&func, sizeof(func),
                    ARG_TYPE_FUNC, NULL, get_BR(lex_arr));
}

static void syntax_error (lexem* lex_arr)
{
    fprintf(stderr, "SYNTAX ERROR IN P = %d\n", p);
    free(lex_arr);
    exit(EXIT_FAILURE);
}
