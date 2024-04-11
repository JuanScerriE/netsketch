// client
#include <gui.hpp>

// std
#include <cmath>

// raylib
#include <raylib.h>
#include <raymath.h>

namespace client {

inline void gui_t::draw_scene() {
    DrawCircle(0, 0, 50, YELLOW);
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
    // window configuration flags
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(
        m_screen_width,
        m_screen_height,
        m_window_name
    );

    SetTargetFPS(m_traget_fps);

    // detect window close button or ESC key
    while (!WindowShouldClose()) {
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
                "width:%d, height:%d",
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
