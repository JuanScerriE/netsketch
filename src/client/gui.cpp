// client
#include "gui.hpp"
#include "share.hpp"

// common
#include "../common/overload.hpp"
#include "../common/threading.hpp"
#include "../common/types.hpp"

// raylib
#include <raylib.h>
#include <raymath.h>

namespace client {

void Gui::operator()()
{
    setup_logging();

    game_loop();
}

[[nodiscard]] Color to_raylib_colour(Colour colour)
{
    return { colour.r, colour.g, colour.b, 255 };
}

void Gui::process_draw(Draw& draw)
{
    std::visit(
        overload {
            [](TextDraw& arg) {
                DrawText(
                    arg.string.c_str(),
                    arg.x,
                    arg.y,
                    20, // NOTE: maybe change this?
                    to_raylib_colour(arg.colour)
                );
            },
            [](CircleDraw& arg) {
                DrawCircle(arg.x, arg.y, arg.r, to_raylib_colour(arg.colour));
            },
            [](RectangleDraw& arg) {
                DrawRectangle(
                    arg.x,
                    arg.y,
                    arg.w,
                    arg.h,
                    to_raylib_colour(arg.colour)
                );
            },
            [](LineDraw& arg) {
                DrawLineEx(
                    { static_cast<float>(arg.x0), static_cast<float>(arg.y0) },
                    { static_cast<float>(arg.x1), static_cast<float>(arg.y1) },
                    1.1f, // NOTE: maybe change this?
                    to_raylib_colour(arg.colour)
                );
            },
        },
        draw
    );
}

inline void Gui::draw_scene()
{
    for (;;) {
        {
            threading::unique_rwlock_rdguard guard {
                share::rwlock1,
                threading::unique_guard_policy::try_to_lock
            };

            if (guard.is_owning()) {
                if (share::show_mine) {
                    for (auto& tagged_draw : share::vec1) {
                        if (tagged_draw.username == share::username && !tagged_draw.adopted)
                            process_draw(tagged_draw.draw);
                    }
                } else {
                    for (auto& tagged_draw : share::vec1) {
                        process_draw(tagged_draw.draw);
                    }
                }

                return;
            }
        }

        {
            threading::unique_rwlock_rdguard guard {
                share::rwlock2,
                threading::unique_guard_policy::try_to_lock
            };

            if (guard.is_owning()) {
                if (share::show_mine) {
                    for (auto& tagged_draw : share::vec2) {
                        if (tagged_draw.username == share::username && !tagged_draw.adopted)
                            process_draw(tagged_draw.draw);
                    }
                } else {
                    for (auto& tagged_draw : share::vec2) {
                        process_draw(tagged_draw.draw);
                    }
                }

                return;
            }
        }
    }
}

void Gui::draw()
{
    BeginDrawing();
    {
        Vector2 target_world_pos = GetScreenToWorld2D({ 0, 0 }, m_camera);

        Vector2 mouse_world_pos
            = GetScreenToWorld2D(GetMousePosition(), m_camera);

        ClearBackground(WHITE);

        BeginMode2D(m_camera);
        {
            draw_scene();
        }
        EndMode2D();

        int gap = static_cast<int>(50 * m_camera.zoom);
        int width = GetScreenWidth();
        float left = target_world_pos.x;
        // round to the nearest 100
        left = 100 * roundf(left / 100);
        int screenLeft
            = static_cast<int>(GetWorldToScreen2D({ left, 0 }, m_camera).x);
        int widthSteps = width / gap;

        for (int x = 0; x <= widthSteps; x++) {
            DrawLine(screenLeft + (x * gap), 0, screenLeft + x * gap, 20, GRAY);
        }

        int height = GetScreenHeight();
        float top = target_world_pos.y;
        // round to the nearest 100
        top = 100 * roundf(top / 100);
        int screenTop
            = static_cast<int>(GetWorldToScreen2D({ 0, top }, m_camera).y);
        int heightSteps = height / gap;

        for (int y = 0; y <= heightSteps; y++) {
            DrawLine(0, screenTop + y * gap, 20, screenTop + y * gap, GRAY);
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

void Gui::game_loop()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(m_screen_width, m_screen_height, m_window_name.c_str());

    SetTargetFPS(m_target_fps);

    while (share::run_gui) {
        if (IsKeyPressed(KEY_H)) {
            m_camera = { { 0, 0 }, { 0, 0 }, 0, 1.0 };
        }

        // translate based on mouse left click
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0f / m_camera.zoom);

            m_camera.target = Vector2Add(m_camera.target, delta);
        }

        // zoom based on mouse wheel
        float wheel = GetMouseWheelMove();

        if (wheel != 0) {
            // get the world point that is under the
            // mouse
            Vector2 mouseWorldPos
                = GetScreenToWorld2D(GetMousePosition(), m_camera);

            // get the offset to where the mouse is
            m_camera.offset = GetMousePosition();

            // set the target to match, so that the
            // m_camera maps the world space point under
            // the cursor to the screen space point
            // under the cursor at any zoom
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

    CloseWindow(); // Close window and OpenGL context
}

logging::log Gui::log {};

void Gui::setup_logging()
{
    using namespace logging;

    log.set_level(log::level::debug);

    log.set_prefix("[raylib]");

    log.set_file(share::log_file);

    SetTraceLogCallback(logger_wrapper);
}

void Gui::logger_wrapper(int msg_type, const char* fmt, va_list args)
{
    using namespace logging;

    log::level log_level { log::level::info };

    switch (msg_type) {
    case LOG_DEBUG:
        log_level = log::level::debug;
        break;
    case LOG_INFO:
        log_level = log::level::info;
        break;
    case LOG_WARNING:
        log_level = log::level::warn;
        break;
    case LOG_ERROR:
        log_level = log::level::error;
        break;
    default:
        break;
    }

    log.c_write(log_level, fmt, args);

    log.flush();
}

} // namespace client

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
