#pragma once

// std
#include <optional>
#include <string_view>

// common
#include "../common/types.hpp"

namespace client {

enum class Option : uint8_t {
    ALL,
    MINE,
    NONE,
    LINE,
    RECTANGLE,
    CIRCLE,
    TEXT
};

class InputHandler {
   public:
    void operator()();

    void shutdown();

   private:
    void process_line(std::string_view line_view);

    // defaults
    std::optional<long> m_selected_id {};

    Option m_tool { Option::LINE };

    Colour m_colour { 0, 0, 0 };

    bool m_should_exit { false };
};

} // namespace client
