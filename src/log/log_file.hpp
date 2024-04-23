#pragma once

// std
#include <cerrno>
#include <cstdio>
#include <string>

// fmt
#include <fmt/core.h>

// common
#include <abort.hpp>

namespace logging {

class log_file {
public:
    log_file() = default;

    log_file(const log_file&) = delete;

    log_file& operator=(const log_file&) = delete;

    void open(const std::string& filename) noexcept
    {
        m_file = std::fopen(filename.c_str(), "w");

        if (!m_file) {
            m_has_error = true;
        }

        m_name = filename;
    }

    [[nodiscard]] bool is_open() const noexcept
    {
        return m_file;
    }

    [[nodiscard]] char* reason() const noexcept
    {
        ABORTIF(!m_has_error,
            "trying to get an error reason when there is "
            "no error");

        return strerror(errno);
    }

    void close()
    {
        ABORTIF(
            !m_file, "trying to close a nullptr FILE *");

        // NOTE: cppreference.com on std::fclose => Whether
        // or not the operation succeeds, the stream is no
        // longer associated with a file, and the buffer
        // allocated by std::setbuf or std::setvbuf, if any,
        // is also disassociated and deallocated if
        // automatic allocation was used.
        //
        // the only reasonable thing to do is to warn the
        // user
        if (std::fclose(m_file) == EOF) {
            fmt::println(stderr,
                "warn: closing file {} failed", m_name);
        }

        m_file = nullptr;
    }

    bool write(std::basic_string_view<char> message)
    {
        ABORTIF(
            !m_file, "trying to write to a nullptr FILE *");

        // NOTE: each of the individual elements
        // of a string view must be chars, for
        // example "á0" is considered to be
        // 3 chars. there using fwrite in
        // this manner should be correct

        std::fwrite(message.data(), sizeof(char),
            message.length(), m_file);

        if (std::ferror(m_file)) {
            m_has_error = true;
        }

        return m_has_error;
    }

    bool eof() noexcept
    {
        ABORTIF(!m_file,
            "trying to test the eof indicator with a "
            "nullptr "
            "FILE *");

        return std::feof(m_file);
    }

    [[nodiscard]] bool error() const noexcept
    {
        return m_has_error;
    }

    [[nodiscard]] FILE* native_handle() noexcept
    {
        ABORTIF(!m_file,
            "trying to get a native handle to a "
            "nullptr FILE *");

        return m_file;
    }

private:
    std::string m_name {};

    bool m_has_error { false };

    FILE* m_file { nullptr };
};

} // namespace logging
