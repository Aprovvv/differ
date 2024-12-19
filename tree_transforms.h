#ifndef TREE_TRANSORMS_H
#define TREE_TRANSORMS_H
#include "tree/tree.h"

#define LEFT_IS_NUM(x)   (node_to_left(x)  && node_get_type(node_to_left(x))  == ARG_TYPE_NUM)
#define RIGHT_IS_NUM(x)  (node_to_right(x) && node_get_type(node_to_right(x)) == ARG_TYPE_NUM)
#define LEFT_IS_VAR(x)   (node_to_left(x)  && node_get_type(node_to_left(x))  == ARG_TYPE_VAR)
#define RIGHT_IS_VAR(x)  (node_to_right(x) && node_get_type(node_to_right(x)) == ARG_TYPE_VAR)
#define LEFT_IS_OP(x)    (node_to_left(x)  && node_get_type(node_to_left(x))  == ARG_TYPE_OP)
#define RIGHT_IS_OP(x)   (node_to_right(x) && node_get_type(node_to_right(x)) == ARG_TYPE_OP)
#define LEFT_IS_FUNC(x)  (node_to_left(x)  && node_get_type(node_to_left(x))  == ARG_TYPE_FUNC)
#define RIGHT_IS_FUNC(x) (node_to_right(x) && node_get_type(node_to_right(x)) == ARG_TYPE_FUNC)

const double EPS = 10e-6;

struct ARG {
    const char* str;
    int arg_code;
    tree_node_t* (*diff_func)(FILE* fp, tree_node_t* f);
};

const size_t ARG_LEN = 32;

enum ARG_TYPE {
    ARG_TYPE_NUM = 1,
    ARG_TYPE_VAR,
    ARG_TYPE_OP,
    ARG_TYPE_FUNC
};
//TODO: PREFIX
enum OP_CODE {
    OP_CODE_ADD = 1,
    OP_CODE_SUB,
    OP_CODE_MULT,
    OP_CODE_DIV,
    OP_CODE_POW,
    OP_CODE_LBRACKET,
    OP_CODE_RBRACKET,
    OP_CODE_ENDOFEQ
};

enum FUNC_CODE {
    FUNC_CODE_SIN = 1,
    FUNC_CODE_COS,
    FUNC_CODE_TG,
    FUNC_CODE_CTG,
    FUNC_CODE_LN,
    FUNC_CODE_EXP,
    FUNC_CODE_SQRT
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
