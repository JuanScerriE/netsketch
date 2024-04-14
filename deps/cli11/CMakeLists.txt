message(STATUS "Fetching cli11...")

FetchContent_Declare(
        cli11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG v2.4.1
        GIT_PROGRESS true
        SYSTEM
)

FetchContent_MakeAvailable(cli11)
