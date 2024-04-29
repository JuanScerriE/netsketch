message(STATUS "Fetching cereal...")

FetchContent_Declare(
        cereal
        GIT_REPOSITORY https://github.com/USCiLab/cereal
        GIT_TAG v1.3.2
        GIT_PROGRESS true
        SYSTEM
)

set(BUILD_DOC OFF)
set(BUILD_SANDBOX OFF)
set(SKIP_PERFORMANCE_COMPARISON ON)

FetchContent_MakeAvailable(cereal)
