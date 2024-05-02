#pragma once

// common
#include "../common/types.hpp"

// cstd
#include <cstdint>

// NOTE: that a limitation of Cereal is that it doesn't
// really help you a lot if you want a human readable format
// which you can modify and de-serialize. These
// because you need to augment the serialize method
// that Cereal expects with a CEREAL_NVP macro
// call to give the produced JSON human readable names.
// But still probably dealing with variants is not easy
// since C++ internally uses indices.

namespace test_client {

Colour pick_random_colour();

std::string generate_random_string();

LineDraw random_line_draw();
RectangleDraw random_rectangle_draw();
CircleDraw random_circle_draw();
TextDraw random_text_draw();

Action generate_random_action();

void simulate_behaviour(uint32_t iterations, double interval);

} // namespace test_client
