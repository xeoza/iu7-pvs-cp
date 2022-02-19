#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

#ifdef DEBUG
#define debug(...) (printf(__VA_ARGS__))
#else
#define debug(...) 0
#endif  // DEBUG

#endif  // DEBUG_H
