#pragma once

// std
#include <optional>
#include <string_view>

// common
#include <types.hpp>

// client
#include <gui.hpp>

namespace client {

class input_handler_t {
public:
    void start();

    void operator()();

private:
    void process_line(std::string_view line_view);

    // defaults
    std::optional<unsigned long> m_selected_id {};

    common::option_e m_tool { common::option_e::LINE };
    common::colour_t m_colour { 255, 255, 255 };

    bool m_should_exit { false };
};

} // namespace client
