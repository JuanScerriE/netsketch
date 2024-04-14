// client
#include <gui.hpp>

// std
#include <chrono>
#include <cmath>
#include <typeinfo>
#include <variant>

// raylib
#include <raylib.h>
#include <raymath.h>

// fmt
#include <fmt/chrono.h>
#include <fmt/core.h>

// common
#include <abort.hpp>
#include <types.hpp>

// share
#include <share.hpp>

namespace client {

common::log_file_t gui_t::s_log_file{};

gui_t::gui_t(bool log)
    : m_log(log) {
}

void gui_t::operator()() {
    AbortIf(
        m_in_game_loop,
        "calling operator()() twice on a gui_t object"
    );

    SetTraceLogCallback(noop_logger);

    if (m_log) {
        using std::chrono::system_clock;

        auto now = system_clock::now();

        s_log_file.open(fmt::format(
            "netsketch-raylib-log {:%Y-%m-%d %H:%M:%S}",
            now
        ));

        if (s_log_file.error()) {
            fmt::println(
                stderr,
                "warn: opening a log file failed because - "
                "{}",
                s_log_file.reason()
            );
        } else {
            SetTraceLogCallback(file_logger);
        }
    }

    game_loop();

    if (m_log) {
        if (s_log_file.is_open()) {
            s_log_file.close();
        }
    }
}

void gui_t::noop_logger(int, const char *, va_list) {
    // do nothing
}

void gui_t::file_logger(
    int msg_type,
    const char *text,
    va_list args
) {
    // TODO: add error handling for each call to printf (and
    // friends)
    switch (msg_type) {
        case LOG_INFO:
            fprintf(
                gui_t::s_log_file.native_handle(),
                "[INFO]: "
            );
            break;
        case LOG_ERROR:
            fprintf(
                gui_t::s_log_file.native_handle(),
                "[ERROR]: "
            );
            break;
        case LOG_WARNING:
            fprintf(
                gui_t::s_log_file.native_handle(),
                "[WARN] : "
            );
            break;
        case LOG_DEBUG:
            fprintf(
                gui_t::s_log_file.native_handle(),
                "[DEBUG]: "
            );
            break;
        default:
            break;
    }

    vfprintf(gui_t::s_log_file.native_handle(), text, args);

    fprintf(gui_t::s_log_file.native_handle(), "\n");
}

inline void gui_t::draw_scene() {
    for (auto &draw : m_draws) {
        if (std::holds_alternative<common::line_draw_t>(draw
            )) {
            auto &line =
                std::get<common::line_draw_t>(draw);

            DrawLineEx(
                {static_cast<float>(line.x0),
                 static_cast<float>(line.y0)},
                {static_cast<float>(line.x1),
                 static_cast<float>(line.y1)},
                1.1f,  // NOTE: maybe change this?
                line.colour.to_raylib_colour()
            );
            continue;
        }
        if (std::holds_alternative<
                common::rectangle_draw_t>(draw)) {
            auto &rectangle =
                std::get<common::rectangle_draw_t>(draw);
            DrawRectangle(
                rectangle.x,
                rectangle.y,
                rectangle.w,
                rectangle.h,
                rectangle.colour.to_raylib_colour()
            );
            continue;
        }
        if (std::holds_alternative<common::circle_draw_t>(
                draw
            )) {
            auto &circle =
                std::get<common::circle_draw_t>(draw);
            DrawCircle(
                circle.x,
                circle.y,
                circle.r,
                circle.colour.to_raylib_colour()
            );
            continue;
        }
        if (std::holds_alternative<common::text_draw_t>(draw
            )) {
            auto &text =
                std::get<common::text_draw_t>(draw);
            DrawText(
                text.string.c_str(),
                text.x,
                text.y,
                16,  // NOTE: maybe change this?
                text.colour.to_raylib_colour()
            );
            continue;
        }

        using Type = std::decay_t<decltype(draw)>;

        AbortV("unkown draw type {}", typeid(Type).name());
    }
}

void gui_t::draw() {
    BeginDrawing();
    {
        Vector2 target_world_pos =
            GetScreenToWorld2D({0, 0}, m_camera);

        Vector2 mouse_world_pos = GetScreenToWorld2D(
            GetMousePosition(),
            m_camera
        );

        ClearBackground(WHITE);

        BeginMode2D(m_camera);
        { draw_scene(); }
        EndMode2D();

        int gap = static_cast<int>(50 * m_camera.zoom);
        int width = GetScreenWidth();
        float left = target_world_pos.x;
        // round to the nearest 100
        left = 100 * roundf(left / 100);
        int screenLeft = static_cast<int>(
            GetWorldToScreen2D({left, 0}, m_camera).x
        );
        int widthSteps = width / gap;

        for (int x = 0; x <= widthSteps; x++) {
            DrawLine(
                screenLeft + (x * gap),
                0,
                screenLeft + x * gap,
                20,
                GRAY
            );
        }

        int height = GetScreenHeight();
        float top = target_world_pos.y;
        // round to the nearest 100
        top = 100 * roundf(top / 100);
        int screenTop = static_cast<int>(
            GetWorldToScreen2D({0, top}, m_camera).y
        );
        int heightSteps = height / gap;

        for (int y = 0; y <= heightSteps; y++) {
            DrawLine(
                0,
                screenTop + y * gap,
                20,
                screenTop + y * gap,
                GRAY
            );
        }

        DrawText(
            TextFormat(
                "X:%f, Y:%f, ZOOM:%f",
                mouse_world_pos.x,
                mouse_world_pos.y,
                m_camera.zoom
            ),
            30,
            height - 25,
            20,
            BLACK
        );
    }
    EndDrawing();
}

void gui_t::game_loop() {
    // mark as in_game_loop
    m_in_game_loop = true;

    // window configuration flags
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(
        m_screen_width,
        m_screen_height,
        m_window_name.c_str()
    );

    SetTargetFPS(m_target_fps);

    // detect window close button or ESC key
    // or when the main tells us so
    while (!share::e_stop_gui()) {
        if (IsKeyPressed(KEY_H)) {
            m_camera = {{0, 0}, {0, 0}, 0, 1.0};
        }

        // translate based on mouse left click
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 delta = GetMouseDelta();
            delta =
                Vector2Scale(delta, -1.0f / m_camera.zoom);

            m_camera.target =
                Vector2Add(m_camera.target, delta);
        }

        // zoom based on mouse wheel
        float wheel = GetMouseWheelMove();

        if (wheel != 0) {
            // get the world point that is under the mouse
            Vector2 mouseWorldPos = GetScreenToWorld2D(
                GetMousePosition(),
                m_camera
            );

            // get the offset to where the mouse is
            m_camera.offset = GetMousePosition();

            // set the target to match, so that the m_camera
            // maps the world space point under the cursor
            // to the screen space point under the cursor at
            // any zoom
            m_camera.target = mouseWorldPos;

            // Zoom increment
            const float zoomIncrement = 0.125f;

            m_camera.zoom += (wheel * zoomIncrement);

            if (m_camera.zoom < zoomIncrement)
                m_camera.zoom = zoomIncrement;
        }

        if (IsWindowResized()) {
            TraceLog(
                LOG_INFO,
                "Width: %d, Height: %d",
                GetScreenWidth(),
                GetScreenHeight()
            );
        }

        draw();
    }

    CloseWindow();  // Close window and OpenGL context
}

}  // namespace client

/*******************************************************************************************
 *
 *   raylib [core] example - 2d camera mouse zoom
 *
 *   Example originally created with raylib 4.2, last time
 *updated with raylib 4.2
 *
 *   Example licensed under an unmodified zlib/libpng
 *license, which is an OSI-certified, BSD-like license that
 *allows static linking with closed source software
 *
 *   Copyright (c) 2022-2024 Jeffery Myers (@JeffM2501)
 *
 ********************************************************************************************/
