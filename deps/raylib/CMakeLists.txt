message(STATUS "Fetching raylib...")

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES OFF CACHE BOOL "" FORCE) # don't build the supplied example games

if (DEFINED ENV{WAYLAND_DISPLAY})
    message("-- Using Wayland (and turning off X11)...")
    set(GLFW_BUILD_X11 OFF CACHE BOOL "" FORCE) # don't use x11 please
    set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE) # use wayland please
endif ()

FetchContent_Declare(
        raylib
        GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
        GIT_TAG 5.0
        GIT_PROGRESS true
        SYSTEM
)

FetchContent_MakeAvailable(raylib)
