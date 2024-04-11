#pragma once

// raylib
#include <raylib.h>

namespace client {

class gui_t {
   public:
    inline void draw_scene();
    void draw();
    void game_loop();
    void operator()();

   private:
    Camera2D m_camera{{0, 0}, {0, 0}, 0, 1.0};

    const char *m_window_name{"hello"};
    const int m_traget_fps{60};
    const int m_screen_width{800};
    const int m_screen_height{450};
};

}  // namespace client
