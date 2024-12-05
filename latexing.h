#ifndef LATEXING_H
#define LATEXING_H
FILE* create_texfile (const char* filename);
int latex_tree (FILE* fp, tree_node_t* node);
void smart_double_print(FILE* fp, double x);
#endif
