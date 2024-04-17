#pragma once

// std
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

// common
#include <abort.hpp>
#include <header.hpp>
#include <types.hpp>

namespace common {

std::optional<draw_t> deserialize(serialized_t serialized)
{
    deserializer_t deser { serialized.data() };

    auto type = deser.read<type_e>();

    switch (type) {
    case type_e::DRAW: {
        auto tool = deser.read<tool_e>();
        switch (tool) {
        case tool_e::LINE:
            return deser.read<line_draw_t>();
        case tool_e::RECTANGLE:
            return deser.read<rectangle_draw_t>();
        case tool_e::CIRCLE:
            return deser.read<circle_draw_t>();
        case tool_e::TEXT:
            auto colour = deser.read<colour_t>();
            auto x = deser.read<int>();
            auto y = deser.read<int>();
            auto size = deser.read<size_t>();

            std::string text;

            for (size_t i = 0; i < size; i++) {
                text.push_back(deser.read<char>());
            }

            return text_draw_t { colour, x, y, text };
        }
    }
        return {};
    case type_e::UNDO: {
        return {};
    }
    case type_e::CLEAR: {
        return {};
    }
    default:
        throw std::runtime_error(
            fmt::format("unexpected type {}",
                static_cast<uint16_t>(type)));
    }
}

} // namespace common
