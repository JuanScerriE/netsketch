#pragma once

// std
#include <cstdio>
#include <string>

namespace common {

class log_file_t {
   public:
    log_file_t(const char* filename, const char* mode) {
        m_file = std::fopen(filename, mode);

        if (!m_file) {
            throw std::runtime_error("Failed to open file");
        }
    }

    ~log_file_t() {
        if (m_file) {
            std::fclose(m_file);
        }
    }

    // Function to write to file
    size_t write(const std::string& message) {
        return std::fwrite(
            message.c_str(),
            sizeof(char),
            message.length(),
            m_file
        );
    }

    // Function to check end-of-file
    bool eof() {
        return std::feof(m_file);
    }

    // Function to check for error
    bool error() {
        return std::ferror(m_file);
    }

    FILE* native_handle() {
        return m_file;
    }

   private:
    FILE* m_file{nullptr};
};

}  // namespace common
