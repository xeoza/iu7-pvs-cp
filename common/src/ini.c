#include "ini.h"

#include <string.h>

#include "string_utils.h"

static int __ini_read_line(dict_t* dict, char* line) {
    const size_t len = strlen(line);
    char* key = line;
    char* value = NULL;
    for (size_t i = 0; i < len; ++i) {
        if (line[i] == ';') {
            break;
        } else if (line[i] == '=') {
            line[i] = 0;
            value = line + i + 1;
        }
    }
    if (!value) {
        return -1;
    }
    key = strtrim(key);
    value = strtrim(value);
    key = strdup(key);
    if (key) {
        value = strdup(value);
        if (value) {
            dict_set(dict, key, (void*)value);
            return 0;
        }
        free(key);
    }
    return -1;
}

int ini_read_file_to_dict(dict_t* dict, FILE* file) {
    char line[INI_MAX_LINE_LEN] = { 0 };
    while (fgets(line, INI_MAX_LINE_LEN, file)) {
        if (__ini_read_line(dict, line) != 0) {
            return -1;
        }
    }
    return 0;
}

