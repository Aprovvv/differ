#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "tree/tree.h"

int is_num (const char* str);
char next_nonspace (FILE* fp);
char getc_until (FILE* fp, const char* str);
tree_node_t* tree_from_file (FILE* input);
int pr (FILE* fp, const void* ptr, int type);

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
    LN
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
    branch_delete(root);
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
        return fprintf(fp, "%c", *((const char*)ptr));
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
    int i = 0;
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
        node = new_node(&num, sizeof(num), NUM);
        node_add_left(node, node_temp_ptr);
    }

    if (node == NULL)
    {
        for (i = 0; i < sizeof(VARS)/sizeof(VARS[0]); i++)
        {
            if (strcmp(VARS[i].str, arg) == 0)
            {
                int code = VARS[i].arg_code;
                node = new_node(&code, sizeof(code), VAR);
                node_add_left(node, node_temp_ptr);
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
                node = new_node(&code, sizeof(code), OP);
                node_add_left(node, node_temp_ptr);
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
                node = new_node(&code, sizeof(code), FUNC);
                node_add_left(node, node_temp_ptr);
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

int is_num (const char* str)
{
    int i = 0;
    while(str[i] != 0)
        if (!isdigit(str[i++]))
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
