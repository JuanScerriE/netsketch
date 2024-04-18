#pragma once

// fmt
#include <fmt/core.h>

// common
#include <abort.hpp>
#include <log_file.hpp>

class log {
public:
    enum class level : int {
        disable,
        debug,
        info,
        warn,
        error
    };

    log() = delete;

    static void set_file(FILE* file) { s_file = file; }

    // NOTE: this is not ideal we are specifying the
    // log_file_t because we have some semblance of security
    // in the sense that a log_file_t is at least guaranteed
    // to be writeable. But by no means is this any form of
    // secure logging.
    static void set_file(common::log_file_t& file)
    {
        s_file = file.native_handle();
    }

    static void set_level(level log_level)
    {
        s_log_level = log_level;
    }

    static void disable() { s_log_level = level::disable; }

    template <typename... T>
    static void write(level log_level,
        fmt::format_string<T...> fmt, T&&... args)
    {
        if (log_level < s_log_level)
            return;

        switch (log_level) {
        case level::debug:
            fmt::print(s_file, "[debug] : ");
            break;
        case level::info:
            fmt::print(s_file, "[info] : ");
            break;
        case level::warn:
            fmt::print(s_file, "[warn] : ");
            break;
        case level::error:
            fmt::print(s_file, "[error] : ");
            break;
        default:
            Abort("unreachable");
        }

        fmt::println(s_file, fmt, args...);

        // make sure output is visible as soon as possible
        fflush(s_file);
    }

    template <typename... T>
    static void debug(
        fmt::format_string<T...> fmt, T&&... args)
    {
        write(level::debug, fmt, args...);
    }

    template <typename... T>
    static void info(
        fmt::format_string<T...> fmt, T&&... args)
    {
        write(level::info, fmt, args...);
    }

    template <typename... T>
    static void warn(
        fmt::format_string<T...> fmt, T&&... args)
    {
        write(level::warn, fmt, args...);
    }

    template <typename... T>
    static void error(
        fmt::format_string<T...> fmt, T&&... args)
    {
        write(level::error, fmt, args...);
    }

private:
    inline static level s_log_level { level::info };

    inline static FILE* s_file { stderr };
};
