#ifndef STRING_UTILS_H
#define STRING_UTILS_H

char* strtrim(char* str);

int strstartswith(const char* str, const char* prefix, int ignore_case);

const char* strcrlf(const char* str);

#endif  // STRING_UTILS_H

