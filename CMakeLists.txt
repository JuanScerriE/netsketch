cmake_minimum_required(VERSION 3.15)

project(
        netsketch
        VERSION 0.1
        LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(deps)
add_subdirectory(src)
