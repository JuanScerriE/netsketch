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

#include <fmt/core.h>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main() {
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE
    );  // Window configuration flags
    InitWindow(
        screenWidth,
        screenHeight,
        "raylib [core] example - 2d camera mouse zoom"
    );

    Camera2D camera = {0};
    camera.zoom = 1.0f;

    SetTargetFPS(60
    );  // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()
    )  // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Translate based on mouse right click
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 delta = GetMouseDelta();
            delta =
                Vector2Scale(delta, -1.0f / camera.zoom);

            camera.target =
                Vector2Add(camera.target, delta);
        }

        // Get the world point that is under the mouse
        Vector2 mouseWorldPos =
            GetScreenToWorld2D(GetMousePosition(), camera);

        // Zoom based on mouse wheel
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            // Set the offset to where the mouse is
            camera.offset = GetMousePosition();

            // Set the target to match, so that the camera
            // maps the world space point under the cursor
            // to the screen space point under the cursor at
            // any zoom
            camera.target = mouseWorldPos;

            // Zoom increment
            const float zoomIncrement = 0.125f;

            camera.zoom += (wheel * zoomIncrement);
            if (camera.zoom < zoomIncrement)
                camera.zoom = zoomIncrement;
        }

        if (IsWindowResized()) {
            TraceLog(
                LOG_INFO,
                "width:%d, height:%d",
                GetScreenWidth(),
                GetScreenHeight()
            );
        }

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        {
            ClearBackground(WHITE);

            BeginMode2D(camera);
            {
                // Draw the 3d grid, rotated 90 degrees and
                // centered around 0,0 just so we have
                // something in the XY plane
                // rlPushMatrix();
                // rlTranslatef(0, 25 * 50, 0);
                // rlRotatef(90, 1, 0, 0);
                // DrawGrid(100, 50);
                // rlPopMatrix();

                // Draw a reference circle
                DrawCircle(100, 100, 50, YELLOW);

                int gap = 50;

                int width = GetScreenWidth();
                int widthSteps = width / gap;

                for (int x = 0; x < widthSteps; x++) {
                    DrawLine(
                        x * gap,
                        0,
                        x * gap,
                        20,
                        BLACK
                    );
                }

                int height = GetScreenHeight();
                int heightSteps = height / gap;

                for (int y = 0; y < heightSteps; y++) {
                    DrawLine(
                        0,
                        y * gap,
                        20,
                        y * gap,
                        BLACK
                    );
                }
            }
            EndMode2D();

            DrawText(
                fmt::format(
                    "X:{}, Y:{}",
                    mouseWorldPos.x,
                    mouseWorldPos.y
                )
                    .c_str(),
                10,
                10,
                20,
                BLACK
            );
        }
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    return 0;
}
