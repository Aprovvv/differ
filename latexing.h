#ifndef LATEXING_H
#define LATEXING_H
FILE* create_texfile (char* filename);
int latex_tree (FILE* fp, tree_node_t* node);
#endif
