#ifndef TREE_TRANSORMS_H
#define TREE_TRANSORMS_H

#define LEFT_IS_NUM(x) (node_to_left(x) && node_get_type(node_to_left(x)) == NUM)
#define RIGHT_IS_NUM(x) (node_to_right(x) && node_get_type(node_to_right(x)) == NUM)
#define LEFT_IS_VAR(x) (node_to_left(x) && node_get_type(node_to_left(x)) == VAR)
#define RIGHT_IS_VAR(x) (node_to_right(x) && node_get_type(node_to_right(x)) == VAR)
#define LEFT_IS_OP(x) (node_to_left(x) && node_get_type(node_to_left(x)) == OP)
#define RIGHT_IS_OP(x) (node_to_right(x) && node_get_type(node_to_right(x)) == OP)
#define LEFT_IS_FUNC(x) (node_to_left(x) && node_get_type(node_to_left(x)) == FUNC)
#define RIGHT_IS_FUNC(x) (node_to_right(x) && node_get_type(node_to_right(x)) == FUNC)

//Функции, дифферириинцирующие разные узлы.
//хотел сделать static, но они нужны в это хедере
tree_node_t* diff_num(tree_node_t* f);
tree_node_t* diff_var(tree_node_t* f);
tree_node_t* diff_add_sub (tree_node_t* f);
tree_node_t* diff_mult (tree_node_t* f);
tree_node_t* diff_div (tree_node_t* f);
tree_node_t* diff_pow (tree_node_t* f);
tree_node_t* diff_sin (tree_node_t* f);
tree_node_t* diff_cos (tree_node_t* f);
tree_node_t* diff_tg (tree_node_t* f);
tree_node_t* diff_ctg (tree_node_t* f);
tree_node_t* diff_ln (tree_node_t* f);
tree_node_t* diff_exp (tree_node_t* f);
tree_node_t* diff_sqrt (tree_node_t* f);

const double EPS = 10e-6;

struct ARG {
    const char* str;
    int arg_code;
    tree_node_t* (*diff_func)(tree_node_t* f);
};

const size_t ARG_LEN = 32;

enum ARG_TYPES {
    NUM = 1,
    VAR,
    OP,
    FUNC
};
//TODO: PREFIX
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
    EXP,
    SQRT
};

//TODO: external

const struct ARG VARS[] = {
    {"x", 'x', diff_var},
};

const struct ARG OPS[] = {
    {"+", ADD, diff_add_sub},
    {"-", SUB, diff_add_sub},
    {"*", MULT, diff_mult},
    {"/", DIV, diff_div},
    {"^", POW, diff_pow}
};

const struct ARG FUNCS[] = {
    {"sin", SIN, diff_sin},
    {"cos", COS, diff_cos},
    {"tg", TG, diff_tg},
    {"ctg", CTG, diff_ctg},
    {"ln", LN, diff_ln},
    {"exp", EXP, diff_exp},
    {"sqrt", SQRT, diff_sqrt}
};

tree_node_t* diff (tree_node_t* f);
tree_node_t* calc_node (tree_node_t* node);
tree_node_t* simplify (tree_node_t* node);
tree_node_t* delete_trivials (tree_node_t* node);
void diff_and_tex (tree_node_t* root);

#endif
