// test_client
#include "simulate_user.hpp"
#include "share.hpp"

// cstd
#include <cstdlib>
#include <ctime>

// unix
#include <unistd.h>

// common
#include "../common/abort.hpp"
#include "../common/types.hpp"

// std
#include <random>

namespace test_client {

static std::random_device rd {};

static std::mt19937 mt { rd() };

static std::uniform_int_distribution<> char_dist { 0, 255 };

static std::uniform_int_distribution<> x_boundary_dist { -2000, 2000 };
static std::uniform_int_distribution<> y_boundary_dist { -2000, 2000 };

static std::uniform_int_distribution<> length_dist { 1, 64 };

Colour pick_random_colour()
{
    Colour colour {};

    colour.r = static_cast<uint8_t>(char_dist(mt));
    colour.g = static_cast<uint8_t>(char_dist(mt));
    colour.b = static_cast<uint8_t>(char_dist(mt));

    return colour;
}

LineDraw random_line_draw()
{
    LineDraw draw {};

    draw.colour = pick_random_colour();
    draw.x0 = x_boundary_dist(mt);
    draw.y0 = y_boundary_dist(mt);
    draw.x1 = x_boundary_dist(mt);
    draw.y1 = y_boundary_dist(mt);

    return draw;
}

RectangleDraw random_rectangle_draw()
{
    RectangleDraw draw {};

    draw.colour = pick_random_colour();
    draw.x0 = x_boundary_dist(mt);
    draw.y0 = y_boundary_dist(mt);
    draw.x1 = x_boundary_dist(mt);
    draw.y1 = y_boundary_dist(mt);

    return draw;
}

CircleDraw random_circle_draw()
{
    CircleDraw draw {};

    draw.colour = pick_random_colour();
    draw.x = x_boundary_dist(mt);
    draw.y = y_boundary_dist(mt);
    draw.r = static_cast<float>(char_dist(mt));

    return draw;
}

std::string generate_random_string()
{
    int length = length_dist(mt);

    std::string string {};

    string.reserve(static_cast<size_t>(length));

    for (int i = 0; i < length; i++) {
        // NOTE: we are using 'a'-'z' as our list cause the
        // olive.c supports a very limited character set
        string.push_back(std::clamp(static_cast<char>(char_dist(mt)), 'a', 'z')
        );
    }

    return string;
}

TextDraw random_text_draw()
{
    TextDraw draw {};

    draw.colour = pick_random_colour();
    draw.x = x_boundary_dist(mt);
    draw.y = y_boundary_dist(mt);
    draw.string = generate_random_string();

    return draw;
}

Action generate_random_action()
{
    std::uniform_int_distribution<> distrib { 0,
                                              (share::only_drawing) ? 3 : 7 };

    int choice = distrib(mt);

    switch (choice) {
    case 0:
        return random_line_draw();
    case 1:
        return random_rectangle_draw();
    case 2:
        return random_circle_draw();
    case 3:
        return random_text_draw();
    case 4:
        return Clear { Qualifier::MINE };
    case 5:
        return Clear { Qualifier::ALL };
    case 6:
        return Undo {};
    case 7: {
        std::uniform_int_distribution<> distrib(0, 255);

        return Delete { distrib(mt) };
    } break;
    default:
        ABORT("unreachable");
    }
}

void simulate_behaviour(uint32_t iterations, double interval)
{
    struct timespec spec { };

    // all this arithmetic can definitely mess stuff up
    spec.tv_sec = static_cast<time_t>(interval);

    uint64_t nanosecs = static_cast<uint64_t>(interval * 1e9)
                        - static_cast<uint64_t>(interval);

    spec.tv_nsec
        = (nanosecs < 999999999) ? static_cast<long>(nanosecs) : 999999998;

    for (uint32_t i = 0; i < iterations; i++) {
        nanosleep(&spec, nullptr);

        {
            threading::mutex_guard guard { share::writer_mutex };

            share::writer_queue.push(generate_random_action());
        }

        share::writer_cond.notify_one();
    }
}

} // namespace test_client
