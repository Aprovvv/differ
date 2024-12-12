#ifndef TREE_TRANSORMS_H
#define TREE_TRANSORMS_H
#include "tree/tree.h"

#define LEFT_IS_NUM(x)   (node_to_left(x)  && node_get_type(node_to_left(x))  == NUM)
#define RIGHT_IS_NUM(x)  (node_to_right(x) && node_get_type(node_to_right(x)) == NUM)
#define LEFT_IS_VAR(x)   (node_to_left(x)  && node_get_type(node_to_left(x))  == VAR)
#define RIGHT_IS_VAR(x)  (node_to_right(x) && node_get_type(node_to_right(x)) == VAR)
#define LEFT_IS_OP(x)    (node_to_left(x)  && node_get_type(node_to_left(x))  == OP)
#define RIGHT_IS_OP(x)   (node_to_right(x) && node_get_type(node_to_right(x)) == OP)
#define LEFT_IS_FUNC(x)  (node_to_left(x)  && node_get_type(node_to_left(x))  == FUNC)
#define RIGHT_IS_FUNC(x) (node_to_right(x) && node_get_type(node_to_right(x)) == FUNC)

const double EPS = 10e-6;

struct ARG {
    const char* str;
    int arg_code;
    tree_node_t* (*diff_func)(FILE* fp, tree_node_t* f);
};

const size_t ARG_LEN = 32;

enum ARG_TYPE {
    NUM = 1,
    VAR,
    OP,
    FUNC
};
//TODO: PREFIX
enum OP_CODE {
    ADD = 1,
    SUB,
    MULT,
    DIV,
    POW,
    LBRACKET,
    RBRACKET,
    DOLLAR
};

enum FUNC_CODE {
    SIN,
    COS,
    TG,
    CTG,
    LN,
    EXP,
    SQRT
};

//TODO: external

extern const struct ARG VARS[];
extern const struct ARG OPS[];
extern const struct ARG FUNCS[];

extern const size_t VARS_COUNT;
extern const size_t OPS_COUNT;
extern const size_t FUNCS_COUNT;

tree_node_t* diff (FILE* fp, tree_node_t* f);
tree_node_t* calc_node (tree_node_t* node);
tree_node_t* simplify (tree_node_t* node);
tree_node_t* delete_trivials (tree_node_t* node);
tree_node_t* diff_and_tex (tree_node_t* root);

#endif
