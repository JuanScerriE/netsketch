message(STATUS "Fetching cereal...")

FetchContent_Declare(
        cereal
        GIT_REPOSITORY https://github.com/USCiLab/cereal
        GIT_TAG v1.3.2
        GIT_PROGRESS true
        SYSTEM
)

FetchContent_MakeAvailable(cereal)
