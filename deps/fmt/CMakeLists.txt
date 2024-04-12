message(STATUS "Fetching fmt...")

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 10.2.1
        GIT_PROGRESS true
        SYSTEM
)

FetchContent_MakeAvailable(fmt)
