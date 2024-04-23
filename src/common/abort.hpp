#pragma once

// fmt
#include <fmt/core.h>

#define STRINGIFY_(x) #x

#define STRINGIFY(x) STRINGIFY_(x)

#define LINE_STRING STRINGIFY(__LINE__)

#ifndef NDEBUG

#define ABORTIF(cond, msg)                                 \
    do {                                                   \
        if (cond) {                                        \
            fmt::println(stderr,                           \
                __FILE__ ":" LINE_STRING ":: " msg);       \
            std::abort();                                  \
        }                                                  \
    } while (0)

#define ABORTIFV(cond, msg, ...)                           \
    do {                                                   \
        if (cond) {                                        \
            fmt::println(stderr,                           \
                __FILE__ ":" LINE_STRING ":: " msg,        \
                __VA_ARGS__);                              \
            std::abort();                                  \
        }                                                  \
    } while (0)

#define ABORT(msg)                                         \
    do {                                                   \
        fmt::println(                                      \
            stderr, __FILE__ ":" LINE_STRING ":: " msg);   \
        std::abort();                                      \
    } while (0)

#define ABORTV(msg, ...)                                   \
    do {                                                   \
        fmt::println(stderr,                               \
            __FILE__ ":" LINE_STRING ":: " msg,            \
            __VA_ARGS__);                                  \
        std::abort();                                      \
    } while (0)

#else

#define ABORTIF(cond, msg)                                 \
    do {                                                   \
    } while (0)

#define ABORTIFV(cond, msg, ...)                           \
    do {                                                   \
    } while (0)

#define ABORT(msg)                                         \
    do {                                                   \
    } while (0)

#define ABORTV(msg, ...)                                   \
    do {                                                   \
    } while (0)

#endif
