#include "string_utils_test.h"

#include <CUnit/CUnit.h>

void test_strtrim() {
    {
        char s[] = "";
        CU_ASSERT_STRING_EQUAL("", strtrim(s));
    }

    {
        char s[] = "   ";
        CU_ASSERT_STRING_EQUAL("", strtrim(s)); 
    }

    {
        char s[] = "abcd";
        CU_ASSERT_STRING_EQUAL("abcd", strtrim(s));
    }

    {
        char s[] = "  abcd   ";
        CU_ASSERT_STRING_EQUAL("abcd", strtrim(s));
    }

    {
        char s[] = "  ab cd   ";
        CU_ASSERT_STRING_EQUAL("ab cd", strtrim(s));
    }
}

void test_strstartswith() {
    {
        CU_ASSERT_TRUE(strstartswith("abcd", ""));
    }

    {
        CU_ASSERT_TRUE(strstartswith("abcd", "ab"));
    }

    {
        CU_ASSERT_FALSE(strstartswith("abcd", "bc"));
    }

    {
        CU_ASSERT_FALSE(strstartswith("abc", "abcd"));
    }
}

