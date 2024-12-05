#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include "tree/tree.h"
#include "tree_transforms.h"
#include "latexing.h"

//TODO: texlive

int is_num (const char* str);
char next_nonspace (FILE* fp);
char getc_until (FILE* fp, const char* str);
tree_node_t* tree_from_file (FILE* input);
int pr (FILE* fp, const void* ptr, int type);

int main()
{
    FILE* input = fopen("txt.txt", "r");
    tree_node_t* root = tree_from_file(input);
    if (root == NULL)
        return EXIT_FAILURE;

    tree_graph_dump(root, pr);

    tree_node_t* droot = diff(root);
    tree_graph_dump(droot, pr);

    root = simplify(root);
    tree_graph_dump(root, pr);

    droot = simplify(droot);
    tree_graph_dump(droot, pr);

    FILE* latex_root = fopen("root.txt", "w");
    FILE* latex_droot = fopen("droot.txt", "w");
    latex_tree(latex_root, root);
    latex_tree(latex_droot, droot);
    diff_and_tex(root);
    fclose(latex_root);
    fclose(latex_droot);

    branch_delete(root);
    branch_delete(droot);
    fclose(input);
}

int pr (FILE* fp, const void* ptr, int type)
{
    switch (type)
    {
    case NUM:
        return fprintf(fp, "%f", (*(double*)ptr));
    case VAR:
        return fprintf(fp, "%c", *((const int*)ptr));
    case OP:
        for (size_t i = 0; i < sizeof(OPS)/sizeof(OPS[0]); i++)
            if (OPS[i].arg_code == *((const int*)ptr))
                return fprintf(fp, "%s", OPS[i].str);
        assert(0);
        break;
    case FUNC:
        for (size_t i = 0; i < sizeof(FUNCS)/sizeof(FUNCS[0]); i++)
            if (FUNCS[i].arg_code == *((const int*)ptr))
                return fprintf(fp, "%s", FUNCS[i].str);
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
    //fprintf(stderr, "start ch = %c\n", ch);
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
                int code = FUNCS[i].arg_code;
                node = new_node(&code, sizeof(code), FUNC, node_temp_ptr, NULL);
                break;
            }
        }
    }
    if (ch == '(')
    {
        node_temp_ptr = tree_from_file(fp);
        node_add_right(node, node_temp_ptr);
        getc(fp);
    }
    //fprintf(stderr, "arg = %s; ch = %c\n", arg, ch);
    return node;
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
