#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include "tree/tree.h"
#include "tree_transforms.h"
#include "latexing.h"

static int is_int(double x);

void diff_and_tex (tree_node_t* root)
{
    char* filename = "latex/file.tex";
    FILE* texfile = create_texfile(filename);
    fprintf(texfile, "Дана функция f(x):\n");
    fprintf(texfile, "$$f(x) = ");
    latex_tree(texfile, root);
    fprintf(texfile, "$$\n");
    fprintf(texfile, "Её производная f'(x):\n");
    fprintf(texfile, "$$f'(x) = ");
    tree_node_t* droot = diff(root);
    latex_tree(texfile, droot);
    fprintf(texfile, "$$\n");
    fprintf(texfile, "Но мы ж не долбоебы, мы не будем в таком виде осталять\n");
    fprintf(texfile, "$$f'(x) = ");
    droot = simplify(droot);
    latex_tree(texfile, droot);
    fprintf(texfile, "$$\n");

    fprintf(texfile, "\\end{document}\n");
    fclose(texfile);
    branch_delete(droot);
    system("pdflatex -output-directory=latex/ latex/file.tex");
}

tree_node_t* diff (tree_node_t* f)
{
    if (node_get_type(f) == NUM)
        return diff_num(f);
    if (node_get_type(f) == VAR)
        return diff_var(f);
    if (node_get_type(f) == OP)
    {
        int val = 0;
        node_get_val(f, &val);
        for (size_t i = 0; i < sizeof(OPS)/sizeof(OPS[0]); i++)
            if (OPS[i].arg_code == val)
                return OPS[i].diff_func(f);

        assert(0 && "UNDEFINED OPERATOR");
    }
    //TODO: остальные функции
    if (node_get_type(f) == FUNC)
    {
        int val = 0;
        node_get_val(f, &val);
        for (size_t i = 0; i < sizeof(FUNCS)/sizeof(FUNCS[0]); i++)
            if (FUNCS[i].arg_code == val)
                return FUNCS[i].diff_func(f);

        assert(0 && "UNDEFINED FUNCTION");
    }
    return NULL;
}

tree_node_t* diff_num (tree_node_t* f)
{
    double val = 0;
    return new_node(&val, sizeof(val), NUM, NULL, NULL);
}

tree_node_t* diff_var (tree_node_t* f)
{
    double val = 1;
    return new_node(&val, sizeof(val), NUM, NULL, NULL);
}

tree_node_t* diff_add_sub (tree_node_t* f)
{
    int val = 0;
    node_get_val(f, &val);
    return new_node(&val, sizeof(val), OP,
                    diff(node_to_left(f)), diff(node_to_right(f)));
}

tree_node_t* diff_mult (tree_node_t* f)
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

tree_node_t* diff_div (tree_node_t* f)
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


tree_node_t* diff_pow (tree_node_t* f)
//FIXME: общий случай
{
    if (RIGHT_IS_NUM(f))
    {
        int mult = MULT, pw = POW;
        double old_pow = 0;
        node_get_val(node_to_right(f), &old_pow);
        double new_pow = old_pow - 1;
        tree_node_t* result =
            new_node(&pw, sizeof(pw), OP,
                     branch_copy(node_to_left(f)),
                     new_node(&new_pow, sizeof(new_pow), NUM, NULL, NULL));
        result =
            new_node(&mult, sizeof(mult), OP,
                     new_node(&old_pow, sizeof(old_pow), NUM, NULL, NULL),
                     result);
        return new_node(&mult, sizeof(mult), OP,
                        result,
                        diff(node_to_left(f)));
    }

    //общий случай:
    //(f(x)^g(x))' = ( (g(x)/f(x)) * f'(x) + ln(f(x)) * g'(x) ) * f(x)^g(x)
    //                      first_term            second_term       f
    int div = DIV, mult = MULT, ln = LN, sum = ADD;
    tree_node_t* first_term =
        new_node(&mult, sizeof(mult), OP,
                 new_node(&div, sizeof(div), OP,
                          branch_copy(node_to_right(f)),
                          branch_copy(node_to_left(f))),
                 diff(node_to_left(f)));
    tree_node_t* second_term =
        new_node(&mult, sizeof(mult), OP,
                 new_node(&ln, sizeof(ln), FUNC,
                          NULL,
                          branch_copy(node_to_left(f))),
                 diff(node_to_right(f)));

    return new_node(&mult, sizeof(mult), OP,
                    new_node(&sum, sizeof(sum), OP,
                             first_term,
                             second_term),
                    branch_copy(f));
}

tree_node_t* diff_sin (tree_node_t* f)
{
    int cos = COS, mult = MULT;

    tree_node_t* cos_node =
        new_node(&cos, sizeof(cos), FUNC,
                    NULL, branch_copy(node_to_right(f)));
    return new_node(&mult, sizeof(mult), OP,
                    cos_node, diff(node_to_right(f)));
}

tree_node_t* diff_cos (tree_node_t* f)
{
    int sin = SIN, mult = MULT;
    double m_1 = -1;

    tree_node_t* minus_1 =
        new_node(&m_1, sizeof(m_1), NUM,
                    NULL, NULL);
    tree_node_t* sin_node =
        new_node(&sin, sizeof(sin), FUNC,
                    NULL, branch_copy(node_to_right(f)));
    tree_node_t* minus_sin =
        new_node(&mult, sizeof(mult), OP, minus_1, sin_node);
    return new_node(&mult, sizeof(mult), OP,
                    minus_sin, diff(node_to_right(f)));
}

tree_node_t* diff_tg (tree_node_t* f)
{
    int div = DIV, pw = POW, cos = COS;
    double two = 2;
    return new_node(&div, sizeof(div), OP,
                    diff(node_to_right(f)),
                    new_node(&pw, sizeof(pw), OP,
                             new_node(&cos, sizeof(cos), FUNC,
                                      NULL, branch_copy(node_to_right(f))),
                             new_node(&two, sizeof(two), NUM,
                                      NULL, NULL)));

}

tree_node_t* diff_ctg (tree_node_t* f)
{
    int div = DIV, pw = POW, sin = SIN, mult = MULT;
    double two = 2, minus_1 = -1;
    return new_node(&div, sizeof(div), OP,
                    new_node(&mult, sizeof(mult), OP,
                             new_node(&minus_1, sizeof(minus_1), NUM, NULL, NULL),
                             diff(node_to_right(f))),
                    new_node(&pw, sizeof(pw), OP,
                             new_node(&sin, sizeof(sin), FUNC,
                                      NULL, branch_copy(node_to_right(f))),
                             new_node(&two, sizeof(two), NUM,
                                      NULL, NULL)));

}

tree_node_t* diff_ln (tree_node_t* f)
{
    int div = DIV;
    return new_node(&div, sizeof(div), OP,
                    diff(node_to_right(f)),
                    branch_copy(node_to_right(f)));
}

tree_node_t* diff_exp (tree_node_t* f)
{
    int mult = MULT;
    return new_node(&mult, sizeof(mult), OP,
                    diff(node_to_right(f)),
                    branch_copy(f));
}

tree_node_t* diff_sqrt (tree_node_t* f)
{
    int div = DIV, mult = MULT;
    double two = 2;
    return new_node(&div, sizeof(div), OP,
                    diff(node_to_right(f)),
                    new_node(&mult, sizeof(mult), OP,
                             new_node(&two, sizeof(two), NUM, NULL, NULL),
                             branch_copy(f)));
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
        //fprintf(stderr, "calcing node %p\n", node);
        result = calc_node(result);
    }

    if (node_get_type(result) == OP && (LEFT_IS_NUM(result) || RIGHT_IS_NUM(result)))
    {
        //fprintf(stderr, "deleting node %p, \n", node);
        result = delete_trivials(result);
    }

    //tree_graph_dump(node, pr);

    return result;
}

tree_node_t* delete_trivials (tree_node_t* node)
{
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
            //fprintf(stderr, "ASD;LFKJAKLW; NHGASDFKJ\n");
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
    int val = 0;
    double val_left = 0, val_right = 0, result = 0;

    node_get_val(node, &val);
    node_get_val(node_to_left(node), &val_left);
    node_get_val(node_to_right(node), &val_right);
    //fprintf(stderr, "&node = %p; type = %d; left %f; right %f\n", node, node_get_type(node), val_left, val_right);
    //fprintf(stderr, "is_int = %d\n", is_int(val_left));

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
            //FIXME бля поч не работает
            /*if (val_right == 0)
                return NULL;

            if (is_int(val_left) && is_int(val_right) && !is_int(val_left/val_right))
            {
                int NOD = nod((int)val_left, (int)val_right);
                val_left /= NOD;
                val_right /= NOD;
                node_set_val(node_to_left(node), &val_left);
                node_set_val(node_to_right(node), &val_right);
                return node;
            }*/

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

static int is_int(double x)
{
    double temp = 0;
    if (abs(modf(x, &temp)) < EPS)
        return 1;
    return 0;
}

