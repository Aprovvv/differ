#ifndef TREE_TRANSORMS_H
#define TREE_TRANSORMS_H

#define LEFT_IS_NUM(x) (node_to_left(x) && node_get_type(node_to_left(x)) == NUM)
#define RIGHT_IS_NUM(x) (node_to_right(x) && node_get_type(node_to_right(x)) == NUM)
#define LEFT_IS_VAR(x) (node_to_left(x) && node_get_type(node_to_left(x)) == VAR)
#define RIGHT_IS_VAR(x) (node_to_right(x) && node_get_type(node_to_right(x)) == VAR)
#define LEFT_IS_OP(x) (node_to_left(x) && node_get_type(node_to_left(x)) == OP)
#define RIGHT_IS_OP(x) (node_to_right(x) && node_get_type(node_to_right(x)) == OP)

const double EPS = 10e-5;

struct ARG {
    const char* str;
    int arg_code;
};

const size_t ARG_LEN = 32;

enum ARG_TYPES {
    NUM = 1,
    VAR,
    OP,
    FUNC
};

enum OP_CODES {
    ADD = 1,
    SUB,
    MULT,
    DIV,
    POW
};

enum FUNC_CODES {
    SIN,
    COS,
    TG,
    CTG,
    LN,
    EXP
};

const struct ARG VARS[] = {
    {"x", 'x'},
};

const struct ARG OPS[] = {
    {"+", ADD},
    {"-", SUB},
    {"*", MULT},
    {"/", DIV},
    {"^", POW}
};

const struct ARG FUNCS[] = {
    {"sin", SIN},
    {"cos", COS},
    {"tg", TG},
    {"ctg", CTG},
    {"ln", LN},
    {"exp", EXP}
};

tree_node_t* diff (tree_node_t* f);
tree_node_t* calc_node (tree_node_t* node);
tree_node_t* simplify (tree_node_t* node);
tree_node_t* delete_trivials (tree_node_t* node);

#endif
