message(STATUS "Fetching cereal...")

set(BUILD_DOC OFF CACHE BOOL "" FORCE)
set(BUILD_SANDBOX OFF CACHE BOOL "" FORCE)
set(SKIP_PERFORMANCE_COMPARISON ON CACHE BOOL "" FORCE)

FetchContent_Declare(
        cereal
        GIT_REPOSITORY https://github.com/USCiLab/cereal
        GIT_TAG v1.3.2
        GIT_PROGRESS true
        SYSTEM
)

FetchContent_MakeAvailable(cereal)
