#include "tree.h"
#include "tree_test.h"

#include <CUnit/CUnit.h>
#include <unistd.h>

void test_make_tree_node() {
    tree_node_t* node = make_tree_node("key", (void*)"value");
    CU_ASSERT(node->parent == NULL);
    CU_ASSERT(node->left == NULL);
    CU_ASSERT(node->right == NULL);
    CU_ASSERT(node->color == 'b');
    CU_ASSERT_STRING_EQUAL(node->key, "key");
    CU_ASSERT_STRING_EQUAL((char*)node->value, "value");
    free_tree_node(node);
}

void test_insert_tree_node() {
    {
        tree_node_t* a = make_tree_node("A", NULL);
        tree_node_t* b = make_tree_node("B", NULL);
        tree_node_t* c = make_tree_node("C", NULL);
        tree_node_t* root = NULL;

        insert_tree_node(&root, b);
        CU_ASSERT_PTR_EQUAL(root, b);

        insert_tree_node(&root, a);
        CU_ASSERT_PTR_EQUAL(root->left, a);
    
        insert_tree_node(&root, c);
        CU_ASSERT_PTR_EQUAL(root->right, c);

        free_tree_node(root);
    }

    {
        tree_node_t* a = make_tree_node("A", NULL);
        tree_node_t* b = make_tree_node("B", NULL);
        tree_node_t* c = make_tree_node("C", NULL);
        tree_node_t* root = NULL;

        insert_tree_node(&root, a);
        insert_tree_node(&root, b);
        insert_tree_node(&root, c);

        CU_ASSERT_PTR_EQUAL(root, b);
        CU_ASSERT_PTR_EQUAL(root->left, a);
        CU_ASSERT_PTR_EQUAL(root->right, c);

        free_tree_node(root);
    }
}

void test_find_tree_node() {
    tree_node_t* a = make_tree_node("A", NULL);
    tree_node_t* b = make_tree_node("B", NULL);
    tree_node_t* c = make_tree_node("C", NULL);
    tree_node_t* d = make_tree_node("D", NULL);
    tree_node_t* root = NULL;

    insert_tree_node(&root, a);
    insert_tree_node(&root, b);
    insert_tree_node(&root, c);
    insert_tree_node(&root, d);

    CU_ASSERT_PTR_EQUAL(b, find_tree_node(root, "B"));

    CU_ASSERT_PTR_EQUAL(d, find_tree_node(root, "D"));

    CU_ASSERT_PTR_NULL(find_tree_node(root, "E"));

    free_tree_node(root);
}

void test_dict_init() {
    dict_t d;
    dict_init(&d);
    CU_ASSERT_PTR_NULL(d.root);
    CU_ASSERT_EQUAL(0, d.size);
    dict_free(&d);
}

void test_dict_size() {
    dict_t d;
    d.size = 10;
    CU_ASSERT_EQUAL(10, dict_size(&d));
    dict_free(&d);
}

void test_dict_has() {
    dict_t d;
    dict_init(&d);
    CU_ASSERT_FALSE(dict_has(&d, "x"));
    dict_set(&d, "x", "1");
    CU_ASSERT_TRUE(dict_has(&d, "x"));
    CU_ASSERT_FALSE(dict_has(&d, "y"));
    dict_free(&d);
}

void test_dict_set() {
    dict_t d;
    dict_init(&d);
    dict_set(&d, "x", "1");
    CU_ASSERT_PTR_NOT_NULL_FATAL(d.root);
    CU_ASSERT_STRING_EQUAL("x", d.root->key);
    CU_ASSERT_STRING_EQUAL("1", (char*)d.root->value);
    dict_free(&d);
}

void test_dict_get() {
    dict_t d;
    dict_init(&d);
    dict_set(&d, "x", "1");
    char* res = NULL;
    CU_ASSERT_EQUAL(0, dict_get(&d, "x", &res));
    CU_ASSERT_STRING_EQUAL("1", res);
    CU_ASSERT_NOT_EQUAL(0, dict_get(&d, "y", &res));
    dict_free(&d);
}

