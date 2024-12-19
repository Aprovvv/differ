#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include "tree/tree.h"
#include "tree_transforms.h"
#include "latexing.h"
#include "lexer.h"
#include "recursive_reader.h"

int is_num (const char* str);
char next_nonspace (FILE* fp);
char getc_until (FILE* fp, const char* str);
tree_node_t* tree_from_file (FILE* input);
int pr (FILE* fp, const void* ptr, int type);

int main()
{
    lexarr lexem_array = init_lexem_array("eq.txt");
    //tree_graph_dump((tree_node_t*)lexem_array.ptr, pr);
    tree_node_t* root = get_grammar(lexem_array.ptr);
    tree_graph_dump(root, pr);
    branch_delete(root);
    free(lexem_array.ptr);

    /*FILE* input = fopen("txt.txt", "r");
    tree_node_t* root = tree_from_file(input);
    if (root == NULL)
        return EXIT_FAILURE;

    tree_graph_dump(root, pr);

    tree_node_t* droot = diff_and_tex(root);

    branch_delete(root);
    branch_delete(droot);
    fclose(input);*/
}

int pr (FILE* fp, const void* ptr, int type)
{
    switch (type)
    {
    case NUM:
        return fprintf(fp, "%f", (*(double*)ptr));
    case VAR:
        return fprintf(fp, "%c", *((const long*)ptr));
    case OP:
        for (size_t i = 0; i < OPS_COUNT; i++)
            if (OPS[i].arg_code == *((const long*)ptr))
                return fprintf(fp, "%s", OPS[i].str);
        fprintf(stderr, "%p\n", ptr);
        assert(0);
        break;
    case FUNC:
        for (size_t i = 0; i < FUNCS_COUNT; i++)
            if (FUNCS[i].arg_code == *((const long*)ptr))
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
        for (i = 0; i < VARS_COUNT; i++)
        {
            if (strcmp(VARS[i].str, arg) == 0)
            {
                long code = VARS[i].arg_code;
                node = new_node(&code, sizeof(code), VAR, node_temp_ptr, NULL);
                break;
            }
        }
    }

    if (node == NULL)
    {
        for (i = 0; i < OPS_COUNT; i++)
        {
            if (strcmp(OPS[i].str, arg) == 0)
            {
                long code = OPS[i].arg_code;
                node = new_node(&code, sizeof(code), OP, node_temp_ptr, NULL);
                break;
            }
        }
    }
    if (node == NULL)
    {
        for (i = 0; i < FUNCS_COUNT; i++)
        {
            if (strcmp(FUNCS[i].str, arg) == 0)
            {
                long code = FUNCS[i].arg_code;
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
    return node;
}

int is_num (const char* str)
{
    int i = 0;
    int is_minus = 0;
    while(str[i] != 0)
    {
        if (isdigit(str[i]) || str[i] == '.')
            i++;
        else
            if (str[i] == '-' && i == 0)
            {
                i++;
                is_minus = 1;
            }
            else
                return 0;
    }
    if (i == 1 && is_minus)
        return 0;
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
