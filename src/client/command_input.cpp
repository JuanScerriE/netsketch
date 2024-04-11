// client
#include <command_input.hpp>

// std
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

// fmt
#include <fmt/core.h>

namespace client {

void command_input_t::start() {
    do {
        std::string line{};

        fmt::print("> ");
        std::getline(std::cin, line);

        process_line(line);
    } while (!should_exit());
}

std::vector<std::string_view> split(std::string_view line_view) {
    std::vector<std::string_view> lexemes{};

    size_t pos = 0;

    while (pos < line_view.length()) {
        while (isspace(line_view[pos])) {
            pos++;
        }

        if (pos >= line_view.length()) {
            break;
        }

        size_t count = 0;

        while (!isspace(line_view[pos + count])) {
            count++;
        }

        lexemes.push_back(line_view.substr(pos, count));

        pos += count;
    }

    return lexemes;
}

void command_input_t::process_line(std::string_view line_view) {
    for (auto& token : split(line_view)) {
        fmt::println(token);
    }
}

bool command_input_t::should_exit() {
    return true;
}

}  // namespace client
