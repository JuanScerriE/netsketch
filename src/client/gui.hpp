#pragma once

// raylib
#include <raylib.h>

// common
#include "../common/log.hpp"
#include "../common/types.hpp"

namespace client {

class Gui {
   public:
    void operator()();

   private:
    void process_draw(Draw& draw);

    void draw_scene();

    void draw();

    void game_loop();

    Camera2D m_camera { { 0, 0 }, { 0, 0 }, 0, 1.0 };

    // defaults
    const std::string m_window_name { "NetSketch Whiteboard" };
    const int m_target_fps { 60 };
    const int m_screen_width { 800 };
    const int m_screen_height { 450 };

    // logging
    static logging::log log;

    static void setup_logging();

    static void logger_wrapper(int msg_type, const char* fmt, va_list args);
};

} // namespace client
