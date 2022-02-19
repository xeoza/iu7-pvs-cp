#ifndef INI_H
#define INI_H

#include <stdio.h>

#include "tree.h"

#define INI_MAX_LINE_LEN 256

int ini_read_file_to_dict(dict_t* dict, FILE* file);

#endif  // INI_H

