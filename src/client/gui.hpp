#pragma once

// raylib
#include <raylib.h>

// client
#include <log_file.hpp>

// std
#include <vector>

// common
#include <types.hpp>

namespace client {

class gui_t {
public:
    gui_t(const gui_t&) = delete;

    gui_t& operator=(const gui_t&) = delete;

    explicit gui_t(bool log = true);

    void operator()();

private:
    static void noop_logger(int, const char*, va_list);
    static void file_logger(
        int msg_type, const char* text, va_list args);

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

    // file logging
    bool m_log;
    static common::log_file_t s_log_file;

    // draws
    std::vector<common::draw_t> m_draws {
        common::rectangle_draw_t {
            { 0, 255, 0 }, 100, 100, 200, 150 },
        common::circle_draw_t {
            { 255, 255, 0 },
            150,
            150,
            30.5,
        },
        common::line_draw_t {
            { 0, 0, 0 },
            150,
            150,
            200,
            200,
        },
        common::text_draw_t {
            { 0, 0, 0 },
            20,
            20,
            "this is some example text",
        },
    };
};

} // namespace client
