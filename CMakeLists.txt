cmake_minimum_required(VERSION 3.20)

project(
  parl
  VERSION 0.1
  LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(deps)
add_subdirectory(src)
