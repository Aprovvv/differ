#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"

#define NODE_VAL_P(node) (void*)((char*)&node->right + sizeof(tree_node_t*))
#define NODE_SIZE(node) (sizeof(long) + 2*sizeof(tree_node_t*) + sizeof(size_t) + node->val_size)
//#define DETAILED_DUMP

struct tree_node_t
{
    int type;
    size_t val_size;
    tree_node_t* left;
    tree_node_t* right;
};

extern const size_t NODE_BASE_SIZE = sizeof(tree_node_t);

static void add_dot_node(FILE* fp, tree_node_t* node,
                         int(*print_func)(FILE* fp, const void* ptr, int type),
                         size_t code);

tree_node_t* new_node(void* val_ptr,
                      size_t val_size,
                      int type,
                      tree_node_t* left, tree_node_t* right)
{
    size_t node_size =
        sizeof(long) +
        2*sizeof(tree_node_t*) +
        sizeof(size_t) +
        val_size;
    tree_node_t* p = (tree_node_t*)calloc(node_size, 1);
    if (p)
    {
        memcpy(NODE_VAL_P(p), val_ptr, val_size);
        p->left = left;
        p->right = right;
        p->val_size = val_size;
        p->type = type;
    }
    return p;
}

size_t node_get_size(tree_node_t* node)
{
    return NODE_SIZE(node);
}


void node_set_size(tree_node_t* node, size_t size)
{
    node->val_size = size;
}

size_t node_get_val_size(tree_node_t* node)
{
    return node->val_size;
}

int node_get_type(tree_node_t* node)
{
    return node->type;
}

void node_set_type(tree_node_t* node, int type)
{
    node->type = type;
}

void node_get_val(tree_node_t* node, void* dest)
{
    memcpy(dest, NODE_VAL_P(node), node->val_size);
}

void node_set_val(tree_node_t* node, void* src)
{
    memcpy(NODE_VAL_P(node), src, node->val_size);
}

tree_node_t* node_to_left(tree_node_t* node)
{
    return node->left;
}

tree_node_t* node_to_right(tree_node_t* node)
{
    return node->right;
}

tree_node_t* node_add_left(tree_node_t* node, tree_node_t* next_node)
{
    return node->left = next_node;
}

tree_node_t* node_add_right(tree_node_t* node, tree_node_t* next_node)
{
    return node->right = next_node;
}

tree_node_t* branch_copy(tree_node_t* node)
{
    tree_node_t* new_left =
        node->left == NULL ? NULL : branch_copy(node->left);
    tree_node_t* new_right =
        node->right == NULL ? NULL : branch_copy(node->right);
    return new_node(NODE_VAL_P(node), node->val_size, node->type,
                    new_left, new_right);
}

void branch_delete(tree_node_t* node)
{
    if (node == NULL)
        return;
    if (node->left)
        branch_delete(node->left);
    if (node->right)
        branch_delete(node->right);
    //tree_graph_dump(node, pr);
    free(node);
}

void tree_print(FILE* fp, tree_node_t* node,
                int(*print_func)(FILE* fp, const void*, int type))
{
    if (node == NULL)
        return;
    fprintf(fp, "(");
    if (node->left)
        tree_print(fp, node->left, print_func);
    print_func(fp, NODE_VAL_P(node), node->type);
    if (node->right)
        tree_print(fp, node->right, print_func);
    fprintf(fp, ")");
}

void tree_graph_dump(tree_node_t* node,
                     int(*print_func)(FILE* fp, const void*, int type))
{
    static int numb = 0;
    numb++;
    if (numb == 1)
    {
        system("rm -rf pngs/*.png");
        system("rm -rf dots/*.dot");
    }
    char dot_name[20];
    char png_name[20];
    sprintf(dot_name, "dots/%d.dot", numb);
    sprintf(png_name, "pngs/%d.png", numb);
    FILE* fp = fopen(dot_name, "w");
    if (!fp)
        return;
#ifdef DETAILED_DUMP
    fprintf(fp, "digraph G{\n"
                "\tnode[shape=none; margin=0]\n"
                "\t{\n");
#else
    fprintf(fp, "digraph G{\n"
                "\tnode[shape=ellipse]\n\t{\n");
#endif
    add_dot_node(fp, node, print_func, 1);
    //add_arrows(fp, node, 1);

    fprintf(fp, "\t}\n}");
    fclose(fp);
    char command[55] = "";
    sprintf(command, "dot -Tpng %s -o %s", dot_name, png_name);
    system(command);
}

static void add_dot_node(FILE* fp, tree_node_t* node,
                         int(*print_func)(FILE* fp, const void*, int type),
                         size_t code)
{
#ifdef DETAILED_DUMP
    int type = node_get_type(node);
    fprintf(fp, "n%zu [label = <\n"
                    "\t<TABLE BORDER=\"0\" CELLBORDER=\"1\" "
                    "CELLSPACING=\"0\" CELLPADDING=\"4\">\n"
                    "\t<TR><TD PORT=\"addr\">addr %p</TD></TR>\n"
                    "\t<TR><TD PORT=\"type\">type %p</TD></TR>\n"
                    "\t<TR><TD PORT=\"left\">left %p</TD></TR>\n"
                    "\t<TR><TD PORT=\"right\">right %p</TD></TR>\n"
                    "\t<TR><TD PORT=\"val\"> ",
                    code, node, type, node->left, node->right);
    print_func(fp, NODE_VAL_P(node), node->type);
    fprintf(fp, "</TD></TR>\n</TABLE>>");
    fprintf(fp, "];\n");
#else
    fprintf(fp, "\tn%zu [label = \"", code);
    print_func(fp, NODE_VAL_P(node), node->type);
    fprintf(fp, "\"];\n");
#endif
    if (node->left)
    {
        fprintf(fp, "\tn%zu -> n%zu;\n", code, code*2 + 0);
        add_dot_node(fp, node->left, print_func, code*2 + 0);
    }
    if (node->right)
    {
        fprintf(fp, "\tn%zu -> n%zu;\n", code, code*2 + 1);
        add_dot_node(fp, node->right, print_func, code*2 + 1);
    }
}
