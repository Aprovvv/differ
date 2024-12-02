#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include "tree/tree.h"
#include "tree_transforms.h"

static int is_int(double x);

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
                int cos = COS, mult = MULT;

                tree_node_t* cos_node =
                    new_node(&cos, sizeof(cos), FUNC,
                             NULL, branch_copy(node_to_right(f)));
                return new_node(&mult, sizeof(mult), OP,
                                cos_node, diff(node_to_right(f)));
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
                tree_node_t* minus_sin =
                    new_node(&mult, sizeof(mult), OP, minus_1, sin_node);
                return new_node(&mult, sizeof(mult), OP,
                                minus_sin, diff(node_to_right(f)));
            }
            //case LN:
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


static int is_int(double x)
{
    double temp = 0;
    if (abs(modf(x, &temp)) < EPS)
        return 1;
    return 0;
}
