// client
#include <cstring>
#include <input_handler.hpp>

// std
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

// unix
#include <poll.h>
#include <unistd.h>

// common
#include <types.hpp>

// fmt
#include <fmt/core.h>

// share
#include <share.hpp>

namespace client {

void input_handler_t::operator()() {
    start();
}

void input_handler_t::start() {
    do {
        std::string line{};

        fmt::print("> ");

        std::getline(std::cin, line);

        process_line(line);
    } while (!m_should_exit);

    pollfd poll_fd{
        share::e_stop_event->write_fd(),
        POLLOUT,
        0
    };

    if (poll(&poll_fd, 1, -1) == -1) {
        AbortV(
            "poll of event file descriptor failed, reason: "
            "{}",
            strerror(errno)
        );
    }

    if ((poll_fd.revents & POLLOUT) == 0) {
        Abort("cannot write to event file descriptor");
    }

    share::e_stop_event->notify();

    // make sure to stop the gui
    common::mutable_t<bool>{share::e_stop_gui}() = true;
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

void input_handler_t::process_line(
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
            "\n 2. tool {{line | rectangle | circle | "
            "text}} "
            "- select a tool for drawing"
            "\n 3. colour {{RED}} {{GREEN}} {{BLUE}} - "
            "sets "
            "the drawing colour using RED, GREEN, BLUE "
            "values"
            "\n 4. draw {{...}}... - draw a shape "
            "according "
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
            "\n 5. list [all | line | rectangle | circle | "
            "text] [all | mine] - displays issued draw "
            "commands in the console, filtered by tool "
            "type and/or user"
            "\n 6. select {{none | ID}} - select an "
            "existing "
            "draw command (with the specified id ID) to be "
            "modified by a subsequent "
            "draw command"
            "\n 7. delete {{ID}} - deletes the draw "
            "command "
            "with the specified id ID"
            "\n 8. undo - revert the last action"
            "\n 9. clear {{all | mine}} - clears the canvas"
            "\n    \tusing 'all' clears all draw commands"
            "\n    \tusing 'mine' clears only draw "
            "commands "
            "issued by the running client"
            "\n10. show {{all | mine}} - controls what is "
            "displayed on the canvas"
            "\n    \tusing 'all' displays all draw commands"
            "\n    \tusing 'mine' displays only draw "
            "commands "
            "issued by the running client"
            "\n11. exit - (if the client is running in "
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
            m_tool = common::option_e::LINE;
            return;
        }

        if (second_token == "rectangle") {
            m_tool = common::option_e::RECTANGLE;
            return;
        }

        if (second_token == "circle") {
            m_tool = common::option_e::CIRCLE;
            return;
        }

        if (second_token == "text") {
            m_tool = common::option_e::TEXT;
            return;
        }

        fmt::println(
            stderr,
            "warn: expected one of {{'line' | 'rectangle' "
            "| 'circle' | 'text'}}"
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

        int red_value{0};

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

        int green_value{0};

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

        int blue_value{0};

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

        m_colour = {
            static_cast<uint8_t>(red_value),
            static_cast<uint8_t>(green_value),
            static_cast<uint8_t>(blue_value)
        };

        return;
    }

    if (first_token == "draw") {
        return;
    }

    if (first_token == "list") {
        if (tokens.size() != 3) {
            fmt::println(
                stderr,
                "warn: an unexpected number of tokens for "
                "list"
            );

            return;
        }

        std::string_view second_token = tokens[1];

        bool second_match{false};

        if (second_token == "all") {
            second_match = true;
        }

        if (second_token == "line") {
            second_match = true;
        }

        if (second_token == "rectangle") {
            second_match = true;
        }

        if (second_token == "circle") {
            second_match = true;
        }

        if (second_token == "text") {
            second_match = true;
        }

        if (!second_match) {
            fmt::println(
                stderr,
                "warn: expected one of {{'all' | 'line' | "
                "'rectangle' | 'circle' | 'text'}}"
            );

            return;
        }

        std::string_view third_token = tokens[2];

        bool third_match{false};

        if (third_token == "all") {
            third_match = true;
        }

        if (third_token == "mine") {
            third_match = true;
        }

        if (!third_match) {
            fmt::println(
                stderr,
                "warn: expected one of {{'all' | 'mine'}}"
            );

            return;
        }

        // handle

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

        std::string_view second_token = tokens[1];

        if (second_token == "none") {
            m_selected_id = {};

            return;
        }

        unsigned long id{0};

        bool id_has_error{false};

        try {
            id = std::stoul(std::string{second_token});
        } catch (std::invalid_argument&) {
            id_has_error = true;
        } catch (std::out_of_range&) {
            id_has_error = true;
        }

        if (!id_has_error) {
            fmt::println(
                stderr,
                "warn: expected an 'none' or an "
                "integer greater than "
                "or equal to 0 for ID"
            );

            return;
        }

        m_selected_id = id;

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

        std::string_view second_token = tokens[1];

        unsigned long id{0};

        bool id_has_error{false};

        try {
            id = std::stoul(std::string{second_token});
        } catch (std::invalid_argument&) {
            id_has_error = true;
        } catch (std::out_of_range&) {
            id_has_error = true;
        }

        if (!id_has_error) {
            fmt::println(
                stderr,
                "warn: expected an integer greater than or "
                "equal to 0 for ID"
            );

            return;
        }

        (void)id;

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

        std::string_view second_token = tokens[1];

        if (second_token == "all") {
            return;
        }

        if (second_token == "mine") {
            return;
        }

        fmt::println(
            stderr,
            "warn: expected one of {{'all' | 'mine'}}"
        );

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

        std::string_view second_token = tokens[1];

        if (second_token == "all") {
            common::mutable_t<bool>{share::e_show_mine}() =
                false;

            return;
        }

        if (second_token == "mine") {
            common::mutable_t<bool>{share::e_show_mine}() =
                true;

            return;
        }

        fmt::println(
            stderr,
            "warn: expected one of {{'all' | 'mine'}}"
        );

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
