#pragma once

// raylib
#include <raylib.h>

// client
#include <log_file.hpp>

// prot
#include <protocol.hpp>

// std
#include <vector>

// common
#include <types.hpp>

namespace client {

class gui_t {
public:
    void operator()();

private:
    static void noop_logger(int, const char*, va_list);

    static void file_logger(
        int msg_type, const char* text, va_list args);

    inline void process_draw(prot::draw_t& draw);
    inline void draw_scene();
    void draw();
    void game_loop();

    // init
    bool m_in_game_loop { false };

    // defaults
    Camera2D m_camera { { 0, 0 }, { 0, 0 }, 0, 1.0 };
    const std::string m_window_name {
        "NetSketch Whiteboard"
    };
    const int m_target_fps { 60 };
    const int m_screen_width { 800 };
    const int m_screen_height { 450 };
};

} // namespace client
