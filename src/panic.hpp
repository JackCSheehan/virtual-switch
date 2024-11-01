#pragma once

#include <cstdio>

/*
 * Macro implementation used to "panic" the application. That is, print an error message, print the
 * location in the translation unit where the error occurred, then exit the program.
 */
#define PANIC(...)                                                                                 \
    fprintf(                                                                                       \
        stderr,                                                                                    \
        "PANIC " __FILE_NAME__ ":"                                                                 \
        "%d: ",                                                                                    \
        __LINE__                                                                                   \
    );                                                                                             \
    fprintf(stderr, __VA_ARGS__);                                                                  \
    exit(1);
