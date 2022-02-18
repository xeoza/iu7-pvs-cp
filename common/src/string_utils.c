#include "string_utils.h"

#include <assert.h>
#include <string.h>

char* strtrim(char* str) {
    assert(str);
    char* start = str;
    char* end = str;
    for (char* c = str; *c != 0; ++c) {
        if (*c != ' ') {
            if (start == end)
                start = c;
            end = c + 1;
        }
    }
    *end = 0;
    return start;
}

