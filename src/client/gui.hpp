#pragma once

// raylib
#include <raylib.h>

// std
#include <atomic>

// client
#include <log_file.hpp>

namespace client {

class gui_t {
   public:
    explicit gui_t(std::atomic_bool& stop, bool log = true);

    void operator()();

   private:
    static void noop_logger(int, const char*, va_list);

    static void file_logger(
        int msg_type,
        const char* text,
        va_list args
    );

    inline void draw_scene();
    void draw();
    void game_loop();

    Camera2D m_camera{{0, 0}, {0, 0}, 0, 1.0};

    const std::string m_window_name{"NetSketch Whiteboard"};
    const int m_target_fps{60};

    const int m_screen_width{800};
    const int m_screen_height{450};

    std::atomic_bool& a_stop;

    bool m_log;

    static common::log_file_t s_log_file;
};

}  // namespace client
