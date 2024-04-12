// client
#include <command_input.hpp>

// std
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

// fmt
#include <fmt/core.h>

namespace client {

command_input_t::command_input_t(std::atomic_bool& stop)
    : a_stop(stop) {
}

void command_input_t::operator()() {
    start();
}

void command_input_t::start() {
    do {
        std::string line{};

        fmt::print("> ");

        std::getline(std::cin, line);

        process_line(line);

    } while (!m_should_exit);

    // make sure to stop the gui
    a_stop = true;
}

std::vector<std::string_view> split(
    std::string_view line_view
) {
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

void command_input_t::process_line(
    std::string_view line_view
) {
    std::vector<std::string_view> tokens = split(line_view);

    if (tokens.empty()) {
        return;
    }

    std::string_view& first_token = tokens[0];

    if (first_token == "help") {
        if (tokens.size() != 1) {
            fmt::println(
                stderr,
                "warn: an unexpected number of token for "
                "tool"
            );

            return;
        }
        fmt::println(
            " 1. help - list all available commands and "
            "their usage"
        );
        fmt::println(
            " 2. tool {{line | rectangle | circle | text}} "
            "- select a tool for drawing"
        );
        fmt::println(
            " 3. colour {{RED}} {{GREEN}} {{BLUE}} - sets "
            "the drawing colour using RED, GREEN, BLUE "
            "values"
        );
        fmt::println(
            " 4. draw {{...}}... - draw a shape according "
            "to the selected tool and colour on the canvas"
            "\n    \twhen tool is 'line', draw {{X0}} "
            "{{Y0}} {{X1}} {{Y1}} - draw a line between "
            "the points (X0, Y0) to (X1, Y1)"
            "\n    \twhen tool is 'rectangle', draw {{X}} "
            "{{Y}} {{WIDTH}} {{HEIGHT}} - draw a rectangle "
            "starting at (X, Y) having width WIDTH and "
            "height HEIGHT"
            "\n    \twhen tool is 'circle', draw {{X}} "
            "{{Y}} {{RADIUS}} - draw a circle at (X, Y) "
            "with radius RADIUS"
            "\n    \twhen tool is 'text', draw {{X}} {{Y}} "
            "{{STRING}} - draw a string at (X, Y) with the "
            "characters in STRING"
        );
        fmt::println(
            " 5. list [all | line | rectangle | circle | "
            "text] [all | mine] - displays issued draw "
            "commands in the console, filtered by tool "
            "type and/or user"
        );
        fmt::println(
            " 6. select {{none | ID}} - select an existing "
            "draw command (with the specified id ID) to be "
            "modified by a subsequent "
            "draw command"
        );
        fmt::println(
            " 7. delete {{ID}} - deletes the draw command "
            "with the specified id ID"
        );
        fmt::println(" 8. undo - revert the last action");
        fmt::println(
            " 9. clear {{all | mine}} - clears the canvas"
            "\n    \tusing 'all' clears all draw commands"
            "\n    \tusing 'mine' clears only draw "
            "commands "
            "issued by the running client"
        );
        fmt::println(
            "10. show {{all | mine}} - controls what is "
            "displayed on the canvas"
            "\n    \tusing 'all' displays all draw commands"
            "\n    \tusing 'mine' displays only draw "
            "commands "
            "issued by the running client"
        );
        fmt::println(
            "11. exit - (if the client is running in "
            "--server mode disconnect from the "
            "server and) exit the application"
        );
        return;
    }

    if (first_token == "tool") {
        if (tokens.size() != 2) {
            fmt::println(
                stderr,
                "warn: an unexpected number of token for "
                "tool"
            );

            return;
        }

        std::string_view second_token = tokens[1];

        if (second_token == "line") {
            return;
        }

        if (second_token == "rectangle") {
            return;
        }

        if (second_token == "circle") {
            return;
        }

        if (second_token == "text") {
            return;
        }

        fmt::println(
            stderr,
            "warn: {} is not a known tool",
            second_token
        );

        return;
    }

    if (first_token == "colour") {
        if (tokens.size() != 4) {
            fmt::println(
                stderr,
                "warn: an unexpected number of tokens for "
                "colour"
            );

            return;
        }

        std::string_view red = tokens[1];

        int red_value{};

        bool red_has_error{false};

        try {
            red_value = std::stoi(std::string{red});
        } catch (std::invalid_argument&) {
            red_has_error = true;
        } catch (std::out_of_range&) {
            red_has_error = true;
        }

        if (!(0 < red_value && red_value < 256)) {
            red_has_error = true;
        }

        if (red_has_error) {
            fmt::println(
                stderr,
                "warn: expected an integer in the range "
                "0-255 (inclusive) for RED"
            );

            return;
        }

        std::string_view green = tokens[2];

        int green_value{};

        bool green_has_error{false};

        try {
            green_value = std::stoi(std::string{green});
        } catch (std::invalid_argument&) {
            green_has_error = true;
        } catch (std::out_of_range&) {
            green_has_error = true;
        }

        if (!(0 < green_value && green_value < 256)) {
            green_has_error = true;
        }

        if (green_has_error) {
            fmt::println(
                stderr,
                "warn: expected an integer in the range "
                "0-255 (inclusive) for GREEN"
            );

            return;
        }

        std::string_view blue = tokens[3];

        int blue_value{};

        bool blue_has_error{false};

        try {
            blue_value = std::stoi(std::string{blue});
        } catch (std::invalid_argument&) {
            blue_has_error = true;
        } catch (std::out_of_range&) {
            blue_has_error = true;
        }

        if (!(0 < blue_value && blue_value < 256)) {
            blue_has_error = true;
        }

        if (blue_has_error) {
            fmt::println(
                stderr,
                "warn: expected an integer in the range "
                "0-255 (inclusive) for BLUE"
            );

            return;
        }

        // handle

        return;
    }

    if (first_token == "draw") {
        return;
    }

    if (first_token == "list") {
        if (tokens.size() > 3) {
            fmt::println(
                stderr,
                "warn: an unexpected number of tokens for "
                "list"
            );

            return;
        }


        if (tokens.size() == 2) {
            std::string_view second_token = tokens[1];

            bool match{false};

            if (second_token == "line") {
            }

            if (second_token == "rectangle") {
            }

            if (second_token == "circle") {
            }

            if (second_token == "text") {
            }

            if (!match) {
                return;
            }
        }

        if (tokens.size() == 3) {
            // handle
            return;
        }

        return;
    }

    if (first_token == "select") {
        if (tokens.size() != 2) {
            fmt::println(
                stderr,
                "warn: an unexpected number of tokens for "
                "select"
            );

            return;
        }
        return;
    }

    if (first_token == "delete") {
        if (tokens.size() != 2) {
            fmt::println(
                stderr,
                "warn: an unexpected number of tokens for "
                "delete"
            );

            return;
        }

        return;
    }

    if (first_token == "undo") {
        if (tokens.size() != 1) {
            fmt::println(
                stderr,
                "warn: an unexpected number of tokens for "
                "undo"
            );

            return;
        }

        return;
    }

    if (first_token == "clear") {
        if (tokens.size() != 2) {
            fmt::println(
                stderr,
                "warn: an unexpected number of tokens for "
                "clear"
            );

            return;
        }
        return;
    }

    if (first_token == "show") {
        if (tokens.size() != 2) {
            fmt::println(
                stderr,
                "warn: an unexpected number of tokens for "
                "show"
            );

            return;
        }
        return;
    }

    if (first_token == "exit") {
        if (tokens.size() != 1) {
            fmt::println(
                stderr,
                "warn: an unexpected number of tokens for "
                "exit"
            );

            return;
        }

        m_should_exit = true;

        return;
    }

    fmt::println(
        stderr,
        "warn: {} is not a known command",
        first_token
    );
}

}  // namespace client
