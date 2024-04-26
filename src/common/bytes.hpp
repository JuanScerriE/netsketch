#pragma once

// cstd
#include <cstring>

// std
#include <array>
#include <vector>

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
