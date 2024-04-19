#pragma once

// std
#include <exception>
#include <string>

// util
#include <utils.hpp>

namespace util {

class serial_error_t : public std::exception {
public:
    explicit serial_error_t(std::string message)
        : m_message(std::move(message))
    {
    }

    [[nodiscard]] const char* what() const noexcept override
    {
        return m_message.c_str();
    }

private:
    std::string m_message {};
};

class fserial_t {
public:
    template <typename T> void write(T value)
    {
        static_assert(std::is_trivially_copyable_v<T>,
            "cannot write non-trivially copyable type");

        for (auto& byte : to_bytes(value)) {
            m_bytes.emplace_back(byte);
        }
    }

    [[nodiscard]] byte_vector bytes() const
    {
        return m_bytes;
    }

private:
    byte_vector m_bytes {};
};

class bserial_t {
public:
    explicit bserial_t(byte_vector bytes)
        : m_bytes(std::move(bytes))
    {
    }

    template <typename T> T read()
    {
        static_assert(std::is_trivially_copyable_v<T>,
            "cannot read non-trivially copyable type");

        if (m_offset + sizeof(T) > m_bytes.size()) {
            throw serial_error_t("exceeded vector size");
        }

        byte_array<sizeof(T)> byte_array {};

        for (size_t i = 0; i < sizeof(T); i++) {
            byte_array[i] = m_bytes[m_offset + i];
        }

        m_offset += sizeof(T);

        return from_bytes<T>(byte_array);
    }

private:
    size_t m_offset { 0 };

    byte_vector m_bytes {};
};

} // namespace util
