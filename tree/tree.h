#ifndef TREE_H
#define TREE_H

struct tree_node_t;

tree_node_t* new_node(void* val_ptr, size_t val_size, int type, tree_node_t* left, tree_node_t* right);
tree_node_t* node_add_left(tree_node_t* node, tree_node_t* next_node);
tree_node_t* node_add_right(tree_node_t* node, tree_node_t* next_node);
tree_node_t* node_to_left(tree_node_t* node);
tree_node_t* node_to_right(tree_node_t* node);
tree_node_t* branch_copy(tree_node_t* node);

void node_get_val(tree_node_t* node, void* dest);
void branch_delete(tree_node_t* node);
void tree_print(FILE* fp, tree_node_t* node, int(*print_func)(FILE* fp, const void* ptr, int type));
void tree_graph_dump(tree_node_t* node, int(*print_func)(FILE* fp, const void* ptr, int type));
void node_set_val(tree_node_t* node, void* src);
void node_set_type(tree_node_t* node, int type);
void node_set_size(tree_node_t* node, int size);

size_t node_get_size(tree_node_t* node);
size_t node_get_val_size(tree_node_t* node);

int node_get_type(tree_node_t* node);

#endif
