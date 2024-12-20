#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <sys/stat.h>
#include "tree/tree.h"
#include "tree_transforms.h"
#include "latexing.h"

static int latex_num (FILE* fp, tree_node_t* node);
static int latex_var (FILE* fp, tree_node_t* node);
static int latex_func (FILE* fp, tree_node_t* node);
static int latex_op (FILE* fp, tree_node_t* node);
static int dblcmp (double a, double b);

int pr (FILE* fp, const void* ptr, int type);

FILE* create_texfile (const char* filename)
{
    struct stat fileinfo = {};
    FILE* hat = fopen("latex/texhat.txt", "r");
    if (!hat)
        return NULL;
    stat("latex/texhat.txt", &fileinfo);
    char* buf = (char*) calloc(1, (size_t)fileinfo.st_size);

    fread(buf, 1, (size_t)fileinfo.st_size, hat);

    FILE* texfile = fopen(filename, "w");
    fwrite(buf, 1, (size_t)fileinfo.st_size, texfile);

    fclose(hat);
    free(buf);
    return texfile;
}

int latex_tree (FILE* fp, tree_node_t* node)
{
    //tree_graph_dump(node, pr);
    switch (node_get_type(node))
    {
    case ARG_TYPE_NUM:
        latex_num(fp, node);
        break;
    case ARG_TYPE_VAR:
        latex_var(fp, node);
        break;
    case ARG_TYPE_FUNC:
        latex_func(fp, node);
        break;
    case ARG_TYPE_OP:
        latex_op(fp, node);
        break;
    default:
        assert(0);
    }
    return 0;
}

static int latex_num (FILE* fp, tree_node_t* node)
{
    double val;
    node_get_val(node, &val);
    if (val >= 0)
    {
        smart_double_print(fp, val);
    }
    else
    {
        fprintf(fp, "(");
        smart_double_print(fp, val);
        fprintf(fp, ")");
    }
    return 0;
}

static int latex_var (FILE* fp, tree_node_t* node)
{
    long val;
    node_get_val(node, &val);
    fprintf(fp, "%c", (int)val);
    return 0;
}

static int latex_func (FILE* fp, tree_node_t* node)
{
    long val;
    node_get_val(node, &val);
    for (size_t i = 0; i < FUNCS_COUNT; i++)
    {
        if (val == FUNCS[i].arg_code)
        {
            fprintf(fp, "\\%s{", FUNCS[i].str);
            break;;
        }
    }
    if (RIGHT_IS_NUM(node) || RIGHT_IS_VAR(node))
    {
        latex_tree(fp, node_to_right(node));
        fprintf(fp, "}");
    }
    else
    {
        fprintf(fp, "(");
        latex_tree(fp, node_to_right(node));
        fprintf(fp, ")}");
    }
    return 0;
}

static int latex_op (FILE* fp, tree_node_t* node)
{
    long val = 0;
    node_get_val(node, &val);
    switch (val)
    {
    case OP_CODE_ADD:
    {
        latex_tree(fp, node_to_left(node));
        fprintf(fp, "+");
        latex_tree(fp, node_to_right(node));
        return 0;
    }
    case OP_CODE_SUB:
    {
        double val = 0;
        node_get_val(node_to_left(node), &val);
        if (LEFT_IS_NUM(node) && dblcmp(val, 0) != 0)
            latex_tree(fp, node_to_left(node));
        fprintf(fp, "-");
        latex_tree(fp, node_to_right(node));
        return 0;
    }
    case OP_CODE_MULT:
        if (LEFT_IS_OP(node))
        {
            long left_val = 0;
            node_get_val(node_to_left(node), &left_val);
            if (left_val == OP_CODE_SUB || left_val == OP_CODE_ADD)
            {
                fprintf(fp, "(");
                latex_tree(fp, node_to_left(node));
                fprintf(fp, ")");
            }
            else
            {
                latex_tree(fp, node_to_left(node));
            }
        }
        else
        {
            latex_tree(fp, node_to_left(node));
        }

        if (!RIGHT_IS_VAR(node) || !LEFT_IS_NUM(node))
            fprintf(fp, " \\cdot ");

        if (RIGHT_IS_OP(node))
        {
            long right_val = 0;
            node_get_val(node_to_right(node), &right_val);
            if (right_val == OP_CODE_SUB || right_val == OP_CODE_ADD)
            {
                fprintf(fp, "(");
                latex_tree(fp, node_to_right(node));
                fprintf(fp, ")");
            }
            else
            {
                latex_tree(fp, node_to_right(node));
            }
        }
        else
        {
            latex_tree(fp, node_to_right(node));
        }

        return 0;
    case OP_CODE_DIV:
        fprintf(fp, "\\frac{");
        latex_tree(fp, node_to_left(node));
        fprintf(fp, "}{");
        latex_tree(fp, node_to_right(node));
        fprintf(fp, "}");
        return 0;
    case OP_CODE_POW:
        if (LEFT_IS_OP(node) || LEFT_IS_FUNC(node))
        {
            fprintf(fp, "(");
            latex_tree(fp, node_to_left(node));
            fprintf(fp, ")");
        }
        else
        {
            latex_tree(fp, node_to_left(node));
        }

        fprintf(fp, "^{");
        latex_tree(fp, node_to_right(node));
        fprintf(fp, "}");
        return 0;
    default:
        assert(0);
    }
}

void smart_double_print(FILE* fp, double x)
{
    int sign_count = 0;
    double i_part = 0;
    double frac_part = abs(modf(x, &i_part));
    int int_part = (int)i_part;

    fprintf(fp, "%d", int_part);

    if ((int)(frac_part*10e6) > 0)
    {
        for (int i = 10e6; i >= 10; i /= 10)
            if ((int)(frac_part*i)%10 > 0)
                sign_count++;

        fprintf(fp, ".%*d", sign_count,
                (int)(frac_part*pow(10, sign_count)));
    }
}

static int dblcmp (double a, double b)
{
    if (a - b > EPS)
        return 1;
    if (b - a > EPS)
        return -1;
    return 0;
}
