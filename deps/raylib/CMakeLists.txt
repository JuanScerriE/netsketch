message(STATUS "Fetching raylib...")

FetchContent_Declare(
        raylib
        GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
        GIT_TAG 5.0
        GIT_PROGRESS true
        SYSTEM
)

set(BUILD_EXAMPLES OFF) # don't build the supplied examples
set(BUILD_GAMES OFF) # don't build the supplied example games

if (DEFINED ENV{WAYLAND_DISPLAY})
    message("-- Using Wayland (and turning off X11)...")
    set(GLFW_BUILD_X11 OFF) # don't use x11 please
    set(GLFW_BUILD_WAYLAND ON) # use wayland please
endif ()

FetchContent_MakeAvailable(raylib)
