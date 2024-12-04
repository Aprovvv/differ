#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "tree/tree.h"
#include "tree_transforms.h"
#include "latexing.h"

static int latex_num (FILE* fp, tree_node_t* node);
static int latex_var (FILE* fp, tree_node_t* node);
static int latex_func (FILE* fp, tree_node_t* node);
static int latex_op (FILE* fp, tree_node_t* node);
static void smart_double_print(FILE* fp, double x);


int pr (FILE* fp, const void* ptr, int type);

int latex_tree (FILE* fp, tree_node_t* node)
{
    //tree_graph_dump(node, pr);
    switch (node_get_type(node))
    {
    case NUM:
        latex_num(fp, node);
        break;
    case VAR:
        latex_var(fp, node);
        break;
    case FUNC:
        latex_func(fp, node);
        break;
    case OP:
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
    smart_double_print(fp, val);
    return 0;
}

static int latex_var (FILE* fp, tree_node_t* node)
{
    int val;
    node_get_val(node, &val);
    fprintf(fp, "%c", val);
    return 0;
}

static int latex_func (FILE* fp, tree_node_t* node)
{
    int val;
    node_get_val(node, &val);
    for (size_t i = 0; i < sizeof(FUNCS)/sizeof(FUNCS[0]); i++)
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
    int val;
    node_get_val(node, &val);
    switch (val)
    {
    case ADD:
        latex_tree(fp, node_to_left(node));
        fprintf(fp, "+");
        latex_tree(fp, node_to_right(node));
        return 0;
    case SUB:
        latex_tree(fp, node_to_left(node));
        fprintf(fp, "-");
        latex_tree(fp, node_to_right(node));
        return 0;
    case MULT:
        if (LEFT_IS_OP(node))
        {
            int val = 0;
            node_get_val(node_to_left(node), &val);
            if (val == SUB || val == ADD)
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

        //if (RIGHT_IS_NUM(node))
            fprintf(fp, "*");

        if (RIGHT_IS_OP(node))
        {
            int val = 0;
            node_get_val(node_to_right(node), &val);
            if (val == SUB || val == ADD)
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
    case DIV:
        fprintf(fp, "\\frac{");
        latex_tree(fp, node_to_left(node));
        fprintf(fp, "}{");
        latex_tree(fp, node_to_right(node));
        fprintf(fp, "}");
        return 0;
    case POW:
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

static void smart_double_print(FILE* fp, double x)
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

        fprintf(fp, ".%*d", sign_count, (int)(frac_part*pow(10, sign_count)));
    }
}
