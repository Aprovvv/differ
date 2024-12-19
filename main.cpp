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

int pr (FILE* fp, const void* ptr, int type);

int main()
{
    lexarr lexem_array = init_lexem_array("eq.txt");
    tree_node_t* root = get_grammar(lexem_array.ptr);
    tree_graph_dump(root, pr);
    free(lexem_array.ptr);

    FILE* input = fopen("txt.txt", "r");
    if (root == NULL)
        return EXIT_FAILURE;

    tree_graph_dump(root, pr);

    tree_node_t* droot = diff_and_tex(root);

    branch_delete(root);
    branch_delete(droot);
    fclose(input);
}

int pr (FILE* fp, const void* ptr, int type)
{
    switch (type)
    {
    case ARG_TYPE_NUM:
        return fprintf(fp, "%f", (*(double*)ptr));
    case ARG_TYPE_VAR:
        return fprintf(fp, "%c", *((const long*)ptr));
    case ARG_TYPE_OP:
        for (size_t i = 0; i < OPS_COUNT; i++)
            if (OPS[i].arg_code == *((const long*)ptr))
                return fprintf(fp, "%s", OPS[i].str);
        fprintf(stderr, "%p\n", ptr);
        assert(0);
        break;
    case ARG_TYPE_FUNC:
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
