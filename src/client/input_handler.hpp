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
    void operator()();

    void shutdown();

    void dtor();

private:
    void process_line(std::string_view line_view);

    // defaults
    std::optional<long> m_selected_id {};

    common::option_e m_tool { common::option_e::LINE };

    prot::colour_t m_colour { 0, 0, 0 };

    bool m_should_exit { false };
};

} // namespace client
