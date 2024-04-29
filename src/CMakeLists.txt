add_executable(netsketch_server
        server/main.cpp
        server/share.cpp
        server/server.cpp
        server/conn_handler.cpp
)

if (BENCHMARK)
    message("-- Adding timers...")

    target_compile_definitions(netsketch_server PRIVATE BENCHMARK)
endif ()

target_link_libraries(netsketch_server PRIVATE
        CLI11::CLI11
        fmt::fmt
        cereal::cereal
        spdlog::spdlog
)
target_compile_options(netsketch_server PRIVATE -Wall -Wextra -Wpedantic -Weffc++ -Wconversion)

#---------------------------------

add_executable(netsketch_client
        client/main.cpp
        client/share.cpp
        client/gui.cpp
        client/input_handler.cpp
        client/input_parser.cpp
        client/network_manager.cpp
        client/reader.cpp
        client/writer.cpp
)

if (BENCHMARK)
    message("-- Adding timers...")

    target_compile_definitions(netsketch_client PRIVATE BENCHMARK)
endif ()

# Checks if OSX and links appropriate frameworks (only required on MacOS)
if (APPLE)
    target_link_libraries(netsketch_client PRIVATE "-framework IOKit")
    target_link_libraries(netsketch_client PRIVATE "-framework Cocoa")
    target_link_libraries(netsketch_client PRIVATE "-framework OpenGL")
endif ()

target_link_libraries(netsketch_client PRIVATE
        CLI11::CLI11
        fmt::fmt
        cereal::cereal
        spdlog::spdlog
        raylib
)
target_compile_options(netsketch_client PRIVATE -Wall -Wextra -Wpedantic -Weffc++ -Wconversion)

add_executable(netsketch_test_client
        test_client/main.cpp
        test_client/share.cpp
        test_client/network_manager.cpp
        test_client/reader.cpp
        test_client/writer.cpp
)

if (BENCHMARK)
    message("-- Adding timers...")

    target_compile_definitions(netsketch_test_client PRIVATE BENCHMARK)
endif ()

target_link_libraries(netsketch_test_client PRIVATE
        CLI11::CLI11
        fmt::fmt
        cereal::cereal
        spdlog::spdlog
)
target_compile_options(netsketch_test_client PRIVATE -Wall -Wextra -Wpedantic -Weffc++ -Wconversion)
