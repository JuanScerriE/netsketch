// client
#include "input_handler.hpp"
#include "input_parser.hpp"
#include "share.hpp"

// std
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

// common
#include "../common/abort.hpp"
#include "../common/overload.hpp"
#include "../common/threading.hpp"
#include "../common/types.hpp"

// fmt
#include <fmt/core.h>

namespace client {

void InputHandler::operator()()
{
    do {
        std::string line {};

        fmt::print("> ");

        std::getline(std::cin, line);

        process_line(line);
    } while (!m_should_exit);

    shutdown();
}

void InputHandler::shutdown()
{
    if (share::writer_thread.is_initialized()
        && share::writer_thread.is_alive())
        share::writer_thread.cancel();
    if (share::reader_thread.is_initialized()
        && share::reader_thread.is_alive())
        share::reader_thread.cancel();

    share::run_gui = false;
}

static void print_draw(size_t index, Draw draw)
{
    std::visit(
        overload {
            [index](TextDraw& arg) {
                fmt::println(
                    "[{}] => [text] [{} {} {}] [{} {} \"{}\"]",
                    index,
                    arg.colour.r,
                    arg.colour.g,
                    arg.colour.b,
                    arg.x,
                    arg.y,
                    arg.string
                );
            },
            [index](CircleDraw& arg) {
                fmt::println(
                    "[{}] => [circle] [{} {} {}] [{} {} {}]",
                    index,
                    arg.colour.r,
                    arg.colour.g,
                    arg.colour.b,
                    arg.x,
                    arg.y,
                    arg.r
                );
            },
            [index](RectangleDraw& arg) {
                fmt::println(
                    "[{}] => [rectangle] [{} {} {}] [{} {} {} {}]",
                    index,
                    arg.colour.r,
                    arg.colour.g,
                    arg.colour.b,
                    arg.x,
                    arg.y,
                    arg.w,
                    arg.h
                );
            },
            [index](LineDraw& arg) {
                fmt::println(
                    "[{}] => [line] [{} {} {}] [{} {} {} {}]",
                    index,
                    arg.colour.r,
                    arg.colour.g,
                    arg.colour.b,
                    arg.x0,
                    arg.y0,
                    arg.x1,
                    arg.y1
                );
            },
        },
        draw
    );
}

static void
list_draws(Option tool_type, Option user_qual, TaggedDrawVector& draw_vector)
{
    size_t index = 0;

    for (auto& tagged_draw : draw_vector) {
        switch (user_qual) {
        case Option::ALL:
            switch (tool_type) {
            case Option::ALL:
                print_draw(index, tagged_draw.draw);
                break;
            case Option::LINE:
                if (std::holds_alternative<LineDraw>(tagged_draw.draw)) {
                    print_draw(index, tagged_draw.draw);
                }
                break;
            case Option::RECTANGLE:
                if (std::holds_alternative<RectangleDraw>(tagged_draw.draw)) {
                    print_draw(index, tagged_draw.draw);
                }
                break;
            case Option::CIRCLE:
                if (std::holds_alternative<CircleDraw>(tagged_draw.draw)) {
                    print_draw(index, tagged_draw.draw);
                }
                break;
            case Option::TEXT:
                if (std::holds_alternative<TextDraw>(tagged_draw.draw)) {
                    print_draw(index, tagged_draw.draw);
                }
                break;
            default:
                ABORT("unreachable");
            }
            break;
        case Option::MINE:
            if (share::username == tagged_draw.username) {
                switch (tool_type) {
                case Option::ALL:
                    print_draw(index, tagged_draw.draw);
                    break;
                case Option::LINE:
                    if (std::holds_alternative<LineDraw>(tagged_draw.draw)) {
                        print_draw(index, tagged_draw.draw);
                    }
                    break;
                case Option::RECTANGLE:
                    if (std::holds_alternative<RectangleDraw>(tagged_draw.draw
                        )) {
                        print_draw(index, tagged_draw.draw);
                    }
                    break;
                case Option::CIRCLE:
                    if (std::holds_alternative<CircleDraw>(tagged_draw.draw)) {
                        print_draw(index, tagged_draw.draw);
                    }
                    break;
                case Option::TEXT:
                    if (std::holds_alternative<TextDraw>(tagged_draw.draw)) {
                        print_draw(index, tagged_draw.draw);
                    }
                    break;
                default:
                    ABORT("unreachable");
                }
            }
            break;
        default:
            ABORT("unreachable");
        }

        index++;
    }
}

// The process line function is just a massive function
// for handling the users input. I decided to
// keep like so instead of breaking it up into
// separate functions so I can have more control
// over the control flow using returns.

void InputHandler::process_line(std::string_view line_view)
{
    InputParser parser { line_view };

    try {
        parser.scan_tokens();
    } catch (std::runtime_error& err) {
        fmt::println(stderr, "warn: {}", err.what());

        return;
    }

    std::vector<std::string> tokens = parser.get_tokens();

    if (tokens.empty()) {
        return;
    }

    std::string& first_token = tokens[0];

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
            " 1. help - list all available commands and their usage"
            "\n 2. tool {{line | rectangle | circle | text}} - select a tool "
            "for drawing"
            "\n 3. colour {{RED}} {{GREEN}} {{BLUE}} - sets the drawing colour "
            "using RED, GREEN, BLUE values"
            "\n 4. draw {{...}}... - draw a shape according to the selected "
            "tool and colour on the canvas"
            "\n    \twhen tool is 'line', draw {{X0}} {{Y0}} {{X1}} {{Y1}} - "
            "draw a line between the points (X0, Y0) to (X1, Y1)"
            "\n    \twhen tool is 'rectangle', draw {{X}} {{Y}} {{WIDTH}} "
            "{{HEIGHT}} - draw a rectangle starting at (X, Y) having width "
            "WIDTH and height HEIGHT"
            "\n    \twhen tool is 'circle', draw {{X}} {{Y}} {{RADIUS}} - draw "
            "a circle at (X, Y) with radius RADIUS"
            "\n    \twhen tool is 'text', draw {{X}} {{Y}} {{STRING}} - draw a "
            "string at (X, Y) with the characters in STRING"
            "\n 5. list [all | line | rectangle | circle | text] [all | mine] "
            "- displays issued draw commands in, filtered by tool type and/or "
            "user"
            "\n 6. select {{none | ID}} - select an existing draw command "
            "(with the specified id ID) to be modified by a subsequent draw "
            "command"
            "\n 7. delete {{ID}} - deletes the draw command with the specified "
            "id ID"
            "\n 8. undo - revert the last action"
            "\n 9. clear {{all | mine}} - clears the canvas"
            "\n    \tusing 'all' clears all draw commands"
            "\n    \tusing 'mine' clears only draw commands issued by the "
            "running client"
            "\n10. show {{all | mine}} - controls what is displayed on the "
            "canvas"
            "\n    \tusing 'all' displays all draw commands"
            "\n    \tusing 'mine' displays only draw commands issued by the "
            "running client"
            "\n11. exit - (if the client is running in --server mode "
            "disconnect from the server and) exit the application"
        );

        return;
    }

    if (first_token == "tool") {
        if (tokens.size() != 2) {
            fmt::println(
                stderr,
                "warn: an unexpected number of token for tool"
            );

            return;
        }

        std::string_view second_token = tokens[1];

        if (second_token == "line") {
            m_tool = Option::LINE;
            return;
        }

        if (second_token == "rectangle") {
            m_tool = Option::RECTANGLE;
            return;
        }

        if (second_token == "circle") {
            m_tool = Option::CIRCLE;
            return;
        }

        if (second_token == "text") {
            m_tool = Option::TEXT;
            return;
        }

        fmt::println(
            stderr,
            "warn: expected one of {{'line' | 'rectangle' | 'circle' | 'text'}}"
        );

        return;
    }

    if (first_token == "colour") {
        if (tokens.size() != 4) {
            fmt::println(
                stderr,
                "warn: an unexpected number of tokens for colour"
            );

            return;
        }

        std::string red = tokens[1];

        int red_value { 0 };

        bool red_has_error { false };

        try {
            red_value = std::stoi(red);
        } catch (std::invalid_argument&) {
            red_has_error = true;
        } catch (std::out_of_range&) {
            red_has_error = true;
        }

        if (!(0 <= red_value && red_value < 256)) {
            red_has_error = true;
        }

        if (red_has_error) {
            fmt::println(
                stderr,
                "warn: expected an integer in the range 0-255 (inclusive) for "
                "RED"
            );

            return;
        }

        std::string green = tokens[2];

        int green_value { 0 };

        bool green_has_error { false };

        try {
            green_value = std::stoi(green);
        } catch (std::invalid_argument&) {
            green_has_error = true;
        } catch (std::out_of_range&) {
            green_has_error = true;
        }

        if (!(0 <= green_value && green_value < 256)) {
            green_has_error = true;
        }

        if (green_has_error) {
            fmt::println(
                stderr,
                "warn: expected an integer in the range 0-255 (inclusive) for "
                "GREEN"
            );

            return;
        }

        std::string blue = tokens[3];

        int blue_value { 0 };

        bool blue_has_error { false };

        try {
            blue_value = std::stoi(blue);
        } catch (std::invalid_argument&) {
            blue_has_error = true;
        } catch (std::out_of_range&) {
            blue_has_error = true;
        }

        if (!(0 <= blue_value && blue_value < 256)) {
            blue_has_error = true;
        }

        if (blue_has_error) {
            fmt::println(
                stderr,
                "warn: expected an integer in the range 0-255 (inclusive) for "
                "BLUE"
            );

            return;
        }

        m_colour = { static_cast<uint8_t>(red_value),
                     static_cast<uint8_t>(green_value),
                     static_cast<uint8_t>(blue_value) };

        return;
    }

    if (first_token == "draw") {
        switch (m_tool) {
        case Option::LINE: {
            if (tokens.size() != 5) {
                fmt::println(
                    stderr,
                    "warn: an unexpected number of tokens for draw with line "
                    "selected"
                );

                return;
            }

            int x0 {};

            try {
                x0 = std::stoi(tokens[1]);
            } catch (std::invalid_argument&) {
                fmt::println(stderr, "warn: expected integer (32-bit) for x0");

                return;
            } catch (std::out_of_range&) {
                fmt::println(stderr, "warn: expected integer (32-bit) for x0");

                return;
            }

            int y0 {};

            try {
                y0 = std::stoi(tokens[2]);
            } catch (std::invalid_argument&) {
                fmt::println(stderr, "warn: expected integer (32-bit) for y0");

                return;
            } catch (std::out_of_range&) {
                fmt::println(stderr, "warn: expected integer (32-bit) for y0");

                return;
            }

            int x1 {};

            try {
                x1 = std::stoi(tokens[3]);
            } catch (std::invalid_argument&) {
                fmt::println(stderr, "warn: expected integer (32-bit) for x1");

                return;
            } catch (std::out_of_range&) {
                fmt::println(stderr, "warn: expected integer (32-bit) for x1");

                return;
            }

            int y1 {};

            try {
                y1 = std::stoi(tokens[4]);
            } catch (std::invalid_argument&) {
                fmt::println(stderr, "warn: expected integer (32-bit) for y1");

                return;
            } catch (std::out_of_range&) {
                fmt::println(stderr, "warn: expected integer (32-bit) for y1");

                return;
            }

            LineDraw draw { m_colour, x0, y0, x1, y1 };

            Action action { draw };

            if (m_selected_id) {
                action = Select { *m_selected_id, draw };
            }

            {
                threading::mutex_guard guard { share::writer_mutex };

                share::writer_queue.push(action);
            }

            share::writer_cond.notify_one();
        } break;
        case Option::RECTANGLE: {
            if (tokens.size() != 5) {
                fmt::println(
                    stderr,
                    "warn: an unexpected number of tokens for draw with "
                    "rectangle selected"
                );

                return;
            }

            int x {};

            try {
                x = std::stoi(tokens[1]);
            } catch (std::invalid_argument&) {
                fmt::println(stderr, "warn: expected integer (32-bit) for x");

                return;
            } catch (std::out_of_range&) {
                fmt::println(stderr, "warn: expected integer (32-bit) for x");

                return;
            }

            int y {};

            try {
                y = std::stoi(tokens[2]);
            } catch (std::invalid_argument&) {
                fmt::println(stderr, "warn: expected integer (32-bit) for y");

                return;
            } catch (std::out_of_range&) {
                fmt::println(stderr, "warn: expected integer (32-bit) for y");

                return;
            }

            int w {};

            try {
                w = std::stoi(tokens[3]);
            } catch (std::invalid_argument&) {
                fmt::println(stderr, "warn: expected integer (32-bit) for w");

                return;
            } catch (std::out_of_range&) {
                fmt::println(stderr, "warn: expected integer (32-bit) for w");

                return;
            }

            int h {};

            try {
                h = std::stoi(tokens[4]);
            } catch (std::invalid_argument&) {
                fmt::println(stderr, "warn: expected integer (32-bit) for h");

                return;
            } catch (std::out_of_range&) {
                fmt::println(stderr, "warn: expected integer (32-bit) for h");

                return;
            }

            RectangleDraw draw { m_colour, x, y, w, h };

            Action action { draw };

            if (m_selected_id) {
                action = Select { *m_selected_id, draw };
            }

            {
                threading::mutex_guard guard { share::writer_mutex };

                share::writer_queue.push(action);
            }

            share::writer_cond.notify_one();
        } break;
        case Option::CIRCLE: {
            if (tokens.size() != 4) {
                fmt::println(
                    stderr,
                    "warn: an unexpected number of tokens for draw with circle "
                    "selected"
                );

                return;
            }

            int x {};

            try {
                x = std::stoi(tokens[1]);
            } catch (std::invalid_argument&) {
                fmt::println(stderr, "warn: expected integer (32-bit) for x");

                return;
            } catch (std::out_of_range&) {
                fmt::println(stderr, "warn: expected integer (32-bit) for x");

                return;
            }

            int y {};

            try {
                y = std::stoi(tokens[2]);
            } catch (std::invalid_argument&) {
                fmt::println(stderr, "warn: expected integer (32-bit) for y");

                return;
            } catch (std::out_of_range&) {
                fmt::println(stderr, "warn: expected integer (32-bit) for y");

                return;
            }

            float r {};

            try {
                r = std::stof(tokens[3]);
            } catch (std::invalid_argument&) {
                fmt::println(stderr, "warn: expected float (32-bit) for r");

                return;
            } catch (std::out_of_range&) {
                fmt::println(stderr, "warn: expected float (32-bit) for r");

                return;
            }

            if (0 > r) {
                fmt::println(
                    stderr,
                    "warn: expected positive float (32-bit) for r"
                );

                return;
            }

            CircleDraw draw { m_colour, x, y, r };

            Action action { draw };

            if (m_selected_id) {
                action = Select { *m_selected_id, draw };
            }

            {
                threading::mutex_guard guard { share::writer_mutex };

                share::writer_queue.push(action);
            }

            share::writer_cond.notify_one();
        } break;
        case Option::TEXT: {
            if (tokens.size() != 4) {
                fmt::println(
                    stderr,
                    "warn: an unexpected number of tokens for draw with text "
                    "selected"
                );

                return;
            }

            int x {};

            try {
                x = std::stoi(tokens[1]);
            } catch (std::invalid_argument&) {
                fmt::println(stderr, "warn: expected integer (32-bit) for x");

                return;
            } catch (std::out_of_range&) {
                fmt::println(stderr, "warn: expected integer (32-bit) for x");

                return;
            }

            int y {};

            try {
                y = std::stoi(tokens[2]);
            } catch (std::invalid_argument&) {
                fmt::println(stderr, "warn: expected integer (32-bit) for y");

                return;
            } catch (std::out_of_range&) {
                fmt::println(stderr, "warn: expected integer (32-bit) for y");

                return;
            }

            std::string text { tokens[3] };

            TextDraw draw { m_colour, x, y, text };

            Action action { draw };

            if (m_selected_id) {
                action = Select { *m_selected_id, draw };
            }

            {
                threading::mutex_guard guard { share::writer_mutex };

                share::writer_queue.push(action);
            }

            share::writer_cond.notify_one();
        } break;
        default:
            ABORT("unreachable");
        }

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

        bool second_match { false };

        Option tool_qual { Option::ALL };

        if (second_token == "all") {
            tool_qual = Option::ALL;
            second_match = true;
        }

        if (second_token == "line") {
            tool_qual = Option::LINE;
            second_match = true;
        }

        if (second_token == "rectangle") {
            tool_qual = Option::RECTANGLE;
            second_match = true;
        }

        if (second_token == "circle") {
            tool_qual = Option::CIRCLE;
            second_match = true;
        }

        if (second_token == "text") {
            tool_qual = Option::TEXT;
            second_match = true;
        }

        if (!second_match) {
            fmt::println(
                stderr,
                "warn: expected one of {{'all' | 'line' | 'rectangle' | "
                "'circle' | 'text'}}"
            );

            return;
        }

        std::string_view third_token = tokens[2];

        bool third_match { false };

        Option user_qual { Option::ALL };

        if (third_token == "all") {
            user_qual = Option::ALL;
            third_match = true;
        }

        if (third_token == "mine") {
            user_qual = Option::MINE;
            third_match = true;
        }

        if (!third_match) {
            fmt::println(stderr, "warn: expected one of {{'all' | 'mine'}}");

            return;
        }

        for (;;) {
            {
                threading::unique_rwlock_rdguard guard {
                    share::rwlock1,
                    threading::unique_guard_policy::try_to_lock
                };

                if (guard.is_owning()) {
                    list_draws(tool_qual, user_qual, share::vec1);

                    return;
                }
            }

            {
                threading::unique_rwlock_rdguard guard {
                    share::rwlock2,
                    threading::unique_guard_policy::try_to_lock
                };

                if (guard.is_owning()) {
                    list_draws(tool_qual, user_qual, share::vec2);

                    return;
                }
            }
        }

        return;
    }

    if (first_token == "select") {
        if (tokens.size() != 2) {
            fmt::println(
                stderr,
                "warn: an unexpected number of tokens for select"
            );

            return;
        }

        std::string_view second_token = tokens[1];

        if (second_token == "none") {
            m_selected_id = {};

            return;
        }

        long id { 0 };

        bool id_has_error { false };

        try {
            id = std::stol(std::string { second_token });
        } catch (std::invalid_argument&) {
            id_has_error = true;
        } catch (std::out_of_range&) {
            id_has_error = true;
        }

        if (id_has_error) {
            fmt::println(
                stderr,
                "warn: expected an 'none' or an integer greater than or equal "
                "to 0 for ID"
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
                "warn: an unexpected number of tokens for delete"
            );

            return;
        }

        std::string_view second_token = tokens[1];

        long id { 0 };

        bool id_has_error { false };

        try {
            id = std::stol(std::string { second_token });
        } catch (std::invalid_argument&) {
            id_has_error = true;
        } catch (std::out_of_range&) {
            id_has_error = true;
        }

        if (id_has_error) {
            fmt::println(
                stderr,
                "warn: expected an integer greater than or equal to 0 for ID"
            );

            return;
        }

        {
            threading::mutex_guard guard { share::writer_mutex };

            share::writer_queue.emplace(Delete { id });
        }

        share::writer_cond.notify_one();

        return;
    }

    if (first_token == "undo") {
        if (tokens.size() != 1) {
            fmt::println(
                stderr,
                "warn: an unexpected number of tokens for undo"
            );

            return;
        }

        {
            threading::mutex_guard guard { share::writer_mutex };

            share::writer_queue.emplace(Undo {});
        }

        share::writer_cond.notify_one();

        return;
    }

    if (first_token == "clear") {
        if (tokens.size() != 2) {
            fmt::println(
                stderr,
                "warn: an unexpected number of tokens for clear"
            );

            return;
        }

        std::string_view second_token = tokens[1];

        if (second_token == "all") {
            {
                threading::mutex_guard guard { share::writer_mutex };

                share::writer_queue.emplace(Clear { Qualifier::ALL });
            }

            share::writer_cond.notify_one();

            return;
        }

        if (second_token == "mine") {
            {
                threading::mutex_guard guard { share::writer_mutex };

                share::writer_queue.emplace(Clear { Qualifier::MINE });
            }

            share::writer_cond.notify_one();

            return;
        }

        fmt::println(stderr, "warn: expected one of {{'all' | 'mine'}}");

        return;
    }

    if (first_token == "show") {
        if (tokens.size() != 2) {
            fmt::println(
                stderr,
                "warn: an unexpected number of tokens for show"
            );

            return;
        }

        std::string_view second_token = tokens[1];

        if (second_token == "all") {
            share::show_mine = false;

            return;
        }

        if (second_token == "mine") {
            share::show_mine = true;

            return;
        }

        fmt::println(stderr, "warn: expected one of {{'all' | 'mine'}}");

        return;
    }

    if (first_token == "exit") {
        if (tokens.size() != 1) {
            fmt::println(
                stderr,
                "warn: an unexpected number of tokens for exit"
            );

            return;
        }

        m_should_exit = true;

        return;
    }

    fmt::println(stderr, "warn: {} is not a known command", first_token);
}

} // namespace client
