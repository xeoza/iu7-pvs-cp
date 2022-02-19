#ifndef TREE_H
#define TREE_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

#define TREE_KEY_MAX_LEN 128

typedef struct tree_node_struct tree_node_t;

struct tree_node_struct {
    char key[TREE_KEY_MAX_LEN];
    void* value;
    tree_node_t* parent;
    tree_node_t* left;
    tree_node_t* right;
    char color;
    struct list_head list;
};

static inline tree_node_t* make_tree_node(const char* key, void* value) {
    tree_node_t* node = (tree_node_t*)malloc(sizeof(tree_node_t));
    if (!node) {
        return NULL;
    }
    strncpy(node->key, key, TREE_KEY_MAX_LEN);
    node->value = value;
    node->parent = NULL;
    node->left = NULL;
    node->right = NULL;
    node->color = 'b';
    list_init(&node->list);
    return node;
}

void free_tree_node(tree_node_t* node);

void insert_tree_node(tree_node_t** root, tree_node_t* node);

void delete_tree_node(tree_node_t* node);

tree_node_t* find_tree_node(tree_node_t* root, const char* key);

typedef struct dict_struct {
    tree_node_t* root;
    size_t size;
} dict_t;

void dict_init(dict_t* dict);

void dict_free(dict_t* dict);

static inline size_t dict_size(const dict_t* dict) {
    return dict->size;
}

int dict_has(const dict_t* dict, const char* key);

int dict_set(dict_t* dict, const char* key, void* value);

int dict_get(const dict_t* dict, const char* key, void** value); 

#define tree_foreach(root, node) list_foreach(node, &(root)->list, list)

#endif // TREE_H

