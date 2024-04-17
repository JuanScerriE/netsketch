#pragma once

// std
#include <cerrno>
#include <cstdio>
#include <string>

// fmt
#include <fmt/core.h>

// client
#include <abort.hpp>

namespace common {

class log_file_t {
public:
    log_file_t() = default;

    log_file_t(const common::log_file_t&) = delete;

    log_file_t& operator=(const common::log_file_t&)
        = delete;

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
        return !m_file;
    }

    [[nodiscard]] char* reason() const noexcept
    {
        AbortIf(!m_has_error,
            "trying to get an error reason when there is "
            "no error");

        return strerror(errno);
    }

    void close()
    {
        AbortIf(
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
        AbortIf(
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
        AbortIf(!m_file,
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
        AbortIf(!m_file,
            "trying to get a native handle to a "
            "nullptr FILE *");

        return m_file;
    }

private:
    std::string m_name {};

    bool m_has_error { false };

    FILE* m_file { nullptr };
};

} // namespace common
