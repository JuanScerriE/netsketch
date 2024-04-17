#pragma once

// fmt
#include <fmt/core.h>

// std
#include <utility>

#define STRINGIFY_(x) #x

#define STRINGIFY(x) STRINGIFY_(x)

#define LINE_STRING STRINGIFY(__LINE__)

#ifndef NDEBUG

#define AbortIf(cond, msg)                                 \
    do {                                                   \
        if (cond) {                                        \
            fmt::println(stderr,                           \
                __FILE__ ":" LINE_STRING ":: " msg);       \
            std::abort();                                  \
        }                                                  \
    } while (0)

#define AbortIfV(cond, msg, ...)                           \
    do {                                                   \
        if (cond) {                                        \
            fmt::println(stderr,                           \
                __FILE__ ":" LINE_STRING ":: " msg,        \
                __VA_ARGS__);                              \
            std::abort();                                  \
        }                                                  \
    } while (0)

#define Abort(msg)                                         \
    do {                                                   \
        fmt::println(                                      \
            stderr, __FILE__ ":" LINE_STRING ":: " msg);   \
        std::abort();                                      \
    } while (0)

#define AbortV(msg, ...)                                   \
    do {                                                   \
        fmt::println(stderr,                               \
            __FILE__ ":" LINE_STRING ":: " msg,            \
            __VA_ARGS__);                                  \
        std::abort();                                      \
    } while (0)

#else

#define AbortIf(cond, msg)                                 \
    do {                                                   \
    } while (0)

#define AbortIfV(cond, msg, ...)                           \
    do {                                                   \
    } while (0)

#define Abort(msg)                                         \
    do {                                                   \
    } while (0)

#define AbortV(msg, ...)                                   \
    do {                                                   \
    } while (0)

#endif
