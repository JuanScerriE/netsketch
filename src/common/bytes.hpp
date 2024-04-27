#pragma once

// cstd
#include <cstring>

// std
#include <array>
#include <vector>

// fmt
#include <fmt/core.h>
#include <fmt/format.h>

// ATTRIBUTION
// https://navinmohan.medium.com/dealing-with-endianness-in-c-e3159d5068f0

enum class Endianness {
    LITTLE = 0,
    BIG = 1,
    NATIVE = __BYTE_ORDER__
};

template <typename T, Endianness FROM, Endianness TO>
struct OrderedBytes {
    union {
        T data;

        std::byte bytes[sizeof(T)];
    };

    std::byte& operator[](std::size_t i)
    {
        if (FROM == TO) {
            return bytes[i];
        } else {
            return bytes[sizeof(T) - i - 1];
        }
    }
};

template <std::size_t N>
using ByteArray = std::array<std::byte, N>;

using ByteVector = std::vector<std::byte>;

template <typename T>
ByteArray<sizeof(T)> to_bytes(T value) noexcept
{
    ByteArray<sizeof(T)> bytes {};

    std::memcpy(&bytes, &value, sizeof(T));

    return bytes;
}

template <typename T>
T from_bytes(ByteArray<sizeof(T)> bytes) noexcept
{
    T value {};

    std::memcpy(&value, &bytes, sizeof(T));

    return value;
}

template <
    typename T,
    std::enable_if_t<
        std::is_same_v<T, ByteArray<sizeof(T)>>
            || std::is_same_v<T, ByteVector>,
        bool>
    = true>
std::string bytes_to_string(T bytes)
{
    fmt::memory_buffer buffer {};

    for (std::byte& byte : bytes) {
        fmt::format_to(std::back_inserter(buffer), "{:02x}", byte);
    }

    return fmt::to_string(buffer);
}
