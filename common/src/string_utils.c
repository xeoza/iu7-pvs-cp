#include "string_utils.h"

#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <string.h>

char* strtrim(char* str) {
    assert(str);
    char* start = str;
    char* end = str;
    for (char* c = str; *c != 0; ++c) {
        if (!isspace(*c)) {
            if (start == end)
                start = c;
            end = c + 1;
        }
    }
    *end = 0;
    return start;
}

int strstartswith(const char* str, const char* prefix) {
    assert(str);
    assert(prefix);
    size_t i;
    for (i = 0; str[i] != 0 && prefix[i] != 0; ++i) {
        if (str[i] != prefix[i]) {
            return 0;
        }
    }
    return prefix[i] == 0;
}

const char* strcrlf(const char* str) {
    const char *pattern = "\r\n";
    const char* sp;
    const char* pp;
    assert(str);
    assert(pattern);
    for (sp = str, pp = pattern; *sp != 0 && *pp != 0; ++sp) {
        if (*sp == *pp) {
            ++pp;
        } else {
            pp = pattern;
        }
    }
    if (*pp != 0) {
        return NULL;
    }
    return sp - 2;
}

