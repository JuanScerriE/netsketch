#pragma once

// fmt
#include <fmt/core.h>

// These are a number of macros which force the program to abort. Useful when we
// as developers know that the state reached by the program is a very bad idea
// and should never be reached. I decided to keep them in even in release
// because there are some cases where an error which is unrecoverable might
// occur. Especially those that come from glibc.

#define STRINGIFY_(x) #x

#define STRINGIFY(x) STRINGIFY_(x)

#define LINE_STRING STRINGIFY(__LINE__)

#define ABORTIF(cond, msg)                                            \
    do {                                                              \
        if (cond) {                                                   \
            fmt::println(stderr, __FILE__ ":" LINE_STRING ":: " msg); \
            std::abort();                                             \
        }                                                             \
    } while (0)

#define ABORTIFV(cond, msg, ...)                    \
    do {                                            \
        if (cond) {                                 \
            fmt::println(                           \
                stderr,                             \
                __FILE__ ":" LINE_STRING ":: " msg, \
                __VA_ARGS__                         \
            );                                      \
            std::abort();                           \
        }                                           \
    } while (0)

#define ABORT(msg)                                                \
    do {                                                          \
        fmt::println(stderr, __FILE__ ":" LINE_STRING ":: " msg); \
        std::abort();                                             \
    } while (0)

#define ABORTV(msg, ...)                                                       \
    do {                                                                       \
        fmt::println(stderr, __FILE__ ":" LINE_STRING ":: " msg, __VA_ARGS__); \
        std::abort();                                                          \
    } while (0)
