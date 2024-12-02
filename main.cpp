#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include "tree/tree.h"

#define LEFT_IS_NUM(x) (node_to_left(x) && node_get_type(node_to_left(x)) == NUM)
#define RIGHT_IS_NUM(x) (node_to_right(x) && node_get_type(node_to_right(x)) == NUM)
#define LEFT_IS_VAR(x) (node_to_left(x) && node_get_type(node_to_left(x)) == VAR)
#define RIGHT_IS_VAR(x) (node_to_right(x) && node_get_type(node_to_right(x)) == VAR)
#define LEFT_IS_OP(x) (node_to_left(x) && node_get_type(node_to_left(x)) == OP)
#define RIGHT_IS_OP(x) (node_to_right(x) && node_get_type(node_to_right(x)) == OP)

int is_num (const char* str);
int is_int(double x);
char next_nonspace (FILE* fp);
char getc_until (FILE* fp, const char* str);
tree_node_t* tree_from_file (FILE* input);
int pr (FILE* fp, const void* ptr, int type);
tree_node_t* diff (tree_node_t* f);
tree_node_t* calc_node (tree_node_t* node);
tree_node_t* simplify (tree_node_t* node);
tree_node_t* delete_trivials (tree_node_t* node);

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

int main()
{
    FILE* input = fopen("txt.txt", "r");
    tree_node_t* root = tree_from_file(input);
    if (root == NULL)
        return EXIT_FAILURE;
    fprintf(stderr, "tree:\n");
    tree_print(stderr, root, pr);
    tree_graph_dump(root, pr);
    putchar('\n');

    tree_node_t* droot = diff(root);
    tree_print(stderr, droot, pr);
    tree_graph_dump(droot, pr);
    putchar('\n');

    root = simplify(root);
    tree_print(stderr, root, pr);
    tree_graph_dump(root, pr);
    putchar('\n');

    droot = simplify(droot);
    tree_print(stderr, droot, pr);
    tree_graph_dump(droot, pr);
    putchar('\n');

    branch_delete(root);
    branch_delete(droot);
    fclose(input);
}

int pr (FILE* fp, const void* ptr, int type)
{
    //fprintf(stderr, "in pr type = %d\n", type);
    switch (type)
    {
    case NUM:
        return fprintf(fp, "%f", (*(double*)ptr));
    case VAR:
        return fprintf(fp, "%c", *((const int*)ptr));
    case OP:
        for (size_t i = 0; i < sizeof(OPS)/sizeof(OPS[0]); i++)
        {
            //fprintf(stderr, "OPS[i].arg_code = %d; *((const long*)ptr) = %d\n", OPS[i].arg_code, *((const long*)ptr));
            if (OPS[i].arg_code == *((const int*)ptr))
            {
                //fprintf(stderr, "in pr i = %d\n", i);
                return fprintf(fp, "%s", OPS[i].str);
            }
        }
        assert(0);
        break;
    case FUNC:
        for (size_t i = 0; i < sizeof(FUNCS)/sizeof(FUNCS[0]); i++)
        {
            //fprintf(stderr, "OPS[i].arg_code = %d; *((const long*)ptr) = %d\n", OPS[i].arg_code, *((const long*)ptr));
            if (FUNCS[i].arg_code == *((const int*)ptr))
            {
                //fprintf(stderr, "in pr i = %d\n", i);
                return fprintf(fp, "%s", FUNCS[i].str);
            }
        }
        assert(0);
        break;
    default:
        assert(0);
    }
    return 0;
}

tree_node_t* tree_from_file (FILE* fp)
{
    char ch = getc(fp);
    fprintf(stderr, "start ch = %c\n", ch);
    tree_node_t* node_temp_ptr = NULL;
    tree_node_t* node = NULL;
    if (ch == '(')
    {
        node_temp_ptr = tree_from_file(fp);
        ch = getc(fp);
    }
    char arg[ARG_LEN] = "";
    size_t i = 0;
    while (ch != '(' && ch != ')' && i < ARG_LEN && ch != EOF)
    {
        arg[i++] = ch;
        ch = getc(fp);
    }
    arg[i] = 0;

    if (is_num(arg))
    {
        double num = 0;
        sscanf(arg, "%lf", &num);
        //fprintf(stderr, "num = %f\n", num);
        node = new_node(&num, sizeof(num), NUM, NULL, NULL);
        node_add_left(node, node_temp_ptr);
    }

    if (node == NULL)
    {
        for (i = 0; i < sizeof(VARS)/sizeof(VARS[0]); i++)
        {
            if (strcmp(VARS[i].str, arg) == 0)
            {
                int code = VARS[i].arg_code;
                node = new_node(&code, sizeof(code), VAR, node_temp_ptr, NULL);
                break;
            }
        }
    }

    if (node == NULL)
    {
        for (i = 0; i < sizeof(OPS)/sizeof(OPS[0]); i++)
        {
            if (strcmp(OPS[i].str, arg) == 0)
            {
                //fprintf(stderr, "i = %d\n", i);
                int code = OPS[i].arg_code;
                node = new_node(&code, sizeof(code), OP, node_temp_ptr, NULL);
                break;
            }
        }
    }
    if (node == NULL)
    {
        for (i = 0; i < sizeof(FUNCS)/sizeof(FUNCS[0]); i++)
        {
            if (strcmp(FUNCS[i].str, arg) == 0)
            {
                //fprintf(stderr, "i = %d\n", i);
                int code = FUNCS[i].arg_code;
                node = new_node(&code, sizeof(code), FUNC, node_temp_ptr, NULL);
                break;
            }
        }
    }
    if (ch == '(')
    {
        //ungetc('(', fp);
        node_temp_ptr = tree_from_file(fp);
        node_add_right(node, node_temp_ptr);
        getc(fp);
    }
    fprintf(stderr, "arg = %s; ch = %c\n", arg, ch);
    return node;
}

tree_node_t* diff (tree_node_t* f)
{
    tree_node_t* df = NULL;
    if (node_get_type(f) == NUM)
    {
        double val = 0;
        df = new_node(&val, sizeof(val), NUM, NULL, NULL);
        return df;
    }
    if (node_get_type(f) == VAR)
    {
        double val = 1;
        df = new_node(&val, sizeof(val), NUM, NULL, NULL);
        return df;
    }
    if (node_get_type(f) == OP)
    {
        int val = 0;
        node_get_val(f, &val);
        //fprintf(stderr, "val = %c = %d\n", val, val);
        switch(val)
        {
        case ADD:
        case SUB:
            return new_node(&val, sizeof(val), OP,
                            diff(node_to_left(f)), diff(node_to_right(f)));
        case MULT:
        {
            int op = MULT;
            tree_node_t* left_mult =
                new_node(&op, sizeof(op), OP,
                         diff(node_to_left(f)), branch_copy(node_to_right(f)));
            tree_node_t* right_mult =
                new_node(&op, sizeof(op), OP,
                         branch_copy(node_to_left(f)), diff(node_to_right(f)));
            op = ADD;
            return new_node(&op, sizeof(op), OP,
                        left_mult, right_mult);
        }
        case DIV:
        {
            int mult = MULT, sub = SUB, div = DIV, pw = POW;
            double two = 2;
            tree_node_t* numer =
                new_node(&sub, sizeof(sub), OP,
                         new_node(&mult, sizeof(mult), OP,
                                  diff(node_to_left(f)), branch_copy(node_to_right(f))),
                         new_node(&mult, sizeof(mult), OP,
                                  branch_copy(node_to_left(f)), diff(node_to_right(f))));
            tree_node_t* denumer =
                new_node(&pw, sizeof(pw), OP,
                         branch_copy(node_to_right(f)),
                         new_node(&two, sizeof(two), NUM,
                                  NULL, NULL));
            return new_node(&div, sizeof(div), OP, numer, denumer);
        }
        case POW://FIXME: общий случай
        {
            if (LEFT_IS_VAR(f) && RIGHT_IS_NUM(f))
            {
                int mult = MULT, pw = POW;
                double old_pow = 0;
                node_get_val(node_to_right(f), &old_pow);
                double new_pow = old_pow - 1;
                tree_node_t* base =
                    new_node(&mult, sizeof(mult), OP,
                             new_node(&old_pow, sizeof(old_pow), NUM, NULL, NULL),
                             branch_copy(node_to_left(f)));
                return new_node(&pw, sizeof(pw), OP,
                                base,
                                new_node(&new_pow, sizeof(new_pow), NUM, NULL, NULL));

            }
            if (LEFT_IS_NUM(f) && RIGHT_IS_NUM(f))
            {
                double val = 0;
                df = new_node(&val, sizeof(val), NUM, NULL, NULL);
                return df;
            }
            break;
        }
        default:
            assert(0);
        }
    }

    if (node_get_type(f) == FUNC)
    {
        int val = 0;
        node_get_val(f, &val);
        switch(val)
        {
            case SIN:
            {
                int cos = COS;
                return new_node(&cos, sizeof(cos), FUNC,
                                NULL, branch_copy(node_to_right(f)));
            }
            case COS:
            {
                int sin = SIN, mult = MULT;
                double m_1 = -1;
                tree_node_t* minus_1 =
                    new_node(&m_1, sizeof(m_1), NUM,
                             NULL, NULL);
                tree_node_t* sin_node =
                    new_node(&sin, sizeof(sin), FUNC,
                             NULL, branch_copy(node_to_right(f)));
                return new_node(&mult, sizeof(mult), OP, minus_1, sin_node);
            }
        }
    }
    return NULL;
}

tree_node_t* simplify (tree_node_t* node)
{
    tree_node_t* result = node;

    if (node_to_left(node))
        node_add_left(node, simplify(node_to_left(node)));
    if (node_to_right(node))
        node_add_right(node, simplify(node_to_right(node)));


    if (node_get_type(result) == OP && LEFT_IS_NUM(result) && RIGHT_IS_NUM(result))
    {
        fprintf(stderr, "calcing node %p\n", node);
        result = calc_node(result);
    }

    if (node_get_type(result) == OP && (LEFT_IS_NUM(result) || RIGHT_IS_NUM(result)))
    {
        fprintf(stderr, "deleting node %p, \n", node);
        result = delete_trivials(result);
    }

    //tree_graph_dump(node, pr);

    return result;
}

tree_node_t* delete_trivials (tree_node_t* node)
{
    int type = node_get_type(node);
    int val = 0;
    double val_left = 666, val_right = 666;
    node_get_val(node, &val);
    if (LEFT_IS_NUM(node))
        node_get_val(node_to_left(node), &val_left);
    if (RIGHT_IS_NUM(node))
        node_get_val(node_to_right(node), &val_right);
    tree_node_t* result = NULL;

    switch(val)
    {
        case ADD:
        //TODO: здесь копируется и возвращается вся ненулевая ветка.
        //было бы лучше удалить всего два узла, но нет нужной функции
            if (val_left == 0)
            {
                result = branch_copy(node_to_right(node));
                branch_delete(node);
                return result;
            }
        case SUB:
            if (val_right == 0)
            {
                result = branch_copy(node_to_left(node));
                branch_delete(node);
                return result;
            }
            break;
        case MULT:
            if (val_right == 0)
            {
                double zero = 0;
                branch_delete(node);
                fprintf(stderr, "ASD;LFKJAKLW; NHGASDFKJ\n");
                return new_node(&zero, sizeof(zero), NUM, NULL, NULL);
            }
            if (val_left == 1)
            {
                result = branch_copy(node_to_right(node));
                branch_delete(node);
                return result;
            }
        case DIV:
            if (val_left == 0)
            {
                double zero = 0;
                branch_delete(node);
                return new_node(&zero, sizeof(zero), NUM, NULL, NULL);
            }
            if (val_right == 1)
            {
                result = branch_copy(node_to_left(node));
                branch_delete(node);
                return result;
            }
            break;
        case POW:
            if (val_right == 1)
            {
                result = branch_copy(node_to_left(node));
                branch_delete(node);
                return result;
            }
            if (val_right == 0)
            {
                double one = 1;
                branch_delete(node);
                return new_node(&one, sizeof(one), NUM, NULL, NULL);
            }
            break;
        default:
            return NULL;
    }
    return node;
}

tree_node_t* calc_node (tree_node_t* node)
{
    int type = node_get_type(node);
    int val = 0;
    double val_left = 0, val_right = 0, result;
    node_get_val(node, &val);
    node_get_val(node_to_left(node), &val_left);
    node_get_val(node_to_right(node), &val_right);
    //fprintf(stderr, "val = %d\n", val);
    fprintf(stderr, "&node = %p; type = %d; left %f; right %f\n", node, node_get_type(node), val_left, val_right);
    fprintf(stderr, "is_int = %d\n", is_int(val_left));

    if (val == POW)
        if (!is_int(val_right))
            return node;
    switch(val)
    {
        case ADD:
            result = val_left + val_right;
            break;
        case SUB:
            result = val_left - val_right;
            break;
        case MULT:
            result = val_left * val_right;
            break;
        case DIV:
            result = val_left / val_right;
            break;
        case POW:
            result = pow(val_left, val_right);
            break;
        default:
            return NULL;
    }
    branch_delete(node);
    return new_node(&result, sizeof(result), NUM, NULL, NULL);
}

int is_num (const char* str)
{
    int i = 0;
    while(str[i] != 0)
    {
        if (!isdigit(str[i]) && str[i] != '.')
            return 0;
        i++;
    }
    return 1;
}

char next_nonspace (FILE* fp)
{
    char ch = (char)getc(fp);
    while (isspace(ch) && ch != 0 && ch != EOF)
        ch = (char)getc(fp);
    return ch;
}

char getc_until (FILE* fp, const char* str)
{
    char ch = (char)getc(fp);
    while (strchr(str, ch) == NULL && ch != 0 && ch != EOF)
        ch = (char)getc(fp);
    return ch;
}

int is_int(double x)
{
    double temp = 0;
    //fprintf(stderr, "x = %f\n", x);
    if (abs(modf(x, &temp)) < EPS)
        return 1;
    return 0;
}
