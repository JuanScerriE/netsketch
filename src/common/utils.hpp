#pragma once

// std
#include <array>
#include <cstring>
#include <vector>

namespace util {

template <std::size_t N>
using byte_array = std::array<std::byte, N>;

using byte_vector = std::vector<std::byte>;

template <typename T>
byte_array<sizeof(T)> to_bytes(T value)
{
    byte_array<sizeof(T)> bytes {};

    std::memcpy(&bytes, &value, sizeof(T));

    return bytes;
}

template <typename T>
T from_bytes(byte_array<sizeof(T)> bytes)
{
    T value {};

    std::memcpy(&value, &bytes, sizeof(T));

    return value;
}

} // namespace util
