#include "tree.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void free_tree_node(tree_node_t* node) {
    if (!node) {
        return;
    }
    free_tree_node(node->right);
    free_tree_node(node->left);
    free(node);
}

static void right_rotate(tree_node_t** root, tree_node_t* node) {
    tree_node_t* right = node->right;
    tree_node_t* parent = node->parent;
    node->right = right->left;
    node->parent = right;
    right->left = node;
    right->color = node->color;
    right->parent = parent;
    node->color = 'r';
    if (!parent) {
        *root = right;
    } else if (parent->left == node) {
        parent->left = right;
    } else {
        parent->right = right;
    }
}

static void left_rotate(tree_node_t** root, tree_node_t* node) {
    tree_node_t* left = node->left;
    tree_node_t* parent = node->parent;
    node->left = left->right;
    node->parent = left;
    left->right = node;
    left->color = node->color;
    left->parent = parent;
    node->color = 'r';
    
    if (!parent) {
        *root = left;
    } else if (parent->left == node) {
        parent->left = left;
    } else {
        parent->right = left;
    }
}

static inline tree_node_t* get_uncle(tree_node_t* node) {
    tree_node_t* gp = node->parent->parent;
    if (gp->left == node->parent) {
        return gp->right;
    } else {
        return gp->left;
    }
}

static void fix_after_insert(tree_node_t** root, tree_node_t* node) {
    while (node->parent && node->parent->color == 'r') {
        tree_node_t* p = node->parent;
        tree_node_t* gp = p->parent;
        tree_node_t* uncle = get_uncle(node);
        if (uncle && uncle->color == 'r') {
            p->color = 'b';
            uncle->color = 'b';
            node = gp;
            node->color = 'r';
        } else if (gp->left == p) {
            if (p->right == node) {
                right_rotate(root, p);
            }
            left_rotate(root, gp);
        } else {
            if (p->left == node) {
                left_rotate(root, p);
            }
            right_rotate(root, gp);
        }
    }
    (*root)->color = 'b';
}

void insert_tree_node(tree_node_t** root, tree_node_t* node) {
    assert(root);
    if (!node) {
        return;
    }

    tree_node_t** p = root;
    tree_node_t* parent = NULL;
    while (*p != NULL) {
        tree_node_t* pp = *p;
        parent = pp;
        int cmp = strcmp(pp->key, node->key);
        if (cmp > 0) {
            p = &pp->left;
        } else if (cmp < 0) {
            p = &pp->right;
        } else {
            pp->value = node->value;
            return;
        }
    }
    *p = node;
    node->color = 'r';
    node->parent = parent;
    if (!parent) {
        list_init(&node->list);
    } else {
        list_add(&parent->list, &node->list);
    }
    fix_after_insert(root, node);
}

void delete_tree_node(tree_node_t* node) {
    
}

tree_node_t* find_tree_node(tree_node_t* root, const char* key) {
    tree_node_t* p = root;
    while (p) {
        int cmp = strcmp(p->key, key);
        if (cmp > 0) {
            p = p->left;
        } else if (cmp < 0) {
            p = p->right;
        } else {
            return p;
        }
    }
    return NULL;
}

void dict_init(dict_t *dict) {
    assert(dict);
    dict->root = NULL;
    dict->size = 0;
}

void dict_free(dict_t* dict) {
    assert(dict);
    free_tree_node(dict->root);
    dict->size = 0;
    dict->root = NULL;
}

int dict_has(const dict_t* dict, const char* key) {
    assert(dict);
    return find_tree_node(dict->root, key) != NULL;
}

int dict_set(dict_t *dict, const char *key, void *value) {
    assert(dict);
    tree_node_t* node = make_tree_node(key, value);
    if (!node) {
        return -1;
    }

    insert_tree_node(&dict->root, node);
    return 0;
}

int dict_get(const dict_t* dict, const char* key, void** value) {
    assert(dict);
    assert(value);
    tree_node_t* node = find_tree_node(dict->root, key);
    if (!node) {
        return -1;
    }
    *value = node->value;
    return 0;
}

