#pragma once

// std
#include <cstdint>
#include <cstdio>

// fmt
#include <fmt/core.h>

// common
#include <abort.hpp>
#include <log_file.hpp>

namespace common {

enum class level_e : uint8_t {
    DISABLE,
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class logger_t {
   public:
    // NOTE: this is not ideal we are specifying the
    // log_file_t because we have some semblance of security
    // in the sense that a log_file_t is at least guaranteed
    // to be writeable. But by no means is this any form of
    // secure logging.
    void set_file(log_file_t &file) {
        m_file = file.native_handle();
    }

    void set_level(level_e level) {
        m_log_level = level;
    }

    void disable() {
        m_log_level = level_e::DISABLE;
    }

    template <typename... T>
    void log(
        level_e level,
        fmt::format_string<T...> fmt,
        T &&...args
    ) {
        if (level < m_log_level)
            return;

        switch (level) {
            case level_e::DEBUG:
                fmt::print(m_file, "[DEBUG] : ");
                break;
            case level_e::INFO:
                fmt::print(m_file, "[INFO] : ");
                break;
            case level_e::WARN:
                fmt::print(m_file, "[WARN] : ");
                break;
            case level_e::ERROR:
                fmt::print(m_file, "[ERROR] : ");
                break;
            default:
                Abort("unreachable");
        }

        fmt::println(m_file, fmt, args...);

        // make sure output is visible as soon as possible
        fflush(m_file);
    }

   private:
    level_e m_log_level{level_e::INFO};

    FILE *m_file{stderr};
};

}  // namespace common
