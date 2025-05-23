#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include "tree/tree.h"
#include "tree_transforms.h"
#include "latexing.h"

static tree_node_t* diff_num     (FILE* fp, tree_node_t* f);
static tree_node_t* diff_var     (FILE* fp, tree_node_t* f);
static tree_node_t* diff_add_sub (FILE* fp, tree_node_t* f);
static tree_node_t* diff_mult    (FILE* fp, tree_node_t* f);
static tree_node_t* diff_div     (FILE* fp, tree_node_t* f);
static tree_node_t* diff_pow     (FILE* fp, tree_node_t* f);
static tree_node_t* diff_sin     (FILE* fp, tree_node_t* f);
static tree_node_t* diff_cos     (FILE* fp, tree_node_t* f);
static tree_node_t* diff_tg      (FILE* fp, tree_node_t* f);
static tree_node_t* diff_ctg     (FILE* fp, tree_node_t* f);
static tree_node_t* diff_ln      (FILE* fp, tree_node_t* f);
static tree_node_t* diff_exp     (FILE* fp, tree_node_t* f);
static tree_node_t* diff_sqrt    (FILE* fp, tree_node_t* f);

extern const struct ARG VARS[] = {
    {"x", 'x', diff_var},
};

extern const struct ARG OPS[] = {
    {"+", OP_CODE_ADD,      diff_add_sub},
    {"-", OP_CODE_SUB,      diff_add_sub},
    {"*", OP_CODE_MULT,     diff_mult},
    {"/", OP_CODE_DIV,      diff_div},
    {"^", OP_CODE_POW,      diff_pow},
    {"(", OP_CODE_LBRACKET, NULL},
    {")", OP_CODE_RBRACKET, NULL},
    {"\n", OP_CODE_ENDOFEQ,  NULL}
};

extern const struct ARG FUNCS[] = {
    {"sin", FUNC_CODE_SIN,  diff_sin},
    {"cos", FUNC_CODE_COS,  diff_cos},
    {"tg",  FUNC_CODE_TG,   diff_tg},
    {"ctg", FUNC_CODE_CTG,  diff_ctg},
    {"ln",  FUNC_CODE_LN,   diff_ln},
    {"exp", FUNC_CODE_EXP,  diff_exp},
    {"sqrt",FUNC_CODE_SQRT, diff_sqrt}
};

extern const size_t VARS_COUNT = sizeof(VARS)/sizeof(VARS[0]);
extern const size_t OPS_COUNT = sizeof(OPS)/sizeof(OPS[0]);
extern const size_t FUNCS_COUNT = sizeof(FUNCS)/sizeof(FUNCS[0]);

static int is_int(double x);
static int dblcmp (double a, double b);
static int NOD(int a, int b);

tree_node_t* diff_and_tex (tree_node_t* root)
{
    const char* filename = "latex/file.tex";
    FILE* texfile = create_texfile(filename);
    if (!texfile)
        return NULL;

    fprintf(texfile, "\n\nДана функция f(x):\n\\begin{center}\n");
    fprintf(texfile, "$f(x) = ");
    latex_tree(texfile, root);
    fprintf(texfile, "$\n\\end{center}\n\n");
    fprintf(texfile, "Найдем ее производную f'(x):\n\n");
    tree_node_t* droot = diff(texfile, root);
    fprintf(texfile, "\n\\begin{center}\n$f'(x) = ");
    latex_tree(texfile, droot);
    fprintf(texfile, "$\n\\end{center}\n\n");
    fprintf(texfile, "Ну и если чуть-чуть упростить:\n\\begin{center}\n");
    fprintf(texfile, "$f'(x) = ");

    droot = simplify(droot);
    latex_tree(texfile, droot);
    fprintf(texfile, "$\n\\end{center}\n\n");

    fprintf(texfile, "\\end{document}\n");
    fclose(texfile);
    system("pdflatex -output-directory=latex/ latex/file.tex");

    return droot;
}

tree_node_t* diff (FILE* fp, tree_node_t* f)
{
    if (node_get_type(f) == ARG_TYPE_NUM)
        return diff_num(fp, f);
    if (node_get_type(f) == ARG_TYPE_VAR)
        return diff_var(fp, f);
    if (node_get_type(f) == ARG_TYPE_OP)
    {
        long val = 0;
        node_get_val(f, &val);
        for (size_t i = 0; i < sizeof(OPS)/sizeof(OPS[0]); i++)
            if (OPS[i].arg_code == val)
                return OPS[i].diff_func(fp, f);

        assert(0 && "UNDEFINED OPERATOR");
    }
    //TODO: остальные функции
    if (node_get_type(f) == ARG_TYPE_FUNC)
    {
        long val = 0;
        node_get_val(f, &val);
        for (size_t i = 0; i < sizeof(FUNCS)/sizeof(FUNCS[0]); i++)
            if (FUNCS[i].arg_code == val)
                return FUNCS[i].diff_func(fp, f);

        assert(0 && "UNDEFINED FUNCTION");
    }
    return NULL;
}

static tree_node_t* diff_num (FILE* fp, tree_node_t* f)
{
    if (fp)
    {
        fprintf(fp, "Производная от константы это, "
                    "слава Знамке, ноль:\n\n");
        fprintf(fp, "\\begin{center}\n$(");
        latex_tree(fp, f);
        fprintf(fp, ")' = 0$\n\\end{center}\n\n");
    }
    double val = 0;
    return new_node(&val, sizeof(val), ARG_TYPE_NUM, NULL, NULL);
}

static tree_node_t* diff_var (FILE* fp, tree_node_t* f)
{
    if (fp)
    {
        fprintf(fp, "Производная от х это, "
                    "хвала Петровичу, единица:\n\n");
        fprintf(fp, "\\begin{center}\n$(");
        latex_tree(fp, f);
        fprintf(fp, ")' = 1$\n\\end{center}\n\n");
    }
    double val = 1;
    return new_node(&val, sizeof(val), ARG_TYPE_NUM, NULL, NULL);
}

static tree_node_t* diff_add_sub (FILE* fp, tree_node_t* f)
{
    if (fp)
    {
        fprintf(fp, "Производная от суммы есть сумма производных "
                    "(эх, всегда бы так...):\n\n");
        fprintf(fp, "\\begin{center}\n$(");
        latex_tree(fp, f);
        fprintf(fp, ")' = (");
        latex_tree(fp, node_to_left(f));
        fprintf(fp, ")' ");
        long val = 0;
        node_get_val(f, &val);
        if (val == OP_CODE_MULT)
            fprintf(fp, "+");
        else
            fprintf(fp, "-");
        fprintf(fp, " (");
        latex_tree(fp, node_to_right(f));
        fprintf(fp, ")'$\n\\end{center}\n\n");
    }
    long val = 0;
    node_get_val(f, &val);
    return new_node(&val, sizeof(val), ARG_TYPE_OP,
                    diff(fp, node_to_left(f)),
                    diff(fp, node_to_right(f)));
}

static tree_node_t* diff_mult (FILE* fp, tree_node_t* f)
{
    if (fp)
    {
        fprintf(fp, "Производная произведения "
                    "это вооть такая штука:\n\n");
        fprintf(fp, "\\begin{center}\n$(");
        latex_tree(fp, f);
        fprintf(fp, ")' = (");
        latex_tree(fp, node_to_left(f));
        fprintf(fp, ")' \\cdot (");
        latex_tree(fp, node_to_right(f));
        fprintf(fp, ") + (");
        latex_tree(fp, node_to_right(f));
        fprintf(fp, ")' \\cdot (");
        latex_tree(fp, node_to_left(f));
        fprintf(fp, ")'$\n\\end{center}\n\n");
    }
    long op = OP_CODE_MULT;
    tree_node_t* left_mult =
        new_node(&op, sizeof(op), ARG_TYPE_OP,
                    diff(fp, node_to_left(f)),
                         branch_copy(node_to_right(f)));
    tree_node_t* right_mult =
        new_node(&op, sizeof(op), ARG_TYPE_OP,
                    branch_copy(node_to_left(f)),
                                diff(fp, node_to_right(f)));
    op = OP_CODE_ADD;
    return new_node(&op, sizeof(op), ARG_TYPE_OP,
                left_mult, right_mult);
}

static tree_node_t* diff_div (FILE* fp, tree_node_t* f)
{
    if (fp)
    {
        fprintf(fp, "Производная от отношения есть... эээ.. "
                    "ну короче ща все сами увидите:\n\n");
        fprintf(fp, "\\begin{center}\n$(");
        latex_tree(fp, f);
        fprintf(fp, ")' = \\frac{(");
        latex_tree(fp, node_to_left(f));
        fprintf(fp, ")' \\cdot (");
        latex_tree(fp, node_to_right(f));
        fprintf(fp, ") + (");
        latex_tree(fp, node_to_right(f));
        fprintf(fp, ")' \\cdot (");
        latex_tree(fp, node_to_left(f));
        fprintf(fp, ")}{(");
        latex_tree(fp, node_to_right(f));
        fprintf(fp, ")^2}$\n\\end{center}\n\n");
    }
    long mult = OP_CODE_MULT, sub = OP_CODE_SUB;
    long div = OP_CODE_DIV, pw = OP_CODE_POW;
    double two = 2;
    tree_node_t* numer =
        new_node(&sub, sizeof(sub), ARG_TYPE_OP,
                new_node(&mult, sizeof(mult), ARG_TYPE_OP,
                            diff(fp, node_to_left(f)),
                            branch_copy(node_to_right(f))),
                new_node(&mult, sizeof(mult), ARG_TYPE_OP,
                            branch_copy(node_to_left(f)),
                            diff(fp, node_to_right(f))));
    tree_node_t* denumer =
        new_node(&pw, sizeof(pw), ARG_TYPE_OP,
                branch_copy(node_to_right(f)),
                new_node(&two, sizeof(two), ARG_TYPE_NUM,
                        NULL, NULL));
    return new_node(&div, sizeof(div), ARG_TYPE_OP, numer, denumer);
}


static tree_node_t* diff_pow (FILE* fp, tree_node_t* f)
{
    simplify(f);
    if (RIGHT_IS_NUM(f))
    {
        long mult = OP_CODE_MULT, pw = OP_CODE_POW;
        double old_pow = 0;
        node_get_val(node_to_right(f), &old_pow);
        double new_pow = old_pow - 1;

        if (fp)
        {
            fprintf(fp, "Ну слава кпсс. Степенная функция. "
                        "Тут все понятно. Главное - "
                        "не забыть домножить на производную "
                        "того, что в скобках\n\n");
            fprintf(fp, "\\begin{center}\n$(");
            latex_tree(fp, f);
            fprintf(fp, ")' = ");
            smart_double_print(fp, old_pow);
            fprintf(fp, "(");
            latex_tree(fp, node_to_left(f));
            fprintf(fp, ")^{");
            smart_double_print(fp, new_pow);
            fprintf(fp, "} \\cdot (");
            latex_tree(fp, node_to_left(f));
            fprintf(fp, ")'$\n\\end{center}\n\n");
        }

        tree_node_t* result =
            new_node(&pw, sizeof(pw), ARG_TYPE_OP,
                     branch_copy(node_to_left(f)),
                     new_node(&new_pow, sizeof(new_pow),
                              ARG_TYPE_NUM, NULL, NULL));
        result =
            new_node(&mult, sizeof(mult), ARG_TYPE_OP,
                     new_node(&old_pow, sizeof(old_pow),
                              ARG_TYPE_NUM, NULL, NULL),
                     result);
        return new_node(&mult, sizeof(mult), ARG_TYPE_OP,
                        result,
                        diff(fp, node_to_left(f)));
    }

    //общий случай:
    //(f(x)^g(x))' = ( (g(x)/f(x)) * f'(x) + ln(f(x)) * g'(x) ) * f(x)^g(x)
    //                      first_term            second_term       f
    if (fp)
    {
        fprintf(fp, "Хотели степенную функцию? Пососите! "
                    "Так как и основание и показатель - функции, "
                    "придется все делать по-честному:\n\n");
        fprintf(fp, "\\begin{center}\n$(");
        latex_tree(fp, f);
        fprintf(fp, ")' = (\\frac{");
        latex_tree(fp, node_to_right(f));
        fprintf(fp, "}{");
        latex_tree(fp, node_to_left(f));
        fprintf(fp, "} \\cdot (");
        latex_tree(fp, node_to_left(f));
        fprintf(fp, ")'+\\ln{(");
        latex_tree(fp, node_to_left(f));
        fprintf(fp, ")} \\cdot (");
        latex_tree(fp, node_to_right(f));
        fprintf(fp, ")') \\cdot (");
        latex_tree(fp, f);
        fprintf(fp, ")'$\n\\end{center}\n\n");
    }
    long div = OP_CODE_DIV, mult = OP_CODE_MULT;
    long ln = FUNC_CODE_LN, sum = OP_CODE_ADD;
    tree_node_t* first_term =
        new_node(&mult, sizeof(mult), ARG_TYPE_OP,
                 new_node(&div, sizeof(div), ARG_TYPE_OP,
                          branch_copy(node_to_right(f)),
                          branch_copy(node_to_left(f))),
                 diff(fp, node_to_left(f)));
    tree_node_t* second_term =
        new_node(&mult, sizeof(mult), ARG_TYPE_OP,
                 new_node(&ln, sizeof(ln), ARG_TYPE_FUNC,
                          NULL,
                          branch_copy(node_to_left(f))),
                 diff(fp, node_to_right(f)));

    return new_node(&mult, sizeof(mult), ARG_TYPE_OP,
                    new_node(&sum, sizeof(sum), ARG_TYPE_OP,
                             first_term,
                             second_term),
                    branch_copy(f));
}

static tree_node_t* diff_sin (FILE* fp, tree_node_t* f)
{
    long cos = FUNC_CODE_COS, mult = OP_CODE_MULT;
    if (fp)
    {
        fprintf(fp, "Производная синуса это косинус. Главное - не забыть "
                     "про производную того, что в скобках\n\n");
        fprintf(fp, "\\begin{center}\n$(");
        latex_tree(fp, f);
        fprintf(fp, ")' = \\cos{(");
        latex_tree(fp, node_to_right(f));
        fprintf(fp, ")} \\cdot (");
        latex_tree(fp, node_to_right(f));
        fprintf(fp, ")'$\n\\end{center}\n\n");
    }
    tree_node_t* cos_node =
        new_node(&cos, sizeof(cos), ARG_TYPE_FUNC,
                    NULL, branch_copy(node_to_right(f)));
    return new_node(&mult, sizeof(mult), ARG_TYPE_OP,
                    cos_node, diff(fp, node_to_right(f)));
}

static tree_node_t* diff_cos (FILE* fp, tree_node_t* f)
{
    long sin = FUNC_CODE_SIN, mult = OP_CODE_MULT;
    double m_1 = -1;

    if (fp)
    {
        fprintf(fp, "Производная косинуса это минус синус. Главное - "
                    "не забыть про производную того, что в скобках\n\n");
        fprintf(fp, "\\begin{center}\n$(");
        latex_tree(fp, f);
        fprintf(fp, ")' = -\\sin{(");
        latex_tree(fp, node_to_right(f));
        fprintf(fp, ")} \\cdot (");
        latex_tree(fp, node_to_right(f));
        fprintf(fp, ")'$\n\\end{center}\n\n");
    }

    tree_node_t* minus_1 =
        new_node(&m_1, sizeof(m_1), ARG_TYPE_NUM,
                    NULL, NULL);
    tree_node_t* sin_node =
        new_node(&sin, sizeof(sin), ARG_TYPE_FUNC,
                    NULL, branch_copy(node_to_right(f)));
    tree_node_t* minus_sin =
        new_node(&mult, sizeof(mult), ARG_TYPE_OP, minus_1, sin_node);
    return new_node(&mult, sizeof(mult), ARG_TYPE_OP,
                    minus_sin, diff(fp, node_to_right(f)));
}

static tree_node_t* diff_tg (FILE* fp, tree_node_t* f)
{
    if (fp)
    {
        fprintf(fp, "Производная тангенса - $\\frac{1}{\\cos^2}$."
                    "Доказательство предоставляется читателю "
                    "в качестве несложного упражения.\n\n");
        fprintf(fp, "\\begin{center}\n$(");
        latex_tree(fp, f);
        fprintf(fp, ")' = \\frac{1}{\\cos^2{(");
        latex_tree(fp, node_to_right(f));
        fprintf(fp, ")}} \\ cdot (");
        latex_tree(fp, node_to_right(f));
        fprintf(fp, ")'$\n\\end{center}\n\n");
    }
    long div = OP_CODE_DIV, pw = OP_CODE_POW, cos = FUNC_CODE_COS;
    double two = 2;
    return new_node(&div, sizeof(div), ARG_TYPE_OP,
                    diff(fp, node_to_right(f)),
                    new_node(&pw, sizeof(pw), ARG_TYPE_OP,
                             new_node(&cos, sizeof(cos), ARG_TYPE_FUNC,
                                      NULL, branch_copy(node_to_right(f))),
                             new_node(&two, sizeof(two), ARG_TYPE_NUM,
                                      NULL, NULL)));

}

static tree_node_t* diff_ctg (FILE* fp, tree_node_t* f)
{
    long div = OP_CODE_DIV, pw = OP_CODE_POW;
    long sin = FUNC_CODE_SIN, mult = OP_CODE_MULT;
    if (fp)
    {
        fprintf(fp, "Производная тангенса - $-\\frac{1}{\\sin^2}$."
                    "Доказательство предоставляется читателю "
                    "в качестве несложного упражения.\n\n");
        fprintf(fp, "\\begin{center}\n$(");
        latex_tree(fp, f);
        fprintf(fp, ")' = -\\frac{1}{\\sin^2{(");
        latex_tree(fp, node_to_right(f));
        fprintf(fp, ")}} \\cdot (");
        latex_tree(fp, node_to_right(f));
        fprintf(fp, ")'$\n\\end{center}\n\n");
    }
    double two = 2, minus_1 = -1;
    return new_node(&div, sizeof(div), ARG_TYPE_OP,
                    new_node(&mult, sizeof(mult), ARG_TYPE_OP,
                             new_node(&minus_1, sizeof(minus_1),
                                      ARG_TYPE_NUM, NULL, NULL),
                             diff(fp, node_to_right(f))),
                    new_node(&pw, sizeof(pw), ARG_TYPE_OP,
                             new_node(&sin, sizeof(sin), ARG_TYPE_FUNC,
                                      NULL, branch_copy(node_to_right(f))),
                             new_node(&two, sizeof(two),
                                      ARG_TYPE_NUM, NULL, NULL)));

}

static tree_node_t* diff_ln (FILE* fp, tree_node_t* f)
{
    if (fp)
    {
        fprintf(fp, "Я люблю производную логарифма!\n\n");
        fprintf(fp, "\\begin{center}\n$(");
        latex_tree(fp, f);
        fprintf(fp, ")' = -\\frac{(");
        latex_tree(fp, node_to_right(f));
        fprintf(fp, ")'}{");
        latex_tree(fp, node_to_right(f));
        fprintf(fp, "}$\n\\end{center}\n\n");
    }
    long div = OP_CODE_DIV;
    return new_node(&div, sizeof(div), ARG_TYPE_OP,
                    diff(fp, node_to_right(f)),
                    branch_copy(node_to_right(f)));
}

static tree_node_t* diff_exp (FILE* fp, tree_node_t* f)
{
    if (fp)
    {
        fprintf(fp, "Экспонента. Самая приятная производная:)\n\n");
        fprintf(fp, "\\begin{center}\n$(");
        latex_tree(fp, f);
        fprintf(fp, ")' = (");
        latex_tree(fp, node_to_right(f));
        fprintf(fp, ")' \\cdot ");
        latex_tree(fp, f);
        fprintf(fp, "$\n\\end{center}\n\n");
    }
    long mult = OP_CODE_MULT;
    return new_node(&mult, sizeof(mult), ARG_TYPE_OP,
                    diff(fp, node_to_right(f)),
                    branch_copy(f));
}

static tree_node_t* diff_sqrt (FILE* fp, tree_node_t* f)
{
    if (fp)
    {
        fprintf(fp, "Возьмем производную квадратного корня (простите, у меня 0"
                    "закончилась фантазия на \"смешные\" фразочки):\n\n");
        fprintf(fp, "\\begin{center}\n$(");
        latex_tree(fp, f);
        fprintf(fp, ")' = \\frac{(");
        latex_tree(fp, node_to_right(f));
        fprintf(fp, ")'}{2 \\cdot ");
        latex_tree(fp, f);
        fprintf(fp, "}$\n\\end{center}\n\n");
    }
    long div = OP_CODE_DIV, mult = OP_CODE_MULT;
    double two = 2;
    return new_node(&div, sizeof(div), ARG_TYPE_OP,
                    diff(fp, node_to_right(f)),
                    new_node(&mult, sizeof(mult), ARG_TYPE_OP,
                             new_node(&two, sizeof(two),
                                      ARG_TYPE_NUM, NULL, NULL),
                             branch_copy(f)));
}


tree_node_t* simplify (tree_node_t* node)
{
    tree_node_t* result = node;

    if (node_to_left(node))
        node_add_left(node, simplify(node_to_left(node)));
    if (node_to_right(node))
        node_add_right(node, simplify(node_to_right(node)));


    if (node_get_type(result) == ARG_TYPE_OP &&
        LEFT_IS_NUM(result) && RIGHT_IS_NUM(result))
    {
        result = calc_node(result);
    }

    if (node_get_type(result) == ARG_TYPE_OP &&
        (LEFT_IS_NUM(result) || RIGHT_IS_NUM(result)))
    {
        result = delete_trivials(result);
    }

    return result;
}

tree_node_t* delete_trivials (tree_node_t* node)
{
    long val = 0;
    double val_left = 666, val_right = 666;

    node_get_val(node, &val);
    if (LEFT_IS_NUM(node))
        node_get_val(node_to_left(node), &val_left);
    if (RIGHT_IS_NUM(node))
        node_get_val(node_to_right(node), &val_right);
    tree_node_t* result = NULL;

    switch(val)
    {
    case OP_CODE_ADD:
    //TODO: здесь копируется и возвращается вся ненулевая ветка.
    //было бы лучше удалить всего два узла, но нет нужной функции
        if (dblcmp(val_left, 0) == 0)
        {
            result = branch_copy(node_to_right(node));
            branch_delete(node);
            return result;
        }
    case OP_CODE_SUB:
        if (dblcmp(val_right, 0) == 0)
        {
            result = branch_copy(node_to_left(node));
            branch_delete(node);
            return result;
        }
        break;
    case OP_CODE_MULT:
        if (dblcmp(val_right, 0) == 0)
        {
            double zero = 0;
            branch_delete(node);
            return new_node(&zero, sizeof(zero),
                            ARG_TYPE_NUM, NULL, NULL);
        }
        if (dblcmp(val_left, 1) == 0)
        {
            result = branch_copy(node_to_right(node));
            branch_delete(node);
            return result;
        }
    case OP_CODE_DIV:
        if (dblcmp(val_left, 0) == 0)
        {
            double zero = 0;
            branch_delete(node);
            return new_node(&zero, sizeof(zero),
                            ARG_TYPE_NUM, NULL, NULL);
        }
        if (dblcmp(val_right, 1) == 0)
        {
            result = branch_copy(node_to_left(node));
            branch_delete(node);
            return result;
        }
        break;
    case OP_CODE_POW:
        if (dblcmp(val_right, 1) == 0)
        {
            result = branch_copy(node_to_left(node));
            branch_delete(node);
            return result;
        }
        if (dblcmp(val_right, 0) == 0)
        {
            double one = 1;
            branch_delete(node);
            return new_node(&one, sizeof(one),
                            ARG_TYPE_NUM, NULL, NULL);
        }
        break;
    default:
        return NULL;
    }
    return node;
}

tree_node_t* calc_node (tree_node_t* node)
{
    long val = 0;
    double val_left = 0, val_right = 0, result = 0;

    node_get_val(node, &val);
    node_get_val(node_to_left(node), &val_left);
    node_get_val(node_to_right(node), &val_right);

    if (val == OP_CODE_POW)
        if (!is_int(val_right))
            return node;
    switch(val)
    {
        case OP_CODE_ADD:
            result = val_left + val_right;
            break;
        case OP_CODE_SUB:
            result = val_left - val_right;
            break;
        case OP_CODE_MULT:
            result = val_left * val_right;
            break;
        case OP_CODE_DIV:
            if (dblcmp(val_right, 0) == 0)
                return NULL;
            result = val_left / val_right;
            break;
        case OP_CODE_POW:
            result = pow(val_left, val_right);
            break;
        default:
            return NULL;
    }
    branch_delete(node);
    return new_node(&result, sizeof(result), ARG_TYPE_NUM, NULL, NULL);
}

static int is_int (double x)
{
    double temp = 0;
    if (abs(modf(x, &temp)) < EPS)
        return 1;
    return 0;
}

static int dblcmp (double a, double b)
{
    if (a - b > EPS)
        return 1;
    if (b - a > EPS)
        return -1;
    return 0;
}

static int NOD(int a, int b)
{
    while(a > 0 && b > 0)
        if(a > b)
            a %= b;
        else
            b %= a;
    return a + b;
}
