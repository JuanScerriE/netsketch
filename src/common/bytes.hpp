#pragma once

// std
#include <string>

// fmt
#include <fmt/core.h>
#include <fmt/format.h>

using ByteString = std::string;

// std::string bytes_to_hex(ByteString bytes)
// {
//     fmt::memory_buffer buffer {};
//
//     for (auto& byte : bytes) {
//         fmt::format_to(std::back_inserter(buffer), "{:02x}", byte);
//     }
//
//     return fmt::to_string(buffer);
// }
