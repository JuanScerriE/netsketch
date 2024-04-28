#pragma once

// cstd
#include <cstdarg>
#include <cstdio>

// fmt
#include <fmt/chrono.h>
#include <fmt/core.h>

// logging
#include "../common/log_file.hpp"

namespace logging {

class log {
   public:
    enum class level : int {
        debug,
        info,
        warn,
        error
    };

    void set_file(FILE* file)
    {
        m_file = file;
    }

    // NOTE: this is not ideal we are specifying the
    // log_file_t because we have some semblance of security
    // in the sense that a log_file_t is at least guaranteed
    // to be writeable. But by no means is this any form of
    // secure logging.
    void set_file(log_file& file)
    {
        m_file = file.native_handle();
    }

    void set_prefix(std::string prefix)
    {
        m_prefix = std::move(prefix);
    }

    void set_level(level log_level)
    {
        m_log_level = log_level;
    }

    void toggle()
    {
        m_disabled = !m_disabled;
    }

    void flush()
    {
        if (fflush(m_file) == EOF) {
            ABORTV("flushing log file failed, reason: {}", strerror(errno));
        }
    }

    template <typename... T>
    void write(level log_level, fmt::format_string<T...> fmt, T&&... args)
    {
        if (m_disabled)
            return;

        log_level = clamp_level(log_level);

        if (log_level < m_log_level)
            return;

        write_level(log_level);

        fmt::println(m_file, fmt, args...);

        fflush(m_file);
    }

    void c_write(level log_level, const char* fmt, va_list args)
    {
        if (m_disabled)
            return;

        log_level = clamp_level(log_level);

        if (log_level < m_log_level)
            return;

        write_level(log_level);

        vfprintf(m_file, fmt, args);

        fprintf(m_file, "\n");

        fflush(m_file);
    }

    void c_write(level log_level, const char* fmt, ...)
    {
        if (m_disabled)
            return;

        log_level = clamp_level(log_level);

        if (log_level < m_log_level)
            return;

        va_list args;

        va_start(args, fmt);
        vfprintf(m_file, fmt, args);
        va_end(args);

        fprintf(m_file, "\n");

        fflush(m_file);
    }

    template <typename... T>
    void debug(fmt::format_string<T...> fmt, T&&... args)
    {
        write(level::debug, fmt, args...);
    }

    template <typename... T>
    void info(fmt::format_string<T...> fmt, T&&... args)
    {
        write(level::info, fmt, args...);
    }

    template <typename... T>
    void warn(fmt::format_string<T...> fmt, T&&... args)
    {
        write(level::warn, fmt, args...);
    }

    template <typename... T>
    void error(fmt::format_string<T...> fmt, T&&... args)
    {
        write(level::error, fmt, args...);
    }

   private:
    static level clamp_level(level log_level)
    {
        if (log_level < level::debug) {
            log_level = level::debug;
        }

        if (log_level > level::error) {
            log_level = level::error;
        }

        return log_level;
    }

    void write_level(level log_level)
    {
        switch (log_level) {
        case level::debug:
            fmt::print(m_file, "{}[debug] : ", m_prefix);
            break;
        case level::info:
            fmt::print(m_file, "{}[info] : ", m_prefix);
            break;
        case level::warn:
            fmt::print(m_file, "{}[warn] : ", m_prefix);
            break;
        case level::error:
            fmt::print(m_file, "{}[error] : ", m_prefix);
            break;
        }
    }

    bool m_disabled { false };

    level m_log_level { level::info };

    FILE* m_file { stderr };

    std::string m_prefix {};
};

} // namespace common
