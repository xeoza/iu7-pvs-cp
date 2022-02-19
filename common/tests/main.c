#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#include "string_utils_test.h"
#include "tree_test.h"

int main(int argc, char* argv[]) {
    if (CU_initialize_registry() != CUE_SUCCESS) {
    }

    CU_pSuite tree_suite = CU_add_suite("Test tree.h", NULL, NULL);
    CU_ADD_TEST(tree_suite, test_make_tree_node);
    CU_ADD_TEST(tree_suite, test_insert_tree_node);

    CU_pSuite string_utils_suite = CU_add_suite("Test string_utils.h", NULL, NULL);
    CU_ADD_TEST(string_utils_suite, test_strtrim);
    CU_ADD_TEST(string_utils_suite, test_strstartswith);

    CU_basic_run_tests();

    CU_cleanup_registry();
    return 0;
}

